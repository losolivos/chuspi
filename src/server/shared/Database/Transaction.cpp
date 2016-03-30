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

#include "Transaction.h"
#include "Common.h"
#include "MySQLConnection.h"
#include "MySQLPreparedStatement.h"
#include "PreparedStatement.h"
#include "Log.h"

#include <ace/Assert.h>

#include <cstdarg>
#include <cstring>
#include <cstdlib>

Transaction::Transaction()
    : _cleanedUp(false)
{ }

Transaction::~Transaction()
{
    Cleanup();
}

//- Append a raw ad-hoc query to the transaction
void Transaction::Append(const char* sql)
{
    SQLElementData data;
    data.type = SQL_ELEMENT_RAW;
    data.element.query = strdup(sql);
    m_queries.push_back(data);
}

void Transaction::PAppend(const char* sql, ...)
{
    std::va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, sql);
    vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
    va_end(ap);

    Append(szQuery);
}

//- Append a prepared statement to the transaction
void Transaction::Append(PreparedStatement *stmtData)
{
    SQLElementData data;
    data.type = SQL_ELEMENT_PREPARED;
    data.element.stmtData = stmtData;
    m_queries.push_back(data);
}

void Transaction::Cleanup()
{
    // This might be called by explicit calls to Cleanup or by the auto-destructor
    if (_cleanedUp)
        return;

    while (!m_queries.empty())
    {
        SQLElementData const &data = m_queries.front();
        switch (data.type)
        {
        case SQL_ELEMENT_PREPARED:
            delete data.element.stmtData;
            break;
        case SQL_ELEMENT_RAW:
            std::free(data.element.query);
            break;
        }

        m_queries.pop_front();
    }

    _cleanedUp = true;
}

bool Transaction::execute(MySQLConnection *conn)
{
    if (m_queries.empty())
        return false;

    conn->BeginTransaction();

    for (StorageType::const_iterator itr = m_queries.begin(); itr != m_queries.end(); ++itr)
    {
        SQLElementData const& data = *itr;
        switch (itr->type)
        {
            case SQL_ELEMENT_PREPARED:
            {
                PreparedStatement *stmtData = data.element.stmtData;
                ACE_ASSERT(stmtData);
                if (!conn->Execute(stmtData))
                {
                    TC_LOG_ERROR("sql.sql", "[Warning] Transaction aborted. %u queries not executed.", (uint32)m_queries.size());
                    conn->RollbackTransaction();
                    return false;
                }
            }
            break;
            case SQL_ELEMENT_RAW:
            {
                const char* sql = data.element.query;
                ACE_ASSERT(sql);
                if (!conn->Execute(sql))
                {
                    TC_LOG_ERROR("sql.sql", "[Warning] Transaction aborted. %u queries not executed.", (uint32)m_queries.size());
                    conn->RollbackTransaction();
                    return false;
                }
            }
            break;
        }
    }

    // we might encounter errors during certain queries, and depending on the kind of error
    // we might want to restart the transaction. So to prevent data loss, we only clean up when it's all done.
    // This is done in calling functions DatabaseWorkerPool<T>::DirectCommitTransaction and TransactionTask::Execute,
    // and not while iterating over every element.

    conn->CommitTransaction();
    return true;
}
