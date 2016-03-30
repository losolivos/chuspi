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

#ifndef _FIELD_H
#define _FIELD_H

#include "Define.h"
#include "MySQLFwd.h"

#include <string>

class Field
{
    struct InternalDataRep
    {
        std::size_t size;
        int32 type;             // Field type
        char *value;            // Actual data in memory
        uint8 raw;              // Raw bytes? (Prepared statement or ad hoc)
    };

public:
    Field();
    ~Field();

    void SetByteValue(char const *newValue, size_t newSize, int32 newType);
    void SetStructuredValue(char const *newValue, size_t newSize, int32 newType);

    bool IsNull() const { return m_data.value == NULL; }

    bool GetBool() const { return GetUInt8() == 1; }

    uint8 GetUInt8() const;
    int8 GetInt8() const;

    uint16 GetUInt16() const;
    int16 GetInt16() const;

    uint32 GetUInt32() const;
    int32 GetInt32() const;

    uint64 GetUInt64() const;
    int64 GetInt64() const;

    double GetDouble() const;
    float GetFloat() const;

    char const * GetCString() const { return m_data.value; }

    std::string GetString() const;

private:
    void CleanUp();

    InternalDataRep m_data;
};

#endif

