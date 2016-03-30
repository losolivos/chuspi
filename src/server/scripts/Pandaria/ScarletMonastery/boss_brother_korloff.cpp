#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scarlet_monastery.h"

enum spells
{
    SPELL_BLAZING_FISTS = 114807,
    SPELL_FIRESTORM_KICK = 113764,
    SPELL_RISING_FLAME = 114410,
    SPELL_SCORCHED_EARTH = 114460,
    SPELL_FLYING_KICK = 114487,
    SPELL_FLYING_KICK_KNOCK_BACK = 110283,
    SPELL_TRIGGER_SCORCHED_EARTH = 114464
};

enum events
{
    EVENT_BLAZING_FISTS = 1,
    EVENT_FIRESTORM_KICK = 2,
    EVENT_FIRESTORM_START = 3
};

enum Yells
{
    TALK_AGGRO = 0,
    TALK_DEATH = 1,
    TALK_SLAY = 2,
    TALK_FISTS = 3
};

enum phases
{
    PHASE_ONE = 1,
    PHASE_TWO = 2
};

class boss_brother_korloff : public CreatureScript
{
public:
    boss_brother_korloff() : CreatureScript("boss_brother_korloff") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_brother_korloffAI(creature);
    }

    struct boss_brother_korloffAI : public BossAI
    {
        boss_brother_korloffAI(Creature* creature) : BossAI(creature, BOSS_BROTHER_KORLOFF) {}
        EventMap events;

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_AGGRESSIVE);
            if (instance)
            {
                instance->SetData(BOSS_BROTHER_KORLOFF, NOT_STARTED);
                instance->DoRemoveAurasDueToSpellOnPlayers(125852);
            }
            phase = PHASE_ONE;
            phase = 100.0f;
        }

        uint32 phase;
        float heal;

        void JustDied(Unit* killer) override
        {
            _JustDied();
            Talk(TALK_DEATH);
            if (instance)
            {
                instance->SetData(BOSS_BROTHER_KORLOFF, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->DoRemoveAurasDueToSpellOnPlayers(125852);
            }

        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_BROTHER_KORLOFF, FAIL);
            }
            summons.DespawnAll();
        }


        void EnterCombat(Unit* who) override
        {
            _EnterCombat();
            Talk(TALK_AGGRO);
            if (instance)
            {
                instance->SetData(BOSS_BROTHER_KORLOFF, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            heal = me->GetHealthPct();
            events.ScheduleEvent(EVENT_FIRESTORM_KICK, 6000);
            events.ScheduleEvent(EVENT_BLAZING_FISTS, 20500);
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

			if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
			
            if (me->GetHealthPct() <= 50 && phase!=PHASE_TWO)
            {
                phase = PHASE_TWO;
                me->CastSpell(me, SPELL_SCORCHED_EARTH, false);
                me->SummonCreature(59507, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), TEMPSUMMON_MANUAL_DESPAWN);
            }

            if (heal - me->GetHealthPct() >= 10)
            {
                heal = me->GetHealthPct();
                me->CastSpell(me, SPELL_RISING_FLAME,false);
            }

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BLAZING_FISTS:
                    me->CastSpell(me, SPELL_BLAZING_FISTS, false);
                    Talk(TALK_SLAY);
                    events.ScheduleEvent(EVENT_BLAZING_FISTS, urand(20000, 25000) + 6000);
                    break;
                case EVENT_FIRESTORM_KICK:
                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 0, true))
                    {
                        me->CastSpell(target, SPELL_FLYING_KICK_KNOCK_BACK, false);
                        me->GetMotionMaster()->MoveJump(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 50.0f, 30.f);
                        events.ScheduleEvent(EVENT_FIRESTORM_START, urand(1000, 1500));
                    }
                    break;
                case EVENT_FIRESTORM_START:
                    Talk(TALK_SLAY);
                    DoCastAOE(SPELL_FIRESTORM_KICK);
                    events.ScheduleEvent(EVENT_FIRESTORM_KICK, 16000);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};
// ###59507###
class trigger_scorched_flame : public CreatureScript
{
public:
    trigger_scorched_flame() : CreatureScript("trigger_scorched_flame")
    {
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new trigger_scorched_flameAI(creature);
    }

    struct trigger_scorched_flameAI : public ScriptedAI
    {
        trigger_scorched_flameAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) {}

        void JustDied(Unit* /*killer*/) {}

        void IsSummonedBy(Unit* summoner) { me->CastSpell(me, SPELL_TRIGGER_SCORCHED_EARTH, false); me->DespawnOrUnsummon(30000); trig = false; }
        bool trig;

        void UpdateAI(const uint32 diff)
        {

            events.Update(diff);

            if (Creature* pBoss = GetClosestCreatureWithEntry(me, BROTHER_KORLOFF, 200.f))
            {
                if (me->GetDistance2d(pBoss) >= 3 && !trig)
                {
                    me->SummonCreature(59507, pBoss->GetPositionX(), pBoss->GetPositionY(), pBoss->GetPositionZ(), TEMPSUMMON_MANUAL_DESPAWN);
                    trig = true;
                }
            }
        }
    };
};


void AddSC_boss_brother_korloff()
{
    new boss_brother_korloff();
    new trigger_scorched_flame();
}