#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Vehicle.h"
#include "MoveSplineInit.h"
#include "deadmines.h"

enum eSpells
{
    SPELL_CLEARALLDEBUFFS                 = 34098,
    SPELL_FIERY_BLAZE                     = 93484,
    SPELL_VANESSA_COSMETIC_BOMB_STATE     = 96280,
    SPELL_POWDER_EXPLOSION                = 96283,
    SPELL_DEADLY_BLADES                   = 92622,
    SPELL_SUMMON_ENFORCER                 = 92616,
    SPELL_SUMMON_BLOOD_WIZARD             = 92617,
    SPELL_SUMMON_SHADOWGUARD              = 92618,
    SPELL_DEFLECTION                      = 92614,
    SPELL_VENGEANCE_OF_VANCLEEF           = 95542,
    SPELL_ROPE_BEAM                       = 43785,
    SPELL_CLICK_ME                        = 95527,
    SPELL_CHARGE                          = 90928,
    SPELL_RECKLESSNESS                    = 90929,
    SPELL_CAMOUFLAGE                      = 90954,
    SPELL_EVASION                         = 90958,
    SPELL_SINISTER_STRIKE                 = 90951,
    SPELL_SHADOWSTEP                      = 90956,
    SPELL_WHIRLING_BLADES                 = 90960,
    SPELL_RAGEZONE                        = 90932,
    SPELL_BLOODBOLT                       = 90938,
    SPELL_BLOODWASH                       = 90946,
    SPELL_BLOODBATH                       = 90925,
    SPELL_BLOODBATH_TRIGGER               = 90923,
};

enum eCreatures
{
    NPC_GPB_JMF                           = 45979,
    NPC_ROPE                              = 49550,
    NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY      = 51594,
    NPC_VANESSAS_ROPE_ANCHOR              = 49552,
};

enum eScriptTexts
{
    VANESSA_YELL_START                    = 0,
    VANESSA_YELL_SHIP_EXPLOSIVE           = 1,
    VANESSA_YELL_SHIP_MORE_EXPLOSIVE      = 2,
    VANESSA_YELL_DIED_1                   = 3,
    VANESSA_YELL_DIED_2                   = 4,
    VANESSA_YELL_DIED_3                   = 5,
    VANESSA_YELL_SUMMON_MINION            = 6,
    VANESSA_YELL_KILL_PLAYER              = 7,
    VANESSA_EMOTE_EXPLOSIVE               = 8,
    VANESSA_EMOTE_DIED                    = 9,
};

enum eEvents
{
    // Vanessa
    EVENT_JUMP_ROOF                       = 1,
    EVENT_YELL_SHIP_EXPLOSIVE             = 3,
    EVENT_EMOTE_SHIP_EXPLOSIVE            = 4,
    EVENT_SUMMON_ROPE                     = 5,
    EVENT_SUMMON_FIRE                     = 6,
    EVENT_DISABLE_FIRE_LEVEL_3            = 7,
    EVENT_DISABLE_FIRE_LEVEL_4            = 8,
    EVENT_BACK_TO_FLOOR                   = 9,
    EVENT_CHECK_ATTACKERS                 = 10,
    EVENT_YELL_MORE_SHIP_EXPLOSIVE        = 11,
    EVENT_ENABLE_FIRE                     = 12,
    EVENT_VANESSA_DIED_YELL               = 14,
    EVENT_VANESSA_DIED_CAST_VISUAL        = 15,
    EVENT_VANESSA_DIED_EMOTE              = 16,
    EVENT_VANESSA_DIED                    = 17,
    EVENT_BACKSLASH                       = 18,
    EVENT_SUMMON_ENFORCER                 = 19,
    EVENT_SUMMON_BLOOD_WIZARD             = 20,
    EVENT_SUMMON_SHADOWGUARD              = 21,
    EVENT_DEFLECTION                      = 22,
    EVENT_VENGEANCE_OF_VANCLEEF           = 23,
    EVENT_CHECK_ATTACKERS_ON_SHIP         = 24,
    EVENT_START_MOVE                      = 2,
    EVENT_DONE_MOVE                       = 25,
    EVENT_VANESSA_JUMP_DONE               = 26,
    // Defias Enforcer
    EVENT_BLOODBATH                       = 1,
    // Defias Shadowguard
    EVENT_SINISTER_STRIKE                 = 1,
    EVENT_SHADOWSTEP                      = 2,
    EVENT_WHIRLING_BLADES                 = 3,
    // Defias Blood Wizard
    EVENT_BLOODBOLT                       = 1,
    EVENT_BLOODWASH                       = 2,
};

enum eData
{
    DATA_ROPE_ID                          = 1,
};

enum ePoints
{
    POINT_VANESSA_JUMP                    = 1010,
    POINT_VANESSA_JUMP_FLOOR              = 1011,
};

enum eActions
{
    ACTION_DESPAWN_ROPE                   = 2,
};

const Position FireLevel_1[6]=
{
    {-38.1145f, -795.295f, 39.4513f, 3.92699f},{-64.0225f, -797.432f, 39.1230f, 3.92699f},
    {-46.4843f, -791.807f, 39.1209f, 3.92699f},{-75.5069f, -787.159f, 39.1632f, 3.92699f},
    {-66.1822f, -789.277f, 39.7883f, 3.92699f},{-70.9809f, -795.500f, 39.1444f, 3.92699f},
};

const Position FireLevel_2[48]=
{
    {-79.61111f, -794.7726f, 38.50275f, 3.926991f},{-54.19097f, -789.9566f, 38.85344f, 3.926991f},
    {-59.27778f, -787.6597f, 39.37201f, 3.926991f},{-44.94792f, -801.1077f, 40.70300f, 3.926991f},
    {-83.51563f, -847.0851f, 18.62521f, 3.926991f},{-77.33160f, -848.7101f, 16.88625f, 3.926991f},
    {-47.97917f, -851.7882f, 18.46356f, 3.926991f},{-54.41667f, -846.2222f, 22.62641f, 3.926991f},
    {-46.15799f, -843.5104f, 23.03055f, 3.926991f},{-32.24479f, -844.8195f, 19.15213f, 3.926991f},
    {-28.87847f, -840.0226f, 19.30344f, 3.926991f},{-31.54861f, -830.5191f, 23.84564f, 3.926991f},
    {-17.43924f, -821.8785f, 19.89213f, 3.926991f},{-17.77431f, -814.7327f, 19.87694f, 3.926991f},
    {-22.31250f, -807.7292f, 19.61356f, 3.926991f},{-126.0694f, -787.8212f, 17.43893f, 3.926991f},
    {-59.65799f, -786.8472f, 18.00813f, 3.926991f},{-51.85590f, -788.4774f, 18.27221f, 3.926991f},
    {-65.53993f, -768.6545f, 28.01228f, 3.926991f},{-122.8872f, -816.6580f, 17.01834f, 3.926991f},
    {-123.3351f, -807.2153f, 16.98381f, 3.926991f},{-81.68750f, -791.7795f, 25.86889f, 3.926991f},
    {-75.52952f, -788.3438f, 26.06530f, 3.926991f},{-46.44965f, -809.7448f, 42.93429f, 3.926991f},
    {-54.49479f, -826.5833f, 42.19642f, 3.926991f},{-85.18403f, -822.6042f, 39.48589f, 3.926991f},
    {-74.00000f, -813.6320f, 40.37197f, 3.926991f},{-37.77257f, -824.4930f, 43.78171f, 3.926991f},
    {-38.67708f, -816.1389f, 43.68202f, 3.926991f},{-68.94444f, -805.1528f, 40.82362f, 3.926991f},
    {-76.88889f, -806.1927f, 40.12347f, 3.926991f},{-53.39583f, -808.4184f, 42.27031f, 2.757620f},
    {-93.49653f, -809.6979f, 38.80095f, 3.926991f},{-101.7240f, -815.1875f, 38.20302f, 3.926991f},
    {-101.6024f, -822.2257f, 38.22025f, 3.926991f},{-95.20313f, -826.7986f, 38.69661f, 3.926991f},
    {-92.31250f, -835.5712f, 31.01794f, 3.926991f},{-54.32118f, -840.3108f, 33.48670f, 3.926991f},
    {-34.31597f, -825.7864f, 35.14482f, 3.926991f},{-41.65799f, -834.1059f, 34.51352f, 3.926991f},
    {-106.7882f, -827.8351f, 30.36550f, 3.926991f},{-110.0000f, -821.9080f, 30.27846f, 3.926991f},
    {-108.5208f, -840.7136f, 17.11709f, 3.926991f},{-116.1042f, -835.6597f, 17.07181f, 3.926991f},
    {-109.6736f, -832.7049f, 20.99191f, 3.926991f},{-82.74479f, -853.8368f, 17.44633f, 3.926991f},
    {-115.3142f, -827.2327f, 20.94427f, 3.926991f},{-92.09202f, -843.8507f, 21.32712f, 3.926991f},
};

const Position FireLevel_3[7]=
{
    {-54.53820f, -833.9739f, 42.20368f, 2.75762f},{-81.48264f, -832.3143f, 39.79344f, 2.75762f},
    {-76.38541f, -821.6371f, 40.15092f, 2.75762f},{-84.03125f, -817.4323f, 39.54358f, 2.75762f},
    {-60.28472f, -806.1979f, 41.61631f, 2.75762f},{-46.09722f, -830.2518f, 43.00029f, 2.75762f},
    {-104.3299f, -806.0121f, 30.58083f, 3.92699f},
};

const Position FireLevel_4[10]=
{
    {-87.68750f, -830.8108f, 39.29154f, 2.757620f},{-46.73611f, -821.1771f, 52.58150f, 3.193953f},
    {-62.43750f, -825.9114f, 41.42738f, 3.193953f},{-75.33681f, -833.4340f, 40.29444f, 3.193953f},
    {-69.06944f, -834.3680f, 40.86119f, 3.193953f},{-62.18924f, -834.6563f, 41.49004f, 3.193953f},
    {-70.38889f, -824.9722f, 40.70635f, 3.193953f},{-66.54514f, -815.1180f, 41.03350f, 3.193953f},
    {-57.90972f, -816.8611f, 41.85588f, 2.757620f},{-85.05209f, -808.5243f, 39.47224f, 3.926991f},
};

const Position VanessaJumpPos = {-52.1371f, -820.155f, 51.5868f, 3.0688f};

const Position VanessaFloorPos = {-64.8153f, -820.049f, 41.0983f, 3.0688f};

struct VanessasRope
{
    Position RopePos;
    Position AnchorPos;
};

const VanessasRope Rope[5]=
{
    {{-62.170f, -839.844f, 41.4851f, 5.0440f},{-63.605f, -862.786f, 202.730f, 4.6076f}},
    {{-64.970f, -840.009f, 41.2267f, 4.8345f},{-64.217f, -866.182f, 195.395f, 4.6076f}},
    {{-67.791f, -840.174f, 40.9666f, 4.7123f},{-69.154f, -868.964f, 195.248f, 4.6076f}},
    {{-70.621f, -840.010f, 40.7185f, 4.5378f},{-71.114f, -868.464f, 200.771f, 4.6076f}},
    {{-73.423f, -839.865f, 40.4726f, 4.3458f},{-74.296f, -877.101f, 195.044f, 4.6076f}},
};

class boss_vanessa_vancleef : public CreatureScript
{
public:
    boss_vanessa_vancleef() : CreatureScript("boss_vanessa_vancleef") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_vanessa_vancleefAI (creature);
    }

    struct boss_vanessa_vancleefAI : public BossAI
    {
        boss_vanessa_vancleefAI(Creature* creature) : BossAI(creature, DATA_VANESSA_VANCLEEF) { }

        uint64 FireLevel3GUID[7];
        uint64 FireLevel4GUID[10];
        uint8 eventHealthPct;
        bool died;
        bool canCheck;

        void Reset()
        {
            _Reset();
            died = false;
            canCheck = false;
            events.Reset();
            eventHealthPct = 75;
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(VANESSA_YELL_KILL_PLAYER);
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(VANESSA_YELL_START);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            me->CastSpell(me, 95654, true);
            me->setActive(true);
            canCheck = true;
            events.ScheduleEvent(EVENT_BACKSLASH, urand(5000, 25000));
            events.ScheduleEvent(EVENT_SUMMON_ENFORCER, urand(10000, 20000));
            events.ScheduleEvent(EVENT_DEFLECTION, urand(10000, 20000));
        }

        void DamageTaken(Unit* done_by, uint32 &damage)
        {
            if (me->GetHealthPct() <= eventHealthPct && canCheck)
            {
                switch (eventHealthPct)
                {
                    case 75:
                        {
                            eventHealthPct = 50;
                            Talk(VANESSA_YELL_START);

                            for (int i = 0; i < 6; ++i)
                                me->SummonCreature(NPC_GPB_JMF, FireLevel_1[i]);
                        }
                        break;
                    case 50:
                        eventHealthPct = 25;
                        events.ScheduleEvent(EVENT_JUMP_ROOF, 1000);
                        break;
                    case 25:
                        canCheck = false;
                        eventHealthPct = 0;
                        events.ScheduleEvent(EVENT_JUMP_ROOF, 1000);
                        break;
                }
            }

            if (damage >= me->GetHealth())
            {
                if (done_by->GetGUID() != me->GetGUID())
                {
                    damage = 0;
                    me->SetHealth(1);
                }

                if (!died)
                {
                    died = true;
                    Talk(VANESSA_YELL_DIED_1);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->RemoveAllAuras();
                    events.Reset();
                    events.ScheduleEvent(EVENT_VANESSA_DIED_CAST_VISUAL, 3000);
                }
            }
        }

        void JustSummoned(Creature* summoned)
        {
            switch (summoned->GetEntry())
            {
                case NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY:
                    summoned->SetReactState(REACT_PASSIVE);
                    summoned->SetInCombatWithZone();
                    summoned->RemoveAllAuras();
                    summoned->CastSpell(summoned, SPELL_FIERY_BLAZE, false);
                    break;
                case NPC_GPB_JMF:
                    summoned->SetReactState(REACT_PASSIVE);
                    summoned->SetInCombatWithZone();
                    summoned->RemoveAllAuras();
                    summoned->CastSpell(summoned, SPELL_FIERY_BLAZE, false);
                    break;
            }

            summons.Summon(summoned);
            summoned->SetInCombatWithZone();
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
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
                    case EVENT_JUMP_ROOF:
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->StopMoving();
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveIdle();
                        me->CastSpell(me, SPELL_CLEARALLDEBUFFS, false);
                        me->RemoveAura(SPELL_DEADLY_BLADES);
                        events.Reset();
                        me->GetMotionMaster()->MoveJump(VanessaJumpPos, 10.0f, 15.0f);
                        events.ScheduleEvent(EVENT_VANESSA_JUMP_DONE, me->GetSplineDuration());
                        break;
                    case EVENT_VANESSA_JUMP_DONE:
                        {
                            switch (eventHealthPct)
                            {
                                case 25:
                                    events.ScheduleEvent(EVENT_YELL_SHIP_EXPLOSIVE, 1000);
                                    events.ScheduleEvent(EVENT_DISABLE_FIRE_LEVEL_3, 16000);
                                    break;
                                case 0:
                                    events.ScheduleEvent(EVENT_YELL_MORE_SHIP_EXPLOSIVE, 1000);
                                    break;
                            }

                            events.ScheduleEvent(EVENT_DISABLE_FIRE_LEVEL_4, 16000);
                            events.ScheduleEvent(EVENT_BACK_TO_FLOOR, 17000);
                        }
                        break;
                    case EVENT_YELL_SHIP_EXPLOSIVE:
                        me->SetFacingTo(VanessaJumpPos.GetOrientation());
                        Talk(VANESSA_YELL_SHIP_EXPLOSIVE);
                        events.ScheduleEvent(EVENT_EMOTE_SHIP_EXPLOSIVE, 4000);
                        events.ScheduleEvent(EVENT_SUMMON_FIRE, 10000);
                        break;
                    case EVENT_EMOTE_SHIP_EXPLOSIVE:
                        Talk(VANESSA_EMOTE_EXPLOSIVE);
                        events.ScheduleEvent(EVENT_SUMMON_ROPE, 2000);
                        break;
                    case EVENT_SUMMON_ROPE:
                        {
                            for (int i = 0; i < 5; ++i)
                                if (Creature* rope = me->SummonCreature(NPC_ROPE, Rope[i].RopePos))
                                    if (rope->IsAIEnabled)
                                        rope->AI()->SetData(DATA_ROPE_ID, i);

                            summons.DespawnEntry(49850);
                            summons.DespawnEntry(49852);
                            summons.DespawnEntry(49854);
                        }
                        break;
                    case EVENT_SUMMON_FIRE:
                        {
                            for (int i = 0; i < 48; ++i)
                                me->SummonCreature(NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY, FireLevel_2[i]);

                            for (int i = 0; i < 7; ++i)
                                if (Creature* fire = me->SummonCreature(NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY, FireLevel_3[i]))
                                    FireLevel3GUID[i] = fire->GetGUID();

                            for (int i = 0; i < 10; ++i)
                                if (Creature* fire = me->SummonCreature(NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY, FireLevel_4[i]))
                                    FireLevel4GUID[i] = fire->GetGUID();
                        }
                        break;
                    case EVENT_DISABLE_FIRE_LEVEL_3:
                        {
                            for (int i = 0; i < 7; ++i)
                                if (Creature* fire = ObjectAccessor::GetCreature(*me, FireLevel3GUID[i]))
                                    fire->RemoveAllAuras();
                        }
                        break;
                    case EVENT_DISABLE_FIRE_LEVEL_4:
                        {
                            for (int i = 0; i < 10; ++i)
                                if (Creature* fire = ObjectAccessor::GetCreature(*me, FireLevel4GUID[i]))
                                    fire->RemoveAllAuras();
                        }
                        break;
                    case EVENT_BACK_TO_FLOOR:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->GetMotionMaster()->MoveJump(VanessaFloorPos, 10.0f, 15.0f);
                        events.ScheduleEvent(EVENT_CHECK_ATTACKERS_ON_SHIP, 2000);
                        events.ScheduleEvent(EVENT_CHECK_ATTACKERS, 15000);
                        break;
                    case EVENT_CHECK_ATTACKERS_ON_SHIP:
                        {
                            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();

                            for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                                if (Player* player = itr->GetSource())
                                    if (!player->GetVehicle() && me->GetDistance2d(player) <= 30.0f && me->IsWithinLOSInMap(player))
                                    {
                                        me->SetReactState(REACT_AGGRESSIVE);
                                        AttackStart(player);
                                        me->CastSpell(me, SPELL_VENGEANCE_OF_VANCLEEF, false);
                                        events.ScheduleEvent(EVENT_BACKSLASH, urand(10000, 30000));

                                        if (eventHealthPct != 0)
                                        {
                                            events.ScheduleEvent(EVENT_DEFLECTION, urand(10000, 20000));
                                            events.ScheduleEvent(EVENT_SUMMON_ENFORCER, urand(10000, 30000));
                                        }
                                        else
                                            events.ScheduleEvent(EVENT_VENGEANCE_OF_VANCLEEF, urand(5000, 10000));

                                        EntryCheckPredicate pred(NPC_ROPE);
                                        summons.DoAction(ACTION_DESPAWN_ROPE, pred);
                                        return;
                                    }

                            events.ScheduleEvent(EVENT_CHECK_ATTACKERS_ON_SHIP, 500);
                        }
                        break;
                    case EVENT_CHECK_ATTACKERS:
                        {
                            if (me->GetReactState() == REACT_PASSIVE)
                                EnterEvadeMode();
                        }
                        break;
                    case EVENT_YELL_MORE_SHIP_EXPLOSIVE:
                        me->SetFacingTo(VanessaJumpPos.GetOrientation());
                        Talk(VANESSA_YELL_SHIP_MORE_EXPLOSIVE);
                        events.ScheduleEvent(EVENT_EMOTE_SHIP_EXPLOSIVE, 4000);
                        events.ScheduleEvent(EVENT_ENABLE_FIRE, 10000);
                        break;
                    case EVENT_ENABLE_FIRE:
                        {
                            for (int i = 0; i < 7; ++i)
                                if (Creature* fire = ObjectAccessor::GetCreature(*me, FireLevel3GUID[i]))
                                    fire->CastSpell(fire, SPELL_FIERY_BLAZE, false);

                            for (int i = 0; i < 10; ++i)
                                if (Creature* fire = ObjectAccessor::GetCreature(*me, FireLevel4GUID[i]))
                                    fire->CastSpell(fire, SPELL_FIERY_BLAZE, false);
                        }
                        break;
                    case EVENT_VANESSA_DIED_CAST_VISUAL:
                        Talk(VANESSA_YELL_DIED_2);
                        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, 0);
                        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, 0);
                        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, 0);
                        me->CastSpell(me, SPELL_VANESSA_COSMETIC_BOMB_STATE, false);
                        events.ScheduleEvent(EVENT_VANESSA_DIED_EMOTE, 2000);
                        break;
                    case EVENT_VANESSA_DIED_EMOTE:
                        Talk(VANESSA_EMOTE_DIED);
                        events.ScheduleEvent(EVENT_VANESSA_DIED_YELL, 4000);
                        break;
                    case EVENT_VANESSA_DIED_YELL:
                        Talk(VANESSA_YELL_DIED_3);
                        events.ScheduleEvent(EVENT_VANESSA_DIED, 1000);
                        break;
                    case EVENT_VANESSA_DIED:
                        me->CastSpell(me, SPELL_POWDER_EXPLOSION, false);
                        break;
                    case EVENT_BACKSLASH:
                        me->CastSpell(me, SPELL_DEADLY_BLADES, false);
                        events.ScheduleEvent(EVENT_BACKSLASH, urand(30000, 40000));
                        break;
                    case EVENT_SUMMON_ENFORCER:
                        me->CastSpell(me, SPELL_SUMMON_ENFORCER, false);
                        Talk(VANESSA_YELL_SUMMON_MINION);
                        events.ScheduleEvent(EVENT_SUMMON_BLOOD_WIZARD, 11000);
                        break;
                    case EVENT_SUMMON_BLOOD_WIZARD:
                        me->CastSpell(me, SPELL_SUMMON_BLOOD_WIZARD, false);
                        events.ScheduleEvent(EVENT_SUMMON_SHADOWGUARD, 11000);
                        break;
                    case EVENT_SUMMON_SHADOWGUARD:
                        me->CastSpell(me, SPELL_SUMMON_SHADOWGUARD, false);
                        events.ScheduleEvent(EVENT_SUMMON_ENFORCER, urand(25000, 50000));
                        break;
                    case EVENT_DEFLECTION:
                        me->CastSpell(me, SPELL_DEFLECTION, false);
                        events.ScheduleEvent(EVENT_DEFLECTION, urand(20000, 40000));
                        break;
                    case EVENT_VENGEANCE_OF_VANCLEEF:
                        me->CastSpell(me, SPELL_VENGEANCE_OF_VANCLEEF, false);
                        events.ScheduleEvent(EVENT_VENGEANCE_OF_VANCLEEF, urand(5000, 10000));
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_vanessas_rope : public CreatureScript
{
public:
    npc_vanessas_rope() : CreatureScript("npc_vanessas_rope") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_vanessas_ropeAI(creature);
    }

    struct npc_vanessas_ropeAI : public ScriptedAI
    {
        npc_vanessas_ropeAI(Creature* creature) : ScriptedAI(creature)
        {
            anchorGUID = 0;
            ropeId = 0;
        }

        EventMap events;
        uint64 anchorGUID;
        uint8 ropeId;

        void SetData(uint32 data, uint32 value)
        {
            if (data == DATA_ROPE_ID)
            {
                ropeId = value;

                if (Creature* anchor = me->SummonCreature(NPC_VANESSAS_ROPE_ANCHOR, Rope[ropeId].AnchorPos))
                {
                    anchorGUID = anchor->GetGUID();
                    anchor->setActive(true);
                    anchor->CastSpell(me, SPELL_ROPE_BEAM, false);
                }
            }
        }

        void FindPath()
        {
            std::list<Position> BackWaypoints;
            float angle = me->GetAngle(Rope[ropeId].AnchorPos.m_positionX, Rope[ropeId].AnchorPos.m_positionY);
            float dist = 0;
            float x, y, z = me->GetPositionZ();
            me->GetNearPoint2D(x, y, 3.0f, -angle);
            Position pos = {x, y, z + 3.0f, 0.0f};
            BackWaypoints.push_front(pos);
            Movement::MoveSplineInit init(me);

            for (int i = 0; i < 10; ++i)
            {
                dist += 8.0f;

                if (i < 4)
                    z -= 2.5f;
                else if (i > 6)
                    z += 2.5f;

                me->GetNearPoint2D(x, y, dist, angle);
                G3D::Vector3 vertice(x, y, z);
                init.Path().push_back(vertice);
                pos.Relocate(x, y, z + 1.0f);
                BackWaypoints.push_front(pos);
            }

            for (std::list<Position>::const_iterator itr = BackWaypoints.begin(); itr != BackWaypoints.end(); ++itr)
            {
                G3D::Vector3 vertice(itr->m_positionX, itr->m_positionY, itr->m_positionZ);
                init.Path().push_back(vertice);
            }

            init.SetFly();
            init.SetSmooth();
            init.SetUncompressed();
            init.SetVelocity(20.0f);
            init.Launch();
            events.ScheduleEvent(EVENT_DONE_MOVE, me->GetSplineDuration());
        }

        void DoAction(const int32 action)  override
        {
            if (action == ACTION_DESPAWN_ROPE)
                if (Vehicle* vehicle = me->GetVehicleKit())
                    if (!vehicle->GetPassenger(0))
                    {
                        me->DespawnOrUnsummon();

                        if (Creature* anchor = Unit::GetCreature(*me, anchorGUID))
                            anchor->DespawnOrUnsummon();
                    }
        }

        void PassengerBoarded(Unit* /*who*/, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                me->RemoveAura(SPELL_CLICK_ME);
                events.ScheduleEvent(EVENT_START_MOVE, 1000);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_MOVE:
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        me->SetDisplayId(11686);
                        FindPath();
                        break;
                    case EVENT_DONE_MOVE:
                        {
                            me->RemoveAllAuras();
                            me->DespawnOrUnsummon();

                            if (Creature* anchor = Unit::GetCreature(*me, anchorGUID))
                                anchor->DespawnOrUnsummon();
                        }
                        break;
                }
            }
        }
    };
};

class npc_defias_enforcer : public CreatureScript
{
public:
    npc_defias_enforcer() : CreatureScript("npc_defias_enforcer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_defias_enforcerAI (creature);
    }

    struct npc_defias_enforcerAI : public ScriptedAI
    {
        npc_defias_enforcerAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;
        bool Enrage;

        void Reset()
        {
            events.Reset();
            Enrage = false;
        }

        void EnterCombat(Unit* who)
        {
            me->CastSpell(who, SPELL_CHARGE, false);
            events.ScheduleEvent(EVENT_BLOODBATH, urand(3000, 10000));
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
        {
            if (me->GetHealthPct() <= 25.0f && !Enrage && !me->IsInEvadeMode())
            {
                Enrage = true;
                me->CastSpell(me, SPELL_RECKLESSNESS, false);
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            events.Reset();
        }

        void UpdateAI(const uint32 diff)  override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (events.ExecuteEvent() == EVENT_BLOODBATH)
            {
                me->CastSpell(me->GetVictim(), SPELL_BLOODBATH, false);
                events.ScheduleEvent(EVENT_BLOODBATH, urand(3000, 10000));
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_defias_shadowguard : public CreatureScript
{
public:
    npc_defias_shadowguard() : CreatureScript("npc_defias_shadowguard") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_defias_shadowguardAI (creature);
    }

    struct npc_defias_shadowguardAI : public ScriptedAI
    {
        npc_defias_shadowguardAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;
        bool Enrage;

        void Reset()
        {
            me->AddAura(SPELL_CAMOUFLAGE, me);
            events.Reset();
            Enrage = false;
        }

        void EnterCombat(Unit* who)
        {
            me->AddThreat(who, 100500.0f);
            events.ScheduleEvent(EVENT_SHADOWSTEP, 1000);
            events.ScheduleEvent(EVENT_WHIRLING_BLADES, urand(3000, 5000));
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
        {
            if (me->GetHealthPct() <= 30.0f && !Enrage && !me->IsInEvadeMode())
            {
                Enrage = true;
                me->CastSpell(me, SPELL_EVASION, false);
            }
        }

        void JustDied(Unit* /*killer*/)  override
        {
            events.Reset();
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
                    case EVENT_SINISTER_STRIKE:
                        me->CastSpell(me->GetVictim(), SPELL_SINISTER_STRIKE, false);
                        events.ScheduleEvent(EVENT_SHADOWSTEP, urand(5000, 10000));
                        break;
                    case EVENT_SHADOWSTEP:
                        me->CastSpell(me->GetVictim(), SPELL_SHADOWSTEP, false);
                        events.ScheduleEvent(EVENT_SINISTER_STRIKE, 500);
                        break;
                    case EVENT_WHIRLING_BLADES:
                        me->CastSpell(me->GetVictim(), SPELL_WHIRLING_BLADES, false);
                        events.ScheduleEvent(EVENT_WHIRLING_BLADES, urand(3000, 5000));
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_defias_blood_wizard : public CreatureScript
{
public:
    npc_defias_blood_wizard() : CreatureScript("npc_defias_blood_wizard") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_defias_blood_wizardAI (creature);
    }

    struct npc_defias_blood_wizardAI : public ScriptedAI
    {
        npc_defias_blood_wizardAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;
        bool Enrage;

        void Reset()
        {
            events.Reset();
            Enrage = false;
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_BLOODBOLT, urand(3000, 5000));
            events.ScheduleEvent(EVENT_BLOODWASH, urand(5000, 10000));
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
        {
            if (me->GetHealthPct() <= 30.0f && !Enrage && !me->IsInEvadeMode())
            {
                Enrage = true;
                me->CastSpell(me->GetVictim(), SPELL_RAGEZONE, false);
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            events.Reset();
        }

        void UpdateAI(const uint32 diff)
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
                    case EVENT_BLOODBOLT:
                        me->CastSpell(me->GetVictim(), SPELL_BLOODBOLT, false);
                        events.ScheduleEvent(EVENT_BLOODBOLT, urand(3000, 5000));
                        break;
                    case EVENT_BLOODWASH:
                        {
                            Unit* target = NULL;

                            if (me->IsSummon())
                                target = me->ToTempSummon()->GetSummoner();

                            if (target)
                                me->CastSpell(target, SPELL_BLOODWASH, false);
                            else
                                me->CastSpell(me, SPELL_BLOODWASH, false);
                        }
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class spell_bloodwash : public SpellScriptLoader
{
    public:
        spell_bloodwash() : SpellScriptLoader("spell_bloodwash") { }

        class spell_bloodwash_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_bloodwash_AuraScript)

            void OnPeriodic(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();

                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster->GetVictim(), SPELL_BLOODBATH_TRIGGER, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_bloodwash_AuraScript::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_bloodwash_AuraScript();
        }
};

class spell_backslash_targeting : public SpellScriptLoader
{
    public:
        spell_backslash_targeting() : SpellScriptLoader("spell_backslash_targeting") { }

        class spell_backslash_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_backslash_targeting_SpellScript)

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
                OnEffectHitTarget += SpellEffectFn(spell_backslash_targeting_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_backslash_targeting_SpellScript();
        }
};

class spell_deadmines_backslash : public SpellScriptLoader
{
    public:
        spell_deadmines_backslash() : SpellScriptLoader("spell_deadmines_backslash") { }

        class spell_deadmines_backslash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deadmines_backslash_SpellScript)

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
                Creature* vanessa = NULL;

                if (caster)
                    vanessa = caster->ToCreature();

                if (!(vanessa && target))
                    return;

                float _x, _y;
                float x, y, z;
                target->GetPosition(_x, _y);
                x = _x + 1.5f * std::cos(target->GetOrientation() + M_PI);
                y = _y + 1.5f * std::sin(target->GetOrientation() + M_PI);
                z = caster->GetMap()->GetHeight(caster->GetPhaseMask(), x, y, MAX_HEIGHT);

                if (std::abs(z - target->GetPositionZ()) > 10.0f)
                    z = target->GetPositionZ();

                vanessa->NearTeleportTo(x, y, z, target->GetOrientation(), true);
                vanessa->getThreatManager().resetAllAggro();

                if (vanessa->IsAIEnabled)
                    vanessa->AI()->AttackStart(target);

                vanessa->AddThreat(target, 100.0f);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_deadmines_backslash_SpellScript::JumpToTarget, EFFECT_0, SPELL_EFFECT_TELEPORT_UNITS);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deadmines_backslash_SpellScript();
        }
};

void AddSC_boss_vanessa_vancleef()
{
    new boss_vanessa_vancleef();
    new npc_vanessas_rope();
    new npc_defias_enforcer();
    new npc_defias_shadowguard();
    new npc_defias_blood_wizard();

    new spell_bloodwash();
    new spell_backslash_targeting();
    new spell_deadmines_backslash();
}
