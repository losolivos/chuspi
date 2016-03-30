#include "DatabaseWorkerPool.h"
#include "Common.h"
#include "DatabaseWorker.h"
#include "MySQLConnection.h"
#include "MySQLHelper.h"
#include "MySQLPreparedStatement.h"
#include "PreparedResultSet.h"
#include "PreparedStatement.h"
#include "QueryHolder.h"
#include "ResultSet.h"
#include "SQLOperation.h"
#include "Transaction.h"
#include "Log.h"

#include <ace/Assert.h>
#include <mysqld_error.h>

#include <cstdarg>
#include <string>

namespace {

class DirectQueryTask : public SQLOperation
{
public:
    DirectQueryTask(char const *sql)
        : m_sql(sql)
    { }

private:
    void executeImpl(MySQLConnection *conn)
    {
        conn->Execute(m_sql.c_str());
    }

    std::string m_sql;
};

class AsyncQueryTask : public SQLOperation
{
public:
    AsyncQueryTask(const char* sql, QueryResultFuture &result)
        : m_sql(sql)
        , m_result(result)
    { }

private:
    void executeImpl(MySQLConnection *conn)
    {
        ResultSet *result = conn->Query(m_sql.c_str());
        if (!result || !result->GetRowCount())
        {
            delete result;
            m_result.set(QueryResult());
        }
        else
        {
            result->NextRow();
            m_result.set(QueryResult(result));
        }
    }

    std::string m_sql;
    QueryResultFuture m_result;
};

class DirectPreparedStatementTask : public SQLOperation
{
public:
    DirectPreparedStatementTask(PreparedStatement *data)
        : m_data(data)
    { }

    ~DirectPreparedStatementTask() { delete m_data; }

private:
    void executeImpl(MySQLConnection *conn)
    {
        conn->Execute(m_data);
    }

    PreparedStatement *m_data;
};

class AsyncPreparedStatementTask : public SQLOperation
{
public:
    AsyncPreparedStatementTask(PreparedStatement *data, PreparedQueryResultFuture result)
        : m_data(data)
        , m_result(result)
    { }

    ~AsyncPreparedStatementTask() { delete m_data; }

private:
    void executeImpl(MySQLConnection *conn)
    {
        PreparedResultSet *result = conn->Query(m_data);
        if (!result || !result->GetRowCount())
            m_result.set(PreparedQueryResult());
        else
            m_result.set(PreparedQueryResult(result));
    }

    PreparedStatement *m_data;
    PreparedQueryResultFuture m_result;
};

class SQLQueryHolderTask : public SQLOperation
{
public:
    SQLQueryHolderTask(SQLQueryHolder *holder, QueryResultHolderFuture res)
        : m_holder(holder)
        , m_result(res)
    { }

private:
    void executeImpl(MySQLConnection *conn)
    {
        m_holder->executeAll(conn);
        m_result.set(m_holder);
    }

    SQLQueryHolder *m_holder;
    QueryResultHolderFuture m_result;
};

class TransactionTask : public SQLOperation
{
public:
    TransactionTask(SQLTransaction trans)
        : m_trans(trans)
    { }

private:
    void executeImpl(MySQLConnection *conn)
    {
        if (m_trans->execute(conn))
            return;

        switch (conn->GetLastError())
        {
        case ER_LOCK_DEADLOCK:
        case ER_KEY_NOT_FOUND:
        {
            // Handle MySQL Errno 1213 without extending deadlock to the core itself
            for (uint8 i = 0; i < 5; ++i)
                if (m_trans->execute(conn))
                    return;
            break;
        }
        default:
            break;
        }

        // Clean up now.
        m_trans->Cleanup();
    }

    SQLTransaction m_trans;
};

} // namespace

DatabaseWorkerPool::DatabaseWorkerPool()
    : m_tssConn(new DbConnectionTSS)
    , m_asyncWorker(NULL)
{
    ACE_ASSERT(MySQLHelper::libraryThreadSafe());
}

bool DatabaseWorkerPool::Open(std::string const &infoString, uint8 numThreads, MySQLConnectionInitHook initHookFnPtr)
{
    m_connectionInfo = MySQLConnectionInfo(infoString);
    m_initHookFnPtr = initHookFnPtr;
    m_asyncWorker = new DatabaseWorker(m_connectionInfo, numThreads, m_initHookFnPtr);
    TC_LOG_INFO("sql.sql", "Opening databasepool '%s'. %u async connections running.", m_connectionInfo.database.c_str(), numThreads);
    return true;
}

void DatabaseWorkerPool::Close()
{
    TC_LOG_INFO("sql.sql", "Closing down databasepool '%s'.", m_connectionInfo.database.c_str());
    delete m_tssConn;
    delete m_asyncWorker;
}

void DatabaseWorkerPool::Execute(char const *sql)
{
    if (sql)
        Enqueue(new DirectQueryTask(sql));
}

void DatabaseWorkerPool::PExecute(char const *sql, ...)
{
    if (!sql)
        return;

    std::va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, sql);
    vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
    va_end(ap);

    Execute(szQuery);
}

void DatabaseWorkerPool::Execute(PreparedStatement *data)
{
    Enqueue(new DirectPreparedStatementTask(data));
}

void DatabaseWorkerPool::DirectExecute(char const *sql)
{
    if (sql)
        GetConnection()->Execute(sql);
}

void DatabaseWorkerPool::DirectPExecute(char const *sql, ...)
{
    if (!sql)
        return;

    std::va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, sql);
    vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
    va_end(ap);

    return DirectExecute(szQuery);
}

void DatabaseWorkerPool::DirectExecute(PreparedStatement *data)
{
    GetConnection()->Execute(data);
    delete data;
}

QueryResult DatabaseWorkerPool::Query(char const *sql)
{
    ResultSet *result = GetConnection()->Query(sql);
    if (!result || !result->GetRowCount())
    {
        delete result;
        return QueryResult();
    }
    result->NextRow();
    return QueryResult(result);
}

QueryResult DatabaseWorkerPool::PQuery(char const *sql, ...)
{
    if (!sql)
        return QueryResult();

    std::va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, sql);
    vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
    va_end(ap);

    return Query(szQuery);
}

PreparedQueryResult DatabaseWorkerPool::Query(PreparedStatement *data)
{
    PreparedResultSet *ret = GetConnection()->Query(data);
    delete data;

    if (!ret || !ret->GetRowCount())
    {
        delete ret;
        return PreparedQueryResult();
    }

    return PreparedQueryResult(ret);
}

QueryResultFuture DatabaseWorkerPool::AsyncQuery(char const *sql)
{
    QueryResultFuture res;
    Enqueue(new AsyncQueryTask(sql, res));
    return res;         //! Actual return value has no use yet
}

QueryResultFuture DatabaseWorkerPool::AsyncPQuery(char const *sql, ...)
{
    std::va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, sql);
    vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
    va_end(ap);

    return AsyncQuery(szQuery);
}

PreparedQueryResultFuture DatabaseWorkerPool::AsyncQuery(PreparedStatement *data)
{
    PreparedQueryResultFuture res;
    Enqueue(new AsyncPreparedStatementTask(data, res));
    return res;
}

QueryResultHolderFuture DatabaseWorkerPool::DelayQueryHolder(SQLQueryHolder *holder)
{
    QueryResultHolderFuture res;
    Enqueue(new SQLQueryHolderTask(holder, res));
    return res;
}

SQLTransaction DatabaseWorkerPool::BeginTransaction()
{
    return std::make_shared<Transaction>();
}

void DatabaseWorkerPool::CommitTransaction(SQLTransaction transaction)
{
    Enqueue(new TransactionTask(transaction));
}

void DatabaseWorkerPool::DirectCommitTransaction(SQLTransaction transaction)
{
    MySQLConnection *conn = GetConnection();
    if (transaction->execute(conn))
        return;

    switch (conn->GetLastError())
    {
    case ER_LOCK_DEADLOCK:
    case ER_KEY_NOT_FOUND:
    {
        // Handle MySQL Errno 1213 without extending deadlock to the core itself
        for (uint8 i = 0; i < 5; ++i)
            if (transaction->execute(conn))
                return;
        break;
    }
    default:
        break;
    }


    // Clean up now.
    transaction->Cleanup();
}

void DatabaseWorkerPool::ExecuteOrAppend(SQLTransaction trans, PreparedStatement *data)
{
    if (trans)
        trans->Append(data);
    else
        Execute(data);
}

void DatabaseWorkerPool::ExecuteOrAppend(SQLTransaction trans, char const*sql)
{
    if (trans)
        trans->Append(sql);
    else
        Execute(sql);
}

PreparedStatement * DatabaseWorkerPool::GetPreparedStatement(uint32 index)
{
    return new PreparedStatement(index);
}

void DatabaseWorkerPool::EscapeString(std::string &str)
{
    if (str.empty())
        return;

    char* buf = new char[str.size()*2+1];
    GetConnection()->EscapeString(buf, str.c_str(), str.size());
    str = buf;
    delete[] buf;
}

void DatabaseWorkerPool::Enqueue(SQLOperation *op)
{
    if (m_asyncWorker->enqueue(op) == -1)
        delete op;
}

MySQLConnection * DatabaseWorkerPool::GetConnection()
{
    MySQLConnection *conn = m_tssConn->ts_object();
    if (!conn)
    {
        conn = new MySQLConnection;
        conn->Open(m_connectionInfo, m_initHookFnPtr);
        m_tssConn->ts_object(conn);
    }
    return conn;
}
