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

#include "MySQLPreparedStatement.h"
#include "PreparedStatement.h"

#include <ace/Assert.h>
#ifdef _WIN32
  #include <winsock2.h>
#endif
#include <mysql.h>

#include <cstring>

namespace {

void setNumericHelper(MYSQL_BIND *param, enum_field_types type, void const *value, std::size_t len, bool isUnsigned)
{
    param->buffer_type = type;
    delete[] static_cast<char *>(param->buffer);
    param->buffer = new char[len];
    param->buffer_length = 0;
    param->is_null_value = 0;
    param->length = NULL;
    param->is_unsigned = isUnsigned;

    std::memcpy(param->buffer, value, len);
}

void setStringHelper(MYSQL_BIND *param, char const *value, std::size_t len)
{
    param->buffer_type = MYSQL_TYPE_VAR_STRING;
    delete[] static_cast<char *>(param->buffer);
    param->buffer = new char[len];
    param->buffer_length = len;
    param->is_null_value = 0;
    param->length = NULL;

    std::memcpy(param->buffer, value, len);
}

void setBinaryHelper(MYSQL_BIND *param, uint8 const *value, std::size_t size)
{
    param->buffer_type = MYSQL_TYPE_MEDIUM_BLOB;
    delete[] static_cast<char *>(param->buffer);
    param->buffer = new char[size];
    param->buffer_length = size;
    param->is_null_value = 0;
    param->length = NULL;

    std::memcpy(param->buffer, value, size);
}

void setNullHelper(MYSQL_BIND *param)
{
    param->buffer_type = MYSQL_TYPE_NULL;
}

} // namespace

MySQLPreparedStatement::MySQLPreparedStatement(MYSQL_STMT *stmt, const std::string &pattern)
    : m_stmt(stmt)
    , m_paramCount(mysql_stmt_param_count(m_stmt))
    , m_bind(m_paramCount ? new MYSQL_BIND[m_paramCount]() : NULL)
    , m_queryPattern(pattern)
{ }

MySQLPreparedStatement::~MySQLPreparedStatement()
{
    if (m_bind)
    {
        for (std::size_t i = 0; i < m_paramCount; ++i)
            delete[] static_cast<char *>(m_bind[i].buffer);
        delete[] m_bind;
    }
    mysql_stmt_close(m_stmt);
}

uint32 MySQLPreparedStatement::execute(PreparedStatement *data)
{
    if (m_paramCount)
    {
        bindParameters(data);
        if (mysql_stmt_bind_param(m_stmt, m_bind) != 0)
            return mysql_stmt_errno(m_stmt);
    }

    return (mysql_stmt_execute(m_stmt) != 0)
            ? mysql_stmt_errno(m_stmt) : 0;
}

void MySQLPreparedStatement::bindParameters(PreparedStatement *data)
{
    ACE_ASSERT(m_paramCount == data->paramCount());

    MYSQL_BIND *bindPtr = m_bind;

    for (PreparedStatement::const_iterator i = data->begin(); i != data->end(); ++i)
    {
        PreparedStatement::ValueType const &field = *i;
        switch (field.type)
        {
            case PreparedStatement::TYPE_BOOL:
            case PreparedStatement::TYPE_UI8:
                setNumericHelper(bindPtr++, MYSQL_TYPE_TINY, &field.num.as_uint8, sizeof(uint8), true);
                break;
            case PreparedStatement::TYPE_I8:
                setNumericHelper(bindPtr++, MYSQL_TYPE_TINY, &field.num.as_int8, sizeof(int8), false);
                break;
            case PreparedStatement::TYPE_UI16:
                setNumericHelper(bindPtr++, MYSQL_TYPE_SHORT, &field.num.as_uint16, sizeof(uint16), true);
                break;
            case PreparedStatement::TYPE_I16:
                setNumericHelper(bindPtr++, MYSQL_TYPE_SHORT, &field.num.as_int16, sizeof(int16), false);
                break;
            case PreparedStatement::TYPE_UI32:
                setNumericHelper(bindPtr++, MYSQL_TYPE_LONG, &field.num.as_uint32, sizeof(uint32), true);
                break;
            case PreparedStatement::TYPE_I32:
                setNumericHelper(bindPtr++, MYSQL_TYPE_LONG, &field.num.as_int32, sizeof(int32), false);
                break;
            case PreparedStatement::TYPE_UI64:
                setNumericHelper(bindPtr++, MYSQL_TYPE_LONGLONG, &field.num.as_uint64, sizeof(uint64), true);
                break;
            case PreparedStatement::TYPE_I64:
                setNumericHelper(bindPtr++, MYSQL_TYPE_LONGLONG, &field.num.as_int64, sizeof(int64), false);
                break;
            case PreparedStatement::TYPE_FLOAT:
                setNumericHelper(bindPtr++, MYSQL_TYPE_FLOAT, &field.num.as_float, sizeof(float), false);
                break;
            case PreparedStatement::TYPE_DOUBLE:
                setNumericHelper(bindPtr++, MYSQL_TYPE_DOUBLE, &field.num.as_double, sizeof(double), false);
                break;
            case PreparedStatement::TYPE_STRING:
                setStringHelper(bindPtr++, field.str.c_str(), field.str.length());
                break;
            case PreparedStatement::TYPE_BINARY:
                setBinaryHelper(bindPtr++, &field.bin[0], field.bin.size());
                break;
            case PreparedStatement::TYPE_NULL:
                setNullHelper(bindPtr++);
                break;
        }
    }
}
