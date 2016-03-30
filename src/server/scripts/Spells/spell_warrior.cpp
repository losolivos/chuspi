/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Scripts for spells with SPELLFAMILY_WARRIOR and SPELLFAMILY_GENERIC spells used by warrior players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_warr_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"

enum WarriorSpells
{
    WARRIOR_SPELL_DEFENSIVE_STANCE              = 71,
    WARRIOR_SPELL_LAST_STAND_TRIGGERED          = 12976,
    WARRIOR_SPELL_VICTORY_RUSH_DAMAGE           = 34428,
    WARRIOR_SPELL_VICTORY_RUSH_HEAL             = 118779,
    WARRIOR_SPELL_VICTORIOUS_STATE              = 32216,
    WARRIOR_SPELL_BLOODTHIRST                   = 23881,
    WARRIOR_SPELL_BLOODTHIRST_HEAL              = 117313,
    WARRIOR_SPELL_DEEP_WOUNDS                   = 115767,
    WARRIOR_SPELL_THUNDER_CLAP                  = 6343,
    WARRIOR_SPELL_WEAKENED_BLOWS                = 115798,
    WARRIOR_SPELL_BLOOD_AND_THUNDER             = 84615,
    WARRIOR_SPELL_SHOCKWAVE_STUN                = 132168,
    WARRIOR_SPELL_HEROIC_LEAP_DAMAGE            = 52174,
    WARRIOR_SPELL_RALLYING_CRY                  = 97463,
    WARRIOR_SPELL_SWORD_AND_BOARD               = 50227,
    WARRIOR_SPELL_SHIELD_SLAM                   = 23922,
    WARRIOR_SPELL_MOCKING_BANNER_TAUNT          = 114198,
    WARRIOR_NPC_MOCKING_BANNER                  = 59390,
    WARRIOR_SPELL_BERZERKER_RAGE_EFFECT         = 23691,
    WARRIOR_SPELL_ENRAGE                        = 12880,
    WARRIOR_SPELL_COLOSSUS_SMASH                = 86346,
    WARRIOR_SPELL_SECOND_WIND_REGEN             = 16491,
    WARRIOR_SPELL_SECOND_WIND_DUMMY             = 125667,
    WARRIOR_SPELL_UNBRIDLED_WRATH_REGEN         = 29842,
    WARRIOR_SPELL_DRAGON_ROAR_KNOCK_BACK        = 118895,
    WARRIOR_SPELL_PHYSICAL_VULNERABILITY        = 81326,
    WARRIOR_SPELL_STORM_BOLT_STUN               = 132169,
    WARRIOR_SPELL_SHIELD_BLOCKC_TRIGGERED       = 132404,
    WARRIOR_SPELL_GLYPH_OF_HINDERING_STRIKES    = 58366,
    WARRIOR_SPELL_SLUGGISH                      = 129923,
    WARRIOR_SPELL_IMPENDING_VICTORY             = 103840,
    WARRIOR_SPELL_ITEM_PVP_SET_4P_BONUS         = 133277,
    WARRIOR_SPELL_HEROIC_LEAP_SPEED             = 133278,
};

// Victorious State - 32216
class spell_warr_victorious_state : public SpellScriptLoader
{
    public:
        spell_warr_victorious_state() : SpellScriptLoader("spell_warr_victorious_state") { }

        class spell_warr_victorious_state_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_victorious_state_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasSpellCooldown(WARRIOR_SPELL_IMPENDING_VICTORY))
                        _player->RemoveSpellCooldown(WARRIOR_SPELL_IMPENDING_VICTORY, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_victorious_state_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_victorious_state_SpellScript();
        }
};

// Called by Heroic Strike - 78 and Cleave - 845
// Glyph of Hindering Strikes - 58366
class spell_warr_glyph_of_hindering_strikes : public SpellScriptLoader
{
    public:
        spell_warr_glyph_of_hindering_strikes() : SpellScriptLoader("spell_warr_glyph_of_hindering_strikes") { }

        class spell_warr_glyph_of_hindering_strikes_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_glyph_of_hindering_strikes_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(WARRIOR_SPELL_GLYPH_OF_HINDERING_STRIKES))
                            _player->CastSpell(target, WARRIOR_SPELL_SLUGGISH, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_glyph_of_hindering_strikes_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_glyph_of_hindering_strikes_SpellScript();
        }
};

// Stampeding Shout - 122294
class spell_warr_stampeding_shout : public SpellScriptLoader
{
    public:
        spell_warr_stampeding_shout() : SpellScriptLoader("spell_warr_stampeding_shout") { }

        class spell_warr_stampeding_shout_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_stampeding_shout_SpellScript);

            void HandleOnHit()
            {
                if (GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        target->RemoveMovementImpairingAuras();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_stampeding_shout_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_stampeding_shout_SpellScript();
        }
};

// Shield Barrier - 112048
class spell_warr_shield_barrier final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SHIELD_BARRIER = 112048
        };

        void calculateAmount(AuraEffect const *, int32 &amount, bool &canBeRecalculated)
        {
            canBeRecalculated = false;

            auto const caster = GetCaster();
            if (!caster)
                return;

            auto rageCost = 0;
            if (auto const currentSpell = caster->FindCurrentSpellBySpellId(SHIELD_BARRIER))
                rageCost = currentSpell->GetPowerCost();

            auto const totalRage = caster->GetPower(POWER_RAGE) + rageCost;
            auto const consumedRage = std::min(600, totalRage);

            caster->SetPower(POWER_RAGE, totalRage - consumedRage);

            auto const ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
            auto const strength = caster->GetStat(STAT_STRENGTH) - 10;
            auto const stamina = caster->GetStat(STAT_STAMINA);

            amount = std::max(2 * (ap - 2 * strength), stamina * 2.5f) * consumedRage / 600;
        }

        void Register() final
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::calculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        }
    };

public:
    spell_warr_shield_barrier()
        : SpellScriptLoader("spell_warr_shield_barrier")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Shield Block - 2565
class spell_warr_shield_block : public SpellScriptLoader
{
    public:
        spell_warr_shield_block() : SpellScriptLoader("spell_warr_shield_block") { }

        class spell_warr_shield_block_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_shield_block_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->CastSpell(_player, WARRIOR_SPELL_SHIELD_BLOCKC_TRIGGERED, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_shield_block_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_shield_block_SpellScript();
        }
};

// Storm Bolt - 107570
class spell_warr_storm_bolt : public SpellScriptLoader
{
    public:
        spell_warr_storm_bolt() : SpellScriptLoader("spell_warr_storm_bolt") { }

        class spell_warr_storm_bolt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_storm_bolt_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* unitTarget = GetHitUnit())
                        _player->CastSpell(unitTarget, WARRIOR_SPELL_STORM_BOLT_STUN, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_storm_bolt_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_storm_bolt_SpellScript();
        }
};

// Storm Bolt damage (145585, 107570)
class spell_warr_storm_bolt_damage : public SpellScriptLoader
{
    public:
        spell_warr_storm_bolt_damage() : SpellScriptLoader("spell_warr_storm_bolt_damage") { }

        class spell_warr_storm_bolt_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_storm_bolt_damage_SpellScript);

            void HandleOnHit()
            {
                if (Unit* unitTarget = GetHitUnit())
                    if (unitTarget->GetTypeId() != TYPEID_PLAYER && unitTarget->IsImmunedToSpellEffect(sSpellMgr->GetSpellInfo(WARRIOR_SPELL_STORM_BOLT_STUN), 0))
                        SetHitDamage(GetHitDamage() * 4);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_storm_bolt_damage_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_storm_bolt_damage_SpellScript();
        }
};


// Colossus Smash - 86346
class spell_warr_colossus_smash : public SpellScriptLoader
{
    public:
        spell_warr_colossus_smash() : SpellScriptLoader("spell_warr_colossus_smash") { }

        class spell_warr_colossus_smash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_colossus_smash_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, WARRIOR_SPELL_PHYSICAL_VULNERABILITY, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_colossus_smash_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_colossus_smash_SpellScript();
        }
};

// Dragon Roar - 118000
class spell_warr_dragon_roar : public SpellScriptLoader
{
    public:
        spell_warr_dragon_roar() : SpellScriptLoader("spell_warr_dragon_roar") { }

        class spell_warr_dragon_roar_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_dragon_roar_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, WARRIOR_SPELL_DRAGON_ROAR_KNOCK_BACK, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_dragon_roar_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_dragon_roar_SpellScript();
        }
};

// Staggering Shout - 107566
class spell_warr_staggering_shout : public SpellScriptLoader
{
    public:
        spell_warr_staggering_shout() : SpellScriptLoader("spell_warr_staggering_shout") { }

        class spell_warr_staggering_shout_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_staggering_shout_SpellScript);

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraTypeCheck(false, SPELL_AURA_MOD_DECREASE_SPEED));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warr_staggering_shout_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_staggering_shout_SpellScript();
        }
};

// Second Wind - 29838
class spell_warr_second_wind final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl);

        bool checkProc(ProcEventInfo & info)
        {
            Unit * const caster = GetCaster();
            if (!caster || caster->HasAura(WARRIOR_SPELL_SECOND_WIND_REGEN) || !caster->HealthBelowPctDamaged(35, info.GetDamageInfo()->GetDamage()))
                return false;

            return true;
        }

        void onProc(AuraEffect const *, ProcEventInfo &)
        {
            GetCaster()->CastSpell(GetCaster(), WARRIOR_SPELL_SECOND_WIND_REGEN, true);
        }

        void Register() final
        {
            DoCheckProc += AuraCheckProcFn(script_impl::checkProc);
            OnEffectProc += AuraEffectProcFn(script_impl::onProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_warr_second_wind() : SpellScriptLoader("spell_warr_second_wind") { }

    AuraScript* GetAuraScript() const final
    {
        return new script_impl();
    }
};

class spell_warr_second_wind_aura : public SpellScriptLoader
{
    public:
        spell_warr_second_wind_aura() : SpellScriptLoader("spell_warr_second_wind_aura") { }

        class spell_warr_second_wind_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warr_second_wind_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->AddAura(125667, caster);
            }

            void OnRemove(const AuraEffect* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(125667))
                        caster->RemoveAura(125667);
            }

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                    caster->AddAura(125667, caster);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warr_second_wind_AuraScript::OnApply, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_warr_second_wind_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warr_second_wind_AuraScript::OnTick, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warr_second_wind_AuraScript();
        }
};

// Unbridled Wrath - 143268
class spell_warr_unbridled_wrath final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl);

        bool checkProc(ProcEventInfo &eventInfo)
        {
            auto const caster = eventInfo.GetActionTarget();
            auto const target = eventInfo.GetProcTarget();

            if (!caster || !target || caster == target)
                return false;

            if (!eventInfo.GetSpellInfo() || !(eventInfo.GetHitMask() & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT)))
                return false;

            if (!(eventInfo.GetSpellInfo()->GetAllEffectsMechanicMask() & ((1<<MECHANIC_ROOT)|(1<<MECHANIC_STUN))))
                return false;

            return true;
        }

        void onProc(AuraEffect const *, ProcEventInfo &eventInfo)
        {
            PreventDefaultAction();
            eventInfo.GetActionTarget()->CastSpell(eventInfo.GetActionTarget(), WARRIOR_SPELL_UNBRIDLED_WRATH_REGEN, true);
        }

        void Register() final
        {
            DoCheckProc += AuraCheckProcFn(script_impl::checkProc);
            OnEffectProc += AuraEffectProcFn(script_impl::onProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_warr_unbridled_wrath() : SpellScriptLoader("spell_warr_unbridled_wrath") { }

    AuraScript* GetAuraScript() const final
    {
        return new script_impl();
    }
};

// Berserker Rage - 18499
class spell_warr_berserker_rage : public SpellScriptLoader
{
    public:
        spell_warr_berserker_rage() : SpellScriptLoader("spell_warr_berserker_rage") { }

        class spell_warr_berserker_rage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_berserker_rage_SpellScript);

            void HandleOnHit()
            {
                GetCaster()->CastSpell(GetCaster(), WARRIOR_SPELL_ENRAGE, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_berserker_rage_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_berserker_rage_SpellScript();
        }
};

// Mocking Banner - 114192
class spell_warr_mocking_banner : public SpellScriptLoader
{
    public:
        spell_warr_mocking_banner() : SpellScriptLoader("spell_warr_mocking_banner") { }

        class spell_warr_mocking_banner_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warr_mocking_banner_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (Player* player = GetTarget()->ToPlayer())
                {
                    std::list<Creature*> bannerList;
                    std::list<Creature*> tempList;

                    GetTarget()->GetCreatureListWithEntryInGrid(tempList, WARRIOR_NPC_MOCKING_BANNER, 30.0f);

                    bannerList = tempList;

                    // Remove other players banners
                    for (auto itr : tempList)
                    {
                        Unit* owner = itr->GetOwner();
                        if (owner && owner == player && itr->IsSummon())
                            continue;

                        bannerList.remove(itr);
                    }

                    for (auto itr : bannerList)
                        player->CastSpell(itr, WARRIOR_SPELL_MOCKING_BANNER_TAUNT, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warr_mocking_banner_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warr_mocking_banner_AuraScript();
        }
};

// Called by the proc of Enrage - 12880 
// Raging Blow (allow to use it) - 131116
class spell_warr_enrage_raging_blow final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            RAGING_BLOW_ENABLER = 131116,
        };

        void afterHit()
        {
            auto const player = GetCaster()->ToPlayer();
            if (player
                    && player->getLevel() >= 30
                    && player->GetSpecializationId(player->GetActiveSpec()) == SPEC_WARRIOR_FURY)
            {
                player->CastSpell(player, RAGING_BLOW_ENABLER, true);
            }
        }

        void Register() final
        {
            AfterHit += SpellHitFn(script_impl::afterHit);
        }
    };

public:
    spell_warr_enrage_raging_blow()
        : SpellScriptLoader("spell_warr_enrage_raging_blow")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

// Called by Raging Blow - 85288
// Meat Cleaver - 85739
class spell_warr_raging_blow final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            MEAT_CLEAVER_PROC   = 85739,
            RAGING_BLOW_ENABLER = 131116,
        };

        void filterTargets(std::list<WorldObject*> &targetList)
        {
            // Meat Cleaver chain targets are added to main spell, this should
            // not happen
            targetList.clear();
            if (auto const unitTarget = GetExplTargetUnit())
                targetList.push_back(unitTarget);
        }

        void handleAfterHit()
        {
            auto const caster = GetCaster();
            if (!caster)
                return;

            // HasAura check results in double loop to get aura application
            caster->RemoveAura(MEAT_CLEAVER_PROC);

            if (auto const aur = caster->GetAura(RAGING_BLOW_ENABLER))
                aur->ModStackAmount(-1);
        }

        void Register() final
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::filterTargets, EFFECT_0, TARGET_UNIT_TARGET_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::filterTargets, EFFECT_1, TARGET_UNIT_TARGET_ENEMY);
            AfterHit += SpellHitFn(script_impl::handleAfterHit);
        }
    };

public:
    spell_warr_raging_blow()
        : SpellScriptLoader("spell_warr_raging_blow")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

class spell_warr_sword_and_board : public SpellScriptLoader
{
public:
    spell_warr_sword_and_board() : SpellScriptLoader("spell_warr_sword_and_board") { }

    class spell_warr_sword_and_board_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warr_sword_and_board_AuraScript);

        void OnProc(AuraEffect const * /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            Unit * const caster = GetCaster();
            if (!caster)
                return;

            if (Player* const player = caster->ToPlayer())
            {
                player->CastSpell(player, WARRIOR_SPELL_SWORD_AND_BOARD, true);
                player->RemoveSpellCooldown(WARRIOR_SPELL_SHIELD_SLAM, true);
            }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_warr_sword_and_board_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warr_sword_and_board_AuraScript();
    }
};

// Mortal strike - 12294
class spell_warr_mortal_strike final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            GLYPH_OF_MORTAL_STRIKE  = 58368
        };

        void onInitEffects(uint32 &effectMask)
        {
            auto const caster = GetCaster();
            if (!caster || !caster->HasAura(GLYPH_OF_MORTAL_STRIKE))
                effectMask &= ~(1 << EFFECT_4);
        }

        void Register() final
        {
            OnInitEffects += AuraInitEffectsFn(script_impl::onInitEffects);
        }
    };

public:
    spell_warr_mortal_strike()
        : SpellScriptLoader("spell_warr_mortal_strike")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Rallying cry - 97462
class spell_warr_rallying_cry : public SpellScriptLoader
{
    public:
        spell_warr_rallying_cry() : SpellScriptLoader("spell_warr_rallying_cry") { }

        class spell_warr_rallying_cry_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_rallying_cry_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                int32 basePoints0 = int32(GetHitUnit()->CountPctFromMaxHealth(GetEffectValue()));
                GetCaster()->CastCustomSpell(GetHitUnit(), WARRIOR_SPELL_RALLYING_CRY, &basePoints0, NULL, NULL, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warr_rallying_cry_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_rallying_cry_SpellScript();
        }
};

// Heroic leap - 6544
class spell_warr_heroic_leap : public SpellScriptLoader
{
    public:
        spell_warr_heroic_leap() : SpellScriptLoader("spell_warr_heroic_leap") { }

        class spell_warr_heroic_leap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_heroic_leap_SpellScript);

            std::list<Unit*> targetList;

            SpellCastResult CheckElevation()
            {
                Unit* caster = GetCaster();

                WorldLocation* dest = const_cast<WorldLocation*>(GetExplTargetDest());
                if (!dest)
                    return SPELL_FAILED_DONT_REPORT;

                if (dest->GetPositionZ() > caster->GetPositionZ() + 5.0f)
                    return SPELL_FAILED_NOPATH;
                else if (caster->HasAuraType(SPELL_AURA_MOD_ROOT))
                    return SPELL_FAILED_ROOTED;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_warr_heroic_leap_SpellScript::CheckElevation);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_heroic_leap_SpellScript();
        }

        class spell_warr_heroic_leap_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warr_heroic_leap_AuraScript);

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster, WARRIOR_SPELL_HEROIC_LEAP_DAMAGE, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_warr_heroic_leap_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warr_heroic_leap_AuraScript();
        }
};

// Heroic Leap (damage) - 52174
class spell_warr_heroic_leap_damage : public SpellScriptLoader
{
    public:
        spell_warr_heroic_leap_damage() : SpellScriptLoader("spell_warr_heroic_leap_damage") { }

        class spell_warr_heroic_leap_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_heroic_leap_damage_SpellScript);

            void HandleOnHit()
            {
                if (auto caster = GetCaster())
                {
                    int32 damage = int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f);
                    // Seasoned Soldier
                    if (caster->GetTypeId() == TYPEID_PLAYER && caster->HasAura(12712) && caster->ToPlayer()->IsTwoHandUsed())
                        AddPct(damage, 25);

                    SetHitDamage(damage);
                }
            }
            
            void HandleOnCast()
            {
                // Item - Warrior PvP Set 4P Bonus
                if (GetCaster()->HasAura(WARRIOR_SPELL_ITEM_PVP_SET_4P_BONUS))
                    GetCaster()->CastSpell(GetCaster(), WARRIOR_SPELL_HEROIC_LEAP_SPEED, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_heroic_leap_damage_SpellScript::HandleOnHit);
                OnCast += SpellCastFn(spell_warr_heroic_leap_damage_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_heroic_leap_damage_SpellScript();
        }
};

// Shockwave - 46968
class spell_warr_shockwave : public SpellScriptLoader
{
    public:
        spell_warr_shockwave() : SpellScriptLoader("spell_warr_shockwave") { }

        class spell_warr_shockwave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_shockwave_SpellScript);

            void CountTargets(std::list<WorldObject*>& targetList)
            {
                _targetCount = targetList.size();
            }

            void HandleReduce()
            {
                if (_targetCount >= 3)
                    if (GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER)
                        GetCaster()->ToPlayer()->ReduceSpellCooldown(GetSpellInfo()->Id, GetSpellInfo()->Effects[EFFECT_3].BasePoints * IN_MILLISECONDS);
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* const _player = GetCaster()->ToPlayer())
                    if (Unit* const target = GetHitUnit())
                        _player->CastSpell(target, WARRIOR_SPELL_SHOCKWAVE_STUN, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warr_shockwave_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
                AfterCast += SpellCastFn(spell_warr_shockwave_SpellScript::HandleReduce);
                OnEffectHitTarget += SpellEffectFn(spell_warr_shockwave_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }

        private:
            uint32 _targetCount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_shockwave_SpellScript();
        }
};

// Bloodthirst - 23881
class spell_warr_bloodthirst : public SpellScriptLoader
{
    public:
        spell_warr_bloodthirst() : SpellScriptLoader("spell_warr_bloodthirst") { }

        class spell_warr_bloodthirst_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_bloodthirst_SpellScript);

            bool Validate(SpellInfo const* /*SpellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(WARRIOR_SPELL_BLOODTHIRST))
                    return false;
                return true;
            }
            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (GetHitUnit() && GetHitDamage())
                        player->CastSpell(player, WARRIOR_SPELL_BLOODTHIRST_HEAL, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_bloodthirst_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_bloodthirst_SpellScript();
        }
};

// Victory Rush - 34428
class spell_warr_victory_rush : public SpellScriptLoader
{
    public:
        spell_warr_victory_rush() : SpellScriptLoader("spell_warr_victory_rush") { }

        class spell_warr_victory_rush_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_victory_rush_SpellScript);

            bool Validate(SpellInfo const* /*SpellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(WARRIOR_SPELL_VICTORY_RUSH_DAMAGE))
                    return false;
                return true;
            }
            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        player->CastSpell(player, WARRIOR_SPELL_VICTORY_RUSH_HEAL, true);
                        player->RemoveAura(WARRIOR_SPELL_VICTORIOUS_STATE);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_victory_rush_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_victory_rush_SpellScript();
        }
};

class spell_warr_last_stand : public SpellScriptLoader
{
    public:
        spell_warr_last_stand() : SpellScriptLoader("spell_warr_last_stand") { }

        class spell_warr_last_stand_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_last_stand_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(WARRIOR_SPELL_LAST_STAND_TRIGGERED))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 healthModSpellBasePoints0 = int32(caster->CountPctFromMaxHealth(30));
                    caster->CastCustomSpell(caster, WARRIOR_SPELL_LAST_STAND_TRIGGERED, &healthModSpellBasePoints0, NULL, NULL, true, NULL);
                }
            }

            void Register()
            {
                // add dummy effect spell handler to Last Stand
                OnEffectHit += SpellEffectFn(spell_warr_last_stand_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_last_stand_SpellScript();
        }
};

// Thunder Clap - 6343
class spell_warr_thunder_clap : public SpellScriptLoader
{
    public:
        spell_warr_thunder_clap() : SpellScriptLoader("spell_warr_thunder_clap") { }

        class spell_warr_thunder_clap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warr_thunder_clap_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, WARRIOR_SPELL_WEAKENED_BLOWS, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warr_thunder_clap_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warr_thunder_clap_SpellScript();
        }
};

class spell_warr_deep_wounds : public SpellScriptLoader
{
public:
    spell_warr_deep_wounds() : SpellScriptLoader("spell_warr_deep_wounds") { }

    class spell_warr_deep_wounds_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warr_deep_wounds_AuraScript);

        void OnProc(AuraEffect const * /*aurEff*/, ProcEventInfo& eventInfo)
        {
            Unit* const caster = GetCaster();
            Unit* const target = eventInfo.GetProcTarget();
            if (!caster || !target || target == caster)
                return;

            caster->CastSpell(target, WARRIOR_SPELL_DEEP_WOUNDS, true);
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_warr_deep_wounds_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warr_deep_wounds_AuraScript();
    }
};

// Charge - 100
class spell_warr_charge : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            CHARGE_STUN        = 7922,
            DOUBLE_TIME        = 103827,
            WARBRINGER         = 103828,
            WARBRINGER_STUN    = 105771,
            WARBRINGER_SLOW    = 137637,
            DOUBLE_TIME_MARKER = 124184,
        };

        bool canGenerateCharge;

        bool Load()
        {
            Unit * const caster = GetCaster();
            if (!caster)
                return false;

            canGenerateCharge = !caster->HasAura(DOUBLE_TIME) || !caster->HasAura(DOUBLE_TIME_MARKER);
            return true;
        }

        void HandleCharge(SpellEffIndex)
        {
            Unit * const target = GetHitUnit();
            if (!target)
                return;

            Unit * const caster = GetCaster();
            if (!caster)
                return;

            uint32 const stunSpellId = caster->HasAura(WARBRINGER) ? WARBRINGER_STUN : CHARGE_STUN;
            if (caster->HasAura(WARBRINGER))
                caster->CastSpell(target, WARBRINGER_SLOW, true);

            caster->CastSpell(target, stunSpellId, true);
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Unit * const caster = GetCaster();
            if (canGenerateCharge && caster)
                caster->EnergizeBySpell(caster, GetSpellInfo()->Id, GetEffectValue(), POWER_RAGE);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleCharge, EFFECT_0, SPELL_EFFECT_CHARGE);
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

public:
    spell_warr_charge()
        : SpellScriptLoader("spell_warr_charge")
    { }

    SpellScript * GetSpellScript() const
    {
        return new script_impl;
    }
};

class spell_warr_taste_for_blood_talent final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            MORTAL_STRIKE           = 12294,
            TASTE_FOR_BLOOD_EFFECT  = 60503
        };

        int32 stacksToAdd_;

        bool checkProc(ProcEventInfo &eventInfo)
        {
            auto const caster = eventInfo.GetActor();
            auto const target = eventInfo.GetActionTarget();

            if (!caster || !target || caster == target)
                return false;

            if (eventInfo.GetHitMask() & PROC_EX_DODGE)
            {
                stacksToAdd_ = GetSpellInfo()->Effects[EFFECT_0].BasePoints;
                return true;
            }

            auto const spellInfo = eventInfo.GetSpellInfo();
            if (spellInfo && spellInfo->Id == MORTAL_STRIKE)
            {
                stacksToAdd_ = GetSpellInfo()->Effects[EFFECT_1].BasePoints;
                return true;
            }

            return false;
        }

        void onProc(AuraEffect const *, ProcEventInfo &eventInfo)
        {
            PreventDefaultAction();

            CustomSpellValues values;
            values.AddSpellMod(SPELLVALUE_AURA_STACK, stacksToAdd_);

            auto const caster = eventInfo.GetActor();
            caster->CastCustomSpell(TASTE_FOR_BLOOD_EFFECT, values, caster, true);
        }

        void Register() final
        {
            DoCheckProc += AuraCheckProcFn(script_impl::checkProc);
            OnEffectProc += AuraEffectProcFn(script_impl::onProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_warr_taste_for_blood_talent()
        : SpellScriptLoader("spell_warr_taste_for_blood_talent")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

class spell_warr_taste_for_blood_effect final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        void prepareProc(ProcEventInfo &)
        {
            PreventDefaultAction();
        }

        void afterProc(ProcEventInfo &)
        {
            GetAura()->ModStackAmount(-1);
        }

        void Register() final
        {
            DoPrepareProc += AuraProcFn(script_impl::prepareProc);
            AfterProc += AuraProcFn(script_impl::afterProc);
        }
    };

public:
    spell_warr_taste_for_blood_effect()
        : SpellScriptLoader("spell_warr_taste_for_blood_effect")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// 12292 - Bloodbath
class spell_warr_bloodbath final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            BLOODBATH_BLEED = 113344
        };

        bool checkProc(ProcEventInfo &eventInfo)
        {
            return eventInfo.GetSpellInfo()
                    && eventInfo.GetDamageInfo()->GetDamage() != 0;
        }

        void onProc(AuraEffect const *aurEff, ProcEventInfo &eventInfo)
        {
            PreventDefaultAction();

            auto const caster = eventInfo.GetActor();
            auto const target = eventInfo.GetActionTarget();

            if (!caster || !target || caster == target)
                return;

            auto const damage = eventInfo.GetDamageInfo()->GetDamage();
            auto const remaining = target->GetRemainingPeriodicAmount(caster->GetGUID(), BLOODBATH_BLEED, SPELL_AURA_PERIODIC_DAMAGE);

            int32 const bp = CalculatePct(damage, aurEff->GetAmount()) + remaining.total();
            caster->CastCustomSpell(target, BLOODBATH_BLEED, &bp, nullptr, nullptr, true);
        }

        void Register() final
        {
            DoCheckProc += AuraCheckProcFn(script_impl::checkProc);
            OnEffectProc += AuraEffectProcFn(script_impl::onProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_warr_bloodbath()
        : SpellScriptLoader("spell_warr_bloodbath")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

class spell_warr_bloodbath_bleed final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        void calculateAmount(AuraEffect const *aurEff, int32 &amount, bool &canBeRecalculated)
        {
            canBeRecalculated = false;
            amount /= aurEff->GetTotalTicks();
        }

        void Register() final
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::calculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

public:
    spell_warr_bloodbath_bleed()
        : SpellScriptLoader("spell_warr_bloodbath_bleed")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// 145674 - Riposte
class spell_warr_riposte final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        void calculateAmount(AuraEffect const *, int32 &amount, bool &canBeRecalculated)
        {
            canBeRecalculated = false;

            Unit * const caster = GetCaster();
            if (!caster)
                return;

            if (Player * player = caster->ToPlayer())
            {
                uint32 rating = player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_PARRY);
                rating += player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_DODGE);
                amount = CalculatePct(rating, 75);
            }
        }

        void Register() final
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::calculateAmount, EFFECT_0, SPELL_AURA_MOD_RATING);
        }
    };

public:
    spell_warr_riposte()
        : SpellScriptLoader("spell_warr_riposte")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Called by Revenge and Shield Slam - preventing rage gain bonus if not in Defensive Stance
class spell_warr_revenge_shield_slam : public SpellScriptLoader
{
public:
    spell_warr_revenge_shield_slam() : SpellScriptLoader("spell_warr_revenge_shield_slam") { }

    class spell_warr_revenge_shield_slam_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warr_revenge_shield_slam_SpellScript);

        void HandleCast(SpellEffIndex effIndex)
        {
            if (!GetCaster()->HasAura(WARRIOR_SPELL_DEFENSIVE_STANCE))
                PreventHitEffect(effIndex);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_warr_revenge_shield_slam_SpellScript::HandleCast, EFFECT_1, SPELL_EFFECT_ENERGIZE);
            OnEffectHitTarget += SpellEffectFn(spell_warr_revenge_shield_slam_SpellScript::HandleCast, EFFECT_2, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_warr_revenge_shield_slam_SpellScript();
    }
};

class spell_warr_glyph_of_die_by_the_sword : public SpellScriptLoader
{
public:
    spell_warr_glyph_of_die_by_the_sword() : SpellScriptLoader("spell_warr_glyph_of_die_by_the_sword") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void OnProc(AuraEffect const * /*aurEff*/, ProcEventInfo& eventInfo)
        {
            Unit * const caster = GetCaster();
            if (!caster || !eventInfo.GetSpellInfo())
                return;

            Aura * const aura = caster->GetAura(118038);
            if (!aura)
                return;

            // Overpower
            if (eventInfo.GetSpellInfo()->Id == 7384)
                aura->SetDuration(aura->GetDuration() + 1000);
            else // Wild Strike
                aura->SetDuration(aura->GetDuration() + 500);
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

// 7384 - Overpower
class spell_warr_overpower : public SpellScriptLoader
{
public:
    spell_warr_overpower() : SpellScriptLoader("spell_warr_overpower") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            MORTAL_STRIKE           = 12294
        };

        void HandleCast(SpellEffIndex effIndex)
        {
            Player * player = GetCaster()->ToPlayer();
            if (!player)
                return;

            // Reduce Mortal Strike cooldown by 0.5 sec
            if (player->HasSpellCooldown(MORTAL_STRIKE))
                player->ReduceSpellCooldown(MORTAL_STRIKE, 500);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleCast, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};
// 29725 - Sudden Death
class spell_warr_sudden_death : public SpellScriptLoader
{
    enum
    {
        SPELL_EXECUTE            = 5308,
        SPELL_SUDDEN_DEATH_PROC  = 139958,
    };

public:
    spell_warr_sudden_death() : SpellScriptLoader("spell_warr_sudden_death") { }

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void OnProc(AuraEffect const * eff, ProcEventInfo &eventInfo)
        {
            PreventDefaultAction();
            auto caster = GetCaster();


            if (!caster)
                return;

            auto procSpellInfo = eventInfo.GetSpellInfo();

            if (roll_chance_i(10) && (!procSpellInfo || procSpellInfo->Id == 76858))
            {
                caster->CastSpell(caster, 52437, true); // Reset Cooldown of Colossus Smash
                if (caster->GetTypeId() == TYPEID_PLAYER)
                    caster->ToPlayer()->RemoveSpellCooldown(WARRIOR_SPELL_COLOSSUS_SMASH, true);
            }

            if (procSpellInfo && procSpellInfo->Id == SPELL_EXECUTE)
                caster->CastSpell(caster, SPELL_SUDDEN_DEATH_PROC, true);
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(aura_impl::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

// 12328, 18765, 35429 - Sweeping Strikes
class spell_warr_sweeping_strikes : public SpellScriptLoader
{
    enum
    {
        SPELL_SWEEPING_STRIKES_EXTRA_ATTACK     = 12723,
        SPELL_GLYPH_OF_SWEEPING_STRIKES         = 58384,
        SPELL_GLYPH_OF_SWEEPING_STRIKES_PROC    = 124333,
        SPELL_SLAM                              = 1464,
        SPELL_SLAM_PROC                         = 146361,
    };

public:
    spell_warr_sweeping_strikes() : SpellScriptLoader("spell_warr_sweeping_strikes") { }

    class spell_warr_sweeping_strikes_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warr_sweeping_strikes_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_SWEEPING_STRIKES_EXTRA_ATTACK) || !sSpellMgr->GetSpellInfo(SPELL_GLYPH_OF_SWEEPING_STRIKES_PROC))
                return false;
            return true;
        }

        bool Load()
        {
            _procTarget = NULL;
            return true;
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            _procTarget = eventInfo.GetActor()->SelectNearbyTarget(eventInfo.GetProcTarget());
            return _procTarget;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            auto caster = GetTarget();
            PreventDefaultAction();

            bool bladeStormProc = false;
            if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == 50622 && caster->GetTypeId() == TYPEID_PLAYER)
            {
                if (caster->ToPlayer()->HasSpellCooldown(SPELL_SWEEPING_STRIKES_EXTRA_ATTACK))
                    return;

                bladeStormProc = true;
            }

            // Glyph of Sweeping Strikes
            if (caster->HasAura(SPELL_GLYPH_OF_SWEEPING_STRIKES))
                caster->CastSpell(caster, SPELL_GLYPH_OF_SWEEPING_STRIKES_PROC, true);

            int32 bp = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount());
            caster->CastCustomSpell(_procTarget, SPELL_SWEEPING_STRIKES_EXTRA_ATTACK, &bp, NULL, NULL, true, NULL, aurEff);

            // Bladestorm must tick once per rotation
            if (bladeStormProc)
                if (auto player = caster->ToPlayer())
                    player->AddSpellCooldown(SPELL_SWEEPING_STRIKES_EXTRA_ATTACK, 0, 500);

            // Slam bonus
            if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_SLAM)
            {
                int32 slamBp = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), 35);
                caster->CastCustomSpell(_procTarget, SPELL_SLAM_PROC, &slamBp, NULL, NULL, true);
            }
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_warr_sweeping_strikes_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_warr_sweeping_strikes_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }

    private:
        Unit* _procTarget;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warr_sweeping_strikes_AuraScript();
    }
};

class spell_warr_unshackled_fury : public SpellScriptLoader
{
public:
    spell_warr_unshackled_fury() : SpellScriptLoader("spell_warr_unshackled_fury") { }

    class spell_warr_unshackled_fury_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warr_unshackled_fury_AuraScript);

        void CalculateAmount(const AuraEffect* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
        {
            if (Unit* caster = GetCaster())
                if (!caster->HasAuraState(AURA_STATE_ENRAGE))
                    amount = 0;
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warr_unshackled_fury_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warr_unshackled_fury_AuraScript();
    }
};

class spell_warr_glyph_of_burning_anger : public SpellScriptLoader
{
public: 
    spell_warr_glyph_of_burning_anger() : SpellScriptLoader("spell_warr_glyph_of_burning_anger") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            SPELL_WARRIOR_GLYPH_OF_BURNING_ANGER = 115946,
            SPELL_WARRIOR_BURNING_ANGER_VISUAL = 115993
        };

        void HandleBurning(SpellEffIndex /*effIndex*/)
        {
            if (auto caster = GetCaster())
                if (caster->HasAura(SPELL_WARRIOR_GLYPH_OF_BURNING_ANGER))
                    caster->CastSpell(GetHitUnit(), SPELL_WARRIOR_BURNING_ANGER_VISUAL, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleBurning, EFFECT_0, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};


void AddSC_warrior_spell_scripts()
{
    new spell_warr_victorious_state();
    new spell_warr_glyph_of_hindering_strikes();
    new spell_warr_stampeding_shout();
    new spell_warr_shield_barrier();
    new spell_warr_shield_block();
    new spell_warr_storm_bolt();
    new spell_warr_colossus_smash();
    new spell_warr_dragon_roar();
    new spell_warr_staggering_shout();
    new spell_warr_second_wind();
    new spell_warr_unbridled_wrath();
    new spell_warr_berserker_rage();
    new spell_warr_mocking_banner();
    new spell_warr_enrage_raging_blow();
    new spell_warr_raging_blow();
    new spell_warr_sword_and_board();
    new spell_warr_mortal_strike();
    new spell_warr_rallying_cry();
    new spell_warr_heroic_leap_damage();
    new spell_warr_heroic_leap();
    new spell_warr_shockwave();
    new spell_warr_bloodthirst();
    new spell_warr_victory_rush();
    new spell_warr_last_stand();
    new spell_warr_thunder_clap();
    new spell_warr_deep_wounds();
    new spell_warr_charge();
    new spell_warr_taste_for_blood_talent();
    new spell_warr_taste_for_blood_effect();
    new spell_warr_bloodbath();
    new spell_warr_bloodbath_bleed();
    new spell_warr_riposte();
    new spell_warr_revenge_shield_slam();
    new spell_warr_glyph_of_die_by_the_sword();
    new spell_warr_overpower();
    new spell_warr_sudden_death();
    new spell_warr_sweeping_strikes();
    new spell_warr_unshackled_fury();
    new spell_warr_storm_bolt_damage();
    new spell_warr_second_wind_aura();
    new spell_warr_glyph_of_burning_anger();
}
