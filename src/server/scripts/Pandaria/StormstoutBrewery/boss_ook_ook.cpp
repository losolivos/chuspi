
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "stormstout_brewery.h"
#include "Vehicle.h"

enum eSpells
{
    SPELL_BARREL_EXPLOSION_HOSTILE = 106769,
    SPELL_BARREL_EXPLOSION_PLAYER = 107016,

    SPELL_FORCECAST_BARREL_DROP = 122385,
    SPELL_CANCEL_BARREL_AURA = 94465,
    SPELL_ROLLING_BARREL_COSMETIC = 106647,
    SPELL_BARREL_TOSS = 106847,

    SPELL_BARREL_PERIODIC_PLAYER = 115868,
    SPELL_BARREL_PERIODIC_HOSTILE = 106768,

    SPELL_BARREL_RIDE = 106614
};

static const Position pOokJumpPos = { -755.68f, 1351.83f, 146.92f, 1.82f };
static const Position pBarrelPos[] =
{
    { -733.33f, 1372.51f, 146.73f, 4.66f },
    { -777.73f, 1357.66f, 147.79f, 1.64f }
};

static const Position aDoorPos = { -766.863f, 1391.67f, 146.739f, 0.298219f };

// 4.98 6.28
class boss_ook_ook : public CreatureScript
{
    public:
        boss_ook_ook() : CreatureScript("boss_ook_ook") { }

        enum eSpells : uint32
        {
            SPELL_GOING_BANANAS     = 106651,
            SPELL_GROUND_POUND      = 106807,
            
        };

        enum eCreatures : uint32
        {
            NPC_HOZEN_HOLLERER          = 56783,
            NPC_ROLLING_BARREL          = 56682
        };

        enum eEvents : uint32
        {
            EVENT_NONE,
            EVENT_INTROCHECK,
            EVENT_GOING_BANANAS,
            EVENT_GROUND_POUND,
            EVENT_BARREL_TOSS
        };

        enum eTalks : uint32
        {
            TALK_INTRO,
            TALK_AGGRO,
            TALK_SPELL,
            EMOTE_GOING_BANANAS,
            TALK_DEATH
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_ook_ook_AI(creature);
        }

        struct boss_ook_ook_AI : public BossAI
        {
            boss_ook_ook_AI(Creature* creature) : BossAI(creature, DATA_OOK_OOK)
            {
                Initialize();
            }

            bool m_bIntroDone;
            bool m_bInitializedBarrels;
            uint64 m_uiDoorGuid;

            void Initialize()
            {
                me->SetPhaseMask(128, true);
                m_bIntroDone = false;
                m_bInitializedBarrels = false;
                events.ScheduleEvent(EVENT_INTROCHECK, 3000);

                HandleDoor(true);
            }

            bool DoSummonBarrels(int n)
            {
                if (n < 0)
                    return false;

                for (int i = 0; i < irand(2, 3); ++i)
                {
                    float m_fOrientation;

                    if (n > 0)
                        m_fOrientation = urand(0, 1) ? frand(4.98f, 6.28f) : frand(1.53f, 0.01f);
                    else
                        m_fOrientation = frand(2.45f, 4.63f);

                    if (Creature* pBarrel = me->SummonCreature(NPC_ROLLING_BARREL, pBarrelPos[n].GetPositionX(), pBarrelPos[n].GetPositionY(), pBarrelPos[n].GetPositionZ(), m_fOrientation))
                    {
                        pBarrel->AddAura(SPELL_BARREL_PERIODIC_PLAYER, pBarrel);
                        pBarrel->AddAura(SPELL_BARREL_PERIODIC_HOSTILE, pBarrel);
                        pBarrel->AddAura(SPELL_ROLLING_BARREL_COSMETIC, pBarrel);
                    }
                }

                return DoSummonBarrels(n - 1);
            }

            void StartIntro()
            {
                m_bIntroDone = true;
                events.CancelEvent(EVENT_INTROCHECK);

                me->SetPhaseMask(1, true);

                DoAction(1);

                Talk(TALK_INTRO);
                me->GetMotionMaster()->MoveJump(pOokJumpPos.GetPositionX(), pOokJumpPos.GetPositionY(), pOokJumpPos.GetPositionZ(), 25.f, 25.f);
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetHomePosition(pOokJumpPos);
            }

            void EnterCombat(Unit* /*pWho*/)
            {
                Talk(TALK_AGGRO);

                events.Reset();
                
                events.ScheduleEvent(EVENT_BARREL_TOSS, 1000);
                events.ScheduleEvent(EVENT_GOING_BANANAS, 2000);
                events.ScheduleEvent(EVENT_GROUND_POUND, (8000, 14000));

                HandleDoor(false);
            }

            void DoAction(const int32 iAction)
            {
                if (iAction == 0)
                    StartIntro();
                else if (iAction == 1)
                    events.ScheduleEvent(EVENT_BARREL_TOSS, 1000);
            }

            float GetNeededHealthPercent() 
            {
                if (me->GetAura(SPELL_GOING_BANANAS))
                {                   // 90, 60 and 30%
                    return 90 - ((me->GetAura(SPELL_GOING_BANANAS)->GetStackAmount()) * 30);
                }

                return 90;
            }

            void EnterEvadeMode()
            {
                events.Reset();
                
                me->CombatStop(true);
                me->AttackStop();
                me->RemoveAllAuras();
                me->SetLootRecipient(NULL);

                if (me->HasUnitState(UNIT_STATE_CANNOT_TURN))
                    me->ClearUnitState(UNIT_STATE_CANNOT_TURN);

                me->GetMotionMaster()->MovePoint(4, pOokJumpPos);

                HandleDoor(true);
            }

            void MovementInform(uint32 uiType, uint32 uiPointId)
            {
                if (uiType != POINT_MOTION_TYPE)
                    return;

                if (uiPointId == 4)
                    JustReachedHome();
            }

            void JustDied(Unit* /*pKiller*/)
            {
                if (me->GetInstanceScript())
                    me->GetInstanceScript()->SetData(DATA_OOK_OOK, DONE);

                _JustDied();

                HandleDoor(true);
            }

            uint32 GetBarrelTimer() const
            {
                if (me->GetAura(SPELL_GOING_BANANAS))
                {
                    switch (me->GetAura(SPELL_GOING_BANANAS)->GetStackAmount())
                    {
                    case 1:
                        return urand(8000, 12000);
                    case 2:
                        return urand(6000, 10000);
                    case 3:
                        return urand(4000, 7000);
                    }
                }

                return urand(10000, 14000);
            }

            void HandleDoor(bool open)
            {
                if (me->GetInstanceScript())
                {
                    if (open)
                    {
                        if (GameObject* pGo = ObjectAccessor::GetGameObject(*me, me->GetInstanceScript()->GetData64(GO_OOK_DOOR)))
                            pGo->AddObjectToRemoveList();
                    }
                    else
                    {
                        if (GameObject* pGo = me->SummonGameObject(GO_OOK_DOOR, aDoorPos.GetPositionX(), aDoorPos.GetPositionY(), aDoorPos.GetPositionZ(), aDoorPos.GetOrientation(), 0, 0, 0, 0, 14400))
                        {
                            pGo->SetGoState(GO_STATE_ACTIVE);
                            pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                        }

                    }
                }
            }

            void MoveInLineOfSight(Unit* pWho)
            {
                ScriptedAI::MoveInLineOfSight(pWho);
            }

            void UpdateAI(const uint32 uiDiff)
            {
                events.Update(uiDiff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_INTROCHECK:
                        events.ScheduleEvent(EVENT_INTROCHECK, 3000);
                        if (me->GetInstanceScript())
                        {
                            if (me->GetInstanceScript()->GetData(DATA_OOK_OOK) == SPECIAL)
                                StartIntro();
                        }
                        break;
                    case EVENT_GOING_BANANAS:
                        if (me->GetHealthPct() < GetNeededHealthPercent())
                        {
                            DoCast(SPELL_GOING_BANANAS);
                            Talk(TALK_SPELL);
                            Talk(EMOTE_GOING_BANANAS);
                        }
                        events.ScheduleEvent(EVENT_GOING_BANANAS, 2000);
                        break;
                    case EVENT_GROUND_POUND:
                        DoCast(me, SPELL_GROUND_POUND, false);
                        Talk(TALK_SPELL);
                        events.ScheduleEvent(EVENT_GROUND_POUND, urand(10000, 14000));
                        break;
                    case EVENT_BARREL_TOSS:
                        DoSummonBarrels(1);
                        events.ScheduleEvent(EVENT_BARREL_TOSS, GetBarrelTimer());
                        break;
                    }
                }

                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };
};

class npc_barrel : public CreatureScript
{
    public:
        npc_barrel() : CreatureScript("npc_barrel") { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_barrel_AI(creature);
        }

        enum eEvents : uint32
        {
            EVENT_NONE,
            EVENT_MOVE
        };

        struct npc_barrel_AI : public ScriptedAI
        {
            npc_barrel_AI(Creature* creature) : ScriptedAI(creature) 
            {
                if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK))
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                //me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }

            uint64 m_playerGuid;

            void Reset()
            {
                Initialize();
            }

            void Initialize()
            {
                me->AddAura(SPELL_ROLLING_BARREL_COSMETIC, me);
                me->AddAura(SPELL_BARREL_PERIODIC_HOSTILE, me);
                me->AddAura(SPELL_BARREL_PERIODIC_PLAYER, me);
                Move();
            }

            // tempp disabled
            void OnSpellClick(Unit* pClicker, bool& result)
            {   
                /*me->RemoveAurasDueToSpell(SPELL_BARREL_PERIODIC_PLAYER);
                me->RemoveAurasDueToSpell(SPELL_BARREL_PERIODIC_HOSTILE);
                pClicker->CastSpell(me, SPELL_BARREL_RIDE, true);

                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);*/
                return;
            }
    
            void Move()
            {
                float x = 0, y = 0;
                GetPositionWithDistInOrientation(me, 5.0f, me->GetOrientation(), x, y);

                me->GetMotionMaster()->MovePoint(100, x, y, me->GetPositionZ());

                events.ScheduleEvent(EVENT_MOVE, 300);
            }

            bool CheckIfAgainstUnit()
            {
                if (Unit* pBunny = me->SelectNearbyTarget(nullptr, 3.f))
                {                                       // General purpose bunny JMF
                    if (pBunny->ToCreature() && pBunny->ToCreature()->GetEntry() == 45979)
                        return true;
                }

                if (Unit* pBunny = GetClosestCreatureWithEntry(me, 45979, 10.f))
                {
                    if (me->GetDistance(pBunny) < 3.3f)
                        return true;
                }

                return false;
            }

            void DoExplode()
            {
                /*
                if (Unit* pPassenger = GetPassengerUnit())
                {
                    pPassenger->CastSpell(pPassenger, SPELL_FORCECAST_BARREL_DROP, true);
                    pPassenger->RemoveAurasDueToSpell(SPELL_BARREL_PERIODIC_HOSTILE);
                    pPassenger->RemoveAurasDueToSpell(SPELL_ROLLING_BARREL_COSMETIC);
                }*/

                DoCast(SPELL_BARREL_EXPLOSION_HOSTILE);
                DoCast(SPELL_BARREL_EXPLOSION_PLAYER);

                me->Kill(me);
            }

            void UpdateAI(const uint32 uiDiff)
            {               
                if (CheckIfAgainstUnit())
                    DoExplode();

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
};

class npc_hozen_hollerer : public CreatureScript
{
public:
    npc_hozen_hollerer() : CreatureScript("npc_hozen_hollerer") {}

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_BARREL_TOSS
    };

    enum eSpells : uint32
    {
        SPELL_BARREL_TOSS       = 106847
    };

    struct npc_hozen_hollererAI : public ScriptedAI
    {
        npc_hozen_hollererAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            events.ScheduleEvent(EVENT_BARREL_TOSS, 1000);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void DoCastBarrel()
        {
            float x, y, z;

            GetPositionWithDistInOrientation(me, frand(5.f, 10.f), me->GetOrientation(), x, y);
            z = me->GetMap()->GetHeight(x, y, me->GetPositionZ());

            me->CastSpell(x, y, z, SPELL_BARREL_TOSS, false);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BARREL_TOSS:
                    DoCastBarrel();
                    events.ScheduleEvent(EVENT_BARREL_TOSS, urand(4000, 6000));
                    break;
                }
            }
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_hozen_hollererAI(pCreature);
    }
};

class spell_ook_ook_barrel_ride : public SpellScriptLoader
{
    public:
        spell_ook_ook_barrel_ride() :  SpellScriptLoader("spell_ook_ook_barrel_ride") { }

        class spell_ook_ook_barrel_ride_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ook_ook_barrel_ride_AuraScript);

            void OnApply(const AuraEffect *  /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* barrelBase = GetTarget())
                {
                    if (GetCaster())
                    {
                        //barrelBase->SetCharmedBy(GetCaster(), CHARM_TYPE_VEHICLE);
                        GetCaster()->CastSpell(barrelBase, SPELL_CANCEL_BARREL_AURA, true);
                        GetCaster()->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVEMENT);
                        GetCaster()->CastSpell(GetCaster(), SPELL_ROLLING_BARREL_COSMETIC, true);
                    }
                }
            }

            // unused atm
            void OnRemove(const AuraEffect *  /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* pCaster = GetCaster();
                Unit* pTarget = GetTarget();

                if (pCaster && pTarget)
                {
                    pCaster->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVEMENT);
                    pCaster->RemoveAurasDueToSpell(SPELL_ROLLING_BARREL_COSMETIC);
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_ook_ook_barrel_ride_AuraScript::OnApply, EFFECT_0, SPELL_AURA_CONTROL_VEHICLE, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_ook_ook_barrel_ride_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_CONTROL_VEHICLE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ook_ook_barrel_ride_AuraScript();
        }
};

class OokOokPredicate
{
public:
    bool operator()(WorldObject* target) const
    {
        return target && (target->ToCreature() && target->ToCreature()->GetEntry() != NPC_OOK_OOK) || target->GetTypeId() == TYPEID_PLAYER;
    }
};

class HozuPredicate
{
public:
    bool operator()(WorldObject* target) const
    {
        return target && target->ToCreature() && target->ToCreature()->getFaction() != 190;
    }
};

class NotPlayerPredicate
{
public:
    bool operator()(WorldObject* target) const
    {
        return target && !target->ToPlayer();
    }
};

class spell_barrel_periodic : public SpellScriptLoader
{
public:
    spell_barrel_periodic() : SpellScriptLoader("spell_barrel_periodic") {}

    class spell_barrel_periodic_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_barrel_periodic_SpellScript);

        bool foundTargets;

        bool Load()
        {
            foundTargets = false;
            return true;
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            if (Unit* pOwner = GetCaster())
            {
                if (!targets.empty())
                    foundTargets = true;
                else 
                    switch (GetSpellInfo()->Id)
                {
                    case 107016:
                        if (Unit* pPotential = pOwner->SelectNearbyTarget(nullptr, 2.2f))
                        {
                            if (pPotential->GetTypeId() == TYPEID_PLAYER)
                                foundTargets = true;
                        }
                        else if (Unit* pPotential = pOwner->SelectNearbyAlly(nullptr, 3.f))
                        {
                            if (pPotential->GetTypeId() == TYPEID_PLAYER)
                                foundTargets = true;
                        }
                        break;
                    case 106769:
                        if (pOwner->SelectNearbyTarget(nullptr, 3.f))
                            foundTargets = true;
                        else if (pOwner->SelectNearbyAlly(nullptr, 3.3f))
                            foundTargets = true;
                        break;
                }

                if (!foundTargets)
                {
                    if (Creature* pTrigger = GetClosestCreatureWithEntry(pOwner, 45979, 3.f))
                        foundTargets = true;
                }
            }
        }

        void HandleEffectHit(SpellEffIndex effIdx)
        {
            if (!foundTargets)
                PreventHitEffect(effIdx);

        }

        void HandleAfterHit()
        {
            if (foundTargets)
            {
                if (Unit* pCaster = GetCaster())
                {
                    if (pCaster->IsOnVehicle())
                    {
                        Creature* pBarrel = pCaster->GetVehicleCreatureBase();
                        pCaster->RemoveAurasDueToSpell(SPELL_BARREL_RIDE);
                        //pCaster->ExitVehicle();
                        pCaster->GetMotionMaster()->MoveKnockbackFrom(pCaster->GetPositionX(), pCaster->GetPositionY(), 15.f, 15.f);
                        pCaster->RemoveAurasDueToSpell(SPELL_BARREL_PERIODIC_HOSTILE);

                        if (pBarrel)
                            pBarrel->Kill(pBarrel);
                    }
                    else
                    if (pCaster->GetTypeId() != TYPEID_PLAYER)
                        pCaster->Kill(pCaster);
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_barrel_periodic_SpellScript::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_barrel_periodic_SpellScript::SelectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHit += SpellEffectFn(spell_barrel_periodic_SpellScript::HandleEffectHit, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
            OnEffectHit += SpellEffectFn(spell_barrel_periodic_SpellScript::HandleEffectHit, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
            AfterHit += SpellHitFn(spell_barrel_periodic_SpellScript::HandleAfterHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_barrel_periodic_SpellScript();
    }
};

class spell_ground_pound : public SpellScriptLoader
{
public:
    spell_ground_pound() : SpellScriptLoader("spell_ground_pound") {}

    class spell_ground_pound_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ground_pound_AuraScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void HandleOnApply(const AuraEffect *  aurEff, AuraEffectHandleModes mode)
        {
            if (GetCaster())
            {
                GetCaster()->AddUnitState(UNIT_STATE_CANNOT_TURN);
            }
        }

        void HandleOnPeriodic(AuraEffect const *aurEff)
        {
            if (Unit* pOwner = GetOwner()->ToUnit())
            {
                PreventDefaultAction();
                pOwner->CastSpell(pOwner, 106808, true);
            }
        }

        void HandleOnRemove(const AuraEffect *  aurEff, AuraEffectHandleModes mode)
        {
            if (GetCaster())
            {
                GetCaster()->ClearUnitState(UNIT_STATE_CANNOT_TURN);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_ground_pound_AuraScript::HandleOnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_ground_pound_AuraScript::HandleOnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            //OnEffectPeriodic += AuraEffectPeriodicFn(spell_ground_pound_AuraScript::HandleOnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_ground_pound_AuraScript();
    }
};

void AddSC_boss_ook_ook()
{
    new boss_ook_ook();
    new npc_barrel();
    new npc_hozen_hollerer();
    new spell_ook_ook_barrel_ride();
    //new spell_ook_ook_barrel();
    //new spell_ook_ook_barrel2();
    new spell_barrel_periodic();
    //new spell_barrel_hostile();
    new spell_ground_pound();
}
