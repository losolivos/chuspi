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
 * You should have received a copy of the GNU General Public    License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __BATTLEGROUNDQUEUE_H
#define __BATTLEGROUNDQUEUE_H

#include "Common.h"
#include "DBCEnums.h"
#include "Battleground.h"

#include <deque>
#include <list>
#include <map>

//this container can't be deque, because deque doesn't like removing the last element - if you remove it, it invalidates next iterator and crash appears
typedef std::list<Battleground*> BGFreeSlotQueueContainer;

#define COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME 10

struct GroupQueueInfo;                                      // type predefinition
struct PlayerQueueInfo                                      // stores information for players in queue
{
    uint32  LastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
    GroupQueueInfo* GroupInfo;                             // pointer to the associated groupqueueinfo
};

struct GroupQueueInfo                                       // stores information about the group in queue (also used when joined as solo!)
{
    std::map<uint64, PlayerQueueInfo*> Players;             // player queue info map
    uint32  Team;                                           // Player team (ALLIANCE/HORDE)
    uint32  OriginalTeam;
    bool IsTeamChanged;                                     // Player team was changed to allow same faction battle
    BattlegroundTypeId BgTypeId;                            // battleground type id
    bool    IsRated;                                        // rated
    uint8   ArenaType;                                      // 2v2, 3v3, 5v5 or 0 when BG
    uint32  JoinTime;                                       // time when group was added
    uint32  RemoveInviteTime;                               // time when we will remove invite for players in group
    uint32  IsInvitedToBGInstanceGUID;                      // was invited to certain BG
    uint32  ArenaTeamRating;                                // if rated match, inited to the rating of the team
    uint32  ArenaMatchmakerRating;                          // if rated match, inited to the rating of the team
    uint32  OpponentsTeamRating;                            // for rated arena matches
    uint32  OpponentsMatchmakerRating;                      // for rated arena matches
};

enum BattlegroundQueueGroupTypes
{
    BG_QUEUE_ALLIANCE          = 0,
    BG_QUEUE_HORDE             = 1,
    BG_QUEUE_MIXED             = 2,
    BG_QUEUE_GROUP_TYPES_COUNT
};

class Battleground;
class BattlegroundQueue
{
    public:
        BattlegroundQueue();
        ~BattlegroundQueue();

        void update(BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id, uint8 arenaType = 0, uint32 minRating = 0);

        bool FillCrossPlayersToBG(Battleground* bg, BattlegroundBracketId bracket_id, bool start = false);
        typedef std::multimap<int32, GroupQueueInfo*> QueuedGroupMap;
        int32 PreAddPlayers(QueuedGroupMap m_PreGroupMap, int32 MaxAdd, uint32 MaxInTeam);
        bool CheckCrossFactionMatch(Battleground* bg, BattlegroundBracketId bracket_id);

        void FillPlayersToBG(Battleground* bg, BattlegroundBracketId bracket_id);
        bool CheckNormalMatch(BattlegroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers);
        GroupQueueInfo* AddGroup(Player* leader, Group* group, BattlegroundTypeId bgTypeId, PvPDifficultyEntry const*  bracketEntry, uint8 ArenaType, uint32 ArenaRating, uint32 MatchmakerRating);
        void RemovePlayer(uint64 guid, bool decreaseInvitedCount);
        bool IsPlayerInvited(uint64 pl_guid, const uint32 bgInstanceGuid, const uint32 removeTime);
        bool GetPlayerGroupInfoData(uint64 guid, GroupQueueInfo* ginfo);
        void PlayerInvitedToBGUpdateAverageWaitTime(GroupQueueInfo* ginfo, BattlegroundBracketId bracket_id);
        uint32 GetAverageQueueWaitTime(GroupQueueInfo* ginfo, BattlegroundBracketId bracket_id) const;

        void updateRatedBgQueue(BattlegroundTypeId bgTypeId, PvPDifficultyEntry const *bracketEntry, uint32 matchmakerRating);
        void updateRatedArenaQueue(BattlegroundTypeId bgTypeId, PvPDifficultyEntry const *bracketEntry, uint8 arenaType, uint32 arenaRating);

        typedef std::map<uint64, PlayerQueueInfo> QueuedPlayersMap;
        QueuedPlayersMap m_QueuedPlayers;

        //we need constant add to begin and constant remove / add from the end, therefore deque suits our problem well
        typedef std::deque<GroupQueueInfo*> GroupsQueueType;

        /*
        This two dimensional array is used to store All queued groups
        First dimension specifies the bgTypeId
        Second dimension specifies the player's group types -
             BG_QUEUE_ALLIANCE   is used for normal (or small) alliance groups or non-rated arena matches
             BG_QUEUE_HORDE      is used for normal (or small) horde groups or non-rated arena matches
        */
        GroupsQueueType m_QueuedGroups[MAX_BATTLEGROUND_BRACKETS][BG_QUEUE_GROUP_TYPES_COUNT];

        // class to select and invite groups to bg
        class SelectionPool
        {
        public:
            SelectionPool() : PlayerCount(0) { }

            void Init();
            bool AddGroup(GroupQueueInfo* ginfo, uint32 desiredCount);
            bool KickGroup(uint32 size);
            uint32 GetPlayerCount() const {return PlayerCount;}
        public:
            GroupsQueueType SelectedGroups;
        private:
            uint32 PlayerCount;
        };

        //one selection pool for horde, other one for alliance
        SelectionPool m_SelectionPools[BG_TEAMS_COUNT];

        bool InviteGroupToBG(GroupQueueInfo* ginfo, Battleground* bg, uint32 side);

    private:
        uint32 m_WaitTimes[BG_TEAMS_COUNT][MAX_BATTLEGROUND_BRACKETS][COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME];
        uint32 m_WaitTimeLastPlayer[BG_TEAMS_COUNT][MAX_BATTLEGROUND_BRACKETS];
        uint32 m_SumOfWaitTimes[BG_TEAMS_COUNT][MAX_BATTLEGROUND_BRACKETS];
};

#endif
