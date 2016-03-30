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
#include "MoveSplineInit.h"
#include "GameObjectAI.h"
#include "temple_of_the_jade_serpent.h"

const float triggerAngles[8] = {0.0f, M_PI, -M_PI / 2, M_PI / 2, -3 * M_PI / 4, -M_PI / 4, 3 * M_PI / 4, M_PI / 4};

class boss_liu_flameheart : public CreatureScript
{
public:
    boss_liu_flameheart() : CreatureScript("boss_liu_flameheart") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_liu_flameheart_AI(creature);
    }

    enum eStatus
    {
        PHASE_1,
        PHASE_2,
        PHASE_3
    };

    enum eEvents
    {
        EVENT_SERPENT_STRIKE      = 1,
        EVENT_SERPENT_KICK        = 2,
        EVENT_SERPENT_WAVE        = 3,
        EVENT_JADE_SERPENT_STRIKE = 4,
        EVENT_JADE_SERPENT_KICK   = 5,
        EVENT_JADE_SERPENT_WAVE   = 6,
        EVENT_SUMMON_YULON        = 7,
        EVENT_AURA_JADE           = 8
    };

    enum eSpells
    {
        SPELL_DUST_VISUAL              = 110518,
        SPELL_SERPENT_STRIKE           = 106823,
        SPELL_SERPENT_KICK             = 106856,
        SPELL_JADE_SERPENT_STRIKE      = 106841,
        SPELL_JADE_SERPENT_KICK        = 106864,
        SPELL_SUMMON_JADE_SERPENT      = 106895,
        SPELL_JADE_SOUL                = 106909,
        SPELL_JADE_SERPENT_WAVE_VISUAL = 107002,
        SPELL_SERPENT_WAVE_VISUAL      = 106939,
        SPELL_SERPENT_WAVE_PERIODIC    = 106959
    };

    enum eTexts
    {
        TALK_AGGRO   = 0,
        TALK_DEATH   = 1,
        TALK_EVENT   = 2,
        TALK_EVENT_2 = 3,
        TALK_KILL    = 4,
        TALK_INTRO   = 5
    };

    enum eActions
    {
        ACTION_JADE_FIRE = 0
    };

    enum eCreatures
    {
        NPC_TRIGGER_WAVE = 56789,
    };

    struct boss_liu_flameheart_AI : public BossAI
    {
        boss_liu_flameheart_AI(Creature* creature) : BossAI(creature, DATA_LIU) {}

        bool inCombat;
        eStatus status;

        void InitializeAI()
        {
            me->setActive(true);
            Talk(TALK_INTRO);
            me->CastSpell(me, SPELL_POSSESSED_BY_SHA, false);           
            me->CastSpell(me, SPELL_DUST_VISUAL, false);     

            Reset();
        }

        void Reset()
        {
            inCombat = false;
            me->SetReactState(REACT_AGGRESSIVE);
            events.Reset();
            status = PHASE_1;     
        }

        void KilledUnit(Unit* unit)
        {
            if (unit->GetTypeId() == TYPEID_PLAYER)
                Talk(TALK_KILL);
        }

        void EnterCombat(Unit* unit)
        {
            if (!inCombat)
            {
                inCombat = true;
                
                _EnterCombat();
                Talk(TALK_AGGRO);
                events.ScheduleEvent(EVENT_SERPENT_STRIKE, 5000);
                events.ScheduleEvent(EVENT_SERPENT_KICK, 5000);

                if (instance)
                {
                    instance->SetData(DATA_LIU, NOT_STARTED);
                    instance->SetData(DATA_TWIN_DRAGONS_DOOR, NOT_STARTED);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                }
            }
        }

        void EnterEvadeMode()
        {
            summons.DespawnAll();
            me->RemoveAllAuras();
            me->ClearUnitState(UNIT_STATE_ROOT);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_LIU, FAIL);
                instance->SetBossState(DATA_LIU, FAIL);
                instance->SetData(DATA_TWIN_DRAGONS_DOOR, DONE);
            }
        }

        void JustDied(Unit* killer)
        {
            Talk(TALK_DEATH);
            if (instance)
            {
                instance->SetBossState(DATA_LIU, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_LIU, DONE);
                instance->SetData(DATA_TWIN_DRAGONS_DOOR, DONE);
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (me->GetHealthPct() <= 70.f && status == PHASE_1)
            {
                status = PHASE_2;
                Talk(TALK_EVENT);
                me->CastSpell(me, SPELL_JADE_ESSENCE, false);
                events.Reset();
                events.ScheduleEvent(EVENT_JADE_SERPENT_STRIKE, 5000);
                events.ScheduleEvent(EVENT_JADE_SERPENT_KICK, 10000);              
            }

            if (me->GetHealthPct() <= 30.f && status == PHASE_2)
            {
                status = PHASE_3;
                Talk(TALK_EVENT_2);
                events.Reset();
                events.ScheduleEvent(EVENT_SUMMON_YULON, 500);

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);             
            }
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
                    case EVENT_SERPENT_STRIKE:
                         {
                             me->CastSpell(me->GetVictim(), SPELL_SERPENT_STRIKE, false);
                             events.ScheduleEvent(EVENT_SERPENT_STRIKE, 10000);
                             events.ScheduleEvent(EVENT_SERPENT_WAVE, 4000);
                        }
                        break;
                    case EVENT_SERPENT_KICK:
                        me->CastSpell(me->GetVictim(), SPELL_SERPENT_KICK, false);
                        events.ScheduleEvent(EVENT_SERPENT_KICK, 10000);
                        break;
                    case EVENT_SERPENT_WAVE:
                         {
                             for(int i = 0; i < 4; ++i)
                             {
                                 float x, y, z;
                                 z = me->GetPositionZ();
                                 me->GetNearPoint2D(x, y, 0.1f, triggerAngles[i]);
                                 if (Creature* trigger = me->SummonCreature(NPC_TRIGGER_WAVE, x, y, z, me->GetAngle(x, y)))
                                 {
                                    trigger->SetReactState(REACT_PASSIVE);
                                    trigger->SetDisplayId(11686);
                                    trigger->CastSpell(trigger, SPELL_SERPENT_WAVE_VISUAL, false);
                                    trigger->CastSpell(trigger, SPELL_SERPENT_WAVE_PERIODIC, false);

                                    float dx, dy;
                                    GetPositionWithDistInOrientation(trigger, 100.0f, trigger->GetOrientation(), dx, dy);
                                    Movement::MoveSplineInit init(trigger);
                                    init.MoveTo(dx, dy, me->GetPositionZ());
                                    init.SetVelocity(11.5f);
                                    init.Launch();

                                    trigger->DespawnOrUnsummon(trigger->GetSplineDuration());
                                 }
                              }
                        }
                        break;
                    case EVENT_JADE_SERPENT_STRIKE:
                         {
                             me->CastSpell((Unit*)NULL, SPELL_JADE_SERPENT_STRIKE, false);
                             std::list<Player*> playerList;
                             GetPlayerListInGrid(playerList, me, 10.0f);
                             for(auto player : playerList)
                                 player->KnockbackFrom(me->GetPositionX(), me->GetPositionY(), 10, 10);

                              events.ScheduleEvent(EVENT_JADE_SERPENT_STRIKE, 10000);
                              events.ScheduleEvent(EVENT_JADE_SERPENT_WAVE, 4000);
                        }
                        break;
                    case EVENT_JADE_SERPENT_KICK:
                        me->CastSpell(me->GetVictim(), SPELL_JADE_SERPENT_KICK, false);
                        events.ScheduleEvent(EVENT_JADE_SERPENT_KICK, 10000);
                        break;
                    case EVENT_JADE_SERPENT_WAVE:
                         {
                             for(int i = 4; i < 8; ++i)
                             {
                                 float x, y, z;
                                 z = me->GetPositionZ();
                                 me->GetNearPoint2D(x, y, 0.1f, triggerAngles[i]);
                                 if (Creature* trigger = me->SummonCreature(NPC_TRIGGER_WAVE, x, y, z, me->GetAngle(x, y)))
                                 {
                                    trigger->SetReactState(REACT_PASSIVE);
                                    trigger->SetDisplayId(11686);
                                    trigger->CastSpell(trigger, SPELL_JADE_SERPENT_WAVE_VISUAL, false);
                                    trigger->CastSpell(trigger, SPELL_SERPENT_WAVE_PERIODIC, false);

                                    if (trigger->IsAIEnabled)
                                        trigger->AI()->DoAction(ACTION_JADE_FIRE);

                                    float dx, dy;
                                    GetPositionWithDistInOrientation(trigger, 100.0f, trigger->GetOrientation(), dx, dy);
                                    Movement::MoveSplineInit init(trigger);
                                    init.MoveTo(dx, dy, me->GetPositionZ());
                                    init.SetVelocity(11.5f);
                                    init.Launch();

                                    trigger->DespawnOrUnsummon(trigger->GetSplineDuration());
                                  }
                             }
                        }
                        break;
                    case EVENT_SUMMON_YULON:
                        me->CastSpell(me, SPELL_SUMMON_JADE_SERPENT, false);
                        me->CastSpell(me, SPELL_JADE_SOUL, false);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        me->SetReactState(REACT_PASSIVE);
                        me->AddUnitState(UNIT_STATE_ROOT);
                        events.ScheduleEvent(EVENT_AURA_JADE, 3000);
                        break;
                    case EVENT_AURA_JADE:
                        me->CastSpell(me, SPELL_JADE_SOUL, false);
                        events.ScheduleEvent(EVENT_AURA_JADE, 2500);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

const float YuLonLeaveWPPath[3][3] =
{
    {938.1285f, -2542.773f, 226.7539f},
    {965.3004f, -2489.177f, 256.0829f},
    {978.6389f, -2419.773f, 299.6773f}
};

class boss_yu_lon_tjs : public CreatureScript
{
public:
    boss_yu_lon_tjs() : CreatureScript("boss_yu_lon_tjs") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_yu_lon_tjs_AI(creature);
    }

    enum eEvents
    {
        EVENT_JADE_FIRE = 1,
        EVENT_BREATH    = 2,
        EVENT_LEAVE     = 3
    };

    enum eSpells
    {
        SPELL_CLEANSING_BREATH = 132387,
        SPELL_JADE_FIRE        = 107045
    };

    enum eTalks
    {
        TALK_LEAVE = 0
    };

    struct boss_yu_lon_tjs_AI : public ScriptedAI
    {
        boss_yu_lon_tjs_AI(Creature* creature) : ScriptedAI(creature) {}

        InstanceScript* instance;
        bool leaveEvent;
        EventMap events;

        void InitializeAI()
        {
            instance = me->GetInstanceScript();
            leaveEvent = false;
            me->SetHealth(me->ToUnit()->CountPctFromMaxHealth(30));
            me->SetReactState(REACT_AGGRESSIVE);
            events.ScheduleEvent(EVENT_JADE_FIRE, 0.1 * IN_MILLISECONDS);

            if (instance)
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (me->GetHealth() <= damage)
            {
                damage = 0;
                if (!leaveEvent)
                {
                    leaveEvent = true;

                    if (instance)
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                    me->SetReactState(REACT_PASSIVE);
                    me->CombatStop(true);
                    me->DeleteThreatList();
                    me->getThreatManager().resetAllAggro();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    events.Reset();

                    if (Creature* liu = Unit::GetCreature(*me, instance->GetData64(DATA_LIU)))
                    {
                        float x = liu->GetPositionX();
                        float y = liu->GetPositionY();
                        float z = liu->GetPositionZ();

                        GetPositionWithDistInOrientation(liu, 15.0f, liu->GetOrientation(), x, y);
                        Movement::MoveSplineInit init(me);
                        init.MoveTo(x, y, z);
                        init.SetVelocity(9.0f);
                        init.SetFly();
                        init.Launch();

                        events.ScheduleEvent(EVENT_BREATH, me->GetSplineDuration());
                    }
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_JADE_FIRE:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM), SPELL_JADE_FIRE, false);
                        events.ScheduleEvent(EVENT_JADE_FIRE, urand(4,6) * IN_MILLISECONDS);
                        break;
                    case EVENT_BREATH:
                        if (Creature* liu = Unit::GetCreature(*me, instance->GetData64(DATA_LIU)))
                        {
                            me->SetFacingToObject(liu);
                            liu->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        }

                        me->CastSpell((Unit*)NULL, SPELL_CLEANSING_BREATH, false);
                        events.ScheduleEvent(EVENT_LEAVE, 2 * IN_MILLISECONDS);
                        break;
                    case EVENT_LEAVE:
                         {
                             Talk(TALK_LEAVE);
                             me->setFaction(35);
                             Movement::MoveSplineInit init(me);
                             for(uint8 i = 0; i < 3; ++i)
                             {
                                 G3D::Vector3 path(YuLonLeaveWPPath[i][0], YuLonLeaveWPPath[i][1], YuLonLeaveWPPath[i][2]);
                                 init.Path().push_back(path);
                             }
                             init.SetSmooth();
                             init.SetFly();
                             init.SetVelocity(15.0f);
                             init.Launch();

                             me->DespawnOrUnsummon(me->GetSplineDuration());
                        }
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class mob_trigger_liu_flameheart : public CreatureScript
{
public:
    mob_trigger_liu_flameheart() : CreatureScript("mob_trigger_liu_flameheart") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_trigger_liu_flameheart_AI(creature);
    }

    enum eEvents
    {
        EVENT_JADE_FIRE_SUMMON = 1
    };

    enum eSpells
    {
        SPELL_JADE_FIRE_SUMMON = 107103
    };

    enum eActions
    {
        ACTION_JADE_FIRE = 0
    };

    struct mob_trigger_liu_flameheart_AI : public ScriptedAI
    {
        mob_trigger_liu_flameheart_AI(Creature* creature) : ScriptedAI(creature) {}

        void DoAction(const int32 action)
        {
            if (action == ACTION_JADE_FIRE)
                events.ScheduleEvent(EVENT_JADE_FIRE_SUMMON, 0.5 * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_JADE_FIRE_SUMMON)
            {
                me->CastSpell(me, SPELL_JADE_FIRE_SUMMON, false);
                events.ScheduleEvent(EVENT_JADE_FIRE_SUMMON, 1 * IN_MILLISECONDS);
            }
        }
    };
};

class mob_minion_of_doubt : public CreatureScript
{
public:
    mob_minion_of_doubt() : CreatureScript("mob_minion_of_doubt") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_minion_of_doubt_AI(creature);
    }

    enum eEvents
    {
        EVENT_SHADOW_OF_DOUBT   = 1,
        EVENT_SHATTERED_RESOLVE = 2
    };

    enum eSpells
    {
        SPELL_SHADOW_OF_DOUBT   = 110099,
        SPELL_SHATTERED_RESOLVE = 110125
    };

    struct mob_minion_of_doubt_AI : public ScriptedAI
    {
        mob_minion_of_doubt_AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void EnterCombat(Unit* unit)
        {
            events.ScheduleEvent(EVENT_SHADOW_OF_DOUBT, 2 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SHATTERED_RESOLVE, 4 * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->ToCreature()->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(who, 15.0f) &&
                (me->IsInCombat() && me->GetVictim() && me->GetVictim()->GetTypeId() != TYPEID_PLAYER) ||
                !me->IsInCombat())
               {
                   me->getThreatManager().resetAllAggro();
                   me->getThreatManager().addThreat(who, 100.0f);
                   AttackStart(who);
               }
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
                    case EVENT_SHADOW_OF_DOUBT:
                        me->CastSpell(me->GetVictim(), SPELL_SHADOW_OF_DOUBT, false);
                        events.ScheduleEvent(EVENT_SHADOW_OF_DOUBT, urand(4,8) * IN_MILLISECONDS);
                        break;
                    case EVENT_SHATTERED_RESOLVE:
                        me->CastSpell(me->GetVictim(), SPELL_SHATTERED_RESOLVE, false);
                        events.ScheduleEvent(EVENT_SHATTERED_RESOLVE, urand(4,8) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class mob_lesser_sha : public CreatureScript
{
public:
    mob_lesser_sha() : CreatureScript("mob_lesser_sha") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_lesser_sha_AI(creature);
    }

    enum eEvents
    {
        EVENT_SHA_SCREECH = 1
    };

    enum eSpells
    {
        SPELL_SHA_SCREECH = 122527
    };

    struct mob_lesser_sha_AI : public ScriptedAI
    {
        mob_lesser_sha_AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void EnterCombat(Unit* unit)
        {
            events.ScheduleEvent(EVENT_SHA_SCREECH, 2 * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->ToCreature()->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(who, 15.0f) && 
                (me->IsInCombat() && me->GetVictim() && me->GetVictim()->GetTypeId() != TYPEID_PLAYER) ||
                !me->IsInCombat())
            {
                me->getThreatManager().resetAllAggro();
                me->getThreatManager().addThreat(who, 100.0f);
                AttackStart(who);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SHA_SCREECH)
            {
                me->CastSpell(me->GetVictim(), SPELL_SHA_SCREECH, false);
                events.ScheduleEvent(EVENT_SHA_SCREECH, urand(4,7) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

const float TrashLeaveWPPath[2][3] =
{
    {937.553f, -2536.750f, 179.821f},
    {955.057f, -2483.663f, 180.499f}
};

class mob_lie_flameheart_trash_base_ai : public CreatureScript
{
public:
    mob_lie_flameheart_trash_base_ai() : CreatureScript("mob_lie_flameheart_trash_base_ai") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_lie_flameheart_trash_base_ai_CreatureScript(creature);
    }

    enum eTalks
    {
        TALK_MOVE_AWAY = 0
    };

    enum eEvents
    {
        EVENT_LEAVE = 1
    };

    struct mob_lie_flameheart_trash_base_ai_CreatureScript : public ScriptedAI
    {
        mob_lie_flameheart_trash_base_ai_CreatureScript(Creature* creature) : ScriptedAI(creature) {}

        bool talked;

        void InitializeAI()
        {
            talked = false;
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetInCombatState(false);
        }

        bool CanAIAttack(Unit const* who) const override
        {
            if (who->GetEntry() == 56732 || who->GetEntry() == 56762)
                return false;

            return true;
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->ToCreature()->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
        }

        void EnterEvadeMode()
        {
            if (!talked)
            {
                talked = true;
                Talk(TALK_MOVE_AWAY);
                me->SetReactState(REACT_PASSIVE);
                Movement::MoveSplineInit init(me);
                for(uint8 i = 0; i < 2; ++i)
                {
                    G3D::Vector3 path(TrashLeaveWPPath[i][0], TrashLeaveWPPath[i][1], TrashLeaveWPPath[i][2]);
                    init.Path().push_back(path);
                }
                init.SetSmooth();
                init.SetVelocity(7.0f);
                init.SetUncompressed();
                init.Launch();

                me->DespawnOrUnsummon(me->GetSplineDuration());
            }
        }
    };
};

class spell_liu_flameheart_cleansing_breath : public SpellScriptLoader
{
public:
    spell_liu_flameheart_cleansing_breath() : SpellScriptLoader("spell_liu_flameheart_cleansing_breath") { }

    enum eSpells
    {
        SPELL_QUIET_SUICIDE = 115372
    };

    enum eTalks
    {
        TALK_DEATH = 9
    };

    class spell_liu_flameheart_cleansing_breath_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_liu_flameheart_cleansing_breath_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Creature* target = GetHitCreature();

            if (!target)
                return;

            if (target->IsAIEnabled)
                target->AI()->Talk(TALK_DEATH);

            if (Player* originalCaster = target->GetLootRecipient())
                target->CastSpell(target, SPELL_QUIET_SUICIDE, false, NULL, NULL, originalCaster->GetGUID());
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_liu_flameheart_cleansing_breath_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_liu_flameheart_cleansing_breath_SpellScript();
    }
};

class go_tjs_twin_dragons_door : public GameObjectScript
{
public:
    GameObjectAI* GetAI(GameObject* go) const
    {
        return new go_tjs_twin_dragons_doorAI(go);
    }

    go_tjs_twin_dragons_door() : GameObjectScript("go_tjs_twin_dragons_door") { }

    struct go_tjs_twin_dragons_doorAI : public GameObjectAI
    {
        go_tjs_twin_dragons_doorAI(GameObject* go) : GameObjectAI(go)
        {
            go->setActive(true);

            if (go->GetInstanceScript())
            {
                if (go->GetInstanceScript()->GetData(DATA_TWIN_DRAGONS_DOOR) == DONE)
                    go->SetGoState(GO_STATE_ACTIVE);

                if (go->GetInstanceScript()->GetData(DATA_LIU_STATE) == DONE && go->GetInstanceScript()->GetBossState(DATA_LIU) != DONE)
                    go->GetInstanceScript()->instance->SummonCreature(NPC_LIU_FLAMEHEART, LiuSpawnPosition);
            }
        }
    };
};

void AddSC_boss_liu_flameheat()
{
    new boss_liu_flameheart();
    new boss_yu_lon_tjs();
    new mob_trigger_liu_flameheart();
    new spell_liu_flameheart_cleansing_breath();
    //Trashes
    new mob_minion_of_doubt();
    new mob_lesser_sha();
    new mob_lie_flameheart_trash_base_ai();
    new go_tjs_twin_dragons_door();
}