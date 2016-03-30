/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Westfall
SD%Complete: 90
SDComment: Quest support: 155, 1651
SDCategory: Westfall
EndScriptData */

/* ContentData
npc_daphne_stilwell
npc_defias_traitor
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"

/*######
## npc_daphne_stilwell
######*/

enum eEnums
{
    SAY_DS_START        = -1000293,
    SAY_DS_DOWN_1       = -1000294,
    SAY_DS_DOWN_2       = -1000295,
    SAY_DS_DOWN_3       = -1000296,
    SAY_DS_PROLOGUE     = -1000297,

    SPELL_SHOOT         = 6660,
    QUEST_TOME_VALOR    = 1651,
    NPC_DEFIAS_RAIDER   = 6180,
    EQUIP_ID_RIFLE      = 2511
};

class npc_daphne_stilwell : public CreatureScript
{
public:
    npc_daphne_stilwell() : CreatureScript("npc_daphne_stilwell") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_TOME_VALOR)
        {
            DoScriptText(SAY_DS_START, creature);

            if (npc_escortAI* pEscortAI = CAST_AI(npc_daphne_stilwell::npc_daphne_stilwellAI, creature->AI()))
                pEscortAI->Start(true, true, player->GetGUID());
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_daphne_stilwellAI(creature);
    }

    struct npc_daphne_stilwellAI : public npc_escortAI
    {
        npc_daphne_stilwellAI(Creature* creature) : npc_escortAI(creature) {}

        uint32 uiWPHolder;
        uint32 uiShootTimer;

        void Reset()
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
            {
                switch (uiWPHolder)
                {
                    case 7:
                        DoScriptText(SAY_DS_DOWN_1, me);
                        break;
                    case 8:
                        DoScriptText(SAY_DS_DOWN_2, me);
                        break;
                    case 9:
                        DoScriptText(SAY_DS_DOWN_3, me);
                        break;
                }
            }
            else
                uiWPHolder = 0;

            uiShootTimer = 0;
        }

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            uiWPHolder = waypointId;

            switch (waypointId)
            {
                case 4:
                    SetEquipmentSlots(false, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE, EQUIP_ID_RIFLE);
                    me->SetSheath(SHEATH_STATE_RANGED);
                    me->HandleEmoteCommand(EMOTE_STATE_USE_STANDING_NO_SHEATHE);
                    break;
                case 7:
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836f, 1569.755f, 54.267f, 4.230f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697f, 1569.124f, 54.421f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237f, 1568.307f, 54.620f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    break;
                case 8:
                    me->SetSheath(SHEATH_STATE_RANGED);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836f, 1569.755f, 54.267f, 4.230f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697f, 1569.124f, 54.421f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237f, 1568.307f, 54.620f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.037f, 1570.213f, 54.961f, 4.283f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    break;
                case 9:
                    me->SetSheath(SHEATH_STATE_RANGED);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836f, 1569.755f, 54.267f, 4.230f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697f, 1569.124f, 54.421f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237f, 1568.307f, 54.620f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.037f, 1570.213f, 54.961f, 4.283f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.018f, 1570.738f, 54.828f, 4.220f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    break;
                case 10:
                    SetRun(false);
                    break;
                case 11:
                    DoScriptText(SAY_DS_PROLOGUE, me);
                    break;
                case 13:
                    SetEquipmentSlots(true);
                    me->SetSheath(SHEATH_STATE_UNARMED);
                    me->HandleEmoteCommand(EMOTE_STATE_USE_STANDING_NO_SHEATHE);
                    break;
                case 17:
                    player->GroupEventHappens(QUEST_TOME_VALOR, me);
                    break;
            }
        }

        void AttackStart(Unit* who)
        {
            if (!who)
                return;

            if (me->Attack(who, false))
            {
                me->AddThreat(who, 0.0f);
                me->SetInCombatWith(who);
                who->SetInCombatWith(me);

                me->GetMotionMaster()->MoveChase(who, 30.0f);
            }
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->AI()->AttackStart(me);
        }

        void Update(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            if (!UpdateVictim())
                return;

            if (uiShootTimer <= diff)
            {
                uiShootTimer = 1500;

                if (!me->IsWithinDist(me->GetVictim(), ATTACK_DISTANCE))
                    DoCast(me->GetVictim(), SPELL_SHOOT);
            }
            else
                uiShootTimer -= diff;
        }
    };
};

/*######
## npc_defias_traitor
######*/

#define SAY_START                   -1000101
#define SAY_PROGRESS                -1000102
#define SAY_END                     -1000103
#define SAY_AGGRO_1                 -1000104
#define SAY_AGGRO_2                 -1000105

#define QUEST_DEFIAS_BROTHERHOOD    155

class npc_defias_traitor : public CreatureScript
{
public:
    npc_defias_traitor() : CreatureScript("npc_defias_traitor") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_DEFIAS_BROTHERHOOD)
        {
            if (npc_escortAI* pEscortAI = CAST_AI(npc_defias_traitor::npc_defias_traitorAI, creature->AI()))
                pEscortAI->Start(true, true, player->GetGUID());

            DoScriptText(SAY_START, creature, player);
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_defias_traitorAI(creature);
    }

    struct npc_defias_traitorAI : public npc_escortAI
    {
        npc_defias_traitorAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 35:
                    SetRun(false);
                    break;
                case 36:
                    DoScriptText(SAY_PROGRESS, me, player);
                    break;
                case 44:
                    DoScriptText(SAY_END, me, player);
                    player->GroupEventHappens(QUEST_DEFIAS_BROTHERHOOD, me);
                    break;
            }
        }

        void EnterCombat(Unit* who)
        {
            DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2), me, who);
        }

        void Reset() {}
    };
};

enum homelessStormwindCitizen
{
    QUEST_MURDER_WAS_THE_CASE_THAT_THEY_GAVE_ME  = 26209,
    QUEST_OBJECTIVE_CLUE_OBTAINED                = 265754,
};

class npc_homeless_stormwind_citizen : public CreatureScript
{
public:
    npc_homeless_stormwind_citizen() : CreatureScript("npc_homeless_stormwind_citizen") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_homeless_stormwind_citizenAI(creature);
    }

    struct npc_homeless_stormwind_citizenAI : public ScriptedAI
    {
        npc_homeless_stormwind_citizenAI(Creature* creature) : ScriptedAI(creature) { Reset(); }

        bool feedingStarted;
        uint64 playerGUID;

        void Reset()
        {
            playerGUID = 0;
            feedingStarted = false;
            me->RestoreFaction();
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void MoveInLineOfSight(Unit * who)
        {
            if (who->GetEntry() == 42617 && !feedingStarted && who->GetDistance(me) < 7.0f)
            {
                if (auto invocer = who->ToTempSummon())
                    playerGUID = invocer->GetSummonerGUID();

                feedingStarted = true;
                float x,y,z;
                who->GetClosePoint(x,y,z, 1.0f, 0.0f, me->GetAngle(who));
                me->GetMotionMaster()->MovePoint(1, x,y,z);
            }

            ScriptedAI::MoveInLineOfSight(who);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE && id == 1)
            {
                if (auto player = Player::GetPlayer(*me, playerGUID))
                    player->KilledMonsterCredit(42617);

                me->SetStandState(UNIT_STAND_STATE_SIT);
                me->ForcedDespawn(5000);
            }
        }
    };

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF+1:
        case GOSSIP_ACTION_INFO_DEF+2:
            {
                player->CLOSE_GOSSIP_MENU();
                if (urand(0,1))
                {
                    // Must have first completed to trigger second
                    QuestStatusMap::iterator itr = player->getQuestStatusMap().find(QUEST_MURDER_WAS_THE_CASE_THAT_THEY_GAVE_ME);
                    if (itr == player->getQuestStatusMap().end())
                        return false;

                    for (uint32 i = 0; i < 4; ++i)
                    {
                        if (player->GetQuestObjectiveCounter(QUEST_OBJECTIVE_CLUE_OBTAINED + i) == 0)
                        {
                            creature->AI()->Talk(i);
                            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            creature->ForcedDespawn(10000);
                            player->QuestObjectiveSatisfy(QUEST_OBJECTIVE_CLUE_OBTAINED + i, 1);
                            break;
                        }
                    }
                }
                else
                {
                    creature->setFaction(14);
                    creature->AI()->AttackStart(player);
                }
                break;
            }
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_MURDER_WAS_THE_CASE_THAT_THEY_GAVE_ME) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Did you see who killed the Furlbrows?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_CHAT, "Maybe a couple of copper will loosen your tongue? Now tell me, did you see who killed the Furlbrows?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2, "Are you sure you want to give this hobo money?", 2, false);
        }

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }
};

void AddSC_westfall()
{
    new npc_daphne_stilwell();
    new npc_defias_traitor();
    new npc_homeless_stormwind_citizen();
}
