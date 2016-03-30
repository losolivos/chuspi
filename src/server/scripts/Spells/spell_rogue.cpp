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
 * Scripts for spells with SPELLFAMILY_ROGUE and SPELLFAMILY_GENERIC spells used by rogue players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_rog_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "ArraySize.h"
#include "ObjectVisitors.hpp"

enum RogueSpells
{
    ROGUE_SPELL_SHIV_TRIGGERED                   = 5940,
    ROGUE_SPELL_DEADLY_POISON                    = 2823,
    ROGUE_SPELL_WOUND_POISON                     = 8679,
    ROGUE_SPELL_MIND_NUMBLING_POISON             = 5761,
    ROGUE_SPELL_CRIPPLING_POISON                 = 3408,
    ROGUE_SPELL_CRIPPLING_POISON_DEBUFF          = 3409,
    ROGUE_SPELL_LEECHING_POISON                  = 108211,
    ROGUE_SPELL_LEECHING_POISON_DEBUFF           = 112961,
    ROGUE_SPELL_PARALYTIC_POISON                 = 108215,
    ROGUE_SPELL_PARALYTIC_POISON_DEBUFF          = 113952,
    ROGUE_SPELL_DEBILITATING_POISON              = 115196,
    ROGUE_SPELL_MIND_PARALYSIS                   = 115194,
    ROGUE_SPELL_LEECH_VITALITY                   = 116921,
    ROGUE_SPELL_PARTIAL_PARALYSIS                = 115197,
    ROGUE_SPELL_TOTAL_PARALYSIS                  = 113953,
    ROGUE_SPELL_DEADLY_POISON_DOT                = 2818,
    ROGUE_SPELL_DEADLY_POISON_INSTANT_DAMAGE     = 113780,
    ROGUE_SPELL_SLICE_AND_DICE                   = 5171,
    ROGUE_SPELL_MASTER_POISONER_AURA             = 58410,
    ROGUE_SPELL_MASTER_POISONER_DEBUFF           = 93068,
    ROGUE_SPELL_CRIMSON_TEMPEST_DOT              = 122233,
    ROGUE_SPELL_SHROUD_OF_CONCEALMENT_AURA       = 115834,
    ROGUE_SPELL_VENOMOUS_VIM_ENERGIZE            = 51637,
    ROGUE_SPELL_VENOMOUS_WOUND_DAMAGE            = 79136,
    ROGUE_SPELL_GARROTE_DOT                      = 703,
    ROGUE_SPELL_RUPTURE_DOT                      = 1943,
    ROGUE_SPELL_CUT_TO_THE_CHASE_AURA            = 51667,
    ROGUE_SPELL_ADRENALINE_RUSH                  = 13750,
    ROGUE_SPELL_KILLING_SPREE                    = 51690,
    ROGUE_SPELL_REDIRECT                         = 73981,
    ROGUE_SPELL_SPRINT                           = 2983,
    ROGUE_SPELL_HEMORRHAGE_DOT                   = 89775,
    ROGUE_SPELL_SANGUINARY_VEIN_DEBUFF           = 124271,
    ROGUE_SPELL_NIGHTSTALKER_AURA                = 14062,
    ROGUE_SPELL_NIGHTSTALKER_DAMAGE_DONE         = 130493,
    ROGUE_SPELL_SHADOW_FOCUS_AURA                = 108209,
    ROGUE_SPELL_SHADOW_FOCUS_COST_PCT            = 112942,
    ROGUE_SPELL_NERVE_STRIKE_AURA                = 108210,
    ROGUE_SPELL_NERVE_STRIKE_REDUCE_DAMAGE_DONE  = 112947,
    ROGUE_SPELL_COMBAT_READINESS                 = 74001,
    ROGUE_SPELL_COMBAT_INSIGHT                   = 74002,
    ROGUE_SPELL_BLADE_FLURRY                     = 13877,
    ROGUE_SPELL_BLADE_FLURRY_DAMAGE              = 22482,
    ROGUE_SPELL_CHEAT_DEATH_REDUCE_DAMAGE        = 45182,
    ROGUE_SPELL_ENERGETIC_RECOVERY_AURA          = 79152,
    ROGUE_SPELL_GLYPH_OF_EXPOSE_ARMOR            = 56803,
    ROGUE_SPELL_WEAKENED_ARMOR                   = 113746,
    ROGUE_SPELL_DEADLY_BREW                      = 51626,
    ROGUE_SPELL_GLYPH_OF_HEMORRHAGE              = 56807,
};

// Called by Expose Armor - 8647
// Glyph of Expose Armor - 56803
class spell_rog_glyph_of_expose_armor : public SpellScriptLoader
{
    public:
        spell_rog_glyph_of_expose_armor() : SpellScriptLoader("spell_rog_glyph_of_expose_armor") { }

        class spell_rog_glyph_of_expose_armor_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_glyph_of_expose_armor_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(ROGUE_SPELL_GLYPH_OF_EXPOSE_ARMOR))
                        {
                            _player->CastSpell(target, ROGUE_SPELL_WEAKENED_ARMOR, true);
                            _player->CastSpell(target, ROGUE_SPELL_WEAKENED_ARMOR, true);
                            _player->CastSpell(target, ROGUE_SPELL_WEAKENED_ARMOR, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_glyph_of_expose_armor_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_glyph_of_expose_armor_SpellScript();
        }
};

// Cheat Death - 31230
class spell_rog_cheat_death : public SpellScriptLoader
{
    public:
        spell_rog_cheat_death() : SpellScriptLoader("spell_rog_cheat_death") { }

        class spell_rog_cheat_death_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_cheat_death_AuraScript);

            void CalculateAmount(AuraEffect const * /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect * /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                if (Unit* target = GetTarget())
                {
                    if (dmgInfo.GetDamage() < target->GetHealth())
                        return;

                    if (target->ToPlayer()->HasSpellCooldown(ROGUE_SPELL_CHEAT_DEATH_REDUCE_DAMAGE))
                        return;

                    target->CastSpell(target, ROGUE_SPELL_CHEAT_DEATH_REDUCE_DAMAGE, true);
                    target->ToPlayer()->AddSpellCooldown(ROGUE_SPELL_CHEAT_DEATH_REDUCE_DAMAGE, 0, 90 * IN_MILLISECONDS);

                    uint32 health10 = target->CountPctFromMaxHealth(10);

                    // hp > 10% - absorb hp till 10%
                    if (target->GetHealth() > health10)
                        absorbAmount = dmgInfo.GetDamage() - target->GetHealth() + health10;
                    // hp lower than 10% - absorb everything
                    else
                        absorbAmount = dmgInfo.GetDamage();
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_rog_cheat_death_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_rog_cheat_death_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_cheat_death_AuraScript();
        }
};

// Blade Flurry - 13877
class spell_rog_blade_flurry : public SpellScriptLoader
{
    public:
        spell_rog_blade_flurry() : SpellScriptLoader("spell_rog_blade_flurry") { }

        class spell_rog_blade_flurry_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_blade_flurry_AuraScript);

            void OnProc(AuraEffect const * /*aurEff*/, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Unit * const caster = GetCaster();
                if (!caster)
                    return;

                if (eventInfo.GetActor()->GetGUID() != GetTarget()->GetGUID())
                    return;

                if (Player* const _player = caster->ToPlayer())
                {
                    int32 damage = eventInfo.GetDamageInfo()->GetDamage();
                    SpellInfo const* spellInfo = eventInfo.GetDamageInfo()->GetSpellInfo();

                    if (!damage || eventInfo.GetDamageInfo()->GetDamageType() == DOT)
                        return;

                    if (spellInfo && !spellInfo->CanTriggerBladeFlurry())
                        return;

                    damage = CalculatePct(damage, 40);

                    _player->CastCustomSpell(eventInfo.GetActionTarget(), ROGUE_SPELL_BLADE_FLURRY_DAMAGE, &damage, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_rog_blade_flurry_AuraScript::OnProc, EFFECT_0, SPELL_AURA_MOD_POWER_REGEN_PERCENT);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_blade_flurry_AuraScript();
        }
};

// Blade Flurry damage - 22482
class spell_rog_blade_flurry_damage : public SpellScriptLoader
{
public:
    spell_rog_blade_flurry_damage() : SpellScriptLoader("spell_rog_blade_flurry_damage") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        void RemoveInvalidTargets(std::list<WorldObject*>& targets)
        {
            auto caster = GetCaster();
            if (!caster)
                return;
            // Clear targets, we make the new list
            targets.clear();
            Trinity::AllWorldObjectsInRange objects(caster, 5.0f);
            Trinity::WorldObjectListSearcher<Trinity::AllWorldObjectsInRange> searcher(caster, targets, objects);
            Trinity::VisitNearbyObject(caster, 5.0f, searcher);

            targets.remove(GetExplTargetUnit());

            targets.remove_if([caster](WorldObject * obj)
            {
                if (auto unit = obj->ToUnit())
                {
                    return unit->isTotem() || !caster->IsValidAttackTarget(unit);
                }
                else
                    return true;
            });

            if (targets.size() > 4)
                targets.resize(4);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Growl - 113613
class spell_rog_growl : public SpellScriptLoader
{
    public:
        spell_rog_growl() : SpellScriptLoader("spell_rog_growl") { }

        class spell_rog_growl_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_growl_SpellScript);

            void HandleOnHit()
            {
                if (!GetCaster())
                    return;

                if (!GetHitUnit())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->IsValidAttackTarget(target))
                            _player->CastSpell(target, 355, true); // Taunt
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_growl_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_growl_SpellScript();
        }
};

// Cloak of Shadows - 31224 and Cloak of Shadows - 110788 (Symbiosis)
class spell_rog_cloak_of_shadows : public SpellScriptLoader
{
    public:
        spell_rog_cloak_of_shadows() : SpellScriptLoader("spell_rog_cloak_of_shadows") { }

        class spell_rog_cloak_of_shadows_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_cloak_of_shadows_SpellScript);

            void HandleOnHit()
            {
                if (!GetCaster())
                    return;
                const SpellInfo* m_spellInfo = GetSpellInfo();

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    uint32 dispelMask = SpellInfo::GetDispelMask(DISPEL_ALL);
                    Unit::AuraApplicationMap& Auras = _player->GetAppliedAuras();
                    for (Unit::AuraApplicationMap::iterator iter = Auras.begin(); iter != Auras.end();)
                    {
                        // remove all harmful spells on you...
                        SpellInfo const* spell = iter->second->GetBase()->GetSpellInfo();
                        if ((!(spell->SchoolMask & SPELL_SCHOOL_MASK_NORMAL) || ((spell->GetDispelMask()) & dispelMask))
                            // ignore positive and passive auras
                            && !iter->second->IsPositive() && !iter->second->GetBase()->IsPassive())
                        {
                            _player->RemoveAura(iter);
                        }
                        else
                            ++iter;
                    }
                    return;
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_cloak_of_shadows_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_cloak_of_shadows_SpellScript();
        }
};

// Combat Readiness - 74001
class spell_rog_combat_readiness : public SpellScriptLoader
{
    public:
        spell_rog_combat_readiness() : SpellScriptLoader("spell_rog_combat_readiness") { }

        class spell_rog_combat_readiness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_combat_readiness_AuraScript);

            uint32 update;
            bool hit;

            void HandleApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                {
                    update = 10000;
                    hit = false;
                }
            }

            void OnUpdate(uint32 diff)
            {
                update -= diff;

                if (GetCaster())
                    if (GetCaster()->HasAura(ROGUE_SPELL_COMBAT_INSIGHT))
                        hit = true;

                if (update <= 0)
                    if (Player* _player = GetCaster()->ToPlayer())
                        if (!hit)
                            _player->RemoveAura(ROGUE_SPELL_COMBAT_READINESS);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_rog_combat_readiness_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnAuraUpdate += AuraUpdateFn(spell_rog_combat_readiness_AuraScript::OnUpdate);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_combat_readiness_AuraScript();
        }
};

// Called by Kidney Shot - 408 and Cheap Shot - 1833
// Nerve Strike - 108210
class spell_rog_nerve_strike : public SpellScriptLoader
{
    public:
        spell_rog_nerve_strike() : SpellScriptLoader("spell_rog_nerve_strike") { }

        class spell_rog_combat_readiness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_combat_readiness_AuraScript);

            void HandleRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster() && GetTarget())
                    if (GetCaster()->HasAura(ROGUE_SPELL_NERVE_STRIKE_AURA))
                        GetCaster()->CastSpell(GetTarget(), ROGUE_SPELL_NERVE_STRIKE_REDUCE_DAMAGE_DONE, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_rog_combat_readiness_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_combat_readiness_AuraScript();
        }
};

// Called by Stealth - 1784
// Nightstalker - 14062
class spell_rog_nightstalker : public SpellScriptLoader
{
    public:
        spell_rog_nightstalker() : SpellScriptLoader("spell_rog_nightstalker") { }

        class spell_rog_nightstalker_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_nightstalker_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(ROGUE_SPELL_NIGHTSTALKER_AURA))
                        _player->CastSpell(_player, ROGUE_SPELL_NIGHTSTALKER_DAMAGE_DONE, true);

                    if (_player->HasAura(ROGUE_SPELL_SHADOW_FOCUS_AURA))
                        _player->CastSpell(_player, ROGUE_SPELL_SHADOW_FOCUS_COST_PCT, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_nightstalker_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_nightstalker_SpellScript();
        }

        class spell_rog_nightstalker_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_nightstalker_AuraScript);

            void HandleRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit * const caster = GetCaster())
                {
                    if (Aura * const nightstalker = caster->GetAura(ROGUE_SPELL_NIGHTSTALKER_DAMAGE_DONE))
                        nightstalker->SetDuration(100); // Set duration instead of remove as stealth removal is done before spell-cast damage calculation

                    if (caster->HasAura(ROGUE_SPELL_SHADOW_FOCUS_COST_PCT))
                        caster->RemoveAura(ROGUE_SPELL_SHADOW_FOCUS_COST_PCT);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_rog_nightstalker_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_nightstalker_AuraScript();
        }
};

// Called by Rupture - 1943, Garrote - 703 and Crimson Tempest - 122233 and Hemorrhage - 89775 (required for glyph handling)
// Sanguinary Vein - 79147
class spell_rog_sanguinary_vein : public SpellScriptLoader
{
    public:
        spell_rog_sanguinary_vein() : SpellScriptLoader("spell_rog_sanguinary_vein") { }

        class spell_rog_sanguinary_vein_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_sanguinary_vein_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                auto caster = GetCaster();
                auto target = GetTarget();
                if (!caster || !target)
                    return;

                // Glyph of Hemorrhaging Veins
                if (GetId() == 89775 && !caster->HasAura(146631))
                    return;

                if (caster->HasAura(79147))
                    caster->CastSpell(target, ROGUE_SPELL_SANGUINARY_VEIN_DEBUFF, true);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (auto target = GetTarget())
                {
                    bool hasFound = false;
                    uint32 spellsAffected[3] = {1943, 703, 121411};
                    for (uint32 i = 0; i < 3; ++i)
                        if (target->HasAura(spellsAffected[i], GetCasterGUID()))
                            hasFound = true;

                    // Glyph of Hemorrhaging Veins
                    if (GetCaster() && GetCaster()->HasAura(146631) && target->HasAura(89775, GetCasterGUID()))
                        hasFound = true;

                    if (!hasFound)
                        target->RemoveAurasDueToSpell(ROGUE_SPELL_SANGUINARY_VEIN_DEBUFF, GetCasterGUID());
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_rog_sanguinary_vein_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_rog_sanguinary_vein_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_sanguinary_vein_AuraScript();
        }
};

// Hemorrhage - 16511
class spell_rog_hemorrhage : public SpellScriptLoader
{
    public:
        spell_rog_hemorrhage() : SpellScriptLoader("spell_rog_hemorrhage") { }

        class spell_rog_hemorrhage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_hemorrhage_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(ROGUE_SPELL_GLYPH_OF_HEMORRHAGE))
                        {
                            if (!target->HasAuraState(AURA_STATE_BLEEDING))
                                return;
                        }

                        int32 bp = int32(GetHitDamage() / 2 / 8);

                        _player->CastCustomSpell(target, ROGUE_SPELL_HEMORRHAGE_DOT, &bp, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_hemorrhage_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_hemorrhage_SpellScript();
        }
};

// 79096 - Restless Blades
class spell_rog_restless_blades : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetSpellInfo() && !eventInfo.GetSpellInfo()->NeedsComboPoints())
                return false;

            return GetUnitOwner()->GetTypeId() == TYPEID_PLAYER;
        }

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            Player* player = GetUnitOwner()->ToPlayer();
            int32 const cooldownReduction = aurEff->GetAmount() * player->GetComboPoints();

            uint32 const affectedSpells[] = { 13750, 51690, 73981, 121471, 2983 };

            for (size_t i = 0; i < TC_ARRAY_SIZE(affectedSpells); ++i)
                player->ReduceSpellCooldown(affectedSpells[i], cooldownReduction);
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(script_impl::CheckProc);
            OnEffectProc += AuraEffectProcFn(script_impl::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_rog_restless_blades()
        : SpellScriptLoader("spell_rog_restless_blades")
    { }

    AuraScript * GetAuraScript() const
    {
        return new script_impl;
    }
};

// Called by Envenom - 32645 and Eviscerate - 2098
// Cut to the Chase - 51667
class spell_rog_cut_to_the_chase : public SpellScriptLoader
{
    public:
        spell_rog_cut_to_the_chase() : SpellScriptLoader("spell_rog_cut_to_the_chase") { }

        class spell_rog_cut_to_the_chase_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_cut_to_the_chase_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (GetHitUnit())
                        if (player->HasAura(ROGUE_SPELL_CUT_TO_THE_CHASE_AURA))
                            if (Aura * const sliceAndDice = player->GetAura(ROGUE_SPELL_SLICE_AND_DICE, player->GetGUID()))
                            {
                                sliceAndDice->SetMaxDuration(36 * IN_MILLISECONDS);
                                sliceAndDice->SetDuration(36 * IN_MILLISECONDS);
                            }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_cut_to_the_chase_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_cut_to_the_chase_SpellScript();
        }
};

// Called by Garrote - 703 and Rupture - 1943
// Venomous Wounds - 79134
class spell_rog_venomous_wounds : public SpellScriptLoader
{
    public:
        spell_rog_venomous_wounds() : SpellScriptLoader("spell_rog_venomous_wounds") { }

        class spell_rog_venomous_wounds_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_venomous_wounds_AuraScript);

            enum
            {
                SPELL_VENOMOUS_WOUNDS = 79134,
            };

            void HandleEffectPeriodic(AuraEffect const * /*aurEff*/)
            {
                auto caster = GetCaster();
                auto target = GetTarget();
                if (!caster || !target)
                    return;

                if (caster->HasAura(SPELL_VENOMOUS_WOUNDS))
                {
                    // Each time your Rupture or Garrote deals damage to an enemy that you have poisoned ...
                    if (target->HasAura(8680, caster->GetGUID())
                        || target->HasAura(2818, caster->GetGUID())
                        || target->HasAura(5760, caster->GetGUID())
                        || target->HasAura(3409, caster->GetGUID())
                        || target->HasAura(113952, caster->GetGUID())
                        || target->HasAura(112961, caster->GetGUID()))
                    {
                        if (target->GetAura(ROGUE_SPELL_RUPTURE_DOT, caster->GetGUID()))
                            ProcEffect(caster, target);
                        // Garrote will not trigger this effect if the enemy is also afflicted by your Rupture
                        else if (target->GetAura(ROGUE_SPELL_GARROTE_DOT, caster->GetGUID()))
                        {
                            ProcEffect(caster, target);
                        }
                    }
                }
            }

            void ProcEffect(Unit * const caster, Unit * const target)
            {
                // ... you have a 75% chance ...
                if (roll_chance_i(75))
                {
                    // ... to deal [ X + 16% of AP ] additional Nature damage and to regain 10 Energy
                    caster->CastSpell(target, ROGUE_SPELL_VENOMOUS_WOUND_DAMAGE, true);
                    int32 bp = 10;
                    caster->CastCustomSpell(caster, ROGUE_SPELL_VENOMOUS_VIM_ENERGIZE, &bp, NULL, NULL, true);
                }
            }

            void OnRemove(AuraEffect const *aurEff, AuraEffectHandleModes /*mode*/)
            {
                auto caster = GetCaster();
                if (caster && caster->HasAura(SPELL_VENOMOUS_WOUNDS) && GetId() == ROGUE_SPELL_RUPTURE_DOT)
                {
                    if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                    {
                        if (Aura const * const rupture = aurEff->GetBase())
                        {
                            // If an enemy dies while afflicted by your Rupture, you regain energy proportional to the remaining Rupture duration
                            int32 duration = int32(rupture->GetDuration() / 1000);
                            caster->CastCustomSpell(caster, ROGUE_SPELL_VENOMOUS_VIM_ENERGIZE, &duration, NULL, NULL, true);

                        }
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_venomous_wounds_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                OnEffectRemove += AuraEffectRemoveFn(spell_rog_venomous_wounds_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_venomous_wounds_AuraScript();
        }
};

// Redirect - 73981 and Redirect - 110730
class spell_rog_redirect : public SpellScriptLoader
{
    public:
        spell_rog_redirect() : SpellScriptLoader("spell_rog_redirect") { }

        class spell_rog_redirect_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_redirect_SpellScript);

            SpellCastResult CheckCast()
            {
                if (GetCaster())
                {
                    if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                        return SPELL_FAILED_DONT_REPORT;

                    if (!GetCaster()->ToPlayer()->GetComboPoints())
                        return SPELL_FAILED_NO_COMBO_POINTS;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                auto const _player = GetCaster()->ToPlayer();
                auto const target = GetHitUnit();
                if (!_player || !target)
                    return;

                uint8 cp = _player->GetComboPoints();
                if (cp > 5)
                    cp = 5;

                _player->ClearComboPoints();
                _player->AddComboPoints(target, cp, GetSpell());
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_rog_redirect_SpellScript::CheckCast);
                OnHit += SpellHitFn(spell_rog_redirect_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_redirect_SpellScript();
        }
};

// Shroud of Concealment - 115834
class spell_rog_shroud_of_concealment : public SpellScriptLoader
{
    public:
        spell_rog_shroud_of_concealment() : SpellScriptLoader("spell_rog_shroud_of_concealment") { }

        class spell_rog_shroud_of_concealment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_shroud_of_concealment_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (target->GetAura(ROGUE_SPELL_SHROUD_OF_CONCEALMENT_AURA, _player->GetGUID()))
                            if (!target->IsInRaidWith(_player) && !target->IsInPartyWith(_player))
                                target->RemoveAura(ROGUE_SPELL_SHROUD_OF_CONCEALMENT_AURA, _player->GetGUID());
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_shroud_of_concealment_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_shroud_of_concealment_SpellScript();
        }
};

// Crimson Tempest - 121411
class spell_rog_crimson_tempest : public SpellScriptLoader
{
    public:
        spell_rog_crimson_tempest() : SpellScriptLoader("spell_rog_crimson_tempest") { }

        class spell_rog_crimson_tempest_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_crimson_tempest_SpellScript);

            void HandleOnHit()
            {
                if (Player* const _player = GetCaster()->ToPlayer())
                {
                    if (Unit* const target = GetHitUnit())
                    {
                        int32 const damage = CalculatePct(GetHitDamage(), 40); // 240% / 6 (tick count)
                        _player->CastCustomSpell(target, ROGUE_SPELL_CRIMSON_TEMPEST_DOT, &damage, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_crimson_tempest_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_crimson_tempest_SpellScript();
        }
};

// Called by Wound Poison - 8680, Deadly Poison - 2818, Mind-Numbing Poison - 5760, Crippling Poison - 3409
// Paralytic Poison - 113952, Leeching Poison - 112961 and Deadly Poison : Instant damage - 113780
// Master Poisoner - 58410
class spell_rog_master_poisoner : public SpellScriptLoader
{
    public:
        spell_rog_master_poisoner() : SpellScriptLoader("spell_rog_master_poisoner") { }

        class spell_rog_master_poisoner_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_master_poisoner_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(ROGUE_SPELL_MASTER_POISONER_AURA))
                            _player->CastSpell(target, ROGUE_SPELL_MASTER_POISONER_DEBUFF, true);

                        if (GetSpellInfo()->IsLethalPoison())
                            if (_player->HasAura(ROGUE_SPELL_DEADLY_BREW))
                                _player->CastSpell(target, ROGUE_SPELL_CRIPPLING_POISON_DEBUFF, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_master_poisoner_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_master_poisoner_SpellScript();
        }
};

// Slice and Dice - 5171
class spell_rog_slice_and_dice : public SpellScriptLoader
{
    public:
        spell_rog_slice_and_dice() : SpellScriptLoader("spell_rog_slice_and_dice") { }

        class spell_rog_slice_and_dice_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_slice_and_dice_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Aura *sliceAndDice = _player->GetAura(ROGUE_SPELL_SLICE_AND_DICE))
                    {
                        int32 duration = sliceAndDice->GetDuration();
                        int32 maxDuration = sliceAndDice->GetMaxDuration();

                        // Replace old duration of Slice and Dice by the new duration ...
                        // ... five combo points : 36s instead of 30s
                        if (maxDuration >= 30000)
                        {
                            sliceAndDice->SetDuration(duration + 6000);
                            sliceAndDice->SetMaxDuration(maxDuration + 6000);
                        }
                        // ... four combo points : 30s instead of 25s
                        else if (maxDuration >= 25000)
                        {
                            sliceAndDice->SetDuration(duration + 5000);
                            sliceAndDice->SetMaxDuration(maxDuration + 5000);
                        }
                        // ... three combo points : 24s instead of 20s
                        else if (maxDuration >= 20000)
                        {
                            sliceAndDice->SetDuration(duration + 4000);
                            sliceAndDice->SetMaxDuration(maxDuration + 4000);
                        }
                        // ... two combo points : 18s instead of 15s
                        else if (maxDuration >= 15000)
                        {
                            sliceAndDice->SetDuration(duration + 3000);
                            sliceAndDice->SetMaxDuration(maxDuration + 3000);
                        }
                        // ... one combo point : 12s instead of 10s
                        else
                        {
                            sliceAndDice->SetDuration(duration + 2000);
                            sliceAndDice->SetMaxDuration(maxDuration + 2000);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_slice_and_dice_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_slice_and_dice_SpellScript();
        }
};

// Called by Deadly Poison - 2818
// Deadly Poison : Instant damage - 113780
class spell_rog_deadly_poison_instant_damage : public SpellScriptLoader
{
    public:
        spell_rog_deadly_poison_instant_damage() : SpellScriptLoader("spell_rog_deadly_poison_instant_damage") { }

        class spell_rog_deadly_poison_instant_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_deadly_poison_instant_damage_SpellScript);

            void HandleOnCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetExplTargetUnit())
                        if (target->HasAura(ROGUE_SPELL_DEADLY_POISON_DOT, _player->GetGUID()))
                            _player->CastSpell(target, ROGUE_SPELL_DEADLY_POISON_INSTANT_DAMAGE, true);
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_rog_deadly_poison_instant_damage_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_deadly_poison_instant_damage_SpellScript();
        }
};

// Paralytic Poison - 108215
class spell_rog_paralytic_poison : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetActionTarget();
        }

        void OnProc(AuraEffect const *, ProcEventInfo& eventInfo)
        {
            Unit * const target = eventInfo.GetActionTarget();
            if (Aura * const paralyticPoison = target->GetAura(ROGUE_SPELL_PARALYTIC_POISON_DEBUFF))
            {
                if (paralyticPoison->GetStackAmount() == 3 && !target->HasAura(ROGUE_SPELL_TOTAL_PARALYSIS))
                {
                    PreventDefaultAction();
                    target->RemoveAurasDueToSpell(ROGUE_SPELL_PARALYTIC_POISON_DEBUFF);
                    GetUnitOwner()->CastSpell(target, ROGUE_SPELL_TOTAL_PARALYSIS, true);
                }
            }
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(script_impl::CheckProc);
            OnEffectProc += AuraEffectProcFn(script_impl::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

public:
    spell_rog_paralytic_poison()
        : SpellScriptLoader("spell_rog_paralytic_poison")
    { }

    AuraScript * GetAuraScript() const
    {
        return new script_impl;
    }
};

// Shiv - 5938
class spell_rog_shiv : public SpellScriptLoader
{
    public:
        spell_rog_shiv() : SpellScriptLoader("spell_rog_shiv") { }

        class spell_rog_shiv_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_shiv_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(ROGUE_SPELL_MIND_NUMBLING_POISON))
                            _player->CastSpell(target, ROGUE_SPELL_MIND_PARALYSIS, true);
                        else if (_player->HasAura(ROGUE_SPELL_CRIPPLING_POISON))
                            _player->CastSpell(target, ROGUE_SPELL_DEBILITATING_POISON, true);
                        else if (_player->HasAura(ROGUE_SPELL_LEECHING_POISON))
                            _player->CastSpell(_player, ROGUE_SPELL_LEECH_VITALITY, true);
                        else if (_player->HasAura(ROGUE_SPELL_PARALYTIC_POISON))
                            _player->CastSpell(target, ROGUE_SPELL_PARTIAL_PARALYSIS, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_shiv_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_shiv_SpellScript();
        }
};

// All Poisons
// Deadly Poison - 2823, Wound Poison - 8679, Mind-numbing Poison - 5761, Leeching Poison - 108211, Paralytic Poison - 108215 or Crippling Poison - 3408
class spell_rog_poisons : public SpellScriptLoader
{
    public:
        spell_rog_poisons() : SpellScriptLoader("spell_rog_poisons") { }

        class spell_rog_poisons_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_poisons_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    switch (GetSpellInfo()->Id)
                    {
                        case ROGUE_SPELL_WOUND_POISON:
                        {
                            if (_player->HasAura(ROGUE_SPELL_DEADLY_POISON))
                                _player->RemoveAura(ROGUE_SPELL_DEADLY_POISON);
                            break;
                        }
                        case ROGUE_SPELL_MIND_NUMBLING_POISON:
                        {
                            if (_player->HasAura(ROGUE_SPELL_CRIPPLING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_CRIPPLING_POISON);
                            if (_player->HasAura(ROGUE_SPELL_LEECHING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_LEECHING_POISON);
                            if (_player->HasAura(ROGUE_SPELL_PARALYTIC_POISON))
                                _player->RemoveAura(ROGUE_SPELL_PARALYTIC_POISON);
                            break;
                        }
                        case ROGUE_SPELL_CRIPPLING_POISON:
                        {
                            if (_player->HasAura(ROGUE_SPELL_MIND_NUMBLING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_MIND_NUMBLING_POISON);
                            if (_player->HasAura(ROGUE_SPELL_LEECHING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_LEECHING_POISON);
                            if (_player->HasAura(ROGUE_SPELL_PARALYTIC_POISON))
                                _player->RemoveAura(ROGUE_SPELL_PARALYTIC_POISON);
                            break;
                        }
                        case ROGUE_SPELL_LEECHING_POISON:
                        {
                            if (_player->HasAura(ROGUE_SPELL_MIND_NUMBLING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_MIND_NUMBLING_POISON);
                            if (_player->HasAura(ROGUE_SPELL_CRIPPLING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_CRIPPLING_POISON);
                            if (_player->HasAura(ROGUE_SPELL_PARALYTIC_POISON))
                                _player->RemoveAura(ROGUE_SPELL_PARALYTIC_POISON);
                            break;
                        }
                        case ROGUE_SPELL_PARALYTIC_POISON:
                        {
                            if (_player->HasAura(ROGUE_SPELL_MIND_NUMBLING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_MIND_NUMBLING_POISON);
                            if (_player->HasAura(ROGUE_SPELL_CRIPPLING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_CRIPPLING_POISON);
                            if (_player->HasAura(ROGUE_SPELL_LEECHING_POISON))
                                _player->RemoveAura(ROGUE_SPELL_LEECHING_POISON);
                            break;
                        }
                        case ROGUE_SPELL_DEADLY_POISON:
                        {
                            if (_player->HasAura(ROGUE_SPELL_WOUND_POISON))
                                _player->RemoveAura(ROGUE_SPELL_WOUND_POISON);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_poisons_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_poisons_SpellScript();
        }
};

// Recuperate - 73651
class spell_rog_recuperate final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            GLYPH_OF_RECUPERATE = 56806,
            GLYPH_OF_RECOVERY   = 146625
        };

        void initEffects(uint32 &effectMask)
        {
            auto const caster = GetCaster();
            if (!caster || !caster->HasAura(GLYPH_OF_RECOVERY))
                effectMask &= ~(1 << EFFECT_1);
        }

        void calculateAmount(AuraEffect const *, int32 &amount, bool &canBeRecalculated)
        {
            canBeRecalculated = false;

            auto const caster = GetCaster();
            if (!caster)
                return;

            amount *= 1000;
            if (auto const eff = caster->GetAuraEffect(GLYPH_OF_RECUPERATE, EFFECT_0))
                amount += eff->GetAmount();

            amount = caster->CountPctFromMaxHealth(amount / 1000.0f);
        }

        void Register() final
        {
            OnInitEffects += AuraInitEffectsFn(script_impl::initEffects);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::calculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
        }
    };

public:
    spell_rog_recuperate()
        : SpellScriptLoader("spell_rog_recuperate")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Preparation - 14185
class spell_rog_preparation : public SpellScriptLoader
{
    public:
        spell_rog_preparation() : SpellScriptLoader("spell_rog_preparation") { }

        class spell_rog_preparation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_preparation_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player * const caster = GetCaster()->ToPlayer();
                if (!caster)
                    return;

                SpellCooldowns const &cm = caster->GetSpellCooldownMap();

                SpellCooldowns::const_iterator i = cm.begin();
                SpellCooldowns::const_iterator next = i;

                for (; i != cm.end(); i = next)
                {
                    ++next;

                    SpellInfo const * const spellInfo = sSpellMgr->GetSpellInfo(i->first);
                    if (!spellInfo)
                        continue;

                    switch (spellInfo->Id)
                    {
                        case 1856:  // Vanish
                        case 2983:  // Sprint
                        case 5277:  // Evasion
                        case 51722: // Dismantle
                            caster->RemoveSpellCooldown(spellInfo->Id, true);
                            break;
                    }
                }
            }

            void Register()
            {
                // add dummy effect spell handler to Preparation
                OnEffectHitTarget += SpellEffectFn(spell_rog_preparation_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_preparation_SpellScript();
        }
};

class spell_rog_deadly_poison : public SpellScriptLoader
{
    public:
        spell_rog_deadly_poison() : SpellScriptLoader("spell_rog_deadly_poison") { }

        class spell_rog_deadly_poison_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_deadly_poison_SpellScript);

            bool Load()
            {
                _stackAmount = 0;
                // at this point CastItem must already be initialized
                return GetCaster()->GetTypeId() == TYPEID_PLAYER && GetCastItem();
            }

            void HandleBeforeHit()
            {
                if (Unit* target = GetHitUnit())
                {
                    // Deadly Poison
                    Trinity::Flag128 flags(0x10000, 0x80000);
                    if (AuraEffect const *aurEff = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_ROGUE, flags, GetCaster()->GetGUID()))
                        _stackAmount = aurEff->GetBase()->GetStackAmount();
                }
            }

            void HandleAfterHit()
            {
                if (_stackAmount < 5)
                    return;

                Player* player = GetCaster()->ToPlayer();

                if (Unit* target = GetHitUnit())
                {

                    Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

                    if (item == GetCastItem())
                        item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                    if (!item)
                        return;

                    // item combat enchantments
                    for (uint8 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
                    {
                        if (slot > PRISMATIC_ENCHANTMENT_SLOT || slot < PROP_ENCHANTMENT_SLOT_0)    // not holding enchantment id
                            continue;

                        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(item->GetEnchantmentId(EnchantmentSlot(slot)));
                        if (!enchant)
                            continue;

                        for (uint8 s = 0; s < MAX_ITEM_ENCHANTMENT_EFFECTS; ++s)
                        {
                            if (enchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                continue;

                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(enchant->spellid[s]);
                            if (!spellInfo)
                            {
                                TC_LOG_ERROR("spells", "Player::CastItemCombatSpell Enchant %i, player (Name: %s, GUID: %u) cast unknown spell %i",
                                             enchant->ID, player->GetName().c_str(), player->GetGUIDLow(), enchant->spellid[s]);
                                continue;
                            }

                            // Proc only rogue poisons
                            if (spellInfo->SpellFamilyName != SPELLFAMILY_ROGUE || spellInfo->Dispel != DISPEL_POISON)
                                continue;

                            // Do not reproc deadly
                            if (spellInfo->SpellFamilyFlags.IsEqual(0x10000, 0x80000, 0))
                                continue;

                            if (spellInfo->IsPositive())
                                player->CastSpell(player, enchant->spellid[s], true, item);
                            else
                                player->CastSpell(target, enchant->spellid[s], true, item);
                        }
                    }
                }
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_rog_deadly_poison_SpellScript::HandleBeforeHit);
                AfterHit += SpellHitFn(spell_rog_deadly_poison_SpellScript::HandleAfterHit);
            }

            uint8 _stackAmount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_deadly_poison_SpellScript();
        }
};

// Shadowstep - 36554
class spell_rog_shadowstep : public SpellScriptLoader
{
    public:
        spell_rog_shadowstep() : SpellScriptLoader("spell_rog_shadowstep") { }

        class spell_rog_shadowstep_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_shadowstep_SpellScript);

            SpellCastResult CheckCast()
            {
                if (GetCaster()->HasUnitState(UNIT_STATE_ROOT))
                    return SPELL_FAILED_ROOTED;
                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_rog_shadowstep_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_shadowstep_SpellScript();
        }
};

// Marked for Death - 137619
class spell_rog_marked_for_death : public SpellScriptLoader
{
public:
    spell_rog_marked_for_death() : SpellScriptLoader("spell_rog_marked_for_death") { }

    class spell_rog_marked_for_death_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_marked_for_death_AuraScript );

        void HandleRemove(AuraEffect const *, AuraEffectHandleModes)
        {
            if (GetCaster() && GetTarget() && GetCaster()->GetTypeId() == TYPEID_PLAYER)
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                    GetCaster()->ToPlayer()->RemoveSpellCooldown(GetId(), true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_rog_marked_for_death_AuraScript ::HandleRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rog_marked_for_death_AuraScript();
    }
};

// Cloak and Dagger
class spell_rog_cloak_and_dagger : public SpellScriptLoader
{
public:
    spell_rog_cloak_and_dagger() : SpellScriptLoader("spell_rog_cloak_and_dagger") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            TALENT_CLOAK_AND_SHADOW     = 138106,
            CLOAK_AND_SHADOW_TELEPORT   = 132987
        };

        void HandleOnHit()
        {
            auto caster = GetCaster();
            auto target = GetHitUnit();
            if (caster && target && caster->HasAura(TALENT_CLOAK_AND_SHADOW))
            {
                caster->CastSpell(target, CLOAK_AND_SHADOW_TELEPORT, true);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(script_impl::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// 76577 Smoke Bomb
class spell_rog_smoke_bomb : public SpellScriptLoader
{
    public:
        spell_rog_smoke_bomb() : SpellScriptLoader("spell_rog_smoke_bomb") { }

        class spell_rog_smoke_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_smoke_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (DynamicObject* dyn = GetTarget()->GetDynObject(aurEff->GetId()))
                    GetTarget()->CastSpell(dyn->GetPositionX(), dyn->GetPositionY(), dyn->GetPositionZ(), 88611, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_smoke_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_smoke_AuraScript();
        }
};

// Shuriken Toss - 114014
class spell_rog_shuriken_toss : public SpellScriptLoader
{
public:
    spell_rog_shuriken_toss() : SpellScriptLoader("spell_rog_shuriken_toss") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            SPELL_SHURIKEN_TOSS_PROC = 137586,
        };

        void HandleOnHit()
        {
            auto caster = GetCaster();
            auto target = GetHitUnit();
            if (caster && target && caster->GetDistance(target) > 10.f)
            {
                caster->CastSpell(caster, SPELL_SHURIKEN_TOSS_PROC, true);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(script_impl::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Fan of Knives - 51723
class spell_rog_fan_of_knives final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        void filterTargets(std::list<WorldObject*> &targets)
        {
            if (auto player = GetCaster()->ToPlayer())
            {
                auto comboTarget = player->GetComboTarget();
                if (!comboTarget)
                    return;

                targets.remove_if([comboTarget](WorldObject * obj)
                {
                    return obj->GetGUID() != comboTarget;
                });
            }
        }

        void Register() final
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::filterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

public:
    spell_rog_fan_of_knives()
        : SpellScriptLoader("spell_rog_fan_of_knives")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

// Killing Spree
class spell_rog_killing_spree : public SpellScriptLoader
{
public:
    spell_rog_killing_spree() : SpellScriptLoader("spell_rog_killing_spree") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void HandleEffectPeriodic(AuraEffect const * /*aurEff*/)
        {
            auto target = GetTarget();
            if (!target)
                return;

            bool hasBladeFlurry = target->HasAura(ROGUE_SPELL_BLADE_FLURRY);
            Unit* spellTarget = target->GetVictim();
            if (hasBladeFlurry)
            {
                UnitList targets;
                {
                    // eff_radius == 0
                    float radius = GetSpellInfo()->GetMaxRange(false);

                    CellCoord p(Trinity::ComputeCellCoord(target->GetPositionX(), target->GetPositionY()));
                    Cell cell(p);

                    Trinity::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck u_check(target, radius);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck> checker(target, targets, u_check);

                    cell.Visit(p, Trinity::makeGridVisitor(checker), *GetOwner()->GetMap(), *target, radius);
                    cell.Visit(p, Trinity::makeWorldVisitor(checker), *GetOwner()->GetMap(), *target, radius);
                }

                if (targets.empty())
                    return;

                spellTarget = Trinity::Containers::SelectRandomContainerElement(targets);
            }

            if (!spellTarget)
                spellTarget = target->SelectNearestTarget(10.f);

            if (spellTarget)
            {
                target->CastSpell(spellTarget, 57840, true);
                target->CastSpell(spellTarget, 57841, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    class script_impl_spell : public SpellScript
    {
        PrepareSpellScript(script_impl_spell);

        SpellCastResult CheckCast()
        {
            Unit* caster = GetCaster();
            if (caster->GetTypeId() != TYPEID_PLAYER)
                return SPELL_FAILED_DONT_REPORT;

            if (Unit* target = caster->ToPlayer()->GetSelectedUnit())
            {
                if (target->IsFriendlyTo(caster))
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else
                return SPELL_FAILED_BAD_TARGETS;

            return SPELL_CAST_OK;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(script_impl_spell::CheckCast);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl_spell();
    }
};

class spell_rog_deadly_throw : public SpellScriptLoader
{
public:
    spell_rog_deadly_throw() : SpellScriptLoader("spell_rog_deadly_throw") {}
 
    class spell_rog_deadly_throw_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rog_deadly_throw_SpellScript);
 
        SpellCastResult HandleCheckCast()
        {
            if (Player* player = GetCaster()->ToPlayer())
                if (player->GetComboPoints() < 3)
                    return SPELL_FAILED_DONT_REPORT;

            return SPELL_CAST_OK;
        }
 
        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_rog_deadly_throw_SpellScript::HandleCheckCast);
        }
    };
 
    SpellScript* GetSpellScript() const
    {
        return new spell_rog_deadly_throw_SpellScript();
    }
};

// Vanish - 1856
class spell_rog_glyph_of_decoy : public SpellScriptLoader
{
public:
    spell_rog_glyph_of_decoy() : SpellScriptLoader("spell_rog_glyph_of_decoy") {}
 
    class spell_rog_glyph_of_decoy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rog_glyph_of_decoy_SpellScript);
 
        enum
        {
            SPELL_ROGUE_GLYPH_OF_DECOY_PASSIVE = 56800,
            SPELL_ROGUE_GLYPH_OF_DECOY = 89765
        };

        void HandleCast()
        {
            if (!GetCaster()->HasAura(SPELL_ROGUE_GLYPH_OF_DECOY_PASSIVE))
                return;

            GetCaster()->CastSpell(GetCaster(), SPELL_ROGUE_GLYPH_OF_DECOY, true);
        }
 
        void Register()
        {
            OnCast += SpellCastFn(spell_rog_glyph_of_decoy_SpellScript::HandleCast);
        }
    };
 
    SpellScript* GetSpellScript() const
    {
        return new spell_rog_glyph_of_decoy_SpellScript();
    }
};

void AddSC_rogue_spell_scripts()
{
    new spell_rog_deadly_throw();
    new spell_rog_glyph_of_expose_armor();
    new spell_rog_cheat_death();
    new spell_rog_blade_flurry();
    new spell_rog_blade_flurry_damage();
    new spell_rog_growl();
    new spell_rog_cloak_of_shadows();
    new spell_rog_combat_readiness();
    new spell_rog_nerve_strike();
    new spell_rog_nightstalker();
    new spell_rog_sanguinary_vein();
    new spell_rog_hemorrhage();
    new spell_rog_restless_blades();
    new spell_rog_cut_to_the_chase();
    new spell_rog_venomous_wounds();
    new spell_rog_redirect();
    new spell_rog_shroud_of_concealment();
    new spell_rog_crimson_tempest();
    new spell_rog_master_poisoner();
    new spell_rog_slice_and_dice();
    new spell_rog_deadly_poison_instant_damage();
    new spell_rog_paralytic_poison();
    new spell_rog_shiv();
    new spell_rog_poisons();
    new spell_rog_recuperate();
    new spell_rog_preparation();
    new spell_rog_deadly_poison();
    new spell_rog_shadowstep();
    new spell_rog_marked_for_death();
    new spell_rog_cloak_and_dagger();
    new spell_rog_smoke_bomb();
    new spell_rog_shuriken_toss();
    new spell_rog_fan_of_knives();
    new spell_rog_killing_spree();
    new spell_rog_glyph_of_decoy();
}
