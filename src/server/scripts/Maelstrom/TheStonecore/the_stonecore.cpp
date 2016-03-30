/*
 * Copyright (C) 2010-2011 Project Trinity <http://www.projecttrinity.org/>
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

// 0551_world_stonecore.sql
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "InstanceScript.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "CombatAI.h"
#include "ScriptedGossip.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "the_stonecore.h"

class npc_stonecore_berserker : public CreatureScript
{
    enum
    {
        EVENT_CHARGE            = 1,
        EVENT_SPINNING_SLASH,
        EVENT_SPINNING_SLASH_CD,

        SPELL_CHARGE            = 81574,
        SPELL_SPINNING_SLASH    = 81568
    };

    struct npc_stonecore_berserkerAI : public ScriptedAI
    {
        npc_stonecore_berserkerAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            events.Reset();
            slashReady = true;
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_CHARGE, 100);
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
                case EVENT_CHARGE:
                    if (Unit * target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 0.0f, true))
                    {
                        DoResetThreat();
                        DoCast(target, SPELL_CHARGE);
                        me->AddThreat(target, 10000.0f);
                    }
                    if (slashReady)
                        events.ScheduleEvent(EVENT_SPINNING_SLASH, 1000);
                    events.ScheduleEvent(EVENT_CHARGE, urand(8000, 10000));
                    break;
                case EVENT_SPINNING_SLASH:
                    slashReady = false;
                    DoCast(me, SPELL_SPINNING_SLASH);
                    events.ScheduleEvent(EVENT_SPINNING_SLASH_CD, 12000);
                    break;
                case EVENT_SPINNING_SLASH_CD:
                    slashReady = true;
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        bool slashReady;
    };

public:
    npc_stonecore_berserker() : CreatureScript("npc_stonecore_berserker") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stonecore_berserkerAI(creature);
    }
};

class npc_stonecore_earthshaper : public CreatureScript
{
    enum
    {
        NPC_EARTHSHAPER         = 43537,
        NPC_FORCE_OF_EARTH      = 43552,
        DISPLAY_FORCE_OF_EARTH  = 26693,
        SPELL_FORCE_OF_EARTH    = 81459,
        SPELL_DUST_STORM        = 81463,
        SPELL_GROUND_SHOCK      = 81530,
        SPELL_LAVA_BURST        = 81576,

        SAY_FORCE_OF_EARTH      = 1,
        EVENT_FORCE_OF_EARTH,
        EVENT_DUST_STORM,
        EVENT_GROUND_SHOCK,
        EVENT_LAVA_BURST
    };

    struct npc_stonecore_earthshaperAI : public ScriptedAI
    {
        npc_stonecore_earthshaperAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            events.Reset();
            me->SetEntry(NPC_EARTHSHAPER);
            me->RestoreDisplayId();
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_FORCE_OF_EARTH, urand(500, 2500));
            events.ScheduleEvent(EVENT_GROUND_SHOCK, urand(5000, 7000));
            events.ScheduleEvent(EVENT_LAVA_BURST, urand(2000, 3000));
        }

        void SpellHit(Unit * /*caster*/, const SpellInfo * spell)
        {
            if (spell->Id == SPELL_FORCE_OF_EARTH)
            {
                me->SetEntry(NPC_FORCE_OF_EARTH);
                me->SetDisplayId(DISPLAY_FORCE_OF_EARTH);
                events.CancelEvent(EVENT_FORCE_OF_EARTH);
                events.ScheduleEvent(EVENT_DUST_STORM, 2000);
            }
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
                case EVENT_DUST_STORM:
                    DoCast(me, SPELL_DUST_STORM);
                    events.ScheduleEvent(EVENT_DUST_STORM, urand(18000, 22000));
                    break;
                case EVENT_GROUND_SHOCK:
                    DoCast(me, SPELL_GROUND_SHOCK);
                    events.ScheduleEvent(EVENT_GROUND_SHOCK, urand(8000, 10000));
                    break;
                case EVENT_FORCE_OF_EARTH:
                    Talk(SAY_FORCE_OF_EARTH);
                    DoCast(me, SPELL_FORCE_OF_EARTH, false);
                    events.ScheduleEvent(EVENT_FORCE_OF_EARTH, urand(15000, 20000));
                    break;
                case EVENT_LAVA_BURST:
                    DoCastVictim(SPELL_LAVA_BURST, false);
                    events.ScheduleEvent(EVENT_LAVA_BURST, 8000);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
    };

public:
    npc_stonecore_earthshaper() : CreatureScript("npc_stonecore_earthshaper") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stonecore_earthshaperAI(creature);
    }
};

// Custom aura - ticks every 500 ms
static const Emote randomEmotes[6] = { EMOTE_ONESHOT_LAUGH, EMOTE_ONESHOT_YES, EMOTE_ONESHOT_TALK, EMOTE_ONESHOT_QUESTION, EMOTE_ONESHOT_NO, EMOTE_ONESHOT_EXCLAMATION };

class spell_stonecore_social_aura : public SpellScriptLoader
{
    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        uint64 nextTick;

        bool Load()
        {
            nextTick = urand(1, 20);
            return true;
        }

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            if (aurEff->GetTickNumber() == nextTick)
            {
                GetTarget()->HandleEmoteCommand(randomEmotes[urand(0, 5)]);
                nextTick = aurEff->GetTickNumber() + urand(10, 14);
            }
        }

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->SetStandState(UNIT_STAND_STATE_SIT);
        }

        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->SetStandState(UNIT_STAND_STATE_STAND);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(aura_impl::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            OnEffectApply += AuraEffectApplyFn(aura_impl::HandleEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectApplyFn(aura_impl::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

public:
    spell_stonecore_social_aura() : SpellScriptLoader("spell_stonecore_social_aura") { }

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

class npc_crystalspawn_giant : public CreatureScript
{
    enum
    {
        SPELL_QUAKE             = 81008,
        SPELL_CRYSTAL_SHARD     = 81015,
        SPELL_CRYSTAL_SHARD_2   = 92150,
        NPC_PRE_EFFECT          = 49473,

        EVENT_CRYSTAL_SHARD     = 1,
        EVENT_QUAKE,
    };

    class CrystalSpawnEvent : public BasicEvent
    {
    public:
        CrystalSpawnEvent(Creature& owner) : BasicEvent(), m_owner(owner) { }

        bool Execute(uint64 /*eventTime*/, uint32 /*diff*/)
        {
            m_owner.RemoveAllAuras();
            m_owner.CastSpell(m_owner, SPELL_CRYSTAL_SHARD_2, true);
            return true;
        }

    private:
        Creature& m_owner;
    };

    struct npc_crystalspawn_giantAI : public ScriptedAI
    {
        npc_crystalspawn_giantAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_QUAKE, urand(5000, 8000));
            events.ScheduleEvent(EVENT_CRYSTAL_SHARD, urand(8000, 10000));
        }

        void JustSummoned(Creature * summon)
        {
            if (summon->GetEntry() == NPC_PRE_EFFECT)
            {
                summon->DespawnOrUnsummon(10000);
                summon->m_Events.AddEvent(new CrystalSpawnEvent(*summon), summon->m_Events.CalculateTime(2000));
            }
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
                case EVENT_QUAKE:
                    DoCast(me, SPELL_QUAKE);
                    events.ScheduleEvent(EVENT_QUAKE, urand(10000, 15000));
                    break;
                case EVENT_CRYSTAL_SHARD:
                    DoCast(SELECT_TARGET_RANDOM, SPELL_CRYSTAL_SHARD);
                    events.ScheduleEvent(EVENT_CRYSTAL_SHARD, urand(12000, 15000));
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
    };

public:
    npc_crystalspawn_giant() : CreatureScript("npc_crystalspawn_giant") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crystalspawn_giantAI(creature);
    }
};

class spell_stonecore_quake : public SpellScriptLoader
{
    class JumpFilter
    {
    public:
        JumpFilter() { }

        bool operator()(WorldObject* target)
        {
            return target->ToUnit()->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING);
        }
    };

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl)

        void FilterTargets(std::list<WorldObject*> &objList)
        {
            objList.remove_if(JumpFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };
public:
    spell_stonecore_quake() : SpellScriptLoader("spell_stonecore_quake") { }

    SpellScript * GetSpellScript() const
    {
        return new spell_impl();
    }
};

class npc_crystal_shard_trash : public CreatureScript
{
    enum
    {
        SPELL_CRYSTAL_SHARDS_TARGET     = 80912,
        SPELL_CRYSTAL_SHARDS_DAMAGE     = 80913
    };

    struct npc_crystal_shard_trashAI : public ScriptedAI
    {
        npc_crystal_shard_trashAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() { }

        void IsSummonedBy(Unit * )
        {
            DoZoneInCombat();
            me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
        }

        void SpellHitTarget(Unit * /*target*/, const SpellInfo * spell)
        {
            if (spell->Id == SPELL_CRYSTAL_SHARDS_TARGET)
            {
                DoCast(SPELL_CRYSTAL_SHARDS_DAMAGE);
                me->DespawnOrUnsummon(100);
            }
        }

        void UpdateAI(uint32 const )
        {
            UpdateVictim();
        }
    };

public:
    npc_crystal_shard_trash() : CreatureScript("npc_crystal_shard_trash") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_crystal_shard_trashAI(creature);
    }
};

class npc_stonecore_bruiser : public CreatureScript
{
    enum
    {
        EVENT_SHOCKWAVE         = 1,
        EVENT_BODY_SLAM,

        SPELL_BODY_SLAM         = 80180,
        SPELL_SHOCKWAVE         = 80195
    };

    struct npc_stonecore_bruiserAI : public ScriptedAI
    {
        npc_stonecore_bruiserAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            events.Reset();
            SetCombatMovement(true);
            bodySlamStep = -1;
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_BODY_SLAM, urand(8000, 10000));
            events.ScheduleEvent(EVENT_SHOCKWAVE, urand(5000, 7000));
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
                case EVENT_BODY_SLAM:
                    if (bodySlamStep == -1)
                    {
                        ++bodySlamStep;
                        SetCombatMovement(false);
                        me->StopMoving();
                    }
                    if (bodySlamStep < 5)
                    {
                        DoCast(SELECT_TARGET_RANDOM, SPELL_BODY_SLAM, false, 0, -5.0f);
                        events.ScheduleEvent(EVENT_BODY_SLAM, 2000);
                        ++bodySlamStep;
                    }
                    else
                    {
                        bodySlamStep = -1;
                        SetCombatMovement(true);
                        events.ScheduleEvent(EVENT_BODY_SLAM, urand(15000, 18000));
                    }
                    break;
                case EVENT_SHOCKWAVE:
                    DoCast(me, SPELL_SHOCKWAVE);
                    events.ScheduleEvent(EVENT_SHOCKWAVE, urand(8000, 12000));
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        int8 bodySlamStep;
    };

public:
    npc_stonecore_bruiser() : CreatureScript("npc_stonecore_bruiser") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stonecore_bruiserAI(creature);
    }
};

class npc_stonecore_magmalord : public CreatureScript
{
    enum
    {
        SPELL_IGNITE            = 80151,
        SPELL_MAGMA_ERUPTION    = 80038,

        EVENT_IGNITE            = 1,
        EVENT_MAGMA_ERUPTION,
    };

    struct npc_stonecore_magmalordAI : public ScriptedAI
    {
        npc_stonecore_magmalordAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_IGNITE, urand(2000, 3000));
            events.ScheduleEvent(EVENT_MAGMA_ERUPTION, urand(3000, 5000));
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
                case EVENT_IGNITE:
                    DoCastVictim(SPELL_IGNITE);
                    events.ScheduleEvent(EVENT_IGNITE, urand(8000, 10000));
                    break;
                case EVENT_MAGMA_ERUPTION:
                    DoCast(me, SPELL_MAGMA_ERUPTION);
                    events.ScheduleEvent(EVENT_MAGMA_ERUPTION, urand(12000, 15000));
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
    };

public:
    npc_stonecore_magmalord() : CreatureScript("npc_stonecore_magmalord") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stonecore_magmalordAI(creature);
    }
};

class npc_stonecore_imp : public CreatureScript
{
    struct npc_stonecore_impAI : public CasterAI
    {
        npc_stonecore_impAI(Creature* c) : CasterAI(c) { }
    };

public:
    npc_stonecore_imp() : CreatureScript("npc_stonecore_imp") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stonecore_impAI(creature);
    }
};

// Rock Borer AI
class npc_rock_borer : public CreatureScript
{
    enum
    {
        SPELL_ROCK_BORE          = 80028,
        SPELL_ROCK_BORE_HC       = 92630,
    };

    struct npc_rock_borerAI : public ScriptedAI
    {
        npc_rock_borerAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            rockboreTimer = urand(5000, 7000);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (rockboreTimer <= diff)
            {
                if (Unit * victim = me->GetVictim())
                {
                    if (Aura * aur = victim->GetAura(IsHeroic() ? SPELL_ROCK_BORE_HC : SPELL_ROCK_BORE))
                    {
                        aur->ModStackAmount(1);
                        aur->RefreshDuration();
                    }
                    else
                        DoCastVictim(SPELL_ROCK_BORE);
                }
                rockboreTimer = urand(10000, 12000);
            }else rockboreTimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        uint32 rockboreTimer;
    };

public:
    npc_rock_borer() : CreatureScript("npc_rock_borer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rock_borerAI(creature);
    }
};

class npc_stonecore_teleporter : public CreatureScript
{
    enum
    {
        NPC_TELEPORTER_ENTRANCE         = 51396,
        NPC_TELEPORTER_SLABHIDE         = 51397
    };

public:
    virtual bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript * instance = player->GetInstanceScript())
        {
            if (creature->GetEntry() == NPC_TELEPORTER_SLABHIDE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "The Winding Halls", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if (creature->GetEntry() == NPC_TELEPORTER_ENTRANCE && (instance->GetBossState(DATA_SLABHIDE) == DONE || player->isGameMaster()))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "The Overlook", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        return true;
    }

    virtual bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 sender, uint32 action)
    {
        if (sender != GOSSIP_SENDER_MAIN)
            return false;

        if (action == GOSSIP_ACTION_INFO_DEF + 1)
            player->NearTeleportTo(853.8577f, 999.7518f, 318.3986f, 0.0f); // Entrance
        else
            player->NearTeleportTo(1313.26f, 1236.833f, 248.2859f, 4.310963f); // Slabhide area

        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    npc_stonecore_teleporter() : CreatureScript("npc_stonecore_teleporter") { }
};


static const Position advancePos[3] =
{
    {983.225281f,  890.187134f, 304.697876f, 2.85937f},
    {1057.792358f, 867.816589f, 293.912659f, 2.85937f},
    {1151.198486f, 885.426575f, 284.963074f, 3.14f}
};

class npc_stonecore_millhouse_manastorm : public CreatureScript
{
    enum
    {
        SPELL_BLUR                  = 81216,
        SPELL_FEAR                  = 81442,
        SPELL_FROSTBOLT_VOLLEY      = 81440,
        SPELL_IMPENDING_DOOOOM      = 86830,
        SPELL_IMPENDING_DOOM_GROW   = 86838,
        SPELL_SHADOW_BOLT           = 81439,
        SPELL_SHADOWFURY            = 81441,
        SPELL_SPECIAL_BLEND         = 81220,
        SPELL_WYRM_CHARGE           = 81237,
        SPELL_DOOR_BREAK            = 81232,
        SPELL_READYUNARMED          = 94610,

        EVENT_FEAR                  = 1,
        EVENT_FROSTBOLT_VOLLEY,
        EVENT_SHADOW_BOLT,
        EVENT_SHADOWFURY,
        EVENT_IMPENDING_DOOM,
        EVENT_IMPENDING_DOOM_CANCEL,
        EVENT_BLUR,
        EVENT_BLUR_ADVANCE,
        EVENT_SPECIAL_BLEND,
        EVENT_SPECIAL_BLEND_END,
        EVENT_SAY_DOOM,
        EVENT_BREAK_WALL,
        EVENT_CORBORUS_CHARGE,
        EVENT_MASS_KNOCKBACK,
        EVENT_NEXT_POINT,

        POINT_NEXT                  = 1,
        POINT_FIRST_PACK,
        POINT_SECOND_PACK,
        POINT_CORBORUS,

        SAY_DOOM_1                  = 3,
        SAY_DOOM_2,
        SAY_DOOM_ALTERNATIVE
    };

    struct npc_stonecore_millhouse_manastormAI : public ScriptedAI
    {
        npc_stonecore_millhouse_manastormAI(Creature* c) : ScriptedAI(c)
        {
            stage = 0;
            castingDoom = false;
            isAdvancing = false;
            me->setActive(true);
            instance = c->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            //SetCombatMovement(false);
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_FEAR, 100);
            events.ScheduleEvent(EVENT_SHADOW_BOLT, 500);
            events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(5000, 7000));
            events.ScheduleEvent(EVENT_SHADOWFURY, urand(7000, 10000));
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_MILLHOUSE_DEMISE && stage > 2)
            {
                instance->SetData(DATA_MILLHOUSE_EVENT, SPECIAL);
                events.Reset();
                me->InterruptNonMeleeSpells(false);
                DoCast(me, SPELL_IMPENDING_DOOOOM);
                DoCast(me, SPELL_IMPENDING_DOOM_GROW, true);
                events.ScheduleEvent(EVENT_BREAK_WALL, 15000);
            }
        }

        void DamageTaken(Unit * /*done_by*/, uint32 &damage)
        {
            if (stage > 2)
                damage = 0;

            if (me->HealthBelowPctDamaged(50, damage))
            {
                damage = 0;
                me->RemoveAllAuras();
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                events.Reset();
                DoCast(me, SPELL_BLUR);
                if (stage <= 1)
                    Talk(stage);
                isAdvancing = true;
                events.ScheduleEvent(EVENT_BLUR_ADVANCE, 1000);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!isAdvancing && !UpdateVictim())
                return;

            events.Update(diff);

            if (!castingDoom && me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FEAR:
                    DoCast(SELECT_TARGET_RANDOM, SPELL_FEAR);
                    events.ScheduleEvent(EVENT_FEAR, urand(5000, 8000));
                    break;
                case EVENT_FROSTBOLT_VOLLEY:
                    DoCast(me, SPELL_FROSTBOLT_VOLLEY);
                    events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(8000, 10000));
                    break;
                case EVENT_SHADOW_BOLT:
                    DoCastVictim(SPELL_SHADOW_BOLT);
                    events.ScheduleEvent(EVENT_SHADOW_BOLT, 2000);
                    break;
                case EVENT_SHADOWFURY:
                    DoCast(SELECT_TARGET_RANDOM, SPELL_SHADOWFURY);
                    events.ScheduleEvent(EVENT_SHADOWFURY, urand(12000, 18000));
                    break;
                case EVENT_IMPENDING_DOOM:
                    castingDoom = true;
                    DoCast(me, SPELL_IMPENDING_DOOOOM);
                    DoCast(me, SPELL_IMPENDING_DOOM_GROW, true);
                    if (urand(0,1))
                    {
                        Talk(SAY_DOOM_1);
                        events.ScheduleEvent(EVENT_SAY_DOOM, 5500);
                    }
                    else
                        Talk(SAY_DOOM_ALTERNATIVE);

                    events.ScheduleEvent(EVENT_IMPENDING_DOOM_CANCEL, 15000);
                    break;
                case EVENT_IMPENDING_DOOM_CANCEL:
                    me->InterruptNonMeleeSpells(false);
                    me->RemoveAurasDueToSpell(SPELL_IMPENDING_DOOM_GROW);
                    DoCast(me, SPELL_IMPENDING_DOOOOM);
                    DoCast(me, SPELL_IMPENDING_DOOM_GROW, true);
                    events.ScheduleEvent(EVENT_IMPENDING_DOOM_CANCEL, 15000);
                    break;
                case EVENT_BLUR_ADVANCE:
                    me->SetHomePosition(advancePos[stage]);
                    me->GetMotionMaster()->MoveSplinePath(stage);
                    events.ScheduleEvent(EVENT_NEXT_POINT, me->GetSplineDuration());
                    //me->GetMotionMaster()->MovePoint(POINT_FIRST_PACK + stage, advancePos[stage]);
                    break;
                case EVENT_NEXT_POINT:
                    if (stage < 2)
                    {
                        me->SetFacingTo(advancePos[stage].GetOrientation());
                        events.ScheduleEvent(EVENT_SPECIAL_BLEND, 1000);
                    }
                    else if (stage == 2)
                    {
                        me->SetFacingTo(M_PI);
                        me->RemoveAurasDueToSpell(SPELL_BLUR);
                        me->SetReactState(REACT_PASSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        events.ScheduleEvent(EVENT_IMPENDING_DOOM, 1000);
                    }
                    ++stage;
                    break;
                case EVENT_SPECIAL_BLEND:
                    me->RemoveAurasDueToSpell(SPELL_BLUR);
                    me->CombatStop();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    DoCast(me, SPELL_SPECIAL_BLEND, true);
                    events.ScheduleEvent(EVENT_SPECIAL_BLEND_END, 10000);
                    break;
                case EVENT_SPECIAL_BLEND_END:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    isAdvancing = false;
                    break;
                case EVENT_SAY_DOOM:
                    Talk(SAY_DOOM_2);
                    break;
                case EVENT_BREAK_WALL:
                    if (Creature * trigger = me->FindNearestCreature(NPC_WORLD_TRIGGER, 40.0f))
                        trigger->CastSpell(trigger, SPELL_DOOR_BREAK, true);
                    instance->SetData(DATA_MILLHOUSE_EVENT, DONE);
                    events.ScheduleEvent(EVENT_CORBORUS_CHARGE, 3000);
                    break;
                case EVENT_CORBORUS_CHARGE:
                    if (Creature * corborus = me->FindNearestCreature(BOSS_CORBORUS, 100.0f))
                    {
                        corborus->SetHomePosition(1154.55f, 878.843f, 286.963f, M_PI);

                        corborus->CastSpell(corborus, SPELL_WYRM_CHARGE);
                        events.ScheduleEvent(EVENT_MASS_KNOCKBACK, 1000);
                    }
                    break;
                case EVENT_MASS_KNOCKBACK:
                    {
                        // find npcs with ReadyUnarmed aura and send them to Narnia
                        std::list<Creature*> cList;

                        CellCoord pair(Trinity::ComputeCellCoord(me->GetPositionX(), me->GetPositionY()));
                        Cell cell(pair);
                        cell.SetNoCreate();
                        Trinity::UnitAuraCheck check(true, SPELL_READYUNARMED);
                        Trinity::CreatureListSearcher<Trinity::UnitAuraCheck> searcher(me, cList, check);

                        cell.Visit(pair, Trinity::makeGridVisitor(searcher), *me->GetMap(), *me, 30.0f);

                        if (cList.empty())
                        {
                            TC_LOG_ERROR("misc", "Stonecore: Millhouse Manastorn - target creatures are missing");
                            return;
                        }

                        cList.push_back(me);

                        for (std::list<Creature*>::const_iterator itr = cList.begin(); itr != cList.end(); ++itr)
                        {
                            (*itr)->InterruptNonMeleeSpells(false);
                            (*itr)->KnockbackFrom(1162.596313f, 896.778118f, 200.0f, 25.0f);
                            (*itr)->DespawnOrUnsummon(10000);
                        }
                    }
                    break;

                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        uint8 stage;
        bool isAdvancing;
        bool castingDoom;
        EventMap events;
        InstanceScript * instance;
    };

public:
    npc_stonecore_millhouse_manastorm() : CreatureScript("npc_stonecore_millhouse_manastorm") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stonecore_millhouse_manastormAI(creature);
    }
};

class at_stonecore_corborus : public AreaTriggerScript
{
public:
    at_stonecore_corborus() : AreaTriggerScript("at_stonecore_corborus") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/)
    {
        if (InstanceScript * instance = player->GetInstanceScript())
            instance->SetData(DATA_CORBORUS_AREATRIGGER, DONE);

        return false;
    }
};

class at_stonecore_slabhide : public AreaTriggerScript
{
public:
    at_stonecore_slabhide() : AreaTriggerScript("at_stonecore_slabhide") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/)
    {
        if (InstanceScript * instance = player->GetInstanceScript())
            instance->SetData(DATA_SLABHIDE_AREATRIGGER, DONE);

        return false;
    }
};

class npc_stalactite_trigger_trash : public CreatureScript
{
    enum
    {
        SPELL_PERIODIC_EFFECT       = 81035,
        SPELL_SUMMON                = 81028
    };

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature * creature) : ScriptedAI(creature)
        {
            slabhideGUID = 0;
        }

        void Reset()
        {
        }

        void SpellHitTarget(Unit *, const SpellInfo * spell)
        {
            if (spell->Id == SPELL_PERIODIC_EFFECT)
            {
                //if (!slabhideEmoteCooldown)
                //{
                //    if (!slabhideGUID)
                //        if (InstanceScript * instance = me->GetInstanceScript())
                //            slabhideGUID = instance->GetData64(DATA_SLABHIDE);
                //
                //    if (Creature * slabhide = Creature::GetCreature(*me, slabhideGUID))
                //        slabhide->
                //}
                DoCast(me, SPELL_SUMMON, false);
            }
        }

        //void UpdateAI(uint32 const diff)
        //{
        //    if (!slabhideEmoteCooldown)
        //        return;
        //
        //    if (diff <= slabhideEmoteCooldown)
        //    {
        //        slabhideEmoteCooldown = 0;
        //    }else slabhideEmoteCooldown -= diff;
        //}

    private:
        uint64 slabhideGUID;
        //uint32 slabhideEmoteCooldown;
    };
public:
    npc_stalactite_trigger_trash() : CreatureScript("npc_stalactite_trigger_trash") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new ai_impl(creature);
    }
};


class npc_stonecore_sentry : public CreatureScript
{
    struct npc_stonecore_sentryAI : public ScriptedAI
    {
        npc_stonecore_sentryAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            assistTimer = 100;
            moving = false;
            SetCombatMovement(false);
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterCombat(Unit * /*who*/)
        {
            Talk(0);
            me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
            assistTimer = 100;
        }

        void MovementInform(uint32 type, uint32 /*id*/)
        {
            if (type == ASSISTANCE_MOTION_TYPE)
            {
                me->SetTarget(0);
                me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
            }
            else if (type == ASSISTANCE_DISTRACT_MOTION_TYPE)
            {
                assistTimer = 1500;
                moving = false;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (moving)
                return;

            if (assistTimer <= diff)
            {
                moving = true;
                //me->SetReactState(REACT_PASSIVE);
                me->SetTarget(0);
                me->DoFleeToGetAssistance();
            }else assistTimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        uint32 assistTimer;
        bool moving;
    };

public:
    npc_stonecore_sentry() : CreatureScript("npc_stonecore_sentry") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stonecore_sentryAI(creature);
    }
};

void AddSC_the_stonecore()
{
    new npc_stonecore_berserker();
    new npc_stonecore_earthshaper();
    new npc_crystalspawn_giant();
    new npc_stonecore_bruiser();
    new spell_stonecore_social_aura();
    new spell_stonecore_quake();
    new npc_crystal_shard_trash();
    new npc_stonecore_magmalord();
    new npc_rock_borer();
    new npc_stonecore_teleporter();
    new npc_stonecore_millhouse_manastorm();
    new at_stonecore_corborus();
    new at_stonecore_slabhide();
    new npc_stalactite_trigger_trash();
    new npc_stonecore_imp();
    new npc_stonecore_sentry();
}
