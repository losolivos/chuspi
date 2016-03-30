/*
    Yan-zhu the Uncasked
    By Emre
*/

#include "stormstout_brewery.h"

struct ValuePair
{
    uint32 uiEntry_1;
    uint32 uiEntry_2;
};

class NotCorrectHeightPredicate
{
public:
    bool operator()(WorldObject* target) const
    {
        return target && target->GetPositionZ() < 160.f;
    }
};

static const ValuePair aYanzhuPairs[3] =
{
    { NPC_YEASTY_ALEMENTAL, NPC_BUBBLING_ALEMENTAL },
    { NPC_FIZZY_ALEMENTAL, NPC_SUDSY_ALEMENTAL },
    { NPC_BLOATED_ALEMENTAL, NPC_STOUT_ALEMENTAL },
};

static const Position apGaoWps[] = 
{
    { 0.f, 0.f, 0.f },
    { -703.3f, 1187.5f, 166.7f },
    { -707.4f, 1189.8f, 166.7f },
    { -709.6f, 1185.f, 166.7f, 4.905f}
};

static const Position apChenWps[] =
{
    { -673.7f, 1155.7f, 166.7f },
    { 0.f, 0.f, 0.f }, // empty
    { -708.1f, 1180.2f, 166.7f, 1.832f }
};

static const Position apLargeBrewPos[] =
{
    { -702.09f, 1158.76f, 166.22f, 0.24f },
    { -704.20f, 1166.94f, 166.22f, 0.24f }
};

static const Position apSmallBrewPos[] =
{
    { -709.23f, 1152.57f, 166.22f, 0.17f },
    { -711.52f, 1160.55f, 166.22f, 0.17f },
    { -713.58f, 1168.67f, 166.22f, 0.17f },
    { -701.03f, 1154.85f, 166.22f, 0.17f },
    { -703.38f, 1162.76f, 166.22f, 0.17f },
    { -705.75f, 1170.97f, 166.22f, 0.17f },
    { -693.20f, 1157.04f, 166.22f, 0.17f },
    { -695.33f, 1165.07f, 166.22f, 0.17f },
    { -697.44f, 1173.05f, 166.22f, 0.17f },
};

static const Position apMiddleBrewPos[] =
{
    { -700.52f, 1167.84f, 166.22f, 0.25f },
    { -698.25f, 1159.97f, 166.22f, 0.25f },
    { -708.61f, 1165.69f, 166.22f, 0.25f },
    { -706.43f, 1157.70f, 166.22f, 0.25f },
};

static const Position pYanzhuPos[] = { -703.44f, 1162.43f, 166.22f, 0.24f };
static const Position pGaoPotPos[] = { -676.96f, 1193.96f, 166.79f, 1.82f };

static const Position apSudsPos[2] = 
{ 
    { -764.81f, 1190.57f, 167.99f, 0.26f },
    { -652.95f, 1136.13f, 167.99f, 1.82f }
};

const uint32 m_auiBrewSpells[3] =
{
    128253,
    128255,
    128257,
};

class AliveCheck
{
public:
    AliveCheck(Creature* pCreature) : _creature(pCreature) { }

    bool operator()(uint64 uiGuid) 
    {
        return (GetAffectedCreature(uiGuid) && !GetAffectedCreature(uiGuid)->IsAlive());
    }

private:
    Creature* _creature;
    Creature* GetAffectedCreature(uint64 guid)
    {
        Creature* pCreature = ObjectAccessor::GetCreature(*_creature, guid);
        return pCreature ? pCreature : nullptr;
    }
};

class npc_uncle_gao : public CreatureScript
{
public:
    npc_uncle_gao() : CreatureScript("npc_uncle_gao") { }

    enum eEncounterStage : uint32
    {
        STAGE_MIDDLE_ADDS,
        STAGE_SMALL_ADDS,
        STAGE_LARGE_ADDS,
        STAGE_BOSS
    };

    enum eTalks : uint32
    {
        TALK_SUMMON1,
        TALK_SUMMON2,
        TALK_SUMMON3,
        TALK_SUMMON_BOSS,
        TALK_BOSS_DEATH,
        TALK_OUTRO_1,
        TALK_OUTRO_2,
        TALK_OUTRO_3,
        TALK_OUTRO_4,
        TALK_OUTRO_5,
        TALK_OUTRO_6
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_HANDLE_SUMMONING,
        EVENT_HANDLE_SPELLCASTING,
        EVENT_HANDLE_CRAFT_ANIMATION,
        EVENT_NEXT_POT,
        EVENT_OUTRO_1, // such flavor
        EVENT_OUTRO_2, // name of stormstout
        EVENT_OUTRO_3,
        EVENT_OUTRO_4,
        EVENT_OUTRO_5,
        EVENT_OUTRO_6,
        EVENT_OUTRO_7,
        EVENT_MOVECHECK,
        EVENT_UPDATE_ENCOUNTER,
        EVENT_UPDATE_DOOR_STATE
    };

    enum eActions : int32
    {
        ACTION_START_OUTRO,
        ACTION_OUTRO_3,
        ACTION_OUTRO_4,
        ACTION_OUTRO_6,
        ACTION_START_BOSS,
        ACTION_SUMMON_BOSS
    };

    enum eSpells : uint32
    {
        SPELL_BREW_FINALE_WHEAT    = 128253,
        SPELL_BREW_FINALE_MEDIUM   = 128255,
        SPELL_BREW_FINALE_DARK     = 128257,

        SPELL_SMALL_SPAWN          = 128242,
        SPELL_MEDIUM_SPAWN         = 128243,
        SPELL_LARGE_SPAWN          = 128244
    };

    enum eCreatures : uint32
    {
        NPC_JMF_BUNNY    = 45979,
    };

    struct npc_uncle_gaoAI : public ScriptedAI
    {
        npc_uncle_gaoAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_bEncounterStarted = false;
        }

        uint32 m_uiEncounterStage;
        uint32 m_uiWaypoint;
        
        uint32 m_auiAddStore[3];

        uint64 m_uiDoorGuid;

        bool m_bEncounterStarted;

        std::list<uint64> m_lCurrentStageGuids;

        void Reset()
        {
            InitializeMyself();
        }

        uint32 GetAddToSummonEntry(uint32 uiType)
        {
            if (me->GetInstanceScript())
                return me->GetInstanceScript()->GetData(uiType) ? aYanzhuPairs[uiType - 3].uiEntry_2 : aYanzhuPairs[uiType - 3].uiEntry_1;

            return 0;
        }

        void MoveInLineOfSight(Unit* pWho)
        {
            if (!m_bEncounterStarted)
            {
                if (pWho && pWho->ToPlayer() && pWho->GetDistance(me) < 30.f && pWho->GetPositionZ() > 160.f)
                    StartEncounter();
            }
        }

        void InitializeMyself()
        {
            if (me->GetInstanceScript())
            {
                if (me->GetInstanceScript()->GetData(DATA_YAN_ZHU) == DONE)
                {
                    m_bEncounterStarted = true;

                    me->NearTeleportTo(apGaoWps[3].m_positionX, apGaoWps[3].m_positionY, apGaoWps[3].m_positionZ, apGaoWps[3].m_orientation);

                    GetChenAndDoAction(4);
                    return;
                }
            }
        }

        void GetDoor()
        {
            std::list<GameObject*>temp;
            GetGameObjectListWithEntryInGrid(temp, me, GO_SLIDING_DOOR, 70.f);
            temp.remove_if(NotCorrectHeightPredicate());
            std::list<GameObject*>::iterator doorItr = temp.begin();
            if (doorItr != temp.end())
                m_uiDoorGuid = (*doorItr)->GetGUID();
        }

        void StartEncounter()
        {
            m_bEncounterStarted = true;
            m_uiEncounterStage = 0;

            for (int i = DATA_SMALL_ADDS; i < DATA_LARGE_ADDS + 1; ++i)
            {
                m_auiAddStore[i - 3] = GetAddToSummonEntry(i);
            }

            GetDoor();
            AdvanceEncounter();
        }

        bool DoSummonSmallAdds(int n)
        {
            if (n < 0)
            {
                events.ScheduleEvent(EVENT_UPDATE_DOOR_STATE, 5000);
                return false;
            }

            if (Creature* pCreature = me->SummonCreature(m_auiAddStore[0], apSmallBrewPos[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, urand(14000, 24000)))
            {
                m_lCurrentStageGuids.push_back(pCreature->GetGUID());
                pCreature->CastSpell(pCreature, SPELL_SMALL_SPAWN);
            }

            return DoSummonSmallAdds(n - 1);
        }

        bool DoSummonMediumAdds(int n)
        {
            if (n < 0)
            {
                events.ScheduleEvent(EVENT_UPDATE_DOOR_STATE, 5000);
                return false;
            }

            if (Creature* pCreature = me->SummonCreature(m_auiAddStore[1], apMiddleBrewPos[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, urand(14000, 24000)))
            {
                m_lCurrentStageGuids.push_back(pCreature->GetGUID());
                pCreature->CastSpell(pCreature, SPELL_MEDIUM_SPAWN);
            }

            return DoSummonMediumAdds(n - 1);
        }

        bool DoSummonLargeAdds(int n)
        {
            if (n < 0)
            {
                events.ScheduleEvent(EVENT_UPDATE_DOOR_STATE, 5000);
                return false;
            }

            if (Creature* pCreature = me->SummonCreature(m_auiAddStore[2], apLargeBrewPos[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, urand(14000, 24000)))
            {
                m_lCurrentStageGuids.push_back(pCreature->GetGUID());
                pCreature->CastSpell(pCreature, SPELL_LARGE_SPAWN);
            }

            return DoSummonLargeAdds(n - 1);
        }

        void DoSummonBoss()
        {
            events.ScheduleEvent(EVENT_UPDATE_DOOR_STATE, 5000);

            DoTalk(TALK_SUMMON_BOSS);
            if (Creature* pCreature = me->SummonCreature(NPC_YAN_ZHU, *pYanzhuPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5 * MINUTE*IN_MILLISECONDS))
            {
                m_lCurrentStageGuids.push_back(pCreature->GetGUID());
                pCreature->CastSpell(pCreature, SPELL_LARGE_SPAWN);
                pCreature->CastSpell(pCreature, SPELL_MEDIUM_SPAWN);
                pCreature->CastSpell(pCreature, SPELL_SMALL_SPAWN);
            }
        }

        void StartOutro()
        {
            me->HandleEmoteCommand(EMOTE_STATE_NONE);

            if (me->GetInstanceScript())
            {
                if (Creature* pYanZhu = ObjectAccessor::GetCreature(*me, me->GetInstanceScript()->GetData64(NPC_YAN_ZHU)))
                {
                    float x, y;
                    GetPositionWithDistInOrientation(me, (me->GetDistance(pYanZhu) - 1.5f), me->GetAngle(pYanZhu), x, y);

                    m_uiWaypoint = 0;
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(m_uiWaypoint, x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ(), true, 100.0f));
                }
            }

            events.Reset();
            Talk(TALK_BOSS_DEATH);
        }

        void GetChenAndDoAction(int32 iAction)
        {
            if (me->GetInstanceScript())
            {
                if (Creature* pChen = ObjectAccessor::GetCreature(*me, me->GetInstanceScript()->GetData64(NPC_CHEN_YANZHU)))
                {
                    if (pChen->AI())
                        pChen->AI()->DoAction(iAction);
                }
            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            switch (uiPointId)
            {
            case 0:
                ++m_uiWaypoint;
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                events.ScheduleEvent(EVENT_OUTRO_1, 3300);
                break;
            case 1:
            case 2:
                ++m_uiWaypoint;
                events.ScheduleEvent(EVENT_MOVECHECK, 100);
                break;
            case 3:
                ++m_uiWaypoint;
                events.ScheduleEvent(EVENT_OUTRO_7, 3400);
                me->SetFacingTo(apGaoWps[uiPointId].m_orientation);
                break;
            case 101:
                me->SetFacingTo(pGaoPotPos->m_orientation);
                me->HandleEmoteCommand(EMOTE_STATE_USE_STANDING);
                break;
            }
        }

        void DoAction(const int32 iAction)
        {
            switch (iAction)
            {
            case ACTION_START_OUTRO:
                StartOutro();
                break;
            case ACTION_OUTRO_3:
                events.ScheduleEvent(EVENT_OUTRO_3, 6000);
                break;
            case ACTION_OUTRO_4:
                events.ScheduleEvent(EVENT_OUTRO_4, 5300);
                break;
            case ACTION_OUTRO_6:
                events.ScheduleEvent(EVENT_OUTRO_6, 3800);
                break;
            case ACTION_START_BOSS:
                if (m_bEncounterStarted)
                    return;

                StartEncounter();
                break;
            case ACTION_SUMMON_BOSS:
                AdvanceEncounter();
                break;
            }
        }

        bool DoUpdateDoorState()
        {
            events.ScheduleEvent(EVENT_UPDATE_DOOR_STATE, 5000);

            for (auto itr : m_lCurrentStageGuids)
            {
                if (Creature* pCreature = ObjectAccessor::GetCreature(*me, itr))
                {
                    if (pCreature->IsInCombat())
                        return false;
                }
            }

            return me->GetInstanceScript()->GetData(DATA_YAN_ZHU) != IN_PROGRESS;
        }

        bool UpdateCurrentEncounterState(bool m_bIsBoss)
        {
            if (!m_bIsBoss)
            {
                if (m_lCurrentStageGuids.empty())
                    return AdvanceEncounter();

                m_lCurrentStageGuids.remove_if(AliveCheck(me));
            }

            return true;
        }

        void JustSummoned(Creature* pSummoned)
        {
            if (pSummoned)
            {
                if (pSummoned->AI())
                    pSummoned->AI()->DoZoneInCombat();
            }
        }

        bool AdvanceEncounter()
        {
            events.CancelEvent(EVENT_UPDATE_ENCOUNTER);
            events.CancelEvent(EVENT_UPDATE_DOOR_STATE);
         
            switch (m_uiEncounterStage)
            {
            case 0:
                events.ScheduleEvent(EVENT_HANDLE_SUMMONING, 9000);
                events.ScheduleEvent(EVENT_HANDLE_SPELLCASTING, 5000);
                events.ScheduleEvent(EVENT_NEXT_POT, 20000);
                DoTalk(TALK_SUMMON1);
                break;
            case 1:
                events.ScheduleEvent(EVENT_HANDLE_SUMMONING, 12000);
                events.ScheduleEvent(EVENT_HANDLE_SPELLCASTING, 7000);
                DoTalk(TALK_SUMMON2);
                break;
            case 2:
                events.ScheduleEvent(EVENT_HANDLE_SUMMONING, 9000);
                events.ScheduleEvent(EVENT_HANDLE_SPELLCASTING, 5000);
                DoTalk(TALK_SUMMON3);
                break;
            case 3:
                events.ScheduleEvent(EVENT_HANDLE_SPELLCASTING, 100);
                events.ScheduleEvent(EVENT_HANDLE_SUMMONING, 5100);
                break;
            }

            ++m_uiEncounterStage;

            return true;
        }

        void DoHandleSpellcasting()
        {
            if (Creature* pCaster = GetClosestCreatureWithEntry(me, NPC_JMF_BUNNY, 10.f))
            {
                uint32 m_uiSpell = 0;

                switch (m_uiEncounterStage-1)
                {
                case 0:
                case 1:
                case 2:
                    m_uiSpell = m_auiBrewSpells[m_uiEncounterStage - 1];
                    break;
                case 3:
                    for (int i = 0; i < 2; ++i)
                        pCaster->CastSpell(pCaster, m_auiBrewSpells[i], true);
                    return;

                }

                pCaster->CastSpell(pCaster, m_uiSpell, true);             
            }
        }

        void HandleDoor(bool open)
        {
            if (GameObject* pGo = ObjectAccessor::GetGameObject(*me, m_uiDoorGuid))
            {
                if (open)
                {              
                    if (pGo->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND))
                        pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);

                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                else
                {
                    if (!pGo->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND))
                        pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);

                    pGo->SetGoState(GO_STATE_READY);
                }
            }
        }

        void DoTalk(uint8 m_uiId)
        {
            Talk(m_uiId);
            events.ScheduleEvent(EVENT_HANDLE_CRAFT_ANIMATION, 2100);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_OUTRO_1:
                    Talk(TALK_OUTRO_1);
                    events.ScheduleEvent(EVENT_OUTRO_2, 8900);
                    break;
                case EVENT_OUTRO_2:
                    Talk(TALK_OUTRO_2);
                    me->SetStandState(UNIT_STAND_STATE_STAND);

                    // Start Chen's part of the script
                    GetChenAndDoAction(0);
                    break;
                case EVENT_OUTRO_3:
                    Talk(TALK_OUTRO_3);

                    // Set Chen's next timer (EVENT_OUTRO_1) 10k
                    GetChenAndDoAction(1);
                    break;
                case EVENT_OUTRO_4:
                    Talk(TALK_OUTRO_4);

                    GetChenAndDoAction(2);
                    // call action to chens script setting his timer to 2500
                    // and move timer to 6500
                    // also set our own move timer (which also is timer for next event) to 6700
                    // "and there are virmen"
                    events.ScheduleEvent(EVENT_OUTRO_5, 6600);
                    break;
                case EVENT_OUTRO_5:
                    Talk(TALK_OUTRO_5);

                    me->SetWalk(true);
                    events.ScheduleEvent(EVENT_MOVECHECK, 100);
                    break;
                case EVENT_OUTRO_6:
                    Talk(TALK_OUTRO_6);
                    break;
                case EVENT_OUTRO_7:
                    GetChenAndDoAction(3);
                    break;
                case EVENT_MOVECHECK:
                    me->GetMotionMaster()->MovePoint(m_uiWaypoint, apGaoWps[m_uiWaypoint]);
                    break;
                case EVENT_HANDLE_SUMMONING:
                    events.ScheduleEvent(EVENT_UPDATE_ENCOUNTER, 3000);

                    switch (m_uiEncounterStage-1)
                    {
                    case STAGE_MIDDLE_ADDS:
                        DoSummonMediumAdds(3);
                        break;
                    case STAGE_SMALL_ADDS:
                        DoSummonSmallAdds(8);
                        break;
                    case STAGE_LARGE_ADDS:
                        DoSummonLargeAdds(1);
                        break;
                    case STAGE_BOSS:
                        events.CancelEvent(EVENT_UPDATE_ENCOUNTER);
                        DoSummonBoss();
                        break;
                    }

                    break;
                case EVENT_HANDLE_SPELLCASTING:
                    DoHandleSpellcasting();
                    break;
                case EVENT_UPDATE_ENCOUNTER:
                    events.ScheduleEvent(EVENT_UPDATE_ENCOUNTER, 1000);
                    UpdateCurrentEncounterState(m_uiEncounterStage > STAGE_BOSS);
                    break;
                case EVENT_NEXT_POT:
                    me->GetMotionMaster()->MovePoint(101, *pGaoPotPos);
                    break;
                case EVENT_HANDLE_CRAFT_ANIMATION:
                    me->HandleEmoteCommand(EMOTE_STATE_USE_STANDING);
                    break;
                case EVENT_UPDATE_DOOR_STATE:
                    HandleDoor(DoUpdateDoorState());
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_uncle_gaoAI(pCreature);
    }
};

class npc_chen_yanzhu : public CreatureScript
{
public:
    npc_chen_yanzhu() : CreatureScript("npc_chen_yanzhu") { }

    enum eTalks : uint32
    {
        TALK_CARE,
        TALK_COST,
        TALK_VIRMEN,
        TALK_FIRE
    };

    enum eActions : int32
    {
        ACTION_INIT,
        ACTION_OUTRO_1,
        ACTION_OUTRO_2,
        ACTION_OUTRO_3,
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_MOVECHECK,
        EVENT_MOVEBOSS,
        EVENT_CARE,
        EVENT_OUTRO_1,
        EVENT_OUTRO_2
    };

    struct npc_chen_yanzhuAI : public ScriptedAI
    {
        npc_chen_yanzhuAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        uint32 m_uiWaypoint;

        void Reset()
        {
            InitializeMyself();
        }

        void InitScript()
        {
            m_uiWaypoint = 0;
            me->SetWalk(false);
            events.ScheduleEvent(EVENT_MOVECHECK, 100);
            me->SetPhaseMask(1, true);

            if (GameObject* pGo = GetClosestGameObjectWithEntry(me, GO_SLIDING_DOOR, 20))
            {
                pGo->SetGoState(GO_STATE_ACTIVE);
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
            }
        }

        void DoAction(const int32 iAction)
        {
            switch (iAction)
            {
            case ACTION_INIT:
                InitScript();
                break;
            case ACTION_OUTRO_1:
                events.ScheduleEvent(EVENT_OUTRO_1, 10000);
                break;
            case ACTION_OUTRO_2:
                events.ScheduleEvent(EVENT_OUTRO_2, 2500);
                events.ScheduleEvent(EVENT_MOVECHECK, 6500);
                me->SetWalk(true);
                break;
            case ACTION_OUTRO_3:
                Talk(TALK_FIRE);
                GetGaoAndDoAction(3);
                break;
            case 4:
                InitializeMyself();
                break;
            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            switch (uiPointId)
            {
            case 0:
                events.ScheduleEvent(EVENT_MOVEBOSS, 100);
                break;
            case 1:
                events.ScheduleEvent(EVENT_CARE, 2400);
                break;
            case 2:
                me->SetFacingTo(apChenWps[uiPointId].m_orientation);
                break;
            }

            ++m_uiWaypoint;          
        }

        void GetGaoAndDoAction(int32 iAction)
        {
            if (me->GetInstanceScript())
            {
                if (Creature* pGao = ObjectAccessor::GetCreature(*me, me->GetInstanceScript()->GetData64(NPC_UNCLE_GAO)))
                {
                    if (pGao->AI())
                        pGao->AI()->DoAction(iAction);
                }
            }
        }
        
        void InitializeMyself()
        {
            if (me->GetInstanceScript())
            {
                if (me->GetInstanceScript()->GetData(DATA_YAN_ZHU) == DONE)
                {
                    me->NearTeleportTo(apChenWps[2].m_positionX, apChenWps[2].m_positionY, apChenWps[2].m_positionZ, apChenWps[2].m_orientation);
                    me->SetPhaseMask(1, true);
                }
            }

        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CARE:
                    Talk(TALK_CARE);

                    // Start Gao's next timer
                    GetGaoAndDoAction(1);
                    break;
                case EVENT_OUTRO_1:
                    Talk(TALK_COST);

                    // Start Gao's next timer
                    GetGaoAndDoAction(2);
                    break;
                case EVENT_OUTRO_2:
                    Talk(TALK_VIRMEN);
                    break;
                case EVENT_MOVECHECK:
                    me->GetMotionMaster()->MovePoint(m_uiWaypoint, apChenWps[m_uiWaypoint]);
                    break;
                case EVENT_MOVEBOSS:
                    if (me->GetInstanceScript())
                    {
                        if (Creature* pYanzhu = ObjectAccessor::GetCreature(*me, me->GetInstanceScript()->GetData64(NPC_YAN_ZHU)))
                        {
                            float x, y;

                            GetPositionWithDistInOrientation(me, (me->GetDistance(pYanzhu) - 1.5f), me->GetAngle(pYanzhu), x, y);
                            me->GetMotionMaster()->MovePoint(1, x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ(), true, 100.f));
                        }
                    }
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_chen_yanzhuAI(pCreature);
    }
};


class boss_yanzhu : public CreatureScript
{
public:
    boss_yanzhu() : CreatureScript("boss_yanzhu") { }

    enum eSpells : uint32
    {
        SPELL_BLACKOUT             = 106851,
        SPELL_BREW_BOLT            = 114548,
        SPELL_BLOAT                = 106546,
        SPELL_CARBONATION          = 115003,
        SPELL_BUBBLE_SHIELD        = 106563,
        SPELL_FORCE_VEHICLE        = 46598,
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_WALL_OF_SUDS,
        EVENT_BLOAT,
        EVENT_BLACKOUT,
        EVENT_ADDS,
        EVENT_BUBBLE_SHIELD,
        EVENT_CARBONATION
    };

    enum eCreatures : uint32
    {
        NPC_BUBBLE_SHIELD           = 65522
    };

    struct boss_yanzhuAI : public BossAI
    {
        boss_yanzhuAI(Creature* pCreature) : BossAI(pCreature, DATA_YAN_ZHU)
        {
            SetCombatMovement(false);
        }

        std::vector<uint64> m_lGuidList;

        void EnterCombat(Unit* pWho)
        {
            if (me->GetInstanceScript())
                me->GetInstanceScript()->SetData(DATA_YAN_ZHU, IN_PROGRESS);

            // Disabled until targeting type is fixed
      /*      if (me->HasAura(SPELL_SUDSY_BREW))
                events.ScheduleEvent(EVENT_WALL_OF_SUDS, urand(9000,17000)); */

            if (me->HasAura(SPELL_YEASTY_BREW))
                events.ScheduleEvent(EVENT_ADDS, urand(9000, 17000));

            if (me->HasAura(SPELL_BUBBLING_BREW))
            {
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                events.ScheduleEvent(EVENT_BUBBLE_SHIELD, 6000, 14000);
            }

            events.ScheduleEvent(EVENT_BLACKOUT, 8000);
            events.ScheduleEvent(EVENT_BLOAT, 8000);
            events.ScheduleEvent(EVENT_CARBONATION, urand(9000, 17000));       
        }

        void Reset()
        {
            if (me->GetInstanceScript())
                me->GetInstanceScript()->SetData(DATA_YAN_ZHU, NOT_STARTED);

            events.Reset();
            summons.DespawnAll();
        }

        void EnterEvadeMode()
        {
            for (auto uiGuid : m_lGuidList)
            {
                if (Creature* pBubble = ObjectAccessor::GetCreature(*me, uiGuid))
                {
                    pBubble->ExitVehicle();
                    pBubble->AddObjectToRemoveList();
                }
            }

            m_lGuidList.clear();

            me->AttackStop();
            me->CombatStop(true);
            me->RemoveAllAurasExceptType(SPELL_AURA_DUMMY);
            me->SetLootRecipient(NULL);

            me->GetMotionMaster()->MoveTargetedHome();

            Reset();
        }

        void JustDied(Unit* pKiller)
        {
            if (me->GetInstanceScript())
            {
                if (Creature* pGao = ObjectAccessor::GetCreature(*me, me->GetInstanceScript()->GetData64(NPC_UNCLE_GAO)))
                {
                    if (pGao->AI())
                        pGao->AI()->DoAction(0);
                }

                me->GetInstanceScript()->SetData(DATA_YAN_ZHU, DONE);
            }

            for (auto uiGuid : m_lGuidList)
            {
                if (Creature* pBubble = ObjectAccessor::GetCreature(*me, uiGuid))
                {
                    pBubble->AddObjectToRemoveList();
                }
            }

            m_lGuidList.clear();

            BossAI::JustDied(pKiller);
        }

        void DoSummonWallOfSuds()
        {
            for (int i = 0; i < 2; ++i)
            {
                if (Creature* pSuds = me->SummonCreature(NPC_WALL_OF_SUDS, apSudsPos[i]))
                    pSuds->SetCanFly(true);
            }
        }

        void JustSummoned(Creature* pSummoned)
        {
            if (pSummoned)
                summons.Summon(pSummoned);
        }

        bool DoSummonFizzyBubbles(int n)
        {
            if (n < 0)
                return false;

            Position pos;

            me->GetRandomNearPosition(pos, 20.f);
            pos.m_positionZ += frand(3.f, 6.f);

            me->SummonCreature(NPC_FIZZY_BUBBLE, pos, TEMPSUMMON_MANUAL_DESPAWN);

            return DoSummonFizzyBubbles(n - 1);
        }

        // function should return false if it does what we want it to do
        bool UpdateAttackState()
        {
            if (me->isAttackReady() && !me->HasUnitState(UNIT_STATE_CASTING))
            {
                if (me->GetVictim())
                {
                    if (me->IsWithinMeleeRange(me->GetVictim()))
                    {
                        me->AttackerStateUpdate(me->GetVictim());
                        me->resetAttackTimer();
                        return false;
                    }
                    else
                    {
                        if (!DoSpellAttackIfReady(SPELL_BREW_BOLT) && !IsCombatMovementAllowed())
                        {
                            std::list<Unit*> lTargetList;
                            me->GetAttackableUnitListInRange(lTargetList, 35.f);

                            if (lTargetList.empty())
                                return true;

                            std::list<Unit*>::iterator itr = lTargetList.begin();
                            Unit* pUnit;

                            for (; itr != lTargetList.end();)
                            {
                                pUnit = *itr;

                                if (pUnit)
                                {
                                    AttackStart(pUnit);
                                    return false;
                                }
                                else
                                    ++itr;
                            }

                            std::list<HostileReference*> const &tList = me->getThreatManager().getThreatList();

                            if (tList.empty())
                                return false;

                            for (std::list<HostileReference*>::const_iterator itr = tList.begin(); itr != tList.end(); ++itr)
                            {
                                if (Unit* pUnit = ObjectAccessor::GetUnit(*me, (*itr)->getUnitGuid()))
                                {
                                    if (me->GetDistance(pUnit) < 70.f)
                                    {
                                        AttackStart(pUnit);
                                        return false;
                                    }
                                }

                            }
                        }
                    }
                }
                return true;
            }
            return false;
        }

        void ApplyBubbleShield(int n)
        {
            Position pos;
            me->GetPosition(&pos);

            for (int i = n; i < 8; ++i)
            {
                if (Creature* pBubbleShield = me->SummonCreature(NPC_BUBBLE_SHIELD, pos, TEMPSUMMON_CORPSE_DESPAWN))
                {
                    me->AddAura(SPELL_BUBBLE_SHIELD, me);

                    pBubbleShield->CastSpell(me, SPELL_FORCE_VEHICLE, true);
                    m_lGuidList.push_back(pBubbleShield->GetGUID());
                }
            }
        }

        bool DoSummonAdds(int n)
        {
            if (n < 0)
                return false;

            Position pos;
            me->GetRandomNearPosition(pos, frand(5.f, 11.f));

            if (Creature* pAdd = me->SummonCreature(NPC_YEASTY_ALEMENTAL, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, urand(6000, 14000)))
            {
                if (pAdd->AI())
                    pAdd->AI()->DoZoneInCombat();
            }

            return DoSummonAdds(n - 1);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_WALL_OF_SUDS:
                    DoSummonWallOfSuds();
                    events.ScheduleEvent(EVENT_WALL_OF_SUDS, 13000);
                    break;
                case EVENT_BLOAT:
                    if (Unit* pUnit = SelectTarget(SELECT_TARGET_RANDOM, NULL, 50.f, true, NULL))
                    {
                        DoCast(pUnit, SPELL_BLOAT);
                        events.ScheduleEvent(EVENT_BLOAT, urand(10000, 18000));
                    }
                    else
                        events.ScheduleEvent(EVENT_BLOAT, 1000);
                    break;
                case EVENT_BLACKOUT:
                    DoCast(SPELL_BLACKOUT);
                    events.ScheduleEvent(EVENT_BLACKOUT, 8000);
                    break;
                case EVENT_BUBBLE_SHIELD:
                    if (me->GetAura(SPELL_BUBBLE_SHIELD))
                        ApplyBubbleShield(0 + me->GetAura(SPELL_BUBBLE_SHIELD)->GetStackAmount());
                    else
                        ApplyBubbleShield(0);

                    events.ScheduleEvent(EVENT_BUBBLE_SHIELD, 30000);
                    break;
                case EVENT_ADDS:
                    DoSummonAdds(7);
                    events.ScheduleEvent(EVENT_ADDS, 30000);
                    break;
                case EVENT_CARBONATION:
                    events.ScheduleEvent(EVENT_CARBONATION, urand(35000, 48000));
                    DoCast(SPELL_CARBONATION);
                    DoSummonFizzyBubbles(5);
                    break;
                }
            }

            SetCombatMovement(UpdateAttackState());
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_yanzhuAI(pCreature);
    }
};

class npc_yeasty_alemental : public CreatureScript
{
public:
    npc_yeasty_alemental() : CreatureScript("npc_yeasty_alemental") {}

    enum eSpells : uint32
    {
        SPELL_BREW_BOLT     = 116155,
        SPELL_FERMENT       = 106859
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_FERMENT
    };

    struct npc_yeasty_alementalAI : public ScriptedAI
    {
        npc_yeasty_alementalAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            SetCombatMovement(false);
        }

        void EnterCombat(Unit* pTarget)
        {
            events.ScheduleEvent(EVENT_FERMENT, 10000);

            std::list<Creature*>temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 10.f);

            for (auto itr : temp)
            {
                if (itr->AI())
                    itr->AI()->DoZoneInCombat();
            }
        }

        void Reset()
        {}

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FERMENT:
                    if (me->GetInstanceScript())
                    {
                        if (Creature* pBoss = ObjectAccessor::GetCreature(*me, me->GetInstanceScript()->GetData(NPC_YAN_ZHU)))
                            DoCast(pBoss, SPELL_FERMENT);

                        events.ScheduleEvent(EVENT_FERMENT, 15000);
                    }
                    break;
                }
            }

            if (!DoSpellAttackIfReady(SPELL_BREW_BOLT))
            {
                SetCombatMovement(true);
                AttackStart(me->GetVictim());
                DoMeleeAttackIfReady();
                return;
            }
            
            SetCombatMovement(false);
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_yeasty_alementalAI(pCreature);
    }
};

class npc_sudsy_alemental : public CreatureScript
{
public:
    npc_sudsy_alemental() : CreatureScript("npc_sudsy_alemental") { }
    
    enum eSpells : uint32
    {
        SPELL_BREW_BOLT   = 115650,
        SPELL_SUDS        = 116178,
        SPELL_SUDS_AURA   = 116179
    };

    enum eCreatures : uint32
    {
        NPC_SUDS_TRIGGER = 56748
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_SUDS,
        EVENT_BREW_BOLT
    };

    struct npc_sudsy_alementalAI : public ScriptedAI
    {
        npc_sudsy_alementalAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        void EnterCombat(Unit* pWho)
        {
            events.ScheduleEvent(EVENT_SUDS, urand(5000, 15000));
            events.ScheduleEvent(EVENT_BREW_BOLT, urand(4000, 11000));

            std::list<Creature*>temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 10.f);

            for (auto itr : temp)
            {
                if (itr->AI())
                    itr->AI()->DoZoneInCombat();
            }
        }

        void Reset()
        {}

        void JustSummoned(Creature* pSummoned)
        {
            if (pSummoned)
                pSummoned->CastSpell(pSummoned, SPELL_SUDS_AURA, true);
        }

        void EnterEvadeMode()
        {
            me->AttackStop();
            me->CombatStop(true);
            me->RemoveAllAurasExceptType(SPELL_AURA_DUMMY);
            me->SetLootRecipient(NULL);

            me->GetMotionMaster()->MoveTargetedHome();
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SUDS:
                    DoCast(SPELL_SUDS);
                    events.ScheduleEvent(EVENT_SUDS, urand(10000, 16000));
                    break;
                case EVENT_BREW_BOLT:
                    if (Unit* pUnit = SelectTarget(SELECT_TARGET_RANDOM, NULL, 50.f, true, NULL))
                        DoCast(pUnit, SPELL_BREW_BOLT);
                    events.ScheduleEvent(EVENT_BREW_BOLT, urand(6000, 14000));
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_sudsy_alementalAI(pCreature);
    }
};

class npc_fizzy_alemental : public CreatureScript
{
public:
    npc_fizzy_alemental() : CreatureScript("npc_fizzy_alemental") {}

    enum eSpells : uint32
    {
        SPELL_CARBONATION    = 116162,
        SPELL_BREW_BOLT      = 115650,
        SPELL_CARBONATE      = 116170
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_BREW_BOLT,
        EVENT_CARBONATION
    };

    struct npc_fizzy_alementalAI : public ScriptedAI
    {
        npc_fizzy_alementalAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        void EnterCombat(Unit* pWho)
        {
            events.ScheduleEvent(EVENT_CARBONATION, urand(5000, 15000));
            events.ScheduleEvent(EVENT_BREW_BOLT, urand(4000, 11000));

            std::list<Creature*>temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 10.f);

            for (auto itr : temp)
            {
                if (itr->AI())
                    itr->AI()->DoZoneInCombat();
            }
        }

        void Reset()
        {}

        void EnterEvadeMode()
        {
            me->AttackStop();
            me->CombatStop(true);
            me->RemoveAllAurasExceptType(SPELL_AURA_DUMMY);
            me->SetLootRecipient(NULL);

            me->GetMotionMaster()->MoveTargetedHome();
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BREW_BOLT:
                    if (Unit* pUnit = SelectTarget(SELECT_TARGET_RANDOM, NULL, 50.f, true, NULL))
                        DoCast(pUnit, SPELL_BREW_BOLT);
                    events.ScheduleEvent(EVENT_BREW_BOLT, urand(6000, 14000));
                    break;
                case EVENT_CARBONATION:
                    if (Unit* pUnit = SelectTarget(SELECT_TARGET_RANDOM, NULL, 50.f, true, NULL))
                        DoCast(pUnit, SPELL_CARBONATION);
                    events.ScheduleEvent(EVENT_CARBONATION, urand(10000, 1600));
                }
            }

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_fizzy_alementalAI(pCreature);
    }
};

class npc_bloated_alemental : public CreatureScript
{
public:
    npc_bloated_alemental() : CreatureScript("npc_bloated_alemental") {}

    enum eSpells : uint32
    {
        SPELL_BLOAT        = 106546,
        SPELL_BREW_BOLT    = 115652
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_BLOAT,
        EVENT_BREW_BOLT
    };

    struct npc_bloated_alementalAI : public ScriptedAI
    {
        npc_bloated_alementalAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        void EnterCombat(Unit* pWho)
        {
            events.ScheduleEvent(EVENT_BLOAT, urand(5000, 15000));
            events.ScheduleEvent(EVENT_BREW_BOLT, urand(4000, 11000));

            std::list<Creature*>temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 10.f);

            for (auto itr : temp)
            {
                if (itr->AI())
                    itr->AI()->DoZoneInCombat();
            }
        }

        void Reset()
        {}

        void EnterEvadeMode()
        {
            me->AttackStop();
            me->CombatStop(true);
            me->RemoveAllAurasExceptType(SPELL_AURA_DUMMY);
            me->SetLootRecipient(NULL);

            me->GetMotionMaster()->MoveTargetedHome();
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BREW_BOLT:
                    if (Unit* pUnit = SelectTarget(SELECT_TARGET_RANDOM, NULL, 50.f, true, NULL))
                        DoCast(pUnit, SPELL_BREW_BOLT);
                    events.ScheduleEvent(EVENT_BREW_BOLT, urand(6000, 14000));
                    break;
                case EVENT_BLOAT:
                    if (Unit* pUnit = SelectTarget(SELECT_TARGET_RANDOM, NULL, 99.f, true, NULL))
                        DoCast(pUnit, SPELL_BLOAT);
                    events.ScheduleEvent(EVENT_BLOAT, urand(8000,16000));
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_bloated_alementalAI(pCreature);
    }
};

class npc_stout_alemental : public CreatureScript
{
public:
    npc_stout_alemental() : CreatureScript("npc_stout_alemental") { }

    enum eSpells : uint32
    {
        SPELL_BLACKOUT     = 106851,
        SPELL_BREW_BOLT    = 115652
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_BLACKOUT,
        EVENT_BREW_BOLT
    };

    struct npc_stout_alementalAI : public ScriptedAI
    {
        npc_stout_alementalAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        void EnterCombat(Unit* pWho)
        {
            events.ScheduleEvent(EVENT_BLACKOUT, 8000);
            events.ScheduleEvent(EVENT_BREW_BOLT, urand(4000, 11000));

            std::list<Creature*>temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 10.f);

            for (auto itr : temp)
            {
                if (itr->AI())
                    itr->AI()->DoZoneInCombat();
            }
        }

        void Reset()
        {}

        void EnterEvadeMode()
        {
            me->AttackStop();
            me->CombatStop(true);
            me->RemoveAllAurasExceptType(SPELL_AURA_DUMMY);
            me->SetLootRecipient(NULL);

            me->GetMotionMaster()->MoveTargetedHome();
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BREW_BOLT:
                    if (Unit* pUnit = SelectTarget(SELECT_TARGET_RANDOM, NULL, 50.f, true, NULL))
                        DoCast(pUnit, SPELL_BREW_BOLT);
                    events.ScheduleEvent(EVENT_BREW_BOLT, urand(6000, 14000));
                    break;
                case EVENT_BLACKOUT:
                    DoCast(SPELL_BLACKOUT);
                    events.ScheduleEvent(EVENT_BLACKOUT, urand(6000, 10000));
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_stout_alementalAI(pCreature);
    }
};

class npc_bubbling_alemental : public CreatureScript
{
public:
    npc_bubbling_alemental() : CreatureScript("npc_bubbling_alemental") {}

    enum eSpells : uint32
    {
        SPELL_FORCE_VEHICLE = 46598,
        SPELL_BUBBLE_SHIELD = 128708,
        SPELL_BREW_BOLT     = 116155
    };

    enum eCreatures : uint32
    {
        NPC_BUBBLE_SHIELD   = 65522
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_BUBBLE_SHIELD
    };

    struct npc_bubbling_alementalAI : public ScriptedAI
    {
        npc_bubbling_alementalAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            SetCombatMovement(false);
        }

        std::vector<uint64>m_lGuidList;

        void EnterCombat(Unit* pWho)
        {
            events.ScheduleEvent(EVENT_BUBBLE_SHIELD, 5000);

            std::list<Creature*>temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 10.f);

            for (auto itr : temp)
            {
                if (itr->AI())
                    itr->AI()->DoZoneInCombat();
            }
        }

        void ApplyBubbleShield(int n)
        {
            Position pos;
            me->GetPosition(&pos);

            for (int i = n; i < 2; ++i)
            {
                if (Creature* pBubbleShield = me->SummonCreature(NPC_BUBBLE_SHIELD, pos, TEMPSUMMON_CORPSE_DESPAWN))
                {
                    me->AddAura(SPELL_BUBBLE_SHIELD, me);

                    pBubbleShield->CastSpell(me, SPELL_FORCE_VEHICLE, true);
                    m_lGuidList.push_back(pBubbleShield->GetGUID());
                }
            }
        }

        void EnterEvadeMode()
        {
            for (auto uiGuid : m_lGuidList)
            {
                if (Creature* pBubble = ObjectAccessor::GetCreature(*me, uiGuid))
                {
                    pBubble->ExitVehicle();
                    pBubble->AddObjectToRemoveList();
                }
            }

            m_lGuidList.clear();

            me->AttackStop();
            me->CombatStop(true);
            me->RemoveAllAurasExceptType(SPELL_AURA_DUMMY);
            me->SetLootRecipient(NULL);

            me->GetMotionMaster()->MoveTargetedHome();
        }

        void JustDied(Unit* pKiller)
        {
            for (auto uiGuid : m_lGuidList)
            {
                if (Creature* pBubble = ObjectAccessor::GetCreature(*me, uiGuid))
                {
                    pBubble->AddObjectToRemoveList();
                }
            }

            m_lGuidList.clear();

            ScriptedAI::JustDied(pKiller);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BUBBLE_SHIELD:
                    if (me->GetAura(SPELL_BUBBLE_SHIELD))
                        ApplyBubbleShield(0 + me->GetAura(SPELL_BUBBLE_SHIELD)->GetStackAmount());
                    else
                        ApplyBubbleShield(0);

                    events.ScheduleEvent(EVENT_BUBBLE_SHIELD, 12000);
                    break;
                }
            }

            if (!DoSpellAttackIfReady(SPELL_BREW_BOLT))
            {
                SetCombatMovement(true);
                AttackStart(me->GetVictim());
                DoMeleeAttackIfReady();
                return;
            }

            SetCombatMovement(false);
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_bubbling_alementalAI(pCreature);
    }
};

class npc_carbonate_trigger : public CreatureScript
{
public:
    npc_carbonate_trigger() : CreatureScript("npc_carbonate_trigger") {}

    enum eSpells : uint32
    {
        SPELL_CARBONATE_AURA  = 116168
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_DESPAWN,
        EVENT_CARBONATE
    };

    struct npc_carbonate_triggerAI : public ScriptedAI
    {
        npc_carbonate_triggerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset()
        {
            if (me->HasAura(SPELL_CARBONATE_AURA))
                return;

            events.ScheduleEvent(EVENT_DESPAWN, 8200);
            events.ScheduleEvent(EVENT_CARBONATE, 100);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_DESPAWN:
                    me->AddObjectToRemoveList();
                    break;
                case EVENT_CARBONATE:
                    DoCast(SPELL_CARBONATE_AURA);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_carbonate_triggerAI(pCreature);
    }
};

class npc_suds_trigger : public CreatureScript
{
public:
    npc_suds_trigger() : CreatureScript("npc_suds_trigger") {}

    enum eSpells : uint32
    {
        SPELL_SUDS_AURA = 116179
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_DESPAWN,
        EVENT_SUDS
    };

    struct npc_suds_triggerAI : public ScriptedAI
    {
        npc_suds_triggerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset()
        {
            if (me->HasAura(SPELL_SUDS_AURA))
                return;

            events.ScheduleEvent(EVENT_DESPAWN, 8200);
            events.ScheduleEvent(EVENT_SUDS, 100);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_DESPAWN:
                    me->AddObjectToRemoveList();
                    break;
                case EVENT_SUDS:
                    me->AddAura(SPELL_SUDS_AURA, me);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_suds_triggerAI(pCreature);
    }
};

class npc_bubble_shield : public CreatureScript
{
public:
    npc_bubble_shield() : CreatureScript("npc_bubble_shield") {}

    enum eSpells : uint32
    {
        SPELL_REMOVE_SHIELD     = 106563,
        SPELL_REMOVE_SHIELD2    = 128708
    };

    struct npc_bubble_shieldAI : public ScriptedAI
    {
        npc_bubble_shieldAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            SetCombatMovement(false);
        }
        
        void DamageTaken(Unit* pWho, uint32& uiDamage)
        {
            if (uiDamage >= me->GetHealth())
            {
                uiDamage = 0;
                if (Unit* pUnit = me->GetVehicleBase())
                    pUnit->RemoveAuraFromStack(pUnit->GetEntry() == NPC_YAN_ZHU ? SPELL_REMOVE_SHIELD : SPELL_REMOVE_SHIELD2);

                me->ExitVehicle();
                me->AddObjectToRemoveList();
            }
        }

        void Reset(Unit* pWho)
        {}

        void EnterCombat(Unit* pWho)
        {}

        void UpdateAI(const uint32 uiDiff)
        {
            return;
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_bubble_shieldAI(pCreature);
    }
};

class npc_fizzy_bubble : public CreatureScript
{
public:
    npc_fizzy_bubble() : CreatureScript("npc_fizzy_bubble") {}

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_DESPAWN
    };

    struct npc_fizzy_bubbleAI : public ScriptedAI
    {
        npc_fizzy_bubbleAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
            me->SetCanFly(true);
        }

        void OnSpellClick(Unit* pClicker, bool& result)
        {
            pClicker->AddAura(114459, pClicker);

            me->AddObjectToRemoveList();
            return;
        }

        void Reset()
        {
            events.ScheduleEvent(EVENT_DESPAWN, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_DESPAWN:
                    me->AddObjectToRemoveList();
                    return;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_fizzy_bubbleAI(pCreature);
    }
};

class npc_wall_of_suds : public CreatureScript
{
public:
    npc_wall_of_suds() : CreatureScript("npc_wall_of_suds") {}

    enum eSpells : uint32
    {
        SPELL_WALL_OF_SUDS    = 114467,
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_INITIALIZE,
        EVENT_MOVE_NEXT
    };

    struct npc_wall_of_sudsAI : public ScriptedAI
    {
        npc_wall_of_sudsAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            events.ScheduleEvent(EVENT_INITIALIZE, 250);
            m_bIsInitialSummon = true;
        }

        void Initialize()
        {
            me->CastSpell(me, SPELL_WALL_OF_SUDS, false);
            m_bIsTopSideWall = me->GetOrientation() < 1.f;
            m_fConstOrientation = m_bIsTopSideWall ? 4.95f : 3.404f;
            
            m_uiMaxNumberOfPorts = m_bIsTopSideWall ? 15 : 25;
            m_uiNumberOfPorts = 0;

            if (m_bIsInitialSummon)
            {
                for (int i = 1; i < (m_bIsTopSideWall ? 2 : 3); ++i)
                {
                    float x, y;
                    GetPositionWithDistInOrientation(me, (30.f)*i, m_fConstOrientation, x, y);
                    if (Creature* pSud = me->SummonCreature(NPC_WALL_OF_SUDS, x, y, me->GetPositionZ(), m_bIsTopSideWall ? 0.26f : 1.82f))
                    {
                        if (pSud->AI())
                            pSud->AI()->DoAction(0);
                    }
                }
            }

            events.ScheduleEvent(EVENT_MOVE_NEXT, 300);

        }

        void DoAction(const int32 uiAction)
        {
            if (uiAction == 0)
                m_bIsInitialSummon = false;
        }

        // 0.26 4.95
        // 1.83 3.4
        bool m_bIsTopSideWall;
        bool m_bIsInitialSummon;
        float m_fConstOrientation;
        uint32 m_uiNumberOfPorts;
        uint32 m_uiMaxNumberOfPorts;

        void MoveNextOrientatedPosition()
        {
            if (m_uiNumberOfPorts > m_uiMaxNumberOfPorts)
            {
                me->DespawnOrUnsummon();
                return;
            }

            float newX, newY;
            GetPositionWithDistInOrientation(me, 4.f, m_fConstOrientation, newX, newY);

            me->NearTeleportTo(newX, newY, me->GetPositionZ(), m_fConstOrientation);

            ++m_uiNumberOfPorts;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_INITIALIZE:
                    Initialize();
                    break;
                case EVENT_MOVE_NEXT:
                    MoveNextOrientatedPosition();
                    events.ScheduleEvent(EVENT_MOVE_NEXT, 300);
                    break;
                }
            }
        }


    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_wall_of_sudsAI(pCreature);
    }
};

class CorrectUnitCheck
{
public:
    CorrectUnitCheck(Unit* caster) : _caster(caster) { }

    bool operator()(WorldObject* target)
    {
        return (target->GetDistance(_caster) > 15.f && target->GetDistance(_caster) < 38.f);
    }

private:
    Unit* _caster;
};

class spell_brew_finale : public SpellScriptLoader
{
public:
    spell_brew_finale() : SpellScriptLoader("spell_brew_finale") { }

    class spell_brew_finale_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_brew_finale_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void OnBeforeCast()
        {
            if (!GetCaster())
                return;

            std::list<Creature*>temps;
            GetCreatureListWithEntryInGrid(temps, GetCaster(), 45979, 50.f);

            temps.remove_if(CorrectUnitCheck(GetCaster()));
            std::list<Creature*>::iterator itr = temps.begin();

        }

        void Register()
        {
            BeforeCast += SpellCastFn(spell_brew_finale_SpellScript::OnBeforeCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_brew_finale_SpellScript();
    }
};

class NotCasterCheck
{
public:
    NotCasterCheck(Unit* caster) : _caster(caster) {}

    bool operator()(WorldObject* target)
    {
        return target->GetGUID() == _caster->GetGUID();
    }

private:
    Unit* _caster;
};

class spell_wall_of_suds : public SpellScriptLoader
{
public:
    spell_wall_of_suds() : SpellScriptLoader("spell_wall_of_suds") { }

    class spell_wall_of_suds_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_wall_of_suds_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(NotCasterCheck(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_wall_of_suds_SpellScript::SelectTargets, EFFECT_0, 130);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_wall_of_suds_SpellScript::SelectTargets, EFFECT_1, 130);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_wall_of_suds_SpellScript::SelectTargets, EFFECT_2, 130);
        }
    };
};

class spell_blackout_brew : public SpellScriptLoader
{
public:
    spell_blackout_brew() : SpellScriptLoader("spell_blackout_brew") { }

    enum eSpells : uint32
    {
        SPELL_BLACKOUT_BREW  = 106851,
        SPELL_BLACKOUT_DRUNK = 106857
    };

    class spell_blackout_brew_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_blackout_brew_AuraScript);

        bool m_bFirstTick;

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void HandleEffectPeriodic(const AuraEffect *  aurEff)
        {
            if (GetOwner() && GetOwner()->ToUnit())
            {
                                      // if jumping or moving, we should take a stack away
                if (GetOwner()->ToUnit()->isMoving() || GetOwner()->ToUnit()->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
                {
                    if (GetStackAmount() - 1 <= 0)
                        Remove(AURA_REMOVE_BY_EXPIRE);
                    else
                        SetStackAmount(GetStackAmount() - 1);
                }
                else
                {
                    if (GetStackAmount() + 1 >= 10)
                    {
                        GetOwner()->ToUnit()->AddAura(SPELL_BLACKOUT_DRUNK, GetOwner()->ToUnit());
                        Remove(AURA_REMOVE_BY_EXPIRE);
                    }
                    else
                        SetStackAmount(GetStackAmount() + 1);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_blackout_brew_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_blackout_brew_AuraScript();
    }
};

class spell_bloat_aura : public SpellScriptLoader
{
public:
    spell_bloat_aura() : SpellScriptLoader("spell_bloat_aura") {}

    enum eCreatures : uint32
    {
        NPC_BLOATED_STALKER      = 59482,
        SPELL_FORCE_VEHICLE      = 46598,
        SPELL_GUSHING_BREW       = 106549
    };

    class spell_bloat_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_bloat_aura_AuraScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void HandleOnApply(const AuraEffect *  /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetOwner() && GetOwner()->ToUnit())
            {
                Position pos;
                GetOwner()->GetPosition(&pos);

                for (int i = 0; i < 2; ++i)
                {
                    if (Creature* pStalker = GetOwner()->SummonCreature(NPC_BLOATED_STALKER, GetOwner()->GetPositionX(), GetOwner()->GetPositionY(), GetOwner()->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 30000))
                    {
                        pStalker->CastSpell(GetOwner()->ToUnit(), SPELL_FORCE_VEHICLE, true);
                        pStalker->CastSpell(pStalker, SPELL_GUSHING_BREW, true);
                    }
                }
            }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_bloat_aura_AuraScript::HandleOnApply, EFFECT_1, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_bloat_aura_AuraScript();
    }
};

class TargetHeightPredicate
{
public:
    TargetHeightPredicate(Unit* caster) : _caster(caster) {}

    bool operator()(WorldObject* target) const
    {
        return target && (target->GetPositionZ() > (_caster->GetPositionZ() + 4.9f));
    }

private:
    Unit* _caster;
};

class spell_carbonation : public SpellScriptLoader
{
public:
    spell_carbonation() : SpellScriptLoader("spell_carbonation") {}

    class spell_carbonation_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_carbonation_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            if (GetCaster())
                targets.remove_if(TargetHeightPredicate(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_carbonation_SpellScript::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_carbonation_SpellScript::SelectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_carbonation_SpellScript();
    }
};

void AddSC_boss_yanzhu()
{
    new npc_uncle_gao();
    new npc_chen_yanzhu();
    new boss_yanzhu();
    new npc_bubble_shield();
    new npc_yeasty_alemental();
    new npc_sudsy_alemental();
    new npc_fizzy_alemental();
    new npc_bloated_alemental();
    new npc_stout_alemental();
    new npc_bubbling_alemental();
    new npc_carbonate_trigger();
    new npc_suds_trigger();
    new npc_fizzy_bubble();
    new npc_wall_of_suds();
    new spell_blackout_brew();
    new spell_bloat_aura();
    new spell_carbonation();
    //new spell_brew_finale();
}