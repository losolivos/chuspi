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

#include "Common.h"
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"

#include "Arena.h"
#include "BattlegroundMgr.h"
#include "BattlegroundAV.h"
#include "BattlegroundAB.h"
#include "BattlegroundEY.h"
#include "BattlegroundWS.h"
#include "BattlegroundNA.h"
#include "BattlegroundBE.h"
#include "BattlegroundRL.h"
#include "BattlegroundSA.h"
#include "BattlegroundDS.h"
#include "BattlegroundRV.h"
#include "BattlegroundIC.h"
#include "BattlegroundTP.h"
#include "BattlegroundBFG.h"
#include "BattlegroundKT.h"
#include "BattlegroundSSM.h"
#include "BattlegroundTTP.h"
#include "BattlegroundTA.h"
#include "BattlegroundDG.h"
#include "Chat.h"
#include "Map.h"
#include "MapInstanced.h"
#include "MapManager.h"
#include "Player.h"
#include "GameEventMgr.h"
#include "SharedDefines.h"
#include "Formulas.h"
#include "DisableMgr.h"
#include "LFG.h"

/*********************************************************/
/***            BATTLEGROUND MANAGER                   ***/
/*********************************************************/

BattlegroundMgr::BattlegroundMgr() :
    m_nextRatingUpdate(sWorld->getIntConfig(CONFIG_ARENA_RATING_DIFFERENCE_UPDATE_TIMER)),
    m_ArenaTesting(false), m_Testing(false)
{ }

BattlegroundMgr::~BattlegroundMgr()
{
    DeleteAllBattlegrounds();
}

void BattlegroundMgr::DeleteAllBattlegrounds()
{
    for (BattlegroundDataContainer::iterator itr1 = bgDataStore.begin(); itr1 != bgDataStore.end(); ++itr1)
    {
        BattlegroundData& data = itr1->second;

        while (!data.m_Battlegrounds.empty())
            delete data.m_Battlegrounds.begin()->second;
        data.m_Battlegrounds.clear();

        while (!data.BGFreeSlotQueue.empty())
            delete data.BGFreeSlotQueue.front();
    }

    bgDataStore.clear();
}

// used to update running battlegrounds, and delete finished ones
void BattlegroundMgr::Update(uint32 diff)
{
    for (BattlegroundDataContainer::iterator itr1 = bgDataStore.begin(); itr1 != bgDataStore.end(); ++itr1)
    {
        BattlegroundContainer& bgs = itr1->second.m_Battlegrounds;
        BattlegroundContainer::iterator itrDelete = bgs.begin();
        // first one is template and should not be deleted
        for (BattlegroundContainer::iterator itr = ++itrDelete; itr != bgs.end();)
        {
            itrDelete = itr++;
            Battleground* bg = itrDelete->second;

            bg->Update(diff);
            if (bg->ToBeDeleted())
            {
                itrDelete->second = NULL;
                bgs.erase(itrDelete);
                BattlegroundClientIdsContainer& clients = itr1->second.m_ClientBattlegroundIds[bg->GetBracketId()];
                if (!clients.empty())
                    clients.erase(bg->GetClientInstanceID());

                delete bg;
            }
        }
    }

    // update scheduled queues
    if (!m_QueueUpdateScheduler.empty())
    {
        decltype(m_QueueUpdateScheduler) schedulerCopy;
        std::swap(schedulerCopy, m_QueueUpdateScheduler);

        for (auto const &scheduled : schedulerCopy)
        {
            auto const &arenaMMRating = scheduled.arenaMMRating;
            auto const &arenaType = scheduled.arenaType;
            auto const &bgQueueTypeId = scheduled.bgQueueTypeId;
            auto const &bgTypeId = scheduled.bgTypeId;
            auto const &bracketId = scheduled.bracketId;

            m_BattlegroundQueues[bgQueueTypeId].update(bgTypeId, bracketId, arenaType, arenaMMRating);
        }
    }

    // if rating difference counts, maybe force-update queues
    if (m_nextRatingUpdate < diff)
    {
        // forced update for rated arenas (scan all, but skipped non rated)
        TC_LOG_TRACE("bg.arena", "BattlegroundMgr: UPDATING ARENA QUEUES");
        for (int qtype = BATTLEGROUND_QUEUE_2v2; qtype <= BATTLEGROUND_QUEUE_5v5; ++qtype)
        {
            // Rated arenas are only for level 85 players, no need to loop over all brackets
            m_BattlegroundQueues[qtype].update(BATTLEGROUND_AA, BG_BRACKET_ID_LAST,
                BattlegroundMgr::BGArenaType(BattlegroundQueueTypeId(qtype)), 0);
        }

        m_BattlegroundQueues[BATTLEGROUND_QUEUE_RATED].update(BATTLEGROUND_RA_BG, BG_BRACKET_ID_LAST);

        m_nextRatingUpdate = sWorld->getIntConfig(CONFIG_ARENA_RATING_DIFFERENCE_UPDATE_TIMER);
    }
    else
        m_nextRatingUpdate -= diff;
}

void BattlegroundMgr::BuildBattlegroundStatusPacket(WorldPacket* data, Battleground* bg, Player * player, uint8 queueSlot, uint8 statusId, uint32 time1, uint32 time2, uint8 /*globalQueueType*/)
{
    // we can be in 2 queues in same time...
    if (!bg)
        statusId = STATUS_NONE;

    ObjectGuid requesterGuid = player->GetGUID();
    ObjectGuid bg_guid = bg ? bg->GetGUID() : 0;

    // GlobalQueueType is hard coded to 5 (GLOBAL_QUEUE_TYPE_ARENA) are there other values for BG's ect?
    switch (statusId)
    {
        case STATUS_NONE:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS, 4 + 4 + 4 + 1 + 8);

            // --- CliRideTicket ---
            *data << uint32(GLOBAL_QUEUE_TYPE_ARENA);                   // GlobalQueueType
            *data << uint32(queueSlot);                                 // ID
            *data << uint32(time1);                                     // Timestamp (Join Time)

            data->WriteBitSeq<1, 2, 4, 6, 5, 7, 0, 3>(requesterGuid);
            data->WriteByteSeq<7, 6, 4, 3, 0, 1, 2, 5>(requesterGuid);
            // ---------------------

            break;
        }
        case STATUS_WAIT_QUEUE:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS_QUEUED, 1 + 8 + 1 + 8 + 1 + 4
                + 1 + 4 + 4 + 4 + 1 + 1 + 4 + 4 + 4);

            data->WriteBitSeq<7>(bg_guid);
            data->WriteBitSeq<6>(requesterGuid);
            data->WriteBit(1);                                          // EligibleForMatchmaking
            data->WriteBitSeq<0>(bg_guid);
            data->WriteBit(bg->isRated());                              // TournamentRules
            data->WriteBit(0);                                          // AsGroup
            data->WriteBitSeq<5, 1, 3>(requesterGuid);
            data->WriteBitSeq<6>(bg_guid);
            data->WriteBitSeq<4, 0>(requesterGuid);
            data->WriteBitSeq<4>(bg_guid);
            data->WriteBit(0);                                          // SuspendedQueue
            data->WriteBitSeq<5>(bg_guid);
            data->WriteBitSeq<7>(requesterGuid);
            data->WriteBitSeq<1, 2>(bg_guid);
            data->WriteBitSeq<2>(requesterGuid);
            data->WriteBitSeq<3>(bg_guid);
            data->FlushBits();

            data->WriteByteSeq<1>(bg_guid);
            *data << uint32(time1);                                     // Timestamp (Estimated Wait Time)
            *data << uint8(0);                                          // RangeMax
            *data << uint32(time2);                                     // Timestamp2 (Join Time)
            *data << uint32(GetMSTimeDiffToNow(time2));
            data->WriteByteSeq<2, 3>(requesterGuid);
            data->WriteByteSeq<5, 3>(bg_guid);
            data->WriteByteSeq<5>(requesterGuid);
            *data << uint8(bg->GetMinLevel());
            *data << uint8(0);                                          // RangeMin
            data->WriteByteSeq<4>(bg_guid);
            *data << uint32(queueSlot);
            *data << uint32(GLOBAL_QUEUE_TYPE_ARENA);                   // GlobalQueueType
            data->WriteByteSeq<2>(bg_guid);
            *data << uint32(bg->GetClientInstanceID());                 // InstanceID
            data->WriteByteSeq<1, 0, 4>(requesterGuid);
            data->WriteByteSeq<0, 6, 7>(bg_guid);
            data->WriteByteSeq<6, 7>(requesterGuid);
            break;
        }
        case STATUS_WAIT_JOIN:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS_NEED_CONFIRMATION, 1 + 8 + 1 + 8
                + 1 + 1 + 4 + 1 + 4 + 4 + 1 + 4 + 1 + 4 + 4);

            // FIXME: the queue system must be similar to LFG, but passed roles
            // are unused now.

            int32 role;
            if (auto const specId = player->GetSpecializationId(player->GetActiveSpec()))
                role = sChrSpecializationsStore.LookupEntry(specId)->role;
            else
                role = player->getClass() != CLASS_PRIEST ? SPEC_ROLE_DAMAGE : SPEC_ROLE_HEALING;

            data->WriteBitSeq<4, 6, 1, 3>(bg_guid);
            data->WriteBitSeq<2, 3>(requesterGuid);
            data->WriteBitSeq<2>(bg_guid);
            data->WriteBitSeq<5>(requesterGuid);
            data->WriteBit(role == SPEC_ROLE_DAMAGE);
            data->WriteBitSeq<0, 7, 5>(bg_guid);
            data->WriteBitSeq<7, 0, 6>(requesterGuid);
            data->WriteBit(bg->isRated());                              // TournamentRules
            data->WriteBitSeq<1, 4>(requesterGuid);
            data->FlushBits();

            if (role != SPEC_ROLE_DAMAGE)
                *data << int8(role);

            *data << uint32(bg->GetClientInstanceID());
            data->WriteByteSeq<5>(bg_guid);
            *data << uint8(0);                                          // RangeMax
            *data << uint32(GLOBAL_QUEUE_TYPE_ARENA);                   // GlobalQueueType
            data->WriteByteSeq<0, 6>(requesterGuid);
            *data << uint32(time2);                                     // Timestamp2 (Timeout)
            *data << uint8(bg->GetMinLevel());
            data->WriteByteSeq<1, 5, 2>(requesterGuid);
            data->WriteByteSeq<4>(bg_guid);
            data->WriteByteSeq<7>(requesterGuid);
            data->WriteByteSeq<1, 7, 0>(bg_guid);
             *data << uint32(time1);
            data->WriteByteSeq<6, 2>(bg_guid);
            *data << uint8(0);                                          // RangeMin
            data->WriteByteSeq<3, 4>(requesterGuid);
            *data << uint32(bg->GetMapId());
            *data << uint32(queueSlot);
            data->WriteByteSeq<3>(bg_guid);
            break;
        }
        case STATUS_IN_PROGRESS:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS_ACTIVE, 1 + 8 + 1 + 8 + 1 + 1
                + 1 + 1 + 4 + 4 + 4 + 4 + 4 + 4 + 4);

            data->WriteBit(player->GetTeam() == HORDE ? 0 : 1);       // Battlefield Faction ( 0 horde, 1 alliance )
            data->WriteBitSeq<1, 7>(bg_guid);
            data->WriteBitSeq<0, 5, 3>(requesterGuid);
            data->WriteBitSeq<0>(bg_guid);
            data->WriteBitSeq<2, 7>(requesterGuid);
            data->WriteBitSeq<4>(bg_guid);
            data->WriteBit(bg->isRated());                              // byte38
            data->WriteBitSeq<6>(requesterGuid);
            data->WriteBitSeq<2, 5, 6, 3>(bg_guid);
            data->WriteBitSeq<4, 1>(requesterGuid);
            data->FlushBits();

            *data << uint8(0);                                          // RangeMin
            *data << uint8(bg->GetMinLevel());
            *data << uint8(0);                                          // RangeMax
            data->WriteByteSeq<7, 0, 1, 2>(requesterGuid);
            data->WriteByteSeq<7>(bg_guid);
            *data << uint32(time2);                                     // Timestamp (Elapsed Time)
            *data << uint32(bg->GetClientInstanceID());
            *data << uint32(bg->GetMapId());
            data->WriteByteSeq<4, 3, 1>(bg_guid);
            *data << uint32(GLOBAL_QUEUE_TYPE_ARENA);                   // GlobalQueueType
            data->WriteByteSeq<3, 4>(requesterGuid);
            *data << uint32(0);
            data->WriteByteSeq<2>(bg_guid);
            data->WriteByteSeq<5>(requesterGuid);
            data->WriteByteSeq<6>(bg_guid);
            *data << uint32(GetMSTimeDiffToNow(time2));
            data->WriteByteSeq<5>(bg_guid);
            *data << uint32(queueSlot);
            data->WriteByteSeq<0>(bg_guid);
            data->WriteByteSeq<6>(requesterGuid);
            break;
        }
        case STATUS_WAIT_LEAVE:
        {
          /*  data->Initialize(SMSG_BATTLEFIELD_STATUS_WAIT_FOR_GROUPS, 48);

            *data << uint8(0);                          // unk
            *data << uint32(bg->GetStatus());           // Status
            *data << uint32(QueueSlot);                 // Queue slot
            *data << uint32(Time1);                     // Time until closed
            *data << uint32(0);                         // unk
            *data << uint8(0);                          // unk
            *data << uint8(0);                          // unk
            *data << uint8(bg->GetMinLevel());          // Min Level
            *data << uint8(0);                          // unk
            *data << uint8(0);                          // unk
            *data << uint32(bg->GetMapId());            // Map Id
            *data << uint32(Time2);                     // Time
            *data << uint8(0);                          // unk

            data->WriteBit(bg_guid[0]);
            data->WriteBit(bg_guid[1]);
            data->WriteBit(bg_guid[7]);
            data->WriteBit(player_guid[7]);
            data->WriteBit(player_guid[0]);
            data->WriteBit(bg_guid[4]);
            data->WriteBit(player_guid[6]);
            data->WriteBit(player_guid[2]);
            data->WriteBit(player_guid[3]);
            data->WriteBit(bg_guid[3]);
            data->WriteBit(player_guid[4]);
            data->WriteBit(bg_guid[5]);
            data->WriteBit(player_guid[5]);
            data->WriteBit(bg_guid[2]);
            data->WriteBit(bg->isRated());              // Is Rated
            data->WriteBit(player_guid[1]);
            data->WriteBit(bg_guid[6]);

            data->FlushBits();

            data->WriteByteSeq(player_guid[0]);
            data->WriteByteSeq(bg_guid[4]);
            data->WriteByteSeq(player_guid[3]);
            data->WriteByteSeq(bg_guid[1]);
            data->WriteByteSeq(bg_guid[0]);
            data->WriteByteSeq(bg_guid[2]);
            data->WriteByteSeq(player_guid[2]);
            data->WriteByteSeq(bg_guid[7]);
            data->WriteByteSeq(player_guid[1]);
            data->WriteByteSeq(player_guid[6]);
            data->WriteByteSeq(bg_guid[6]);
            data->WriteByteSeq(bg_guid[5]);
            data->WriteByteSeq(player_guid[5]);
            data->WriteByteSeq(player_guid[4]);
            data->WriteByteSeq(player_guid[7]);
            data->WriteByteSeq(bg_guid[3]);*/
            break;
        }
    }
}

void BattlegroundMgr::BuildPvpLogDataPacket(WorldPacket* data, Battleground* bg)
{
    uint8 isRated = (bg->isRated() ? 1 : 0);               // type (normal=0/rated=1) -- ATM arena or bg, RBG NYI
    uint8 isArena = (bg->isArena() ? 1 : 0);               // Arena names

    data->Initialize(SMSG_PVP_LOG_DATA);

    Battleground::BattlegroundScoreMap::const_iterator itr2 = bg->GetPlayerScoresBegin();
    for (Battleground::BattlegroundScoreMap::const_iterator itr = itr2; itr != bg->GetPlayerScoresEnd();)
    {
        itr2 = itr++;
        if (!isArena && !bg->IsPlayerInBattleground(itr2->first))
        {
            TC_LOG_ERROR("bg.battleground", "Player " UI64FMTD " has scoreboard entry for battleground %u but is not in battleground!", itr->first, bg->GetTypeID(true));
            continue;
        }

        Player* player = ObjectAccessor::FindPlayer(itr2->first);
        if (!player)
            continue;
    }

    *data << uint8(bg->GetPlayersCountByTeam(ALLIANCE));
    *data << uint8(bg->GetPlayersCountByTeam(HORDE));
    data->WriteBit(bg->GetStatus() == STATUS_WAIT_LEAVE);    // If Ended
    data->WriteBit(isRated);                                 // HaveArenaData
    data->WriteBits(bg->GetPlayerScoresSize(), 19);

    itr2 = bg->GetPlayerScoresBegin();
    for (Battleground::BattlegroundScoreMap::const_iterator itr = itr2; itr != bg->GetPlayerScoresEnd();)
    {
        itr2 = itr++;
        if (!isArena && !bg->IsPlayerInBattleground(itr2->first))
        {
            TC_LOG_ERROR("bg.battleground", "Player " UI64FMTD " has scoreboard entry for battleground %u but is not in battleground!", itr->first, bg->GetTypeID(true));
            continue;
        }

        ObjectGuid guid = itr2->first;
        Player* player = ObjectAccessor::FindPlayer(itr2->first);
        if (!player)
            continue;

        data->WriteBitSeq<6, 0>(guid);
        data->WriteBit(0); // unkbit1
        data->WriteBitSeq<1, 7>(guid);
        data->WriteBit(1); // unkbit2

        if (isArena)
            data->WriteBit(bg->GetPlayerTeam(guid) == ALLIANCE);
        else
            data->WriteBit(player->GetTeam() == ALLIANCE);

        data->WriteBitSeq<2>(guid);
        data->WriteBit(0); // unkbit4
        data->WriteBitSeq<4>(guid);

        switch (bg->GetTypeID(true))                             // Custom values
        {
            case BATTLEGROUND_RB:
                switch (bg->GetMapId())
                {
                    case 489:
                    case 529:
                    case 607:
                    case 628:
                    case 726:
                        data->WriteBits(0x00000002, 22);
                        break;
                    case 761:
                        data->WriteBits(0x00000002, 22);
                        break;
                    case 30:
                        data->WriteBits(0x00000005, 22);
                        break;
                    case 566:
                        data->WriteBits(0x00000001, 22);
                        break;
                    default:
                        data->WriteBits(0, 22);
                        break;
                }
                break;
            case BATTLEGROUND_AV:
                data->WriteBits(0x00000005, 22);
                break;
            case BATTLEGROUND_EY:
            case BATTLEGROUND_SSM:
                data->WriteBits(0x00000001, 22);
                break;
            case BATTLEGROUND_WS:
            case BATTLEGROUND_AB:
            case BATTLEGROUND_SA:
            case BATTLEGROUND_IC:
            case BATTLEGROUND_TP:
            case BATTLEGROUND_BFG:
            case BATTLEGROUND_KT:
                data->WriteBits(0x00000002, 22);
                break;
            case BATTLEGROUND_DG:
                data->WriteBits(0x00000004, 22);
                break;
            default:
                data->WriteBits(0, 22);
                break;
        }

        data->WriteBitSeq<5, 3>(guid);
        data->WriteBit(!isArena); // HaveBonusData
        data->WriteBit(0); // unkbit6
        data->WriteBit(isArena);
    }

    data->WriteBit(false);                                 // HaveArenaData2

    data->FlushBits();

    /*if (isArena)
    {
        ArenaTeam* at1 = sArenaTeamMgr->GetArenaTeamById(bg->GetArenaTeamIdByIndex(0));
        ArenaTeam* at2 = sArenaTeamMgr->GetArenaTeamById(bg->GetArenaTeamIdByIndex(1));
        ObjectGuid TeamGuid1 = 0;
        ObjectGuid TeamGuid2 = 0;

        if (at1)
            data->WriteBits(at1->GetName().length(), 7);
        else
            data->WriteBits(0, 7);
        data->WriteBitSeq<2>(TeamGuid1);
        data->WriteBitSeq<0, 7, 2>(TeamGuid2);
        data->WriteBitSeq<5, 6, 4, 3>(TeamGuid1);
        data->WriteBitSeq<6>(TeamGuid2);
        data->WriteBitSeq<1, 0>(TeamGuid1);
        data->WriteBitSeq<1, 5>(TeamGuid2);
        if (at1)
            data->WriteBits(at2->GetName().length(), 7);
        else
            data->WriteBits(0, 7);
        data->WriteBitSeq<4, 3>(TeamGuid2);
        data->WriteBitSeq<7>(TeamGuid1);

        data->WriteByteSeq<4, 0>(TeamGuid2);
        data->WriteByteSeq<6>(TeamGuid1);
        data->WriteByteSeq<3, 6>(TeamGuid2);

        if (at2)
            data->WriteString(at2->GetName());

        data->WriteByteSeq<3>(TeamGuid1);

        if (at1)
            data->WriteString(at1->GetName());

        data->WriteByteSeq<7>(TeamGuid2);
        data->WriteByteSeq<1, 5, 7>(TeamGuid1);
        data->WriteByteSeq<5>(TeamGuid2);
        data->WriteByteSeq<4>(TeamGuid1);
        data->WriteByteSeq<1>(TeamGuid2);
        data->WriteByteSeq<2, 0>(TeamGuid1);
        data->WriteByteSeq<2>(TeamGuid2);
    }*/

    itr2 = bg->GetPlayerScoresBegin();
    for (Battleground::BattlegroundScoreMap::const_iterator itr = itr2; itr != bg->GetPlayerScoresEnd();)
    {
        itr2 = itr++;
        if (!isArena && !bg->IsPlayerInBattleground(itr2->first))
        {
            TC_LOG_ERROR("bg.battleground", "Player " UI64FMTD " has scoreboard entry for battleground %u but is not in battleground!", itr->first, bg->GetTypeID(true));
            continue;
        }

        ObjectGuid guid = itr2->first;
        Player* player = ObjectAccessor::FindPlayer(itr2->first);

        if (!player)
            continue;

        *data << uint32(itr2->second->KillingBlows);
        data->WriteByteSeq<4, 0>(guid);

        if (!isArena) // HaveBonusData
        {
            *data << uint32(itr2->second->BonusHonor / 100);
            *data << uint32(itr2->second->Deaths);
            *data << uint32(itr2->second->HonorableKills);
        }

        *data << uint32(itr2->second->HealingDone);             // healing done

        //if (unkbit6)
        //    *data << uint32(dword15);

        data->WriteByteSeq<1>(guid);
        *data << uint32(player->GetSpecializationId(player->GetActiveSpec()));
        data->WriteByteSeq<7>(guid);

        //if (unkbit4)
        //    *data << uint32(dword13);
        //if (unkbit1)
        //    *data << uint32(dword9);

        data->WriteByteSeq<6, 5, 3>(guid);

        switch (bg->GetTypeID(true))                             // Custom values
        {
            case BATTLEGROUND_RB:
                switch (bg->GetMapId())
                {
                    case 489:
                        *data << uint32(((BattlegroundWGScore*)itr2->second)->FlagCaptures);        // flag captures
                        *data << uint32(((BattlegroundWGScore*)itr2->second)->FlagReturns);         // flag returns
                        break;
                    case 566:
                        *data << uint32(((BattlegroundEYScore*)itr2->second)->FlagCaptures);        // flag captures
                        break;
                    case 529:
                        *data << uint32(((BattlegroundABScore*)itr2->second)->BasesAssaulted);      // bases asssulted
                        *data << uint32(((BattlegroundABScore*)itr2->second)->BasesDefended);       // bases defended
                        break;
                    case 30:
                        *data << uint32(((BattlegroundAVScore*)itr2->second)->GraveyardsAssaulted); // GraveyardsAssaulted
                        *data << uint32(((BattlegroundAVScore*)itr2->second)->GraveyardsDefended);  // GraveyardsDefended
                        *data << uint32(((BattlegroundAVScore*)itr2->second)->TowersAssaulted);     // TowersAssaulted
                        *data << uint32(((BattlegroundAVScore*)itr2->second)->TowersDefended);      // TowersDefended
                        *data << uint32(((BattlegroundAVScore*)itr2->second)->MinesCaptured);       // MinesCaptured
                        break;
                    case 607:
                        *data << uint32(((BattlegroundSAScore*)itr2->second)->demolishers_destroyed);
                        *data << uint32(((BattlegroundSAScore*)itr2->second)->gates_destroyed);
                        break;
                    case 628:                                   // IC
                        *data << uint32(((BattlegroundICScore*)itr2->second)->BasesAssaulted);       // bases asssulted
                        *data << uint32(((BattlegroundICScore*)itr2->second)->BasesDefended);        // bases defended
                        break;
                    case 726:
                        *data << uint32(((BattlegroundTPScore*)itr2->second)->FlagCaptures);         // flag captures
                        *data << uint32(((BattlegroundTPScore*)itr2->second)->FlagReturns);          // flag returns
                        break;
                    case 761:
                        *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesAssaulted);      // bases asssulted
                        *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesDefended);       // bases defended
                        break;
                    case 727:
                        *data << uint32(((BattlegroundSSMScore*)itr2->second)->MineCartCaptures);    // mine carts captured
                        break;
                    case 1105:
                        *data << uint32(((BattlegroundDGScore*)itr2->second)->cartCaptured);
                        *data << uint32(((BattlegroundDGScore*)itr2->second)->cartReturned);
                        *data << uint32(((BattlegroundDGScore*)itr2->second)->minesAssaulted);
                        *data << uint32(((BattlegroundDGScore*)itr2->second)->minesDefended);
                        break;
                }
                break;
            case BATTLEGROUND_AV:
                *data << uint32(((BattlegroundAVScore*)itr2->second)->GraveyardsAssaulted); // GraveyardsAssaulted
                *data << uint32(((BattlegroundAVScore*)itr2->second)->GraveyardsDefended);  // GraveyardsDefended
                *data << uint32(((BattlegroundAVScore*)itr2->second)->TowersAssaulted);     // TowersAssaulted
                *data << uint32(((BattlegroundAVScore*)itr2->second)->TowersDefended);      // TowersDefended
                *data << uint32(((BattlegroundAVScore*)itr2->second)->MinesCaptured);       // MinesCaptured
                break;
            case BATTLEGROUND_WS:
                *data << uint32(((BattlegroundWGScore*)itr2->second)->FlagCaptures);        // flag captures
                *data << uint32(((BattlegroundWGScore*)itr2->second)->FlagReturns);         // flag returns
                break;
            case BATTLEGROUND_AB:
                *data << uint32(((BattlegroundABScore*)itr2->second)->BasesAssaulted);      // bases asssulted
                *data << uint32(((BattlegroundABScore*)itr2->second)->BasesDefended);       // bases defended
                break;
            case BATTLEGROUND_EY:
                *data << uint32(((BattlegroundEYScore*)itr2->second)->FlagCaptures);        // flag captures
                break;
            case BATTLEGROUND_SA:
                *data << uint32(((BattlegroundSAScore*)itr2->second)->demolishers_destroyed);
                *data << uint32(((BattlegroundSAScore*)itr2->second)->gates_destroyed);
                break;
            case BATTLEGROUND_IC:
                *data << uint32(((BattlegroundICScore*)itr2->second)->BasesAssaulted);       // bases asssulted
                *data << uint32(((BattlegroundICScore*)itr2->second)->BasesDefended);        // bases defended
                break;
            case BATTLEGROUND_TP:
                *data << uint32(((BattlegroundTPScore*)itr2->second)->FlagCaptures);         // flag captures
                *data << uint32(((BattlegroundTPScore*)itr2->second)->FlagReturns);          // flag returns
                break;
            case BATTLEGROUND_BFG:
                *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesAssaulted);      // bases asssulted
                *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesDefended);       // bases defended
                break;
            case BATTLEGROUND_KT:
                *data << uint32(((BattleGroundKTScore*)itr2->second)->OrbHandles);
                *data << uint32(((BattleGroundKTScore*)itr2->second)->Score);
                break;
            case BATTLEGROUND_DG:
                *data << uint32(((BattlegroundDGScore*)itr2->second)->cartCaptured);
                *data << uint32(((BattlegroundDGScore*)itr2->second)->cartReturned);
                *data << uint32(((BattlegroundDGScore*)itr2->second)->minesAssaulted);
                *data << uint32(((BattlegroundDGScore*)itr2->second)->minesDefended);
                break;
            case BATTLEGROUND_SSM:
                *data << uint32(((BattlegroundSSMScore*)itr2->second)->MineCartCaptures);    // mine carts captured
                break;
            default:
                break;
        }

        *data << uint32(itr2->second->DamageDone);              // damage done
        data->WriteByteSeq<2>(guid);
        if (isArena)
            *data << int32(itr2->second->RatingChange);
    }

    if (isRated)                                             // arena TODO : Fix Order on Rated Implementation
    {
        // it seems this must be according to BG_WINNER_A/H and _NOT_ BG_TEAM_A/H
        for (int8 i = BG_TEAMS_COUNT - 1; i >= 0; --i)
        {
            int32 rating_change = bg->GetArenaTeamRatingChangeByIndex(i);

            uint32 pointsLost = rating_change < 0 ? -rating_change : 0;
            uint32 pointsGained = rating_change > 0 ? rating_change : 0;
            uint32 MatchmakerRating = bg->GetArenaMatchmakerRatingByIndex(i);

            if(i == 1)
            {
                *data << uint32(pointsLost);                    // Rating Lost
                *data << uint32(MatchmakerRating);              // Matchmaking Value
                *data << uint32(pointsGained);                  // Rating gained
            }
            else
            {
                //FIXME: duplicate?
                *data << uint32(pointsLost);                    // Rating Lost
                *data << uint32(MatchmakerRating);              // Matchmaking Value
                *data << uint32(pointsGained);                  // Rating gained
            }

            TC_LOG_DEBUG("bg.battleground", "rating change: %d", rating_change);
        }
    }

    if (bg->GetStatus() == STATUS_WAIT_LEAVE)
        *data << uint8(bg->GetWinner());                    // who win
}

void BattlegroundMgr::BuildStatusFailedPacket(WorldPacket* data, Battleground* bg, Player* player, uint8 QueueSlot, GroupJoinBattlegroundResult result)
{
    ObjectGuid player_guid = player->GetGUID(); // player who caused the error
    ObjectGuid bg_guid = bg->GetGUID();
    ObjectGuid unkguid = 0;

    data->Initialize(SMSG_BATTLEFIELD_STATUS_FAILED);

    data->WriteBitSeq<3>(player_guid);
    data->WriteBitSeq<3>(bg_guid);
    data->WriteBitSeq<5, 1, 4>(player_guid);
    data->WriteBitSeq<2>(bg_guid);
    data->WriteBitSeq<0>(player_guid);
    data->WriteBitSeq<6>(bg_guid);
    data->WriteBitSeq<6>(player_guid);
    data->WriteBitSeq<0>(unkguid);
    data->WriteBitSeq<2>(player_guid);
    data->WriteBitSeq<4, 3, 5, 1>(unkguid);
    data->WriteBitSeq<7>(player_guid);
    data->WriteBitSeq<1>(bg_guid);
    data->WriteBitSeq<2>(unkguid);
    data->WriteBitSeq<7>(bg_guid);
    data->WriteBitSeq<7, 6>(unkguid);
    data->WriteBitSeq<5, 4, 0>(bg_guid);
    data->FlushBits();

    data->WriteByteSeq<7>(player_guid);
    data->WriteByteSeq<5>(unkguid);
    data->WriteByteSeq<6>(player_guid);
    data->WriteByteSeq<3>(unkguid);
    data->WriteByteSeq<5>(player_guid);
    data->WriteByteSeq<2>(unkguid);
    data->WriteByteSeq<6>(bg_guid);
    data->WriteByteSeq<6>(unkguid);
    data->WriteByteSeq<3>(player_guid);
    data->WriteByteSeq<1, 7>(bg_guid);

    *data << uint32(QueueSlot);                 // Queue slot

    data->WriteByteSeq<4>(unkguid);
    data->WriteByteSeq<4>(bg_guid);
    data->WriteByteSeq<0>(unkguid);
    data->WriteByteSeq<4, 2>(player_guid);

    *data << uint32(bg->isArena() ? bg->GetMaxPlayersPerTeam() : 1);                         // Unk, always 1

    data->WriteByteSeq<0>(bg_guid);
    data->WriteByteSeq<1>(unkguid);
    data->WriteByteSeq<5>(bg_guid);

    *data << uint32(result);

    data->WriteByteSeq<0>(player_guid);
    data->WriteByteSeq<7>(unkguid);
    data->WriteByteSeq<2>(bg_guid);
    data->WriteByteSeq<1>(player_guid);

    *data << uint32(player->GetBattlegroundQueueJoinTime(bg->GetTypeID())); // Join Time RANDOM

    data->WriteByteSeq<3>(bg_guid);
}

void BattlegroundMgr::BuildUpdateWorldStatePacket(WorldPacket* data, uint32 field, uint32 value)
{
    data->Initialize(SMSG_UPDATE_WORLD_STATE, 4+4);
    *data << uint32(field);
    *data << uint32(value);
    data->WriteBit(0);
    data->FlushBits();
}

void BattlegroundMgr::BuildPlayerLeftBattlegroundPacket(WorldPacket* data, uint64 guid)
{
    ObjectGuid guidBytes = guid;

    data->Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT, 8);
    data->WriteBitSeq<5, 6, 1, 7, 0, 2, 4, 3>(guidBytes);
    data->WriteByteSeq<6, 1, 5, 7, 4, 0, 3, 2>(guidBytes);
}

void BattlegroundMgr::BuildPlayerJoinedBattlegroundPacket(WorldPacket* data, uint64 guid)
{
    ObjectGuid playerGuid = guid;

    data->Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    data->WriteBitSeq<0, 1, 7, 2, 4, 3, 5, 6>(playerGuid);
    data->WriteByteSeq<2, 3, 1, 0, 5, 4, 7, 6>(playerGuid);
}

Battleground* BattlegroundMgr::GetBattleground(uint32 instanceId, BattlegroundTypeId bgTypeId)
{
    if (!instanceId)
        return NULL;

    BattlegroundDataContainer::const_iterator begin, end;

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        begin = bgDataStore.begin();
        end = bgDataStore.end();
    }
    else
    {
        end = bgDataStore.find(bgTypeId);
        if (end == bgDataStore.end())
            return NULL;
        begin = end++;
    }

    for (BattlegroundDataContainer::const_iterator it = begin; it != end; ++it)
    {
        BattlegroundContainer const& bgs = it->second.m_Battlegrounds;
        BattlegroundContainer::const_iterator itr = bgs.find(instanceId);
        if (itr != bgs.end())
           return itr->second;
    }

    return NULL;
}

Battleground* BattlegroundMgr::GetBattlegroundTemplate(BattlegroundTypeId bgTypeId)
{
    BattlegroundDataContainer::const_iterator itr = bgDataStore.find(bgTypeId);
    if (itr == bgDataStore.end())
        return NULL;

    BattlegroundContainer const& bgs = itr->second.m_Battlegrounds;
    // map is sorted and we can be sure that lowest instance id has only BG template
    return bgs.empty() ? NULL : bgs.begin()->second;
}

uint32 BattlegroundMgr::CreateClientVisibleInstanceId(BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id)
{
    if (IsArenaType(bgTypeId))
        return 0;                                           //arenas don't have client-instanceids

    // we create here an instanceid, which is just for
    // displaying this to the client and without any other use..
    // the client-instanceIds are unique for each battleground-type
    // the instance-id just needs to be as low as possible, beginning with 1
    // the following works, because std::set is default ordered with "<"
    // the optimalization would be to use as bitmask std::vector<uint32> - but that would only make code unreadable

    BattlegroundClientIdsContainer& clientIds = bgDataStore[bgTypeId].m_ClientBattlegroundIds[bracket_id];
    uint32 lastId = 0;
    for (BattlegroundClientIdsContainer::const_iterator itr = clientIds.begin(); itr != clientIds.end(); ++itr)
    {
        if ((lastId + 1) != *itr)                             //if there is a gap between the ids, we will break..
            break;
        lastId = *itr;
    }

    clientIds.insert(++lastId);

    return lastId;
}

// create a new battleground that will really be used to play
Battleground* BattlegroundMgr::CreateNewBattleground(BattlegroundTypeId originalBgTypeId, PvPDifficultyEntry const* bracketEntry, uint8 arenaType, bool rated)
{
    BattlegroundTypeId bgTypeId = originalBgTypeId;
    bool isRated = false;
    bool isRandom = false;

    switch (originalBgTypeId)
    {
        case BATTLEGROUND_RB:
            isRandom = true;
            bgTypeId = GetRandomBG(originalBgTypeId);
            break;
        case BATTLEGROUND_RA_BG:
            /// Intentional fallback, "All Arenas" is random too
        case BATTLEGROUND_AA:
            isRated = rated;
            bgTypeId = GetRandomBG(originalBgTypeId);
            break;
        default:
            break;
    }

    // get the template BG
    Battleground* bg_template = GetBattlegroundTemplate(bgTypeId);

    if (!bg_template)
    {
        TC_LOG_ERROR("bg.battleground", "Battleground: CreateNewBattleground - bg template not found for %u", bgTypeId);
        return NULL;
    }

    Battleground* bg = NULL;
    // create a copy of the BG template
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattlegroundAV(*(BattlegroundAV*)bg_template);
            break;
        case BATTLEGROUND_WS:
            bg = new BattlegroundWS(*(BattlegroundWS*)bg_template);
            break;
        case BATTLEGROUND_AB:
            bg = new BattlegroundAB(*(BattlegroundAB*)bg_template);
            break;
        case BATTLEGROUND_NA:
            bg = new BattlegroundNA(*(BattlegroundNA*)bg_template);
            break;
        case BATTLEGROUND_TA:
            bg = new BattlegroundTA(*(BattlegroundTA*)bg_template);
            break;
        case BATTLEGROUND_TTP:
            bg = new BattlegroundTTP(*(BattlegroundTTP*)bg_template);
            break;
        case BATTLEGROUND_BE:
            bg = new BattlegroundBE(*(BattlegroundBE*)bg_template);
            break;
        case BATTLEGROUND_EY:
            bg = new BattlegroundEY(*(BattlegroundEY*)bg_template);
            break;
        case BATTLEGROUND_RL:
            bg = new BattlegroundRL(*(BattlegroundRL*)bg_template);
            break;
        case BATTLEGROUND_SA:
            bg = new BattlegroundSA(*(BattlegroundSA*)bg_template);
            break;
        case BATTLEGROUND_DS:
            bg = new BattlegroundDS(*(BattlegroundDS*)bg_template);
            break;
        case BATTLEGROUND_RV:
            bg = new BattlegroundRV(*(BattlegroundRV*)bg_template);
            break;
        case BATTLEGROUND_IC:
            bg = new BattlegroundIC(*(BattlegroundIC*)bg_template);
            break;
        case BATTLEGROUND_TP:
            bg = new BattlegroundTP(*(BattlegroundTP*)bg_template);
            break;
        case BATTLEGROUND_BFG:
            bg = new BattlegroundBFG(*(BattlegroundBFG*)bg_template);
            break;
        case BATTLEGROUND_KT:
            bg = new BattlegroundKT(*(BattlegroundKT*)bg_template);
            break;
        case BATTLEGROUND_SSM:
            bg = new BattlegroundSSM(*(BattlegroundSSM*)bg_template);
            break;
        case BATTLEGROUND_DG:
            bg = new BattlegroundDG(*(BattlegroundDG*)bg_template);
            break;
        case BATTLEGROUND_RB:
        case BATTLEGROUND_RA_BG:
        case BATTLEGROUND_AA:
            bg = new Battleground(*bg_template);
            break;
        default:
            return NULL;
    }

    bg->SetBracket(bracketEntry);
    bg->SetInstanceID(sMapMgr->GenerateInstanceId());
    bg->SetClientInstanceID(CreateClientVisibleInstanceId(isRandom ? BATTLEGROUND_RB : bgTypeId, bracketEntry->GetBracketId()));
    bg->Reset();
    bg->SetStatus(STATUS_WAIT_JOIN);
    bg->SetArenaType(arenaType);
    bg->SetTypeID(originalBgTypeId);
    bg->SetRandomTypeID(bgTypeId);
    bg->SetRated(isRated);
    bg->SetRandom(isRandom);
    bg->SetGuid(MAKE_NEW_GUID(isRandom ? BATTLEGROUND_RB : bgTypeId, 0, HIGHGUID_BATTLEGROUND));

    // Set up correct min/max player counts for scoreboards
    if (bg->isArena())
    {
        uint32 maxPlayersPerTeam = 0;
        switch (arenaType)
        {
            case ARENA_TYPE_2v2:
                maxPlayersPerTeam = 2;
                break;
            case ARENA_TYPE_3v3:
                maxPlayersPerTeam = 3;
                break;
            case ARENA_TYPE_5v5:
                maxPlayersPerTeam = 5;
                break;
        }

        bg->SetMaxPlayersPerTeam(maxPlayersPerTeam);
        bg->SetMaxPlayers(maxPlayersPerTeam * 2);
    }

    return bg;
}

// used to create the BG templates
bool BattlegroundMgr::CreateBattleground(CreateBattlegroundData& data)
{
    // Create the BG
    Battleground* bg = NULL;
    switch (data.bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattlegroundAV;
            break;
        case BATTLEGROUND_WS:
            bg = new BattlegroundWS;
            break;
        case BATTLEGROUND_AB:
            bg = new BattlegroundAB;
            break;
        case BATTLEGROUND_NA:
            bg = new BattlegroundNA;
            break;
        case BATTLEGROUND_TA:
            bg = new BattlegroundTA;
            break;
        case BATTLEGROUND_TTP:
            bg = new BattlegroundTTP;
            break;
        case BATTLEGROUND_BE:
            bg = new BattlegroundBE;
            break;
        case BATTLEGROUND_EY:
            bg = new BattlegroundEY;
            break;
        case BATTLEGROUND_RL:
            bg = new BattlegroundRL;
            break;
        case BATTLEGROUND_SA:
            bg = new BattlegroundSA;
            break;
        case BATTLEGROUND_DS:
            bg = new BattlegroundDS;
            break;
        case BATTLEGROUND_RV:
            bg = new BattlegroundRV;
            break;
        case BATTLEGROUND_IC:
            bg = new BattlegroundIC;
            break;
        case BATTLEGROUND_AA:
            bg = new Battleground;
            break;
        case BATTLEGROUND_RB:
            bg = new Battleground;
            bg->SetRandom(true);
            break;
        case BATTLEGROUND_TP:
            bg = new BattlegroundTP;
            break;
        case BATTLEGROUND_BFG:
            bg = new BattlegroundBFG;
            break;
        case BATTLEGROUND_KT:
            bg = new BattlegroundKT;
            break;
        case BATTLEGROUND_DG:
            bg = new BattlegroundDG;
            break;
        case BATTLEGROUND_SSM:
            bg = new BattlegroundSSM;
            break;
        case BATTLEGROUND_RA_BG:
            bg = new Battleground;
            bg->SetRated(true);
            break;
        default:
            return false;
    }

    bg->SetMapId(data.MapID);
    bg->SetTypeID(data.bgTypeId);
    bg->SetInstanceID(0);
    bg->SetArenaorBGType(data.IsArena);
    bg->SetMinPlayersPerTeam(data.MinPlayersPerTeam);
    bg->SetMaxPlayersPerTeam(data.MaxPlayersPerTeam);
    bg->SetMinPlayers(data.MinPlayersPerTeam * 2);
    bg->SetMaxPlayers(data.MaxPlayersPerTeam * 2);
    bg->SetName(data.BattlegroundName);
    bg->SetTeamStartLoc(ALLIANCE, data.Team1StartLocX, data.Team1StartLocY, data.Team1StartLocZ, data.Team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    data.Team2StartLocX, data.Team2StartLocY, data.Team2StartLocZ, data.Team2StartLocO);
    bg->SetStartMaxDist(data.StartMaxDist);
    bg->SetLevelRange(data.LevelMin, data.LevelMax);
    bg->SetScriptId(data.scriptId);
    bg->SetGuid(MAKE_NEW_GUID(data.bgTypeId, 0, HIGHGUID_BATTLEGROUND));

    AddBattleground(bg);

    return true;
}

void BattlegroundMgr::CreateInitialBattlegrounds()
{
    uint32 oldMSTime = getMSTime();

    //                                               0   1                  2                  3       4       5                 6               7              8            9             10      11
    QueryResult result = WorldDatabase.Query("SELECT id, MinPlayersPerTeam, MaxPlayersPerTeam, MinLvl, MaxLvl, AllianceStartLoc, AllianceStartO, HordeStartLoc, HordeStartO, StartMaxDist, Weight, ScriptName FROM battleground_template");

    if (!result)
    {
        TC_LOG_ERROR("server.loading", ">> Loaded 0 battlegrounds. DB table `battleground_template` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 bgTypeId = fields[0].GetUInt32();
        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, bgTypeId, NULL))
            continue;

        // can be overwrite by values from DB
        auto const bl = sBattlemasterListStore.LookupEntry(bgTypeId);
        if (!bl)
        {
            TC_LOG_ERROR("bg.battleground", "Battleground ID %u not found in BattlemasterList.dbc. Battleground not created.", bgTypeId);
            continue;
        }

        CreateBattlegroundData data;
        data.bgTypeId = BattlegroundTypeId(bgTypeId);
        data.IsArena = (bl->type == TYPE_ARENA);
        data.MinPlayersPerTeam = fields[1].GetUInt16();
        data.MaxPlayersPerTeam = fields[2].GetUInt16();
        data.LevelMin = fields[3].GetUInt8();
        data.LevelMax = fields[4].GetUInt8();
        float dist = fields[9].GetFloat();
        data.StartMaxDist = dist * dist;

        data.scriptId = sObjectMgr->GetScriptId(fields[11].GetCString());
        data.BattlegroundName = bl->name;
        data.MapID = bl->mapid[0];

        // check values from DB
        if (data.MaxPlayersPerTeam == 0 || data.MinPlayersPerTeam > data.MaxPlayersPerTeam)
        {
            TC_LOG_ERROR("sql.sql", "Table `battleground_template` for id %u has bad values for MinPlayersPerTeam (%u) and MaxPlayersPerTeam(%u)",
                data.bgTypeId, data.MinPlayersPerTeam, data.MaxPlayersPerTeam);
            continue;
        }

        if (data.LevelMin == 0 || data.LevelMax == 0 || data.LevelMin > data.LevelMax)
        {
            TC_LOG_ERROR("sql.sql", "Table `battleground_template` for id %u has bad values for LevelMin (%u) and LevelMax(%u)",
                data.bgTypeId, data.LevelMin, data.LevelMax);
            continue;
        }

        if (data.bgTypeId == BATTLEGROUND_AA || data.bgTypeId == BATTLEGROUND_RB || data.bgTypeId == BATTLEGROUND_RA_BG)
        {
            data.Team1StartLocX = 0;
            data.Team1StartLocY = 0;
            data.Team1StartLocZ = 0;
            data.Team1StartLocO = fields[6].GetFloat();
            data.Team2StartLocX = 0;
            data.Team2StartLocY = 0;
            data.Team2StartLocZ = 0;
            data.Team2StartLocO = fields[8].GetFloat();
        }
        else
        {
            uint32 startId = fields[5].GetUInt32();
            if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(startId))
            {
                data.Team1StartLocX = start->x;
                data.Team1StartLocY = start->y;
                data.Team1StartLocZ = start->z;
                data.Team1StartLocO = fields[6].GetFloat();
            }
            else
            {
                TC_LOG_ERROR("sql.sql", "Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.", data.bgTypeId, startId);
                continue;
            }

            startId = fields[7].GetUInt32();
            if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(startId))
            {
                data.Team2StartLocX = start->x;
                data.Team2StartLocY = start->y;
                data.Team2StartLocZ = start->z;
                data.Team2StartLocO = fields[8].GetFloat();
            }
            else
            {
                TC_LOG_ERROR("sql.sql", "Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.", data.bgTypeId, startId);
                continue;
            }
        }

        if (!CreateBattleground(data))
            continue;

        if (data.IsArena)
        {
            if (data.bgTypeId != BATTLEGROUND_AA)
                m_ArenaSelectionWeights[data.bgTypeId] = fields[10].GetUInt8();
        }
        else if (data.bgTypeId != BATTLEGROUND_RB && data.bgTypeId != BATTLEGROUND_RA_BG)
            m_BGSelectionWeights[data.bgTypeId] = fields[10].GetUInt8() * 120 / data.MaxPlayersPerTeam;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u battlegrounds in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BattlegroundMgr::BuildBattlegroundListPacket(WorldPacket* data, ObjectGuid guid, Player* player, BattlegroundTypeId bgTypeId)
{
    if (!player)
        return;

    BattlegroundDataContainer::iterator it = bgDataStore.find(bgTypeId);
    if (it == bgDataStore.end())
        return;

    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(it->second.m_Battlegrounds.begin()->second->GetMapId(), player->getLevel());
    if (!bracketEntry)
        return;

    uint32 winnerConquest = (player->GetRandomWinner() ? BG_REWARD_WINNER_CONQUEST_LAST : BG_REWARD_WINNER_CONQUEST_FIRST) / CURRENCY_PRECISION;
    uint32 winnerHonor = (player->GetRandomWinner() ? BG_REWARD_WINNER_HONOR_LAST : BG_REWARD_WINNER_HONOR_FIRST) / CURRENCY_PRECISION;

    ByteBuffer dataBuffer;

    data->Initialize(SMSG_BATTLEFIELD_LIST);
    data->WriteBit(0); // unk1
    data->WriteBitSeq<4>(guid);
    data->WriteBit(1); // byte2C
    data->WriteBitSeq<2, 6>(guid);

    if (bgTypeId == BATTLEGROUND_AA)                         // arena
    {
        data->WriteBits(0, 22);
    }
    else                                                    // battleground
    {
        uint32 count = 0;
        BattlegroundBracketId bracketId = bracketEntry->GetBracketId();
        BattlegroundClientIdsContainer& clientIds = it->second.m_ClientBattlegroundIds[bracketId];
        for (BattlegroundClientIdsContainer::const_iterator itr = clientIds.begin(); itr != clientIds.end(); ++itr)
        {
            dataBuffer << uint32(*itr);
            ++count;
        }
        data->WriteBits(count, 22);
    }

    data->WriteBit(0); // byte40
    data->WriteBitSeq<7>(guid);
    data->WriteBit(1); // byte50
    data->WriteBitSeq<1, 0, 3, 5>(guid);

    *data << uint32(winnerConquest); // dword28
    data->WriteByteSeq<4>(guid);
    *data << uint32(BG_REWARD_LOSER_HONOR); // dword20
    data->WriteByteSeq<2, 7>(guid);
    *data << uint32(bgTypeId); // dword24
    *data << uint8(bracketEntry->maxLevel); // byte41
    data->WriteByteSeq<1, 5, 0>(guid);
    *data << uint32(BG_REWARD_LOSER_HONOR); // dword18
    *data << uint8(bracketEntry->minLevel); // byte48
    data->WriteByteSeq<3>(guid);
    *data << uint32(winnerHonor); // dword4C
    *data << uint32(winnerConquest); // dword1C
    data->append(dataBuffer);
    data->WriteByteSeq<6>(guid);
    *data << uint32(winnerHonor); // dword44
}

void BattlegroundMgr::SendToBattleground(Player* player, uint32 instanceId, BattlegroundTypeId bgTypeId)
{
    Battleground* bg = GetBattleground(instanceId, bgTypeId);
    if (bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, O;
        uint32 team = player->GetTeam();
        if (team == 0)
            team = player->GetTeam();
        bg->GetTeamStartLoc(team, x, y, z, O);

        TC_LOG_INFO("bg.battleground", "BATTLEGROUND: Sending %s to map %u, X %f, Y %f, Z %f, O %f", player->GetName().c_str(), mapid, x, y, z, O);
        player->TeleportTo(mapid, x, y, z, O);
    }
    else
    {
        TC_LOG_ERROR("bg.battleground", "player %u is trying to port to non-existent bg instance %u", player->GetGUIDLow(), instanceId);
    }
}

void BattlegroundMgr::SendAreaSpiritHealerQueryOpcode(Player* player, Battleground* bg, uint64 guid)
{
    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    ObjectGuid npcGuid = guid;

    uint32 time = 30000 - bg->GetLastResurrectTime();      // resurrect every 30 seconds
    if (time == uint32(-1))
        time = 0;

    data.WriteBitSeq<0, 4, 2, 6, 3, 5, 1, 7>(npcGuid);

    data.WriteByteSeq<4, 5, 3, 2, 6, 7>(npcGuid);
    data << uint32(time);
    data.WriteByteSeq<0, 1>(npcGuid);

    player->GetSession()->SendPacket(&data);
}

bool BattlegroundMgr::IsArenaType(BattlegroundTypeId bgTypeId)
{
    return bgTypeId == BATTLEGROUND_AA
            || bgTypeId == BATTLEGROUND_BE
            || bgTypeId == BATTLEGROUND_NA
            || bgTypeId == BATTLEGROUND_DS
            || bgTypeId == BATTLEGROUND_RV
            || bgTypeId == BATTLEGROUND_RL
            || bgTypeId == BATTLEGROUND_TA
            || bgTypeId == BATTLEGROUND_TTP;
}

BattlegroundQueueTypeId BattlegroundMgr::BGQueueTypeId(BattlegroundTypeId bgTypeId, uint8 arenaType)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_WS:
            return BATTLEGROUND_QUEUE_WS;
        case BATTLEGROUND_AB:
            return BATTLEGROUND_QUEUE_AB;
        case BATTLEGROUND_AV:
            return BATTLEGROUND_QUEUE_AV;
        case BATTLEGROUND_EY:
            return BATTLEGROUND_QUEUE_EY;
        case BATTLEGROUND_SA:
            return BATTLEGROUND_QUEUE_SA;
        case BATTLEGROUND_IC:
            return BATTLEGROUND_QUEUE_IC;
        case BATTLEGROUND_TP:
            return BATTLEGROUND_QUEUE_TP;
        case BATTLEGROUND_BFG:
            return BATTLEGROUND_QUEUE_BFG;
        case BATTLEGROUND_RB:
            return BATTLEGROUND_QUEUE_RB;
        case BATTLEGROUND_KT:
            return BATTLEGROUND_QUEUE_KT;
        case BATTLEGROUND_CTF3:
            return BATTLEGROUND_QUEUE_CTF3;
        case BATTLEGROUND_SSM:
            return BATTLEGROUND_QUEUE_SSM;
        case BATTLEGROUND_DG:
            return BATTLEGROUND_QUEUE_DG;
        case BATTLEGROUND_AA:
        case BATTLEGROUND_NA:
        case BATTLEGROUND_RL:
        case BATTLEGROUND_BE:
        case BATTLEGROUND_DS:
        case BATTLEGROUND_RV:
        case BATTLEGROUND_TA:
        case BATTLEGROUND_TTP:
            switch (arenaType)
            {
                case ARENA_TYPE_2v2:
                    return BATTLEGROUND_QUEUE_2v2;
                case ARENA_TYPE_3v3:
                    return BATTLEGROUND_QUEUE_3v3;
                case ARENA_TYPE_5v5:
                    return BATTLEGROUND_QUEUE_5v5;
                default:
                    return BATTLEGROUND_QUEUE_NONE;
            }
        case BATTLEGROUND_RA_BG:
            return BATTLEGROUND_QUEUE_RATED;
        default:
            return BATTLEGROUND_QUEUE_NONE;
    }
}

BattlegroundTypeId BattlegroundMgr::BGTemplateId(BattlegroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_WS:
            return BATTLEGROUND_WS;
        case BATTLEGROUND_QUEUE_AB:
            return BATTLEGROUND_AB;
        case BATTLEGROUND_QUEUE_AV:
            return BATTLEGROUND_AV;
        case BATTLEGROUND_QUEUE_EY:
            return BATTLEGROUND_EY;
        case BATTLEGROUND_QUEUE_SA:
            return BATTLEGROUND_SA;
        case BATTLEGROUND_QUEUE_IC:
            return BATTLEGROUND_IC;
        case BATTLEGROUND_QUEUE_TP:
            return BATTLEGROUND_TP;
        case BATTLEGROUND_QUEUE_BFG:
            return BATTLEGROUND_BFG;
        case BATTLEGROUND_QUEUE_RB:
            return BATTLEGROUND_RB;
        case BATTLEGROUND_QUEUE_KT:
            return BATTLEGROUND_KT;
        case BATTLEGROUND_QUEUE_CTF3:
            return BATTLEGROUND_CTF3;
        case BATTLEGROUND_QUEUE_SSM:
            return BATTLEGROUND_SSM;
        case BATTLEGROUND_QUEUE_DG:
            return BATTLEGROUND_DG;
        case BATTLEGROUND_QUEUE_2v2:
        case BATTLEGROUND_QUEUE_3v3:
        case BATTLEGROUND_QUEUE_5v5:
            return BATTLEGROUND_AA;
        case BATTLEGROUND_QUEUE_RATED:
            return BATTLEGROUND_RA_BG;
        default:
            return BattlegroundTypeId(0);                   // used for unknown template (it existed and do nothing)
    }
}

uint8 BattlegroundMgr::BGArenaType(BattlegroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_2v2:
            return ARENA_TYPE_2v2;
        case BATTLEGROUND_QUEUE_3v3:
            return ARENA_TYPE_3v3;
        case BATTLEGROUND_QUEUE_5v5:
            return ARENA_TYPE_5v5;
        default:
            return 0;
    }
}

void BattlegroundMgr::ToggleTesting()
{
    m_Testing = !m_Testing;
    if (m_Testing)
        sWorld->SendWorldText(LANG_DEBUG_BG_ON);
    else
        sWorld->SendWorldText(LANG_DEBUG_BG_OFF);
}

void BattlegroundMgr::ToggleArenaTesting()
{
    m_ArenaTesting = !m_ArenaTesting;
    if (m_ArenaTesting)
        sWorld->SendWorldText(LANG_DEBUG_ARENA_ON);
    else
        sWorld->SendWorldText(LANG_DEBUG_ARENA_OFF);
}

void BattlegroundMgr::SetHolidayWeekends(uint32 bgQueueTypeId)
{
    for (BattlegroundDataContainer::const_iterator itr = bgDataStore.begin(); itr != bgDataStore.end(); ++itr)
    {
        BattlegroundContainer const& bgs = itr->second.m_Battlegrounds;

        if (!bgs.empty())
            bgs.begin()->second->SetHoliday(bgQueueTypeId == itr->first);
    }
}

void BattlegroundMgr::ScheduleQueueUpdate(uint32 arenaMatchmakerRating, uint8 arenaType, BattlegroundQueueTypeId bgQueueTypeId, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracketId)
{
    //This method must be atomic, TODO add mutex
    //we will use only 1 number created of bgTypeId and bracket_id

    QueueSchedulerItem const scheduleId = {
        arenaMatchmakerRating, arenaType, bgQueueTypeId, bgTypeId, bracketId
    };

    if (std::find(m_QueueUpdateScheduler.begin(), m_QueueUpdateScheduler.end(), scheduleId) == m_QueueUpdateScheduler.end())
        m_QueueUpdateScheduler.push_back(scheduleId);
}

uint32 BattlegroundMgr::GetPrematureFinishTime() const
{
    return sWorld->getIntConfig(CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

HolidayIds BattlegroundMgr::BGTypeToWeekendHolidayId(BattlegroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV: return HOLIDAY_CALL_TO_ARMS_AV;
        case BATTLEGROUND_EY: return HOLIDAY_CALL_TO_ARMS_EY;
        case BATTLEGROUND_WS: return HOLIDAY_CALL_TO_ARMS_WS;
        case BATTLEGROUND_SA: return HOLIDAY_CALL_TO_ARMS_SA;
        case BATTLEGROUND_AB: return HOLIDAY_CALL_TO_ARMS_AB;
        case BATTLEGROUND_IC: return HOLIDAY_CALL_TO_ARMS_IC;
        case BATTLEGROUND_TP: return HOLIDAY_CALL_TO_ARMS_TP;
        case BATTLEGROUND_BFG: return HOLIDAY_CALL_TO_ARMS_BFG;
        case BATTLEGROUND_SSM: return HOLIDAY_CALL_TO_ARMS_SM;
        case BATTLEGROUND_KT: return HOLIDAY_CALL_TO_ARMS_KT;
        case BATTLEGROUND_DG: return HOLIDAY_CALL_TO_ARMS_DG;
        default: return HOLIDAY_NONE;
    }
}

BattlegroundTypeId BattlegroundMgr::WeekendHolidayIdToBGType(HolidayIds holiday)
{
    switch (holiday)
    {
        case HOLIDAY_CALL_TO_ARMS_AV: return BATTLEGROUND_AV;
        case HOLIDAY_CALL_TO_ARMS_EY: return BATTLEGROUND_EY;
        case HOLIDAY_CALL_TO_ARMS_WS: return BATTLEGROUND_WS;
        case HOLIDAY_CALL_TO_ARMS_SA: return BATTLEGROUND_SA;
        case HOLIDAY_CALL_TO_ARMS_AB: return BATTLEGROUND_AB;
        case HOLIDAY_CALL_TO_ARMS_IC: return BATTLEGROUND_IC;
        case HOLIDAY_CALL_TO_ARMS_TP: return BATTLEGROUND_TP;
        case HOLIDAY_CALL_TO_ARMS_BFG: return BATTLEGROUND_BFG;
        case HOLIDAY_CALL_TO_ARMS_SM: return BATTLEGROUND_SSM;
        case HOLIDAY_CALL_TO_ARMS_KT: return BATTLEGROUND_KT;
        case HOLIDAY_CALL_TO_ARMS_DG: return BATTLEGROUND_DG;
        default: return BATTLEGROUND_TYPE_NONE;
    }
}

bool BattlegroundMgr::IsBGWeekend(BattlegroundTypeId bgTypeId)
{
    return IsHolidayActive(BGTypeToWeekendHolidayId(bgTypeId));
}

BattlegroundTypeId BattlegroundMgr::GetRandomBG(BattlegroundTypeId bgTypeId)
{
    uint32 weight = 0;
    BattlegroundTypeId returnBgTypeId = BATTLEGROUND_TYPE_NONE;
    BattlegroundSelectionWeightMap selectionWeights;

    if (bgTypeId == BATTLEGROUND_AA)
    {
        for (BattlegroundSelectionWeightMap::const_iterator it = m_ArenaSelectionWeights.begin(); it != m_ArenaSelectionWeights.end(); ++it)
        {
            if (it->second)
            {
                weight += it->second;
                selectionWeights[it->first] = it->second;
            }
        }
    }
    else if (bgTypeId == BATTLEGROUND_RB || bgTypeId == BATTLEGROUND_RA_BG)
    {
        uint8 const size = (bgTypeId == BATTLEGROUND_RA_BG) ? 15 : 0;

        for (BattlegroundSelectionWeightMap::const_iterator it = m_BGSelectionWeights.begin(); it != m_BGSelectionWeights.end(); ++it)
        {
            // Filter the bg's by size
            if (size)
            {
                auto const bg = GetBattlegroundTemplate(it->first);
                if (!bg || bg->GetMaxPlayersPerTeam() > size)
                    continue;
            }

            if (it->second)
            {
                weight += it->second;
                selectionWeights[it->first] = it->second;
            }
        }
    }

    if (weight)
    {
        // Select a random value
        uint32 selectedWeight = urand(0, weight - 1);
        // Select the correct bg (if we have in DB A(10), B(20), C(10), D(15) --> [0---A---9|10---B---29|30---C---39|40---D---54])
        weight = 0;
        for (BattlegroundSelectionWeightMap::const_iterator it = selectionWeights.begin(); it != selectionWeights.end(); ++it)
        {
            weight += it->second;
            if (selectedWeight < weight)
            {
                returnBgTypeId = it->first;
                break;
            }
        }
    }

    return returnBgTypeId;
}

void BattlegroundMgr::AddBattleground(Battleground* bg)
{
    if (bg)
        bgDataStore[bg->GetTypeID()].m_Battlegrounds[bg->GetInstanceID()] = bg;
}

void BattlegroundMgr::RemoveBattleground(BattlegroundTypeId bgTypeId, uint32 instanceId)
{
    bgDataStore[bgTypeId].m_Battlegrounds.erase(instanceId);
}

BGFreeSlotQueueContainer& BattlegroundMgr::GetBGFreeSlotQueueStore(BattlegroundTypeId bgTypeId)
{
    return bgDataStore[bgTypeId].BGFreeSlotQueue;
}

void BattlegroundMgr::AddToBGFreeSlotQueue(BattlegroundTypeId bgTypeId, Battleground* bg)
{
    bgDataStore[bgTypeId].BGFreeSlotQueue.push_front(bg);
}

void BattlegroundMgr::RemoveFromBGFreeSlotQueue(BattlegroundTypeId bgTypeId, uint32 instanceId)
{
    BGFreeSlotQueueContainer& queues = bgDataStore[bgTypeId].BGFreeSlotQueue;
    for (BGFreeSlotQueueContainer::iterator itr = queues.begin(); itr != queues.end(); ++itr)
        if ((*itr)->GetInstanceID() == instanceId)
        {
            queues.erase(itr);
            return;
        }
}
