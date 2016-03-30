#include "the_stonecore.h"
#include "Spell.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Vehicle.h"

enum Spells
{
    SPELL_CURSE_OF_BLOOD            = 79345,
    SPELL_FORCE_GRIP                = 79351,
    SPELL_FORCE_GRIP_DOWN           = 79359,
    SPELL_FORCE_GRIP_UP             = 79358,
    SPELL_SUMMON_GRAVITY_WELL       = 79340,
    SPELL_SEISMIC_SHARD             = 79002, // visual
    SPELL_SEISMIC_SHARD_CHARGE      = 79014, // damage + leap
    SPELL_SEISMIC_SHARD_PULL        = 86862, // pulls the shard -> makes it enter vehicle
    SPELL_SEISMIC_SHARD_TAR         = 80511, // target visual
    SPELL_SEISMIC_SHARD_THROW       = 79015, // throw visual
    SPELL_SEISMIC_SHARD_SUMM_1      = 86856, // summons shards
    SPELL_SEISMIC_SHARD_SUMM_2      = 86858,
    SPELL_SEISMIC_SHARD_SUMM_3      = 86860,
    SPELL_ENERGY_SHIELD             = 82858,
};

enum Events
{
    EVENT_CURSE_OF_BLOOD            = 1,
    EVENT_FORCE_GRIP,
    EVENT_SEISMIC_SHARD,
    EVENT_SEISMIC_SHARD_THROW,
    EVENT_SHIELD_PHASE_END,
    EVENT_GRAVITY_WELL,
    EVENT_ENERGY_SHIELD,
    EVENT_ENERGY_SHIELD_END,
    EVENT_ADDS_SUMMON,
};

enum Phases
{
    PHASE_NORMAL                    = 1,
    PHASE_SHIELD
};

enum Misc
{
    VEHICLE_GRIP                    = 892,
    VEHICLE_NORMAL                  = 903,
    NPC_SEISMIC_SHARD               = 42355,
    NPC_FOLLOWER                    = 42428,
    POINT_FLY                       = 1,
    POINT_PLATFORM
};

enum Quotes
{
    SAY_AGGRO,
    SAY_DEATH,
    SAY_SLAY,
    SAY_SHIELD,
};

static const Position summonPos[2] =
{
    {1271.93f, 1042.73f, 210.0f, 0.0f}, // W
    {1250.99f, 949.48f, 205.5f, 0.0f} // E
};

static const Position jumpPos = {1332.59f, 983.41f, 207.62f, 0.0f};

class boss_azil : public CreatureScript
{
    struct boss_azilAI : public BossAI
    {
        boss_azilAI(Creature * creature) : BossAI(creature, DATA_HIGH_PRIESTESS_AZIL)
        {
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        }

        void Reset()
        {
            forceGrip = false;
            gripStep = 0;
            seismicShards = 0;
            forcegripTimer = 500;
            me->SetVehicleId(VEHICLE_NORMAL);
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetCanFly(false);
            _Reset();
        }

        void EnterCombat(Unit * /*victim*/)
        {
            Talk(SAY_AGGRO);
            me->GetMotionMaster()->MoveJump(jumpPos, 10.0f, 10.0f);
            events.SetPhase(PHASE_NORMAL);
            events.ScheduleEvent(EVENT_FORCE_GRIP, 10000);
            events.ScheduleEvent(EVENT_ENERGY_SHIELD, 45000);
            events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, urand(5000, 8000));
            events.ScheduleEvent(EVENT_GRAVITY_WELL, urand(3000, 5000));
            events.ScheduleEvent(EVENT_ADDS_SUMMON, urand(10000, 15000));
            _EnterCombat();
        }

        void SpellHit(Unit * /*caster*/, const SpellInfo *spell)
        {
            Spell * curSpell = me->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (curSpell && curSpell->m_spellInfo->Id == SPELL_FORCE_GRIP)
                for (uint8 i = 0; i < 3; ++i)
                    if (spell->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
                        me->InterruptSpell(CURRENT_GENERIC_SPELL, false);
        }

        void JustSummoned(Creature * summon)
        {
            if (summon->GetEntry() == NPC_SEISMIC_SHARD)
            {
                summon->setActive(true); // make it visible at any distance
                summon->GetMotionMaster()->MoveJump(summon->GetPositionX(), summon->GetPositionY(), (float)urand(220, 225), 15.0f, 15.0f);
            }
            BossAI::JustSummoned(summon);
        }

        void KilledUnit(Unit * victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void JustDied(Unit * /*killer*/)
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void PassengerBoarded(Unit * passenger, int8 , bool apply)
        {
            if (apply && passenger->ToPlayer())
                forceGrip = true;
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == EFFECT_MOTION_TYPE && id == POINT_PLATFORM)
            {
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();

                events.ScheduleEvent(EVENT_SHIELD_PHASE_END, 30000);
                me->SetFacingTo(me->GetHomePosition().GetOrientation());
                DoCast(me, SPELL_SEISMIC_SHARD);
                DoCast(me, SPELL_SEISMIC_SHARD_SUMM_1, true);
                DoCast(me, SPELL_SEISMIC_SHARD_SUMM_2, true);
                DoCast(me, SPELL_SEISMIC_SHARD_SUMM_3, true);
                events.ScheduleEvent(EVENT_SEISMIC_SHARD, 4000);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!forceGrip && !UpdateVictim())
                return;

            events.Update(diff);

            if (forceGrip)
            {
                Unit * victim = me->GetVictim();
                Vehicle * veh = me->GetVehicleKit();

                if (forcegripTimer <= diff)
                {
                    if (!victim || !veh || !veh->GetPassenger(1))
                    {
                        forceGrip = false;
                        return;
                    }
                    switch(gripStep)
                    {
                    case 0: // down
                    case 2:
#if 0
                        veh->SendSeatChange(victim, 2);
#endif
                        DoCast(victim, SPELL_FORCE_GRIP_DOWN, true);
                        break;
                    case 1: // up + damage
                    case 3:
#if 0
                        veh->SendSeatChange(victim, 1);
#endif
                        DoCast(victim, SPELL_FORCE_GRIP_UP, true);
                        break;
                    case 4: // exit vehicle + knockback
                        {
                            gripStep = 0;
                            forceGrip = false;
                            Position pos;
                            pos.m_positionX = me->GetPositionX() + 4.888855f * std::cos(me->GetOrientation()) - -2.645142f * std::sin(me->GetOrientation());
                            pos.m_positionY = me->GetPositionY() - 2.645142f * std::cos(me->GetOrientation()) + 4.888855f * std::sin(me->GetOrientation());
                            pos.m_positionZ = me->GetPositionZ();
                            //veh->GetSeatPosition(&pos, 1);
                            victim->ExitVehicle(&pos);
                            victim->KnockbackFrom(me->GetPositionX(), me->GetPositionY(), 15.0f, 15.0f);
                        }
                        break;
                    }
                    if (forceGrip)
                        ++gripStep;
                    forcegripTimer = 700;
                }else forcegripTimer -= diff;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                case EVENT_CURSE_OF_BLOOD:
                    DoCastVictim(SPELL_CURSE_OF_BLOOD);
                    events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, urand(8000, 10000), 0, PHASE_NORMAL);
                    break;
                case EVENT_FORCE_GRIP:
                    me->SetVehicleId(VEHICLE_GRIP);
                    DoCastVictim(SPELL_FORCE_GRIP);
                    gripStep = 0;
                    forcegripTimer = 500;
                    events.ScheduleEvent(EVENT_FORCE_GRIP, urand(15000, 20000), 0, PHASE_NORMAL);
                    break;
                case EVENT_GRAVITY_WELL:
                    DoCast(SELECT_TARGET_RANDOM, SPELL_SUMMON_GRAVITY_WELL, false);
                    events.ScheduleEvent(EVENT_GRAVITY_WELL, urand(15000, 20000), 0, PHASE_NORMAL);
                    break;
                case EVENT_SEISMIC_SHARD:
                    if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    {
                        target->GetPosition(&shardPos);
                        DoCast(me, SPELL_SEISMIC_SHARD_PULL, true);
                        me->CastSpell(shardPos, SPELL_SEISMIC_SHARD_TAR, true);
                        DoCast(me, SPELL_SEISMIC_SHARD_THROW, false);
                        events.ScheduleEvent(EVENT_SEISMIC_SHARD_THROW, 3000);
                    }
                    if (seismicShards < 2)
                    {
                        ++seismicShards;
                        events.ScheduleEvent(EVENT_SEISMIC_SHARD, 7000);
                    }
                    break;
                case EVENT_SEISMIC_SHARD_THROW:
                    if (Vehicle * veh = me->GetVehicleKit())
                    {
                        if (Unit * passenger = veh->GetPassenger(0))
                        {
                            //Position pos;
                            passenger->ExitVehicle();
                            //me->GetVehicleKit()->GetSeatPosition(&pos, -1, passenger);
                            //passenger->ExitVehicle(&pos);
                            passenger->CastSpell(shardPos, SPELL_SEISMIC_SHARD_CHARGE, false);
                            passenger->ToCreature()->DespawnOrUnsummon(3000);
                        }
                    }
                    break;
                case EVENT_ENERGY_SHIELD:
                    Talk(SAY_SHIELD);
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveIdle();
                    me->SetVehicleId(VEHICLE_NORMAL);
                    me->SetTarget(0);
                    events.SetPhase(PHASE_SHIELD);
                    DoCast(me, SPELL_ENERGY_SHIELD);
                    seismicShards = 0;
                    events.ScheduleEvent(EVENT_ENERGY_SHIELD_END, 2000);
                    break;
                case EVENT_ENERGY_SHIELD_END: // fly up
                    me->GetMotionMaster()->MoveJump(me->GetHomePosition(), 10.0f, 30.0f, POINT_PLATFORM);
                    break;
                case EVENT_SHIELD_PHASE_END:
                    me->RemoveAurasDueToSpell(SPELL_SEISMIC_SHARD);
                    me->RemoveAurasDueToSpell(SPELL_ENERGY_SHIELD);
                    me->GetMotionMaster()->MoveJump(jumpPos, 10.0f, 20.0f);
                    if (Unit * victim = me->SelectVictim())
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoStartMovement(victim);
                        me->SetTarget(victim->GetGUID());
                    }
                    events.SetPhase(PHASE_NORMAL);
                    events.RescheduleEvent(EVENT_CURSE_OF_BLOOD, urand(8000, 10000), 0, PHASE_NORMAL);
                    events.RescheduleEvent(EVENT_GRAVITY_WELL, urand(15000, 20000), 0, PHASE_NORMAL);
                    events.RescheduleEvent(EVENT_FORCE_GRIP, urand(10000, 12000), 0, PHASE_NORMAL);
                    events.ScheduleEvent(EVENT_ENERGY_SHIELD, urand(40000, 45000), 0, PHASE_NORMAL);
                    break;
                case EVENT_ADDS_SUMMON:
                    {
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            uint8 amount = 3;
                            uint8 pos = urand(0, 1);
                            if (events.IsInPhase(PHASE_SHIELD))
                                amount = urand(8, 10);
                            for(int i=0; i < amount; ++i)
                            {
                                Position tarPos;
                                me->GetRandomPoint(summonPos[pos], 5.0f, tarPos);
                                if (Creature * summon = me->SummonCreature(NPC_FOLLOWER, tarPos, TEMPSUMMON_DEAD_DESPAWN, 1000))
                                {
                                    summon->AI()->AttackStart(target);
                                    summon->AI()->DoZoneInCombat();
                                }
                            }
                        }
                        events.ScheduleEvent(EVENT_ADDS_SUMMON, urand(10000, 12000));
                    }
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        bool forceGrip;
        int8 gripStep;
        int8 seismicShards;
        uint32 forcegripTimer;
        Position shardPos;
    };
public:
    boss_azil() : CreatureScript("boss_azil") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_azilAI(creature);
    }
};

class npc_gravity_well_azil : public CreatureScript
{
    enum
    {
        SPELL_GRAVITY_WELL_VIS_1        = 79245, // after 8 sec - removed
        SPELL_GRAVITY_WELL_PERIODIC     = 79244,
        SPELL_GRAVITY_WELL_SCRIPT       = 79251,
        SPELL_GRAVITY_WELL_DMG          = 79249,
        SPELL_GRAVITY_WELL_PULL         = 79333,
        SPELL_GRAVITY_WELL_SCALE        = 92475, // hc only
    };

    struct npc_gravity_well_azilAI : public ScriptedAI
    {
        npc_gravity_well_azilAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset()
        {
            DoCast(me, SPELL_GRAVITY_WELL_VIS_1);
            active = false;
            activeTimer = 8000;
            if (!IsHeroic())
                me->DespawnOrUnsummon(20000);
            killCount = 0;
        }

        void SpellHitTarget(Unit * target, const SpellInfo * spell)
        {
            if (target->IsAlive() && spell->Id == SPELL_GRAVITY_WELL_SCRIPT)
            {
                int bp = IsHeroic() ? 20000 : 10000; // evtl needs to be increased / lowered
                uint32 distFkt = uint32(me->GetDistance(target)) * 5;
                bp -= (bp * distFkt) / 100;

                me->CastCustomSpell(target, SPELL_GRAVITY_WELL_DMG, &bp, NULL, NULL, true);
            }
        }

        void KilledUnit(Unit * victim)
        {
            if (IsHeroic() && victim->GetEntry() == NPC_FOLLOWER)
            {
                if (killCount == 3)
                    me->DespawnOrUnsummon();
                else
                {
                    DoCast(me, SPELL_GRAVITY_WELL_SCALE, true);
                    ++killCount;
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!active)
            {
                if (activeTimer <= diff)
                {
                    active = true;
                    me->RemoveAurasDueToSpell(SPELL_GRAVITY_WELL_VIS_1);
                    DoCast(me, SPELL_GRAVITY_WELL_PERIODIC, true);
                    DoCast(me, SPELL_GRAVITY_WELL_PULL, true);
                }else activeTimer -= diff;
            }
        }

    private:
        bool active;
        uint32 activeTimer;
        uint8 killCount;
    };
public:
    npc_gravity_well_azil() : CreatureScript("npc_gravity_well_azil") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_gravity_well_azilAI(creature);
    }
};

void AddSC_boss_azil()
{
    new boss_azil();
    new npc_gravity_well_azil();
}
