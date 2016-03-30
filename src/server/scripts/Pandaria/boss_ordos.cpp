/*
 * Copyright (C) 2012-2014 Trinity <http://www.pandashan.com/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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
#include "ObjectMgr.h"
#include "ScriptMgr.h"

#define BOSS_ORDOS 0

enum eSpells
{
    SPELL_ORDOS_ANCIENT_FLAME = 144695,
    SPELL_ORDOS_BURNING_SOUL = 144689,
    SPELL_ORDOS_ETERNAL_AGONY = 144696,
    SPELL_ORDOS_MAGMA_CRUSH = 144688,
    SPELL_ORDOS_POOL_OF_FIRE = 144692
};

enum eEvents
{
    EVENT_ORDOS_ANCIENT_FLAME = 1,
    EVENT_ORDOS_BURNING_SOUL,
    EVENT_ORDOS_POOL_OF_FIRE,
    EVENT_ORDOS_ETERNAL_AGONY,
    EVENT_ORDOS_MAGMA_CRUSH
};

enum eCreatures
{
};

enum eSays
{
    SAY_ORDOS_AGGRO = 0,
    SAY_ORDOS_INTRO_1,
    SAY_ORDOS_INTRO_2,
    SAY_ORDOS_INTRO_3,
    SAY_ORDOS_DEATH,
    SAY_ORDOS_KILL,
    SAY_ORDOS_POOL_OF_FIRE,
    SAY_ORDOS_ANCIENT_FLAME,
    SAY_ORDOS_BURNING_SOUL,
    SAY_ORDOS_ETERNAL_AGONY
};

enum eActions
{
};

class boss_ordos : public CreatureScript
{
    public:
        boss_ordos() : CreatureScript("boss_ordos") { }

        struct boss_ordosAI : public BossAI
        {
            boss_ordosAI(Creature* creature) : BossAI(creature, BOSS_ORDOS) { }

            EventMap events;

            void Reset()
            {
                events.Reset();
                me->RemoveAreaTrigger(SPELL_ORDOS_POOL_OF_FIRE);;
                _Reset();

                events.ScheduleEvent(EVENT_ORDOS_MAGMA_CRUSH, urand(7500, 15000));
                events.ScheduleEvent(urand(EVENT_ORDOS_ANCIENT_FLAME, EVENT_ORDOS_POOL_OF_FIRE), urand(15000, 20000));
                events.ScheduleEvent(EVENT_ORDOS_ETERNAL_AGONY, 5 * MINUTE * IN_MILLISECONDS);
            }

            void KilledUnit(Unit* who)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_ORDOS_KILL);
                        return;
            }

            void EnterCombat(Unit* /*attacker*/)
            {
                Talk(SAY_ORDOS_AGGRO);
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_ORDOS_DEATH);
                me->RemoveAreaTrigger(SPELL_ORDOS_POOL_OF_FIRE);
                summons.DespawnAll();
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
                switch (events.ExecuteEvent())
                {
                    case EVENT_ORDOS_MAGMA_CRUSH:
                    {
                        DoCast(SPELL_ORDOS_MAGMA_CRUSH);
                        events.ScheduleEvent(EVENT_ORDOS_MAGMA_CRUSH, urand(7500, 15000));
                        break;
                    }
                    case EVENT_ORDOS_ANCIENT_FLAME:
                    {
                        DoCast(SPELL_ORDOS_ANCIENT_FLAME);
                        Talk(SAY_ORDOS_ANCIENT_FLAME);
                        events.ScheduleEvent(urand(EVENT_ORDOS_BURNING_SOUL, EVENT_ORDOS_POOL_OF_FIRE), urand(15000, 20000));
                        break;
                    }
                    case EVENT_ORDOS_BURNING_SOUL:
                    {
                        DoCast(SPELL_ORDOS_BURNING_SOUL);
                        Talk(SAY_ORDOS_BURNING_SOUL);
                        events.ScheduleEvent(urand(EVENT_ORDOS_ANCIENT_FLAME, EVENT_ORDOS_POOL_OF_FIRE), urand(15000, 20000));
                        break;
                    }
                    case EVENT_ORDOS_ETERNAL_AGONY:
                    {
                        DoCast(SPELL_ORDOS_ETERNAL_AGONY);
                        Talk(SAY_ORDOS_ETERNAL_AGONY);
                        break;
                    }
                    case EVENT_ORDOS_POOL_OF_FIRE:
                    {
                        DoCast(SPELL_ORDOS_POOL_OF_FIRE);
                        Talk(SAY_ORDOS_POOL_OF_FIRE);
                        events.ScheduleEvent(urand(EVENT_ORDOS_ANCIENT_FLAME, EVENT_ORDOS_BURNING_SOUL), urand(15000, 20000));
                        break;
                    }
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_ordosAI(creature);
        }
};

// Magma Crush - 144688
class spell_ordos_magma_crush : public SpellScriptLoader
{
public:
    spell_ordos_magma_crush() : SpellScriptLoader("spell_ordos_magma_crush") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        bool Load()
        {
            _targets = 1;
            return true;
        }

        void CountTargets(std::list<WorldObject*>& targets)
        {
            _targets = targets.size();
        }

        void SplitDamage(SpellEffIndex /*eff*/)
        {
            SetHitDamage(int32(GetHitDamage() / _targets));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(script_impl::SplitDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }

        private:
            uint8 _targets;
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

enum PoolOfFire
{
    SPELL_ORDOS_POOL_OF_FIRE_AURA = 144693
};

// 1090 - Pool of Fire
class sat_ordos_pool_of_fire : public SpellAreaTriggerScript
{
public:
    sat_ordos_pool_of_fire() : SpellAreaTriggerScript("sat_ordos_pool_of_fire") {}

    class script_impl : public IAreaTriggerAura
    {
        bool CheckTriggering(WorldObject* triggering)
        {
            Unit* unit = triggering->ToUnit();
            if (!unit)
                return false;

            if (!m_target->IsWithinDistInMap(unit, m_range))
                return false;

            if (!m_caster->_IsValidAttackTarget(unit, m_spellInfo, m_target))
                return false;

            return true;
        }

        void OnTriggeringApply(WorldObject* triggering)
        {
            m_caster->AddAura(SPELL_ORDOS_POOL_OF_FIRE_AURA, triggering->ToUnit());
        }

        void OnTriggeringRemove(WorldObject* triggering)
        {
            triggering->ToUnit()->RemoveAura(SPELL_ORDOS_POOL_OF_FIRE_AURA, m_caster->GetGUID());
        }
    };

    IAreaTrigger* GetInterface() const override
    {
        return new script_impl();
    }
};

void AddSC_boss_ordos()
{

    new sat_ordos_pool_of_fire();
    new spell_ordos_magma_crush();
    new boss_ordos();
}
