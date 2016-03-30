#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Vehicle.h"
#include "deadmines.h"

enum eSpells
{
    SPELL_TFT_G_CORN                      = 89268,
    SPELL_TFT_R_CORN                      = 89740,
    SPELL_TFT_G_STEAK                     = 90562,
    SPELL_TFT_R_STEAK                     = 90583,
    SPELL_TFT_G_LOAF                      = 90564,
    SPELL_TFT_R_LOAF                      = 90585,
    SPELL_TFT_G_MELON                     = 90561,
    SPELL_TFT_R_MELON                     = 90582,
    SPELL_TFT_G_MM                        = 90563,
    SPELL_TFT_R_MM                        = 90584,
    SPELL_TFT_G_BUN                       = 90565,
    SPELL_TFT_R_BUN                       = 90586,
    SPELL_TFT_MURLOC                      = 90681,
    SPELL_WHOS_THAT                       = 89339,
    SPELL_COOKIES_CAULDRON_SPAWN_VISUAL   = 89251,
    SPELL_COOKIES_CAULDRON                = 89250,
    SPELL_RIDE_VEHICLE_HARDCODED          = 46598,
    SPELL_CAULDRON_FIRE                   = 89252,
    SPELL_BABY_MURLOC                     = 24983,
    SPELL_BABY_MURLOC_DANCE               = 24984,
};

enum eCreatures
{
    NPC_COOKIES_CAULDRON                  = 47754,
};

enum eScriptTexts
{
    COOKIE_EMOTE_EXIT_INVIS               = 0,
};

enum eEvents
{
    EVENT_SUMMON_CAULDRON                 = 1,
    EVENT_GOOD_FOOD                       = 2,
    EVENT_ROTTEN_FOOD                     = 3,
    EVENT_MURLOC                          = 4,
    EVENT_ENTER_CAULDRON                  = 5,
    EVENT_CAULDRON_FIRE                   = 6,
    EVENT_CHECK_VALID_TARGET              = 7,
    EVENT_CLEAR_INVIS                     = 8,
};

struct CCFood
{
    uint32 GoodFood;
    uint32 RottenFood;
};

const CCFood Food[6]=
{
    {SPELL_TFT_G_CORN,  SPELL_TFT_R_CORN },
    {SPELL_TFT_G_STEAK, SPELL_TFT_R_STEAK},
    {SPELL_TFT_G_LOAF,  SPELL_TFT_R_LOAF },
    {SPELL_TFT_G_MELON, SPELL_TFT_R_MELON},
    {SPELL_TFT_G_MM,    SPELL_TFT_R_MM   },
    {SPELL_TFT_G_BUN,   SPELL_TFT_R_BUN  },
};

#if 0
const Position CauldronSpawnPos = {-64.25521f, -820.2448f, 41.32077f, 0.0f};
#endif

const Position CauldronMovePos = {-67.2908f, -820.12f, 40.8532f, 0.0f};

class boss_captain_cookie : public CreatureScript
{
public:
    boss_captain_cookie() : CreatureScript("boss_captain_cookie") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_captain_cookieAI (creature);
    }

    struct boss_captain_cookieAI : public BossAI
    {
        boss_captain_cookieAI(Creature* creature) : BossAI(creature, DATA_CAPTAIN_COOKIE) { }

        EventMap intro_events;
        uint8 currendFoodId;
        bool active;
        bool canAttack;

        void InitializeAI()
        {
            SetCombatMovement(false);
            JustReachedHome();
            Reset();
        }

        void Reset()
        {
            _Reset();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            intro_events.Reset();
            currendFoodId = 0;
            canAttack = false;
        }

        void JustReachedHome()
        {
            if (instance->GetBossState(DATA_ADMIRA_RIPSNARL) != DONE)
            {
                active = false;
                me->SetVisible(false);
                me->SetReactState(REACT_PASSIVE);
            }
            else
            {
                active = true;
                me->SetReactState(REACT_AGGRESSIVE);
            }

            me->CastSpell(me, SPELL_WHOS_THAT, false);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (active)
                if (me->GetExactDist2d(who) <= 5.0f)
                {
                    active = false;
                    canAttack = true;
                    AttackStart(who);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                    me->RemoveAura(SPELL_WHOS_THAT);
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->GetMotionMaster()->MovePoint(0, CauldronMovePos);
                    events.ScheduleEvent(EVENT_SUMMON_CAULDRON, me->GetSplineDuration());
                }
        }

        void SetData(uint32 data, uint32 value)
        {
            if (data == DATA_ADMIRA_RIPSNARL && value == DONE)
                intro_events.ScheduleEvent(EVENT_CLEAR_INVIS, 7000);
        }

        void JustSummoned(Creature* summoned)
        {
            if (summoned->GetEntry() == NPC_COOKIES_CAULDRON)
                summoned->CastSpell(summoned, SPELL_COOKIES_CAULDRON_SPAWN_VISUAL, false);

            summons.Summon(summoned);
            summoned->SetInCombatWithZone();
        }

        void AttackStart(Unit* who)
        {
            if (canAttack)
                BossAI::AttackStart(who);
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            if (me->GetMap()->GetDifficulty() == HEROIC_DIFFICULTY)
                Talk(1);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                intro_events.Update(diff);

                if (intro_events.ExecuteEvent() == EVENT_CLEAR_INVIS)
                {
                    active = true;
                    me->SetVisible(true);
                    me->SetReactState(REACT_AGGRESSIVE);
                    Talk(COOKIE_EMOTE_EXIT_INVIS);
                }

                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SUMMON_CAULDRON:
                        me->CastSpell(me, SPELL_COOKIES_CAULDRON, false);
                        events.ScheduleEvent(EVENT_ENTER_CAULDRON, 1000);
                        break;
                    case EVENT_ENTER_CAULDRON:
                    {
                        if (Creature* cauldron = me->FindNearestCreature(NPC_COOKIES_CAULDRON, 15.0f))
                        {
                            me->CastSpell(cauldron, SPELL_RIDE_VEHICLE_HARDCODED, false);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        }

                        canAttack = true;
                        me->CastSpell(me, 95650, true);
                        me->SetReactState(REACT_AGGRESSIVE);
                        events.ScheduleEvent(EVENT_CAULDRON_FIRE, 3000);
                        events.ScheduleEvent(EVENT_CHECK_VALID_TARGET, 5000);
                        events.ScheduleEvent(EVENT_GOOD_FOOD, 2000);
                        break;
                    }
                    case EVENT_CAULDRON_FIRE:
                        me->AddAura(SPELL_CAULDRON_FIRE, me);
                        break;
                    case EVENT_GOOD_FOOD:
                        me->CastSpell((Unit*)NULL, Food[currendFoodId].GoodFood, false);
                        events.ScheduleEvent(EVENT_ROTTEN_FOOD, 1000);
                        break;
                    case EVENT_ROTTEN_FOOD:
                    {
                        me->CastSpell((Unit*)NULL, Food[currendFoodId].RottenFood, false);

                        if (currendFoodId < 5)
                        {
                            ++currendFoodId;
                            events.ScheduleEvent(EVENT_GOOD_FOOD, 1000);
                        }
                        else
                        {
                            currendFoodId = 0;
                            events.ScheduleEvent(EVENT_MURLOC, 1000);
                        }
                        break;
                    }
                    case EVENT_MURLOC:
                        me->CastSpell((Unit*)NULL, SPELL_TFT_MURLOC, false);
                        events.ScheduleEvent(EVENT_GOOD_FOOD, 3000);
                        break;
                    case EVENT_CHECK_VALID_TARGET:
                    {
                        bool Evade = true;
                        std::list<HostileReference*> const& tList = me->getThreatManager().getThreatList();

                        for (std::list<HostileReference*>::const_iterator itr = tList.begin(); itr != tList.end(); ++itr)
                            if (Unit* target = Unit::GetUnit((*me), (*itr)->getUnitGuid()))
                                if (me->IsWithinDistInMap(target, 30.0f) &&
                                    me->IsWithinLOSInMap(target) &&
                                    target->GetTypeId() == TYPEID_PLAYER)
                                {
                                    Evade = false;
                                    break;
                                }

                        if (Evade)
                        {
                            me->ExitVehicle();
                            BossAI::EnterEvadeMode();
                            return;
                        }

                        events.ScheduleEvent(EVENT_CHECK_VALID_TARGET, 1000);
                        break;
                    }
                }
            }
        }
    };
};

class npc_cookies_cauldron : public CreatureScript
{
public:
    npc_cookies_cauldron() : CreatureScript("npc_cookies_cauldron") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_cookies_cauldronAI (creature);
    }

    struct npc_cookies_cauldronAI : public ScriptedAI
    {
        npc_cookies_cauldronAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            if (Vehicle* vehicle = me->GetVehicleKit())
                if (Unit* passenger = vehicle->GetPassenger(0))
                    if (Creature* cookie = passenger->ToCreature())
                        cookie->ExitVehicle();

            me->SetReactState(REACT_PASSIVE);
        }
    };
};

class npc_baby_murloc : public CreatureScript
{
public:
    npc_baby_murloc() : CreatureScript("npc_baby_murloc") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_baby_murlocAI (creature);
    }

    struct npc_baby_murlocAI : public ScriptedAI
    {
        npc_baby_murlocAI(Creature* creature) : ScriptedAI(creature)
        {
            me->CastSpell(me, SPELL_BABY_MURLOC, false);
            me->GetMotionMaster()->MoveRandom(5.0f);

            if (Aura* aura = me->GetAura(SPELL_BABY_MURLOC))
                if (AuraEffect* effect = aura->GetEffect(EFFECT_0))
                    effect->SetPeriodicTimer(5000);
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_BABY_MURLOC_DANCE)
            {
                me->StopMoving();
                me->GetMotionMaster()->MoveIdle();
                me->DespawnOrUnsummon(15000);
                me->RemoveAura(SPELL_BABY_MURLOC);
            }
        }
    };
};

class npc_captain_cookie_food : public CreatureScript
{
public:
    npc_captain_cookie_food() : CreatureScript("npc_captain_cookie_food") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_captain_cookie_foodAI (creature);
    }

    struct npc_captain_cookie_foodAI : public ScriptedAI
    {
        npc_captain_cookie_foodAI(Creature* creature) : ScriptedAI(creature)
        {
            InstanceScript* instance = me->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_PASSIVE);

            if (instance)
            {
                if (instance->GetBossState(DATA_CAPTAIN_COOKIE) != IN_PROGRESS)
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (Creature* cookie = Unit::GetCreature(*me, instance->GetData64(DATA_CAPTAIN_COOKIE)))
                    if (cookie->IsAIEnabled)
                        cookie->AI()->JustSummoned(me);
            }
        }

        void OnSpellClick(Unit * /*player*/, bool& result)
        {
            if (!result)
                return;

            me->RemoveAllAuras();
            me->DespawnOrUnsummon(250);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }
    };
};

class spell_throw_food_targeting : public SpellScriptLoader
{
    public:
        spell_throw_food_targeting() : SpellScriptLoader("spell_throw_food_targeting") { }

        class spell_throw_food_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_throw_food_targeting_SpellScript)

            void TriggerSpell(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!(caster && target))
                    return;

                caster->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, false);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_throw_food_targeting_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_throw_food_targeting_SpellScript();
        }
};

class spell_satiated : public SpellScriptLoader
{
    public:
        spell_satiated() : SpellScriptLoader("spell_satiated") { }

        class spell_satiated_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_satiated_SpellScript)

            void TriggerSpell(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* target = GetHitUnit();

                if (!target)
                    return;

                target->RemoveAuraFromStack(GetSpellInfo()->Effects[effIndex].BasePoints);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_satiated_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_satiated_SpellScript();
        }
};

class spell_nauseated : public SpellScriptLoader
{
    public:
        spell_nauseated() : SpellScriptLoader("spell_nauseated") { }

        class spell_nauseated_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_nauseated_SpellScript)

            void CheckStackAmount(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                    if (Aura* aura = target->GetAura(GetSpellInfo()->Id))
                        if (aura->GetStackAmount() > 1)
                            target->RemoveAura(95650);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_nauseated_SpellScript::CheckStackAmount, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_nauseated_SpellScript();
        }
};

void AddSC_boss_captain_cookie()
{
    new boss_captain_cookie();
    new npc_cookies_cauldron();
    new npc_baby_murloc();
    new npc_captain_cookie_food();

    new spell_throw_food_targeting();
    new spell_satiated();
    new spell_nauseated();
}
