#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scarlet_monastery.h"

enum ePhases
{
    PHASE_ONE,
    PHASE_TWO
};

enum eSpells
{
    SPELL_FLASH_OF_STEEL = 115629, //Durand
    SPELL_DASHING_STRIKE = 115629,
    SPELL_DASHING_STRIKE_TARGET = 122519,
    SPELL_POWER_WORD_SHIELD = 22187, //Whitemane
    SPELL_SMITE = 114848,
    SPELL_HEAL = 36678,
    SPELL_MASS_RESURRETION = 113134,
    SPELL_DEEP_SLEEP = 9256, // at 50% health of Whitemane
    SPELL_REVIVE = 9232 // same
};

enum eDurandEvents
{
    EVENT_FLASH_OF_STEEL = 1,
    EVENT_DASHING_STRIKE = 2,
    EVENT_DASHING_STRIKE_BACK = 3
};

enum eWhitemaneEvents
{
    EVENT_INTRO = 1,
    EVENT_SMITE = 2,
    EVENT_MASS_RESURRETION = 3,
    EVENT_HEAL = 4,
    EVENT_RESURRETION = 5,
    EVENT_RESURRETION_DOWN = 6
};

enum eActions
{
    ACTION_DURAND = 1,
    ACTION_INTRO = 2,
    ACTION_LAST_PHASE = 3
};

enum Durand_Yells
{
    DURAND_TALK_INTRO=1,
    DURAND_TALK_SLAY=2
};

enum Whitemane_Yells
{
    WHITEMANE_TALK_RESSURETION = 2,
    WHITEMANE_TALK_INTRO = 3,
    WHITEMANE_TALK_DEATH = 4
};

const Position Whitemane_intro = { 740.48f, 606.35f, 15.027f, 6.26f };
class boss_commander_durand : public CreatureScript
{
public:
    boss_commander_durand() : CreatureScript("boss_commander_durand") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_commander_durandAI(creature);
    }

    struct boss_commander_durandAI : public BossAI
    {
        boss_commander_durandAI(Creature* creature) : BossAI(creature, BOSS_DURAND) {}
        EventMap events;
        ePhases phases;
        uint32 _flashcount;
        uint32 _dashingcount;
        bool _dashingcheck;
        bool _fakedeath;
        bool _restore;

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetStandState(UNIT_STAND_STATE_STAND);
            if (instance)
                instance->SetData(BOSS_DURAND, NOT_STARTED);
            phases = PHASE_ONE;
            _flashcount = 0;
            _dashingcount = 0;
            _dashingcheck = false;
            _fakedeath = false;
            _restore = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void JustDied(Unit* killer) override
        {
                _JustDied();
                if (instance)
                {
                    instance->SetData(BOSS_DURAND, DONE);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }
        }

        void DamageTaken(Unit* /*doneBy*/, uint32 &damage)
        {
            if (me->GetHealth() <= damage && !_fakedeath)
            {

                //On first death, fake death and open door, as well as initiate whitemane if exist
                if (Unit* Whitemane = Unit::GetCreature(*me, instance->GetData64(BOSS_WHITEMANE)))
                {
                    damage = 0;
                    Whitemane->GetMotionMaster()->MovePoint(0, Whitemane_intro);
                    instance->SetBossState(BOSS_WHITEMANE, IN_PROGRESS);
                    me->GetMotionMaster()->MovementExpired();
                    me->GetMotionMaster()->MoveIdle();

                    if (me->IsNonMeleeSpellCasted(false))
                        me->InterruptNonMeleeSpells(false);

                    me->ClearComboPointHolders();
                    me->RemoveAllAuras();
                    me->ClearAllReactives();
                    _fakedeath = true;
                    _restore = true;
                    me->SetHealth(1000000);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    events.Reset();
                }
            }

            if (_dashingcheck)
                damage = 0;
        }

        void DoAction(const int32 action) override
        {
            if (action == ACTION_DURAND)
                BossAI::EnterEvadeMode();
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_DURAND, FAIL);
            }
            summons.DespawnAll();
        }


        void EnterCombat(Unit* who) override
        {
            _EnterCombat();
            if (instance)
            {
                instance->SetData(BOSS_DURAND, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            Talk(DURAND_TALK_INTRO);
            events.ScheduleEvent(EVENT_FLASH_OF_STEEL, 9000);
            events.ScheduleEvent(EVENT_DASHING_STRIKE, urand(24000, 25000));
            me->CallForHelp(VISIBLE_RANGE);
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (_restore && me->GetHealthPct() >= 50)
            {
                _restore = false;
                events.ScheduleEvent(EVENT_FLASH_OF_STEEL, 9000);
                events.ScheduleEvent(EVENT_DASHING_STRIKE, urand(24000, 25000));
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->GetMotionMaster()->MoveChase(target);
            }

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FLASH_OF_STEEL:
                    if (_flashcount < 6)
                    {
                        _flashcount++;
                        if (Unit* player = SelectTarget(SELECT_TARGET_FARTHEST))
                            me->CastSpell(player, SPELL_FLASH_OF_STEEL, false);
                        events.ScheduleEvent(EVENT_FLASH_OF_STEEL, urand(500, 1000));
                    }
                    else { _flashcount = 0; events.ScheduleEvent(EVENT_DASHING_STRIKE, urand(21000, 26000)); }
                    break;
                case EVENT_DASHING_STRIKE:
                    if (_dashingcount < 6)
                    {
                        if (!_dashingcheck) { me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE); _dashingcheck = true; Talk(DURAND_TALK_SLAY); }
                        _dashingcount++;
                        if (Unit* player = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            DoCastAOE(SPELL_DASHING_STRIKE_TARGET);
                            me->CastSpell(player, SPELL_DASHING_STRIKE, false);
                        }
                        events.ScheduleEvent(EVENT_DASHING_STRIKE, urand(500, 1000));
                    }
                    else { _dashingcount = 0; _dashingcheck = false; events.ScheduleEvent(EVENT_DASHING_STRIKE, urand(24000, 25000)); me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE); }
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class boss_high_inqusitior_whitemane : public CreatureScript
{
public:
    boss_high_inqusitior_whitemane() : CreatureScript("boss_high_inqusitior_whitemane") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_high_inqusitior_whitemaneAI(creature);
    }

    struct boss_high_inqusitior_whitemaneAI : public BossAI
    {
        boss_high_inqusitior_whitemaneAI(Creature* creature) : BossAI(creature, BOSS_WHITEMANE) {}
        EventMap events;
        bool _switch;
        

        void DoAction(const int32 action) override
        {
            switch (action)
            {
            case ACTION_INTRO:
                me->GetMotionMaster()->MovePoint(0, Whitemane_intro);
                events.ScheduleEvent(EVENT_INTRO, urand(3000, 3500));
                break;
            case ACTION_LAST_PHASE:
                events.ScheduleEvent(EVENT_HEAL, urand(15000, 16000));
                events.ScheduleEvent(EVENT_MASS_RESURRETION, 10000);
                break;
            default:
                break;
            }
        }

        void DamageTaken(Unit* /*doneBy*/, uint32 &damage)
        {
            if (me->GetHealth() <= damage&& !_switch)
            {
                damage = 0;
                me->RemoveAllAuras();
                me->SetHealth(uint32(me->GetMaxHealth() / 2));
            }
        }

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            if (instance)
                instance->SetData(BOSS_WHITEMANE, NOT_STARTED);
            _switch = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void JustDied(Unit* killer) override
        {
            Talk(WHITEMANE_TALK_DEATH);
            _JustDied();
            if (instance)
            {
                instance->SetData(BOSS_WHITEMANE, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_WHITEMANE, FAIL);
            }
            summons.DespawnAll();
        }


        void EnterCombat(Unit* who) override
        {
            _EnterCombat();
            if (instance)
            {
                instance->SetData(BOSS_WHITEMANE, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type==POINT_MOTION_TYPE && id == 0)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                if (Player* _player = me->FindNearestPlayer(VISIBLE_RANGE))
                    me->CombatStart(_player);
                else
                {
                    BossAI::EnterEvadeMode();
                    if (Unit* Whitemane = Unit::GetCreature(*me, instance->GetData64(BOSS_WHITEMANE)))
                        Whitemane->GetAI()->DoAction(ACTION_DURAND);
                }

                Talk(WHITEMANE_TALK_INTRO);
            }
                
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->GetHealthPct() <= 50 && !_switch) { _switch = true; events.Reset(); events.ScheduleEvent(EVENT_RESURRETION, urand(500, 1000)); }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM), SPELL_SMITE, false);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_INTRO:
                    events.ScheduleEvent(EVENT_SMITE, urand(3000, 3500));
                    events.ScheduleEvent(EVENT_HEAL, urand(9000, 13000));
                    break;
                case EVENT_HEAL:
                    me->CastSpell(me, SPELL_HEAL, false);
                    events.ScheduleEvent(EVENT_HEAL, urand(9000, 13000));
                    break;
                case EVENT_RESURRETION:
                    Talk(WHITEMANE_TALK_RESSURETION);

                    if (me->IsNonMeleeSpellCasted(false))
                        me->InterruptNonMeleeSpells(false);

                    DoCastAOE(SPELL_DEEP_SLEEP);
                    me->CastSpell(me, SPELL_POWER_WORD_SHIELD);
                    if (Unit* Durand = Unit::GetCreature(*me, instance->GetData64(BOSS_DURAND)))
                    {
                        me->AI()->DoAction(ACTION_LAST_PHASE);
                        DoCast(Durand, SPELL_REVIVE);
                        events.ScheduleEvent(EVENT_RESURRETION_DOWN, 2000);
                    }
                    break;
                case EVENT_RESURRETION_DOWN:
                    instance->SetBossState(BOSS_DURAND, SPECIAL);
                    me->SetFullHealth();
                    break;
                default:
                    break;
                }
            }

        }

    };
};

void AddSC_boss_whitemane_and_durand()
{
    new boss_commander_durand();
    new boss_high_inqusitior_whitemane();
}