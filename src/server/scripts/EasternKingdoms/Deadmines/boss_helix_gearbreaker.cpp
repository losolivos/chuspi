#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Vehicle.h"
#include "deadmines.h"

enum eSpells
{
    SPELL_ATTACK_THROWN                   = 88374,
    SPELL_VEHICLE_SWITCH_TO_SEAT_3        = 84225,
    SPELL_HELIX_RIDE                      = 88337,
    SPELL_HELIX_RIDE_FACE_TIMER_AURA      = 88351,
    SPELL_RIDE_VEHICLE                    = 52391,
    SPELL_EMOTE_TALK                      = 79506,
    SPELL_SUICIDE                         = 51744,
    SPELL_HOLD_THROWN                     = 88373,
    SPELL_RIDE_FACE_TARGETTING            = 88349,
    SPELL_OAF_GRAB_TARGETTING             = 88289,
    SPELL_ARMING_VISUAL_YELLOW            = 88315,
    SPELL_ARMING_VISUAL_RED               = 88317,
    SPELL_ARMING_VISUAL_ORANGE            = 88316,
    SPELL_STICKY_BOMB_ARMED_STATE         = 88319,
    SPELL_STICKY_BOMB_PERIODIC_TRIGGER    = 88329,
    SPELL_EXPLODE                         = 88974,
    SPELL_THROW_BOMB_TARGETTING           = 88268,
    SPELL_CHEST_BOMB_PERIODIC             = 88352,
    SPELL_FORCE_CAST_EJECT_PASSENGER_1    = 88353,
    SPELL_CHEST_BOMB                      = 88250,
    SPELL_CHARGE                          = 88295,
    SPELL_BERSERK                         = 47008,
    SPELL_OAFGUARD                        = 90546,
};

enum eCreatures
{
    NPC_HELIX_CREW_1                      = 49136,
    NPC_HELIX_CREW_2                      = 49137,
    NPC_HELIX_CREW_3                      = 49138,
    NPC_HELIX_CREW_4                      = 49139,
    NPC_LUMBERING_OAF                     = 47297,
};

enum eScriptTexts
{
    // Helix
    HELIX_YELL_DIED                       = 0,
    HELIX_YELL_KILL_PLAYER                = 1,
    HELIX_YELL_OAF_DIED                   = 2,
    HELIX_YELL_BOOM                       = 3,
    HELIX_YELL_OAF_PUSH                   = 4,
    HELIX_YELL_START                      = 5,
    HELIX_EMOTE_CHEST_BOMB                = 6,
    // Crew
    HELIX_CREW_SAY_BOOM                   = 0,
    // Oaf
    OAF_YELL_SMASH                        = 0,
    OAF_YELL_NO                           = 1,
    OAF_YELL_DIED                         = 2,
};

enum eActions
{
    ACTION_EXIT_PLAYER                    = 1,
    ACTION_OAF_DIED                       = 2,
};

enum eEvents
{
    // Helix
    EVENT_HELIX_RIDE_1                    = 1,
    EVENT_HELIX_RIDE_2                    = 2,
    EVENT_THROW_BOMB                      = 3,
    EVENT_CHECK_VALID_TARGET              = 4,
    EVENT_SUMMON_HELIX_CREW               = 5,
    EVENT_SUMMON_MORE_HELIX_CREW          = 7,
    EVENT_HELIX_RIDE_PLAYER               = 8,
    // Oaf
    EVENT_OAF_SMASH                       = 6,
    EVENT_OAF_SMASH_CHANGE_ANGLE          = 7,
    EVENT_OAF_CHECK_REACT_STATE           = 8,
    // Bomb
    EVENT_ARMING_YELLOW                   = 1,
    EVENT_ARMING_ORANGE                   = 2,
    EVENT_ARMING_RED                      = 3,
    EVENT_ARMING_ARMED                    = 4,
    EVENT_BOOM                            = 5,
    EVENT_CHECK_PLAYER                    = 6,
};

enum ePoints
{
    POINT_CHARGE                          = 1,
};

const Position OafSpawnPos = {-299.693f, -514.074f, 51.012f, 0.789456f};

const Position OafSmashStartPos = {-289.869f, -518.859f, 49.5425f, 1.59f};

const Position OafSmashPos = {-289.5868f, -489.5746f, 49.91263f, 1.54f};

const uint32 HelixCrewEntry[4]=
{
    NPC_HELIX_CREW_1,
    NPC_HELIX_CREW_2,
    NPC_HELIX_CREW_3,
    NPC_HELIX_CREW_4,
};

const Position HelixCrewSpawnPos[8]=
{
    {-283.843f, -503.369f, 60.512f, 1.90240f},
    {-285.868f, -503.826f, 60.553f, 5.00909f},
    {-292.678f, -503.727f, 60.273f, 4.46804f},
    {-289.831f, -503.406f, 60.363f, 1.81514f},
    {-291.078f, -504.093f, 60.251f, 4.60766f},
    {-294.163f, -503.232f, 60.125f, 1.86750f},
    {-282.263f, -503.833f, 60.602f, 5.20108f},
    {-287.474f, -503.270f, 60.448f, 1.67551f},
};

class boss_helix_gearbreaker : public CreatureScript
{
public:
    boss_helix_gearbreaker() : CreatureScript("boss_helix_gearbreaker") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_helix_gearbreakerAI (creature);
    }

    struct boss_helix_gearbreakerAI : public BossAI
    {
        boss_helix_gearbreakerAI(Creature* creature) : BossAI(creature, DATA_HELIX_GEARBREAKER) { }

        uint64 oafGUID;
        bool OafDied;
        bool HelixDied;

        void InitializeAI()
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            oafGUID = 0;
            Reset();
            JustReachedHome();
        }

        void Reset()
        {
            if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
            {
                oafGUID = 0;
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, oaf);
            }

            _Reset();
            OafDied = false;
            HelixDied = false;
            me->AddAura(SPELL_OAFGUARD, me);
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void JustReachedHome()
        {
            if (Creature* oaf = me->SummonCreature(NPC_LUMBERING_OAF, OafSpawnPos))
            {
                oafGUID = oaf->GetGUID();
                me->CastSpell(oaf, SPELL_RIDE_VEHICLE, true);
            }
        }

        void EnterEvadeMode()
        {
            me->ExitVehicle();
            BossAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(HELIX_YELL_KILL_PLAYER);
        }

        void WillEnterVehicle(Unit* vehicle, Position& enterPos, int8 /*seatId*/)
        {
            if (vehicle->GetTypeId() == TYPEID_PLAYER)
            {
                Position offset = {0.0f, 0.0f, 0.0f, M_PI};
                me->m_movementInfo.t_pos.RelocateOffset(offset);
                enterPos.RelocateOffset(offset);
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(HELIX_YELL_START);
            events.ScheduleEvent(EVENT_THROW_BOMB, urand(2000, 7000));
            events.ScheduleEvent(EVENT_OAF_SMASH, 5000);
            events.ScheduleEvent(EVENT_CHECK_VALID_TARGET, 1000);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, oaf);

            if (IsHeroic())
                events.ScheduleEvent(EVENT_SUMMON_HELIX_CREW, 5000);
        }

        void JustSummoned(Creature* summoned)
        {
            summons.Summon(summoned);

            if (me->IsInCombat())
                summoned->SetInCombatWithZone();
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_EXIT_PLAYER:
                    {
                        if (OafDied)
                        {
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            events.ScheduleEvent(EVENT_HELIX_RIDE_PLAYER, 1000);
                        }
                        else
                        {
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            events.ScheduleEvent(EVENT_OAF_SMASH, urand(10000, 20000));

                            if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
                                if (oaf->IsAlive())
                                    me->CastSpell(oaf, SPELL_RIDE_VEHICLE, true);
                        }
                    }
                    break;
                case ACTION_OAF_DIED:
                    {
                        OafDied = true;
                        Talk(HELIX_YELL_OAF_DIED);
                        me->CastSpell(me, SPELL_EMOTE_TALK, true);
                        me->RemoveAura(SPELL_OAFGUARD);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        events.Reset();
                        events.ScheduleEvent(EVENT_HELIX_RIDE_PLAYER, 1500);
                        events.ScheduleEvent(EVENT_THROW_BOMB, urand(2000, 7000));
                        events.ScheduleEvent(EVENT_CHECK_VALID_TARGET, 1000);

                        if (IsHeroic())
                            events.ScheduleEvent(EVENT_SUMMON_MORE_HELIX_CREW, 5000);

                        if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
                        {
                            oafGUID = 0;
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, oaf);
                        }
                    }
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
            {
                oafGUID = 0;
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, oaf);
            }
        }

        void DamageTaken(Unit* done_by, uint32 &damage)
        {
            if (!OafDied && me->GetHealthPct() < 20.0f)
            {
                me->SetHealth(me->GetMaxHealth());

                if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
                {
                    oaf->SetHealth(oaf->GetMaxHealth());
                    oaf->CastSpell(oaf, SPELL_BERSERK, true);
                }
            }

            if (damage >= me->GetHealth() && !HelixDied)
            {
                damage = 0;
                HelixDied = true;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                Talk(HELIX_YELL_DIED);
                me->ExitVehicle();
                me->CastSpell(me, SPELL_SUICIDE, true, NULL, NULL, done_by->GetGUID());
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdatePlayerVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HELIX_RIDE_PLAYER:
                        me->CastSpell((Unit*)NULL, SPELL_HELIX_RIDE, true);
                        me->CastSpell((Unit*)NULL, SPELL_RIDE_FACE_TARGETTING, true);
                        break;
                    case EVENT_HELIX_RIDE_1:
                        {
                            me->CastSpell((Unit*)NULL, SPELL_HELIX_RIDE, true);
                            events.ScheduleEvent(EVENT_HELIX_RIDE_2, 2000);

                            if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
                                if (oaf->IsAlive())
                                {
                                    Talk(HELIX_YELL_OAF_PUSH);
                                    oaf->CastSpell(oaf, SPELL_HOLD_THROWN, true);
                                    me->ExitVehicle();
                                    me->CastSpell(oaf, SPELL_VEHICLE_SWITCH_TO_SEAT_3, true);
                                }
                        }
                        break;
                    case EVENT_HELIX_RIDE_2:
                        {
                            if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
                                if (oaf->IsAlive())
                                {
                                    oaf->RemoveAura(SPELL_HOLD_THROWN);
                                    oaf->CastSpell(oaf, SPELL_ATTACK_THROWN, true);
                                }

                            me->ExitVehicle();
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->CastSpell((Unit*)NULL, SPELL_RIDE_FACE_TARGETTING, true);
                        }
                        break;
                    case EVENT_THROW_BOMB:
                        me->CastSpell(me, SPELL_THROW_BOMB_TARGETTING, true);
                        events.ScheduleEvent(EVENT_THROW_BOMB, urand(2000, 7000));
                        break;
                    case EVENT_OAF_SMASH:
                        {
                            if (Creature* oaf = Unit::GetCreature(*me, oafGUID))
                                if (oaf->IsAlive() && oaf->IsAIEnabled)
                                {
                                    oaf->CastSpell(oaf, SPELL_OAF_GRAB_TARGETTING, true);
                                    oaf->AI()->Talk(OAF_YELL_NO);
                                }

                            events.ScheduleEvent(EVENT_HELIX_RIDE_1, urand(10000, 30000));
                        }
                        break;
                    case EVENT_CHECK_VALID_TARGET:
                        {
                            Unit* target = NULL;

                            if (Unit* vehicle = me->GetVehicleBase())
                            {
                                if (vehicle->GetGUID() == oafGUID)
                                    target = vehicle->GetVictim();
                                else
                                    target = vehicle;
                            }

                            if (target)
                            {
                                if (me->GetVictim() != target)
                                {
                                    me->getThreatManager().resetAllAggro();
                                    me->AddThreat(target, 100.f);
                                    AttackStart(target);
                                }
                            }

                            events.ScheduleEvent(EVENT_CHECK_VALID_TARGET, 1000);
                        }
                        break;
                    case EVENT_SUMMON_HELIX_CREW:
                        {
                            for (int i = 0; i < 4; ++i)
                                me->SummonCreature(HelixCrewEntry[i], HelixCrewSpawnPos[i]);

                            Talk(HELIX_YELL_BOOM);
                        }
                        break;
                    case EVENT_SUMMON_MORE_HELIX_CREW:
                        {
                            for (int i = 4; i < 8; ++i)
                                me->SummonCreature(HelixCrewEntry[i - 4], HelixCrewSpawnPos[i]);

                            Talk(HELIX_YELL_BOOM);
                        }
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_helix_oaf : public CreatureScript
{
    public:
        npc_helix_oaf() : CreatureScript("npc_helix_oaf") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_helix_oafAI (creature);
        }

        struct npc_helix_oafAI : public ScriptedAI
        {
            npc_helix_oafAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset()
            {
                if (Vehicle* vehicle = me->GetVehicleKit())
                    vehicle->RemoveAllPassengers();

                events.Reset();
            }

            void PassengerBoarded(Unit* /*who*/, int8 seatId, bool apply)
            {
                if (seatId == 1)
                {
                    if (apply)
                    {
                        events.ScheduleEvent(EVENT_OAF_CHECK_REACT_STATE, 15000);
                        Talk(OAF_YELL_SMASH);
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->GetMotionMaster()->MovePoint(POINT_CHARGE, OafSmashStartPos);

                        if (me->IsSummon())
                            if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                            {
                                summoner->SetReactState(REACT_PASSIVE);
                                summoner->AttackStop();
                            }
                    }
                    else
                    {
                        uint8 count = urand(3, 8);

                        for (int i = 0; i < count; ++i)
                        {
                            Position pos = OafSmashPos;
                            me->MovePosition(pos, 1.0f * (float)rand_norm(), (float)rand_norm() * static_cast<float>(2 * M_PI));

                            if (Creature* rat = me->SummonCreature(51462, pos, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 25000))
                                rat->GetMotionMaster()->MoveRandom(20.0f);
                        }

                        me->SetReactState(REACT_AGGRESSIVE);

                        if (me->IsSummon())
                            if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                                summoner->SetReactState(REACT_AGGRESSIVE);
                    }
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == POINT_CHARGE)
                    events.ScheduleEvent(EVENT_OAF_SMASH_CHANGE_ANGLE, 350);
            }

            void JustDied(Unit* /*killer*/)
            {
                if (me->IsSummon())
                    if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                        if (summoner->IsAIEnabled)
                            summoner->AI()->DoAction(ACTION_OAF_DIED);

                Talk(OAF_YELL_DIED);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    if (me->IsSummon())
                        if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                            if (summoner->IsAIEnabled)
                                summoner->AI()->Talk(HELIX_YELL_KILL_PLAYER);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_OAF_SMASH:
                            me->CastSpell((Unit*)NULL, SPELL_CHARGE, true);
                            break;
                        case EVENT_OAF_SMASH_CHANGE_ANGLE:
                            me->SetFacingTo(OafSmashStartPos.m_orientation);
                            events.ScheduleEvent(EVENT_OAF_SMASH, 350);
                            break;
                        case EVENT_OAF_CHECK_REACT_STATE:
                            {
                                if (me->GetReactState() == REACT_PASSIVE)
                                {
                                    if (Vehicle* vehicle = me->GetVehicleKit())
                                        if (Unit* passenger = vehicle->GetPassenger(1))
                                            passenger->ExitVehicle();

                                    if (me->IsSummon())
                                        if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                                            summoner->SetReactState(REACT_AGGRESSIVE);

                                    me->CastStop();
                                    me->SetReactState(REACT_AGGRESSIVE);
                                }
                            }
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_chest_bomb : public CreatureScript
{
    public:
        npc_chest_bomb() : CreatureScript("npc_chest_bomb") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_chest_bombAI (creature);
        }

        struct npc_chest_bombAI : public ScriptedAI
        {
            npc_chest_bombAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            bool canBoom;

            void InitializeAI()
            {
                me->SetReactState(REACT_PASSIVE);
                canBoom = false;
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_ARMING_YELLOW, 1000);
            }

            Unit* GetTarget()
            {
                ThreatContainer::StorageType const& threatlist = me->getThreatManager().getThreatList();

                for (ThreatContainer::StorageType::const_iterator i = threatlist.begin(); i != threatlist.end(); ++i)
                    if (Unit* target = ObjectAccessor::GetUnit((*me), (*i)->getUnitGuid()))
                        if (target->GetTypeId() == TYPEID_PLAYER && me->GetExactDist2dSq(target) < 4 && !target->GetVehicle())
                            return target;

                return NULL;
            }

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ARMING_YELLOW:
                            me->CastSpell(me, canBoom ? SPELL_ARMING_VISUAL_RED: SPELL_ARMING_VISUAL_YELLOW, true);
                            events.ScheduleEvent(EVENT_ARMING_ORANGE, 2000);
                            break;
                        case EVENT_ARMING_ORANGE:
                            me->CastSpell(me, canBoom ? SPELL_ARMING_VISUAL_RED: SPELL_ARMING_VISUAL_ORANGE, true);
                            events.ScheduleEvent(EVENT_ARMING_RED, 2000);
                            break;
                        case EVENT_ARMING_RED:
                            me->CastSpell(me, SPELL_ARMING_VISUAL_RED, true);
                            events.ScheduleEvent(canBoom ? EVENT_BOOM : EVENT_ARMING_ARMED, 1500);
                            break;
                        case EVENT_ARMING_ARMED:
                            canBoom = true;
                            me->CastSpell(me, SPELL_STICKY_BOMB_ARMED_STATE, true);
                            events.ScheduleEvent(EVENT_CHECK_PLAYER, 250);
                            events.ScheduleEvent(EVENT_ARMING_YELLOW, 14000);
                            break;
                        case EVENT_BOOM:
                            me->CastSpell(me, SPELL_STICKY_BOMB_PERIODIC_TRIGGER, true);
                            break;
                        case EVENT_CHECK_PLAYER:
                            {
                                if (Unit* target = GetTarget())
                                {
                                    me->CastSpell(target, SPELL_EXPLODE, true);
                                    me->SetDisplayId(11686);
                                    me->DespawnOrUnsummon(2000);
                                    events.Reset();
                                    return;
                                }

                                events.ScheduleEvent(EVENT_CHECK_PLAYER, 250);
                            }
                            break;
                    }
                }
            }
        };
};

class npc_helix_crew : public CreatureScript
{
    public:
        npc_helix_crew() : CreatureScript("npc_helix_crew") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_helix_crewAI (creature);
        }

        struct npc_helix_crewAI : public ScriptedAI
        {
            npc_helix_crewAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()) { }

            InstanceScript* instance;
            EventMap events;

            void InitializeAI()
            {
                ASSERT(instance);
                Talk(HELIX_CREW_SAY_BOOM);
                me->SetReactState(REACT_PASSIVE);
                Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_THROW_BOMB, urand(2000, 7000));
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_THROW_BOMB)
                {
                    me->CastSpell((Unit*)NULL, SPELL_THROW_BOMB_TARGETTING, true, NULL, NULL, instance->GetData64(DATA_HELIX_GEARBREAKER));
                    events.ScheduleEvent(EVENT_THROW_BOMB, urand(2000, 7000));
                }
            }
        };
};

class spell_ride_face_targetting : public SpellScriptLoader
{
    public:
        spell_ride_face_targetting() : SpellScriptLoader("spell_ride_face_targetting") { }

        class spell_ride_face_targetting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ride_face_targetting_SpellScript)

            void TriggerSpell(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!(caster && target))
                    return;

                caster->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, true);
                caster->CastSpell(target, SPELL_HELIX_RIDE_FACE_TIMER_AURA, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_ride_face_targetting_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_ride_face_targetting_SpellScript();
        }
};

class spell_helix_ride_face_timer_aura : public SpellScriptLoader
{
    public:
        spell_helix_ride_face_timer_aura() : SpellScriptLoader("spell_helix_ride_face_timer_aura") { }

        class spell_helix_ride_face_timer_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_helix_ride_face_timer_aura_AuraScript)

            void ScriptEffect(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster()->GetTypeId() != TYPEID_UNIT || !GetCaster()->IsAIEnabled)
                    return;

                if (Unit* target = GetCaster()->GetVehicleBase())
                {
                    if (GetCaster()->GetMap()->GetDifficulty() == HEROIC_DIFFICULTY)
                    {
                        GetCaster()->AddAura(SPELL_CHEST_BOMB_PERIODIC, target);
                        GetCaster()->ToCreature()->AI()->Talk(HELIX_EMOTE_CHEST_BOMB, target->GetGUID());
                    }

                    GetCaster()->ExitVehicle();
                }

                GetCaster()->ToCreature()->AI()->DoAction(ACTION_EXIT_PLAYER);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_helix_ride_face_timer_aura_AuraScript::ScriptEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_helix_ride_face_timer_aura_AuraScript();
        }
};

class spell_eject_passenger : public SpellScriptLoader
{
    public:
        spell_eject_passenger() : SpellScriptLoader("spell_eject_passenger") { }

        class spell_eject_passenger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eject_passenger_SpellScript)

            void EjectPassenger(SpellEffIndex effIndex)
            {
                Unit* target = GetCaster();

                if (!target)
                    return;

                if (Vehicle* vehicle = target->GetVehicleKit())
                    if (Unit* passenger = vehicle->GetPassenger(GetSpellInfo()->Effects[effIndex].BasePoints))
                        passenger->ExitVehicle();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_eject_passenger_SpellScript::EjectPassenger, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_eject_passenger_SpellScript();
        }
};

class spell_chest_bomb : public SpellScriptLoader
{
    public:
        spell_chest_bomb() : SpellScriptLoader("spell_chest_bomb") { }

        class spell_chest_bomb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_chest_bomb_AuraScript)

            void HandleTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* target = GetTarget())
                    target->CastSpell(target, SPELL_CHEST_BOMB, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_chest_bomb_AuraScript::HandleTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_chest_bomb_AuraScript();
        }
};

class spell_helix_script_trigger_standart : public SpellScriptLoader
{
    public:
        spell_helix_script_trigger_standart() : SpellScriptLoader("spell_helix_script_trigger_standart") { }

        class spell_helix_script_trigger_standart_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_helix_script_trigger_standart_SpellScript)

            void TriggerSpell(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!(caster && target))
                    return;

                caster->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, true, NULL, NULL, GetOriginalCaster()->GetGUID());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_helix_script_trigger_standart_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_helix_script_trigger_standart_SpellScript();
        }
};

class spell_sticky_bomb_periodic : public SpellScriptLoader
{
    public:
        spell_sticky_bomb_periodic() : SpellScriptLoader("spell_sticky_bomb_periodic") { }

        class spell_sticky_bomb_periodic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sticky_bomb_periodic_AuraScript)

            void HandleTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* target = GetTarget())
                    if (Creature* bomb = target->ToCreature())
                    {
                        bomb->SetDisplayId(11686);
                        bomb->DespawnOrUnsummon(2000);
                        bomb->RemoveAllAuras();
                    }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sticky_bomb_periodic_AuraScript::HandleTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_sticky_bomb_periodic_AuraScript();
        }
};

class spell_player_to_ride_oaf : public SpellScriptLoader
{
    public:
        spell_player_to_ride_oaf() : SpellScriptLoader("spell_player_to_ride_oaf") { }

        class spell_player_to_ride_oaf_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_player_to_ride_oaf_SpellScript)

            void TriggerSpell(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitPlayer();

                if (!(caster && target))
                    return;

                target->CastSpell(caster, GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_player_to_ride_oaf_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_player_to_ride_oaf_SpellScript();
        }
};

void AddSC_boss_helix_gearbreaker()
{
    new boss_helix_gearbreaker();
    new npc_helix_oaf();
    new npc_chest_bomb();
    new npc_helix_crew();

    new spell_ride_face_targetting();
    new spell_helix_ride_face_timer_aura();
    new spell_chest_bomb();
    new spell_helix_script_trigger_standart();
    new spell_sticky_bomb_periodic();
    new spell_eject_passenger();
    new spell_player_to_ride_oaf();
}
