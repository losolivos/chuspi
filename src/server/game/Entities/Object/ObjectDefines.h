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

#ifndef TRINITY_OBJECTDEFINES_H
#define TRINITY_OBJECTDEFINES_H

#include "Define.h"

enum HighGuid
{
    HIGHGUID_ITEM               = 0x400,                        // blizz 4000
    HIGHGUID_CONTAINER          = 0x400,                        // blizz 4000
    HIGHGUID_PLAYER             = 0x018,                        // blizz 0018
    HIGHGUID_GAMEOBJECT         = 0xF11,                        // blizz F110
    HIGHGUID_TRANSPORT          = 0xF12,                        // blizz F120 (for GAMEOBJECT_TYPE_TRANSPORT)
    HIGHGUID_UNIT               = 0xF13,                        // blizz F130
    HIGHGUID_PET                = 0xF14,                        // blizz F140
    HIGHGUID_VEHICLE            = 0xF15,                        // blizz F550
    HIGHGUID_DYNAMICOBJECT      = 0xF10,                        // blizz F100
    HIGHGUID_CORPSE             = 0xF101,                       // blizz F100
    HIGHGUID_AREATRIGGER        = 0xF1B,                        // blizz F100
    HIGHGUID_BATTLEGROUND       = 0x1F1,                       // new 4.x
    HIGHGUID_MO_TRANSPORT       = 0x1FC,                        // blizz 1FC0 (for GAMEOBJECT_TYPE_MO_TRANSPORT)
    HIGHGUID_GROUP              = 0x1F5,
    HIGHGUID_GUILD              = 0x1FF,                        // new 4.x
    HIGHGUID_INSTANCE_SAVE      = 0x104,                        // new 5.x
    HIGHGUID_LOOT               = 0xF19                         // new 5.4.x
};

// used for creating values for respawn for example
uint64 MAKE_PAIR64(uint32 l, uint32 h);
uint32 PAIR64_HIPART(uint64 x);
uint32 PAIR64_LOPART(uint64 x);
uint16 MAKE_PAIR16(uint8 l, uint8 h);
uint32 MAKE_PAIR32(uint16 l, uint16 h);
uint16 PAIR32_HIPART(uint32 x);
uint16 PAIR32_LOPART(uint32 x);

bool IS_EMPTY_GUID(uint64 guid);
bool IS_CREATURE_GUID(uint64 guid);
bool IS_PET_GUID(uint64 guid);
bool IS_VEHICLE_GUID(uint64 guid);
bool IS_CRE_OR_VEH_GUID(uint64 guid);
bool IS_CRE_OR_VEH_OR_PET_GUID(uint64 guid);
bool IS_PLAYER_GUID(uint64 guid);
bool IS_GUILD_GUID(uint64 guid);
bool IS_UNIT_GUID(uint64 guid);
bool IS_ITEM_GUID(uint64 guid);
bool IS_GAMEOBJECT_GUID(uint64 guid);
bool IS_DYNAMICOBJECT_GUID(uint64 guid);
bool IS_CORPSE_GUID(uint64 guid);
bool IS_TRANSPORT_GUID(uint64 guid);
bool IS_MO_TRANSPORT_GUID(uint64 guid);
bool IS_GROUP_GUID(uint64 guid);
bool IS_AREATRIGGER_GUID(uint64 guid);

// l - OBJECT_FIELD_GUID
// e - OBJECT_FIELD_ENTRY for GO (except GAMEOBJECT_TYPE_MO_TRANSPORT) and creatures or UNIT_FIELD_PETNUMBER for pets
// h - OBJECT_FIELD_GUID + 1
uint64 MAKE_NEW_GUID(uint32 l, uint32 e, uint32 h);

//#define GUID_HIPART(x)   (uint32)((uint64(x) >> 52)) & 0x0000FFFF)
uint32 GUID_HIPART(uint64 guid);
uint32 GUID_ENPART(uint64 x);
uint32 GUID_LOPART(uint64 x);

bool IsGuidHaveEnPart(uint64 guid);
char const* GetLogNameForGuid(uint64 guid);

inline uint64 MAKE_PAIR64(uint32 l, uint32 h)
{
    return uint64(l | (uint64(h) << 32));
}

inline uint32 PAIR64_HIPART(uint64 x)
{
    return (uint32)((x >> 32) & UI64LIT(0x00000000FFFFFFFF));
}

inline uint32 PAIR64_LOPART(uint64 x)
{
    return (uint32)(x & UI64LIT(0x00000000FFFFFFFF));
}

inline uint16 MAKE_PAIR16(uint8 l, uint8 h)
{
    return uint16(l | (uint16(h) << 8));
}

inline uint32 MAKE_PAIR32(uint16 l, uint16 h)
{
    return uint32(l | (uint32(h) << 16));
}

inline uint16 PAIR32_HIPART(uint32 x)
{
    return (uint16)((x >> 16) & 0x0000FFFF);
}

inline uint16 PAIR32_LOPART(uint32 x)
{
    return (uint16)(x & 0x0000FFFF);
}

inline bool IS_EMPTY_GUID(uint64 guid)
{
    return guid == 0;
}

inline bool IS_CREATURE_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_UNIT;
}

inline bool IS_PET_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_PET;
}

inline bool IS_VEHICLE_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_VEHICLE;
}

inline bool IS_CRE_OR_VEH_GUID(uint64 guid)
{
    return IS_CREATURE_GUID(guid) || IS_VEHICLE_GUID(guid);
}

inline bool IS_CRE_OR_VEH_OR_PET_GUID(uint64 guid)
{
    return IS_CRE_OR_VEH_GUID(guid) || IS_PET_GUID(guid);
}

inline bool IS_PLAYER_GUID(uint64 guid)
{
    return guid != 0 && GUID_HIPART(guid) == HIGHGUID_PLAYER;
}

inline bool IS_GUILD_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_GUILD;
}

inline bool IS_UNIT_GUID(uint64 guid)
{
    return IS_CRE_OR_VEH_OR_PET_GUID(guid) || IS_PLAYER_GUID(guid);
}

inline bool IS_ITEM_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_ITEM;
}

inline bool IS_GAMEOBJECT_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_GAMEOBJECT;
}

inline bool IS_DYNAMICOBJECT_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_DYNAMICOBJECT;
}

inline bool IS_CORPSE_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_CORPSE;
}

inline bool IS_TRANSPORT_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_TRANSPORT;
}

inline bool IS_MO_TRANSPORT_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_MO_TRANSPORT;
}

inline bool IS_GROUP_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_GROUP;
}

inline bool IS_AREATRIGGER_GUID(uint64 guid)
{
    return GUID_HIPART(guid) == HIGHGUID_AREATRIGGER;
}

inline uint64 MAKE_NEW_GUID(uint32 l, uint32 e, uint32 h)
{
    return uint64(uint64(l) | (uint64(e) << 32) | (uint64(h) << ((h == HIGHGUID_CORPSE || h == HIGHGUID_AREATRIGGER) ? 48 : 52)));
}

inline uint32 GUID_HIPART(uint64 guid)
{
    uint32 t = ((uint64(guid) >> 48) & 0x0000FFFF);
    return (t == HIGHGUID_CORPSE || t == HIGHGUID_AREATRIGGER) ? t : ((t >> 4) & 0x00000FFF);
}

inline uint32 GUID_ENPART(uint64 x)
{
    return IsGuidHaveEnPart(x)
            ? ((uint32)((x >> 32) & UI64LIT(0x00000000000FFFFF)))
            : 0;
}

inline uint32 GUID_LOPART(uint64 x)
{
    // _GUID_LOPART_3 and _GUID_LOPART_2 were both equal to PAIR64_LOPART
    return PAIR64_LOPART(x);
}

inline bool IsGuidHaveEnPart(uint64 guid)
{
    switch (GUID_HIPART(guid))
    {
        case HIGHGUID_ITEM:
        case HIGHGUID_PLAYER:
        case HIGHGUID_DYNAMICOBJECT:
        case HIGHGUID_CORPSE:
        case HIGHGUID_GROUP:
        case HIGHGUID_GUILD:
            return false;
        case HIGHGUID_GAMEOBJECT:
        case HIGHGUID_TRANSPORT:
        case HIGHGUID_UNIT:
        case HIGHGUID_PET:
        case HIGHGUID_VEHICLE:
        case HIGHGUID_MO_TRANSPORT:
        case HIGHGUID_AREATRIGGER:
        default:
            return true;
    }
}

inline char const* GetLogNameForGuid(uint64 guid)
{
    switch (GUID_HIPART(guid))
    {
        case HIGHGUID_ITEM:         return "item";
        case HIGHGUID_PLAYER:       return guid ? "player" : "none";
        case HIGHGUID_GAMEOBJECT:   return "gameobject";
        case HIGHGUID_TRANSPORT:    return "transport";
        case HIGHGUID_UNIT:         return "creature";
        case HIGHGUID_PET:          return "pet";
        case HIGHGUID_VEHICLE:      return "vehicle";
        case HIGHGUID_DYNAMICOBJECT:return "dynobject";
        case HIGHGUID_CORPSE:       return "corpse";
        case HIGHGUID_MO_TRANSPORT: return "mo_transport";
        case HIGHGUID_GROUP:        return "group";
        case HIGHGUID_GUILD:        return "guild";
        case HIGHGUID_AREATRIGGER:  return "areatrigger";
        default:
            return "<unknown>";
    }
}

#endif

