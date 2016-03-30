#include "the_stonecore.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum Spells
{
    SPELL_RUPTURE_PERIODIC      = 92393,
    SPELL_SPIKE_SHIELD          = 78835,
    SPELL_ENRAGE                = 80467,
    SPELL_SHATTER               = 78807,
    SPELL_GROUND_SLAM           = 78903,
    SPELL_BULWARK               = 78939,
    SPELL_PARALYZE              = 92426
};

enum Entities
{
    NPC_RUPTURE                 = 49576,
    NPC_RUPTURE_CONTROLLER      = 49597,
};

enum Events
{
    EVENT_BULWARK               = 1,
    EVENT_GROUND_SLAM,
    EVENT_GROUND_SLAM_END,
    EVENT_SHATTER,
    EVENT_PARALYZE,
    EVENT_SPIKE_SHIELD
};

enum Quotes
{
    SAY_AGGRO,
    SAY_DEATH,
    SAY_SLAY,
    SAY_SHIELD
};

class boss_ozruk : public CreatureScript
{
    struct boss_ozrukAI : public BossAI
    {
        boss_ozrukAI(Creature * creature) : BossAI(creature, DATA_OZRUK) { }

        void Reset()
        {
            enraged = false;
            _Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_BULWARK, 10000);
            events.ScheduleEvent(EVENT_GROUND_SLAM, 30000);
            events.ScheduleEvent(EVENT_SPIKE_SHIELD, 15000);
            if (!IsHeroic())
                events.ScheduleEvent(EVENT_SHATTER, 10000);
            _EnterCombat();
        }

        void JustSummoned(Creature * summon)
        {
            if (summon->GetEntry() == NPC_RUPTURE_CONTROLLER)
            {
                summon->CastSpell(summon, SPELL_RUPTURE_PERIODIC, false);
                summon->DespawnOrUnsummon(5000);
            }
        }

        void JustDied(Unit * /*killer*/)
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void KilledUnit(Unit * victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
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
                case EVENT_BULWARK:
                    DoCast(me, SPELL_BULWARK);
                    events.ScheduleEvent(EVENT_BULWARK, urand(30000, 35000));
                    break;
                case EVENT_GROUND_SLAM:
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveIdle();
                    me->SetTarget(0);
                    me->SetFacingToObject(me->GetVictim());
                    DoCast(me, SPELL_GROUND_SLAM);
                    events.ScheduleEvent(EVENT_GROUND_SLAM, 30000);
                    events.ScheduleEvent(EVENT_GROUND_SLAM_END, 3500);
                    break;
                case EVENT_GROUND_SLAM_END:
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (Unit * victim = me->GetVictim())
                    {
                        me->SetTarget(victim->GetGUID());
                        DoStartMovement(victim);
                    }
                    break;
                case EVENT_SHATTER:
                    DoCast(me, SPELL_SHATTER);
                    if (!IsHeroic())
                        events.ScheduleEvent(EVENT_SHATTER, urand(20000, 25000));
                    break;
                case EVENT_PARALYZE:
                    DoCast(me, SPELL_PARALYZE);
                    events.ScheduleEvent(EVENT_SHATTER, 3000);
                    break;
                case EVENT_SPIKE_SHIELD:
                    Talk(SAY_SHIELD);
                    DoCast(me, SPELL_SPIKE_SHIELD);
                    events.ScheduleEvent(EVENT_SPIKE_SHIELD, 45000);
                    if (IsHeroic())
                        events.ScheduleEvent(EVENT_PARALYZE, urand(7000, 8000));
                    break;
                }
            }

            if (!enraged && me->HealthBelowPct(30))
            {
                enraged = true;
                DoCast(me, SPELL_ENRAGE);
            }

            DoMeleeAttackIfReady();
        }
    private:
        bool enraged;
    };
public:
    boss_ozruk() : CreatureScript("boss_ozruk") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_ozrukAI(creature);
    }
};

class spell_rupture_periodic : public SpellScriptLoader
{
    enum
    {
        SPELL_RUPTURE_DAM           = 92381,
    };

    class spell_rupture_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rupture_periodic_AuraScript)

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            Unit * caster = GetCaster();
            if (!caster)
                return;

            Position pos;
            float dist = aurEff->GetTickNumber() * 5.0f;
            caster->GetNearPosition(pos, dist, 0.0f);

            for(int i=0; i<3; ++i)
            {
                if (i == 1)
                    caster->MovePosition(pos, 3.0f, M_PI/2);
                else if (i == 2)
                    caster->MovePosition(pos, 6.0f, -M_PI/2);

                if (Creature * creature = caster->SummonCreature(NPC_RUPTURE, pos, TEMPSUMMON_TIMED_DESPAWN, 1000))
                    creature->CastSpell(creature, SPELL_RUPTURE_DAM, false);
            }

        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_rupture_periodic_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

public:
    spell_rupture_periodic() : SpellScriptLoader("spell_rupture_periodic") { }

    AuraScript* GetAuraScript() const
    {
        return new spell_rupture_periodic_AuraScript();
    }
};

void AddSC_boss_ozruk()
{
    new boss_ozruk();
    new spell_rupture_periodic();
}
