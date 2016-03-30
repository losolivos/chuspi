/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#ifndef _WARDENCHECKMGR_H
#define _WARDENCHECKMGR_H

#include "BigNumber.h"

#include <ace/Guard_T.h>
#include <ace/RW_Thread_Mutex.h>

#include <map>
#include <string>
#include <vector>

struct WardenCheck
{
    uint8 Type;
    BigNumber Data;
    uint32 Address;                                         // PROC_CHECK, MEM_CHECK, PAGE_CHECK
    uint8 Length;                                           // PROC_CHECK, MEM_CHECK, PAGE_CHECK
    std::string Str;                                        // LUA, MPQ, DRIVER
    std::string Comment;
    uint16 CheckId;
};

struct WardenCheckResult
{
    BigNumber Result;                                       // MEM_CHECK
};

class WardenCheckMgr
{
public:
    typedef ACE_RW_Thread_Mutex LockType;
    typedef ACE_Write_Guard<LockType> WriteGuardType;
    typedef ACE_Read_Guard<LockType> ReadGuardType;

private:
    WardenCheckMgr();
    ~WardenCheckMgr();

public:
    // We have a linear key without any gaps, so we use vector for fast access
    typedef std::vector<WardenCheck*> CheckContainer;
    typedef std::map<uint32, WardenCheckResult*> CheckResultContainer;

    static WardenCheckMgr * instance()
    {
        static WardenCheckMgr mgr;
        return &mgr;
    }

    WardenCheck const * GetWardenDataById(uint16 Id) const;
    WardenCheckResult const * GetWardenResultById(uint16 Id) const;

    std::vector<uint16> MemChecksIdPool;
    std::vector<uint16> OtherChecksIdPool;

    void LoadWardenChecks();

    LockType _checkStoreLock;

private:
    CheckContainer CheckStore;
    CheckResultContainer CheckResultStore;
};

#define sWardenCheckMgr WardenCheckMgr::instance()

#endif
