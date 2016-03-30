/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef TRINITY_OBJECTGUID_H
#define TRINITY_OBJECTGUID_H

#include "Define.h"

//! Class to ease conversions from single 64 bit integer guid into individual bytes, for packet sending purposes
//! Nuke this out when porting ObjectGuid from MaNGOS, but preserve the per-byte storage
class ObjectGuid
{
    union DataUnion
    {
        uint64 u64;
        uint8 byte[sizeof(uint64)];
    };

public:
    ObjectGuid();

    ObjectGuid(uint64 guid);

    ObjectGuid(ObjectGuid const& other);

    ObjectGuid & operator=(uint64 guid);

    ObjectGuid & operator=(ObjectGuid const& other);

    uint8 & operator[](uint32 index);

    uint8 const & operator[](uint32 index) const;

    operator uint64() const;

private:
    DataUnion _data;
};

inline ObjectGuid::ObjectGuid()
    : _data()
{ }

inline ObjectGuid::ObjectGuid(uint64 guid)
{
    _data.u64 = guid;
}

inline ObjectGuid::ObjectGuid(ObjectGuid const& other)
{
    _data.u64 = other._data.u64;
}

inline ObjectGuid & ObjectGuid::operator=(uint64 guid)
{
    _data.u64 = guid;
    return *this;
}

inline ObjectGuid & ObjectGuid::operator=(ObjectGuid const& other)
{
    _data.u64 = other._data.u64;
    return *this;
}

inline uint8 & ObjectGuid::operator[](uint32 index)
{
#if TRINITY_ENDIAN == TRINITY_LITTLEENDIAN
    return _data.byte[index];
#else
    return _data.byte[sizeof(_data.byte) - 1 - index];
#endif
}

inline uint8 const & ObjectGuid::operator[](uint32 index) const
{
#if TRINITY_ENDIAN == TRINITY_LITTLEENDIAN
    return _data.byte[index];
#else
    return _data.byte[sizeof(_data.byte) - 1 - index];
#endif
}

inline ObjectGuid::operator uint64() const
{
    return _data.u64;
}

#endif // TRINITY_OBJECTGUID_H
