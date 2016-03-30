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

#ifndef _DATABASEWORKERPOOL_H
#define _DATABASEWORKERPOOL_H

#include "Define.h"
#include "MySQLFwd.h"
#include "MySQLPtrTypesFwd.h"
#include "MySQLConnectionInfo.h"

#include <ace/Future.h>
#include <ace/TSS_T.h>

#include <memory>
#include <string>

typedef ACE_Future<QueryResult> QueryResultFuture;
typedef ACE_Future<PreparedQueryResult> PreparedQueryResultFuture;
typedef ACE_Future<SQLQueryHolder *> QueryResultHolderFuture;

class DatabaseWorkerPool
{
    typedef ACE_TSS<MySQLConnection> DbConnectionTSS;

public:
    DatabaseWorkerPool();

    bool Open(std::string const &infoString, uint8 numThreads, MySQLConnectionInitHook initHookFnPtr);
    void Close();

    /**
      * Delayed one-way statement methods.
      */

    //! Enqueues a one-way SQL operation in string format that will be executed asynchronously.
    void Execute(char const *sql);

    //! Enqueues a one-way SQL operation in string format -with variable args- that will be executed asynchronously.
    void PExecute(char const *sql, ...);

    //! Enqueues a one-way SQL operation in prepared statement format that will be executed asynchronously.
    void Execute(PreparedStatement *data);

    /**
      * Direct syncrhonous one-way statement methods.
      */

    //! Directly executes a one-way SQL operation in string format, that will block the calling thread until finished.
    void DirectExecute(char const *sql);

    //! Directly executes a one-way SQL operation in string format -with variable args-, that will block the calling thread until finished.
    void DirectPExecute(char const *sql, ...);

    //! Directly executes a one-way SQL operation in prepared statement format, that will block the calling thread until finished.
    void DirectExecute(PreparedStatement *data);

    /**
      * Syncrhonous query (with resultset) methods.
      */

    //! Directly executes an SQL query in string format that will block the calling thread until finished.
    //! Returns reference counted auto pointer, no need for manual memory management in upper level code.
    QueryResult Query(char const *sql);

    //! Directly executes an SQL query in string format -with variable args- that will block the calling thread until finished.
    //! Returns reference counted auto pointer, no need for manual memory management in upper level code.
    QueryResult PQuery(char const *sql, ...);

    //! Directly executes an SQL query in prepared format that will block the calling thread until finished.
    //! Returns reference counted auto pointer, no need for manual memory management in upper level code.
    PreparedQueryResult Query(PreparedStatement *data);

    /**
      * Asynchronous query (with resultset) methods.
      */

    //! Enqueues a query in string format that will set the value of the QueryResultFuture return object as soon as the query is executed.
    //! The return value is then processed in ProcessQueryCallback methods.
    QueryResultFuture AsyncQuery(char const *sql);

    //! Enqueues a query in string format -with variable args- that will set the value of the QueryResultFuture return object as soon as the query is executed.
    //! The return value is then processed in ProcessQueryCallback methods.
    QueryResultFuture AsyncPQuery(char const *sql, ...);

    //! Enqueues a query in prepared format that will set the value of the PreparedQueryResultFuture return object as soon as the query is executed.
    //! The return value is then processed in ProcessQueryCallback methods.
    PreparedQueryResultFuture AsyncQuery(PreparedStatement *data);

    //! Enqueues a vector of SQL operations (can be both adhoc and prepared) that will set the value of the QueryResultHolderFuture
    //! return object as soon as the query is executed.
    //! The return value is then processed in ProcessQueryCallback methods.
    QueryResultHolderFuture DelayQueryHolder(SQLQueryHolder *holder);

    /**
      * Transaction context methods.
      */

    //! Begins an automanaged transaction pointer that will automatically rollback if not commited. (Autocommit=0)
    SQLTransaction BeginTransaction();

    //! Enqueues a collection of one-way SQL operations (can be both adhoc and prepared). The order in which these operations
    //! were appended to the transaction will be respected during execution.
    void CommitTransaction(SQLTransaction transaction);

    //! Directly executes a collection of one-way SQL operations (can be both adhoc and prepared). The order in which these operations
    //! were appended to the transaction will be respected during execution.
    void DirectCommitTransaction(SQLTransaction transaction);

    //! Method used to execute prepared statements in a diverse context.
    //! Will be wrapped in a transaction if valid object is present, otherwise executed standalone.
    void ExecuteOrAppend(SQLTransaction trans, PreparedStatement *data);

    //! Method used to execute ad-hoc statements in a diverse context.
    //! Will be wrapped in a transaction if valid object is present, otherwise executed standalone.
    void ExecuteOrAppend(SQLTransaction trans, char const *sql);

    /**
      * Other
      */

    //! Automanaged (internally) pointer to a prepared statement object for usage in upper level code.
    //! This object is not tied to the prepared statement on the MySQL context yet until execution.
    PreparedStatement * GetPreparedStatement(uint32 index);

    //! Apply escape string'ing for current collation. (utf8)
    void EscapeString(std::string &str);

private:
    void Enqueue(SQLOperation *op);

    MySQLConnection * GetConnection();

    MySQLConnectionInfo m_connectionInfo;
    MySQLConnectionInitHook m_initHookFnPtr;

    DbConnectionTSS *m_tssConn;           //! Holds a mysql connection per thread.
    DatabaseWorker *m_asyncWorker;        //! Async connection pool.
};

#endif
