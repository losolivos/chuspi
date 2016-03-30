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
 * Scripts for spells with SPELLFAMILY_PALADIN and SPELLFAMILY_GENERIC spells used by paladin players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pal_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"

enum PaladinSpells
{
    PALADIN_SPELL_DIVINE_PLEA                    = 54428,
    PALADIN_SPELL_JUDGMENT                       = 20271,
    PALADIN_SPELL_JUDGMENTS_OF_THE_BOLD          = 111529,
    PALADIN_SPELL_JUDGMENTS_OF_THE_WISE          = 105424,
    PALADIN_SPELL_PHYSICAL_VULNERABILITY         = 81326,
    PALADIN_SPELL_LONG_ARM_OF_THE_LAW            = 87172,
    PALADIN_SPELL_LONG_ARM_OF_THE_LAW_RUN_SPEED  = 87173,
    PALADIN_SPELL_BURDEN_OF_GUILT                = 54931,
    PALADIN_SPELL_BURDEN_OF_GUILT_DECREASE_SPEED = 110300,
    PALADIN_SPELL_HOLY_SHOCK_R1                  = 20473,
    PALADIN_SPELL_HOLY_SHOCK_R1_DAMAGE           = 25912,
    PALADIN_SPELL_HOLY_SHOCK_R1_HEALING          = 25914,
    SPELL_BLESSING_OF_LOWER_CITY_DRUID           = 37878,
    SPELL_BLESSING_OF_LOWER_CITY_PALADIN         = 37879,
    SPELL_BLESSING_OF_LOWER_CITY_PRIEST          = 37880,
    SPELL_BLESSING_OF_LOWER_CITY_SHAMAN          = 37881,
    SPELL_DIVINE_STORM                           = 53385,
    SPELL_DIVINE_STORM_DUMMY                     = 54171,
    SPELL_DIVINE_STORM_HEAL                      = 54172,
    SPELL_FORBEARANCE                            = 25771,
    PALADIN_SPELL_WORD_OF_GLORY                  = 85673,
    PALADIN_SPELL_WORD_OF_GLORY_HEAL             = 130551,
    PALADIN_SPELL_WORD_OF_GLORY_DAMAGE           = 150552,
    PALADIN_SPELL_GLYPH_OF_WORD_OF_GLORY         = 54936,
    PALADIN_SPELL_GLYPH_OF_WORD_OF_GLORY_DAMAGE  = 115522,
    PALADIN_SPELL_CONSECRATION_AREA_DUMMY        = 81298,
    PALADIN_SPELL_CONSECRATION_DAMAGE            = 81297,
    PALADIN_SPELL_HOLY_PRISM_ALLIES              = 114871,
    PALADIN_SPELL_HOLY_PRISM_ENNEMIES            = 114852,
    PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL       = 114862,
    PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL_2     = 114870,
    PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL         = 121551,
    PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL_2       = 121552,
    PALADIN_SPELL_ARCING_LIGHT_HEAL              = 119952,
    PALADIN_SPELL_ARCING_LIGHT_DAMAGE            = 114919,
    PALADIN_SPELL_INQUISITION                    = 84963,
    PALADIN_SPELL_GLYPH_OF_BLINDING_LIGHT        = 54934,
    PALADIN_SPELL_BLINDING_LIGHT_CONFUSE         = 105421,
    PALADIN_SPELL_BLINDING_LIGHT_STUN            = 115752,
    PALADIN_SPELL_EXORCISM                       = 879,
    PALADIN_SPELL_MASS_EXORCISM                  = 122032,
    PALADIN_SPELL_SACRED_SHIELD                  = 65148,
    PALADIN_SPELL_ARDENT_DEFENDER_HEAL           = 66235,
    PALADIN_SPELL_TOWER_OF_RADIANCE_ENERGIZE     = 88852,
    PALADIN_SPELL_BEACON_OF_LIGHT                = 53563,
    PALADIN_SPELL_SELFLESS_HEALER_STACK          = 114250,
    PALADIN_SPELL_ETERNAL_FLAME                  = 114163,
    PALADIN_SPELL_SHIELD_OF_THE_RIGHTEOUS_PROC   = 132403,
    PALADIN_SPELL_BASTION_OF_GLORY               = 114637,
    PALADIN_SPELL_DIVINE_PURPOSE                 = 90174,
    PALADIN_SPELL_DIVINE_SHIELD                  = 642,
    PALADIN_SPELL_LAY_ON_HANDS                   = 633,
    PALADIN_SPELL_DIVINE_PROTECTION              = 498,
    PALADIN_SPELL_GLYPH_OF_FALLING_AVENGER       = 115931,
    PALADIN_SPELL_SACRED_CLEANSING               = 53551,
    PALADIN_SPELL_DAYBREAK_AURA                  = 88821,
    PALADIN_SPELL_DAYBREAK_PROC                  = 88819,
    PALADIN_SPELL_DAYBREAK_HEAL                  = 121129,
    PALADIN_SPELL_TOWER_OF_RADIANCE              = 85512,
    PALADIN_SPELL_HOLY_AVENGER                   = 105809,
    PALADIN_ITEM_PVP_HOLY_4P_BONUS               = 131665,
	PALADIN_SPELL_GLYPH_OF_CONTEMPLATION         = 121183,
    PALADIN_SPELL_GLYPH_OF_DIVINE_STORM          = 63220,
    PALADIN_SPELL_GLYPH_OF_DIVINE_STORM_HEAL     = 115515,
    PALADIN_SPELL_WEAKENED_BLOWS                 = 115798,
};

class spell_pal_glyph_of_devotian_aura : public SpellScriptLoader
{
    public:
        spell_pal_glyph_of_devotian_aura() : SpellScriptLoader("spell_pal_glyph_of_devotian_aura") { }

        class spell_pal_glyph_of_devotian_aura_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_glyph_of_devotian_aura_SpellScript);

            bool Load()
            {
                return true;
            }

            void CountTargets(std::list<WorldObject*>& targets)
            {
                if (GetCaster() && GetCaster()->HasAura(146955))
                {
                    targets.clear();
                    targets.push_back(GetCaster());
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_glyph_of_devotian_aura_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_glyph_of_devotian_aura_SpellScript();
        }
};

class spell_pal_glyph_of_devotian_trigger_aura : public SpellScriptLoader
{
    public:
        spell_pal_glyph_of_devotian_trigger_aura() : SpellScriptLoader("spell_pal_glyph_of_devotian_trigger_aura") { }

        class spell_pal_glyph_of_devotian_aura_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_glyph_of_devotian_aura_SpellScript);

            bool Load()
            {
                return true;
            }

            void CountTargets(std::list<WorldObject*>& targets)
            {
                if (GetCaster() && GetCaster()->HasAura(146955))
                {
                    targets.clear();
                    targets.push_back(GetCaster());
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_glyph_of_devotian_aura_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_glyph_of_devotian_aura_SpellScript::CountTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_glyph_of_devotian_aura_SpellScript::CountTargets, EFFECT_2, TARGET_UNIT_CASTER_AREA_RAID);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_glyph_of_devotian_aura_SpellScript();
        }
};

// Called by Divine Storm - 53385
// Glyph of Divine Storm - 63220
class spell_pal_glyph_of_divine_storm : public SpellScriptLoader
{
    public:
        spell_pal_glyph_of_divine_storm() : SpellScriptLoader("spell_pal_glyph_of_divine_storm") { }

        class spell_pal_glyph_of_divine_storm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_glyph_of_divine_storm_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PALADIN_SPELL_GLYPH_OF_DIVINE_STORM))
                        _player->CastSpell(_player, PALADIN_SPELL_GLYPH_OF_DIVINE_STORM_HEAL, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_pal_glyph_of_divine_storm_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_glyph_of_divine_storm_SpellScript();
        }
};

// Holy Radiance - 82327
class spell_pal_daybreak : public SpellScriptLoader
{
    public:
        spell_pal_daybreak() : SpellScriptLoader("spell_pal_daybreak") { }

        class spell_pal_daybreak_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_daybreak_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PALADIN_SPELL_DAYBREAK_AURA))
                        _player->CastSpell(_player, PALADIN_SPELL_DAYBREAK_PROC, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_pal_daybreak_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_daybreak_SpellScript();
        }
};

class spell_pal_daybreak_heal : public SpellScriptLoader
{
    public:
        spell_pal_daybreak_heal() : SpellScriptLoader("spell_pal_daybreak_heal") { }

        class spell_pal_daybreak_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_daybreak_heal_SpellScript);

            bool Load()
            {
                _targets = 0;
                return true;
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                SetHitHeal(GetHitHeal() / _targets);
            }

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove(GetExplTargetUnit());
                _targets = unitList.size();
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_daybreak_heal_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
                OnEffectHitTarget += SpellEffectFn(spell_pal_daybreak_heal_SpellScript::HandleHeal, EFFECT_1, SPELL_EFFECT_HEAL);
            }

            uint32 _targets;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_daybreak_heal_SpellScript();
        }
};

// Hand of Purity - 114039
class spell_pal_hand_of_purity : public SpellScriptLoader
{
    public:
        spell_pal_hand_of_purity() : SpellScriptLoader("spell_pal_hand_of_purity") { }

        class spell_pal_hand_of_purity_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_hand_of_purity_AuraScript);

            uint32 healtPct;

            bool Load()
            {
                healtPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return GetUnitOwner()->ToPlayer();
            }

            void CalculateAmount(AuraEffect const * /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect * /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                SpellInfo const* procSpell = dmgInfo.GetSpellInfo();
                if (!procSpell)
                    return;

                if (!procSpell->HasAura(SPELL_AURA_PERIODIC_DAMAGE) && !procSpell->HasAura(SPELL_AURA_PERIODIC_DAMAGE_PERCENT))
                    return;

                absorbAmount = CalculatePct(dmgInfo.GetDamage(), healtPct);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_hand_of_purity_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_hand_of_purity_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_hand_of_purity_AuraScript();
        }
};

// Avenging Wrath - 31884
// Glyph of Avenging Wrath - 54927
class spell_pal_glyph_of_avenging_wrath final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        void initEffects(uint32 &effectMask)
        {
            auto const caster = GetCaster();
            if (!caster || !caster->HasAura(PALADIN_SPELL_GLYPH_OF_FALLING_AVENGER))
                effectMask &= ~(1 << EFFECT_3);
        }

        void Register() final
        {
            OnInitEffects += AuraInitEffectsFn(script_impl::initEffects);
        }
    };

public:
    spell_pal_glyph_of_avenging_wrath()
        : SpellScriptLoader("spell_pal_glyph_of_avenging_wrath")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Shield of the Righteous - 53600
class spell_pal_shield_of_the_righteous : public SpellScriptLoader
{
    public:
        spell_pal_shield_of_the_righteous() : SpellScriptLoader("spell_pal_shield_of_the_righteous") { }

        class spell_pal_shield_of_the_righteous_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_shield_of_the_righteous_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        // -30% damage taken for 3s
                        if (auto proc = _player->GetAura(PALADIN_SPELL_SHIELD_OF_THE_RIGHTEOUS_PROC, _player->GetGUID()))
                        {
                            auto duration = proc->GetDuration();
                            proc->SetDuration(duration + 3 * IN_MILLISECONDS);
                            proc->SetMaxDuration(duration + 3 * IN_MILLISECONDS);
                        }
                        else
                            _player->CastSpell(_player, PALADIN_SPELL_SHIELD_OF_THE_RIGHTEOUS_PROC, true);
                        
                        _player->CastSpell(_player, PALADIN_SPELL_BASTION_OF_GLORY, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_shield_of_the_righteous_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_shield_of_the_righteous_SpellScript();
        }
};

// Called by Flash of Light - 19750 and Divine Light - 82326
// Tower of Radiance - 85512
class spell_pal_tower_of_radiance : public SpellScriptLoader
{
    public:
        spell_pal_tower_of_radiance() : SpellScriptLoader("spell_pal_tower_of_radiance") { }

        class spell_pal_tower_of_radiance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_tower_of_radiance_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(PALADIN_SPELL_TOWER_OF_RADIANCE))
                            if (target->HasAura(PALADIN_SPELL_BEACON_OF_LIGHT, _player->GetGUID()))
                                _player->EnergizeBySpell(_player, PALADIN_SPELL_TOWER_OF_RADIANCE_ENERGIZE, 1, POWER_HOLY_POWER);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_tower_of_radiance_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_tower_of_radiance_SpellScript();
        }
};

// Sacred shield - 20925
class spell_pal_sacred_shield : public SpellScriptLoader
{
    public:
        spell_pal_sacred_shield() : SpellScriptLoader("spell_pal_sacred_shield") { }

        class spell_pal_sacred_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_sacred_shield_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (Unit* _player = GetCaster())
                    if (Unit* target = GetTarget())
                            _player->CastSpell(target, PALADIN_SPELL_SACRED_SHIELD, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_sacred_shield_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_sacred_shield_AuraScript();
        }
};

// Sacred shield absorb - 65148
class spell_pal_sacred_shield_absorb : public SpellScriptLoader
{
    public:
        spell_pal_sacred_shield_absorb() : SpellScriptLoader("spell_pal_sacred_shield_absorb") { }

        class spell_pal_sacred_shield_absorb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_sacred_shield_absorb_AuraScript);

            void CalculateAmount(AuraEffect const *, int32 & amount, bool & )
            {
                if (GetCaster())
                    amount = int32(30 + GetCaster()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY) * 1.17f);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_sacred_shield_absorb_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_sacred_shield_absorb_AuraScript();
        }
};

// Emancipate - 121783
class spell_pal_emancipate : public SpellScriptLoader
{
    public:
        spell_pal_emancipate() : SpellScriptLoader("spell_pal_emancipate") { }

        class spell_pal_emancipate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_emancipate_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    std::list<Aura*> auraList;

                    for (auto itr : _player->GetAppliedAuras())
                    {
                        Aura *aura = itr.second->GetBase();
                        if (aura && aura->GetSpellInfo()->GetAllEffectsMechanicMask() & ((1<<MECHANIC_SNARE)|(1<<MECHANIC_ROOT)))
                            auraList.push_back(aura);
                    }

                    if (!auraList.empty())
                    {
                        Trinity::Containers::RandomResizeList(auraList, 1);
                        _player->RemoveAura(*auraList.begin());
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_emancipate_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_emancipate_SpellScript();
        }
};

// Art of War - 59578
class spell_pal_art_of_war : public SpellScriptLoader
{
    public:
        spell_pal_art_of_war() : SpellScriptLoader("spell_pal_art_of_war") { }

        class spell_pal_art_of_war_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_art_of_war_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasSpellCooldown(PALADIN_SPELL_EXORCISM))
                        _player->RemoveSpellCooldown(PALADIN_SPELL_EXORCISM, true);

                    // Glyph of Mass Exorcism
                    if (_player->HasSpellCooldown(PALADIN_SPELL_MASS_EXORCISM))
                        _player->RemoveSpellCooldown(PALADIN_SPELL_MASS_EXORCISM, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_art_of_war_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_art_of_war_SpellScript();
        }
};

// Seal of Insight - 20167
class spell_pal_seal_of_insight : public SpellScriptLoader
{
    public:
        spell_pal_seal_of_insight() : SpellScriptLoader("spell_pal_seal_of_insight") { }

        class spell_pal_seal_of_insight_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_seal_of_insight_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (GetHitUnit())
                        player->EnergizeBySpell(player, GetSpellInfo()->Id, int32(player->GetMaxPower(POWER_MANA) * 0.04), POWER_MANA);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_seal_of_insight_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_seal_of_insight_SpellScript();
        }
};

// Blinding Light - 115750
class spell_pal_blinding_light : public SpellScriptLoader
{
    public:
        spell_pal_blinding_light() : SpellScriptLoader("spell_pal_blinding_light") { }

        class spell_pal_blinding_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_blinding_light_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(PALADIN_SPELL_GLYPH_OF_BLINDING_LIGHT))
                            _player->CastSpell(target, PALADIN_SPELL_BLINDING_LIGHT_STUN, true);
                        else
                            _player->CastSpell(target, PALADIN_SPELL_BLINDING_LIGHT_CONFUSE, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_blinding_light_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_blinding_light_SpellScript();
        }
};

// Hand of Protection - 1022
class spell_pal_hand_of_protection : public SpellScriptLoader
{
    public:
        spell_pal_hand_of_protection() : SpellScriptLoader("spell_pal_hand_of_protection") { }

        class spell_pal_hand_of_protection_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_hand_of_protection_SpellScript);

            SpellCastResult CheckForbearance()
            {
                if (Unit* target = GetExplTargetUnit())
                    if (target->HasAura(SPELL_FORBEARANCE))
                        return SPELL_FAILED_TARGET_AURASTATE;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, SPELL_FORBEARANCE, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_hand_of_protection_SpellScript::CheckForbearance);
                OnHit += SpellHitFn(spell_pal_hand_of_protection_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_hand_of_protection_SpellScript();
        }
};

// Divine Shield - 642 and Divine Shield - 110700
class spell_pal_divine_shield : public SpellScriptLoader
{
    public:
        spell_pal_divine_shield() : SpellScriptLoader("spell_pal_divine_shield") { }

        class spell_pal_divine_shield_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_shield_SpellScript);

            SpellCastResult CheckForbearance()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_FORBEARANCE))
                        return SPELL_FAILED_TARGET_AURASTATE;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, SPELL_FORBEARANCE, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_divine_shield_SpellScript::CheckForbearance);
                OnHit += SpellHitFn(spell_pal_divine_shield_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_divine_shield_SpellScript();
        }
};

// Inquisition - 84963
class spell_pal_inquisition : public SpellScriptLoader
{
    public:
        spell_pal_inquisition() : SpellScriptLoader("spell_pal_inquisition") { }

        class spell_pal_inquisition_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_inquisition_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Aura *inquisition = _player->GetAura(PALADIN_SPELL_INQUISITION))
                    {
                        int32 holyPower = _player->GetPower(POWER_HOLY_POWER);

                        if (holyPower > 2)
                            holyPower = 2;

                        if (_player->HasAura(PALADIN_SPELL_DIVINE_PURPOSE))
                            holyPower = 2;

                        int32 maxDuration = inquisition->GetMaxDuration();
                        int32 newDuration = inquisition->GetDuration() + maxDuration * holyPower;
                        inquisition->SetDuration(newDuration);

                        if (newDuration > maxDuration)
                            inquisition->SetMaxDuration(newDuration);

                        if (!_player->HasAura(PALADIN_SPELL_DIVINE_PURPOSE))
                            _player->SetPower(POWER_HOLY_POWER, _player->GetPower(POWER_HOLY_POWER) - holyPower);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_inquisition_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_inquisition_SpellScript();
        }
};

// Execution Sentence - 114157
class spell_pal_execution_sentence final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            EXECUTION_SENTENCE    = 114916,
            STAY_OF_EXECUTION     = 114917,
        };

        void effectHitTarget(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            auto const caster = GetCaster();
            auto const target = GetHitUnit();

            if (!caster || !target)
                return;

            auto const spellId = caster->IsFriendlyTo(target)
                    ? STAY_OF_EXECUTION
                    : EXECUTION_SENTENCE;

            caster->CastSpell(target, spellId, true);
        }

        void Register() final
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::effectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

public:
    spell_pal_execution_sentence()
        : SpellScriptLoader("spell_pal_execution_sentence")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

class spell_pal_execution_sentence_effect final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            EXECUTION_SENTENCE_DISPEL_DAMAGE = 146585,
            EXECUTION_SENTENCE_DISPEL_HEAL   = 146586
        };

        AuraType auraType_;

    public:
        script_impl(AuraType auraType)
            : auraType_(auraType)
        { }

    private:
        void effectCalcAmount(AuraEffect const *, int32 &amount, bool &canBeRecalculated)
        {
            canBeRecalculated = false;

            auto const caster = GetCaster();
            if (!caster)
                return;

            auto const sph = caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);
            auto const fromDummy = GetSpellInfo()->Effects[EFFECT_1].CalcValue(caster);

            // Formula is taken from description in DBC
            amount = sph * fromDummy / 1000.0f + 26.72716306f * amount;
        }

        void onRemove(AuraEffect const *aurEff, AuraEffectHandleModes)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
                return;

            auto const caster = GetCaster();
            auto const target = GetTarget();

            if (!caster || !target)
                return;

            auto const spellId = (auraType_ == SPELL_AURA_PERIODIC_DAMAGE)
                    ? EXECUTION_SENTENCE_DISPEL_DAMAGE
                    : EXECUTION_SENTENCE_DISPEL_HEAL;

            int32 const bp = aurEff->GetFixedDamageInfo().GetFixedDamage() * 0.44125f;
            caster->CastCustomSpell(target, spellId, &bp, nullptr, nullptr, true);
        }

        void Register() final
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::effectCalcAmount, EFFECT_0, auraType_);
            OnEffectRemove += AuraEffectRemoveFn(script_impl::onRemove, EFFECT_0, auraType_, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraType auraType_;

public:
    spell_pal_execution_sentence_effect(char const *scriptName, AuraType auraType)
        : SpellScriptLoader(scriptName)
        , auraType_(auraType)
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl(auraType_);
    }
};

// Light's Hammer (periodic dummy for npc) - 114918
class spell_pal_lights_hammer : public SpellScriptLoader
{
    public:
        spell_pal_lights_hammer() : SpellScriptLoader("spell_pal_lights_hammer") { }

        class spell_pal_lights_hammer_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_lights_hammer_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster() && GetCaster()->GetOwner())
                {
                    GetCaster()->CastSpell(GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ(), PALADIN_SPELL_ARCING_LIGHT_HEAL, true, 0, NULL, GetCaster()->GetOwnerGUID());
                    GetCaster()->CastSpell(GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ(), PALADIN_SPELL_ARCING_LIGHT_DAMAGE, true, 0, NULL, GetCaster()->GetOwnerGUID());
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_lights_hammer_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_lights_hammer_AuraScript();
        }
};

// called by Holy Prism (damage) - 114852 or Holy Prism (heal) - 114871
// Holy Prism visual for other targets
class spell_pal_holy_prism_visual : public SpellScriptLoader
{
    public:
        spell_pal_holy_prism_visual() : SpellScriptLoader("spell_pal_holy_prism_visual") { }

        class spell_pal_holy_prism_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_prism_visual_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->IsValidAttackTarget(target))
                        {
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL_2, true);
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL_2, true);
                        }
                        else
                        {
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL_2, true);
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL_2, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_holy_prism_visual_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_holy_prism_visual_SpellScript();
        }
};

// called by Holy Prism (visual damage) - 114862 or Holy Prism (visual heal) - 121551
// Holy Prism (damage) - 114852 or Holy Prism (heal) - 114871
class spell_pal_holy_prism_effect : public SpellScriptLoader
{
    public:
        spell_pal_holy_prism_effect() : SpellScriptLoader("spell_pal_holy_prism_effect") { }

        class spell_pal_holy_prism_effect_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_prism_effect_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // damage
                        if (GetSpellInfo()->Id == 114862)
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_ENNEMIES, true);
                        // heal
                        else if (GetSpellInfo()->Id == 121551)
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_ALLIES, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_holy_prism_effect_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_holy_prism_effect_SpellScript();
        }
};

// Holy Prism - 114165
class spell_pal_holy_prism : public SpellScriptLoader
{
    public:
        spell_pal_holy_prism() : SpellScriptLoader("spell_pal_holy_prism") { }

        class spell_pal_holy_prism_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_prism_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->IsValidAttackTarget(target))
                        {
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL, true);
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL_2, true);
                        }
                        else
                        {
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL, true);
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL_2, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_holy_prism_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_holy_prism_SpellScript();
        }
};

// Consecration - 26573, 116467 (periodic dummy)
class spell_pal_consecration : public SpellScriptLoader
{
    public:
        spell_pal_consecration() : SpellScriptLoader("spell_pal_consecration") { }

        class spell_pal_consecration_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_consecration_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                auto const caster = GetCaster();
                if (!caster)
                    return;

                if (DynamicObject* dynObj = caster->GetDynObject(PALADIN_SPELL_CONSECRATION_AREA_DUMMY))
                    caster->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), PALADIN_SPELL_CONSECRATION_DAMAGE, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_consecration_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_consecration_AuraScript();
        }
};

// Consecration - 26573, 116467
class spell_pal_consecration_area : public SpellScriptLoader
{
    public:
        spell_pal_consecration_area() : SpellScriptLoader("spell_pal_consecration_area") { }

        class spell_pal_consecration_area_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_consecration_area_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    Position pos;
                    _player->GetPosition(&pos);
                    if (GetSpellInfo()->Id == 116467) // Glyphed
                        GetExplTargetDest()->GetPosition(&pos);

                    _player->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), PALADIN_SPELL_CONSECRATION_AREA_DUMMY, true);
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_pal_consecration_area_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_consecration_area_SpellScript();
        }
};

// Word of Glory - 85673
class spell_pal_word_of_glory : public SpellScriptLoader
{
    public:
        spell_pal_word_of_glory() : SpellScriptLoader("spell_pal_word_of_glory") { }

        class spell_pal_word_of_glory_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_word_of_glory_SpellScript);

            bool Validate(SpellInfo const *)
            {
                if (!sSpellMgr->GetSpellInfo(PALADIN_SPELL_WORD_OF_GLORY))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        if ((unitTarget->GetTypeId() != TYPEID_PLAYER && !unitTarget->isPet()) || unitTarget->IsHostileTo(_player))
                            unitTarget = _player;

                        int32 holyPower = _player->GetPower(POWER_HOLY_POWER);

                        if (holyPower > 2)
                            holyPower = 2;

                        _player->CastSpell(unitTarget, PALADIN_SPELL_WORD_OF_GLORY_HEAL, true);

                        if (_player->HasAura(PALADIN_SPELL_GLYPH_OF_WORD_OF_GLORY))
                        {
                            Aura *aura = _player->AddAura(PALADIN_SPELL_GLYPH_OF_WORD_OF_GLORY_DAMAGE, _player);

                            if (aura)
                            {
                                aura->GetEffect(0)->ChangeAmount(aura->GetEffect(0)->GetAmount() * (holyPower + 1));
                                aura->SetNeedClientUpdateForTargets();
                            }
                        }

                        if (!_player->HasAura(PALADIN_SPELL_DIVINE_PURPOSE))
                            _player->ModifyPower(POWER_HOLY_POWER, -holyPower);

                        // Item - Paladin PvP Set Holy 4P Bonus
                        if (_player->HasAura(PALADIN_ITEM_PVP_HOLY_4P_BONUS) && holyPower == 2)
                            _player->ModifyPower(POWER_HOLY_POWER, 1);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_word_of_glory_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_word_of_glory_SpellScript();
        }
};

// Judgment - 20271
class spell_pal_judgment : public SpellScriptLoader
{
    public:
        spell_pal_judgment() : SpellScriptLoader("spell_pal_judgment") { }

        class spell_pal_judgment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_judgment_SpellScript);

            bool Validate(SpellInfo const *)
            {
                if (!sSpellMgr->GetSpellInfo(PALADIN_SPELL_JUDGMENT))
                    return false;
                return true;
            }

            void HandleHit(SpellEffIndex /*effIndex*/)
            {
                if (auto _player = GetCaster()->ToPlayer())
                {
                    if (auto unitTarget = GetHitUnit())
                    {
                        // Selfless Healer in Holy Spec
                        if (_player->HasAura(85804) && _player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_PALADIN_HOLY)
                            _player->CastSpell(_player, 148502, true);

                        if (_player->HasAura(PALADIN_SPELL_JUDGMENTS_OF_THE_BOLD) || _player->HasAura(PALADIN_SPELL_JUDGMENTS_OF_THE_WISE))
                        {
                            int32 power = 1;

                            // Sanctified Wrath with Protection Spec
                            if (_player->HasAura(114232) && _player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_PALADIN_PROTECTION)
                                power = 2;

                            if (_player->HasAura(PALADIN_SPELL_HOLY_AVENGER))
                                power = 3;

                            _player->SetPower(POWER_HOLY_POWER, _player->GetPower(POWER_HOLY_POWER) + power);
                        }

                        if (_player->HasAura(PALADIN_SPELL_JUDGMENTS_OF_THE_BOLD))
                            _player->CastSpell(unitTarget, PALADIN_SPELL_PHYSICAL_VULNERABILITY, true);

                        if (_player->HasAura(PALADIN_SPELL_LONG_ARM_OF_THE_LAW))
                            _player->CastSpell(_player, PALADIN_SPELL_LONG_ARM_OF_THE_LAW_RUN_SPEED, true);

                        if (_player->HasAura(PALADIN_SPELL_BURDEN_OF_GUILT))
                            _player->CastSpell(unitTarget, PALADIN_SPELL_BURDEN_OF_GUILT_DECREASE_SPEED, true);
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_judgment_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_judgment_SpellScript();
        }
};

// Ardent Defender - 31850
class spell_pal_ardent_defender : public SpellScriptLoader
{
    public:
        spell_pal_ardent_defender() : SpellScriptLoader("spell_pal_ardent_defender") { }

        class spell_pal_ardent_defender_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_ardent_defender_AuraScript);

            uint32 absorbPct, healPct;

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
                return GetUnitOwner()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect *aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* victim = GetTarget();
                int32 remainingHealth = victim->GetHealth() - dmgInfo.GetDamage();
                // If damage kills us
                if (remainingHealth <= 0 && !victim->ToPlayer()->HasSpellCooldown(PALADIN_SPELL_ARDENT_DEFENDER_HEAL))
                {
                    // Cast healing spell, completely avoid damage
                    absorbAmount = dmgInfo.GetDamage();

                    int32 healAmount = int32(victim->CountPctFromMaxHealth(healPct));
                    victim->CastCustomSpell(victim, PALADIN_SPELL_ARDENT_DEFENDER_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff);
                    victim->ToPlayer()->AddSpellCooldown(PALADIN_SPELL_ARDENT_DEFENDER_HEAL, 0, 120 * IN_MILLISECONDS);
                }
                else
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_ardent_defender_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_ardent_defender_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_ardent_defender_AuraScript();
        }
};

class spell_pal_blessing_of_faith : public SpellScriptLoader
{
    public:
        spell_pal_blessing_of_faith() : SpellScriptLoader("spell_pal_blessing_of_faith") { }

        class spell_pal_blessing_of_faith_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_blessing_of_faith_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_BLESSING_OF_LOWER_CITY_DRUID) || !sSpellMgr->GetSpellInfo(SPELL_BLESSING_OF_LOWER_CITY_PALADIN) || !sSpellMgr->GetSpellInfo(SPELL_BLESSING_OF_LOWER_CITY_PRIEST) || !sSpellMgr->GetSpellInfo(SPELL_BLESSING_OF_LOWER_CITY_SHAMAN))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint32 spell_id = 0;
                    switch (unitTarget->getClass())
                    {
                        case CLASS_DRUID:   spell_id = SPELL_BLESSING_OF_LOWER_CITY_DRUID; break;
                        case CLASS_PALADIN: spell_id = SPELL_BLESSING_OF_LOWER_CITY_PALADIN; break;
                        case CLASS_PRIEST:  spell_id = SPELL_BLESSING_OF_LOWER_CITY_PRIEST; break;
                        case CLASS_SHAMAN:  spell_id = SPELL_BLESSING_OF_LOWER_CITY_SHAMAN; break;
                        default: return;                    // ignore for non-healing classes
                    }
                    Unit* caster = GetCaster();
                    caster->CastSpell(caster, spell_id, true);
                }
            }

            void Register()
            {
                // add dummy effect spell handler to Blessing of Faith
                OnEffectHitTarget += SpellEffectFn(spell_pal_blessing_of_faith_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_blessing_of_faith_SpellScript();
        }
};

// Holy Shock - 20473
class spell_pal_holy_shock : public SpellScriptLoader
{
    public:
        spell_pal_holy_shock() : SpellScriptLoader("spell_pal_holy_shock") { }

        class spell_pal_holy_shock_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_shock_SpellScript);

            enum
            {
                HOLY_SHOCK_DAMAGE   = 25912,
                HOLY_SHOCK_HEALING  = 25914,
                HOLY_SHOCK_ENERGIZE = 148976,
            };

            bool Validate(SpellInfo const *)
            {
                return sSpellMgr->GetSpellInfo(HOLY_SHOCK_DAMAGE)
                    && sSpellMgr->GetSpellInfo(HOLY_SHOCK_HEALING)
                    && sSpellMgr->GetSpellInfo(HOLY_SHOCK_ENERGIZE);
            }

            SpellCastResult CheckCast()
            {
                Unit const * const target = GetExplTargetUnit();
                if (!target)
                    return SPELL_FAILED_BAD_TARGETS;

                Unit const * const caster = GetCaster();
                if (!caster->IsFriendlyTo(target))
                {
                    if (!caster->IsValidAttackTarget(target))
                        return SPELL_FAILED_BAD_TARGETS;
                    if (!caster->isInFront(target))
                        return SPELL_FAILED_UNIT_NOT_INFRONT;
                }

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit * const caster = GetCaster();
                Unit * const target = GetHitUnit();

                uint32 const spellId = caster->IsFriendlyTo(target)
                    ? HOLY_SHOCK_HEALING
                    : HOLY_SHOCK_DAMAGE;

                caster->CastSpell(target, spellId, true);
                // Add Holy Power
                caster->CastSpell(caster, HOLY_SHOCK_ENERGIZE, true);
            }

            void Register()
            {
                // add dummy effect spell handler to Holy Shock
                OnCheckCast += SpellCheckCastFn(spell_pal_holy_shock_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_shock_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_holy_shock_SpellScript();
        }
};

class spell_pal_divine_storm : public SpellScriptLoader
{
    public:
        spell_pal_divine_storm() : SpellScriptLoader("spell_pal_divine_storm") { }

        class spell_pal_divine_storm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_storm_SpellScript);

            uint32 healPct;

            bool Validate(SpellInfo const* /* spell */)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DIVINE_STORM_DUMMY))
                    return false;
                return true;
            }

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                return true;
            }

            void TriggerHeal()
            {
                Unit* caster = GetCaster();
                caster->CastCustomSpell(SPELL_DIVINE_STORM_DUMMY, SPELLVALUE_BASE_POINT0, (GetHitDamage() * healPct) / 100, caster, true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_pal_divine_storm_SpellScript::TriggerHeal);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_divine_storm_SpellScript();
        }
};

// Lay on Hands - 633
class spell_pal_lay_on_hands : public SpellScriptLoader
{
    public:
        spell_pal_lay_on_hands() : SpellScriptLoader("spell_pal_lay_on_hands") { }

        class spell_pal_lay_on_hands_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_lay_on_hands_SpellScript);

            SpellCastResult CheckForbearance()
            {
                if (Unit* target = GetExplTargetUnit())
                    if (target->HasAura(SPELL_FORBEARANCE))
                        return SPELL_FAILED_TARGET_AURASTATE;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, SPELL_FORBEARANCE, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_lay_on_hands_SpellScript::CheckForbearance);
                OnHit += SpellHitFn(spell_pal_lay_on_hands_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_lay_on_hands_SpellScript();
        }
};

class spell_pal_righteous_defense : public SpellScriptLoader
{
    public:
        spell_pal_righteous_defense() : SpellScriptLoader("spell_pal_righteous_defense") { }

        class spell_pal_righteous_defense_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_righteous_defense_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_DONT_REPORT;

                if (Unit* target = GetExplTargetUnit())
                {
                    if (!target->IsFriendlyTo(caster) || target->getAttackers().empty())
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_righteous_defense_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_righteous_defense_SpellScript();
        }
};

// Crusader Strike - 35395
class spell_pal_crusader_strike : public SpellScriptLoader
{
public:
    spell_pal_crusader_strike() : SpellScriptLoader("spell_pal_crusader_strike") { }

    class spell_pal_crusader_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_crusader_strike_SpellScript);

        void HandleOnHit()
        {
            if (Player* const _player = GetCaster()->ToPlayer())
                if (Unit* const target = GetHitUnit())
                {
                    if (_player != target)
                    _player->CastSpell(target, PALADIN_SPELL_WEAKENED_BLOWS, true);
                }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_pal_crusader_strike_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_crusader_strike_SpellScript();
    }
};

// Seal of Truth - 31801
class spell_pal_seal_of_truth : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            CENSURE = 31803,
            SEAL_OF_TRUTH_PROC = 42463
        };

        bool Validate(SpellInfo const *)
        {
            return sSpellMgr->GetSpellInfo(CENSURE) != NULL
                && sSpellMgr->GetSpellInfo(SEAL_OF_TRUTH_PROC) != NULL;
        }

        bool CheckProc(ProcEventInfo &eventInfo)
        {
            Unit const * const caster = eventInfo.GetActor();
            Unit const * const target = eventInfo.GetActionTarget();
            return caster && target && caster != target;
        }

        void HandleProc(AuraEffect const *aurEff, ProcEventInfo &eventInfo)
        {
            Unit * const caster = eventInfo.GetActor();
            Unit * const target = eventInfo.GetActionTarget();

            caster->CastSpell(target, SEAL_OF_TRUTH_PROC, true, NULL, aurEff);
            caster->CastSpell(target, CENSURE, true, NULL, aurEff);
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(script_impl::CheckProc);
            OnEffectProc += AuraEffectProcFn(script_impl::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_pal_seal_of_truth()
        : SpellScriptLoader("spell_pal_seal_of_truth")
    { }

    AuraScript * GetAuraScript() const
    {
        return new script_impl;
    }
};

// 119072 - Holy Wrath
class spell_pal_holy_wrath : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl)

        void HandleEnergize(SpellEffIndex /*effIndex*/)
        {
            if (Creature * cr = GetHitCreature())
            {
                uint32 mask = CREATURE_TYPEMASK_DEMON_OR_UNDEAD;
                // Glyph of Holy Wrath
                if (GetCaster() && GetCaster()->HasAura(54923))
                    mask = (1 << (CREATURE_TYPE_DRAGONKIN-1)) | (1 << (CREATURE_TYPE_ELEMENTAL-1)) | (1 << (CREATURE_TYPE_DEMON-1)) | (1 << (CREATURE_TYPE_UNDEAD-1));

                if ((cr->GetCreatureTypeMask() & mask))
                    return;
            }

            // Player in Lichborn is considered undead type
            if (Player * pl = GetHitPlayer())
                if (pl->HasAura(50397))
                    return;

            PreventHitAura();
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleEnergize, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

public:
    spell_pal_holy_wrath()
        : SpellScriptLoader("spell_pal_holy_wrath")
    { }

    SpellScript * GetSpellScript() const
    {
        return new script_impl;
    }
};

// Exorcism - 879
class spell_pal_exorcism : public SpellScriptLoader
{
public:
    spell_pal_exorcism() : SpellScriptLoader("spell_pal_exorcism") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { EXORCISM_ENERGIZE = 147715 };

        bool Validate(SpellInfo const *)
        {
            return sSpellMgr->GetSpellInfo(EXORCISM_ENERGIZE);
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            // Add Holy Power
            GetCaster()->CastSpell(GetCaster(), EXORCISM_ENERGIZE, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Hammer of Wrath - 24275
class spell_pal_hammer_of_wrath : public SpellScriptLoader
{
public:
    spell_pal_hammer_of_wrath() : SpellScriptLoader("spell_pal_hammer_of_wrath") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { HAMMER_OF_WRATH_ENERGIZE = 141459 };

        bool Validate(SpellInfo const *)
        {
            return sSpellMgr->GetSpellInfo(HAMMER_OF_WRATH_ENERGIZE);
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            // Add Holy Power
            GetCaster()->CastSpell(GetCaster(), HAMMER_OF_WRATH_ENERGIZE, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Avenging Wrath
class spell_pal_avenging_wrath : public SpellScriptLoader
{
public:
    spell_pal_avenging_wrath() : SpellScriptLoader("spell_pal_avenging_wrath") { }

    class spell_pal_avenging_wrath_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_avenging_wrath_SpellScript);

        void HandleOnHit()
        {
            Unit* caster = GetCaster();
            if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            Player* player = caster->ToPlayer();
            if (caster->HasAura(53376)) // Sanctified Wrath
            {
                caster->CastSpell(caster, 114232, true);
                if (Aura* aura = caster->GetAura(114232))
                {
                    std::list<uint32> effList;

                    switch(player->GetSpecializationId(player->GetActiveSpec()))
                    {
                        case SPEC_PALADIN_HOLY:
                            effList.push_back(EFFECT_0);
                            effList.push_back(EFFECT_4);
                            break;
                        case SPEC_PALADIN_PROTECTION:
                            effList.push_back(EFFECT_1);
                            effList.push_back(EFFECT_3);
                            break;
                        case SPEC_PALADIN_RETRIBUTION:
                            effList.push_back(EFFECT_2);
                            break;
                        default:
                            break;
                    }

                    for (uint32 i = 0; i < aura->GetSpellInfo()->Effects.size(); ++i)
                    {
                        std::list<uint32>::iterator found = std::find(effList.begin(), effList.end(), i);
                        if (found != effList.end())
                            continue;

                        if (AuraEffect* aurEff = aura->GetEffect(i))
                            aurEff->ChangeAmount(0);
                    }
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_pal_avenging_wrath_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_avenging_wrath_SpellScript();
    }
};

// Divine Plea - 54428
class spell_pal_divine_plea : public SpellScriptLoader
{
public:
    spell_pal_divine_plea() : SpellScriptLoader("spell_pal_divine_plea") {}

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void CalculateAmount(AuraEffect const *aurEff, int32 & amount, bool &)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            // @TODO: (excluding short-duration Spirit bonuses)
            uint32 tmpAmount = CalculatePct(caster->GetStat(STAT_SPIRIT), GetSpellInfo()->Effects[EFFECT_0].BasePoints);

            float manaPct = CalculatePct(caster->GetMaxPower(POWER_MANA), GetSpellInfo()->Effects[EFFECT_1].BasePoints);
            uint32 minAmount = manaPct / aurEff->GetTotalTicks();

            amount = std::max(tmpAmount, minAmount);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_impl::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

// Glyph of Double Jeopardy - 54922
class spell_pal_glyph_of_double_jeopardy : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_DOUBLE_JEOPARDY_PROC = 121027,
        };

        bool CheckProc(ProcEventInfo &eventInfo)
        {
            return eventInfo.GetActor() && eventInfo.GetActionTarget();
        }

        void HandleProc(AuraEffect const *aurEff, ProcEventInfo &eventInfo)
        {
            if (auto auraEff = eventInfo.GetActor()->GetAuraEffect(SPELL_DOUBLE_JEOPARDY_PROC, EFFECT_0))
                auraEff->SetUserData(eventInfo.GetActionTarget()->GetGUIDLow());
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(script_impl::CheckProc);
            AfterEffectProc += AuraEffectProcFn(script_impl::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

public:
    spell_pal_glyph_of_double_jeopardy()
        : SpellScriptLoader("spell_pal_glyph_of_double_jeopardy")
    { }

    AuraScript * GetAuraScript() const
    {
        return new script_impl;
    }
};

// 19740 - Blessing of Might, 20217 - Blessing of Knight
class spell_pal_blessing : public SpellScriptLoader
{
public:
    spell_pal_blessing() : SpellScriptLoader("spell_pal_blessing") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        void HandleOnHit()
        {
            auto caster = GetCaster();
            if (auto target = GetHitUnit())
            {
                if (target->IsInRaidWith(caster))
                {
                    std::list<Unit*> playerList;
                    caster->GetPartyMembers(playerList);
                    // AddAura required to prevent infinite script calls loop
                    for (auto raidMember : playerList)
                        caster->AddAura(GetSpellInfo()->Id, raidMember);
                }
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

// Guardian of ancient kings
class spell_pal_guardian : public SpellScriptLoader
{
public:
    spell_pal_guardian() : SpellScriptLoader("spell_pal_guardian") { }

    class spell_pal_guardian_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_guardian_SpellScript);

        void HandleOnHit()
        {
            Player* caster = GetCaster()->ToPlayer();
            if (!caster)
                return;

            // Choose which guardian to summon based on spec
            switch (caster->GetSpecializationId(caster->GetActiveSpec()))
            {
                case SPEC_PALADIN_HOLY:
                    caster->CastSpell(caster, 86674, true);
                    break;
                default:
                    return;
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_pal_guardian_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_guardian_SpellScript();
    }
};

class spell_pal_glyph_of_harsh_words : public SpellScriptLoader
{
    public:
        spell_pal_glyph_of_harsh_words() : SpellScriptLoader("spell_pal_glyph_of_harsh_words") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void HandleEffect(SpellEffIndex effIndex)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        _player->CastSpell(unitTarget, _player->IsValidAttackTarget(unitTarget) ? PALADIN_SPELL_WORD_OF_GLORY_DAMAGE : PALADIN_SPELL_WORD_OF_GLORY_HEAL, true);

                        int32 holyPower = _player->GetPower(POWER_HOLY_POWER);
                        if (holyPower > 2)
                            holyPower = 2;

                        if (!_player->HasAura(PALADIN_SPELL_DIVINE_PURPOSE))
                            _player->ModifyPower(POWER_HOLY_POWER, -holyPower);

                        // Item - Paladin PvP Set Holy 4P Bonus
                        if (_player->HasAura(PALADIN_ITEM_PVP_HOLY_4P_BONUS) && holyPower == 2)
                            _player->ModifyPower(POWER_HOLY_POWER, 1);

                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(script_impl::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

// Glyph of Contemplation - 125043
class spell_pal_glyph_of_Contemplation : public SpellScriptLoader
{
    public:
        spell_pal_glyph_of_Contemplation() : SpellScriptLoader("spell_pal_glyph_of_Contemplation") { }

        class spell_pal_glyph_of_Contemplation_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_glyph_of_Contemplation_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(PALADIN_SPELL_GLYPH_OF_CONTEMPLATION, false);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(PALADIN_SPELL_GLYPH_OF_CONTEMPLATION))
                        _player->removeSpell(PALADIN_SPELL_GLYPH_OF_CONTEMPLATION, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_pal_glyph_of_Contemplation_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pal_glyph_of_Contemplation_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_glyph_of_Contemplation_AuraScript();
        }
};

void AddSC_paladin_spell_scripts()
{
    new spell_pal_glyph_of_devotian_aura();
    new spell_pal_glyph_of_devotian_trigger_aura();
    new spell_pal_glyph_of_divine_storm();
    new spell_pal_daybreak();
    new spell_pal_hand_of_purity();
    new spell_pal_glyph_of_avenging_wrath();
    new spell_pal_shield_of_the_righteous();
    new spell_pal_tower_of_radiance();
    new spell_pal_sacred_shield();
    new spell_pal_sacred_shield_absorb();
    new spell_pal_emancipate();
    new spell_pal_art_of_war();
    new spell_pal_seal_of_insight();
    new spell_pal_blinding_light();
    new spell_pal_hand_of_protection();
    new spell_pal_divine_shield();
    new spell_pal_inquisition();
    new spell_pal_execution_sentence();
    new spell_pal_execution_sentence_effect("spell_pal_execution_sentence_damage", SPELL_AURA_PERIODIC_DAMAGE);
    new spell_pal_execution_sentence_effect("spell_pal_execution_sentence_heal", SPELL_AURA_PERIODIC_HEAL);
    new spell_pal_lights_hammer();
    new spell_pal_holy_prism_visual();
    new spell_pal_holy_prism_effect();
    new spell_pal_holy_prism();
    new spell_pal_consecration();
    new spell_pal_consecration_area();
    new spell_pal_word_of_glory();
    new spell_pal_judgment();
    new spell_pal_ardent_defender();
    new spell_pal_blessing_of_faith();
    new spell_pal_holy_shock();
    new spell_pal_divine_storm();
    new spell_pal_lay_on_hands();
    new spell_pal_righteous_defense();
    new spell_pal_crusader_strike();
    new spell_pal_seal_of_truth();
    new spell_pal_holy_wrath();
    new spell_pal_exorcism();
    new spell_pal_hammer_of_wrath();
    new spell_pal_avenging_wrath();
    new spell_pal_divine_plea();
    new spell_pal_glyph_of_double_jeopardy();
    new spell_pal_blessing();
    new spell_pal_daybreak_heal();
    new spell_pal_guardian();
    new spell_pal_glyph_of_harsh_words();
	new spell_pal_glyph_of_Contemplation();
}
