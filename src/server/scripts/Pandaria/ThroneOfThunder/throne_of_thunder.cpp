#include "throne_of_thunder.h"
#include "CreatureTextMgr.h"

/* 3/14-2015 Emre */

enum sSpells : uint32
{
    SPELL_FOCUSED_LIGHTNING_TARGET      = 139203,
    SPELL_FOCUSED_LIGHTNING_AOE         = 139209,
    SPELL_FOCUSED_LIGHTNING             = 139206,
    SPELL_FOCUSED_LIGHTNING_DAMAGE      = 139210,
    SPELL_FOCUSED_LIGHTNING_DETONATION  = 139211
};

enum eCreatures : uint32
{
    NPC_CRAZED_STORMCALLER          = 70491,
    NPC_ZANDALARI_STORMCALLER       = 70236
};

class npc_zandalari_spearshaper : public CreatureScript
{
    enum eSpells : uint32
    {
        // needs disarm
        // cmake, scriptloader
        SPELL_SPEAR_DISARM          = 137066,
        SPELL_SPEAR_PULSE_AOE       = 137058,
        SPELL_BERSERKING            = 137096,
        SPELL_THROW_SPEAR           = 136986,
        SPELL_RETRIEVE_SPEAR_JUMP   = 137070,
        SPELL_RETRIEVE_SPEAR_STUN   = 137072,
        SPELL_SPEAR_SPIN            = 137077
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_ZERK,
        EVENT_SPEAR_THROW,
        EVENT_RETRIEVE_SPEAR,
        EVENT_RE_SPIN,
    };

    enum eCreatures : uint32
    {
        NPC_SPEAR                   = 69438
    };

public:
    npc_zandalari_spearshaper() : CreatureScript("npc_zandalari_spearshaper") {}

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature* pCreature) : ScriptedAI(pCreature) {}

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
            m_triggerGuid = 0;

            if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        uint64 m_triggerGuid;

        void JustSummoned(Creature* pSummoned)
        {
            summons.Summon(pSummoned);
            
            if (pSummoned->GetEntry() == NPC_SPEAR)
                m_triggerGuid = pSummoned->GetGUID();
        }

        void SummonedCreatureDespawn(Creature* pSummoned)
        {
            summons.Despawn(pSummoned);
        }

        void EnterCombat(Unit* pWho)
        {
            events.ScheduleEvent(EVENT_ZERK, 6000 + rand() % 5000);
            events.ScheduleEvent(EVENT_SPEAR_THROW, 10000 + rand() % 7000);
        }

        void JumpToSpear()
        {
            Position pos;

            if (Creature* pSpear = ObjectAccessor::GetCreature(*me, m_triggerGuid))
            {
                pSpear->GetPosition(&pos);
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveJump(pos, 10.f, 20.f, 1948);

            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiPointId == 1948)
            {
                DoCast(me, SPELL_RETRIEVE_SPEAR_STUN, true);

                if (Creature* pSpear = ObjectAccessor::GetCreature(*me, m_triggerGuid))
                    pSpear->DespawnOrUnsummon();

                me->RemoveAurasDueToSpell(SPELL_SPEAR_DISARM);

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                events.ScheduleEvent(EVENT_RE_SPIN, 800);
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            events.Update(uiDiff);

            if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_SPEAR_SPIN))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ZERK:
                    DoCast(SPELL_BERSERKING);
                    events.ScheduleEvent(EVENT_ZERK, 8000 + rand() % 5000);
                    break;
                case EVENT_SPEAR_THROW:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1))
                        DoCast(pTarget, SPELL_THROW_SPEAR);
                    events.ScheduleEvent(EVENT_RETRIEVE_SPEAR, 5000);
                    events.ScheduleEvent(EVENT_SPEAR_THROW, 25000 + rand() % 8000);
                    break;
                case EVENT_RETRIEVE_SPEAR:
                    JumpToSpear();
                    break;
                case EVENT_RE_SPIN:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    me->GetMotionMaster()->MoveChase(me->GetVictim());
                    DoCast(me, SPELL_SPEAR_SPIN, true);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new ai_impl(pCreature);
    }
};

class npc_zandalari_stormcaller : public CreatureScript
{
    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_STUN,
        EVENT_STORM_ENERGY
    };

    enum eSpells : uint32
    {
        SPELL_STORM_WEAPON      = 139319,
        SPELL_STUN              = 35865,
        SPELL_STORM_ENERGY      = 139559
    };

public:
    npc_zandalari_stormcaller() : CreatureScript("npc_zandalari_stormcaller") {}

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature* pCreature) : ScriptedAI(pCreature) {}

        void Reset()
        {
            events.Reset();

            if (me->GetEntry() != NPC_ZANDALARI_STORMCALLER)
                me->UpdateEntry(NPC_ZANDALARI_STORMCALLER);
        }

        /*
        void JustReachedHome()
        {
            const CreatureDisplayInfoEntry* pModel = sCreatureDisplayInfoStore.LookupEntry(me->GetDisplayId());
            if (pModel)
                me->SetObjectScale(pModel->scale);
        }*/

        void EnterCombat(Unit* pWho)
        {
            if (me->GetEntry() == NPC_ZANDALARI_STORMCALLER)
                DoCast(SPELL_STORM_WEAPON);

            events.ScheduleEvent(EVENT_STUN, 5000);
            events.ScheduleEvent(EVENT_STORM_ENERGY, 5000 + rand() % 4000);
        }

        void JustDied(Unit* pKiller)
        {
            if (InstanceScript* pInstance = me->GetInstanceScript())
                pInstance->SetData(TYPE_JINROKH_INTRO, DONE);
        }

        void DoStun()
        {
            std::list<HostileReference*> threatList = me->getThreatManager().getThreatList();
            std::list<Unit*> spellTargets;

            for (auto ref : threatList)
            {
                if (Unit* pTarget = ObjectAccessor::GetUnit(*me, ref->getUnitGuid()))
                    spellTargets.push_back(pTarget);
            }

            uint32 m_uiMaxTargets = me->GetMap()->Is25ManRaid() ? 4 : 1;

            if (!spellTargets.empty())
            {
                uint32 m_size = spellTargets.size();

                if (m_size > m_uiMaxTargets)
                {
                    Trinity::Containers::RandomResizeList(spellTargets, m_uiMaxTargets);
                }

                for (auto const pUnit : spellTargets)
                    pUnit->CastSpell(pUnit, SPELL_STUN, true, 0, 0, me->GetGUID());
            }          
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            events.Update(uiDiff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
            case EVENT_STORM_ENERGY:
                DoCast(SPELL_STORM_ENERGY);
                events.ScheduleEvent(EVENT_STORM_ENERGY, 4000 + rand() % 2500);
                break;
            case EVENT_STUN:
                DoStun();
                events.ScheduleEvent(EVENT_STUN, 6000 + rand() % 3000);
                break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new ai_impl(pCreature);
    }
};

class npc_focused_lightning_trash : public CreatureScript
{
    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_PULSE,
        EVENT_FIXATE_CHECK
    };

public:
    npc_focused_lightning_trash() : CreatureScript("npc_focused_lightning_trash") {}

    struct npc_focused_lightningAI : public ScriptedAI
    {
        npc_focused_lightningAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            Initialize();
        }

        uint64 m_targetGuid;
        EventMap m_mEvents;

        void SetGUID(uint64 guid, int32)
        {
            m_targetGuid = guid;
        }

        uint64 GetGUID(int32)
        {
            return m_targetGuid;
        }

        void Initialize()
        {
            m_mEvents.ScheduleEvent(EVENT_PULSE, 500);
            m_mEvents.ScheduleEvent(EVENT_FIXATE_CHECK, 1500);
            m_targetGuid = 0;
            me->AddAura(SPELL_FOCUSED_LIGHTNING_VISUAL, me);

            DoCast(SPELL_FOCUSED_LIGHTNING_TARGET);
        }

        void GetFixatedPlayerOrGetNewIfNeeded()
        {
            std::list<Player*> players;
            GetPlayerListInGrid(players, me, 200.f);

            if (players.empty())
            {
                TC_LOG_ERROR("scripts", "Focused Lightning guid %u found no players in instance %u, possible exploit", me->GetGUID(), me->GetMap()->GetInstanceId());
                return;
            }

            for (Player* pPlayer : players)
            {
                if (pPlayer->HasAura(SPELL_FOCUSED_LIGHTNING_FIXATE, me->GetGUID()))
                {
                    m_targetGuid = pPlayer->GetGUID();
                    return;
                }
            }

            if (Player* pPlayer = Trinity::Containers::SelectRandomContainerElement(players))
            {
                DoCast(pPlayer, SPELL_FOCUSED_LIGHTNING_FIXATE, true);
                m_targetGuid = pPlayer->GetGUID();
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            m_mEvents.Update(uiDiff);

            switch (m_mEvents.ExecuteEvent())
            {
            case EVENT_FIXATE_CHECK:
                if (Unit* pTarget = ObjectAccessor::GetPlayer(*me, m_targetGuid))
                {
                    me->GetMotionMaster()->MoveFollow(pTarget, 0.f, 0.f);
                }
                else
                {
                    GetFixatedPlayerOrGetNewIfNeeded();
                    me->GetMotionMaster()->MoveFollow(pTarget, 0.f, 0.f);
                }
                m_mEvents.ScheduleEvent(EVENT_FIXATE_CHECK, 500);
                break;
            case EVENT_PULSE:
                DoCast(me, SPELL_FOCUSED_LIGHTNING_AOE, true);
                m_mEvents.ScheduleEvent(EVENT_PULSE, 500);
                break;
            }
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_focused_lightningAI(pCreature);
    }
};

class spell_focused_lightning_aoe_trash : public SpellScriptLoader
{
public:
    spell_focused_lightning_aoe_trash() : SpellScriptLoader("spell_focused_lightning_aoe_trash") {}

    class spell_focused_lightning_aoe_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_focused_lightning_aoe_SpellScript);

        bool Validate()
        {
            return true;
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(notPlayerPredicate());
        }

        void HandleEffectHitTarget(SpellEffIndex eff_idx)
        {
            if (Unit* pCaster = GetCaster())
            {

                if (Unit* pUnit = GetHitUnit())
                {
                    if (pUnit->HasAura(SPELL_FOCUSED_LIGHTNING_FIXATE, pCaster->GetGUID()))
                    {
                        pCaster->CastSpell(pUnit, SPELL_FOCUSED_LIGHTNING_DETONATION, true);
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

class spell_storm_weapon : public SpellScriptLoader
{
public:
    spell_storm_weapon() : SpellScriptLoader("spell_storm_weapon") {}

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void HandleOnApply(AuraEffect const* eff, AuraEffectHandleModes mode)
        {
            if (Creature* pCaster = GetCaster()->ToCreature())
            {
                pCaster->UpdateEntry(NPC_CRAZED_STORMCALLER);
            }
        }

        void HandleOnRemove(AuraEffect const* eff, AuraEffectHandleModes mode)
        {
            if (Creature* pCaster = GetCaster()->ToCreature())
            {
                pCaster->UpdateEntry(NPC_ZANDALARI_STORMCALLER);
            }
        }

        void HandleOnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            if (Unit* caster = GetCaster())
            {
                if (caster->ToCreature() && caster->ToCreature()->GetVictim())
                    caster->CastSpell(caster->ToCreature()->GetVictim(), 139220, true);
            }
        }


        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(aura_impl::HandleOnApply, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectApplyFn(aura_impl::HandleOnRemove, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectProc += AuraEffectProcFn(aura_impl::HandleOnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

class spell_storm_energy : public SpellScriptLoader
{
public:
    spell_storm_energy() : SpellScriptLoader("spell_storm_energy") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        bool Validate()
        {
            return sSpellMgr->GetSpellInfo(139559);
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(notPlayerPredicate());

            if (Unit* caster = GetCaster())
            {
                uint32 m_maxSize = caster->GetMap()->Is25ManRaid() ? 3 : 1;

                if (targets.size() > m_maxSize)
                    Trinity::Containers::RandomResizeList(targets, m_maxSize);
            }
        }

        void HandleEffectHitTarget(SpellEffIndex eff_idx)
        {
            Unit* target = GetHitUnit();
            Unit* caster = GetCaster();

            if (!caster || !target)
                return;

            caster->AddAura(GetSpellInfo()->Effects[0].BasePoints, target);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_storm_weapon_proc : public SpellScriptLoader
{
public:
    spell_storm_weapon_proc() : SpellScriptLoader("spell_storm_weapon_proc") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTarget(WorldObject*&target)
        {
            Unit* caster = GetCaster();

            if (caster && caster->ToCreature())
            {
                if (Unit* pVictim = caster->ToCreature()->GetVictim())
                {
                    target = pVictim;
                }
            }
        }

        void Register()
        {
            OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_impl::SelectTarget, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};


class spell_storm_weapon_aura : public SpellScriptLoader
{
public:
    spell_storm_weapon_aura() : SpellScriptLoader("spell_storm_weapon_aura") {}

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void HandleOnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            if (Unit* caster = GetCaster())
            {
                if (caster->ToCreature() && caster->ToCreature()->GetVictim())
                    caster->CastSpell(caster->ToCreature()->GetVictim(), 139220, true);
            }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(aura_impl::HandleOnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

class spell_eruption : public SpellScriptLoader
{
    enum enums : uint32
    {
        NPC_ERUPTION        = 70029,
        SPELL_ERUPTION      = 138655
    };

public:
    spell_eruption() : SpellScriptLoader("spell_eruption") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(notPlayerPredicate());

            if (targets.size() > 1)
                Trinity::Containers::RandomResizeList(targets, 1);
        }

        void HandleEffectHitTarget(SpellEffIndex eff_idx)
        {
            Unit* target = GetHitUnit();
            Unit* caster = GetCaster();

            if (!target || !caster)
                return;

            if (Creature* pEruption = caster->SummonCreature(NPC_ERUPTION, *target, TEMPSUMMON_TIMED_DESPAWN, 6000))
                pEruption->AddAura(SPELL_ERUPTION, pEruption);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY); // Changed in SpellMgr::138652
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);    // Changed in SpellMgr::138652
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class inRangePredicate
{
private:
    Unit* caster;
public:
    inRangePredicate(Unit* _caster) : caster(_caster) {}

    bool operator()(WorldObject* target) const
    {
        return target->GetExactDist2d(caster) < 15.1f;
    }
};

class spell_siphon_life_tot : public SpellScriptLoader
{
public:
    spell_siphon_life_tot() : SpellScriptLoader("spell_siphon_life_tot") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(inRangePredicate(GetCaster()));
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

class go_ancient_mogu_bell : public GameObjectScript
{
public:
    go_ancient_mogu_bell() : GameObjectScript("go_ancient_mogu_bell")
    {

    }

    void OnGameObjectStateChanged(GameObject* pGo, uint32 state)
    {
        if (state == GO_STATE_ACTIVE)
        {
            if (pGo->GetInstanceScript())
            {
                if (pGo->GetInstanceScript()->GetData(TYPE_BELLS_RUNG) > 2)
                    return;

                pGo->GetInstanceScript()->SetData(TYPE_BELLS_RUNG, (pGo->GetInstanceScript()->GetData(TYPE_BELLS_RUNG)) + 1);

                if (Creature* pTalk = GetClosestCreatureWithEntry(pGo, 68553, 5.f))
                {
                    if (pTalk->AI())
                        pTalk->AI()->Talk(pTalk->GetInstanceScript()->GetData(TYPE_BELLS_RUNG) - 1, 0, false, TEXT_RANGE_AREA);

                }
            }
        }
    }
};

void AddSC_throne_of_thunder()
{
    new npc_zandalari_spearshaper();
    new npc_zandalari_stormcaller();
    new npc_focused_lightning_trash();
    new spell_focused_lightning_aoe_trash();
    new spell_storm_weapon();
    new spell_storm_energy();
    //new spell_storm_weapon_proc();
    new spell_storm_weapon_aura();
    new spell_eruption();
    new spell_siphon_life_tot();
    new go_ancient_mogu_bell();
}