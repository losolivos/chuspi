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

enum
{
    ACTION_BULWARK_BREAKS       = 10,
    NPC_REINFORCEMENTS_STALKER  = 61483,
    NPC_BLADE_RUSH_STALKER      = 63720,
    POINT_BULWARK               = 10
};

static const Position reinforcements[4] = 
{
    {1741.793f, 5338.747f, 136.2742f, 4.686225f },
    {1727.012f, 5342.524f, 138.5872f, 4.686225f },
    {1733.038f, 5332.926f, 136.088f, 4.686225f },
    {1750.8f, 5332.092f, 134.8477f, 4.686225f}
};

class boss_general_pavalak : public CreatureScript
{
    enum Yells
    {
        SAY_INTRO,
        SAY_AGGRO,
        SAY_BULWARK,
        SAY_SLAY,
        SAY_DEATH,
        EMOTE_BULWARK
    };

    enum Spells
    {
        SPELL_BULWARK           = 119476,
        SPELL_BLADE_RUSH_SUMMON = 124278, // summons 63720
        SPELL_BLADE_RUSH_EFF    = 124283, // condition targets 63720
        SPELL_BLADE_RUSH_DUMMY  = 124291,
        SPELL_BLADE_RUSH_CHARGE = 124312,
        SPELL_BLADE_RUSH_DMG_C  = 124317,
        SPELL_BLADE_RUSH_DMG_A  = 124290,
        SPELL_TEMPEST           = 119875
    };

    enum Events
    {
        EVENT_GROUP_COMBAT      = 1,

        EVENT_BLADE_RUSH        = 1,
        EVENT_BLADE_RUSH_CAST,
        EVENT_BLADE_RUSH_CHARGE,
        EVENT_BLADE_RUSH_DAMAGE,
        EVENT_BLADE_RUSH_END,
        EVENT_TEMPEST,
        EVENT_PLAYER_CHECK,
        EVENT_BULWARK_END,
    };

    struct boss_general_pavalakAI : public BossAI
    {
        boss_general_pavalakAI(Creature * creature) : BossAI(creature, BOSS_PAVALAK)
        {
            introDone = false;
        }

        void Reset()
        {
            phase = 0;
            rushTargetGUID = 0;
            me->SetReactState(REACT_AGGRESSIVE);
            _Reset();
        }

        void EnterCombat(Unit* )
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_BLADE_RUSH, 10000, EVENT_GROUP_COMBAT);
            events.ScheduleEvent(EVENT_TEMPEST, 15000, EVENT_GROUP_COMBAT);
            _EnterCombat();
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_BULWARK_BREAKS)
            {
                events.ScheduleEvent(EVENT_BLADE_RUSH, 10000, EVENT_GROUP_COMBAT);
                events.ScheduleEvent(EVENT_TEMPEST, 15000, EVENT_GROUP_COMBAT);
                events.ScheduleEvent(EVENT_PLAYER_CHECK, 5000);
                summons.DespawnEntry(NPC_REINFORCEMENTS_STALKER);
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_BULWARK_END, 200);
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!introDone && who->GetTypeId() == TYPEID_PLAYER && me->IsWithinDist2d(who, 50.0f))
            {
                Talk(SAY_INTRO);
                introDone = true;
            }
            BossAI::MoveInLineOfSight(who);
        }

        void JustDied(Unit* )
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_BLADE_RUSH_DUMMY)
            {
                me->SetFacingToObject(target);
                target->CastSpell(target, 124307, true);
                target->CastSpell(target, SPELL_BLADE_RUSH_DMG_A, true);
                events.ScheduleEvent(EVENT_BLADE_RUSH_CHARGE, 200);
            }

        }

        void DamageTaken(Unit*, uint32& damage)
        {
            if (phase < 2)
            {
                // Use bulwark at 65 and 35 %
                uint32 healthChk = phase ? 35 : 65;

                if (me->HealthBelowPctDamaged(healthChk, damage))
                {
                    damage = 0;
                    me->SetHealth(me->CountPctFromMaxHealth(healthChk));

                    ++phase;
                    me->InterruptNonMeleeSpells(true);
                    events.CancelEventGroup(EVENT_GROUP_COMBAT);
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MovePoint(POINT_BULWARK, 1701.693f, 5242.439f, 123.9606f);

                }
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (id == EVENT_CHARGE)
                {
                    me->StopMoving();
                    me->AttackStop();
                    events.RescheduleEvent(EVENT_BLADE_RUSH_DAMAGE, 200, EVENT_GROUP_COMBAT);
                }
                else if (id == POINT_BULWARK)
                {
                    Talk(SAY_BULWARK);
                    Talk(EMOTE_BULWARK);
                    DoCast(me, SPELL_BULWARK, false);

                    // Reinforcement waves
                    for (auto itr : reinforcements)
                        me->SummonCreature(NPC_REINFORCEMENTS_STALKER, itr);
                }
                    
            }
        }

        void JustSummoned(Creature* summon)
        {
            if (summon->GetEntry() == NPC_BLADE_RUSH_STALKER)
            {
                rushTargetGUID = summon->GetGUID();
                me->SetTarget(summon->GetGUID());
                me->SetFacingToObject(summon);
                events.ScheduleEvent(EVENT_BLADE_RUSH_CAST, 500, EVENT_GROUP_COMBAT);
            }

            BossAI::JustSummoned(summon);
        }

        void UpdateAI(uint32 const diff)
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
                    case EVENT_BLADE_RUSH:
                    {
                        me->SetReactState(REACT_PASSIVE);

                        //DoCast(SELECT_TARGET_RANDOM, SPELL_BLADE_RUSH_SUMMON, true);
                        Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f, true);
                        if (!target)
                            target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true);
                        if (target)
                        {
                            events.DelayEvents(15000);
                            me->SummonCreature(NPC_BLADE_RUSH_STALKER, *target, TEMPSUMMON_TIMED_DESPAWN, 15000);
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MoveIdle();
                            me->StopMoving();
                            me->AttackStop();
                            events.ScheduleEvent(EVENT_BLADE_RUSH_END, 30000, EVENT_GROUP_COMBAT);
                        }

                        events.ScheduleEvent(EVENT_BLADE_RUSH, 30000, EVENT_GROUP_COMBAT);
                        break;
                    }
                    case EVENT_BLADE_RUSH_CAST:
                        if (Creature * rushTarget = Creature::GetCreature(*me, rushTargetGUID))
                            DoCast(rushTarget, SPELL_BLADE_RUSH_EFF, false);
                        break;
                    case EVENT_BLADE_RUSH_CHARGE:
                        if (Creature * rushTarget = Creature::GetCreature(*me, rushTargetGUID))
                            DoCast(rushTarget, SPELL_BLADE_RUSH_CHARGE, false);
                        break;
                    case EVENT_BLADE_RUSH_DAMAGE:
                        //DoCast(me, SPELL_BLADE_RUSH_DMG_C, true);
                        if (Creature * rushTarget = Creature::GetCreature(*me, rushTargetGUID))
                            rushTarget->RemoveAllAuras();
                        events.RescheduleEvent(EVENT_BLADE_RUSH_END, 1000, EVENT_GROUP_COMBAT);
                        break;
                    case EVENT_BLADE_RUSH_END:
                        rushTargetGUID = 0;
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (Unit * victim = me->GetVictim())
                            AttackStart(victim);
                        break;
                    case EVENT_TEMPEST:
                        DoCast(me, SPELL_TEMPEST, false);
                        events.ScheduleEvent(EVENT_TEMPEST, 20000, EVENT_GROUP_COMBAT);
                        break;
                    case EVENT_BULWARK_END:
                        if (Unit * victim = me->GetVictim())
                            me->GetMotionMaster()->MoveChase(victim);
                        break;
                    case EVENT_PLAYER_CHECK:
                    {
                        bool playersAlive = false;
                        Map::PlayerList const &players = me->GetMap()->GetPlayers();
                        for (auto itr = players.begin(); itr != players.end(); ++itr)
                        {
                            if (Player* player = itr->GetSource())
                                if (player->IsAlive() && !player->isGameMaster() && player->GetAreaId() == 6411)
                                {
                                    playersAlive = true;
                                    break;
                                }
                        }

                        if (!playersAlive)
                            EnterEvadeMode();
                        else
                            events.ScheduleEvent(EVENT_PLAYER_CHECK, 5000);
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        bool introDone;
        int8 phase;
        uint64 rushTargetGUID;
    };
public:
    boss_general_pavalak() : CreatureScript("boss_general_pavalak") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new boss_general_pavalakAI(creature);
    }
};

// Bulwark - 119476
class spell_general_pavalak_bulwark : public SpellScriptLoader
{

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
                if (Creature * creatureTarget = target->ToCreature())
                    creatureTarget->AI()->DoAction(ACTION_BULWARK_BREAKS);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(aura_impl::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        }
    };

public:
    spell_general_pavalak_bulwark() : SpellScriptLoader("spell_general_pavalak_bulwark") {}

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

// Siege Explosive -- 61452
class npc_pavalak_siege_explosive : public CreatureScript
{
    enum
    {
        SPELL_SIEGE_EXPLOSIVE_PARENT = 119388,
        // 40:07
        SPELL_ARMING_VISUAL_YELLOW  = 88315, // 00:01 40:08
        SPELL_ARMING_VISUAL_ORANGE  = 88316, // 00:02 40:10
        SPELL_ARMING_VISUAL_RED     = 88317, // 00:03 40:11
        SPELL_BOMB_ARMED            = 119702,// 00:05 40:12
        SPELL_DETONATE              = 119703,// 00:09 40:16

        EVENT_ARMING_YELLOW = 1,
        EVENT_ARMING_ORANGE,
        EVENT_ARMING_RED,
        EVENT_ARMED,
        EVENT_DETONATE,
    };

    struct npc_pavalak_siege_explosiveAI : public ScriptedAI
    {
        npc_pavalak_siege_explosiveAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit*)
        {
            events.ScheduleEvent(EVENT_ARMING_YELLOW, 1000);
        }

        void OnSpellClick(Unit* clicker, bool &)
        {
            //clicker->RemoveAurasDueToSpell(SPELL_SIEGE_EXPLOSIVE_PARENT);
            me->DisappearAndDie();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ARMING_YELLOW:
                        DoCast(me, SPELL_ARMING_VISUAL_YELLOW, true);
                        events.ScheduleEvent(EVENT_ARMING_ORANGE, 1000);
                        break;
                    case EVENT_ARMING_ORANGE:
                        DoCast(me, SPELL_ARMING_VISUAL_ORANGE, true);
                        events.ScheduleEvent(EVENT_ARMING_RED, 1000);
                        break;
                    case EVENT_ARMING_RED:
                        DoCast(me, SPELL_ARMING_VISUAL_RED, true);
                        events.ScheduleEvent(EVENT_ARMED, 2000);
                        break;
                    case EVENT_ARMED:
                        DoCast(me, SPELL_BOMB_ARMED, true);
                        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        events.ScheduleEvent(EVENT_DETONATE, 4000);
                        break;
                    case EVENT_DETONATE:
                        DoCast(me, SPELL_DETONATE, true);
                        me->DespawnOrUnsummon(500);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
    };

public:
    npc_pavalak_siege_explosive() : CreatureScript("npc_pavalak_siege_explosive") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_pavalak_siege_explosiveAI(creature);
    }
};

// Sik'Thik Amber-Sapper - 61484
class npc_pavalak_amber_sapper : public CreatureScript
{
    enum
    {
        SPELL_CARRYING_EXPLOSIVES       = 119698,
        SPELL_SIEGE_EXPLOSIVE_MISSILE   = 119376,
        POINT_TARGET                    = 10,
        POINT_RETURN
    };

    struct npc_pavalak_amber_sapperAI : public ScriptedAI
    {
        npc_pavalak_amber_sapperAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit* summoner)
        {
            DoCast(me, SPELL_CARRYING_EXPLOSIVES, true);
            Position pos;
            summoner->GetRandomNearPosition(pos, 40.0f);
            pos.m_positionZ = 143.65f;

            me->GetMotionMaster()->MovePoint(POINT_TARGET, pos);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE && id == POINT_TARGET)
            {
                DoCast(me, SPELL_SIEGE_EXPLOSIVE_MISSILE, false);
                me->RemoveAurasDueToSpell(SPELL_CARRYING_EXPLOSIVES);
                moveTimer = 2000;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!moveTimer)
                return;

            if (moveTimer <= diff)
            {
                me->GetMotionMaster()->MoveTargetedHome();
                me->DespawnOrUnsummon(5000);
                moveTimer = 0;
            } else moveTimer -= diff;
        }

    private:
        uint32 moveTimer;
    };

public:
    npc_pavalak_amber_sapper() : CreatureScript("npc_pavalak_amber_sapper") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_pavalak_amber_sapperAI(creature);
    }
};


static const Position sapperPos = { 1837.330f, 5235.642f, 176.60643f, 2.62f };
// bombardment - 119512
class spell_pavalak_bombardment : public SpellScriptLoader
{
    enum
    {
        NPC_AMBER_SAPPER            = 61484
    };
    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void HandlePeriodic(AuraEffect const * /*aurEff*/)
        {
            if (Unit * target = GetTarget())
            {
                Position pos = sapperPos;
                float dist = frand(20.0f, 50.0f);
                float angle = (float)rand_norm() * static_cast<float>(2 * M_PI);
                pos.m_positionX += dist * std::cos(angle);
                pos.m_positionY += dist * std::sin(angle);

                target->SummonCreature(NPC_AMBER_SAPPER, pos);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(aura_impl::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

public:
    spell_pavalak_bombardment() : SpellScriptLoader("spell_pavalak_bombardment") {}

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

// Reinforcements Summoner - 61483
// Summon Proxy - > Trigger JustSummoner in boss script
class npc_pavalak_reinforcements_summoner : public CreatureScript
{
    struct npc_pavalak_reinforcements_summonerAI : public ScriptedAI
    {
        npc_pavalak_reinforcements_summonerAI(Creature * creature) : ScriptedAI(creature)
        {
            summonerGUID = 0;
        }

        void IsSummonedBy(Unit* summoner)
        {
            summonerGUID = summoner->GetGUID();
        }

        void JustSummoned(Creature* summon)
        {
            if (Creature * pavalak = Creature::GetCreature(*me, summonerGUID))
                pavalak->AI()->JustSummoned(summon);
        }

    private:
        uint64 summonerGUID;
    };

public:
    npc_pavalak_reinforcements_summoner() : CreatureScript("npc_pavalak_reinforcements_summoner") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_pavalak_reinforcements_summonerAI(creature);
    }
};

void AddSC_general_pavalak()
{
    new boss_general_pavalak();
    new spell_general_pavalak_bulwark();
    new npc_pavalak_siege_explosive();
    new npc_pavalak_amber_sapper();
    new spell_pavalak_bombardment();
    new npc_pavalak_reinforcements_summoner();
}