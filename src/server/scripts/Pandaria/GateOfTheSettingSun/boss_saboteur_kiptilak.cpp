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
#include "MoveSplineInit.h"
#include "gate_of_the_setting_sun.h"

enum eSpells
{
    SPELL_PLANT_EXPLOSIVE         = 107187,
    SPELL_SABOTAGE                = 107268,
    SPELL_SABOTAGE_EXPLOSION      = 113645,
    SPELL_PLAYER_EXPLOSION        = 113654,
    SPELL_MUNITION_STABLE         = 109987,
    SPELL_MUNITION_EXPLOSION      = 107153,
    SPELL_MANTID_EXPLOSION        = 107215, 
    SPELL_MUNITION_EXPLOSION_AURA = 107216
};

enum eEvents
{
    EVENT_EXPLOSIVES = 1,
    EVENT_SABOTAGE   = 2,
};

enum eWorldInFlames
{
    WIF_NONE = 0,
    WIF_70   = 1,
    WIF_30   = 2,
};

enum eActions
{
    ACTION_INTRO   = 1,
};

enum eTalks
{
    SAY_AGGRO      = 0,
    SAY_DEATH      = 3,
    SAY_SLAY       = 2,
    SAY_SPECIAL    = 4,
    SAY_INTRO      = 1,
};

class boss_saboteur_kiptilak : public CreatureScript
{
public:
    boss_saboteur_kiptilak() : CreatureScript("boss_saboteur_kiptilak") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_saboteur_kiptilakAI(creature);
    }

    struct boss_saboteur_kiptilakAI : public BossAI
    {
        boss_saboteur_kiptilakAI(Creature* creature) : BossAI(creature, DATA_KIPTILAK), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        SummonList summons;
        uint8 WorldInFlamesEvents;
        bool intro;

        void InitializeAI()
        {
            me->setActive(true);
            intro = false;

            Reset();
        }

        void Reset()
        {
            _Reset();
            events.Reset();
            WorldInFlamesEvents = 0;
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

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);

            if (instance)
               instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            events.ScheduleEvent(EVENT_EXPLOSIVES, urand(7500, 10000));
            events.ScheduleEvent(EVENT_SABOTAGE, urand(22500, 30000));
        }

        void EnterEvadeMode()
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetBossState(DATA_KIPTILAK, FAIL);
            }

            summons.DespawnAll();
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            switch (attacker->GetEntry())
            {
            case NPC_EXPLOSION_BUNNY_N_M:
            case NPC_EXPLOSION_BUNNY_S_M:
            case NPC_EXPLOSION_BUNNY_E_M:
            case NPC_EXPLOSION_BUNNY_W_M:
            case NPC_EXPLOSION_BUNNY_N_P:
            case NPC_EXPLOSION_BUNNY_S_P:
            case NPC_EXPLOSION_BUNNY_E_P:
            case NPC_EXPLOSION_BUNNY_W_P:
                damage = 0;
                return;
            }

            float nextHealthPct = ((float(me->GetHealth()) - damage) / float(me->GetMaxHealth())) * 100;

            if (WorldInFlamesEvents < WIF_70 && nextHealthPct <= 70.0f)
            {
                DoWorldInFlamesEvent();
                ++WorldInFlamesEvents;
            }
            else if (WorldInFlamesEvents < WIF_30 && nextHealthPct <= 30.0f)
            {
                DoWorldInFlamesEvent();
                ++WorldInFlamesEvents;
            }
        }

        void DoWorldInFlamesEvent()
        {
            Talk(SAY_SPECIAL);

            std::list<Creature*> munitionList;
            GetCreatureListWithEntryInGrid(munitionList, me, NPC_STABLE_MUNITION, 100.0f);

            for (auto itr : munitionList)
            {
                itr->RemoveAurasDueToSpell(SPELL_MUNITION_STABLE);
                itr->CastSpell(itr, SPELL_MUNITION_EXPLOSION, true);
                itr->DespawnOrUnsummon(2000);
            }
        }

        void JustSummoned(Creature* summoned)
        {
            if (summoned->GetEntry() == NPC_STABLE_MUNITION)
            {
                summoned->AddAura(SPELL_MUNITION_STABLE, summoned);
                summoned->SetReactState(REACT_PASSIVE);
            }

            summons.Summon(summoned);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_EXPLOSIVES:
                for (uint8 i = 0; i < urand(1, 3); ++i)
                    me->CastSpell(frand(702, 740), frand(2292, 2320), 388.5f, SPELL_PLANT_EXPLOSIVE, true);

                events.ScheduleEvent(EVENT_EXPLOSIVES, urand(7500, 12500));
                break;
            case EVENT_SABOTAGE:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 15.0f, true))
                    me->CastSpell(target, SPELL_SABOTAGE, false);

                events.ScheduleEvent(EVENT_SABOTAGE, urand(22500, 30000));
                break;
            }

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            Talk(SAY_DEATH);
            summons.DespawnAll();

            if (instance)
               instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }
    };
};

class npc_munition_explosion_bunny : public CreatureScript
{
public:
    npc_munition_explosion_bunny() : CreatureScript("npc_munition_explosion_bunny") { }

    enum eEvents
    {
        EVENT_CHECK = 1
    };

    struct npc_munition_explosion_bunnyAI : public ScriptedAI
    {
        npc_munition_explosion_bunnyAI(Creature* creature) : ScriptedAI(creature) {}

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

            std::list<Creature*> munitionList;
            me->GetCreatureListWithEntryInGrid(munitionList, NPC_STABLE_MUNITION, 2.0f);
            for(auto munition : munitionList)
            {
                if (munition->HasAura(SPELL_MUNITION_STABLE))
                {
                    munition->RemoveAurasDueToSpell(SPELL_MUNITION_STABLE);
                    munition->CastSpell(munition, SPELL_MUNITION_EXPLOSION, true);
                    munition->DespawnOrUnsummon(2 * IN_MILLISECONDS);
                }
            }
            events.ScheduleEvent(EVENT_CHECK, 0.5 * IN_MILLISECONDS);

            float x = 0.0f;
            float y = 0.0f;
            GetPositionWithDistInOrientation(me, 40.0f, orientation, x, y);
            Movement::MoveSplineInit init(me);
            init.MoveTo(x, y, me->GetPositionZ());
            init.SetVelocity(10.0f);
            init.Launch();

            me->DespawnOrUnsummon(me->GetSplineDuration());
            me->CastSpell(me, SPELL_MUNITION_EXPLOSION_AURA, false);
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_CHECK)
            {
                std::list<Creature*> munitionList;
                me->GetCreatureListWithEntryInGrid(munitionList, NPC_STABLE_MUNITION, 2.0f);
                if (!munitionList.empty())
                {
                    for(auto munition : munitionList)
                    {
                        if (munition->HasAura(SPELL_MUNITION_STABLE))
                        {
                            munition->RemoveAurasDueToSpell(SPELL_MUNITION_STABLE);
                            munition->CastSpell(munition, SPELL_MUNITION_EXPLOSION, true);
                            munition->DespawnOrUnsummon(2 * IN_MILLISECONDS);
                        }
                    }
                }

                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 2.0f);
                if (!playerList.empty())
                {
                    for(auto player : playerList)
                        me->CastSpell(player, SPELL_MANTID_EXPLOSION, false);
                }

                events.ScheduleEvent(EVENT_CHECK, 0.5 * IN_MILLISECONDS);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_munition_explosion_bunnyAI(creature);
    }
};

class CheckMunitionExplosionPredicate
{
public:
    CheckMunitionExplosionPredicate(Unit* caster) : _caster(caster) {}

    bool operator()(WorldObject* target)
    {
        if (!_caster || !target)
            return true;

        if (!_caster->ToTempSummon())
            return true;

        Unit* creator = _caster->ToTempSummon()->GetSummoner();

        if (!creator || creator == target)
            return true;

        uint32 entry = target->GetEntry();
        if (target->ToCreature() && (entry == NPC_KRITHIK_DEMOLISHER || entry == NPC_KRITHIK_WND_SHAPER || entry == NPC_KRITHIK_INFILTRATOR))
            return true;

        return false;
    }

private:
    Unit* _caster;
};

class spell_kiptilak_munitions_explosion : public SpellScriptLoader
{
public:
    spell_kiptilak_munitions_explosion() : SpellScriptLoader("spell_kiptilak_munitions_explosion") { }

    class spell_kiptilak_munitions_explosion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kiptilak_munitions_explosion_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (Unit* caster = GetCaster())
                unitList.remove_if(CheckMunitionExplosionPredicate(caster));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kiptilak_munitions_explosion_SpellScript::FilterTargets, EFFECT_0, TARGET_SRC_CASTER);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kiptilak_munitions_explosion_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kiptilak_munitions_explosion_SpellScript();
    }
};

class spell_kiptilak_sabotage : public SpellScriptLoader
{
public:
    spell_kiptilak_sabotage() : SpellScriptLoader("spell_kiptilak_sabotage") { }

    class spell_kiptilak_sabotage_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kiptilak_sabotage_AuraScript);

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();

            if (!target)
                return;

            target->CastSpell(target, SPELL_PLAYER_EXPLOSION, true);
            target->CastSpell(target, SPELL_SABOTAGE_EXPLOSION, true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_kiptilak_sabotage_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_kiptilak_sabotage_AuraScript();
    }
};

//8359
class AreaTrigger_at_first_door : public AreaTriggerScript
{
public:
    AreaTrigger_at_first_door() : AreaTriggerScript("at_first_door") {}

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            if (instance->GetData(DATA_OPEN_FIRST_DOOR) != DONE)
            {
                instance->SetData(DATA_OPEN_FIRST_DOOR, DONE);
                if (Unit* kiptilak = Unit::GetCreature(*player, instance->GetData64(DATA_KIPTILAK)))
                    kiptilak->GetAI()->DoAction(ACTION_INTRO);
            }
        }

        return false;
    }
};

class achievement_bomberman_gss : public AchievementCriteriaScript
{
public:
    achievement_bomberman_gss() : AchievementCriteriaScript("achievement_bomberman_gss") { }

    bool OnCheck(uint32 /*criteriaId*/, uint64 /*miscValue*/, Player* source, Unit* /*target*/)
    {
        if (source->GetMap()->GetDifficulty() != HEROIC_DIFFICULTY)
            return false;

        return true;
    }
};

void AddSC_boss_saboteur_kiptilak()
{
    new boss_saboteur_kiptilak();
    new npc_munition_explosion_bunny();
    new spell_kiptilak_munitions_explosion();
    new spell_kiptilak_sabotage();
    new AreaTrigger_at_first_door();
    new achievement_bomberman_gss();
}
