/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MySQLConnection.h"
#include "MySQLConnectionInfo.h"
#include "MySQLPreparedStatement.h"
#include "ResultSet.h"
#include "PreparedResultSet.h"
#include "PreparedStatement.h"
#include "Log.h"
#include "Util.h"
#include "Config.h"
#include "Common.h"

#include <ace/OS_NS_unistd.h>
#include <ace/Assert.h>
#ifdef _WIN32
  #include <winsock2.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#include <errmsg.h>

#include <cstdlib>
#include <list>

MySQLConnection::MySQLConnection()
    : m_handle(NULL)
    , m_connectionInfo()
    , m_reconnecting(false)
    , m_prepareError(false)
    , m_lastError()
{ }

MySQLConnection::~MySQLConnection()
{
    ACE_ASSERT(m_handle);
    for (size_t i = 0; i < m_stmts.size(); ++i)
        delete m_stmts[i];
    mysql_close(m_handle);
}

bool MySQLConnection::DoOpen()
{
    MYSQL *mysqlInit;
    mysqlInit = mysql_init(NULL);
    if (!mysqlInit)
    {
        TC_LOG_ERROR("sql.sql", "Could not initialize Mysql connection to database `%s`", m_connectionInfo->database.c_str());
        return false;
    }

    int port;
    char const* unix_socket;

    mysql_options(mysqlInit, MYSQL_SET_CHARSET_NAME, "utf8");

    if (m_connectionInfo->host == ".")
    {
#ifdef _WIN32
        // named pipe use option (Windows)
        unsigned int opt = MYSQL_PROTOCOL_PIPE;
        mysql_options(mysqlInit, MYSQL_OPT_PROTOCOL, (char const*)&opt);
        unix_socket = 0;
#else
        // socket use option (Unix/Linux)
        unsigned int opt = MYSQL_PROTOCOL_SOCKET;
        mysql_options(mysqlInit, MYSQL_OPT_PROTOCOL, (char const*)&opt);
        m_connectionInfo->host = "localhost";
        unix_socket = m_connectionInfo->port_or_socket.c_str();
#endif
        port = 0;
    }
    else
    {
        // generic case
        port = atoi(m_connectionInfo->port_or_socket.c_str());
        unix_socket = 0;
    }

    m_handle = mysql_real_connect(mysqlInit, m_connectionInfo->host.c_str(), m_connectionInfo->user.c_str(),
                                  m_connectionInfo->password.c_str(), m_connectionInfo->database.c_str(), port, unix_socket, 0);

    if (m_handle)
    {
        auto const version = mysql_get_server_version(m_handle);

        if (!m_reconnecting)
        {
            TC_LOG_INFO("sql.sql", "MySQL client version: %s", mysql_get_client_info());
            TC_LOG_INFO("sql.sql", "MySQL server version: %s", mysql_get_server_info(m_handle));

            if (mysql_get_server_version(m_handle) != mysql_get_client_version())
            {
                TC_LOG_INFO("sql.sql", "[WARNING] MySQL client/server version mismatch, "
                            "may conflict with behaviour of prepared statements.");
            }
        }

        // MariaDB >= 10.0.10 supports Global Transaction ID feature, that, among
        // other benefits, allows to set up parallel replication streams
        if (version >= 100010)
        {
            auto const roleName = sConfigMgr->GetStringDefault("GtidDomain.Role", "");
            if (!roleName.empty())
            {
                char str[128];

                BeginTransaction();

                snprintf(str, sizeof(str), "SET ROLE %s", roleName.c_str());
                Execute(str);

                auto const realm = sConfigMgr->GetIntDefault("RealmID", 0);
                snprintf(str, sizeof(str), "SET SESSION gtid_domain_id = %u", realm);
                Execute(str);

                Execute("SET ROLE NONE");

                CommitTransaction();
            }
        }

        TC_LOG_INFO("sql.sql", "Connected to MySQL database at %s", m_connectionInfo->host.c_str());
        mysql_autocommit(m_handle, 1);

        // set connection properties to UTF8 to properly handle locales for different
        // server configs - core sends data in UTF8, so MySQL must expect UTF8 too
        mysql_set_character_set(m_handle, "utf8");
        return PrepareStatements();
    }
    else
    {
        TC_LOG_ERROR("sql.sql", "Could not connect to MySQL database at %s: %s\n", m_connectionInfo->host.c_str(), mysql_error(mysqlInit));
        mysql_close(mysqlInit);
        return false;
    }
}

bool MySQLConnection::Open(MySQLConnectionInfo &connInfo, MySQLConnectionInitHook initHookFnPtr)
{
    m_connectionInfo = &connInfo;
    m_initHookFnPtr = initHookFnPtr;

    return DoOpen();
}

bool MySQLConnection::PrepareStatements()
{
    m_initHookFnPtr(*this);
    return !m_prepareError;
}

bool MySQLConnection::Execute(const char* sql)
{
    // Will return NULL every time for non-SELECT statements
    _Query(sql);
    return mysql_affected_rows(m_handle) != ((my_ulonglong)-1);
}

bool MySQLConnection::Execute(PreparedStatement *data)
{
    return _Query(data) != NULL;
}

ResultSet * MySQLConnection::Query(const char* sql)
{
    MYSQL_RES * const result = _Query(sql);
    return result ? new ResultSet(result) : NULL;
}

PreparedResultSet * MySQLConnection::Query(PreparedStatement *data)
{
    MYSQL_STMT *stmt = _Query(data);
    return stmt ? new PreparedResultSet(stmt) : NULL;
}

MYSQL_RES * MySQLConnection::_Query(char const *sql)
{
    TC_LOG_TRACE("sql.sql", "%s", sql);
    if (mysql_query(m_handle, sql))
    {
        uint32 const errnum = mysql_errno(m_handle);

        TC_LOG_ERROR("sql.sql", "SQL: %s", sql);
        TC_LOG_ERROR("sql.sql", "ERROR: [%u] %s", errnum, mysql_error(m_handle));

        // If it returns true, an error was handled successfully (i.e. reconnection)
        // and we try again
        if (_HandleMySQLErrno(errnum))
            return _Query(sql);

        setLastError(errnum);
        return NULL;
    }

    setLastError(0);
    return mysql_store_result(m_handle);
}

MYSQL_STMT * MySQLConnection::_Query(PreparedStatement *data)
{
    MySQLPreparedStatement *stmt = m_stmts[data->index()];
    ACE_ASSERT(stmt);
    TC_LOG_TRACE("sql.sql", "%s", stmt->queryPattern());
    if (uint32 const errnum = stmt->execute(data))
    {
        TC_LOG_ERROR("sql.sql", "SQL(p): %s\n [ERROR]: [%u] %s", stmt->queryPattern(), errnum, mysql_stmt_error(stmt->handle()));

        // If it returns true, an error was handled successfully (i.e. reconnection)
        // and we try again
        if (_HandleMySQLErrno(errnum))
            return _Query(data);

        setLastError(errnum);
        return NULL;
    }

    setLastError(0);
    return stmt->handle();
}

void MySQLConnection::BeginTransaction()
{
    Execute("START TRANSACTION");
}

void MySQLConnection::RollbackTransaction()
{
    mysql_rollback(m_handle);
    TC_LOG_TRACE("sql.sql", "ROLLBACK TRANSACTION");
}

void MySQLConnection::CommitTransaction()
{
    mysql_commit(m_handle);
    TC_LOG_TRACE("sql.sql", "COMMIT TRANSACTION");
}

void MySQLConnection::prepareStatement(uint32 index, std::string const &sql)
{
    // For reconnection case
    if (m_reconnecting)
        delete m_stmts[index];

    MYSQL_STMT *stmt = mysql_stmt_init(m_handle);
    if (!stmt)
    {
        TC_LOG_ERROR("sql.sql", "[ERROR]: In mysql_stmt_init() id: %u, sql: \"%s\"", index, sql.c_str());
        TC_LOG_ERROR("sql.sql", "[ERROR]: %s", mysql_error(m_handle));
        m_prepareError = true;
    }
    else
    {
        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) == 0)
            m_stmts[index] = new MySQLPreparedStatement(stmt, sql);
        else
        {
            TC_LOG_ERROR("sql.sql", "[ERROR]: In mysql_stmt_prepare() id: %u, sql: \"%s\"", index, sql.c_str());
            TC_LOG_ERROR("sql.sql", "[ERROR]: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            m_prepareError = true;
        }
    }
}

void MySQLConnection::setLastError(uint32 value)
{
    m_lastError = value;
}

bool MySQLConnection::_HandleMySQLErrno(uint32 errNo)
{
    switch (errNo)
    {
        case CR_SERVER_GONE_ERROR:
        case CR_SERVER_LOST:
        case CR_INVALID_CONN_HANDLE:
        case CR_SERVER_LOST_EXTENDED:
        {
            m_reconnecting = true;
            uint64 oldThreadId = mysql_thread_id(m_handle);
            mysql_close(m_handle);
            if (DoOpen())
            {
                TC_LOG_INFO("sql.sql", "Connection to the MySQL server is active.");
                if (oldThreadId != mysql_thread_id(m_handle))
                {
                    TC_LOG_INFO("sql.sql", "Successfully reconnected to %s @%s:%s.",
                                m_connectionInfo->database.c_str(), m_connectionInfo->host.c_str(), m_connectionInfo->port_or_socket.c_str());
                }
                m_reconnecting = false;
                setLastError(0);
                return true;
            }

            uint32 const lErrno = mysql_errno(m_handle);
            setLastError(lErrno);

            // It's possible this attempted reconnect throws 2006 at us. To
            // prevent crazy recursive calls, sleep here for 3 seconds.
            ACE_OS::sleep(3);
            return _HandleMySQLErrno(lErrno);
        }

        case ER_LOCK_DEADLOCK:
            return false;    // Implemented in TransactionTask::Execute and DatabaseWorkerPool<T>::DirectCommitTransaction

        // Query related errors - skip query
        case ER_WRONG_VALUE_COUNT:
        case ER_DUP_ENTRY:
            return false;

        // Outdated table or database structure - terminate core
        case ER_BAD_FIELD_ERROR:
        case ER_NO_SUCH_TABLE:
            TC_LOG_ERROR("sql.sql", "Your database structure is not up to date. Please make sure you've executed all queries in the sql/updates folders.");
            ACE_OS::sleep(10);
            std::abort();
            return false;

        default:
            TC_LOG_ERROR("sql.sql", "Unhandled MySQL errno %u. Unexpected behaviour possible.", errNo);
            return false;
    }
}

unsigned long MySQLConnection::EscapeString(char *to, const char *from, unsigned long length)
{
    return mysql_real_escape_string(m_handle, to, from, length);
}
