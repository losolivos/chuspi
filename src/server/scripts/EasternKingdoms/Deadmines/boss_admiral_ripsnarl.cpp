#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "deadmines.h"

enum eSpells
{
    SPELL_RIPSNARL_CANNON_KILL             = 95408,
    SPELL_EMOTE_TALK                       = 79506,
    SPELL_SWIPE                            = 88839,
    SPELL_FOG                              = 88768,
    SPELL_FOG_VISUAL                       = 88755,
    SPELL_RIPSNARLS_FOG_AURA               = 89247,
    SPELL_VANISH                           = 88840,
    SPELL_SUMMON_VAPOR_TARGETING           = 88833,
    SPELL_GO_FOR_THE_THROAT_TARGETING      = 88838,
    SPELL_STEAM_AURA                       = 95503,
    SPELL_CONDENSATION                     = 92013,
    SPELL_CONDENSE_TYPE_1                  = 92017,
    SPELL_CONDENSE_TYPE_2                  = 92021,
    SPELL_CONDENSE_TYPE_3                  = 92038,
    SPELL_SWIRLING_VAPOR                   = 92007,
    SPELL_CONDENSING_VAPOR                 = 92008,
    SPELL_FREEZING_VAPOR                   = 92011,
    SPELL_COALESCE                         = 92042,
};

enum eCreatures
{
    NPC_GPB_JMF_LOOK_2                     = 47242,
    NPC_GPB_JMF                            = 45979,
    NPC_VAPOR                              = 47714,
};

enum eScriptTexts
{
    ADMIRAL_RIPSNARL_YELL_START            = 0,
    ADMIRAL_RIPSNARL_YELL_DIED             = 1,
    ADMIRAL_RIPSNARL_YELL_KILL_PLAYER      = 2,
    ADMIRAL_RIPSNARL_YELL_VAPOR_P1         = 3,
    ADMIRAL_RIPSNARL_YELL_VAPOR_P2         = 4,
    ADMIRAL_RIPSNARL_YELL_VAPOR_EVENT_2    = 5,
    ADMIRAL_RIPSNARL_YELL_VAPOR_EVENT_3    = 6,
    ADMIRAL_RIPSNARL_EMOTE_ENRAGE          = 7,
};

enum eEvents
{
    // Ripsnarl
    EVENT_SUMMON_VAPOR_IN_INVIS            = 1,
    EVENT_SUMMON_VAPOR_IN_COMBAT           = 2,
    EVENT_SWIPE                            = 3,
    EVENT_FIRST_VAPOR                      = 4,
    EVENT_VAPOR_CAST_VAPOR                 = 5,
    EVENT_EXIT_INVIS                       = 6,
    EVENT_GO_FOR_THE_THROAT                = 7,
    EVENT_SECOND_VAPOR                     = 8,
    EVENT_THIRD_VAPOR                      = 9,
    // Vapor
    EVENT_SWIRLING_VAPOR                   = 1,
    EVENT_CONDENSING_VAPOR                 = 2,
    EVENT_FREEZING_VAPOR                   = 3,
    EVENT_VAPOR_ANIMUS                     = 4,
};

const Position MainVaporPos = {-66.8785f, -820.351f, 40.9776f, 0.0f};

const Position AllVaporPos[39]=
{
    {-51.88f, -833.49f, 42.46043f, 0.0f},{-57.38f, -822.72f, 41.91503f, 0.0f},{-47.43f, -831.12f, 42.87633f, 0.0f},
    {-51.82f, -829.84f, 42.45503f, 0.0f},{-58.03f, -812.93f, 41.83743f, 0.0f},{-73.64f, -814.60f, 40.40203f, 0.0f},
    {-56.41f, -827.82f, 42.01633f, 0.0f},{-61.98f, -829.88f, 41.50033f, 0.0f},{-73.97f, -819.94f, 40.35883f, 0.0f},
    {-53.28f, -810.45f, 42.28443f, 0.0f},{-56.95f, -832.98f, 41.97313f, 0.0f},{-65.40f, -833.35f, 41.20173f, 0.0f},
    {-60.97f, -835.97f, 41.59823f, 0.0f},{-68.93f, -829.36f, 40.87723f, 0.0f},{-72.99f, -825.99f, 40.48343f, 0.0f},
    {-56.90f, -818.26f, 41.95403f, 0.0f},{-62.65f, -823.65f, 41.40153f, 0.0f},{-68.13f, -822.91f, 40.88803f, 0.0f},
    {-67.70f, -814.16f, 40.93630f, 0.0f},{-62.69f, -814.03f, 41.38433f, 0.0f},{-62.65f, -823.65f, 41.40153f, 0.0f},
    {-70.94f, -835.52f, 40.68593f, 0.0f},{-79.24f, -834.88f, 39.97583f, 0.0f},{-88.10f, -831.63f, 39.25593f, 0.0f},
    {-95.36f, -828.31f, 38.68673f, 0.0f},{-99.68f, -818.97f, 38.35423f, 0.0f},{-101.1f, -824.67f, 38.25353f, 0.0f},
    {-104.3f, -819.27f, 38.02033f, 0.0f},{-101.1f, -813.93f, 38.24383f, 0.0f},{-94.22f, -809.92f, 38.74693f, 0.0f},
    {-85.66f, -808.54f, 39.42283f, 0.0f},{-79.32f, -806.66f, 39.93003f, 0.0f},{-72.87f, -808.34f, 40.48073f, 0.0f},
    {-66.66f, -805.14f, 41.03343f, 0.0f},{-60.00f, -807.57f, 41.64553f, 0.0f},{-55.04f, -805.92f, 42.10873f, 0.0f},
    {-47.61f, -808.85f, 42.82733f, 0.0f},{-82.56f, -830.49f, 39.70443f, 0.0f},{-75.52f, -831.81f, 40.27493f, 0.0f},
};

class boss_admiral_ripsnarl : public CreatureScript
{
public:
    boss_admiral_ripsnarl() : CreatureScript("boss_admiral_ripsnarl") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_admiral_ripsnarlAI (creature);
    }

    struct boss_admiral_ripsnarlAI : public BossAI
    {
        boss_admiral_ripsnarlAI(Creature* creature) : BossAI(creature, DATA_ADMIRA_RIPSNARL) { }

        uint64 casterVaporGUID;
        uint8 currentHealthPct;
        bool canCheckHP;

        void Reset()
        {
            _Reset();
            me->CastSpell((Unit*)NULL, 95648, false);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->SetReactState(REACT_AGGRESSIVE);
            casterVaporGUID = 0;
            currentHealthPct = 75;
            canCheckHP = true;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(ADMIRAL_RIPSNARL_YELL_START);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            me->CastSpell((Unit*)NULL, 95648, false);
            me->CastSpell(me, SPELL_RIPSNARL_CANNON_KILL, false);
            events.ScheduleEvent(EVENT_SWIPE, urand(3000, 7000));

            if (IsHeroic())
                events.ScheduleEvent(EVENT_GO_FOR_THE_THROAT, urand(5000, 25000));
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
        {
            if (canCheckHP && me->GetHealthPct() <= currentHealthPct)
            {
                if (currentHealthPct != 10)
                {
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    events.Reset();
                }

                switch (currentHealthPct)
                {
                    case 75:
                        Talk(ADMIRAL_RIPSNARL_YELL_VAPOR_P1);
                        me->CastSpell(me, SPELL_EMOTE_TALK, false);
                        currentHealthPct = 50;
                        events.ScheduleEvent(EVENT_FIRST_VAPOR, 4000);
                        break;
                    case 50:
                        me->CastSpell(me, SPELL_VANISH, false);
                        events.ScheduleEvent(EVENT_SECOND_VAPOR, 12500);
                        events.ScheduleEvent(EVENT_VAPOR_CAST_VAPOR, 100);
                        events.ScheduleEvent(EVENT_SUMMON_VAPOR_IN_INVIS, 4500);
                        currentHealthPct = 25;
                        break;
                    case 25:
                        {
                            me->CastSpell(me, SPELL_VANISH, false);
                            events.ScheduleEvent(EVENT_THIRD_VAPOR, 12500);
                            events.ScheduleEvent(EVENT_VAPOR_CAST_VAPOR, 100);
                            events.ScheduleEvent(EVENT_SUMMON_VAPOR_IN_INVIS, 4500);

                            if (!IsHeroic())
                            {
                                canCheckHP = false;
                                currentHealthPct = 0;
                            }
                            else
                                currentHealthPct = 10;
                        }
                        break;
                    case 10:
                        {
                            Talk(ADMIRAL_RIPSNARL_EMOTE_ENRAGE);
                            currentHealthPct = 0;
                            canCheckHP = false;

                            for (int i = 0; i < 3; ++i)
                                me->CastSpell((Unit*)NULL, SPELL_SUMMON_VAPOR_TARGETING, false);
                        }
                        break;
                }
            }
        }

        void JustSummoned(Creature* summoned)
        {
            summons.Summon(summoned);

            if (me->IsInCombat())
                summoned->SetInCombatWithZone();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(ADMIRAL_RIPSNARL_YELL_KILL_PLAYER);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            me->CastSpell((Unit*)NULL, 95648, false);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            Talk(ADMIRAL_RIPSNARL_YELL_DIED);
        }

        void PlaySoundToZone(uint32 sound)
        {
            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();

            for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
               if (Player* player = itr->GetSource())
                   me->PlayDirectSound(sound, player);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SWIPE:
                        events.ScheduleEvent(EVENT_SWIPE, urand(3000, 7000));
                        me->CastSpell(me->GetVictim(), SPELL_SWIPE, false);
                        break;
                    case EVENT_FIRST_VAPOR:
                        {
                            Talk(ADMIRAL_RIPSNARL_YELL_VAPOR_P2);
                            me->CastSpell(me, SPELL_VANISH, false);

                            for (int i = 0; i < 39; ++i)
                                if (Creature* vapor = me->SummonCreature(NPC_GPB_JMF_LOOK_2, AllVaporPos[i]))
                                    vapor->CastSpell(vapor, SPELL_FOG, false);

                            if (Creature* vapor = me->SummonCreature(NPC_GPB_JMF, MainVaporPos))
                                vapor->CastSpell(vapor, SPELL_FOG_VISUAL, false);

                            if (Creature* vapor = me->SummonCreature(NPC_GPB_JMF, MainVaporPos))
                            {
                                casterVaporGUID = vapor->GetGUID();
                                events.ScheduleEvent(EVENT_VAPOR_CAST_VAPOR, 100);
                            }

                            events.ScheduleEvent(EVENT_SUMMON_VAPOR_IN_INVIS, 4500);
                            events.ScheduleEvent(EVENT_EXIT_INVIS, 25000);
                        }
                        break;
                    case EVENT_VAPOR_CAST_VAPOR:
                        {
                            if (Creature* vapor = Unit::GetCreature(*me, casterVaporGUID))
                                vapor->CastSpell(vapor, SPELL_RIPSNARLS_FOG_AURA, false);
                        }
                        break;
                    case EVENT_SUMMON_VAPOR_IN_INVIS:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            {
                                Position pos;
                                target->GetRandomNearPosition(pos, 15.0f);
                                me->SummonCreature(NPC_VAPOR, pos);
                            }

                            events.ScheduleEvent(EVENT_SUMMON_VAPOR_IN_INVIS, 4500);
                        }
                        break;
                    case EVENT_EXIT_INVIS:
                        me->RemoveAura(SPELL_VANISH);
                        me->CastSpell((Unit*)NULL, SPELL_SUMMON_VAPOR_TARGETING, false);
                        me->CastSpell((Unit*)NULL, SPELL_GO_FOR_THE_THROAT_TARGETING, false);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->PlayDirectSound(19881);
                        events.Reset();
                        events.ScheduleEvent(EVENT_SWIPE, urand(3000, 7000));
                        events.ScheduleEvent(EVENT_SUMMON_VAPOR_IN_COMBAT, 12000);
                        events.ScheduleEvent(EVENT_GO_FOR_THE_THROAT, urand(5000, 25000));
                        break;
                    case EVENT_SUMMON_VAPOR_IN_COMBAT:
                        me->CastSpell((Unit*)NULL, SPELL_SUMMON_VAPOR_TARGETING, false);
                        events.ScheduleEvent(EVENT_SUMMON_VAPOR_IN_COMBAT, 12000);
                        break;
                    case EVENT_GO_FOR_THE_THROAT:
                        me->CastSpell((Unit*)NULL, SPELL_GO_FOR_THE_THROAT_TARGETING, false);
                        events.ScheduleEvent(EVENT_GO_FOR_THE_THROAT, urand(5000, 25000));
                        break;
                    case EVENT_SECOND_VAPOR:
                        PlaySoundToZone(19879);
                        Talk(ADMIRAL_RIPSNARL_YELL_VAPOR_EVENT_2);
                        events.ScheduleEvent(EVENT_EXIT_INVIS, 12500);
                        break;
                    case EVENT_THIRD_VAPOR:
                        PlaySoundToZone(19880);
                        Talk(ADMIRAL_RIPSNARL_YELL_VAPOR_EVENT_3);
                        events.ScheduleEvent(EVENT_EXIT_INVIS, 12500);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_ripsnarl_vapor : public CreatureScript
{
public:
    npc_ripsnarl_vapor() : CreatureScript("npc_ripsnarl_vapor") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ripsnarl_vaporAI (creature);
    }

    struct npc_ripsnarl_vaporAI : public ScriptedAI
    {
        npc_ripsnarl_vaporAI(Creature* creature) : ScriptedAI(creature)
        {
            me->CastSpell(me, SPELL_STEAM_AURA, false);
        }

        EventMap events;

        void EnterCombat(Unit* /*who*/)
        {
            if (IsHeroic())
            {
                me->CastSpell(me, SPELL_CONDENSATION, false);
                events.ScheduleEvent(EVENT_CONDENSING_VAPOR, 5000);
            }
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
        {
            switch (spell->Id)
            {
                case SPELL_CONDENSE_TYPE_1:
                    events.ScheduleEvent(EVENT_SWIRLING_VAPOR, 4000);
                    break;
                case SPELL_CONDENSE_TYPE_2:
                    events.ScheduleEvent(EVENT_FREEZING_VAPOR, 3000);
                    break;
                case SPELL_CONDENSE_TYPE_3:
                    events.ScheduleEvent(EVENT_VAPOR_ANIMUS, urand(2000, 8000));
                    break;
                case SPELL_COALESCE:
                    me->CastSpell((Unit*)NULL, 95647, true);
                    break;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CONDENSING_VAPOR:
                        me->CastSpell(me->GetVictim(), SPELL_SWIRLING_VAPOR, false);
                        break;
                    case EVENT_SWIRLING_VAPOR:
                        me->CastSpell(me->GetVictim(), SPELL_CONDENSING_VAPOR, false);
                        break;
                    case EVENT_FREEZING_VAPOR:
                        me->CastSpell(me->GetVictim(), SPELL_FREEZING_VAPOR, false);
                        break;
                    case EVENT_VAPOR_ANIMUS:
                        me->CastSpell((Unit*)NULL, SPELL_COALESCE, false);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class spell_admiral_ripsnarl_targeting : public SpellScriptLoader
{
    public:
        spell_admiral_ripsnarl_targeting() : SpellScriptLoader("spell_admiral_ripsnarl_targeting") { }

        class spell_admiral_ripsnarl_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_admiral_ripsnarl_targeting_SpellScript)

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
                OnEffectHitTarget += SpellEffectFn(spell_admiral_ripsnarl_targeting_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_admiral_ripsnarl_targeting_SpellScript();
        }
};

void AddSC_boss_admiral_ripsnarl()
{
    new boss_admiral_ripsnarl();
    new npc_ripsnarl_vapor();

    new spell_admiral_ripsnarl_targeting();
}
