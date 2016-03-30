/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#include "MoveSplineInit.h"
#include "ScriptedCreature.h"
#include "ScriptedFollowerAI.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "Vehicle.h"

const float FirstPathWP[10][3] =
{
    { 770.34f,  949.726f,  34.8637f },
    { 792.222f, 737.495f,  34.8637f },
    { 920.221f, 698.87f,   3.86366f },
    { 985.457f, 788.34f,  -31.0252f },
    { 937.471f, 883.339f, -39.7197f },
    { 835.682f, 853.524f, -54.1363f },
    { 842.057f, 757.833f, -80.2752f },
    { 948.002f, 751.8f,   -107.581f },
    { 947.352f, 798.976f, -147.995f },
    { 893.021f, 788.55f,  -245.521f }
};

const float SecondPathWP[7][3] =
{
    { 1109.14f, 649.396f,   413.65f },
    { 954.589f, 657.99f,   289.706f },
    { 875.491f, 621.337f,  191.761f },
    { 837.281f, 565.34f,   122.955f },
    { 820.571f, 487.705f,  39.7609f },
    { 903.094f, 501.446f, -36.7289f },
    { 935.057f, 506.95f,  -48.1708f }
};

enum eSpells
{
    SPELL_DEEPHOLM_INTRO_TELEPORT = 84073,
    SPELL_DEEPHOLM_INTRO_TAXI     = 84101,
    SPELL_CAMERA                  = 84364
};

enum eNpc
{
    NPC_AGGRA                     = 45028,
    NPC_AGGRA_SECOND              = 45027
};

enum eEvents
{
    EVENT_ENTER_FLY             = 1,
    EVENT_ENTER_OWNER             = 2,
    EVENT_TELEPORT                = 3,
    EVENT_ENTER_AGGRA_SECOND      = 4,
    EVENT_ENTER_OWNER_SECOND      = 5,
    EVENT_TALK_1                  = 6,
    EVENT_TALK_2                  = 7,
    EVENT_TALK_3                  = 8,
    EVENT_TALK_4                  = 9,
    EVENT_TALK_5                  = 10,
    EVENT_TALK_6                  = 11,
    EVENT_TALK_7                  = 12,
    EVENT_TALK_8                  = 13,
    EVENT_TALK_9                  = 14,
    EVENT_TALK_10                 = 15,
    EVENT_TALK_11                 = 16,
    EVENT_DONE                    = 17
};

enum eTalks
{
    TALK_AGGRA                    = 0,
    TALK_AGGRA_1                  = 1,
    TALK_AGGRA_2                  = 2,
    TALK_AGGRA_3                  = 3,
    TALK_AGGRA_4                  = 4,
    TALK_AGGRA_5                  = 5,
    TALK_AGGRA_6                  = 6,
    TALK_AGGRA_INTRO              = 0,
    TALK_AGGRA_INTRO_1            = 1,
    TALK_AGGRA_INTRO_2            = 2,
    TALK_AGGRA_INTRO_3            = 3
};

enum eActions
{
    ACTION_TALK_INTRO            = 1,
    ACTION_TALK_INTRO_2          = 2
};

class npc_first_wyvern_qdre : public CreatureScript
{
public:
    npc_first_wyvern_qdre() : CreatureScript("npc_first_wyvern_qdre") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_first_wyvern_qdreAI(creature);
    }

    struct npc_first_wyvern_qdreAI : public ScriptedAI
    {
        npc_first_wyvern_qdreAI(Creature * creature)
            : ScriptedAI(creature)
            , aggraGUID(0)
        { }

        EventMap events;
        uint64 aggraGUID;

        void IsSummonedBy(Unit* summoner)
        {
            if (Unit* aggra = summoner->SummonCreature(NPC_AGGRA, *me))
            {
                aggraGUID = aggra->GetGUID();
                aggra->EnterVehicle(me, 0);
            }

            summoner->EnterVehicle(me, 2);
            events.ScheduleEvent(EVENT_ENTER_FLY, 1000);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ENTER_FLY:
                    {
                        Movement::MoveSplineInit init(me);

                        for (uint8 i = 0; i < 10; ++i)
                        {
                            G3D::Vector3 path(FirstPathWP[i][0], FirstPathWP[i][1], FirstPathWP[i][2]);
                            init.Path().push_back(path);
                        }

                        init.SetSmooth();
                        init.SetFly();
                        init.SetUncompressed();
                        init.SetVelocity(35.0f);
                        init.Launch();

                        events.ScheduleEvent(EVENT_TELEPORT, me->GetSplineDuration());
                        me->CastSpell((Unit*)NULL, SPELL_CAMERA, false);

                        if (Vehicle* vehicle = me->GetVehicleKit())
                            if (Unit* aggra = vehicle->GetPassenger(0))
                                if (aggra->IsAIEnabled)
                                    aggra->GetAI()->DoAction(ACTION_TALK_INTRO);
                    }
                    break;
                case EVENT_TELEPORT:
                    if (me->IsSummon())
                    {
                        if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                        {
                            summoner->ExitVehicle();
                            summoner->CastSpell(summoner, SPELL_DEEPHOLM_INTRO_TELEPORT, false);
                        }

                        if (Creature* aggra = ObjectAccessor::GetCreature(*me, aggraGUID))
                        {
                            aggra->ExitVehicle();
                            aggra->DespawnOrUnsummon();
                        }

                        me->DespawnOrUnsummon();
                    }
                    break;
                }
            }
        }
    };
};

class npc_second_wyvern_qdre : public CreatureScript
{
public:
    npc_second_wyvern_qdre() : CreatureScript("npc_second_wyvern_qdre") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_second_wyvern_qdreAI(creature);
    }

    struct npc_second_wyvern_qdreAI : public ScriptedAI
    {
        npc_second_wyvern_qdreAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;
        uint64 aggraGUID;

        void IsSummonedBy(Unit* summoner)
        {
            if (Unit* aggra = me->SummonCreature(NPC_AGGRA_SECOND, *me))
            {
                aggraGUID = aggra->GetGUID();
                aggra->EnterVehicle(me, 0);
            }

            summoner->EnterVehicle(me, 1);
            events.ScheduleEvent(EVENT_ENTER_FLY, 1000);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ENTER_FLY:
                    {
                        Movement::MoveSplineInit init(me);
                        for (uint8 i = 0; i < 7; ++i)
                        {
                            G3D::Vector3 path(SecondPathWP[i][0], SecondPathWP[i][1], SecondPathWP[i][2]);
                            init.Path().push_back(path);
                        }
                        init.SetSmooth();
                        init.SetFly();
                        init.SetUncompressed();
                        init.SetVelocity(15.0f);
                        init.Launch();

                        events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
                        if (Vehicle* vehicle = me->GetVehicleKit())
                            if (Unit* aggra = vehicle->GetPassenger(0))
                                if (aggra->IsAIEnabled)
                                    aggra->GetAI()->DoAction(ACTION_TALK_INTRO_2);
                    }
                    break;
                case EVENT_DONE:
                    if (me->IsSummon())
                    {
                        if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                        {
                            summoner->ExitVehicle();

                            if (Player* player = summoner->ToPlayer())
                                player->SaveToDB();
                        }

                        if (Creature* aggra = ObjectAccessor::GetCreature(*me, aggraGUID))
                        {
                            aggra->ExitVehicle();
                            aggra->DespawnOrUnsummon();
                        }

                        me->DespawnOrUnsummon();
                    }
                    break;
                }
            }
        }
    };
};

class npc_first_aggra_qdre: public CreatureScript
{
public:
    npc_first_aggra_qdre() : CreatureScript("npc_first_aggra_qdre") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_first_aggra_qdreAI(creature);
    }

    struct npc_first_aggra_qdreAI : public ScriptedAI
    {
        npc_first_aggra_qdreAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            ASSERT(me->IsSummon());
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_TALK_INTRO)
                events.ScheduleEvent(EVENT_TALK_1, 0);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_TALK_1:
                    Talk(TALK_AGGRA_INTRO, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_2, 10 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_2:
                    Talk(TALK_AGGRA_INTRO_1, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_3, 10 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_3:
                    Talk(TALK_AGGRA_INTRO_2, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_4, 10 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_4:
                    Talk(TALK_AGGRA_INTRO_3, me->ToTempSummon()->GetSummonerGUID(), true);
                    break;
                }
            }
        }
    };
};

class npc_second_aggra_qdre : public CreatureScript
{
public:
    npc_second_aggra_qdre() : CreatureScript("npc_second_aggra_qdre") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_second_aggra_qdreAI(creature);
    }

    struct npc_second_aggra_qdreAI : public ScriptedAI
    {
        npc_second_aggra_qdreAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            ASSERT(me->IsSummon());
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_TALK_INTRO_2)
                events.ScheduleEvent(EVENT_TALK_5, 0);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_TALK_5:
                    Talk(TALK_AGGRA, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_6, 5 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_6:
                    Talk(TALK_AGGRA_1, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_7, 6 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_7:
                    Talk(TALK_AGGRA_2, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_8, 13 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_8:
                    Talk(TALK_AGGRA_3, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_9, 14 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_9:
                    Talk(TALK_AGGRA_4, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_10, 10 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_10:
                    Talk(TALK_AGGRA_5, me->ToTempSummon()->GetSummonerGUID(), true);
                    events.ScheduleEvent(EVENT_TALK_11, 8 * IN_MILLISECONDS);
                    break;
                case EVENT_TALK_11:
                    Talk(TALK_AGGRA_6, me->ToTempSummon()->GetSummonerGUID(), true);
                    break;
                }
            }
        }
    };
};

class spell_deepholm_intro_teleport_periodic : public SpellScriptLoader
{
public:
    spell_deepholm_intro_teleport_periodic() : SpellScriptLoader("spell_deepholm_intro_teleport_periodic") {}

    class spell_deepholm_intro_teleport_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_deepholm_intro_teleport_periodic_AuraScript)

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetCaster())
            {
                if (!caster->GetVehicle())
                {
                    caster->CastSpell(caster, SPELL_DEEPHOLM_INTRO_TAXI, false);
                    const_cast<AuraEffect*>(aurEff)->SetPeriodic(false);
                }
            }
        }

        void HandleEffectCalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 0.01 * IN_MILLISECONDS;
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_deepholm_intro_teleport_periodic_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_deepholm_intro_teleport_periodic_AuraScript::HandleEffectCalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_deepholm_intro_teleport_periodic_AuraScript();
    }
};

// Energized Geode and Lodestone Elemental for Quest: Elemental Energy (27136)
class npc_elemental_energy_quest : public CreatureScript
{
    public:
        npc_elemental_energy_quest() : CreatureScript("npc_elemental_energy_quest") { }

        struct npc_elemental_energy_questAI : public ScriptedAI
        {
            npc_elemental_energy_questAI(Creature* creature) : ScriptedAI(creature) { }

            void JustDied(Unit * /*who*/)
            {
                if (Creature * totem = GetClosestCreatureWithEntry(me, 45088, 25.0f))
                    totem->CastSpell(totem, 84170, true);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_elemental_energy_questAI(creature);
        }
};

// Boden the Imposing for Quest: Imposing Confrontation (26315)
class npc_boden_the_imposing : public CreatureScript
{
    public:
        npc_boden_the_imposing() : CreatureScript("npc_boden_the_imposing") { }

        struct npc_boden_the_imposingAI : public ScriptedAI
        {
            npc_boden_the_imposingAI(Creature* creature) : ScriptedAI(creature) { Reset(); }

            bool eventStarted;
            uint8 phase;
            int32 phaseTimer;
            uint64 initiator;

            void Reset()
            {
                eventStarted = false;
                phase = 0;
                phaseTimer = 0;
                initiator = 0;
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (spell->Id == 79715)
                {
                    caster->MonsterSay("Boden the Imposing. I come on behalf of the Earthen Ring. We wish your kind no harm. We seek to repair the rift between our worlds. Why do you attack us?", 0, 0);
                    eventStarted = true;
                    phaseTimer = 7000;
                    initiator = caster->GetGUID();
                    me->GetMotionMaster()->MoveIdle();
                    me->SetFacingToObject(caster);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (eventStarted)
                {
                    if ((phaseTimer -= diff) <= 0)
                    switch(phase++)
                    {
                        case 0:
                            me->MonsterYell("Hah! Did you mistake me for Diamant, $r? Or perhaps some other whimpering, compliant, stone trogg who cares?", 0, 0);
                            phaseTimer = 9000;
                            break;
                        case 1:
                            me->MonsterYell("If you seek peace, relinquish the World Pillar and leave Deepholm. This is our realm. Your only welcome here shall be found underneath my stone foot.", 0, 0);
                            phaseTimer = 13000;
                            break;
                        case 2:
                            if (Player * player = Unit::GetPlayer(*me, initiator))
                            {
                                me->CastSpell(player, 79843, true);
                                me->GetMotionMaster()->InitDefault();
                            }
                            Reset();
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_boden_the_imposingAI(creature);
        }
};

// Kor the Immovable for Quest: Pebble (28869)
/*###
##  Kor the Immovable - Pebble spawns when player has quest Pebble incomplete or rewarded
##  Pebble will be used for a long questchain
####*/
enum eKorTheImmovableEntries
{
    QUEST_PEBBLE                    =   28869,
    SPELL_SUMMON_PEBBLE             =   80665,
    NPC_PEBBLE                      =   43116,
};

class npc_kor_the_immovable : public CreatureScript
{
    public:
        npc_kor_the_immovable() : CreatureScript("npc_kor_the_immovable") { }

        struct npc_kor_the_immovableAI : public ScriptedAI
        {
            npc_kor_the_immovableAI(Creature* creature) : ScriptedAI(creature) { }

            void MoveInLineOfSight(Unit* who)
            {
                Player* player = who->ToPlayer();

                if (player && (player->GetQuestStatus(QUEST_PEBBLE)==QUEST_STATUS_COMPLETE || player->IsQuestRewarded(QUEST_PEBBLE))
                                && !HasPeebleCompanion(player))
                {
                    player->CastSpell(player, SPELL_SUMMON_PEBBLE);
                }
            }

            bool HasPeebleCompanion(Player* player)
            {
                std::list<Creature*> lMinions;
                player->GetAllMinionsByEntry(lMinions, NPC_PEBBLE);

                if (lMinions.size() == 1 && (*lMinions.begin())->IsAlive())
                {
                    return true;
                }

                return false;
            }

        };

        CreatureAI* GetAI(Creature *creature) const
        {
            return new npc_kor_the_immovableAI(creature);
        }
};

// Pebble for Quest: Clingy (26440)
/*
TODO in npc_pebble
make it attack enemy npcs
When Pebble appears he emotes "You hear the sound of shuffling stones behind you."
When sees crystals emotes "Pebble sees the crystals ahead and excitedly rushes towards them!"
After quest credit is obtained emotes "Pebble bobbles around appreciatively."
when Pebble is hit by SPELL_PETRIFIED_DELICACIES_COMPLETE_TRIGGER he runs around and emotes "Pebble races in circles, clearly happy." then despawns and is not summoned anymore
*/
enum ePebbleEntries
{
    QUEST_CLINGY                                =   26440,
    SPELL_BOULDER                               =   84490,
    SPELL_SHOCKWAVE                             =   84491,
    SPELL_CHANNELING                            =   74379,
    SPELL_PETRIFIED_DELICACIES_COMPLETE_TRIGGER =   84153,
    NPC_CREDIT_BUNNY                            =   43172,
    NPC_FLAYER                                  =   42606,
};

class npc_pebble : public CreatureScript
{
    public:
        npc_pebble() : CreatureScript("npc_pebble") { }

        struct npc_pebbleAI : public ScriptedAI
        {
            npc_pebbleAI(Creature* creature) : ScriptedAI(creature) { }

            Player* Owner;

            void Reset()
            {
                Owner = me->GetCharmerOrOwnerPlayerOrPlayerItself();
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (Owner && Owner->GetQuestStatus(QUEST_CLINGY) == QUEST_STATUS_INCOMPLETE)
                {
                    if (me->FindNearestCreature(NPC_CREDIT_BUNNY, 35.0f))
                    {
                        Owner->KilledMonsterCredit(NPC_CREDIT_BUNNY, 0);
                    }
                }
            }

        };

        CreatureAI* GetAI(Creature *creature) const
        {
            return new npc_pebbleAI(creature);
        }
};

// Flint Oremantle for Quest: Take Him to the Earthcaller (26413)
enum eFlint
{
    QUEST_TAKE_HIM_TO_THE_EARTHCALLER = 26413,
    NPC_YEVAA = 42573,
};

/* ToDo
- Maybe there is a dialog between Flint and Yeeva
*/

class npc_flint_oremantle : public CreatureScript
{
public:
    npc_flint_oremantle() : CreatureScript("npc_flint_oremantle") { }

    bool OnGossipHello(Player* pPlayer, Creature* creature)
    {
        if (creature->IsInCombat() || pPlayer->GetQuestStatus(QUEST_TAKE_HIM_TO_THE_EARTHCALLER) != QUEST_STATUS_INCOMPLETE)
        return false;

        char const* _message = "Follow me to the Earthcaller!";
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, _message, GOSSIP_SENDER_MAIN , GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(68, creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* creature, uint32 /*uiSender*/, uint32 /*uiAction*/)
    {
        pPlayer->PlayerTalkClass->ClearMenus();

        pPlayer->CLOSE_GOSSIP_MENU();

         if (npc_flint_oremantleAI* npc_flint_oremantleAI = CAST_AI(npc_flint_oremantle::npc_flint_oremantleAI, creature->AI()))
         {
                creature->SetStandState(UNIT_STAND_STATE_STAND);
                npc_flint_oremantleAI->StartFollow(pPlayer, 35, 0);

                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
         }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_flint_oremantleAI(creature);
    }

    struct npc_flint_oremantleAI : public FollowerAI
    {
        npc_flint_oremantleAI(Creature* creature) : FollowerAI(creature), HomePosition(creature->GetHomePosition()) { }

        Position HomePosition;

        void Reset()
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void MoveInLineOfSight(Unit* who)
        {
            FollowerAI::MoveInLineOfSight(who);

            if (!me->GetVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && who->GetEntry() == NPC_YEVAA)
            {
                if (me->IsWithinDistInMap(who, INTERACTION_DISTANCE))
                {
                    if (Player* player = GetLeaderForFollower())
                    {
                        if (player->GetQuestStatus(QUEST_TAKE_HIM_TO_THE_EARTHCALLER) == QUEST_STATUS_INCOMPLETE)
                            player->KilledMonsterCredit(44207, 0);

                        me->DespawnOrUnsummon(30000);
                    }

                    SetFollowComplete(true);
                }
            }
        }
    };
};

// Ricket Ticker for Quest: Underground Ecomomy (27048)
enum eRicketTicker
{
    SPELL_CRYSTAL_FORMATION_EXPLOSION = 92789,

    NPC_DEEP_CELESTITE_BUNNY = 49865,
    NPC_DEEP_AMETHYST_BUNNY = 49866,
    NPC_DEEP_GRANAT_BUNNY = 49867,
    NPC_DEEP_ALABASTER_BUNNY = 49824,

    ITEM_DEEP_CELESTITE_CRYSTAL = 65507,
    ITEM_DEEP_AMETHYST_CRYSTAL = 65508,
    ITEM_DEEP_GRANAT_CRYSTAL = 65510,
    ITEM_DEEP_ALABASTER_CRYSTAL = 65504,
};

class npc_ricket_ticker : public CreatureScript
{
public:
    npc_ricket_ticker() : CreatureScript("npc_ricket_ticker") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ricket_tickerAI (creature);
    }

    struct npc_ricket_tickerAI : public ScriptedAI
    {
        npc_ricket_tickerAI(Creature* creature) : ScriptedAI(creature), uiExplode(0) { }

        uint32 uiExplode;
        Player* player;

        void Reset()
        {
            uiExplode = 0;
        }

        void IsSummonedBy(Unit* summoner)
        {
            uiExplode = 3500;
            player = summoner->ToPlayer();
        }

        void UpdateAI(uint32 const diff)
        {
            if (!uiExplode)
                return;

            if (uiExplode <= diff)
            {
                DoCastAOE(SPELL_CRYSTAL_FORMATION_EXPLOSION, true);

                // Checks weather a Deep Crystal is in Range
                if (me->FindNearestCreature(NPC_DEEP_CELESTITE_BUNNY, 7.0f, true))
                        player->AddItem(ITEM_DEEP_CELESTITE_CRYSTAL, 1);

                if (me->FindNearestCreature(NPC_DEEP_AMETHYST_BUNNY, 7.0f, true))
                        player->AddItem(ITEM_DEEP_AMETHYST_CRYSTAL, 1);

                if (me->FindNearestCreature(NPC_DEEP_GRANAT_BUNNY, 7.0f, true))
                        player->AddItem(ITEM_DEEP_GRANAT_CRYSTAL, 1);

                if (me->FindNearestCreature(NPC_DEEP_ALABASTER_BUNNY, 7.0f, true))
                        player->AddItem(ITEM_DEEP_ALABASTER_CRYSTAL, 1);

                me->DespawnOrUnsummon();
            } else uiExplode -= diff;
        }
    };
};

// Stonefather's Banner for Quest: Stonefathers Boon (26499)
enum eBanner
{
    NPC_STONEHEART_DEFENDER = 43138,
    SPELL_BUFF_OF_THE_STONEFATHER = 80668,
    SPELL_BANNER_HITS_GROUND = 80669,
};

class npc_stonefathers_banner : public CreatureScript
{
public:
    npc_stonefathers_banner() : CreatureScript("npc_stonefathers_banner") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stonefathers_bannerAI (creature);
    }

    struct npc_stonefathers_bannerAI : public ScriptedAI
    {
        npc_stonefathers_bannerAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void IsSummonedBy(Unit* summoner)
        {
            DoCastAOE(SPELL_BANNER_HITS_GROUND, true);

            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, NPC_STONEHEART_DEFENDER, 10.0f);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                if (!(*iter)->HasAura(SPELL_BUFF_OF_THE_STONEFATHER))
                {
                    (*iter)->CastSpell((*iter), SPELL_BUFF_OF_THE_STONEFATHER, true);
                    summoner->ToPlayer()->KilledMonsterCredit(NPC_STONEHEART_DEFENDER, 0);
                }
        }
    };
};

// Opalescent Guardian and Terrath The Steady for Quest: Don't. Stop. Moving. (26656)
enum eTerrathTheSteadyEntries
{
    QUEST_DONT_STOP_MOVING      =   26656,
    NPC_OPALESCENT_GUARDIAN     =   43591,
    SPEAK_TO_TERRATH_CREDIT     =   46139,
    NPC_OPAL_STONETHROWER       =   43586,
    NPC_OPALESCENT_GUARD_CREDIT =   43597,

};

class npc_opalescent_guardian : public CreatureScript
{
    public:
        npc_opalescent_guardian() : CreatureScript("npc_opalescent_guardian") { }

        struct npc_opalescent_guardianAI : public FollowerAI
        {
            npc_opalescent_guardianAI(Creature* creature) : FollowerAI(creature) { }

            void Reset() { }

            void MoveInLineOfSight(Unit* who)
            {
                FollowerAI::MoveInLineOfSight(who);

                if (!me->GetVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && who->GetEntry() == NPC_OPAL_STONETHROWER)
                {
                    if (me->IsWithinDistInMap(who, 15.0f))
                    {
                        DoComplete();
                    }
                }
            }

            void DoComplete()
            {
                if (Player* player = GetLeaderForFollower())
                {
                    if (player->GetQuestStatus(QUEST_DONT_STOP_MOVING) == QUEST_STATUS_INCOMPLETE)
                        player->KilledMonsterCredit(NPC_OPALESCENT_GUARD_CREDIT, 0);
                }

                SetFollowComplete();
            }

            void UpdateFollowerAI(uint32 const /*uiDiff*/)
            {
                if (!UpdateVictim())
                    return;

                if (Player* player = GetLeaderForFollower())
                {
                    if (me->GetDistance(player) > 75.0f)
                    {
                        me->RemoveFromWorld();
                        return;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_opalescent_guardianAI(creature);
        }
};

#define GOSSIP_TERRATH_THE_STEADY   "I'm ready to escort a group of elementals across the open."
#define TERRATH_TEXT_EMOTE_01       "Speak with Terrath the Steady when you are ready to begin."

class npc_terrath_the_steady: public CreatureScript
{
    public:
        npc_terrath_the_steady() : CreatureScript("npc_terrath_the_steady") { }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
        {
            if (quest->GetQuestId() == QUEST_DONT_STOP_MOVING)
            {
                creature->MonsterTextEmote(TERRATH_TEXT_EMOTE_01, player->GetGUID(), true);
            }
            return true;
        }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (creature->IsQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            if (player->GetQuestStatus(QUEST_DONT_STOP_MOVING)==QUEST_STATUS_INCOMPLETE)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TERRATH_THE_STEADY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            }

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 /*action*/)
        {
            player->KilledMonsterCredit(SPEAK_TO_TERRATH_CREDIT, 0);
            player->PlayerTalkClass->ClearMenus();

            for (uint32 i = 0 ; i < 5 ; i++)
            {
                Creature* pGuardian = creature->SummonCreature(NPC_OPALESCENT_GUARDIAN, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());
                if (pGuardian)
                {

                    if (npc_opalescent_guardian::npc_opalescent_guardianAI* psGuardian = CAST_AI(npc_opalescent_guardian::npc_opalescent_guardianAI, pGuardian->AI()))
                        psGuardian->StartFollow(player);
                }
            }

            player->CLOSE_GOSSIP_MENU();

            return true;
        }

        struct npc_terrath_the_steadyAI : public ScriptedAI
        {
            npc_terrath_the_steadyAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP+UNIT_NPC_FLAG_QUESTGIVER);
            }

        };

        CreatureAI* GetAI(Creature *creature) const
        {
            return new npc_terrath_the_steadyAI(creature);
        }
};

// War Guardian and Giant Mushroom for Quests: Sprout No More (26791) and Fungal Monstrosities (26792)
enum eWarGuardianEntries
{
    QUEST_FUNGAL_MONSTROSITIES      =   26792,
    QUEST_SPROUT_NO_MORE            =   26791,
    SPELL_ARTIFACT_EXPLOSION        =   65429,
    SPELL_CHOCKING_WOUND            =   35247,
    SPELL_EARTH_SHOCK               =   25025,
    SPELL_GROUND_SMASH              =   82120,
    SPELL_THUNDERCLAP               =   8078,
    NPC_WAR_GUARDIAN                =   44118,
    NPC_GIANT_MUSHROOM              =   44049,
    NPC_STONE_TROGG_EARTHRAGER      =   43616
};

class npc_war_guardian : public CreatureScript
{
    public:
        npc_war_guardian() : CreatureScript("npc_war_guardian") { }

        struct npc_war_guardianAI : public ScriptedAI
        {
            npc_war_guardianAI(Creature* creature) : ScriptedAI(creature) { }

            uint32  uiChokingWoundTimer;
            uint32  uiEarthShockTimer;
            uint32  uiGroundSmashTimer;
            uint32  uiThunderClapTimer;

            void Reset()
            {
                uiChokingWoundTimer =   urand(20000, 25000);
                uiEarthShockTimer   =   urand(5000, 7000);
                uiGroundSmashTimer  =   urand(3000, 5000);
                uiThunderClapTimer  =   urand(12000, 15000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                //choking wound
                if (uiChokingWoundTimer <= diff)
                {
                    DoCastVictim(SPELL_CHOCKING_WOUND);
                    uiChokingWoundTimer = urand(20000, 25000);
                }
                else    uiChokingWoundTimer -= diff;

                //earth shok
                if (uiEarthShockTimer <= diff)
                {
                    DoCastVictim(SPELL_EARTH_SHOCK);
                    uiEarthShockTimer = urand(5000, 7000);
                }
                else    uiEarthShockTimer -= diff;

                //ground smash
                if (uiGroundSmashTimer <= diff)
                {
                    DoCastVictim(SPELL_GROUND_SMASH);
                    uiGroundSmashTimer = urand(3000, 5000);
                }
                else    uiGroundSmashTimer -= diff;

                //thunderclap
                if (uiThunderClapTimer <= diff)
                {
                    DoCastVictim(SPELL_THUNDERCLAP);
                    uiThunderClapTimer = urand(12000, 15000);
                }
                else    uiThunderClapTimer -= diff;

                DoMeleeAttackIfReady();
            }

        };

        CreatureAI* GetAI(Creature *creature) const
        {
            return new npc_war_guardianAI(creature);
        }
};


class npc_giant_mushroom : public CreatureScript
{
    public:
        npc_giant_mushroom() : CreatureScript("npc_giant_mushroom") { }

        struct npc_giant_mushroomAI : public ScriptedAI
        {
            npc_giant_mushroomAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
            }

            void MoveInLineOfSight(Unit* who)
            {
                Player* player = who->ToPlayer();

                if (player && (me->GetDistance(player) < 10.0f) &&
                    (player->GetQuestStatus(QUEST_FUNGAL_MONSTROSITIES)==QUEST_STATUS_INCOMPLETE))
                {
                    Unit* unit = player->GetSelectedUnit();
                    if (unit && unit->GetGUID() == me->GetGUID())
                    {
                        me->CastSpell(me, SPELL_ARTIFACT_EXPLOSION);
                        player->KilledMonsterCredit(NPC_GIANT_MUSHROOM, 0);
                        if (Unit* unit = me->SummonCreature(NPC_STONE_TROGG_EARTHRAGER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f))
                            unit->Attack(player, true);
                        me->DespawnOrUnsummon(1000);

                    }
                }
            }
        };

        CreatureAI* GetAI(Creature *creature) const
        {
            return new npc_giant_mushroomAI(creature);
        }
};

void AddSC_deepholm()
{
    new npc_first_wyvern_qdre();
    new npc_second_wyvern_qdre();
    new npc_first_aggra_qdre();
    new npc_second_aggra_qdre();
    new spell_deepholm_intro_teleport_periodic();
    new npc_elemental_energy_quest();
    new npc_boden_the_imposing();
    new npc_kor_the_immovable();
    new npc_pebble();
    new npc_flint_oremantle();
    new npc_ricket_ticker();
    new npc_stonefathers_banner();
    new npc_opalescent_guardian();
    new npc_terrath_the_steady();
    new npc_war_guardian();
    new npc_giant_mushroom();
}
