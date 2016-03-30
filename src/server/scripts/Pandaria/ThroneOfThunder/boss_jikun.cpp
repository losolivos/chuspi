#include "GameObjectAI.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Spell.h"
#include "Vehicle.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CreatureTextMgr.h"
#include "Unit.h"
#include "Player.h"
#include "Creature.h"
#include "InstanceScript.h"
#include "Map.h"
#include "VehicleDefines.h"
#include "SpellInfo.h"

#include "throne_of_thunder.h"

/* 
    NOTE:
    Ji-Kun's young are located throughout the roost and progress through different stages of life. After a period of incubation Young Eggs of Ji-Kun will hatch into Hatchlings. 
    When fed by Ji-Kun, the Hatchlings then mature into Fledglings. Both Fledgling Eggs and Mature Eggs will hatch into Juveniles.
*/

enum Spells
{
    // Ji-Kun.

    // Infected talons
    SPELL_INFECTED_TALONS_AURA          = 140094, // Boss check aura on successful melee ability.
    SPELL_INFECTED_TALONS_DAMAGE        = 140092, // Player debuff.

    // Talon Rake
    SPELL_TALON_RAKE                    = 134366, // Damage + increase.

    // Quills
    SPELL_QUILLS                        = 134380, // Triggers SPELL_QUILLS_DAMAGE damage and SPELL_QUILLS_PERIODIC (Effect 1).
    SPELL_QUILLS_DAMAGE                 = 134381, // Quills damage.
    SPELL_QUILLS_PERIODIC               = 138649, // Periodic dummy - cast of SPELL_QUILLS_VISUAL.
    SPELL_QUILLS_VISUAL                 = 138650, // Quills visual.

    // Down Draft
    SPELL_DOWN_DRAFT                    = 134370, // Areatrigger aura.

    // Caw
    SPELL_CAW                           = 138923, // Script Effect for SPELL_CAW_MISSILE (Effect 0).
    SPELL_CAW_MISSILE                   = 138926, // Triggers SPELL_CAW_DAMAGE.
    SPELL_CAW_DAMAGE                    = 134375, // Damage in 8 yards.

    /*
        Flying mechanic:

        When an egg or hatchling from the lower nests located around Ji-Kun's roost is slain it leaves behind a feather for players to loot. 
        This feather grants the player Daedalian Wings. Daedalian Wings grants the player an extra action button that allows players to take flight for 10 sec.
    */

    // Drop feather
    SPELL_DROP_FEATHER                  = 134338, // 140016 Summons lootable feather.

    // Daedelian Wings
    SPELL_DAEDELIAN_WINGS               = 134339, // 140014 LFR version (No CD trigger). Flight spell + SPELL_LESSON_OF_ICARUS trigger. Stacks added / aura removed using this spell.
    SPELL_LESSON_OF_ICARUS              = 140571, // Cooldown 1 minute Dummy (Effect 0) - Cannot pickup wings.
    SPELL_FLIGHT_ACTION_BUTTON          = 133755, // Action button spell on Daedelian Wings. Removes 1 stack and makes player fly.

    /*
        Feed mechanic:

        Ji-Kun spits up globs of food for her young. If a Hatchling has called out for food, Ji-Kun will aim this ability in their direction; 
        otherwise the food will land at random locations around Ji-Kun's platform. When the food lands on the ground, it forms a Feed Pool.
        While a glob of food is traveling through the air, players with Flight can intercept it to prevent a Feed Pool from forming. 
        Doing so afflicts the player with Slimed, but also grants Primal Nutriment.
    */

    // Feed young
    SPELL_FEED_YOUNG                    = 137528, // Triggers SPELL_REGURGITATE each 1 sec on Normal / 2.5 sec on Heroic.
    SPELL_REGURGITATE                   = 134385, // Script effect (Effect 0) for summoning NPC_FEED_POOL / NPC_FEED_HATCHLINGS.

    // Feed pool
    SPELL_FEED_POOL_DMG                 = 138319, // Periodic damage.
    SPELL_FEED_POOL_VISUAL              = 138854, // Green visual, on platform.
    SPELL_FEED_POOL_VISUAL_HATCHLING    = 139284, // Yellow visual, on hatchlings in nest.
    SPELL_SUMMON_POOL                   = 134259, // Main platform, summons NPC_POOL_OF_FEED.
    SPELL_SUMMON_POOL_HATCHLINGS        = 139285, // Hatchling nest, summons NPC_POOL_OF_FEED_HATCHLING.

    SPELL_SLIMED                        = 134256, // On players intercepting Ji-Kun's food globules. Periodic damage and Script Effect (Effect 1) for SPELL_SLIMED_DMG_INCREASE.
    SPELL_SLIMED_DMG_INCREASE           = 138309, // 10% damage increase from next Slimed.

    SPELL_PRIMAL_NUTRIMENT              = 140741, // On players intercepting Ji-Kun's food globules. Eff 0 + 1 Dummy A. H / D incr., SPELL_PRIMAL_NUTRIMENT_INCREASE trigger on Eff 2.
    SPELL_PRIMAL_NUTRIMENT_INCREASE     = 112879, // 30% Healing and 100% Damage increase.

    // Ji-Kun's Flock.

    // Hatchling

    SPELL_HATCHLING_CHEEP               = 139296, // Triggers SPELL_HATCHLING_CHEEP_DAMAGE in 10 yards.
    SPELL_HATCHLING_CHEEP_DAMAGE        = 139298,
    SPELL_CHEEP_AOE                     = 140129,
    SPELL_INCUBATION_LOW_NEST           = 134347, // 10 sec incubation
    SPELL_INCUBATION_HIGH_NEST          = 134335, // 20 sec incubation

    SPELL_SUMMON_YOUNG_HATCHLING        = 134336,
    SPELL_SUMMON_JUVENILE               = 138905,

    SPELL_HATCHLING_EVOLUTION           = 134322,

    SPELL_HATCHLING_EATING              = 134321,

    // Fledgling

    SPELL_FLEDGLING_LAY_EGG             = 134367, // Summons NPC_FLEDGLING_EGG_JIKUN.
    SPELL_FLEDGLING_CHEEP               = 140570, // Needs target limitation.

    // Juvenile

    SPELL_JUVENILE_CHEEP                = 140227, // Script effect (Effect 0) for SPELL_JUVENILE_CHEEP_DAMAGE in 60 yards.
    SPELL_JUVENILE_CHEEP_DAMAGE         = 140129, // Damage in 10 yards.

    // Nest Guardian - HEROIC!

    SPELL_GUARDIAN_TALON_STRIKE         = 139100, // Weapon damage 180 degree arc.
    SPELL_GUARDIAN_SCREECH              = 140640, // Damage, Interrupt, Script Effect (Effect 2) for SPELL_GUARDIAN_SCREECH_SLOW. If not actively engaged in melee combat.
    SPELL_GUARDIAN_SCREECH_SLOW         = 134372, // Slow casting on targets (Removed after casting any spell). Mana users.

    SPELL_BEAM_VISUAL                   = 137526, // visual aur of beam

    SPELL_EGG_EXPLOSION                 = 142398,

    SPELL_PLAYER_JUMP                   = 149418,
    SPELL_PLAYER_VISUAL_SPIN            = 89428
};

enum FeedMovementPoints
{
    POINT_GREEN_FEED_AIR = 1,
    POINT_GREEN_FEED_GROUND = 2,
    POINT_FEED_CAME_TO_HATCHLING = 3,
};

enum JuvenileMovementPoints
{
    POINT_MOVE_TO_CENTER_FLY = 1,
};

enum HatchlingMovementPoints
{
    POINT_MOVE_TO_EAT_POOL = 1,
};

enum JikunMovementPoints
{
    POINT1 = 1,
    POINT2 = 2,
};

enum EventsJiKun
{
    EVENT_TALON_RAKE    = 1,
    EVENT_QUILLS,
    EVENT_DOWN_DRAFT,
    EVENT_DOWN_DRAFT_FALL,
    EVENT_CAW,
    EVENT_ACTIVATE_NEST,
    EVENT_FEED_YOUNG
};

enum EventsHatchlings
{
    EVENT_FIND_EAT    = 1,
    EVENT_CHEEP,
    EVENT_LAY_EGG,
};

enum EventsJuvenile
{
    EVENT_START_FLY_AWAY    = 1,
    EVENT_JUVENILE_CHEEP,
};

enum EggsJikun
{
    EVENT_SUMMON_HATCHLING    = 1,
    EVENT_CHECK_FOR_PLAYERS   = 2,
};

enum Npcs
{
    // Ji-Kun.
    NPC_YOUNG_EGG_OF_JIKUN     = 68194, // Hatches to Hatchling.
    NPC_FLEDGLING_EGG_JIKUN    = 68202, // Hatches to Juvenile.
    NPC_MATURE_EGG_OF_JIKUN    = 69628, // Hatches to Juvenile.

    NPC_JIKUN_HATCHLING        = 68192,
    NPC_JIKUN_FLEDGLING        = 68193,
    NPC_JIKUN_JUVENILE         = 70095,

    NPC_JIKUN_NEST_GUARDIAN    = 70134, // HEROIC only!

    NPC_FEED_POOL              = 68178, // Feed NPC's.
    NPC_FEED_HATCHLINGS        = 70130,

    NPC_POOL_OF_FEED           = 68188, // Feed Pool NPC's.
    NPC_POOL_OF_FEED_HATCHLING = 70216,

    NPC_BEAM_NEST              = 68208, // Visual beam of active nest
};

// Center
Position Center = { 6146.085f,  4319.261f,  -30.608f, 2.739f };

// Ji-Kun intro movement.
Position const IntroMoving[2] =
{
    { 6213.971f,  4289.072f,  -14.402f, 2.873f },
    { 6146.085f,  4319.261f,  -30.608f, 2.739f }
};

// Ji-Kun low nest positions.
Position const NestPositionsGround[5] =
{
    { 6071.182f,  4285.108f, -101.469f, 1.873f },
    { 6096.028f,  4339.460f,  -93.655f, 1.873f },
    { 6159.814f,  4370.529f,  -70.502f, 1.873f },
    { 6220.071f,  4333.520f,  -57.075f, 1.873f },
    { 6192.708f,  4267.664f,  -70.764f, 1.873f }
};

// Ji-Kun high nest positions.
Position const NestPositionsHigh[5] =
{
    { 6078.422f,  4270.403f,   42.407f, 1.873f },
    { 6082.500f,  4371.428f,   45.238f, 1.873f },
    { 6151.905f,  4330.750f,   72.997f, 1.873f },
    { 6217.987f,  4352.961f,   68.138f, 1.873f },
    { 6173.894f,  4239.375f,   43.848f, 1.873f }
};

uint32 const EggsLowNest = 15; // size of eggs on low nests
uint32 const EggsHighNest = 10;// size of eggs on high nests

Unit* SelectRandomTargetWithGuidOnRange(Unit* who, uint32 entry, float range, bool player)
{
   std::list<Player*> playerList;
   std::list<Creature*> creatureList;

   if (player)
       GetPlayerListInGrid(playerList, who, range);
   else
       GetCreatureListWithEntryInGrid(creatureList, who, entry, range);

   if (player && !playerList.empty())
   {
       if (Player* target = Trinity::Containers::SelectRandomContainerElement(playerList))
           return target->ToUnit();
   }
   else if (!creatureList.empty())
   {
       if (Creature* target = Trinity::Containers::SelectRandomContainerElement(creatureList))
           return target->ToUnit();
   }

    return NULL;
}

bool isSpawner(Unit* unit)
{
    return unit->GetPositionZ() > -32.0f && unit->GetPositionZ() < -30.0f && unit->GetEntry() == NPC_FLEDGLING_EGG_JIKUN;
}

// Ji-Kun - 69712.
class boss_jikun : public CreatureScript
{
    public:
        boss_jikun() : CreatureScript("boss_jikun") { }

        struct boss_jikunAI : public BossAI
        {
            boss_jikunAI(Creature* creature) : BossAI(creature, DATA_JI_KUN), summons(me)
            {
                instance = creature->GetInstanceScript();
                PreIntro();
            }

            uint64 NestLowEggs[5][EggsLowNest],NestHighEggs[5][EggsHighNest];
            InstanceScript* instance;
            EventMap events;
            SummonList summons;
            bool lastNestHigh,introDone;

            /*** SPECIAL FUNCTIONS ***/

            void PreIntro()
            {
                introDone = false;
                me->SetVisible(false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
            }

            void ActivateFeather()
            {
                std::list<GameObject*> jikunFeatherList;
                GetGameObjectListWithEntryInGrid(jikunFeatherList, me, GOB_JIKUN_FEATHER, 200.0f);

                for (auto feather : jikunFeatherList)
                {
                    feather->SetRespawnTime(0);
                    feather->UpdateObjectVisibility();
                }
            }

            void SummonEggs()
            {
                // 5 places of eggs
                for (uint8 i = 0; i < 5; ++i)
                {
                    // bottom places 15 eggs in nest
                    for (uint8 j = 0; j < EggsLowNest; ++j)
                    {
                        Position summonPos;
                        me->GetRandomPoint(NestPositionsGround[i], 7.0f, summonPos);

                        if (Creature* egg = me->SummonCreature(NPC_YOUNG_EGG_OF_JIKUN, summonPos.GetPositionX(), summonPos.GetPositionY(), summonPos.GetPositionZ()))
                            NestLowEggs[i][j] = egg->GetGUID();
                    }

                    // top places 10 eggs in nest
                    for (uint8 c = 0; c < EggsHighNest; ++c)
                    {
                        Position summonPosHigh;
                        me->GetRandomPoint(NestPositionsHigh[i], 6.0f, summonPosHigh);

                        if (Creature* egg = me->SummonCreature(NPC_MATURE_EGG_OF_JIKUN, summonPosHigh.GetPositionX(), summonPosHigh.GetPositionY(), summonPosHigh.GetPositionZ()+3.0f))
                            NestHighEggs[i][c] = egg->GetGUID();
                    }
                }
            }

            void RemoveAllAurasAndDespawnSummons()
            {
                if (instance)
                {
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PRIMAL_NUTRIMENT);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TALON_RAKE);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DAEDELIAN_WINGS);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SLIMED_DMG_INCREASE);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INFECTED_TALONS_DAMAGE);
                }

                DespawnCreature(NPC_YOUNG_EGG_OF_JIKUN);
                DespawnCreature(NPC_MATURE_EGG_OF_JIKUN);
                DespawnCreature(NPC_FLEDGLING_EGG_JIKUN);
                DespawnCreature(NPC_FEED_POOL); // reverse 
                DespawnCreature(NPC_POOL_OF_FEED);
                DespawnCreature(NPC_POOL_OF_FEED_HATCHLING);
                DespawnCreature(NPC_JIKUN_JUVENILE);
                DespawnCreature(NPC_JIKUN_HATCHLING);
            }

            void DespawnCreature(uint32 entry)
            {
                std::list<Creature*> creatureList;
                GetCreatureListWithEntryInGrid(creatureList, me, entry, 250.0f);
                for (auto itr: creatureList)
                {
                    if (isSpawner(itr))
                        continue;

                    itr->DespawnOrUnsummon();
                }
            }

            uint64 GetEggGUID(uint8 NestNumber,bool HighEggs)
            {
                if (!instance)
                    return 0;

                std::list<Creature*> eggList;
                for (uint8 i = 0; i <= (HighEggs ? (EggsHighNest-1) : (EggsLowNest-1)); ++i)
                    if (Creature* egg = instance->instance->GetCreature(HighEggs ? NestHighEggs[NestNumber][i] : NestLowEggs[NestNumber][i]))
                        if (egg->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
                            eggList.push_back(egg);

                if (!eggList.empty())
                    if (Creature* RandomEgg = Trinity::Containers::SelectRandomContainerElement(eggList))
                        return RandomEgg->GetGUID();

                return 0;
            }

            /*** NORMAL FUNCTIONS ***/

            void Reset()
            {
                RemoveAllAurasAndDespawnSummons();

                lastNestHigh = true;

                me->SetDisableGravity(true);
                me->SetCanFly(true);

                events.Reset();

                _Reset();

                SummonEggs();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                //DoAction(0); ?
            }

            void EnterCombat(Unit* who)
            {
                DoCast(me, SPELL_INFECTED_TALONS_AURA);

                events.ScheduleEvent(EVENT_TALON_RAKE, 5000); 
                events.ScheduleEvent(EVENT_QUILLS, 40000);
                events.ScheduleEvent(EVENT_DOWN_DRAFT, 90000);
                events.ScheduleEvent(EVENT_CAW, 16000);
                DoAction(ACTION_ACTIVATE_NEST);

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                _EnterCombat();
            }

			void EnterEvadeMode()
            {
                me->RemoveAllAuras();
                Reset();
                me->DeleteThreatList();
                me->CombatStop(true);
                me->GetMotionMaster()->MoveTargetedHome();

                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FEED_POOL_DMG);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FLIGHT_ACTION_BUTTON);

                me->AddUnitState(UNIT_STATE_EVADE);

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove.

                _EnterEvadeMode();
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
				summon->setActive(true);

				if (me->IsInCombat())
					summon->SetInCombatWithZone();
            }

            void JustDied(Unit* pKiller)
            {
                RemoveAllAurasAndDespawnSummons();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FEED_POOL_DMG);
                summons.DespawnAll();

                if (instance)
                {
                    ActivateFeather();
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                _JustDied();
            }

            void DoAction(const int32 id)
            {
                switch (id)
                {
                    case ACTION_START_INTRO:
                    {
                        if (introDone)
                            return;

                        me->GetMotionMaster()->MovePoint(POINT1, IntroMoving[0]);

                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetVisible(true);
                        me->SetHomePosition(IntroMoving[1]);

                        DoPlaySoundToSet(me, 36213); // pre agro

                        introDone = true;
                        break;
                    }
                    case ACTION_ACTIVATE_NEST:
                    {
                        bool IsHighActivate = lastNestHigh ? false : true;
                        lastNestHigh = IsHighActivate;
                        uint8 NestNumber = urand(0, 4);

                        if (instance)
                        {
                            for (uint8 i = 0; i <= (IsHighActivate ? 1 : 3); ++i)
                            {
                                if (Creature* Egg = instance->instance->GetCreature(GetEggGUID(NestNumber,IsHighActivate)))
                                {
                                    Egg->AddAura(IsHighActivate ? SPELL_INCUBATION_HIGH_NEST : SPELL_INCUBATION_LOW_NEST, Egg);
                                    Egg->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                                }
                            }
                        }

                        me->SummonCreature(NPC_BEAM_NEST,
                                           IsHighActivate ? NestPositionsHigh[NestNumber].GetPositionX() : NestPositionsGround[NestNumber].GetPositionX(),
                                           IsHighActivate ? NestPositionsHigh[NestNumber].GetPositionY() : NestPositionsGround[NestNumber].GetPositionY(),
                                           IsHighActivate ? -31.00f : NestPositionsGround[NestNumber].GetPositionZ() + 7.0f,
                                           0.0f, TEMPSUMMON_TIMED_DESPAWN, 25000);
                        me->MonsterTextEmote(IsHighActivate ? "The eggs in one of the upper nests begin to hatch!" : "The eggs in one of the lower nests begin to hatch!", 0, false);

                        if (!IsHighActivate)
                            events.ScheduleEvent(EVENT_FEED_YOUNG, 12000);

                        events.ScheduleEvent(EVENT_ACTIVATE_NEST, 25000);
                        break;
                    }
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
                    return;

                switch (id)
                {
                    case POINT1: 
                        //float x, float y, float z, float speedXY, float speedZ, uint32 id, float orientation
                        me->GetMotionMaster()->MoveJump(IntroMoving[1].GetPositionX(), IntroMoving[1].GetPositionY(), IntroMoving[1].GetPositionZ(), 15.0f, 15.0f, POINT2, 10.0f);
                        break;
                    case POINT2:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        introDone = true;
                        break;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || !introDone)
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_ACTIVATE_NEST:
                    {
                        DoAction(ACTION_ACTIVATE_NEST);
                        break;
                    }

                    case EVENT_FEED_YOUNG:
                        me->StopMoving();
                        DoCast(me, SPELL_FEED_YOUNG);
                        break;

                    case EVENT_TALON_RAKE:
                        me->StopMoving();
                        if (!me->HasUnitState(UNIT_STATE_CASTING) && me->GetVictim())
                        {
                            DoCast(me->GetVictim(), SPELL_TALON_RAKE);
                            events.ScheduleEvent(EVENT_TALON_RAKE, 20000);
                        }
                        else
                            events.ScheduleEvent(EVENT_TALON_RAKE, 2000);
                        break;

                    case EVENT_QUILLS:
                        me->MonsterTextEmote("|TInterface\\Icons\\ability_hunter_pet_dragonhawk.blp:20|tJi-Kun sends her |cFFFF0000|Hspell:134380|h[Quills]|h|r flying in all directions!", 0, true);
                        me->StopMoving();
                        DoCast(me, SPELL_QUILLS);
                        events.ScheduleEvent(EVENT_QUILLS, 60000);
                        break;

                    case EVENT_DOWN_DRAFT:
                        me->MonsterTextEmote("|TInterface\\Icons\\ability_druid_galewinds.blp:20|tJi-Kun uses her wings to create a massive |cFFFF0000|Hspell:134370|h[Down Draft]|h|r!", 0, true);
                        me->StopMoving();
                        DoCast(SPELL_DOWN_DRAFT);
                        events.ScheduleEvent(EVENT_DOWN_DRAFT, 90000);
                        events.ScheduleEvent(EVENT_DOWN_DRAFT_FALL, 2500);
                        break;

                    case EVENT_DOWN_DRAFT_FALL:
                    {
                        std::list<Player*> list;
                        GetPlayerListInGrid(list, me, 50.0f);
                        for (auto itr: list)
                            if (!itr->HasAura(SPELL_DAEDELIAN_WINGS))
                                itr->NearTeleportTo(itr->GetPositionX() - 3.0f* cos(itr->GetAngle(me)), itr->GetPositionY() - 1.2f* sin(itr->GetAngle(me)) ,itr->GetPositionZ(), itr->GetOrientation());

                        if (me->HasAura(SPELL_DOWN_DRAFT))
                            events.ScheduleEvent(EVENT_DOWN_DRAFT_FALL, 500);
                        break;
                    }
                    case EVENT_CAW:
                        me->MonsterTextEmote("|TInterface\\Icons\\ability_hunter_animalhandler.blp:20|tJi-Kun |cFFFF0000|Hspell:138923|h[Caws]|h|r, sending powerful sound waves at her enemies!", 0, true);
                        me->StopMoving();
                        if (!me->HasUnitState(UNIT_STATE_CASTING))
                        {
                            DoCast(SPELL_CAW);
                            events.ScheduleEvent(EVENT_CAW, 16000);
                        }
                        else
                            events.ScheduleEvent(EVENT_CAW, 2000);
                        break;

                    default: break;
                }

                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_jikunAI(creature);
        }
};

// Beam target - 68208.
class jikun_beam_target : public CreatureScript
{
    public:
        jikun_beam_target() : CreatureScript("jikun_beam_target") { }

        struct jikun_beam_targetAI : public ScriptedAI
        {
            jikun_beam_targetAI(Creature* creature) : ScriptedAI(creature) 
            { 
                me->SetDisableGravity(true);
                me->SetCanFly(true);
                me->SetDisplayId(11686);
                me->setFaction(14);
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->AddAura(SPELL_BEAM_VISUAL, me);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new jikun_beam_targetAI(creature);
        }
};

enum fledglingActions
{
    FLEDGLING_ACTION_LAY_EGG = 1
};

// Young hatchling - 68192.
class young_hatchling_jikun : public CreatureScript
{
    public:
        young_hatchling_jikun() : CreatureScript("young_hatchling_jikun") { }

        struct young_hatchling_jikunAI : public ScriptedAI
        {
            young_hatchling_jikunAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void IsSummonedBy(Unit* summoner)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); // no regen
                me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                me->SetDisableGravity(true);
                me->SetCanFly(true);

                events.ScheduleEvent(EVENT_CHEEP, urand(5000,15000)); 
                events.ScheduleEvent(EVENT_FIND_EAT, urand(5000,7000)); 

                me->SetMaxHealth(summoner->GetMaxHealth());
                me->SetHealth(summoner->GetHealth());
                me->setFaction(summoner->getFaction());

                if (summoner->GetTypeId() == TYPEID_UNIT)
                    summoner->SetVisible(false);
            }

            void DoAction(const int32 id)
            {
                switch (id)
                {
                    case FLEDGLING_ACTION_LAY_EGG:
                    {
                        events.ScheduleEvent(EVENT_LAY_EGG, urand(25000, 35000)); 
                        break;
                    }
                }
            }

            void UpdateAI(const uint32 diff)
            { 
                if (me->GetDistance(me->GetHomePosition()) > 8.0f && me->GetReactState() != REACT_PASSIVE)
                {
                    me->CombatStop(true);
                    me->GetMotionMaster()->MoveTargetedHome();
                }

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_FIND_EAT: // call this event when eat is came to platform and hatchling must start find it
                    {
                        if (Unit* target = SelectRandomTargetWithGuidOnRange(me->ToUnit(), NPC_POOL_OF_FEED_HATCHLING, 10.0f, false))
                            if (Creature* eat = target->ToCreature())
                                if (me->GetReactState() != REACT_PASSIVE)
                                {
                                    me->SetReactState(REACT_PASSIVE);
                                    me->GetMotionMaster()->MovePoint(POINT_MOVE_TO_EAT_POOL,eat->GetPositionX(),eat->GetPositionY(),eat->GetPositionZ()-3.0f);
                                }

                        events.ScheduleEvent(EVENT_FIND_EAT, urand(2000,4000)); 
                        break;
                    }
                    case EVENT_CHEEP:
                        me->StopMoving();
                        if (me->GetVictim() && me->GetReactState() != REACT_PASSIVE)
                            DoCast(me->GetVictim(), me->HasAura(SPELL_HATCHLING_EVOLUTION) ? SPELL_FLEDGLING_CHEEP : SPELL_HATCHLING_CHEEP);
                        events.ScheduleEvent(EVENT_CHEEP, urand(5000,15000));
                        break;
                    case EVENT_LAY_EGG:
                        me->StopMoving();
                        DoCast(SPELL_FLEDGLING_LAY_EGG);
                        events.ScheduleEvent(EVENT_LAY_EGG, urand(40000, 50000)); 
                        break;
                    default: break;
                }

                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    DoMeleeAttackIfReady();
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE) // prevent spam of this function
                    return;

                switch (id)
                {
                    case POINT_MOVE_TO_EAT_POOL: // when came to eat pool must find nearst eat pool and start eat
                    {
                        bool eating = false;
                        if (Unit* target = SelectRandomTargetWithGuidOnRange(me->ToUnit(), NPC_POOL_OF_FEED_HATCHLING, 3.0f, false))
                            if (Creature* eat = target->ToCreature())
                            {
                                eating = true;
                                DoCast(SPELL_HATCHLING_EATING);
                                events.CancelEvent(EVENT_FIND_EAT);
                                eat->DespawnOrUnsummon();
                            }

                        if (!eating)
                            me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    }
                }
            }

            void JustDied(Unit* killer)
            {
                std::list<Player*> PlayerList;
                GetPlayerListInGrid(PlayerList, me, 10.0f);
                for (auto playerTarget : PlayerList)
                    if (!playerTarget->HasAura(SPELL_LESSON_OF_ICARUS))
                    {
                        playerTarget->CastSpell(playerTarget,SPELL_DAEDELIAN_WINGS, true);
                        if (playerTarget->HasAura(SPELL_DAEDELIAN_WINGS))
                            playerTarget->GetAura(SPELL_DAEDELIAN_WINGS)->SetStackAmount(4);
                        break;
                    }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new young_hatchling_jikunAI(creature);
        }
};

// Juvenile - 70095.
class npc_juvenile : public CreatureScript
{
    public:
        npc_juvenile() : CreatureScript("npc_juvenile") { }

        struct npc_juvenileAI : public ScriptedAI
        {
            npc_juvenileAI(Creature* creature) : ScriptedAI(creature) 
            {
            }

            EventMap events;
            
            void IsSummonedBy(Unit* summoner)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); // no regen
                me->SetDisableGravity(true);
                me->SetCanFly(true);
                me->SetReactState(REACT_PASSIVE);

                events.ScheduleEvent(EVENT_START_FLY_AWAY, 4000); 

                me->SetMaxHealth(summoner->GetMaxHealth());
                me->SetHealth(summoner->GetHealth());
                me->setFaction(summoner->getFaction());

                if (summoner->GetTypeId() == TYPEID_UNIT)
                    summoner->SetVisible(false);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                switch (id)
                {
                    case POINT_MOVE_TO_CENTER_FLY: // when came to position in air must start flying and cast caw
                        events.ScheduleEvent(EVENT_JUVENILE_CHEEP, urand(5000,15000)); 
                        break;
                }
            }

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_START_FLY_AWAY:
                    {
                        Position NextPoint;
                        me->GetRandomPoint(Center, 25.0f, NextPoint);
                        me->GetMotionMaster()->MovePoint(POINT_MOVE_TO_CENTER_FLY, NextPoint.GetPositionX(), NextPoint.GetPositionY(), 28.0f);
                        break;
                    }
                    case EVENT_JUVENILE_CHEEP:
                    {
                        std::list<Player*> list;
                        GetPlayerListInGrid(list, me, 150.0f);
                        list.remove_if(Trinity::UnitAuraCheck(true, SPELL_DAEDELIAN_WINGS));

                        if (!list.empty())
                            if (Player* target = Trinity::Containers::SelectRandomContainerElement(list))
                                DoCast(target, SPELL_CHEEP_AOE);

                        events.ScheduleEvent(EVENT_JUVENILE_CHEEP, urand(5000,15000)); 
                        break;
                    }

                    default: break;
                }

                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_juvenileAI(creature);
        }
};

// Egg of Ji-Kun - 69628
class egg_of_jikun : public CreatureScript
{
    public:
        egg_of_jikun() : CreatureScript("egg_of_jikun") { }

        struct egg_of_jikunAI : public ScriptedAI
        {
            egg_of_jikunAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            enum _events
            {
                EVENT_FETCHLING_HATCH = 1
            };

            void Reset()
            {
                if (isSpawner(me))
                {
                    me->setFaction(35);
                    me->SetVisible(true);
                    events.ScheduleEvent(EVENT_CHECK_FOR_PLAYERS, 1000);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    return;
                }

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT | UNIT_FLAG_NOT_SELECTABLE); // no regen
                me->SetReactState(REACT_PASSIVE);
            }

            void IsSummonedBy(Unit* summoner)
            {
                if (isSpawner(me))
                    return;

                if (me->GetEntry() != NPC_FLEDGLING_EGG_JIKUN)
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                else if (summoner->GetEntry() == NPC_JIKUN_HATCHLING && summoner->HasAura(SPELL_HATCHLING_EVOLUTION))
                    events.ScheduleEvent(EVENT_FETCHLING_HATCH, urand(25000, 35000));

                me->setFaction(summoner->getFaction());
            }

            void UpdateAI(const uint32 diff)
            { 
                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_FETCHLING_HATCH:
                    {
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        me->CastSpell(me, SPELL_INCUBATION_HIGH_NEST, true);
                        break;
                    }
                    case EVENT_CHECK_FOR_PLAYERS:
                    {
                        std::list<Player*> players;
                        me->GetPlayerListInGrid(players, 2.0f);
                        if (!players.empty())
                        {
                            if (me->GetInstanceScript())
                                if (me->GetInstanceScript()->GetBossState(BOSS_JI_KUN) !=  IN_PROGRESS)
                                {
                                    DoCast(SPELL_EGG_EXPLOSION);
                                    me->HandleEmoteCommand(ANIM_EMOTE_DEAD);
                                    me->SetVisible(false);
                                    std::list<Creature*> JiKun;
                                    GetCreatureListWithEntryInGrid(JiKun, me, 69712, 500.0f);
                                    for (auto Ji : JiKun)
                                        Ji->GetAI()->DoAction(ACTION_START_INTRO); 

                                    me->SetVisible(false);
                                    return;
                                }
                        }

                        events.ScheduleEvent(EVENT_CHECK_FOR_PLAYERS, 1000);
                        break;
                    }
                    default: break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new egg_of_jikunAI(creature);
        }
};

// Ji kun platform teleporter - 70640.
class npc_jikun_teleport : public CreatureScript
{
    public:
        npc_jikun_teleport() : CreatureScript("npc_jikun_teleport") { }

        struct npc_jikun_teleportAI : public ScriptedAI
        {
            npc_jikun_teleportAI(Creature* creature) : ScriptedAI(creature) 
            {
                me->SetDisableGravity(true);
                me->SetCanFly(true);
            }

            void UpdateAI(const uint32 diff)
            {
                std::list<Player*> PlayerList;
                GetPlayerListInGrid(PlayerList, me, 220);

                for (auto playerTarget : PlayerList)
                {
                    if (playerTarget->IsFlying())
                        continue;

                    if (playerTarget->GetAreaId() != 6622) // Roost of Ji-Kun
                        continue;

                    float angle = playerTarget->GetAngle(Center.GetPositionX(),Center.GetPositionY());

                    if (playerTarget->GetDistance(me) <= 5.0f && !playerTarget->HasAura(SPELL_PLAYER_JUMP)) // Jump to platform of jikun
                    {
                        playerTarget->CastSpell(playerTarget, SPELL_PLAYER_JUMP, true);
                        playerTarget->JumpTo(24.0f,20.0f);
                    }

                    if (!playerTarget->HasAura(SPELL_PLAYER_VISUAL_SPIN) && playerTarget->GetPositionZ() < -108.0f) // fall from platform to bottom
                    {
                        playerTarget->AddAura(SPELL_PLAYER_VISUAL_SPIN,playerTarget);

                        float dist = playerTarget->GetDistance(Center)-87.0f;
                        playerTarget->GetMotionMaster()->MoveJump(playerTarget->GetPositionX() + dist* cos(angle),playerTarget->GetPositionY() + dist* sin(angle),-20.0f,15.0f,0,11.0f);
                    }

                    if (playerTarget->HasAura(SPELL_PLAYER_VISUAL_SPIN) && playerTarget->GetPositionZ() > -21.0f) // come to return jump
                    {
                        playerTarget->RemoveAurasDueToSpell(SPELL_PLAYER_VISUAL_SPIN);

                        float dist = playerTarget->GetDistance(Center)-37.0f;
                        playerTarget->GetMotionMaster()->MoveJump(playerTarget->GetPositionX() + dist* cos(angle),playerTarget->GetPositionY() + dist* sin(angle),-31.0f,30.0f,0,22.0f);
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_jikun_teleportAI(creature);
        }
};

// Feed of Ji-Kun 68178 70130
class npc_jikun_feed : public CreatureScript
{
    public:
        npc_jikun_feed() : CreatureScript("npc_jikun_feed") { }

        struct npc_jikun_feedAI : public ScriptedAI
        {
            npc_jikun_feedAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void UpdateAI(const uint32 diff)
            {
                // When eat flyed, player may get it when move to it
                std::list<Player*> list;
                GetPlayerListInGrid(list, me, 2.0f);
                for (auto itr: list)
                    if (itr->HasAura(SPELL_DAEDELIAN_WINGS) && !itr->HasAura(SPELL_PRIMAL_NUTRIMENT))
                    {
                        itr->CastSpell(itr, SPELL_SLIMED, true);
                        itr->CastSpell(itr, SPELL_PRIMAL_NUTRIMENT, true);
                        me->DespawnOrUnsummon();
                    }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                switch (id)
                {
                    case POINT_GREEN_FEED_AIR: // when came to position in air must fall to ground
                        me->GetMotionMaster()->MoveJump(me->GetPositionX(),me->GetPositionY(),-31.0f,5.0f,17.0f, POINT_GREEN_FEED_GROUND, 10.0f);
                        break;
                    case POINT_GREEN_FEED_GROUND: // when falled on ground must summon pool
                        DoCast(SPELL_SUMMON_POOL);
                        me->DespawnOrUnsummon();
                        break;
                    case POINT_FEED_CAME_TO_HATCHLING: // when came summon eat for hatchling and select hatchling to eat it
                        DoCast(SPELL_SUMMON_POOL_HATCHLINGS);
                        me->DespawnOrUnsummon();
                        break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_jikun_feedAI(creature);
        }
};

// Pool of feed(dmg) 68188, 70216
class pool_of_feed_dmg : public CreatureScript
{
    public:
        pool_of_feed_dmg() : CreatureScript("pool_of_feed_dmg") { }

        struct pool_of_feed_dmgAI : public ScriptedAI
        {
            pool_of_feed_dmgAI(Creature* creature) : ScriptedAI(creature) 
            {
                me->SetDisplayId(11686);
                me->AddAura(me->GetEntry() == NPC_POOL_OF_FEED_HATCHLING ? SPELL_FEED_POOL_VISUAL_HATCHLING : SPELL_FEED_POOL_VISUAL, me);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }

            void UpdateAI(const uint32 diff)
            { 
                if (me->GetEntry() != NPC_POOL_OF_FEED_HATCHLING)
                {
                    std::list<Player*> list;
                    GetPlayerListInGrid(list, me, 4.0f);
                    for (auto itr: list)
                        if (!itr->HasAura(SPELL_FEED_POOL_DMG))
                            me->CastSpell(itr, SPELL_FEED_POOL_DMG, true);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new pool_of_feed_dmgAI(creature);
        }
};

// Caw 138923
class spell_caw : public SpellScriptLoader
{
    public:
        spell_caw() : SpellScriptLoader("spell_caw") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void SelectTarget(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (Unit* target = SelectRandomTargetWithGuidOnRange(GetCaster()->ToUnit(), 0, 50.0f, true))
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        GetCaster()->CastSpell(target, SPELL_CAW_MISSILE, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

// Infected talons aura proc 140094.
class spell_infected_talons : public SpellScriptLoader
{
    public:
        spell_infected_talons() : SpellScriptLoader("spell_infected_talons") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                if (!GetCaster())
                    return;

                if (Unit* victim = GetCaster()->GetVictim())
                    GetCaster()->CastSpell(victim, SPELL_INFECTED_TALONS_DAMAGE, true);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(script_impl::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

// Hatchling eated 134321
class spell_hatchling_eated : public SpellScriptLoader
{
    public:
        spell_hatchling_eated() : SpellScriptLoader("spell_hatchling_eated") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void OnRemove(AuraEffect const*  /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = GetTarget())
                {
                    if (target->GetTypeId() == TYPEID_UNIT)
                    {
                        target->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                        target->CastSpell(target, SPELL_HATCHLING_EVOLUTION, true);
                        target->GetAI()->DoAction(FLEDGLING_ACTION_LAY_EGG);
                    }
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(script_impl::OnRemove, EFFECT_1, SPELL_AURA_MOD_ROOT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

// Hatchling 
class spell_hatchling_incubator : public SpellScriptLoader
{
    public:
        spell_hatchling_incubator() : SpellScriptLoader("spell_hatchling_incubator") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void OnRemove(AuraEffect const*  /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Creature* hatchling = GetCaster()->ToCreature())
                    hatchling->CastSpell(hatchling, hatchling->GetEntry() == NPC_YOUNG_EGG_OF_JIKUN ? SPELL_SUMMON_YOUNG_HATCHLING : SPELL_SUMMON_JUVENILE, true);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(script_impl::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);

            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

// 139319 pool of feed dmg aur
class pool_of_feed_dmg_aura : public SpellScriptLoader
{
    public:
        pool_of_feed_dmg_aura() : SpellScriptLoader("pool_of_feed_dmg_aura") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void OnPeriodic(AuraEffect const* aurEff)
            {
                if (!GetTarget() || !aurEff || GetId() == SPELL_SLIMED)
                    return;

                if (!GetCaster() && GetTarget()->HasAura(SPELL_FEED_POOL_DMG))
                {
                    GetTarget()->RemoveAura(SPELL_FEED_POOL_DMG);
                    return;
                }

                if (!GetCaster())
                    return;

                // if somebody get 3 ticks, pool must be despawned
                if (aurEff->GetTickNumber() == 3)
                    if (TempSummon* pool = GetCaster()->ToTempSummon())
                    {
                        if (GetTarget()->HasAura(SPELL_FEED_POOL_DMG))
                            GetTarget()->RemoveAura(SPELL_FEED_POOL_DMG);

                        pool->DespawnOrUnsummon();
                        return;
                    }

                if (GetTarget()->GetDistance(GetCaster()) > 4.0f)
                    GetTarget()->RemoveAura(SPELL_FEED_POOL_DMG);
            }

            void OnRemove(AuraEffect const*  /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget())
                    GetTarget()->CastSpell(GetTarget(),SPELL_SLIMED_DMG_INCREASE,true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                AfterEffectRemove += AuraEffectRemoveFn(script_impl::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);

            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

// Stack of wings remove 133755
class remove_stack_wings : public SpellScriptLoader
{
    public:
        remove_stack_wings() : SpellScriptLoader("remove_stack_wings") { }

        class remove_stack_wings_AuraScript : public AuraScript
        {
            PrepareAuraScript(remove_stack_wings_AuraScript);

            void OnApply(AuraEffect const*  /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                if (GetTarget()->HasAura(SPELL_DAEDELIAN_WINGS))
                    GetTarget()->GetAura(SPELL_DAEDELIAN_WINGS)->ModStackAmount(-1);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(remove_stack_wings_AuraScript::OnApply, EFFECT_0, SPELL_AURA_FLY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);

            }
        };

        AuraScript* GetAuraScript() const
        {
            return new remove_stack_wings_AuraScript();
        }
};

// Feed summon 137528.
class spell_feed_summon : public SpellScriptLoader
{
    public:
        spell_feed_summon() : SpellScriptLoader("spell_feed_summon") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void OnTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (aurEff->GetTickNumber() == 1) // first tick send 3 eat for hatchlings
                {
                    if (Unit* target = SelectRandomTargetWithGuidOnRange(GetCaster()->ToUnit(), NPC_BEAM_NEST, 500.0f, false))
                        if (Creature* beam = target->ToCreature())
                            for (uint8 i = 0;i<=3;++i)
                                if (Creature* Feed = GetCaster()->SummonCreature(NPC_FEED_HATCHLINGS,GetCaster()->GetPositionX(),GetCaster()->GetPositionY(),GetCaster()->GetPositionZ()+6.0f,0.0f,TEMPSUMMON_TIMED_DESPAWN,12000))
                                {
                                    Position newPos,src;
                                    beam->GetPosition(&src);
                                    Feed->GetRandomPoint(src, 7.0f,newPos);
                                    Feed->GetMotionMaster()->MoveJump(newPos.GetPositionX(),newPos.GetPositionY(),newPos.GetPositionZ()+1.0f,15.0f,50.0f,POINT_FEED_CAME_TO_HATCHLING,10.0f);
                                }
                }
                else // tick 2 send "green eat" for pools
                {
                    for (uint8 i = 0;i<=3;++i)
                        if (Creature* FeedGreen = GetCaster()->SummonCreature(NPC_FEED_POOL,GetCaster()->GetPositionX(),GetCaster()->GetPositionY(),GetCaster()->GetPositionZ()+6.0f,0.0f,TEMPSUMMON_TIMED_DESPAWN,12000))
                        {
                            Position jumpPos;
                            FeedGreen->GetRandomPoint(Center,40.0f,jumpPos);
                            FeedGreen->GetMotionMaster()->MoveJump(jumpPos.GetPositionX(),jumpPos.GetPositionY(),6.0f,35.0f,20.0f,POINT_GREEN_FEED_AIR,10.0f);
                        }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

void AddSC_boss_jikun()
{
    new boss_jikun();
    new egg_of_jikun();
    new young_hatchling_jikun();
    new npc_juvenile();
    new jikun_beam_target();
    new npc_jikun_feed();
    new pool_of_feed_dmg();
    new remove_stack_wings();
    new npc_jikun_teleport();
    new spell_hatchling_incubator();
    new spell_infected_talons();
    new spell_caw();
    new spell_hatchling_eated();
    new pool_of_feed_dmg_aura();
    new spell_feed_summon();
}