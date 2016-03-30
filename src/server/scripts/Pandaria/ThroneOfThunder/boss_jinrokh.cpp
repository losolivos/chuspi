#include "throne_of_thunder.h"
#include "ObjectVisitors.hpp"
#include "GridNotifiers.h"

enum eSpells : uint32
{
    SPELL_CONDUCTIVE_WATER_SUMMON           = 137145,
    SPELL_CONDUCTIVE_WATERFALL              = 137340,
    SPELL_CONDUCTIVE_WATER_VISUAL           = 137277,
    SPELL_CONDUCTIVE_WATER_DUMMY            = 137168,
    SPELL_CONDUCTIVE_WATER_GROW             = 137694,
    SPELL_ELECTRIFY_WATERS                  = 138568,
    SPELL_ELECTRIFIED_WATER_VISUAL          = 137978,

    SPELL_LIGHTNING_FISSURE_SUMMON          = 137479,
    SPELL_LIGHTNING_FISSURE_VISUAL          = 137480,
    SPELL_LIGHTNING_FISSURE_CONDUCTION      = 138133,
    SPELL_LIGHTNING_FISSURE_AURA            = 137484,

    SPELL_FOCUSED_LIGHTNING                 = 137399,
//  SPELL_FOCUSED_LIGHTNING_VISUAL          = 137425,
//  SPELL_FOCUSED_LIGHTNING_FIXATE          = 137422,
    SPELL_FOCUSED_LIGHTNING_SPEED           = 137389,
    SPELL_FOCUSED_LIGHTNING_AOE             = 137429,
    SPELL_FOCUSED_LIGHTNING_DETONATION      = 137374,
    SPELL_VIOLENT_LIGHTNING_DETONATION      = 138990,
    SPELL_FOCUSED_LIGHTNING_DAMAGE          = 137423,
    SPELL_FOCUSED_LIGHTNING_TARGET          = 137194,
    SPELL_FOCUSED_LIGHTNING_CONDUCTION      = 137530,

    SPELL_IMPLOSION                         = 137507,

    SPELL_CONDUCTIVE_WATER_GROW_AURA        = 137694,
    SPELL_CONDUCTIVE_WATERS                 = 138470,
    SPELL_ELECTRIFIED_WATERS                = 138006,
    SPELL_FLUIDITY                          = 138002,

    SPELL_STATIC_BURST                      = 137162,
    SPELL_STATIC_WOUND                      = 138349,
    SPELL_STATIC_WOUND_DAMAGE               = 138389,

    SPELL_LIGHTNING_STORM                   = 137313,
    SPELL_LIGHTNING_STORM_VISUAL            = 138568,

        // Thundering Throw
    SPELL_THUNDERING_THROW                  = 137180, // Need SpellScript to handle ScriptEffect
    SPELL_THUNDERING_THROW_JUMP             = 137173, // Casted by player on a statue
    SPELL_THUNDERING_THROW_SILENCE          = 137161, // Silence, visuals
    SPELL_THUNDERING_THROW_FLY_VISUAL       = 140594, // Visual in flight
    SPELL_THUNDERING_THROW_HIT_DAMAGE       = 137370, // Damage on hit statue
    SPELL_THUNDERING_THROW_HIT_AOE_DAMAGE   = 137167, // AOE Damage on hit statue
    SPELL_THUNDERING_THROW_STUN             = 137371, // Stun after aoe damage on hit statue
    SPELL_THUNDERING_THROW_IMPACT_VISUAL    = 140606, // Visual of the impact on ground
};

enum eCreatures : uint32
{
    NPC_LIGHTNING_FISSURE               = 69609,
    NPC_CONDUCTIVE_WATER                = 69469,
    NPC_FOCUSED_LIGHTNING               = 69593
};

enum iActions : int32
{
    ACTION_NONE,
    ACTION_DESTROYED,
    ACTION_RESET,
    ACTION_ELECTRIFY
};

static const float floorZ = 124.03f;

enum eJDatas : uint32
{
    DATA_STATUE_DESTROYED,
};

class notInLosPredicate
{
public:
    notInLosPredicate(Unit* caster) : _caster(caster) {}
    bool operator()(WorldObject* target) const
    {
        return target && !target->IsWithinLOSInMap(_caster);
    }

private:
    Unit* _caster;
};

class validStatuePredicate
{
public:
    bool operator() (WorldObject* target) const
    {
        return target && target->ToPlayer() || ((target->ToCreature()->AI() && target->ToCreature()->AI()->GetData(DATA_STATUE_DESTROYED) == 1) || target->GetEntry() != NPC_JINROKH_STATUE);
    }
};

class electrifiedPredicate
{
public:
    bool operator()(Creature* target) const
    {
        return target && target->HasAura(SPELL_ELECTRIFIED_WATER_VISUAL);
    }
};

class scaleCheckPredicate
{
public:
    scaleCheckPredicate(Unit* caster) : _caster(caster) {}

    bool operator()(WorldObject* target) const
    {
        if (target && target->GetExactDist2d(_caster) > GetSizeProp(_caster))
            return true;
        return false;
    }
private:
    Unit* _caster;

    float GetSizeProp(Unit* propagator) const
    {
        if (Aura* pAura = propagator->GetAura(SPELL_CONDUCTIVE_WATER_GROW))
        {
            return ((float)0.5f * pAura->GetStackAmount()) + propagator->GetFloatValue(UNIT_FIELD_BOUNDING_RADIUS);
        }

        return 0;
    }
};

class conductionPredicate
{
public:
    conductionPredicate(Creature* _waters) : waters(_waters) {}

    bool operator()(WorldObject* target) const
    {
        if (target && target->ToUnit())
        {
            if (target->ToUnit()->HasAura(SPELL_FLUIDITY, waters->GetGUID()))
                return false;
            if (target->ToUnit()->HasAura(SPELL_ELECTRIFIED_WATERS, waters->GetGUID()))
                return false;
        }

        return true;
    }
private:
    Creature* waters;
};

class notPlayerOrPetPredicate
{
public:
    bool operator()(WorldObject*target) const
    {
        return target && target->GetTypeId() != TYPEID_PLAYER;
    }
};

class focusedLightningPredicate
{
public:
    bool operator()(WorldObject* target) const
    {
        if (target)
        {
            if (target->ToCreature())
            {
                if (target->ToCreature()->GetEntry() != NPC_LIGHTNING_FISSURE)
                    return true;
                else
                    return false;
            }

            if (target->GetTypeId() == TYPEID_PLAYER)
                return false;

            return true;
        }
        return true;
    }
};

static const Position aWaterPos[4] = 
{
    { 5864.490f, 6290.628f, 124.03f, 5.51f },
    { 5917.633f, 6289.476f, 124.03f, 3.93f },
    { 5918.487f, 6236.848f, 124.03f, 2.36f },
    { 5865.241f, 6236.743f, 128.03f, 0.77f }
};

static const Position aCenterPos = { 5892.16f, 6263.58f, 124.7f, 0.0f };

class FocusedLightningSelection : public std::unary_function<Unit*, bool>
{
public:
    FocusedLightningSelection() { }

    bool operator()(Unit const* pTarget) const
    {
        if (!pTarget->ToPlayer())
            return false;

        Player const* pPlayer = pTarget->ToPlayer();

        switch (pPlayer->getClass())
        {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
            return false;

        case CLASS_PALADIN:
            return pPlayer->GetSpecializationId(pPlayer->GetActiveSpec()) == SPEC_PALADIN_HOLY;

        case CLASS_DRUID:
            return pPlayer->GetSpecializationId(pPlayer->GetActiveSpec()) == SPEC_DRUID_BALANCE || pPlayer->GetSpecializationId(pPlayer->GetActiveSpec()) == SPEC_DRUID_RESTORATION;

        case CLASS_MONK:
            return pPlayer->GetSpecializationId(pPlayer->GetActiveSpec()) == SPEC_MONK_MISTWEAVER;

        case CLASS_SHAMAN:
            return pPlayer->GetSpecializationId(pPlayer->GetActiveSpec()) != SPEC_SHAMAN_ENHANCEMENT;

        case CLASS_MAGE:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
        case CLASS_HUNTER:
            return true;

        default:
            return false;
        }
    }
};

class boss_jinrokh : public CreatureScript
{
public:
    boss_jinrokh() : CreatureScript("boss_jinrokh") {}

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_STATIC_BURST,
        EVENT_FOCUSED_LIGHTNING,
        EVENT_THUNDERING_THROW,
        EVENT_LIGHTNING_STORM,
        EVENT_IONIZATION,
        EVENT_BERSERK,
        EVENT_HEIGHT_CHECK,
        EVENT_PROPAGATE_STORM,
        EVENT_INTRO_YELL
    };

    enum eTalks : uint32
    {
        TALK_INTRO,
        TALK_AGGRO,
        TALK_STATIC_BURST,
        TALK_THUNDERING_THROW,
        TALK_LIGHTNING_STORM,
        TALK_FOCUSED_LIGHTNING,
        EMOTE_THUNDERING_THROW,
        EMOTE_LIGHTNING_STORM,
        EMOTE_LIGHTNING_STORM_2,
        TALK_SLAY,
        TALK_BERSERK,
        TALK_DEATH
    };

    struct boss_jinrokhAI : public BossAI
    {
        boss_jinrokhAI(Creature* pCreature) : BossAI(pCreature, DATA_JINROKH)
        {
        }

        uint32 m_uiPushTimer;
        EventMap m_mEvents;

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
            ResetStatues();

            instance->SetBossState(DATA_JINROKH, NOT_STARTED);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void EnterCombat(Unit* pWho)
        {
            events.ScheduleEvent(EVENT_STATIC_BURST, urand(18000, 24000));
            events.ScheduleEvent(EVENT_FOCUSED_LIGHTNING, 10000);
            events.ScheduleEvent(EVENT_THUNDERING_THROW, 30000);
            events.ScheduleEvent(EVENT_LIGHTNING_STORM, 90000); // 1,5 minutes
            events.ScheduleEvent(EVENT_BERSERK, 6 * MINUTE*IN_MILLISECONDS + 5000);
            events.ScheduleEvent(EVENT_HEIGHT_CHECK, 2000);

            instance->SetBossState(DATA_JINROKH, IN_PROGRESS);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            Talk(TALK_AGGRO);
        }

        void JustSummoned(Creature* pSummoned)
        {
            summons.Summon(pSummoned);
        }

        void SummonedCreatureDespawn(Creature* pSummoned)
        {
            summons.Despawn(pSummoned);
        }

        void CheckHeight()
        {
            if (me->GetPositionZ() > floorZ + 3.5f || me->GetPositionZ() < floorZ - 1.2f)
                me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), floorZ + 0.2f, me->GetOrientation());

            events.ScheduleEvent(EVENT_HEIGHT_CHECK, 2000);
        }

        void DoCastBossSpell(Unit* target, uint32 spellId, bool trig, uint32 push = 0)
        {
            DoCast(target, spellId, trig);

            if (push)
                m_uiPushTimer = push;
        }

        void JustDied(Unit* pKiller)
        {
            _JustDied();

            Talk(TALK_DEATH);
            instance->SetBossState(DATA_JINROKH, DONE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            ResetStatues();
        }

        void KilledUnit(Unit* pVictim)
        {
            Talk(TALK_SLAY);
        }

        void DoAction(const int32 iAction)
        {
            if (iAction == ACTION_START_INTRO)
            {
                if (Aura* pVisual = me->AddAura(SPELL_LIGHTNING_STORM_VISUAL, me))
                    pVisual->SetDuration(15000);

                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_ATTACK1H);
                m_mEvents.ScheduleEvent(EVENT_INTRO_YELL, 15000);
            }
        }

        void ResetStatues()
        {
            std::list<Creature*> pStatues;
            GetCreatureListWithEntryInGrid(pStatues, me, NPC_JINROKH_STATUE, 200.f);

            for (auto const pCreature : pStatues)
            {
                if (pCreature->AI())
                    pCreature->AI()->DoAction(ACTION_RESET);
            }
        }

        void SpellHitTarget(Unit* pHit, const SpellInfo* pSpellInfo)
        {
            if (pHit)
            {
                if (pSpellInfo->Id == SPELL_THUNDERING_THROW)
                {
                    if (me->getThreatManager().getThreat(pHit))
                        me->getThreatManager().modifyThreatPercent(pHit, -100);
                }
            }
        }

        void DoHandleLightningStorm()
        {
            std::list<Creature*>pWaters;
            GetCreatureListWithEntryInGrid(pWaters, me, NPC_CONDUCTIVE_WATER, 200.f);

            pWaters.remove_if(electrifiedPredicate());

            for (Creature* pWater : pWaters)
            {
                if (pWater->AI())
                    pWater->AI()->DoAction(ACTION_ELECTRIFY);
            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiPointId == 1948)
            {
                me->NearTeleportTo(aCenterPos.GetPositionX(), aCenterPos.GetPositionY(), aCenterPos.GetPositionZ(), aCenterPos.GetOrientation());
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

                DoHandleLightningStorm();

                events.ScheduleEvent(EVENT_PROPAGATE_STORM, 300);

                me->UpdateObjectVisibility();
                me->UpdatePosition(me->GetPosition());
            }
        }

        void PropagateStorm()
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

            Talk(TALK_LIGHTNING_STORM);
            Talk(EMOTE_LIGHTNING_STORM);

            DoCastBossSpell(me->GetVictim(), SPELL_LIGHTNING_STORM, false, 3000);
            if (Aura* pAura = me->AddAura(SPELL_LIGHTNING_STORM_VISUAL, me))
                pAura->SetDuration(15000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            m_mEvents.Update(uiDiff);

            switch (m_mEvents.ExecuteEvent())
            {
            case EVENT_INTRO_YELL:
                Talk(TALK_INTRO);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                break;
            }

            if (!UpdateVictim())
                return;

            events.Update(uiDiff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_STATIC_BURST:
                    Talk(TALK_STATIC_BURST);
                    DoCastBossSpell(me->GetVictim(), SPELL_STATIC_BURST, false, 1000);
                    events.ScheduleEvent(EVENT_STATIC_BURST, urand(14000, 19000));
                    break;
                case EVENT_FOCUSED_LIGHTNING:
                    Talk(TALK_FOCUSED_LIGHTNING);
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, FocusedLightningSelection()))
                        DoCast(pTarget, SPELL_FOCUSED_LIGHTNING);
                    else if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(pTarget, SPELL_FOCUSED_LIGHTNING);
                    events.ScheduleEvent(EVENT_FOCUSED_LIGHTNING, urand(12000, 15000));
                    break;
                case EVENT_LIGHTNING_STORM:
                    me->GetMotionMaster()->MoveJump(aCenterPos, 35.f, 35.f, 1948);
                    events.ScheduleEvent(EVENT_LIGHTNING_STORM, 90000);
                    events.ScheduleEvent(EVENT_THUNDERING_THROW, 30000);
                    break;
                case EVENT_PROPAGATE_STORM:
                    PropagateStorm();
                    break;
                case EVENT_THUNDERING_THROW:
                    DoCast(me->GetVictim(), SPELL_THUNDERING_THROW);
                    Talk(TALK_THUNDERING_THROW);
                    Talk(EMOTE_THUNDERING_THROW, me->GetVictim()->GetGUID());
                    break;
                case EVENT_BERSERK:
                    Talk(TALK_BERSERK);
                    DoCast(me, SPELL_BERSERK, true);
                    break;
                case EVENT_HEIGHT_CHECK:
                    CheckHeight();
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_jinrokhAI(pCreature);
    }
};

class npc_focused_lightning : public CreatureScript
{
    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_FISSURE_CHECK
    };

public:
    npc_focused_lightning() : CreatureScript("npc_focused_lightning") {}

    struct npc_focused_lightningAI : public ScriptedAI
    {
        npc_focused_lightningAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            Initialize();
        }

        uint64 m_targetGuid;
        EventMap m_mEvents;

        void SetGUID(uint64 guid, int32 integer)
        {
            m_targetGuid = guid;
        }

        uint64 GetGUID(int32)
        {
            return m_targetGuid;
        }

        void Initialize()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            m_mEvents.ScheduleEvent(EVENT_FISSURE_CHECK, 1000);
            m_targetGuid = 0;
            me->AddAura(SPELL_FOCUSED_LIGHTNING_VISUAL, me);
            me->AddAura(SPELL_FOCUSED_LIGHTNING_SPEED, me);

            DoCast(SPELL_FOCUSED_LIGHTNING_TARGET);
        }

        void CheckHeight()
        {
            if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

            if (me->GetPositionZ() > floorZ + 3.5f || me->GetPositionZ() < floorZ - 1.2f)
                me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), floorZ + 0.2f, me->GetOrientation());
        }
        
        void GetFixatedPlayerOrGetNewIfNeeded()
        {
            std::list<Player*> players;
            GetPlayerListInGrid(players, me, 200.f);
            if (players.empty())
            {
                TC_LOG_DEBUG("scripts", "Focused Lightning guid %u found no players in instance %u, possible exploit", me->GetGUID(), me->GetMap()->GetInstanceId());
                return;
            }

            for (Player* pPlayer : players)
            {
                if (pPlayer->HasAura(SPELL_FOCUSED_LIGHTNING_FIXATE, me->GetGUID()))
                {
                    m_targetGuid = pPlayer->GetGUID();
                    Talk(0, m_targetGuid, true);
                    return;
                }
            }

            if (Player* pPlayer = Trinity::Containers::SelectRandomContainerElement(players))
            {
                DoCast(pPlayer, SPELL_FOCUSED_LIGHTNING_FIXATE, true);
                m_targetGuid = pPlayer->GetGUID();
                Talk(0, m_targetGuid, true);
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            m_mEvents.Update(uiDiff);

            switch (m_mEvents.ExecuteEvent())
            {
            case EVENT_FISSURE_CHECK:
                CheckHeight();
                if (Unit* pTarget = ObjectAccessor::GetPlayer(*me, m_targetGuid))
                {
                    me->GetMotionMaster()->MoveFollow(pTarget, 0.f, 0.f);
                }
                else
                {
                    GetFixatedPlayerOrGetNewIfNeeded();
                    me->GetMotionMaster()->MoveFollow(pTarget, 0.f, 0.f);
                }
                m_mEvents.ScheduleEvent(EVENT_FISSURE_CHECK, 400);
                break;
            }
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_focused_lightningAI(pCreature);
    }

};

class npc_lightning_fissure : public CreatureScript
{
public:
    npc_lightning_fissure() : CreatureScript("npc_lightning_fissure") {}

    struct npc_lightning_fissureAI : public ScriptedAI
    {
        npc_lightning_fissureAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            Initialize();
        }

        EventMap m_mEvents;

        void Initialize()
        {
            me->AddAura(SPELL_LIGHTNING_FISSURE_VISUAL, me);
            me->AddAura(SPELL_LIGHTNING_FISSURE_AURA, me);

            m_mEvents.ScheduleEvent(1, 500);
        }

        float GetSizeProp(Unit* propagator) const
        {
            if (Aura* pAura = propagator->GetAura(SPELL_CONDUCTIVE_WATER_GROW))
            {
                return ((float)0.5f * pAura->GetStackAmount()) + propagator->GetFloatValue(UNIT_FIELD_BOUNDING_RADIUS);
            }

            return 0;
        }

        void DoCheckFissure()
        {
            if (Creature* pWaters = GetClosestCreatureWithEntry(me, NPC_CONDUCTIVE_WATER, 100.f))
            {
                if (me->GetExactDist2d(pWaters) < GetSizeProp(pWaters))
                {
                    DoCast(me, SPELL_LIGHTNING_FISSURE_CONDUCTION, true);
                    me->DespawnOrUnsummon();
                    return;
                }
            }

            m_mEvents.ScheduleEvent(1, 500);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            m_mEvents.Update(uiDiff);

            switch (m_mEvents.ExecuteEvent())
            {
            case 1:
                DoCheckFissure();
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lightning_fissureAI(pCreature);
    }
};

class spell_focused_lightning_targeting : public SpellScriptLoader
{
public:
    spell_focused_lightning_targeting() : SpellScriptLoader("spell_focused_lightning_targeting") {}

    class spell_focused_lightning_targeting_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_focused_lightning_targeting_SpellScript);

        bool Validate()
        {
            return true;
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            if (GetCaster())
            {
                targets.remove_if(notPlayerPredicate());
                targets.remove_if(notInLosPredicate(GetCaster()));

                if (targets.size() > 1)
                {
                    Trinity::Containers::RandomResizeList(targets, 1);
                }
            }
        }

        void HandleHit()
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();

            if (!caster || !target)
                return;

            caster->CastSpell(target, SPELL_FOCUSED_LIGHTNING_FIXATE, true);

            if (caster->ToCreature() && caster->ToCreature()->AI())
            {
                caster->ToCreature()->AI()->Talk(0, target->GetGUID(), true);
                caster->ToCreature()->AI()->SetGUID(target->GetGUID());
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_focused_lightning_targeting_SpellScript::HandleHit);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_focused_lightning_targeting_SpellScript::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_focused_lightning_targeting_SpellScript();
    }
};

class spell_focused_lightning_aoe : public SpellScriptLoader
{
public:
    spell_focused_lightning_aoe() : SpellScriptLoader("spell_focused_lightning_aoe") {}

    class spell_focused_lightning_aoe_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_focused_lightning_aoe_SpellScript);

        bool Validate()
        {
            return true;
        }
        
        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(focusedLightningPredicate());
        }

        void HandleEffectHitTarget(SpellEffIndex eff_idx)
        {
            if (Unit* pCaster = GetCaster())
            {
                if (Creature* pLightningFissure = GetHitCreature())
                {
                    if (!pLightningFissure->HasAura(SPELL_LIGHTNING_FISSURE_AURA))
                        return;

                    // implosion
                    pLightningFissure->DisappearAndDie();
                    pCaster->CastSpell(pCaster, SPELL_IMPLOSION, true);
                    pCaster->Kill(pCaster);
                    return;
                }

                if (Unit* pUnit = GetHitUnit())
                {
                    // Should we detonate?
                    if (pUnit->HasAura(SPELL_FOCUSED_LIGHTNING_FIXATE, pCaster->GetGUID()))
                    {
                        bool violent = false;
                        bool should_conduct = false;

                        if (pUnit->HasAura(SPELL_FLUIDITY))
                        {
                            pUnit->CastSpell(pUnit, SPELL_FOCUSED_LIGHTNING_CONDUCTION, true, 0, 0, pCaster->GetGUID());
                            should_conduct = true;
                        }
                        else if (pUnit->HasAura(SPELL_ELECTRIFIED_WATERS))
                        {
                            pUnit->CastSpell(pUnit, SPELL_FOCUSED_LIGHTNING_CONDUCTION, true, 0, 0, pCaster->GetGUID());
                            violent = true;
                            should_conduct = true;
                        }

                        pCaster->CastSpell(pUnit, violent ? SPELL_VIOLENT_LIGHTNING_DETONATION : SPELL_FOCUSED_LIGHTNING_DETONATION, true);

                        if (!should_conduct)
                        {
                            if (Unit* pBoss = GetClosestCreatureWithEntry(pCaster, BOSS_JINROKH, 250.f))
                                pBoss->SummonCreature(NPC_LIGHTNING_FISSURE, pCaster->GetPosition());
                        }

                        if (pCaster->ToCreature())
                        {
                            pCaster->ToCreature()->RemoveAllAuras();
                            pCaster->ToCreature()->DespawnOrUnsummon(2000);
                            return;
                        }
                        pCaster->Kill(pCaster);
                        return;
                    }

                    pCaster->CastSpell(pUnit, SPELL_FOCUSED_LIGHTNING_DAMAGE, true);
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_focused_lightning_aoe_SpellScript::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_focused_lightning_aoe_SpellScript::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_focused_lightning_aoe_SpellScript();
    }
};

class spell_focused_lightning_speed : public SpellScriptLoader
{
public:
    spell_focused_lightning_speed() : SpellScriptLoader("spell_focused_lightning_speed") {}

    class spell_focused_lightning_speed_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_focused_lightning_speed_AuraScript);

        bool Validate()
        {
            return true;
        }

        void HandleOnPeriodic(AuraEffect const* aurEff)
        {
            if (WorldObject* pOwner = GetOwner())
            {
                if (pOwner->ToUnit())
                {
                    pOwner->ToUnit()->CastSpell(pOwner->ToUnit(), SPELL_FOCUSED_LIGHTNING_AOE, true);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_focused_lightning_speed_AuraScript::HandleOnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_focused_lightning_speed_AuraScript();
    }
};

class spell_focused_lightning_detonation : public SpellScriptLoader
{
public:
    spell_focused_lightning_detonation() : SpellScriptLoader("spell_focused_lightning_detonation") {}

    class spell_focused_lightning_detonation_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_focused_lightning_detonation_SpellScript);

        bool Validate()
        {
            return true;
        }

        void HandleAfterHit()
        {
            if (GetCaster())
                GetCaster()->UpdateObjectVisibility();
        }

        void Register()
        {
            AfterHit += SpellHitFn(spell_focused_lightning_detonation_SpellScript::HandleAfterHit);
        }

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_focused_lightning_detonation_SpellScript();
    }
};

class spell_implosion : public SpellScriptLoader
{
public:
    spell_implosion() : SpellScriptLoader("spell_implosion") {}

    class spell_implosion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_implosion_SpellScript)

        bool Validate()
        {
            return true;
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(notPlayerPredicate());
        }

        void HandleAfterHit()
        {
            if (GetCaster())
                GetCaster()->Kill(GetCaster());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_implosion_SpellScript::SelectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_implosion_SpellScript::SelectTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            AfterHit += SpellHitFn(spell_implosion_SpellScript::HandleAfterHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_implosion_SpellScript();
    }
};

class spell_static_wound : public SpellScriptLoader
{
public:
    spell_static_wound() : SpellScriptLoader("spell_static_wound") {}

    class spell_static_wound_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_static_wound_AuraScript)
        
        bool Validate()
        {
            return true;
        }

        bool Load()
        {
            m_timer = 3000;
            return true;
        }

        uint32 m_timer;

        void HandleOnApply(AuraEffect const *aurEff, AuraEffectHandleModes mode)
        {
            if (Aura* pAura = GetAura())
                pAura->SetStackAmount(10);
        }

        void HandleOnReApply(AuraEffect const *aurEff, AuraEffectHandleModes mode)
        {
            if (Aura* pAura = GetAura())
                pAura->SetStackAmount(pAura->GetStackAmount() + 10 > 30 ? 30 : pAura->GetStackAmount() + 10);
        }

        void HandleOnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            uint32 m_stacks = GetAura()->GetStackAmount();

            if (const SpellInfo* pSpellInfo = sSpellMgr->GetSpellInfo(SPELL_STATIC_WOUND, GetOwner()->GetMap()->GetDifficulty()))
            {
                int32 m_bp = pSpellInfo->Effects[0].BasePoints;
                int32 final_dmg = (m_bp * m_stacks); /// 3;

                if (Unit* pOwner = GetOwner()->ToUnit())
                {
                    pOwner->CastCustomSpell(pOwner, SPELL_STATIC_WOUND_DAMAGE, &final_dmg, 0, 0, true);
                }
            }
        }

        void HandleOnUpdate(const uint32 uiDiff)
        {
            if (m_timer <= uiDiff)
            {
                m_timer = 3000;
                //SetStackAmount(GetStackAmount() - 1);
                ModStackAmount(-1);
            }
            else
                m_timer -= uiDiff;
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_static_wound_AuraScript::HandleOnProc, EFFECT_0, SPELL_AURA_DUMMY);
            OnAuraUpdate += AuraUpdateFn(spell_static_wound_AuraScript::HandleOnUpdate);
            //OnEffectApply += AuraEffectApplyFn(spell_static_wound_AuraScript::HandleOnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            //OnEffectApply += AuraEffectApplyFn(spell_static_wound_AuraScript::HandleOnReApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAPPLY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_static_wound_AuraScript();
    }
};

class spell_static_wound_damage : public SpellScriptLoader
{
public:
    spell_static_wound_damage() : SpellScriptLoader("spell_static_wound_damage") {}

    class spell_static_wound_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_static_wound_damage_SpellScript);

        bool Validate()
        {
            return true;
        }

        void HandleEffectHitTarget(SpellEffIndex eff_idx)
        {
            if (Unit* pCaster = GetCaster())
            {
                if (Unit* pHit = GetHitUnit())
                {
                    if (pCaster == pHit)
                        SetHitDamage(GetHitDamage() * 3);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_static_wound_damage_SpellScript::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_static_wound_damage_SpellScript();
    }
};

class spell_static_burst : public SpellScriptLoader
{
public:
    spell_static_burst() : SpellScriptLoader("spell_static_burst") {}

    class spell_static_burst_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_static_burst_AuraScript);

        bool Validate()
        {
            return true;
        }

        void HandleOnPeriodic(AuraEffect const *aurEff)
        {
            if (Unit* pUnit = GetOwner()->ToUnit())
            {
                if (Aura* pAura = pUnit->GetAura(SPELL_STATIC_WOUND))
                {
                    pAura->ModStackAmount(pAura->GetStackAmount() + 10 < 30 ? (pAura->GetStackAmount() + 10) : 30);
                    pAura->SetDuration(25000);
                }
                else if (Aura* pAura = pUnit->AddAura(SPELL_STATIC_WOUND, pUnit))
                    pAura->SetStackAmount(10);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_static_burst_AuraScript::HandleOnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_static_burst_AuraScript();
    }
};

class spell_lightning_storm_visual : public SpellScriptLoader
{
public:
    spell_lightning_storm_visual() : SpellScriptLoader("spell_lightning_storm_visual") {}

    class spell_lightning_storm_visual_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_lightning_storm_visual_AuraScript);

        bool Validate()
        {
            return true;
        }

        void OnAuraEffectApply(AuraEffect const *aurEff, AuraEffectHandleModes mode)
        {
            if (GetCaster())
            {
                if (GetCaster()->HasAura(SPELL_LIGHTNING_STORM))
                    SetDuration(15000);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_lightning_storm_visual_AuraScript::OnAuraEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_lightning_storm_visual_AuraScript();
    }
};

class spell_thundering_throw_silence : public SpellScriptLoader
{
public:
    spell_thundering_throw_silence() : SpellScriptLoader("spell_thundering_throw_silence") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(validStatuePredicate());

            if (targets.size() > 1)
            {
                if (GetCaster())
                    targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));

                //if (WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets))
                {
                    //targets.emplace(targets.begin(), target);
                    targets.resize(1);
                }
            }
        }

        void HandleOnEffectHit(SpellEffIndex effIdx)
        {
            Unit* caster = GetCaster();
            Creature* target = GetHitCreature();

            if (!caster || !target)
                return;

            caster->CastSpell(target, SPELL_THUNDERING_THROW_JUMP, true);

            if (target->AI())
            {
                target->AI()->DoAction(ACTION_DESTROYED);
                target->AI()->SetGUID(caster->GetGUID());
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleOnEffectHit, EFFECT_4, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_4, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_thundering_throw_damage : public SpellScriptLoader
{
public:
    spell_thundering_throw_damage() : SpellScriptLoader("spell_thundering_throw_damage") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleOnEffectHit(SpellEffIndex effIdx)
        {
            if (Unit* pUnit = GetHitUnit())
                pUnit->CastSpell(pUnit, SPELL_THUNDERING_THROW_STUN, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleOnEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_thundering_throw : public SpellScriptLoader
{
public:
    spell_thundering_throw() : SpellScriptLoader("spell_thundering_throw") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleOnHit()
        {
            if (Unit* pUnit = GetHitUnit())
                pUnit->CastSpell(pUnit, SPELL_THUNDERING_THROW_SILENCE);
        }
        
        void Register()
        {
            OnHit += SpellHitFn(spell_impl::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

static uint32 max_casts = 66;

class npc_jinrokh_statue : public CreatureScript
{
    enum eEvents
    {
        EVENT_NONE,
        EVENT_WATER_BEAM,
        EVENT_SPAWN_WATER,
    };

public:
    npc_jinrokh_statue() : CreatureScript("npc_jinrokh_statue") {}

    struct npc_jinrokh_statueAI : public ScriptedAI
    {
        npc_jinrokh_statueAI(Creature* pCreature) : ScriptedAI(pCreature) 
        {
            statueGuid = 0;
            statueData = 0;
            m_phase = 0;
            playerGuid = 0;
            me->SetFloatValue(OBJECT_FIELD_SCALE_X, me->GetFloatValue(OBJECT_FIELD_SCALE_X) * 1.4f);
        }

        uint32 statueData;
        uint32 m_phase;
        uint64 playerGuid;
        uint64 statueGuid;
        
        void InitializeStatue()
        {
            statueGuid = 0;

            float fDist = 1000.f;

            if (InstanceScript* pInstance = me->GetInstanceScript())
            {
                for (uint32 i = 0; i < 4; ++i)
                {
                    if (GameObject* pGo = ObjectAccessor::GetGameObject(*me, pInstance->GetData64(GOB_MOGU_STATUE_1 + i)))
                    {
                        if (pGo->GetExactDist2d(me) < fDist)
                        {
                            fDist = pGo->GetExactDist2d(me);
                            statueGuid = pGo->GetGUID();
                        }
                    }
                }
            }

        }
        
        void HandleStatue(bool active)
        {
            if (GameObject* pStatue = ObjectAccessor::GetGameObject(*me, statueGuid))
                pStatue->SetGoState(active ? GO_STATE_ACTIVE : GO_STATE_READY);
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            if (uiType == DATA_STATUE_DESTROYED)
                statueData = uiData;
        }

        uint32 GetData(uint32 uiType)
        {
            if (uiType == DATA_STATUE_DESTROYED)
                return statueData;

            return 0;
        }

        void SetGUID(uint64 guid, int32 integer)
        {
            playerGuid = guid;
        }

        void DoAction(const int32 iAction)
        {
            if (iAction == ACTION_DESTROYED)
            {
                InitializeStatue();
                m_phase = 1;
                events.ScheduleEvent(EVENT_WATER_BEAM, 4000);
                events.ScheduleEvent(EVENT_SPAWN_WATER, 7000);
                SetData(DATA_STATUE_DESTROYED, 1);
            }

            if (iAction == ACTION_RESET)
            {
                HandleStatue(false);
                m_phase = 0;
                playerGuid = 0;
                me->SetVisible(false);
                me->SetVisible(true);
                me->UpdateObjectVisibility();
                me->RemoveAllAuras();
                events.Reset();
                SetData(DATA_STATUE_DESTROYED, 0);

                if (Creature* pWaters = GetClosestCreatureWithEntry(me, NPC_CONDUCTIVE_WATER, 100.f))
                    pWaters->DespawnOrUnsummon();
            }
        }

        Position DoSpawnWater()
        {
            uint8 dist = 0;
            float i_range = 1000.f;

            for (uint8 i = 0; i < 4; ++i)
            {
                float new_dist = me->GetDistance(aWaterPos[i]);

                if (new_dist < i_range)
                {
                    dist = i;
                    i_range = new_dist;
                }
            }

            return aWaterPos[dist];
        }

        void DoTossPlayer(Player* pPlayer)
        {
            HandleStatue(true);
            playerGuid = pPlayer->GetGUID();
            pPlayer->CastSpell(DoSpawnWater(), SPELL_THUNDERING_THROW_JUMP, true);
            pPlayer->CastSpell(pPlayer, SPELL_THUNDERING_THROW_HIT_DAMAGE, true);
            m_phase = 2;
        }

        void CheckPlayerState()
        {
            if (m_phase == 1)
            {
                if (Player* pPlayer = ObjectAccessor::GetPlayer(*me, playerGuid))
                {
                    if (!pPlayer->HasUnitState(UNIT_STATE_JUMPING))
                        DoTossPlayer(pPlayer);
                }
            }
            else if (m_phase == 2)
            {
                if (Player* pPlayer = ObjectAccessor::GetPlayer(*me, playerGuid))
                {
                    if (!pPlayer->HasUnitState(UNIT_STATE_JUMPING))
                        DoStunPlayer(pPlayer);
                }
            }
        }

        void DoStunPlayer(Player* pPlayer)
        {
            pPlayer->CastSpell(pPlayer, SPELL_THUNDERING_THROW_HIT_AOE_DAMAGE, true);
            pPlayer->CastSpell(pPlayer, SPELL_THUNDERING_THROW_STUN, true);
            pPlayer->RemoveAurasDueToSpell(SPELL_THUNDERING_THROW_SILENCE);

            m_phase = 0;
            playerGuid = 0;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            CheckPlayerState();

            switch (events.ExecuteEvent())
            {
            case EVENT_WATER_BEAM:
                me->AddAura(SPELL_CONDUCTIVE_WATERFALL, me);
                break;
            case EVENT_SPAWN_WATER:
                if (Creature* pWater = me->SummonCreature(NPC_CONDUCTIVE_WATER, DoSpawnWater()))
                {
                    if (pWater->AI())
                        pWater->AI()->DoAction(ACTION_RESET);
                }
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_jinrokh_statueAI(pCreature);
    }
};

class npc_conductive_water : public CreatureScript
{
    enum eEvents
    {
        EVENT_NONE,
        EVENT_GROW,
        EVENT_ELECTRIFY,
        EVENT_FINALIZE_ELECTRIFY
    };

public:
    npc_conductive_water() : CreatureScript("npc_conductive_water") {}

    struct npc_conductive_waterAI : public ScriptedAI
    {
        npc_conductive_waterAI(Creature* pCreature) : ScriptedAI(pCreature) {}

        uint32 m_size;

        void DoAction(const int32 iAction)
        {
            if (iAction == ACTION_RESET)
            {
                m_size = 0;
                me->AddAura(SPELL_CONDUCTIVE_WATER_VISUAL, me);
                events.ScheduleEvent(EVENT_GROW, 500);
            }

            if (iAction == ACTION_ELECTRIFY)
            {
                events.ScheduleEvent(EVENT_ELECTRIFY, 5000);
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            switch (events.ExecuteEvent())
            {
            case EVENT_GROW:
                events.ScheduleEvent(EVENT_GROW, 500);
                if (m_size < max_casts)
                {
                    DoCast(me, SPELL_CONDUCTIVE_WATER_GROW, true);
                    ++m_size;
                }
                DoCast(me, SPELL_CONDUCTIVE_WATER_DUMMY, true);
                break;
            case EVENT_ELECTRIFY:
                events.ScheduleEvent(EVENT_FINALIZE_ELECTRIFY, 1500);
                me->AddAura(SPELL_ELECTRIFY_WATERS, me);
                break;
            case EVENT_FINALIZE_ELECTRIFY:
                me->AddAura(SPELL_ELECTRIFIED_WATER_VISUAL, me);
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_conductive_waterAI(pCreature);
    }
};

class spell_conductive_water_dummy : public SpellScriptLoader
{
public:
    spell_conductive_water_dummy() : SpellScriptLoader("spell_conductive_water_dummy") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            if (Unit* caster = GetCaster())
            {
                targets.remove_if(notPlayerOrPetPredicate());
                targets.remove_if(scaleCheckPredicate(caster));
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        float GetSizeProp(Unit* propagator) const
        {
            if (Aura* pAura = propagator->GetAura(SPELL_CONDUCTIVE_WATER_GROW))
            {
                return ((float)0.5f * pAura->GetStackAmount()) + propagator->GetFloatValue(UNIT_FIELD_BOUNDING_RADIUS);
            }

            return 0;
        }

        uint32 spellId() const
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->HasAura(SPELL_ELECTRIFIED_WATER_VISUAL))
                    return SPELL_ELECTRIFIED_WATERS;

                return SPELL_FLUIDITY;
            }

            return 0;
        }

        void HandleOnApply(AuraEffect const *aurEff, AuraEffectHandleModes mode)
        {
            Unit* owner = GetTarget();
            Unit* caster = GetCaster();

            if (!owner || !caster)
                return;

            if (Aura* pAura = owner->GetAura(spellId(), caster->GetGUID()))
            {
                pAura->RefreshDuration();

                if (!owner->HasAura(SPELL_CONDUCTIVE_WATERS))
                    owner->AddAura(SPELL_CONDUCTIVE_WATERS, owner);
            }
            else
                caster->AddAura(spellId(), owner);
        }

        void HandleOnRemove(AuraEffect const *aurEff, AuraEffectHandleModes mode)
        {
            Unit* owner = GetTarget();
            Unit* caster = GetCaster();

            if (!owner || !caster)
                return;

            float dist_ex = owner->GetExactDist2d(caster);

            if (Aura* pAura = owner->GetAura(SPELL_FLUIDITY, caster->GetGUID()))
            {
                if (dist_ex > GetSizeProp(caster))
                    pAura->Remove(AURA_REMOVE_BY_EXPIRE);
            }
            else if (Aura* pAura = owner->GetAura(SPELL_ELECTRIFIED_WATERS, caster->GetGUID()))
            {
                if (dist_ex > GetSizeProp(caster))
                    pAura->Remove(AURA_REMOVE_BY_EXPIRE);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(aura_impl::HandleOnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            //OnEffectRemove += AuraEffectRemoveFn(aura_impl::HandleOnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

class spell_water_auras : public SpellScriptLoader
{
public:
    spell_water_auras() : SpellScriptLoader("spell_water_auras") {}

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void HandleOnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            if (Unit* owner = GetOwner()->ToUnit())
            {
                if (Aura* pAura = owner->GetAura(SPELL_CONDUCTIVE_WATERS))
                {
                    pAura->RefreshDuration();
                }
                else if (Aura* pAura = owner->AddAura(SPELL_CONDUCTIVE_WATERS, owner))
                {
                    pAura->SetMaxDuration(GetMaxDuration());
                    pAura->RefreshDuration();
                }
            }
        }

        void HandleOnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            if (Unit* owner = GetOwner()->ToUnit())
                owner->RemoveAurasDueToSpell(SPELL_CONDUCTIVE_WATERS);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(aura_impl::HandleOnApply, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(aura_impl::HandleOnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(aura_impl::HandleOnRemove, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(aura_impl::HandleOnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

class spell_focused_lightning_conduction : public SpellScriptLoader
{
public:
    spell_focused_lightning_conduction() : SpellScriptLoader("spell_focused_lightning_conduction") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            if (Unit* caster = GetCaster())
            {
                if (Creature* pWaters = GetClosestCreatureWithEntry(caster, NPC_CONDUCTIVE_WATER, 100.f))
                {
                    targets.remove_if(conductionPredicate(pWaters));
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_lightning_fissure_conduction : public SpellScriptLoader
{
public:
    spell_lightning_fissure_conduction() : SpellScriptLoader("spell_lightning_fissure_conduction") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(notPlayerPredicate());

            if (Unit* caster = GetCaster())
            {
                if (Creature* pWaters = GetClosestCreatureWithEntry(caster, NPC_CONDUCTIVE_WATER, 100.f))
                {
                    targets.remove_if(conductionPredicate(pWaters));
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_focused_lightning : public SpellScriptLoader
{
public:
    spell_focused_lightning() : SpellScriptLoader("spell_focused_lightning") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleOnHit()
        {
            Unit* pCaster = GetCaster();
            Unit* pHit = GetHitUnit();

            if (!pCaster || !pHit)
                return;

            Position pos;
            pCaster->GetRandomNearPosition(pos, 10.f);
            pCaster->SummonCreature(NPC_FOCUSED_LIGHTNING, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_impl::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_lightning_fissure_damage : public SpellScriptLoader
{
public:
    spell_lightning_fissure_damage() : SpellScriptLoader("spell_lightning_fissure_damage") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(notPlayerPredicate());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

void AddSC_boss_jinrokh()
{
    new boss_jinrokh();
    new npc_focused_lightning();
    new npc_lightning_fissure();
    new spell_focused_lightning_targeting();
    new spell_focused_lightning_aoe();
    new spell_focused_lightning_speed();
    new spell_focused_lightning_detonation();
    new spell_implosion();
    new spell_static_wound();
    new spell_static_wound_damage();
    new spell_static_burst();
    new spell_lightning_storm_visual();
    new spell_thundering_throw_silence();
    new spell_thundering_throw_damage();
    new spell_thundering_throw();
    new npc_jinrokh_statue();
    new npc_conductive_water();
    new spell_conductive_water_dummy();
    new spell_water_auras();
    new spell_focused_lightning_conduction();
    new spell_lightning_fissure_conduction();
    new spell_focused_lightning();
    new spell_lightning_fissure_damage();
}
