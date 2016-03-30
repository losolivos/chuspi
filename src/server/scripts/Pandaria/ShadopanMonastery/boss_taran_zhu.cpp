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

class boss_taran_zhu : public CreatureScript
{
public:
    boss_taran_zhu() : CreatureScript("boss_taran_zhu") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_taran_zhuAI(creature);
    }

    enum eTalks
    {
        TALK_INTRO = 0,
        TALK_AGGRO = 1,
        TALK_DEATH = 2,
        TALK_SLAY  = 3,
        TALK_OUTRO = 4
    };

    enum eEvents
    {
        EVENT_RISING_HATE            = 1,
        EVENT_RING_OF_MALICE         = 2,
        EVENT_SHA_BLAST              = 3,
        EVENT_SUMMON_GRIPPING_HATRED = 4,
        EVENT_GRIP_OF_HATE           = 5,
        EVENT_OUTRO                  = 6,
        EVENT_CHEST                  = 7
    };

    enum eSpells
    {
        SPELL_CORRUPTED              = 131530,
        SPELL_RISING_HATE            = 107356,
        SPELL_RING_OF_MALICE         = 131521,
        SPELL_SHA_BLAST              = 114999,
        SPELL_SUMMON_GRIPPING_HATRED = 115002,
        SPELL_KNEEL                  = 130491
    };

    enum eCreature
    {
        NPC_SNOWDRIFT_QUESTENDER = 64387
    };

    struct boss_taran_zhuAI : public BossAI
    {
        boss_taran_zhuAI(Creature* creature) : BossAI(creature, DATA_TARAN_ZHU) {}

        InstanceScript* instance;
        EventMap events, nonCombatEvents;
        bool introStarted, isFightWon;
        uint32 newPower;

        void InitializeAI()
        {
            instance = me->GetInstanceScript();
            introStarted = false;
            isFightWon = false;
            me->setActive(true);
            me->AddAura(SPELL_CORRUPTED, me);

            if (instance)
            {
                if (instance->GetBossState(DATA_TARAN_ZHU) == DONE)
                {
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    me->SetReactState(REACT_PASSIVE);
                    me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_KNEEL);
                    me->SummonCreature(NPC_SNOWDRIFT_QUESTENDER, 3855.144f, 2632.581f, 754.541f, 4.964f);
                    nonCombatEvents.ScheduleEvent(EVENT_CHEST, 1 * IN_MILLISECONDS);

                    float x, y, z;
                    z = me->GetPositionZ();
                    GetPositionWithDistInOrientation(me, -5.0f, me->GetOrientation(), x, y);
                    me->NearTeleportTo(x, y, z, me->GetOrientation());
                }
            }

            Reset();
        }

        void Reset()
        {
            if (instance)
            {
                if (instance->GetBossState(DATA_TARAN_ZHU) != DONE)
                {
                    _Reset();
                    events.Reset();
                    nonCombatEvents.Reset();
                    me->SetReactState(REACT_AGGRESSIVE);
                }
            }
            
            newPower = 0;
        }

        void EnterCombat(Unit * who)
        {
            _EnterCombat();
            Talk(TALK_AGGRO);
            if (instance)
            {
                instance->SetData(DATA_TARAN_ZHU, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            events.ScheduleEvent(EVENT_RISING_HATE, urand(25, 35) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_RING_OF_MALICE, urand(7.5, 12.5) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SHA_BLAST, urand(2.5, 5) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SUMMON_GRIPPING_HATRED, urand(10, 15) * IN_MILLISECONDS);
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
                instance->SetData(DATA_TARAN_ZHU, FAIL);
            }
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType /*damageType*/, const SpellInfo * /*spellInfo*/)
        {
            if (auto const player = target->ToPlayer())
            {
                newPower = player->GetPower(POWER_ALTERNATE_POWER) + std::floor(damage / 5000);
                player->SetPower(POWER_ALTERNATE_POWER, newPower > 100 ? 100 : newPower);
            }
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (!isFightWon)
                {
                    isFightWon = true;

                    _JustDied();
                    me->StopMoving();
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    me->CombatStop(true);
                    me->DeleteThreatList();
                    me->SetReactState(REACT_PASSIVE);
                    Talk(TALK_DEATH);
                    events.Reset();
                    nonCombatEvents.ScheduleEvent(EVENT_OUTRO, 3 * IN_MILLISECONDS);
                    me->SummonCreature(NPC_SNOWDRIFT_QUESTENDER, 3855.144f, 2632.581f, 754.541f, 4.964f);

                    if (instance)
                    {
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                        instance->SetData(DATA_TARAN_ZHU, DONE);

                        if (auto const snowdrift = Unit::GetCreature(*me, instance->GetData64(DATA_MASTER_SNOWDRIFT)))
                            snowdrift->SetPhaseMask(2, true);
                    }

                    std::list<Player*> playerList;
                    GetPlayerListInGrid(playerList, me, 150.0f);
                    if (!playerList.empty())
                    {
                        for(auto player : playerList)
                            player->ModifyCurrency(395, player->GetMap()->IsHeroic() ? 100 * CURRENCY_PRECISION : 70 * CURRENCY_PRECISION);
                    }

                    playerList.clear();
                }
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->ToPlayer())
            {
                if (me->GetDistance(who) < 51.0f && !introStarted && instance->GetBossState(DATA_TARAN_ZHU) != DONE)
                {
                    introStarted = true;
                    Talk(TALK_INTRO);
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (uint32 eventId = nonCombatEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_OUTRO:
                        Talk(TALK_OUTRO);
                        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_KNEEL);
                        break;
                    case EVENT_CHEST:
                        if (auto const cache = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_TARAN_ZHU_CACHE)))
                            cache->SetPhaseMask(1, true);
                        break;
                }
            }

            nonCombatEvents.Update(diff);
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RISING_HATE:
                        me->CastSpell(me, SPELL_RISING_HATE, false);
                        events.ScheduleEvent(EVENT_RISING_HATE, urand(10, 15) * IN_MILLISECONDS);
                        break;
                    case EVENT_RING_OF_MALICE:
                        me->CastSpell(me, SPELL_RING_OF_MALICE, true);
                        events.ScheduleEvent(EVENT_RING_OF_MALICE, urand(28, 33) * IN_MILLISECONDS);
                        break;
                    case EVENT_SHA_BLAST:
                        me->CastSpell((Unit*)NULL, SPELL_SHA_BLAST, false);
                        events.ScheduleEvent(EVENT_SHA_BLAST, urand(4, 6) * IN_MILLISECONDS);
                        break;
                    case EVENT_SUMMON_GRIPPING_HATRED:
                        me->CastSpell(me, SPELL_SUMMON_GRIPPING_HATRED, false);
                        events.ScheduleEvent(EVENT_SUMMON_GRIPPING_HATRED, urand(20, 30) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
            EnterEvadeIfOutOfCombatArea(diff, 90.0f);
        }
    };
};

class npc_gripping_hatred : public CreatureScript
{
public:
    npc_gripping_hatred() : CreatureScript("npc_gripping_hatred") { }

    enum eSpells
    {
        SPELL_GRIP_OF_HATE    = 115010,
        SPELL_POOL_OF_SHADOWS = 112929
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gripping_hatredAI(creature);
    }

    struct npc_gripping_hatredAI : public ScriptedAI
    {
        npc_gripping_hatredAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            me->CastSpell(me, SPELL_POOL_OF_SHADOWS, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!me->HasUnitState(UNIT_STATE_CASTING))
                me->CastSpell(me, SPELL_GRIP_OF_HATE, false);
        }
    };
};

class spell_taran_zhu_hate : public SpellScriptLoader
{
public:
    spell_taran_zhu_hate() : SpellScriptLoader("spell_taran_zhu_hate") { }

    AuraScript* GetAuraScript() const
    {
        return new spell_taran_zhu_hate_AuraScript();
    }

    class spell_taran_zhu_hate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_taran_zhu_hate_AuraScript);

        void HandlePeriodic(AuraEffect const * /*aurEff*/)
        {
            if (auto const target = GetTarget())
            {
                if (target->GetPower(POWER_ALTERNATE_POWER) >= 100)
                {
                    if (!target->HasAura(SPELL_HAZE_OF_HATE))
                    {
                        target->CastSpell(target, SPELL_HAZE_OF_HATE, true);
                        target->CastSpell(target, SPELL_HAZE_OF_HATE_VISUAL, true);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_taran_zhu_hate_AuraScript::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    }; 
};

class spell_taran_zhu_meditation : public SpellScriptLoader
{
public:
    spell_taran_zhu_meditation() : SpellScriptLoader("spell_taran_zhu_meditation") { }

    AuraScript* GetAuraScript() const
    {
        return new spell_taran_zhu_meditation_AuraScript();
    }

    class spell_taran_zhu_meditation_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_taran_zhu_meditation_AuraScript);

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            {
                if (auto const target = GetTarget())
                {
                    target->SetPower(POWER_ALTERNATE_POWER, 0);
                    target->RemoveAurasDueToSpell(SPELL_HAZE_OF_HATE);
                    target->RemoveAurasDueToSpell(SPELL_HAZE_OF_HATE_VISUAL);
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_taran_zhu_meditation_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };
};

class spell_taran_zhu_grip_of_hate : public SpellScriptLoader
{
public:
    spell_taran_zhu_grip_of_hate() : SpellScriptLoader("spell_taran_zhu_grip_of_hate") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_taran_zhu_grip_of_hate_SpellScript();
    }

    class spell_taran_zhu_grip_of_hate_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_taran_zhu_grip_of_hate_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            if (auto const caster = GetCaster())
               if (auto const target = GetHitUnit())
                   target->CastSpell(caster, GetSpellInfo()->Effects[effIndex].BasePoints, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_taran_zhu_grip_of_hate_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };
};

class RingOfMaliceTargetSelector
{
public:
    explicit RingOfMaliceTargetSelector(Unit* _caster) : caster(_caster) {}

    bool operator() (WorldObject* unit) const
    {
        if (caster->GetExactDist2d(unit) <= 8.5f)
            return true;

        return false;
    }
private:
    Unit* caster;
};

class spell_taran_zhu_ring_of_malice : public SpellScriptLoader
{
public:
    spell_taran_zhu_ring_of_malice() : SpellScriptLoader("spell_taran_zhu_ring_of_malice") {}

    SpellScript* GetSpellScript() const
    {
        return new spell_taran_zhu_ring_of_malice_SpellScript();
    }

    class spell_taran_zhu_ring_of_malice_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_taran_zhu_ring_of_malice_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(RingOfMaliceTargetSelector(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_taran_zhu_ring_of_malice_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };
};

void AddSC_boss_taran_zhu()
{
    new boss_taran_zhu();
    new npc_gripping_hatred();
    new spell_taran_zhu_hate();
    new spell_taran_zhu_meditation();
    new spell_taran_zhu_grip_of_hate();
    new spell_taran_zhu_ring_of_malice();
}
