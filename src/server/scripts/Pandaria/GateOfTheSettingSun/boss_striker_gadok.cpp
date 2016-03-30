/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "gate_of_the_setting_sun.h"
#include "Vehicle.h"
#include "VehicleDefines.h"

struct StrafPointStruct
{
    uint8 pointIdBeginOutside;
    Position beginOutside;

    uint8 pointIdBegin;
    Position begin;

    uint8 pointIdEnd;
    Position end;

    uint8 pointIdOutside;
    Position outside;
};

enum eMovements
{
    POINT_NORTH_START = 1,
    POINT_SOUTH_START = 2,
    POINT_WEST_START = 3,
    POINT_EAST_START = 4,

    POINT_NORTH_END = 5,
    POINT_SOUTH_END = 6,
    POINT_WEST_END = 7,
    POINT_EAST_END = 8,

    POINT_NORTH_OUTSIDE = 9,
    POINT_SOUTH_OUTSIDE = 10,
    POINT_WEST_OUTSIDE = 11,
    POINT_EAST_OUTSIDE = 12,

    POINT_KRIKTHIK_CIRCLE = 13,

    MOV_NORTH_SOUTH = 0,
    MOV_SOUTH_NORTH = 1,
    MOV_WEST_EAST = 2,
    MOV_EAST_WEST = 3
};

StrafPointStruct StrafPoints[4] =
{
    { POINT_NORTH_OUTSIDE, { 1258.0f, 2304.644f, 438.0f, 0.0f }, POINT_NORTH_START, { 1238.007f, 2304.644f, 435.0f, 0.0f }, POINT_NORTH_END, { 1153.398f, 2304.578f, 435.0f, 0.0f }, POINT_SOUTH_OUTSIDE, { 1133.4f, 2304.578f, 438.0f, 0.0f } }, // North -> South
    { POINT_SOUTH_OUTSIDE, { 1133.4f, 2304.578f, 438.0f, 0.0f }, POINT_SOUTH_START, { 1153.398f, 2304.578f, 435.0f, 0.0f }, POINT_SOUTH_END, { 1238.007f, 2304.644f, 435.0f, 0.0f }, POINT_NORTH_OUTSIDE, { 1258.0f, 2304.644f, 438.0f, 0.0f } }, // South -> North
    { POINT_WEST_OUTSIDE, { 1195.3f, 2366.941f, 438.0f, 0.0f }, POINT_WEST_START, { 1195.299f, 2348.941f, 435.0f, 0.0f }, POINT_WEST_END, { 1195.392f, 2263.441f, 435.0f, 0.0f }, POINT_EAST_OUTSIDE, { 1195.4f, 2243.441f, 438.0f, 0.0f } }, // West  -> East
    { POINT_EAST_OUTSIDE, { 1195.4f, 2243.441f, 438.0f, 0.0f }, POINT_EAST_START, { 1195.392f, 2263.441f, 435.0f, 0.0f }, POINT_EAST_END, { 1195.299f, 2348.941f, 435.0f, 0.0f }, POINT_WEST_OUTSIDE, { 1195.3f, 2366.941f, 438.0f, 0.0f } }  // East  -> West
};

Position CenterPos = { 1195.0f, 2304.0f, 438.0f };

enum eSpells
{
    SPELL_PREY_TIME = 106933,
    SPELL_IMPALING_STRIKE = 107047,

    SPELL_STRAFING_RUN = 107342,
    SPELL_STRAFIND_RUN_DMG = 116298,

    SPELL_PREY_TIME_VEHICLE_AURA = 106934,
    SPELL_EJECT_PASSENGERS = 68576,

    // Disruptor
    SPELL_BOMB = 115110,
    SPELL_TELEPORT_VISUAL = 52096
};

enum eEvents
{
    EVENT_PREY_TIME         = 1,
    EVENT_IMPALING_STRIKE   = 2,
    EVENT_DISRUPTOR_BOMBARD = 3,
    EVENT_SUMMONS           = 4
};

enum ePhases
{
    PHASE_MAIN        = 1,
    PHASE_NORTH_SOUTH = 2,
    PHASE_WEST_EAST   = 3
};

enum eStrafing
{
    STRAF_NONE = 0,
    STRAF_70   = 1,
    STRAF_30   = 2
};

enum eActions
{
    ACTION_INTRO = 1
};

enum eTalks
{
    SAY_INTRO       = 0,
    SAY_AGGRO       = 1,
    SAY_DEATH       = 2,
    SAY_SLAY        = 3,
    SAY_STIKE_EMOTE = 4
};

#define MAX_DISRUPTOR   5
#define MAX_STRIKER     10
#define RADIUS_CIRCLE   100.0f

class boss_striker_gadok : public CreatureScript
{
public:
    boss_striker_gadok() : CreatureScript("boss_striker_gadok") {}

    struct boss_striker_gadokAI : public BossAI
    {
        boss_striker_gadokAI(Creature* creature) : BossAI(creature, DATA_GADOK)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        bool isStrafing;
        bool intro;

        uint8 strafingEventCount;
        uint8 strafingEventProgress;
        uint8 move;
        uint32 strafingTimer;
    
        EventMap events;
        EventMap nonAggroEvents;

        void InitializeAI()
        {
            me->setActive(true);
            intro = false;

            Reset();
        }

        void Reset()
        {
            _Reset();
            isStrafing = false;

            strafingTimer = 0;
            strafingEventCount = 0;
            strafingEventProgress = 0;
            move = 0;
            me->GetMotionMaster()->Clear();
            events.Reset();
            nonAggroEvents.ScheduleEvent(EVENT_SUMMONS, 2 * IN_MILLISECONDS);

            events.Reset();
            me->SetWalk(false);
            me->SetDisableGravity(true);
            me->SetCanFly(true);
            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);

            if (instance)
            {
                instance->SetData(DATA_GADOK, PHASE_MAIN);

                if (GameObject* portal = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_PORTALS)))
                    portal->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            }   
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_INTRO)
            {
                if (!intro)
                {
                    intro = true;
                    Talk(SAY_INTRO);
                }
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void DoSpawnKrikThik()
        {
            for (uint8 i = 0; i < MAX_DISRUPTOR; ++i)
                SummonKrikThik(NPC_KRIKTHIK_DISRUPTOR);

            for (uint8 i = 0; i < MAX_STRIKER; ++i)
                SummonKrikThik(NPC_KRIKTHIK_STRIKER);
        }

        TempSummon* SummonKrikThik(uint32 creatureId)
        {
            float angle = frand(0, 2 * M_PI);
            float x = CenterPos.GetPositionX() + (RADIUS_CIRCLE * std::cos(angle));
            float y = CenterPos.GetPositionY() + (RADIUS_CIRCLE * std::sin(angle));

            return me->SummonCreature(creatureId, x, y, CenterPos.GetPositionZ());
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);

            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                instance->SetData(DATA_GADOK, IN_PROGRESS);
                if (GameObject* portal = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_PORTALS)))
                    portal->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            }

            events.ScheduleEvent(EVENT_PREY_TIME, urand(9, 15) * IN_MILLISECONDS, PHASE_MAIN);
            events.ScheduleEvent(EVENT_IMPALING_STRIKE, urand(13, 19) * IN_MILLISECONDS, PHASE_MAIN);
        }

        void EnterEvadeMode()
        {
            if (Vehicle* vehicle = me->GetVehicleKit())
               if (!vehicle->HasEmptySeat(0))
                   return;

            StopStrafingEvent();
            BossAI::EnterEvadeMode();
            if (instance)
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void JustReachedHome()
        {
            if (instance)
            {
                instance->SetBossState(DATA_GADOK, FAIL);

                if (GameObject* portal = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_PORTALS)))
                    portal->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            }

            summons.DespawnAll();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (id)
            {
                case POINT_NORTH_START:
                case POINT_SOUTH_START:
                case POINT_WEST_START:
                case POINT_EAST_START:
                case POINT_NORTH_END:
                case POINT_SOUTH_END:
                case POINT_WEST_END:
                case POINT_EAST_END:
                case POINT_NORTH_OUTSIDE:
                case POINT_SOUTH_OUTSIDE:
                case POINT_WEST_OUTSIDE:
                case POINT_EAST_OUTSIDE:
                    DoStrafingEvent();
                    break;
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            float nextHealthPct = ((float(me->GetHealth()) - damage) / float(me->GetMaxHealth())) * 100;

            if (!isStrafing)
            {
                if ((strafingEventCount < STRAF_70 && nextHealthPct <= 70.0f) ||
                    (strafingEventCount < STRAF_30 && nextHealthPct <= 30.0f))
                {
                    DoStrafingEvent();
                    ++strafingEventCount;
                }
            }
            // Must not die during Strafing
            else if (damage >= me->GetHealth())
                damage = 0;
        }

        uint8 SelectNextStartPoint()
        {
            // In videos, Gadok follow the opposite direction of clockwise, to confirm
            switch (move)
            {
                case MOV_NORTH_SOUTH:
                    return MOV_WEST_EAST;
                case MOV_SOUTH_NORTH:
                    return MOV_EAST_WEST;
                case MOV_WEST_EAST:
                    return MOV_NORTH_SOUTH;
                case MOV_EAST_WEST:
                    return MOV_SOUTH_NORTH;
            }

            return MOV_NORTH_SOUTH;
        }

        void DoStrafingEvent()
        {
            if (Vehicle* vehicle = me->GetVehicleKit())
               if (!vehicle->HasEmptySeat(0))
                   me->CastSpell(me, SPELL_EJECT_PASSENGERS, TRIGGERED_FULL_MASK);

            switch (strafingEventProgress)
            {
                case 0: // Begin, Gadok is 70% or 30% health, he go to the first POINT_START
                    isStrafing = true;
                    me->SetReactState(REACT_PASSIVE);
                    events.CancelEventGroup(PHASE_MAIN);

                    move = urand(MOV_NORTH_SOUTH, MOV_EAST_WEST);

                    if (instance)
                        instance->SetData(DATA_GADOK, move <= MOV_SOUTH_NORTH ? PHASE_NORTH_SOUTH : PHASE_WEST_EAST);

                    me->GetMotionMaster()->MovePoint(StrafPoints[move].pointIdBegin, StrafPoints[move].begin.GetPositionX(), StrafPoints[move].begin.GetPositionY(), StrafPoints[move].begin.GetPositionZ());

                    for (uint8 i = 0; i < 2; ++i)
                         if (Creature* striker = SummonKrikThik(NPC_KRIKTHIK_STRIKER))
                            if (striker->AI())
                                striker->AI()->DoAction(0);

                    ++strafingEventProgress;
                    strafingTimer = 0;
                    break;
                case 1: // We are a POINT_START, wait 2 sec then continue
                    ++strafingEventProgress;
                    strafingTimer = 2000;
                    break;
                case 2: // 2 sec passed, move to POINT_END with the spell
                    me->SetSpeed(MOVE_FLIGHT, 2.0f, true);
                    me->GetMotionMaster()->MovePoint(StrafPoints[move].pointIdEnd, StrafPoints[move].end.GetPositionX(), StrafPoints[move].end.GetPositionY(), StrafPoints[move].end.GetPositionZ());
                    me->CastSpell(me, SPELL_STRAFING_RUN, true);

                    ++strafingEventProgress;
                    break;
                case 3: // We are a POINT_END, wait 500 ms to let movement finish then continue
                    ++strafingEventProgress;
                    strafingTimer = 50;
                    break;
                case 4: // First strafing finished, we are at a POINT_END and go to POINT_OUTSIDE
                    if (instance)
                        instance->SetData(DATA_GADOK, PHASE_MAIN);

                    me->GetMotionMaster()->MovePoint(StrafPoints[move].pointIdOutside, StrafPoints[move].outside.GetPositionX(), StrafPoints[move].outside.GetPositionY(), StrafPoints[move].outside.GetPositionZ());
                    ++strafingEventProgress;
                    break;
                case 5: // We are a POINT_OUTSIDE, wait 500 ms to let movement finish then continue
                    ++strafingEventProgress;
                    strafingTimer = 50;
                    break;
                case 6: // We are POINT_OUTSIDE, go to the next POINT_OUTSIDE_START
                    move = SelectNextStartPoint();

                    me->GetMotionMaster()->MovePoint(StrafPoints[move].pointIdBeginOutside, StrafPoints[move].beginOutside.GetPositionX(), StrafPoints[move].beginOutside.GetPositionY(), StrafPoints[move].beginOutside.GetPositionZ());
                    ++strafingEventProgress;
                    break;
                case 7: // We are POINT_OUTSIDE_START, wait 500 ms to let movement finish then continue
                    ++strafingEventProgress;
                    strafingTimer = 50;
                    break;
                case 8: // We are at POINT_OUTSIDE_START, go to POINT_START

                    if (instance)
                        instance->SetData(DATA_GADOK, move <= MOV_SOUTH_NORTH ? PHASE_NORTH_SOUTH : PHASE_WEST_EAST);

                    me->GetMotionMaster()->MovePoint(StrafPoints[move].pointIdBegin, StrafPoints[move].begin.GetPositionX(), StrafPoints[move].begin.GetPositionY(), StrafPoints[move].begin.GetPositionZ());

                    ++strafingEventProgress;
                    break;
                case 9: // Just arrived to second POINT_START, wait 2 sec
                    ++strafingEventProgress;
                    strafingTimer = 50;
                    break;
                case 10: // 2 sec passed, move to POINT_END with the spell
                    me->SetSpeed(MOVE_FLIGHT, 2.0f, true);
                    me->GetMotionMaster()->MovePoint(StrafPoints[move].pointIdEnd, StrafPoints[move].end.GetPositionX(), StrafPoints[move].end.GetPositionY(), StrafPoints[move].end.GetPositionZ());
                    me->CastSpell(me, SPELL_STRAFING_RUN, true);

                    ++strafingEventProgress;
                    break;
                case 11: // We are a POINT_END, wait 500 ms to let movement finish then continue
                    ++strafingEventProgress;
                    strafingTimer = 50;
                    break;
                case 12: // POINT_END, End Strafing Event, go back to fight
                    if (me->GetVictim())
                        me->GetMotionMaster()->MoveChase(me->GetVictim());
                    else if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 15.0f, true))
                        AttackStart(target);

                    if (instance)
                        instance->SetData(DATA_GADOK, PHASE_MAIN);

                    move = 0;
                    me->SetReactState(REACT_AGGRESSIVE);
                    strafingTimer = 0;
                    strafingEventProgress = 0;
                    isStrafing = false;
                    me->SetSpeed(MOVE_FLIGHT, 1.134f, true);

                    events.ScheduleEvent(EVENT_PREY_TIME, urand(9, 15) * IN_MILLISECONDS, PHASE_MAIN);
                    events.ScheduleEvent(EVENT_IMPALING_STRIKE, urand(13, 19) * IN_MILLISECONDS, PHASE_MAIN);
                    break;
            }
        }

        void StopStrafingEvent()
        {
            me->GetMotionMaster()->Clear();
            move = 0;
            me->SetReactState(REACT_AGGRESSIVE);
            strafingTimer = 0;
            strafingEventProgress = 0;
            isStrafing = false;
            me->SetSpeed(MOVE_FLIGHT, 1.134f, true);
        }

        void JustSummoned(Creature* summoned)
        {
            summons.Summon(summoned);

            switch (summoned->GetEntry())
            {
                case NPC_KRIKTHIK_STRIKER:
                case NPC_KRIKTHIK_DISRUPTOR:
                    summoned->SetSpeed(MOVE_FLIGHT, 2);
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);
            nonAggroEvents.Update(diff);

            if (nonAggroEvents.ExecuteEvent() == EVENT_SUMMONS)
            {
                DoSpawnKrikThik();
            }

            if (!UpdateVictim())
                return;

            if (strafingTimer)
            {
                if (strafingTimer <= diff)
                {
                    strafingTimer = 0;
                    DoStrafingEvent();
                }
                else strafingTimer -= diff;
            }

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_PREY_TIME:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 15.0f, true))
                            me->CastSpell(target, SPELL_PREY_TIME, false);
                        events.ScheduleEvent(EVENT_PREY_TIME, urand(9, 15) * IN_MILLISECONDS, PHASE_MAIN);
                        break;
                    case EVENT_IMPALING_STRIKE:
                        me->CastSpell((Unit*)NULL, SPELL_IMPALING_STRIKE, false);
                        events.ScheduleEvent(EVENT_IMPALING_STRIKE, urand(13, 19) * IN_MILLISECONDS, PHASE_MAIN);
                        break;
                }
            }

            if (Vehicle* vehicle = me->GetVehicleKit())
               if (vehicle->HasEmptySeat(0))
                   DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/)
        {
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetBossState(DATA_GADOK, DONE);
                instance->SaveToDB();

                if (GameObject* portal = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_PORTALS)))
                    portal->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            }

            Talk(SAY_DEATH);
            Talk(SAY_STIKE_EMOTE);         
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_striker_gadokAI(creature);
    }
};

// Base struct for circle movements
struct npc_krikthik : public ScriptedAI
{
    npc_krikthik(Creature* creature) : ScriptedAI(creature) {}

    uint32 nextMovementTimer;
    uint32 updateCheckTimer;
    float actualAngle;
    float myPositionZ;
    bool direction;

    void Reset()
    {
        updateCheckTimer = 500;
        nextMovementTimer = 0;
        actualAngle = me->GetAngle(CenterPos.GetPositionX(), CenterPos.GetPositionY());
        direction = urand(0, 1);

        if (direction)
            myPositionZ = 435.0f;
        else
            myPositionZ = 440.0f;

        // Enable Movements
        MovementInform(POINT_MOTION_TYPE, POINT_KRIKTHIK_CIRCLE);

        me->setActive(true);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == POINT_KRIKTHIK_CIRCLE)
            nextMovementTimer = 50;
    }

    void SelectNextWaypoint(float& x, float& y)
    {
        if (direction)
            actualAngle -= M_PI / 8;
        else
            actualAngle += M_PI / 8;

        x = CenterPos.GetPositionX() + (me->GetObjectSize() + RADIUS_CIRCLE) * std::cos(actualAngle);
        y = CenterPos.GetPositionY() + (me->GetObjectSize() + RADIUS_CIRCLE) * std::sin(actualAngle);
    }

    void UpdateAI(const uint32 diff)
    {
        if (updateCheckTimer <= diff)
        {
            Map::PlayerList const& lPlayers = me->GetMap()->GetPlayers();

            if (lPlayers.isEmpty())
            {
                me->DespawnOrUnsummon();
                return;
            }

            updateCheckTimer = 5000;
        }
        else
            updateCheckTimer -= diff;

        if (nextMovementTimer)
        {
            if (nextMovementTimer <= diff)
            {
                nextMovementTimer = 0;

                float x = 0.0f;
                float y = 0.0f;
                SelectNextWaypoint(x, y);

                me->GetMotionMaster()->MovePoint(POINT_KRIKTHIK_CIRCLE, x, y, myPositionZ);
            }
            else nextMovementTimer -= diff;
        }
    }
};

class npc_krikthik_striker : public CreatureScript
{
public:
    npc_krikthik_striker() : CreatureScript("npc_krikthik_striker") { }

    struct npc_krikthik_strikerAI : public npc_krikthik
    {
        npc_krikthik_strikerAI(Creature* creature) : npc_krikthik(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        bool isAttackerStriker;

        void Reset()
        {
            npc_krikthik::Reset();
            isAttackerStriker = false;
        }

        void DoAction(const int32 action)
        {
            isAttackerStriker = true;

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            Map::PlayerList const &PlayerList = pInstance->instance->GetPlayers();
            Map::PlayerList::const_iterator it = PlayerList.begin();
            // Randomize it, everything is done in the "for"
            for (uint8 i = 0; i < urand(0, PlayerList.getSize() - 1); ++i, ++it);

            if (Player* player = it->GetSource())
                AttackStart(player);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!isAttackerStriker)
                npc_krikthik::UpdateAI(diff);
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_krikthik_strikerAI(creature);
    }
};

class npc_krikthik_disruptor : public CreatureScript
{
public:
    npc_krikthik_disruptor() : CreatureScript("npc_krikthik_disruptor") { }

    struct npc_krikthik_disruptorAI : public npc_krikthik
    {
        npc_krikthik_disruptorAI(Creature* creature) : npc_krikthik(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        EventMap events;

        void Reset()
        {
            npc_krikthik::Reset();
            events.ScheduleEvent(EVENT_DISRUPTOR_BOMBARD, urand(5000, 20000));
        }

        void UpdateAI(const uint32 diff)
        {
            npc_krikthik::UpdateAI(diff);
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_DISRUPTOR_BOMBARD)
            {
                if (pInstance->GetBossState(DATA_GADOK) == IN_PROGRESS)
                {
                    std::list<Player*> playerList;
                    GetPlayerListInGrid(playerList, me, 200.0f); 
                    if (!playerList.empty())
                    {
                        if (Player* player = Trinity::Containers::SelectRandomContainerElement(playerList))
                            me->CastSpell(player, SPELL_BOMB, true);
                    }
                }

                events.ScheduleEvent(EVENT_DISRUPTOR_BOMBARD, urand(5000, 20000));
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_krikthik_disruptorAI(creature);
    }
};

class npc_flak_cannon : public CreatureScript
{
public:
    npc_flak_cannon() : CreatureScript("npc_flak_cannon") { }

    struct npc_flak_cannonAI : public ScriptedAI
    {
        npc_flak_cannonAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;

        void Reset() {}

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
        {
            if (!pInstance)
                return;

            if (pInstance->GetBossState(DATA_GADOK) != DONE)
                return;

            if (spell->Id == 116554) // Fire Flak Cannon
            {
                for (uint8 i = 0; i < 5; ++i)
                {
                    if (Creature* bombarder = pInstance->instance->GetCreature(pInstance->GetData64(DATA_RANDOM_BOMBARDER)))
                    {
                        me->CastSpell(bombarder, 116553, true);
                        bombarder->GetMotionMaster()->MoveFall();
                        bombarder->DespawnOrUnsummon(2000);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_flak_cannonAI(creature);
    }
};

class go_setting_sun_temp_portal : public GameObjectScript
{
public:
    go_setting_sun_temp_portal() : GameObjectScript("go_setting_sun_temp_portal") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (go->GetPositionZ() < 400.0f)
        {
            player->NearTeleportTo(1194.993f, 2304.397f, 431.206f, go->GetOrientation());
            player->CastSpell(player, SPELL_TELEPORT_VISUAL, false);
        }
        else
        {
            player->NearTeleportTo(go->GetPositionX(), go->GetPositionY(), 388.5f, go->GetOrientation());
            player->CastSpell(player, SPELL_TELEPORT_VISUAL, false);
        }

        return false;
    }
};

class spell_gadok_strafing : public SpellScriptLoader
{
public:
    spell_gadok_strafing() : SpellScriptLoader("spell_gadok_strafing") { }

    class spell_gadok_strafing_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gadok_strafing_SpellScript);

        void HandleBeforeCast()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    uint8 actualStrafPhase = instance->GetData(DATA_GADOK);

                    if (actualStrafPhase == PHASE_MAIN)
                        return;

                    uint32 stalkerEntry = actualStrafPhase == PHASE_NORTH_SOUTH ? NPC_STALKER_NORTH_SOUTH : NPC_STALKER_WEST_EAST;

                    std::list<Creature*> stalkerList;
                    GetCreatureListWithEntryInGrid(stalkerList, caster, stalkerEntry, 10.0f);

                    for (auto itr : stalkerList)
                    if (!itr->HasAura(SPELL_STRAFIND_RUN_DMG))
                        itr->CastSpell(itr, SPELL_STRAFIND_RUN_DMG, true);
                }
            }
        }

        void Register()
        {
            BeforeCast += SpellCastFn(spell_gadok_strafing_SpellScript::HandleBeforeCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_gadok_strafing_SpellScript();
    }
};

class spell_prey_time : public SpellScriptLoader
{
public:
    spell_prey_time() : SpellScriptLoader("spell_prey_time") { }

    class spell_prey_time_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_prey_time_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();

            if (!caster || !target)
                return;

            target->CastSpell(caster, GetSpellInfo()->Effects[1].BasePoints, false);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_prey_time_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_prey_time_SpellScript();
    }
};

class PreyTimeTargetSelector
{
public:
    PreyTimeTargetSelector() { }

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (cre->GetEntry() == NPC_GADOK)
                return false;

        return true;
    }
};

class spell_prey_time_vehicle_aura : public SpellScriptLoader
{
public:
    spell_prey_time_vehicle_aura() : SpellScriptLoader("spell_prey_time_vehicle_aura") { }

    class spell_prey_time_vehicle_aura_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_prey_time_vehicle_aura_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (Unit* caster = GetCaster())
                targets.remove_if(PreyTimeTargetSelector());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_prey_time_vehicle_aura_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_prey_time_vehicle_aura_SpellScript();
    }
};

void AddSC_boss_striker_gadok()
{
    new boss_striker_gadok();
    new npc_krikthik_striker();
    new npc_krikthik_disruptor();
    new npc_flak_cannon();
    new go_setting_sun_temp_portal();
    new spell_gadok_strafing();
    new spell_prey_time();
    new spell_prey_time_vehicle_aura();
}
