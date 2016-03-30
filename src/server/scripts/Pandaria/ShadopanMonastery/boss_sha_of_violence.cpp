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

class boss_sha_of_violence : public CreatureScript
{
public:
    boss_sha_of_violence() : CreatureScript("boss_sha_of_violence") {}

    enum eSpells
    {
        SPELL_SMOKE_BLADES       = 106826,
        SPELL_SHA_SPIKE          = 106871,
        SPELL_DISORIENTING_SMASH = 106872,
        SPELL_ENRAGE             = 130196,
        SPELL_ICE_TRAP           = 110610
    };

    enum eEvents
    {
        EVENT_SMOKE_BLADES       = 1,
        EVENT_SHA_SPIKE          = 2,
        EVENT_DISORIENTING_SMASH = 3
    };

    enum eTalks
    {
        TALK_AGGRO = 0,
        TALK_DEATH = 1,
        TALK_SLAY  = 2
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_sha_of_violenceAI(creature);
    }

    struct boss_sha_of_violenceAI : public BossAI
    {
        boss_sha_of_violenceAI(Creature* creature) : BossAI(creature, DATA_SHA_VIOLENCE)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        bool enrageDone;

        void InitializeAI()
        {
            me->setActive(true);

            Reset();
        }

        void Reset()
        {
            _Reset();          
            enrageDone = false;
        }
         
        void EnterCombat(Unit * who)
        {
            _EnterCombat();
            Talk(TALK_AGGRO);
            if (instance)
            {
                instance->SetData(DATA_SHA_VIOLENCE, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            events.ScheduleEvent(EVENT_SMOKE_BLADES, urand(25000, 35000));
            events.ScheduleEvent(EVENT_SHA_SPIKE, urand(10000, 20000));
            events.ScheduleEvent(EVENT_DISORIENTING_SMASH, urand(20000, 30000));
        }

        void JustDied(Unit* killer)
        {
            _JustDied();
            Talk(TALK_DEATH);
            if (instance)
            {
                instance->SetData(DATA_SHA_VIOLENCE, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(TALK_SLAY);
        }

        void EnterEvadeMode()
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_SHA_VIOLENCE, FAIL);
                instance->SetBossState(DATA_SHA_VIOLENCE, FAIL);
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->CastSpell(summon, SPELL_ICE_TRAP, true);
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (!enrageDone && me->HealthBelowPctDamaged(20, damage))
            {
                enrageDone = true;
                me->CastSpell(me, SPELL_ENRAGE, true);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            switch(events.ExecuteEvent())
            {
                case EVENT_SMOKE_BLADES:
                    me->CastSpell(me, SPELL_SMOKE_BLADES, false);
                    events.ScheduleEvent(EVENT_SMOKE_BLADES, urand(25000, 35000));
                    break;
                case EVENT_SHA_SPIKE:
                    if (auto const target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                        me->CastSpell(target, SPELL_SHA_SPIKE, false);
                    events.ScheduleEvent(EVENT_SHA_SPIKE, urand(10000, 20000));
                    break;
                case EVENT_DISORIENTING_SMASH:
                    if (auto const target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                        me->CastSpell(target, SPELL_DISORIENTING_SMASH, false);
                    events.ScheduleEvent(EVENT_DISORIENTING_SMASH, urand(20000, 30000));
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_sv_lesser : public CreatureScript
{
public:
    npc_sv_lesser() : CreatureScript("npc_sv_lesser") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sv_lesser_AI(creature);
    }

    enum eSpells
    {
        SPELL_EXPLOSION = 106966
    };

    enum eEvents
    {
        EVENT_DIE = 1
    };

    struct npc_sv_lesser_AI : public ScriptedAI
    {
        npc_sv_lesser_AI(Creature* creature) : ScriptedAI(creature) {}

        InstanceScript* instance;
        EventMap events;
        bool canDead;

        void InitializeAI()
        {
            canDead = false;
            events.ScheduleEvent(EVENT_DIE, 30 * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (me->GetHealth() <= damage)
            {
                damage = 0;
                if (!canDead)
                {
                    canDead = true;
                    me->CastSpell((Unit*)NULL, SPELL_EXPLOSION, false);
                    attacker->Kill(me);
                    me->DespawnOrUnsummon(2 * IN_MILLISECONDS);
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_DIE)
                me->DealDamage(me, me->GetHealth());

            DoMeleeAttackIfReady();
        }
    };
};

class SmokeBladesTargetSelector
{
public:
    bool operator()(WorldObject* object) const
    {
        if (object->GetTypeId() == TYPEID_PLAYER && !object->ToPlayer()->HasAura(SPELL_SMOKE_BLADES))
            return false;

        return true;
    }

private:
    enum eSpells
    {
        SPELL_SMOKE_BLADES = 106827
    };
};

class spell_sv_smoke_blades : public SpellScriptLoader
{
public:
    spell_sv_smoke_blades() : SpellScriptLoader("spell_sv_smoke_blades") { }

    AuraScript* GetAuraScript() const override
    {
        return new spell_sv_smoke_blades_AuraScript();
    }

    SpellScript* GetSpellScript() const override
    {
        return new spell_sv_smoke_blades_SpellScript();
    }

    enum eSpells
    {
        SPELL_PARTING_SMOKE = 127576
    };

    class spell_sv_smoke_blades_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sv_smoke_blades_AuraScript);

        void HadleEffect(AuraEffect const * aurEff, bool apply, Player* owner)
        {
            for(int i = 0; i < MAX_ATTACK; ++i)
                if (Item* pItem = owner->GetWeaponForAttack(WeaponAttackType(i), true))
                    owner->_ApplyWeaponDependentAuraCritMod(pItem, WeaponAttackType(i), aurEff, apply);

            if (GetSpellInfo()->EquippedItemClass == -1)
            {
                owner->HandleBaseModValue(CRIT_PERCENTAGE, FLAT_MOD, float(aurEff->GetAmount()), apply);
                owner->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, float(aurEff->GetAmount()), apply);
                owner->HandleBaseModValue(RANGED_CRIT_PERCENTAGE, FLAT_MOD, float(aurEff->GetAmount()), apply);
            }
            else
            {
                // done in Player::_ApplyWeaponDependentAuraMods
            }
        }

        void OnApply(AuraEffect const * aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (auto const owner = GetOwner()->ToPlayer())
                HadleEffect(aurEff, true, owner);
        }

        void HanleOnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {      
            if (auto const caster = eventInfo.GetActor())
            {
                if (auto const smoke = caster->GetAura(aurEff->GetId()))
                {
                    if (smoke->GetDuration() > 5 * IN_MILLISECONDS)
                        smoke->SetDuration(smoke->GetDuration() - 5 * IN_MILLISECONDS);
                    else
                        caster->RemoveAura(aurEff->GetId());
                }
            }
        }

        void OnRemove(AuraEffect const * aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (auto const owner = GetOwner()->ToPlayer())
            {
                HadleEffect(aurEff, false, owner);
                owner->CastSpell(owner, SPELL_PARTING_SMOKE, false);
            }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_sv_smoke_blades_AuraScript::HanleOnProc, EFFECT_1, SPELL_AURA_DUMMY);
            OnEffectApply += AuraEffectApplyFn(spell_sv_smoke_blades_AuraScript::OnApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_sv_smoke_blades_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    class spell_sv_smoke_blades_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sv_smoke_blades_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(SmokeBladesTargetSelector());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sv_smoke_blades_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };
};

void AddSC_boss_sha_of_violence()
{
    new boss_sha_of_violence();
    new npc_sv_lesser();
    new spell_sv_smoke_blades();
}
