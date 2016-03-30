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

#ifndef _GROUPMGR_H
#define _GROUPMGR_H

#include "Define.h"
#include "Group.h"

#include <unordered_map>

class GroupMgr
{
    GroupMgr();
    ~GroupMgr();

public:
    typedef std::unordered_map<uint32, Group*> StorageType;

    static GroupMgr * instance();

    Group* GetGroupByGUID(uint32 guid) const;

    void SetNextGroupId(uint32 id) { nextGroupId_ = id; }

    void LoadGroups();
    uint32 GenerateGroupId();
    void AddGroup(Group* group);
    void RemoveGroup(Group* group);

private:
    uint32 nextGroupId_;
    StorageType groupStore_;
};

inline GroupMgr * GroupMgr::instance()
{
    static GroupMgr mgr;
    return &mgr;
}

#define sGroupMgr GroupMgr::instance()

#endif
