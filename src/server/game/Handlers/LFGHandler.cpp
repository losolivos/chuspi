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

#include "WorldSession.h"
#include "WorldPacket.h"
#include "DBCStores.h"
#include "Player.h"
#include "Group.h"
#include "LFGMgr.h"
#include "ObjectMgr.h"
#include "GroupMgr.h"
#include "GameEventMgr.h"
#include "InstanceScript.h"

void WorldSession::HandleLfgJoinOpcode(WorldPacket& recvData)
{
    uint32 roles, dungeon, numDungeons;
    uint8 partyIndex, commentLength;
    bool queueAsGroup;

    recvData >> partyIndex;

    for (int i = 0; i < 3; ++i)
        recvData.read_skip<uint32>();

    recvData >> roles;

    numDungeons = recvData.ReadBits(22);
    queueAsGroup = recvData.ReadBit();
    commentLength = recvData.ReadBits(8);

    if (!numDungeons)
    {
        TC_LOG_DEBUG("network", "CMSG_LFG_JOIN [" UI64FMTD "] no dungeons selected", GetPlayer()->GetGUID());
        recvData.rfinish();
        return;
    }

    LfgDungeonSet newDungeons;
    for (uint32 i = 0; i < numDungeons; ++i)
    {
        recvData >> dungeon;
        dungeon &= 0xFFFFFF;
        newDungeons.insert(dungeon);       // remove the type from the dungeon entry
    }

    std::string comment = recvData.ReadString(commentLength);

    uint8 maxGroupSize = sLFGMgr->GetGroupSizeFromEntry(sLFGDungeonStore.LookupEntry(*newDungeons.begin() & 0xFFFFFF));
    if (!sWorld->getBoolConfig(CONFIG_DUNGEON_FINDER_ENABLE) ||
        (GetPlayer()->GetGroup() && GetPlayer()->GetGroup()->GetLeaderGUID() != GetPlayer()->GetGUID() &&
        (GetPlayer()->GetGroup()->GetMembersCount() == maxGroupSize || !GetPlayer()->GetGroup()->isLFGGroup())))
    {
        recvData.rfinish();
        return;
    }

    TC_LOG_DEBUG("network", "CMSG_LFG_JOIN [" UI64FMTD "] roles: %u, Dungeons: %u, Comment: %s", GetPlayer()->GetGUID(), roles, uint8(newDungeons.size()), comment.c_str());
    sLFGMgr->Join(GetPlayer(), uint8(roles), newDungeons, comment);
}

void WorldSession::HandleLfgLeaveOpcode(WorldPacket&  /*recvData*/)
{
    Group* grp = GetPlayer()->GetGroup();

    TC_LOG_DEBUG("network", "CMSG_LFG_LEAVE [" UI64FMTD "] in group: %u", GetPlayer()->GetGUID(), grp ? 1 : 0);

    // Check cheating - only leader can leave the queue
    if (!grp || grp->GetLeaderGUID() == GetPlayer()->GetGUID())
        sLFGMgr->Leave(GetPlayer(), grp);
}

void WorldSession::HandleLfgProposalResultOpcode(WorldPacket& recvData)
{
    uint32 lfgGroupID;                                     // Internal lfgGroupID
    bool accept;                                           // Accept to join?

    recvData.read_skip<uint32>();                          // QueueId
    recvData.read_skip<uint32>();                          // Time
    recvData >> lfgGroupID;                                // ProposalId
    recvData.read_skip<uint32>();                          // Const flag 3

    ObjectGuid guid1;
    ObjectGuid guid2;

    recvData.ReadBitSeq<3, 5>(guid1);
    recvData.ReadBitSeq<3>(guid2);
    recvData.ReadBitSeq<1, 0, 2>(guid1);
    recvData.ReadBitSeq<1>(guid2);

    accept = recvData.ReadBit();

    recvData.ReadBitSeq<4>(guid2);
    recvData.ReadBitSeq<4>(guid1);
    recvData.ReadBitSeq<0>(guid2);
    recvData.ReadBitSeq<7>(guid1);
    recvData.ReadBitSeq<2, 7>(guid2);
    recvData.ReadBitSeq<6>(guid1);
    recvData.ReadBitSeq<6, 5>(guid2);

    recvData.ReadByteSeq<2, 3, 4>(guid1);
    recvData.ReadByteSeq<2, 0>(guid2);
    recvData.ReadByteSeq<6>(guid1);
    recvData.ReadByteSeq<7>(guid2);
    recvData.ReadByteSeq<5>(guid1);

    recvData.ReadByteSeq<1>(guid2);
    recvData.ReadByteSeq<0, 7>(guid1);
    recvData.ReadByteSeq<3, 4>(guid2);
    recvData.ReadByteSeq<1>(guid1);
    recvData.ReadByteSeq<5, 6>(guid2);

    TC_LOG_DEBUG("network", "CMSG_LFG_PROPOSAL_RESULT [" UI64FMTD "] proposal: %u accept: %u", GetPlayer()->GetGUID(), lfgGroupID, accept ? 1 : 0);
    sLFGMgr->UpdateProposal(lfgGroupID, GetPlayer()->GetGUID(), accept);
}

void WorldSession::HandleLfgSetRolesOpcode(WorldPacket& recvData)
{
    uint32 roles;
    uint8 unk;

    recvData >> roles;                                    // Player Group Roles
    recvData >> unk;

    uint64 guid = GetPlayer()->GetGUID();
    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
    {
        TC_LOG_DEBUG("network", "CMSG_LFG_SET_ROLES [" UI64FMTD "] Not in group", guid);
        return;
    }
    uint64 gguid = grp->GetGUID();
    TC_LOG_DEBUG("network", "CMSG_LFG_SET_ROLES: Group [" UI64FMTD "], Player [" UI64FMTD "], Roles: %u", gguid, guid, roles);
    sLFGMgr->UpdateRoleCheck(gguid, guid, roles);
}

void WorldSession::HandleLfgSetCommentOpcode(WorldPacket&  recvData)
{
    std::string comment;
    recvData >> comment;
    uint64 guid = GetPlayer()->GetGUID();
    TC_LOG_DEBUG("network", "CMSG_SET_LFG_COMMENT [" UI64FMTD "] comment: %s", guid, comment.c_str());

    sLFGMgr->SetComment(guid, comment);
}

void WorldSession::HandleLfgSetBootVoteOpcode(WorldPacket& recvData)
{
    bool agree;                                            // Agree to kick player
    agree = recvData.ReadBit();

    TC_LOG_DEBUG("network", "CMSG_LFG_SET_BOOT_VOTE [" UI64FMTD "] agree: %u", GetPlayer()->GetGUID(), agree ? 1 : 0);
    sLFGMgr->UpdateBoot(GetPlayer(), agree);
}

void WorldSession::HandleLfgTeleportOpcode(WorldPacket& recvData)
{
    bool out;
    out = recvData.ReadBit();

    TC_LOG_DEBUG("network", "CMSG_LFG_TELEPORT [" UI64FMTD "] out: %u", GetPlayer()->GetGUID(), out ? 1 : 0);
    sLFGMgr->TeleportPlayer(GetPlayer(), out, true);
}

void WorldSession::HandleLfgLockInfoRequestOpcode(WorldPacket& recvData)
{
    uint8 value;

    recvData >> value;
    recvData.ReadBit();

    ObjectGuid guid = GetPlayer()->GetGUID();
    TC_LOG_DEBUG("network", "CMSG_LFD_LOCK_INFO_REQUEST [" UI64FMTD "]", uint64(guid));

    // Get Random dungeons that can be done at a certain level and expansion
    LfgDungeonSet randomDungeons;
    uint8 level = GetPlayer()->getLevel();
    uint8 expansion = GetPlayer()->GetSession()->Expansion();
    for (uint32 i = 0; i < sLFGDungeonStore.GetNumRows(); ++i)
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(i);
        if (dungeon && dungeon->expansion <= expansion && dungeon->minlevel <= level && level <= dungeon->maxlevel)
        {
            if (dungeon->flags & LFG_FLAG_SEASONAL)
            {
                if (HolidayIds holiday = sLFGMgr->GetDungeonSeason(dungeon->ID))
                    if (holiday == HOLIDAY_WOTLK_LAUNCH || !IsHolidayActive(holiday))
                        continue;
            }
            else if (dungeon->type != TYPEID_RANDOM_DUNGEON)
                continue;

            randomDungeons.insert(dungeon->Entry());
        }
    }

    // Get player locked Dungeons
    LfgLockMap const &lock = sLFGMgr->GetLockedDungeons(guid);

    ByteBuffer dataBuffer;
    WorldPacket data(SMSG_LFG_PLAYER_INFO, 1 + randomDungeons.size() * (4 + 1 + 4 + 4 + 4 + 4 + 1 + 4 + 4 + 4)
        + 4 + lock.size() * (1 + 4 + 4 + 4 + 4 + 1 + 4 + 4 + 4));

    data.WriteBit(1);                                                               // PlayerGUID
    data.WriteBitSeq<7, 2, 1, 6, 3, 5, 0, 4>(guid);
    data.WriteBits(randomDungeons.size(), 17);

    for (auto const &randomDungeon : randomDungeons)
    {
        Quest const* rewardQuest = nullptr;
        bool firstReward = false;

        if (LfgReward const* reward = sLFGMgr->GetRandomDungeonReward(randomDungeon, level))
        {
            rewardQuest = sObjectMgr->GetQuestTemplate(reward->reward[0].questId);
            if (rewardQuest)
            {
                firstReward = !GetPlayer()->CanRewardQuest(rewardQuest, false);
                if (firstReward)
                    rewardQuest = sObjectMgr->GetQuestTemplate(reward->reward[1].questId);
            }
        }

        data.WriteBits(0, 21);                                                      // BonusCurrency
        data.WriteBits(rewardQuest ? rewardQuest->GetRewCurrencyCount() : 0, 21);
        data.WriteBits(0, 19);                                                      // ShortageReward
        data.WriteBit(!firstReward);
        data.WriteBits(rewardQuest ? rewardQuest->GetRewItemsCount(): 0, 20);
        data.WriteBit(0);                                                           // ShortageEligible

        /*
         *  Values that are missing and need to named:
         *      Mask, CompletedMask, CompletionQuantity, CompletionLimit, CompletionCurrencyID
         *      SpecificQuantity, SpecificLimit, OverallQuantity, OverallLimit, PurseWeeklyQuantity
         *      PurseWeeklyLimit, PurseQuantity, PurseLimit, Quantity
         */

        dataBuffer << uint32(0);
        dataBuffer << uint32(0);

        if (rewardQuest)
        {
            for (uint8 i = 0; i < QUEST_REWARDS_COUNT; ++i)
            {
                if (!rewardQuest->RewardCurrencyId[i])
                    continue;

                dataBuffer << uint32(rewardQuest->RewardCurrencyCount[i]);
                dataBuffer << uint32(rewardQuest->RewardCurrencyId[i]);
            }
        }

        dataBuffer << uint32(0);

        if (rewardQuest)
        {
            for (uint8 i = 0; i < QUEST_REWARDS_COUNT; ++i)
            {
                if (!rewardQuest->RewardItemId[i])
                    continue;

                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(rewardQuest->RewardItemId[i]);

                dataBuffer << uint32(itemTemplate ? itemTemplate->DisplayInfoID : 0);
                dataBuffer << uint32(rewardQuest->RewardItemId[i]);
                dataBuffer << uint32(rewardQuest->RewardItemIdCount[i]);
            }
        }

        dataBuffer << uint32(0);
        dataBuffer << uint32(0);

        /*for (uint32 i = 0; i < shortageRewardCount; i++)
        {
        }*/

        dataBuffer << uint32(0);
        dataBuffer << uint32(0);
        dataBuffer << uint32(randomDungeon);
        dataBuffer << uint32(0);
        dataBuffer << uint32(0);
        dataBuffer << uint32(0);
        dataBuffer << uint32(0);

        /*for (uint32 i = 0; i < bonusCurrencyCount; i++)
        {
        }*/

        dataBuffer << uint32(rewardQuest ? rewardQuest->XPValue(GetPlayer()) : 0);
        dataBuffer << uint32(0);
        dataBuffer << uint32(0);
        dataBuffer << uint32(0);
        dataBuffer << uint32(rewardQuest ? rewardQuest->GetRewardMoney() : 0);
    }

    data.WriteBits(lock.size(), 20);
    data.FlushBits();
    data.append(dataBuffer);

    for (auto const& lfgLock : lock)
    {
        auto lockData = lfgLock.second;

        data << uint32(lockData.itemLevel);
        data << uint32(lfgLock.first);
        data << uint32(lockData.lockstatus);
        data << uint32(GetPlayer()->GetAverageItemLevel());
    }

    data.WriteByteSeq<3, 1, 2, 6, 4, 7, 0, 5>(guid);

    SendPacket(&data);
    TC_LOG_DEBUG("network", "SMSG_LFG_PLAYER_INFO [" UI64FMTD "]", uint64(guid));
}

void WorldSession::HandleLfrSearchOpcode(WorldPacket& recvData)
{
    uint32 entry;                                          // Raid id to search
    recvData >> entry;
    TC_LOG_DEBUG("network", "CMSG_SEARCH_LFG_JOIN [" UI64FMTD "] dungeon entry: %u", GetPlayer()->GetGUID(), entry);
    //SendLfrUpdateListOpcode(entry);
}

void WorldSession::HandleLfrLeaveOpcode(WorldPacket& recvData)
{
    uint32 dungeonId;                                      // Raid id queue to leave
    recvData >> dungeonId;
    TC_LOG_DEBUG("network", "CMSG_SEARCH_LFG_LEAVE [" UI64FMTD "] dungeonId: %u", GetPlayer()->GetGUID(), dungeonId);
    //sLFGMgr->LeaveLfr(GetPlayer(), dungeonId);
}
void WorldSession::HandleLfgGetStatus(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_LFG_GET_STATUS %s", GetPlayer()->GetName().c_str());

    /*uint64 guid = GetPlayer()->GetGUID();
    LfgUpdateData updateData = sLFGMgr->GetLfgStatus(guid);

    if (GetPlayer()->GetGroup())
    {
        sLFGMgr->SendUpdateStatus(GetPlayer(), updateData);
        updateData.dungeons.clear();
        sLFGMgr->SendUpdateStatus(GetPlayer(), updateData);
    }
    else
    {
        sLFGMgr->SendUpdateStatus(GetPlayer(), updateData);
        updateData.dungeons.clear();
        sLFGMgr->SendUpdateStatus(GetPlayer(), updateData);
    }*/
}

void WorldSession::SendLfgRoleChosen(ObjectGuid guid, uint8 roles)
{
    TC_LOG_DEBUG("network", "SMSG_LFG_ROLE_CHOSEN [" UI64FMTD "] guid: [" UI64FMTD "] roles: %u", GetPlayer()->GetGUID(), uint64(guid), roles);

    WorldPacket data(SMSG_LFG_ROLE_CHOSEN, 12);

    data.WriteBitSeq<6, 2, 3, 7>(guid);
    data.WriteBit(roles > 0);
    data.WriteBitSeq<1, 5, 4, 0>(guid);
    data << uint32(roles);
    data.WriteByteSeq<4, 2, 5, 1, 6, 3, 0, 7>(guid);
    SendPacket(&data);
}

void WorldSession::SendLfgRoleCheckUpdate(const LfgRoleCheck* pRoleCheck)
{
    ASSERT(pRoleCheck);

    LfgDungeonSet dungeons;
    if (pRoleCheck->rDungeonId)
        dungeons.insert(pRoleCheck->rDungeonId);
    else
        dungeons = pRoleCheck->dungeons;

    // calculate if role check initiated message needs to be displayed
    bool isBeginning = pRoleCheck->state == LFG_ROLECHECK_INITIALITING ? true : false;
    for (auto const &memberRole : pRoleCheck->roles)
        if (memberRole.first != pRoleCheck->leader)
            if (memberRole.second)
            {
                isBeginning = false;
                break;
            }

    ObjectGuid bgQueueId = 0;           // name dumped from client
    ByteBuffer dataBuffer;

    WorldPacket data(SMSG_LFG_ROLE_CHECK_UPDATE, 1 + 8 + 6 + 1 + 1 + dungeons.size() * 4 + 1 + pRoleCheck->roles.size() * (8 + 1 + 4 + 1));
    data.WriteBit(isBeginning);
    data.WriteBitSeq<5, 3, 6, 4>(bgQueueId);
    data.WriteBits(dungeons.size(), 22);
    data.WriteBitSeq<0, 1>(bgQueueId);
    data.WriteBits(pRoleCheck->roles.size(), 21);

    if (!pRoleCheck->roles.empty())
    {
        ObjectGuid leaderGuid = pRoleCheck->leader;
        uint8 leaderRoleFlags = pRoleCheck->roles.find(leaderGuid)->second;
        Player* leader = ObjectAccessor::FindPlayer(leaderGuid);

        data.WriteBitSeq<7, 6>(leaderGuid);
        data.WriteBit(leaderRoleFlags);
        data.WriteBitSeq<4, 3, 1, 0, 2, 5>(leaderGuid);

        dataBuffer << uint32(leaderRoleFlags);
        dataBuffer.WriteByteSeq<1>(leaderGuid);
        dataBuffer << uint8(leader ? leader->getLevel() : 0);
        dataBuffer.WriteByteSeq<3, 7, 4, 0, 5, 2, 6>(leaderGuid);

        for (LfgRolesMap::const_reverse_iterator it = pRoleCheck->roles.rbegin(); it != pRoleCheck->roles.rend(); ++it)
        {
            if (it->first == pRoleCheck->leader)
                continue;

            ObjectGuid memeberGuid = it->first;
            uint8 memeberRoleFlags = it->second;
            Player* memeber = ObjectAccessor::FindPlayer(memeberGuid);

            data.WriteBitSeq<7, 6>(memeberGuid);
            data.WriteBit(memeberRoleFlags);
            data.WriteBitSeq<4, 3, 1, 0, 2, 5>(memeberGuid);

            dataBuffer << uint32(memeberRoleFlags);
            dataBuffer.WriteByteSeq<1>(memeberGuid);
            dataBuffer << uint8(memeber ? memeber->getLevel() : 0);
            dataBuffer.WriteByteSeq<3, 7, 4, 0, 5, 2, 6>(memeberGuid);
        }
    }

    data.WriteBitSeq<7, 2>(bgQueueId);
    data.FlushBits();

    data.WriteByteSeq<1>(bgQueueId);
    data.append(dataBuffer);
    data << uint8(1);
    data.WriteByteSeq<3, 4, 0, 5, 6>(bgQueueId);
    data << uint8(pRoleCheck->state);
    data.WriteByteSeq<7>(bgQueueId);

    if (!dungeons.empty())
    {
        for (auto dungeon : dungeons)
        {
            LFGDungeonEntry const* dungeonEntry = sLFGDungeonStore.LookupEntry(dungeon);
            data << uint32(dungeonEntry ? dungeonEntry->Entry() : 0);
        }
    }

    data.WriteByteSeq<2>(bgQueueId);
    SendPacket(&data);

    TC_LOG_DEBUG("network", "SMSG_LFG_ROLE_CHECK_UPDATE [" UI64FMTD "]", GetPlayer()->GetGUID());
}

void WorldSession::SendLfgJoinResult(uint64 guid_, const LfgJoinResultData& joinData)
{
    ObjectGuid guid = guid_;

    uint32 size = 0;
    for (LfgLockPartyMap::const_iterator it = joinData.lockmap.begin(); it != joinData.lockmap.end(); ++it)
        size += 8 + 4 + uint32(it->second.size()) * (4 + 4 + 4 + 4);

    TC_LOG_DEBUG("network", "SMSG_LFG_JOIN_RESULT [" UI64FMTD "] checkResult: %u checkValue: %u", GetPlayer()->GetGUID(), joinData.result, joinData.state);

    WorldPacket data(SMSG_LFG_JOIN_RESULT, 4 + 4 + size);

    data.WriteBitSeq<4, 5, 0, 2>(guid);

    data.WriteBits(joinData.lockmap.size(), 22);

    for (LfgLockPartyMap::const_iterator it = joinData.lockmap.begin(); it != joinData.lockmap.end(); ++it)
    {
        ObjectGuid guid1 = it->first;

        data.WriteBitSeq<4, 5, 6, 0>(guid1);

        data.WriteBits(it->second.size(), 20);

        data.WriteBitSeq<7, 3, 2, 1>(guid1);
    }

    data.WriteBitSeq<1, 3, 6, 7>(guid);

    for (LfgLockPartyMap::const_iterator it = joinData.lockmap.begin(); it != joinData.lockmap.end(); ++it)
    {
        LfgLockMap second = it->second;
        for (LfgLockMap::const_iterator itr = second.begin(); itr != second.end(); ++itr)
        {
            auto lockData = itr->second;
            data << uint32(GetPlayer()->GetAverageItemLevel());
            data << uint32(itr->first);                         // Dungeon entry (id + type)
            data << uint32(lockData.itemLevel);                 // Lock status
            data << uint32(lockData.lockstatus);
        }

        ObjectGuid guid1 = it->first;
        data.WriteByteSeq<0, 1, 2, 5, 3, 6, 4, 7>(guid1);
    }

    data << uint8(joinData.result);                       // Check Result
    data << uint8(joinData.state);                        // Check Value

    data.WriteByteSeq<0, 7, 5>(guid);

    data << uint32(3);                                    // Unk
    data << uint32(getMSTime());                          // Time

    data.WriteByteSeq<1, 4, 3>(guid);

    data << uint32(0);                                    // Queue Id

    data.WriteByteSeq<2, 6>(guid);

    SendPacket(&data);
}

void WorldSession::SendLfgQueueStatus(uint32 dungeon, int32 waitTime, int32 avgWaitTime, int32 waitTimeTanks, int32 waitTimeHealer, int32 waitTimeDps, uint32 queuedTime, uint8 tanks, uint8 healers, uint8 dps)
{
    TC_LOG_DEBUG("network", "SMSG_LFG_QUEUE_STATUS [" UI64FMTD "] dungeon: %u - waitTime: %d - avgWaitTime: %d - waitTimeTanks: %d - waitTimeHealer: %d - waitTimeDps: %d - queuedTime: %u - tanks: %u - healers: %u - dps: %u", GetPlayer()->GetGUID(), dungeon, waitTime, avgWaitTime, waitTimeTanks, waitTimeHealer, waitTimeDps, queuedTime, tanks, healers, dps);
    ObjectGuid guid = GetPlayer()->GetGUID();

    LfgQueueInfo* info = sLFGMgr->GetLfgQueueInfo(GetPlayer()->GetGroup() ? GetPlayer()->GetGroup()->GetGUID() : GetPlayer()->GetGUID());
    if (!info)
        return;

    WorldPacket data(SMSG_LFG_QUEUE_STATUS, 4 + 4 + 4 + 4 + 4 +4 + 1 + 1 + 1 + 4);
    data << uint32(dungeon);                               // Dungeon
    data << uint32(info->joinTime);                        // Time
    data << uint32(waitTime);
    data << uint32(0);                                     // QueueId
    data << uint32(3);                                     // Some Flags
    data << uint32(avgWaitTime);

    data << int32(waitTimeTanks);                          // Wait Tanks
    data << uint8(tanks);                                  // Tanks needed

    data << int32(waitTimeHealer);                         // Wait Healers
    data << uint8(healers);                                // Healers needed

    data << int32(waitTimeDps);                            // Wait Dps
    data << uint8(dps);                                    // Dps needed

    data << int32(queuedTime);

    data.WriteBitSeq<7, 6, 5, 2, 3, 0, 4, 1>(guid);

    data.WriteByteSeq<4, 7, 2, 3, 1, 6, 5, 0>(guid);
    SendPacket(&data);
}

void WorldSession::SendLfgPlayerReward(uint32 rdungeonEntry, uint32 sdungeonEntry, uint8 done, const LfgReward* /*reward*/, const Quest* qRew)
{
    if (!rdungeonEntry || !sdungeonEntry || !qRew)
        return;

    uint8 itemNum = uint8(qRew ? qRew->GetRewItemsCount() + qRew->GetRewCurrencyCount()  : 0);

    TC_LOG_DEBUG("network", "SMSG_LFG_PLAYER_REWARD [" UI64FMTD "] rdungeonEntry: %u - sdungeonEntry: %u - done: %u", GetPlayer()->GetGUID(), rdungeonEntry, sdungeonEntry, done);

    ByteBuffer bytereward;
    WorldPacket data(SMSG_LFG_PLAYER_REWARD, 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 1 + itemNum * (4 + 4 + 4));
    data << uint32(rdungeonEntry);                         // Random Dungeon Finished
    data << uint32(sdungeonEntry);                         // Dungeon Finished
    data << uint32(qRew->GetRewardMoney());
    data << uint32(qRew->XPValue(GetPlayer()));
    data.WriteBits(itemNum, 20);

    if (qRew && qRew->GetRewItemsCount())
    {
        ItemTemplate const* iProto = NULL;
        for (uint8 i = 0; i < QUEST_REWARDS_COUNT; ++i)
        {
            if (!qRew->RewardItemId[i])
                continue;

            data.WriteBit(0);

            iProto = sObjectMgr->GetItemTemplate(qRew->RewardItemId[i]);

            bytereward << uint32(iProto ? iProto->DisplayInfoID : 0);
            bytereward << uint32(qRew->RewardItemId[i]);
            bytereward << uint32(0);
            bytereward << uint8(qRew->RewardItemIdCount[i]);
        }
    }
    if (qRew && qRew->GetRewCurrencyCount())
    {
        for (uint8 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
        {
            if (!qRew->RewardCurrencyId[i])
                continue;

            data.WriteBit(1);

            bytereward << uint32(0);
            bytereward << uint32(qRew->RewardCurrencyId[i]);
            bytereward << uint32(0);
            bytereward << uint32(qRew->RewardCurrencyCount[i]);
        }
    }

    data.FlushBits();
    data.append(bytereward);
    SendPacket(&data);
}

void WorldSession::SendLfgBootPlayer(const LfgPlayerBoot* pBoot)
{
    uint64 guid = GetPlayer()->GetGUID();
    LfgAnswer playerVote = pBoot->votes.find(guid)->second;
    uint8 votesNum = 0;
    uint8 agreeNum = 0;
    uint32 secsleft = uint8((pBoot->cancelTime - time(NULL)) / 1000);
    for (LfgAnswerMap::const_iterator it = pBoot->votes.begin(); it != pBoot->votes.end(); ++it)
    {
        if (it->second != LFG_ANSWER_PENDING)
        {
            ++votesNum;
            if (it->second == LFG_ANSWER_AGREE)
                ++agreeNum;
        }
    }
    TC_LOG_DEBUG("network", "SMSG_LFG_BOOT_PROPOSAL_UPDATE [" UI64FMTD "] inProgress: %u - didVote: %u - agree: %u - victim: [" UI64FMTD "] votes: %u - agrees: %u - left: %u - needed: %u - reason %s",
        guid, uint8(pBoot->inProgress), uint8(playerVote != LFG_ANSWER_PENDING), uint8(playerVote == LFG_ANSWER_AGREE), pBoot->victim, votesNum, agreeNum, secsleft, pBoot->votedNeeded, pBoot->reason.c_str());
    WorldPacket data(SMSG_LFG_BOOT_PROPOSAL_UPDATE, 1 + 1 + 1 + 8 + 4 + 4 + 4 + 4 + pBoot->reason.length());
    data << uint8(pBoot->inProgress);                      // Vote in progress
    data << uint8(playerVote != LFG_ANSWER_PENDING);       // Did Vote
    data << uint8(playerVote == LFG_ANSWER_AGREE);         // Agree
    data << uint8(0);                                      // Unknown 4.2.2
    data << uint64(pBoot->victim);                         // Victim GUID
    data << uint32(votesNum);                              // Total Votes
    data << uint32(agreeNum);                              // Agree Count
    data << uint32(secsleft);                              // Time Left
    data << uint32(pBoot->votedNeeded);                    // Needed Votes
    data << pBoot->reason.c_str();                         // Kick reason
    SendPacket(&data);
}

void WorldSession::SendLfgUpdateProposal(uint32 proposalId, const LfgProposal* pProp)
{
    if (!pProp)
        return;

    uint64 guid = GetPlayer()->GetGUID();
    LfgProposalPlayerMap::const_iterator itPlayer = pProp->players.find(guid);
    if (itPlayer == pProp->players.end())                  // Player MUST be in the proposal
        return;

    LfgProposalPlayer* ppPlayer = itPlayer->second;
    uint32 pLowGroupGuid = ppPlayer->groupLowGuid;
    uint32 dLowGuid = pProp->groupLowGuid;
    uint32 dungeonId = pProp->dungeonId;
    bool isSameDungeon = false;
    bool isContinue = false;
    Group* grp = dLowGuid ? sGroupMgr->GetGroupByGUID(dLowGuid) : NULL;
    uint32 completedEncounters = 0;
    if (grp)
    {
        uint64 gguid = grp->GetGUID();
        isContinue = grp->isLFGGroup() && sLFGMgr->GetState(gguid) != LFG_STATE_FINISHED_DUNGEON;
        isSameDungeon = GetPlayer()->GetGroup() == grp && isContinue;
    }

    TC_LOG_DEBUG("network", "SMSG_LFG_PROPOSAL_UPDATE [" UI64FMTD "] state: %u", GetPlayer()->GetGUID(), pProp->state);
    WorldPacket data(SMSG_LFG_PROPOSAL_UPDATE, 4 + 1 + 4 + 4 + 1 + 1 + pProp->players.size() * (4 + 1 + 1 + 1 + 1 +1));

    if (!isContinue)                                       // Only show proposal dungeon if it's continue
    {
        LfgDungeonSet playerDungeons = sLFGMgr->GetSelectedDungeons(guid);
        if (playerDungeons.size() == 1)
            dungeonId = (*playerDungeons.begin());
    }

    if (LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId))
    {
        dungeonId = dungeon->Entry();

        // Select a player inside to be get completed encounters from
        if (grp)
        {
            for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* groupMember = itr->GetSource();
                if (groupMember && groupMember->GetMapId() == uint32(dungeon->map))
                {
                    if (InstanceScript* instance = groupMember->GetInstanceScript())
                        completedEncounters = instance->GetCompletedEncounterMask();
                    break;
                }
            }
        }
    }

    ObjectGuid playerGUID = guid;
    ObjectGuid InstanceSaveGUID = MAKE_NEW_GUID(dungeonId, 0, HIGHGUID_INSTANCE_SAVE);

    data.WriteBitSeq<1>(playerGUID);

    data.WriteBit(isContinue);

    data.WriteBitSeq<0, 1>(InstanceSaveGUID);
    data.WriteBitSeq<4>(playerGUID);
    data.WriteBitSeq<7>(InstanceSaveGUID);
    data.WriteBitSeq<2>(playerGUID);
    data.WriteBitSeq<3, 5>(InstanceSaveGUID);

    data.WriteBits(pProp->players.size(), 21);

    data.WriteBitSeq<4>(InstanceSaveGUID);
    data.WriteBit(isSameDungeon);
    data.WriteBitSeq<6>(playerGUID);

    for (itPlayer = pProp->players.begin(); itPlayer != pProp->players.end(); ++itPlayer)
    {
        bool inDungeon = false;
        bool inSameGroup = false;

        if (itPlayer->second->groupLowGuid)
        {
            inDungeon = itPlayer->second->groupLowGuid == dLowGuid;
            inSameGroup = itPlayer->second->groupLowGuid == pLowGroupGuid;
        }

        data.WriteBit(inDungeon);                                       // In dungeon (silent)
        data.WriteBit(itPlayer->second->accept == LFG_ANSWER_AGREE);    // Accepted
        data.WriteBit(itPlayer->first == guid);                         // Self player
        data.WriteBit(itPlayer->second->accept!= LFG_ANSWER_PENDING);   // Answered
        data.WriteBit(inSameGroup);                                     // Same Group than player
    }

    data.WriteBitSeq<5, 7, 3>(playerGUID);

    data.WriteBitSeq<2, 6>(InstanceSaveGUID);

    data.WriteBitSeq<0>(playerGUID);

    data.WriteByteSeq<5, 1>(InstanceSaveGUID);
    data.WriteByteSeq<5>(playerGUID);

    data << uint32(proposalId);                            // Proposal Id

    data.WriteByteSeq<2, 3, 7>(InstanceSaveGUID);

    data << uint8(pProp->state);                           // Result state
    data << uint32(dungeonId);                             // Dungeon

    data.WriteByteSeq<4, 6>(InstanceSaveGUID);

    data << uint32(completedEncounters);                   // Bosses killed

    data.WriteByteSeq<4, 3, 0, 2, 6>(playerGUID);

    data << uint32(getMSTime());                           // Date

    data.WriteByteSeq<7>(playerGUID);

    for (itPlayer = pProp->players.begin(); itPlayer != pProp->players.end(); ++itPlayer)
        data << uint32(itPlayer->second->role);                    // Role

    data.WriteByteSeq<1>(playerGUID);

    data << uint32(0x03);                                  // unk id or flags ? always 3

    data.WriteByteSeq<0>(InstanceSaveGUID);

    data << uint32(0);                                     // QueueId

    SendPacket(&data);
}

void WorldSession::SendLfgUpdateSearch(bool update)
{
    TC_LOG_DEBUG("network", "SMSG_LFG_UPDATE_SEARCH [" UI64FMTD "] update: %u", GetPlayer()->GetGUID(), update ? 1 : 0);
    WorldPacket data(SMSG_LFG_UPDATE_SEARCH, 1);
    data << uint8(update);                                 // In Lfg Queue?
    SendPacket(&data);
}

void WorldSession::SendLfgDisabled()
{
    TC_LOG_DEBUG("network", "SMSG_LFG_DISABLED [" UI64FMTD "]", GetPlayer()->GetGUID());
    WorldPacket data(SMSG_LFG_DISABLED, 0);
    SendPacket(&data);
}

void WorldSession::SendLfgOfferContinue(uint32 dungeonEntry)
{
    TC_LOG_DEBUG("network", "SMSG_LFG_OFFER_CONTINUE [" UI64FMTD "] dungeon entry: %u", GetPlayer()->GetGUID(), dungeonEntry);
    WorldPacket data(SMSG_LFG_OFFER_CONTINUE, 4);
    data << uint32(dungeonEntry);
    SendPacket(&data);
}

void WorldSession::SendLfgTeleportError(uint8 err)
{
    TC_LOG_DEBUG("network", "SMSG_LFG_TELEPORT_DENIED [" UI64FMTD "] reason: %u", GetPlayer()->GetGUID(), err);
    WorldPacket data(SMSG_LFG_TELEPORT_DENIED, 4);
    //Not sure it is no 4bits.
    data.WriteBits(err, 4);                                   // Error
    data.FlushBits();
    SendPacket(&data);
}

/*
void WorldSession::SendLfrUpdateListOpcode(uint32 dungeonEntry)
{
    TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "SMSG_LFG_UPDATE_LIST [" UI64FMTD "] dungeon entry: %u", GetPlayer()->GetGUID(), dungeonEntry);
    WorldPacket data(SMSG_LFG_UPDATE_LIST);
    SendPacket(&data);
}
*/
