#include "Chat.h"
#include "BattlegroundMgr.h"
#include "BattlegroundQueue.h"
#include "DisableMgr.h"
#include "Group.h"
#include "SocialMgr.h"
#include "Battleground.h"
#include "WorldPacket.h"
#include "Player.h"
#include "WorldSession.h"

class challenge_commandscript : public CommandScript
{
public:
    challenge_commandscript() : CommandScript("challenge_commandscript") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand commandTable[] =
        {
            { "challengeplayers", SEC_PLAYER, false, &HandleChallengePlayer, "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };
        return commandTable;
    }

    static bool HandleChallengePlayer(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* playerOneName = strtok((char*)args, " ");
        Player* playerOne;
        char* playerTwoName = strtok(NULL, " ");
        Player* playerTwo;

        char* arenaName = strtok(NULL, " ");
        BattlegroundTypeId bgType = BATTLEGROUND_AA;
        if (arenaName)
        {
            if (strstr(arenaName, "nagrand") != NULL || strstr(arenaName, "na") != NULL)
                bgType = BATTLEGROUND_NA;
            else if (strstr(arenaName, "blade") != NULL || strstr(arenaName, "blades") != NULL || strstr(arenaName, "edge") != NULL)
                bgType = BATTLEGROUND_BE;
            else if (strstr(arenaName, "ruins") != NULL || strstr(arenaName, "lordaeron") != NULL)
                bgType = BATTLEGROUND_RL;
            // these are not implemented in this core.. 21/3/2015
            //else if (strstr(arenaName, "dalaran") != NULL || strstr(arenaName, "dala") != NULL)
            //    bgType = BATTLEGROUND_DS;
            //else if (strstr(arenaName, "ring") != NULL || strstr(arenaName, "rov") != NULL)
            //    bgType = BATTLEGROUND_RV;
            else
            {
                handler->SendSysMessage("Could not determine the arena map to fight on. Challenge not created.");
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        if (!handler->extractPlayerTarget(playerOneName, &playerOne) || !handler->extractPlayerTarget(playerTwoName, &playerTwo))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (playerOne == playerTwo)
        {
            handler->SendSysMessage("You can't challenge players with themselfs.");
            return true;
        }
        if (playerOne->IsInSameGroupWith(playerTwo))
        {
            handler->SendSysMessage("Both targets are in the same group.");
            return true;
        }
        if (playerOne->InBattlegroundQueue() || playerTwo->InBattlegroundQueue())
        {
            handler->SendSysMessage("One of the selected groups is in a Bg / Arena Queue.");
            return true;
        }
        if (playerOne->InBattleground() || playerTwo->InBattleground())
        {
            handler->SendSysMessage("One of the selected groups is currently in Arena / Battleground.");
            return true;
        }

        if (playerOne->GetGroup() && playerTwo->GetGroup())
        {
            if (playerOne->GetGroup()->GetMembersCount() > 5 || playerTwo->GetGroup()->GetMembersCount() > 5)
            {
                handler->SendSysMessage("The maximum group size for Challenges is 5 players.");
                return true;
            }

            if (playerOne->GetGroup()->GetMembersCount() != playerTwo->GetGroup()->GetMembersCount())
            {
                handler->SendSysMessage("The selected groups are not thesame size.");
                return true;
            }
        }
        if (playerOne->GetGroup() && !playerTwo->GetGroup())
        {
            handler->SendSysMessage("One of the groups is not a group.");
            return true;
        }
        if (!playerOne->GetGroup() && playerTwo->GetGroup())
        {
            handler->SendSysMessage("One of the groups is not a group.");
            return true;
        }

        ChatHandler(playerOne->GetSession()).PSendSysMessage("Player %s is challenging you / your group to a wargame", playerTwo->GetName().c_str());
        ChatHandler(playerTwo->GetSession()).PSendSysMessage("Player %s is challenging you / your group to a wargame", playerOne->GetName().c_str());

        createChallenge(playerOne, playerTwo, bgType);
        return true;
    }

    static void createChallenge(Player* playerOne, Player* playerTwo, BattlegroundTypeId bgTypeId = BATTLEGROUND_AA)
    {
        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
        {
            TC_LOG_ERROR("bg.battleground", "Battleground: template bg (all arenas) not found");
            return;
        }

        uint8 arenaType = ARENA_TYPE_2v2;
        uint8 playercount = 1;
        if (playerOne->GetGroup() && playerTwo->GetGroup())
        {
            switch (playerOne->GetGroup()->GetMembersCount())
            {
            case 3:
                arenaType = ARENA_TYPE_3v3;
                break;
            case 4:
            case 5:
                arenaType = ARENA_TYPE_5v5;
                break;
            }
            playercount = playerOne->GetGroup()->GetMembersCount();

            for (GroupReference* itr = playerOne->GetGroup()->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *member = itr->GetSource();
                if (!member)
                    continue;

                uint32 queueSlot = member->AddBattlegroundQueueId(BattlegroundMgr::BGQueueTypeId(bg->GetTypeID(), arenaType));
                WorldPacket data;
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, member, queueSlot, STATUS_WAIT_QUEUE, 0, 0, arenaType);
                member->GetSession()->SendPacket(&data);
            }
            for (GroupReference* itr = playerTwo->GetGroup()->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *member = itr->GetSource();
                if (!member)
                    continue;

                uint32 queueSlot = member->AddBattlegroundQueueId(BattlegroundMgr::BGQueueTypeId(bg->GetTypeID(), arenaType));
                WorldPacket data;
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, member, queueSlot, STATUS_WAIT_QUEUE, 0, 0, arenaType);
                member->GetSession()->SendPacket(&data);
            }
        }
        else
        {
            // Add challenger
            uint32 queueSlot = playerOne->AddBattlegroundQueueId(BattlegroundMgr::BGQueueTypeId(bg->GetTypeID(), arenaType));
            WorldPacket data;
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, playerOne, queueSlot, STATUS_WAIT_QUEUE, 0, 0, arenaType);
            playerOne->GetSession()->SendPacket(&data);
            // Add challenged person
            queueSlot = playerTwo->AddBattlegroundQueueId(BattlegroundMgr::BGQueueTypeId(bg->GetTypeID(), arenaType));
            WorldPacket data2;
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data2, bg, playerTwo, queueSlot, STATUS_WAIT_QUEUE, 0, 0, arenaType);
            playerTwo->GetSession()->SendPacket(&data2);
        }

        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), playerOne->getLevel());
        if (!bracketEntry)
            return;

        bg = sBattlegroundMgr->CreateNewBattleground(bgTypeId, bracketEntry, arenaType, false);

        BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(BattlegroundMgr::BGQueueTypeId(bg->GetTypeID(), arenaType));
        if (playerOne->GetGroup())
        {
            for (GroupReference* itr = playerOne->GetGroup()->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *member = itr->GetSource();
                if (!member)
                    continue;

                AddGroupQueueInfo(bg, HORDE, member, &bgQueue, arenaType, bracketEntry->GetBracketId());
            }
            for (GroupReference* itr = playerTwo->GetGroup()->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *member = itr->GetSource();
                if (!member)
                    continue;

                AddGroupQueueInfo(bg, ALLIANCE, member, &bgQueue, arenaType, bracketEntry->GetBracketId());
            }
        }
        else
        {
            AddGroupQueueInfo(bg, HORDE, playerOne, &bgQueue, arenaType, bracketEntry->GetBracketId());
            AddGroupQueueInfo(bg, ALLIANCE, playerTwo, &bgQueue, arenaType, bracketEntry->GetBracketId());
        }
        bg->StartBattleground();
    }

    static void AddGroupQueueInfo(Battleground* bg, uint32 Team, Player* plr, BattlegroundQueue* bgQueue, uint8 arenaType, BattlegroundBracketId bracketId)
    {
        GroupQueueInfo* ginfo = new GroupQueueInfo;
        ginfo->BgTypeId = bg->GetTypeID();
        ginfo->ArenaType = arenaType;
        ginfo->IsRated = false;
        ginfo->IsInvitedToBGInstanceGUID = 0;
        ginfo->JoinTime = getMSTime();
        ginfo->RemoveInviteTime = getMSTime() + INVITE_ACCEPT_WAIT_TIME_ARENA;
        ginfo->Team = Team;
        ginfo->ArenaTeamRating = 0;
        ginfo->ArenaMatchmakerRating = 0;
        ginfo->OpponentsTeamRating = 0;
        ginfo->OpponentsMatchmakerRating = 0;
        ginfo->Players.clear();
        ginfo->IsTeamChanged = true;

        PlayerQueueInfo& pl_info = bgQueue->m_QueuedPlayers[plr->GetGUID()];
        pl_info.LastOnlineTime = getMSTime();
        pl_info.GroupInfo = ginfo;
        ginfo->Players[plr->GetGUID()] = &pl_info;
        bgQueue->m_QueuedGroups[bracketId][Team == HORDE ? BG_QUEUE_ALLIANCE : BG_QUEUE_HORDE].push_back(ginfo);

        plr->AddBattlegroundQueueJoinTime(ginfo->BgTypeId, ginfo->JoinTime);

        bgQueue->InviteGroupToBG(ginfo, bg, ginfo->Team);
    }

};

void AddSC_challenge_commandscript()
{
    new challenge_commandscript();
}