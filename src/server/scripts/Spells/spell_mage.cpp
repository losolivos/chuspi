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
 * Scripts for spells with SPELLFAMILY_MAGE and SPELLFAMILY_GENERIC spells used by mage players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_mage_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "ScriptedCreature.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectVisitors.hpp"
#include "Containers.h"

enum MageSpells
{
    SPELL_MAGE_COLD_SNAP                         = 11958,
    SPELL_MAGE_GLYPH_OF_ETERNAL_WATER            = 70937,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT  = 70908,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY  = 70907,
    SPELL_MAGE_GLYPH_OF_BLAST_WAVE               = 62126,
    SPELL_MAGE_ALTER_TIME_OVERRIDED              = 127140,
    SPELL_MAGE_ALTER_TIME                        = 110909,
    NPC_PAST_SELF                                = 58542,
    SPELL_MAGE_TEMPORAL_DISPLACEMENT             = 80354,
    HUNTER_SPELL_INSANITY                        = 95809,
    SPELL_SHAMAN_SATED                           = 57724,
    SPELL_SHAMAN_EXHAUSTED                       = 57723,
    SPELL_MAGE_MANA_GEM_ENERGIZE                 = 10052,
    SPELL_MAGE_ARCANE_BRILLIANCE                 = 1459,
    SPELL_MAGE_INFERNO_BLAST                     = 108853,
    SPELL_MAGE_INFERNO_BLAST_IMPACT              = 118280,
    SPELL_MAGE_IGNITE                            = 12654,
    SPELL_MAGE_PYROBLAST                         = 11366,
    SPELL_MAGE_COMBUSTION_DOT                    = 83853,
    SPELL_MAGE_COMBUSTION_IMPACT                 = 118271,
    SPELL_MAGE_FROSTJAW                          = 102051,
    SPELL_MAGE_NETHER_TEMPEST_DIRECT_DAMAGE      = 114954,
    SPELL_MAGE_NETHER_TEMPEST_VISUAL             = 114924,
    SPELL_MAGE_NETHER_TEMPEST_MISSILE            = 114956,
    SPELL_MAGE_LIVING_BOMB_TRIGGERED             = 44461,
    SPELL_MAGE_FROST_BOMB_TRIGGERED              = 113092,
    SPELL_MAGE_INVOKERS_ENERGY                   = 116257,
    SPELL_MAGE_INVOCATION                        = 114003,
    SPELL_MAGE_GLYPH_OF_EVOCATION                = 56380,
    SPELL_MAGE_BRAIN_FREEZE                      = 44549,
    SPELL_MAGE_BRAIN_FREEZE_TRIGGERED            = 57761,
    SPELL_MAGE_SLOW                              = 31589,
    SPELL_MAGE_ARCANE_CHARGE                     = 36032,
    SPELL_MAGE_ARCANE_BARRAGE_TRIGGERED          = 50273,
    SPELL_MAGE_PYROMANIAC_AURA                   = 132209,
    SPELL_MAGE_PYROMANIAC_DAMAGE_DONE            = 132210,
    SPELL_MAGE_MIRROR_IMAGE_SUMMON               = 58832,
    SPELL_MAGE_CAUTERIZE                         = 87023,
    SPELL_MAGE_CAUTERIZE_MARKER                  = 87024,
    SPELL_MAGE_ARCANE_MISSILES                   = 79683,
    SPELL_MAGE_ARCANE_MISSILES_DOUBLE_AURASTATE  = 79808,
    SPELL_MAGE_INCANTERS_WARD_ENERGIZE           = 113842,
    SPELL_MAGE_INCANTERS_ABSORBTION              = 116267,
    SPELL_MAGE_INCANTERS_ABSORBTION_PASSIVE      = 118858,
    SPELL_MAGE_GLYPH_OF_ICE_BLOCK                = 115723,
    SPELL_MAGE_GLYPH_OF_ICE_BLOCK_IMMUNITY       = 115760,
    SPELL_MAGE_GLYPH_OF_ICE_BLOCK_FROST_NOVA     = 115757,
    SPELL_MAGE_IMPROVED_COUNTERSPELL             = 12598,
    SPELL_MAGE_COUNTERSPELL_SILENCE              = 55021,
    SPELL_MAGE_FINGER_OF_FROST_EFFECT            = 44544,
    SPELL_MAGE_GLYPH_OF_SLOW                     = 86209,
    SPELL_MAGE_GREATER_INVISIBILITY_LESS_DAMAGE  = 113862,
    SPELL_MAGE_REMOVE_INVISIBILITY_REMOVED_TIMER = 122293,
    SPELL_MAGE_BLAST_WAVE_SNARE                  = 11113,
    SPELL_MAGE_ICE_BLOCK                         = 45438,
    SPELL_MAGE_CONE_OF_COLD                      = 120,
    SPELL_MAGE_FROST_NOVA                        = 122,
    SPELL_MAGE_FINGERS_OF_FROST_AURA             = 112965,
    SPELL_MAGE_RING_OF_FROST_DUMMY               = 91264,
    SPELL_MAGE_RING_OF_FROST_FREEZE              = 82691,
    SPELL_MAGE_RING_OF_FROST_SUMMON              = 113724,
    SPELL_MAGE_GLYPH_OF_MIRROR_IMAGE             = 63093,
    SPELL_MAGE_SUMMON_IMAGES_FROST               = 58832,
    SPELL_MAGE_SUMMON_IMAGES_FIRE                = 88092,
    SPELL_MAGE_SUMMON_IMAGES_ARCANE              = 88091,
    SPELL_MAGE_BLINK = 65793,
};

// Flamestrike - 2120
class spell_mage_flamestrike : public SpellScriptLoader
{
    public:
        spell_mage_flamestrike() : SpellScriptLoader("spell_mage_flamestrike") { }

        class spell_mage_flamestrike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_flamestrike_SpellScript);

            void HandleOnHit()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (caster->GetSpecializationId(caster->GetActiveSpec()) == SPEC_MAGE_FIRE)
                            caster->CastSpell(target, SPELL_MAGE_BLAST_WAVE_SNARE, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_flamestrike_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_flamestrike_SpellScript();
        }
};

// Greater Invisibility (remove timer) - 122293
class spell_mage_greater_invisibility_removed : public SpellScriptLoader
{
    public:
        spell_mage_greater_invisibility_removed() : SpellScriptLoader("spell_mage_greater_invisibility_removed") { }

        class spell_mage_greater_invisibility_removed_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_greater_invisibility_removed_AuraScript);

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasAura(SPELL_MAGE_GREATER_INVISIBILITY_LESS_DAMAGE))
                        _player->RemoveAurasDueToSpell(SPELL_MAGE_GREATER_INVISIBILITY_LESS_DAMAGE);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_greater_invisibility_removed_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_greater_invisibility_removed_AuraScript();
        }
};

// Greater Invisibility (triggered) - 110960
class spell_mage_greater_invisibility_triggered : public SpellScriptLoader
{
    public:
        spell_mage_greater_invisibility_triggered() : SpellScriptLoader("spell_mage_greater_invisibility_triggered") { }

        class spell_mage_greater_invisibility_triggered_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_greater_invisibility_triggered_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->CastSpell(_player, SPELL_MAGE_GREATER_INVISIBILITY_LESS_DAMAGE, true);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->CastSpell(_player, SPELL_MAGE_REMOVE_INVISIBILITY_REMOVED_TIMER, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_mage_greater_invisibility_triggered_AuraScript::OnApply, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_greater_invisibility_triggered_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_greater_invisibility_triggered_AuraScript();
        }
};

// Called by Arcane Blast - 30451
// Glyph of Slow - 86209
class spell_mage_glyph_of_slow : public SpellScriptLoader
{
    public:
        spell_mage_glyph_of_slow() : SpellScriptLoader("spell_mage_glyph_of_slow") { }

        class spell_mage_glyph_of_slow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_glyph_of_slow_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HasAura(SPELL_MAGE_GLYPH_OF_SLOW))
                        return;

                    if (Unit* target = GetHitUnit())
                    {
                        std::list<Unit*> targetList;
                        float radius = 50.0f;
                        bool found = false;

                        Trinity::NearestAttackableUnitInObjectRangeCheck u_check(caster, caster, radius);
                        Trinity::UnitListSearcher<Trinity::NearestAttackableUnitInObjectRangeCheck> searcher(caster, targetList, u_check);
                        Trinity::VisitNearbyObject(caster, radius, searcher);

                        for (auto itr : targetList)
                            if (itr->HasAura(SPELL_MAGE_SLOW))
                                found = true;

                        if (found)
                            return;
                        else
                            caster->CastSpell(target, SPELL_MAGE_SLOW, true);
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_glyph_of_slow_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_glyph_of_slow_SpellScript();
        }
};

// Frost Nova (Water Elemental) - 33395
class spell_mage_pet_frost_nova : public SpellScriptLoader
{
    public:
        spell_mage_pet_frost_nova() : SpellScriptLoader("spell_mage_pet_frost_nova") { }

        class spell_mage_pet_frost_nova_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_pet_frost_nova_SpellScript);

            bool Load()
            {
                bool result = true;

                if (!GetCaster())
                    return false;

                result &= GetCaster()->GetTypeId() == TYPEID_UNIT;

                if (Unit * const owner = GetCaster()->GetOwner())
                {
                    result &= owner->GetTypeId() == TYPEID_PLAYER;
                    result &= owner->getLevel() >= 24;
                    return result;
                }
                else
                    return false;
            }

            void HandleOnHit()
            {
                if (Unit * const owner = GetCaster()->GetOwner())
                {
                    if (Player * const _player = owner->ToPlayer())
                    {
                        if (!_player->HasSpell(SPELL_MAGE_FINGERS_OF_FROST_AURA))
                            return;

                        _player->CastSpell(_player, SPELL_MAGE_FINGER_OF_FROST_EFFECT, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_pet_frost_nova_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_pet_frost_nova_SpellScript();
        }
};

// Counterspell - 2139
class spell_mage_counterspell : public SpellScriptLoader
{
    public:
        spell_mage_counterspell() : SpellScriptLoader("spell_mage_counterspell") { }

        class spell_mage_counterspell_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_counterspell_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(SPELL_MAGE_IMPROVED_COUNTERSPELL))
                            _player->CastSpell(target, SPELL_MAGE_COUNTERSPELL_SILENCE, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_counterspell_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_counterspell_SpellScript();
        }
};

// Called by Ice Block - 45438
// Glyph of Ice Block - 115723
class spell_mage_glyph_of_ice_block : public SpellScriptLoader
{
    public:
        spell_mage_glyph_of_ice_block() : SpellScriptLoader("spell_mage_glyph_of_ice_block") { }

        class spell_mage_glyph_of_ice_block_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_glyph_of_ice_block_AuraScript);

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetTarget()->ToPlayer())
                {
                    if (_player->HasAura(SPELL_MAGE_GLYPH_OF_ICE_BLOCK))
                    {
                        _player->CastSpell(_player, SPELL_MAGE_GLYPH_OF_ICE_BLOCK_FROST_NOVA, true);
                        _player->CastSpell(_player, SPELL_MAGE_GLYPH_OF_ICE_BLOCK_IMMUNITY, true);
                    }
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_glyph_of_ice_block_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_glyph_of_ice_block_AuraScript();
        }
};

// Incanter's Ward (Cooldown marker) - 118859
class spell_mage_incanters_ward_cooldown : public SpellScriptLoader
{
    public:
        spell_mage_incanters_ward_cooldown() : SpellScriptLoader("spell_mage_incanters_ward_cooldown") { }

        class spell_mage_incanters_ward_cooldown_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_incanters_ward_cooldown_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->RemoveAura(SPELL_MAGE_INCANTERS_ABSORBTION_PASSIVE);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (!caster->HasAura(SPELL_MAGE_INCANTERS_ABSORBTION_PASSIVE))
                        caster->CastSpell(caster, SPELL_MAGE_INCANTERS_ABSORBTION_PASSIVE, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_mage_incanters_ward_cooldown_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_incanters_ward_cooldown_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_incanters_ward_cooldown_AuraScript();
        }
};

// Incanter's Ward - 1463
class spell_mage_incanters_ward : public SpellScriptLoader
{
    public:
        spell_mage_incanters_ward() : SpellScriptLoader("spell_mage_incanters_ward") { }

        class spell_mage_incanters_ward_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_incanters_ward_AuraScript);

            float absorbTotal;
            float absorbtionAmount;

            bool Load()
            {
                absorbTotal = 0.0f;
                absorbtionAmount = 0.0f;
                return true;
            }

            void CalculateAmount(AuraEffect const *, int32 & amount, bool & )
            {
                if (Unit* caster = GetCaster())
                    amount += caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ARCANE);

                absorbtionAmount = float(amount);
            }

            void OnAbsorb(AuraEffect *aurEff, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
            {
                if (Unit* caster = dmgInfo.GetVictim())
                {
                    if (dmgInfo.GetAttacker())
                    {
                        absorbTotal += float(dmgInfo.GetDamage());
                        absorbTotal = std::min(absorbTotal, absorbtionAmount);

                        int32 pct = aurEff->GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                        int32 manaGain = CalculatePct(caster->GetMaxPower(POWER_MANA), CalculatePct(((float(dmgInfo.GetDamage()) / absorbtionAmount) * 100.0f), pct));

                        if (manaGain > caster->CountPctFromMaxMana(pct))
                            manaGain = caster->CountPctFromMaxMana(pct);

                        caster->EnergizeBySpell(caster, SPELL_MAGE_INCANTERS_WARD_ENERGIZE, manaGain, POWER_MANA);
                    }
                }
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 cap = sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION)->Effects[EFFECT_0].BasePoints;
                    int32 damageGain = CalculatePct(cap, ((absorbTotal / absorbtionAmount) * 100.0f));
                    if (!damageGain)
                        return;

                    caster->CastCustomSpell(caster, SPELL_MAGE_INCANTERS_ABSORBTION, &damageGain, NULL, NULL, true);
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_incanters_ward_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_incanters_ward_AuraScript::OnAbsorb, EFFECT_0);
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_incanters_ward_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_incanters_ward_AuraScript();
        }
};

// Arcane Missiles - 5143
class spell_mage_arcane_missile : public SpellScriptLoader
{
    public:
        spell_mage_arcane_missile() : SpellScriptLoader("spell_mage_arcane_missile") { }

        class spell_mage_arcane_missile_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_arcane_missile_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                auto caster = GetCaster();
                if (!caster)
                    return;

                caster->CastSpell(caster, SPELL_MAGE_ARCANE_CHARGE, true);

                if (Aura *arcaneMissiles = caster->GetAura(SPELL_MAGE_ARCANE_MISSILES))
                {
                    arcaneMissiles->ModStackAmount(-1);
                    caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_DOUBLE_AURASTATE);
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_mage_arcane_missile_AuraScript::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }

        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_arcane_missile_AuraScript();
        }
};

// Cauterize - 86949
class spell_mage_cauterize : public SpellScriptLoader
{
    public:
        spell_mage_cauterize() : SpellScriptLoader("spell_mage_cauterize") { }

        class spell_mage_cauterize_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_cauterize_AuraScript);

            uint32 absorbChance;
            uint32 healtPct;

            bool Load()
            {
                absorbChance = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                healtPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                return GetUnitOwner()->ToPlayer();
            }

            void CalculateAmount(AuraEffect const * /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect * /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                Unit* target = GetTarget();

                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                if (target->ToPlayer()->HasSpellCooldown(SPELL_MAGE_CAUTERIZE))
                    return;

                if (!roll_chance_i(absorbChance))
                    return;

                int bp1 = target->CountPctFromMaxHealth(healtPct);
                target->CastCustomSpell(target, SPELL_MAGE_CAUTERIZE, NULL, &bp1, NULL, true);
                target->CastSpell(target, SPELL_MAGE_CAUTERIZE_MARKER, true);
                target->ToPlayer()->AddSpellCooldown(SPELL_MAGE_CAUTERIZE, 0, 120 * IN_MILLISECONDS);

                absorbAmount = dmgInfo.GetDamage();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_cauterize_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_cauterize_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_cauterize_AuraScript();
        }
};

// Called by Nether Tempest - 114923, Frost Bomb - 112948 and Living Bomb - 44457
// Pyromaniac - 132209
class spell_mage_pyromaniac : public SpellScriptLoader
{
    public:
        spell_mage_pyromaniac() : SpellScriptLoader("spell_mage_pyromaniac") { }

        class spell_mage_pyromaniac_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_pyromaniac_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(SPELL_MAGE_PYROMANIAC_AURA))
                            _player->CastSpell(target, SPELL_MAGE_PYROMANIAC_DAMAGE_DONE, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_pyromaniac_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_pyromaniac_SpellScript();
        }
};

class CheckArcaneBarrageImpactPredicate
{
    public:
        CheckArcaneBarrageImpactPredicate(Unit* caster, Unit* mainTarget) : _caster(caster), _mainTarget(mainTarget) {}

        bool operator()(Unit* target)
        {
            if (!_caster || !_mainTarget)
                return true;

            if (!_caster->IsValidAttackTarget(target))
                return true;

            if (!target->IsWithinLOSInMap(_caster))
                return true;

            if (!_caster->isInFront(target))
                return true;

            if (target->GetGUID() == _caster->GetGUID())
                return true;

            if (target->GetGUID() == _mainTarget->GetGUID())
                return true;

            return false;
        }

    private:
        Unit* _caster;
        Unit* _mainTarget;
};

// Arcane Barrage - 44425
class spell_mage_arcane_barrage : public SpellScriptLoader
{
    public:
        spell_mage_arcane_barrage() : SpellScriptLoader("spell_mage_arcane_barrage") { }

        class spell_mage_arcane_barrage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_arcane_barrage_SpellScript);

            void HandleAfterHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Aura *arcaneCharge = _player->GetAura(SPELL_MAGE_ARCANE_CHARGE))
                        _player->RemoveAura(SPELL_MAGE_ARCANE_CHARGE);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_mage_arcane_barrage_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_arcane_barrage_SpellScript();
        }
};

// Arcane Explosion - 1449
class spell_mage_arcane_explosion final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            ARCANE_CHARGE_ENABLER = 114664
        };

        bool canGenerateCharge_;

        void filterTargets(std::list<WorldObject*> &targets)
        {
            canGenerateCharge_ = !targets.empty();
        }

        void afterCast()
        {
            auto const caster = GetCaster();
            if (!caster || !caster->HasAura(ARCANE_CHARGE_ENABLER))
                return;

            if (auto const aura = caster->GetAura(SPELL_MAGE_ARCANE_CHARGE))
                aura->RefreshDuration();

            if (canGenerateCharge_ && roll_chance_i(GetSpellInfo()->Effects[EFFECT_1].BasePoints))
                caster->CastSpell(caster, SPELL_MAGE_ARCANE_CHARGE, true);
        }

        void Register() final
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::filterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            AfterCast += SpellCastFn(script_impl::afterCast);
        }
    };

public:
    spell_mage_arcane_explosion()
        : SpellScriptLoader("spell_mage_arcane_explosion")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

// Slow - 31589
class spell_mage_slow : public SpellScriptLoader
{
    public:
        spell_mage_slow() : SpellScriptLoader("spell_mage_slow") { }

        class spell_mage_slow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_slow_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER)
                        {
                            if (Aura *frostjaw = target->GetAura(SPELL_MAGE_SLOW, _player->GetGUID()))
                            {
                                // Only half time against players
                                frostjaw->SetDuration(frostjaw->GetMaxDuration() / 2);
                                frostjaw->SetMaxDuration(frostjaw->GetDuration());
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_slow_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_slow_SpellScript();
        }
};

// Frostbolt - 116
class spell_mage_frostbolt final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            FROSTBOLT_HEAL = 126201,
        };

        bool shouldHeal_;

        SpellCastResult checkTarget()
        {
            auto const target = GetExplTargetUnit();

            if (!target)
                return SPELL_FAILED_NO_VALID_TARGETS;

            // attacking
            auto const caster = GetCaster();
            if (caster->IsValidAttackTarget(target))
            {
                shouldHeal_ = false;
                return SPELL_CAST_OK;
            }

            // healing own Water Elemental
            if (target->GetOwnerGUID() == caster->GetGUID() && target->GetEntry() == ENTRY_WATER_ELEMENTAL)
            {
                shouldHeal_ = true;
                return SPELL_CAST_OK;
            }

            return SPELL_FAILED_BAD_TARGETS;
        }

        void maybePrevent(SpellEffIndex effIndex)
        {
            if (!shouldHeal_)
                return;

            PreventHitDefaultEffect(effIndex);
            if (effIndex == EFFECT_0)
                PreventHitAura();
        }

        void maybeHeal(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (!shouldHeal_)
                return;

            auto const caster = GetCaster();
            auto const target = GetExplTargetUnit();

            caster->CastSpell(target, FROSTBOLT_HEAL, true);
        }

        void Register() final
        {
            OnCheckCast += SpellCheckCastFn(script_impl::checkTarget);
            OnEffectHitTarget += SpellEffectFn(script_impl::maybePrevent, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            OnEffectLaunchTarget += SpellEffectFn(script_impl::maybePrevent, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            OnEffectHitTarget += SpellEffectFn(script_impl::maybeHeal, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

public:
    spell_mage_frostbolt()
        : SpellScriptLoader("spell_mage_frostbolt")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

// Mastery: Icicles Called by Frostbolt and Frostfire Bolt - 116, 44614
class spell_mastery_icicles final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            ICICLE_STORE   = 148012,
            ICICLE_DAMAGE  = 148022,
            ICICILE_VISUAL = 148017,
        };

        void handleMastery()
        {
            auto caster = GetCaster()->ToPlayer();
            if (!caster || caster == GetHitUnit())
                return;

            if (caster->getLevel() >= 80 && caster->HasAura(76613))
            {
                int32 damage = GetHitDamage() + GetAbsorbedDamage();
                if (!damage)
                    return;

                float Mastery = caster->GetFloatValue(PLAYER_MASTERY) * 2.0f;
                damage = CalculatePct(damage, Mastery);
                for (int32 i = 0; i < 5; ++i)
                {
                    if (caster->HasAura(ICICLE_STORE+i))
                    {
                        // If all icicles are stored, fire last and replace it with new
                        if (i == 4)
                        {
                            if (auto icicle = caster->GetAuraEffect(ICICLE_STORE, EFFECT_0))
                            {
                                int32 amount = icicle->GetAmount();
                                caster->CastCustomSpell(GetHitUnit(), ICICILE_VISUAL, &amount, NULL, NULL, true);
                                caster->CastCustomSpell(GetHitUnit(), ICICLE_DAMAGE, &amount, NULL, NULL, true);
                                caster->RemoveAurasDueToSpell(ICICLE_STORE);
                                caster->CastCustomSpell(caster, ICICLE_STORE, &damage, NULL, NULL, true);
                                break;
                            }
                        }
                        else
                            continue;
                    }
                    else
                    {
                        caster->CastCustomSpell(caster, ICICLE_STORE+i, &damage, NULL, NULL, true);
                        break;
                    }
                }
            }
        }

        void Register() final
        {
            AfterHit += SpellHitFn(script_impl::handleMastery);
        }
    };

public:
    spell_mastery_icicles()
        : SpellScriptLoader("spell_mastery_icicles")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

// Ice Lance - 30455
class spell_mastery_icicles_trigger final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            ICICILE_TRIGGER_AURA = 148023
        };

        void handleMastery(SpellEffIndex effIndex)
        {
            auto caster = GetCaster()->ToPlayer();
            if (!caster)
                return;

            if (caster->getLevel() >= 80 && caster->HasAura(76613))
                caster->CastSpell(GetHitUnit(), ICICILE_TRIGGER_AURA, true);
        }

        void Register() final
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::handleMastery, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

public:
    spell_mastery_icicles_trigger()
        : SpellScriptLoader("spell_mastery_icicles_trigger")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

// 148023 - Icicles (periodic aura to trigger all stored icicles)
class spell_mastery_icicles_periodic : public SpellScriptLoader
{
public:
    spell_mastery_icicles_periodic() : SpellScriptLoader("spell_mastery_icicles_periodic") { }

    enum
    {
        ICICLE_STORE   = 148012,
        ICICLE_DAMAGE  = 148022,
        ICICILE_VISUAL = 148017,
    };

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void OnTick(AuraEffect const * /*aurEff*/)
        {
            if (!GetCaster())
                return;
            if (auto player = GetCaster()->ToPlayer())
            {
                for (int32 i = 0; i < 5; ++i)
                {
                    if (auto icicle = player->GetAuraEffect(ICICLE_STORE + i, EFFECT_0))
                    {
                        int32 amount = icicle->GetAmount();
                        player->CastCustomSpell(GetTarget(), ICICILE_VISUAL+i, &amount, NULL, NULL, true);
                        player->CastCustomSpell(GetTarget(), ICICLE_DAMAGE, &amount, NULL, NULL, true);
                        player->RemoveAurasDueToSpell(ICICLE_STORE + i);
                        break;
                    }
                    if (i == 4)
                        SetDuration(0);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Called by Evocation - 12051
// Invocation - 114003
class spell_mage_invocation : public SpellScriptLoader
{
    public:
        spell_mage_invocation() : SpellScriptLoader("spell_mage_invocation") { }

        class spell_mage_invocation_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_invocation_AuraScript);

            void AfterRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(SPELL_MAGE_INVOCATION))
                    {
                        caster->CastSpell(caster, SPELL_MAGE_INVOKERS_ENERGY, true);

                        if (caster->HasAura(SPELL_MAGE_GLYPH_OF_EVOCATION))
                        {
                            int32 healthGain = caster->CountPctFromMaxHealth(10);
                            auto spellInfo = sSpellMgr->GetSpellInfo(12051);
                            healthGain = caster->SpellHealingBonusDone(caster, spellInfo, EFFECT_0, healthGain, HEAL);
                            healthGain = caster->SpellHealingBonusTaken(caster, spellInfo, EFFECT_0, healthGain, HEAL);
                            caster->HealBySpell(caster, spellInfo, healthGain, false);
                        }
                    }
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_invocation_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_OBS_MOD_POWER, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_invocation_AuraScript();
        }
};

// Frost Bomb - 112948
class spell_mage_frost_bomb : public SpellScriptLoader
{
    public:
        spell_mage_frost_bomb() : SpellScriptLoader("spell_mage_frost_bomb") { }

        class spell_mage_frost_bomb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_frost_bomb_AuraScript);

            void AfterRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (Unit* caster = GetCaster())
                {
                    caster->CastSpell(GetTarget(), SPELL_MAGE_FROST_BOMB_TRIGGERED, true);

                    if (caster->HasAura(SPELL_MAGE_BRAIN_FREEZE))
                        caster->CastSpell(caster, SPELL_MAGE_BRAIN_FREEZE_TRIGGERED, true);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_frost_bomb_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_frost_bomb_AuraScript();
        }
};

class CheckNetherImpactPredicate
{
    public:
        CheckNetherImpactPredicate(Unit* caster, Unit* mainTarget) : _caster(caster), _mainTarget(mainTarget) {}

        bool operator()(Unit* target)
        {
            if (!_caster || !_mainTarget)
                return true;

            if (!_caster->IsValidAttackTarget(target))
                return true;

            if (!target->IsWithinLOSInMap(_caster))
                return true;

            if (!_caster->isInFront(target))
                return true;

            if (target->GetGUID() == _caster->GetGUID())
                return true;

            if (target->GetGUID() == _mainTarget->GetGUID())
                return true;

            return false;
        }

    private:
        Unit* _caster;
        Unit* _mainTarget;
};

// Nether Tempest - 114923
class spell_mage_nether_tempest : public SpellScriptLoader
{
    public:
        spell_mage_nether_tempest() : SpellScriptLoader("spell_mage_nether_tempest") { }

        class spell_mage_nether_tempest_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_nether_tempest_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                {
                    if (Player* _player = GetCaster()->ToPlayer())
                    {
                        std::list<Unit*> targetList;

                        GetTarget()->GetAttackableUnitListInRange(targetList, 10.0f);
                        targetList.remove_if(CheckNetherImpactPredicate(_player, GetTarget()));

                        Trinity::Containers::RandomResizeList(targetList, 1);

                        for (auto itr : targetList)
                        {
                            GetCaster()->CastSpell(itr, SPELL_MAGE_NETHER_TEMPEST_DIRECT_DAMAGE, true);
                            GetCaster()->CastSpell(itr, SPELL_MAGE_NETHER_TEMPEST_VISUAL, true);
                            GetTarget()->CastSpell(itr, SPELL_MAGE_NETHER_TEMPEST_MISSILE, true);
                        }


                        Player::AuraTargetList const *targets = _player->ToPlayer()->listOfTargetsOfAura(GetId());
                        if (!targets || targets->empty())
                            return;

                        Unit const * const lastTarget = Unit::GetUnit(*_player, targets->back());
                        if (GetTarget() == lastTarget && roll_chance_i(10) && _player->HasAura(SPELL_MAGE_BRAIN_FREEZE))
                            _player->CastSpell(_player, SPELL_MAGE_BRAIN_FREEZE_TRIGGERED, true);
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_nether_tempest_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_nether_tempest_AuraScript();
        }
};

// Blazing Speed - 108843
class spell_mage_blazing_speed : public SpellScriptLoader
{
    public:
        spell_mage_blazing_speed() : SpellScriptLoader("spell_mage_blazing_speed") { }

        class spell_mage_blazing_speed_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_blazing_speed_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->RemoveMovementImpairingAuras();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_blazing_speed_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_blazing_speed_SpellScript();
        }
};

// Frostjaw - 102051
class spell_mage_frostjaw : public SpellScriptLoader
{
    public:
        spell_mage_frostjaw() : SpellScriptLoader("spell_mage_frostjaw") { }

        class spell_mage_frostjaw_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_frostjaw_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER)
                        {
                            if (Aura *frostjaw = target->GetAura(SPELL_MAGE_FROSTJAW, _player->GetGUID()))
                            {
                                // Only half time against players
                                frostjaw->SetDuration(frostjaw->GetMaxDuration() / 2);
                                frostjaw->SetMaxDuration(frostjaw->GetDuration());
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_frostjaw_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_frostjaw_SpellScript();
        }
};

// Combustion - 11129
class spell_mage_combustion : public SpellScriptLoader
{
    public:
        spell_mage_combustion() : SpellScriptLoader("spell_mage_combustion") { }

        class spell_mage_combustion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_combustion_SpellScript);

            void HandleOnHit()
            {
                auto player = GetCaster()->ToPlayer();
                auto target = GetHitUnit();
                if (!player || !target)
                    return;

                player->CastSpell(target, SPELL_MAGE_COMBUSTION_IMPACT, true);

                if (player->HasSpellCooldown(SPELL_MAGE_INFERNO_BLAST))
                {
                    player->RemoveSpellCooldown(SPELL_MAGE_INFERNO_BLAST, true);
                    player->RemoveSpellCooldown(SPELL_MAGE_INFERNO_BLAST_IMPACT, true);
                }

                // 20% of ignite tick damage
                if (auto ignite = target->GetAuraEffect(SPELL_MAGE_IGNITE, EFFECT_0, player->GetGUID()))
                    if (int32 combustionBp = CalculatePct(ignite->GetAmount(), GetSpellInfo()->Effects[EFFECT_0].BasePoints))
                        player->CastCustomSpell(target, SPELL_MAGE_COMBUSTION_DOT, &combustionBp, NULL, NULL, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_combustion_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_combustion_SpellScript();
        }
};

class CheckInfernoBlastImpactPredicate
{
    public:
        CheckInfernoBlastImpactPredicate(Unit* caster, Unit* mainTarget) : _caster(caster), _mainTarget(mainTarget) {}

        bool operator()(Unit* target)
        {
            if (!_caster || !_mainTarget)
                return true;

            if (!_caster->IsValidAttackTarget(target))
                return true;

            if (!target->IsWithinLOSInMap(_caster))
                return true;

            if (!_caster->isInFront(target))
                return true;

            if (target->GetGUID() == _caster->GetGUID())
                return true;

            if (target->GetGUID() == _mainTarget->GetGUID())
                return true;

            return false;
        }

    private:
        Unit* _caster;
        Unit* _mainTarget;
};

class spell_mage_inferno_blast : public SpellScriptLoader
{
    public:
        spell_mage_inferno_blast() : SpellScriptLoader("spell_mage_inferno_blast") { }

        class spell_mage_inferno_blast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_inferno_blast_SpellScript);

            void HandleOnHit()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    caster->RemoveSpellCooldown(118280);
                    if (Unit* target = GetHitUnit())
                        caster->CastSpell(target, SPELL_MAGE_INFERNO_BLAST_IMPACT, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_inferno_blast_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_inferno_blast_SpellScript();
        }
};

class spell_mage_inferno_blast_spread : public SpellScriptLoader
{
    public:
        spell_mage_inferno_blast_spread() : SpellScriptLoader("spell_mage_inferno_blast_spread") { }

        class spell_mage_inferno_blast_spread_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_inferno_blast_spread_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (GetExplTargetUnit())
                    targets.remove(GetExplTargetUnit());

                if (targets.size() > 3)
                    Trinity::Containers::RandomResizeList(targets, 3);
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetExplTargetUnit())
                {
                    Unit::AuraEffectList const & aurasA = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraEffectList::const_iterator itr = aurasA.begin(); itr != aurasA.end(); ++itr)
                    {
                        if (((*itr)->GetCasterGUID() != caster->GetGUID()) 
                            || ((*itr)->GetSpellInfo()->Id != 12654
                            && (*itr)->GetSpellInfo()->Id != 11366
                            && (*itr)->GetSpellInfo()->Id != 83853))
                            continue;
                        
                        caster->AddAuraForTarget((*itr)->GetBase(), GetHitUnit());
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_inferno_blast_spread_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_mage_inferno_blast_spread_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_inferno_blast_spread_SpellScript();
        }
};

// Arcane Brillance - 1459
class spell_mage_arcane_brilliance : public SpellScriptLoader
{
    public:
        spell_mage_arcane_brilliance() : SpellScriptLoader("spell_mage_arcane_brilliance") { }

        class spell_mage_arcane_brilliance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_arcane_brilliance_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    _player->AddAura(SPELL_MAGE_ARCANE_BRILLIANCE, _player);

                    std::list<Unit*> memberList;
                    _player->GetPartyMembers(memberList);

                    for (auto itr : memberList)
                        _player->AddAura(SPELL_MAGE_ARCANE_BRILLIANCE, itr);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_arcane_brilliance_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_arcane_brilliance_SpellScript();
        }
};

// Evocation - 12051
class spell_mage_evocation : public SpellScriptLoader
{
    public:
        spell_mage_evocation() : SpellScriptLoader("spell_mage_evocation") { }

        class spell_mage_evocation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_evocation_SpellScript);

            void HandleOnHit()
            {
                auto caster = GetCaster();
                caster->EnergizeBySpell(caster, GetSpellInfo()->Id, int32(caster->GetMaxPower(POWER_MANA) * 0.15), POWER_MANA);
                caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_CHARGE);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_evocation_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_evocation_SpellScript();
        }
};

// 42955 Conjure Refreshment
/// Updated 5.4.2
struct ConjureRefreshmentData
{
    uint32 minLevel;
    uint32 maxLevel;
    uint32 spellId;
};

uint8 const MAX_CONJURE_REFRESHMENT_SPELLS = 8;
ConjureRefreshmentData const _conjureData[MAX_CONJURE_REFRESHMENT_SPELLS] =
{
    { 33, 43, 92739 },
    { 44, 53, 92799 },
    { 54, 63, 92802 },
    { 64, 73, 92805 },
    { 74, 79, 74625 },
    { 80, 84, 92822 },
    { 85, 89, 92727 },
    { 90, 90, 116130 }
};

// 42955 - Conjure Refreshment
class spell_mage_conjure_refreshment : public SpellScriptLoader
{
    public:
        spell_mage_conjure_refreshment() : SpellScriptLoader("spell_mage_conjure_refreshment") { }

        class spell_mage_conjure_refreshment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_conjure_refreshment_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                for (uint8 i = 0; i < MAX_CONJURE_REFRESHMENT_SPELLS; ++i)
                    if (!sSpellMgr->GetSpellInfo(_conjureData[i].spellId))
                        return false;
                return true;
            }

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                uint8 level = GetHitUnit()->getLevel();
                for (uint8 i = 0; i < MAX_CONJURE_REFRESHMENT_SPELLS; ++i)
                {
                    ConjureRefreshmentData const& spellData = _conjureData[i];
                    if (level < spellData.minLevel || level > spellData.maxLevel)
                        continue;
                    GetHitUnit()->CastSpell(GetHitUnit(), spellData.spellId);
                    break;
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_conjure_refreshment_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_conjure_refreshment_SpellScript();
        }
};

// Time Warp - 80353
class spell_mage_time_warp : public SpellScriptLoader
{
    public:
        spell_mage_time_warp() : SpellScriptLoader("spell_mage_time_warp") { }

        class spell_mage_time_warp_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_time_warp_SpellScript);

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, HUNTER_SPELL_INSANITY));
                targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_SHAMAN_EXHAUSTED));
                targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_SHAMAN_SATED));
                targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_MAGE_TEMPORAL_DISPLACEMENT));
            }

            void ApplyDebuff()
            {
                if (Unit* target = GetHitUnit())
                    target->CastSpell(target, SPELL_MAGE_TEMPORAL_DISPLACEMENT, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_time_warp_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_time_warp_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_time_warp_SpellScript::RemoveInvalidTargets, EFFECT_2, TARGET_UNIT_CASTER_AREA_RAID);
                AfterHit += SpellHitFn(spell_mage_time_warp_SpellScript::ApplyDebuff);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_time_warp_SpellScript();
        }
};

// Alter Time - 127140 (overrided)
class spell_mage_alter_time_overrided : public SpellScriptLoader
{
    public:
        spell_mage_alter_time_overrided() : SpellScriptLoader("spell_mage_alter_time_overrided") { }

        class spell_mage_alter_time_overrided_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_alter_time_overrided_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(SPELL_MAGE_ALTER_TIME))
                        _player->RemoveAura(SPELL_MAGE_ALTER_TIME);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_mage_alter_time_overrided_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_alter_time_overrided_SpellScript();
        }
};

// Alter Time - 110909
class spell_mage_alter_time : public SpellScriptLoader
{
    public:
        spell_mage_alter_time() : SpellScriptLoader("spell_mage_alter_time") { }

        class spell_mage_alter_time_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_alter_time_AuraScript);

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_DEATH)
                        return;

                    if (removeMode == AURA_REMOVE_BY_CANCEL || removeMode == AURA_REMOVE_BY_ENEMY_SPELL)
                        return;

                    std::list<Creature*> mirrorList;
                    _player->GetCreatureListWithEntryInGrid(mirrorList, NPC_PAST_SELF, 50.0f);

                    if (mirrorList.empty())
                        return;

                    for (std::list<Creature*>::const_iterator itr = mirrorList.begin(); itr != mirrorList.end(); ++itr)
                        if (Creature* pMirror = (*itr)->ToCreature())
                            if (TempSummon* pastSelf = pMirror->ToTempSummon())
                                if (pastSelf->IsAlive() && pastSelf->IsInWorld())
                                    if (pastSelf->GetSummoner() && pastSelf->GetSummoner()->GetGUID() == _player->GetGUID())
                                        pastSelf->AI()->DoAction(1);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_alter_time_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_alter_time_AuraScript();
        }
};

// Cold Snap - 11958
class spell_mage_cold_snap : public SpellScriptLoader
{
    public:
        spell_mage_cold_snap() : SpellScriptLoader("spell_mage_cold_snap") { }

        class spell_mage_cold_snap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_cold_snap_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    // Resets cooldown of Ice Block, Frost Nova and Cone of Cold
                    player->RemoveSpellCooldown(SPELL_MAGE_ICE_BLOCK, true);
                    player->RemoveSpellCooldown(SPELL_MAGE_FROST_NOVA, true);
                    player->RemoveSpellCooldown(SPELL_MAGE_CONE_OF_COLD, true);
                }
            }

            void Register()
            {
                // add dummy effect spell handler to Cold Snap
                OnEffectHit += SpellEffectFn(spell_mage_cold_snap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_cold_snap_SpellScript();
        }
};

class spell_mage_incanters_absorbtion_base_AuraScript : public AuraScript
{
    public:
        enum Spells
        {
            SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED = 44413,
            SPELL_MAGE_INCANTERS_ABSORBTION_R1 = 44394,
        };

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED)
                && sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_R1);
        }

        void Trigger(AuraEffect *aurEff, DamageInfo & /*dmgInfo*/, uint32 & absorbAmount)
        {
            Unit* target = GetTarget();

            if (AuraEffect *talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_MAGE_INCANTERS_ABSORBTION_R1, EFFECT_0))
            {
                int32 bp = CalculatePct(absorbAmount, talentAurEff->GetAmount());
                target->CastCustomSpell(target, SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
            }
        }
};

// Incanter's Absorption
class spell_mage_incanters_absorbtion_absorb : public SpellScriptLoader
{
    public:
        spell_mage_incanters_absorbtion_absorb() : SpellScriptLoader("spell_mage_incanters_absorbtion_absorb") { }

        class spell_mage_incanters_absorbtion_absorb_AuraScript : public spell_mage_incanters_absorbtion_base_AuraScript
        {
            PrepareAuraScript(spell_mage_incanters_absorbtion_absorb_AuraScript);

            void Register()
            {
                 AfterEffectAbsorb += AuraEffectAbsorbFn(spell_mage_incanters_absorbtion_absorb_AuraScript::Trigger, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_incanters_absorbtion_absorb_AuraScript();
        }
};

// Incanter's Absorption
class spell_mage_incanters_absorbtion_manashield : public SpellScriptLoader
{
    public:
        spell_mage_incanters_absorbtion_manashield() : SpellScriptLoader("spell_mage_incanters_absorbtion_manashield") { }

        class spell_mage_incanters_absorbtion_manashield_AuraScript : public spell_mage_incanters_absorbtion_base_AuraScript
        {
            PrepareAuraScript(spell_mage_incanters_absorbtion_manashield_AuraScript);

            void Register()
            {
                 AfterEffectManaShield += AuraEffectManaShieldFn(spell_mage_incanters_absorbtion_manashield_AuraScript::Trigger, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_incanters_absorbtion_manashield_AuraScript();
        }
};

// Living Bomb - 44457
class spell_mage_living_bomb : public SpellScriptLoader
{
    public:
        spell_mage_living_bomb() : SpellScriptLoader("spell_mage_living_bomb") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void AfterRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_DEATH && removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (Unit* caster = GetCaster())
                {
                    caster->CastSpell(GetTarget(), SPELL_MAGE_LIVING_BOMB_TRIGGERED, true);
                    handleBrainFreeze();
                }
            }

            void HandleApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                {
                    if (aurEff->GetTotalTicks() - aurEff->GetTickNumber() <= 1)
                    {
                        caster->CastSpell(GetTarget(), SPELL_MAGE_LIVING_BOMB_TRIGGERED, true);
                        handleBrainFreeze();
                    }
                }
            }

            void handleBrainFreeze()
            {
                if (auto caster = GetCaster()->ToPlayer())
                {
                    Player::AuraTargetList const *targets = caster->ToPlayer()->listOfTargetsOfAura(GetId());
                    if (!targets || targets->empty())
                        return;
                    Unit const * const lastTarget = Unit::GetUnit(*caster, targets->back());
                    if (GetTarget() == lastTarget && roll_chance_i(25) && caster->HasAura(SPELL_MAGE_BRAIN_FREEZE))
                        caster->CastSpell(caster, SPELL_MAGE_BRAIN_FREEZE_TRIGGERED, true);
                }
            }

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                    handleBrainFreeze();
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(script_impl::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAPPLY);
                AfterEffectRemove += AuraEffectRemoveFn(script_impl::AfterRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

// Temporal Shield - 115610
class spell_mage_temporal_shield : public SpellScriptLoader
{
public:
    spell_mage_temporal_shield() : SpellScriptLoader("spell_mage_temporal_shield") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void HandleProc(ProcEventInfo& eventInfo)
        {
            Unit* caster = GetCaster();
            if (!caster || !eventInfo.GetDamageInfo())
                return;

            uint32 damage = eventInfo.GetDamageInfo()->GetDamage();
            if (!damage || eventInfo.GetActionTarget()->IsFriendlyTo(eventInfo.GetActor()))
                return;

            uint32 damagePercent = GetSpellInfo()->Effects[EFFECT_0].CalcValue(caster);
            int32 amount = (int32)CalculatePct(damage, damagePercent);

            SpellInfo const* hot = sSpellMgr->GetSpellInfo(115611);
            amount += caster->GetRemainingPeriodicAmount(caster->GetGUID(), 115611, SPELL_AURA_PERIODIC_HEAL).total();
            amount /= hot->GetMaxTicks();
            // Temporary hack for wrong bp per level implementation?
            amount -= caster->getLevel() - 1;

            caster->CastCustomSpell(115611, SPELLVALUE_BASE_POINT0, amount, caster, true);
            PreventDefaultAction();
        }

        void Register()
        {
            OnProc += AuraProcFn(script_impl::HandleProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Flameglow - 140468
class spell_mage_flameglow : public SpellScriptLoader
{
public:
    spell_mage_flameglow() : SpellScriptLoader("spell_mage_flameglow") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void CalculateAmount(AuraEffect const * /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            amount = -1;
        }

        void Absorb(AuraEffect * /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            Unit* target = GetTarget();
            if (target->GetTypeId() != TYPEID_PLAYER)
                return;

            int32 bonusPct = GetAura()->GetSpellInfo()->Effects[EFFECT_1].BasePoints;
            int32 damagePct = GetAura()->GetSpellInfo()->Effects[EFFECT_2].BasePoints;
            // Calculate %s
            uint32 spellAbsorb = CalculatePct(target->ToPlayer()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FIRE), bonusPct);
            uint32 absorbLimit  = CalculatePct(dmgInfo.GetDamage(), damagePct);

            absorbAmount = std::min(spellAbsorb, absorbLimit);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(script_impl::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// 42208 - Blizzard
/// Updated 4.3.4
class spell_mage_blizzard : public SpellScriptLoader
{
public:
    spell_mage_blizzard() : SpellScriptLoader("spell_mage_blizzard") { }

    class spell_mage_blizzard_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_blizzard_SpellScript);

        enum
        {
            SPELL_MAGE_BLIZZARD_SLOW = 12486
        };

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_BLIZZARD_SLOW))
                return false;
            return true;
        }

        void AddChillEffect(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (Unit* unitTarget = GetHitUnit())
                caster->CastSpell(unitTarget, SPELL_MAGE_BLIZZARD_SLOW, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_mage_blizzard_SpellScript::AddChillEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_blizzard_SpellScript();
    }
};

// Frozen orb target filter
class spell_mage_orb_filter : public SpellScriptLoader
{
public:
    spell_mage_orb_filter() : SpellScriptLoader("spell_mage_orb_filter") { }

    class spell_mage_orb_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_orb_filter_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* owner = caster->GetOwner();
            if (!owner)
                return;

            if (Unit* target = GetHitUnit())
            {
                if (!caster->HasAura(82736))
                    caster->CastSpell(caster, 82736);
                if (roll_chance_i(15))
                    owner->CastSpell(owner, 44544, true);

                // After hitting a target periodc timer has to be changed to 1 second
                if (AuraEffect* aura = GetCaster()->GetAuraEffect(84717, EFFECT_0))
                {
                    aura->SetPeriodicTimer(GetSpellInfo()->Effects[EFFECT_0].BasePoints);
                    aura->SetAmplitude(GetSpellInfo()->Effects[EFFECT_0].BasePoints);
                }
            }
        }

        void HandleAfterCast()
        {
            Unit* caster = GetCaster();
            Unit* owner = caster->GetOwner();
            if (!owner)
                return;

            caster->CastSpell(caster, 84721, true, NULL, NULL, owner->GetGUID());
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_mage_orb_filter_SpellScript::HandleAfterCast);
            OnEffectHitTarget += SpellEffectFn(spell_mage_orb_filter_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_orb_filter_SpellScript();
    }
};

// 136511 - Ring of Frost periodic
class spell_mage_ring_of_frost : public SpellScriptLoader
{
public:
    spell_mage_ring_of_frost() : SpellScriptLoader("spell_mage_ring_of_frost") { }

    class spell_mage_ring_of_frost_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_ring_of_frost_AuraScript);

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            std::list<Creature*> MinionList;
            GetTarget()->GetAllMinionsByEntry(MinionList, 44199);
            for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
            {
                TempSummon* ringOfFrost = (*itr)->ToTempSummon();
                GetTarget()->CastSpell(ringOfFrost->GetPositionX(), ringOfFrost->GetPositionY(), ringOfFrost->GetPositionZ(), SPELL_MAGE_RING_OF_FROST_FREEZE, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_ring_of_frost_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_mage_ring_of_frost_AuraScript();
    }
};

// 82691 - Ring of Frost (freeze effect)
class spell_mage_ring_of_frost_freeze : public SpellScriptLoader
{
public:
    spell_mage_ring_of_frost_freeze() : SpellScriptLoader("spell_mage_ring_of_frost_freeze") { }

    class spell_mage_ring_of_frost_freeze_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_ring_of_frost_freeze_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            SpellInfo const* triggeredBy = sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON);
            float inRadius = 4.0f;  // Radius 1 / 2
            float outRadius = 7.25f; // Radius 1 + Radius 0 / 2

            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
            if (Unit* unit = (*itr)->ToUnit())
            {
                if (unit->HasAura(SPELL_MAGE_RING_OF_FROST_DUMMY) || unit->HasAura(SPELL_MAGE_RING_OF_FROST_FREEZE) || unit->GetExactDist(GetExplTargetDest()) > outRadius || unit->GetExactDist(GetExplTargetDest()) < inRadius)
                {
                    WorldObject* temp = (*itr);
                    itr++;
                    targets.remove(temp);
                }
                else
                    itr++;
            }
            else
                itr++;
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_ring_of_frost_freeze_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_ring_of_frost_freeze_SpellScript();
    }

    class spell_mage_ring_of_frost_freeze_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_ring_of_frost_freeze_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_DUMMY))
                return false;
            return true;
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster())
                GetCaster()->CastSpell(GetTarget(), SPELL_MAGE_RING_OF_FROST_DUMMY, true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_mage_ring_of_frost_freeze_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_mage_ring_of_frost_freeze_AuraScript();
    }
};

// Fingers of frost
class spell_mage_fingers_of_frost : public SpellScriptLoader
{
public:
    spell_mage_fingers_of_frost() : SpellScriptLoader("spell_mage_fingers_of_frost") { }

    class spell_mage_fingers_of_frost_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_fingers_of_frost_SpellScript);

        void HandleOnHit()
        {
            Unit* caster = GetCaster();
            uint32 chance = 0;
            if (caster->HasSpell(112965))
            {
                SpellInfo const* info = sSpellMgr->GetSpellInfo(112965);
                switch (GetSpellInfo()->Id)
                {
                    case 116:
                    case 44614:
                        chance = info->Effects[0].BasePoints;
                        break;
                    case 42208:
                        chance = info->Effects[1].BasePoints;
                        break;
                }
            }

            if (roll_chance_i(chance))
                caster->CastSpell(caster, 44544, true); // Main fingers of frost spell, also left side visual
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_mage_fingers_of_frost_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_fingers_of_frost_SpellScript();
    }
};

class spell_mage_ring_of_frost_override : public SpellScriptLoader
{
public:
    spell_mage_ring_of_frost_override() : SpellScriptLoader("spell_mage_ring_of_frost_override") { }

    class spell_mage_ring_of_frost_override_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_ring_of_frost_override_SpellScript);

        void HandleDummy()
        {
            Unit* caster = GetCaster();
            SpellInfo const* info = sSpellMgr->GetSpellInfo(140384);
            SpellCastTargets targets;
            targets.SetDst(GetExplTargetDest()->GetPositionX(), GetExplTargetDest()->GetPositionY(), GetExplTargetDest()->GetPositionZ(), GetExplTargetDest()->GetOrientation());
            Spell* spell = new Spell(GetCaster(), info, TRIGGERED_FULL_MASK, caster->GetGUID());
            spell->InitExplicitTargets(targets);
            
            spell->setState(SPELL_STATE_PREPARING);

            SpellEvent* Event = new SpellEvent(spell);
            caster->m_Events.AddEvent(Event, caster->m_Events.CalculateTime(2 * IN_MILLISECONDS));
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_mage_ring_of_frost_override_SpellScript::HandleDummy);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_ring_of_frost_override_SpellScript();
    }
};

enum GlyphOfIllusion
{
    MAGE_GLYPH_OF_ILLUSION_ACTIVE = 131784
};

class spell_mage_glyph_of_illusion : public SpellScriptLoader
{
    public:
        spell_mage_glyph_of_illusion() : SpellScriptLoader("spell_mage_glyph_of_illusion") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster()->ToPlayer())
                    GetCaster()->ToPlayer()->learnSpell(MAGE_GLYPH_OF_ILLUSION_ACTIVE, false);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster()->ToPlayer())
                    GetCaster()->ToPlayer()->removeSpell(MAGE_GLYPH_OF_ILLUSION_ACTIVE);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(script_impl::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(script_impl::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

enum GlyphOfConjureFamiliar
{
    MAGE_GLYPH_OF_CONJURE_FAMILIAR_ACTIVE = 126578
};

class spell_mage_glyph_of_conjure_familiar : public SpellScriptLoader
{
    public:
        spell_mage_glyph_of_conjure_familiar() : SpellScriptLoader("spell_mage_glyph_of_conjure_familiar") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster()->ToPlayer())
                    GetCaster()->ToPlayer()->learnSpell(MAGE_GLYPH_OF_CONJURE_FAMILIAR_ACTIVE, false);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster()->ToPlayer())
                    GetCaster()->ToPlayer()->removeSpell(MAGE_GLYPH_OF_CONJURE_FAMILIAR_ACTIVE);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(script_impl::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(script_impl::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

class spell_mage_illusion : public SpellScriptLoader
{
    public:
        spell_mage_illusion() : SpellScriptLoader("spell_mage_illusion") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;
                
                target = Trinity::Containers::SelectRandomContainerElement(targets)->ToUnit();
            }

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = target)
                    target->CastSpell(caster, GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            private:
                Unit* target;

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                OnEffectHitTarget += SpellEffectFn(script_impl::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

class spell_mage_glyph_of_momentum : public SpellScriptLoader
{
    public:
        spell_mage_glyph_of_momentum() : SpellScriptLoader("spell_mage_glyph_of_momentum") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            enum
            {
                SPELL_MAGE_GLYPH_OF_MOMENTUM_PASSIVE = 56384,
                SPELL_MAGE_GLYPH_OF_MOMENTUM = 119415

            };

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(SPELL_MAGE_GLYPH_OF_MOMENTUM_PASSIVE))
                    {
                        PreventHitDefaultEffect(effIndex);
                        caster->CastSpell(caster, SPELL_MAGE_GLYPH_OF_MOMENTUM, true);
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(script_impl::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_LEAP);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};
// spell_blink id 1953
class spell_mage_blink : public SpellScriptLoader
{
public:
    spell_mage_blink() : SpellScriptLoader("spell_mage_blink") { }

    class spell_mage_blink_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_blink_SpellScript);

        SpellCastResult CheckCast()
        {
            if (!GetCaster())
                return SPELL_FAILED_DONT_REPORT;

            if (Unit* caster = GetCaster())
            {
                if (caster->HasUnitState(UNIT_STATE_STUNNED))
                    caster->ClearUnitState(UNIT_STATE_STUNNED);
                if (caster->HasUnitState(UNIT_STATE_ROOT))
                    caster->ClearUnitState(UNIT_STATE_ROOT);
                caster->CastSpell(caster, SPELL_MAGE_BLINK, true);
            }

            return SPELL_CAST_OK;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_mage_blink_SpellScript::CheckCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_blink_SpellScript();
    }
};

// 55342 - Mirror Image
class spell_mage_mirror_image : public SpellScriptLoader
{
    public:
        spell_mage_mirror_image() : SpellScriptLoader("spell_mage_mirror_image") { }

        class spell_mage_mirror_image_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_mirror_image_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_GLYPH_OF_MIRROR_IMAGE) ||
                    !sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_IMAGES_ARCANE) ||
                    !sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_IMAGES_FIRE) ||
                    !sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_IMAGES_FROST))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();

                uint32 spellId = SPELL_MAGE_SUMMON_IMAGES_FROST;

                if (Player* player = caster->ToPlayer())
                {
                    bool hasGlyph = false;

                    for (uint32 i = 0; i < MAX_GLYPH_SLOT_INDEX; ++i)
                        if (uint32 glyphId = player->GetGlyph(player->GetActiveSpec(), i))
                            if (GlyphPropertiesEntry const* glyph = sGlyphPropertiesStore.LookupEntry(glyphId))
                                if (glyph->SpellId == SPELL_MAGE_GLYPH_OF_MIRROR_IMAGE)
                                {
                                    hasGlyph = true;
                                    break;
                                }

                    if (hasGlyph)
                    {
                        switch (player->GetSpecializationId(player->GetActiveSpec()))
                        {

                            case SPEC_MAGE_ARCANE:
                                spellId = SPELL_MAGE_SUMMON_IMAGES_ARCANE;
                                break;
                            case SPEC_MAGE_FIRE:
                                spellId = SPELL_MAGE_SUMMON_IMAGES_FIRE;
                                break;
                            case SPEC_MAGE_FROST:
                                spellId = SPELL_MAGE_SUMMON_IMAGES_FROST;
                                break;
                        }
                    }
                }

                caster->CastSpell(caster, spellId, true);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_mage_mirror_image_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_mirror_image_SpellScript();
        }
};

// 2P mage pvp - 131618
class spell_mage_2p_pvp : public SpellScriptLoader
{
    public:
        spell_mage_2p_pvp() : SpellScriptLoader("spell_mage_2p_pvp") { }

        class aura_impl : public AuraScript
        {
            PrepareAuraScript(aura_impl);

            enum
            {
                SPELL_COUNTERSPELL = 2139
            };

            void AfterProc(AuraEffect const *aurEff, ProcEventInfo& eventInfo)
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (Player* player = GetCaster()->ToPlayer())
                    player->ReduceSpellCooldown(SPELL_COUNTERSPELL, 4000);
            }

            void Register()
            {
                AfterEffectProc += AuraEffectProcFn(aura_impl::AfterProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new aura_impl();
        }
};

void AddSC_mage_spell_scripts()
{
    new spell_mage_flamestrike();
    new spell_mage_greater_invisibility_removed();
    new spell_mage_greater_invisibility_triggered();
    new spell_mage_glyph_of_slow();
    new spell_mage_pet_frost_nova();
    new spell_mage_counterspell();
    new spell_mage_glyph_of_ice_block();
    new spell_mage_incanters_ward_cooldown();
    new spell_mage_incanters_ward();
    new spell_mage_arcane_missile();
    new spell_mage_cauterize();
    new spell_mage_pyromaniac();
    new spell_mage_arcane_barrage();
    new spell_mage_arcane_explosion();
    new spell_mage_slow();
    new spell_mage_frostbolt();
    new spell_mage_invocation();
    new spell_mage_frost_bomb();
    new spell_mage_nether_tempest();
    new spell_mage_blazing_speed();
    new spell_mage_frostjaw();
    new spell_mage_combustion();
    new spell_mage_inferno_blast();
    new spell_mage_arcane_brilliance();
    new spell_mage_evocation();
    new spell_mage_conjure_refreshment();
    new spell_mage_time_warp();
    new spell_mage_alter_time_overrided();
    new spell_mage_alter_time();
    new spell_mage_cold_snap();
    new spell_mage_incanters_absorbtion_absorb();
    new spell_mage_incanters_absorbtion_manashield();
    new spell_mage_living_bomb();
    new spell_mage_temporal_shield();
    new spell_mage_flameglow();
    new spell_mastery_icicles();
    new spell_mastery_icicles_trigger();
    new spell_mastery_icicles_periodic();
    new spell_mage_blizzard();
    new spell_mage_orb_filter();
    new spell_mage_inferno_blast_spread();
    new spell_mage_ring_of_frost_freeze();
    new spell_mage_ring_of_frost();
    new spell_mage_fingers_of_frost();
    new spell_mage_ring_of_frost_override();
    new spell_mage_glyph_of_illusion();
    new spell_mage_glyph_of_conjure_familiar();
    new spell_mage_illusion();
    new spell_mage_glyph_of_momentum();
    new spell_mage_mirror_image();
    new spell_mage_2p_pvp();
    new spell_mage_blink();
}
