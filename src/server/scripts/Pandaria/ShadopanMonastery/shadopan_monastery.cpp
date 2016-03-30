/*
* Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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
#include "shadopan_monastery.h"

class npc_shadopan_ambusher : public CreatureScript
{
public:
    npc_shadopan_ambusher() : CreatureScript("npc_shadopan_ambusher") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadopan_ambusherAI(creature);
    }

    enum eSpells
    {
        SPELL_SHA_CORRUPTION   = 124337,
        SPELL_STEALTH_COSMETIC = 91194,
        SPELL_CRISE            = 128245,
        SPELL_FLIP_OUT_AURA    = 128248,
        SPELL_STEALTH          = 102921,
        SPELL_SHADOWSTEP       = 128766 
    };

    enum eEvents
    {
        EVENT_CRISE      = 1,
        EVENT_SHADOWSTEP = 2
    };

    struct npc_shadopan_ambusherAI : public ScriptedAI
    {
        npc_shadopan_ambusherAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
            me->CastSpell(me, SPELL_STEALTH, false);
            me->CastSpell(me, SPELL_STEALTH_COSMETIC, false);
            me->CastSpell(me, SPELL_SHA_CORRUPTION, false);
            me->RemoveAurasDueToSpell(SPELL_FLIP_OUT_AURA);
        }

        void EnterCombat(Unit* who)
        {
            me->RemoveAura(SPELL_STEALTH);
            events.ScheduleEvent(EVENT_CRISE, urand(6, 8) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SHADOWSTEP, 0.5 * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CRISE:
                        me->CastSpell(me, SPELL_CRISE, false);
                        events.ScheduleEvent(EVENT_CRISE, urand(10, 14) * IN_MILLISECONDS);
                        break;
                    case EVENT_SHADOWSTEP:
                        if (auto const target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                            me->CastSpell(target, SPELL_SHADOWSTEP, true);
                        break;
                }
            }

           DoMeleeAttackIfReady();
        }
    };
};

const float spawnAngles[5] = {M_PI, -M_PI / 2, M_PI / 2, -3 * M_PI / 4, 3 * M_PI / 4};

class npc_shadopan_warden: public CreatureScript
{
public:
    npc_shadopan_warden() : CreatureScript("npc_shadopan_warden") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadopan_wardenAI(creature);
    }

    enum eSpells
    {
        SPELL_INCAPACITATING_LEAP = 115517
    };

    enum eEvents
    {
        EVENT_INCAPACITATING_LEAP = 1
    };

    enum eTalks
    {
        TALK_AGGRO = 0
    };

    struct npc_shadopan_wardenAI : public ScriptedAI
    {
        npc_shadopan_wardenAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        bool aggroTalked;
        uint8 aggroTalk;

        void InitializeAI()
        {
            if (me->IsAlive())
            {
                for(int i = 0; i < 5; ++i)
                {
                    float x, y, z;
                    z = me->GetPositionZ();
                    me->GetNearPoint2D(x, y, 3.0f, spawnAngles[i]);
                    me->SummonCreature(NPC_SHADO_PAN_AMBUSHER, x, y, z, me->GetAngle(x, y), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 604800);
                }
            }

            aggroTalked = false;

            Reset();
        }

        void Reset()
        {
            events.Reset();
        }
 
        void EnterCombat(Unit* who)
        {
            if (!aggroTalked)
            {
                aggroTalked = true;
                Talk(TALK_AGGRO);
            }

            events.ScheduleEvent(EVENT_INCAPACITATING_LEAP, urand(10, 14) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_INCAPACITATING_LEAP)
            {
                if (auto const target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, SPELL_INCAPACITATING_LEAP, false);
                events.ScheduleEvent(EVENT_INCAPACITATING_LEAP, urand(10, 14) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_shadopan_stormbringer : public CreatureScript
{
public:
    npc_shadopan_stormbringer() : CreatureScript("npc_shadopan_stormbringer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadopan_stormbringerAI(creature);
    }

    enum eSpells
    {
        SPELL_FOCUSING_ENERGY = 115009,
        SPELL_FOCUSING_COMBAT = 115433,
        SPELL_ARC_LIGHTNING   = 115026 
    };

    enum eEvents
    {
        EVENT_FOCUSING_COMBAT = 1
    };

    struct npc_shadopan_stormbringerAI : public ScriptedAI
    {
        npc_shadopan_stormbringerAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            SetCombatMovement(false);

            Reset();
        }

        void Reset()
        {
            events.Reset();
            if (auto const energy = me->FindNearestCreature(NPC_UNSTABLE_ENERGY, 10.0f))
                if (energy->HasAura(SPELL_FOCUSING_COMBAT))
                    energy->RemoveAura(SPELL_FOCUSING_COMBAT);
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_FOCUSING_COMBAT, urand(4, 8) * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            if (auto const energy = me->FindNearestCreature(NPC_UNSTABLE_ENERGY, 10.0f))
                if (energy->IsAIEnabled)
                    energy->AI()->DoAction(0); // ACTION_STORMBRINGER_KILLED
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_FOCUSING_COMBAT)
            {
                if (auto const sTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                    target->CastSpell(sTarget, SPELL_ARC_LIGHTNING, false);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!me->HasUnitState(UNIT_STATE_CASTING))
            {
                if (auto const energy = me->FindNearestCreature(NPC_UNSTABLE_ENERGY, 10.0f))
                    me->CastSpell(energy, SPELL_FOCUSING_ENERGY, false);
            }        

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_FOCUSING_COMBAT)
            {
                if (auto const energy = me->FindNearestCreature(NPC_UNSTABLE_ENERGY, 10.0f))
                    me->CastSpell(energy, SPELL_FOCUSING_COMBAT, true);
                events.ScheduleEvent(EVENT_FOCUSING_COMBAT, urand(6, 14) * IN_MILLISECONDS);
            }
        }
    };
};

class npc_shadopan_disciple : public CreatureScript
{
public:
    npc_shadopan_disciple() : CreatureScript("npc_shadopan_disciple") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadopan_discipleAI(creature);
    }

    enum eSpells
    {
        SPELL_PALM_STRIKE       = 106421,
        SPELL_ROUNDHOUSE_STRIKE = 128304
    };

    enum eEvents
    {
        EVENT_PALM_STRIKE       = 1,
        EVENT_ROUNDHOUSE_STRIKE = 2
    };

    struct npc_shadopan_discipleAI : public ScriptedAI
    {
        npc_shadopan_discipleAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_PALM_STRIKE, urand(4, 8) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_ROUNDHOUSE_STRIKE, urand(6, 15) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_PALM_STRIKE:
                        me->CastSpell((Unit*)NULL, SPELL_PALM_STRIKE, false);
                        events.ScheduleEvent(EVENT_PALM_STRIKE, urand(16, 21) * IN_MILLISECONDS);
                        break;
                    case EVENT_ROUNDHOUSE_STRIKE:
                        me->CastSpell((Unit*)NULL, SPELL_ROUNDHOUSE_STRIKE, false);
                        events.ScheduleEvent(EVENT_ROUNDHOUSE_STRIKE, urand(13, 20) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_shadopan_unstable_energy : public CreatureScript
{
public:
    npc_shadopan_unstable_energy() : CreatureScript("npc_shadopan_unstable_energy") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadopan_unstable_energyAI(creature);
    }

    enum eActions
    {
        ACTION_STORMBRINGER_KILLED = 0
    };

    struct npc_shadopan_unstable_energyAI : public ScriptedAI
    {
        npc_shadopan_unstable_energyAI(Creature* creature) : ScriptedAI(creature) {}

        uint8 stormbringerKilled;

        void InitializeAI()
        {
            stormbringerKilled = 0;
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_STORMBRINGER_KILLED)
            {
                if (++stormbringerKilled == 6)
                    me->DespawnOrUnsummon(2 * IN_MILLISECONDS);
            }
        }
    };
};

class npc_ethereal_sha : public CreatureScript
{
public:
    npc_ethereal_sha() : CreatureScript("npc_ethereal_sha") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ethereal_shaAI(creature);
    }

    enum eSpells
    {
        SPELL_SHA_BLAST              = 110117,
        SPELL_REACH_THROUGH_THE_VOID = 128339
    };

    enum eEvents
    {
        EVENT_SHA_BLAST              = 1,
        EVENT_REACH_THROUGH_THE_VOID = 2
    };

    struct npc_ethereal_shaAI : public ScriptedAI
    {
        npc_ethereal_shaAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_SHA_BLAST, urand(4, 8) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_REACH_THROUGH_THE_VOID, urand(11, 13) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SHA_BLAST:
                        me->CastSpell(me->GetVictim(), SPELL_SHA_BLAST, false);
                        events.ScheduleEvent(EVENT_SHA_BLAST, urand(12, 14) * IN_MILLISECONDS);
                        break;
                    case EVENT_REACH_THROUGH_THE_VOID:
                        me->CastSpell(me, SPELL_REACH_THROUGH_THE_VOID, false);
                        events.ScheduleEvent(EVENT_REACH_THROUGH_THE_VOID, urand(18, 23) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_spm_void_sha : public CreatureScript
{
public:
    npc_spm_void_sha() : CreatureScript("npc_spm_void_sha") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_spm_void_shaI(creature);
    }

    EventMap events;

    enum eSpells
    {
        SPELL_ON_SPAWN    = 128342,
        SPELL_VOID_ENERGY = 128343
    };

    enum eEvents
    {
        EVENT_VOID_ENERGY = 1
    };

    struct npc_spm_void_shaI : public ScriptedAI
    {
        npc_spm_void_shaI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI()
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->CastSpell(me, SPELL_ON_SPAWN, false);

            Reset();
        }

        void Reset()
        {
            DoZoneInCombat();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_VOID_ENERGY, 5 * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_VOID_ENERGY)
            {
                me->CastSpell((Unit*)NULL, SPELL_VOID_ENERGY, false);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_consuming_sha : public CreatureScript
{
public:
    npc_consuming_sha() : CreatureScript("npc_consuming_sha") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_consuming_shaAI(creature);
    }

    enum eSpells
    {
        SPELL_CONSUMING_BITE = 106925,
        SPELL_CONSUMPTION    = 106929
    };

    enum eEvents
    {
        EVENT_CONSUMING_BITE = 1,
        EVENT_CONSUMPTION    = 2
    };

    struct npc_consuming_shaAI : public ScriptedAI
    {
        npc_consuming_shaAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_CONSUMING_BITE, urand(4, 8) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_CONSUMPTION, urand(6, 10) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_CONSUMING_BITE:
                        me->CastSpell((Unit*)NULL, SPELL_CONSUMING_BITE, false);
                        events.ScheduleEvent(EVENT_CONSUMING_BITE, urand(10, 14) * IN_MILLISECONDS);
                        break;
                    case EVENT_CONSUMPTION:
                        me->CastSpell((Unit*)NULL, SPELL_CONSUMPTION, false);
                        events.ScheduleEvent(EVENT_CONSUMPTION, urand(15, 19) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_destroying_sha : public CreatureScript
{
public:
    npc_destroying_sha() : CreatureScript("npc_destroying_sha") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_destroying_shaAI(creature);
    }

    enum eSpells
    {
        SPELL_SHADOWS_OF_DESTRUCTION = 106942
    };

    enum eEvents
    {
        EVENT_SHADOWS_OF_DESTRUCTION = 1
    };

    struct npc_destroying_shaAI : public ScriptedAI
    {
        npc_destroying_shaAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_SHADOWS_OF_DESTRUCTION, urand(4, 8) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SHADOWS_OF_DESTRUCTION)
            {
                me->CastSpell((Unit*)NULL, SPELL_SHADOWS_OF_DESTRUCTION, false);
                events.ScheduleEvent(EVENT_SHADOWS_OF_DESTRUCTION, urand(16, 20) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_shadopan_novice : public CreatureScript
{
public:
    npc_shadopan_novice() : CreatureScript("npc_shadopan_novice") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadopan_noviceAI(creature);
    }

    struct npc_shadopan_noviceAI : public ScriptedAI
    {
        npc_shadopan_noviceAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            if (auto const training = me->FindNearestCreature(NPC_TRAINING_TARGET, 10.0f))
                AttackStart(training);
        }
    };
};

class npc_shadopan_archery : public CreatureScript
{
public:
    npc_shadopan_archery() : CreatureScript("npc_shadopan_archery") { }

    enum eEvents
    {
        EVENT_ICE_ARROW  = 1,
        EVENT_FIRE_ARROW = 2
    };

    enum eSpells
    {
        SPELL_ICE_ARROW = 126114,
        SPELL_FIRE_ARROW = 106992
    };

    struct npc_shadopan_archeryAI : public ScriptedAI
    {
        npc_shadopan_archeryAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        InstanceScript* instance;      

        void InitializeAI()
        {
            instance = me->GetInstanceScript();
            me->setActive(true);
            switch(me->GetEntry())
            {
                case NPC_ARCHERY_FIRST:
                    if (instance)
                        events.ScheduleEvent(EVENT_ICE_ARROW, 2 * IN_MILLISECONDS);
                    break;
                case NPC_ARCHERY_SECOND:
                    if (instance)
                        events.ScheduleEvent(EVENT_FIRE_ARROW, 2 * IN_MILLISECONDS);
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_ICE_ARROW:
                        me->UpdateObjectVisibility();
                        if (uint64 targetGUID = instance->GetData64(DATA_ARCHERY_TARGET))
                           if (Unit* target = ObjectAccessor::FindUnit(targetGUID))
                               me->CastSpell(target, SPELL_ICE_ARROW, false);
                        events.ScheduleEvent(EVENT_ICE_ARROW, urand(1, 2) * IN_MILLISECONDS);
                        break;
                    case EVENT_FIRE_ARROW:
                        me->CastSpell(me, SPELL_FIRE_ARROW, false);
                        events.ScheduleEvent(EVENT_FIRE_ARROW, urand(33, 36) * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadopan_archeryAI(creature);
    }
};

class npc_residual_hatred : public CreatureScript
{
public:
    npc_residual_hatred() : CreatureScript("npc_residual_hatred") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_residual_hatredAI(creature);
    }

    enum eSpells
    {
        SPELL_CURSE_OF_AGONY    = 112999,
        SPELL_RING_OF_MALICE    = 112932,
        SPELL_SHADOW_BOLT       = 112998
    };

    enum eEvents
    {
        EVENT_CURSE_OF_AGONY = 1,
        EVENT_RING_OF_MALICE = 2,
        EVENT_SHADOW_BOLT    = 3
    };

    struct npc_residual_hatredAI : public ScriptedAI
    {
        npc_residual_hatredAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_CURSE_OF_AGONY, urand(5, 10) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_RING_OF_MALICE, urand(6, 10) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SHADOW_BOLT, 1.3 * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_CURSE_OF_AGONY:
                        me->CastSpell((Unit*)NULL, SPELL_CURSE_OF_AGONY, false);
                        events.ScheduleEvent(EVENT_CURSE_OF_AGONY, urand(15, 20) * IN_MILLISECONDS);
                        break;
                    case EVENT_RING_OF_MALICE:
                        me->CastSpell(me, SPELL_RING_OF_MALICE, false);
                        events.ScheduleEvent(EVENT_RING_OF_MALICE, urand(21, 26) * IN_MILLISECONDS);
                        break;
                    case EVENT_SHADOW_BOLT:
                        me->CastSpell((Unit*)NULL, SPELL_SHADOW_BOLT, false);
                        events.ScheduleEvent(EVENT_SHADOW_BOLT, 1.3 * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };
};

class npc_vistige_of_hatred : public CreatureScript
{
public:
    npc_vistige_of_hatred() : CreatureScript("npc_vistige_of_hatred") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_vistige_of_hatredAI(creature);
    }

    enum eSpells
    {
        SPELL_BLACK_CLEAVE      = 113020,
        SPELL_DEATH_GRIP        = 113021,
        SPELL_TOUCH_OF_WEAKNESS = 113022
    };

    enum eEvents
    {
        EVENT_BLACK_CLEAVE      = 1,
        EVENT_TOUCH_OF_WEAKNESS = 2,
        EVENT_DEATH_GRIP        = 3
    };

    struct npc_vistige_of_hatredAI : public ScriptedAI
    {
        npc_vistige_of_hatredAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_BLACK_CLEAVE, 15 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_TOUCH_OF_WEAKNESS, urand(20, 25) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_DEATH_GRIP, urand(7, 10) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_BLACK_CLEAVE:
                        me->CastSpell((Unit*)NULL, SPELL_BLACK_CLEAVE, false);
                        events.ScheduleEvent(EVENT_BLACK_CLEAVE, 15 * IN_MILLISECONDS);
                        break;
                    case EVENT_TOUCH_OF_WEAKNESS:
                        me->CastSpell((Unit*)NULL, SPELL_TOUCH_OF_WEAKNESS, false);
                        events.ScheduleEvent(EVENT_TOUCH_OF_WEAKNESS, urand(20, 25) * IN_MILLISECONDS);
                        break;
                    case EVENT_DEATH_GRIP:
                        if (me->GetVictim() && !me->IsWithinMeleeRange(me->GetVictim()))
                            me->CastSpell(me->GetVictim(), SPELL_DEATH_GRIP, false);
                        events.ScheduleEvent(EVENT_DEATH_GRIP, urand(7, 10) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_fragment_of_hatred : public CreatureScript
{
public:
    npc_fragment_of_hatred() : CreatureScript("npc_fragment_of_hatred") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fragment_of_hatredAI(creature);
    }

    enum eSpells
    {
        SPELL_SINISTER_STRIKE  = 112931
    };

    enum eEvents
    {
        EVENT_SINISTER_STRIKE = 1
    };

    struct npc_fragment_of_hatredAI : public ScriptedAI
    {
        npc_fragment_of_hatredAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_SINISTER_STRIKE, urand(3, 10) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SINISTER_STRIKE)
            {
                me->CastSpell((Unit*)NULL, SPELL_SINISTER_STRIKE, false);
                events.ScheduleEvent(EVENT_SINISTER_STRIKE, urand(3, 10) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_hateful_essence : public CreatureScript
{
public:
    npc_hateful_essence() : CreatureScript("npc_hateful_essence") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hateful_essenceAI(creature);
    }

    struct npc_hateful_essenceAI : public ScriptedAI
    {
        npc_hateful_essenceAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI()
        {
            me->SetReactState(REACT_PASSIVE);
        }
    };
};

class spell_shadopan_explosion : public SpellScriptLoader
{
public:
    spell_shadopan_explosion() : SpellScriptLoader("spell_shadopan_explosion") { }

    AuraScript* GetAuraScript() const
    {
        return new spell_shadopan_explosion_AuraScript();
    }

    enum eSpells
    {
        SPELL_EXPLOSION_DAMAGE = 106966
    };

    class spell_shadopan_explosion_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_shadopan_explosion_AuraScript);

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
               if (auto const caster = GetCaster())
                   caster->CastSpell(caster, SPELL_EXPLOSION_DAMAGE, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_shadopan_explosion_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };
};

class ApparitionsTargetSelector
{
public:
    bool operator()(WorldObject* object) const
    {
        if (object->ToCreature() && (object->GetEntry() == 58807 || object->GetEntry() == 58810 || object->GetEntry() == 58803))
            return false;

        return true;
    }
};

class spell_shadopan_apparitions : public SpellScriptLoader
{
public:
    spell_shadopan_apparitions() : SpellScriptLoader("spell_shadopan_apparitions") { }

    SpellScript* GetSpellScript() const override
    {
        return new spell_shadopan_apparitions_SpellScript();
    }

    AuraScript* GetAuraScript() const override
    {
        return new spell_shadopan_apparitions_AuraScript();
    }

    class spell_shadopan_apparitions_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shadopan_apparitions_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ApparitionsTargetSelector());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_shadopan_apparitions_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    class spell_shadopan_apparitions_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_shadopan_apparitions_AuraScript);

        void CalculateAmount(AuraEffect const * auraEffect, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (auto const owner = GetOwner()->ToCreature())
                amount = owner->GetMaxHealth();
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_shadopan_apparitions_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        }
    };
};

class spell_purification_ritual : public SpellScriptLoader
{
public:
    spell_purification_ritual() : SpellScriptLoader("spell_purification_ritual") { }

    AuraScript* GetAuraScript() const override
    {
        return new spell_purification_ritual_AuraScript();
    }

    enum eSpells
    {
        SPELL_APPARITIONS = 111698
    };

    class spell_purification_ritual_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_purification_ritual_AuraScript);

        void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            auto const owner = GetOwner()->ToCreature();

            if (owner)
            {
                owner->RemoveAura(SPELL_APPARITIONS);
                owner->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_purification_ritual_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };
};

struct PlayerTargetSelector
{
    bool operator ()(WorldObject const *object) const
    {
        if (object->GetTypeId() == TYPEID_PLAYER)
            return false;
        return true;
    }
};

class spell_spm_fire_arrow : public SpellScriptLoader
{
public:
    spell_spm_fire_arrow() : SpellScriptLoader("spell_spm_fire_arrow") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_spm_fire_arrow_SpellScript();
    }

    class spell_spm_fire_arrow_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_spm_fire_arrow_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(PlayerTargetSelector());
            if (!targets.empty() && targets.size() > 1)
                targets.resize(1);
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            auto const caster = GetCaster();
            auto const target = GetHitUnit();
            if (!caster || !target)
                return;

            caster->UpdateObjectVisibility();
            caster->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_spm_fire_arrow_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_spm_fire_arrow_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };
};

class spell_spm_flip_out : public SpellScriptLoader
{
public:
    spell_spm_flip_out() : SpellScriptLoader("spell_spm_flip_out") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_spm_flip_out_SpellScript();
    }

    class spell_spm_flip_out_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_spm_flip_out_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(PlayerTargetSelector());
            if (!targets.empty() && targets.size() > 1)
                Trinity::Containers::SelectRandomContainerElement(targets);
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            auto const caster = GetCaster();
            auto const target = GetHitUnit();
            if (!caster || !target)
                return;

            caster->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_spm_flip_out_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_spm_flip_out_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };
};

class spell_spm_shadows_of_destruction : public SpellScriptLoader
{
public:
    spell_spm_shadows_of_destruction() : SpellScriptLoader("spell_spm_shadows_of_destruction") { }

    SpellScript* GetSpellScript() const override
    {
        return new spell_spm_shadows_of_destruction_SpellScript();
    }

    AuraScript* GetAuraScript() const override
    {
        return new spell_spm_shadows_of_destruction_AuraScript();
    }

    class spell_spm_shadows_of_destruction_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_spm_shadows_of_destruction_SpellScript);

        void HandleBeforeCast()
        {
            auto const caster = GetCaster();
            if (!caster)
                return;

            if (caster->ToCreature())
                caster->ToCreature()->SetReactState(REACT_PASSIVE);
        }

        void Register()
        {
            BeforeCast += SpellCastFn(spell_spm_shadows_of_destruction_SpellScript::HandleBeforeCast);
        }
    };

    class spell_spm_shadows_of_destruction_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_spm_shadows_of_destruction_AuraScript);

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            auto const owner = GetOwner()->ToCreature();
            if (!owner)
                return;

            owner->SetReactState(REACT_AGGRESSIVE);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_spm_shadows_of_destruction_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_MOD_PACIFY, AURA_EFFECT_HANDLE_REAL);
        }
    };
};

class areatrigger_at_shadopan_archery : public AreaTriggerScript
{
public:
    areatrigger_at_shadopan_archery() : AreaTriggerScript("areatrigger_at_shadopan_archery") {}

    enum areaTrigger
    {
        AREATRIGGER_ARCHERY_FIRST_BEGIN  = 8271,
        AREATRIGGER_ARCHERY_FIRST_END    = 8272,
        AREATRIGGER_ARCHERY_SECOND_FIRST = 7121,
        AREATRIGGER_ARCHERY_SECOND_END   = 7126
    };

    bool OnTrigger(Player* player, AreaTriggerEntry const* trigger)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            switch(trigger->id)
            {
                case AREATRIGGER_ARCHERY_FIRST_BEGIN:
                    if (instance->GetData(DATA_ARCHERY) != IN_PROGRESS && instance->GetData(DATA_ARCHERY) != DONE)
                        instance->SetData(DATA_ARCHERY, IN_PROGRESS);
                    break;
                case AREATRIGGER_ARCHERY_FIRST_END:
                    if (instance->GetData(DATA_ARCHERY) != DONE)
                        instance->SetData(DATA_ARCHERY, DONE);
                    break;
                case AREATRIGGER_ARCHERY_SECOND_FIRST:
                    if (instance->GetData(DATA_FIRE_ARCHERY) != IN_PROGRESS && instance->GetData(DATA_FIRE_ARCHERY) != DONE)
                        instance->SetData(DATA_FIRE_ARCHERY, IN_PROGRESS);
                    break;
                case AREATRIGGER_ARCHERY_SECOND_END:
                    if (instance->GetData(DATA_FIRE_ARCHERY) != DONE)
                        instance->SetData(DATA_FIRE_ARCHERY, DONE);
                    break;
            }
        }

        return false;
    }
};

class spell_spm_death_grip : public SpellScriptLoader
{
public:
    spell_spm_death_grip() : SpellScriptLoader("spell_spm_death_grip") {}

    SpellScript* GetSpellScript() const
    {
        return new spell_spm_death_grip_SpellScript();
    }

    class spell_spm_death_grip_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_spm_death_grip_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            if (auto const caster = GetCaster())
                if (auto const target = GetHitUnit())
                     target->CastSpell(caster, GetSpellInfo()->Effects[effIndex].BasePoints, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_spm_death_grip_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };
};

void AddSC_shadopan_monastery()
{
    new npc_shadopan_ambusher();
    new npc_shadopan_warden();
    new npc_shadopan_stormbringer();
    new npc_shadopan_disciple();
    new npc_shadopan_unstable_energy();
    new npc_ethereal_sha();
    new npc_spm_void_sha();
    new npc_consuming_sha();
    new npc_destroying_sha();
    new npc_shadopan_novice();
    new npc_shadopan_archery();
    new npc_residual_hatred();
    new npc_vistige_of_hatred();
    new npc_fragment_of_hatred();
    new npc_hateful_essence();
    new spell_shadopan_explosion();
    new spell_purification_ritual();
    new spell_shadopan_apparitions();
    new spell_spm_fire_arrow();
    new spell_spm_flip_out();
    new spell_spm_shadows_of_destruction();
    new spell_spm_death_grip();
    new areatrigger_at_shadopan_archery();
}
