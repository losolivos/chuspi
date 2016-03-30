/*
 * Copyright (C) 2008-2014 MoltenCore <http://www.molten-wow.com/>
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
#include "siege_of_niuzao_temple.h"
#include "MoveSplineInit.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Map.h"
#include "Vehicle.h"
#include "CreatureTextMgr.h"

/*

Patch 5.4.7 - Siege of Niuzao Temple: A Challenge Gong has been added, allowing players the option to trigger waves of attackers more quickly during the Commander Vo'jak encounter.

*/

enum
{
    NPC_LI_CHU              = 61812, // Left side
    NPC_LO_CHU              = 62794, // right side
    NPC_YANG_IRONCLAW       = 61620,
    NPC_SIKTHIK_WARDEN      = 62795,
    NPC_MANTID_TAR_KEG      = 61817,
    NPC_AMBERWING           = 61699,
    NPC_BREACHING_CHARGE    = 65168,

    // Adds
    ACTION_ENGAGE_COMBAT    = 1, // DNC
    ACTION_RESET_COMBAT,
    ACTION_CALL_WAVE        = 10,
    WAVE_COUNT              = 4,
    LEFT_SIDE_MASK          = (1 << 10),

    POINT_INTRO             = 10,
    POINT_BOMB,

};

enum WaveCreatures
{
    NPC_SIKTHIK_SWARMER         = 63106,
    NPC_SIKTHIK_DEMOLISHER      = 61670,
    NPC_SIKTHIK_WARRIOR         = 61701,
    NPC_BARREL_TARGET           = 62684,
    NPC_INVISIBLE_STALKER       = 57478
};

enum
{
    SPELL_CARRYING_CAUSTIC_TAR      = 123032,
    SPELL_KEG_ACTIVE                = 123215, // keg sparkle visual
    SPELL_KEG_INACTIVE              = 123218,
    SPELL_MAKESHIFT_CHARGE          = 127408,
    SPELL_BREACH_DOOR_TARGET        = 127418,
    SPELL_BREACH_DOOR_EFFECT        = 127417
};

enum Yells
{
    SAY_INTRO_1                 = 0,
    SAY_INTRO_2,
    SAY_INTRO_3,
    SAY_INTRO_4,
    SAY_AGGRO_1,
    SAY_AGGRO_2,
    SAY_AGGRO_3,
    SAY_AGGRO_4,
    SAY_AGGRO_5,
    EMOTE_WAVE_ONE,
    SAY_WAVE_ONE,
    EMOTE_WAVE_TWO,
    SAY_WAVE_TWO,
    EMOTE_WAVE_THREE,
    SAY_WAVE_THREE,
    EMOTE_WAVE_FOUR,
    EMOTE_BOMBARD,
    SAY_AMBERWING,
    SAY_OUTRO_1,
    SAY_OUTRO_2,
    SAY_GATE_1,
    SAY_GATE_2,
    SAY_GATE_3,
    SAY_GATE_4,

    SAY_VOJAK_AGGRO             = 0,
    SAY_VOJAK_WAVE_ONE,
    SAY_VOJAK_WAVE_TWO,
    SAY_VOJAK_WAVE_THREE,
    SAY_VOJAK_WAVE_FOUR,
    SAY_VOJAK_AIR_SUPPORT,
    SAY_VOJAK_ENGAGE_1,
    SAY_VOJAK_ENGAGE_2,
    SAY_VOJAK_AMBERWING,
    SAY_VOJAK_DEATH,
    SAY_VOJAK_SLAY,
};

struct SpawnData
{
    uint32 entry;
    Position spawnPos;
};

std::vector<SpawnData> LeftSpawns[WAVE_COUNT] =
{
    // row 1
    {
        { NPC_SIKTHIK_SWARMER, { 1492.31f, 5381.32f, 139.523f, 5.21757f } },
        { NPC_SIKTHIK_SWARMER, { 1497.94f, 5382.99f, 139.517f, 5.08107f } },
        { NPC_SIKTHIK_SWARMER, { 1503.62f, 5384.27f, 138.987f, 4.98993f } },
    },
    // row 2
    {
        { NPC_SIKTHIK_DEMOLISHER, { 1485.81f, 5387.15f, 139.537f, 5.13307f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1490.2f, 5388.31f, 139.501f, 5.07617f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1494.62f, 5389.79f, 139.373f, 5.21757f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1499.56f, 5390.67f, 139.093f, 5.08107f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1504.1f, 5391.9f, 138.702f, 4.98993f } }
    },
    // row 3
    {
        { NPC_SIKTHIK_SWARMER, { 1490.23f, 5395.73f, 140.125f, 5.1752f } },
        { NPC_SIKTHIK_SWARMER, { 1496.01f, 5397.63f, 140.189f, 5.0387f } },
        { NPC_SIKTHIK_SWARMER, { 1501.61f, 5398.63f, 139.612f, 4.94756f } },
    },
    // row 4
    {
        { NPC_SIKTHIK_DEMOLISHER, { 1486.66f, 5399.95f, 142.134f, 5.1069f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1490.58f, 5401.87f, 142.353f, 5.0562f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1495.18f, 5403.71f, 142.512f, 5.1752f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1499.73f, 5404.56f, 141.805f, 5.0387f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1504.15f, 5404.81f, 140.792f, 4.94756f } }
    }
};

std::vector<SpawnData> RightSpawns[WAVE_COUNT] =
{
    // row 1
    {
        { NPC_SIKTHIK_SWARMER, { 1518.57f, 5386.58f, 138.899f, 4.9688f } },
        { NPC_SIKTHIK_SWARMER, { 1524.63f, 5385.99f, 139.761f, 4.63027f } },
        { NPC_SIKTHIK_SWARMER, { 1529.82f, 5384.92f, 139.999f, 4.53913f } },
    },
    // row 2
    {
        { NPC_SIKTHIK_DEMOLISHER, {1517.42f, 5394.66f, 139.189f, 4.50366f} },
        { NPC_SIKTHIK_DEMOLISHER, {1522.11f, 5394.43f, 139.199f, 4.59403f} },
        { NPC_SIKTHIK_DEMOLISHER, {1526.45f, 5393.73f, 139.188f, 4.70598f} },
        { NPC_SIKTHIK_DEMOLISHER, {1531.14f, 5393.0f, 139.114f, 4.56948f} },
        { NPC_SIKTHIK_DEMOLISHER, {1534.96f, 5390.47f, 139.256f, 4.47834f} },
    },
    // row 3
    {
        { NPC_SIKTHIK_SWARMER, { 1519.44f, 5400.56f, 139.6f, 4.66401f } },
        { NPC_SIKTHIK_SWARMER, { 1534.09f, 5398.92f, 139.13f, 4.43637f } },
        { NPC_SIKTHIK_SWARMER, { 1526.4f, 5400.53f, 139.564f, 4.52751f } },
    },
    // row 4
    {
        { NPC_SIKTHIK_DEMOLISHER, { 1517.7f, 5406.8f, 140.215f, 4.67264f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1522.76f, 5407.42f, 140.269f, 4.71994f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1527.97f, 5406.99f, 140.135f, 4.66401f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1533.31f, 5406.38f, 139.854f, 4.52751f } },
        { NPC_SIKTHIK_DEMOLISHER, { 1538.09f, 5404.21f, 139.43f, 4.43637f } },
    }
};

static const Position bombsPos[] = 
{
    { 1621.851f, 5411.984f, 138.73839f, 6.125f },
    { 1621.851f, 5408.082f, 138.74924f, 6.126f },
    { 1621.229f, 5404.804f, 138.73306f, 6.176f },
    { 1620.734f, 5400.718f, 138.73073f, 6.141f },
    { 1620.033f, 5397.146f, 139.07486f, 6.090f }
};

/*
    Koz:
    This is a script for Yang Ironclaw
    It functions as controller NPC:
    - Instance State
    - NPC spawns / resets
    - Wave Generation / Timing
    - Exploit checks
    - Boss Summoning
*/

class npc_yang_ironclaw : public CreatureScript
{

    class CreatureTalkEvent final : public BasicEvent
    {
    public:
        explicit CreatureTalkEvent(Creature * speaker, uint32 textId) : 
            _speaker(speaker), _textId(textId)
        {
        }

        bool Execute(uint64, uint32) final
        {
            sCreatureTextMgr->SendChat(_speaker, _textId, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, NULL, false);
            return true;
        }

    private:
        Creature * _speaker;
        uint32 _textId;
    };

    enum 
    {
        EVENT_START_EVENT       = 1,
        EVENT_CALL_FIRST_WAVE,
        EVENT_CALL_SECOND_WAVE,
        EVENT_CALL_THIRD_WAVE,
        EVENT_CALL_FOURTH_WAVE,
        EVENT_CALL_BOSS,
        EVENT_PLAYER_CHECK,
        EVENT_INTRO_MOVE,
        EVENT_AMBERWING,
        EVENT_OUTRO_MOVE,
        EVENT_PLACE_BOMBS,
        EVENT_DETONATE_SPEECH,
        EVENT_DETONATE_BOMBS,

        PATH_OUTRO              = 1
    };

   //enum Quotes
   //{
   //    EMOTE_FIRST_WAVE,
   //    EMOTE_SECOND_WAVE,
   //    EMOTE_THIRD_WAVE,
   //    EMOTE_FOURTH_WAVE,
   //};

    enum Waves
    {
        WAVE_ONE,
        WAVE_TWO,
        WAVE_THREE,
        WAVE_FOUR,
        WAVE_BOSS
    };

    enum Misc
    {
        ACTION_START_EVENT      = 1,
    };

    struct npc_yang_ironclawAI : public ScriptedAI
    {
        npc_yang_ironclawAI(Creature * creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            encounterInProgress = false;
        }

        void Reset()
        {
            
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
            if (instance->GetBossState(BOSS_VOJAK) == DONE)
                me->DespawnOrUnsummon(500);
            else
            {
                ResetEncounter();
                events.Reset();
            }
        }

        void SummonWaveNPC(uint32 entry, const Position  &spawnPos, uint8 waveId, uint8 addNumber, bool isLeftSide)
        {
            // send everything in a bitmask (8 bit / value)
            uint32 mask = (waveId << 0);
            mask |= (addNumber << 8);
            mask |= (isLeftSide << 16);

            if (Creature * creature = me->SummonCreature(entry, spawnPos))
                creature->AI()->SetData(0, mask);
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_BREACH_DOOR_TARGET)
            {
                target->CastSpell(target, SPELL_BREACH_DOOR_EFFECT, true);
                target->ToCreature()->DespawnOrUnsummon(300);
            }
        }

        void DamageTaken(Unit* , uint32& damage)
        {
            damage = 0;
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_START_EVENT)
                if (instance->GetBossState(BOSS_VOJAK) != DONE)
                    StartEncounter();
        }

        void ResetEncounter()
        {
            me->GetMotionMaster()->MoveTargetedHome();
            instance->SetBossState(BOSS_VOJAK, NOT_STARTED);
            encounterInProgress = false;
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            summons.DespawnAll();
            events.Reset();
            bombCnt = 0;

            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CARRYING_CAUSTIC_TAR);


            std::list<Creature*> cList;
            me->GetCreatureListWithEntryInGrid(cList, NPC_BARREL_TARGET, 500.0f);
            for (auto itr : cList)
                itr->RemoveAllAuras();

            cList.clear();

            me->GetCreatureListWithEntryInGrid(cList, NPC_MANTID_TAR_KEG, 500.0f);
            for (auto itr : cList)
            {
                itr->ExitVehicle();
                itr->ForcedDespawn(200);
            }

            if (Creature * loChu = me->FindNearestCreature(NPC_LO_CHU, 500.0f))
                loChu->AI()->DoAction(ACTION_RESET_COMBAT);

            if (Creature * liChu = me->FindNearestCreature(NPC_LI_CHU, 500.0f))
                liChu->AI()->DoAction(ACTION_RESET_COMBAT);

            // Summon waves

            uint32 spawnCounter = 0;

            me->GetMap()->LoadGrid(1509.67f, 5424.98f);

            for (int wave = 0; wave < WAVE_COUNT; ++wave)
            {
                // left side spawns
                spawnCounter = 0;
                for (auto itr = LeftSpawns[wave].begin(); itr != LeftSpawns[wave].end(); ++itr, ++spawnCounter)
                    SummonWaveNPC(itr->entry, itr->spawnPos, wave, spawnCounter, true);

                // right side spawns
                spawnCounter = 0;
                for (auto itr = RightSpawns[wave].begin(); itr != RightSpawns[wave].end(); ++itr, ++spawnCounter)
                    SummonWaveNPC(itr->entry, itr->spawnPos, wave, spawnCounter, false);

            }

            if (Creature * creature = me->SummonCreature(NPC_SIKTHIK_WARRIOR, 1498.52f, 5411.15f, 144.389f, 4.91132f))
                creature->AI()->SetData(0, 3 | (1 << 16)); // join the fourth wave

            if (Creature * creature = me->SummonCreature(NPC_SIKTHIK_WARRIOR, 1523.97f, 5414.13f, 141.615f, 4.91132f))
                creature->AI()->SetData(0, 2 | (1 << 16)); // join the third wave

            // boss spawn

            if (Creature * vojak = Creature::GetCreature(*me, vojakGUID))
                vojak->DespawnOrUnsummon();

            if (Creature * creature = me->SummonCreature(NPC_VOJAK, 1509.67f, 5424.98f, 145.687f, 5.07204f))
            {
                vojakGUID = creature->GetGUID();
                creature->GetMotionMaster()->MoveRandom(5.0f);
            }
        }

        Creature * GetVojak() const
        {
            ASSERT(vojakGUID);

            if (Creature * vojak = Creature::GetCreature(*me, vojakGUID))
                return vojak;

            return NULL;
        }

        void DelayedTalk(Creature * speaker, uint32 id, uint32 timer)
        {
            if (!speaker)
                return;
             // Should be safe to pass a pointer here
            CreatureTalkEvent * talkEvent = new CreatureTalkEvent(speaker, id);
            speaker->m_Events.AddEvent(talkEvent, speaker->m_Events.CalculateTime(timer));
        }

        void JustSummoned(Creature* summon)
        {
            summon->setActive(true);
            if (summon->GetEntry() != NPC_VOJAK)
                summons.Summon(summon);
        }

        void SummonedCreatureDies(Creature* victim, Unit* )
        {
            if (victim->GetEntry() == NPC_VOJAK)
            {
                instance->SetBossState(BOSS_VOJAK, DONE);
                summons.DespawnAll();
                events.Reset();

                if (Creature * loChu = me->FindNearestCreature(NPC_LO_CHU, 500.0f))
                    loChu->AI()->DoAction(ACTION_RESET_COMBAT);

                if (Creature * liChu = me->FindNearestCreature(NPC_LI_CHU, 500.0f))
                    liChu->AI()->DoAction(ACTION_RESET_COMBAT);

                DelayedTalk(me, SAY_OUTRO_1, 2000);
                DelayedTalk(me, SAY_OUTRO_2, 5000);
                events.ScheduleEvent(EVENT_OUTRO_MOVE, 6000);

                std::list<Creature*> cList;
                me->GetCreatureListWithEntryInGrid(cList, NPC_MANTID_TAR_KEG, 500.0f);
                for (auto itr : cList)
                {
                    itr->SetRespawnDelay(DAY);
                    itr->ForcedDespawn();
                }
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (id == POINT_INTRO)
                {
                    Talk(SAY_AGGRO_3);
                    DelayedTalk(me, SAY_AGGRO_4, 3000);
                    DelayedTalk(me, SAY_AGGRO_5, 7000);
                }
                else if (id == POINT_BOMB)
                {
                    if (bombCnt < 5)
                    {
                        DoCast(me, SPELL_MAKESHIFT_CHARGE, false);
                        events.ScheduleEvent(EVENT_PLACE_BOMBS, 1000);
                    }
                    else
                    {
                        DoCast(me, SPELL_MAKESHIFT_CHARGE, false);
                        events.ScheduleEvent(EVENT_DETONATE_SPEECH, 1000);
                    }
                }
            }
        }

        void StartEncounter()
        {
            if (encounterInProgress)
                return;
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            encounterInProgress = true;
            instance->SetBossState(BOSS_VOJAK, IN_PROGRESS);
            events.ScheduleEvent(EVENT_CALL_FIRST_WAVE, 20000);
            events.ScheduleEvent(EVENT_CALL_SECOND_WAVE, 60000);
            events.ScheduleEvent(EVENT_CALL_THIRD_WAVE, 100000);
            events.ScheduleEvent(EVENT_CALL_FOURTH_WAVE, 140000);
            events.ScheduleEvent(EVENT_CALL_BOSS, 180000);
            events.ScheduleEvent(EVENT_PLAYER_CHECK, 5000);

            Talk(SAY_AGGRO_1);
            DelayedTalk(me, SAY_AGGRO_2, 3000);
            events.ScheduleEvent(EVENT_INTRO_MOVE, 5000);
            events.ScheduleEvent(EVENT_AMBERWING, 30000);

            if (Creature * loChu = me->FindNearestCreature(NPC_LO_CHU, 500.0f))
                loChu->AI()->DoAction(ACTION_ENGAGE_COMBAT);

            if (Creature * liChu = me->FindNearestCreature(NPC_LI_CHU, 500.0f))
                liChu->AI()->DoAction(ACTION_ENGAGE_COMBAT);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CALL_FIRST_WAVE: // Swarmers
                    {
                        Talk(EMOTE_WAVE_ONE);
                        DelayedTalk(GetVojak(), SAY_VOJAK_WAVE_ONE, 500);
                        DelayedTalk(me, SAY_WAVE_ONE, 6000);
                        EntryCheckPredicate pred(NPC_SIKTHIK_SWARMER);
                        summons.DoAction((ACTION_ENGAGE_COMBAT << WAVE_ONE), pred);
                        break;
                    }
                    case EVENT_CALL_SECOND_WAVE: // Demolishers
                    {
                        Talk(EMOTE_WAVE_TWO);
                        DelayedTalk(GetVojak(), SAY_VOJAK_WAVE_TWO, 500);
                        DelayedTalk(me, SAY_WAVE_TWO, 5000);
                        EntryCheckPredicate pred(NPC_SIKTHIK_DEMOLISHER);
                        summons.DoAction((ACTION_ENGAGE_COMBAT << WAVE_TWO), pred);
                        break;
                    }
                    case EVENT_CALL_THIRD_WAVE: // Swarmers + Warrior
                    {
                        Talk(EMOTE_WAVE_THREE);
                        DelayedTalk(GetVojak(), SAY_VOJAK_WAVE_THREE, 500);
                        DelayedTalk(me, SAY_WAVE_THREE, 5000);
                        EntryCheckPredicate pred(NPC_SIKTHIK_WARRIOR);
                        EntryCheckPredicate pred2(NPC_SIKTHIK_SWARMER);
                        summons.DoAction((ACTION_ENGAGE_COMBAT << WAVE_THREE), pred);
                        summons.DoAction((ACTION_ENGAGE_COMBAT << WAVE_THREE), pred2);
                        break;
                    }
                    case EVENT_CALL_FOURTH_WAVE: // Demolishers + Warrior
                    {
                        Talk(EMOTE_WAVE_FOUR);
                        DelayedTalk(GetVojak(), SAY_VOJAK_WAVE_FOUR, 500);
                        EntryCheckPredicate pred(NPC_SIKTHIK_WARRIOR);
                        EntryCheckPredicate pred2(NPC_SIKTHIK_DEMOLISHER);
                        summons.DoAction((ACTION_ENGAGE_COMBAT << WAVE_FOUR), pred);
                        summons.DoAction((ACTION_ENGAGE_COMBAT << WAVE_FOUR), pred2);
                        break;
                    }
                    case EVENT_AMBERWING:
                        DelayedTalk(me, EMOTE_BOMBARD, 4000);
                        DelayedTalk(me, SAY_AMBERWING, 4500);
                        me->SummonCreature(NPC_AMBERWING, 1651.384f, 5397.722f, 152.5865f);
                        break;
                    case EVENT_CALL_BOSS: // Vo'Jak
                        if (Creature * vojak = Creature::GetCreature(*me, vojakGUID))
                            vojak->AI()->DoAction(ACTION_ENGAGE_COMBAT);
                        break;
                    case EVENT_INTRO_MOVE:
                        me->GetMotionMaster()->MovePoint(POINT_INTRO, 1523.673584f, 5313.781250f, 185.226746f);
                        break;
                    case EVENT_PLAYER_CHECK:
                    {
                        bool playersAlive = false;
                        Map::PlayerList const &players = me->GetMap()->GetPlayers();
                        for (auto itr = players.begin(); itr != players.end(); ++itr)
                        {
                            if (Player* player = itr->GetSource())
                                if (player->IsAlive() && !player->isGameMaster() && player->GetWMOAreaId() == WMO_REAR_STAGING_AREA)
                                {
                                    playersAlive = true;
                                    break;
                                }
                        }

                        if (!playersAlive)
                            ResetEncounter();
                        else
                            events.ScheduleEvent(EVENT_PLAYER_CHECK, 5000);
                        break;
                    }
                    case EVENT_OUTRO_MOVE:
                        me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

                        if (Creature * loChu = me->FindNearestCreature(NPC_LO_CHU, 500.0f))
                            loChu->GetMotionMaster()->MoveSplinePath(PATH_OUTRO, false, false, 5.0f, false, false, false);

                        if (Creature * liChu = me->FindNearestCreature(NPC_LI_CHU, 500.0f))
                            liChu->GetMotionMaster()->MoveSplinePath(PATH_OUTRO, false, false, 5.0f, false, false, false);

                        me->GetMotionMaster()->MoveSplinePath(PATH_OUTRO, false, false, 8.0f, false, false, false);
                        DelayedTalk(me, SAY_GATE_1, me->GetSplineDuration());
                        events.ScheduleEvent(EVENT_PLACE_BOMBS, me->GetSplineDuration() + 5000);
                        break;
                    case EVENT_DETONATE_SPEECH:
                        Talk(SAY_GATE_2);
                        me->GetMotionMaster()->MovePoint(0, 1615.413f, 5407.448f, 138.68094f);
                        DelayedTalk(me, SAY_GATE_3, 5000);
                        events.ScheduleEvent(EVENT_DETONATE_BOMBS, 7500);
                        break;
                    case EVENT_PLACE_BOMBS:
                        me->GetMotionMaster()->MovePoint(POINT_BOMB, bombsPos[bombCnt++]);
                        break;
                    case EVENT_DETONATE_BOMBS:
                        instance->SetData(DATA_VOJAK_DOOR, 0);
                        DoCast(me, SPELL_BREACH_DOOR_TARGET, true);
                        me->GetMotionMaster()->MovePoint(0, 1615.413f, 5407.448f, 138.68094f);
                        DelayedTalk(me, SAY_GATE_4, 4000);
                        break;
                    default:
                        break;
                }
            }
        }
    private:
        uint8 bombCnt;
        EventMap events;
        SummonList summons;
        InstanceScript * instance;
        uint64 vojakGUID;
        bool encounterInProgress;
    };

public:

    bool OnGossipHello(Player* player, Creature* creature) final
    {
        //if (InstanceScript * instance = player->GetInstanceScript())
        //{
        //    //if (instance->GetBossState())
        //}

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "We're ready to defend!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

        // "Thanks for saving us! Are you ready to get this party started?"
        player->SEND_GOSSIP_MENU(2475, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) final
    {
        player->PlayerTalkClass->ClearMenus();
        player->CLOSE_GOSSIP_MENU();

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 5:
            {
                if (creature->IsAIEnabled)
                    creature->GetAI()->DoAction(ACTION_START_EVENT);
            }
            break;
        }

        return true;
    }

    npc_yang_ironclaw() : CreatureScript("npc_yang_ironclaw") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_yang_ironclawAI(creature);
    }
};


class npc_sikthik_warden : public CreatureScript
{
    struct npc_sikthik_wardenAI : public ScriptedAI
    {
        npc_sikthik_wardenAI(Creature * creature) : ScriptedAI(creature) {}
    };

public:
    npc_sikthik_warden() : CreatureScript("npc_sikthik_warden") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sikthik_wardenAI(creature);
    }
};

static const Position PATH_POINTS[] =
{
    
    {1511.885f, 5378.232f, 138.95970f, 4.761f},
    {1512.162f, 5372.485f, 139.08124f, 4.765f},
    {1512.879f, 5359.946f, 146.22418f, 4.816f},
    {1524.942f, 5348.655f, 146.23088f, 0.053f},
    {1549.231f, 5350.165f, 160.99088f, 0.037f},
    {1567.434f, 5341.903f, 161.21361f, 3.485f},
    {1555.366f, 5329.321f, 161.66699f, 3.151f},
    {1507.551f, 5326.483f, 161.21100f, 3.233f},
    {1480.644f, 5324.358f, 176.12379f, 3.237f},
    {1470.566f, 5301.831f, 176.12379f, 4.769f},
    {1471.052f, 5292.643f, 179.70631f, 4.789f},
    {1483.235f, 5286.341f, 179.77177f, 0.159f},
    {1493.571f, 5287.627f, 184.71986f, 0.127f}
};

/*
    Koz:
    This is a generic AI for the Demolishers, Swarmers and Warriors
    It handles wave movement:
    - wave assignment (side -> movement timer)
    - randomized path generation
    - engaging when platform is reached
*/

struct npc_vojak_addAI : public ScriptedAI
{
    enum
    {
        ADD_STEP_DELAY          = 1000, // delay per add number
        RIGHT_SIDE_DELAY        = 3000  // added delay if on right side
    };

    npc_vojak_addAI(Creature * creature) : ScriptedAI(creature)
    {
        triggerTimer = 0;
        waveNumber = 0;
        summonerGUID = 0;
        addCounter = 0;
        TRIGGER_TIMER = 0;
        aggrotimer = 0;
    }


    void DoAction(const int32 action)
    {
        if (action & (ACTION_ENGAGE_COMBAT << waveNumber))
        {
            //me->MonsterYell("triggered", 0, 0);
            me->SetReactState(REACT_PASSIVE);
            triggerTimer = TRIGGER_TIMER;
            //TC_LOG_FATAL("script", "Triggering %s, %u seconds", me->GetName().c_str(), triggerTimer);
        }
    }

    void SetData(uint32 type, uint32 data)
    {
        if (type == 0)
        {
            // read the mask (left side, add number, wave Id)
            bool leftSideAdd = (data >> 16) & 0xFF;
            addCounter = (data >> 8) & 0xFF;
            waveNumber = data & 0xFF;
            uint32 entry = me->GetEntry();
            
            TRIGGER_TIMER = ADD_STEP_DELAY + addCounter * ADD_STEP_DELAY + (leftSideAdd ? 0 : RIGHT_SIDE_DELAY);
        }
    }

    //void MovementInform(uint32 type, uint32 id)
    //{
    //    if (type == WAYPOINT_MOTION_TYPE && id == 13)
    //        aggrotimer = 500;
    //}

    void UpdateAI(uint32 const diff)
    {
        if (triggerTimer)
        {
            if (triggerTimer <= diff)
            {
                // move towards the upper platform
                Movement::MoveSplineInit init(me);
                for (auto itr : PATH_POINTS)
                {
                    float x, y, z;
                    me->GetRandomPoint(itr, 5.0f, x, y, z);
                    init.Path().push_back(G3D::Vector3(x, y, z));
                }
                
                init.SetSmooth();
                init.SetUncompressed();
                init.Launch();
                //me->GetMotionMaster()->MovePath(6162001, false);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
                me->SetReactState(REACT_PASSIVE);
                aggrotimer = me->GetSplineDuration();
                triggerTimer = 0;
            } else triggerTimer -= diff;
        }

        if (aggrotimer)
        {
            if (aggrotimer <= diff)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 500.0f);
                aggrotimer = 0;
            } else aggrotimer -= diff;
        }

        if (me->HasReactState(REACT_PASSIVE))
            return;

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void IsSummonedBy(Unit* summoner)
    {

    }

private:
    uint8 waveNumber;
    uint64 summonerGUID;
    uint32 triggerTimer;
    uint32 TRIGGER_TIMER;
    uint32 aggrotimer;
    //bool leftSideAdd;
    uint8 addCounter;
};

class boss_commander_vojak : public CreatureScript
{
    enum Yells
    {

    };

    enum Spells
    {
        SPELL_FRANTIC_FIGHTER       = 120757,
        SPELL_DASHING_STRIKE        = 120789,
        SPELL_THOUSAND_BLADES       = 120759
    };

    enum Events
    {
        EVENT_ENGAGE_COMBAT     = 1,
        EVENT_RISING_SPEED,
        EVENT_DASHING_STRIKE,
        EVENT_THOUSAND_BLADES,
        EVENT_THOUSAND_BLADES_END
    };

    struct boss_commander_vojakAI : public ScriptedAI
    {
        boss_commander_vojakAI(Creature * creature) : ScriptedAI(creature)
        {
            moving = true;
        }

        void Reset()
        {
            events.Reset();

        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_ENGAGE_COMBAT)
            {
                moving = true;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_UNK_6 | UNIT_FLAG_UNK_15);
                me->SetReactState(REACT_PASSIVE);
                DoZoneInCombat(me);

                // move towards the upper platform
                Movement::MoveSplineInit init(me);
                for (auto itr : PATH_POINTS)
                {
                    float x, y, z;
                    me->GetRandomPoint(itr, 5.0f, x, y, z);
                    init.Path().push_back(G3D::Vector3(x, y, z));
                }

                init.SetSmooth();
                init.SetUncompressed();
                init.Launch();

                events.ScheduleEvent(EVENT_ENGAGE_COMBAT, me->GetSplineDuration());
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!moving)
                if (!UpdateVictimWithGaze() || !UpdateVictim())
                    return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENGAGE_COMBAT:
                        moving = false;
                        me->SetReactState(REACT_AGGRESSIVE);
                        events.ScheduleEvent(EVENT_RISING_SPEED, 3000);
                        events.ScheduleEvent(EVENT_DASHING_STRIKE, 5000);
                        break;
                    case EVENT_RISING_SPEED:
                        DoCast(me, SPELL_FRANTIC_FIGHTER, true);
                        break;
                    case EVENT_DASHING_STRIKE:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        {
                            SetGazeOn(target);
                            DoCast(target, SPELL_DASHING_STRIKE, false);
                            dashingTargetGUID = target->GetGUID();
                        }
                        events.ScheduleEvent(EVENT_THOUSAND_BLADES, 3000);
                        events.ScheduleEvent(EVENT_DASHING_STRIKE, 13500);
                        break;
                    case EVENT_THOUSAND_BLADES:
                        DoCast(me, SPELL_THOUSAND_BLADES, false);
                        events.ScheduleEvent(EVENT_THOUSAND_BLADES_END, 4000);
                        break;
                    case EVENT_THOUSAND_BLADES_END:
                        me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    default:
                        break;
                }
            }

            if (!moving)
                DoMeleeAttackIfReady();
        }
    private:
        bool moving;
        EventMap events;
        uint64 dashingTargetGUID;
    };
public:
    boss_commander_vojak() : CreatureScript("boss_commander_vojak") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new boss_commander_vojakAI(creature);
    }
};

// 63106
class npc_sikthik_swarmer : public CreatureScript
{

public:
    npc_sikthik_swarmer() : CreatureScript("npc_sikthik_swarmer") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_vojak_addAI(creature);
    }
};

// 61670
class npc_sikthik_demolisher : public CreatureScript
{

public:
    npc_sikthik_demolisher() : CreatureScript("npc_sikthik_demolisher") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_vojak_addAI(creature);
    }
};

// 61701
class npc_sikthik_warrior : public CreatureScript
{

public:
    npc_sikthik_warrior() : CreatureScript("npc_sikthik_warrior") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_vojak_addAI(creature);
    }
};


    /*
    Koz:
    This is a script for Lo Chu and Li Chu
    Keg Rotation:
        1. NPC runs towards the keg pool
        2. NPC casts Assignment Spell (periodic trigger) on self when arrived at the pool
    If Assignment spell hits an NPC (Barrel Target without "Has Barrel" aura):
        3. NPC picks up a random Keg in 10yd radius (Grab Barrel spell)
        4. NPC moves (WALKS!) to an invisible stalker near the said target NPC
        5. When arrived, NPC ejects the keg (jumps to targeted Barrel Target NPC)
        6. NPC moves (RUNS!) back to the keg pool
    */

enum HelperData
{
    SIDE_LEFT           = 0,
    SIDE_RIGHT,

    POINT_KEG_POOL      = 0,
    POINT_KEG,
    POINT_KEG_MID,
    POINT_KEG_SIDE,

};

static const Position helperPositions[2][3] = 
{
    // SIDE_LEFT
    {
        {1488.913f, 5299.012f, 184.64816f, 4.785f}, // Keg pool
        {1513.255f, 5309.223f, 184.64906f, 1.741f},
        {1488.649f, 5307.495f, 184.64957f, 1.564f}
    },
    // SIDE_RIGHT
    {
        {1548.154f, 5287.516f, 184.74574f, 4.749f}, // keg pool
        {1540.258f, 5310.451f, 184.65018f, 1.600f},
        {1556.005f, 5312.284f, 184.65018f, 1.639f}
    }
};

enum 
{
    SPELL_GRAB_BARREL           = 120405, // condition target keg, triggers SPELL_KEG_ENTER_VEHICLE
    SPELL_KEG_ENTER_VEHICLE     = 120402,
    SPELL_ASSIGNMENT            = 122347, // periodic, find nearest keg target
    SPELL_ASSIGNMENT_EFF        = 122346, // condition target Barrel Target, triggers SPELL_HAS_BARREL
    SPELL_HAS_BARREL            = 122345,
    SPELL_CLEAR_HAS_BARREL      = 122518, // 5yd aoe
    SPELL_EJECT_PASSENGERS      = 79737,
    SPELL_BARREL_DROP_FORCE     = 122385,
    SPELL_BARREL_JUMP           = 122376, // condition target Barrel Target?
};

class npc_chu_helper : public CreatureScript
{
    enum
    {
        EVENT_PICK_UP_KEG           = 1,
        EVENT_MOVE_TO_TARGET,
        EVENT_PLACE_KEG,
        EVENT_MOVE_TO_POOL,

    };

    enum
    {
    };



    struct npc_chu_helperAI : public ScriptedAI
    {
        npc_chu_helperAI(Creature * creature) : ScriptedAI(creature)
        {
            side = (me->GetEntry() == NPC_LO_CHU) ? SIDE_RIGHT : SIDE_LEFT;
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
            barrelTargetGUID = 0;
            barrelGUID = 0;
            eventInProgress = false;
            events.Reset();
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (!eventInProgress)
                return;

            switch (spell->Id)
            {
                case SPELL_GRAB_BARREL:
                    barrelGUID = target->GetGUID();
                    target->CastSpell(me, SPELL_KEG_ENTER_VEHICLE, true);
                    events.ScheduleEvent(EVENT_MOVE_TO_TARGET, 1000);
                    break;
                case SPELL_ASSIGNMENT_EFF:
                    if (barrelTargetGUID)
                        break;
                    barrelTargetGUID = target->GetGUID();
                    me->RemoveAurasDueToSpell(SPELL_ASSIGNMENT);
                    DoCast(target, SPELL_HAS_BARREL, true);
                    DoCast(me, SPELL_GRAB_BARREL, true);
                    break;
                default:
                    break;
            }
        }

        void DamageTaken(Unit* , uint32& damage)
        {
            damage = 0;
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_ENGAGE_COMBAT:
                    eventInProgress = true;
                    events.ScheduleEvent(EVENT_MOVE_TO_POOL, side ? 1000 : 2000);
                    barrelTargetGUID = 0;
                    break;
                case ACTION_RESET_COMBAT:
                    //me->RemoveAllAuras();
                    //me->GetVehicleKit()->RemoveAllPassengers();
                    events.Reset();
                    eventInProgress = false;
                    barrelTargetGUID = 0;
                    EnterEvadeMode();
                    break;
                default:
                    break;
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case POINT_KEG_POOL:
                        events.ScheduleEvent(EVENT_PICK_UP_KEG, 1000);
                        break;
                    case POINT_KEG:
                        //DoCast(me, SPELL_EJECT_PASSENGERS, true);
                        me->GetVehicleKit()->RemoveAllPassengers();
                        if (Creature * barrel = Creature::GetCreature(*me, barrelGUID))
                            if (Creature * barrelTarget = Creature::GetCreature(*me, barrelTargetGUID))
                            {
                                barrel->ExitVehicle();
                                //barrel->CastSpell(barrelTarget, SPELL_BARREL_JUMP, true);
                                barrel->AI()->SetGUID(barrelTargetGUID);
                            }
                        barrelTargetGUID = 0;

                        events.ScheduleEvent(EVENT_MOVE_TO_POOL, 1000);
                        break;
                }
            }

        }

        void UpdateAI(const uint32 diff)
        {
            if (UpdateVictim())
                DoMeleeAttackIfReady();

            if (!eventInProgress)
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_PICK_UP_KEG:
                        DoCast(me, SPELL_ASSIGNMENT, true);
                        break;
                    case EVENT_MOVE_TO_TARGET:
                    {
                        me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                        if (Creature * target = Creature::GetCreature(*me, barrelTargetGUID))
                            if (Creature * stalker = target->FindNearestCreature(NPC_INVISIBLE_STALKER, 10.0f, true))
                                me->GetMotionMaster()->MovePoint(POINT_KEG, *stalker);
                        break;
                    }
                    case EVENT_PLACE_KEG:
                        break;
                    case EVENT_MOVE_TO_POOL:
                        me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                        me->GetMotionMaster()->MovePoint(POINT_KEG_POOL, helperPositions[side][POINT_KEG_POOL]);
                        break;
                }
            }
        }

    private:
        uint64 barrelTargetGUID;
        uint64 barrelGUID;
        uint8 side;
        bool eventInProgress;
        EventMap events;
    };



public:
    npc_chu_helper() : CreatureScript("npc_chu_helper") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_chu_helperAI(creature);
    }
};

// 120405
class spell_grab_barrel : public SpellScriptLoader
{
public:
    spell_grab_barrel() : SpellScriptLoader("spell_grab_barrel") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_KEG_ACTIVE));
            Trinity::Containers::RandomResizeList(targets, 1);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

// 122346
class spell_barrel_assignment : public SpellScriptLoader
{
public:
    spell_barrel_assignment() : SpellScriptLoader("spell_barrel_assignment") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (Unit * caster = GetCaster())
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_HAS_BARREL));
                Trinity::Containers::RandomResizeList(targets, 1);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

/*
    Koz:
    This is the script for Mantid Tar Keg
    We handle respawn on wipe aswell as 
    jumping after delivery here.
*/
class npc_mantid_tar_keg : public CreatureScript
{
    enum
    {
    };

    struct npc_mantid_tar_kegAI : public ScriptedAI
    {
        npc_mantid_tar_kegAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetCorpseDelay(1);
            me->SetRespawnDelay(5);
            jumptimer = 0;
        }

        void SetGUID(uint64 guid, int32 /* = 0 */)
        {
            targetGUID = guid;
            jumptimer = 250;
            if (Creature * target = Creature::GetCreature(*me, targetGUID))
            {
                Position pos = *target;
                target->UpdateGroundPositionZ(pos.GetPositionX(), pos.GetPositionY(), pos.m_positionZ);
                me->GetMotionMaster()->MoveJump(pos, 10.0f, 1.0f);
            }
        }

        void Reset()
        {
            me->SetPosition(me->GetHomePosition());
            if (me->m_movementInfo.t_guid)
                me->m_movementInfo.t_guid = 0;
            me->SendMovementFlagUpdate();
            targetGUID = 0;
            me->GetMotionMaster()->MoveIdle();
            me->SetReactState(REACT_PASSIVE);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            DoCast(me, SPELL_KEG_INACTIVE, true);
        }

        bool CanRespawn()
        {
            return instance->GetBossState(BOSS_VOJAK) == NOT_STARTED;
        }

        void OnSpellClick(Unit* clicker, bool &)
        {
            //clicker->CastSpell(clicker, SPELL_CARRYING_CAUSTIC_TAR, true);
            if (Creature * target = Creature::GetCreature(*me, targetGUID))
                target->RemoveAllAuras();
            me->DisappearAndDie();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == EFFECT_MOTION_TYPE && id == EVENT_JUMP)
            {
                me->RemoveAurasDueToSpell(SPELL_KEG_INACTIVE);
                DoCast(me, SPELL_KEG_ACTIVE, true);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!jumptimer)
                return;

            if (jumptimer <= diff)
            {

                jumptimer = 0;
            } else jumptimer -= diff;
        }

    private:
        uint32 jumptimer;
        uint64 targetGUID;
        InstanceScript * instance;
    };

public:
    npc_mantid_tar_keg() : CreatureScript("npc_mantid_tar_keg") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_mantid_tar_kegAI(creature);
    }
};

class npc_sap_puddle_vojak : public CreatureScript
{
    enum
    {
        SPELL_DRAIN_BARREL_TOP          = 122522, // Areatrigger 349
        SPELL_DRAIN_BARREL_BASE         = 120473  // Areatrigger 325, 359
    };

    struct npc_sap_puddle_vojakAI : public ScriptedAI
    {
        npc_sap_puddle_vojakAI(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            aoeTimer = 500;

            uint32 spellId = SPELL_DRAIN_BARREL_BASE;
            if (fabs(me->GetPositionZ() - 185.22f) < INTERACTION_DISTANCE)
                spellId = SPELL_DRAIN_BARREL_TOP;

            DoCast(me, spellId, true);
        }

        void MoveInLineOfSight(Unit* who)
        {

        }

        void UpdateAI(uint32 const diff)
        {
            if (aoeTimer <= diff)
            {
                DoCast(me, 120270, true);
                aoeTimer = 500;
            } else aoeTimer -= diff;
        }
    private:
        uint32 aoeTimer;
    };

public:
    npc_sap_puddle_vojak() : CreatureScript("npc_sap_puddle_vojak") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sap_puddle_vojakAI(creature);
    }
};

// Sik'Thik amberwing - 61699
class npc_sikthik_amberwing : public CreatureScript
{
    enum
    {
        SPELL_GREEN_WINGS           = 126316,
        SPELL_BOMBARD               = 120559,
        SPELL_BOMBARD_DUMMY         = 120202,
        SPELL_BOMBARD_PROTECTION    = 120561,

        PATH_PLATFORM               = 1,
        PATH_HOME
    };

    struct npc_sikthik_amberwingAI : public ScriptedAI
    {
        npc_sikthik_amberwingAI(Creature * creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            arrivalTimer = 0;
            flyTimer = 0;
        }

        void IsSummonedBy(Unit* )
        {
            DoCast(me, SPELL_GREEN_WINGS, true);
            me->GetMotionMaster()->MoveSplinePath(PATH_PLATFORM, true, false, 10.0f);
            arrivalTimer = me->GetSplineDuration() + 1000;
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_BOMBARD_DUMMY && target->HasAura(SPELL_BOMBARD_PROTECTION))
                DoCast(target, SPELL_BOMBARD_PROTECTION, true);
        }

        void UpdateAI(uint32 const diff)
        {
            if (arrivalTimer)
            {
                if (arrivalTimer <= diff)
                {
                    me->SetFacingTo(4.81f);
                    DoCast(me, SPELL_BOMBARD, false);
                    flyTimer = 16000;
                    arrivalTimer = 0;
                } else arrivalTimer -= diff;
            }

            if (flyTimer)
            {
                if (flyTimer <= diff)
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->GetMotionMaster()->MoveSplinePath(PATH_HOME, true, false, 10.0f);
                    flyTimer = 0;
                } else flyTimer -= diff;
            }
        }


    private:
        uint32 arrivalTimer;
        uint32 flyTimer;
    };

public:
    npc_sikthik_amberwing() : CreatureScript("npc_sikthik_amberwing") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sikthik_amberwingAI(creature);
    }
};


/*

class npc_vojak_add : public CreatureScript
{
    struct npc_vojak_addAI : public ScriptedAI
    {
        npc_vojak_addAI(Creature * creature) : ScriptedAI(creature) {}
    };

public:
    npc_vojak_add() : CreatureScript("npc_vojak_add") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_vojak_addAI(creature);
    }
};
*/

void AddSC_commander_vojak()
{
    new boss_commander_vojak();
    new npc_yang_ironclaw();

    new npc_sikthik_demolisher();
    new npc_sikthik_swarmer();
    new npc_sikthik_warden();
    new npc_sikthik_warrior();
    new npc_chu_helper();

    new npc_mantid_tar_keg();
    new spell_barrel_assignment();
    new spell_grab_barrel();
    new npc_sap_puddle_vojak();

    new npc_sikthik_amberwing();
}