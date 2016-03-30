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

#ifndef _MYSQLCONNECTION_H
#define _MYSQLCONNECTION_H

#include "Define.h"
#include "MySQLFwd.h"

#include <string>
#include <vector>

class MySQLConnection
{
public:
    MySQLConnection();
    ~MySQLConnection();

    bool Open(MySQLConnectionInfo &connInfo, MySQLConnectionInitHook initHookFnPtr);

    bool reconnecting() const { return m_reconnecting; }

    void setMaxPreparedStatements(size_t sz) { m_stmts.resize(sz); }
    void prepareStatement(uint32 index, std::string const &sql);

    bool Execute(const char* sql);
    bool Execute(PreparedStatement *data);

    ResultSet * Query(const char* sql);
    PreparedResultSet * Query(PreparedStatement *data);

    void BeginTransaction();
    void RollbackTransaction();
    void CommitTransaction();

    unsigned long EscapeString(char *to, const char *from, unsigned long length);

    uint32 GetLastError() const;

private:
    MYSQL_RES * _Query(char const *sql);
    MYSQL_STMT * _Query(PreparedStatement *data);

    bool PrepareStatements();

    bool DoOpen();

    void setLastError(uint32 value);

    bool _HandleMySQLErrno(uint32 errNo);

    mutable MYSQL *m_handle;                //! MySQL Handle.
    MySQLConnectionInfo *m_connectionInfo;  //! Connection info (used for logging)

    MySQLConnectionInitHook m_initHookFnPtr;

    std::vector<MySQLPreparedStatement*> m_stmts;   //! PreparedStatements storage
    bool m_reconnecting;                            //! Are we reconnecting?
    bool m_prepareError;                            //! Was there any error while preparing statements?

    uint32 m_lastError;
};

inline uint32 MySQLConnection::GetLastError() const
{
    return m_lastError;
}

#endif
