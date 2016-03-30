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
SDName: The_Barrens
SD%Complete: 90
SDComment: Quest support: 863
SDCategory: Barrens
EndScriptData */

/* ContentData
npc_wizzlecrank_shredder
EndContentData */

#include "CombatAI.h"
#include "MoveSplineInit.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

/*#####
## npc_wizzlecrank_shredder
#####*/

enum Wizzlecrank
{
    SAY_START           = -1000298,
    SAY_STARTUP1        = -1000299,
    SAY_STARTUP2        = -1000300,
    SAY_MERCENARY       = -1000301,
    SAY_PROGRESS_1      = -1000302,
    SAY_PROGRESS_2      = -1000303,
    SAY_PROGRESS_3      = -1000304,
    SAY_END             = -1000305,

    QUEST_ESCAPE        = 863,
    FACTION_RATCHET     = 637,
    NPC_PILOT_WIZZ      = 3451,
    NPC_MERCENARY       = 3282,
};

class npc_wizzlecrank_shredder : public CreatureScript
{
public:
    npc_wizzlecrank_shredder() : CreatureScript("npc_wizzlecrank_shredder") { }

    struct npc_wizzlecrank_shredderAI : public npc_escortAI
    {
        npc_wizzlecrank_shredderAI(Creature* creature) : npc_escortAI(creature)
        {
            IsPostEvent = false;
            PostEventTimer = 1000;
            PostEventCount = 0;
        }

        bool IsPostEvent;
        uint32 PostEventTimer;
        uint32 PostEventCount;

        void Reset()
        {
            if (!HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (me->getStandState() == UNIT_STAND_STATE_DEAD)
                     me->SetStandState(UNIT_STAND_STATE_STAND);

                IsPostEvent = false;
                PostEventTimer = 1000;
                PostEventCount = 0;
            }
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 0:
                    DoScriptText(SAY_STARTUP1, me);
                    break;
                case 9:
                    SetRun(false);
                    break;
                case 17:
                    if (Creature* temp = me->SummonCreature(NPC_MERCENARY, 1128.489f, -3037.611f, 92.701f, 1.472f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
                    {
                        DoScriptText(SAY_MERCENARY, temp);
                        me->SummonCreature(NPC_MERCENARY, 1160.172f, -2980.168f, 97.313f, 3.690f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    }
                    break;
                case 24:
                    IsPostEvent = true;
                    break;
            }
        }

        void WaypointStart(uint32 PointId)
        {
            Player* player = GetPlayerForEscort();

            if (!player)
                return;

            switch (PointId)
            {
                case 9:
                    DoScriptText(SAY_STARTUP2, me, player);
                    break;
                case 18:
                    DoScriptText(SAY_PROGRESS_1, me, player);
                    SetRun();
                    break;
            }
        }

        void JustSummoned(Creature* summoned)
        {
            if (summoned->GetEntry() == NPC_PILOT_WIZZ)
                me->SetStandState(UNIT_STAND_STATE_DEAD);

            if (summoned->GetEntry() == NPC_MERCENARY)
                summoned->AI()->AttackStart(me);
        }

        void UpdateEscortAI(const uint32 Diff)
        {
            if (!UpdateVictim())
            {
                if (IsPostEvent)
                {
                    if (PostEventTimer <= Diff)
                    {
                        switch (PostEventCount)
                        {
                            case 0:
                                DoScriptText(SAY_PROGRESS_2, me);
                                break;
                            case 1:
                                DoScriptText(SAY_PROGRESS_3, me);
                                break;
                            case 2:
                                DoScriptText(SAY_END, me);
                                break;
                            case 3:
                                if (Player* player = GetPlayerForEscort())
                                {
                                    player->GroupEventHappens(QUEST_ESCAPE, me);
                                    me->SummonCreature(NPC_PILOT_WIZZ, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 180000);
                                }
                                break;
                        }

                        ++PostEventCount;
                        PostEventTimer = 5000;
                    }
                    else
                        PostEventTimer -= Diff;
                }

                return;
            }

            DoMeleeAttackIfReady();
        }
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_ESCAPE)
        {
            creature->setFaction(FACTION_RATCHET);
            if (npc_escortAI* pEscortAI = CAST_AI(npc_wizzlecrank_shredder::npc_wizzlecrank_shredderAI, creature->AI()))
                pEscortAI->Start(true, false, player->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wizzlecrank_shredderAI(creature);
    }

};

/*#####
## npc_brutusk_qgw
#####*/

const float BrutuskWPPath[12][3] =
{
    { 1216.619f, -2253.758f, 91.847f },
    { 1235.647f, -2243.865f, 91.763f },
    { 1249.403f, -2229.486f, 92.269f },
    { 1284.208f, -2221.406f, 91.758f },
    { 1312.357f, -2237.044f, 91.735f },
    { 1344.717f, -2256.614f, 90.167f },
    { 1390.280f, -2258.193f, 89.898f },
    { 1436.188f, -2287.756f, 89.986f },
    { 1477.342f, -2347.694f, 91.630f },
    { 1518.445f, -2394.406f, 95.107f },
    { 1558.744f, -2431.286f, 97.992f },
    { 1582.841f, -2484.444f, 97.991f }
};

class npc_brutusk_qgw : public CreatureScript
{
public:
    npc_brutusk_qgw() : CreatureScript("npc_brutusk_qgw") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_brutusk_qgwAI(creature);
    }

    enum eEvents
    {
        EVENT_DONE = 1
    };

    enum eSpells
    {
        SPELL_EJECT_PASSENGERS = 68576,
        SPELL_KILL_CREDIT      = 62890
    };

    struct npc_brutusk_qgwAI : public VehicleAI
    {
        npc_brutusk_qgwAI(Creature* creature) : VehicleAI(creature) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            Movement::MoveSplineInit init(me);
            for (uint8 i = 0; i < 12; ++i)
            {
                G3D::Vector3 path(BrutuskWPPath[i][0], BrutuskWPPath[i][1], BrutuskWPPath[i][2]);
                init.Path().push_back(path);
            }
            init.SetSmooth();
            init.SetVelocity(15.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (apply)
               if (who->GetTypeId() == TYPEID_PLAYER)
                   me->CastSpell(who, SPELL_KILL_CREDIT, false);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_DONE)
            {
                me->CastSpell(me, SPELL_EJECT_PASSENGERS, TRIGGERED_FULL_MASK);
                me->DespawnOrUnsummon(1 * IN_MILLISECONDS);
            }
        }
    };
};

class npc_captured_razormane : public CreatureScript
{
public:
    npc_captured_razormane() : CreatureScript("npc_captured_razormane") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action <= GOSSIP_ACTION_INFO_DEF + 2)
        {
            if (roll_chance_i(15))
            {
                creature->AI()->Talk(3);
                creature->CastSpell(player, 65629, true);
                player->KilledMonsterCredit(34529);
                // TODO: Delayed message
                //creature->AI()->Talk(4);
            }
            else
                creature->AI()->Talk(0);
        }
        else
        {
            creature->AI()->Talk(action - GOSSIP_ACTION_INFO_DEF - 2);
            creature->CastSpell(player, 65628, true);
            player->KilledMonsterCredit(34529);
            //creature->AI()->Talk(4);
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(13963) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "<Punch him in the mouth.>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "<Kick him in his big fat face.>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "<Offer food.>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "<Tickle Time!>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
        }
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }
};


void AddSC_the_barrens()
{
    new npc_wizzlecrank_shredder();
    new npc_brutusk_qgw();
    new npc_captured_razormane();
}
