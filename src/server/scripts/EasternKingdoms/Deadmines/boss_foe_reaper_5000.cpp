#include "ScriptPCH.h"
#include "deadmines.h"

enum eSpells
{
    SPELL_ZERO_POWER                            = 87239,
    SPELL_SAFETY_RESTRICTIONS_OFF_LINE          = 88522,
    SPELL_ELECTRIC_CHARGE                       = 89202,
    SPELL_ON_LINE                               = 89198,
    SPELL_RED_EYES                              = 24263,
    SPELL_OFF_LINE                              = 88348,
    SPELL_ON_LINE_VISUAL                        = 89121,
    SPELL_ENERGIZE                              = 91846,
    SPELL_ACQUIRING_TARGET                      = 88492,
    SPELL_HARVEST                               = 88497,
    SPELL_HARVEST_SWEEP                         = 88521,
    SPELL_REAPER_STRIKE                         = 88490,
    SPELL_OVERDRIVE                             = 88481,
    SPELL_FIXATE                                = 91830,
    SPELL_FIXATE_TAUNT                          = 91829,
    SPELL_MOLTEN_SHIELD                         = 91815,
    SPELL_FIXATE_TARGETING                      = 91831,
    SPELL_WATCH_TARGETING                       = 90100,
    SPELL_DEFIAS_REAPER_STRIKE                  = 91723,
    SPELL_REAPER_CHARGE                         = 91726,
    SPELL_ENERGIZE_25                           = 89132,
    SPELL_ON_FIRE                               = 91737,
    SPELL_EXPLOSIVE_SUICIDE                     = 91738,
    SPELL_ENERGIZE_ENRAGE                       = 90978,
    SPELL_CLEAVE                                = 90980,
};

enum eCreatures
{
    NPC_DEFIAS_WATCHER                          = 47404,
    NPC_DEFIAS_REAPER                           = 47403,
    NPC_PROTOTYPE_REAPER                        = 49208,
    NPC_GP_BUNNY_JMF_2                          = 47242,
    NPC_GP_BUNNY_JMF_1                          = 45979,
    NPC_HARVEST_TARGETING_BUNNY                 = 47468,
};

enum eScriptTexts
{
    FOE_REAPER_5000_YELL_ENABLED                = 0,
    FOE_REAPER_5000_YELL_DIED                   = 1,
    FOE_REAPER_5000_YELL_KILL_PLAYER            = 2,
    FOE_REAPER_5000_YELL_SPELL_START_HARVEST    = 3,
    FOE_REAPER_5000_YELL_SPELL_GO_HARVEST       = 4,
    FOE_REAPER_5000_YELL_OVERDRIVE              = 5,
    FOE_REAPER_5000_YELL_ENRAGE                 = 6,
    FOE_REAPER_5000_EMOTE_ACTIVATE_PROTOTYPE    = 7,
    FOE_REAPER_5000_EMOTE_OVERDRIVE             = 8,
    FOE_REAPER_5000_EMOTE_ENRAGE                = 9,
    SLAG_SUMMONER_EMOTE_SUMMONED                = 10,
    SLAG_SUMMONER_EMOTE_SUMMON                  = 1,
};

enum eEvents
{
    // Foe Reaper 5000
    EVENT_ENERGIZE_VISUAL_1                     = 1,
    EVENT_ENERGIZE_VISUAL_2                     = 2,
    EVENT_ENERGIZE_VISUAL_3                     = 3,
    EVENT_RED_EYE                               = 4,
    EVENT_ON_LINE                               = 5,
    EVENT_HARVEST                               = 6,
    EVENT_GOOD_HARVEST                          = 7,
    EVENT_REAPER_STRIKE                         = 8,
    EVENT_OVERDRIVE_START                       = 9,
    EVENT_OVERDRIVE_FINISH                      = 10,
    EVENT_ACTIVATED                             = 11,
    // Defias Watcher & Defias Reaper
    EVENT_DW_WATCHING_ENEMY                     = 1,
    EVENT_DW_CLEAVE                             = 2,
    EVENT_ENERGIZE                              = 5,
};

enum eActions
{
    ACTION_ENERGIZE                             = 1,
};

enum eData
{
    DATA_ACHIEVEMENT_PROGRESS                   = 1,
};

enum ePoints
{
    POINT_HARVEST                               = 1,
    POINT_OVERDRIVE                             = 2,
};

struct BeamSpawnPos
{
    Position CasterPos;
    Position TargetPos;
};

const BeamSpawnPos BeamPos[8]=
{
    {{-208.258f, -567.291f, 21.06013f, 0.0f},{-205.199f, -561.354f, 21.06013f, 0.0f}},
    {{-208.552f, -568.121f, 25.20743f, 0.0f},{-202.276f, -571.060f, 21.06013f, 0.0f}},
    {{-211.880f, -568.484f, 23.04893f, 0.0f},{-216.793f, -571.972f, 21.06013f, 0.0f}},
    {{-209.265f, -569.715f, 27.31943f, 0.0f},{-208.491f, -576.697f, 27.95013f, 0.0f}},
    {{-212.121f, -570.392f, 26.43263f, 0.0f},{-218.965f, -565.441f, 21.06013f, 0.0f}},
    {{-206.163f, -568.848f, 24.63523f, 0.0f},{-200.029f, -565.434f, 21.06013f, 0.0f}},
    {{-204.349f, -596.536f, 22.56110f, 0.0f},{-208.445f, -581.790f, 27.86680f, 0.0f}},
    {{-202.342f, -595.708f, 21.80260f, 0.0f},{-203.113f, -581.688f, 27.86680f, 0.0f}},
};

const Position DWatcherPos[2]=
{
    {-229.724f, -590.372f, 19.3898f, 0.71558f},
    {-205.535f, -552.747f, 19.3898f, 4.53786f},
};

const Position DReaperPos[2]=
{
    {-182.743f, -565.969f, 19.3898f, 3.35103f},
    {-228.675f, -565.753f, 19.3898f, 5.98648f},
};

const Position ReaperHomePos = {-209.837f, -568.622f, 21.0601f, 1.97222f};

const Position PrototypeHomePos = {-202.923f, -595.974f, 20.9769f, 1.65336f};

const float SlagSummonPos[4][3]=
{
    {-205.750f, -584.432f, 21.4223f},
    {-210.822f, -579.180f, 21.4232f},
    {-200.573f, -579.406f, 21.4232f},
    {-205.583f, -574.135f, 21.4232f},
};

class boss_foe_reaper_5000 : public CreatureScript
{
public:
    boss_foe_reaper_5000() : CreatureScript("boss_foe_reaper_5000") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_foe_reaper_5000AI (creature);
    }

    struct boss_foe_reaper_5000AI : public BossAI
    {
        boss_foe_reaper_5000AI(Creature* creature) : BossAI(creature, DATA_FOE_REAPER_5000) { }

        EventMap intro_events;
        uint64 SlagSummonerGUID;
        uint64 PrototypeGUID;
        uint32 SummonSlagTimer;
        uint32 MoveTimer;
        uint8 EnergizeCount;
        uint8 AchievementProgress;
        bool Harvest;
        bool Enrage;
        bool Move;

        void InitializeAI()
        {
            me->setPowerType(POWER_ENERGY);
            Reset();
            JustReachedHome();
        }

        void Reset()
        {
            _Reset();
            intro_events.Reset();
            Move = false;
            Enrage = false;
            Harvest = false;
            MoveTimer = 3000;
            EnergizeCount = 0;
            SummonSlagTimer = 5000;
            PrototypeGUID = 0;
            SlagSummonerGUID = 0;
            AchievementProgress = DONE;
            me->SetPower(POWER_ENERGY, 0);
            me->CastSpell(me, SPELL_ZERO_POWER, false);
            me->CastSpell(me, SPELL_OFF_LINE, false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            if (Creature* summoner = Unit::GetCreature(*me, SlagSummonerGUID))
                summoner->DespawnOrUnsummon();
        }

        void JustReachedHome()
        {
            for (int i = 0; i < 2; ++i)
            {
                me->SummonCreature(NPC_DEFIAS_WATCHER, DWatcherPos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                me->SummonCreature(NPC_DEFIAS_REAPER, DReaperPos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
            }

            if (Creature* Prototype = me->SummonCreature(NPC_PROTOTYPE_REAPER, PrototypeHomePos))
            {
                PrototypeGUID = Prototype->GetGUID();
                Prototype->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                Prototype->CastSpell(me, SPELL_ZERO_POWER, false);
                Prototype->CastSpell(me, SPELL_OFF_LINE, false);
                Prototype->SetPower(POWER_ENERGY, 0);
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            if (type == DATA_ACHIEVEMENT_PROGRESS)
                AchievementProgress = data;
        }

        uint32 GetData(uint32 type)
        {
            if (type == DATA_ACHIEVEMENT_PROGRESS)
                return AchievementProgress;

            return 0;
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_ENERGIZE)
            {
                ++EnergizeCount;

                if (EnergizeCount == 4)
                {
                    EnergizeCount = 0;
                    intro_events.ScheduleEvent(EVENT_ENERGIZE_VISUAL_1, 1000);
                }
            }
        }

        void JustSummoned(Creature* summoned)
        {
            switch (summoned->GetEntry())
            {
                case NPC_GP_BUNNY_JMF_1:
                case NPC_GP_BUNNY_JMF_2:
                    return;
                case NPC_HARVEST_TARGETING_BUNNY:
                    Talk(FOE_REAPER_5000_YELL_SPELL_GO_HARVEST);
                    Harvest = true;
                    float x, y, z;
                    summoned->GetPosition(x, y, z);
                    me->GetMotionMaster()->MovePoint(POINT_HARVEST, x, y, z);
                    break;
            }

            summons.Summon(summoned);
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            events.ScheduleEvent(EVENT_REAPER_STRIKE, 2500);
            events.ScheduleEvent(EVENT_OVERDRIVE_START, 3500);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            if (Creature* summoner = me->SummonCreature(NPC_GP_BUNNY_JMF_1, ReaperHomePos))
            {
                SlagSummonerGUID = summoner->GetGUID();
                Talk(SLAG_SUMMONER_EMOTE_SUMMONED);
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (id)
            {
                case POINT_HARVEST:
                    events.DelayEvents(2100);
                    events.ScheduleEvent(EVENT_GOOD_HARVEST, 2000);
                    me->ClearUnitState(UNIT_STATE_CASTING);
                    Harvest = false;
                    break;
                case POINT_OVERDRIVE:
                    {
                        if (me->GetReactState() == REACT_PASSIVE)
                            Move = true;
                    }
                    break;
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(FOE_REAPER_5000_YELL_KILL_PLAYER);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            Talk(FOE_REAPER_5000_YELL_DIED);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            if (Creature* summoner = Unit::GetCreature(*me, SlagSummonerGUID))
                summoner->DespawnOrUnsummon();
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
        {
            if (me->GetHealthPct() < 40.0f && !Enrage)
            {
                Enrage = true;
                Talk(FOE_REAPER_5000_EMOTE_ENRAGE);
                Talk(FOE_REAPER_5000_YELL_ENRAGE);
                me->CastSpell(me, SPELL_SAFETY_RESTRICTIONS_OFF_LINE, false);
            }
        }

        void SpellHitTarget(Unit* /*target*/, const SpellInfo* spell)
        {
            switch(spell->Id)
            {
                case SPELL_OVERDRIVE:
                    Talk(FOE_REAPER_5000_YELL_OVERDRIVE);
                    break;
                case SPELL_ACQUIRING_TARGET:
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    Talk(FOE_REAPER_5000_YELL_SPELL_START_HARVEST);
                    break;
            }
        }

        void SummonBeam(uint8 first, uint8 last)
        {
            for (uint32 i = first; i <= last; ++i)
                if (Creature* caster = me->SummonCreature(NPC_GP_BUNNY_JMF_2, BeamPos[i].CasterPos, TEMPSUMMON_TIMED_DESPAWN, 2000))
                    if (Creature* target = me->SummonCreature(NPC_GP_BUNNY_JMF_2, BeamPos[i].TargetPos, TEMPSUMMON_TIMED_DESPAWN, 2000))
                        caster->CastSpell(target, SPELL_ELECTRIC_CHARGE, false);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                intro_events.Update(diff);

                if (uint32 eventId = intro_events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ENERGIZE_VISUAL_1:
                            SummonBeam(0, 0);
                            me->CastSpell(me, SPELL_ON_LINE, false);
                            intro_events.ScheduleEvent(EVENT_ENERGIZE_VISUAL_2, 2000);
                            break;
                        case EVENT_ENERGIZE_VISUAL_2:
                            SummonBeam(1, 2);
                            intro_events.ScheduleEvent(EVENT_ENERGIZE_VISUAL_3, 3000);
                            break;
                        case EVENT_ENERGIZE_VISUAL_3:
                            SummonBeam(3, 5);
                            intro_events.ScheduleEvent(EVENT_RED_EYE, 4000);
                            break;
                        case EVENT_RED_EYE:
                            me->CastSpell(me, SPELL_RED_EYES, false);
                            intro_events.ScheduleEvent(EVENT_ON_LINE, 4000);
                            break;
                        case EVENT_ON_LINE:
                            {
                                if (IsHeroic())
                                {
                                    Talk(FOE_REAPER_5000_EMOTE_ACTIVATE_PROTOTYPE);
                                    SummonBeam(6, 7);

                                    if (Creature* Prototype = Unit::GetCreature(*me, PrototypeGUID))
                                    {
                                        Prototype->RemoveAura(SPELL_OFF_LINE);
                                        Prototype->SetPower(POWER_ENERGY, 100);
                                        Prototype->CastSpell(Prototype, SPELL_ON_LINE_VISUAL, false);
                                    }
                                }

                                intro_events.ScheduleEvent(EVENT_ACTIVATED, 5000);
                                Talk(FOE_REAPER_5000_YELL_ENABLED);
                                SummonBeam(3, 5);
                                me->RemoveAura(SPELL_OFF_LINE);
                                me->CastSpell(me, SPELL_ON_LINE_VISUAL, false);
                                Position pos;
                                me->GetPosition(&pos);

                                if (Creature* beam = me->SummonCreature(NPC_GP_BUNNY_JMF_1, pos, TEMPSUMMON_TIMED_DESPAWN, 2000))
                                    beam->CastSpell(me, SPELL_ENERGIZE, false);
                            }
                            break;
                        case EVENT_ACTIVATED:
                            {
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                                if (Creature* Prototype = Unit::GetCreature(*me, PrototypeGUID))
                                {
                                    Prototype->Respawn();
                                    Prototype->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                                }
                            }
                            break;
                    }
                }

                return;
            }

            events.Update(diff);

            if (Move)
            {
                if (MoveTimer <= diff)
                {
                    MoveTimer = 100;
                    Move = false;

                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 50.0f, true))
                    {
                        float x, y, z;
                        target->GetPosition(x, y, z);
                        me->GetMotionMaster()->MovePoint(POINT_OVERDRIVE, x, y, z);
                    }
                }
                else
                    MoveTimer -= diff;
            }

            if (SummonSlagTimer <= diff)
            {
                if (Enrage)
                    SummonSlagTimer = 12000;
                else
                    SummonSlagTimer = urand(15000, 25000);

                if (!IsHeroic())
                    return;

                Creature* summoner = Unit::GetCreature(*me, SlagSummonerGUID);

                if (!summoner)
                    if ((summoner = me->SummonCreature(NPC_GP_BUNNY_JMF_1, ReaperHomePos)))
                        SlagSummonerGUID = summoner->GetGUID();

                if (summoner)
                {
                    uint8 roll = urand(0, 3);
                    summoner->CastSpell(SlagSummonPos[roll][0], SlagSummonPos[roll][1], SlagSummonPos[roll][2], 91841, false);
                }
            }
            else
                SummonSlagTimer -= diff;

            if (me->HasUnitState(UNIT_STATE_CASTING) || Harvest)
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HARVEST:
                        me->CastSpell(me, SPELL_ACQUIRING_TARGET, true);
                        events.ScheduleEvent(EVENT_OVERDRIVE_START, urand(20000, 50000));
                        break;
                    case EVENT_GOOD_HARVEST:
                        me->SetReactState(REACT_AGGRESSIVE);
                        summons.DespawnEntry(NPC_HARVEST_TARGETING_BUNNY);
                        me->RemoveAura(SPELL_HARVEST);
                        me->CastSpell(me, SPELL_HARVEST_SWEEP, false);
                        break;
                    case EVENT_REAPER_STRIKE:
                        me->CastSpell(me->GetVictim(), SPELL_REAPER_STRIKE, false);
                        events.ScheduleEvent(EVENT_REAPER_STRIKE, 2000);
                        break;
                    case EVENT_OVERDRIVE_START:
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        Move = true;
                        MoveTimer = 3000;
                        me->CastSpell(me, SPELL_OVERDRIVE, false);
                        Talk(FOE_REAPER_5000_EMOTE_OVERDRIVE);
                        events.ScheduleEvent(EVENT_OVERDRIVE_FINISH, 100);
                        break;
                    case EVENT_OVERDRIVE_FINISH:
                        me->StopMoving();
                        Move = false;
                        me->SetReactState(REACT_AGGRESSIVE);
                        events.ScheduleEvent(EVENT_HARVEST, urand(10000, 35000));
                        break;
                }
            }
        }
    };
};


class npc_prototype_reaper : public CreatureScript
{
public:
    npc_prototype_reaper() : CreatureScript("npc_prototype_reaper") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prototype_reaperAI (creature);
    }

    struct npc_prototype_reaperAI : public VehicleAI
    {
        npc_prototype_reaperAI(Creature* creature) : VehicleAI(creature) { }

        bool AchievemetFail;

        void InitializeAI()
        {
            me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
            me->SetReactState(REACT_PASSIVE);
            AchievemetFail = false;
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if (!AchievemetFail && me->HealthBelowPctDamaged(90, damage))
            {
                AchievemetFail = true;
                InstanceScript* instance = me->GetInstanceScript();

                if (instance)
                    if (Creature* reaper = Unit::GetCreature(*me, instance->GetData64(DATA_FOE_REAPER_5000)))
                        if (reaper->IsAIEnabled)
                            reaper->AI()->SetData(DATA_ACHIEVEMENT_PROGRESS, FAIL);
            }
        }

        void PassengerBoarded(Unit* /*who*/, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                InstanceScript* instance = me->GetInstanceScript();

                if (instance)
                    for (int data = DATA_GLUBTOK; data < MAX_ENCOUNTER; ++data)
                        if (instance->GetData(data) == IN_PROGRESS)
                            if (data != DATA_FOE_REAPER_5000)
                                me->Kill(me);
            }
        }
    };
};

class npc_molten_slag : public CreatureScript
{
public:
    npc_molten_slag() : CreatureScript("npc_molten_slag") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_molten_slagAI (creature);
    }

    struct npc_molten_slagAI : public ScriptedAI
    {
        npc_molten_slagAI(Creature* creature) : ScriptedAI(creature)
        {
            Event = true;
            uiEventPhase = 0;
            uiEventTimer = 250;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            InstanceScript* instance = me->GetInstanceScript();
            Talk(SLAG_SUMMONER_EMOTE_SUMMON);

            if (instance)
                if (Creature* reaper = Unit::GetCreature(*me, instance->GetData64(DATA_FOE_REAPER_5000)))
                    if (reaper->IsAIEnabled)
                        reaper->AI()->JustSummoned(me);
        }

        uint32 uiFixateTimer;
        uint32 uiEventTimer;
        uint32 uiEventPhase;
        bool Event;

        void Reset()
        {
            uiFixateTimer = 500;
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_FIXATE)
                target->CastSpell(me, SPELL_FIXATE_TAUNT, true);
        }

        void UpdateAI(uint32 const diff)
        {
            if (Event)
            {
                if (uiEventTimer <= diff)
                {
                    ++uiEventPhase;

                    switch (uiEventPhase)
                    {
                        case 1:
                            uiEventTimer = 1000;
                            me->CastSpell(me, SPELL_MOLTEN_SHIELD, false);
                            break;
                        case 2:
                            Event = false;
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->SetInCombatWithZone();
                            break;
                    }
                }
                else
                    uiEventTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (uiFixateTimer <= diff)
            {
                me->getThreatManager().resetAllAggro();
                me->CastSpell(me, SPELL_FIXATE_TARGETING, false);
                uiFixateTimer = urand(10000, 20000);
            }
            else
                uiFixateTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

class npc_defias_watcher : public CreatureScript
{
public:
    npc_defias_watcher() : CreatureScript("npc_defias_watcher") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_defias_watcherAI (creature);
    }

    struct npc_defias_watcherAI : public ScriptedAI
    {
        npc_defias_watcherAI(Creature* creature) : ScriptedAI(creature) { }

        bool OnFire;
        EventMap events;

        void Reset()
        {
            OnFire = false;
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (!OnFire)
            {
                events.ScheduleEvent(EVENT_DW_WATCHING_ENEMY, urand(2000, 10000));
                events.ScheduleEvent(EVENT_DW_CLEAVE, 2000);
            }
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_WATCH_TARGETING)
            {
                me->getThreatManager().resetAllAggro();
                AttackStart(target);
                me->AddThreat(target, 100500);
            }
        }

        void OnCharmed(bool)
        {
            me->setFaction(35);
        }

        void EnterEvadeMode()
        {
            if (!OnFire)
                CreatureAI::EnterEvadeMode();
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
        {
            if (me->GetHealthPct() <= 30.0f && !OnFire)
            {
                OnFire = true;
                events.Reset();
                me->RemoveAllAuras();
                me->CastSpell(me, SPELL_ENERGIZE_25, true);
                me->CastSpell(me, SPELL_ON_FIRE, true);
                me->SetReactState(REACT_PASSIVE);
                me->DeleteThreatList();
                me->CombatStop();
                me->setFaction(35);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (me->IsSummon())
                if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                    if (summoner->IsAIEnabled)
                        summoner->AI()->DoAction(ACTION_ENERGIZE);

            me->CastSpell(me, SPELL_EXPLOSIVE_SUICIDE, false);
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
                    case EVENT_DW_WATCHING_ENEMY:
                        me->CastSpell(me, SPELL_WATCH_TARGETING, false);
                        events.ScheduleEvent(EVENT_DW_WATCHING_ENEMY, urand(5000, 10000));
                        break;
                    case EVENT_DW_CLEAVE:
                        me->CastSpell(me->GetVictim(), SPELL_CLEAVE, false);
                        DoMeleeAttackIfReady();
                        events.ScheduleEvent(EVENT_DW_CLEAVE, 2000);
                        break;
                }
            }
        }
    };
};

class npc_defias_reaper : public CreatureScript
{
public:
    npc_defias_reaper() : CreatureScript("npc_defias_reaper") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_defias_reaperAI (creature);
    }

    struct npc_defias_reaperAI : public ScriptedAI
    {
        npc_defias_reaperAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;
        bool OnFire;

        void Reset()
        {
            OnFire = false;
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (!OnFire)
            {
                events.ScheduleEvent(EVENT_ENERGIZE, urand(2000, 10000));
                events.ScheduleEvent(EVENT_DW_CLEAVE, 2000);
            }
        }

        void EnterEvadeMode()
        {
            if (!OnFire)
                CreatureAI::EnterEvadeMode();
        }

        void OnCharmed(bool)
        {
            me->setFaction(35);
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
        {
            if (me->GetHealthPct() <= 30.0f && !OnFire)
            {
                OnFire = true;
                events.Reset();
                me->RemoveAllAuras();
                me->CastSpell(me, SPELL_ENERGIZE_25, true);
                me->CastSpell(me, SPELL_ON_FIRE, true);
                me->SetReactState(REACT_PASSIVE);
                me->DeleteThreatList();
                me->CombatStop();
                me->setFaction(35);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (me->IsSummon())
                if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                    if (summoner->IsAIEnabled)
                        summoner->AI()->DoAction(ACTION_ENERGIZE);

            me->CastSpell(me, SPELL_EXPLOSIVE_SUICIDE, false);
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
                switch (eventId)
                {
                    case EVENT_ENERGIZE:
                        me->CastSpell(me, SPELL_ENERGIZE_ENRAGE, false);
                        events.ScheduleEvent(EVENT_ENERGIZE, urand(10000, 20000));
                        break;
                    case EVENT_DW_CLEAVE:
                        me->CastSpell(me->GetVictim(), SPELL_CLEAVE, false);
                        DoMeleeAttackIfReady();
                        events.ScheduleEvent(EVENT_DW_CLEAVE, 2000);
                        break;
                }
            }
        }
    };
};
//  TODO: this is hack
class spell_foe_reaper_5000_on_fire : public SpellScriptLoader
{
    public:
        spell_foe_reaper_5000_on_fire() : SpellScriptLoader("spell_foe_reaper_5000_on_fire") { }

    private:
        class spell_foe_reaper_5000_on_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_foe_reaper_5000_on_fire_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->Kill(caster);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_foe_reaper_5000_on_fire_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE_PERCENT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_foe_reaper_5000_on_fire_AuraScript();
        }
};

class spell_foe_reaper_5000_targeting : public SpellScriptLoader
{
    public:
        spell_foe_reaper_5000_targeting() : SpellScriptLoader("spell_foe_reaper_5000_targeting") { }

        class spell_foe_reaper_5000_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_foe_reaper_5000_targeting_SpellScript)

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
                OnEffectHitTarget += SpellEffectFn(spell_foe_reaper_5000_targeting_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_foe_reaper_5000_targeting_SpellScript();
        }
};

class achievement_prototype_prodigy : public AchievementCriteriaScript
{
    public:
        achievement_prototype_prodigy() : AchievementCriteriaScript("achievement_prototype_prodigy") { }

        bool OnCheck(uint32 /*criteriaId*/, uint64 /*miscValue*/, Player* /*source*/, Unit* target)
        {
            if (!target || !target->IsAIEnabled)
                return false;

            return target->GetAI()->GetData(DATA_ACHIEVEMENT_PROGRESS) == DONE;
        }
};

void AddSC_boss_foe_reaper_5000()
{
    new boss_foe_reaper_5000();
    new npc_prototype_reaper();
    new npc_molten_slag();
    new npc_defias_watcher();
    new npc_defias_reaper();

    new spell_foe_reaper_5000_on_fire();
    new spell_foe_reaper_5000_targeting();

    new achievement_prototype_prodigy();
}
