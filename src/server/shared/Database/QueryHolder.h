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

#ifndef _QUERYHOLDER_H
#define _QUERYHOLDER_H

#include "MySQLFwd.h"
#include "MySQLDataTypes.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

typedef std::shared_ptr<ResultSet> QueryResult;
typedef std::shared_ptr<PreparedResultSet> PreparedQueryResult;

class SQLQueryHolder
{
    typedef std::pair<SQLElementData, SQLResultSetUnion> SQLResultPair;
    typedef std::vector<SQLResultPair> StorageType;

public:
    ~SQLQueryHolder();

    void executeAll(MySQLConnection *conn);

    bool SetQuery(std::size_t index, const char *sql);
    bool SetPQuery(std::size_t index, const char *format, ...);
    bool SetPreparedQuery(std::size_t index, PreparedStatement *data);

    void SetSize(std::size_t size) { m_queries.resize(size); }

    QueryResult GetResult(std::size_t index);
    PreparedQueryResult GetPreparedResult(std::size_t index);

private:
    StorageType m_queries;
};

#endif
