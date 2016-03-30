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
 * Scripts for spells with SPELLFAMILY_SHAMAN and SPELLFAMILY_GENERIC spells used by shaman players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_sha_".
 */

#include "ScriptMgr.h"
#include "GridNotifiers.h"
#include "Unit.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "ObjectVisitors.hpp"

enum ShamanSpells
{
    SHAMAN_SPELL_SATED                      = 57724,
    SHAMAN_SPELL_EXHAUSTION                 = 57723,
    HUNTER_SPELL_INSANITY                   = 95809,
    MAGE_SPELL_TEMPORAL_DISPLACEMENT        = 80354,
    SPELL_SHA_LIGHTNING_SHIELD_AURA         = 324,
    SPELL_SHA_ASCENDANCE_ELEMENTAL          = 114050,
    SPELL_SHA_ASCENDANCE_RESTORATION        = 114052,
    SPELL_SHA_ASCENDANCE_ENHANCED           = 114051,
    SPELL_SHA_ASCENDANCE                    = 114049,
    SPELL_SHA_HEALING_RAIN                  = 73920,
    SPELL_SHA_HEALING_RAIN_TICK             = 73921,
    SPELL_SHA_EARTHQUAKE                    = 61882,
    SPELL_SHA_EARTHQUAKE_TICK               = 77478,
    SPELL_SHA_EARTHQUAKE_KNOCKING_DOWN      = 77505,
    SPELL_SHA_ELEMENTAL_BLAST               = 117014,
    SPELL_SHA_ELEMENTAL_BLAST_RATING_BONUS  = 118522,
    SPELL_SHA_ELEMENTAL_BLAST_NATURE_VISUAL = 118517,
    SPELL_SHA_ELEMENTAL_BLAST_FROST_VISUAL  = 118515,
    SPELL_SHA_LAVA_LASH                     = 60103,
    SPELL_SHA_FLAME_SHOCK                   = 8050,
    SPELL_SHA_STORMSTRIKE                   = 17364,
    SPELL_SHA_LIGHTNING_SHIELD_ORB_DAMAGE   = 26364,
    SPELL_SHA_HEALING_STREAM                = 52042,
    SPELL_SHA_GLYPH_OF_HEALING_STREAM       = 119523,
    SPELL_SHA_LAVA_SURGE_CAST_TIME          = 77762,
    SPELL_SHA_FULMINATION                   = 88766,
    SPELL_SHA_FULMINATION_TRIGGERED         = 88767,
    SPELL_SHA_FULMINATION_INFO              = 95774,
    SPELL_SHA_ROLLING_THUNDER_AURA          = 88764,
    SPELL_SHA_ROLLING_THUNDER_ENERGIZE      = 88765,
    SPELL_SHA_UNLEASH_ELEMENTS              = 73680,
    SPELL_SHA_FIRE_NOVA                     = 1535,
    SPELL_SHA_FIRE_NOVA_TRIGGERED           = 131786,
    SPELL_SHA_TIDAL_WAVES                   = 51564,
    SPELL_SHA_TIDAL_WAVES_PROC              = 53390,
    SPELL_SHA_MANA_TIDE                     = 16191,
    SPELL_SHA_FROST_SHOCK_FREEZE            = 63685,
    SPELL_SHA_FROZEN_POWER                  = 63374,
    SPELL_SHA_MAIL_SPECIALIZATION_AGI       = 86099,
    SPELL_SHA_MAIL_SPECIALISATION_INT       = 86100,
    SPELL_SHA_UNLEASHED_FURY_TALENT         = 117012,
    SPELL_SHA_UNLEASHED_FURY_FLAMETONGUE    = 118470,
    SPELL_SHA_UNLEASHED_FURY_WINDFURY       = 118472,
    SPELL_SHA_UNLEASHED_FURY_EARTHLIVING    = 118473,
    SPELL_SHA_UNLEASHED_FURY_FROSTBRAND     = 118474,
    SPELL_SHA_UNLEASHED_FURY_ROCKBITER      = 118475,
    SPELL_SHA_STONE_BULWARK_ABSORB          = 114893,
    SPELL_SHA_EARTHGRAB_IMMUNITY            = 116946,
    SPELL_SHA_EARTHBIND_FOR_EARTHGRAB_TOTEM = 116947,
    SPELL_SHA_EARTHGRAB_ROOT                = 64695,
    SPELL_SHA_ECHO_OF_THE_ELEMENTS          = 108283,
    SPELL_SHA_ANCESTRAL_GUIDANCE            = 114911,
    SPELL_SHA_CONDUCTIVITY_TALENT           = 108282,
    SPELL_SHA_CONDUCTIVITY_HEAL             = 118800,
    SPELL_SHA_GLYPH_OF_LAKESTRIDER          = 55448,
    SPELL_SHA_WATER_WALKING                 = 546,
    SPELL_SHA_GLYPH_OF_SHAMANISTIC_RAGE     = 63280,
    SPELL_SHA_GHOST_WOLF                    = 2645,
    SPELL_SHA_ITEM_T14_4P                   = 123124,
	SPELL_SHA_RAIN_OF_FROGS                 = 147709,
    SPELL_SHA_GLYPH_OF_HEALING_STREAM_TOTEM = 55456
};

// Hex - 51514
class spell_sha_hex : public SpellScriptLoader
{
    public:
        spell_sha_hex() : SpellScriptLoader("spell_sha_hex") { }

        class spell_sha_hex_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_hex_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = GetTarget())
                {
                    if (target->IsMounted())
                    {
                        target->Dismount();
                        target->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    }

                    if (target->HasUnitState(UNIT_STATE_CASTING))
                    {
                        target->InterruptSpell(CURRENT_GENERIC_SPELL);
                        target->InterruptSpell(CURRENT_CHANNELED_SPELL);
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_sha_hex_AuraScript::OnApply, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_hex_AuraScript();
        }
};

// Water Ascendant - 114052
class spell_sha_water_ascendant : public SpellScriptLoader
{
    public:
        spell_sha_water_ascendant() : SpellScriptLoader("spell_sha_water_ascendant") { }

        class spell_sha_water_ascendant_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_water_ascendant_AuraScript);

            void OnProc(AuraEffect const * /*aurEff*/, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                Player* player = GetCaster()->ToPlayer();
                if (!player)
                    return;

                if (player->HasSpellCooldown(114083))
                    return;

                if (eventInfo.GetActor()->GetGUID() != player->GetGUID())
                    return;

                if (!eventInfo.GetDamageInfo()->GetSpellInfo())
                    return;

                if (eventInfo.GetDamageInfo()->GetSpellInfo()->Id == 114083 || eventInfo.GetDamageInfo()->GetSpellInfo()->Id == SPELL_SHA_ANCESTRAL_GUIDANCE)
                    return;

                if (!(eventInfo.GetHealInfo()->GetHeal()))
                    return;

                if (!(eventInfo.GetDamageInfo()->GetSpellInfo()->IsPositive()))
                    return;

                if (Unit* target = eventInfo.GetActionTarget())
                {
                    std::list<Unit*> tempList;
                    std::list<Unit*> alliesList;
                    target->GetAttackableUnitListInRange(tempList, 15.0f);

                    for (auto itr : tempList)
                    {
                        if (!player->IsWithinLOSInMap(itr))
                            continue;

                        if (itr->IsHostileTo(player))
                            continue;

                        alliesList.push_back(itr);
                    }

                    if (!alliesList.empty())
                    {
                        // Heal amount distribued for all allies, caster included
                        int32 bp = eventInfo.GetHealInfo()->GetHeal() / alliesList.size();

                        if (bp > 0)
                            player->CastCustomSpell((*alliesList.begin()), 114083, &bp, NULL, NULL, true);   // Restorative Mists

                        player->AddSpellCooldown(114083, 0, 1 * IN_MILLISECONDS);               // This prevent from multiple procs
                    }
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_water_ascendant_AuraScript::OnProc, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_water_ascendant_AuraScript();
        }
};

// Prowl - 113289
class spell_sha_prowl : public SpellScriptLoader
{
    public:
        spell_sha_prowl() : SpellScriptLoader("spell_sha_prowl") { }

        class spell_sha_prowl_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_prowl_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->CastSpell(_player, SPELL_SHA_GHOST_WOLF, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_prowl_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_prowl_SpellScript();
        }
};

// Called by Shamanistic Rage - 30823
// Glyph of Shamanistic Rage - 63280
class spell_sha_glyph_of_shamanistic_rage : public SpellScriptLoader
{
    public:
        spell_sha_glyph_of_shamanistic_rage() : SpellScriptLoader("spell_sha_glyph_of_shamanistic_rage") { }

        class spell_sha_glyph_of_shamanistic_rage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_glyph_of_shamanistic_rage_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(SPELL_SHA_GLYPH_OF_SHAMANISTIC_RAGE))
                    {
                        DispelChargesList dispelList;
                        _player->GetDispellableAuraList(_player, DISPEL_ALL_MASK, dispelList);
                        if (!dispelList.empty())
                        {
                            for (auto itr : dispelList)
                                if (_player->HasAura(itr.first->GetId()))
                                    _player->RemoveAura(itr.first);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_glyph_of_shamanistic_rage_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_glyph_of_shamanistic_rage_SpellScript();
        }
};

// Called by Ghost Wolf - 2645
// Glyph of Lakestrider - 55448
class spell_sha_glyph_of_lakestrider : public SpellScriptLoader
{
    public:
        spell_sha_glyph_of_lakestrider() : SpellScriptLoader("spell_sha_glyph_of_lakestrider") { }

        class spell_sha_glyph_of_lakestrider_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_glyph_of_lakestrider_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(SPELL_SHA_GLYPH_OF_LAKESTRIDER))
                        _player->CastSpell(_player, SPELL_SHA_WATER_WALKING, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_glyph_of_lakestrider_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_glyph_of_lakestrider_SpellScript();
        }
};

// Call of the Elements - 108285
class spell_sha_call_of_the_elements : public SpellScriptLoader
{
    public:
        spell_sha_call_of_the_elements() : SpellScriptLoader("spell_sha_call_of_the_elements") { }

        class spell_sha_call_of_the_elements_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_call_of_the_elements_SpellScript);

            void HandleOnHit()
            {
                auto const player = GetCaster()->ToPlayer();
                if (!player)
                    return;

                std::vector<uint32> spellsToRemoveCd;

                // immediately finishes the cooldown on totems with less than 3min cooldown
                for (auto const &kvPair : player->GetSpellCooldownMap())
                {
                    auto const spellInfo = sSpellMgr->GetSpellInfo(kvPair.first);
                    if (!spellInfo || spellInfo->GetRecoveryTime() == 0)
                        continue;

                    switch (spellInfo->Id)
                    {
                        case 2484:
                        case 5394:
                        case 8143:
                        case 8177:
                        case 51485:
                        case 108269:
                        case 108270:
                        case 108273:
                            spellsToRemoveCd.emplace_back(spellInfo->Id);
                            break;
                    }
                }

                for (auto const &spell : spellsToRemoveCd)
                    player->RemoveSpellCooldown(spell, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_call_of_the_elements_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_call_of_the_elements_SpellScript();
        }
};

// Ancestral Guidance - 108281
class spell_sha_ancestral_guidance : public SpellScriptLoader
{
    public:
        spell_sha_ancestral_guidance() : SpellScriptLoader("spell_sha_ancestral_guidance") { }

        class spell_sha_ancestral_guidance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_ancestral_guidance_AuraScript);

            void OnProc(AuraEffect const * /*aurEff*/, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                Player* _player = GetCaster()->ToPlayer();
                if (!_player)
                    return;

                if (eventInfo.GetActor()->GetGUID() != _player->GetGUID())
                    return;

                if (!eventInfo.GetDamageInfo()->GetSpellInfo())
                    return;

                if (eventInfo.GetDamageInfo()->GetSpellInfo()->Id == SPELL_SHA_ANCESTRAL_GUIDANCE || eventInfo.GetDamageInfo()->GetSpellInfo()->Id == 114083)
                    return;

                if (!(eventInfo.GetDamageInfo()->GetDamage()) && !(eventInfo.GetHealInfo()->GetHeal()))
                    return;

                if (!(eventInfo.GetDamageInfo()->GetDamageType() == SPELL_DIRECT_DAMAGE) && !(eventInfo.GetDamageInfo()->GetDamageType() == HEAL))
                    return;

                if (Unit* target = eventInfo.GetActionTarget())
                {
                    uint32 spellDamage = eventInfo.GetDamageInfo()->GetDamage();
                    uint32 spellHeal = eventInfo.GetHealInfo()->GetHeal();
                    bool isDamage = spellDamage > spellHeal;
                    int32 bp = std::max(spellDamage, spellHeal);
                    if (!bp)
                        return;

                    bp = isDamage ? int32(bp * 0.40f) : int32(bp * 0.60f);

                    _player->CastCustomSpell(target, SPELL_SHA_ANCESTRAL_GUIDANCE, &bp, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_ancestral_guidance_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_ancestral_guidance_AuraScript();
        }
};

// Echo of the Elements - 108283
class spell_sha_echo_of_the_elements : public SpellScriptLoader
{
    public:
        spell_sha_echo_of_the_elements() : SpellScriptLoader("spell_sha_echo_of_the_elements") { }

        class spell_sha_echo_of_the_elements_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_echo_of_the_elements_AuraScript);

            void OnProc(AuraEffect const * /*aurEff*/, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                Player* player = GetCaster()->ToPlayer();
                Unit* target = eventInfo.GetActionTarget();
                if (!player || !target)
                    return;

                if (player->HasSpellCooldown(SPELL_SHA_ECHO_OF_THE_ELEMENTS))
                    return;

                if (eventInfo.GetActor()->GetGUID() != player->GetGUID())
                    return;

                SpellInfo const* procSpell = eventInfo.GetDamageInfo()->GetSpellInfo();
                if (!procSpell)
                    return;

                bool singleTarget = false;
                for (auto const &spellEffect : procSpell->Effects)
                {
                    if ((spellEffect.TargetA.GetTarget() == TARGET_UNIT_TARGET_ALLY || spellEffect.TargetA.GetTarget() == TARGET_UNIT_TARGET_ENEMY)
                            && spellEffect.TargetB.GetTarget() == 0)
                        singleTarget = true;
                }

                if (!singleTarget)
                    return;

                int32 chance = 6;

                // devs told that proc chance is 6% for Elemental and Restoration specs and 30% for Enhancement
                if (player->GetSpecializationId(player->GetActiveSpec()) == SPEC_SHAMAN_ENHANCEMENT && procSpell->Id != SPELL_SHA_ELEMENTAL_BLAST)
                    chance = 30;

                // Chain lightnings individual hits have a reduced chance to proc
                if (procSpell->Id == 421)
                    chance /= 3;

                if (!(eventInfo.GetDamageInfo()->GetDamage()) && !(eventInfo.GetHealInfo()->GetHeal()))
                    return;

                if (!(eventInfo.GetDamageInfo()->GetDamageType() == SPELL_DIRECT_DAMAGE) && !(eventInfo.GetDamageInfo()->GetDamageType() == HEAL))
                    return;

                if (!roll_chance_i(chance))
                    return;

                player->AddSpellCooldown(SPELL_SHA_ECHO_OF_THE_ELEMENTS, 0, 2 * IN_MILLISECONDS); // This prevent from multiple procs
                player->CastSpell(target, procSpell->Id, true);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_echo_of_the_elements_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_echo_of_the_elements_AuraScript();
        }
};

// Earthgrab - 64695
class spell_sha_earthgrab : public SpellScriptLoader
{
    public:
        spell_sha_earthgrab() : SpellScriptLoader("spell_sha_earthgrab") { }

        class spell_sha_earthgrab_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_earthgrab_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->HasAura(SPELL_SHA_EARTHGRAB_IMMUNITY, caster->GetGUID()) && !target->HasAura(SPELL_SHA_EARTHGRAB_ROOT))
                        {
                            caster->CastSpell(target, SPELL_SHA_EARTHBIND_FOR_EARTHGRAB_TOTEM, true);
                            PreventHitDefaultEffect(EFFECT_0);
                        }
                        else
                            caster->CastSpell(target, SPELL_SHA_EARTHGRAB_IMMUNITY, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_earthgrab_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_earthgrab_SpellScript();
        }
};

// Stone Bulwark - 114893
class spell_sha_stone_bulwark : public SpellScriptLoader
{
    public:
        spell_sha_stone_bulwark() : SpellScriptLoader("spell_sha_stone_bulwark") { }

        class spell_sha_stone_bulwark_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_stone_bulwark_AuraScript);

            void CalculateAmount(AuraEffect const *, int32 & amount, bool & )
            {
                auto caster = GetCaster();
                if (!caster)
                    return;

                if (Unit* owner = caster->GetOwner())
                {
                    if (AuraEffect* triggerEffect = caster->GetAuraEffect(114889, EFFECT_0))
                    {
                        bool bonus = triggerEffect->GetTickNumber() < 2;
                        float AP = owner->GetTotalAttackPowerValue(BASE_ATTACK);
                        float SP = owner->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask());
                        float coeff = 0.875f * (bonus ? 4.0f : 1.0f);
                        amount = std::max(0.625f * AP, SP) * coeff;
                    }
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_sha_stone_bulwark_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_stone_bulwark_AuraScript();
        }
};

// Mail Specialization - 86529
class spell_sha_mail_specialization : public SpellScriptLoader
{
    public:
        spell_sha_mail_specialization() : SpellScriptLoader("spell_sha_mail_specialization") { }

        class spell_sha_mail_specialization_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_mail_specialization_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_SHAMAN_ELEMENTAL
                            || _player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_SHAMAN_RESTORATION)
                        _player->CastSpell(_player, SPELL_SHA_MAIL_SPECIALISATION_INT, true);
                    else if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_SHAMAN_ENHANCEMENT)
                        _player->CastSpell(_player, SPELL_SHA_MAIL_SPECIALIZATION_AGI, true);
                }
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(SPELL_SHA_MAIL_SPECIALISATION_INT))
                        _player->RemoveAura(SPELL_SHA_MAIL_SPECIALISATION_INT);
                    else if (_player->HasAura(SPELL_SHA_MAIL_SPECIALIZATION_AGI))
                        _player->RemoveAura(SPELL_SHA_MAIL_SPECIALIZATION_AGI);
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_sha_mail_specialization_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_sha_mail_specialization_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_mail_specialization_AuraScript();
        }
};

// Frost Shock - 8056
class spell_sha_frozen_power : public SpellScriptLoader
{
    public:
        spell_sha_frozen_power() : SpellScriptLoader("spell_sha_frozen_power") { }

        class spell_sha_frozen_power_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_frozen_power_SpellScript);

            bool Validate(SpellInfo const * /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(8056))
                    return false;
                return true;
            }

            void HandleAfterHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(SPELL_SHA_FROZEN_POWER))
                            _player->CastSpell(target, SPELL_SHA_FROST_SHOCK_FREEZE, true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_sha_frozen_power_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_frozen_power_SpellScript();
        }
};

// Spirit Link - 98020 : triggered by 98017
// Spirit Link Totem
class spell_sha_spirit_link : public SpellScriptLoader
{
    public:
        spell_sha_spirit_link() : SpellScriptLoader("spell_sha_spirit_link") { }

        class spell_sha_spirit_link_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_spirit_link_SpellScript);

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->GetEntry() == 53006)
                    {
                        if (caster->GetOwner())
                        {
                            if (Player* _player = caster->GetOwner()->ToPlayer())
                            {
                                std::list<Unit*> memberList;
                                _player->GetPartyMembers(memberList);

                                float totalRaidHealthPct = 0;

                                for (auto itr : memberList)
                                    totalRaidHealthPct += itr->GetHealthPct();

                                totalRaidHealthPct /= memberList.size() * 100.0f;

                                for (auto itr : memberList)
                                    itr->SetHealth(uint32(totalRaidHealthPct * itr->GetMaxHealth()));
                            }
                        }
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_sha_spirit_link_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_spirit_link_SpellScript();
        }
};

// Mana Tide - 16191
class spell_sha_mana_tide : public SpellScriptLoader
{
    public:
        spell_sha_mana_tide() : SpellScriptLoader("spell_sha_mana_tide") { }

        class spell_sha_mana_tide_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_mana_tide_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHA_ELEMENTAL_BLAST))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                if (Unit* target = GetHitUnit())
                {
                    if (Player* _player = GetCaster()->GetOwner()->ToPlayer())
                    {
                        AuraApplication* aura = target->GetAuraApplication(SPELL_SHA_MANA_TIDE, GetCaster()->GetGUID());

                        aura->GetBase()->GetEffect(0)->ChangeAmount(0);

                        int32 spirit = _player->GetStat(STAT_SPIRIT) * 2;

                        aura->GetBase()->GetEffect(0)->ChangeAmount(spirit);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_mana_tide_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_mana_tide_SpellScript();
        }
};

// Called by Chain Heal - 1064 or Riptide - 61295
// Tidal Waves - 51564
class spell_sha_tidal_waves : public SpellScriptLoader
{
    public:
        spell_sha_tidal_waves() : SpellScriptLoader("spell_sha_tidal_waves") { }

        class spell_sha_tidal_waves_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_tidal_waves_SpellScript)

            bool Validate(SpellInfo const * /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(1064) || !sSpellMgr->GetSpellInfo(61295))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(SPELL_SHA_TIDAL_WAVES))
                    {
                        if (GetHitUnit())
                        {
                            int32 bp0 = -(sSpellMgr->GetSpellInfo(SPELL_SHA_TIDAL_WAVES)->Effects[EFFECT_0].BasePoints);
                            int32 bp1 = sSpellMgr->GetSpellInfo(SPELL_SHA_TIDAL_WAVES)->Effects[EFFECT_0].BasePoints;
                            _player->CastCustomSpell(_player, SPELL_SHA_TIDAL_WAVES_PROC, &bp0, &bp1, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_tidal_waves_SpellScript::HandleOnHit);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_sha_tidal_waves_SpellScript();
        }
};

// Fire Nova - 1535
class spell_sha_fire_nova : public SpellScriptLoader
{
    public:
        spell_sha_fire_nova() : SpellScriptLoader("spell_sha_fire_nova") { }

        class spell_sha_fire_nova_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_fire_nova_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    if (target->HasAura(SPELL_SHA_FLAME_SHOCK))
                    {
                        caster->CastSpell(target, SPELL_SHA_FIRE_NOVA_TRIGGERED, true);
                    }
                }
            }

            SpellCastResult HandleCheckCast()
            {
                UnitList targets;
                Trinity::AnyUnitHavingBuffInObjectRangeCheck u_check(GetCaster(), GetCaster(), 100, SPELL_SHA_FLAME_SHOCK, false);
                Trinity::UnitListSearcher<Trinity::AnyUnitHavingBuffInObjectRangeCheck> searcher(GetCaster(), targets, u_check);
                Trinity::VisitNearbyObject(GetCaster(), 100, searcher);

                return targets.size() == 0 ? SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW : SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_sha_fire_nova_SpellScript::HandleCheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_sha_fire_nova_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_fire_nova_SpellScript();
        }
};

// Unleash Elements - 73680
class spell_sha_unleash_elements : public SpellScriptLoader
{
    public:
        spell_sha_unleash_elements() : SpellScriptLoader("spell_sha_unleash_elements") { }

        class spell_sha_unleash_elements_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_unleash_elements_SpellScript);

            SpellCastResult CheckEnchant()
            {
                if (!GetCaster())
                    return SPELL_FAILED_DONT_REPORT;

                if (Player* player = GetCaster()->ToPlayer())
                {
                    Item* weapons[2];
                    weapons[0] = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    weapons[1] = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    bool enchantFound = false;

                    for (int i = 0; i < 2; i++)
                    {
                        if (!weapons[i])
                            continue;

                        switch (weapons[i]->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                        {
                            case 3345:// Earthliving Weapon
                            case 5:   // Flametongue Weapon
                            case 2:   // Frostbrand Weapon
                            case 3021:// Rockbiter Weapon
                            case 283: // Windfury Weapon
                                enchantFound = true;
                                break;
                            default:
                                break;
                        }
                    }

                    if (!enchantFound)
                        return SPELL_FAILED_DONT_REPORT;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        Item* weapons[2];
                        weapons[0] = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                        weapons[1] = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                        for (int i = 0; i < 2; i++)
                        {
                            if (!weapons[i])
                                continue;

                            uint32 unleashSpell = 0;
                            uint32 furySpell = 0;
                            bool hasFuryTalent = _player->HasAura(SPELL_SHA_UNLEASHED_FURY_TALENT);
                            bool hostileTarget = _player->IsValidAttackTarget(target);
                            bool hostileSpell = true;
                            switch (weapons[i]->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                            {
                                case 3345: // Earthliving Weapon
                                    unleashSpell = 73685; // Unleash Life
                                    hostileSpell = false;

                                    if (hasFuryTalent)
                                        furySpell = SPELL_SHA_UNLEASHED_FURY_EARTHLIVING;
                                    break;
                                case 5: // Flametongue Weapon
                                    unleashSpell = 73683; // Unleash Flame

                                    if (hasFuryTalent)
                                        furySpell = SPELL_SHA_UNLEASHED_FURY_FLAMETONGUE;
                                    break;
                                case 2: // Frostbrand Weapon
                                    unleashSpell = 73682; // Unleash Frost

                                    if (hasFuryTalent)
                                        furySpell = SPELL_SHA_UNLEASHED_FURY_FROSTBRAND;
                                    break;
                                case 3021: // Rockbiter Weapon
                                    unleashSpell = 73684; // Unleash Earth

                                    if (hasFuryTalent)
                                        furySpell = SPELL_SHA_UNLEASHED_FURY_ROCKBITER;
                                    break;
                                case 283: // Windfury Weapon
                                    unleashSpell = 73681; // Unleash Wind

                                    if (hasFuryTalent)
                                        furySpell = SPELL_SHA_UNLEASHED_FURY_WINDFURY;
                                    break;
                            }

                            if (hostileSpell && !hostileTarget)
                                return; // don't allow to attack non-hostile targets. TODO: check this before cast

                            if (!hostileSpell && hostileTarget)
                                target = _player;   // heal ourselves instead of the enemy

                            if (unleashSpell)
                                _player->CastSpell(target, unleashSpell, true);

                            if (furySpell)
                                _player->CastSpell(target, furySpell, true);

                            target = GetHitUnit();

                            // If weapons are enchanted by same enchantment, only one should be unleashed
                            if (i == 0 && weapons[0] && weapons[1])
                                if (weapons[0]->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == weapons[1]->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                                    return;
                        }
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_sha_unleash_elements_SpellScript::CheckEnchant);
                OnHit += SpellHitFn(spell_sha_unleash_elements_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_unleash_elements_SpellScript();
        }
};

// Called by Lightning Bolt - 403 and Chain Lightning - 421
// Lightning Bolt (Mastery) - 45284, Chain Lightning - 45297 and Lava Beam (Fire Ascendant) - 114074
// Rolling Thunder - 88764
class spell_sha_rolling_thunder : public SpellScriptLoader
{
    public:
        spell_sha_rolling_thunder() : SpellScriptLoader("spell_sha_rolling_thunder") { }

        class spell_sha_rolling_thunder_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_rolling_thunder_SpellScript)

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        // Glyph of Telluric Currents - 10% for Enhancement
                        if (_player->HasAura(55453) && GetSpellInfo()->Id == 403 && !_player->HasAura(123099) && !_player->HasAura(112858))
                            _player->EnergizeBySpell(_player, 55453, _player->CountPctFromMaxMana(10), POWER_MANA);
                        // 2% for Elemental and Restoration
                        else if (_player->HasAura(55453) && GetSpellInfo()->Id == 403 && (_player->HasAura(123099) || _player->HasAura(112858)))
                            _player->EnergizeBySpell(_player, 55453, _player->CountPctFromMaxMana(2), POWER_MANA);

                        if (roll_chance_i(60) && _player->HasAura(SPELL_SHA_ROLLING_THUNDER_AURA))
                        {
                            if (Aura *lightningShield = _player->GetAura(SPELL_SHA_LIGHTNING_SHIELD_AURA))
                            {
                                _player->CastSpell(_player, SPELL_SHA_ROLLING_THUNDER_ENERGIZE, true);

                                uint8 lsCharges = lightningShield->GetCharges();

                                if (lsCharges < 6)
                                {
                                    uint8 chargesBonus = _player->HasAura(SPELL_SHA_ITEM_T14_4P) ? 2 : 1;
                                    lightningShield->SetCharges(lsCharges + chargesBonus);
                                    lightningShield->RefreshDuration();
                                }
                                else if (lsCharges < 7)
                                {
                                    lightningShield->SetCharges(lsCharges + 1);
                                    lightningShield->RefreshDuration();
                                }

                                // refresh to handle Fulmination visual
                                lsCharges = lightningShield->GetCharges();

                                if (lsCharges >= 7 && _player->HasAura(SPELL_SHA_FULMINATION))
                                    _player->CastSpell(_player, SPELL_SHA_FULMINATION_INFO, true);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_rolling_thunder_SpellScript::HandleOnHit);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_sha_rolling_thunder_SpellScript();
        }
};

// 88766 Fulmination handled in 8042 Earth Shock
class spell_sha_fulmination : public SpellScriptLoader
{
    public:
        spell_sha_fulmination() : SpellScriptLoader("spell_sha_fulmination") { }

        class spell_sha_fulmination_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_fulmination_SpellScript)

            void HandleAfterHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (!GetHitDamage())
                            return;

                        AuraEffect *fulminationAura = _player->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 2010, 0);
                        if (!fulminationAura)
                            return;

                        Aura *lightningShield = _player->GetAura(SPELL_SHA_LIGHTNING_SHIELD_AURA);
                        if (!lightningShield)
                            return;

                        uint8 lsCharges = lightningShield->GetCharges();
                        if (lsCharges <= 1)
                            return;

                        uint8 usedCharges = lsCharges - 1;

                        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_SHA_LIGHTNING_SHIELD_ORB_DAMAGE);
                        if (!spellInfo)
                            return;
                        int32 basePoints = _player->CalculateSpellDamage(target, spellInfo, EFFECT_0);
                        uint32 damage = usedCharges * _player->SpellDamageBonusDone(target, spellInfo, EFFECT_0, basePoints, SPELL_DIRECT_DAMAGE);

                        _player->CastCustomSpell(SPELL_SHA_FULMINATION_TRIGGERED, SPELLVALUE_BASE_POINT0, damage, target, true, NULL, fulminationAura);
                        lightningShield->SetCharges(lsCharges - usedCharges);

                        _player->RemoveAura(SPELL_SHA_FULMINATION_INFO);
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_sha_fulmination_SpellScript::HandleAfterHit);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_sha_fulmination_SpellScript();
        }
};

// Triggered by Flame Shock - 8050
// Lava Surge - 77756
class spell_sha_lava_surge : public SpellScriptLoader
{
    public:
        spell_sha_lava_surge() : SpellScriptLoader("spell_sha_lava_surge") { }

        class spell_sha_lava_surge_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_lava_surge_AuraScript);

            void HandleEffectPeriodic(AuraEffect const * /*aurEff*/)
            {
                // 20% chance to reset the cooldown of Lavaburst and make the next to be instantly casted
                if (GetCaster())
                {
                    if (Player* _player = GetCaster()->ToPlayer())
                    {
                        if (_player->HasAura(77756))
                        {
                            if (roll_chance_i(20))
                            {
                                _player->CastSpell(_player, SPELL_SHA_LAVA_SURGE_CAST_TIME, true);
                                _player->RemoveSpellCooldown(51505, true);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_lava_surge_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_lava_surge_AuraScript();
        }
};

// Healing Stream - 52042
class spell_sha_healing_stream : public SpellScriptLoader
{
    public:
        spell_sha_healing_stream() : SpellScriptLoader("spell_sha_healing_stream") { }

        class spell_sha_healing_stream_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_healing_stream_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHA_HEALING_STREAM))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                if (!GetCaster()->GetOwner())
                    return;

                if (Player* _player = GetCaster()->GetOwner()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        // Glyph of Healing Stream Totem
                        if (target->GetGUID() != _player->GetGUID() && _player->HasAura(SPELL_SHA_GLYPH_OF_HEALING_STREAM_TOTEM))
                            _player->CastSpell(target, SPELL_SHA_GLYPH_OF_HEALING_STREAM, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_healing_stream_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_healing_stream_SpellScript();
        }
};

// Called by Stormstrike - 17364 and Lava Lash - 60103
// Static Shock - 51527
class spell_sha_static_shock : public SpellScriptLoader
{
    public:
        spell_sha_static_shock() : SpellScriptLoader("spell_sha_static_shock") { }

        class spell_sha_static_shock_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_static_shock_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // While have Lightning Shield active
                        if (target && _player->HasAura(SPELL_SHA_LIGHTNING_SHIELD_AURA) && _player->HasAura(51527))
                        {
                            // Item - Shaman T9 Enhancement 2P Bonus (Static Shock)
                            if (_player->HasAura(67220))
                            {
                                if (roll_chance_i(48))
                                {
                                    _player->CastSpell(target, SPELL_SHA_LIGHTNING_SHIELD_ORB_DAMAGE, true);
                                }
                            }
                            else
                            {
                                if (roll_chance_i(45))
                                {
                                    _player->CastSpell(target, SPELL_SHA_LIGHTNING_SHIELD_ORB_DAMAGE, true);
                                }
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_static_shock_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_static_shock_SpellScript();
        }
};

// Elemental Blast - 117014
class spell_sha_elemental_blast : public SpellScriptLoader
{
    public:
        spell_sha_elemental_blast() : SpellScriptLoader("spell_sha_elemental_blast") { }

        class spell_sha_elemental_blast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_elemental_blast_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHA_ELEMENTAL_BLAST))
                    return false;
                return true;
            }

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    _player->CastSpell(_player, SPELL_SHA_ELEMENTAL_BLAST_RATING_BONUS, true);

                    if (Aura* const aura = _player->GetAura(SPELL_SHA_ELEMENTAL_BLAST_RATING_BONUS, _player->GetGUID()))
                    {
                        uint32 randomEffect = 0;

                        if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_SHAMAN_ENHANCEMENT)
                            randomEffect = urand(0, 3);
                        else
                            randomEffect = urand(0, 2);

                        // Iterate over all aura effects
                        for (int i = 0; i < 4; ++i)
                        {
                            if (i == randomEffect)
                                aura->GetEffect(i)->ChangeAmount(3500);
                            else
                            {
                                aura->GetEffect(i)->ChangeAmount(0);
                            }
                        }
                    }

                    if (Unit* target = GetExplTargetUnit())
                    {
                        _player->CastSpell(target, SPELL_SHA_ELEMENTAL_BLAST_FROST_VISUAL, true);
                        _player->CastSpell(target, SPELL_SHA_ELEMENTAL_BLAST_NATURE_VISUAL, true);
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_sha_elemental_blast_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_elemental_blast_SpellScript();
        }
};

// Earthquake : Ticks - 77478
class spell_sha_earthquake_tick : public SpellScriptLoader
{
    public:
        spell_sha_earthquake_tick() : SpellScriptLoader("spell_sha_earthquake_tick") { }

        class spell_sha_earthquake_tick_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_earthquake_tick_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHA_EARTHQUAKE_TICK))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                // With a 10% chance of knocking down affected targets
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (roll_chance_i(10))
                            _player->CastSpell(target, SPELL_SHA_EARTHQUAKE_KNOCKING_DOWN, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_earthquake_tick_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_earthquake_tick_SpellScript();
        }
};

// Earthquake - 61882
class spell_sha_earthquake : public SpellScriptLoader
{
    public:
        spell_sha_earthquake() : SpellScriptLoader("spell_sha_earthquake") { }

        class spell_sha_earthquake_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_earthquake_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (!GetCaster())
                    return;

                if (DynamicObject* dynObj = GetCaster()->GetDynObject(SPELL_SHA_EARTHQUAKE))
                    GetCaster()->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), SPELL_SHA_EARTHQUAKE_TICK, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_earthquake_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_earthquake_AuraScript();
        }
};

// 73920 - Healing Rain
class spell_sha_healing_rain : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

            enum
        {
            SPELL_HEALING_RAIN      = 73920,
            SPELL_HEALING_RAIN_HEAL = 73921
        };

        Position healCastPos;

        bool Load()
        {
            Unit * const caster = GetCaster();
            if (!caster)
                return false;

            DynamicObject const* const obj = caster->GetDynObject(SPELL_HEALING_RAIN);
            if (!obj)
                return false;

            obj->GetPosition(&healCastPos);
            return true;
        }

        void HandlePeriodic(AuraEffect const *)
        {
            GetCaster()->CastSpell(healCastPos.GetPositionX(), healCastPos.GetPositionY(), healCastPos.GetPositionZ(), SPELL_HEALING_RAIN_HEAL, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

public:
    spell_sha_healing_rain()
        : SpellScriptLoader("spell_sha_healing_rain")
    { }

    AuraScript * GetAuraScript() const
    {
        return new script_impl;
    }
};

// Ascendance - 114049
class spell_sha_ascendance : public SpellScriptLoader
{
    public:
        spell_sha_ascendance() : SpellScriptLoader("spell_sha_ascendance") { }

        class spell_sha_ascendance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_ascendance_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHA_ASCENDANCE))
                    return false;
                return true;
            }

            SpellCastResult CheckCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_NONE)
                    {
                        SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_SELECT_TALENT_SPECIAL);
                        return SPELL_FAILED_CUSTOM_ERROR;
                    }

                    return SPELL_CAST_OK;
                }

                return SPELL_CAST_OK;
            }

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    switch(_player->GetSpecializationId(_player->GetActiveSpec()))
                    {
                        case SPEC_SHAMAN_ELEMENTAL:
                            // Remove current spell cooldown
                            _player->RemoveSpellCooldown(51505, true);
                            _player->CastSpell(_player, SPELL_SHA_ASCENDANCE_ELEMENTAL, true);
                            break;
                        case SPEC_SHAMAN_ENHANCEMENT:
                            _player->RemoveSpellCooldown(SPELL_SHA_STORMSTRIKE, true);
                            _player->RemoveSpellCooldown(115356, true);
                            _player->CastSpell(_player, SPELL_SHA_ASCENDANCE_ENHANCED, true);
                            break;
                        case SPEC_SHAMAN_RESTORATION:
                            _player->CastSpell(_player, SPELL_SHA_ASCENDANCE_RESTORATION, true);
                            break;
                        default:
                            break;
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_sha_ascendance_SpellScript::CheckCast);
                AfterCast += SpellCastFn(spell_sha_ascendance_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_ascendance_SpellScript();
        }
};

class EarthenPowerTargetSelector
{
    public:
        EarthenPowerTargetSelector() { }

        bool operator() (WorldObject* target)
        {
            if (!target->ToUnit())
                return true;

            if (!target->ToUnit()->HasAuraWithMechanic(1 << MECHANIC_SNARE))
                return true;

            return false;
        }
};

class spell_sha_bloodlust : public SpellScriptLoader
{
    public:
        spell_sha_bloodlust() : SpellScriptLoader("spell_sha_bloodlust") { }

        class spell_sha_bloodlust_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_bloodlust_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SHAMAN_SPELL_SATED))
                    return false;
                return true;
            }

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, SHAMAN_SPELL_SATED));
                targets.remove_if(Trinity::UnitAuraCheck(true, HUNTER_SPELL_INSANITY));
                targets.remove_if(Trinity::UnitAuraCheck(true, MAGE_SPELL_TEMPORAL_DISPLACEMENT));
            }

            void ApplyDebuff()
            {
                if (Unit* target = GetHitUnit())
                    target->CastSpell(target, SHAMAN_SPELL_SATED, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_bloodlust_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_bloodlust_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_bloodlust_SpellScript::RemoveInvalidTargets, EFFECT_2, TARGET_UNIT_CASTER_AREA_RAID);
                AfterHit += SpellHitFn(spell_sha_bloodlust_SpellScript::ApplyDebuff);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_bloodlust_SpellScript();
        }
};

class spell_sha_heroism : public SpellScriptLoader
{
    public:
        spell_sha_heroism() : SpellScriptLoader("spell_sha_heroism") { }

        class spell_sha_heroism_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_heroism_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SHAMAN_SPELL_EXHAUSTION))
                    return false;
                return true;
            }

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, SHAMAN_SPELL_EXHAUSTION));
                targets.remove_if(Trinity::UnitAuraCheck(true, HUNTER_SPELL_INSANITY));
                targets.remove_if(Trinity::UnitAuraCheck(true, MAGE_SPELL_TEMPORAL_DISPLACEMENT));
            }

            void ApplyDebuff()
            {
                if (Unit* target = GetHitUnit())
                    GetCaster()->CastSpell(target, SHAMAN_SPELL_EXHAUSTION, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_heroism_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_heroism_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_heroism_SpellScript::RemoveInvalidTargets, EFFECT_2, TARGET_UNIT_CASTER_AREA_RAID);
                AfterHit += SpellHitFn(spell_sha_heroism_SpellScript::ApplyDebuff);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_heroism_SpellScript();
        }
};

enum AncestralAwakeningProc
{
    SPELL_ANCESTRAL_AWAKENING_PROC   = 52752,
};

class spell_sha_ancestral_awakening_proc : public SpellScriptLoader
{
    public:
        spell_sha_ancestral_awakening_proc() : SpellScriptLoader("spell_sha_ancestral_awakening_proc") { }

        class spell_sha_ancestral_awakening_proc_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_ancestral_awakening_proc_SpellScript);

            bool Validate(SpellInfo const* /*SpellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ANCESTRAL_AWAKENING_PROC))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                int32 damage = GetEffectValue();
                if (GetCaster() && GetHitUnit())
                    GetCaster()->CastCustomSpell(GetHitUnit(), SPELL_ANCESTRAL_AWAKENING_PROC, &damage, NULL, NULL, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_sha_ancestral_awakening_proc_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_ancestral_awakening_proc_SpellScript();
        }
};

// Lava Lash - 60103
class spell_sha_lava_lash : public SpellScriptLoader
{
    class spell_sha_lava_lash_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sha_lava_lash_SpellScript)

        enum
        {
            SEARING_FLAMES_DAMAGE_DONE = 77661,
            GLYPH_OF_LAVA_LASH = 55444,

        };

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void HandleOnHit()
        {
            Player * const player = GetCaster()->ToPlayer();
            if (!player || !player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                return;

            Unit * const target = GetHitUnit();
            if (!target)
                return;

            int32 hitDamage = GetHitDamage();

            // Damage is increased by 40% if your off-hand weapon is enchanted with Flametongue.
            if (player->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_SHAMAN, Trinity::Flag128(0x200000)))
                AddPct(hitDamage, 40);

            // Unleash Flame increase Lava Lash damage for 30%
            if (player->HasAura(73683))
                AddPct(hitDamage, 30);

            // Your Lava Lash ability will consume Searing Flame effect, dealing
            // 20% increased damage for each application
            if (Aura * const searingFlame = player->GetAura(SEARING_FLAMES_DAMAGE_DONE))
            {
                AddPct(hitDamage, searingFlame->GetStackAmount() * 20);
                player->RemoveAura(searingFlame);
            }

            SetHitDamage(hitDamage);

            // Spreading your Flame shock from the target to up to four enemies
            // within 12 yards. Effect deactivated if has Glyph of Lava Lash

            if (player->HasAura(55444))
                return;

             Aura const * const flameShock = target->GetAura(SPELL_SHA_FLAME_SHOCK, player->GetGUID());
             if (!flameShock)
                 return;

            // Find all the enemies
            std::list<Unit*> targets;
            Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(player, player, 12.0f);
            Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(player, targets, u_check);
            Trinity::VisitNearbyObject(player, 12.0f, searcher);
            // Limit to 4
            int8 count = 0;
            for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
            {
                // Do not check unattackable targets
                if (!(*iter)->isTargetableForAttack() || target->GetGUID() == (*iter)->GetGUID())
                    continue;

                if (++count > 4)
                    return;
                // Hack to initiate attack as AddAura does not do that
                if (Creature * const c = (*iter)->ToCreature())
                    c->AI()->AttackStart(player);

                if (Aura * const newAura = player->AddAura(SPELL_SHA_FLAME_SHOCK, *iter))
                    newAura->SetDuration(flameShock->GetDuration());
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_sha_lava_lash_SpellScript::HandleOnHit);
        }
    };

public:
    spell_sha_lava_lash()
        : SpellScriptLoader("spell_sha_lava_lash")
    { }

    SpellScript * GetSpellScript() const
    {
        return new spell_sha_lava_lash_SpellScript();
    }
};

// Chain Heal - 1064
class spell_sha_chain_heal : public SpellScriptLoader
{
    public:
        spell_sha_chain_heal() : SpellScriptLoader("spell_sha_chain_heal") { }

        class spell_sha_chain_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_chain_heal_SpellScript);

            bool Load()
            {
                firstHeal = true;
                riptide = false;
                amount = 0;
                return true;
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (firstHeal)
                {
                    // Check if the target has Riptide
                    Trinity::Flag128 const flags(0, 0, 0x10);
                    if (AuraEffect *aurEff = GetHitUnit()->GetAuraEffect(SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_SHAMAN, flags, GetCaster()->GetGUID()))
                    {
                        riptide = true;
                        amount = aurEff->GetSpellInfo()->Effects[EFFECT_2].CalcValue();
                    }
                    firstHeal = false;
                }
                // Riptide increases the Chain Heal effect by 25%
                if (riptide)
                {
                    uint32 bonus = CalculatePct(GetHitHeal(), amount);
                    SetHitHeal(GetHitHeal() + bonus);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_sha_chain_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }

            bool firstHeal;
            bool riptide;
            uint32 amount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_chain_heal_SpellScript();
        }
};

// Lava Burst - 51505
class spell_sha_lava_burst : public SpellScriptLoader
{
    class spell_sha_lava_burst_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sha_lava_burst_SpellScript)

        enum
        {
            SPELL_FLAME_SHOCK       = 8050,
            SPELL_LAVA_SURGE_PROC   = 77762,
        };

        void HandleAfterHit()
        {
            Unit * const caster = GetCaster();
            if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            Spell::UsedSpellMods const &mods = appliedSpellMods();
            AuraEffect * const aurEff = caster->GetAuraEffect(SPELL_LAVA_SURGE_PROC, EFFECT_0);

            if (!aurEff || mods.find(aurEff->GetSpellModifier()) != mods.end())
                return;

            caster->ToPlayer()->RemoveSpellCooldown(GetSpellInfo()->Id, true);
        }

        void HandleOnHit(SpellEffIndex /*effIndex*/)
        {
            Unit * const target = GetHitUnit();
            if (!target || !target->HasAura(SPELL_FLAME_SHOCK, GetCaster()->GetGUID()))
                return;

            SetHitDamage(GetHitDamage() * 1.5f);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_sha_lava_burst_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            AfterHit += SpellHitFn(spell_sha_lava_burst_SpellScript::HandleAfterHit);
        }
    };

public:
    spell_sha_lava_burst() : SpellScriptLoader("spell_sha_lava_burst")
    { }

    SpellScript * GetSpellScript() const
    {
        return new spell_sha_lava_burst_SpellScript();
    }
};

// Lava Burst (Mastery Proc) - 77451
class spell_sha_lava_burst_mastery_proc : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            SPELL_FLAME_SHOCK = 8050,
        };

        void HandleOnHit(SpellEffIndex /*effIndex*/)
        {
            Unit * const target = GetHitUnit();
            if (target && target->HasAura(SPELL_FLAME_SHOCK, GetCaster()->GetGUID()))
                SetHitDamage(GetHitDamage() * 1.5f);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

public:
    spell_sha_lava_burst_mastery_proc() : SpellScriptLoader("spell_sha_lava_burst_mastery_proc") { }

    SpellScript * GetSpellScript() const { return new script_impl(); }
};

// 108287 - Totemic Projection
class spell_sha_totemic_projection : public SpellScriptLoader
{
public:
    spell_sha_totemic_projection() : SpellScriptLoader("spell_sha_totemic_projection") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        void ChangeSummonPos(SpellEffIndex /*effIndex*/)
        {
            auto player = GetCaster()->ToPlayer();
            if (!player)
                return;

            for (int32 i = 0; i < 4; ++i)
            {
                Position pos;
                GetHitDest()->GetPosition(&pos);
                Creature* totem = player->GetMap()->GetCreature(player->m_SummonSlot[SUMMON_SLOT_TOTEM + i]);
                if (totem && totem->isTotem())
                {
                    uint32 spell_id = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id);
                    if (!spellInfo)
                        return;

                    auto effectInfo = spellInfo->Effects[EFFECT_0];
                    auto targetInfo = SpellImplicitTargetInfo(effectInfo.TargetA);
                    float angle = targetInfo.CalcDirectionAngle();
                    float objSize = player->GetObjectSize();
                    float dist = effectInfo.CalcRadius(player);
                    if (dist < objSize)
                        dist = objSize;

                    totem->MovePositionToFirstCollision(pos, dist, angle);
                    totem->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), totem->GetOrientation());
                }
            }
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(script_impl::ChangeSummonPos, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Conductivity - 108282
class spell_sha_conductivity : public SpellScriptLoader
{
public:
    spell_sha_conductivity() : SpellScriptLoader("spell_sha_conductivity") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            SPELL_SHAMAN_HEALING_RAIN_AURA = 73920
        };

        void Conductivity()
        {
            if (Aura* aura = GetCaster()->GetAura(SPELL_SHAMAN_HEALING_RAIN_AURA))
            {
                if (aura->GetMaxDuration() + 4 * IN_MILLISECONDS < 50 * IN_MILLISECONDS)
                {
                    aura->SetMaxDuration(std::min(aura->GetMaxDuration() + 4 * IN_MILLISECONDS, 50 * IN_MILLISECONDS));
                    aura->SetDuration(aura->GetDuration() + 4 * IN_MILLISECONDS);
                    if (DynamicObject* rain = GetCaster()->GetDynObject(SPELL_SHAMAN_HEALING_RAIN_AURA))
                        rain->SetDuration(std::min(rain->GetDuration() + 4 * IN_MILLISECONDS, 50 * IN_MILLISECONDS));

                    aura->SetNeedClientUpdateForTargets();
                }
            }
        }

        void Register()
        {
            OnCast += SpellCastFn(script_impl::Conductivity);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// 52039 - healing stream heal
class spell_sha_healing_stream_totem : public SpellScriptLoader
{
    public:
        spell_sha_healing_stream_totem() : SpellScriptLoader("spell_sha_healing_stream_totem") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            enum
            {
                SPELL_SHAMAN_RUSHING_STREAMS = 147074
            };

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                std::list<Unit*> _targets;
                for (auto itr: targets)
                {
                    if (GetCaster()->IsInRaidWith(itr->ToUnit()) || GetCaster()->IsInPartyWith(itr->ToUnit()))
                        _targets.push_back(itr->ToUnit());
                }

                targets.clear();

                if (_targets.empty())
                    return;

                _targets.sort(Trinity::HealthPctOrderPred(false));
                
                std::list<WorldObject*> _new;
                if (Unit* caster = GetCaster()->GetCharmerOrOwner())
                {
                    if (caster->HasAura(SPELL_SHAMAN_RUSHING_STREAMS))
                    {
                        for (uint8 x = 0; x < 2; x++)
                        {
                            if (_targets.empty())
                                continue;

                            _new.push_back(_targets.front());
                            _targets.remove(_targets.front());
                        }
                    }
                }

                _targets.clear();
                targets = _new;
                _new.clear();
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

// Glyph Rain of Frogs - 147707
class spell_sha_glyph_of_frogs : public SpellScriptLoader
{
    public:
        spell_sha_glyph_of_frogs() : SpellScriptLoader("spell_sha_glyph_of_frogs") { }

        class spell_sha_glyph_of_frogs_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_glyph_of_frogs_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(SPELL_SHA_RAIN_OF_FROGS, false);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(SPELL_SHA_RAIN_OF_FROGS))
                        _player->removeSpell(SPELL_SHA_RAIN_OF_FROGS, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_sha_glyph_of_frogs_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_sha_glyph_of_frogs_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_glyph_of_frogs_AuraScript();
        }
};

void AddSC_shaman_spell_scripts()
{
    new spell_sha_conductivity();
    new spell_sha_hex();
    new spell_sha_water_ascendant();
    new spell_sha_prowl();
    new spell_sha_glyph_of_shamanistic_rage();
    new spell_sha_glyph_of_lakestrider();
    new spell_sha_call_of_the_elements();
    new spell_sha_ancestral_guidance();
    new spell_sha_echo_of_the_elements();
    new spell_sha_earthgrab();
    new spell_sha_stone_bulwark();
    new spell_sha_mail_specialization();
    new spell_sha_frozen_power();
    new spell_sha_spirit_link();
    new spell_sha_mana_tide();
    new spell_sha_tidal_waves();
    new spell_sha_fire_nova();
    new spell_sha_unleash_elements();
    new spell_sha_rolling_thunder();
    new spell_sha_fulmination();
    new spell_sha_lava_surge();
    new spell_sha_healing_stream();
    new spell_sha_static_shock();
    new spell_sha_elemental_blast();
    new spell_sha_earthquake_tick();
    new spell_sha_earthquake();
    new spell_sha_healing_rain();
    new spell_sha_ascendance();
    new spell_sha_bloodlust();
    new spell_sha_heroism();
    new spell_sha_ancestral_awakening_proc();
    new spell_sha_lava_lash();
    new spell_sha_chain_heal();
    new spell_sha_lava_burst();
    new spell_sha_lava_burst_mastery_proc();
    new spell_sha_totemic_projection();
	new spell_sha_glyph_of_frogs();
}
