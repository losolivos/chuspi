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

#ifndef _PREPAREDSTATEMENT_H
#define _PREPAREDSTATEMENT_H

#include "Define.h"
#include "MySQLFwd.h"

#include <string>

//- Class of which the instances are unique per MySQLConnection
//- access to these class objects is only done when a prepared statement task
//- is executed.
class MySQLPreparedStatement
{
    public:
        MySQLPreparedStatement(MYSQL_STMT *stmt, std::string const &pattern);
        ~MySQLPreparedStatement();

        uint32 execute(PreparedStatement *data);

        char const * queryPattern() const { return m_queryPattern.c_str(); }
        MYSQL_STMT * handle() const { return m_stmt; }

    private:
        void bindParameters(PreparedStatement *data);

        MYSQL_STMT *m_stmt;
        std::size_t m_paramCount;
        MYSQL_BIND *m_bind;
        std::string m_queryPattern;
};

#endif
