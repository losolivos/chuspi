#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "MoveSplineInit.h"
#include "Vehicle.h"
#include "deadmines.h"
#include "Util.h"

enum eSpells
{
    SPELL_ARCANE_POWER              = 88009,
    SPELL_BLINK                     = 87925,
    SPELL_ARCANE_OVERLOAD_VISUAL    = 88183,
    SPELL_FIRE_BLOSSOM_VISUAL       = 88164,
    SPELL_FIRE_BLOSSOM              = 88129,
    SPELL_FROST_BLOSSOM_VISUAL      = 88165,
    SPELL_FROST_BLOSSOM             = 88169,
    SPELL_TELEPORT                  = 88002,
    SPELL_FISTS_OF_FLAME            = 87859,
    SPELL_FISTS_OF_FROST            = 87861,
    SPELL_EMOTE_TALK                = 79506,
    SPELL_EMOTE_ROAR                = 48350,
    SPELL_ARCANE_FROST_BEAM         = 88093,
    SPELL_ARCANE_FIRE_BEAM          = 88072,
    SPELL_BLOSSOM_TARGETTING        = 88140,
    SPELL_ARCANE_OVERLOAD           = 90520,
    SPELL_GLUBTOK_INVISIBILITY      = 90424,
    SPELL_ARCANE_OVERLOAD_KILL_SELF = 88185,
    SPELL_RIDE_VEHICLE_HARDCODED    = 46598,
    SPELL_FIRE_WALL                 = 91398,
};

enum eCreatures
{
    NPC_GLUBTOK_FIREWALL_PLATTER    = 48974,
    GFPC_LEVEL_1_A                  = 48975,
    GFPC_LEVEL_1_B                  = 49039,
    GFPC_LEVEL_1_C                  = 49040,
    GFPC_LEVEL_2_A                  = 48976,
    GFPC_LEVEL_2_B                  = 49041,
    GFPC_LEVEL_2_C                  = 49042,
    GP_BUNNY_LOOK_1                 = 45979,
    GP_BUNNY_LOOK_2                 = 47242,
    FIRE_BLOSSOM_BUNNY              = 47282,
    FROST_BLOSSOM_BUNNY             = 47284,
};

enum eScriptTexts
{
    GLUBTOK_YELL_START              = 0,
    GLUBTOK_YELL_DYING              = 1,
    GLUBTOK_YELL_KILL_PLAYER        = 2,
    GLUBTOK_YELL_FIRE_FISTS         = 3,
    GLUBTOK_YELL_FROST_FISTS        = 4,
    GLUBTOK_YELL_EVENT_1            = 5,
    GLUBTOK_YELL_EVENT_2            = 6,
    GLUBTOK_YELL_EVENT_3            = 7,
    GLUBTOK_EMOTE_FIRE_WALL         = 8,
};

enum eEvents
{
    // Glubtok
    EVENT_YELL_READY                = 1,
    EVENT_LETS_DO_IT                = 2,
    EVENT_ARCANE_POWER              = 3,
    EVENT_SUMMON_FIRE_WALL          = 4,
    EVENT_DYING_1                   = 5,
    EVENT_DYING_2                   = 6,
    EVENT_BLOSSOM                   = 7,
    EVENT_FIRE_FISTS                = 8,
    EVENT_FROST_FISTS               = 9,
    EVENT_BLINK                     = 10,
    // Fire Wall
    EVENT_INSTALL_WALL_LEVEL_ONE    = 1,
    EVENT_INSTALL_WALL_LEVEL_TWO    = 2,
    EVENT_ENABLE_WALL_LEVEL_TWO     = 3,
    EVENT_CHANGE_ANGLE              = 4,
};

enum eData
{
    DATA_PLAYER_HIT                 = 1,
};

struct WallLevel
{
    uint32 EntryLevel_1;
    uint32 EntryLevel_2;
};

const uint32 Wall[8]=
{
    GFPC_LEVEL_1_A,
    GFPC_LEVEL_1_B,
    GFPC_LEVEL_1_C,
    GFPC_LEVEL_1_C,
    GFPC_LEVEL_1_A,
    GFPC_LEVEL_1_B,
    GFPC_LEVEL_1_C,
    GFPC_LEVEL_1_C,
};

const WallLevel WallLvl[8]=
{
    {GFPC_LEVEL_1_A, GFPC_LEVEL_2_A},
    {GFPC_LEVEL_1_B, GFPC_LEVEL_2_B},
    {GFPC_LEVEL_1_C, GFPC_LEVEL_2_C},
};

const Position ArcanePowerPos = {-193.403f, -441.51f, 53.61f, 1.83427f};

class boss_glubtok : public CreatureScript
{
public:
    boss_glubtok() : CreatureScript("boss_glubtok") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_glubtokAI (creature);
    }

    struct boss_glubtokAI : public BossAI
    {
        boss_glubtokAI(Creature* creature) : BossAI(creature, DATA_GLUBTOK) { }

        std::set<uint64> lPlayersOnFire;
        bool FireWall;
        bool dying;
        bool Dead;

        void Reset()
        {
            _Reset();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetDisableGravity(false);
            me->SendSetPlayHoverAnim(false);
            me->SetCanFly(false);
            lPlayersOnFire.clear();
            FireWall = false;
            dying = false;
            Dead = false;
        }

        void SetGUID(uint64 guid, int32 type)
        {
            if (type == DATA_PLAYER_HIT)
                lPlayersOnFire.insert(guid);
        }

        void SummonBeam(uint32 Spell, int Mult)
        {
            for (int i = 0; i < 4; ++i)
            {
                float x, y, z, angle, dist;
                z = me->GetPositionZ() + frand(3.0f, 7.f);
                dist = frand(5.0f, 12.0f);
                angle = frand(M_PI / 4, 3 * M_PI / 4);
                me->GetNearPoint2D(x, y, dist, me->GetOrientation() + (Mult * angle));

                if (Creature* beam = me->SummonCreature(GP_BUNNY_LOOK_2, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 8000))
                    beam->CastSpell(me, Spell, true);
            }
        }

        void JustSummoned(Creature* summoned)
        {
            switch (summoned->GetEntry())
            {
                case GP_BUNNY_LOOK_2:
                    {
                        float x, y, z, radius, angle, mult;
                        angle = frand(0, M_PI);
                        z = summoned->GetPositionZ();
                        radius = frand(2.0f, 4.0f);
                        summoned->GetNearPoint2D(x, y, radius, summoned->GetOrientation() + angle);
                        Position pos;
                        pos.Relocate(x, y, z);
                        angle = pos.GetAngle(summoned);
                        mult = RAND(-1, 1) * M_PI / 4;

                        Movement::MoveSplineInit init(summoned);
                        G3D::Vector3 vertice(0, 0, 0);
                        init.Path().push_back(vertice);

                        for (int i = 0; i < 8; ++i)
                        {
                            x = pos.m_positionX + radius * sin(angle);
                            y = pos.m_positionY + radius * cos(angle);
                            z = pos.m_positionZ + frand(-4.0f, 4.0f);
                            angle += mult;
                            G3D::Vector3 vertice(x, y, z);
                            init.Path().push_back(vertice);

                        }

                        init.SetFly();
                        init.SetSmooth();
                        init.SetCyclic();
                        init.SetUncompressed();
                        init.SetVelocity(5.0f);
                        init.Launch();
                    }
                    return;
            }

            summons.Summon(summoned);
            summoned->SetInCombatWithZone();
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(GLUBTOK_YELL_START);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            events.ScheduleEvent(EVENT_FIRE_FISTS, urand(2000, 7000));
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(GLUBTOK_YELL_KILL_PLAYER);
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if (!FireWall && me->GetHealthPct() <= 50)
            {
                FireWall = true;
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                me->CastSpell(me, SPELL_TELEPORT, false);
                events.Reset();
                events.ScheduleEvent(EVENT_YELL_READY, 2000);
            }

            if (!Dead && damage >= me->GetHealth())
            {
                damage = 0;

                if (!dying)
                {
                    dying = true;
                    Talk(GLUBTOK_YELL_DYING);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->SetHealth(1);
                    events.Reset();
                    SummonBeam(SPELL_ARCANE_FROST_BEAM, 1);
                    SummonBeam(SPELL_ARCANE_FIRE_BEAM, -1);
                    me->CastSpell(me, SPELL_ARCANE_OVERLOAD_VISUAL, false);
                    events.ScheduleEvent(EVENT_DYING_1, 6000);
                }
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_BLOSSOM_TARGETTING)
            {
                switch (target->GetEntry())
                {
                    case FIRE_BLOSSOM_BUNNY:
                        target->CastSpell(target, SPELL_FIRE_BLOSSOM_VISUAL, false);
                        me->CastSpell(target, SPELL_FIRE_BLOSSOM, false);
                        break;
                    case FROST_BLOSSOM_BUNNY:
                        target->CastSpell(target, SPELL_FROST_BLOSSOM_VISUAL, false);
                        me->CastSpell(target, SPELL_FROST_BLOSSOM, false);
                        break;
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdatePlayerVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FIRE_FISTS:
                        Talk(GLUBTOK_YELL_FIRE_FISTS);
                        me->CastSpell(me, SPELL_FISTS_OF_FLAME, false);
                        events.ScheduleEvent(EVENT_BLINK, 14000);
                        events.ScheduleEvent(EVENT_FROST_FISTS, 14500);
                        break;
                    case EVENT_FROST_FISTS:
                        Talk(GLUBTOK_YELL_FROST_FISTS);
                        me->CastSpell(me, SPELL_FISTS_OF_FROST, false);
                        events.ScheduleEvent(EVENT_BLINK, 14000);
                        events.ScheduleEvent(EVENT_FIRE_FISTS, 14500);
                        break;
                    case EVENT_BLINK:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f))
                            {
                                me->CastSpell(target, SPELL_BLINK, false);

                                if (IsHeroic())
                                {
                                    me->getThreatManager().resetAllAggro();
                                    AttackStart(target);
                                }
                            }
                        }
                        break;
                    case EVENT_YELL_READY:
                        me->CastSpell(me, SPELL_EMOTE_TALK, false);
                        Talk(GLUBTOK_YELL_EVENT_1);
                        events.ScheduleEvent(EVENT_LETS_DO_IT, 3000);
                        break;
                    case EVENT_LETS_DO_IT:
                        me->CastSpell(me, SPELL_EMOTE_ROAR, false);
                        Talk(GLUBTOK_YELL_EVENT_2);
                        events.ScheduleEvent(EVENT_ARCANE_POWER, 2000);
                        break;
                    case EVENT_ARCANE_POWER:
                        {
                            me->SetDisableGravity(true);
                            me->SendSetPlayHoverAnim(true);
                            float x, y, z;
                            me->GetPosition(x, y, z);
                            me->GetMotionMaster()->MoveTakeoff(0, x, y, z + 2.0f);
                            Talk(GLUBTOK_YELL_EVENT_3);
                            SummonBeam(SPELL_ARCANE_FROST_BEAM, 1);
                            SummonBeam(SPELL_ARCANE_FIRE_BEAM, -1);
                            me->CastSpell(me, SPELL_ARCANE_POWER, false);

                            if (IsHeroic())
                                events.ScheduleEvent(EVENT_SUMMON_FIRE_WALL, 1500);
                        }
                        break;
                    case EVENT_SUMMON_FIRE_WALL:
                        Talk(GLUBTOK_EMOTE_FIRE_WALL);
                        float x, y, z;
                        me->GetPosition(x, y, z);
                        me->SummonCreature(NPC_GLUBTOK_FIREWALL_PLATTER, ArcanePowerPos.m_positionX, ArcanePowerPos.m_positionY, ArcanePowerPos.m_positionZ + 2.0f);
                        events.ScheduleEvent(EVENT_BLOSSOM, 4000);
                        break;
                    case EVENT_BLOSSOM:
                        me->CastSpell(me, SPELL_BLOSSOM_TARGETTING, true);
                        events.ScheduleEvent(EVENT_BLOSSOM, 4000);
                        break;
                    case EVENT_DYING_1:
                        {
                            events.ScheduleEvent(EVENT_DYING_2, 250);
                            Position pos;
                            me->GetPosition(&pos);
                            me->CastSpell(me, SPELL_GLUBTOK_INVISIBILITY, false);

                            if (Creature* boom = me->SummonCreature(GP_BUNNY_LOOK_1, pos, TEMPSUMMON_TIMED_DESPAWN, 3000))
                                boom->CastSpell(boom, SPELL_ARCANE_OVERLOAD, false);
                        }
                        break;
                    case EVENT_DYING_2:
                        {
                            uint64 killer_guid = 0;

                            if (!me->getThreatManager().getThreatList().empty())
                                killer_guid = (*me->getThreatManager().getThreatList().begin())->getUnitGuid();

                            Dead = true;
                            me->CastSpell(me, SPELL_ARCANE_OVERLOAD_KILL_SELF, false, NULL, NULL, killer_guid);
                        }
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_glubtok_firewall_platter_master : public CreatureScript
{
public:
    npc_glubtok_firewall_platter_master() : CreatureScript("npc_glubtok_firewall_platter_master") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_glubtok_firewall_platter_masterAI(creature);
    }

    struct npc_glubtok_firewall_platter_masterAI : public ScriptedAI
    {
        npc_glubtok_firewall_platter_masterAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
        {
            ASSERT(instance);
        }

        InstanceScript* instance;
        EventMap events;

        void JustSummoned(Creature* summoned)
        {
            if (Creature* glubtok = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_GLUBTOK)))
                if (glubtok->IsAIEnabled)
                    glubtok->AI()->JustSummoned(summoned);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_INSTALL_WALL_LEVEL_ONE, 100);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INSTALL_WALL_LEVEL_ONE:
                        {
                            Position pos;
                            me->GetPosition(&pos);

                            for (int i = 0; i < 8; ++i)
                                if (Creature* wall = me->SummonCreature(Wall[i], pos))
                                    wall->EnterVehicle(me, i);

                            events.ScheduleEvent(EVENT_INSTALL_WALL_LEVEL_TWO, 1000);
                        }
                        break;
                    case EVENT_INSTALL_WALL_LEVEL_TWO:
                        {
                            Vehicle* vehicle = me->GetVehicleKit();

                            if (!vehicle)
                                return;

                            for (int j = 0; j < 8; ++j)
                                if (Unit* passenger = vehicle->GetPassenger(j))
                                {
                                    Position pos;
                                    passenger->GetPosition(&pos);

                                    for (int i = 0; i < 3; ++i)
                                        if (passenger->GetEntry() == WallLvl[i].EntryLevel_1)
                                            if (/*Vehicle* vehicle = */passenger->GetVehicleKit())
                                            {
                                                uint8 seats[4] = {0, 1, 4, 5};

                                                for (int k = 0; k < 4; ++k)
                                                    if (Creature* wall = me->SummonCreature(WallLvl[i].EntryLevel_2, pos))
                                                        wall->EnterVehicle(passenger, seats[k]);
                                            }
                                }

                            events.ScheduleEvent(EVENT_ENABLE_WALL_LEVEL_TWO, 1000);
                        }
                        break;
                    case EVENT_ENABLE_WALL_LEVEL_TWO:
                        {
                            Vehicle* vehicle = me->GetVehicleKit();

                            if (!vehicle)
                                return;

                            Position pos;
                            me->GetPosition(&pos);

                            for (int j = 0; j < 8; ++j)
                                if (Unit* passenger = vehicle->GetPassenger(j))
                                    if (Vehicle* acc_veh = passenger->GetVehicleKit())
                                        for (SeatMap::const_iterator itr = acc_veh->Seats.begin(); itr != acc_veh->Seats.end(); ++itr)
                                            if (Unit* wall = ObjectAccessor::GetUnit(*me, itr->second.Passenger))
                                                wall->CastSpell(wall, SPELL_FIRE_WALL, false);

                            events.ScheduleEvent(EVENT_CHANGE_ANGLE, 100);
                        }
                        break;
                    case EVENT_CHANGE_ANGLE:
                        {
                            float x, y, z, o;
                            me->GetPosition(x, y, z, o);

                            if (o < -M_PI)
                                o += M_PI * 2;

                            me->NearTeleportTo(x, y, z, o - 0.025f, true);
                            events.ScheduleEvent(EVENT_CHANGE_ANGLE, 100);
                        }
                        break;
                }
            }
        }
    };
};

class npc_glubtok_firewall_damager : public CreatureScript
{
public:
    npc_glubtok_firewall_damager() : CreatureScript("npc_glubtok_firewall_damager") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_glubtok_firewall_damagerAI (creature);
    }

    struct npc_glubtok_firewall_damagerAI : public ScriptedAI
    {
        npc_glubtok_firewall_damagerAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
        {
            ASSERT(instance);
        }

        InstanceScript* instance ;

        void SpellHitTarget(Unit * victim, const SpellInfo * spell)
        {
            if (spell->Id == 91397)
                if (Creature* glubtok = Unit::GetCreature(*me, instance->GetData64(DATA_GLUBTOK)))
                    if (glubtok->IsAIEnabled)
                        glubtok->AI()->SetGUID(victim->GetGUID(), DATA_PLAYER_HIT);
        }
    };
};

class spell_glubtok_teleport : public SpellScriptLoader
{
    public:
        spell_glubtok_teleport() : SpellScriptLoader("spell_glubtok_teleport") { }

        class spell_glubtok_teleport_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_glubtok_teleport_SpellScript)

            void Teleport(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();

                if (!caster)
                    return;

                caster->NearTeleportTo(ArcanePowerPos.GetPositionX(), ArcanePowerPos.GetPositionY(),
                    ArcanePowerPos.GetPositionZ(), ArcanePowerPos.GetOrientation(), true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_glubtok_teleport_SpellScript::Teleport, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_glubtok_teleport_SpellScript();
        }
};

class spell_glubtok_blink : public SpellScriptLoader
{
    public:
        spell_glubtok_blink() : SpellScriptLoader("spell_glubtok_blink") { }

        class spell_glubtok_blink_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_glubtok_blink_SpellScript)

            bool Load()
            {
                if (Unit* caster = GetCaster())
                    return caster->ToCreature();

                return false;
            }

            void JumpToTarget(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                Unit* target = GetExplTargetUnit();
                Creature* glubtok = NULL;

                if (caster)
                    glubtok = caster->ToCreature();

                if (!(glubtok && target))
                    return;

                Position pos;
                target->GetRandomNearPosition(pos, 3.5f);
                glubtok->AttackStop();
                glubtok->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(),
                    pos.GetPositionZ(), pos.GetOrientation(), true);

                if (glubtok->IsAIEnabled)
                    glubtok->AI()->AttackStart(target);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_glubtok_blink_SpellScript::JumpToTarget, EFFECT_0, SPELL_EFFECT_TELEPORT_UNITS);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_glubtok_blink_SpellScript();
        }
};

class achievement_ready_for_raiding : public AchievementCriteriaScript
{
    public:
        achievement_ready_for_raiding() : AchievementCriteriaScript("achievement_ready_for_raiding") { }

        bool OnCheck(uint32 /*criteriaId*/, uint64 /*miscValue*/, Player* source, Unit* target)
        {
            if (!target)
                return false;

            boss_glubtok::boss_glubtokAI* glubtokAI = CAST_AI(boss_glubtok::boss_glubtokAI, target->GetAI());

            if (!glubtokAI)
                return false;

            std::set<uint64>::const_iterator itr = glubtokAI->lPlayersOnFire.find(source->GetGUID());
            return itr == glubtokAI->lPlayersOnFire.end();
        }
};

void AddSC_boss_glubtok()
{
    new boss_glubtok();
    new npc_glubtok_firewall_platter_master();
    new npc_glubtok_firewall_damager();

    new spell_glubtok_teleport();
    new spell_glubtok_blink();

    new achievement_ready_for_raiding();
}
