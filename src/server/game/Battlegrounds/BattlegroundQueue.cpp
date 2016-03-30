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

#include "BattlegroundQueue.h"
#include "Arena.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Group.h"
#include "EventProcessor.h"

namespace {

struct wait_time_comparator final
{
    bool operator ()(GroupQueueInfo const *first, GroupQueueInfo const *second) const
    {
        return first->JoinTime < second->JoinTime;
    }
};

typedef std::set<GroupQueueInfo*, wait_time_comparator> MatchingBattleTeams;

/*
    This class is used to invite player to BG again, when minute lasts from his first invitation
    it is capable to solve all possibilities
*/
class BGQueueInviteEvent final : public BasicEvent
{
public:
    BGQueueInviteEvent(uint64 pl_guid, uint32 BgInstanceGUID,
                       BattlegroundTypeId BgTypeId, uint8 arenaType,
                       uint32 removeTime)
        : m_PlayerGuid(pl_guid)
        , m_BgInstanceGUID(BgInstanceGUID)
        , m_BgTypeId(BgTypeId)
        , m_ArenaType(arenaType)
        , m_RemoveTime(removeTime)
    { }

    bool Execute(uint64, uint32) final
    {
        Player* player = ObjectAccessor::FindPlayer(m_PlayerGuid);
        // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
        if (!player)
            return true;

        Battleground* bg = sBattlegroundMgr->GetBattleground(m_BgInstanceGUID, m_BgTypeId);
        //if battleground ended and its instance deleted - do nothing
        if (!bg)
            return true;

        BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
        uint32 queueSlot = player->GetBattlegroundQueueIndex(bgQueueTypeId);
        if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue or in battleground
        {
            // check if player is invited to this bg
            BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
            if (bgQueue.IsPlayerInvited(m_PlayerGuid, m_BgInstanceGUID, m_RemoveTime))
            {
                uint32 const inviteAcceptWaitTime = bg->isArena()
                        ? INVITE_ACCEPT_WAIT_TIME_ARENA
                        : INVITE_ACCEPT_WAIT_TIME_BG;

                WorldPacket data;
                //we must send remaining time in queue
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, queueSlot, STATUS_WAIT_JOIN, inviteAcceptWaitTime - INVITATION_REMIND_TIME, player->GetBattlegroundQueueJoinTime(m_BgTypeId), m_ArenaType);
                player->SendDirectMessage(&data);
            }
        }

        //event will be deleted
        return true;
    }

private:
    uint64 m_PlayerGuid;
    uint32 m_BgInstanceGUID;
    BattlegroundTypeId m_BgTypeId;
    uint8  m_ArenaType;
    uint32 m_RemoveTime;
};

/*
    This class is used to remove player from BG queue after 1 minute 20 seconds from first invitation
    We must store removeInvite time in case player left queue and joined and is invited again
    We must store bgQueueTypeId, because battleground can be deleted already, when player entered it
*/
class BGQueueRemoveEvent final : public BasicEvent
{
public:
    BGQueueRemoveEvent(uint64 pl_guid, uint32 bgInstanceGUID,
                       BattlegroundTypeId BgTypeId, BattlegroundQueueTypeId bgQueueTypeId,
                       uint32 removeTime)
        : m_PlayerGuid(pl_guid)
        , m_BgInstanceGUID(bgInstanceGUID)
        , m_RemoveTime(removeTime)
        , m_BgTypeId(BgTypeId)
        , m_BgQueueTypeId(bgQueueTypeId)
    { }

    bool Execute(uint64, uint32) final
    {
        Player* player = ObjectAccessor::FindPlayer(m_PlayerGuid);
        if (!player)
            // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
            return true;

        Battleground* bg = sBattlegroundMgr->GetBattleground(m_BgInstanceGUID, m_BgTypeId);
        //battleground can be deleted already when we are removing queue info
        //bg pointer can be NULL! so use it carefully!

        uint32 queueSlot = player->GetBattlegroundQueueIndex(m_BgQueueTypeId);
        if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue, or in Battleground
        {
            // check if player is in queue for this BG and if we are removing his invite event
            BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(m_BgQueueTypeId);
            GroupQueueInfo ginfo;
            if (bgQueue.GetPlayerGroupInfoData(m_PlayerGuid, &ginfo) &&
                bgQueue.IsPlayerInvited(m_PlayerGuid, m_BgInstanceGUID, m_RemoveTime))
            {
                TC_LOG_DEBUG("bg.battleground", "Battleground: removing player %u from bg queue for instance %u because of not pressing enter battle in time.", player->GetGUIDLow(), m_BgInstanceGUID);

                player->RemoveBattlegroundQueueId(m_BgQueueTypeId);
                bgQueue.RemovePlayer(m_PlayerGuid, true);
                //update queues if battleground isn't ended
                if (bg && bg->isBattleground() && bg->GetStatus() != STATUS_WAIT_LEAVE)
                    sBattlegroundMgr->ScheduleQueueUpdate(ginfo.ArenaMatchmakerRating, ginfo.ArenaType, m_BgQueueTypeId, m_BgTypeId, bg->GetBracketId());

                WorldPacket data;
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, queueSlot, STATUS_NONE, player->GetBattlegroundQueueJoinTime(m_BgTypeId), 0, ginfo.ArenaType);
                player->SendDirectMessage(&data);
            }
        }

        //event will be deleted
        return true;
    }

private:
    uint64 m_PlayerGuid;
    uint32 m_BgInstanceGUID;
    uint32 m_RemoveTime;
    BattlegroundTypeId m_BgTypeId;
    BattlegroundQueueTypeId m_BgQueueTypeId;
};

} // namespace

/*********************************************************/
/***            BATTLEGROUND QUEUE SYSTEM              ***/
/*********************************************************/

BattlegroundQueue::BattlegroundQueue()
{
    for (uint32 i = 0; i < BG_TEAMS_COUNT; ++i)
    {
        for (uint32 j = 0; j < MAX_BATTLEGROUND_BRACKETS; ++j)
        {
            m_SumOfWaitTimes[i][j] = 0;
            m_WaitTimeLastPlayer[i][j] = 0;
            for (uint32 k = 0; k < COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME; ++k)
                m_WaitTimes[i][j][k] = 0;
        }
    }
}

BattlegroundQueue::~BattlegroundQueue()
{
    for (int i = 0; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (uint32 j = 0; j < BG_QUEUE_GROUP_TYPES_COUNT; ++j)
        {
            for (GroupsQueueType::iterator itr = m_QueuedGroups[i][j].begin(); itr!= m_QueuedGroups[i][j].end(); ++itr)
                delete (*itr);
        }
    }
}

/*********************************************************/
/***      BATTLEGROUND QUEUE SELECTION POOLS           ***/
/*********************************************************/

// selection pool initialization, used to clean up from prev selection
void BattlegroundQueue::SelectionPool::Init()
{
    SelectedGroups.clear();
    PlayerCount = 0;
}

// remove group info from selection pool
// returns true when we need to try to add new group to selection pool
// returns false when selection pool is ok or when we kicked smaller group than we need to kick
// sometimes it can be called on empty selection pool
bool BattlegroundQueue::SelectionPool::KickGroup(uint32 size)
{
    //find maxgroup or LAST group with size == size and kick it
    bool found = false;
    GroupsQueueType::iterator groupToKick = SelectedGroups.begin();
    for (GroupsQueueType::iterator itr = groupToKick; itr != SelectedGroups.end(); ++itr)
    {
        if (abs((int32)((*itr)->Players.size() - size)) <= 1)
        {
            groupToKick = itr;
            found = true;
        }
        else if (!found && (*itr)->Players.size() >= (*groupToKick)->Players.size())
            groupToKick = itr;
    }
    //if pool is empty, do nothing
    if (GetPlayerCount())
    {
        //update player count
        GroupQueueInfo* ginfo = (*groupToKick);
        SelectedGroups.erase(groupToKick);
        PlayerCount -= ginfo->Players.size();
        //return false if we kicked smaller group or there are enough players in selection pool
        if (ginfo->Players.size() <= size + 1)
            return false;
    }
    return true;
}

// add group to selection pool
// used when building selection pools
// returns true if we can invite more players, or when we added group to selection pool
// returns false when selection pool is full
bool BattlegroundQueue::SelectionPool::AddGroup(GroupQueueInfo* ginfo, uint32 desiredCount)
{
    //if group is larger than desired count - don't allow to add it to pool
    if (!ginfo->IsInvitedToBGInstanceGUID && desiredCount >= PlayerCount + ginfo->Players.size())
    {
        SelectedGroups.push_back(ginfo);
        // increase selected players count
        PlayerCount += ginfo->Players.size();
        return true;
    }
    if (PlayerCount < desiredCount)
        return true;
    return false;
}

/*********************************************************/
/***               BATTLEGROUND QUEUES                 ***/
/*********************************************************/

// add group or player (grp == NULL) to bg queue with the given leader and bg specifications
GroupQueueInfo* BattlegroundQueue::AddGroup(Player* leader, Group* grp, BattlegroundTypeId BgTypeId, PvPDifficultyEntry const*  bracketEntry, uint8 ArenaType, uint32 ArenaRating, uint32 MatchmakerRating)
{
    BattlegroundBracketId bracketId = bracketEntry->GetBracketId();

    // create new ginfo
    GroupQueueInfo* ginfo            = new GroupQueueInfo;
    ginfo->BgTypeId                  = BgTypeId;
    ginfo->ArenaType                 = ArenaType;
    ginfo->IsRated                   = ArenaType > 0 || BgTypeId == BATTLEGROUND_RA_BG;
    ginfo->IsInvitedToBGInstanceGUID = 0;
    ginfo->JoinTime                  = getMSTime();
    ginfo->RemoveInviteTime          = 0;
    ginfo->Team                      = leader->GetTeam();
    ginfo->IsTeamChanged             = false;
    ginfo->ArenaTeamRating           = ArenaRating;
    ginfo->ArenaMatchmakerRating     = MatchmakerRating;
    ginfo->OpponentsTeamRating       = 0;
    ginfo->OpponentsMatchmakerRating = 0;

    uint32 index = ginfo->Team == HORDE ? 1 : 0;

    if (sWorld->getBoolConfig(BATTLEGROUND_CROSSFACTION_ENABLED) && ArenaType == 0)
        index = BG_QUEUE_MIXED;

    TC_LOG_DEBUG("bg.battleground", "Adding Group to BattlegroundQueue bgTypeId : %u, bracket_id : %u, index : %u", BgTypeId, bracketId, index);

    uint32 lastOnlineTime = getMSTime();

    if (grp)
    {
        for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member)
                continue;   // this should never happen
            PlayerQueueInfo& pl_info = m_QueuedPlayers[member->GetGUID()];
            pl_info.LastOnlineTime   = lastOnlineTime;
            pl_info.GroupInfo        = ginfo;
            // add the pinfo to ginfo's list
            ginfo->Players[member->GetGUID()]  = &pl_info;
        }
    }
    else
    {
        PlayerQueueInfo& pl_info = m_QueuedPlayers[leader->GetGUID()];
        pl_info.LastOnlineTime   = lastOnlineTime;
        pl_info.GroupInfo        = ginfo;
        ginfo->Players[leader->GetGUID()]  = &pl_info;
    }

    //add GroupInfo to m_QueuedGroups
    m_QueuedGroups[bracketId][index].push_back(ginfo);

    return ginfo;
}

void BattlegroundQueue::PlayerInvitedToBGUpdateAverageWaitTime(GroupQueueInfo* ginfo, BattlegroundBracketId bracket_id)
{
    uint32 timeInQueue = getMSTimeDiff(ginfo->JoinTime, getMSTime());
    uint8 team_index = TEAM_ALLIANCE;                    //default set to TEAM_ALLIANCE - or non rated arenas!
    if (ginfo->ArenaType || ginfo->Team == HORDE)
        team_index = TEAM_HORDE;

    //store pointer to arrayindex of player that was added first
    uint32* lastPlayerAddedPointer = &(m_WaitTimeLastPlayer[team_index][bracket_id]);
    //remove his time from sum
    m_SumOfWaitTimes[team_index][bracket_id] -= m_WaitTimes[team_index][bracket_id][(*lastPlayerAddedPointer)];
    //set average time to new
    m_WaitTimes[team_index][bracket_id][(*lastPlayerAddedPointer)] = timeInQueue;
    //add new time to sum
    m_SumOfWaitTimes[team_index][bracket_id] += timeInQueue;
    //set index of last player added to next one
    (*lastPlayerAddedPointer)++;
    (*lastPlayerAddedPointer) %= COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME;
}

uint32 BattlegroundQueue::GetAverageQueueWaitTime(GroupQueueInfo* ginfo, BattlegroundBracketId bracket_id) const
{
    uint8 team_index = TEAM_ALLIANCE;                    //default set to TEAM_ALLIANCE - or non rated arenas!
    if (ginfo->ArenaType || ginfo->Team == HORDE)
        team_index = TEAM_HORDE;

    //check if there is enough values (we always add values > 0)
    if (m_WaitTimes[team_index][bracket_id][COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME - 1])
        return (m_SumOfWaitTimes[team_index][bracket_id] / COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME);
    else
        //if there aren't enough values return 0 - not available
        return 0;
}

//remove player from queue and from group info, if group info is empty then remove it too
void BattlegroundQueue::RemovePlayer(uint64 guid, bool decreaseInvitedCount)
{
    int32 bracket_id = -1;                                     // signed for proper for-loop finish
    QueuedPlayersMap::iterator itr;

    //remove player from map, if he's there
    itr = m_QueuedPlayers.find(guid);
    if (itr == m_QueuedPlayers.end())
    {
        std::string playerName = "Unknown";
        if (Player* player = ObjectAccessor::FindPlayer(guid))
            playerName = player->GetName();
        TC_LOG_ERROR("bg.battleground", "BattlegroundQueue: couldn't find player %s (GUID: %u)", playerName.c_str(), GUID_LOPART(guid));
        return;
    }

    GroupQueueInfo* group = itr->second.GroupInfo;

    GroupsQueueType::iterator group_itr;
    // mostly people with the highest levels are in battlegrounds, thats why
    // we count from MAX_BATTLEGROUND_QUEUES - 1 to 0

    uint32 index = (group->Team == HORDE)
            ? (group->IsTeamChanged ? BG_QUEUE_ALLIANCE : BG_QUEUE_HORDE)
            : (group->IsTeamChanged ? BG_QUEUE_HORDE : BG_QUEUE_ALLIANCE);

    for (int32 bracket_id_tmp = MAX_BATTLEGROUND_BRACKETS - 1; bracket_id_tmp >= 0 && bracket_id == -1; --bracket_id_tmp)
    {
        //we must check premade and normal team's queue - because when players from premade are joining bg,
        //they leave groupinfo so we can't use its players size to find out index
        for (uint8 j = 0; j < BG_QUEUE_GROUP_TYPES_COUNT; ++j)
        {
            GroupsQueueType::iterator k = m_QueuedGroups[bracket_id_tmp][j].begin();
            for (; k != m_QueuedGroups[bracket_id_tmp][j].end(); ++k)
            {
                if ((*k) == group)
                {
                    bracket_id = bracket_id_tmp;
                    group_itr = k;
                    //we must store index to be able to erase iterator
                    index = j;
                    break;
                }
            }
        }
    }

    //player can't be in queue without group, but just in case
    if (bracket_id == -1)
    {
        TC_LOG_ERROR("bg.battleground", "BattlegroundQueue: ERROR Cannot find groupinfo for player GUID: %u", GUID_LOPART(guid));
        return;
    }
    TC_LOG_DEBUG("bg.battleground", "BattlegroundQueue: Removing player GUID %u, from bracket_id %u", GUID_LOPART(guid), (uint32)bracket_id);

    // ALL variables are correctly set
    // We can ignore leveling up in queue - it should not cause crash
    // remove player from group
    // if only one player there, remove group

    // remove player queue info from group queue info
    std::map<uint64, PlayerQueueInfo*>::iterator pitr = group->Players.find(guid);
    if (pitr != group->Players.end())
        group->Players.erase(pitr);

    // if invited to bg, and should decrease invited count, then do it
    if (decreaseInvitedCount && group->IsInvitedToBGInstanceGUID)
        if (Battleground* bg = sBattlegroundMgr->GetBattleground(group->IsInvitedToBGInstanceGUID, group->BgTypeId))
            bg->DecreaseInvitedCount(group->Team);

    // remove player queue info
    m_QueuedPlayers.erase(itr);

    // if player leaves queue and he is invited to rated arena match, then he have to lose
    if (group->IsInvitedToBGInstanceGUID && decreaseInvitedCount)
    {
        if (group->IsRated)
        {
            uint32 const ourMMR = group->ArenaMatchmakerRating;
            uint32 const theirMMR = group->OpponentsMatchmakerRating;

            int32 const mmrChange = Arena::GetRatingMod(ourMMR, theirMMR, false);

            if (Player * const player = ObjectAccessor::FindPlayer(guid))
                player->lostRatedBg(theirMMR, mmrChange);
            else
                Player::offlineLostRatedBg(GUID_LOPART(guid), theirMMR, mmrChange);
        }
    }

    // remove group queue info if needed
    if (group->Players.empty())
    {
        m_QueuedGroups[bracket_id][index].erase(group_itr);
        delete group;
        return;
    }

    // if group wasn't empty, so it wasn't deleted, and player have left a rated
    // queue -> everyone from the group should leave too
    // don't remove recursively if already invited to bg!
    // Note: Should only be for rated but we don't know proper join error... so forcing for non rated too
    if (!group->IsInvitedToBGInstanceGUID)
    {
        // remove next player, this is recursive
        // first send removal information
        if (Player* plr2 = ObjectAccessor::FindPlayer(group->Players.begin()->first))
        {
            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(group->BgTypeId);
            BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(group->BgTypeId, group->ArenaType);
            uint32 queueSlot = plr2->GetBattlegroundQueueIndex(bgQueueTypeId);
            plr2->RemoveBattlegroundQueueId(bgQueueTypeId); // must be called this way, because if you move this call to
                                                            // queue->removeplayer, it causes bugs
            WorldPacket data;
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, plr2, queueSlot, STATUS_NONE, plr2->GetBattlegroundQueueJoinTime(group->BgTypeId), 0, group->ArenaType);
            plr2->SendDirectMessage(&data);
        }
        // then actually delete, this may delete the group as well!
        RemovePlayer(group->Players.begin()->first, decreaseInvitedCount);
    }
}

//returns true when player pl_guid is in queue and is invited to bgInstanceGuid
bool BattlegroundQueue::IsPlayerInvited(uint64 pl_guid, const uint32 bgInstanceGuid, const uint32 removeTime)
{
    QueuedPlayersMap::const_iterator qItr = m_QueuedPlayers.find(pl_guid);
    return (qItr != m_QueuedPlayers.end()
        && qItr->second.GroupInfo->IsInvitedToBGInstanceGUID == bgInstanceGuid
        && qItr->second.GroupInfo->RemoveInviteTime == removeTime);
}

bool BattlegroundQueue::GetPlayerGroupInfoData(uint64 guid, GroupQueueInfo* ginfo)
{
    QueuedPlayersMap::const_iterator qItr = m_QueuedPlayers.find(guid);
    if (qItr == m_QueuedPlayers.end())
        return false;
    *ginfo = *(qItr->second.GroupInfo);
    return true;
}

bool BattlegroundQueue::InviteGroupToBG(GroupQueueInfo* ginfo, Battleground* bg, uint32 side)
{
    // set side if needed
    if (side && ginfo->Team != side)
    {
        ginfo->Team = side;
        ginfo->IsTeamChanged = true;
    }

    if (!ginfo->IsInvitedToBGInstanceGUID)
    {
        // not yet invited
        // set invitation
        ginfo->IsInvitedToBGInstanceGUID = bg->GetInstanceID();
        BattlegroundTypeId bgTypeId = bg->GetTypeID();
        BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, bg->GetArenaType());
        BattlegroundBracketId bracket_id = bg->GetBracketId();

        uint32 const inviteAcceptWaitTime = bg->isArena()
                ? INVITE_ACCEPT_WAIT_TIME_ARENA
                : INVITE_ACCEPT_WAIT_TIME_BG;

        ginfo->RemoveInviteTime = getMSTime() + inviteAcceptWaitTime;

        // loop through the players
        for (std::map<uint64, PlayerQueueInfo*>::iterator itr = ginfo->Players.begin(); itr != ginfo->Players.end(); ++itr)
        {
            // get the player
            Player* player = ObjectAccessor::FindPlayer(itr->first);
            // if offline, skip him, this should not happen - player is removed from queue when he logs out
            if (!player)
                continue;

            // invite the player
            PlayerInvitedToBGUpdateAverageWaitTime(ginfo, bracket_id);
            //sBattlegroundMgr->InvitePlayer(player, bg, ginfo->Team);

            // set invited player counters
            bg->IncreaseInvitedCount(ginfo->Team);

            player->SetInviteForBattlegroundQueueType(bgQueueTypeId, ginfo->IsInvitedToBGInstanceGUID);

            // create remind invite events
            BGQueueInviteEvent* inviteEvent = new BGQueueInviteEvent(player->GetGUID(), ginfo->IsInvitedToBGInstanceGUID, bgTypeId, ginfo->ArenaType, ginfo->RemoveInviteTime);
            player->m_Events.AddEvent(inviteEvent, player->m_Events.CalculateTime(INVITATION_REMIND_TIME));
            // create automatic remove events
            BGQueueRemoveEvent* removeEvent = new BGQueueRemoveEvent(player->GetGUID(), ginfo->IsInvitedToBGInstanceGUID, bgTypeId, bgQueueTypeId, ginfo->RemoveInviteTime);
            player->m_Events.AddEvent(removeEvent, player->m_Events.CalculateTime(inviteAcceptWaitTime));

            WorldPacket data;

            uint32 queueSlot = player->GetBattlegroundQueueIndex(bgQueueTypeId);

            TC_LOG_DEBUG("bg.battleground", "Battleground: invited player %s (%u) to BG instance %u queueindex %u bgtype %u",
                 player->GetName().c_str(), player->GetGUIDLow(), bg->GetInstanceID(), queueSlot, bg->GetTypeID());

            // send status packet
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, queueSlot, STATUS_WAIT_JOIN, inviteAcceptWaitTime, player->GetBattlegroundQueueJoinTime(bgTypeId), ginfo->ArenaType);
            player->SendDirectMessage(&data);
        }
        return true;
    }

    return false;
}

/*
This function is inviting players to already running battlegrounds
Invitation type is based on config file
large groups are disadvantageous, because they will be kicked first if invitation type = 1
*/
void BattlegroundQueue::FillPlayersToBG(Battleground* bg, BattlegroundBracketId bracket_id)
{
    int32 hordeFree = bg->GetFreeSlotsForTeam(HORDE);
    int32 aliFree   = bg->GetFreeSlotsForTeam(ALLIANCE);

    if (!bg->isArena())
        if (FillCrossPlayersToBG(bg, bracket_id, false))
            return;

    //iterator for iterating through bg queue
    GroupsQueueType::const_iterator Ali_itr;
    //count of groups in queue - used to stop cycles
    uint32 aliCount = 0;
    //index to queue which group is current
    uint32 aliIndex = 0;

    //the same thing for horde
    GroupsQueueType::const_iterator Horde_itr;
    uint32 hordeCount = 0;
    uint32 hordeIndex = 0;

    Ali_itr = m_QueuedGroups[bracket_id][BG_QUEUE_ALLIANCE].begin();
    aliCount = m_QueuedGroups[bracket_id][BG_QUEUE_ALLIANCE].size();
    for (; aliIndex < aliCount && m_SelectionPools[TEAM_ALLIANCE].AddGroup((*Ali_itr), aliFree); aliIndex++)
        ++Ali_itr;

    Horde_itr = m_QueuedGroups[bracket_id][BG_QUEUE_HORDE].begin();
    hordeCount = m_QueuedGroups[bracket_id][BG_QUEUE_HORDE].size();
    for (; hordeIndex < hordeCount && m_SelectionPools[TEAM_HORDE].AddGroup((*Horde_itr), hordeFree); hordeIndex++)
        ++Horde_itr;

    //if ofc like BG queue invitation is set in config, then we are happy
    if (sWorld->getIntConfig(CONFIG_BATTLEGROUND_INVITATION_TYPE) == 0)
        return;

    /*
    if we reached this code, then we have to solve NP - complete problem called Subset sum problem
    So one solution is to check all possible invitation subgroups, or we can use these conditions:
    1. Last time when BattlegroundQueue::Update was executed we invited all possible players - so there is only small possibility
        that we will invite now whole queue, because only 1 change has been made to queues from the last BattlegroundQueue::Update call
    2. Other thing we should consider is group order in queue
    */

    // At first we need to compare free space in bg and our selection pool
    int32 diffAli   = aliFree   - int32(m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount());
    int32 diffHorde = hordeFree - int32(m_SelectionPools[TEAM_HORDE].GetPlayerCount());
    while (abs(diffAli - diffHorde) > 1 && (m_SelectionPools[TEAM_HORDE].GetPlayerCount() > 0 || m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() > 0))
    {
        //each cycle execution we need to kick at least 1 group
        if (diffAli < diffHorde)
        {
            //kick alliance group, add to pool new group if needed
            if (m_SelectionPools[TEAM_ALLIANCE].KickGroup(diffHorde - diffAli))
            {
                for (; aliIndex < aliCount && m_SelectionPools[TEAM_ALLIANCE].AddGroup((*Ali_itr), (aliFree >= diffHorde) ? aliFree - diffHorde : 0); aliIndex++)
                    ++Ali_itr;
            }
            //if ali selection is already empty, then kick horde group, but if there are less horde than ali in bg - break;
            if (!m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount())
            {
                if (aliFree <= diffHorde + 1)
                    break;
                m_SelectionPools[TEAM_HORDE].KickGroup(diffHorde - diffAli);
            }
        }
        else
        {
            //kick horde group, add to pool new group if needed
            if (m_SelectionPools[TEAM_HORDE].KickGroup(diffAli - diffHorde))
            {
                for (; hordeIndex < hordeCount && m_SelectionPools[TEAM_HORDE].AddGroup((*Horde_itr), (hordeFree >= diffAli) ? hordeFree - diffAli : 0); hordeIndex++)
                    ++Horde_itr;
            }
            if (!m_SelectionPools[TEAM_HORDE].GetPlayerCount())
            {
                if (hordeFree <= diffAli + 1)
                    break;
                m_SelectionPools[TEAM_ALLIANCE].KickGroup(diffAli - diffHorde);
            }
        }
        //count diffs after small update
        diffAli   = aliFree   - int32(m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount());
        diffHorde = hordeFree - int32(m_SelectionPools[TEAM_HORDE].GetPlayerCount());
    }
}

void BattlegroundQueue::updateRatedBgQueue(BattlegroundTypeId bgTypeId, PvPDifficultyEntry const *bracketEntry, uint32 matchmakerRating)
{
    BattlegroundBracketId const bracketId = bracketEntry->GetBracketId();
    MatchingBattleTeams matchingTeams[BG_TEAMS_COUNT];

    {
        uint32 const baseRatingDiff = sWorld->getIntConfig(CONFIG_ARENA_BASE_RATING_DIFFERENCE);
        uint32 const ratingUpdateTimer = sWorld->getIntConfig(CONFIG_ARENA_RATING_DIFFERENCE_UPDATE_TIMER);
        uint32 const ratingDiffStep = sWorld->getIntConfig(CONFIG_ARENA_RATING_DIFFERENCE_UPDATE_STEP);

        for (uint32 i = BG_QUEUE_ALLIANCE; i <= BG_QUEUE_HORDE; ++i)
        {
            GroupsQueueType::const_iterator j = m_QueuedGroups[bracketId][i].begin();

            for (; j != m_QueuedGroups[bracketId][i].end(); ++j)
            {
                GroupQueueInfo * const ginfo = *j;
                if (ginfo->IsInvitedToBGInstanceGUID)
                    continue;

                uint32 const waitTime = GetMSTimeDiffToNow(ginfo->JoinTime);

                uint32 const allowedRatingDiff = std::min(baseRatingDiff + waitTime / ratingUpdateTimer * ratingDiffStep,
                                                          5000u);

                uint32 const minRating = (matchmakerRating > allowedRatingDiff) ? matchmakerRating - allowedRatingDiff : 0;
                uint32 const maxRating = matchmakerRating + allowedRatingDiff;

                // if group matches conditions, then add it to pool
                if (ginfo->ArenaMatchmakerRating >= minRating && ginfo->ArenaMatchmakerRating <= maxRating)
                {
                    // No break here, we try to find all teams that can play next game
                    matchingTeams[i].insert(ginfo);
                }
            }
        }
    }

    MatchingBattleTeams &allianceTeams = matchingTeams[BG_QUEUE_ALLIANCE];
    MatchingBattleTeams &hordeTeams = matchingTeams[BG_QUEUE_HORDE];

    while (!allianceTeams.empty() && !hordeTeams.empty())
    {
        MatchingBattleTeams::iterator firstItr = allianceTeams.begin();
        MatchingBattleTeams::iterator secondItr = hordeTeams.begin();

        GroupQueueInfo *firstTeam = *firstItr;
        GroupQueueInfo *secondTeam = *secondItr;

        allianceTeams.erase(firstItr);
        hordeTeams.erase(secondItr);

        Battleground *bg = sBattlegroundMgr->CreateNewBattleground(bgTypeId, bracketEntry, 0);
        if (!bg)
        {
            TC_LOG_ERROR("bg.battleground", "BattlegroundQueue::updateRatedBgQueue couldn't create BG instance for rated BG match!");
            return;
        }

        firstTeam->OpponentsMatchmakerRating = secondTeam->ArenaMatchmakerRating;
        secondTeam->OpponentsMatchmakerRating = firstTeam->ArenaMatchmakerRating;

        bg->SetArenaMatchmakerRating(ALLIANCE, firstTeam->ArenaMatchmakerRating);
        bg->SetArenaMatchmakerRating(HORDE, secondTeam->ArenaMatchmakerRating);

        InviteGroupToBG(firstTeam, bg, ALLIANCE);
        InviteGroupToBG(secondTeam, bg, HORDE);

        TC_LOG_DEBUG("bg.battleground", "Starting rated BG match!");

        bg->StartBattleground();
    }
}

void BattlegroundQueue::updateRatedArenaQueue(BattlegroundTypeId bgTypeId, PvPDifficultyEntry const *bracketEntry, uint8 arenaType, uint32 arenaRating)
{
    BattlegroundBracketId const bracketId = bracketEntry->GetBracketId();

    // we must find all teams that can play next game
    // Groups are sorted by wait time
    MatchingBattleTeams matchingTeams;

    {
        uint32 const baseRatingDiff = sWorld->getIntConfig(CONFIG_ARENA_BASE_RATING_DIFFERENCE);
        uint32 const ratingUpdateTimer = sWorld->getIntConfig(CONFIG_ARENA_RATING_DIFFERENCE_UPDATE_TIMER);
        uint32 const ratingDiffStep = sWorld->getIntConfig(CONFIG_ARENA_RATING_DIFFERENCE_UPDATE_STEP);

        for (uint32 i = BG_QUEUE_ALLIANCE; i <= BG_QUEUE_HORDE; ++i)
        {
            GroupsQueueType::const_iterator j = m_QueuedGroups[bracketId][i].begin();

            for (; j != m_QueuedGroups[bracketId][i].end(); ++j)
            {
                GroupQueueInfo * const ginfo = *j;
                if (ginfo->IsInvitedToBGInstanceGUID)
                    continue;

                uint32 const waitTime = GetMSTimeDiffToNow(ginfo->JoinTime);

                uint32 const allowedRatingDiff = std::min(baseRatingDiff + waitTime / ratingUpdateTimer * ratingDiffStep,
                                                          5000u);

                uint32 const minRating = (arenaRating > allowedRatingDiff) ? arenaRating - allowedRatingDiff : 0;
                uint32 const maxRating = arenaRating + allowedRatingDiff;

                // if group matches conditions, then add it to pool
                if (ginfo->ArenaMatchmakerRating >= minRating && ginfo->ArenaMatchmakerRating <= maxRating)
                {
                    // No break here, we try to find all teams that can play next game
                    matchingTeams.insert(ginfo);
                }
            }
        }
    }

    //if we have at least 2 teams, then start new arena and invite players!
    if (matchingTeams.size() < 2)
        return;

    while (!matchingTeams.empty())
    {
        MatchingBattleTeams::iterator firstItr = matchingTeams.begin();
        MatchingBattleTeams::iterator secondItr = ++matchingTeams.begin();
        if (secondItr == matchingTeams.end())
            break;

        GroupQueueInfo *firstTeam = *firstItr;
        GroupQueueInfo *secondTeam = *secondItr;

        matchingTeams.erase(firstItr);
        matchingTeams.erase(secondItr);

        Battleground* arena = sBattlegroundMgr->CreateNewBattleground(bgTypeId, bracketEntry, arenaType);
        if (!arena)
        {
            TC_LOG_ERROR("bg.battleground", "BattlegroundQueue::Update couldn't create arena instance for rated arena match!");
            return;
        }

        firstTeam->OpponentsTeamRating = secondTeam->ArenaTeamRating;
        firstTeam->OpponentsMatchmakerRating = secondTeam->ArenaMatchmakerRating;

        secondTeam->OpponentsTeamRating = firstTeam->ArenaTeamRating;
        secondTeam->OpponentsMatchmakerRating = firstTeam->ArenaMatchmakerRating;

        if (firstTeam->Team == HORDE && secondTeam->Team == ALLIANCE)
            std::swap(firstTeam, secondTeam);

        arena->SetArenaMatchmakerRating(ALLIANCE, firstTeam->ArenaMatchmakerRating);
        arena->SetArenaMatchmakerRating(HORDE, secondTeam->ArenaMatchmakerRating);

        InviteGroupToBG(firstTeam, arena, ALLIANCE);
        InviteGroupToBG(secondTeam, arena, HORDE);

        TC_LOG_DEBUG("bg.battleground", "Starting rated arena match!");

        arena->StartBattleground();
    }
}

// this method tries to create battleground or arena with MinPlayersPerTeam against MinPlayersPerTeam
bool BattlegroundQueue::CheckNormalMatch(BattlegroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers)
{
    GroupsQueueType::const_iterator itr_team[BG_TEAMS_COUNT];
    for (uint32 i = 0; i < BG_TEAMS_COUNT; i++)
    {
        itr_team[i] = m_QueuedGroups[bracket_id][BG_QUEUE_ALLIANCE + i].begin();
        for (; itr_team[i] != m_QueuedGroups[bracket_id][BG_QUEUE_ALLIANCE + i].end(); ++(itr_team[i]))
        {
            if (!(*(itr_team[i]))->IsInvitedToBGInstanceGUID)
            {
                m_SelectionPools[i].AddGroup(*(itr_team[i]), maxPlayers);
                if (m_SelectionPools[i].GetPlayerCount() >= minPlayers)
                    break;
            }
        }
    }
    //try to invite same number of players - this cycle may cause longer wait time even if there are enough players in queue, but we want ballanced bg
    uint32 j = TEAM_ALLIANCE;
    if (m_SelectionPools[TEAM_HORDE].GetPlayerCount() < m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount())
        j = TEAM_HORDE;
    if (sWorld->getIntConfig(CONFIG_BATTLEGROUND_INVITATION_TYPE) != 0
        && m_SelectionPools[TEAM_HORDE].GetPlayerCount() >= minPlayers && m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() >= minPlayers)
    {
        //we will try to invite more groups to team with less players indexed by j
        ++(itr_team[j]);                                         //this will not cause a crash, because for cycle above reached break;
        for (; itr_team[j] != m_QueuedGroups[bracket_id][BG_QUEUE_ALLIANCE + j].end(); ++(itr_team[j]))
        {
            if (!(*(itr_team[j]))->IsInvitedToBGInstanceGUID)
                if (!m_SelectionPools[j].AddGroup(*(itr_team[j]), m_SelectionPools[(j + 1) % BG_TEAMS_COUNT].GetPlayerCount()))
                    break;
        }
        // do not allow to start bg with more than 2 players more on 1 faction
        if (abs((int32)(m_SelectionPools[TEAM_HORDE].GetPlayerCount() - m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount())) > 2)
            return false;
    }
    //allow 1v0 if debug bg
    if (sBattlegroundMgr->isTesting() && (m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() || m_SelectionPools[TEAM_HORDE].GetPlayerCount()))
        return true;
    //return true if there are enough players in selection pools - enable to work .debug bg command correctly
    return m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() >= minPlayers && m_SelectionPools[TEAM_HORDE].GetPlayerCount() >= minPlayers;
}

/*
this method is called when group is inserted, or player / group is removed from BG Queue - there is only one player's status changed, so we don't use while (true) cycles to invite whole queue
it must be called after fully adding the members of a group to ensure group joining
should be called from Battleground::RemovePlayer function in some cases
*/
void BattlegroundQueue::update(BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id, uint8 arenaType, uint32 arenaRating)
{
    //if no players in queue - do nothing
    if (m_QueuedGroups[bracket_id][BG_QUEUE_ALLIANCE].empty()
        && m_QueuedGroups[bracket_id][BG_QUEUE_HORDE].empty()
        && m_QueuedGroups[bracket_id][BG_QUEUE_MIXED].empty())
    {
        return;
    }

    // battleground with free slot for player should be always in the beginning of the queue
    // maybe it would be better to create bgfreeslotqueue for each bracket_id
    BGFreeSlotQueueContainer& bgQueues = sBattlegroundMgr->GetBGFreeSlotQueueStore(bgTypeId);
    for (BGFreeSlotQueueContainer::iterator itr = bgQueues.begin(); itr != bgQueues.end();)
    {
        Battleground* bg = *itr; ++itr;
        // DO NOT allow queue manager to invite new player to rated games
        if (!bg->isRated() && bg->GetTypeID() == bgTypeId && bg->GetBracketId() == bracket_id &&
            bg->GetStatus() > STATUS_WAIT_QUEUE && bg->GetStatus() < STATUS_WAIT_LEAVE)
        {
            // clear selection pools
            m_SelectionPools[TEAM_ALLIANCE].Init();
            m_SelectionPools[TEAM_HORDE].Init();

            // call a function that does the job for us
            FillPlayersToBG(bg, bracket_id);

            // now everything is set, invite players
            for (GroupsQueueType::const_iterator citr = m_SelectionPools[TEAM_ALLIANCE].SelectedGroups.begin(); citr != m_SelectionPools[TEAM_ALLIANCE].SelectedGroups.end(); ++citr)
                InviteGroupToBG((*citr), bg, (*citr)->Team);

            for (GroupsQueueType::const_iterator citr = m_SelectionPools[TEAM_HORDE].SelectedGroups.begin(); citr != m_SelectionPools[TEAM_HORDE].SelectedGroups.end(); ++citr)
                InviteGroupToBG((*citr), bg, (*citr)->Team);

            if (!bg->HasFreeSlots())
                bg->RemoveFromBGFreeSlotQueue();
        }
    }

    // finished iterating through the bgs with free slots, maybe we need to create a new bg

    Battleground* bg_template = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg_template)
    {
        TC_LOG_ERROR("bg.battleground", "Battleground: Update: bg template not found for %u", bgTypeId);
        return;
    }

    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketById(bg_template->GetMapId(), bracket_id);
    if (!bracketEntry)
    {
        TC_LOG_ERROR("bg.battleground", "Battleground: Update: bg bracket entry not found for map %u bracket id %u", bg_template->GetMapId(), bracket_id);
        return;
    }

    // get the min. players per team, properly for larger arenas as well. (must have full teams for arena matches!)
    uint32 MinPlayersPerTeam = bg_template->GetMinPlayersPerTeam();
    uint32 MaxPlayersPerTeam = bg_template->GetMaxPlayersPerTeam();

    if (bg_template->isArena())
    {
        MaxPlayersPerTeam = arenaType;
        MinPlayersPerTeam = sBattlegroundMgr->isArenaTesting() ? 1 : arenaType;
    }
    else if (sBattlegroundMgr->isTesting())
        MinPlayersPerTeam = 1;

    m_SelectionPools[TEAM_ALLIANCE].Init();
    m_SelectionPools[TEAM_HORDE].Init();

    if (bg_template->isBattleground() && !bg_template->isRated())
    {
        if (CheckNormalMatch(bracket_id, MinPlayersPerTeam, MaxPlayersPerTeam)
            || CheckCrossFactionMatch(bg_template, bracket_id))
        {
            // create new battleground
            Battleground* bg2 = sBattlegroundMgr->CreateNewBattleground(bgTypeId, bracketEntry, 0);
            if (!bg2)
            {
                TC_LOG_ERROR("bg.battleground", "BattlegroundQueue::Update - Cannot create battleground: %u", bgTypeId);
                return;
            }
            // invite those selection pools
            for (uint32 i = 0; i < BG_TEAMS_COUNT; i++)
                for (GroupsQueueType::const_iterator citr = m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.begin(); citr != m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.end(); ++citr)
                    InviteGroupToBG((*citr), bg2, (*citr)->Team);

            bg2->StartBattleground();
        }

        return;
    }

    // found out the minimum and maximum ratings the newly added team should battle against
    // arenaRating is the rating of the latest joined team, or 0
    // 0 is on (automatic update call) and we must set it to team's with longest wait time
    if (!arenaRating)
    {
        GroupQueueInfo *front1 = NULL;
        GroupQueueInfo *front2 = NULL;

        {
            if (!m_QueuedGroups[bracket_id][BG_QUEUE_ALLIANCE].empty())
            {
                front1 = m_QueuedGroups[bracket_id][BG_QUEUE_ALLIANCE].front();
                arenaRating = front1->ArenaMatchmakerRating;
            }
            if (!m_QueuedGroups[bracket_id][BG_QUEUE_HORDE].empty())
            {
                front2 = m_QueuedGroups[bracket_id][BG_QUEUE_HORDE].front();
                arenaRating = front2->ArenaMatchmakerRating;
            }
        }

        if (front1 && front2)
        {
            if (front1->JoinTime < front2->JoinTime)
                arenaRating = front1->ArenaMatchmakerRating;
        }
        else if (!front1 && !front2)
            return; // queues are empty
    }
    
    if (bg_template->isRated() && bg_template->isBattleground())
        updateRatedBgQueue(bgTypeId, bracketEntry, arenaRating);
    else if (bg_template->isArena())
        updateRatedArenaQueue(bgTypeId, bracketEntry, arenaType, arenaRating);
}
