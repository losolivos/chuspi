/*
 * Copyright (C) 2012-2013 Trinity <http://www.pandashan.com/>
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

#include "mogu_shan_vault.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "CreatureAI.h"
#include "MoveSplineInit.h"
#include "SpellScript.h"

enum eSpells
{
    SPELL_SPIRIT_BOLT = 121224,
    SPELL_GROUND_SLAM = 121087,
    SPELL_PETRIFICATION = 125090,
    SPELL_PETRIFIED = 125092,
    SPELL_FULLY_PETRIFIED = 115877,
    SPELL_MONSTROUS_BITE = 125096,
    SPELL_SUNDERING_BITE = 116970,
    SPELL_PROTECTIVE_FENZY = 116982,
    SPELL_SHATTERING_STONE = 116977,
    SPELL_FOCUSED_ASSAULT = 116990,
    SPELL_POSE_1 = 120650,
    SPELL_POSE_2 = 120613,
    SPELL_STONE = 120663,
    SPELL_BRONZE = 120661,
    SPELL_GHOST_ESSENCE = 120764,
    SPELL_INACTIVE = 118205,
    SPELL_ACTIVATION_VISUAL = 118212,
    SPELL_CHARGED_SHADOWS = 117685,
    SPELL_SHADOW_BLAST = 119365,
    SPELL_ANNIHILATE = 119521,
    SPELL_COWARDICE = 119635,
    SPELL_VOLLEY = 119554,
    SPELL_TROLL_RUSH = 116606,
    SPELL_CRUSHING_ATTACKS = 119514,
    SPELL_SHOOT = 119524,
    SPELL_ENRAGE = 119629,
    SPELL_REFLECTIVE_SHIELDS = 119630,
    SPELL_FLESH_TO_STONE = 118552,
    SPELL_STONE_BLOCK = 118529,
    SPELL_FORCEFUL_SWING = 126955,
    SPELL_TOUCH_OF_NALAK_SHA = 126958,
    SPELL_WARDEN_S_FURY = 126970,
    SPELL_ENHANCED_RECONSTRUCTION = 126980,
    SPELL_RECONSTRUCTING = 126985
};

enum eEvents
{
    EVENT_CURSED_MOGU_SPIRIT_BOLT,
    EVENT_CURSED_MOGU_GROUND_SLAM,
    EVENT_ENORMOUS_QUILEN_BITE,
    EVENT_QUILEN_SUNDERING_BITE,
    EVENT_QUILEN_SHATTERING_STONE,
    EVENT_QUILEN_FOCUSED_ASSAULT,
    EVENT_ZANDALARI_TROLL_RUSH,
    EVENT_ZIAN_CHARGED_SHADOWS,
    EVENT_SORCERER_SHADOW_BLAST,
    EVENT_QIANG_ANNIHILATE,
    EVENT_QIANG_START_SECOND_FIGHT,
    EVENT_MOUNTED_MOGU_CRUSHING_ATTACKS,
    EVENT_SUBETAI_VOLLEY,
    EVENT_SUBETAI_START_THIRD_COMBAT,
    EVENT_MOGU_ARCHER_SHOOT,
    EVENT_MENG_START_FOURTH_COMBAT,
    EVENT_MENG_COWARDICE,
    EVENT_KINGS_GUARD_ENRAGE,
    EVENT_KINGS_GUARD_REFLECTIVE_SHIELDS,
    EVENT_SECRET_FLESH_TO_STONE,
    EVENT_SECRET_STONE_BLOCK,
    EVENT_WARDEN_FORCEFUL_SWING,
    EVENT_WARDEN_TOUCH_OF_NALAK_SHA,
    EVENT_WARDEN_WARDEN_S_FURY,
    EVENT_KEEPER_ENHANCED_RECONSTRUCTION,
    EVENT_KEEPER_RECONSTRUCTING
};

enum eTrashsActions
{
    ACTION_CURSED_MOGU_ATTACK_PLAYER = 1
};

class mob_cursed_mogu_sculpture : public CreatureScript
{
public:
    mob_cursed_mogu_sculpture() : CreatureScript("mob_cursed_mogu_sculture") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_cursed_mogu_sculptureAI(creature);
    }

    struct mob_cursed_mogu_sculptureAI : public ScriptedAI
    {
        mob_cursed_mogu_sculptureAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        EventMap events;
        bool activationDone;
        uint64 playerActivate;

        void Reset()
        {
            events.Reset();

            playerActivate = 0;
            activationDone = false;

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            me->AI()->SetCanSeeEvenInPassiveMode(true);
            me->RemoveAurasDueToSpell(SPELL_GHOST_ESSENCE);

            switch (me->GetEntry())
            {
                case NPC_CURSED_MOGU_SCULPTURE_1:
                    me->AddAura(SPELL_POSE_1, me);
                    me->AddAura(SPELL_STONE, me);
                    break;
                case NPC_CURSED_MOGU_SCULPTURE_2:
                    me->AddAura(SPELL_POSE_2, me);
                    me->AddAura(SPELL_BRONZE, me);
                    break;
            }
        }

        void EnterCombat(Unit* attacker)
        {
            events.Reset();
            events.ScheduleEvent(EVENT_CURSED_MOGU_SPIRIT_BOLT, 15000);
            events.ScheduleEvent(EVENT_CURSED_MOGU_GROUND_SLAM, 25000);

            me->AI()->AttackStart(attacker);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!pInstance)
                return;

            if (!who->IsWithinDist(me, 15.0f))
                return;

            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                switch (me->GetEntry())
                {
                    case NPC_CURSED_MOGU_SCULPTURE_2:
                    {
                        if (activationDone)
                            return;

                        if (Creature* ghostEssence = pInstance->instance->GetCreature(pInstance->GetData64(NPC_GHOST_ESSENCE)))
                            ghostEssence->CastSpell(ghostEssence, SPELL_GHOST_ESSENCE, false);
                        break;
                    }
                    case NPC_CURSED_MOGU_SCULPTURE_1:
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        me->RemoveAurasDueToSpell(SPELL_BRONZE);
                        me->RemoveAurasDueToSpell(SPELL_POSE_2);
                        me->RemoveAurasDueToSpell(SPELL_POSE_1);
                        me->RemoveAurasDueToSpell(SPELL_STONE);
                        DoAction(ACTION_CURSED_MOGU_ATTACK_PLAYER);
                        break;
                    }
                }

                playerActivate = who->GetGUID();
                activationDone = true;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (!GetClosestCreatureWithEntry(me, me->GetEntry(), 50.0f, true))
            {
                Creature* Feng = GetClosestCreatureWithEntry(me, NPC_FENG, 100.0f, true);
                if (Feng && Feng->AI())
                    Feng->AI()->Talk(7);
            }
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_CURSED_MOGU_ATTACK_PLAYER)
            {
                if (Player* plr = ObjectAccessor::FindPlayer(playerActivate))
                    me->AI()->AttackStart(plr);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 id = events.ExecuteEvent())
            {
                switch (id)
                {
                    case EVENT_CURSED_MOGU_SPIRIT_BOLT:
                        if (Unit* target = me->SelectNearestTarget(5.0f))
                            if (!target->IsFriendlyTo(me))
                                me->CastSpell(target, SPELL_SPIRIT_BOLT, false);
                            events.ScheduleEvent(EVENT_CURSED_MOGU_SPIRIT_BOLT, 15000);
                        break;
                    case EVENT_CURSED_MOGU_GROUND_SLAM:
                        if (Unit* target = me->SelectNearestTarget(5.0f))
                            if (!target->IsFriendlyTo(me))
                                me->CastSpell(target, SPELL_GROUND_SLAM, false);
                            events.ScheduleEvent(EVENT_CURSED_MOGU_GROUND_SLAM, 25000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class GhostEssenceTargetSelector
{
public:
    GhostEssenceTargetSelector() { }

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (cre->GetEntry() == NPC_CURSED_MOGU_SCULPTURE_2)
                return false;

            return true;
    }
};

// Ghost Essence - 120764
class spell_ghost_essence : public SpellScriptLoader
{
public:
    spell_ghost_essence() : SpellScriptLoader("spell_ghost_essence") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_ghost_essence_SpellScript();
    }

    class spell_ghost_essence_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ghost_essence_SpellScript);

        void HandleOnHit()
        {
            if (Creature* target = GetHitCreature())
            {
                target->SetReactState(REACT_AGGRESSIVE);
                target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                target->RemoveAurasDueToSpell(SPELL_BRONZE);
                target->RemoveAurasDueToSpell(SPELL_POSE_2);
                target->RemoveAurasDueToSpell(SPELL_POSE_1);
                target->RemoveAurasDueToSpell(SPELL_STONE);
                target->AI()->DoAction(ACTION_CURSED_MOGU_ATTACK_PLAYER);
            }
        }

        void FilterTargets(std::list<WorldObject*>& targetList)
        {
            targetList.remove_if(GhostEssenceTargetSelector());
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_ghost_essence_SpellScript::HandleOnHit);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ghost_essence_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };
};

float quilenNewY[2] = { 1170.0f, 1240.0f };

class mob_enormous_stone_quilen : public CreatureScript
{
public:
    mob_enormous_stone_quilen() : CreatureScript("mob_enormous_stone_quilen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_enormous_stone_quilenAI(creature);
    }

    struct mob_enormous_stone_quilenAI : public ScriptedAI
    {
        mob_enormous_stone_quilenAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            prevPosition = 1;

            if (me->GetPositionX() > 3900)
                prevPosition = 2;

            nextMovementTimer = 0;
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(prevPosition, me->GetPositionX(), quilenNewY[prevPosition - 1], me->GetPositionZ());
        }

        InstanceScript* pInstance;
        EventMap events;
        uint32 nextMovementTimer;
        uint8 prevPosition;

        void Reset()
        {
            events.Reset();
        }

        void JustReachedHome()
        {
            prevPosition = 1;

            if (me->GetPositionX() > 3900)
                prevPosition = 2;

            nextMovementTimer = 0;
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(prevPosition, me->GetPositionX(), quilenNewY[prevPosition - 1], me->GetPositionZ());
        }

        void MovementInform(uint32 typeId, uint32 pointId)
        {
            if (typeId != POINT_MOTION_TYPE)
                return;

            if (me->IsInCombat())
                return;

            prevPosition = pointId;
            nextMovementTimer = 500;
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            me->SetWalk(false);
            events.ScheduleEvent(EVENT_ENORMOUS_QUILEN_BITE, urand(3000, 5000));
        }

        void UpdateAI(const uint32 diff)
        {
            if (nextMovementTimer)
            {
                if (nextMovementTimer <= diff)
                {
                    me->GetMotionMaster()->MovePoint(prevPosition == 2 ? 1 : 2, me->GetPositionX(), quilenNewY[prevPosition == 2 ? 0 : 1], me->GetPositionZ());
                    nextMovementTimer = 0;
                }
                else
                    nextMovementTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_ENORMOUS_QUILEN_BITE)
            {
                if (Unit* target = me->SelectNearestTarget(5.0f))
                {
                    if (!target->IsFriendlyTo(me))
                        me->CastSpell(target, SPELL_MONSTROUS_BITE, true);
                }

                events.ScheduleEvent(EVENT_ENORMOUS_QUILEN_BITE, urand(6000, 8000));
            }

            DoMeleeAttackIfReady();
        }
    };
};

class mob_stone_quilen : public CreatureScript
{
public:
    mob_stone_quilen() : CreatureScript("mob_stone_quilen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_stone_quilenAI(creature);
    }

    enum eActions
    {
        ACTION_KILLED = 1
    };

    struct mob_stone_quilenAI : public ScriptedAI
    {
        mob_stone_quilenAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            events.ScheduleEvent(EVENT_QUILEN_SUNDERING_BITE, urand(5000, 6000));
            events.ScheduleEvent(EVENT_QUILEN_SHATTERING_STONE, urand(10000, 12000));
            events.ScheduleEvent(EVENT_QUILEN_FOCUSED_ASSAULT, urand(500, 5000));
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!UpdateVictim())
                return;

            if (!me->HasAura(SPELL_PROTECTIVE_FENZY) && me->HealthBelowPct(10))
                me->CastSpell(me, SPELL_PROTECTIVE_FENZY, true);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_QUILEN_SUNDERING_BITE:
                        if (Unit* target = me->SelectNearestTarget(5.0f))
                            me->CastSpell(target, SPELL_SUNDERING_BITE, true);
                        events.ScheduleEvent(EVENT_QUILEN_SUNDERING_BITE, urand(5000, 6000));
                        break;
                    case EVENT_QUILEN_SHATTERING_STONE:
                        if (Unit* target = me->SelectNearestTarget(5.0f))
                            me->CastSpell(target, SPELL_SHATTERING_STONE, true);
                        events.ScheduleEvent(EVENT_QUILEN_SHATTERING_STONE, urand(10000, 12000));
                        break;
                    case EVENT_QUILEN_FOCUSED_ASSAULT:
                        if (Unit* target = me->SelectNearestTarget(5.0f))
                            me->AddAura(SPELL_FOCUSED_ASSAULT, target);
                        events.ScheduleEvent(EVENT_QUILEN_FOCUSED_ASSAULT, urand(500, 5000));
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class mob_zandalari_skullcharger : public CreatureScript
{
public:
    mob_zandalari_skullcharger() : CreatureScript("mob_zandalari_skullcharger") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_zandalari_skullchargerAI(creature);
    }

    struct mob_zandalari_skullchargerAI : public ScriptedAI
    {
        mob_zandalari_skullchargerAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            events.ScheduleEvent(EVENT_ZANDALARI_TROLL_RUSH, urand(5000, 6000));
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_ZANDALARI_TROLL_RUSH)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST))
                {
                    me->CastSpell(target, SPELL_TROLL_RUSH, true);
                    me->GetMotionMaster()->MoveChase(target);
                }

                events.ScheduleEvent(EVENT_ZANDALARI_TROLL_RUSH, urand(15000, 18000));
            }

            DoMeleeAttackIfReady();
        }
    };
};

class spell_mogu_petrification : public SpellScriptLoader
{
public:
    spell_mogu_petrification() : SpellScriptLoader("spell_mogu_petrification") { }

    AuraScript* GetAuraScript() const
    {
        return new spell_mogu_petrification_AuraScript();
    }

    class spell_mogu_petrification_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mogu_petrification_AuraScript);

        uint32 stack;

        void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster())
            {
                if (Unit* target = GetTarget())
                {
                    if (target->HasAura(SPELL_PETRIFIED))
                    {
                        stack = GetTarget()->GetAura(SPELL_PETRIFIED)->GetStackAmount();
                        if (stack >= 100)
                            target->AddAura(SPELL_FULLY_PETRIFIED, target);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_mogu_petrification_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED, AURA_EFFECT_HANDLE_REAPPLY);
        }
    };
};

class mob_sorcerer_mogu : public CreatureScript
{
public:
    mob_sorcerer_mogu() : CreatureScript("mob_sorcerer_mogu") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_sorcerer_moguAI(creature);
    }

    struct mob_sorcerer_moguAI : public ScriptedAI
    {
        mob_sorcerer_moguAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        bool allKilled;

        void Reset()
        {
            allKilled = false;

            me->SetDisplayId(11686);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);

            events.Reset();
            events.ScheduleEvent(EVENT_SORCERER_SHADOW_BLAST, 5000);
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_SET_GHOST_VISUAL:
                    me->AddAura(SPELL_INACTIVE, me);
                    break;
                case ACTION_SET_NATIVE_DISPLAYID:
                    me->SetDisplayId(40138);
                    break;
                case ACTION_BEFORE_COMBAT:
                    me->AddAura(SPELL_ACTIVATION_VISUAL, me);
                    break;
                case ACTION_START_FIRST_COMBAT:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
                    me->RemoveAurasDueToSpell(SPELL_ACTIVATION_VISUAL);
                    me->setFaction(14);
                    break;
                case ACTION_END_FIRST_COMBAT:
                    me->DespawnOrUnsummon();
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SORCERER_SHADOW_BLAST)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, SPELL_SHADOW_BLAST, false);

                events.ScheduleEvent(EVENT_SORCERER_SHADOW_BLAST, urand(8000, 17000));
            }

            DoMeleeAttackIfReady();
        }
    };
};

class mob_mogu_secret_keeper : public CreatureScript
{
public:
    mob_mogu_secret_keeper() : CreatureScript("mob_mogu_secret_keeper") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_mogu_secret_keeperAI(creature);
    }

    struct mob_mogu_secret_keeperAI : public ScriptedAI
    {
        mob_mogu_secret_keeperAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            events.ScheduleEvent(EVENT_SECRET_FLESH_TO_STONE, urand(5000, 12000));
            events.ScheduleEvent(EVENT_SECRET_STONE_BLOCK, urand(15000, 20000));
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SECRET_FLESH_TO_STONE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_FLESH_TO_STONE, false);
                        events.ScheduleEvent(EVENT_SECRET_FLESH_TO_STONE, 25000);
                        break;
                    case EVENT_SECRET_STONE_BLOCK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_STONE_BLOCK, false);
                        events.ScheduleEvent(EVENT_SECRET_STONE_BLOCK, 40000);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class mob_mogu_warden : public CreatureScript
{
public:
    mob_mogu_warden() : CreatureScript("mob_mogu_warden") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_mogu_wardenAI(creature);
    }

    struct mob_mogu_wardenAI : public ScriptedAI
    {
        mob_mogu_wardenAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            events.ScheduleEvent(EVENT_WARDEN_FORCEFUL_SWING, urand(5000, 12000));
            events.ScheduleEvent(EVENT_WARDEN_TOUCH_OF_NALAK_SHA, urand(15000, 20000));
            events.ScheduleEvent(EVENT_WARDEN_WARDEN_S_FURY, urand(22000, 28000));
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_WARDEN_FORCEFUL_SWING:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_FORCEFUL_SWING, false);
                        events.ScheduleEvent(EVENT_WARDEN_FORCEFUL_SWING, 30000);
                        break;
                    case EVENT_WARDEN_TOUCH_OF_NALAK_SHA:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_TOUCH_OF_NALAK_SHA, false);
                        events.ScheduleEvent(EVENT_WARDEN_TOUCH_OF_NALAK_SHA, 30000);
                        break;
                    case EVENT_WARDEN_WARDEN_S_FURY:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_WARDEN_S_FURY, false);
                        events.ScheduleEvent(EVENT_WARDEN_WARDEN_S_FURY, 30000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class mob_mogu_engine_keeper : public CreatureScript
{
public:
    mob_mogu_engine_keeper() : CreatureScript("mob_mogu_engine_keeper") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_mogu_engine_keeperAI(creature);
    }

    struct mob_mogu_engine_keeperAI : public ScriptedAI
    {
        mob_mogu_engine_keeperAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            events.ScheduleEvent(EVENT_KEEPER_ENHANCED_RECONSTRUCTION, urand(5000, 12000));
            events.ScheduleEvent(EVENT_KEEPER_RECONSTRUCTING, urand(15000, 20000));
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_KEEPER_ENHANCED_RECONSTRUCTION:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_ENHANCED_RECONSTRUCTION, false);
                        events.ScheduleEvent(EVENT_KEEPER_ENHANCED_RECONSTRUCTION, 30000);
                        break;
                    case EVENT_KEEPER_RECONSTRUCTING:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_RECONSTRUCTING, false);
                        events.ScheduleEvent(EVENT_KEEPER_RECONSTRUCTING, 30000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class mob_shan_xi_watershaper: public CreatureScript
{
public:
    mob_shan_xi_watershaper() : CreatureScript("mob_shan_xi_watershaper") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_shan_xi_watershaperAI(creature);
    }

    enum eSpells
    {
        SPELL_FROST_CHANNELING = 45846,
        SPELL_LIFESTREAM       = 118564
    };

    enum eEvents
    {
        EVENT_FROST_CHANNELING = 1,
        EVENT_LIFESTEAM        = 2
    };

    struct mob_shan_xi_watershaperAI : public ScriptedAI
    {
        mob_shan_xi_watershaperAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        EventMap cosmeticEvents;

        void Reset()
        {
            events.Reset();
            cosmeticEvents.Reset();

            cosmeticEvents.ScheduleEvent(EVENT_FROST_CHANNELING, 1 * IN_MILLISECONDS);
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            me->RemoveAura(SPELL_FROST_CHANNELING);

            events.ScheduleEvent(EVENT_LIFESTEAM, urand(7, 15) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (cosmeticEvents.ExecuteEvent() == EVENT_FROST_CHANNELING)
            {
                me->CastSpell(me, SPELL_FROST_CHANNELING, false);
            }

            cosmeticEvents.Update(diff);
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (events.ExecuteEvent() == EVENT_LIFESTEAM)
            {
                me->CastSpell(me, SPELL_LIFESTREAM, false);
                events.ScheduleEvent(EVENT_LIFESTEAM, urand(7, 15) * IN_MILLISECONDS);
            }
        }
    };
};

class mob_zandalari_fire_dancer : public CreatureScript
{
public:
    mob_zandalari_fire_dancer() : CreatureScript("mob_zandalari_fire_dancer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_zandalari_fire_dancerAI(creature);
    }

    enum eSpells
    {
        SPELL_PYROBLAST = 120670,
        SPELL_BLAZING_SPEED = 116592,
        SPELL_FIREBOLT = 15228,
        SPELL_COSMETIC_FIREBALL = 117822,
        SPELL_FIRE_CHANNELING = 117826,
        SPELL_DETONATE = 117241
    };

    enum eEvents
    {
        EVENT_PYROBLAST = 1,
        EVENT_BLAZING_SPEED = 2,
        EVENT_FIREBOLT = 3,
        EVENT_DETONATE = 4,
        EVENT_COSMETIC_FIREBALL = 5,
        EVENT_FIRE_CHANNELING = 6
    };

    struct mob_zandalari_fire_dancerAI : public ScriptedAI
    {
        mob_zandalari_fire_dancerAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        EventMap cosmeticEvents;

        void Reset()
        {
            events.Reset();
            cosmeticEvents.Reset();

            cosmeticEvents.ScheduleEvent(urand(EVENT_COSMETIC_FIREBALL, EVENT_FIRE_CHANNELING), 1 * IN_MILLISECONDS);
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            cosmeticEvents.Reset();

            events.ScheduleEvent(EVENT_PYROBLAST, urand(5, 9) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_BLAZING_SPEED, urand(12, 17) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_DETONATE, urand(12, 18) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_FIREBOLT, 1 * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (uint32 eventId = cosmeticEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FIRE_CHANNELING:
                        me->CastSpell(me, SPELL_FIRE_CHANNELING, false);
                        cosmeticEvents.ScheduleEvent(EVENT_COSMETIC_FIREBALL, 16 * IN_MILLISECONDS);
                        break;
                    case EVENT_COSMETIC_FIREBALL:
                        me->CastSpell(me, SPELL_COSMETIC_FIREBALL, false);
                        cosmeticEvents.ScheduleEvent(EVENT_FIRE_CHANNELING, 3 * IN_MILLISECONDS);
                        break;
                }
            }

            cosmeticEvents.Update(diff);
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_PYROBLAST:
                        me->CastSpell((Unit*)NULL, SPELL_PYROBLAST, false);
                        events.ScheduleEvent(EVENT_PYROBLAST, urand(5, 9) * IN_MILLISECONDS);
                        break;
                    case EVENT_BLAZING_SPEED:
                        me->CastSpell(me, SPELL_BLAZING_SPEED, false);
                        events.ScheduleEvent(EVENT_BLAZING_SPEED, urand(41, 47) * IN_MILLISECONDS);
                        break;
                    case EVENT_DETONATE:
                        me->CastSpell(me, SPELL_DETONATE, false);
                        events.ScheduleEvent(EVENT_DETONATE, urand(12, 18) * IN_MILLISECONDS);
                        break;
                    case EVENT_FIREBOLT:
                        me->CastSpell((Unit*)NULL, SPELL_FIREBOLT, false);
                        events.ScheduleEvent(EVENT_FIREBOLT, 3 * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };
};

class mob_zandalari_infiltrator : public CreatureScript
{
public:
    mob_zandalari_infiltrator() : CreatureScript("mob_zandalari_infiltrator") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_zandalari_infiltratorAI(creature);
    }

    enum eSpells
    {
        SPELL_SMOKE_BOMB = 116595
    };

    enum eEvents
    {
        EVENT_SMOKE_BOMB = 1
    };

    struct mob_zandalari_infiltratorAI : public ScriptedAI
    {
        mob_zandalari_infiltratorAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            events.ScheduleEvent(EVENT_SMOKE_BOMB, urand(4, 8) * IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SMOKE_BOMB)
            {
                me->CastSpell(me, SPELL_SMOKE_BOMB, false);
                events.ScheduleEvent(EVENT_SMOKE_BOMB, urand(24, 28) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_troll_explosives : public CreatureScript
{
public:
    npc_troll_explosives() : CreatureScript("npc_troll_explosives") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_troll_explosivesAI(creature);
    }

    enum eSpells
    {
        SPELL_EXPLOSIOIN_AURA = 116508
    };

    enum eNpc
    {
        NPC_EXPLOSION_BUNNY_N_M = 60388,
        NPC_EXPLOSION_BUNNY_N_P = 60392,
        NPC_EXPLOSION_BUNNY_S_M = 60390,
        NPC_EXPLOSION_BUNNY_S_P = 60394,
        NPC_EXPLOSION_BUNNY_E_M = 60389,
        NPC_EXPLOSION_BUNNY_E_P = 60395,
        NPC_EXPLOSION_BUNNY_W_M = 60391,
        NPC_EXPLOSION_BUNNY_W_P = 60393
    };

    struct npc_troll_explosivesAI : public ScriptedAI
    {
        npc_troll_explosivesAI(Creature* creature) : ScriptedAI(creature) {}

        float orientation;

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            orientation = 0.0f;

            switch (me->GetEntry())
            {
                case NPC_EXPLOSION_BUNNY_N_M:
                case NPC_EXPLOSION_BUNNY_N_P:
                    orientation = 0.0f;
                    break;
                case NPC_EXPLOSION_BUNNY_S_M:
                case NPC_EXPLOSION_BUNNY_S_P:
                    orientation = M_PI;
                    break;
                case NPC_EXPLOSION_BUNNY_E_M:
                case NPC_EXPLOSION_BUNNY_E_P:
                    orientation = 4.71f;
                    break;
                case NPC_EXPLOSION_BUNNY_W_M:
                case NPC_EXPLOSION_BUNNY_W_P:
                    orientation = 1.57f;
                    break;
            }

            float x, y = 0.0f;
            GetPositionWithDistInOrientation(me, 40.0f, orientation, x, y);
            me->GetMotionMaster()->MovePoint(1, x, y, me->GetPositionZ());

            me->AddAura(SPELL_EXPLOSIOIN_AURA, me);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            damage = 0;
        }

        void MovementInform(uint32 /*type*/, uint32 id)
        {
            if (id == 1)
                me->DespawnOrUnsummon();
        }
    };
};

class DetonateTargetSelector
{
public:
    DetonateTargetSelector() { }

    enum eNpc
    {
        NPC_TROLL_EXPLOSIVES = 60644
    };

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (cre->GetEntry() == NPC_TROLL_EXPLOSIVES)
                return false;

            return true;
    }
};

class spell_mvault_detonate : public SpellScriptLoader
{
public:
    spell_mvault_detonate() : SpellScriptLoader("spell_mvault_detonate") { }

    SpellScript *GetSpellScript() const
    {
        return new spell_mvault_detonate_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_mvault_detonate_AuraScript();
    }

    enum eSpells
    {
        SPELL_TROLL_EXPLOSIVES = 116545
    };

    class spell_mvault_detonate_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mvault_detonate_SpellScript)

        void FilterTargets(std::list<WorldObject*>& targetList)
        {
            targetList.remove_if(DetonateTargetSelector());

            if (targetList.size() > 1)
            {
                targetList.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));
                targetList.resize(1);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mvault_detonate_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    class spell_mvault_detonate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mvault_detonate_AuraScript);

        void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Creature* owner = GetOwner()->ToCreature();
            if (!owner)
                return;

            owner->CastSpell(owner, SPELL_TROLL_EXPLOSIVES, false);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_mvault_detonate_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };
};

class spell_troll_explosives : public SpellScriptLoader
{
public:
    spell_troll_explosives() : SpellScriptLoader("spell_troll_explosives") { }

    AuraScript* GetAuraScript() const
    {
        return new spell_troll_explosives_AuraScript();
    }

    enum eSpells
    {
        SPELL_EXPLOSION = 116493
    };

    class spell_troll_explosives_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_troll_explosives_AuraScript);

        void OnRemove(AuraEffect const *, AuraEffectHandleModes)
        {
            Creature* owner = GetOwner()->ToCreature();
            if (!owner)
                return;

            owner->CastSpell(owner, SPELL_EXPLOSION, false);
            owner->DespawnOrUnsummon();
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_troll_explosives_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };
};

class ExplosionTargetSelector
{
public:
    ExplosionTargetSelector() { }

    bool operator()(WorldObject* object)
    {
        if (object->GetTypeId() == TYPEID_PLAYER)
            return false;

        return true;
    }
};

class spell_mvault_explosion : public SpellScriptLoader
{
public:
    spell_mvault_explosion() : SpellScriptLoader("spell_mvault_explosion") { }

    SpellScript *GetSpellScript() const
    {
        return new spell_mvault_explosion_SpellScript();
    }

    class spell_mvault_explosion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mvault_explosion_SpellScript)

        void FilterTargets(std::list<WorldObject*>& targetList)
        {
            targetList.remove_if(ExplosionTargetSelector());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mvault_explosion_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };
};

void AddSC_mogu_shan_vault()
{
    new mob_cursed_mogu_sculpture();
    new mob_enormous_stone_quilen();
    new mob_stone_quilen();
    new mob_zandalari_skullcharger();
    new mob_sorcerer_mogu();
    new mob_mogu_secret_keeper();
    new mob_mogu_warden();
    new mob_mogu_engine_keeper();
    new mob_shan_xi_watershaper();
    new mob_zandalari_fire_dancer();
    new mob_zandalari_infiltrator();
    new npc_troll_explosives();
    new spell_ghost_essence();
    new spell_mvault_detonate();
    new spell_troll_explosives();
    new spell_mvault_explosion();
    new spell_mogu_petrification();
}
