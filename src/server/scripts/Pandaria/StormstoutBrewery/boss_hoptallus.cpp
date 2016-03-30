/*
    Hoptallus
    By Emre
*/

#include "stormstout_brewery.h"

enum eSpells : uint32
{
    SPELL_HOPPER_ANIM_REPLACEMENT       = 114284,
    SPELL_HAMMER_COSMETIC_1             = 114530,
    SPELL_SUMMON_HAMMER                 = 114531,
    SPELL_SUMMON_HAMMER2                = 114532,
    SPELL_HAMMER_VISUAL                 = 114533,
    SPELL_BLINKING_COSMETIC_AURA        = 114287,
    SPELL_LIT_FUSE                      = 114290,

    SPELL_EXPLOSIVE_BREW                = 114291,

    SPELL_SMASH_AURA                    = 111662,
    SPELL_SMASH                         = 111666,

    SPELL_COSMETIC_EARTHQUAKE           = 114928,
    SPELL_HOPTALLUS_INTRO_SOUND         = 106790,
    SPELL_HOPTALLUS_KEG_SCENE           = 114366,
    SPELL_HOPTALLUS_SCREECH             = 114367,

    SPELL_FURLWIND                      = 112992,
    SPELL_CARROT_BREATH                 = 112944,
    SPELL_SUMMON_BREATH_STALKER         = 112956,

    SPELL_CARROT_DOOR                   = 116005
};

enum eCreatures : uint32
{
    NPC_CARROT_BREATH_STALKER           = 59018
};

static const Position aHoptallusHopPos = { -698.62f, 1259.4f, 162.79f, 0.24f };
static const Position apHoplingSpawns[] = 
{
    { -719.31f, 1248.27f, 166.8f, 0.24f },
    { -719.95f, 1250.89f, 166.8f, 0.24f },
    { -720.66f, 1253.79f, 166.8f, 0.24f },
    { -721.25f, 1256.18f, 166.8f, 0.24f },
    { -721.94f, 1259.02f, 166.8f, 0.24f }
};

static const Position apHopperSpawns[] =
{
    { -726.7f, 1257.9f, 166.8f, 0.24f },
    { -720.86f, 1253.44f, 166.8f, 0.24f },
    { -726.86f, 1247.41f, 166.8f, 0.25f },
    { -719.38f, 1255.37f, 166.8f, 0.25f}
};

static const Position aNibblerWps[] =
{
    { -704.96f, 1292.42f, 162.7f, 4.95f },
    { -698.4f, 1267.73f, 162.7f, 4.97f },
    { -690.44f, 1254.47f, 162.8f, 5.32f },
    { -683.54f, 1252.15f, 162.82f, 6.f }, // pick up here
    { -663.1f, 1256.7f, 154.8f, 5.86f },
    { -653.1f, 1242.9f, 154.8f, 4.91f },
    { -639.69f, 1192.53f, 139.15f, 0.28f }, // jump
    { -601.1f, 1201.59f, 138.5f, 4.85f },
    { -598.2f, 1188.f, 138.65f, 4.7f },
    { -598.4f, 1151.8f, 138.3f, 4.86f },
    { -592.1f, 1127.8f, 138.5f, 4.98f },
    { -566.1f, 1102.2f, 142.1f, 5.44f },
};

static const uint32 auiHoplings[5] =
{
    56631,
    59458,
    59459,
    59460,
    59461
};

static const uint32 auiHoppers[2] =
{
    56718,
    59426
};

class npc_nibbler : public CreatureScript
{
public:
    npc_nibbler() : CreatureScript("npc_nibbler") {}

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_CHECK,
        EVENT_MOVE,
        EVENT_JUMP,
        EVENT_TALKPANDA
    };

    enum eSpells : uint32
    {
        SPELL_RIDE_VEHICLE      = 93970
    };

    struct npc_nibblerAI : public ScriptedAI
    {
        npc_nibblerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            events.ScheduleEvent(EVENT_CHECK, 1000);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetPhaseMask(128, true);
        }

        uint32 m_uiWayPoint;
        uint32 m_uiPandaPoint;

        std::vector<uint64> m_vPandaGuids;
        std::vector<uint64>::const_iterator pandaItr;

        void InitRun()
        {
            me->SetPhaseMask(1, true);
            events.ScheduleEvent(EVENT_MOVE, 100);
            events.CancelEvent(EVENT_CHECK);
            m_uiWayPoint = 0;
        }

        void Move()
        {
            me->GetMotionMaster()->MovePoint(m_uiWayPoint, aNibblerWps[m_uiWayPoint]);
        }

        void PrepPandas()
        {
            m_uiPandaPoint = 0;

            std::list<Creature*> temp;
            GetCreatureListWithEntryInGrid(temp, me, 59075, 20.f);

            for (auto const itr : temp)
                m_vPandaGuids.push_back(itr->GetGUID());

            events.ScheduleEvent(EVENT_TALKPANDA, 8000);
            pandaItr = m_vPandaGuids.begin();
        }

        void Cleanup()
        {
            Creature* pBunny = GetClosestCreatureWithEntry(me, 45979, 10.f);
            if (pBunny)
                pBunny->RemoveAurasDueToSpell(SPELL_CARROT_DOOR);
            me->SetVisible(false);
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            switch (uiPointId)
            {
            case 0:
            case 1:
            case 2:
            case 4:
            case 5:
            case 7:
            case 8:
            case 9:
            case 10:
                events.ScheduleEvent(EVENT_MOVE, 100);
                break;
            case 6:
                events.ScheduleEvent(EVENT_JUMP, 100);
                PrepPandas();
                break;
            case 11:
                Cleanup();
                break;
            case 3:
                Creature* pBunny = GetClosestCreatureWithEntry(me, 45979, 10.f);

                if (pBunny)
                    pBunny->CastSpell(me, SPELL_RIDE_VEHICLE, true);

                if (GameObject* pGo = GetClosestGameObjectWithEntry(me, GO_INVIS_DOOR, 10.f))
                    pGo->AddObjectToRemoveList();

                events.ScheduleEvent(EVENT_MOVE, 100);
                break;
            }

            ++m_uiWayPoint;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);
            
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK:
                    if (me->GetInstanceScript())
                    {
                        events.ScheduleEvent(EVENT_CHECK, 2000);

                        if (me->GetInstanceScript()->GetData(DATA_HOPTALLUS) == DONE)
                            InitRun();
                    }
                    break;
                case EVENT_MOVE:
                    Move();
                    break;
                case EVENT_JUMP:
                    me->GetMotionMaster()->MoveJump(aNibblerWps[m_uiWayPoint].GetPositionX(), aNibblerWps[m_uiWayPoint].GetPositionY(), aNibblerWps[m_uiWayPoint].GetPositionZ(), 15.f, 15.f, m_uiWayPoint);
                    break;
                case EVENT_TALKPANDA:
                    if (pandaItr == m_vPandaGuids.end())
                        break;
                    else
                    {
                        events.ScheduleEvent(EVENT_TALKPANDA, 9500);

                        if (Creature* pPanda = ObjectAccessor::GetCreature(*me, *pandaItr))
                        {
                            if (pPanda->AI())
                                pPanda->AI()->Talk(m_uiPandaPoint);

                            ++m_uiPandaPoint;
                        }
                        ++pandaItr;
                    }
                    break;

                }
            }
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_nibblerAI(pCreature);
    }
};

class boss_hoptallus : public CreatureScript
{
public:
    boss_hoptallus() : CreatureScript("boss_hoptallus") { }

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_FURLWIND,
        EVENT_CARROT_BREATH,
        EVENT_CARROT_STALKER,
        EVENT_SCREECH,
        EVENT_ENTERCOMBAT,
        EVENT_SUMMON_HOPLINGS,
        EVENT_SUMMON_HOPPERS
    };

    enum eTalks : uint32
    {
        TALK_AGGRO,
        TALK_FURLWIND,
        TALK_CARROT_BREATH,
        TALK_DEATH,
        TALK_SCREECH
    };

    struct boss_hoptallusAI : public ScriptedAI
    {
        boss_hoptallusAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            InitDoor();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            m_bIntroStarted = false;
        }

        bool m_bIntroStarted;
        uint32 m_uiHoplingWaves;
        uint32 m_uiHopperWaves;
        uint64 m_uiDoor;

        void InitDoor()
        {
            Creature* pBunny = GetClosestCreatureWithEntry(me, 45979, 45.f);

            if (pBunny)
                pBunny->AddAura(SPELL_CARROT_DOOR, pBunny);
        }

        void InitJump()
        {
            me->SetReactState(REACT_PASSIVE);

            m_bIntroStarted = true;
            m_uiDoor = 0;

            DoCast(SPELL_HOPTALLUS_KEG_SCENE);
            DoCast(SPELL_HOPTALLUS_INTRO_SOUND);
            DoCast(SPELL_COSMETIC_EARTHQUAKE);

            me->GetMotionMaster()->MoveJump(aHoptallusHopPos.m_positionX, aHoptallusHopPos.m_positionY, aHoptallusHopPos.m_positionZ, 10.f, 10.f, 1);
            me->SetHomePosition(aHoptallusHopPos);

            events.ScheduleEvent(EVENT_ENTERCOMBAT, 8000);
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiPointId == 1)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                m_uiDoor = GetDoor();
            }
        }

        void MoveInLineOfSight(Unit* pWho)
        {
            if (!m_bIntroStarted && pWho)
            {                                                                   // Small hack to make sure boss isn't triggered from behind
                if (pWho->GetTypeId() == TYPEID_PLAYER && pWho->GetDistance(me) < 30.f && pWho->GetPositionX() > -705.f)
                    InitJump();
            }

            ScriptedAI::MoveInLineOfSight(pWho);
        }

        uint64 GetDoor() const
        {
            GameObject* pDoor = GetClosestGameObjectWithEntry(me, GO_SLIDING_DOOR, 25.f);

            return pDoor ? pDoor->GetGUID() : 0;
        }

        GameObject* GetDoorGo()
        {
            GameObject* pDoor = ObjectAccessor::GetGameObject(*me, m_uiDoor);

            return pDoor ? pDoor : nullptr;
        }

        bool DoSummonHoplings(int n)
        {
            if (n < 0)
            {
                ++m_uiHoplingWaves;

                if (m_uiHoplingWaves < 3)
                    events.ScheduleEvent(EVENT_SUMMON_HOPLINGS, 4000);
                else
                    m_uiHoplingWaves = 0;

                return false;
            }

            if (Creature* pCreature = me->SummonCreature(auiHoplings[n], apHoplingSpawns[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, urand(2000, 5000)))
            {
                pCreature->AddAura(SPELL_HOPPER_ANIM_REPLACEMENT, pCreature);

                Position pos;
                me->GetRandomNearPosition(pos, 10.f);

                pCreature->GetMotionMaster()->MoveJump(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), 15.f, 15.f);
                if (pCreature->AI())
                    pCreature->AI()->DoZoneInCombat();
            }

            return DoSummonHoplings(n - 1);
        }

        bool DoSummonHoppers(int n)
        {
            if (n < 0)
            {
                ++m_uiHopperWaves;

                if (m_uiHopperWaves < 2)
                    events.ScheduleEvent(EVENT_SUMMON_HOPPERS, 6000);
                else
                    m_uiHopperWaves = 0;

                return false;
            }

            if (Creature* pCreature = me->SummonCreature(auiHoppers[urand(0, 1)], apHopperSpawns[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, urand(4000, 8000)))
            {
                Position pos;
                me->GetRandomNearPosition(pos, 10.f);

                pCreature->GetMotionMaster()->MoveJump(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), 10.f, 10.f);
                if (pCreature->AI())
                    pCreature->AI()->DoZoneInCombat();
            }

            return DoSummonHoppers(n - 1);
        }

        void EnterEvadeMode()
        {
            summons.DespawnAll();
            events.Reset();

            me->ClearUnitState(UNIT_STATE_CANNOT_TURN);

            if (me->GetInstanceScript() && GetDoorGo())
            {
                GetDoorGo()->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                me->GetInstanceScript()->HandleGameObject(0, true, GetDoorGo());
            }

            me->CombatStop(true);
            me->AttackStop();
            me->GetMotionMaster()->MovePoint(194, aHoptallusHopPos);
        }

        void SpellHit(Unit* pCaster, const SpellInfo* pSpell)
        {
            if (pSpell->Id == 112922)
            {
                if (me->GetVictim())
                    me->GetMotionMaster()->MoveChase(me->GetVictim());
            }
        }

        void JustDied(Unit* pKiller)
        {
            Talk(TALK_DEATH);

            if (me->GetInstanceScript())
                me->GetInstanceScript()->SetData(DATA_HOPTALLUS, DONE);

            summons.DespawnAll();

            if (me->GetInstanceScript() && GetDoorGo())
            {
                GetDoorGo()->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                me->GetInstanceScript()->HandleGameObject(0, true, GetDoorGo());
            }
        }

        void EnterCombat(Unit* pWho)
        {
            Talk(TALK_AGGRO);

            events.ScheduleEvent(EVENT_SCREECH, 30000);
            events.ScheduleEvent(EVENT_FURLWIND, urand(10000, 16000));
            events.ScheduleEvent(EVENT_CARROT_BREATH, urand(15000, 30000));

            m_uiHoplingWaves = 0;
            m_uiHopperWaves = 0;

            if (me->GetInstanceScript() && GetDoorGo())
            {
                GetDoorGo()->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                me->GetInstanceScript()->HandleGameObject(0, false, GetDoorGo());
            }
        }

        void JustSummoned(Creature* pSummoned)
        {
            if (pSummoned)
                summons.Summon(pSummoned);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (me->HasAura(SPELL_FURLWIND) || me->HasAura(SPELL_CARROT_BREATH) || me->HasUnitState(UNIT_STATE_CASTING))
                return;

                events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ENTERCOMBAT:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat();
                    break;
                case EVENT_FURLWIND:
                    DoCast(me, SPELL_FURLWIND, true);
                    Talk(TALK_FURLWIND);
                    events.ScheduleEvent(EVENT_FURLWIND, urand(40000, 50000));
                    break;
                case EVENT_CARROT_BREATH:
                    DoCast(SPELL_SUMMON_BREATH_STALKER);
                    DoCast(SPELL_CARROT_BREATH);
                    Talk(TALK_CARROT_BREATH);
                    events.ScheduleEvent(EVENT_CARROT_BREATH, urand(25000, 27000));
                    break;
                case EVENT_SCREECH:
                    events.ScheduleEvent(EVENT_SCREECH, 30000);
                    events.ScheduleEvent(EVENT_SUMMON_HOPLINGS, 2000);
                    events.ScheduleEvent(EVENT_SUMMON_HOPPERS, 4000);
                    DoCast(SPELL_HOPTALLUS_SCREECH);
                    Talk(TALK_SCREECH);
                    break;
                case EVENT_SUMMON_HOPLINGS:
                    DoSummonHoplings(4);
                    break;
                case EVENT_SUMMON_HOPPERS:
                    DoSummonHoppers(3);
                    break;
                }
            }

            if (!UpdateVictim())
                return;

                DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_hoptallusAI(pCreature);
    }
};

class npc_hammer_bopper : public CreatureScript
{
public:
    npc_hammer_bopper() : CreatureScript("npc_hammer_bopper") {}

    struct npc_hammer_bopperAI : public ScriptedAI
    {
        npc_hammer_bopperAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            Initialize();
        }

        void Initialize()
        {
            me->SetWalk(true);
            me->AddAura(SPELL_HAMMER_COSMETIC_1, me);
            me->AddAura(SPELL_HOPPER_ANIM_REPLACEMENT, me);
        }
        
        void JustDied(Unit* pKiller)
        {
            DoCast(me, SPELL_SUMMON_HAMMER, true);
            me->RemoveAurasDueToSpell(SPELL_HAMMER_COSMETIC_1);
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_hammer_bopperAI(pCreature);
    }
};

class npc_explosive_hopper : public CreatureScript
{
public:
    npc_explosive_hopper() : CreatureScript("npc_explosive_hopper") {}

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_EXPLODE
    };

    struct npc_explosive_hopperAI : public ScriptedAI
    {
        npc_explosive_hopperAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            Initialize();
        }

        void Initialize()
        {
            me->SetWalk(true);
            me->AddAura(SPELL_HOPPER_ANIM_REPLACEMENT, me);
        }

        void EnterCombat(Unit* pWho)
        {
            events.ScheduleEvent(EVENT_EXPLODE, urand(3000, 5000));
        }

        void Reset()
        {
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            events.Update(uiDiff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_EXPLODE:
                    DoCast(me, SPELL_LIT_FUSE, true);
                    DoCast(me, SPELL_BLINKING_COSMETIC_AURA, true);
                    DoCast(SPELL_EXPLOSIVE_BREW);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_explosive_hopperAI(pCreature);
    }
};

class npc_hammer : public CreatureScript
{
public:
    npc_hammer() : CreatureScript("npc_hammer") {}

    struct npc_hammerAI : public ScriptedAI
    {
        npc_hammerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->AddAura(SPELL_HAMMER_VISUAL, me);
        }

        void OnSpellClick(Unit* pClicker, bool& result)
        {
            pClicker->AddAura(SPELL_SMASH_AURA, pClicker);

            me->AddObjectToRemoveList();
            return;
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_hammerAI(pCreature);
    }
};

class npc_carrot_breath_stalker : public CreatureScript
{
public:
    npc_carrot_breath_stalker() : CreatureScript("npc_carrot_breath_stalker") {}

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_MOVE
    };

    struct npc_carrot_breath_stalkerAI : public ScriptedAI
    {
        npc_carrot_breath_stalkerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            Initialize();
        }

        float x, y;
        float m_fArcPoint;
        bool m_bCounterClockwise;

        void Initialize()
        {
            me->GetMotionMaster()->Clear();

            events.ScheduleEvent(EVENT_MOVE, 1500);
            x = me->GetPositionX();
            y = me->GetPositionY();
            m_fArcPoint = 0.0f;

            m_bCounterClockwise = urand(0, 1);

            me->SetCanFly(true);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            if (Creature* pHoppy = ObjectAccessor::GetCreature(*me, me->GetInstanceScript()->GetData64(NPC_HOPTALLUS)))
            {
                pHoppy->SetReactState(REACT_PASSIVE);
                pHoppy->SetFacingToObject(me);
                pHoppy->SetOrientation(pHoppy->GetAngle(me));
            }
        }

        void Move()
        {
            float newX = x + 10.0f * cos(m_fArcPoint * M_PI / 16);
            float newY = m_bCounterClockwise ? y - 10.0f * sin(m_fArcPoint * M_PI / 16) : y + 10.0f * sin(m_fArcPoint * M_PI / 16);
            me->GetMotionMaster()->MovePoint(0, newX, newY, me->GetPositionZ());

            ++m_fArcPoint;

            if (Creature* pHoppy = ObjectAccessor::GetCreature(*me, me->GetInstanceScript()->GetData64(NPC_HOPTALLUS)))
            {
                pHoppy->SetOrientation(pHoppy->GetAngle(me));
                pHoppy->SetFacingToObject(me);
            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            if (uiPointId == 0)
            {
                events.ScheduleEvent(EVENT_MOVE, 100);
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MOVE:
                    Move();
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_carrot_breath_stalkerAI(pCreature);
    }
};

class CheckTargetPredicate
{
public:
    bool operator()(WorldObject* target) const
    {
        return target && target->ToUnit() && !target->ToUnit()->HasAura(SPELL_HOPPER_ANIM_REPLACEMENT);
    }
};

class spell_hammer_smash : public SpellScriptLoader
{
public:
    spell_hammer_smash() : SpellScriptLoader("spell_hammer_smash") {}

    class spell_hammer_smash_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hammer_smash_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_SMASH))
                return false;
            return true;
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(CheckTargetPredicate());
        }

        void HandleSpellEffectHit(SpellEffIndex idx)
        {
            if (Unit* pUnit = GetHitUnit())
            {
                pUnit->GetMotionMaster()->MoveKnockbackFrom(pUnit->GetPositionX(), pUnit->GetPositionY(), frand(3.f, 10.f), frand(1.f, 6.f));

                if (pUnit->IsAlive())
                    pUnit->Kill(pUnit);
            }
        }

        void HandleScriptEffectHit(SpellEffIndex idx)
        {
            if (Unit* pCaster = GetCaster())
            {
                pCaster->RemoveAuraFromStack(GetSpellInfo()->Effects[idx].BasePoints);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_hammer_smash_SpellScript::HandleSpellEffectHit, EFFECT_0, SPELL_EFFECT_KNOCK_BACK);
            OnEffectHitTarget += SpellEffectFn(spell_hammer_smash_SpellScript::HandleSpellEffectHit, EFFECT_1, SPELL_EFFECT_KNOCK_BACK);
            OnEffectHit       += SpellEffectFn(spell_hammer_smash_SpellScript::HandleScriptEffectHit, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);

            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hammer_smash_SpellScript::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hammer_smash_SpellScript::SelectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hammer_smash_SpellScript();
    }
};

class notInFrontPredicate
{
public:
    notInFrontPredicate(Unit* pCaster) : _caster(pCaster){}

    bool operator()(WorldObject* target)
    {
        return target && !_caster->HasInArc((_caster->GetOrientation()*M_PI / 4), target);
    }
private:
    Unit* _caster;
};

class spell_carrot_breath : public SpellScriptLoader
{
public:
    spell_carrot_breath() : SpellScriptLoader("spell_carrot_breath") {}

    class spell_carrot_breath_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_carrot_breath_AuraScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }
        
        uint32 m_uiUpdateTimer;

        bool Load(Aura * aura)
        {
            m_uiUpdateTimer = 200;
        }

        void HandleOnRemove(const AuraEffect *  aurEff, AuraEffectHandleModes mode)
        {
            if (Creature* pCreature = GetOwner()->ToCreature())
            {
                pCreature->ClearUnitState(UNIT_STATE_CANNOT_TURN);
                pCreature->SetReactState(REACT_AGGRESSIVE);

                Creature* pStalker = GetClosestCreatureWithEntry(pCreature, NPC_CARROT_BREATH_STALKER, 40.f);

                if (pStalker)
                    pStalker->AddObjectToRemoveList();
            }
        }

        void HandleOnApply(const AuraEffect *  aurEff, AuraEffectHandleModes mode)
        {
            if (Creature* pCreature = GetOwner()->ToCreature())
            {
                pCreature->AddUnitState(UNIT_STATE_CANNOT_TURN);
                pCreature->SetReactState(REACT_PASSIVE);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_carrot_breath_AuraScript::HandleOnRemove, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(spell_carrot_breath_AuraScript::HandleOnApply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_carrot_breath_AuraScript();
    }
};

class spell_carrot_breath_targeting : public SpellScriptLoader
{
public:
    spell_carrot_breath_targeting() : SpellScriptLoader("spell_carrot_breath_targeting") {}

    class spell_carrot_breath_targeting_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_carrot_breath_targeting_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void OnSelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(notInFrontPredicate(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_carrot_breath_targeting_SpellScript::OnSelectTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_carrot_breath_targeting_SpellScript::OnSelectTargets, EFFECT_1, TARGET_UNIT_CONE_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_carrot_breath_targeting_SpellScript::OnSelectTargets, EFFECT_2, TARGET_UNIT_CONE_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_carrot_breath_targeting_SpellScript();
    }
};

void AddSC_boss_hoptallus()
{
    new boss_hoptallus();
    new npc_explosive_hopper();
    new npc_hammer_bopper();
    new npc_hammer();
    new npc_carrot_breath_stalker();
    new npc_nibbler();
    new spell_hammer_smash();
    new spell_carrot_breath();
    new spell_carrot_breath_targeting();
}