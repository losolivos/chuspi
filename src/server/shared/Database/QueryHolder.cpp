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

#include "QueryHolder.h"
#include "Common.h"
#include "MySQLConnection.h"
#include "MySQLPreparedStatement.h"
#include "ResultSet.h"
#include "PreparedResultSet.h"
#include "PreparedStatement.h"

#include <ace/Assert.h>

#include <cstdarg>

bool SQLQueryHolder::SetQuery(size_t index, const char *sql)
{
    ACE_ASSERT(index < m_queries.size());

    /// not executed yet, just stored (it's not called a holder for nothing)
    SQLElementData element;
    element.type = SQL_ELEMENT_RAW;
    element.element.query = strdup(sql);

    SQLResultSetUnion result;
    result.qresult = NULL;

    m_queries[index] = SQLResultPair(element, result);
    return true;
}

bool SQLQueryHolder::SetPQuery(size_t index, const char *format, ...)
{
    ACE_ASSERT(format);

    std::va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int res = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    ACE_ASSERT(res != -1);
    return SetQuery(index, szQuery);
}

bool SQLQueryHolder::SetPreparedQuery(size_t index, PreparedStatement *data)
{
    ACE_ASSERT(index < m_queries.size());

    /// not executed yet, just stored (it's not called a holder for nothing)
    SQLElementData element;
    element.type = SQL_ELEMENT_PREPARED;
    element.element.stmtData = data;

    SQLResultSetUnion result;
    result.presult = NULL;

    m_queries[index] = SQLResultPair(element, result);
    return true;
}

QueryResult SQLQueryHolder::GetResult(size_t index)
{
    ACE_ASSERT(index < m_queries.size());

    ResultSet *result = m_queries[index].second.qresult;
    m_queries[index].second.qresult = NULL;

    if (!result || !result->GetRowCount())
    {
        delete result;
        return QueryResult();
    }

    result->NextRow();
    return QueryResult(result);
}

PreparedQueryResult SQLQueryHolder::GetPreparedResult(size_t index)
{
    ACE_ASSERT(index < m_queries.size());

    PreparedResultSet *result = m_queries[index].second.presult;
    m_queries[index].second.presult = NULL;

    if (!result || !result->GetRowCount())
    {
        delete result;
        return PreparedQueryResult();
    }

    return PreparedQueryResult(result);
}

SQLQueryHolder::~SQLQueryHolder()
{
    for (StorageType::const_iterator i = m_queries.begin(); i != m_queries.end(); ++i)
    {
        // if the result was never used, free the resources
        // results used already (getresult called) are expected to be deleted
        SQLElementData const &data = (*i).first;
        switch (data.type)
        {
        case SQL_ELEMENT_RAW:
            free(data.element.query);
            delete (*i).second.qresult;
            break;
        case SQL_ELEMENT_PREPARED:
            delete data.element.stmtData;
            delete (*i).second.presult;
            break;
        }
    }
}

void SQLQueryHolder::executeAll(MySQLConnection *conn)
{
    for (StorageType::iterator i = m_queries.begin(); i != m_queries.end(); ++i)
    {
        // execute all queries in the holder and pass the results
        SQLElementData const &data = (*i).first;
        switch (data.type)
        {
        case SQL_ELEMENT_RAW:
            if (char const *sql = data.element.query)
                (*i).second.qresult = conn->Query(sql);
            break;
        case SQL_ELEMENT_PREPARED:
            if (PreparedStatement *stmtData = data.element.stmtData)
                (*i).second.presult = conn->Query(stmtData);
            break;
        }
    }
}
