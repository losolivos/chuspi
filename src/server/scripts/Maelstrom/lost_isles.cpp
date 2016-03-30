#include "ObjectMgr.h"
#include "Object.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Spell.h"
#include "SpellScript.h"
#include "MoveSplineInit.h"
#include "ScriptedEscortAI.h"
#include "Pet.h"
#include "Vehicle.h"
#include "TemporarySummon.h"
#include "CombatAI.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"

class npc_pterrordax_scavenger : public CreatureScript
{
public:
    npc_pterrordax_scavenger() : CreatureScript("npc_pterrordax_scavenger") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_pterrordax_scavengerAI(creature);
    }

    struct npc_pterrordax_scavengerAI : public ScriptedAI
    {
        npc_pterrordax_scavengerAI(Creature * creature) : ScriptedAI(creature) {}

        void InitializeAI()
        {
            me->setActive(true);
            me->SetReactState(REACT_PASSIVE);
        }
    };
};

class npc_bomb_throwing_monkey : public CreatureScript
{
public:
    npc_bomb_throwing_monkey() : CreatureScript("npc_bomb_throwing_monkey") {}

    enum
    {
        SPELL_THROW_BOMB_VISUAL        = 66142,
        SPELL_NITRO_POTASSIUM_BANANAS  = 67917,
        SPELL_EXPLODING_BANANAS        = 67919,

        NPC_MONKEY_BUSINESS_CREDIT     = 35760,

        EVENT_THROW_BOMB               = 1,

        QUEST_MONKEY_BUSINESS          = 14019,
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_bomb_throwing_monkeyAI(creature);
    }

    struct npc_bomb_throwing_monkeyAI : public ScriptedAI
    {
        npc_bomb_throwing_monkeyAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.ScheduleEvent(EVENT_THROW_BOMB, urand(9, 15) * IN_MILLISECONDS);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_NITRO_POTASSIUM_BANANAS)
            {
                if (me->isMoving())
                    me->StopMoving();

                if (caster->GetTypeId() == TYPEID_PLAYER && caster->ToPlayer()->GetQuestStatus(QUEST_MONKEY_BUSINESS) == QUEST_STATUS_INCOMPLETE)
                    caster->ToPlayer()->KilledMonsterCredit(NPC_MONKEY_BUSINESS_CREDIT);

                me->CastSpell(me, SPELL_EXPLODING_BANANAS, false);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_THROW_BOMB)
            {
                me->CastSpell((Unit*)NULL, SPELL_THROW_BOMB_VISUAL, false);
                events.ScheduleEvent(EVENT_THROW_BOMB, urand(7, 15) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_evol_fingers_beach : public CreatureScript
{
public:
    npc_evol_fingers_beach() : CreatureScript("npc_evol_fingers_beach") {}

    enum
    {
        SPELL_VOIDWALER   = 5108,
        SPELL_SHADOW_BOLT = 68047,

        EVENT_SHADOW_BOLT  = 1
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_evol_fingers_beachAI(creature);
    }

    struct npc_evol_fingers_beachAI : public ScriptedAI
    {
        npc_evol_fingers_beachAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            me->CastSpell((Unit*)NULL, SPELL_VOIDWALER, false);
            Reset();
        }

        void Reset()
        {
            events.ScheduleEvent(EVENT_SHADOW_BOLT, 1 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SHADOW_BOLT)
            {
                me->CastSpell((Unit*)NULL, SPELL_SHADOW_BOLT, false);
                events.ScheduleEvent(EVENT_SHADOW_BOLT, 2.54 * IN_MILLISECONDS);
            }
        }
    };
};

class npc_bamm_megabomb_beach : public CreatureScript
{
public:
    npc_bamm_megabomb_beach() : CreatureScript("npc_bamm_megabomb_beach") {}

    enum eTalks
    {
        TALK_QA            = 0,
        TALK_QC            = 1,

        SPELL_SHOOTING      = 65977,

        EVENT_SHOOTING       = 1,

        QUEST_MONKEY_BUSINESS = 14019
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_MONKEY_BUSINESS)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QA, player->GetGUID(), true);

        return true;
    }

    bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_MONKEY_BUSINESS)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QC, player->GetGUID(), true);

        return true;
    }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_bamm_megabomb_beachAI(creature);
    }

    struct npc_bamm_megabomb_beachAI : public ScriptedAI
    {
        npc_bamm_megabomb_beachAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_SHOOTING, frand(2.5, 5) * IN_MILLISECONDS);
            Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SHOOTING)
            {
                me->CastSpell((Unit*)NULL, SPELL_SHOOTING, false);
                events.ScheduleEvent(EVENT_SHOOTING, urand(4, 7) * IN_MILLISECONDS);
            }
        }
    };
};

class npc_prince_gallywix_beach : public CreatureScript
{
public:
    npc_prince_gallywix_beach() : CreatureScript("npc_prince_gallywix_beach") {}

    enum
    {
        EVENT_TALK  = 1,
        TALK_RANDOM = 0,
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_prince_gallywix_beachAI(creature);
    }

    struct npc_prince_gallywix_beachAI : public ScriptedAI
    {
        npc_prince_gallywix_beachAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_TALK, 5 * IN_MILLISECONDS);
            Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_TALK)
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 100.0f);
                for(auto player : playerList)
                    Talk(TALK_RANDOM, player->GetGUID(), true);

                events.ScheduleEvent(EVENT_TALK, 300 * IN_MILLISECONDS);
            }
        }
    };
};

class npc_maxx_avalanche_beach : public CreatureScript
{
public:
    npc_maxx_avalanche_beach() : CreatureScript("npc_maxx_avalanche_beach") {}

    enum
    {
        EVENT_TALK              = 1,
        EVENT_AURAS             = 2,

        TALK_QA                  = 0,
        TALK_MATRIARCH           = 1,

        SPELL_FLAMETONGUE_WEAPON = 78273,
        SPELL_LITGHTNING_SHIELD  = 12550,

        QUEST_ITS_OUR_PROBLEM_NOW = 14473,
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_ITS_OUR_PROBLEM_NOW)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QA, player->GetGUID(), true);

        return true;
    }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_maxx_avalanche_beachAI(creature);
    }

    struct npc_maxx_avalanche_beachAI : public ScriptedAI
    {
        npc_maxx_avalanche_beachAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_TALK, 5 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_AURAS, 2 * IN_MILLISECONDS);
            Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_TALK:
                        Talk(TALK_MATRIARCH);
                        events.ScheduleEvent(EVENT_TALK, urand(160, 220) * IN_MILLISECONDS);
                        break;
                    case EVENT_AURAS:
                        me->CastSpell(me, SPELL_LITGHTNING_SHIELD, false);
                        me->CastSpell(me, SPELL_FLAMETONGUE_WEAPON, false);
                        events.ScheduleEvent(EVENT_AURAS, 600 * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };
};

class npc_coach_crosschek_beach : public CreatureScript
{
public:
    npc_coach_crosschek_beach() : CreatureScript("npc_coach_crosschek_beach") {}

    enum eEvents
    {
        EVENT_TALK = 1,

        TALK_SUITS = 0,
        TALK_QA    = 1,
        TALK_QA_2  = 2,
        TALK_QA_3  = 3,

        QUEST_ZOMBIES_VS_BOOSTER_ROCKET_BOOTS = 24942,
        QUEST_ROCKET_BOOT_BOOST               = 24952,
        QUEST_THE_ULTIMATE_FOOTBOOMB_UNIFORM  = 25201
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_ZOMBIES_VS_BOOSTER_ROCKET_BOOTS:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA, player->GetGUID(), true);
                break;
            case QUEST_ROCKET_BOOT_BOOST:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA_2, player->GetGUID(), true);
                break;
            case QUEST_THE_ULTIMATE_FOOTBOOMB_UNIFORM:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_3, player->GetGUID(), true);
                break;
        }

        return true;
    }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_coach_crosschek_beachAI(creature);
    }

    struct npc_coach_crosschek_beachAI : public ScriptedAI
    {
        npc_coach_crosschek_beachAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_TALK, 8 * IN_MILLISECONDS);
            Reset();
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_TALK)
            {
                Talk(TALK_SUITS);
                events.ScheduleEvent(EVENT_TALK, urand(160, 220) * IN_MILLISECONDS);
            }
        }
    };
};

class go_goblin_escape_pod : public GameObjectScript
{
public:
    go_goblin_escape_pod() : GameObjectScript("go_goblin_escape_pod") {}

    enum
    {
        SPELL_PRINCE_CONTROLLER_AURA = 67433,
        SPELL_SUMMON_PRINCE          = 67845,
        SPELL_SUMMON_SURVIVOR        = 66137,
        SPELL_SUMMON_SURVIOR_DEAD    = 66138,

        NPC_CAPTURED_GOBLIN          = 34748
    };

    bool OnGossipHello(Player* player, GameObject* pGO)
    {
        if (player->HasAura(SPELL_PRINCE_CONTROLLER_AURA))
            player->CastSpell((Unit*)NULL, SPELL_SUMMON_PRINCE, true);
        else
            player->CastSpell((Unit*)NULL, urand(0, 1) ? SPELL_SUMMON_SURVIOR_DEAD : SPELL_SUMMON_SURVIVOR, true);

        return false;
    }
};

class npc_captured_goblin : public CreatureScript
{
public:
    npc_captured_goblin() : CreatureScript("npc_captured_goblin") {}

    enum
    {
        TALK_THANKS              = 0,
        SPELL_EMOTESTATE_SWIMING = 37744,
        EVENT_START_SWIMING      = 1,
        QUEST_GOBLIN_ESCAPE_PODS = 14001,
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_captured_goblinAI(creature);
    }

    struct npc_captured_goblinAI : public ScriptedAI
    {
        npc_captured_goblinAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
            Talk(TALK_THANKS, summoner->GetGUID());
            me->CastSpell(me, SPELL_EMOTESTATE_SWIMING, false);

            if (summoner->GetTypeId() == TYPEID_PLAYER)
                if (summoner->ToPlayer()->GetQuestStatus(QUEST_GOBLIN_ESCAPE_PODS) == QUEST_STATUS_INCOMPLETE)
                    summoner->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());

            events.ScheduleEvent(EVENT_START_SWIMING, 3 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_START_SWIMING)
            {
                me->RemoveAura(SPELL_EMOTESTATE_SWIMING);
                Movement::MoveSplineInit init(me);
                init.MoveTo(571.281f, 3181.45f, -2.830002f);
                init.SetVelocity(5.0f);
                init.SetFly();
                init.Launch();

                me->DespawnOrUnsummon(me->GetSplineDuration());
            }
        }
    };
};

class npc_trade_prince_gallywix_qgep : public CreatureScript
{
public:
    npc_trade_prince_gallywix_qgep() : CreatureScript("npc_trade_prince_gallywix_qgep") {}

    enum
    {
        TALK_THANKS                  = 0,

        SPELL_EMOTESTATE_SWIMING     = 37744,
        SPELL_PRINCE_CONTROLLER_AURA = 67433,

        EVENT_START_SWIMING          = 1,
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_trade_prince_gallywix_qgepAI(creature);
    }

    struct npc_trade_prince_gallywix_qgepAI : public ScriptedAI
    {
        npc_trade_prince_gallywix_qgepAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            me->SetCustomVisibility(CUSTOM_VISIBILITY_SEER,summoner->GetGUID());
            me->SetVisible(false);
            me->setActive(true);
            me->SetReactState(REACT_PASSIVE);
            Talk(TALK_THANKS, summoner->GetGUID(), true);
            me->CastSpell(me, SPELL_EMOTESTATE_SWIMING, false);

            if (summoner->HasAura(SPELL_PRINCE_CONTROLLER_AURA))
                summoner->RemoveAura(SPELL_PRINCE_CONTROLLER_AURA);

            events.ScheduleEvent(EVENT_START_SWIMING, 5 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_START_SWIMING)
            {
                me->RemoveAura(SPELL_EMOTESTATE_SWIMING);
                Movement::MoveSplineInit init(me);
                init.MoveTo(574.303f, 3170.926f, -0.9899795f);
                init.SetVelocity(5.0f);
                init.SetFly();
                init.Launch();

                me->DespawnOrUnsummon(me->GetSplineDuration());
            }
        }
    };
};

class npc_geargrinder_gizmo_qgep : public CreatureScript
{
public:
    npc_geargrinder_gizmo_qgep() : CreatureScript("npc_geargrinder_gizmo_qgep") {}

    enum eTalks
    {
        TALK_QA                       = 0,

        SPELL_PRINCE_CONTROLLER_AURA  = 67433,
        SPELL_THERMOHYDRATIC_FLIPPERS = 68258,

        QUEST_GOBLIN_ESCAPE_PODS      = 14001
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_GOBLIN_ESCAPE_PODS)
        {
            player->CastSpell(player, SPELL_PRINCE_CONTROLLER_AURA, false);

            if (creature->IsAIEnabled)
               creature->AI()->Talk(TALK_QA, player->GetGUID(), true);
        }

        return true;
    }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_geargrinder_gizmo_qgepAI(creature);
    }

    struct npc_geargrinder_gizmo_qgepAI : public ScriptedAI
    {
        npc_geargrinder_gizmo_qgepAI(Creature * c) : ScriptedAI(c) {}

        void InitializeAI()
        {
            me->setActive(true);
            me->SetReactState(REACT_AGGRESSIVE);
            Reset();
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                uint8 questStatus = who->ToPlayer()->GetQuestStatus(QUEST_GOBLIN_ESCAPE_PODS);
                if ((questStatus == QUEST_STATUS_INCOMPLETE || questStatus == QUEST_STATUS_COMPLETE) && who->IsInWater())
                    if (!who->HasAura(SPELL_THERMOHYDRATIC_FLIPPERS))
                        who->CastSpell(who, SPELL_THERMOHYDRATIC_FLIPPERS, false);
            }
        }
    };
};

class npc_sassy_hardwrench_qgosb : public CreatureScript
{
public:
    npc_sassy_hardwrench_qgosb() : CreatureScript("npc_sassy_hardwrench_qgosb") {}

    enum
    {
        TALK_QA                 = 0,
        TALK_QA2                = 1,

        QUEST_GET_OUR_STUFF_BACK = 14014,
        QUEST_ENEMY_OF_MY_ENEMY  = 14234
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_GET_OUR_STUFF_BACK:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA, player->GetGUID(), true);
                   break;
            case QUEST_ENEMY_OF_MY_ENEMY:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA2, player->GetGUID(), true);
                   break;

        }
        return true;
    }
};

class spell_summon_dead_goblin_survivor : public SpellScriptLoader
{
public:
    spell_summon_dead_goblin_survivor() : SpellScriptLoader("spell_summon_dead_goblin_survivor") { }

    class spell_summon_dead_goblin_survivor_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_summon_dead_goblin_survivor_SpellScript);

        void ModDestHeight(SpellEffIndex /*effIndex*/)
        {
            Position offset = { 0.0f, 0.0f, -0.7f, 0.0f };
            const_cast<WorldLocation*>(GetExplTargetDest())->RelocateOffset(offset);
            GetHitDest()->RelocateOffset(offset);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_summon_dead_goblin_survivor_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_SUMMON);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_summon_dead_goblin_survivor_SpellScript();
    }
};

class spell_thermohydratic_flippers_periodic : public SpellScriptLoader
{
public:
    spell_thermohydratic_flippers_periodic() : SpellScriptLoader("spell_thermohydratic_flippers_periodic") { }

    enum eSpells
    {
        SPELL_THERMOHYDRATIC_FLIPPERS = 68258
    };

    class spell_thermohydratic_flippers_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_thermohydratic_flippers_periodic_AuraScript);

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();
            Unit* caster = GetCaster();

            if (!caster->IsInWater())
                if (caster->HasAura(SPELL_THERMOHYDRATIC_FLIPPERS))
                   caster->RemoveAura(SPELL_THERMOHYDRATIC_FLIPPERS);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_thermohydratic_flippers_periodic_AuraScript::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_thermohydratic_flippers_periodic_AuraScript();
    }
};

class spell_exploding_bananas_qmb: public SpellScriptLoader
{
public:
    spell_exploding_bananas_qmb() : SpellScriptLoader("spell_exploding_bananas_qmb") { }

    class spell_exploding_bananas_qmb_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_exploding_bananas_qmb_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* owner = GetOwner()->ToUnit();

            if (Creature* creature = owner->ToCreature())
                creature->ToCreature()->DespawnOrUnsummon(3 * IN_MILLISECONDS);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_exploding_bananas_qmb_AuraScript::OnApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_exploding_bananas_qmb_AuraScript();
    }
};

class KtcSnapflashTargetSelector
{
public:
    KtcSnapflashTargetSelector() { }

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (uint32 entry = cre->GetEntry())
               if (entry == 37872 || entry == 37895 || entry == 37896 || entry == 37897)
                  return false;

        return true;
    }
};

class spell_ktc_snapflash_qctu : public SpellScriptLoader
{
public:
    spell_ktc_snapflash_qctu() : SpellScriptLoader("spell_ktc_snapflash_qctu") { }

    enum eSpells
    {
        SPELL_CAST_TO_PLAYER = 68279
    };

    class spell_ktc_snapflash_qctu_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ktc_snapflash_qctu_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (Unit* caster = GetCaster())
                targets.remove_if(KtcSnapflashTargetSelector());
        }

        void HandleEffect(SpellEffIndex effIndex)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();

            target->CastSpell(caster, SPELL_CAST_TO_PLAYER, false);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ktc_snapflash_qctu_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_ktc_snapflash_qctu_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_ktc_snapflash_qctu_SpellScript();
    }
};

class spell_ktc_snapflash_effect_qctu : public SpellScriptLoader
{
public:
    spell_ktc_snapflash_effect_qctu() : SpellScriptLoader("spell_ktc_snapflash_effect_qctu") { }

    enum
    {
        ENTRY_BUNNY_1       = 37872,
        ENTRY_BUNNY_2       = 37895,
        ENTRY_BUNNY_3       = 37896,
        ENTRY_BUNNY_4       = 37897,

        SPELL_BUNNY_1       = 68349,
        SPELL_BUNNY_2       = 68936,
        SPELL_BUNNY_3       = 68943,
        SPELL_BUNNY_4       = 68937,

        SPELL_SCREEN_EFFECT  = 70649,
        SPELL_BIND_SIGHT     = 70641,
        SPELL_SNAPFLASH      = 68281,

        SPELL_REMOVE_SEE_1   = 68349,
        SPELL_REMOVE_SEE_2   = 68936,
        SPELL_REMOVE_SEE_3   = 68943,
        SPELL_REMOVE_SEE_4   = 68937,

        SPELL_SEE_1          = 70661,
        SPELL_SEE_2          = 70678,
        SPELL_SEE_3          = 70680,
        SPELL_SEE_4          = 70681
    };

    class spell_ktc_snapflash_effect_qctu_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ktc_snapflash_effect_qctu_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            Unit* caster = GetCaster();
            Player* player = GetHitPlayer();

            if (!(caster && player))
                return;

            player->CastSpell(player, SPELL_SCREEN_EFFECT, false);
            player->CastSpell((Unit*)NULL, SPELL_BIND_SIGHT, false);
            player->CastSpell(player, SPELL_SNAPFLASH, false);
            player->KilledMonsterCredit(caster->GetEntry(), caster->GetGUID());

            switch(caster->GetEntry())
            {
                case ENTRY_BUNNY_1:
                    player->CastSpell(player, SPELL_REMOVE_SEE_1, false);
                    break;
                case ENTRY_BUNNY_2:
                    player->CastSpell(player, SPELL_REMOVE_SEE_2, false);
                    break;
                case ENTRY_BUNNY_3:
                    player->CastSpell(player, SPELL_REMOVE_SEE_3, false);
                    break;
                case ENTRY_BUNNY_4:
                    player->CastSpell(player, SPELL_REMOVE_SEE_4, false);
                    break;
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ktc_snapflash_effect_qctu_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_ktc_snapflash_effect_qctu_SpellScript();
    }
};

class npc_frightened_miner_qmt : public CreatureScript
{
public:
    npc_frightened_miner_qmt() : CreatureScript("npc_frightened_miner_qmt") {}

    enum
    {
        TALK_1               = 0,
        TALK_2               = 1,
        TALK_3               = 2,
        TALK_4               = 3,
        TALK_5               = 4,
        TALK_6               = 5,

        EVENT_READY           = 1,
        EVENT_FIRST_PAUSE     = 2,
        EVENT_SECOND_PAUSE    = 3,
        EVENT_THIRD_PAUSE     = 4,
        EVENT_FOURTH_PAUSE    = 5,
        EVENT_DONE            = 6,

        SPELL_CART_SUMMON     = 68064,
        SPELL_CART_TRANSFORM  = 68065,
        SPELL_CHAIN           = 68122,

        NPC_CART              = 35814,
        NPC_QMT_CREADIT       = 35816
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_frightened_miner_qmtAI(creature);
    }

    struct npc_frightened_miner_qmtAI : public npc_escortAI
    {
        npc_frightened_miner_qmtAI(Creature* creature) : npc_escortAI(creature), Summons(me) {}

        EventMap events;
        SummonList Summons;

        void IsSummonedBy(Unit* summoner)
        {
            Talk(TALK_1, summoner->GetGUID(), true);
            me->CastSpell((Unit*)NULL, SPELL_CART_SUMMON, false);
            Start(true, false, summoner->GetGUID());
            events.ScheduleEvent(EVENT_READY, 5 * IN_MILLISECONDS);
        }

        void JustSummoned(Creature* summon)
        {
            Summons.Summon(summon);

            if (summon->GetEntry() == NPC_CART)
            {
                summon->CastSpell(me, SPELL_CHAIN, true);
                summon->CastSpell((Unit*)NULL, SPELL_CART_TRANSFORM, true);
                summon->GetMotionMaster()->Clear();
                summon->GetMotionMaster()->MoveFollow(me, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
            }
        }

        void WaypointReached(uint32 point)
        {
            switch(point)
            {
                case 6:
                    SetEscortPaused(true);
                    Talk(TALK_2);
                    events.ScheduleEvent(EVENT_FIRST_PAUSE, 5 * IN_MILLISECONDS);
                    break;
                case 10:
                    me->SetReactState(REACT_PASSIVE);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_MINING);
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_SECOND_PAUSE, 10 * IN_MILLISECONDS);
                    break;
                case 14:
                    me->SetReactState(REACT_PASSIVE);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_MINING);
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_THIRD_PAUSE, 10 * IN_MILLISECONDS);
                    break;
                case 19:
                    me->SetReactState(REACT_PASSIVE);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_MINING);
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_FOURTH_PAUSE, 10 * IN_MILLISECONDS);
                    break;
                case 25:
                    me->SetReactState(REACT_PASSIVE);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_MINING);
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_DONE, 10 * IN_MILLISECONDS);
                    break;
                case 32:
                    me->DespawnOrUnsummon();
                    break;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);
            npc_escortAI::UpdateAI(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_FIRST_PAUSE:
                        SetEscortPaused(false);
                        break;
                    case EVENT_SECOND_PAUSE:
                        me->SetReactState(REACT_DEFENSIVE);
                        if (me->IsSummon())
                            if (Unit* owner = me->ToTempSummon()->GetOwner())
                                Talk(TALK_3, owner->GetGUID(), true);
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                        SetEscortPaused(false);
                        break;
                    case EVENT_THIRD_PAUSE:
                        me->SetReactState(REACT_DEFENSIVE);
                        if (me->IsSummon())
                            if (Unit* owner = me->ToTempSummon()->GetOwner())
                                Talk(TALK_4, owner->GetGUID(), true);
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                        SetEscortPaused(false);
                        break;
                    case EVENT_FOURTH_PAUSE:
                        me->SetReactState(REACT_DEFENSIVE);
                        if (me->IsSummon())
                            if (Unit* owner = me->ToTempSummon()->GetOwner())
                                Talk(TALK_5, owner->GetGUID(), true);
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                        SetEscortPaused(false);
                        break;
                    case EVENT_DONE:
                        me->SetReactState(REACT_DEFENSIVE);
                        if (me->IsSummon())
                            if (Unit* owner = me->ToTempSummon()->GetOwner())
                            {
                                Talk(TALK_6, owner->GetGUID(), true);

                                if (Player* player = owner->ToPlayer())
                                    player->KilledMonsterCredit(NPC_QMT_CREADIT);
                            }
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                        SetEscortPaused(false);
                        SetRun();
                        break;
                }
            }
        }
    };
};

class npc_foreman_dampwick_qmt : public CreatureScript
{
public:
    npc_foreman_dampwick_qmt() : CreatureScript("npc_foreman_dampwick_qmt") {}

    enum
    {
        QUEST_CAPTURING_THE_UNKNOWN = 14031,
        QUEST_MINER_TROUBLES        = 14021,

        SPELL_SEE_1                 = 70661,
        SPELL_SEE_2                 = 70678,
        SPELL_SEE_3                 = 70680,
        SPELL_SEE_4                 = 70681,
        SPELL_QMT_ACCEPT            = 68061
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_CAPTURING_THE_UNKNOWN:
                player->CastSpell(player, SPELL_SEE_1, false);
                player->CastSpell(player, SPELL_SEE_2, false);
                player->CastSpell(player, SPELL_SEE_3, false);
                player->CastSpell(player, SPELL_SEE_4, false);
                break;
            case QUEST_MINER_TROUBLES:
                player->CastSpell(player, SPELL_QMT_ACCEPT, false);
                break;
        }
        player->SaveToDB();
        return true;
    }
};

class npc_pygmy_witchdoctor_qmt : public CreatureScript
{
public:
    npc_pygmy_witchdoctor_qmt() : CreatureScript("npc_pygmy_witchdoctor_qmt") {}

    enum
    {
        TALK_RANDOM       = 0,

        EVENT_SHODOW_BOLT = 1,
        EVENT_HEX         = 2,
        EVENT_HEAL        = 3,
        EVENT_RANDOM_TALK = 4,

        SPELL_CHANNELING  = 51733,
        SPELL_SHADOW_BOLT = 9613,
        SPELL_HEX         = 18503,
        SPELL_HEAL        = 11986,

        NPC_CART          = 35814,
        NPC_QMT_CREADIT   = 35816
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_pygmy_witchdoctor_qmtAI(creature);
    }

    struct npc_pygmy_witchdoctor_qmtAI : public ScriptedAI
    {
        npc_pygmy_witchdoctor_qmtAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            me->CastSpell(me, SPELL_CHANNELING, false);
            events.ScheduleEvent(EVENT_RANDOM_TALK, 180 * IN_MILLISECONDS);
        }

        void EnterCombat(Unit* who)
        {
            me->RemoveAura(SPELL_CHANNELING);
            events.ScheduleEvent(EVENT_SHODOW_BOLT, 0);
            events.ScheduleEvent(EVENT_HEAL, 6 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_HEX, 12 * IN_MILLISECONDS);
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
                switch(eventId)
                {
                    case EVENT_SHODOW_BOLT:
                        me->CastSpell(me->GetVictim(), SPELL_SHADOW_BOLT, false);
                        events.ScheduleEvent(EVENT_SHODOW_BOLT, 3.05 * IN_MILLISECONDS);
                        break;
                    case EVENT_HEAL:
                         me->CastSpell(me, SPELL_HEAL, false);
                         events.ScheduleEvent(EVENT_HEAL, 6 * IN_MILLISECONDS);
                        break;
                    case EVENT_HEX:
                         me->CastSpell(me->GetVictim(), SPELL_HEX, false);
                         events.ScheduleEvent(EVENT_HEX, 12 * IN_MILLISECONDS);
                         break;
                }
            }
        }
    };
};

class npc_kilag_gorefang_qww: public CreatureScript
{
public:
    npc_kilag_gorefang_qww() : CreatureScript("npc_kilag_gorefang_qww") {}

    enum
    {
        SPELL_SHOOTING     = 15620,
        SPELL_WEED_WHACKER = 68212,

        EVENT_SHOOTING     = 1,

        NPC_POISON_SPITTER = 35896,

        QUEST_WEED_WHACKER = 14236,

        TALK_QA            = 0,
        TALK_QC            = 1
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_WEED_WHACKER)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QA, player->GetGUID(), true);

        player->SaveToDB();
        return true;
    }

    bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_WEED_WHACKER)
        {
            player->RemoveAura(SPELL_WEED_WHACKER);
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QC, player->GetGUID(), true);
        }
        player->SaveToDB();
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_kilag_gorefang_qwwAI(creature);
    }

    struct npc_kilag_gorefang_qwwAI : public ScriptedAI
    {
        npc_kilag_gorefang_qwwAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            me->setActive(true);
            events.ScheduleEvent(EVENT_SHOOTING, 2 * IN_MILLISECONDS);
            Reset();
        }

        void DamageTaken(Unit* /*killer*/, uint32 &damage)
        {
            if (me->HealthBelowPctDamaged(70, damage))
                damage = 0;
        }

        void UpdateAI(uint32 const diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_SHOOTING:
                    {
                        std::list<Creature*> spitters;
                        me->GetCreatureListWithEntryInGrid(spitters, NPC_POISON_SPITTER, 30.0f);
                        if (!spitters.empty())
                            if (auto creature = Trinity::Containers::SelectRandomContainerElement(spitters))
                                me->CastSpell(creature, SPELL_SHOOTING, false);
                    }
                    events.ScheduleEvent(EVENT_SHOOTING, 2 * IN_MILLISECONDS);
                    break;
                }
            }
        }
    };
};

class npc_orc_scout_qww : public CreatureScript
{
public:
    npc_orc_scout_qww() : CreatureScript("npc_orc_scout_qww") {}

    enum
    {
        SPELL_SHOOTING   = 15620,
        EVENT_SHOOTING    = 1,
        NPC_POISON_SPITTER = 35896
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_orc_scout_qwwAI(creature);
    }

    struct npc_orc_scout_qwwAI : public ScriptedAI
    {
        npc_orc_scout_qwwAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            me->setActive(true);
            events.ScheduleEvent(EVENT_SHOOTING, 2 * IN_MILLISECONDS);
            Reset();
        }

        void DamageTaken(Unit* /*killer*/, uint32 &damage)
        {
            if (me->HealthBelowPctDamaged(70, damage))
                damage = 0;
        }

        void UpdateAI(uint32 const diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_SHOOTING:
                    {
                        std::list<Creature*> spitters;
                        me->GetCreatureListWithEntryInGrid(spitters, NPC_POISON_SPITTER, 30.0f);
                        if (!spitters.empty())
                        {
                            Trinity::Containers::RandomResizeList(spitters, 1);

                            for(auto creature : spitters)
                                me->CastSpell(creature, SPELL_SHOOTING, false);
                        }
                    }
                    events.ScheduleEvent(EVENT_SHOOTING, 2 * IN_MILLISECONDS);
                    break;
                }
            }
        }
    };
};

class npc_poison_spitter_qww : public CreatureScript
{
public:
    npc_poison_spitter_qww() : CreatureScript("npc_poison_spitter_qww") {}

    enum
    {
        SPELL_POISON      = 68207,
        SPELL_NATURE      = 68208,
        SPELL_FROST       = 68209,
        SPELL_SHOOTING    = 15620,

        NPC_POISON_SPITTER = 35896
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_poison_spitter_qwwAI(creature);
    }

    struct npc_poison_spitter_qwwAI : public ScriptedAI
    {
        npc_poison_spitter_qwwAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT | UNIT_FLAG_DISABLE_MOVE);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_SHOOTING)
            {
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                {
                    switch(urand(0, 2))
                    {
                        case 0:
                            me->CastSpell(caster, SPELL_POISON, false);
                            break;
                        case 1:
                            me->CastSpell(caster, SPELL_NATURE, false);
                            break;
                        case 2:
                            me->CastSpell(caster, SPELL_FROST, false);
                            break;
                    }
                }
            }
        }
    };
};

class npc_ww_channel_bunny : public CreatureScript
{
public:
    npc_ww_channel_bunny() : CreatureScript("npc_ww_channel_bunny") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ww_channel_bunnyAI(creature);
    }

    enum eSpells
    {
        SPELL_RIDE    = 68217,
        SPELL_CHANNEL = 68214
    };

    struct npc_ww_channel_bunnyAI : public ScriptedAI
    {
        npc_ww_channel_bunnyAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* summoner)
        {
            me->CastSpell(summoner, SPELL_RIDE, false);
            me->CastSpell(summoner, SPELL_CHANNEL, false);
        }
    };
};

class npc_stangle_vine_qww : public CreatureScript
{
public:
    npc_stangle_vine_qww() : CreatureScript("npc_stangle_vine_qww") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_stangle_vine_qwwAI(creature);
    }

    enum eTalks
    {
        TALK_RANDOM  = 0,
        NPC_ORC_SCOUT = 36042
    };

    struct npc_stangle_vine_qwwAI : public ScriptedAI
    {
        npc_stangle_vine_qwwAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT | UNIT_FLAG_DISABLE_MOVE);
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (!apply)
            {
                if (who->ToCreature())
                {
                    if (who->GetEntry() == NPC_ORC_SCOUT)
                    {
                        who->ToCreature()->DespawnOrUnsummon(3 * IN_MILLISECONDS);
                        who->ToCreature()->AI()->Talk(TALK_RANDOM, me->GetGUID());
                    }
                }
            }
        }
    };
};

class spell_weed_whacker_qww : public SpellScriptLoader
{
public:
    spell_weed_whacker_qww() : SpellScriptLoader("spell_weed_whacker_qww") { }

    enum
    {
        SPELL_WEED_WHACKER_EFFECT = 68212,
        SPELL_SUMMON_BUNNY        = 68216
    };

    class spell_weed_whacker_qww_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_weed_whacker_qww_SpellScript);

        void HandleDummyLaunch(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();

            caster->CastSpell((Unit*)NULL, SPELL_WEED_WHACKER_EFFECT, false);
            caster->CastSpell(caster, SPELL_SUMMON_BUNNY, false);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_weed_whacker_qww_SpellScript::HandleDummyLaunch, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_weed_whacker_qww_SpellScript();
    }
};

class npc_aggra_qbta : public CreatureScript
{
public:
    npc_aggra_qbta() : CreatureScript("npc_aggra_qbta") {}

    enum
    {
        TALK_QA                = 0,
        TALK_QA2               = 1,

        QUEST_VICIOUS_VALE     = 14235,
        QUEST_FORWARD_MOVEMENT = 14237,

        SPELL_PHASE_01         = 67851,
        SPELL_PHASE_02         = 67852
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_VICIOUS_VALE:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA, player->GetGUID(), true);
                break;
            case QUEST_FORWARD_MOVEMENT:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA2, player->GetGUID(), true);

                player->RemoveAura(SPELL_PHASE_01);
                player->CastSpell(player, SPELL_PHASE_02, false);
               break;
        }
        player->SaveToDB();
        return true;
    }
};

class npc_kilag_gorefang_qii: public CreatureScript
{
public:
    npc_kilag_gorefang_qii() : CreatureScript("npc_kilag_gorefang_qii") {}

    enum
    {
        TALK_QA                   = 0,
        TALK_QA2                  = 1,

        QUEST_INFRARED_INFRADEAD  = 14238,
        QUEST_TO_THE_CLIFFS       = 14240,

        SPELL_QII_ACCEPT           = 68344,
        SPELL_QII_DESPAWN_SCOUT    = 68337,
        SPELL_INFRARED_HEAT_FOCALS = 69303,
        SPELL_ORC_SCOUT            = 68338,
        SPELL_TO_THE_CLIFFS_QA     = 68973,
        SPELL_PHASE_2              = 67852,
        SPELL_PHASE_3              = 67853
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_INFRARED_INFRADEAD:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA, player->GetGUID(), true);

                player->CastSpell(player, SPELL_QII_ACCEPT, false);
                break;
            case QUEST_TO_THE_CLIFFS:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA2, player->GetGUID(), true);

                player->RemoveAura(SPELL_PHASE_2);
                player->CastSpell(player, SPELL_PHASE_3, false);
                player->CastSpell(player, SPELL_TO_THE_CLIFFS_QA, false);
                break;
        }
        player->SaveToDB();
        return true;
    }

    bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_INFRARED_INFRADEAD)
        {
            player->RemoveAura(SPELL_INFRARED_HEAT_FOCALS);
            player->RemoveAura(SPELL_ORC_SCOUT);
            player->CastSpell(player, SPELL_QII_DESPAWN_SCOUT, false);
        }
        player->SaveToDB();
        return true;
    }
};

class OrcScoutTargetSelector
{
public:
    OrcScoutTargetSelector(Unit* caster) : _caster(caster) { }

    enum eNpc
    {
        NPC_ORC_SCOUT = 36100
    };

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (cre->IsSummon() && cre->GetEntry() == NPC_ORC_SCOUT && cre->ToTempSummon()->GetOwner() == _caster)
                return false;

        return true;
    }

private:
    Unit* _caster;
};

class spell_despawn_orc_scout_qii : public SpellScriptLoader
{
public:
    spell_despawn_orc_scout_qii() : SpellScriptLoader("spell_despawn_orc_scout_qii") { }

    class spell_despawn_orc_scout_qii_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_despawn_orc_scout_qii_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Creature* target = GetHitCreature())
                target->DespawnOrUnsummon();
        }

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (Unit* caster = GetCaster())
                targets.remove_if(OrcScoutTargetSelector(caster));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_despawn_orc_scout_qii_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_despawn_orc_scout_qii_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_despawn_orc_scout_qii_SpellScript();
    }
};

class npc_orc_scout_qii : public CreatureScript
{
public:
    npc_orc_scout_qii() : CreatureScript("npc_orc_scout_qii") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_orc_scout_qiiAI(creature);
    }

    enum
    {
        TALK_SUMMONED        = 0,

        SPELL_BATTLE_SHOUT   = 69305,
        SPELL_REND           = 11977,
        SPELL_HEROC_STRIKE   = 25710,
        SPELL_THUNDERCLAP    = 8078,
        SPELL_ORC_SCOUT      = 68338,

        EVENT_BATTLE_SHOUT   = 1,
        EVENT_REND           = 2,
        EVENT_HEROIC_STRIKE  = 3,
        EVENT_THUNDERCLAP    = 4
    };

    struct npc_orc_scout_qiiAI : public ScriptedAI
    {
        npc_orc_scout_qiiAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            Talk(TALK_SUMMONED, summoner->GetGUID(), true);
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_BATTLE_SHOUT, 3 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_REND, urand(5, 7) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_HEROIC_STRIKE, urand(8, 10) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_THUNDERCLAP, urand(11, 13) * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            if (Unit* owner = me->ToTempSummon()->GetOwner())
                owner->RemoveAura(SPELL_ORC_SCOUT);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_BATTLE_SHOUT:
                        me->CastSpell((Unit*)NULL, SPELL_BATTLE_SHOUT, false);
                        events.ScheduleEvent(EVENT_BATTLE_SHOUT, 20 * IN_MILLISECONDS);
                        break;
                    case EVENT_REND:
                        me->CastSpell((Unit*)NULL, SPELL_REND, false);
                        events.ScheduleEvent(EVENT_REND, urand(5, 7) * IN_MILLISECONDS);
                        break;
                    case EVENT_HEROIC_STRIKE:
                        me->CastSpell((Unit*)NULL, SPELL_HEROC_STRIKE, false);
                        events.ScheduleEvent(EVENT_HEROIC_STRIKE, urand(8, 10) * IN_MILLISECONDS);
                        break;
                    case EVENT_THUNDERCLAP:
                        me->CastSpell((Unit*)NULL, SPELL_THUNDERCLAP, false);
                        events.ScheduleEvent(EVENT_THUNDERCLAP, urand(11, 13) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_si_assasin_qii : public CreatureScript
{
public:
    npc_si_assasin_qii() : CreatureScript("npc_si_assasin_qii") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_si_assasin_qiiAI(creature);
    }

    enum
    {
        SPELL_SINISTER_STRIKE = 14873,
        SPELL_INVISIBLE_AURA  = 68322,
        SPELL_HEAT_SIGNATURE  = 76354,

        EVENT_SINISTER_STRIKE = 1
    };

    struct npc_si_assasin_qiiAI : public ScriptedAI
    {
        npc_si_assasin_qiiAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            me->CastSpell((Unit*)NULL, SPELL_INVISIBLE_AURA, false);
            me->CastSpell((Unit*)NULL, SPELL_HEAT_SIGNATURE, false);
            Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_SINISTER_STRIKE, 3 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (events.ExecuteEvent() == EVENT_SINISTER_STRIKE)
            {
                me->CastSpell((Unit*)NULL, SPELL_SINISTER_STRIKE, false);
                events.ScheduleEvent(EVENT_SINISTER_STRIKE, urand(3, 7) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

const float BastiaWPQTTC[29][3] =
{
    { 867.504f, 2842.019f, 102.956f  },
    { 867.684f, 2858.719f, 101.455f  },
    { 871.851f, 2868.342f, 100.859f  },
    { 881.596f, 2872.387f, 100.571f  },
    { 894.561f, 2874.603f, 99.789f   },
    { 905.231f, 2880.923f, 100.019f  },
    { 913.823f, 2890.542f, 99.947f   },
    { 920.299f, 2902.661f, 100.853f  },
    { 934.478f, 2912.587f, 103.701f  },
    { 940.675f, 2918.933f, 105.438f  },
    { 946.172f, 2938.263f, 108.768f  },
    { 970.127f, 2947.259f, 110.135f  },
    { 1024.916f, 2948.956f, 109.137f },
    { 1044.539f, 2966.852f, 111.714f },
    { 1052.031f, 2979.538f, 113.035f },
    { 1068.768f, 2990.980f, 115.915f },
    { 1074.980f, 3000.065f, 117.523f },
    { 1075.094f, 3019.419f, 120.954f },
    { 1079.446f, 3028.739f, 122.222f },
    { 1086.619f, 3040.036f, 122.970f },
    { 1087.516f, 3057.295f, 123.710f },
    { 1074.164f, 3092.691f, 125.126f },
    { 1055.105f, 3111.597f, 125.522f },
    { 1028.890f, 3127.004f, 124.860f },
    { 1022.303f, 3148.233f, 123.149f },
    { 1021.835f, 3162.353f, 120.758f },
    { 1047.746f, 3185.462f, 116.136f },
    { 1057.014f, 3202.407f, 111.368f },
    { 1060.362f, 3223.444f, 97.805f  }
};

class npc_bastia_qttc : public CreatureScript
{
public:
    npc_bastia_qttc() : CreatureScript("npc_bastia_qttc") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bastia_qttcAI(creature);
    }

    enum
    {
        EVENT_DONE                = 1,
        EVENT_JUMP                = 2,

        SPELL_RIDING_BASTIA        = 68974
    };

    struct npc_bastia_qttcAI : public VehicleAI
    {
        npc_bastia_qttcAI(Creature* creature) : VehicleAI(creature) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            summoner->CastSpell(me, SPELL_RIDING_BASTIA, false);

            Movement::MoveSplineInit init(me);
            for(uint8 i = 0; i < 29; ++i)
            {
                G3D::Vector3 path(BastiaWPQTTC[i][0], BastiaWPQTTC[i][1], BastiaWPQTTC[i][2]);
                init.Path().push_back(path);
            }
            init.SetSmooth();
            init.SetUncompressed();
            init.SetVelocity(14.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_JUMP, me->GetSplineDuration());
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_DONE:
                        if (Vehicle* vehicle = me->GetVehicleKit())
                           vehicle->RemoveAllPassengers();
                        me->DespawnOrUnsummon();
                        break;
                    case EVENT_JUMP:
                        me->GetMotionMaster()->MoveJump(1079.635f, 3239.743f, 81.517f, 15.0f, 17.0f);
                        events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
                        break;
                }
            }
        }
    };
};

class npc_generic_qgttg : public CreatureScript
{
public:
    npc_generic_qgttg() : CreatureScript("npc_generic_qgttg") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_generic_qgttgAI(creature);
    }

    enum eNpc
    {
        NPC_ORC_BATTLESWORN = 36104,
        NPC_AGGRA           = 36115,
        NPC_KILLAG_GOREFANG = 36117,
        NPC_CHAWG           = 36464
    };

    struct npc_generic_qgttgAI : public ScriptedAI
    {
        npc_generic_qgttgAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI()
        {
            switch(me->GetEntry())
            {
                case NPC_ORC_BATTLESWORN:
                case NPC_AGGRA:
                case NPC_KILLAG_GOREFANG:
                case NPC_CHAWG:
                    if (Unit* target = me->FindNearestCreature(36103, 1.7f))
                       AttackStart(target);
                break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }
    };
};

const float GyrochoppaWPQPC[6][3] =
{
    { 846.743f, 3335.536f, 10.14332f },
    { 775.8854f, 3374.22f, 14.39331f },
    { 780.3264f, 3449.157f, 14.39331f },
    { 1071.618f, 3589.423f, 26.22275f },
    { 1098.778f, 3729.424f, 93.02834f },
    { 971.5139f, 3802.339f, 14.36161f }
};

class npc_gyrochoppa_pilot_qgttg : public CreatureScript
{
public:
    npc_gyrochoppa_pilot_qgttg() : CreatureScript("npc_gyrochoppa_pilot_qgttg") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_gyrochoppa_pilot_qgttgAI(creature);
    }

    enum eTalks
    {
        TALK_AGGRO = 0
    };

    struct npc_gyrochoppa_pilot_qgttgAI : public ScriptedAI
    {
        npc_gyrochoppa_pilot_qgttgAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI()
        {
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
            Reset();
        }

        void EnterCombat(Unit* who)
        {
            Talk(TALK_AGGRO, who->GetGUID());
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
        }
    };
};

class npc_gyrochoppa_questgiver_qpc: public CreatureScript
{
public:
    npc_gyrochoppa_questgiver_qpc() : CreatureScript("npc_gyrochoppa_questgiver_qpc") {}

    enum
    {
        QUEST_PRECIOUS_CARGO       = 14242,

        SPELL_PRECIOUS_CARGO_AGGRO = 68386,
        SPELL_RIDE_GYROCHOPPA      = 68387,
        SPELL_EXPLOSION            = 69081,

        EVENT_DONE                 = 1
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_PRECIOUS_CARGO)
            player->CastSpell(player, SPELL_PRECIOUS_CARGO_AGGRO, false);

        return true;
    }
};

class npc_gyrochoppa_vehicle_qpc : public CreatureScript
{
public:
    npc_gyrochoppa_vehicle_qpc() : CreatureScript("npc_gyrochoppa_vehicle_qpc") {}

    enum eQuests
    {
        QUEST_PRECIOUS_CARGO       = 14242,

        SPELL_PRECIOUS_CARGO_AGGRO = 68386,
        SPELL_RIDE_GYROCHOPPA      = 68387,
        SPELL_EXPLOSION            = 69081,
        SPELL_EJECT_PASSENGERS     = 68576,

        EVENT_DONE                 = 1
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_gyrochoppa_vehicle_qpcAI(creature);
    }

    struct npc_gyrochoppa_vehicle_qpcAI : public VehicleAI
    {
        npc_gyrochoppa_vehicle_qpcAI(Creature* creature) : VehicleAI(creature) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            summoner->CastSpell(me, SPELL_RIDE_GYROCHOPPA, false);

            Movement::MoveSplineInit init(me);
            for(uint8 i = 0; i < 6; ++i)
            {
                G3D::Vector3 path(GyrochoppaWPQPC[i][0], GyrochoppaWPQPC[i][1], GyrochoppaWPQPC[i][2]);
                init.Path().push_back(path);
            }
            init.SetFly();
            init.SetSmooth();
            init.SetUncompressed();
            init.SetVelocity(24.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_DONE:
                        me->CastSpell(me, SPELL_EJECT_PASSENGERS, true);

                        if (Unit* summoner = me->ToTempSummon()->GetOwner())
                           summoner->CastSpell(summoner, SPELL_EXPLOSION, false);

                        me->DespawnOrUnsummon();
                        break;
                }
            }
        }
    };
};

class npc_arcane_wizard_qpc : public CreatureScript
{
public:
    npc_arcane_wizard_qpc() : CreatureScript("npc_arcane_wizard_qpc") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_arcane_wizard_qpcAI(creature);
    }

    enum
    {
        QUEST_PRECIOUS_CARGO    = 14242,

        SPELL_ARCANE_CHANNELING = 54219,
        SPELL_FIREBALL          = 9053,

        EVENT_FIREBALL          = 1,

        NPC_THRALL_CREDIT       = 36145,

        GO_ARCANE_CAGE          = 195704,
    };

    struct npc_arcane_wizard_qpcAI : public ScriptedAI
    {
        npc_arcane_wizard_qpcAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            me->CastSpell((Unit*)NULL, SPELL_ARCANE_CHANNELING, false);
        }

        void EnterCombat(Unit* who)
        {
            if (me->HasAura(SPELL_ARCANE_CHANNELING))
                me->RemoveAura(SPELL_ARCANE_CHANNELING);

            events.ScheduleEvent(EVENT_FIREBALL, 2 * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            if (GameObject* cage = me->FindNearestGameObject(GO_ARCANE_CAGE, 50.0f))
                if (cage->GetGoState() == GO_STATE_READY)
                    cage->UseDoorOrButton();

            if (killer->GetTypeId() == TYPEID_PLAYER)
                if (killer->ToPlayer()->GetQuestStatus(QUEST_PRECIOUS_CARGO) == QUEST_STATUS_INCOMPLETE)
                    killer->ToPlayer()->KilledMonsterCredit(NPC_THRALL_CREDIT);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_FIREBALL)
            {
                me->CastSpell((Unit*)NULL, SPELL_FIREBALL, false);
                events.ScheduleEvent(EVENT_FIREBALL, urand(5, 7) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_thrall_qmmut : public CreatureScript
{
public:
    npc_thrall_qmmut() : CreatureScript("npc_thrall_qmmut") {}

    enum
    {
        QUEST_MEET_ME_UP_TOP = 14326,

        SPELL_PHASE_4        = 67854,
        SPELL_PHASE_3        = 67853,
        SPELL_RAIN           = 82651, // HACK!!

        TALk_QA              = 0
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_MEET_ME_UP_TOP)
        {
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALk_QA, player->GetGUID(), true);

            player->RemoveAura(SPELL_PHASE_3);
            player->CastSpell(player, SPELL_PHASE_4, false);
            player->CastSpell(player, SPELL_RAIN, false);
        }

        return true;
    }
};

class npc_thrall_qwr : public CreatureScript
{
public:
    npc_thrall_qwr() : CreatureScript("npc_thrall_qwr") {}

    enum
    {
        QUEST_WERCHIEFS_REVENGE   = 14243,

        SPELL_QWR_ACCEPT           = 68408,
        SPELL_LIGHTNING_CHANNELING = 71043,
        SPELL_CHAIN                = 68440,

        TALk_RANDOM                = 0,

        EVENT_RANDOM_TALK          = 1,
        EVENT_CHAIN                = 2
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_WERCHIEFS_REVENGE)
            player->CastSpell(player, SPELL_QWR_ACCEPT, false);

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_thrall_qwrAI(creature);
    }

    struct npc_thrall_qwrAI : public ScriptedAI
    {
        npc_thrall_qwrAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            me->CastSpell((Unit*)NULL, SPELL_LIGHTNING_CHANNELING, false);
            events.ScheduleEvent(EVENT_RANDOM_TALK, 14 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_CHAIN, 35 * IN_MILLISECONDS);
            Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_RANDOM_TALK:
                        Talk(TALk_RANDOM);
                        events.ScheduleEvent(EVENT_RANDOM_TALK, 14 * IN_MILLISECONDS);
                        break;
                    case EVENT_CHAIN:
                        me->CastSpell((Unit*)NULL, SPELL_CHAIN, false);
                        events.ScheduleEvent(EVENT_CHAIN, 35 * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };
};

class npc_inisible_stalker_qwr : public CreatureScript
{
public:
    npc_inisible_stalker_qwr() : CreatureScript("npc_inisible_stalker_qwr") {}

    enum
    {
        SPELL_CHAIN = 68441,
        SPELL_FIRE  = 42345,

        EVENT_CHAIN = 1
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_inisible_stalker_qwrAI(creature);
    }

    struct npc_inisible_stalker_qwrAI : public ScriptedAI
    {
        npc_inisible_stalker_qwrAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_CHAIN, 35 * IN_MILLISECONDS);
            Reset();
        }

        void UpdateAI(uint32 const diff) // HACK! Need to fix spell_chain, when spell_chain hits stalker, stalker casts spell spell_fire,but chain spells not works with 38 target :(
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_CHAIN)
            {
                me->CastSpell((Unit*)NULL, SPELL_CHAIN, false);
                me->CastSpell((Unit*)NULL, SPELL_FIRE, false);
                events.ScheduleEvent(EVENT_CHAIN, 14 * IN_MILLISECONDS);
            }
        }
    };
};

const float CycloneCyclicWPQWR[22][3] =
{
    { 1042.181f, 3857.24f, 23.60183f  },
    { 1071.38f, 3852.879f, 21.18516f  },
    { 1097.259f, 3826.478f, 18.90739f },
    { 1097.905f, 3780.767f, 20.29626f },
    { 1062.802f, 3755.345f, 22.85184f },
    { 1033.106f, 3773.803f, 23.60179f },
    { 1021.788f, 3827.314f, 20.99073f },
    { 1004.835f, 3861.208f, 19.96292f },
    { 962.5104f, 3859.353f, 20.29623f },
    { 937.9149f, 3841.934f, 19.26847f },
    { 903.8854f, 3827.217f, 19.57398f },
    { 867.1771f, 3849.851f, 21.32399f },
    { 866.3246f, 3895.795f, 21.51845f },
    { 893.6545f, 3935.708f, 20.87961f },
    { 936.5087f, 3944.062f, 21.12957f },
    { 968.7101f, 3914.839f, 20.3796f  },
    { 952.6996f, 3861.26f, 24.54637f  },
    { 936.1754f, 3820.981f, 21.93524f },
    { 943.9549f, 3785.924f, 22.26854f },
    { 975.882f, 3771.541f, 20.71306f  },
    { 1004.325f, 3782.394f, 21.71304f },
    { 1022.352f, 3831.351f, 24.62974f }
};

const float CycloneIntroWPQWR[3][3] =
{
    { 984.954f, 3826.142f, 10.025f },
    { 995.303f, 3830.475f, 11.021f },
    { 1005.217f, 3832.418f, 14.525f}
};

const float CycloneDoneWPQWR[9][3] =
{
    { 956.4236f, 3740.38f, 70.96297f  },
    { 950.8785f, 3662.27f, 14.79625f  },
    { 915.7361f, 3451.169f, 2.296269f },
    { 907.1424f, 3327.42f, 23.4872f   },
    { 960.3212f, 3212.715f, 111.3099f },
    { 995.8976f, 3143.984f, 145.0299f },
    { 1007.958f, 3070.123f, 147.641f  },
    { 901.7518f, 2926.978f, 147.641f  },
    { 872.4948f, 2765.59f, 119.8911f  }
};

class npc_cyclone_of_elements_qwr : public CreatureScript
{
public:
    npc_cyclone_of_elements_qwr() : CreatureScript("npc_cyclone_of_elements_qwr") {}

    enum
    {
        EVENT_CYCLIC_PATH  = 1,

        SPELL_RIDE_CYCLONE = 68436,
        SPELL_RAIN         = 82651, // HACK!!

        ACTION_DONE        = 1
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_cyclone_of_elements_qwrAI(creature);
    }

    struct npc_cyclone_of_elements_qwrAI : public VehicleAI
    {
        npc_cyclone_of_elements_qwrAI(Creature* creature) : VehicleAI(creature) {}

        EventMap events;

        void DoAction(const int32 actionId)
        {
            if (actionId == ACTION_DONE)
            {
                me->StopMoving();
                if (Unit* owner = me->ToTempSummon()->GetOwner())
                    if (owner->HasAura(SPELL_RAIN))
                        owner->RemoveAura(SPELL_RAIN);

                Movement::MoveSplineInit init(me);
                for(uint8 i = 0; i < 9; ++i)
                {
                    G3D::Vector3 path(CycloneDoneWPQWR[i][0], CycloneDoneWPQWR[i][1], CycloneDoneWPQWR[i][2]);
                    init.Path().push_back(path);
                }
                init.SetFly();
                init.SetSmooth();
                init.SetUncompressed();
                init.SetVelocity(21.5f);
                init.Launch();

                me->DespawnOrUnsummon(me->GetSplineDuration());
            }
        }

        void IsSummonedBy(Unit* summoner)
        {
            summoner->CastSpell((Unit*)NULL, SPELL_RIDE_CYCLONE, false);

            Movement::MoveSplineInit init(me);
            for(uint8 i = 0; i < 3; ++i)
            {
                G3D::Vector3 path(CycloneIntroWPQWR[i][0], CycloneIntroWPQWR[i][1], CycloneIntroWPQWR[i][2]);
                init.Path().push_back(path);
            }
            init.SetFly();
            init.SetSmooth();
            init.SetUncompressed();
            init.SetVelocity(12.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_CYCLIC_PATH, me->GetSplineDuration());
        }

        void UpdateAI(uint32 const diff)
        {
            VehicleAI::UpdateAI(diff);
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_CYCLIC_PATH:
                    {
                        Movement::MoveSplineInit init(me);
                        for(uint8 i = 0; i < 22; ++i)
                        {
                            G3D::Vector3 path(CycloneCyclicWPQWR[i][0], CycloneCyclicWPQWR[i][1], CycloneCyclicWPQWR[i][2]);
                            init.Path().push_back(path);
                        }
                        init.SetFly();
                        init.SetSmooth();
                        init.SetCyclic();
                        init.SetUncompressed();
                        init.SetVelocity(13.0f);
                        init.Launch();
                    }
                    break;
                }
            }
        }
    };
};

class spell_cyclone_of_elements_qwr_periodic : public SpellScriptLoader
{
public:
    spell_cyclone_of_elements_qwr_periodic() : SpellScriptLoader("spell_cyclone_of_elements_qwr_periodic") { }

    enum eQuests
    {
        QUEST_WERCHIEFS_REVENGE = 14243
    };

    class spell_cyclone_of_elements_qwr_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_cyclone_of_elements_qwr_periodic_AuraScript);

        bool _done;

        bool Load()
        {
            _done = false;
            return true;
        }

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();
            Unit* owner = GetCaster();

            if (owner->GetTypeId() == TYPEID_PLAYER)
            {
                if (owner->ToPlayer()->GetQuestStatus(QUEST_WERCHIEFS_REVENGE) == QUEST_STATUS_COMPLETE && !_done)
                {
                    _done = true;
                    if (Creature* vehicleBase = owner->GetVehicleCreatureBase())
                        vehicleBase->AI()->DoAction(1);
                }
                else if (owner->ToPlayer()->GetQuestStatus(QUEST_WERCHIEFS_REVENGE) == QUEST_STATUS_NONE)
                    if (Creature* vehicleBase = owner->GetVehicleCreatureBase())
                        vehicleBase->DespawnOrUnsummon();
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_cyclone_of_elements_qwr_periodic_AuraScript::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_cyclone_of_elements_qwr_periodic_AuraScript();
    }
};

class npc_sassy_hardwrench_quua : public CreatureScript
{
public:
    npc_sassy_hardwrench_quua() : CreatureScript("npc_sassy_hardwrench_quua") {}

    enum
    {
        QUEST_UP_UP_AND_AWAY = 14244,
        SPELL_QA             = 68815,
        TALk_QA               = 0
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_UP_UP_AND_AWAY)
        {
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALk_QA, player->GetGUID(), true);

            creature->CastSpell((Unit*)NULL, SPELL_QA, false);
        }

        return true;
    }
};

class npc_trade_prince_gallywix_quua : public CreatureScript
{
public:
    npc_trade_prince_gallywix_quua() : CreatureScript("npc_trade_prince_gallywix_quua") {}

    enum
    {
        TALK_1             = 0,
        TALK_2             = 1,
        TALK_3             = 2,

        EVENT_TALK_2        = 1,
        EVENT_TALK_3        = 2,
        EVENT_AWAY          = 3,
        EVENT_SUUMON        = 4,

        SPELL_SUMMON_ROCKET = 68806
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_trade_prince_gallywix_quuaAI(creature);
    }

    struct npc_trade_prince_gallywix_quuaAI : public ScriptedAI
    {
        npc_trade_prince_gallywix_quuaAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            Talk(TALK_1, summoner->GetGUID());
            events.ScheduleEvent(EVENT_TALK_2, 7.4 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_TALK_2:
                        Talk(TALK_2);
                        events.ScheduleEvent(EVENT_TALK_3, 10.4 * IN_MILLISECONDS);
                        break;
                    case EVENT_TALK_3:
                        Talk(TALK_3);
                        events.ScheduleEvent(EVENT_AWAY, 6.5 * IN_MILLISECONDS);
                        break;
                    case EVENT_AWAY:
                        {
                            Movement::MoveSplineInit init(me);
                            init.MoveTo(870.494f, 2741.229f, 122.870f);
                            init.Launch();
                        }
                        events.ScheduleEvent(EVENT_SUUMON, me->GetSplineDuration());
                        break;
                    case EVENT_SUUMON:
                        me->CastSpell(me, SPELL_SUMMON_ROCKET, false);
                        break;
                }
            }
        }
    };
};

const float SlingRocketWPQUUA[8][3] =
{
    { 882.4566f, 2726.064f, 146.0856f },
    { 890.9149f, 2697.69f, 164.5292f  },
    { 916.3698f, 2591.611f, 207.5022f },
    { 928.4583f, 2538.972f, 196.3079f },
    { 932.9393f, 2506.701f, 167.4186f },
    { 941.3195f, 2463.62f, 111.2246f  },
    { 945.6337f, 2440.615f, 69.7246f  },
    { 945.2621f, 2396.825f, 4.585697f }
};

class npc_sling_rocket_quua : public CreatureScript
{
public:
    npc_sling_rocket_quua() : CreatureScript("npc_sling_rocket_quua") {}

    enum eSpells
    {
        SPELL_EXPLOSION        = 68813,
        SPELL_ROCKET_BLAST     = 66110,
        SPELL_EJECT_PASSENGERS = 68576
    };

    enum eEvents
    {
        EVENT_DONE             = 1
    };

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sling_rocket_quuaAI(creature);
    }

    struct npc_sling_rocket_quuaAI : public VehicleAI
    {
        npc_sling_rocket_quuaAI(Creature * c) : VehicleAI(c) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            me->CastSpell((Unit*)NULL, SPELL_ROCKET_BLAST, false);

            Movement::MoveSplineInit init(me);
            for(uint8 i = 0; i < 8; ++i)
            {
                G3D::Vector3 path(SlingRocketWPQUUA[i][0], SlingRocketWPQUUA[i][1], SlingRocketWPQUUA[i][2]);
                init.Path().push_back(path);
            }
            init.SetFly();
            init.SetSmooth();
            init.SetUncompressed();
            init.SetVelocity(20.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
        }

        void UpdateAI(uint32 const diff)
        {
            VehicleAI::UpdateAI(diff);
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_DONE:
                        me->CastSpell(me, SPELL_EJECT_PASSENGERS, true);

                        if (Unit* owner = me->ToTempSummon()->GetOwner())
                        {
                            if (owner->GetTypeId() == TYPEID_PLAYER)
                                owner->CastSpell(owner, SPELL_EXPLOSION, false);
                            else if (owner->ToCreature())
                                owner->ToCreature()->DespawnOrUnsummon();
                        }

                        me->DespawnOrUnsummon();
                        break;
                }
            }
        }
    };
};

class go_rocket_sling_quua : public GameObjectScript
{
public:
    go_rocket_sling_quua() : GameObjectScript("go_rocket_sling_quua") { }

    enum
    {
        SPELL_SUMMON_SLING_ROCKET = 68806,
        QUEST_UP_UP_AND_AWAY      = 14244,
        MENU_ROCKET_SLING         = 14985
    };

    bool OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        if (action == GOSSIP_ACTION_INFO_DEF + 1)
            player->CastSpell(player, SPELL_SUMMON_SLING_ROCKET, false);

        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if ((player->GetQuestStatus(QUEST_UP_UP_AND_AWAY) == QUEST_STATUS_INCOMPLETE))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Up, up & Away!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(MENU_ROCKET_SLING, go->GetGUID());
        return true;
    }
};

class npc_foreman_dampwick_qitib : public CreatureScript
{
public:
    npc_foreman_dampwick_qitib() : CreatureScript("npc_foreman_dampwick_qitib") {}

    enum
    {
        QUEST_ITS_A_TOWN_A_BOX = 14245,
        SPELL_QA               = 68815,
        TALk_QA                = 0
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_ITS_A_TOWN_A_BOX)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALk_QA, player->GetGUID(), true);

        return true;
    }
};

class spell_phase_05_lost_isles : public SpellScriptLoader
{
public:
    spell_phase_05_lost_isles() : SpellScriptLoader("spell_phase_05_lost_isles") { }

    enum eSpells
    {
        SPELL_PHASE_04 = 67854
    };

    class spell_phase_05_lost_isles_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_phase_05_lost_isles_SpellScript);

        void HandleBeforeCast()
        {
            if (Unit* caster = GetCaster())
                caster->RemoveAura(SPELL_PHASE_04);
        }

        void Register()
        {
            BeforeCast += SpellCastFn(spell_phase_05_lost_isles_SpellScript::HandleBeforeCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_phase_05_lost_isles_SpellScript();
    }
};

class npc_maxx_avalanche_wild_overlook : public CreatureScript
{
public:
    npc_maxx_avalanche_wild_overlook() : CreatureScript("npc_maxx_avalanche_wild_overlook") {}

    enum eEvents
    {
        EVENT_AURAS             = 1,

        SPELL_FLAMETONGUE_WEAPON = 78273,
        SPELL_LITGHTNING_SHIELD  = 12550
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_maxx_avalanche_wild_overlookAI(creature);
    }

    struct npc_maxx_avalanche_wild_overlookAI : public ScriptedAI
    {
        npc_maxx_avalanche_wild_overlookAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_AURAS, 2 * IN_MILLISECONDS);
            Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_AURAS)
            {
                me->CastSpell(me, SPELL_LITGHTNING_SHIELD, false);
                me->CastSpell(me, SPELL_FLAMETONGUE_WEAPON, false);
                events.ScheduleEvent(EVENT_AURAS, 600 * IN_MILLISECONDS);
            }
        }
    };
};

class npc_evol_fingers_wild_overlook : public CreatureScript
{
public:
    npc_evol_fingers_wild_overlook() : CreatureScript("npc_evol_fingers_wild_overlook") {}

    enum eEvents
    {
        EVENT_RANDOM_TALK = 1,
        TALK_RANDOM       = 0
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_evol_fingers_wild_overlookAI(creature);
    }

    struct npc_evol_fingers_wild_overlookAI : public ScriptedAI
    {
        npc_evol_fingers_wild_overlookAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_RANDOM_TALK, 2 * IN_MILLISECONDS);
            Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_RANDOM_TALK)
            {
                Talk(TALK_RANDOM);
                events.ScheduleEvent(EVENT_RANDOM_TALK, 120 * IN_MILLISECONDS);
            }
        }
    };
};

class npc_terraptor_matriarch_lost_isles : public CreatureScript
{
public:
    npc_terraptor_matriarch_lost_isles() : CreatureScript("npc_terraptor_matriarch_lost_isles") {}

    enum
    {
        EVENT_ROAR  = 1,
        EVENT_WOUND = 2,

        SPELL_ROAR  = 36629,
        SPELL_WOUND = 35321
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_terraptor_matriarch_lost_islesAI(creature);
    }

    struct npc_terraptor_matriarch_lost_islesAI : public ScriptedAI
    {
        npc_terraptor_matriarch_lost_islesAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            me->setActive(true);
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_ROAR, urand(17, 25) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_WOUND, urand(12, 20) * IN_MILLISECONDS);
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
                switch(eventId)
                {
                    case EVENT_ROAR:
                        me->CastSpell((Unit*)NULL, SPELL_ROAR, false);
                        events.ScheduleEvent(EVENT_ROAR, urand(17, 25) * IN_MILLISECONDS);
                        break;
                    case EVENT_WOUND:
                        me->CastSpell((Unit*)NULL, SPELL_WOUND, false);
                        events.ScheduleEvent(EVENT_WOUND, urand(12, 20) * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };
};

class npc_evol_fingers_town_in_one_box : public CreatureScript
{
public:
    npc_evol_fingers_town_in_one_box() : CreatureScript("npc_evol_fingers_town_in_one_box") {}

    enum
    {
        SPELL_VOIDWALER = 5108,
        SPELL_SHADOWN_BOLT = 73538,
        SPELL_CORRUPTION = 172,

        EVENT_SHADOW_BOLT = 1,
        EVENT_CORRUPTION  = 2
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_evol_fingers_town_in_one_boxAI(creature);
    }

    struct npc_evol_fingers_town_in_one_boxAI : public ScriptedAI
    {
        npc_evol_fingers_town_in_one_boxAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            me->CastSpell((Unit*)NULL, SPELL_VOIDWALER, false);
            Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(4, 6) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_CORRUPTION, urand(3, 7) * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
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
                switch(eventId)
                {
                    case EVENT_SHADOW_BOLT:
                        me->CastSpell(me->GetVictim(), SPELL_SHADOWN_BOLT, false);
                        events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(4, 6) * IN_MILLISECONDS);
                        break;
                    case EVENT_CORRUPTION:
                        me->CastSpell(me->GetVictim(), SPELL_CORRUPTION, false);
                        events.ScheduleEvent(EVENT_CORRUPTION, urand(3, 7) * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };
};

class npc_foreman_dampwick_qhny : public CreatureScript
{
public:
    npc_foreman_dampwick_qhny() : CreatureScript("npc_foreman_dampwick_qhny") {}

    enum
    {
        QUEST_HOBART_NEEDS_YOU     = 27139,
        QUEST_THREE_LITTLE_PYGMIES = 24945,

        TALk_QA                = 0,
        TALK_QA_2              = 1
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_HOBART_NEEDS_YOU:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALk_QA, player->GetGUID(), true);
                break;
            case QUEST_THREE_LITTLE_PYGMIES:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_2, player->GetGUID(), true);
                break;
        }

        return true;
    }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_foreman_dampwick_qhnyAI(creature);
    }

    struct npc_foreman_dampwick_qhnyAI : public ScriptedAI
    {
        npc_foreman_dampwick_qhnyAI(Creature* creature) : ScriptedAI(creature) {}

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }
    };
};

class npc_hobart_grapplehammer_qcc : public CreatureScript
{
public:
    npc_hobart_grapplehammer_qcc() : CreatureScript("npc_hobart_grapplehammer_qcc") {}

    enum
    {
        QUEST_CLUSTER_CLUCK          = 24671,
        QUEST_THE_BIGGEST_EGG_EVER   = 24744,
        QUEST_CHILDREN_OF_TURTLE_GOD = 24954,
        QUEST_VOLCANOTH              = 24958,
        QUEST_ESCAPE_VELOCITY        = 25214,

        TALk_QA             = 0,
        TALK_QA_TBEE        = 1,
        TALk_EGG_EVENT_1    = 2,
        TALK_EGG_EVENT_2    = 3,
        TALK_QA_2           = 4,
        TALK_QA_3           = 5,
        TALK_QA_4           = 6,

        TALK_QC             = 0,

        NPC_BAMM_MEGABOMB   = 38122,
        NPC_BUNNY           = 24021,
        NPC_GREELY          = 38124,

        EVENT_EGG_FOUNTAIN  = 1,
        EVENT_TALK_1        = 2,
        EVENT_TALK_2        = 3,
        EVENT_TALK_3        = 4,
        EVENT_TALK_4        = 5,
        EVENT_CANCEL_EVENT  = 6,

        ACTION_EGG_FOUNTAIN = 1,

        SPELL_EGG_FOUNTAIN = 71608
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_CLUSTER_CLUCK:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALk_QA, player->GetGUID(), true);
                break;
            case QUEST_THE_BIGGEST_EGG_EVER:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_TBEE, player->GetGUID(), true);
                break;
            case QUEST_CHILDREN_OF_TURTLE_GOD:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_2, player->GetGUID(), true);
                break;
            case QUEST_VOLCANOTH:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_3, player->GetGUID(), true);
                break;
            case QUEST_ESCAPE_VELOCITY:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_4, player->GetGUID(), true);
                break;
        }

        return true;
    }

    bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_CLUSTER_CLUCK)
        {
            if (Creature* bamm = creature->FindNearestCreature(NPC_BAMM_MEGABOMB, 15.0f))
                if (bamm->IsAIEnabled)
                    bamm->AI()->Talk(TALK_QC, player->GetGUID(), true);
        }
        return true;
    }

    bool OnQuestReward(Player* /*player*/, Creature* creature, Quest const* quest, uint32 /*item*/)
    {
        if (quest->GetQuestId() == QUEST_THE_BIGGEST_EGG_EVER)
        {
            if (creature->IsAIEnabled)
            {
                creature->AI()->Talk(TALk_EGG_EVENT_1);
                creature->AI()->DoAction(ACTION_EGG_FOUNTAIN);
            }
        }

        return true;
    }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_hobart_grapplehammer_qccAI(creature);
    }

    struct npc_hobart_grapplehammer_qccAI : public ScriptedAI
    {
        npc_hobart_grapplehammer_qccAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void DoAction(const int32 actionId)
        {
            if (actionId == ACTION_EGG_FOUNTAIN)
                events.ScheduleEvent(EVENT_TALK_1, 4 * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_TALK_1:
                        if (Creature* greely = me->FindNearestCreature(NPC_GREELY, 12.0f))
                           greely->AI()->Talk(0);
                        events.ScheduleEvent(EVENT_TALK_2, 5 * IN_MILLISECONDS);
                        break;
                    case EVENT_TALK_2:
                        if (Creature* greely = me->FindNearestCreature(NPC_GREELY, 12.0f))
                           greely->AI()->Talk(1);
                        events.ScheduleEvent(EVENT_TALK_3, 6 * IN_MILLISECONDS);
                        break;
                    case EVENT_TALK_3:
                       if (Creature* greely = me->FindNearestCreature(NPC_GREELY, 12.0f))
                           greely->AI()->Talk(2);
                        events.ScheduleEvent(EVENT_EGG_FOUNTAIN, 5 * IN_MILLISECONDS);
                        events.ScheduleEvent(EVENT_TALK_4, 7 * IN_MILLISECONDS);
                        break;
                    case EVENT_TALK_4:
                       Talk(TALK_EGG_EVENT_2);
                       events.ScheduleEvent(EVENT_CANCEL_EVENT, 7 * IN_MILLISECONDS);
                       break;
                    case EVENT_CANCEL_EVENT:
                       events.CancelEvent(EVENT_EGG_FOUNTAIN);
                       break;
                    case EVENT_EGG_FOUNTAIN:
                    {
                        std::list<Creature*> bunnies;
                        me->GetCreatureListWithEntryInGrid(bunnies, NPC_BUNNY, 30.0f);
                        if (!bunnies.empty())
                        {
                            for(auto creature : bunnies)
                                creature->CastSpell((Unit*)NULL, SPELL_EGG_FOUNTAIN, false);
                        }
                    }
                    events.ScheduleEvent(EVENT_EGG_FOUNTAIN, 1 * IN_MILLISECONDS);
                    break;
                }
            }
        }
    };
};

class npc_maxx_avalanche_town_in_one_box : public CreatureScript
{
public:
    npc_maxx_avalanche_town_in_one_box() : CreatureScript("npc_maxx_avalanche_town_in_one_box") {}

    enum
    {
        EVENT_AURAS          = 1,
        EVENT_LIGHTNING_BOLT = 2,
        EVENT_FLAME_SHOCK    = 3,

        SPELL_FLAMETONGUE_WEAPON = 78273,
        SPELL_LITGHTNING_SHIELD  = 12550,
        SPELL_STONE_SKIN_TOTEM   = 78222,
        SPELL_LIGHTNING_BOLT     = 57780,
        SPELL_FLAME_SHOCK        = 15039,
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_maxx_avalanche_town_in_one_boxAI(creature);
    }

    struct npc_maxx_avalanche_town_in_one_boxAI : public ScriptedAI
    {
        npc_maxx_avalanche_town_in_one_boxAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap comsmeticEvents;
        EventMap combatEvents;

        void InitializeAI()
        {
            comsmeticEvents.ScheduleEvent(EVENT_AURAS, 2 * IN_MILLISECONDS);
            Reset();
        }

        void Reset()
        {
            combatEvents.Reset();
        }

        void EnterCombat(Unit* who)
        {
            combatEvents.ScheduleEvent(EVENT_FLAME_SHOCK, urand(6, 8) * IN_MILLISECONDS);
            combatEvents.ScheduleEvent(EVENT_LIGHTNING_BOLT, urand(6, 8) * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (uint32 eventId = comsmeticEvents.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_AURAS:
                        me->CastSpell(me, SPELL_LITGHTNING_SHIELD, false);
                        me->CastSpell(me, SPELL_FLAMETONGUE_WEAPON, false);
                        me->CastSpell((Unit*)NULL, SPELL_STONE_SKIN_TOTEM, false);
                        comsmeticEvents.ScheduleEvent(EVENT_AURAS, 300 * IN_MILLISECONDS);
                        break;
                }
            }

            comsmeticEvents.Update(diff);
            combatEvents.Update(diff);

            if (!UpdateVictim())
                return;

            if (uint32 eventId = combatEvents.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_FLAME_SHOCK:
                        me->CastSpell(me->GetVictim(), SPELL_FLAME_SHOCK, false);
                        combatEvents.ScheduleEvent(EVENT_FLAME_SHOCK, urand(6, 8) * IN_MILLISECONDS);
                        break;
                    case EVENT_LIGHTNING_BOLT:
                        me->CastSpell(me->GetVictim(), SPELL_LIGHTNING_BOLT, false);
                        combatEvents.ScheduleEvent(EVENT_LIGHTNING_BOLT, urand(6, 8) * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };
};

class npc_wild_clucker_qcc : public CreatureScript
{
public:
    npc_wild_clucker_qcc() : CreatureScript("npc_wild_clucker_qcc") {}

    enum eSpells
    {
        SPELL_REMOTE_CONTROL_FIREWORKS = 71170,
        SPELL_FIREWORKS_VISUAL         = 74177,

        NPC_CLUSTER_CLUCK_QUEST_CREDIT = 38117,

        QUEST_CLUSTER_CLUCK            = 24671,

        EVENT_FLY                       = 1
    };

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_wild_clucker_qccAI(creature);
    }

    struct npc_wild_clucker_qccAI : public ScriptedAI
    {
        npc_wild_clucker_qccAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_REMOTE_CONTROL_FIREWORKS)
            {
                if (me->isMoving())
                    me->StopMoving();

                me->CastSpell((Unit*)NULL, SPELL_FIREWORKS_VISUAL, false);

                if (caster->GetTypeId() == TYPEID_PLAYER && caster->ToPlayer()->GetQuestStatus(QUEST_CLUSTER_CLUCK) == QUEST_STATUS_INCOMPLETE)
                    caster->ToPlayer()->KilledMonsterCredit(NPC_CLUSTER_CLUCK_QUEST_CREDIT);

                events.ScheduleEvent(EVENT_FLY, 2 * IN_MILLISECONDS);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_FLY)
            {
                Movement::MoveSplineInit init(me);
                init.MoveTo(903.623f, 2334.743f, 5.035f);
                init.SetFly();
                init.SetVelocity(18.0f);
                init.Launch();

                me->DespawnOrUnsummon(me->GetSplineDuration());
            }
        }
    };
};

class npc_bamm_megabomb_qtu : public CreatureScript
{
public:
    npc_bamm_megabomb_qtu() : CreatureScript("npc_bamm_megabomb_qtu") {}

    enum eQuests
    {
        QUEST_TRADING_UP = 24741,
        TALk_QA          = 1,
        SPELL_SHOT = 71509,
        EVENT_SHOT = 1
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_TRADING_UP)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALk_QA, player->GetGUID(), true);

        return true;
    }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_bamm_megabomb_qtuAI(creature);
    }

    struct npc_bamm_megabomb_qtuAI : public ScriptedAI
    {
        npc_bamm_megabomb_qtuAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_SHOT, urand(3, 5) * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SHOT)
            {
                me->CastSpell(me->GetVictim(), SPELL_SHOT, false);
                events.ScheduleEvent(EVENT_SHOT, urand(3, 5) * IN_MILLISECONDS);
            }
        }
    };
};

class npc_wild_clucker_egg_qtu : public CreatureScript
{
public:
    npc_wild_clucker_egg_qtu() : CreatureScript("npc_wild_clucker_egg_qtu") {}

    enum eNpc
    {
        NPC_SPINY_RAPTOR = 38187
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_wild_clucker_egg_qtuAI(creature);
    }

    struct npc_wild_clucker_egg_qtuAI : public ScriptedAI
    {
        npc_wild_clucker_egg_qtuAI(Creature * c) : ScriptedAI(c) {}

        void IsSummonedBy(Unit* summoner)
        {
            if (Creature* raptor = me->FindNearestCreature(NPC_SPINY_RAPTOR, 70.0f))
            {
                if (raptor->IsInCombat())
                {
                    raptor->CombatStop(true);
                    raptor->DeleteThreatList();
                }

                raptor->SetReactState(REACT_PASSIVE);
                raptor->GetMotionMaster()->MovePoint(1, *me);
            }
        }
    };
};

class npc_spiny_raptor_qtu: public CreatureScript
{
public:
    npc_spiny_raptor_qtu() : CreatureScript("npc_spiny_raptor_qtu") {}

    enum eSpells
    {
        SPELL_EXPLOSION   = 35309,
        SPELL_SUMMON_EGG  = 66726,
        SPELL_FEIGN_DEATH = 29266,
        GO_RAPTOR_TRAP   = 201972,
        EVENT_KILL       = 1
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_spiny_raptor_qtuAI(creature);
    }

    struct npc_spiny_raptor_qtuAI : public ScriptedAI
    {
        npc_spiny_raptor_qtuAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (id == 1)
                {
                    me->CastSpell((Unit*)NULL, SPELL_EXPLOSION, false);
                    me->CastSpell((Unit*)NULL, SPELL_SUMMON_EGG, false);

                    if (GameObject* trap = me->FindNearestGameObject(GO_RAPTOR_TRAP, 1.0f))
                        if (trap->GetGoState() == GO_STATE_READY)
                           trap->UseDoorOrButton();

                    events.ScheduleEvent(EVENT_KILL, 0.4 * IN_MILLISECONDS);
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_KILL)
            {
                me->DespawnOrUnsummon(5 * IN_MILLISECONDS);
                me->CastSpell((Unit*)NULL, 29266, false);
            }
        }
    };
};

class npc_ravenous_lurker_lost_isles : public CreatureScript
{
public:
    npc_ravenous_lurker_lost_isles() : CreatureScript("npc_ravenous_lurker_lost_isles") {}

    enum eEvents
    {
        EVENT_VICIOUS_BITE = 1,
        SPELL_VICIOUS_BITE = 69203
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ravenous_lurker_lost_islesAI(creature);
    }

    struct npc_ravenous_lurker_lost_islesAI : public ScriptedAI
    {
        npc_ravenous_lurker_lost_islesAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            me->setActive(true);
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_VICIOUS_BITE, 5 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_VICIOUS_BITE)
            {
                me->CastSpell((Unit*)NULL, SPELL_VICIOUS_BITE, false);
                events.ScheduleEvent(EVENT_VICIOUS_BITE, urand(15, 18) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_the_hammer_lost_isles : public CreatureScript
{
public:
    npc_the_hammer_lost_isles() : CreatureScript("npc_the_hammer_lost_isles") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_the_hammer_lost_islesAI(creature);
    }

    struct npc_the_hammer_lost_islesAI : public ScriptedAI
    {
        npc_the_hammer_lost_islesAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->setActive(true);
        }
    };
};

class npc_megachicken_qbee : public CreatureScript
{
public:
    npc_megachicken_qbee() : CreatureScript("npc_megachicken_qbee") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_megachicken_qbeesAI(creature);
    }

    enum
    {
        SPELL_BARRAGE_AURA       = 71416,
        SPELL_OVERLOAD           = 71423,
        SPELL_SUMMON_EGG         = 71395,
        SPELL_VISUAL_BUNNY       = 71422,
        SPELL_DEADLY_EGG_BARRAGE = 71657,

        EVENT_DEADLY_EGG_BARRAGE = 1,
        EVENT_REMOVE_AURAS       = 2,
    };

    struct npc_megachicken_qbeesAI : public npc_escortAI
    {
        npc_megachicken_qbeesAI(Creature* creature) : npc_escortAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            me->setActive(true);
            Start(true, false, 0, NULL, false, true);
            Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_DEADLY_EGG_BARRAGE, 5 * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            me->CastSpell(me, SPELL_SUMMON_EGG, false);
            me->CastSpell(me, SPELL_VISUAL_BUNNY, false);
        }

        void WaypointReached(uint32 point)
        {
            if (roll_chance_i(45))
            {
                SetEscortPaused(true);
                me->CastSpell((Unit*)NULL, SPELL_BARRAGE_AURA, false);
                me->CastSpell((Unit*)NULL, SPELL_OVERLOAD, false);
                me->HandleEmoteCommand(EMOTE_ONESHOT_CUSTOM_SPELL_02);
                events.ScheduleEvent(EVENT_REMOVE_AURAS, 6 * IN_MILLISECONDS);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_DEADLY_EGG_BARRAGE:
                        if (UpdateVictim() && me->IsInCombat())
                        {
                            me->CastSpell((Unit*)NULL, SPELL_DEADLY_EGG_BARRAGE, false);
                            events.ScheduleEvent(EVENT_DEADLY_EGG_BARRAGE, 5 * IN_MILLISECONDS);
                        }
                        break;
                    case EVENT_REMOVE_AURAS:
                        me->RemoveAura(SPELL_OVERLOAD);
                        me->RemoveAura(SPELL_BARRAGE_AURA);
                        SetEscortPaused(false);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class go_mechashark_xsteam_controller_qgisc : public GameObjectScript
{
public:
    go_mechashark_xsteam_controller_qgisc() : GameObjectScript("go_mechashark_xsteam_controller_qgisc") {}

    enum eSpells
    {
        SPELL_CONTROLLER                  = 71662,
        SPELL_AURA_CONTROLLER             = 71648,

        QUEST_A_GOBLIN_IN_SHARKS_CLOTHING = 24817
    };

    bool OnGossipHello(Player* player, GameObject* pGO)
    {
        uint8 questState = player->GetQuestStatus(QUEST_A_GOBLIN_IN_SHARKS_CLOTHING);
        if (questState == QUEST_STATUS_INCOMPLETE && !player->HasAura(SPELL_AURA_CONTROLLER))
           player->CastSpell(player, SPELL_CONTROLLER, false);

        return false;
    }
};

class npc_mechashark_xteam_qgisc : public CreatureScript
{
public:
    npc_mechashark_xteam_qgisc() : CreatureScript("npc_mechashark_xteam_qgisc") {}

    enum
    {
        EVENT_LEAVING_COUNTDOWN   = 1,

        SPELL_LEAVING_DIRE_STRAIT = 71749,
        SPELL_INVOLUNTARY_EXIT    = 71685,

        ACTION_START_COUNTDOWN = 1,
        ACTION_STOP_COUNTDOWN = 2
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_mechashark_xteam_qgiscAI(creature);
    }

    struct npc_mechashark_xteam_qgiscAI : public VehicleAI
    {
        npc_mechashark_xteam_qgiscAI(Creature* creature) : VehicleAI(creature) {}

        EventMap events;

        void DoAction(const int32 actionId)
        {
            switch(actionId)
            {
                case ACTION_START_COUNTDOWN:
                    events.ScheduleEvent(EVENT_LEAVING_COUNTDOWN, 15 * IN_MILLISECONDS);
                    me->CastSpell((Unit*)NULL, SPELL_LEAVING_DIRE_STRAIT, false);
                    break;
                case ACTION_STOP_COUNTDOWN:
                    events.CancelEvent(EVENT_LEAVING_COUNTDOWN);
                    me->RemoveAura(SPELL_LEAVING_DIRE_STRAIT);
                    if (Unit* owner = me->ToTempSummon()->GetOwner())
                        owner->RemoveAura(SPELL_LEAVING_DIRE_STRAIT);
                    break;
            }
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (!apply)
            {
                who->CastSpell((Unit*)NULL, SPELL_INVOLUNTARY_EXIT, false);
                if (who->HasAura(SPELL_LEAVING_DIRE_STRAIT))
                    who->RemoveAura(SPELL_LEAVING_DIRE_STRAIT);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_LEAVING_COUNTDOWN)
                me->DespawnOrUnsummon();

            DoMeleeAttackIfReady();
        }
    };
};

class spell_mechashark_water_controller_qgisc : public SpellScriptLoader
{
public:
    spell_mechashark_water_controller_qgisc() : SpellScriptLoader("spell_mechashark_water_controller_qgisc") { }

    enum
    {
        AREA_DIRE_STRAIT      = 4816,

        ACTION_START_COUNTDOWN = 1,
        ACTION_STOP_COUNTDOWN  = 2
    };

    class spell_mechashark_water_controller_qgisc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mechashark_water_controller_qgisc_AuraScript);

        bool _countdown;

        bool Load()
        {
            _countdown = false;
            return true;
        }

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();
            Unit* caster = GetCaster();

            uint16 area = caster->GetAreaId();
            if (area != AREA_DIRE_STRAIT)
            {
                if (!_countdown)
                    if (caster->ToCreature())
                        if (caster->IsAIEnabled)
                        {
                            _countdown = true;
                            caster->GetAI()->DoAction(ACTION_START_COUNTDOWN);
                        }
            }
            else
            {
                if (_countdown)
                    if (caster->ToCreature())
                        if (caster->IsAIEnabled)
                        {
                            _countdown = false;
                            caster->GetAI()->DoAction(ACTION_STOP_COUNTDOWN);
                        }
            }

            if (!caster->IsInWater())
                if (caster->ToCreature())
                    caster->ToCreature()->DespawnOrUnsummon();
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_mechashark_water_controller_qgisc_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_mechashark_water_controller_qgisc_AuraScript();
    }
};

class npc_assistant_greely_qgisc : public CreatureScript
{
public:
    npc_assistant_greely_qgisc() : CreatureScript("npc_assistant_greely_qgisc") {}

    enum eQuests
    {
        QUEST_A_GOBLIN_IN_SHARKS_CLOTHING = 24817,
        QUEST_ROCKN_POWER                 = 24946,
        QUEST_KAJACOLA_GIVES_YOU_IDEAS    = 25110,
        QUEST_SHREDDER_SHUTTDOWN          = 25200,
        QUEST_THE_SLAVE_PITS              = 25213,
        QUEST_WILD_MINE_CART_RIDE         = 25184,

        TALk_QA   = 3,
        TALK_QA_2 = 4,
        TALK_QA_3 = 5,

        SPELL_DETECT_ISVISIBLITY   = 79549,
        SPELL_SUMMON_ASSISTANT     = 73603,
        SPELL_SUMMON_UNIFORM       = 89164,
        SPELL_ACE_CONTROLLER       = 73633,
        SPELL_IZZY_CONTROLLER      = 73635,
        SPELL_GOBBER_CONTROLLER    = 73637,
        SPELL_ASSISTANT_CONTROLLER = 73616
    };

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 /*item*/)
    {
        if (quest->GetQuestId() == QUEST_KAJACOLA_GIVES_YOU_IDEAS)
        {
            player->RemoveAura(SPELL_DETECT_ISVISIBLITY);
            player->CastSpell(player, SPELL_SUMMON_ASSISTANT, false);
        }

        player->SaveToDB();
        return true;
    }

    bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_WILD_MINE_CART_RIDE)
        {
            player->RemoveAura(SPELL_ACE_CONTROLLER);
            player->RemoveAura(SPELL_IZZY_CONTROLLER);
            player->RemoveAura(SPELL_GOBBER_CONTROLLER);
            player->RemoveAura(SPELL_ASSISTANT_CONTROLLER);
        }

        player->SaveToDB();
        return true;
    }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_A_GOBLIN_IN_SHARKS_CLOTHING:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALk_QA, player->GetGUID(), true);
                break;
            case QUEST_ROCKN_POWER:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_2, player->GetGUID(), true);
                break;
            case QUEST_SHREDDER_SHUTTDOWN:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_3, player->GetGUID(), true);
                break;
            case QUEST_THE_SLAVE_PITS:
                player->CastSpell(player, SPELL_SUMMON_UNIFORM, false);
                break;
        }

        return true;
    }
};

class npc_megs_dreadshredder_lost_isles : public CreatureScript
{
public:
    npc_megs_dreadshredder_lost_isles() : CreatureScript("npc_megs_dreadshredder_lost_isles") {}

    enum
    {
        QUEST_BILGEWATER_CARTEL_REPRESENT = 24858,
        QUEST_IRRESISTIBLE_POOL_PONY      = 24864,
        QUEST_SURRENDER_OR_ELSE           = 24868,
        QUEST_GET_BACK_TO_TOWN            = 24897,

        TALk_QA   = 0,
        TALK_QA_2 = 1,
        TALK_QA_3 = 2,

        SPELL_QSOE_ACCEPT   = 72001,
        SPELL_PHASE_06      = 72157,
        SPELL_PHASE_05      = 68750
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_BILGEWATER_CARTEL_REPRESENT:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALk_QA, player->GetGUID(), true);
                break;
            case QUEST_IRRESISTIBLE_POOL_PONY:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_2, player->GetGUID(), true);
                break;
            case QUEST_SURRENDER_OR_ELSE:
                player->CastSpell(player, SPELL_QSOE_ACCEPT, false);
                break;
            case QUEST_GET_BACK_TO_TOWN:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA_3, player->GetGUID(), true);

                player->RemoveAura(SPELL_PHASE_05);
                player->CastSpell(player, SPELL_PHASE_06, false);
                break;
        }

        player->SaveToDB();
        return true;
    }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_megs_dreadshredder_lost_islesAI(creature);
    }

    struct npc_megs_dreadshredder_lost_islesAI : public ScriptedAI
    {
        npc_megs_dreadshredder_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }
    };
};

class npc_brett_mcquid_lost_isles : public CreatureScript
{
public:
    npc_brett_mcquid_lost_isles() : CreatureScript("npc_brett_mcquid_lost_isles") {}

    enum eQuests
    {
        QUEST_NAGA_HIDE = 24859,

        TALk_QA = 0,
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_NAGA_HIDE)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALk_QA, player->GetGUID(), true);

        return true;
    }
};

class go_naga_banner_qbcr : public GameObjectScript
{
public:
    go_naga_banner_qbcr() : GameObjectScript("go_naga_banner_qbcr") {}

    enum eSpells
    {
        SPELL_BILGEWATER_CARTEL_BANNER = 71855
    };

    bool OnGossipHello(Player* player, GameObject* pGO)
    {
        player->CastSpell(player, SPELL_BILGEWATER_CARTEL_BANNER, false);

        return false;
    }
};

const int32 NagaSummonSpellsQIPP[1][4] =
{
    { 71919, 71918, 83115, 83116 }
};

class npc_naga_hatchling_qiip : public CreatureScript
{
public:
    npc_naga_hatchling_qiip() : CreatureScript("npc_naga_hatchling_qiip") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_naga_hatchling_qiipAI(creature);
    }

    enum
    {
        NPC_QIIP_QUEST_CREDIT = 38413,
        QUEST_IRRESISTIBLE_POOL_PONY = 24864
    };

    struct npc_naga_hatchling_qiipAI : public ScriptedAI
    {
        npc_naga_hatchling_qiipAI(Creature * c) : ScriptedAI(c) {}

        void OnSpellClick(Unit* clicker, bool &/*result*/)
        {
            if (clicker->GetTypeId() == TYPEID_PLAYER)
            {
                uint8 questState = clicker->ToPlayer()->GetQuestStatus(QUEST_IRRESISTIBLE_POOL_PONY);
                if (questState == QUEST_STATUS_INCOMPLETE)
                {
                    for(int8 i = 0; i < irand(1, 4); ++i)
                        clicker->CastSpell(clicker, NagaSummonSpellsQIPP[0][i], false);

                    me->DespawnOrUnsummon();
                }
            }
        }
    };
};

class npc_naga_hatching_summon_qiip : public CreatureScript
{
public:
    npc_naga_hatching_summon_qiip() : CreatureScript("npc_naga_hatching_summon_qiip") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_naga_hatching_summon_qiipAI(creature);
    }

    enum eSpells
    {
        SPELL_PROXIMITY_CONTROL = 71917,
        NPC_QIIP_QUEST_CREDIT = 38413,
        QUEST_IRRESISTIBLE_POOL_PONY = 24864,
    };

    struct npc_naga_hatching_summon_qiipAI : public ScriptedAI
    {
        npc_naga_hatching_summon_qiipAI(Creature * c) : ScriptedAI(c) {}

        void IsSummonedBy(Unit* summoner)
        {
            if (summoner->GetTypeId() == TYPEID_PLAYER)
            {
                uint8 questState = summoner->ToPlayer()->GetQuestStatus(QUEST_IRRESISTIBLE_POOL_PONY);
                if (questState == QUEST_STATUS_INCOMPLETE)
                    summoner->ToPlayer()->KilledMonsterCredit(NPC_QIIP_QUEST_CREDIT);
            }

            me->CastSpell(me, SPELL_PROXIMITY_CONTROL, false);
            me->GetMotionMaster()->MoveFollow(summoner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        }
    };
};

class NagasTargetSelectorQIPP
{
public:
    NagasTargetSelectorQIPP(Unit* caster) : _caster(caster) { }

    enum
    {
        NPC_NAGA_1 = 44588,
        NPC_NAGA_2 = 44589,
        NPC_NAGA_3 = 44590,
        NPC_NAGA_4 = 44591
    };

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (cre->IsSummon() && (cre->GetEntry() == NPC_NAGA_1 || cre->GetEntry() == NPC_NAGA_2 || cre->GetEntry() == NPC_NAGA_3 || cre->GetEntry() == NPC_NAGA_4) && cre->ToTempSummon()->GetOwner() == _caster)
                return false;

        return true;
    }

private:
    Unit* _caster;
};

class spell_despawn_nagas_qipp : public SpellScriptLoader
{
public:
    spell_despawn_nagas_qipp() : SpellScriptLoader("spell_despawn_nagas_qipp") { }

    class spell_despawn_nagas_qipp_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_despawn_nagas_qipp_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Creature* target = GetHitCreature())
                target->DespawnOrUnsummon();
        }

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (Unit* caster = GetCaster())
                targets.remove_if(NagasTargetSelectorQIPP(caster));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_despawn_nagas_qipp_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_despawn_nagas_qipp_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_despawn_nagas_qipp_SpellScript();
    }
};

const float AceWPPathQSOE[8][3] =
{
    {663.256f, 2005.231f, 38.297f},
    {601.276f, 1985.088f, 12.981f},
    {508.762f, 2005.814f, 2.389f },
    {471.652f, 1986.713f, 0.569f },
    {446.532f, 1953.421f, -0.553f},
    {408.993f, 1951.485f, 0.292f },
    {261.781f, 1946.454f, -0.459f},
    {160.057f, 1942.748f, 5.336f }
};

class npc_ace_qsoe : public CreatureScript
{
public:
    npc_ace_qsoe() : CreatureScript("npc_ace_qsoe") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_ace_qsoeAI(creature);
    }

    enum eSpells
    {
        SPELL_SUMMON_NAGAS  = 72073,
        SPELL_GOBLIN_BANNER = 72077,

        TALK_1 = 0,
        TALK_2 = 1,
        TALK_3 = 2,
        TALK_4 = 3,

        EVENT_MOVE            = 1,
        EVENT_MOVE_DONE       = 2,
        EVENT_SUMMON_FACELESS = 3,
        EVENT_DESPAWN         = 4,

        NPC_FACELESS_OF_THE_DEEP = 38448,
        NPC_VOID_ZONE            = 38450
    };

    struct npc_ace_qsoeAI : public ScriptedAI
    {
        npc_ace_qsoeAI(Creature * c) : ScriptedAI(c), summons(me) {}

        EventMap events;
        SummonList summons;

        void InitializeAI()
        {
            ASSERT(me->IsSummon());
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetCustomVisibility(CUSTOM_VISIBILITY_SEER,summoner->GetGUID());
            me->SetVisible(false);
            me->CastSpell((Unit*)NULL, SPELL_SUMMON_NAGAS, false);
            me->CastSpell(me, SPELL_GOBLIN_BANNER, false);
            Talk(TALK_1, summoner->GetGUID());
            events.ScheduleEvent(EVENT_MOVE, 6 * IN_MILLISECONDS);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->SetCustomVisibility(CUSTOM_VISIBILITY_SEER,me->ToTempSummon()->GetSummonerGUID());
            summon->SetVisible(false);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_MOVE:
                    {
                        Talk(TALK_2, me->ToTempSummon()->GetSummonerGUID(), true);
                        Movement::MoveSplineInit init(me);
                        for(uint8 i = 0; i < 8; ++i)
                        {
                            G3D::Vector3 path(AceWPPathQSOE[i][0], AceWPPathQSOE[i][1], AceWPPathQSOE[i][2]);
                            init.Path().push_back(path);
                        }
                        init.SetSmooth();
                        init.SetUncompressed();
                        init.SetVelocity(5.0f);
                        init.Launch();
                    }
                    events.ScheduleEvent(EVENT_MOVE_DONE, me->GetSplineDuration());
                    break;
                    case EVENT_DESPAWN:
                    {
                        Talk(TALK_4, me->ToTempSummon()->GetSummonerGUID());
                        Movement::MoveSplineInit init(me);
                        init.MoveTo(251.278f, 1946.903f, -0.596f);
                        init.SetVelocity(6.0f);
                        init.Launch();
                        me->DespawnOrUnsummon(me->GetSplineDuration());
                    }
                    break;
                    case EVENT_MOVE_DONE:
                        Talk(TALK_3, me->ToTempSummon()->GetSummonerGUID());
                        events.ScheduleEvent(EVENT_SUMMON_FACELESS, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_SUMMON_FACELESS:
                        if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                            summoner->SummonCreature(NPC_FACELESS_OF_THE_DEEP, 132.3455f, 1938.528f, -2.433363f);

                        events.ScheduleEvent(EVENT_DESPAWN, 9 * IN_MILLISECONDS);
                        break;
                }
            }
        }
    };
};

class npc_faceless_of_darkness_qsoe : public CreatureScript
{
public:
    npc_faceless_of_darkness_qsoe() : CreatureScript("npc_faceless_of_darkness_qsoe") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_faceless_of_darkness_qsoeAI(creature);
    }

    enum
    {
        SPELL_ABSORPTION_SHIELD = 72055,
        SPELL_BEAM_EFFECT       = 72076,
        SPELL_FREEZE_ANIM       = 72126,
        SPELL_KILL_CREDIT       = 72035,
        SPELL_SHADOW_CRASH      = 75903,

        TALK_1     = 0,
        TALK_2     = 1,
        TALK_3     = 2,
        TALK_AGGRO = 3,

        EVENT_BEAM         = 1,
        EVENT_JUMP         = 2,
        EVENT_TALK_2       = 3,
        EVENT_TALK_3       = 4,
        EVENT_SHADOW_CRASH = 5,

        NPC_VOID_ZONE = 38450,

        QUEST_SURRENDER_OR_ELSE = 24868
    };

    struct npc_faceless_of_darkness_qsoeAI : public ScriptedAI
    {
        npc_faceless_of_darkness_qsoeAI(Creature * c) : ScriptedAI(c) {}

        EventMap combatEvents;
        EventMap comsmeticEvents;

        void InitializeAI()
        {
            ASSERT(me->IsSummon());
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetCustomVisibility(CUSTOM_VISIBILITY_SEER,summoner->GetGUID());
            me->SetVisible(false);
            me->SetReactState(REACT_PASSIVE);
            me->SummonCreature(NPC_VOID_ZONE, 131.5642f, 1938.316f, 8.622858f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 14 * IN_MILLISECONDS);
            me->CastSpell(me, SPELL_ABSORPTION_SHIELD, false);
            me->CastSpell(me, SPELL_FREEZE_ANIM, false);
            Talk(TALK_1, summoner->GetGUID());

            Movement::MoveSplineInit init(me);
            init.MoveTo(132.3455f, 1938.528f, 17.56664f);
            init.SetVelocity(2.0f);
            init.Launch();

            comsmeticEvents.ScheduleEvent(EVENT_BEAM, 1 * IN_MILLISECONDS);
            comsmeticEvents.ScheduleEvent(EVENT_JUMP, 14 * IN_MILLISECONDS);
            comsmeticEvents.ScheduleEvent(EVENT_TALK_2, 6 * IN_MILLISECONDS);
        }

        void Reset()
        {
            combatEvents.Reset();
        }

        void EnterCombat(Unit* who)
        {
            Talk(TALK_AGGRO, me->ToTempSummon()->GetSummonerGUID(), true);
            combatEvents.ScheduleEvent(EVENT_SHADOW_CRASH, 6 * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, me->ToTempSummon()->GetSummonerGUID()))
                if (player->GetQuestStatus(QUEST_SURRENDER_OR_ELSE) == QUEST_STATUS_INCOMPLETE)
                    player->CastSpell(killer, SPELL_KILL_CREDIT, false);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                comsmeticEvents.Update(diff);

                if (uint32 eventId = comsmeticEvents.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_BEAM:
                        me->CastSpell(me, SPELL_BEAM_EFFECT, false);
                        comsmeticEvents.ScheduleEvent(EVENT_BEAM, 0.2 * IN_MILLISECONDS);
                        break;
                        case EVENT_JUMP:
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->GetMotionMaster()->MoveJump(169.4754f, 1940.462f, 4.841922f, 15.0f, 20.0f);
                        me->RemoveAura(SPELL_FREEZE_ANIM);
                        comsmeticEvents.CancelEvent(EVENT_BEAM);
                        break;
                        case EVENT_TALK_2:
                        Talk(TALK_2, me->ToTempSummon()->GetSummonerGUID(), true);
                        comsmeticEvents.ScheduleEvent(EVENT_TALK_3, 7 * IN_MILLISECONDS);
                        break;
                        case EVENT_TALK_3:
                        Talk(TALK_3, me->ToTempSummon()->GetSummonerGUID(), true);
                        break;
                    }
                }

                return;
            }

            combatEvents.Update(diff);

            if (combatEvents.ExecuteEvent() == EVENT_SHADOW_CRASH)
            {
                me->CastSpell((Unit*)NULL, SPELL_SHADOW_CRASH, false);
                combatEvents.ScheduleEvent(EVENT_SHADOW_CRASH, urand(5, 8) * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_vashjelan_siren_lost_isles : public CreatureScript
{
public:
    npc_vashjelan_siren_lost_isles() : CreatureScript("npc_vashjelan_siren_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_vashjelan_siren_lost_islesAI(creature);
    }

    enum
    {
        SPELL_WATER_COSMETIC = 71802,

        EVENT_WATER_COSMETIC = 1
    };

    struct npc_vashjelan_siren_lost_islesAI : public ScriptedAI
    {
        npc_vashjelan_siren_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_WATER_COSMETIC, 1 * IN_MILLISECONDS);
            Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_WATER_COSMETIC)
            {
                me->CastSpell((Unit*)NULL, SPELL_WATER_COSMETIC, false);
                events.ScheduleEvent(EVENT_WATER_COSMETIC, urand(4, 6) * IN_MILLISECONDS);
            }
        }
    };
};

class npc_izzy_lost_isles : public CreatureScript
{
public:
    npc_izzy_lost_isles() : CreatureScript("npc_izzy_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_izzy_lost_islesAI(creature);
    }

    enum
    {
        EVENT_SUMMON_PYGMY_WARRIOR = 1,

        NPC_OOMLOT_WARRIOR            = 38531,
        NPC_MORALE_BOOST_QUEST_CREDIT = 38647,

        QUEST_OOMLOT_VILLAGE    = 24924,
        QUEST_FREE_THE_CAPTIVES = 24925,
        QUEST_OOMLOT_DEALT_WITH = 24937,
        QUEST_MORALE_BOOST      = 25122,

        SPELL_PYGMY_HELMET        = 66987,
        SPELL_PHASE_06            = 72157,
        SPELL_PHASE_07            = 72676,
        SPELL_KAJA_COLA_ZERO_ONE  = 73583,
        SPELL_IZZY_FREED          = 73613,
        SPELL_DETECT_IZZY         = 73595,

        TALK_QA  = 0,
        TALK_QA2 = 1
    };

    bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
    {
        player->RemoveAura(SPELL_PYGMY_HELMET);
        player->SaveToDB();
        return true;
    }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_FREE_THE_CAPTIVES:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA, player->GetGUID(), true);
                break;
            case QUEST_OOMLOT_DEALT_WITH:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA2, player->GetGUID(), true);

                player->RemoveAura(SPELL_PHASE_06);
                player->CastSpell(player, SPELL_PHASE_07, false);
                break;
        }

        player->SaveToDB();
        return true;
    }

    struct npc_izzy_lost_islesAI : public ScriptedAI
    {
        npc_izzy_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void InitializeAI()
        {
            me->setActive(true);
            if (me->GetPhaseMask() == 4096)
                events.ScheduleEvent(EVENT_SUMMON_PYGMY_WARRIOR, 1 * IN_MILLISECONDS);

            Reset();
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            switch (spell->Id)
            {
                case SPELL_KAJA_COLA_ZERO_ONE:
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (uint8 questState = caster->ToPlayer()->GetQuestStatus(QUEST_MORALE_BOOST))
                           if (questState == QUEST_STATUS_INCOMPLETE)
                           {
                               caster->ToPlayer()->KilledMonsterCredit(NPC_MORALE_BOOST_QUEST_CREDIT);
                               caster->CastSpell(caster, SPELL_IZZY_FREED, false);
                               caster->RemoveAura(SPELL_DETECT_IZZY);
                           }
                    }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SUMMON_PYGMY_WARRIOR)
            {
                me->SummonCreature(NPC_OOMLOT_WARRIOR, 778.994f, 1940.536f, 103.366f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 90 * IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_SUMMON_PYGMY_WARRIOR, 1 * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_oomlot_warrior_lost_isles : public CreatureScript
{
public:
    npc_oomlot_warrior_lost_isles() : CreatureScript("npc_oomlot_warrior_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_oomlot_warrior_lost_islesAI(creature);
    }

    enum eEvents
    {
        EVENT_MOVE_DONE = 1,
        QUEST_TOWN_IN_A_BOX_UNDER_ATTACK = 24901,
        NPC_UNDER_ATTACK_KILL_CREDIT = 38536,
    };

    struct npc_oomlot_warrior_lost_islesAI : public npc_escortAI
    {
        npc_oomlot_warrior_lost_islesAI(Creature * c) : npc_escortAI(c) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            me->setActive(true);
            Start(true, true);
            SetDespawnAtEnd(false);
            me->SetReactState(REACT_DEFENSIVE);
            Reset();
        }

        void JustDied(Unit* killer)
        {
            if (killer->GetTypeId() == TYPEID_PLAYER)
            {
                if (uint8 questState = killer->ToPlayer()->GetQuestStatus(QUEST_TOWN_IN_A_BOX_UNDER_ATTACK))
                    if (questState == QUEST_STATUS_INCOMPLETE)
                        killer->ToPlayer()->KilledMonsterCredit(NPC_UNDER_ATTACK_KILL_CREDIT);
            }
            else if (killer->IsVehicle())
                if (Vehicle* vehicle = killer->GetVehicleKit())
                    if (Unit* passenger = vehicle->GetPassenger(0))
                        if (Player* player = passenger->ToPlayer())
                            if (player->GetQuestStatus(QUEST_TOWN_IN_A_BOX_UNDER_ATTACK) == QUEST_STATUS_INCOMPLETE)
                                player->KilledMonsterCredit(NPC_UNDER_ATTACK_KILL_CREDIT);
        }

        void WaypointReached(uint32 point)
        {
            switch(point)
            {
                case 7:
                    me->SetReactState(REACT_PASSIVE);
                    break;
                case 8:
                    events.ScheduleEvent(EVENT_MOVE_DONE, 1 * IN_MILLISECONDS);
                    break;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_MOVE_DONE)
            {
                me->SetHomePosition(*me);
                if (Unit* target = me->SelectNearbyTarget((Unit*)NULL, 50.0f))
                {
                    me->getThreatManager().addThreat(target, std::numeric_limits<float>::max());
                    AttackStart(target);
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_sassy_hardwrench_lost_isles : public CreatureScript
{
public:
    npc_sassy_hardwrench_lost_isles() : CreatureScript("npc_sassy_hardwrench_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_sassy_hardwrench_lost_islesAI(creature);
    }

    enum
    {
        QUEST_TOWN_IN_A_BOX_UNDER_ATTACK   = 24901,
        QUEST_MINE_DISPOSAL_THE_GOBLIN_WAY = 25058,
        QUEST_THE_PRIDE_OF_KEZAN           = 25066,
        QUEST_WHATS_KIND_OF_NAME_CANDY     = 25244,
        QUEST_FINAL_CONFRONTATION          = 25251,
        QUEST_VICTORY                      = 25265,
        QUEST_WARCHIEFS_EMISSARY           = 25266,
        QUEST_WARCHIEF_WANTS_YOU           = 25098,

        SPELL_SUMMON_BOMBER = 73431,
        SPELL_ROCKET_BOOTS  = 74028,
        SPELL_PHASE_10      = 74025,
        SPELL_PHASE_09      = 73756,
        SPELL_TELEPORT      = 74029,

        TALK_QA   = 0,
        TALK_QA_2 = 1,
        TALK_QA_3 = 2,
        TALK_QA_4 = 3,
        TALK_QA_5 = 4,

        MENU_UP_TO_THE_SKIES = 11146
    };

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        if (action == GOSSIP_ACTION_INFO_DEF + 1)
            player->CastSpell(player, SPELL_SUMMON_BOMBER, false);

        if (action == GOSSIP_ACTION_INFO_DEF + 2)
        {
            player->CastSpell(player, SPELL_TELEPORT, false);
            player->SaveToDB();
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if ((player->GetQuestStatus(QUEST_THE_PRIDE_OF_KEZAN) == QUEST_STATUS_INCOMPLETE))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Get me up into the skies, Sassy!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if ((player->GetQuestStatus(QUEST_WARCHIEFS_EMISSARY) == QUEST_STATUS_COMPLETE))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Sassy, let's set sail for Orgrimmar before the island blows for good!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        player->SEND_GOSSIP_MENU(MENU_UP_TO_THE_SKIES, creature->GetGUID());
        return true;
    }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_TOWN_IN_A_BOX_UNDER_ATTACK:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA, player->GetGUID(), true);
                break;
            case QUEST_MINE_DISPOSAL_THE_GOBLIN_WAY:
                if (creature->IsAIEnabled)
                   creature->AI()->Talk(TALK_QA_2, player->GetGUID(), true);
                break;
            case QUEST_WHATS_KIND_OF_NAME_CANDY:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_3, player->GetGUID(), true);
                break;
            case QUEST_FINAL_CONFRONTATION:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_4, player->GetGUID(), true);
                break;
            case QUEST_VICTORY:
                player->CastSpell(player, SPELL_ROCKET_BOOTS, false);
                player->CastSpell(player, SPELL_PHASE_10, false);
                player->RemoveAura(SPELL_PHASE_09);
                break;
            case QUEST_WARCHIEF_WANTS_YOU:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_QA_5, player->GetGUID(), true);
                break;
        }

        player->SaveToDB();
        return true;
    }

    struct npc_sassy_hardwrench_lost_islesAI : public ScriptedAI
    {
        npc_sassy_hardwrench_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }
    };
};

class npc_town_in_one_box_generic_ai_lost_isles : public CreatureScript
{
public:
    npc_town_in_one_box_generic_ai_lost_isles() : CreatureScript("npc_town_in_one_box_generic_ai_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_town_in_one_box_generic_ai_lost_islesAI(creature);
    }

    struct npc_town_in_one_box_generic_ai_lost_islesAI : public ScriptedAI
    {
        npc_town_in_one_box_generic_ai_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }
    };
};

class npc_warrior_matic_nx_lost_isles : public CreatureScript
{
public:
    npc_warrior_matic_nx_lost_isles() : CreatureScript("npc_warrior_matic_nx_lost_isles") {}

    enum eSpells
    {
        SPELL_HEROIC_STRIKE = 25710,
        SPELL_SUNDER_ARMOR  = 7386,
        SPELL_THUNDER_CLAP  = 8078,

        EVENT_HEROIC_STRIKE = 1,
        EVENT_SUNDER_ARMOR  = 2,
        EVENT_THUNDER_CLAP  = 3
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_warrior_matic_nx_lost_islesAI(creature);
    }

    struct npc_warrior_matic_nx_lost_islesAI : public ScriptedAI
    {
        npc_warrior_matic_nx_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_THUNDER_CLAP, urand(8, 11) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_HEROIC_STRIKE, urand(5, 8) * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SUNDER_ARMOR, urand(6, 10) * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_THUNDER_CLAP:
                        me->CastSpell((Unit*)NULL, SPELL_THUNDER_CLAP, false);
                        events.ScheduleEvent(EVENT_THUNDER_CLAP, urand(8, 12) * IN_MILLISECONDS);
                        break;
                    case EVENT_HEROIC_STRIKE:
                         me->CastSpell((Unit*)NULL, SPELL_HEROIC_STRIKE, false);
                         events.ScheduleEvent(EVENT_HEROIC_STRIKE, urand(5, 8) * IN_MILLISECONDS);
                         break;
                    case EVENT_SUNDER_ARMOR:
                         me->CastSpell((Unit*)NULL, SPELL_SUNDER_ARMOR, false);
                         events.ScheduleEvent(EVENT_SUNDER_ARMOR, urand(6, 10) * IN_MILLISECONDS);
                         break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_oomlot_shaman_lost_isles : public CreatureScript
{
public:
    npc_oomlot_shaman_lost_isles() : CreatureScript("npc_oomlot_shaman_lost_isles") {}

    enum eSpells
    {
        SPELL_ENVELOPING_WIND = 72518,
        SPELL_DUMMY_ON_DEATH  = 72580
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_oomlot_shaman_lost_islesAI(creature);
    }

    struct npc_oomlot_shaman_lost_islesAI : public ScriptedAI
    {
        npc_oomlot_shaman_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        void InitializeAI()
        {
            me->CastSpell((Unit*)NULL, SPELL_ENVELOPING_WIND, false);
            Reset();
        }

        void JustDied(Unit* killer)
        {
            me->CastSpell((Unit*)NULL, SPELL_DUMMY_ON_DEATH, false);
        }
    };
};

class npc_goblin_captive_lost_isles : public CreatureScript
{
public:
    npc_goblin_captive_lost_isles() : CreatureScript("npc_goblin_captive_lost_isles") {}

    enum eSpells
    {
        SPELL_SUMMON_SHAMAN    = 72243,
        SPELL_ENVELOPING_WINDS = 72522,
        SPELL_DUMMY_ON_DEATH   = 72580,

        TALK_THANKS = 0
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_goblin_captive_lost_islesAI(creature);
    }

    struct npc_goblin_captive_lost_islesAI : public ScriptedAI
    {
        npc_goblin_captive_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        void InitializeAI()
        {
            me->CastSpell(me, SPELL_ENVELOPING_WINDS, false);
            me->CastSpell((Unit*)NULL, SPELL_SUMMON_SHAMAN, false);
            Reset();
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_DUMMY_ON_DEATH)
            {
                Talk(TALK_THANKS);
                if (me->HasAura(SPELL_ENVELOPING_WINDS))
                    me->RemoveAura(SPELL_ENVELOPING_WINDS);

                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveRandom(15.0f);
                me->DespawnOrUnsummon(1.5 * IN_MILLISECONDS);
            }
        }
    };
};

class spell_super_booster_rocket_boots_qzvsbrb : public SpellScriptLoader
{
public:
    spell_super_booster_rocket_boots_qzvsbrb() : SpellScriptLoader("spell_super_booster_rocket_boots_qzvsbrb") { }

    enum eSpells
    {
        SPELL_SUMMON_SPELL = 72889,
        SPELL_BOOTS_AURA   = 72887
    };

    class spell_super_booster_rocket_boots_qzvsbrb_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_super_booster_rocket_boots_qzvsbrb_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Unit* caster = GetCaster())
                if (!caster->HasAura(SPELL_BOOTS_AURA))
                    caster->CastSpell(caster, SPELL_SUMMON_SPELL, false);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_super_booster_rocket_boots_qzvsbrb_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_super_booster_rocket_boots_qzvsbrb_SpellScript();
    }
};

class GoblinZombieTargetSelector
{
public:
    GoblinZombieTargetSelector() { }

    enum eNPC
    {
        NPC_GOBLIN_ZOMBIE = 38753
    };

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (cre->GetEntry() == NPC_GOBLIN_ZOMBIE)
               return false;

        return true;
    }
};

class spell_super_booster_rocket_boots_filter_qzvsbrb : public SpellScriptLoader
{
public:
    spell_super_booster_rocket_boots_filter_qzvsbrb() : SpellScriptLoader("spell_super_booster_rocket_boots_filter_qzvsbrb") { }

    class spell_super_booster_rocket_boots_filter_qzvsbrb_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_super_booster_rocket_boots_filter_qzvsbrb_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (Unit* caster = GetCaster())
                targets.remove_if(GoblinZombieTargetSelector());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_super_booster_rocket_boots_filter_qzvsbrb_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_super_booster_rocket_boots_filter_qzvsbrb_SpellScript();
    }
};

class npc_goblin_zombie_qzvsbb : public CreatureScript
{
public:
    npc_goblin_zombie_qzvsbb() : CreatureScript("npc_goblin_zombie_qzvsbb") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_goblin_zombie_qzvsbbAI(creature);
    }

    enum eQuests
    {
        QUEST_ZOMBIES_VS_SUPER_BOOSTER_ROCKET_BOOTS = 24942,
        NPC_QZVSBRB_KILL_CREDIT = 38807
    };

    struct npc_goblin_zombie_qzvsbbAI : public ScriptedAI
    {
        npc_goblin_zombie_qzvsbbAI(Creature * c) : ScriptedAI(c) {}

        void JustDied(Unit* killer)
        {
            if (killer->GetTypeId() == TYPEID_PLAYER)
            {
                if (uint8 questState = killer->ToPlayer()->GetQuestStatus(QUEST_ZOMBIES_VS_SUPER_BOOSTER_ROCKET_BOOTS))
                    if (questState == QUEST_STATUS_INCOMPLETE)
                        killer->ToPlayer()->KilledMonsterCredit(NPC_QZVSBRB_KILL_CREDIT);
            }
            else if (killer->IsVehicle())
            {
                if (Vehicle* vehicle = killer->GetVehicleKit())
                    if (Unit* passenger = vehicle->GetPassenger(0))
                        if (Player* player = passenger->ToPlayer())
                            if (player->GetQuestStatus(QUEST_ZOMBIES_VS_SUPER_BOOSTER_ROCKET_BOOTS) == QUEST_STATUS_INCOMPLETE)
                                player->KilledMonsterCredit(NPC_QZVSBRB_KILL_CREDIT);
            }
        }
    };
};

class npc_gaahl_qzvsbb : public CreatureScript
{
public:
    npc_gaahl_qzvsbb() : CreatureScript("npc_gaahl_qzvsbb") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_gaahl_qzvsbbAI(creature);
    }

    enum
    {
        SPELL_FROST_SHOCK           = 12548,
        SPELL_ZOMBIE_TRANSFORMATION = 72935,

        EVENT_FROST_SHOCK           = 1,
        EVENT_ZOMBIE_TRANSFORMATION = 2,

        NPC_GOBLIN_CAPTIVE = 38812
    };

    struct npc_gaahl_qzvsbbAI : public ScriptedAI
    {
        npc_gaahl_qzvsbbAI(Creature * c) : ScriptedAI(c) {}

        EventMap comsmeticEvents;
        EventMap combatEvents;

        void InitializeAI()
        {
            me->setActive(true);
            comsmeticEvents.ScheduleEvent(EVENT_ZOMBIE_TRANSFORMATION, 2 * IN_MILLISECONDS);
            Reset();
        }

        void Reset()
        {
            combatEvents.Reset();
        }

        void EnterCombat(Unit* who)
        {
            combatEvents.ScheduleEvent(EVENT_FROST_SHOCK, 3 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                comsmeticEvents.Update(diff);

                if (comsmeticEvents.ExecuteEvent() == EVENT_ZOMBIE_TRANSFORMATION)
                {
                    if (Creature* captive = me->FindNearestCreature(NPC_GOBLIN_CAPTIVE, 20.0f))
                        me->CastSpell(captive, SPELL_ZOMBIE_TRANSFORMATION, false);
                }

                return;
            }

            combatEvents.Update(diff);

            if (combatEvents.ExecuteEvent() == EVENT_FROST_SHOCK)
            {
                me->CastSpell((Unit*)NULL, SPELL_FROST_SHOCK, false);
                combatEvents.ScheduleEvent(EVENT_FROST_SHOCK, 8 * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_malmo_qzvsbb : public CreatureScript
{
public:
    npc_malmo_qzvsbb() : CreatureScript("npc_malmo_qzvsbb") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_malmo_qzvsbbAI(creature);
    }

    enum eSpells
    {
        SPELL_LIGHTNING_BOLT        = 57780,
        SPELL_ZOMBIE_TRANSFORMATION = 72935,

        EVENT_LIGHTNING_BOLT        = 1,
        EVENT_ZOMBIE_TRANSFORMATION = 2,

        NPC_GOBLIN_CAPTIVE = 50311
    };

    struct npc_malmo_qzvsbbAI : public ScriptedAI
    {
        npc_malmo_qzvsbbAI(Creature * c) : ScriptedAI(c) {}

        EventMap comsmeticEvents;
        EventMap combatEvents;

        void InitializeAI()
        {
            me->setActive(true);
            comsmeticEvents.ScheduleEvent(EVENT_ZOMBIE_TRANSFORMATION, 2 * IN_MILLISECONDS);
            Reset();
        }

        void Reset()
        {
            combatEvents.Reset();
        }

        void EnterCombat(Unit* who)
        {
            combatEvents.ScheduleEvent(EVENT_LIGHTNING_BOLT, 3 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                comsmeticEvents.Update(diff);
                if (comsmeticEvents.ExecuteEvent() == EVENT_ZOMBIE_TRANSFORMATION)
                    if (Creature* captive = me->FindNearestCreature(NPC_GOBLIN_CAPTIVE, 20.0f))
                        me->CastSpell(captive, SPELL_ZOMBIE_TRANSFORMATION, false);

                return;
            }

            combatEvents.Update(diff);

            if (combatEvents.ExecuteEvent() == EVENT_LIGHTNING_BOLT)
            {
                me->CastSpell((Unit*)NULL, SPELL_LIGHTNING_BOLT, false);
                combatEvents.ScheduleEvent(EVENT_LIGHTNING_BOLT, 6 * IN_MILLISECONDS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_teloch_qzvsbb : public CreatureScript
{
public:
    npc_teloch_qzvsbb() : CreatureScript("npc_teloch_qzvsbb") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_teloch_qzvsbbAI(creature);
    }

    enum
    {
        SPELL_SEARING_TOTEM         = 39591,
        SPELL_ZOMBIE_TRANSFORMATION = 72935,

        EVENT_SEARING_TOTEM         = 1,
        EVENT_ZOMBIE_TRANSFORMATION = 2,

        NPC_GOBLIN_CAPTIVE = 50310,
    };

    struct npc_teloch_qzvsbbAI : public ScriptedAI
    {
        npc_teloch_qzvsbbAI(Creature * c) : ScriptedAI(c) {}

        EventMap comsmeticEvents;
        EventMap combatEvents;

        void InitializeAI()
        {
            me->setActive(true);
            comsmeticEvents.ScheduleEvent(EVENT_ZOMBIE_TRANSFORMATION, 2 * IN_MILLISECONDS);
            Reset();
        }

        void Reset()
        {
            combatEvents.Reset();
        }

        void EnterCombat(Unit* who)
        {
            combatEvents.ScheduleEvent(EVENT_SEARING_TOTEM, 3 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                comsmeticEvents.Update(diff);
                if (comsmeticEvents.ExecuteEvent() == EVENT_ZOMBIE_TRANSFORMATION)
                    if (Creature* captive = me->FindNearestCreature(NPC_GOBLIN_CAPTIVE, 20.0f))
                        me->CastSpell(captive, SPELL_ZOMBIE_TRANSFORMATION, false);

                return;
            }

            combatEvents.Update(diff);

            if (combatEvents.ExecuteEvent() == EVENT_SEARING_TOTEM)
                me->CastSpell((Unit*)NULL, SPELL_SEARING_TOTEM, false);

            DoMeleeAttackIfReady();
        }
    };
};

class npc_volcanoth_champion_lost_isles : public CreatureScript
{
public:
    npc_volcanoth_champion_lost_isles() : CreatureScript("npc_volcanoth_champion_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_volcanoth_champion_lost_islesAI(creature);
    }

    struct npc_volcanoth_champion_lost_islesAI : public ScriptedAI
    {
        npc_volcanoth_champion_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        void InitializeAI()
        {
            if (Unit* target = me->SelectNearbyTarget((Unit*)NULL, 10.0f))
                AttackStart(target);
        }
    };
};

class npc_volcanoth_lost_isles : public CreatureScript
{
public:
    npc_volcanoth_lost_isles() : CreatureScript("npc_volcanoth_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_volcanoth_lost_islesAI(creature);
    }

    enum
    {
        EVENT_COSMETIC_BREATH = 1,
        EVENT_VOLCANIC_BREATH = 2,

        SPELL_COSMETIC_BREATH = 73016,
        SPELL_VOLCANIC_BREATH = 73097,
        SPELL_KILL_CREDIT     = 73060
    };

    struct npc_volcanoth_lost_islesAI : public ScriptedAI
    {
        npc_volcanoth_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        EventMap combatEvents;
        EventMap comsmeticEvents;

        void Reset()
        {
            me->setActive(true);
            combatEvents.Reset();
            comsmeticEvents.ScheduleEvent(EVENT_COSMETIC_BREATH, 40 * IN_MILLISECONDS);
        }

        void EnterCombat(Unit* who)
        {
            combatEvents.ScheduleEvent(EVENT_VOLCANIC_BREATH, 5 * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            me->CastSpell((Unit*)NULL, SPELL_KILL_CREDIT, false);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                comsmeticEvents.Update(diff);

                if (comsmeticEvents.ExecuteEvent() == EVENT_COSMETIC_BREATH)
                {
                    me->CastSpell((Unit*)NULL, SPELL_COSMETIC_BREATH, false);
                    comsmeticEvents.ScheduleEvent(EVENT_COSMETIC_BREATH, 40 * IN_MILLISECONDS);
                }

                return;
            }

            combatEvents.Update(diff);

            if (combatEvents.ExecuteEvent() == EVENT_VOLCANIC_BREATH)
            {
                me->CastSpell((Unit*)NULL, SPELL_VOLCANIC_BREATH, false);
                combatEvents.ScheduleEvent(EVENT_VOLCANIC_BREATH, 30 * IN_MILLISECONDS);
            }
        }
    };
};

class spell_volcanoth_kill_credit_qvolcanoth : public SpellScriptLoader
{
public:
    spell_volcanoth_kill_credit_qvolcanoth() : SpellScriptLoader("spell_volcanoth_kill_credit_qvolcanoth") { }

    enum eSpells
    {
        SPELL_IMMINENT_DEATH          = 73090,
        SPELL_SUMMON_EXPSLOSION_BUNNY = 73194,
        SPELL_DUMMY_TO_SASSY          = 73156,
        SPELL_PHASE_08                = 73065,
        SPELL_PHASE_07                = 72676,
        SPELL_EATHQUAKE               = 72993
    };

    class spell_volcanoth_kill_credit_qvolcanoth_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_volcanoth_kill_credit_qvolcanoth_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Unit* target = GetHitUnit())
            {
                target->RemoveAura(SPELL_PHASE_07);
                target->CastSpell(target, SPELL_PHASE_08, false);
                target->CastSpell(target, SPELL_IMMINENT_DEATH, false);
                target->CastSpell(target, SPELL_DUMMY_TO_SASSY, false);
                target->CastSpell(target, SPELL_SUMMON_EXPSLOSION_BUNNY, false);
                target->CastSpell(target, SPELL_EATHQUAKE, false);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_volcanoth_kill_credit_qvolcanoth_SpellScript::HandleEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_volcanoth_kill_credit_qvolcanoth_SpellScript();
    }
};

class CosmeticBootzookaTargetSelector
{
public:
    CosmeticBootzookaTargetSelector() { }

    enum eNPC
    {
        NPC_BUNNY = 33111
    };

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (cre->GetEntry() == NPC_BUNNY)
                return false;

        return true;
    }
};

class spell_bootzooka_visual_filter_qvolcanoth : public SpellScriptLoader
{
public:
    spell_bootzooka_visual_filter_qvolcanoth() : SpellScriptLoader("spell_bootzooka_visual_filter_qvolcanoth") { }

    class spell_bootzooka_visual_filter_qvolcanoth_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bootzooka_visual_filter_qvolcanoth_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (Unit* caster = GetCaster())
                targets.remove_if(CosmeticBootzookaTargetSelector());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_bootzooka_visual_filter_qvolcanoth_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_bootzooka_visual_filter_qvolcanoth_SpellScript();
    }
};

class spell_bootzooka_qvolcanoth : public SpellScriptLoader
{
public:
    spell_bootzooka_qvolcanoth() : SpellScriptLoader("spell_bootzooka_qvolcanoth") { }

    enum eSpells
    {
        SPELL_BOOZOOKA_COSMETIC = 73000
    };

    class spell_bootzooka_qvolcanoth_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bootzooka_qvolcanoth_SpellScript);

        void HandleAfterCast()
        {
            if (Unit* caster = GetCaster())
                caster->CastSpell(caster, SPELL_BOOZOOKA_COSMETIC, false);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_bootzooka_qvolcanoth_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_bootzooka_qvolcanoth_SpellScript();
    }
};

class spell_fire_jet_periodic_qvolcanoth : public SpellScriptLoader
{
public:
    spell_fire_jet_periodic_qvolcanoth() : SpellScriptLoader("spell_fire_jet_periodic_qvolcanoth") { }

    enum
    {
        NPC_BUNNY_TARGET = 38908,
        SPELL_COSMETIC_BREATH = 73016
    };

    class spell_fire_jet_periodic_qvolcanoth_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fire_jet_periodic_qvolcanoth_AuraScript);

        bool active;

        bool Load()
        {
            active = false;
            return true;
        }

        void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 2 * IN_MILLISECONDS;
        }

        void HandleDummyTick(AuraEffect const* aurEff)
        {
            Unit* caster = GetCaster();

            std::list<Creature*> bunnies;
            caster->GetCreatureListWithEntryInGrid(bunnies, NPC_BUNNY_TARGET, 100.0f);
            if (!bunnies.empty())
            {
                for(auto creature : bunnies)
                {
                    if (!active)
                    {
                        active = true;
                        if (!creature->isActiveObject())
                            creature->setActive(true);
                    }

                    creature->CastSpell((Unit*)NULL, SPELL_COSMETIC_BREATH, false);
                }
            }
        }

        void Register()
        {
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_fire_jet_periodic_qvolcanoth_AuraScript::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_fire_jet_periodic_qvolcanoth_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_fire_jet_periodic_qvolcanoth_AuraScript();
    }
};

class npc_volcanoth_dead_explosion_bunny_lost_isles : public CreatureScript
{
public:
    npc_volcanoth_dead_explosion_bunny_lost_isles() : CreatureScript("npc_volcanoth_dead_explosion_bunny_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_volcanoth_dead_explosion_bunny_lost_islesAI(creature);
    }

    enum eSpells
    {
        SPELL_DEAD_EXPLOSION = 73193
    };

    struct npc_volcanoth_dead_explosion_bunny_lost_islesAI : public ScriptedAI
    {
        npc_volcanoth_dead_explosion_bunny_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        void InitializeAI()
        {
            me->setActive(true);
            me->CastSpell((Unit*)NULL, SPELL_DEAD_EXPLOSION, false);

            Reset();
        }
    };
};

class npc_sassy_hardwrench_qvolcanoth_lost_isles : public CreatureScript
{
public:
    npc_sassy_hardwrench_qvolcanoth_lost_isles() : CreatureScript("npc_sassy_hardwrench_qvolcanoth_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_sassy_hardwrench_qvolcanoth_lost_islesAI(creature);
    }

    enum eSpells
    {
        SPELL_DUMMY_TO_SASSY     = 73156,
        SPELL_VOLCANOTH_COMPLETE = 74103,
        SPELL_OLD_FRIENDS_ACCEPT = 73135,

        TALK_GET_OVER = 0,

        QUEST_VOLCANOTH = 24958,
        QUEST_OLD_FRIENDS = 25023
    };

    bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_VOLCANOTH)
            player->CastSpell(player, SPELL_VOLCANOTH_COMPLETE, false);

        return true;
    }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_OLD_FRIENDS)
            creature->CastSpell(player, SPELL_OLD_FRIENDS_ACCEPT, false);

        return true;
    }

    struct npc_sassy_hardwrench_qvolcanoth_lost_islesAI : public ScriptedAI
    {
        npc_sassy_hardwrench_qvolcanoth_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_DUMMY_TO_SASSY)
                Talk(TALK_GET_OVER, caster->GetGUID(), true);
        }
    };
};

class spell_quest_old_friends_accept : public SpellScriptLoader
{
public:
    spell_quest_old_friends_accept() : SpellScriptLoader("spell_quest_old_friends_accept") { }

    enum eSpells
    {
        SPELL_SUMMON_FLYING_BOMBMER = 73105
    };

    class spell_quest_old_friends_accept_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_quest_old_friends_accept_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Unit* target = GetHitUnit())
                target->CastSpell(target, SPELL_SUMMON_FLYING_BOMBMER, false);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_quest_old_friends_accept_SpellScript::HandleEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_quest_old_friends_accept_SpellScript();
    }
};

const float BomberPathQOF[25][3] =
{
    { 1151.068f, 1115.432f, 129.643f  },
    { 1135.733f, 1140.052f, 142.9035f },
    { 1099.583f, 1166.042f, 160.9143f },
    { 1035.833f, 1199.629f, 140.9983f },
    { 985.6024f, 1262.278f, 123.749f  },
    { 993.0347f, 1328.351f, 114.8601f },
    { 1028.523f, 1418.127f, 106.6932f },
    { 1012.313f, 1533.017f, 129.8396f },
    { 922.4358f, 1584.993f, 168.4136f },
    { 798.5573f, 1588.155f, 174.2195f },
    { 737.9097f, 1636.477f, 142.386f  },
    { 721.8958f, 1756.576f, 120.6636f },
    { 763.0087f, 1888.224f, 119.608f  },
    { 808.3715f, 2035.257f, 112.9135f },
    { 894.9583f, 2161.132f, 93.30235f },
    { 940.9375f, 2306.293f, 39.80235f },
    { 938.3368f, 2458.16f,  23.83012f },
    { 868.5504f, 2537.892f, 11.08012f },
    { 771.257f,  2526.758f, 11.08012f },
    { 746.3073f, 2438.623f, 11.08012f },
    { 807.4288f, 2367.575f, 30.66352f },
    { 930.7847f, 2316.547f, 45.20028f },
    { 1235.776f, 2192.42f,  93.26237f },
    { 1534.63f,  2529.204f, 125.1861f },
    { 1584.891f, 2684.934f, 95.60267f }
};

class npc_flying_bomber_qof : public CreatureScript
{
public:
    npc_flying_bomber_qof() : CreatureScript("npc_flying_bomber_qof") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_flying_bomber_qofAI(creature);
    }

    enum eEvents
    {
        EVENT_FIRST_TALK  = 1,
        EVENT_SECOND_TALK = 2,
        EVENT_THIRD_TALK  = 3,
        EVENT_FOURTH_TALK = 4,
        EVENT_FIFTH_TALK  = 5,
        EVENT_SPLINE_DONE = 6,

        TALK_1 = 0,
        TALK_2 = 1,
        TALK_3 = 2,
        TALK_4 = 3,
        TALK_5 = 4,

        SPELL_DESPAWN_SUMMONS = 73153,
    };

    struct npc_flying_bomber_qofAI : public VehicleAI
    {
        npc_flying_bomber_qofAI(Creature * c) : VehicleAI(c) {}

        EventMap events;

        void InitializeAI()
        {
            ASSERT(me->IsSummon());
        }

        void IsSummonedBy(Unit* summoner)
        {
            Movement::MoveSplineInit init(me);
            for(uint8 i = 0; i < 25; ++i)
            {
                G3D::Vector3 path(BomberPathQOF[i][0], BomberPathQOF[i][1], BomberPathQOF[i][2]);
                init.Path().push_back(path);
            }
            init.SetSmooth();
            init.SetUncompressed();
            init.SetFly();
            init.SetVelocity(27.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_FIRST_TALK, 1 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SPLINE_DONE, me->GetSplineDuration());
        }

        void SassyTalkToSummoner(uint8 textId)
        {
            if (Vehicle* vehicle = me->GetVehicleKit())
                if (Unit* passenger = vehicle->GetPassenger(0))
                    if (Creature* sassy = passenger->ToCreature())
                        sassy->AI()->Talk(textId, me->ToTempSummon()->GetSummonerGUID(), true);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_FIRST_TALK:
                        SassyTalkToSummoner(TALK_1);
                        events.ScheduleEvent(EVENT_SECOND_TALK, 6 * IN_MILLISECONDS);
                        break;
                    case EVENT_SECOND_TALK:
                        SassyTalkToSummoner(TALK_2);
                        events.ScheduleEvent(EVENT_THIRD_TALK, 53 * IN_MILLISECONDS);
                        break;
                    case EVENT_THIRD_TALK:
                        SassyTalkToSummoner(TALK_3);
                        events.ScheduleEvent(EVENT_FOURTH_TALK, 7 * IN_MILLISECONDS);
                        break;
                    case EVENT_FOURTH_TALK:
                        SassyTalkToSummoner(TALK_4);
                        events.ScheduleEvent(EVENT_FIFTH_TALK, 11 * IN_MILLISECONDS);
                        break;
                    case EVENT_FIFTH_TALK:
                        SassyTalkToSummoner(TALK_5);
                        break;
                    case EVENT_SPLINE_DONE:
                        if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                            summoner->CastSpell((Unit*)NULL, SPELL_DESPAWN_SUMMONS, false);
                        break;
                }
            }
        }
    };
};

class FlyingBomberSelectorQOF
{
public:
    FlyingBomberSelectorQOF(Unit* caster) : _caster(caster) { }

    enum eNpc
    {
        NPC_FLYING_BOMBER = 38918
    };

    bool operator()(WorldObject* object)
    {
        if (Creature* cre = object->ToCreature())
            if (cre->IsSummon() && cre->GetEntry() == NPC_FLYING_BOMBER && cre->ToTempSummon()->GetOwner() == _caster)
                return false;

        return true;
    }

private:
    Unit* _caster;
};

class spell_despawn_flying_bomber_qof : public SpellScriptLoader
{
public:
    spell_despawn_flying_bomber_qof() : SpellScriptLoader("spell_despawn_flying_bomber_qof") { }

    class spell_despawn_flying_bomber_qof_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_despawn_flying_bomber_qof_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Creature* target = GetHitCreature())
                target->ToCreature()->DespawnOrUnsummon();
        }

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (Unit* caster = GetCaster())
                targets.remove_if(FlyingBomberSelectorQOF(caster));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_despawn_flying_bomber_qof_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_despawn_flying_bomber_qof_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_despawn_flying_bomber_qof_SpellScript();
    }
};

class npc_gnomeregan_stealth_flighter_lost_isles : public CreatureScript
{
public:
    npc_gnomeregan_stealth_flighter_lost_isles() : CreatureScript("npc_gnomeregan_stealth_flighter_lost_isles") {}

    enum eNpc
    {
        NPC_GNOMEREGAN_STEALTH_FLIGHTER = 39039,
        SPELL_SUMMON_PARATROOPER = 73327,
        EVENT_SUMMON_PARATROOPER = 1
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_gnomeregan_stealth_flighter_lost_islesAI(creature);
    }

    struct npc_gnomeregan_stealth_flighter_lost_islesAI : public ScriptedAI
    {
        npc_gnomeregan_stealth_flighter_lost_islesAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            me->setActive(true);
            me->SetReactState(REACT_PASSIVE);
            events.ScheduleEvent(EVENT_SUMMON_PARATROOPER, urand(30, 60) * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (me->GetHealth() <= damage)
            {
                me->DespawnOrUnsummon(3.5 * IN_MILLISECONDS);
                me->CastSpell(me, SPELL_SUMMON_PARATROOPER, false);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SUMMON_PARATROOPER)
            {
                LiquidData liquid_status;
                ZLiquidStatus res = me->GetMap()->getLiquidStatus(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), MAP_ALL_LIQUIDS, &liquid_status);

                if (res == MAP_LIQUID_TYPE_NO_WATER)
                    me->CastSpell(me, SPELL_SUMMON_PARATROOPER, false);

                events.ScheduleEvent(EVENT_SUMMON_PARATROOPER, urand(30, 60) * IN_MILLISECONDS);
            }
        }
    };
};

class npc_alliance_paratrooper_lost_isles : public CreatureScript
{
public:
    npc_alliance_paratrooper_lost_isles() : CreatureScript("npc_alliance_paratrooper_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_alliance_paratrooper_lost_islesAI(creature);
    }

    enum eSpells
    {
        SPELL_COSMETIC_PARACHUTE = 73363,
        EVENT_GROUNDED = 1,
        NPC_ORC_BATTLESWORN = 39044
    };

    struct npc_alliance_paratrooper_lost_islesAI : public ScriptedAI
    {
        npc_alliance_paratrooper_lost_islesAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->CastSpell(me, SPELL_COSMETIC_PARACHUTE, false);
            me->setActive(true);

            Movement::MoveSplineInit init(me);
            init.SetFly();
            init.MoveTo(me->GetPositionX(), me->GetPositionY(), me->GetMap()->GetHeight(me->GetPhaseMask(), me->GetPositionX(), me->GetPositionY(), MAX_HEIGHT));
            init.SetVelocity(10.0f);
            init.SetAnimation(Movement::ToGround);
            init.Launch();

            events.ScheduleEvent(EVENT_GROUNDED, me->GetSplineDuration());
            Reset();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE && id == 1)
            {
                if (Unit* target = me->SelectNearbyTarget((Unit*)NULL, 30.0f))
                    AttackStart(target);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_GROUNDED:
                        if (me->GetDistance2d(1716.258f, 2880.982f) > me->GetDistance2d(1856.252f, 2766.610f))
                            me->GetMotionMaster()->MovePoint(1, 1856.252f, 2766.610f, 12.878f);
                        else
                            me->GetMotionMaster()->MovePoint(1, 1716.258f, 2880.982f, 9.849f);

                        me->SetReactState(REACT_DEFENSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->RemoveAura(SPELL_COSMETIC_PARACHUTE);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_aggra_lost_isles_wl : public CreatureScript
{
public:
    npc_aggra_lost_isles_wl() : CreatureScript("npc_aggra_lost_isles_wl") {}

    enum eQuests
    {
        QUEST_THE_HEADS_OF_SI_7 = 25093,

        TALK_QA = 0,
        TALK_QR = 1
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_THE_HEADS_OF_SI_7)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QA, player->GetGUID(), true);

        return true;
    }

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 /*item*/)
    {
        if (quest->GetQuestId() == QUEST_THE_HEADS_OF_SI_7)
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QR, player->GetGUID(), true);

        return true;
    }
};

class npc_pride_of_kezan_qtpok : public CreatureScript
{
public:
    npc_pride_of_kezan_qtpok() : CreatureScript("npc_pride_of_kezan_qtpok") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_pride_of_kezan_qtpokAI(creature);
    }

    enum
    {
        SPELL_LEAVING_AREA_WARRING = 73428,
        SPELL_PARACHUTE            = 73455,
        SPELL_EJECT_PASSENGERS     = 68576,

        EVENT_ZONE_CHECK = 1,
        EVENT_COUNTDOWN  = 2,

        ZONE_TRANQUIL_COAST    = 4914,

        ACTION_COUNTDOWN      = 1,
        ACTION_STOP_COUNTDOWN = 2
    };

    struct npc_pride_of_kezan_qtpokAI : public VehicleAI
    {
        npc_pride_of_kezan_qtpokAI(Creature * c) : VehicleAI(c) {}

        EventMap events;
        bool countdown;

        void InitializeAI()
        {
            ASSERT(me->IsSummon());
            countdown = false;
        }

        void IsSummonedBy(Unit* summoner)
        {
            events.ScheduleEvent(EVENT_ZONE_CHECK, 1 * IN_MILLISECONDS);
        }

        void DoAction(const int32 actionId)
        {
            switch(actionId)
            {
                case ACTION_COUNTDOWN:
                    countdown = true;
                    events.ScheduleEvent(EVENT_COUNTDOWN, 15 * IN_MILLISECONDS);
                    break;
                case ACTION_STOP_COUNTDOWN:
                    countdown = false;
                    events.CancelEvent(EVENT_COUNTDOWN);
                    break;
            }
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (!apply)
            {
                me->CastSpell((Unit*)NULL, SPELL_PARACHUTE, false);
                who->RemoveAura(SPELL_LEAVING_AREA_WARRING);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_COUNTDOWN:
                        me->DespawnOrUnsummon(2 * IN_MILLISECONDS);
                        me->CastSpell(me, SPELL_EJECT_PASSENGERS, TRIGGERED_FULL_MASK);
                        break;
                    case EVENT_ZONE_CHECK:
                    {
                        uint16 area = me->GetAreaId();
                        if (area != ZONE_TRANQUIL_COAST)
                        {
                            if (!countdown)
                               if (!me->HasAura(SPELL_LEAVING_AREA_WARRING))
                                    me->CastSpell((Unit*)NULL, SPELL_LEAVING_AREA_WARRING, false);
                        }
                        else
                        {
                            if (me->HasAura(SPELL_LEAVING_AREA_WARRING))
                            {
                                me->RemoveAura(SPELL_LEAVING_AREA_WARRING);

                                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                                    summoner->RemoveAura(SPELL_LEAVING_AREA_WARRING);
                            }
                        }
                    }
                    events.ScheduleEvent(EVENT_ZONE_CHECK, 1 * IN_MILLISECONDS);
                    break;
                }
            }
        }
    };
};

class spell_leaving_area_warring_qtpok : public SpellScriptLoader
{
public:
    spell_leaving_area_warring_qtpok() : SpellScriptLoader("spell_leaving_area_warring_qtpok") { }

    enum
    {
        ACTION_COUNTDOWN      = 1,
        ACTION_STOP_COUNTDOWN = 2
    };

    class spell_leaving_area_warring_qtpok_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_leaving_area_warring_qtpok_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Creature* owner = GetOwner()->ToCreature();
            if (!owner)
                return;

            if (owner->IsAIEnabled)
                owner->AI()->DoAction(ACTION_COUNTDOWN);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Creature* owner = GetOwner()->ToCreature();
            if (!owner)
                return;

            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                if (owner->IsAIEnabled)
                    owner->AI()->DoAction(ACTION_STOP_COUNTDOWN);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_leaving_area_warring_qtpok_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_leaving_area_warring_qtpok_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_leaving_area_warring_qtpok_AuraScript();
    }
};

class npc_kilag_gorefang_qlr : public CreatureScript
{
public:
    npc_kilag_gorefang_qlr() : CreatureScript("npc_kilag_gorefang_qlr") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_kilag_gorefang_qlrAI(creature);
    }

    enum eSpells
    {
        SPELL_QLR_ACCEPT = 73532,
        QUEST_LETS_RIDE = 25100,
        TALK_QA = 0
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_LETS_RIDE)
        {
            player->CastSpell(player, SPELL_QLR_ACCEPT, false);
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QA, player->GetGUID(), true);
        }

        return true;
    }

    struct npc_kilag_gorefang_qlrAI : public ScriptedAI
    {
        npc_kilag_gorefang_qlrAI(Creature * c) : ScriptedAI(c) {}

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }
    };
};

const float BastiaPathWPQLR[50][3] =
{
    { 1714.887f, 2841.918f, 17.80678f },
    { 1715.491f, 2825.375f, 27.50569f },
    { 1713.401f, 2812.724f, 33.05553f },
    { 1707.38f,  2790.811f, 38.42721f },
    { 1691.804f, 2775.447f, 47.07444f },
    { 1677.771f, 2763.518f, 58.11836f },
    { 1662.953f, 2751.408f, 67.96738f },
    { 1650.644f, 2743.627f, 73.39718f },
    { 1634.74f,  2733.807f, 78.37968f },
    { 1618.495f, 2724.411f, 81.52821f },
    { 1599.793f, 2717.84f,  82.99432f },
    { 1572.321f, 2711.193f, 84.76134f },
    { 1565.411f, 2700.459f, 87.49076f },
    { 1564.866f, 2683.269f, 91.48758f },
    { 1566.085f, 2668.811f, 93.96507f },
    { 1575.575f, 2659.29f,  96.09312f },
    { 1592.505f, 2652.558f, 96.36104f },
    { 1618.379f, 2641.814f, 97.60857f },
    { 1662.866f, 2620.088f, 96.43405f },
    { 1698.408f, 2599.822f, 96.41636f },
    { 1725.689f, 2574.59f,  100.8454f },
    { 1738.896f, 2550.74f,  104.2022f },
    { 1746.675f, 2526.562f, 110.6833f },
    { 1748.517f, 2516.984f, 115.5963f },
    { 1755.415f, 2507.885f, 121.5244f },
    { 1774.222f, 2503.575f, 130.3001f },
    { 1781.03f,  2492.333f, 138.3618f },
    { 1784.519f, 2474.472f, 145.5816f },
    { 1779.42f,  2445.134f, 150.6537f },
    { 1775.873f, 2437.181f, 152.0934f },
    { 1773.88f,  2433.24f,  152.5676f },
    { 1768.443f, 2423.182f, 147.0627f },
    { 1762.04f,  2411.097f, 144.8849f },
    { 1756.076f, 2399.76f,  145.5517f },
    { 1749.457f, 2387.339f, 150.2805f },
    { 1744.099f, 2376.351f, 159.5151f },
    { 1741.538f, 2371.378f, 160.2534f },
    { 1736.741f, 2360.335f, 166.1565f },
    { 1741.28f,  2340.196f, 175.3122f },
    { 1744.193f, 2336.554f, 179.2318f },
    { 1746.325f, 2324.137f, 186.6794f },
    { 1750.389f, 2314.833f, 186.9828f },
    { 1773.734f, 2289.837f, 188.1935f },
    { 1790.807f, 2264.365f, 190.670f  },//
    { 1821.380f, 2234.546f, 181.943f  },
    { 1879.892f, 2137.988f, 184.584f  },
    { 1902.137f, 2084.697f, 186.071f  },
    { 1893.653f, 2049.669f, 190.882f  },
    { 1882.045f, 2010.216f, 205.056f  },
    { 1855.219f, 1974.520f, 221.449f  }
};

class npc_bastia_qlr: public CreatureScript
{
public:
    npc_bastia_qlr() : CreatureScript("npc_bastia_qlr") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_bastia_qlrAI(creature);
    }

    enum eSpells
    {
        SPELL_EJECT_PASSENGERS = 68576,
        EVENT_JUMP       = 1,
        EVENT_AFTER_JUMP = 2,
        EVENT_DONE       = 3
    };

    struct npc_bastia_qlrAI : public VehicleAI
    {
        npc_bastia_qlrAI(Creature * c) : VehicleAI(c) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            Movement::MoveSplineInit init(me);
            for(uint8 i = 0; i < 44; ++i)
            {
                G3D::Vector3 path(BastiaPathWPQLR[i][0], BastiaPathWPQLR[i][1], BastiaPathWPQLR[i][2]);
                init.Path().push_back(path);
            }
            init.SetSmooth();
            init.SetUncompressed();
            init.SetVelocity(15.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_JUMP, me->GetSplineDuration());
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_JUMP:
                        me->GetMotionMaster()->MoveJump(1821.380f, 2234.546f, 181.943f, 20.0f, 25.0f);
                        events.ScheduleEvent(EVENT_AFTER_JUMP, me->GetSplineDuration());
                        break;
                    case EVENT_DONE:
                        me->CastSpell(me, SPELL_EJECT_PASSENGERS, TRIGGERED_FULL_MASK);
                        break;
                    case EVENT_AFTER_JUMP:
                    {
                        Movement::MoveSplineInit init(me);
                        for(uint8 i = 44; i < 50; ++i)
                        {
                            G3D::Vector3 path(BastiaPathWPQLR[i][0], BastiaPathWPQLR[i][1], BastiaPathWPQLR[i][2]);
                            init.Path().push_back(path);
                        }
                        init.SetSmooth();
                        init.SetUncompressed();
                        init.SetVelocity(15.0f);
                        init.Launch();
                    }
                    events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
                    break;
                }
            }
        }
    };
};

class npc_slinky_sharpshiv_lost_isles : public CreatureScript
{
public:
	npc_slinky_sharpshiv_lost_isles() : CreatureScript("npc_slinky_sharpshiv_lost_isles") {}

	CreatureAI * GetAI(Creature * creature) const
	{
		return new npc_slinky_sharpshiv_lost_islesAI(creature);
	}

    enum
    {
        TALK_QA = 0,
        QUEST_THE_GALLYWIX_LABOR_MINE = 25109,
        SPELL_DETECT_ISVISIBLITY = 79549,
        SPELL_DETECT_ACE         = 73593,
        SPELL_DETECT_IZZY        = 73595,
        SPELL_DETECT_GOBBER      = 73597
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_THE_GALLYWIX_LABOR_MINE)
        {
            player->CastSpell(player, SPELL_DETECT_ISVISIBLITY, false);
            player->CastSpell(player, SPELL_DETECT_ACE, false);
            player->CastSpell(player, SPELL_DETECT_IZZY, false);
            player->CastSpell(player, SPELL_DETECT_GOBBER, false);
            if (creature->IsAIEnabled)
                creature->AI()->Talk(TALK_QA, player->GetGUID(), true);
        }

        player->SaveToDB();
        return true;
    }

	struct npc_slinky_sharpshiv_lost_islesAI : public ScriptedAI
	{
		npc_slinky_sharpshiv_lost_islesAI(Creature * c) : ScriptedAI(c) {}

		void DamageTaken(Unit* attacker, uint32 &damage)
		{
			if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
			{
				if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
				{
					damage = 0;
					me->getThreatManager().addThreat(attacker, 0.f);
				}
			}
			else
			{
				if (Unit* victim = me->GetVictim())
				{
					if (victim->GetTypeId() == TYPEID_UNIT)
					{
						me->getThreatManager().resetAllAggro();
						me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
						AttackStart(attacker);
					}
				}
			}
		}
	};
};

class npc_assistant_greely_summoned_lost_isles : public CreatureScript
{
public:
    npc_assistant_greely_summoned_lost_isles() : CreatureScript("npc_assistant_greely_summoned_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_assistant_greely_summoned_lost_islesAI(creature);
    }

    enum eQuests
    {
        QUEST_MORALE_BOOST               = 25122,
        QUEST_LIGHT_AT_THE_END_OF_TUNNEL = 25125,

        SPELL_KAJA_COLA       = 73599,

        TALK_KAJA_COLA_HIT = 0,
        TALK_MORALE_BOOST  = 1,
        TALK_LATEOT        = 2,

        EVENT_KAJA_COLA = 1
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_MORALE_BOOST:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_MORALE_BOOST, player->GetGUID(), true);
                break;
            case QUEST_LIGHT_AT_THE_END_OF_TUNNEL:
                if (creature->IsAIEnabled)
                    creature->AI()->Talk(TALK_LATEOT, player->GetGUID(), true);
                break;
        }

        return true;
    }

    struct npc_assistant_greely_summoned_lost_islesAI : public ScriptedAI
    {
        npc_assistant_greely_summoned_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void InitializeAI()
        {
            ASSERT(me->IsSummon());
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
            events.ScheduleEvent(EVENT_KAJA_COLA, 1 * IN_MILLISECONDS);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_KAJA_COLA)
                Talk(TALK_KAJA_COLA_HIT, me->ToTempSummon()->GetSummonerGUID(), true);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_KAJA_COLA)
                me->CastSpell(me, SPELL_KAJA_COLA, false);
        }
    };
};

class npc_quest_morale_boost_generic_ai_lost_isles : public CreatureScript
{
public:
    npc_quest_morale_boost_generic_ai_lost_isles() : CreatureScript("npc_quest_morale_boost_generic_ai_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_quest_morale_boost_generic_ai_lost_islesAI(creature);
    }

    enum
    {
        QUEST_MORALE_BOOST = 25122,

        SPELL_KAJA_COLA          = 73599,
        SPELL_KAJA_COLA_ZERO_ONE = 73583,
        SPELL_ACE_FREED          = 73602,
        SPELL_GOBBER_FREED       = 73614,
        SPELL_DETECT_GOBBER      = 73597,
        SPELL_DETECT_ACE         = 73593,

        NPC_MORALE_BOOST_QUEST_CREDIT        = 38409,
        NPC_MORALE_BOOST_QUEST_CREDIT_GOBBER = 38746,
        NPC_MORALE_BOOST_QUEST_CREDIT_ACE    = 38441,
        NPC_KEZAN_CITIZEN                    = 38745,
        NPC_GOBLIN_SURVIVOR                  = 38409,
        NPC_ACE                              = 38441,
        NPC_GOBBER                           = 38746,

        EVENT_KAJA_COLA = 1,

        TALK_KAJA_COLA = 0
    };

    struct npc_quest_morale_boost_generic_ai_lost_islesAI : public ScriptedAI
    {
        npc_quest_morale_boost_generic_ai_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            switch (spell->Id)
            {
                case SPELL_KAJA_COLA_ZERO_ONE:
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (uint8 questState = caster->ToPlayer()->GetQuestStatus(QUEST_MORALE_BOOST))
                           if (questState == QUEST_STATUS_INCOMPLETE)
                           {
                               switch (me->GetEntry())
                               {
                                   case NPC_KEZAN_CITIZEN:
                                   case NPC_GOBLIN_SURVIVOR:
                                       caster->ToPlayer()->KilledMonsterCredit(NPC_MORALE_BOOST_QUEST_CREDIT);
                                       events.ScheduleEvent(EVENT_KAJA_COLA, 1 * IN_MILLISECONDS);
                                       break;
                                   case NPC_ACE:
                                       caster->ToPlayer()->KilledMonsterCredit(NPC_MORALE_BOOST_QUEST_CREDIT_ACE);
                                       caster->CastSpell(caster, SPELL_ACE_FREED, false);
                                       caster->RemoveAura(SPELL_DETECT_ACE);
                                       break;
                                   case NPC_GOBBER:
                                       caster->ToPlayer()->KilledMonsterCredit(NPC_MORALE_BOOST_QUEST_CREDIT_GOBBER);
                                       caster->CastSpell(caster, SPELL_GOBBER_FREED, false);
                                       caster->RemoveAura(SPELL_DETECT_GOBBER);
                                       break;

                               }
                           }
                    }
                    break;
                case SPELL_KAJA_COLA:
                    Talk(TALK_KAJA_COLA, caster->GetGUID(), true);
                    me->GetMotionMaster()->MoveRandom(15.0f);
                    me->DespawnOrUnsummon(5 * IN_MILLISECONDS);
                    break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_KAJA_COLA)
                me->CastSpell(me, SPELL_KAJA_COLA, false);

            DoMeleeAttackIfReady();
        }
    };
};

class npc_quest_morale_boost_summoned_creatures_generic_ai_lost_isles : public CreatureScript
{
public:
    npc_quest_morale_boost_summoned_creatures_generic_ai_lost_isles() : CreatureScript("npc_quest_morale_boost_summoned_creatures_generic_ai_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_quest_morale_boost_summoned_creatures_generic_ai_lost_islesAI(creature);
    }

    enum eSpells
    {
        SPELL_KAJA_COLA = 73599,
        TALK_KAJA_COLA_HIT = 0,
        EVENT_KAJA_COLA = 1
    };

    struct npc_quest_morale_boost_summoned_creatures_generic_ai_lost_islesAI : public ScriptedAI
    {
        npc_quest_morale_boost_summoned_creatures_generic_ai_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void InitializeAI()
        {
            ASSERT(me->IsSummon());
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetCustomVisibility(CUSTOM_VISIBILITY_SEER,summoner->GetGUID());
            me->SetVisible(false);
            me->SetReactState(REACT_PASSIVE);
            events.ScheduleEvent(EVENT_KAJA_COLA, 0.5 * IN_MILLISECONDS);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_KAJA_COLA)
                Talk(TALK_KAJA_COLA_HIT, me->ToTempSummon()->GetSummonerGUID(), true);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_KAJA_COLA)
                me->CastSpell(me, SPELL_KAJA_COLA, false);
        }
    };
};

class npc_blastshadow_the_brutemaster_lost_isles : public CreatureScript
{
public:
    npc_blastshadow_the_brutemaster_lost_isles() : CreatureScript("npc_blastshadow_the_brutemaster_lost_isles") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_blastshadow_the_brutemaster_lost_islesAI(creature);
    }

    enum eTalks
    {
        TALK_AGGRO = 0,

        SPELL_SUMMON_SUCCUBUS = 712,
        SPELL_SHADOW_BOLT     = 9613,
        SPELL_ON_DEATH        = 73723,
        SPELL_SUMMON_CHEST    = 73703,
        SPELL_SOULSTONE       = 73702,
        SPELL_DUMMY_ASSISTANT = 73711,

        EVENT_SHADOW_BOLT = 1,

        QUEST_THROW_IT_ON_THE_GROUND = 25123,

        NPC_KILL_CREDIT = 39276
    };

    struct npc_blastshadow_the_brutemaster_lost_islesAI : public ScriptedAI
    {
        npc_blastshadow_the_brutemaster_lost_islesAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void InitializeAI()
        {
            me->CastSpell(me, SPELL_SUMMON_SUCCUBUS, false);
            Reset();
        }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            Talk(TALK_AGGRO);
            events.ScheduleEvent(EVENT_SHADOW_BOLT, 1 * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (me->GetHealth() <= damage)
            {
                damage = 0;
                me->CastSpell(me, SPELL_SUMMON_CHEST, false);
                attacker->Kill(me);
            }
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_SOULSTONE)
            {
                me->CastSpell(me, SPELL_ON_DEATH, false);
                if (caster->GetTypeId() == TYPEID_PLAYER)
                    if (caster->ToPlayer()->GetQuestStatus(QUEST_THROW_IT_ON_THE_GROUND) == QUEST_STATUS_INCOMPLETE)
                    {
                        caster->ToPlayer()->KilledMonsterCredit(NPC_KILL_CREDIT);
                        me->CastSpell((Unit*)NULL, SPELL_DUMMY_ASSISTANT, false);
                    }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SHADOW_BOLT)
            {
                me->CastSpell((Unit*)NULL, SPELL_SHADOW_BOLT, false);
                events.ScheduleEvent(EVENT_SHADOW_BOLT, 3 * IN_MILLISECONDS);
            }
        }
    };
};

const float MineCartWPPathQWMCR[13][3] =
{
    { 2079.742f, 1848.729f, 137.876f },
    { 2105.577f, 1855.965f, 129.117f },
    { 2124.564f, 1861.043f, 126.438f },
    { 2135.750f, 1864.652f, 126.600f },
    { 2149.362f, 1868.732f, 127.595f }, //
    { 2223.228f, 1892.209f, 65.009f },
    { 2243.553f, 1898.336f, 56.850f },
    { 2271.590f, 1905.173f, 42.105f },
    { 2302.171f, 1914.811f, 33.269f },
    { 2332.589f, 1925.996f, 28.133f },
    { 2342.695f, 1928.764f, 26.483f },
    { 2353.719f, 1929.717f, 25.172f },
    { 2369.384f, 1933.793f, 21.014f }
};

class npc_mine_cart_qwmcr : public CreatureScript
{
public:
    npc_mine_cart_qwmcr() : CreatureScript("npc_mine_cart_qwmcr") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_mine_cart_qwmcrI(creature);
    }

    enum eSpells
    {
        SPELL_EJECT_PASSENGERS = 68576,
        SPELL_EXIT             = 73755,

        EVENT_JUMP       = 1,
        EVENT_AFTER_JUMP = 2,
        EVENT_DONE       = 3,
        EVENT_START_RIDE = 4,

        NPC_ASSISTANT = 39199,
        NPC_ACE       = 39198,
        NPC_IZZY      = 39200,
        NPC_GOBBER    = 39201
    };

    struct npc_mine_cart_qwmcrI : public VehicleAI
    {
        npc_mine_cart_qwmcrI(Creature * c) : VehicleAI(c) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            events.ScheduleEvent(EVENT_START_RIDE, 1 * IN_MILLISECONDS);
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (apply)
              if (who->GetTypeId() == TYPEID_PLAYER)
              {
                  if (Creature* assistant = who->FindNearestCreature(NPC_ASSISTANT, 20.0f))
                     if (assistant->IsSummon() && assistant->ToTempSummon()->GetSummonerGUID() == who->GetGUID())
                         assistant->EnterVehicle(me, 1);

                   if (Creature* ace = who->FindNearestCreature(NPC_ACE, 20.0f))
                      if (ace->IsSummon() && ace->ToTempSummon()->GetSummonerGUID() == who->GetGUID())
                          ace->EnterVehicle(me, 2);

                   if (Creature* izzy = who->FindNearestCreature(NPC_IZZY, 20.0f))
                       if (izzy->IsSummon() && izzy->ToTempSummon()->GetSummonerGUID() == who->GetGUID())
                           izzy->EnterVehicle(me, 3);

                   if (Creature* gobber = who->FindNearestCreature(NPC_GOBBER, 20.0f))
                      if (gobber->IsSummon() && gobber->ToTempSummon()->GetSummonerGUID() == who->GetGUID())
                          gobber->EnterVehicle(me, 3);
              }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_JUMP:
                        me->GetMotionMaster()->MoveJump(2209.028f, 1887.567f, 74.346f, 20.0f, 18.0f);
                        events.ScheduleEvent(EVENT_AFTER_JUMP, me->GetSplineDuration());
                        break;
                    case EVENT_DONE:
                        me->CastSpell((Unit*)NULL, SPELL_EXIT, false);
                        me->CastSpell(me, SPELL_EJECT_PASSENGERS, TRIGGERED_FULL_MASK);
                        break;
                    case EVENT_AFTER_JUMP:
                    {
                        Movement::MoveSplineInit init(me);
                        for (uint8 i = 5; i < 13; ++i)
                        {
                            G3D::Vector3 path(MineCartWPPathQWMCR[i][0], MineCartWPPathQWMCR[i][1], MineCartWPPathQWMCR[i][2]);
                             init.Path().push_back(path);
                        }
                        init.SetSmooth();
                        init.SetUncompressed();
                        init.SetVelocity(15.0f);
                        init.Launch();
                    }
                    events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
                    break;
                    case EVENT_START_RIDE:
                    {
                         Movement::MoveSplineInit init(me);
                         for (uint8 i = 0; i < 5; ++i)
                         {
                              G3D::Vector3 path(MineCartWPPathQWMCR[i][0], MineCartWPPathQWMCR[i][1], MineCartWPPathQWMCR[i][2]);
                              init.Path().push_back(path);
                         }
                         init.SetSmooth();
                         init.SetUncompressed();
                         init.SetVelocity(21.0f);
                         init.Launch();
                    }
                    events.ScheduleEvent(EVENT_JUMP, me->GetSplineDuration());
                    break;
                }
            }
        }
    };
};

class npc_mine_cart_questgiver_qwmcr : public CreatureScript
{
public:
    npc_mine_cart_questgiver_qwmcr() : CreatureScript("npc_mine_cart_questgiver_qwmcr") {}

    enum
    {
        QUEST_WILD_MINE_CART_RIDE = 25184,
        SPELL_QWMCR_ACCEPT = 73759
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_WILD_MINE_CART_RIDE)
            player->CastSpell(player, SPELL_QWMCR_ACCEPT, false);

        return true;
    }
};

class spell_on_exit_qwmcr : public SpellScriptLoader
{
public:
    spell_on_exit_qwmcr() : SpellScriptLoader("spell_on_exit_qwmcr") { }

    enum
    {
        SPELL_PHASE_09             = 73756,
        SPELL_PHASE_08             = 73065,
        SPELL_ACE_CONTROLLER       = 73633,
        SPELL_IZZY_CONTROLLER      = 73635,
        SPELL_GOBBER_CONTROLLER    = 73637,
        SPELL_ASSISTANT_CONTROLLER = 73616,

        NPC_ASSISTANT = 39199,
        NPC_ACE       = 39198,
        NPC_IZZY      = 39200,
        NPC_GOBBER    = 39201
    };

    class spell_on_exit_qwmcr_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_on_exit_qwmcr_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Unit* target = GetHitUnit())
            {
                target->RemoveAura(SPELL_PHASE_08);
                target->CastSpell(target, SPELL_PHASE_09, false);
                target->RemoveAura(SPELL_ACE_CONTROLLER);
                target->RemoveAura(SPELL_IZZY_CONTROLLER);
                target->RemoveAura(SPELL_GOBBER_CONTROLLER);
                target->RemoveAura(SPELL_ASSISTANT_CONTROLLER);

                if (Creature* assistant = target->FindNearestCreature(NPC_ASSISTANT, 20.0f))
                   if (assistant->IsSummon() && assistant->ToTempSummon()->GetSummonerGUID() == target->GetGUID())
                          assistant->DespawnOrUnsummon();

                if (Creature* ace = target->FindNearestCreature(NPC_ACE, 20.0f))
                   if (ace->IsSummon() && ace->ToTempSummon()->GetSummonerGUID() == target->GetGUID())
                       ace->DespawnOrUnsummon();

                if (Creature* izzy = target->FindNearestCreature(NPC_IZZY, 20.0f))
                   if (izzy->IsSummon() && izzy->ToTempSummon()->GetSummonerGUID() == target->GetGUID())
                       izzy->DespawnOrUnsummon();

                 if (Creature* gobber = target->FindNearestCreature(NPC_GOBBER, 20.0f))
                    if (gobber->IsSummon() && gobber->ToTempSummon()->GetSummonerGUID() == target->GetGUID())
                        gobber->DespawnOrUnsummon();
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_on_exit_qwmcr_SpellScript::HandleEffect, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_on_exit_qwmcr_SpellScript();
    }
};


class spell_abandon_quest_check_qwmcr : public SpellScriptLoader
{
public:
    spell_abandon_quest_check_qwmcr() : SpellScriptLoader("spell_abandon_quest_check_qwmcr") { }

    enum
    {
        SPELL_PHASE_09             = 73756,
        SPELL_PHASE_08             = 73065,
        SPELL_SUMMON_ASSISTANT     = 73603,
        SPELL_ASSISTANT_CONTROLLER = 73616,

        QUEST_WILD_MINE_CART_RIDE = 25184
    };

    class spell_abandon_quest_check_qwmcr_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_abandon_quest_check_qwmcr_AuraScript);

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();
            Unit* caster = GetCaster();

            if (caster->GetTypeId() == TYPEID_PLAYER)
               if (caster->ToPlayer()->GetQuestStatus(QUEST_WILD_MINE_CART_RIDE) == QUEST_STATUS_NONE)
               {
                   caster->RemoveAura(SPELL_PHASE_09);
                   caster->CastSpell(caster, SPELL_PHASE_08, false);
                   caster->CastSpell(caster, SPELL_SUMMON_ASSISTANT, false);
               }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_abandon_quest_check_qwmcr_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_abandon_quest_check_qwmcr_AuraScript();
    }
};

class npc_steamwheedle_shark_qss : public CreatureScript
{
public:
    npc_steamwheedle_shark_qss() : CreatureScript("npc_steamwheedle_shark_qss") {}

    enum
    {
        EVENT_COSMETIC_EMOTE = 1,
        EVENT_SHRED_ARMOR    = 2,
        EVENT_SAW_BLADE      = 3,

        SPELL_SHRED_ARMOR = 75962,
        SPELL_SAW_BLADE   = 32735,
        SPELL_ON_DEATH    = 73852
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_steamwheedle_shark_qssAI(creature);
    }

    struct npc_steamwheedle_shark_qssAI : public ScriptedAI
    {
        npc_steamwheedle_shark_qssAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap comsmeticEvents;
        EventMap combatEvents;

        void InitializeAI()
        {
            comsmeticEvents.ScheduleEvent(EVENT_COSMETIC_EMOTE, 2 * IN_MILLISECONDS);
            Reset();
        }

        void Reset()
        {
            combatEvents.Reset();
            comsmeticEvents.ScheduleEvent(EVENT_COSMETIC_EMOTE, 2 * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            me->CastSpell((Unit*)NULL, SPELL_ON_DEATH, false);
        }

        void EnterCombat(Unit* who)
        {
            comsmeticEvents.CancelEvent(EVENT_COSMETIC_EMOTE);
            combatEvents.ScheduleEvent(EVENT_SHRED_ARMOR, urand(6, 8) * IN_MILLISECONDS);
            combatEvents.ScheduleEvent(EVENT_SAW_BLADE, urand(6, 8) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                comsmeticEvents.Update(diff);

                if (comsmeticEvents.ExecuteEvent() == EVENT_COSMETIC_EMOTE)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                    comsmeticEvents.ScheduleEvent(EVENT_COSMETIC_EMOTE, urand(7, 15) * IN_MILLISECONDS);
                }

                return;
            }

            combatEvents.Update(diff);

            if (uint32 eventId = combatEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SHRED_ARMOR:
                        me->CastSpell((Unit*)NULL, SPELL_SHRED_ARMOR, false);
                        combatEvents.ScheduleEvent(EVENT_SHRED_ARMOR, urand(8, 12) * IN_MILLISECONDS);
                        break;
                    case EVENT_SAW_BLADE:
                        me->CastSpell((Unit*)NULL, SPELL_SAW_BLADE, false);
                        combatEvents.ScheduleEvent(EVENT_SAW_BLADE, urand(6, 8) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_good_bye_sweet_oil_invis_controller : public CreatureScript
{
public:
    npc_good_bye_sweet_oil_invis_controller() : CreatureScript("npc_good_bye_sweet_oil_invis_controller") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_good_bye_sweet_oil_invis_controllerAI(creature);
    }

    enum
    {
        GO_FIRE         = 202614,
    };

    struct npc_good_bye_sweet_oil_invis_controllerAI : public ScriptedAI
    {
        npc_good_bye_sweet_oil_invis_controllerAI(Creature * c) : ScriptedAI(c) {}

        void InitializeAI()
        {
            me->setActive(true);
            std::list<GameObject*> gameobjectList;
            GetGameObjectListWithEntryInGrid(gameobjectList, me, GO_FIRE, 120.0f);
            for (auto go : gameobjectList)
            {
                go->m_invisibility.AddFlag(INVISIBILITY_UNK4);
                go->m_invisibility.SetValue(INVISIBILITY_UNK4, 4);
                go->m_invisibilityDetect.AddFlag(INVISIBILITY_UNK4);
                go->m_invisibilityDetect.SetValue(INVISIBILITY_UNK4, 4);
                go->UpdateObjectVisibility();
            }

            Reset();
        }
    };
};

class go_big_red_button_qgbso : public GameObjectScript
{
public:
    go_big_red_button_qgbso() : GameObjectScript("go_big_red_button_qgbso") {}

    enum eSpells
    {
        SPELL_SUMMON_EXPLOSION_BUNNY = 73888,
        QUEST_GOOD_BYE_SWEET_OIL = 25207
    };

    bool OnGossipHello(Player* player, GameObject* pGO)
    {
        if (player->GetQuestStatus(QUEST_GOOD_BYE_SWEET_OIL) == QUEST_STATUS_INCOMPLETE)
            player->CastSpell(player, SPELL_SUMMON_EXPLOSION_BUNNY, false);

        return false;
    }
};

class npc_explosion_bunny_qgbso : public CreatureScript
{
public:
    npc_explosion_bunny_qgbso() : CreatureScript("npc_explosion_bunny_qgbso") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_explosion_bunny_qgbsoAI(creature);
    }

    enum
    {
        SPELL_EXPLOSION_1 = 73904,
        SPELL_EXPLOSION_2 = 73890,
        SPELL_EXPLOSION_3 = 73891,

        EVENT_EXPLOSION_2 = 1,
        EVENT_EXPLOSION_3 = 2
    };

    struct npc_explosion_bunny_qgbsoAI : public ScriptedAI
    {
        npc_explosion_bunny_qgbsoAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            me->SetCustomVisibility(CUSTOM_VISIBILITY_SEER,summoner->GetGUID());
            me->SetVisible(false);
            me->SetReactState(REACT_PASSIVE);
            me->CastSpell((Unit*)NULL, SPELL_EXPLOSION_1, false);
            events.ScheduleEvent(EVENT_EXPLOSION_2, 1 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_EXPLOSION_2:
                        me->CastSpell((Unit*)NULL, SPELL_EXPLOSION_2, false);
                        events.ScheduleEvent(EVENT_EXPLOSION_3, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_EXPLOSION_3:
                        me->CastSpell((Unit*)NULL, SPELL_EXPLOSION_3, false);
                        break;
                }
            }
        }
    };
};

const float FootBombUniformWPPathQTSP[17][3] =
{
    { 2361.232f, 1938.250f, 22.135f },
    { 2347.334f, 1942.312f, 23.440f },
    { 2329.309f, 1948.325f, 23.129f },
    { 2295.223f, 1972.457f, 24.504f },
    { 2261.451f, 2014.458f, 26.726f },
    { 2244.682f, 2044.414f, 28.812f },
    { 2207.258f, 2051.199f, 35.449f },
    { 2159.023f, 2063.637f, 45.087f },
    { 2146.808f, 2096.367f, 50.917f },
    { 2131.898f, 2114.343f, 58.260f },
    { 2120.570f, 2143.210f, 63.006f },
    { 2138.863f, 2221.361f, 60.091f },
    { 2125.645f, 2262.861f, 58.418f },
    { 2112.761f, 2300.803f, 57.120f },
    { 2096.850f, 2328.852f, 57.421f },
    { 2115.433f, 2370.648f, 49.831f },
    { 2120.774f, 2392.428f, 45.987f }
};

class npc_footbomb_uniform_qtsp : public CreatureScript
{
public:
    npc_footbomb_uniform_qtsp() : CreatureScript("npc_footbomb_uniform_qtsp") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_footbomb_uniform_qtspAI(creature);
    }

    enum eEvents
    {
        EVENT_DONE             = 1,
        SPELL_EJECT_PASSENGERS = 68576
    };

    struct npc_footbomb_uniform_qtspAI : public VehicleAI
    {
        npc_footbomb_uniform_qtspAI(Creature* creature) : VehicleAI(creature) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            Movement::MoveSplineInit init(me);
            for (uint8 i = 0; i < 17; ++i)
            {
                G3D::Vector3 path(FootBombUniformWPPathQTSP[i][0], FootBombUniformWPPathQTSP[i][1], FootBombUniformWPPathQTSP[i][2]);
                init.Path().push_back(path);
            }
            init.SetSmooth();
            init.SetUncompressed();
            init.SetVelocity(17.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_DONE)
                me->CastSpell(me, SPELL_EJECT_PASSENGERS, TRIGGERED_FULL_MASK);
        }
    };
};

class npc_goblin_captured_qev : public CreatureScript
{
public:
    npc_goblin_captured_qev() : CreatureScript("npc_goblin_captured_qev") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_goblin_captured_qevAI(creature);
    }

    enum eSpells
    {
        SPELL_CAGE    = 73926,
        SPELL_ROCKETS = 73948,

        QUEST_ESCAPE_VELOCITY = 25214,

        TALK_ROCKETS = 0,

        EVENT_FLY = 1
    };

    struct npc_goblin_captured_qevAI : public ScriptedAI
    {
        npc_goblin_captured_qevAI(Creature * c) : ScriptedAI(c) {}

        EventMap events;

        void InitializeAI()
        {
            me->setActive(true);
            me->CastSpell(me, SPELL_CAGE, false);
        }

        void OnSpellClick(Unit* clicker, bool &/*result*/)
        {
            if (Player* player = clicker->ToPlayer())
            {
                if (player->GetQuestStatus(QUEST_ESCAPE_VELOCITY) == QUEST_STATUS_INCOMPLETE)
                {
                    player->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
                    me->CastSpell(me, SPELL_ROCKETS, false);
                    Talk(TALK_ROCKETS, player->GetGUID(), true);
                    events.ScheduleEvent(EVENT_FLY, 2 * IN_MILLISECONDS);
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_FLY)
            {
                me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 200.0f, 40.0f, 40.0f);
                me->DespawnOrUnsummon(me->GetSplineDuration());
            }
        }
    };
};

class npc_candy_cane_qwkinca : public CreatureScript
{
public:
    npc_candy_cane_qwkinca() : CreatureScript("npc_candy_cane_qwkinca") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_candy_cane_qwkincaAI(creature);
    }

    enum eSpells
    {
        SPELL_COSMETIC_HEARTBROKEN = 62013,
        SPELL_HEARTBREAKER         = 75924,

        TALK_AGGRO        = 0,
        TALK_HEARTBREAKER = 1
    };

    struct npc_candy_cane_qwkincaAI : public ScriptedAI
    {
        npc_candy_cane_qwkincaAI(Creature * c) : ScriptedAI(c) {}

        bool heartBreaker;

        void Reset()
        {
            heartBreaker = false;
        };

        void EnterCombat(Unit* who)
        {
            Talk(TALK_AGGRO, who->GetGUID());
            me->CastSpell(me, SPELL_COSMETIC_HEARTBROKEN, false);
        }

        void DamageTaken(Unit* killer, uint32 &damage)
        {
            if (me->HealthBelowPctDamaged(20, damage))
                if (!heartBreaker)
                {
                    heartBreaker = true;
                    Talk(TALK_HEARTBREAKER, killer->GetGUID());
                    me->CastSpell((Unit*)NULL, SPELL_HEARTBREAKER, false);
                }
        }
    };
};

class npc_gallywix_docks_fighter_generic_ai : public CreatureScript
{
public:
    npc_gallywix_docks_fighter_generic_ai() : CreatureScript("npc_gallywix_docks_fighter_generic_ai") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_gallywix_docks_fighter_generic_aiAI(creature);
    }

    struct npc_gallywix_docks_fighter_generic_aiAI : public ScriptedAI
    {
        npc_gallywix_docks_fighter_generic_aiAI(Creature * c) : ScriptedAI(c) {}

        void InitializeAI()
        {
            me->setActive(true);
            Reset();
        }

        void Reset()
        {
            if (Unit* target = me->SelectNearestTarget(10.0f))
                AttackStart(target);
        };

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }
    };
};

class npc_trade_prince_gallywix_qtfc : public CreatureScript
{
public:
    npc_trade_prince_gallywix_qtfc() : CreatureScript("npc_trade_prince_gallywix_qtfc") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_trade_prince_gallywix_qtfcAI(creature);
    }

    enum
    {
        SPELL_CALL_A_METTING        = 81000,
        SPELL_REVENUE_STREAM        = 74005,
        SPELL_UNLOAD_TOXIC_ASSETS   = 74000,
        SPELL_REVENUE_STREAM_VISUAL = 74006,
        SPELL_YOURE_FIRED           = 74004,
        SPELL_DOWNIZING             = 74003,

        EVENT_CALL_A_METTING       = 1,
        EVENT_REVENUE_STREAM       = 2,
        EVENT_UNLOAD_TOXIC_ASSETS  = 3,
        EVENT_YOURE_FIRED          = 4,
        EVENT_DOWNIZING            = 5,
        EVENT_OUTRO_2              = 6,
        EVENT_OUTRO_3              = 7,
        EVENT_OUTRO_4              = 8,
        EVENT_DUMMY_TO_THRALL      = 9,
        EVENT_RESPAWN              = 10,
        EVENT_START_COMBAT         = 11,

        NPC_THRALL = 39594,

        TALK_AGGRO           = 0,
        TALK_REVENUE_STREAM  = 1,
        TALK_DOWNIZING       = 2,
        TALK_TOXIC           = 3,
        TALK_OUTRO_1         = 4,
        TALK_OUTRO_2         = 5,
        TALK_OUTRO_3         = 6,
        TALK_OUTRO_4         = 7,

        QUEST_FINAL_CONFRONTATION = 25251
    };

    struct npc_trade_prince_gallywix_qtfcAI : public ScriptedAI
    {
        npc_trade_prince_gallywix_qtfcAI(Creature * c) : ScriptedAI(c) {}

        EventMap comsmeticEvents;
        EventMap combatEvents;

        bool fightStarted;
        bool done;

        void Reset()
        {
            combatEvents.Reset();
            fightStarted = false;
            done = false;

            if (Unit* target = me->SelectNearestTarget(10.0f))
                AttackStart(target);
        };

        void DoAction(const int32 actionId)
        {
            if (actionId == 1)
            {
                Talk(TALK_OUTRO_4);
                comsmeticEvents.ScheduleEvent(EVENT_RESPAWN, 5 * IN_MILLISECONDS);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (me->GetHealth() <= damage)
            {
                damage = 0;
                if (!done)
                {
                    done = true;
                    Talk(TALK_OUTRO_1);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_PASSIVE);
                    me->getThreatManager().resetAllAggro();
                    me->CombatStop(true);
                    combatEvents.Reset();
                    me->GetMotionMaster()->MoveTargetedHome();

                    std::list<Player*> playerList;
                    GetPlayerListInGrid(playerList, me, 20.0f);
                    for (auto player : playerList)
                    {
                        if (player->GetQuestStatus(QUEST_FINAL_CONFRONTATION) == QUEST_STATUS_INCOMPLETE)
                            player->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
                    }

                    comsmeticEvents.ScheduleEvent(EVENT_OUTRO_2, 4.4 * IN_MILLISECONDS);
                }
            }

            if (attacker->GetTypeId() == TYPEID_UNIT && attacker->GetEntry() == NPC_THRALL)
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                    damage = 0;
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        AttackStart(attacker);

                        if (!fightStarted)
                        {
                            fightStarted = true;
                            Talk(TALK_AGGRO, attacker->GetGUID());
                            combatEvents.ScheduleEvent(EVENT_START_COMBAT, 3 * IN_MILLISECONDS);
                        }
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (uint32 eventId = comsmeticEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_OUTRO_2:
                        Talk(TALK_OUTRO_2);
                        comsmeticEvents.ScheduleEvent(EVENT_OUTRO_3, 9 * IN_MILLISECONDS);
                        break;
                    case EVENT_OUTRO_3:
                        Talk(TALK_OUTRO_3);
                        comsmeticEvents.ScheduleEvent(EVENT_DUMMY_TO_THRALL, 3 * IN_MILLISECONDS);
                        break;
                    case EVENT_DUMMY_TO_THRALL:
                        if (Creature* thrall = me->FindNearestCreature(NPC_THRALL, 20.0f))
                            if (thrall->IsAIEnabled)
                                thrall->AI()->DoAction(1);
                            break;
                    case EVENT_RESPAWN:
                        if (Creature* thrall = me->FindNearestCreature(NPC_THRALL, 20.0f))
                            thrall->DespawnOrUnsummon();

                        me->DespawnOrUnsummon();
                        break;

                }
            }

            comsmeticEvents.Update(diff);
            combatEvents.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = combatEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_COMBAT:
                        me->CastSpell((Unit*)NULL, SPELL_CALL_A_METTING, false);
                        combatEvents.ScheduleEvent(EVENT_DOWNIZING, 7 * IN_MILLISECONDS);
                        combatEvents.ScheduleEvent(EVENT_REVENUE_STREAM, 17 * IN_MILLISECONDS);
                        combatEvents.ScheduleEvent(EVENT_UNLOAD_TOXIC_ASSETS, 13 * IN_MILLISECONDS);
                        combatEvents.ScheduleEvent(EVENT_YOURE_FIRED, 3 * IN_MILLISECONDS);
                        break;
                    case EVENT_DOWNIZING:
                        Talk(TALK_DOWNIZING);
                        if (Creature* thrall = me->FindNearestCreature(NPC_THRALL, 20 * IN_MILLISECONDS))
                            me->CastSpell(thrall, SPELL_DOWNIZING, false);
                        combatEvents.ScheduleEvent(EVENT_DOWNIZING, urand(25, 30) * IN_MILLISECONDS);
                        break;
                    case EVENT_REVENUE_STREAM:
                        Talk(TALK_REVENUE_STREAM);
                        me->CastSpell((Unit*)NULL, SPELL_REVENUE_STREAM, false);
                        me->CastSpell((Unit*)NULL, SPELL_REVENUE_STREAM_VISUAL, false);
                        combatEvents.ScheduleEvent(EVENT_REVENUE_STREAM, urand(17, 23) * IN_MILLISECONDS);
                        break;
                    case EVENT_UNLOAD_TOXIC_ASSETS:
                        Talk(TALK_TOXIC);
                        me->CastSpell((Unit*)NULL, SPELL_UNLOAD_TOXIC_ASSETS, false);
                        combatEvents.ScheduleEvent(EVENT_UNLOAD_TOXIC_ASSETS, urand(21, 25) * IN_MILLISECONDS);
                        break;
                    case EVENT_YOURE_FIRED:
                        me->CastSpell((Unit*)NULL, SPELL_YOURE_FIRED, false);
                        combatEvents.ScheduleEvent(EVENT_REVENUE_STREAM, urand(14, 17) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};


class npc_thrall_qtfc : public CreatureScript
{
public:
    npc_thrall_qtfc() : CreatureScript("npc_thrall_qtfc") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_thrall_qtfcAI(creature);
    }

    enum eSpells
    {
        SPELL_CALL_A_METTING  = 81000,
        SPELL_FORCE_OF_NATURE = 74023,
        SPELL_THUNDER         = 74021,
        SPELL_LAVA_BURST      = 74020,
        SPELL_CHAIN_LIGHTNING = 74019,

        EVENT_FORCE_OF_NATURE = 1,
        EVENT_THUNDER         = 2,
        EVENT_LAVA_BUTST      = 3,
        EVENT_CHAIN_LIGHTNING = 4,
        EVENT_TALK_OUTRO_2    = 5,
        EVENT_TALK_OUTRO_3    = 6,
        EVENT_TALK_OUTRO_4    = 7,
        EVENT_DUMMY_TO_PRINCE = 8,

        TALK_OUTRO_1 = 0,
        TALK_OUTRO_2 = 1,
        TALK_OUTRO_3 = 2,
        TALK_OUTRO_4 = 3,

        NPC_TRADE_PRINCE_GALLYWIX = 39582
    };

    struct npc_thrall_qtfcAI : public ScriptedAI
    {
        npc_thrall_qtfcAI(Creature * c) : ScriptedAI(c) {}

        EventMap comsmeticEvents;
        EventMap combatEvents;

        void Reset()
        {
            combatEvents.Reset();

            if (Unit* target = me->SelectNearestTarget(10.0f))
                AttackStart(target);
        };

        void DoAction(const int32 actionId)
        {
            if (actionId == 1)
            {
                Talk(TALK_OUTRO_1);
                comsmeticEvents.ScheduleEvent(EVENT_TALK_OUTRO_2, 3 * IN_MILLISECONDS);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (attacker->GetTypeId() == TYPEID_UNIT && !attacker->isPet())
            {
                if (me->GetHealth() < me->GetMaxHealth() || me->GetHealth() <= damage)
                {
                    damage = 0;
                    me->getThreatManager().addThreat(attacker, 0.f);
                }
            }
            else
            {
                if (Unit* victim = me->GetVictim())
                {
                    if (victim->GetTypeId() == TYPEID_UNIT)
                    {
                        me->getThreatManager().resetAllAggro();
                        me->getThreatManager().addThreat(attacker, std::numeric_limits<float>::max());
                        AttackStart(attacker);
                    }
                }
            }
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_CALL_A_METTING)
            {
                combatEvents.ScheduleEvent(EVENT_FORCE_OF_NATURE, urand(20, 25) * IN_MILLISECONDS);
                combatEvents.ScheduleEvent(EVENT_THUNDER, urand(10, 15) * IN_MILLISECONDS);
                combatEvents.ScheduleEvent(EVENT_LAVA_BUTST, urand(2, 3) * IN_MILLISECONDS);
                combatEvents.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(2, 3) * IN_MILLISECONDS);
            }
        }

        void UpdateAI(uint32 const diff)
        {

            if (uint32 eventId = comsmeticEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_TALK_OUTRO_2:
                        Talk(TALK_OUTRO_2);
                        comsmeticEvents.ScheduleEvent(EVENT_TALK_OUTRO_3, 5 * IN_MILLISECONDS);
                        break;
                    case EVENT_TALK_OUTRO_3:
                        Talk(TALK_OUTRO_3);
                        comsmeticEvents.ScheduleEvent(EVENT_TALK_OUTRO_4, 8 * IN_MILLISECONDS);
                        break;
                    case EVENT_TALK_OUTRO_4:
                        Talk(TALK_OUTRO_4);
                        comsmeticEvents.ScheduleEvent(EVENT_DUMMY_TO_PRINCE, 8 * IN_MILLISECONDS);
                        break;
                    case EVENT_DUMMY_TO_PRINCE:
                        if (Creature* prince = me->FindNearestCreature(NPC_TRADE_PRINCE_GALLYWIX, 20.0f))
                            if (prince->IsAIEnabled)
                                prince->AI()->DoAction(1);
                            break;

                }
            }

            comsmeticEvents.Update(diff);
            combatEvents.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = combatEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FORCE_OF_NATURE:
                        me->CastSpell((Unit*)NULL, SPELL_FORCE_OF_NATURE, false);
                        combatEvents.ScheduleEvent(EVENT_FORCE_OF_NATURE, urand(20, 25) * IN_MILLISECONDS);
                        break;
                    case EVENT_THUNDER:
                        me->CastSpell((Unit*)NULL, SPELL_THUNDER, false);
                        combatEvents.ScheduleEvent(EVENT_THUNDER, urand(10, 15) * IN_MILLISECONDS);
                        break;
                    case EVENT_LAVA_BUTST:
                        me->CastSpell((Unit*)NULL, SPELL_LAVA_BURST, false);
                        combatEvents.ScheduleEvent(EVENT_LAVA_BUTST, urand(2, 3) * IN_MILLISECONDS);
                        break;
                    case EVENT_CHAIN_LIGHTNING:
                        me->CastSpell((Unit*)NULL, SPELL_CHAIN_LIGHTNING, false);
                        combatEvents.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(2, 3) * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_wirldwind_qfc : public CreatureScript
{
public:
    npc_wirldwind_qfc() : CreatureScript("npc_wirldwind_qfc") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_wirldwind_qfcAI(creature);
    }

    enum eEvents
    {
        EVENT_RANDOM_MOVEMENT = 1
    };

    struct npc_wirldwind_qfcAI : public ScriptedAI
    {
        npc_wirldwind_qfcAI(Creature * creature) : ScriptedAI(creature) {}

        EventMap events;

        void InitializeAI()
        {
            events.ScheduleEvent(EVENT_RANDOM_MOVEMENT, 1 * IN_MILLISECONDS);

            Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_RANDOM_MOVEMENT)
            {
                me->GetMotionMaster()->MoveRandom(12.0f);
                events.ScheduleEvent(EVENT_RANDOM_MOVEMENT, urand(4, 6) * IN_MILLISECONDS);
            }
        }
    };
};

class npc_ultimate_footbomb_uniform_qfc : public CreatureScript
{
public:
    npc_ultimate_footbomb_uniform_qfc() : CreatureScript("npc_ultimate_footbomb_uniform_qfc") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ultimate_footbomb_uniform_qfcAI(creature);
    }

    enum eSpells
    {
        SPELL_EJECT_PASSENGERS = 68576,
        SPEL_EXIT              = 74017
    };

    struct npc_ultimate_footbomb_uniform_qfcAI : public VehicleAI
    {
        npc_ultimate_footbomb_uniform_qfcAI(Creature* creature) : VehicleAI(creature) {}

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (me->GetHealth() <= damage)
            {
                damage = 0;
                me->CastSpell(me, SPELL_EJECT_PASSENGERS, TRIGGERED_FULL_MASK);
                if (me->ToTempSummon())
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    summoner->CastSpell(summoner, SPEL_EXIT, false);

                attacker->Kill(me);
            }
        }
    };
};

class npc_brute_overseer_lost_isles : public CreatureScript
{
public:
    npc_brute_overseer_lost_isles() : CreatureScript("npc_brute_overseer_lost_isles") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_brute_overseer_lost_islesAI(creature);
    }

    enum eTalks
    {
        TALK_AGGRO = 0
    };

    struct npc_brute_overseer_lost_islesAI : public ScriptedAI
    {
        npc_brute_overseer_lost_islesAI(Creature* creature) : ScriptedAI(creature) {}

        void EnterCombat(Unit* who)
        {
            Talk(TALK_AGGRO);
        }
    };
};

const float VehicleProtectionPositions[7][4] =
{
    { 1171.520f, 1095.987f, 119.364f, 3.896f },
    { 868.186f, 2744.018f, 122.165f, 5.787f }
};

class spell_vehicle_protection_lost_isles : public SpellScriptLoader
{
public:
    spell_vehicle_protection_lost_isles() : SpellScriptLoader("spell_vehicle_protection_lost_isles") { }

    enum eSpells
    {
        SPELL_SASSYS_FLYING_MACHINE = 73137,
        SPELL_SLING_ROCKET          = 68805
    };

    class spell_vehicle_protection_lost_isles_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_vehicle_protection_lost_isles_AuraScript);

        void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 2.5 * IN_MILLISECONDS;
        }

        void HandleDummyTick(AuraEffect const* aurEff)
        {
            Player* owner = GetOwner()->ToPlayer();

            if (!owner)
                return;

            if (!owner->GetVehicle())
            {
                switch (aurEff->GetSpellInfo()->Id)
                {
                    case SPELL_SASSYS_FLYING_MACHINE:
                        owner->NearTeleportTo(VehicleProtectionPositions[0][0], VehicleProtectionPositions[0][1], VehicleProtectionPositions[0][2], VehicleProtectionPositions[0][3]);
                        break;
                    case SPELL_SLING_ROCKET:
                        owner->NearTeleportTo(VehicleProtectionPositions[1][0], VehicleProtectionPositions[1][1], VehicleProtectionPositions[1][2], VehicleProtectionPositions[1][3]);
                        break;
                }

                owner->RemoveAura(aurEff->GetSpellInfo()->Id);
            }
        }

        void Register()
        {
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_vehicle_protection_lost_isles_AuraScript::CalcPeriodic, EFFECT_1, SPELL_AURA_DUMMY);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_vehicle_protection_lost_isles_AuraScript::HandleDummyTick, EFFECT_1, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_vehicle_protection_lost_isles_AuraScript();
    }
};

class spell_morale_boost_controllers_lost_isles : public SpellScriptLoader
{
public:
    spell_morale_boost_controllers_lost_isles() : SpellScriptLoader("spell_morale_boost_controllers_lost_isles") { }

    enum eSpells
    {
        SPELL_ACE_CONTROLLER       = 73633,
        SPELL_IZZY_CONTROLLER      = 73635,
        SPELL_GOBBER_CONTROLLER    = 73637,
        SPELL_RESUMMON_ACE         = 73601,
        SPELL_RESUMMON_IZZY        = 73609,
        SPELL_RESUMMON_GOBBER      = 73611,

        NPC_ACE                    = 39198,
        NPC_IZZY                   = 39200,
        NPC_GOBBER                 = 39201,

        QUEST_MORALE_BOOST         = 25122,
        QUEST_OBJECTIVE_MIND_FREED = 264649,
    };

    class spell_morale_boost_controllers_lost_isles_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_morale_boost_controllers_lost_isles_AuraScript);

        void HandleDummyTick(AuraEffect const* aurEff)
        {
            Player* owner = GetOwner()->ToPlayer();

            if (!owner)
                return;

            Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_MORALE_BOOST);
            if (!quest)
                return;

            QuestStatusMap::const_iterator itr = owner->getQuestStatusMap().find(QUEST_MORALE_BOOST);

            if (owner->IsAlive() && !owner->GetVehicle())
            {
                switch (aurEff->GetSpellInfo()->Id)
                {
                    case SPELL_ACE_CONTROLLER:
                        if (owner->GetQuestObjectiveCounter(QUEST_OBJECTIVE_MIND_FREED) > 0)
                        {
                            if (!owner->FindNearestCreature(NPC_ACE, 20.0f))
                                owner->CastSpell(owner, SPELL_RESUMMON_ACE, false);
                        }
                        break;
                    case SPELL_IZZY_CONTROLLER:
                        if (owner->GetQuestObjectiveCounter(QUEST_OBJECTIVE_MIND_FREED + 1) > 0)
                        {
                            if (!owner->FindNearestCreature(NPC_IZZY, 20.0f))
                                owner->CastSpell(owner, SPELL_RESUMMON_IZZY, false);
                        }
                        break;
                    case SPELL_GOBBER_CONTROLLER:
                        if (owner->GetQuestObjectiveCounter(QUEST_OBJECTIVE_MIND_FREED + 2) > 0)
                        {
                            if (!owner->FindNearestCreature(NPC_GOBBER, 20.0f))
                                owner->CastSpell(owner, SPELL_RESUMMON_GOBBER, false);
                        }
                        break;
                }
            }

        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_morale_boost_controllers_lost_isles_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_morale_boost_controllers_lost_isles_AuraScript();
    }
};

class spell_morale_boost_assistant_controller_lost_isles : public SpellScriptLoader
{
public:
    spell_morale_boost_assistant_controller_lost_isles() : SpellScriptLoader("spell_morale_boost_assistant_controller_lost_isles") { }

    enum eNpc
    {
        NPC_ASSISTANT = 39199,
        QUEST_KAJACOLA_GIVES_YOU_IDEAS = 25110,
        SPELL_RESUMMON_ASSISTANT = 73603
    };

    class spell_morale_boost_assistant_controller_lost_isles_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_morale_boost_assistant_controller_lost_isles_AuraScript);

        void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 1 * IN_MILLISECONDS;
        }

        void HandleDummyTick(AuraEffect const* aurEff)
        {
            Player* owner = GetOwner()->ToPlayer();

            if (!owner)
                return;

            if (owner->IsAlive() && !owner->GetVehicle())
            {
                if (owner->GetQuestStatus(QUEST_KAJACOLA_GIVES_YOU_IDEAS) == QUEST_STATUS_REWARDED)
                if (!owner->FindNearestCreature(NPC_ASSISTANT, 20.0f))
                    owner->CastSpell(owner, SPELL_RESUMMON_ASSISTANT, false);
            }
        }

        void Register()
        {
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_morale_boost_assistant_controller_lost_isles_AuraScript::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_morale_boost_assistant_controller_lost_isles_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_morale_boost_assistant_controller_lost_isles_AuraScript();
    }
};

class npc_korkron_loyalist_lost_isles : public CreatureScript
{
public:
    npc_korkron_loyalist_lost_isles() : CreatureScript("npc_korkron_loyalist_lost_isles") {}

    enum eSpells
    {
        SPELL_MESSAGE_FOR_GARROSH_ACCEPT = 74032,
        QUEST_MESSAGE_FOR_GARROSH = 25267
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_MESSAGE_FOR_GARROSH)
            player->CastSpell(player, SPELL_MESSAGE_FOR_GARROSH_ACCEPT, false);

        return true;
    }
};

const float BattleWorgWPPath[61][3] =
{
    { 1446.568f, -5014.406f, 12.36172f },
    { 1439.736f, -5008.378f, 11.90149f },
    { 1438.108f, -4994.7f,   12.00306f },
    { 1436.516f, -4980.588f, 11.90683f },
    { 1435.057f, -4967.075f, 11.80224f },
    { 1433.519f, -4952.776f, 11.89868f },
    { 1432.075f, -4939.505f, 11.79961f },
    { 1430.609f, -4925.42f,  11.90367f },
    { 1429.318f, -4912.224f, 12.01049f },
    { 1427.177f, -4889.809f, 11.18419f },
    { 1421.97f,  -4866.588f, 11.86971f },
    { 1416.217f, -4847.741f, 14.71341f },
    { 1411.413f, -4832.224f, 17.69422f },
    { 1408.174f, -4817.137f, 20.75438f },
    { 1399.823f, -4791.056f, 23.67606f },
    { 1384.608f, -4767.722f, 26.07849f },
    { 1374.984f, -4752.335f, 27.67059f },
    { 1369.38f,  -4733.338f, 28.19211f },
    { 1373.592f, -4716.95f,  28.3638f  },
    { 1375.858f, -4699.997f, 28.54995f },
    { 1373.189f, -4684.049f, 28.16743f },
    { 1366.358f, -4666.299f, 27.099f   },
    { 1356.104f, -4648.854f, 25.93214f },
    { 1347.635f, -4633.269f, 24.90203f },
    { 1333.212f, -4612.353f, 24.03452f },
    { 1323.389f, -4599.549f, 23.90697f },
    { 1314.95f,  -4584.443f, 23.6859f  },
    { 1309.24f, - 4566.646f, 23.19465f },
    { 1308.307f, -4550.837f, 22.74647f },
    { 1307.438f, -4533.374f, 22.29239f },
    { 1308.837f, -4515.934f, 22.36367f },
    { 1310.814f, -4500.545f, 22.99549f },
    { 1316.056f,  -4483.51f, 23.76181f },
    { 1318.637f, -4466.776f, 24.49378f },
    { 1317.349f, -4451.759f, 24.95152f },
    { 1313.34f,  -4433.151f, 24.67413f },
    { 1312.212f, -4417.066f, 24.61878f },
    { 1313.345f, -4395.264f, 25.57962f },
    { 1321.396f, -4383.863f, 26.21949f },
    { 1333.224f, -4382.478f, 26.21098f },
    { 1349.036f, -4378.384f, 26.14815f },
    { 1366.66f,   -4374.42f, 26.07024f },
    { 1377.642f, -4371.816f, 26.02369f },
    { 1390.601f, -4369.455f, 25.37086f },
    { 1405.129f,  -4367.361f, 25.4679f },
    { 1422.615f, -4365.811f, 25.57083f },
    { 1428.976f, -4370.463f, 25.57083f },
    { 1431.144f,  -4383.75f, 25.57082f },
    { 1432.858f, -4401.179f, 25.57082f },
    { 1436.486f, -4417.405f, 25.57082f },
    { 1443.717f, -4422.469f, 25.57082f },
    { 1457.017f, -4420.729f, 25.57082f },
    { 1469.524f,   -4419.5f, 25.5719f  },
    { 1485.608f, -4417.561f, 25.33996f },
    { 1501.129f, -4415.694f, 23.43027f },
    { 1514.186f, -4412.816f, 22.02342f },
    { 1527.743f, -4408.971f, 19.41136f },
    { 1542.665f, -4404.818f, 18.3042f  },
    { 1558.083f, -4402.741f, 17.23785f },
    { 1573.519f, -4395.463f, 15.97107f },
    { 1586.547f, -4387.568f, 18.18813f }
};

class npc_battleworg_qmtg : public CreatureScript
{
public:
    npc_battleworg_qmtg() : CreatureScript("npc_battleworg_qmtg") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new nnpc_battleworg_qmtgAI(creature);
    }

    enum
    {
        EVENT_DONE = 1,
        SPELL_EJECT_PASSENGERS = 68576
    };

    struct nnpc_battleworg_qmtgAI : public VehicleAI
    {
        nnpc_battleworg_qmtgAI(Creature* creature) : VehicleAI(creature) {}

        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            Movement::MoveSplineInit init(me);
            for (uint8 i = 0; i < 61; ++i)
            {
                G3D::Vector3 path(BattleWorgWPPath[i][0], BattleWorgWPPath[i][1], BattleWorgWPPath[i][2]);
                init.Path().push_back(path);
            }
            init.SetSmooth();
            init.SetUncompressed();
            init.SetVelocity(13.0f);
            init.Launch();

            events.ScheduleEvent(EVENT_DONE, me->GetSplineDuration());
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_DONE)
                me->CastSpell(me, SPELL_EJECT_PASSENGERS, TRIGGERED_FULL_MASK);
        }
    };
};

class npc_chip_endale_qtfwthh : public CreatureScript
{
public:
    npc_chip_endale_qtfwthh() : CreatureScript("npc_chip_endale_qtfwthh") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_chip_endale_qtfwthhAI(creature);
    }

    enum eSpells
    {
        SPELL_FULL_MONTE   = 75968,
        SPELL_HEART_BROKEN = 62013,

        TALK_AGGRO          = 0
    };

    struct npc_chip_endale_qtfwthhAI : public ScriptedAI
    {
        npc_chip_endale_qtfwthhAI(Creature * c) : ScriptedAI(c) {}

        bool fullMonte;

        void Reset()
        {
            fullMonte = false;
        };

        void EnterCombat(Unit* who)
        {
            Talk(TALK_AGGRO, who->GetGUID());
            me->CastSpell(me, SPELL_HEART_BROKEN, false);
        }

        void DamageTaken(Unit* killer, uint32 &damage)
        {
            if (me->HealthBelowPctDamaged(20, damage))
            if (!fullMonte)
            {
                fullMonte = true;
                me->CastSpell((Unit*)NULL, SPELL_FULL_MONTE, false);
            }
        }
    };
};

void AddSC_lost_isles()
{
    // Creatures fist island
    new npc_pterrordax_scavenger();
    new npc_bomb_throwing_monkey();
    new npc_evol_fingers_beach();
    new npc_bamm_megabomb_beach();
    new npc_prince_gallywix_beach();
    new npc_maxx_avalanche_beach();
    new npc_coach_crosschek_beach();
    new npc_trade_prince_gallywix_qgep();
    new npc_captured_goblin();
    new npc_geargrinder_gizmo_qgep();
    new npc_sassy_hardwrench_qgosb();
    new npc_frightened_miner_qmt();
    new npc_foreman_dampwick_qmt();
    new npc_pygmy_witchdoctor_qmt();
    new npc_kilag_gorefang_qww();
    new npc_orc_scout_qww();
    new npc_poison_spitter_qww();
    new npc_ww_channel_bunny();
    new npc_stangle_vine_qww();
    new npc_aggra_qbta();
    new npc_kilag_gorefang_qii();
    new npc_orc_scout_qii();
    new npc_si_assasin_qii();
    new npc_bastia_qttc();
    new npc_generic_qgttg();
    new npc_gyrochoppa_questgiver_qpc();
    new npc_gyrochoppa_vehicle_qpc();
    new npc_arcane_wizard_qpc();
    new npc_thrall_qmmut();
    new npc_thrall_qwr();
    new npc_cyclone_of_elements_qwr();
    new npc_inisible_stalker_qwr();
    new npc_sassy_hardwrench_quua();
    new npc_trade_prince_gallywix_quua();
    new npc_sling_rocket_quua();
    new npc_foreman_dampwick_qitib();
    new npc_maxx_avalanche_wild_overlook();
    new npc_evol_fingers_wild_overlook();
    new npc_gyrochoppa_pilot_qgttg();
    new npc_terraptor_matriarch_lost_isles();

    // Creatures second island
    new npc_evol_fingers_town_in_one_box();
    new npc_maxx_avalanche_town_in_one_box();
    new npc_foreman_dampwick_qhny();
    new npc_hobart_grapplehammer_qcc();
    new npc_wild_clucker_qcc();
    new npc_bamm_megabomb_qtu();
    new npc_wild_clucker_egg_qtu();
    new npc_spiny_raptor_qtu();
    new npc_ravenous_lurker_lost_isles();
    new npc_the_hammer_lost_isles();
    new npc_megachicken_qbee();
    new npc_mechashark_xteam_qgisc();
    new npc_assistant_greely_qgisc();
    new npc_megs_dreadshredder_lost_isles();
    new npc_brett_mcquid_lost_isles();
    new npc_naga_hatchling_qiip();
    new npc_naga_hatching_summon_qiip();
    new npc_ace_qsoe();
    new npc_faceless_of_darkness_qsoe();
    new npc_vashjelan_siren_lost_isles();
    new npc_izzy_lost_isles();
    new npc_oomlot_warrior_lost_isles();
    new npc_sassy_hardwrench_lost_isles();
    new npc_town_in_one_box_generic_ai_lost_isles();
    new npc_warrior_matic_nx_lost_isles();
    new npc_oomlot_shaman_lost_isles();
    new npc_goblin_captive_lost_isles();
    new npc_goblin_zombie_qzvsbb();
    new npc_gaahl_qzvsbb();
    new npc_malmo_qzvsbb();
    new npc_teloch_qzvsbb();
    new npc_volcanoth_champion_lost_isles();
    new npc_volcanoth_lost_isles();
    new npc_volcanoth_dead_explosion_bunny_lost_isles();
    new npc_sassy_hardwrench_qvolcanoth_lost_isles();
    new npc_flying_bomber_qof();
    new npc_gnomeregan_stealth_flighter_lost_isles();
    new npc_alliance_paratrooper_lost_isles();
    new npc_aggra_lost_isles_wl();
    new npc_pride_of_kezan_qtpok();
    new npc_kilag_gorefang_qlr();
    new npc_bastia_qlr();
	new npc_slinky_sharpshiv_lost_isles();
    new npc_assistant_greely_summoned_lost_isles();
    new npc_quest_morale_boost_generic_ai_lost_isles();
    new npc_quest_morale_boost_summoned_creatures_generic_ai_lost_isles();
    new npc_blastshadow_the_brutemaster_lost_isles();
    new npc_mine_cart_qwmcr();
    new npc_mine_cart_questgiver_qwmcr();
    new npc_steamwheedle_shark_qss();
    new npc_good_bye_sweet_oil_invis_controller();
    new npc_explosion_bunny_qgbso();
    new npc_footbomb_uniform_qtsp();
    new npc_goblin_captured_qev();
    new npc_candy_cane_qwkinca();
    new npc_gallywix_docks_fighter_generic_ai();
    new npc_trade_prince_gallywix_qtfc();
    new npc_wirldwind_qfc();
    new npc_thrall_qtfc();
    new npc_ultimate_footbomb_uniform_qfc();
    new npc_brute_overseer_lost_isles();
    new npc_battleworg_qmtg();
    new npc_korkron_loyalist_lost_isles();
    new npc_chip_endale_qtfwthh();

    // Gameobjects fist island
    new go_goblin_escape_pod();
    new go_rocket_sling_quua();

    // Gameobject second island
    new go_mechashark_xsteam_controller_qgisc();
    new go_naga_banner_qbcr();
    new go_big_red_button_qgbso();

    // Spells fist island
    new spell_summon_dead_goblin_survivor();
    new spell_thermohydratic_flippers_periodic();
    new spell_exploding_bananas_qmb();
    new spell_ktc_snapflash_qctu();
    new spell_ktc_snapflash_effect_qctu();
    new spell_weed_whacker_qww();
    new spell_cyclone_of_elements_qwr_periodic();
    new spell_phase_05_lost_isles();
    new spell_despawn_orc_scout_qii();

    // Spells second island
    new spell_mechashark_water_controller_qgisc();
    new spell_despawn_nagas_qipp();
    new spell_super_booster_rocket_boots_qzvsbrb();
    new spell_super_booster_rocket_boots_filter_qzvsbrb();
    new spell_volcanoth_kill_credit_qvolcanoth();
    new spell_bootzooka_visual_filter_qvolcanoth();
    new spell_bootzooka_qvolcanoth();
    new spell_fire_jet_periodic_qvolcanoth();
    new spell_quest_old_friends_accept();
    new spell_despawn_flying_bomber_qof();
    new spell_leaving_area_warring_qtpok();
    new spell_on_exit_qwmcr();
    new spell_abandon_quest_check_qwmcr();
    new spell_vehicle_protection_lost_isles();
    new spell_morale_boost_controllers_lost_isles();
    new spell_morale_boost_assistant_controller_lost_isles();
}
