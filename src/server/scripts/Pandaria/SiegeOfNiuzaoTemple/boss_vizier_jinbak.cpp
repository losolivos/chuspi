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

enum
{
    SPELL_GROW                      = 119942,
    SPELL_SAPPLING_SUMMON_VISUAL    = 120071,

    NPC_GLOBULE_SUMMON_DEST         = 61629,
    NPC_SAP_PUDDLE                  = 61613,
    NPC_SAPPLING                    = 61623
};

typedef std::pair<float, uint64> SapplingTrigger;

const Position centerPos = { 1529.520020f, 5163.240234f, 160.570999f, 0.0f };

class boss_vizier_jinbak : public CreatureScript
{
    enum Yells
    {
        SAY_INTRO,
        SAY_AGGRO,
        SAY_DEATH,
        SAY_SLAY,
        SAY_DETONATE,
        EMOTE_GLOBULE,
    };

    enum Spells
    {
        SPELL_DETONATE                  = 120001,
        SPELL_DETONATE_VISUAL           = 120095,
        SPELL_GROW_STACKER              = 119937,
        SPELL_SUMMON_SAPLING            = 119990
    };

    enum Events
    {
        EVENT_DETONATE                  = 1,
        EVENT_SUMMON_GLOBULES
    };

    struct boss_vizier_jinbakAI : public BossAI
    {
        boss_vizier_jinbakAI(Creature * creature) : BossAI(creature, BOSS_JINBAK)
        {
            visualTimer = 5000;
            puddleGUID = 0;
            introDone = false;
        }

        void Reset()
        {
            evadeCheckCooldown = 10000;
            events.Reset();

            if (Creature * trigger = Creature::GetCreature(*me, puddleGUID))
                trigger->ToCreature()->AI()->EnterEvadeMode();

            ClearDebuffs();

            summonDestList.clear();

            visualTimer = 5000;
            _Reset();
        }

        void ClearDebuffs()
        {
            std::list<Creature *> stalkers;
            me->GetCreatureListWithEntryInGrid(stalkers, NPC_GLOBULE_SUMMON_DEST, 500.0f);
            if (!stalkers.empty())
                for (auto itr : stalkers)
                    itr->RemoveAllAuras();

            std::list<Creature *> globules;
            me->GetCreatureListWithEntryInGrid(globules, NPC_SAPPLING, 500.0f);
            if (!globules.empty())
                for (auto itr : globules)
                    itr->DespawnOrUnsummon(500);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!introDone && who->GetTypeId() == TYPEID_PLAYER)
            {
                if (me->IsWithinDist2d(who, 30.0f))
                {
                    introDone = true;
                    Talk(SAY_INTRO);
                }
            }
            BossAI::MoveInLineOfSight(who);
        }

        void EnterCombat(Unit* )
        {
            me->InterruptNonMeleeSpells(true);
            visualTimer = 0;

            // Find nearby globule summon targets and sort by angle from centre -> npc
            std::list<Creature *> destList;
            me->GetCreatureListWithEntryInGrid(destList, NPC_GLOBULE_SUMMON_DEST, 500.0f);
            Creature * sapPuddle = me->FindNearestCreature(NPC_SAP_PUDDLE, 500.0f);

            ASSERT(sapPuddle != NULL);

            puddleGUID = sapPuddle->GetGUID();

            sapPuddle->CastSpell(sapPuddle, SPELL_GROW_STACKER, true);

            for (auto itr : destList)
            {
                float angle = sapPuddle->GetAngle(itr);
                summonDestList.push_back(SapplingTrigger(angle, itr->GetGUID()));
            }

            summonDestList.sort([](SapplingTrigger t1, SapplingTrigger t2) { return t1.first < t2.first; });

            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_SUMMON_GLOBULES, 10000);
            events.ScheduleEvent(EVENT_DETONATE, 30000);
            _EnterCombat();
        }

        void JustSummoned(Creature* summon)
        {
            if (summon->GetEntry() == NPC_SAPPLING)
            {
                if (Creature * puddle = Creature::GetCreature(*me, puddleGUID))
                {
                    Movement::MoveSplineInit init(summon);
                    summon->SetReactState(REACT_PASSIVE);
                    float angle = summon->GetAngle(&centerPos);
                    float dist = summon->GetExactDist2d(&centerPos);
                    float xPerStep = dist * std::cos(angle) / 10.0f;
                    float yPerStep = dist * std::sin(angle) / 10.0f;

                    float x, y, z;
                    summon->GetPosition(x, y, z);
                    for (float i = 0; i < 10.0f; ++i)
                    {
                        x += xPerStep;
                        y += yPerStep;
                        z = summon->GetMap()->GetHeight(summon->GetPhaseMask(), x, y, 160.0f);
                        init.Path().push_back(G3D::Vector3(x, y, z));
                    }

                    init.SetWalk(true);
                    init.SetVelocity(5.0f);
                    init.Launch();
                }
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void JustDied(Unit* killer)
        {
            Talk(SAY_DEATH);
            if (Creature * puddle = Creature::GetCreature(*me, puddleGUID))
                puddle->DespawnOrUnsummon();
            ClearDebuffs();
            _JustDied();
        }

        void UpdateAI(const uint32 diff)
        {
            if (visualTimer)
            {
                if (visualTimer <= diff)
                {
                    DoCast(me, SPELL_DETONATE_VISUAL, false);
                    visualTimer = 0;
                } else visualTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DETONATE:
                        Talk(SAY_DETONATE);
                        DoCast(me, SPELL_DETONATE, false);
                        events.ScheduleEvent(EVENT_DETONATE, 45500);
                        break;
                    case EVENT_SUMMON_GLOBULES:
                    {
                        // We need to summon them at random 3 dest NPCS
                        // The summon position pattern looks similar to the Mercedes Logo
                        // First, get a random starting point
                        auto itr = summonDestList.begin();
                        std::advance(itr, urand(0, summonDestList.size() / 3));
                        float lastAngle = -10.0f;
                        float minSectorAngle = 100.0f * M_PI / 180.0f;
                        uint8 count = 3;

                        for (int i = 0; i < 13; ++i)
                        {
                            if (itr->first < lastAngle)
                            {
                                // passed the 2xPI
                                lastAngle -= M_PI * 2.0f;
                            }

                            if (fabs(itr->first - lastAngle) > minSectorAngle)
                            {
                                lastAngle = itr->first;
                                if (Creature * trigger = Creature::GetCreature(*me, itr->second))
                                    DoCast(trigger, SPELL_SUMMON_SAPLING, true);

                                if (!--count)
                                    break;
                            }

                            if (++itr == summonDestList.end())
                                itr = summonDestList.begin();

                        }
                        Talk(EMOTE_GLOBULE);
                        events.ScheduleEvent(EVENT_SUMMON_GLOBULES, 45500);

                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();

            // Room check
            if (evadeCheckCooldown <= diff)
                evadeCheckCooldown = 2500;
            else
            {
                evadeCheckCooldown -= diff;
                return;
            }

            if (!me->IsWithinDist2d(1529.520020f, 5163.240234f, 30.0f))
                EnterEvadeMode();
        }

    private:
        uint32 evadeCheckCooldown;
        uint32 visualTimer;
        std::list<SapplingTrigger > summonDestList;
        uint64 puddleGUID;
        bool introDone;
    };
public:
    boss_vizier_jinbak() : CreatureScript("boss_vizier_jinbak") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new boss_vizier_jinbakAI(creature);
    }
};


class npc_sap_globule : public CreatureScript
{
    enum Spells
    {
        SPELL_SAP_RESIDUE           = 120070,

    };

    struct npc_sap_globuleAI : public ScriptedAI
    {
        npc_sap_globuleAI(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            done = false;
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (Creature * jinbak = me->FindNearestCreature(NPC_JINBAK, 500.0f))
                jinbak->AI()->JustSummoned(me);
        }

        void SpellHit(Unit * caster, SpellInfo const* spell)
        {
            if (!done && spell->Id == SPELL_SAP_RESIDUE)
            {
                done = true;
                caster->ToCreature()->AI()->DoAction(1);
                DoCast(me, SPELL_SAPPLING_SUMMON_VISUAL, true);
                me->DespawnOrUnsummon(500);
            }
        }

        void UpdateAI(uint32 const diff)
        {
        }
    private:
        bool done;
    };

public:
    npc_sap_globule() : CreatureScript("npc_sap_globule") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sap_globuleAI(creature);
    }
};


class npc_sap_puddle : public CreatureScript
{
    enum
    {
        SPELL_RESIDUE_DOT           = 119941
    };
    struct npc_sap_puddleAI : public ScriptedAI
    {
        npc_sap_puddleAI(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            skipTick = 0;
        }

        void DoAction(const int32 action)
        {
            if (action == 1)
            {
                skipTick = 3000;
                Aura * growAura = me->GetAura(SPELL_GROW);
                if (!growAura)
                {
                    const SpellInfo * spell = sSpellMgr->GetSpellInfo(SPELL_GROW);
                    growAura = Aura::TryCreate(spell, MAX_EFFECT_MASK, me, me, &spell->spellPower);
                }

                if (growAura)
                {
                    growAura->ModStackAmount(3); // TODO: find proper value
                }
                else
                    me->MonsterYell("failed to create stack", 0, 0);
            }
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (!skipTick && target->GetTypeId() == TYPEID_PLAYER && spell->Id == SPELL_RESIDUE_DOT)
                if (Aura * growAura = me->GetAura(SPELL_GROW))
                {
                    int32 stackAmt = growAura->GetStackAmount();
                    growAura->SetStackAmount(std::max(stackAmt - 3, 1));// 3 Per player each tick
                }
        }

        void UpdateAI(uint32 const diff)
        {
            if (skipTick)
            {
                if (skipTick <= diff)
                {
                    skipTick = 0;
                } else skipTick -= diff;
            }
        }
    private:
        uint32 skipTick;
    };


public:
    npc_sap_puddle() : CreatureScript("npc_sap_puddle") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sap_puddleAI(creature);
    }
};


class spell_sap_residue : public SpellScriptLoader
{
public:
    spell_sap_residue() : SpellScriptLoader("spell_sap_residue") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (Unit * caster = GetCaster())
            {
                //float dist = 2.0f * caster->GetAuraCount(SPELL_GROW);

                float dist = 5.0f;

                if (AuraEffect* eff = caster->GetAuraEffect(SPELL_GROW, EFFECT_0))
                    dist *= eff->GetAmount() / 100.0f;

                if (!targets.empty())
                    targets.remove_if([&caster, &dist](const WorldObject * unit) { return unit->GetExactDist2d(caster) > dist; });
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

// Detonate - 120001
class spell_jinbak_detonate : public SpellScriptLoader
{
    enum
    {
        SPELL_DETONATE_DAMAGE           = 120002
    };
    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void OnTick(AuraEffect const *aurEff)
        {
            if (Unit * target = GetTarget())
            {
                uint32 stackAmount = std::max<uint32>(1, target->GetAuraCount(SPELL_GROW));
                bool isHeroic = target->GetMap()->GetDifficulty() != REGULAR_DIFFICULTY;
                int32 damage = (isHeroic ? 25500 : 8500) * stackAmount;
                target->CastCustomSpell(target, SPELL_DETONATE_DAMAGE, &damage, NULL, NULL, true);
                target->RemoveAurasDueToSpell(SPELL_GROW);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(aura_impl::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

public:
    spell_jinbak_detonate() : SpellScriptLoader("spell_jinbak_detonate") {}

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

void AddSC_vizier_jinbak()
{
    new boss_vizier_jinbak();
    new npc_sap_globule();
    new npc_sap_puddle();
    new spell_sap_residue();
    new spell_jinbak_detonate();
}