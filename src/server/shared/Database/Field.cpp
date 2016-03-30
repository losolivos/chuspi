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

#include "Field.h"
#include "Log.h"

#include <ace/OS_NS_stdlib.h>
#ifdef _WIN32
#  include <winsock2.h>
#endif
#include <mysql.h>

#include <cstring>

namespace {

template <typename T>
T signed_field_cast(char const *value, bool raw)
{
    if (value == NULL)
        return T(0);
    return raw
            ? *reinterpret_cast<T const *>(value)
            : static_cast<T>(ACE_OS::strtoll(value, NULL, 10));
}

template <typename T>
T unsigned_field_cast(char const *value, bool raw)
{
    if (value == NULL)
        return T(0);
    return raw
            ? *reinterpret_cast<T const *>(value)
            : static_cast<T>(ACE_OS::strtoull(value, NULL, 10));
}

} // namespace

Field::Field()
    : m_data()
{ }

Field::~Field()
{
    CleanUp();
}

void Field::SetByteValue(char const* newValue, size_t newSize, int32 newType)
{
    if (m_data.value)
        CleanUp();

    // This value stores raw bytes that have to be explicitly casted later
    if (newValue)
    {
        switch (enum_field_types(newType))
        {
            case MYSQL_TYPE_TINY_BLOB:
            case MYSQL_TYPE_MEDIUM_BLOB:
            case MYSQL_TYPE_LONG_BLOB:
            case MYSQL_TYPE_BLOB:
            case MYSQL_TYPE_STRING:
            case MYSQL_TYPE_VAR_STRING:
            case MYSQL_TYPE_BIT:
                m_data.value = new char[newSize + 1];
                m_data.value[newSize] = '\0';
                break;
            default:
                m_data.value = new char[newSize];
                break;
        }

        std::memcpy(m_data.value, newValue, newSize);
        m_data.size = newSize;
    }

    m_data.type = newType;
    m_data.raw = true;
}

void Field::SetStructuredValue(char const *newValue, size_t newSize, int32 newType)
{
    if (m_data.value)
        CleanUp();

    // This value stores somewhat structured data that needs function style casting
    if (newValue)
    {
        m_data.size = newSize;
        m_data.value = new char[m_data.size + 1];
        std::memcpy(m_data.value, newValue, m_data.size + 1);
    }

    m_data.type = newType;
    m_data.raw = false;
}

uint8 Field::GetUInt8() const
{
    return unsigned_field_cast<uint8>(m_data.value, m_data.raw);
}

int8 Field::GetInt8() const
{
    return signed_field_cast<int8>(m_data.value, m_data.raw);
}

uint16 Field::GetUInt16() const
{
    return unsigned_field_cast<uint16>(m_data.value, m_data.raw);
}

int16 Field::GetInt16() const
{
    return signed_field_cast<int16>(m_data.value, m_data.raw);
}

uint32 Field::GetUInt32() const
{
    return unsigned_field_cast<uint32>(m_data.value, m_data.raw);
}

int32 Field::GetInt32() const
{
    return signed_field_cast<int32>(m_data.value, m_data.raw);
}

uint64 Field::GetUInt64() const
{
    return unsigned_field_cast<uint64>(m_data.value, m_data.raw);
}

int64 Field::GetInt64() const
{
    return signed_field_cast<int64>(m_data.value, m_data.raw);
}

double Field::GetDouble() const
{
    if (m_data.value == NULL)
        return 0.0;
    return m_data.raw
            ? *reinterpret_cast<double const *>(m_data.value)
            : static_cast<double>(ACE_OS::strtod(m_data.value, NULL));
}

float Field::GetFloat() const
{
    if (m_data.value == NULL)
        return 0.0f;
    return m_data.raw
            ? *reinterpret_cast<float const *>(m_data.value)
            : static_cast<float>(ACE_OS::strtod(m_data.value, NULL));
}

std::string Field::GetString() const
{
    return m_data.value
            ? std::string(m_data.value, m_data.size)
            : "";
}

void Field::CleanUp()
{
    delete[] m_data.value;
    m_data.value = NULL;
}
