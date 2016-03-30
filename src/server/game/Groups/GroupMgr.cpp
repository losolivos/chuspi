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

#include "GroupMgr.h"
#include "InstanceSaveMgr.h"
#include "World.h"
#include "DBCStores.h"

GroupMgr::GroupMgr()
{
    nextGroupId_ = 1;
}

GroupMgr::~GroupMgr()
{
    for (auto itr = groupStore_.begin(); itr != groupStore_.end();)
    {
        Group *group = itr->second;
        itr = groupStore_.erase(itr);

        // It still calls back to GroupMgr::RemoveGroup, but doesn't do anything
        // as key-value pair is already removed from map
        delete group;
    }
}

uint32 GroupMgr::GenerateGroupId()
{
    if (nextGroupId_ >= 0xFFFFFFFE)
    {
        TC_LOG_ERROR("misc", "Group guid overflow!! Can't continue, shutting down server. ");
        sWorld->StopNow(ERROR_EXIT_CODE);
    }
    return nextGroupId_++;
}

Group* GroupMgr::GetGroupByGUID(uint32 groupId) const
{
    auto itr = groupStore_.find(groupId);
    if (itr != groupStore_.end())
        return itr->second;

    return NULL;
}

void GroupMgr::AddGroup(Group* group)
{
    groupStore_[group->GetLowGUID()] = group;
}

void GroupMgr::RemoveGroup(Group* group)
{
    groupStore_.erase(group->GetLowGUID());
}

void GroupMgr::LoadGroups()
{
    {
        uint32 oldMSTime = getMSTime();

        // Delete all groups with less than 2 members
        CharacterDatabase.DirectExecute("DELETE FROM groups WHERE guid NOT IN (SELECT guid FROM group_member GROUP BY guid HAVING COUNT(guid) > 1)");

        //                                                        0              1           2             3                 4      5          6      7         8       9
        QueryResult result = CharacterDatabase.Query("SELECT g.leaderGuid, g.lootMethod, g.looterGuid, g.lootThreshold, g.icon1, g.icon2, g.icon3, g.icon4, g.icon5, g.icon6"
            //  10         11          12         13              14            15         16           17
            ", g.icon7, g.icon8, g.groupType, g.difficulty, g.raiddifficulty, g.guid, lfg.dungeon, lfg.state FROM groups g LEFT JOIN lfg_data lfg ON lfg.guid = g.guid ORDER BY g.guid ASC");

        if (!result)
        {
            TC_LOG_INFO("server.loading", ">> Loaded 0 group definitions. DB table `groups` is empty!");
            return;
        }

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            Group* group = new Group(fields[15].GetUInt32());
            group->LoadGroupFromDB(fields);

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u group definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    TC_LOG_INFO("server.loading", "Loading Group members...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                    0        1           2            3       4
        QueryResult result = CharacterDatabase.Query("SELECT guid, memberGuid, memberFlags, subgroup, roles FROM group_member ORDER BY guid");
        if (!result)
        {
            TC_LOG_INFO("server.loading", ">> Loaded 0 group members. DB table `group_member` is empty!");
            return;
        }

        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();
            Group* group = GetGroupByGUID(fields[0].GetUInt32());

            if (group)
                group->LoadMemberFromDB(fields[1].GetUInt32(), fields[2].GetUInt8(), fields[3].GetUInt8(), fields[4].GetUInt8());
            else
                TC_LOG_ERROR("misc", "GroupMgr::LoadGroups: Consistency failed, can't find group (storage id: %u)", fields[0].GetUInt32());

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u group members in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    TC_LOG_INFO("server.loading", "Loading Group instance saves...");
    {
        uint32 oldMSTime = getMSTime();
        //                                                   0           1        2              3             4             5            6
        QueryResult result = CharacterDatabase.Query("SELECT gi.guid, i.map, gi.instance, gi.permanent, i.difficulty, i.resettime, COUNT(g.guid) "
            "FROM group_instance gi INNER JOIN instance i ON gi.instance = i.id "
            "LEFT JOIN character_instance ci LEFT JOIN groups g ON g.leaderGuid = ci.guid ON ci.instance = gi.instance AND ci.permanent = 1 "
            "GROUP BY gi.guid, i.map, gi.instance, gi.permanent, i.difficulty, i.resettime ORDER BY gi.guid");
        if (!result)
        {
            TC_LOG_INFO("server.loading", ">> Loaded 0 group-instance saves. DB table `group_instance` is empty!");
            return;
        }

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            Group* group = GetGroupByGUID(fields[0].GetUInt32());
            // group will never be NULL (we have run consistency sql's before loading)

            MapEntry const* mapEntry = sMapStore.LookupEntry(fields[1].GetUInt16());
            if (!mapEntry || !mapEntry->IsDungeon())
            {
                TC_LOG_ERROR("sql.sql", "Incorrect entry in group_instance table : no dungeon map %d", fields[1].GetUInt16());
                continue;
            }

            uint32 diff = fields[4].GetUInt8();
            if (diff >= uint32(mapEntry->IsRaid() ? MAX_RAID_DIFFICULTY : MAX_DUNGEON_DIFFICULTY))
            {
                TC_LOG_ERROR("sql.sql", "Wrong dungeon difficulty use in group_instance table: %d", diff + 1);
                diff = 0;                                   // default for both difficaly types
            }

            InstanceSave* save = sInstanceSaveMgr->AddInstanceSave(mapEntry->MapID, fields[2].GetUInt32(), Difficulty(diff), time_t(fields[5].GetUInt32()), (bool)fields[6].GetUInt64(), true);
            group->BindToInstance(save, fields[3].GetBool(), true);
            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u group-instance saves in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
}
