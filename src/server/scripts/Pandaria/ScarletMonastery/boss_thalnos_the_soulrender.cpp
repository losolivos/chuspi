#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scarlet_monastery.h"

enum Spells
{
    SPELL_CHANNEL_SOUL = 60309, // visual to trigger
    SPELL_RITUAL_OF_SOULD = 60429, // only aura
    SPELL_RAISE_FALLEN_CRUSADER = 115139,
    SPELL_SPIRIT_GALE = 115289,
    SPELL_EVICT_SOUL = 115297,
    SPELL_SUMMON_EMPOWERING_SPIRITS = 115147
};

enum Events
{
    EVENT_SPIRIT_GALE = 1,
    EVENT_RAISE_FALLEN_CRUSADER = 2,
    EVENT_EVICT_SOUL = 3,
    EVENT_CHANNELED_SOUL = 4,
    EVENT_EMPOWERING_SPIRITS = 5
};

enum Yells
{
    TALK_INTO = 0,
    TALK_SLAY = 1,
    TALK_EVICT = 2,
    TALK_SPIRITS = 3,
    TALK_DEATH = 4
};
class boss_thalnos_the_soulrender : public CreatureScript
{
public:
    boss_thalnos_the_soulrender() : CreatureScript("boss_thalnos_the_soulrender") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_thalnos_the_soulrenderAI(creature);
    }

    struct boss_thalnos_the_soulrenderAI : public BossAI
    {
        boss_thalnos_the_soulrenderAI(Creature* creature) : BossAI(creature, BOSS_THALNOS_THE_SOULRENDER) {}
        EventMap events;

        void Reset() override
        {
            _Reset();
            events.Reset();
            me->setRegeneratingHealth(true);
            me->SetReactState(REACT_AGGRESSIVE);
            if (instance)
                instance->SetData(BOSS_THALNOS_THE_SOULRENDER, NOT_STARTED);
        }

        void JustDied(Unit* killer) override
        {
            _JustDied();
            Talk(TALK_DEATH);
            if (instance)
            {
                instance->SetData(BOSS_THALNOS_THE_SOULRENDER, DONE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(BOSS_THALNOS_THE_SOULRENDER, FAIL);
            }
            summons.DespawnAll();
        }

        void JustSummoned(Creature* who) override
        {
            who->setActive(true);
            if (Unit* target = who->FindNearestPlayer(3.0f))
                who->AI()->AttackStart(target);
        }

        void EnterCombat(Unit* who) override
        {
            _EnterCombat();
            Talk(TALK_INTO);
            if (instance)
            {
                instance->SetData(BOSS_THALNOS_THE_SOULRENDER, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }
            events.ScheduleEvent(EVENT_SPIRIT_GALE, 1500);
            events.ScheduleEvent(EVENT_RAISE_FALLEN_CRUSADER, 6000);
            events.ScheduleEvent(EVENT_EMPOWERING_SPIRITS, 12000); 
            events.ScheduleEvent(EVENT_EVICT_SOUL, 15500);
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SPIRIT_GALE:
                    DoCast(SelectTarget(SELECT_TARGET_RANDOM), SPELL_SPIRIT_GALE);
                    events.ScheduleEvent(EVENT_SPIRIT_GALE, urand(7000,8000));
                    break;
                case EVENT_RAISE_FALLEN_CRUSADER:
                    Talk(TALK_SLAY);
                    DoCastAOE(SPELL_RAISE_FALLEN_CRUSADER);               
                    break;
                case EVENT_EVICT_SOUL:
                    Talk(TALK_EVICT);
                    DoCast(SelectTarget(SELECT_TARGET_RANDOM), SPELL_EVICT_SOUL);
                    events.ScheduleEvent(EVENT_EVICT_SOUL, 15500);
                    break;
                case EVENT_EMPOWERING_SPIRITS:
                    Talk(TALK_SPIRITS);
                    DoCastAOE(SPELL_SUMMON_EMPOWERING_SPIRITS);
                    events.ScheduleEvent(EVENT_EMPOWERING_SPIRITS, 35000);
                    events.ScheduleEvent(EVENT_RAISE_FALLEN_CRUSADER, 20000);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_evicted_soul : public CreatureScript
{
public:
    npc_evicted_soul() : CreatureScript("npc_evicted_soul")
    {
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_evicted_soulAI(creature);
    }

    struct npc_evicted_soulAI : public ScriptedAI
    {
        npc_evicted_soulAI(Creature* creature) : ScriptedAI(creature) { pInstance = creature->GetInstanceScript(); }
        EventMap events;
        InstanceScript* pInstance;

        void IsSummonedBy(Unit* summoner)
        {
            if (Unit* player = me->SelectNearestPlayer(2.0f))
                player->CastSpell(me, 45204, true); // coppy
            me->CombatStart(me->FindNearestPlayer(VISIBLE_RANGE));
        }

        void UpdateAI(const uint32 diff) override
        {
            if (pInstance->GetBossState(BOSS_THALNOS_THE_SOULRENDER) == FAIL || pInstance->GetBossState(BOSS_THALNOS_THE_SOULRENDER) == DONE)
                me->DespawnOrUnsummon();

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();
        }
    };
};

class npc_fallen_crusader : public CreatureScript
{
public:
    npc_fallen_crusader() : CreatureScript("npc_fallen_crusader")
    {
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fallen_crusaderAI(creature);
    }

    struct npc_fallen_crusaderAI : public ScriptedAI
    {
        npc_fallen_crusaderAI(Creature* creature) : ScriptedAI(creature) { pInstance = creature->GetInstanceScript(); }
        EventMap events;
        InstanceScript* pInstance;

        void IsSummonedBy(Unit* summoner)
        {
            mindroot_timer = urand(3500, 4200);
            me->CombatStart(me->FindNearestPlayer(VISIBLE_RANGE));
        }

        uint32 mindroot_timer;

        void UpdateAI(const uint32 diff) override
        {

            if (mindroot_timer <= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_TOPAGGRO), 115144);
                mindroot_timer = urand(3500, 4200);
            }
            else
                mindroot_timer -= diff;

            if (pInstance->GetBossState(BOSS_THALNOS_THE_SOULRENDER) == FAIL || pInstance->GetBossState(BOSS_THALNOS_THE_SOULRENDER) == DONE)
                me->DespawnOrUnsummon();

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();
        }
    };
};

class npc_empowering_spirit : public CreatureScript
{
public:
    npc_empowering_spirit() : CreatureScript("npc_empowering_spirit") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_empowering_spiritAI(creature);
    }

    struct npc_empowering_spiritAI : public ScriptedAI
    {
        npc_empowering_spiritAI(Creature* creature) : ScriptedAI(creature) { pInstance = creature->GetInstanceScript(); }
        EventMap events;
        InstanceScript* pInstance;

        void IsSummonedBy(Unit* summoner)
        {
            zombie_timer = 10000;
            me->CombatStart(me->FindNearestPlayer(VISIBLE_RANGE));
        }
        uint32 zombie_timer;
        bool check;
        Creature *zombie;

        void JustSummoned(Creature* summoned)
        {
            summoned->CombatStart(summoned->FindNearestPlayer(30.0f));
        }

        void UpdateAI(const uint32 diff) override
        {

            if (pInstance->GetBossState(BOSS_THALNOS_THE_SOULRENDER) == FAIL || pInstance->GetBossState(BOSS_THALNOS_THE_SOULRENDER) == DONE)
                me->DespawnOrUnsummon();

            if (zombie_timer <= diff)
            {
                me->SummonCreature((IsHeroic() ? EMPOWERED_ZOMBIE : EMPOWERED_ZOMBIE_HEROIC), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), TEMPSUMMON_MANUAL_DESPAWN);
                me->DespawnOrUnsummon();
            }
                else zombie_timer -= diff;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();
        }

    };
};

class npc_empowered_zombie : public CreatureScript
{
public:
    npc_empowered_zombie() : CreatureScript("npc_empowered_zombie")
    {
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_empowered_zombieAI(creature);
    }

    struct npc_empowered_zombieAI : public ScriptedAI
    {
        npc_empowered_zombieAI(Creature* creature) : ScriptedAI(creature) { pInstance = creature->GetInstanceScript(); }
        EventMap events;
        InstanceScript* pInstance;

        void IsSummonedBy(Unit* summoner)
        {
            me->CombatStart(me->FindNearestPlayer(VISIBLE_RANGE));
        }
        uint32 combat_timer;

        void UpdateAI(const uint32 diff) override
        {

            if (pInstance->GetBossState(BOSS_THALNOS_THE_SOULRENDER) == FAIL || pInstance->GetBossState(BOSS_THALNOS_THE_SOULRENDER) == DONE)
                me->DespawnOrUnsummon();

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();
        }
    };
};



void AddSC_boss_thalnos_the_soulrender()
{
    new boss_thalnos_the_soulrender();
    new npc_evicted_soul();
    new npc_fallen_crusader();
    new npc_empowering_spirit();
    new npc_empowered_zombie();
}