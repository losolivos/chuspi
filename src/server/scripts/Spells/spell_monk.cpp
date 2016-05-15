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
 * Scripts for spells with SPELLFAMILY_MONK and SPELLFAMILY_GENERIC spells used by monk players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_monk_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"
#include "ObjectVisitors.hpp"

enum MonkSpells
{
    SPELL_MONK_LEGACY_OF_THE_EMPEROR            = 117667,
    SPELL_MONK_FORTIFYING_BREW                  = 120954,
    SPELL_MONK_PROVOKE                          = 118635,
    SPELL_MONK_BLACKOUT_KICK_DOT                = 128531,
    SPELL_MONK_BLACKOUT_KICK_HEAL               = 128591,
    SPELL_MONK_SHUFFLE                          = 115307,
    SPELL_MONK_ZEN_PILGRIMAGE                   = 126892,
    SPELL_MONK_ZEN_PILGRIMAGE_RETURN            = 126895,
    SPELL_MONK_DISABLE_ROOT                     = 116706,
    SPELL_MONK_DISABLE                          = 116095,
    SPELL_MONK_SOOTHING_MIST_VISUAL             = 125955,
    SPELL_MONK_SOOTHING_MIST_ENERGIZE           = 116335,
    SPELL_MONK_BREATH_OF_FIRE_DOT               = 123725,
    SPELL_MONK_BREATH_OF_FIRE_CONFUSED          = 123393,
    SPELL_MONK_ELUSIVE_BREW_STACKS              = 128939,
    SPELL_MONK_ELUSIVE_BREW                     = 115308,
    SPELL_MONK_KEG_SMASH_VISUAL                 = 123662,
    SPELL_MONK_KEG_SMASH_ENERGIZE               = 127796,
    SPELL_MONK_WEAKENED_BLOWS                   = 115798,
    SPELL_MONK_DIZZYING_HAZE                    = 116330,
    SPELL_MONK_CLASH_CHARGE                     = 126452,
    SPELL_MONK_LIGHT_STAGGER                    = 124275,
    SPELL_MONK_MODERATE_STAGGER                 = 124274,
    SPELL_MONK_HEAVY_STAGGER                    = 124273,
    SPELL_MONK_ROLL                             = 109132,
    SPELL_MONK_ROLL_TRIGGER                     = 107427,
    SPELL_MONK_CHI_TORPEDO_HEAL                 = 124040,
    SPELL_MONK_CHI_TORPEDO_DAMAGE               = 117993,
    SPELL_MONK_FLYING_SERPENT_KICK              = 101545,
    SPELL_MONK_FLYING_SERPENT_KICK_NEW          = 115057,
    SPELL_MONK_FLYING_SERPENT_KICK_AOE          = 123586,
    SPELL_MONK_TIGEREYE_BREW                    = 116740,
    SPELL_MONK_TIGEREYE_BREW_STACKS             = 125195,
    SPELL_MONK_SPEAR_HAND_STRIKE_SILENCE        = 116709,
    SPELL_MONK_CHI_BURST_DAMAGE                 = 148135,
    SPELL_MONK_CHI_BURST_HEAL                   = 130654,
    SPELL_MONK_ZEN_SPHERE_DAMAGE                = 124098,
    SPELL_MONK_ZEN_SPHERE_HEAL                  = 124081,
    SPELL_MONK_ZEN_SPHERE_DETONATE_HEAL         = 124101,
    SPELL_MONK_ZEN_SPHERE_DETONATE_DAMAGE       = 125033,
    SPELL_MONK_HEALING_ELIXIRS_AURA             = 122280,
    SPELL_MONK_HEALING_ELIXIRS_RESTORE_HEALTH   = 122281,
    SPELL_MONK_RENEWING_MIST_HOT                = 119611,
    SPELL_MONK_RENEWING_MIST_JUMP_AURA          = 119607,
    SPELL_MONK_GLYPH_OF_RENEWING_MIST           = 123334,
    SPELL_MONK_SURGING_MIST_HEAL                = 116995,
    SPELL_MONK_ENVELOPING_MIST_HEAL             = 132120,
    SPELL_MONK_PLUS_ONE_MANA_TEA                = 123760,
    SPELL_MONK_MANA_TEA_STACKS                  = 115867,
    SPELL_MONK_MANA_TEA_REGEN                   = 115294,
    SPELL_MONK_TEACHINGS_OF_THE_MONASTERY       = 118672,
    SPELL_MONK_SPINNING_CRANE_KICK_HEAL         = 117640,
    MONK_NPC_JADE_SERPENT_STATUE                = 60849,
    SPELL_MONK_UPLIFT_ALLOWING_CAST             = 123757,
    SPELL_MONK_THUNDER_FOCUS_TEA                = 116680,
    SPELL_MONK_PATH_OF_BLOSSOM_AREATRIGGER      = 122035,
    SPELL_MONK_SPINNING_FIRE_BLOSSOM_DAMAGE     = 123408,
    SPELL_MONK_SPINNING_FIRE_BLOSSOM_MISSILE    = 118852,
    SPELL_MONK_SPINNING_FIRE_BLOSSOM_ROOT       = 123407,
    SPELL_MONK_TOUCH_OF_KARMA_REDIRECT_DAMAGE   = 124280,
    SPELL_MONK_JADE_LIGHTNING_ENERGIZE          = 123333,
    SPELL_MONK_CRACKLING_JADE_SHOCK_BUMP        = 117962,
    SPELL_MONK_POWER_STRIKES_TALENT             = 121817,
    SPELL_MONK_POWER_STRIKES_BUFF               = 129914,
    SPELL_MONK_CREATE_CHI_SPHERE                = 121286,
    SPELL_MONK_GLYPH_OF_ZEN_FLIGHT              = 125893,
    SPELL_MONK_ZEN_FLIGHT                       = 125883,
    SPELL_MONK_BEAR_HUG                         = 127361,
    SPELL_MONK_ITEM_2_S12_MISTWEAVER            = 131561,
    SPELL_MONK_ITEM_4_S12_MISTWEAVER            = 124487,
    SPELL_MONK_ZEN_FOCUS                        = 124488,
    SPELL_MONK_EMINENCE_HEAL                    = 126890,
    SPELL_MONK_EMINENCE_HEAL_STATUE             = 117895,
    SPELL_MONK_GRAPPLE_WEAPON_DPS_UPGRADE       = 123231,
    SPELL_MONK_GRAPPLE_WEAPON_TANK_UPGRADE      = 123232,
    SPELL_MONK_GRAPPLE_WEAPON_HEAL_UPGRADE      = 123234,
    SPELL_MONK_GLYPH_OF_BLACKOUT_KICK           = 132005,
    SPELL_MONK_CHI_WAVE_HEAL                    = 132463,
    SPELL_MONK_CHI_WAVE_DAMAGE                  = 132467,
    SPELL_MONK_CHI_WAVE_HEALING_BOLT            = 132464,
    SPELL_MONK_CHI_WAVE_TALENT_AURA             = 115098,
    SPELL_MONK_ITEM_PVP_GLOVES_BONUS            = 124489,
	SPELL_MONK_STORM_EARTH_AND_FIRE				= 137639,
};

// Fists of Fury (stun effect) - 120086
class spell_monk_fists_of_fury_stun : public SpellScriptLoader
{
    public:
        spell_monk_fists_of_fury_stun() : SpellScriptLoader("spell_monk_fists_of_fury_stun") { }

        class spell_monk_fists_of_fury_stun_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_fists_of_fury_stun_SpellScript);

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, GetSpellInfo()->Id));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_fists_of_fury_stun_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_24);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_fists_of_fury_stun_SpellScript();
        }
};

// Expel Harm - 115072
class spell_monk_expel_harm : public SpellScriptLoader
{
    public:
        spell_monk_expel_harm() : SpellScriptLoader("spell_monk_expel_harm") { }

        class spell_monk_expel_harm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_expel_harm_SpellScript);

            void HandleOnHit()
            {
                if (auto caster = GetCaster())
                {
                    int32 bp = CalculatePct(GetHitHeal(), 50);
                    caster->CastCustomSpell(caster, 115129, &bp, NULL, NULL, true);
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_monk_expel_harm_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_expel_harm_SpellScript();
        }
};

// Chi Wave (healing bolt) - 132464
class spell_monk_chi_wave_healing_bolt : public SpellScriptLoader
{
    public:
        spell_monk_chi_wave_healing_bolt() : SpellScriptLoader("spell_monk_chi_wave_healing_bolt") { }

        class spell_monk_chi_wave_healing_bolt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_chi_wave_healing_bolt_SpellScript);

            void HandleOnHit()
            {
                if (!GetOriginalCaster())
                    return;

                if (Player* _player = GetOriginalCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, SPELL_MONK_CHI_WAVE_HEAL, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_chi_wave_healing_bolt_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_chi_wave_healing_bolt_SpellScript();
        }
};

// Chi Wave (damage) - 132467 and Chi Wave (heal) - 132463
class spell_monk_chi_wave_bolt : public SpellScriptLoader
{
    public:
        spell_monk_chi_wave_bolt() : SpellScriptLoader("spell_monk_chi_wave_bolt") { }

        class spell_monk_chi_wave_bolt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_chi_wave_bolt_SpellScript);

            void HandleOnHit()
            {
                auto caster = GetOriginalCaster();
                auto target = GetHitUnit();
                if (!caster || !target)
                    return;

                if (auto player = caster->ToPlayer())
                {
                    uint8 count = 0;
                    bool requireFriendly = true;

                    auto chiWave = player->GetAuraEffect(SPELL_MONK_CHI_WAVE_TALENT_AURA, EFFECT_1);
                    if (chiWave)
                    {
                        count = chiWave->GetAmount();

                        // If we start with healing - make sure to trigger unfriendly requirement
                        if (chiWave->GetUserData() == 1 || (count == 1 && GetSpellInfo()->Id == 132463))
                            requireFriendly = false;

                        if (count >= 7 || (!GetHitDamage() && !GetHitHeal()))
                        {
                            player->RemoveAura(SPELL_MONK_CHI_WAVE_TALENT_AURA);
                            return;
                        }

                        count++;
                        chiWave->SetAmount(count);
                    }
                    else
                        return;

                    std::list<Unit*> targetList;

                    {
                        Trinity::AnyUnitInObjectRangeCheck u_check(player, 20.f);
                        Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(player, targetList, u_check);
                        Trinity::VisitNearbyObject(player, 20.0f, searcher);
                    }

                    targetList.remove_if([player, target, requireFriendly](Unit const *obj)
                    {
                        return (!obj->IsWithinLOSInMap(player) || obj == target ||
                            (requireFriendly ? false : (obj->IsFriendlyTo(player) || !player->IsValidAttackTarget(obj)) || !obj->IsInCombat()) );
                    });

                    if (targetList.empty())
                    {
                        player->RemoveAurasDueToSpell(SPELL_MONK_CHI_WAVE_TALENT_AURA);
                        return;
                    }

                    if (requireFriendly)
                    {
                        targetList.sort(Trinity::HealthPctOrderPred());
                        chiWave->SetUserData(1);
                        target->CastSpell(targetList.front(), SPELL_MONK_CHI_WAVE_HEALING_BOLT, true, NULL, NULL, player->GetGUID());
                    }
                    else
                    {
                        // Select random target
                        auto randomTarget = targetList.begin();
                        std::advance(randomTarget, urand(0, targetList.size() - 1));

                        target->CastSpell(*randomTarget, SPELL_MONK_CHI_WAVE_DAMAGE, true, NULL, NULL, player->GetGUID());
                        chiWave->SetUserData(0);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_chi_wave_bolt_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_chi_wave_bolt_SpellScript();
        }
};

// Chi Wave (talent) - 115098
class spell_monk_chi_wave : public SpellScriptLoader
{
    public:
        spell_monk_chi_wave() : SpellScriptLoader("spell_monk_chi_wave") { }

        class spell_monk_chi_wave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_chi_wave_SpellScript);

            uint64 targetGUID;
            bool done;

            bool Load()
            {
                targetGUID = 0;
                done = false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                    targetGUID = target->GetGUID();
            }

            void HandleApplyAura()
            {
                if (!targetGUID || done)
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = sObjectAccessor->FindUnit(targetGUID))
                    {
                        _player->CastSpell(target, _player->IsValidAttackTarget(target) ? SPELL_MONK_CHI_WAVE_DAMAGE : SPELL_MONK_CHI_WAVE_HEALING_BOLT, true);
                        done = true;
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_chi_wave_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                AfterHit += SpellHitFn(spell_monk_chi_wave_SpellScript::HandleApplyAura);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_chi_wave_SpellScript();
        }
};

// Grapple Weapon - 117368
class spell_monk_grapple_weapon : public SpellScriptLoader
{
    public:
        spell_monk_grapple_weapon() : SpellScriptLoader("spell_monk_grapple_weapon") { }

        class spell_monk_grapple_weapon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_grapple_weapon_SpellScript)

            void HandleBeforeHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->ToPlayer())
                        {
                            Item* mainItem = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                            Item* targetMainItem = target->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

                            if (mainItem && targetMainItem)
                            {
                                if (targetMainItem->GetTemplate()->ItemLevel > mainItem->GetTemplate()->ItemLevel)
                                {
                                    switch (_player->GetSpecializationId(_player->GetActiveSpec()))
                                    {
                                        case SPEC_MONK_BREWMASTER:
                                            _player->CastSpell(_player, SPELL_MONK_GRAPPLE_WEAPON_TANK_UPGRADE, true);
                                            break;
                                        case SPEC_MONK_MISTWEAVER:
                                            _player->CastSpell(_player, SPELL_MONK_GRAPPLE_WEAPON_HEAL_UPGRADE, true);
                                            break;
                                        case SPEC_MONK_WINDWALKER:
                                            _player->CastSpell(_player, SPELL_MONK_GRAPPLE_WEAPON_DPS_UPGRADE, true);
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }
                        }
                        else if (target->GetTypeId() == TYPEID_UNIT)
                        {
                            if (target->getLevel() > _player->getLevel())
                            {
                                switch (_player->GetSpecializationId(_player->GetActiveSpec()))
                                {
                                    case SPEC_MONK_BREWMASTER:
                                        _player->CastSpell(_player, SPELL_MONK_GRAPPLE_WEAPON_TANK_UPGRADE, true);
                                        break;
                                    case SPEC_MONK_MISTWEAVER:
                                        _player->CastSpell(_player, SPELL_MONK_GRAPPLE_WEAPON_HEAL_UPGRADE, true);
                                        break;
                                    case SPEC_MONK_WINDWALKER:
                                        _player->CastSpell(_player, SPELL_MONK_GRAPPLE_WEAPON_DPS_UPGRADE, true);
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                }
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_monk_grapple_weapon_SpellScript::HandleBeforeHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_grapple_weapon_SpellScript();
        }
};

enum Transcendence
{
    SPELL_TRANSCENDENCE_NPC_ENTRY = 54569
};

// Transcendence : Transfer - 101643
class spell_monk_transcendence : public SpellScriptLoader
{
    public:
        spell_monk_transcendence() : SpellScriptLoader("spell_monk_transcendence") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void HandleDummy()
            {
                if (auto player = GetCaster()->ToPlayer())
                    player->RemoveAllMinionsByEntry(SPELL_TRANSCENDENCE_NPC_ENTRY);
            }

            void Register()
            {
                BeforeCast += SpellCastFn(script_impl::HandleDummy);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

// Transcendence : Transfer - 119996
class spell_monk_transcendence_transfer : public SpellScriptLoader
{
    public:
        spell_monk_transcendence_transfer() : SpellScriptLoader("spell_monk_transcendence_transfer") { }

        class spell_monk_transcendence_transfer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_transcendence_transfer_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (auto player = GetCaster()->ToPlayer())
                    if (Unit* pet = player->GetFirstMinionByEntry(SPELL_TRANSCENDENCE_NPC_ENTRY))
                    {
                        if (player->GetDistance(pet) > 40.f)
                            return;

                        if (pet->ToCreature()->AI())
                            pet->ToCreature()->AI()->DoAction(1);
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_transcendence_transfer_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_transcendence_transfer_SpellScript();
        }
};

// Serpent's Zeal - 127722
class spell_monk_serpents_zeal : public SpellScriptLoader
{
    public:
        spell_monk_serpents_zeal() : SpellScriptLoader("spell_monk_serpents_zeal") { }

        class spell_monk_serpents_zeal_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_serpents_zeal_AuraScript);

            void OnProc(AuraEffect const *aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                if (eventInfo.GetActor()->GetGUID() != GetTarget()->GetGUID())
                    return;

                auto damageInfo = eventInfo.GetDamageInfo();
                int32 bp = damageInfo->GetDamage();

                if (damageInfo->GetSpellInfo() || !bp)
                    return;

                if (damageInfo->GetAttackType() != BASE_ATTACK && damageInfo->GetAttackType() != OFF_ATTACK)
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    std::list<Creature*> statueList;

                    if (Aura *serpentsZeal = _player->GetAura(aurEff->GetSpellInfo()->Id))
                    {
                        if (serpentsZeal->GetStackAmount() < 2)
                            bp /= 4;
                        else
                            bp /= 2;
                    }

                    _player->GetCreatureListWithEntryInGrid(statueList, MONK_NPC_JADE_SERPENT_STATUE, 100.0f);
                    // Remove other players jade statue
                    for (auto i = statueList.begin(); i != statueList.end();)
                    {
                        Unit* owner = (*i)->GetOwner();
                        if (owner && owner == _player && (*i)->IsSummon())
                            ++i;
                        else
                            i = statueList.erase(i);
                    }

                    // you gain Serpent's Zeal causing you to heal nearby injured targets equal to 25% of your auto-attack damage. Stacks up to 2 times.
                    _player->CastCustomSpell(_player, SPELL_MONK_EMINENCE_HEAL, &bp, NULL, NULL, true);

                    if (!statueList.empty())
                        if (auto statue = statueList.front())
                            if (statue->GetOwnerGUID() == _player->GetGUID())
                                statue->CastCustomSpell(statue, SPELL_MONK_EMINENCE_HEAL_STATUE, &bp, NULL, NULL, true, 0, NULL, _player->GetGUID()); // Eminence - statue
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_monk_serpents_zeal_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_serpents_zeal_AuraScript();
        }
};

// Dampen Harm - 122278
class spell_monk_dampen_harm : public SpellScriptLoader
{
    public:
        spell_monk_dampen_harm() : SpellScriptLoader("spell_monk_dampen_harm") { }

        class spell_monk_dampen_harm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_dampen_harm_AuraScript);

            uint32 healthPct;

            bool Load()
            {
                healthPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return GetUnitOwner()->ToPlayer();
            }

            void CalculateAmount(AuraEffect const * /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect *auraEffect, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                Unit* target = GetTarget();

                uint32 health = target->CountPctFromMaxHealth(healthPct);

                if (dmgInfo.GetDamage() < health)
                    return;

                // The next 3 attacks within 45 sec that deal damage equal to 10% or more of your total health are reduced by half
                absorbAmount = dmgInfo.GetDamage() / 2;
                auraEffect->GetBase()->DropCharge();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_dampen_harm_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_dampen_harm_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_dampen_harm_AuraScript();
        }
};

// Called by Thunder Focus Tea - 116680
// Item S12 4P - Mistweaver - 124487
class spell_monk_item_s12_4p_mistweaver : public SpellScriptLoader
{
    public:
        spell_monk_item_s12_4p_mistweaver() : SpellScriptLoader("spell_monk_item_s12_4p_mistweaver") { }

        class spell_monk_item_s12_4p_mistweaver_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_item_s12_4p_mistweaver_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(SPELL_MONK_ITEM_4_S12_MISTWEAVER))
                        _player->CastSpell(_player, SPELL_MONK_ZEN_FOCUS, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_item_s12_4p_mistweaver_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_item_s12_4p_mistweaver_SpellScript();
        }
};

struct auraData
{
    auraData(uint32 id, uint64 casterGUID) : m_id(id), m_casterGuid(casterGUID) {}
    uint32 m_id;
    uint64 m_casterGuid;
};

// Diffuse Magic - 122783
class spell_monk_diffuse_magic : public SpellScriptLoader
{
    public:
        spell_monk_diffuse_magic() : SpellScriptLoader("spell_monk_diffuse_magic") { }

        class spell_monk_diffuse_magic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_diffuse_magic_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    std::set<auraData*> auraListToRemove;
                    Unit::AuraApplicationMap AuraList = _player->GetAppliedAuras();
                    for (Unit::AuraApplicationMap::iterator iter = AuraList.begin(); iter != AuraList.end(); ++iter)
                    {
                        Aura *aura = iter->second->GetBase();
                        if (!aura)
                            continue;

                        Unit* caster = aura->GetCaster();
                        if (!caster || caster->GetGUID() == _player->GetGUID())
                            continue;

                        if (!caster->IsWithinDist(_player, 40.0f))
                            continue;

                        if (aura->GetSpellInfo()->IsPositive() || aura->IsPassive())
                            continue;

                        if (!(aura->GetSpellInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC))
                            continue;

                        // We need to manually apply Diminishing Return duration
                        if (auto addedAura = _player->AddAura(aura->GetSpellInfo()->Id, caster))
                        {
                            DiminishingGroup m_diminishGroup = GetDiminishingReturnsGroupForSpell(aura->GetSpellInfo(), false);
                            int32 duration = addedAura->GetMaxDuration();
                            int32 limitduration = GetDiminishingReturnsLimitDuration(m_diminishGroup, addedAura->GetSpellInfo());
                            DiminishingLevels m_diminishLevel = caster->GetDiminishing(m_diminishGroup);
                            float diminishMod = caster->ApplyDiminishingToDuration(m_diminishGroup, duration, _player, m_diminishLevel, limitduration);
                            addedAura->SetDuration(duration, true);
                            addedAura->SetMaxDuration(duration);
                        }

                        if (Aura *targetAura = caster->GetAura(aura->GetSpellInfo()->Id, _player->GetGUID()))
                        {
                            for (size_t i = 0; i < targetAura->GetSpellInfo()->Effects.size(); ++i)
                            {
                                if (targetAura->GetEffect(i) && aura->GetEffect(i))
                                {
                                    AuraEffect *auraEffect = _player->GetAuraEffect(aura->GetSpellInfo()->Id, i);
                                    if (!auraEffect)
                                        continue;

                                    int32 damage = auraEffect->GetAmount();

                                    if (auraEffect->GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE ||
                                        auraEffect->GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE_PERCENT)
                                        damage = caster->SpellDamageBonusDone(_player, aura->GetSpellInfo(), i, damage, DOT, auraEffect->GetBase()->GetStackAmount());

                                    targetAura->GetEffect(i)->SetAmount(damage);
                                }
                            }
                        }

                        auraListToRemove.insert(new auraData(aura->GetSpellInfo()->Id, caster->GetGUID()));
                    }

                    for (std::set<auraData*>::iterator itr = auraListToRemove.begin(); itr != auraListToRemove.end(); ++itr)
                    {
                        _player->RemoveAura((*itr)->m_id, (*itr)->m_casterGuid);
                        delete (*itr);
                    }

                    auraListToRemove.clear();
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_diffuse_magic_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_diffuse_magic_SpellScript();
        }
};

// Guard - 115295
class spell_monk_guard final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            GLYPH_OF_GUARD      = 123401,
            POWER_GUARD         = 118636,
        };

        void calculateAmount(AuraEffect const *, int32 &amount, bool &canBeRecalculated)
        {
            canBeRecalculated = false;

            auto const caster = GetCaster();
            if (!caster)
                return;

            amount += caster->GetTotalAttackPowerValue(BASE_ATTACK) * 1.971f;

            // Glyph of Guard
            if (auto const eff = caster->GetAuraEffect(GLYPH_OF_GUARD, EFFECT_0))
                AddPct(amount, eff->GetAmount());

            // Power Guard
            if (auto const eff = caster->GetAuraEffect(POWER_GUARD, EFFECT_0))
            {
                AddPct(amount, eff->GetAmount());
                eff->GetBase()->Remove();
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::calculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        }
    };

public:
    spell_monk_guard()
        : SpellScriptLoader("spell_monk_guard")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Bear Hug - 127361
class spell_monk_bear_hug : public SpellScriptLoader
{
    public:
        spell_monk_bear_hug() : SpellScriptLoader("spell_monk_bear_hug") { }

        class spell_monk_bear_hug_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_bear_hug_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (auto bearHug = target->GetAuraEffect(SPELL_MONK_BEAR_HUG, EFFECT_1, _player->GetGUID()))
                            bearHug->SetAmount(_player->CountPctFromMaxHealth(2));
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_bear_hug_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_bear_hug_SpellScript();
        }
};

// Zen Flight - 125883
class spell_monk_zen_flight_check : public SpellScriptLoader
{
    public:
        spell_monk_zen_flight_check() : SpellScriptLoader("spell_monk_zen_flight_check") { }

        class spell_monk_zen_flight_check_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_zen_flight_check_SpellScript);

            SpellCastResult CheckTarget()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->GetMap()->IsBattlegroundOrArena())
                        return SPELL_FAILED_NOT_IN_BATTLEGROUND;

                    // In Kalimdor or Eastern Kingdom with Flight Master's License
                    if (!_player->HasSpell(90267) && (_player->GetMapId() == 1 || _player->GetMapId() == 0))
                        return SPELL_FAILED_NOT_HERE;

                    // In Pandaria with Wisdom of the Four Winds
                    if (!_player->HasSpell(115913) && (_player->GetMapId() == 870))
                        return SPELL_FAILED_NOT_HERE;
                }

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_zen_flight_check_SpellScript::CheckTarget);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_zen_flight_check_SpellScript();
        }

        class spell_monk_zen_flight_check_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_zen_flight_check_AuraScript);

            bool Load()
            {
                return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (!GetCaster())
                    return;

                if (Player* caster = GetCaster()->ToPlayer())
                    if (caster->GetSkillValue(SKILL_RIDING) >= 375)
                        amount = 310;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_zen_flight_check_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_zen_flight_check_AuraScript();
        }
};

// Glyph of Zen Flight - 125893
class spell_monk_glyph_of_zen_flight : public SpellScriptLoader
{
    public:
        spell_monk_glyph_of_zen_flight() : SpellScriptLoader("spell_monk_glyph_of_zen_flight") { }

        class spell_monk_glyph_of_zen_flight_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_glyph_of_zen_flight_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(SPELL_MONK_ZEN_FLIGHT, false);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(SPELL_MONK_ZEN_FLIGHT))
                        _player->removeSpell(SPELL_MONK_ZEN_FLIGHT, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_monk_glyph_of_zen_flight_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_monk_glyph_of_zen_flight_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_glyph_of_zen_flight_AuraScript();
        }
};

// Power Strikes - 121817
class spell_monk_power_strikes : public SpellScriptLoader
{
public:
    spell_monk_power_strikes() : SpellScriptLoader("spell_monk_power_strikes") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void OnProc(AuraEffect const *aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            auto caster = GetCaster();
            auto target = eventInfo.GetProcTarget();
            if (!caster || !target)
                return;

            auto player = caster->ToPlayer();
            if (!player)
                return;

            if (!player->HasAura(SPELL_MONK_POWER_STRIKES_BUFF))
                return;

            if (!player->HasSpellCooldown(SPELL_MONK_POWER_STRIKES_TALENT))
            {
                if (player->GetPower(POWER_CHI) < player->GetMaxPower(POWER_CHI))
                    player->EnergizeBySpell(player, GetSpellInfo()->Id, 1, POWER_CHI);
                else
                    player->CastSpell(player, SPELL_MONK_CREATE_CHI_SPHERE, true);

                player->AddSpellCooldown(SPELL_MONK_POWER_STRIKES_TALENT, 0, 20 * IN_MILLISECONDS);
                player->RemoveAurasDueToSpell(SPELL_MONK_POWER_STRIKES_BUFF);
            }
        }

        void OnTick(AuraEffect const * aurEff)
        {
            auto caster = GetCaster();
            if (caster && caster->GetTypeId() == TYPEID_PLAYER)
                if (caster->ToPlayer()->HasSpellCooldown(SPELL_MONK_POWER_STRIKES_TALENT))
                {
                    PreventDefaultAction();
                    const_cast<AuraEffect*>(aurEff)->ResetPeriodic(true);
                }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(script_impl::OnProc, EFFECT_1, SPELL_AURA_DUMMY);
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Crackling Jade Lightning - 117952
class spell_monk_crackling_jade_lightning : public SpellScriptLoader
{
    public:
        spell_monk_crackling_jade_lightning() : SpellScriptLoader("spell_monk_crackling_jade_lightning") { }

        class spell_monk_crackling_jade_lightning_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_crackling_jade_lightning_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                    if (roll_chance_i(30))
                        caster->CastSpell(caster, SPELL_MONK_JADE_LIGHTNING_ENERGIZE, true);
            }

            void OnProc(AuraEffect const *aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                if (eventInfo.GetActor()->GetGUID() != GetTarget()->GetGUID())
                    return;

                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetTarget()->HasAura(aurEff->GetSpellInfo()->Id, player->GetGUID()))
                    {
                        if (!player->HasSpellCooldown(SPELL_MONK_CRACKLING_JADE_SHOCK_BUMP))
                        {
                            player->CastSpell(GetTarget(), SPELL_MONK_CRACKLING_JADE_SHOCK_BUMP, true);
                            player->AddSpellCooldown(SPELL_MONK_CRACKLING_JADE_SHOCK_BUMP, 0, 8 * IN_MILLISECONDS);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_crackling_jade_lightning_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                OnEffectProc += AuraEffectProcFn(spell_monk_crackling_jade_lightning_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_crackling_jade_lightning_AuraScript();
        }
};

// Touch of Karma - 122470
class spell_monk_touch_of_karma : public SpellScriptLoader
{
    public:
        spell_monk_touch_of_karma() : SpellScriptLoader("spell_monk_touch_of_karma") { }

        class spell_monk_touch_of_karma_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_touch_of_karma_AuraScript);

            void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (GetCaster())
                    amount = GetCaster()->GetMaxHealth();
            }

            void OnAbsorb(AuraEffect *aurEff, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
            {
                if (Unit* caster = dmgInfo.GetVictim())
                {
                    if (Unit* attacker = dmgInfo.GetAttacker())
                    {
                        if (attacker->HasAura(aurEff->GetSpellInfo()->Id, caster->GetGUID()))
                        {
                            int32 damage = dmgInfo.GetDamage();
                            if (auto eff = attacker->HasAura(SPELL_MONK_TOUCH_OF_KARMA_REDIRECT_DAMAGE, caster->GetGUID()))
                                damage += attacker->GetRemainingPeriodicAmount(caster->GetGUID(), SPELL_MONK_TOUCH_OF_KARMA_REDIRECT_DAMAGE, SPELL_AURA_PERIODIC_DAMAGE).total();

                            caster->CastCustomSpell(SPELL_MONK_TOUCH_OF_KARMA_REDIRECT_DAMAGE, SPELLVALUE_BASE_POINT0, (damage / 6), attacker);
                        }
                    }
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_touch_of_karma_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_touch_of_karma_AuraScript::OnAbsorb, EFFECT_1);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_touch_of_karma_AuraScript();
        }
};

// Spinning Fire Blossom - 123408
class spell_monk_spinning_fire_blossom_damage : public SpellScriptLoader
{
    public:
        spell_monk_spinning_fire_blossom_damage() : SpellScriptLoader("spell_monk_spinning_fire_blossom_damage") { }

        class spell_monk_spinning_fire_blossom_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_spinning_fire_blossom_damage_SpellScript);

            SpellCastResult CheckTarget()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetExplTargetUnit())
                        if (_player->IsFriendlyTo(target))
                            return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void HandleAfterHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->GetDistance(_player) > 10.0f)
                        {
                            SetHitDamage(int32(GetHitDamage() * 1.5f));
                            _player->CastSpell(target, SPELL_MONK_SPINNING_FIRE_BLOSSOM_ROOT, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_spinning_fire_blossom_damage_SpellScript::CheckTarget);
                AfterHit += SpellHitFn(spell_monk_spinning_fire_blossom_damage_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_spinning_fire_blossom_damage_SpellScript();
        }
};

// Spinning Fire Blossom - 115073
class spell_monk_spinning_fire_blossom : public SpellScriptLoader
{
    public:
        spell_monk_spinning_fire_blossom() : SpellScriptLoader("spell_monk_spinning_fire_blossom") { }

        class spell_monk_spinning_fire_blossom_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_spinning_fire_blossom_SpellScript);

            bool bonusDamage;
            bool Load()
            {
                bonusDamage = false;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                Unit* caster = GetCaster();
                float dist = 50.0f;
                WorldObject* closestTarget = NULL;
                for (auto itr : unitList)
                {
                    if (itr->GetDistance(caster) < dist)
                    {
                        dist = itr->GetDistance(caster);
                        closestTarget = itr;
                    }
                }

                unitList.clear();
                if (closestTarget)
                {
                    unitList.push_back(closestTarget);

                    if (closestTarget->GetDistance(caster) > 10.0f)
                        bonusDamage = true;
                }
            }

            void HandleOnHit()
            {
                // If Spinning Fire Blossom travels further than 10 yards, the damage is increased by 50%
                if (bonusDamage)
                {
                    SetHitDamage(GetHitDamage() * 1.5f);
                    GetCaster()->CastSpell(GetHitUnit(), 123407, true);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_spinning_fire_blossom_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_129);
                OnHit += SpellHitFn(spell_monk_spinning_fire_blossom_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_spinning_fire_blossom_SpellScript();
        }
};

// Path of Blossom - 124336
class spell_monk_path_of_blossom : public SpellScriptLoader
{
    public:
        spell_monk_path_of_blossom() : SpellScriptLoader("spell_monk_path_of_blossom") { }

        class spell_monk_path_of_blossom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_path_of_blossom_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                    GetCaster()->CastSpell(GetCaster(), SPELL_MONK_PATH_OF_BLOSSOM_AREATRIGGER, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_path_of_blossom_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_path_of_blossom_AuraScript();
        }
};

// Called by Uplift - 116670 and Uplift - 130316
// Thunder Focus Tea - 116680
class spell_monk_thunder_focus_tea : public SpellScriptLoader
{
    public:
        spell_monk_thunder_focus_tea() : SpellScriptLoader("spell_monk_thunder_focus_tea") { }

        class spell_monk_thunder_focus_tea_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_thunder_focus_tea_SpellScript)

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(Trinity::UnitAuraCheck(false, SPELL_MONK_RENEWING_MIST_HOT, GetCaster()->GetGUID()));
            }

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        if (player->HasAura(SPELL_MONK_THUNDER_FOCUS_TEA))
                        {
                            std::list<Unit*> groupList;
                            player->GetPartyMembers(groupList);

                            for (auto itr : groupList)
                                if (Aura *renewingMistGroup = itr->GetAura(SPELL_MONK_RENEWING_MIST_HOT, player->GetGUID()))
                                    renewingMistGroup->RefreshDuration();

                            player->RemoveAura(SPELL_MONK_THUNDER_FOCUS_TEA);
                        }
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_thunder_focus_tea_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
                OnHit += SpellHitFn(spell_monk_thunder_focus_tea_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_thunder_focus_tea_SpellScript();
        }
};

// Summon Jade Serpent Statue - 115313
class spell_monk_jade_serpent_statue : public SpellScriptLoader
{
    public:
        spell_monk_jade_serpent_statue() : SpellScriptLoader("spell_monk_jade_serpent_statue") { }

        class spell_monk_jade_serpent_statue_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_jade_serpent_statue_SpellScript)

            void HandleSummon(SpellEffIndex effIndex)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    PreventHitDefaultEffect(effIndex);

                    const SpellInfo* spell = GetSpellInfo();
                    std::list<Creature*> tempList;
                    std::list<Creature*> jadeSerpentlist;

                    if (_player)
                    {
                        _player->GetCreatureListWithEntryInGrid(tempList, MONK_NPC_JADE_SERPENT_STATUE, 500.0f);
                        _player->GetCreatureListWithEntryInGrid(jadeSerpentlist, MONK_NPC_JADE_SERPENT_STATUE, 500.0f);
                    }

                    // Remove other players jade statue
                    for (std::list<Creature*>::iterator i = tempList.begin(); i != tempList.end(); ++i)
                    {
                        Unit* owner = (*i)->GetOwner();
                        if (owner && owner == _player && (*i)->IsSummon())
                            continue;

                        jadeSerpentlist.remove((*i));
                    }

                    // 1 statue max
                    if ((int32)jadeSerpentlist.size() >= spell->Effects[effIndex].BasePoints)
                        jadeSerpentlist.back()->ToTempSummon()->UnSummon();

                    Position pos;
                    GetExplTargetDest()->GetPosition(&pos);
                    const SummonPropertiesEntry* properties = sSummonPropertiesStore.LookupEntry(spell->Effects[effIndex].MiscValueB);
                    TempSummon* summon = _player->SummonCreature(spell->Effects[effIndex].MiscValue, pos, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, spell->GetDuration());
                    if (!summon)
                        return;

                    summon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, _player->GetGUID());
                    summon->setFaction(_player->getFaction());
                    summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellInfo()->Id);
                    summon->SetMaxHealth(_player->CountPctFromMaxHealth(50));
                    summon->SetHealth(summon->GetMaxHealth());
                    summon->setPowerType(POWER_MANA);
                    summon->SetMaxPower(POWER_MANA, _player->GetMaxPower(POWER_MANA));
                    summon->SetPower(POWER_MANA, _player->GetMaxPower(POWER_MANA));
                }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_monk_jade_serpent_statue_SpellScript::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_jade_serpent_statue_SpellScript();
        }
};

// Called by Spinning Crane Kick - 107270
// Teachings of the Monastery - 116645
class spell_monk_teachings_of_the_monastery : public SpellScriptLoader
{
    public:
        spell_monk_teachings_of_the_monastery() : SpellScriptLoader("spell_monk_teachings_of_the_monastery") { }

        class spell_monk_teachings_of_the_monastery_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_teachings_of_the_monastery_SpellScript);

            void HandleHeal(SpellEffIndex /*eff*/)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (!GetCaster()->HasAura(SPELL_MONK_TEACHINGS_OF_THE_MONASTERY))
                        return;

                    std::list<Unit*> list;
                    Trinity::AnyUnitInObjectRangeCheck u_check(player, 8.f);
                    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(player, list, u_check);
                    Trinity::VisitNearbyObject(player, 8.0f, searcher);

                    for (Unit* itr: list)
                        if (player->IsValidAssistTarget(itr))
                            GetCaster()->CastSpell(itr, SPELL_MONK_SPINNING_CRANE_KICK_HEAL, true);
                }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_monk_teachings_of_the_monastery_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_teachings_of_the_monastery_SpellScript();
        }
};

// Mana Tea - 115294
class spell_monk_mana_tea final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        void onApply(AuraEffect const *eff, AuraEffectHandleModes)
        {
            auto const caster = GetCaster();
            if (!caster)
                return;

            auto const manaTea = caster->GetAura(SPELL_MONK_MANA_TEA_STACKS);
            if (!manaTea)
                return;

            GetAura()->SetMaxDuration(eff->GetAmplitude() * manaTea->GetStackAmount());
            GetAura()->RefreshDuration(false);
        }

        void onTick(AuraEffect const *)
        {
            if (auto const caster = GetCaster())
            {
                // remove one charge per tick instead of remove aura on cast
                // "Cancelling the channel will not waste stacks"
                if (auto const manaTea = caster->GetAura(SPELL_MONK_MANA_TEA_STACKS))
                    manaTea->ModStackAmount(-1);
            }
        }

        void Register() final
        {
            OnEffectApply += AuraEffectApplyFn(script_impl::onApply, EFFECT_0, SPELL_AURA_OBS_MOD_POWER, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::onTick, EFFECT_0, SPELL_AURA_OBS_MOD_POWER);
        }
    };

public:
    spell_monk_mana_tea()
        : SpellScriptLoader("spell_monk_mana_tea")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Brewing : Mana Tea - 123766
class spell_monk_mana_tea_stacks : public SpellScriptLoader
{
    public:
        spell_monk_mana_tea_stacks() : SpellScriptLoader("spell_monk_mana_tea_stacks") { }

        class spell_monk_mana_tea_stacks_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_mana_tea_stacks_AuraScript);

            uint32 chiConsumed;

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                chiConsumed = 0;
            }

            void SetData(uint32 /*type*/, uint32 data)
            {
                Unit* caster = GetCaster();
                while ((chiConsumed += data) >= 4)
                {
                    chiConsumed = 0;
                    data = data > 4 ? data - 4: 0;

                    if (GetCaster())
                    {
                        SpellInfo const* triggerInfo = sSpellMgr->GetSpellInfo(SPELL_MONK_MANA_TEA_STACKS);
                        float critChance = caster->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_HOLY);
                        uint8 stacks = 1;
                        if (roll_chance_f(critChance))
                            stacks++;

                        caster->CastCustomSpell(SPELL_MONK_MANA_TEA_STACKS, SPELLVALUE_AURA_STACK, stacks, caster, true);
                        caster->CastSpell(caster, SPELL_MONK_PLUS_ONE_MANA_TEA, true);
                    }
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_monk_mana_tea_stacks_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_mana_tea_stacks_AuraScript();
        }
};

// Enveloping Mist - 124682
class spell_monk_enveloping_mist : public SpellScriptLoader
{
    public:
        spell_monk_enveloping_mist() : SpellScriptLoader("spell_monk_enveloping_mist") { }

        class spell_monk_enveloping_mist_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_enveloping_mist_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetExplTargetUnit())
                        _player->CastSpell(target, SPELL_MONK_ENVELOPING_MIST_HEAL, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_monk_enveloping_mist_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_enveloping_mist_SpellScript();
        }
};

// Surging Mist - 116694
class spell_monk_surging_mist : public SpellScriptLoader
{
    public:
        spell_monk_surging_mist() : SpellScriptLoader("spell_monk_surging_mist") { }

        class spell_monk_surging_mist_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_surging_mist_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetExplTargetUnit())
                        _player->CastSpell(target, SPELL_MONK_SURGING_MIST_HEAL, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_monk_surging_mist_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_surging_mist_SpellScript();
        }
};

// Surging Mist - 116694
class spell_monk_surging_mist_glyphed : public SpellScriptLoader
{
    public:
        spell_monk_surging_mist_glyphed() : SpellScriptLoader("spell_monk_surging_mist_glyphed") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetExplTargetUnit())
                        if (Player* _target = target->ToPlayer())
                        {
                            std::list<Player*> list;
                            target->GetPlayerListInGrid(list, 45.0f);
                            for (auto itr: list)
                                if (!itr->IsInSameRaidWith(_player) || itr->IsInSameGroupWith(_player))
                                    list.remove(itr);

                            if (list.empty())
                                list.push_back(_player);

                            list.sort(Trinity::HealthPctOrderPred(true));
                            Player* realTarget = list.back();
                            _player->CastSpell(realTarget, SPELL_MONK_SURGING_MIST_HEAL, true);
                            list.clear();
                        }
            }

            void Register()
            {
                AfterCast += SpellCastFn(script_impl::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

// Renewing Mist - 119611
class spell_monk_renewing_mist : public SpellScriptLoader
{
    public:
        spell_monk_renewing_mist() : SpellScriptLoader("spell_monk_renewing_mist") { }

        class spell_monk_renewing_mist_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_renewing_mist_AuraScript);

            uint32 update;
            uint8  spreadCount;

            bool Load()
            {
                spreadCount = 0;
                return true;
            }

            bool Validate(SpellInfo const* /*spell*/)
            {
                update = 0;

                if (!sSpellMgr->GetSpellInfo(119611))
                    return false;
                return true;
            }

            void OnUpdate(uint32 diff, AuraEffect * /*aurEff*/)
            {
                update += diff;

                if (update >= 2500)
                {
                    if (GetCaster())
                        if (Player* player = GetCaster()->ToPlayer())
                            player->CastSpell(player, SPELL_MONK_UPLIFT_ALLOWING_CAST, true);

                    update = 0;
                }
            }

            void OnTick(AuraEffect const * aurEff)
            {
                if (spreadCount || aurEff->GetTickNumber() > 1)
                    return;

                auto caster = GetCaster();
                auto target = GetTarget();
                if (!caster || caster->GetTypeId() != TYPEID_PLAYER || !target)
                    return;

                auto player = caster->ToPlayer();

                std::list<Unit*> playerList;
                player->GetPartyMembers(playerList);
                for (auto itr = playerList.begin(); itr != playerList.end();)
                {
                    if ((*itr)->IsWithinDist(player, caster->HasAura(SPELL_MONK_GLYPH_OF_RENEWING_MIST) ? 40.f : 20.f) && !(*itr)->HasAura(SPELL_MONK_RENEWING_MIST_HOT, caster->GetGUID()))
                        ++itr;
                    else
                        itr = playerList.erase(itr);
                }

                if (playerList.empty())
                    return;
                playerList.sort(Trinity::HealthPctOrderPred());
                playerList.resize(1);

                ++spreadCount;
                if (aurEff->GetUserData() > 1)
                    return;

                if (auto newAura = player->AddAura(SPELL_MONK_RENEWING_MIST_HOT, *playerList.begin()))
                    newAura->GetEffect(EFFECT_0)->SetUserData(aurEff->GetUserData() + 1);
            }

            void HandleRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                {
                    if (GetCaster()->GetAura(SPELL_MONK_UPLIFT_ALLOWING_CAST, GetCaster()->GetGUID()))
                        GetCaster()->RemoveAura(SPELL_MONK_UPLIFT_ALLOWING_CAST, GetCaster()->GetGUID());

                    if (GetCaster()->HasAura(SPELL_MONK_ITEM_2_S12_MISTWEAVER))
                    {
                        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                        {
                            GetCaster()->CastSpell(GetCaster(), SPELL_MONK_MANA_TEA_STACKS, true);
                            GetCaster()->CastSpell(GetCaster(), SPELL_MONK_PLUS_ONE_MANA_TEA, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_renewing_mist_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
                OnEffectUpdate += AuraEffectUpdateFn(spell_monk_renewing_mist_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
                OnEffectRemove += AuraEffectApplyFn(spell_monk_renewing_mist_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_renewing_mist_AuraScript();
        }
};

// Zen Sphere - 124081
class spell_monk_zen_sphere : public SpellScriptLoader
{
    public:
        spell_monk_zen_sphere() : SpellScriptLoader("spell_monk_zen_sphere") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void AfterRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_ENEMY_SPELL && removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (auto caster = GetCaster())
                {
                    caster->CastSpell(GetTarget(), SPELL_MONK_ZEN_SPHERE_DETONATE_HEAL, true);
                    caster->CastSpell(GetTarget(), SPELL_MONK_ZEN_SPHERE_DETONATE_DAMAGE, true);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(script_impl::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new script_impl();
        }
};

// Zen Sphere - 124081
class spell_monk_zen_sphere_hot : public SpellScriptLoader
{
    public:
        spell_monk_zen_sphere_hot() : SpellScriptLoader("spell_monk_zen_sphere_hot") { }

        class spell_monk_zen_sphere_hot_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_zen_sphere_hot_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (auto caster = GetCaster())
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                        caster->CastSpell(caster, SPELL_MONK_ZEN_SPHERE_DAMAGE, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_zen_sphere_hot_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_zen_sphere_hot_AuraScript();
        }
};

// Chi Burst - 123986
class spell_monk_chi_burst : public SpellScriptLoader
{
    public:
        spell_monk_chi_burst() : SpellScriptLoader("spell_monk_chi_burst") { }

        class spell_monk_chi_burst_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_chi_burst_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        std::list<Unit*> tempUnitMap;
                        _player->GetAttackableUnitListInRange(tempUnitMap, _player->GetDistance(target));

                        // Chi Burst will always heal the Monk, but not heal twice if Monk targets himself
                        if (target->GetGUID() != _player->GetGUID())
                            _player->CastSpell(_player, SPELL_MONK_CHI_BURST_HEAL, true);

                        if (_player->IsValidAttackTarget(target))
                            _player->CastSpell(target, SPELL_MONK_CHI_BURST_DAMAGE, true);
                        else
                            _player->CastSpell(target, SPELL_MONK_CHI_BURST_HEAL, true);

                        for (auto itr : tempUnitMap)
                        {
                            if (itr->GetGUID() == _player->GetGUID())
                                continue;

                            if (!itr->IsInBetween(_player, target, 3.0f))
                                continue;

                            uint32 spell = _player->IsValidAttackTarget(itr) ? SPELL_MONK_CHI_BURST_DAMAGE : SPELL_MONK_CHI_BURST_HEAL;
                            _player->CastSpell(itr, spell, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_chi_burst_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_chi_burst_SpellScript();
        }
};

// Energizing Brew - 115288
class spell_monk_energizing_brew : public SpellScriptLoader
{
    public:
        spell_monk_energizing_brew() : SpellScriptLoader("spell_monk_energizing_brew") { }

        class spell_monk_energizing_brew_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_energizing_brew_SpellScript);

            SpellCastResult CheckFight()
            {
                if (!GetCaster()->IsInCombat())
                    return SPELL_FAILED_CASTER_AURASTATE;
                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_energizing_brew_SpellScript::CheckFight);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_energizing_brew_SpellScript();
        }
};

// Spear Hand Strike - 116705
class spell_monk_spear_hand_strike : public SpellScriptLoader
{
    public:
        spell_monk_spear_hand_strike() : SpellScriptLoader("spell_monk_spear_hand_strike") { }

        class spell_monk_spear_hand_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_spear_hand_strike_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->isInFront(player))
                        {
                            player->CastSpell(target, SPELL_MONK_SPEAR_HAND_STRIKE_SILENCE, true);
                            player->AddSpellCooldown(116705, 0, 15 * IN_MILLISECONDS);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_spear_hand_strike_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_spear_hand_strike_SpellScript();
        }
};

// Tigereye Brew - 116740
class spell_monk_tigereye_brew : public SpellScriptLoader
{
public:
    spell_monk_tigereye_brew() : SpellScriptLoader("spell_monk_tigereye_brew") { }

    class spell_impl : public AuraScript
    {
        PrepareAuraScript(spell_impl);

        bool Load()
        {
            return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void CalculateAmount(AuraEffect const * auraEffect, int32& amount, bool& /*canBeRecalculated*/)
        {
            Unit * const caster = GetCaster();
            if (auraEffect->GetEffIndex() == EFFECT_0)
            {
                if (Aura * const brewStacks = caster->GetAura(SPELL_MONK_TIGEREYE_BREW_STACKS))
                {
                    uint8 stackAmount = brewStacks->GetStackAmount();
                    uint8 consumeAmount = std::min((uint8)10, stackAmount);
                    if (consumeAmount >= stackAmount)
                        caster->RemoveAurasDueToSpell(SPELL_MONK_TIGEREYE_BREW_STACKS);
                    else
                        brewStacks->SetStackAmount(stackAmount - consumeAmount);

                    amount *= consumeAmount;
                }
            }
            else
            {
                amount = auraEffect->GetBase()->GetEffect(EFFECT_0)->GetAmount();
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_impl::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_impl::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_HEALING_DONE_PERCENT);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_impl();
    }
};

// Tiger's Lust - 116841
class spell_monk_tigers_lust : public SpellScriptLoader
{
    public:
        spell_monk_tigers_lust() : SpellScriptLoader("spell_monk_tigers_lust") { }

        class spell_monk_tigers_lust_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_tigers_lust_SpellScript);

            bool Validate(SpellInfo const *)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MONK_FLYING_SERPENT_KICK_NEW))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                if (GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        target->RemoveMovementImpairingAuras();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_tigers_lust_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_tigers_lust_SpellScript();
        }
};

// Flying Serpent Kick - 115057
class spell_monk_flying_serpent_kick : public SpellScriptLoader
{
public:
    spell_monk_flying_serpent_kick() : SpellScriptLoader("spell_monk_flying_serpent_kick") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (auto target = GetTarget())
            {
                if (target->HasAura(SPELL_MONK_FLYING_SERPENT_KICK))
                    target->RemoveAura(SPELL_MONK_FLYING_SERPENT_KICK);

                if (target->HasAura(SPELL_MONK_ITEM_PVP_GLOVES_BONUS))
                    target->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
            }
        }

        void AfterRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->CastSpell(GetTarget(), SPELL_MONK_FLYING_SERPENT_KICK_AOE, true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(script_impl::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            AfterEffectApply += AuraEffectApplyFn(script_impl::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Chi Torpedo - 115008 or Chi Torpedo (3 charges) - 121828
class spell_monk_chi_torpedo : public SpellScriptLoader
{
    public:
        spell_monk_chi_torpedo() : SpellScriptLoader("spell_monk_chi_torpedo") { }

        class spell_monk_chi_torpedo_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_chi_torpedo_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (player->HasUnitState(UNIT_STATE_ROOT))
                        return SPELL_FAILED_ROOTED;

                return SPELL_CAST_OK;
            }

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* _player = caster->ToPlayer())
                    {
                        std::list<Unit*> tempUnitMap;
                        _player->GetAttackableUnitListInRange(tempUnitMap, 20.0f);

                        for (auto itr : tempUnitMap)
                        {
                            if (!itr->isInFront(_player, M_PI / 3) && itr->GetGUID() != _player->GetGUID())
                                continue;

                            uint32 spell = _player->IsValidAttackTarget(itr) ? SPELL_MONK_CHI_TORPEDO_DAMAGE : SPELL_MONK_CHI_TORPEDO_HEAL;
                            _player->CastSpell(itr, spell, true);
                        }

                        if (caster->HasAura(SPELL_MONK_ITEM_PVP_GLOVES_BONUS))
                            caster->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_chi_torpedo_SpellScript::CheckCast);
                AfterCast += SpellCastFn(spell_monk_chi_torpedo_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_chi_torpedo_SpellScript();
        }
};

// Purifying Brew - 119582
class spell_monk_purifying_brew : public SpellScriptLoader
{
    public:
        spell_monk_purifying_brew() : SpellScriptLoader("spell_monk_purifying_brew") { }

        class spell_monk_purifying_brew_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_purifying_brew_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* _player = caster->ToPlayer())
                    {
                        AuraApplication* staggerAmount = _player->GetAuraApplication(SPELL_MONK_LIGHT_STAGGER);

                        if (!staggerAmount)
                            staggerAmount = _player->GetAuraApplication(SPELL_MONK_MODERATE_STAGGER);
                        if (!staggerAmount)
                            staggerAmount = _player->GetAuraApplication(SPELL_MONK_HEAVY_STAGGER);

                        if (staggerAmount)
                            _player->RemoveAura(staggerAmount->GetBase()->GetId());
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_purifying_brew_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_purifying_brew_SpellScript();
        }
};

// Clash - 122057 and Clash - 126449
class spell_monk_clash : public SpellScriptLoader
{
    public:
        spell_monk_clash() : SpellScriptLoader("spell_monk_clash") { }

        class spell_monk_clash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_clash_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* _player = caster->ToPlayer())
                    {
                        if (Unit* target = GetHitUnit())
                        {
                            int32 basePoint = 2;
                            _player->CastCustomSpell(target, SPELL_MONK_CLASH_CHARGE, &basePoint, NULL, NULL, true);
                            target->CastSpell(_player, SPELL_MONK_CLASH_CHARGE, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_clash_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_clash_SpellScript();
        }
};

// Keg Smash - 121253
class spell_monk_keg_smash : public SpellScriptLoader
{
    public:
        spell_monk_keg_smash() : SpellScriptLoader("spell_monk_keg_smash") { }

        class spell_monk_keg_smash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_keg_smash_SpellScript);

            void HandleOnHit()
            {
                if (auto caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        caster->CastSpell(target, SPELL_MONK_KEG_SMASH_VISUAL, true);
                        caster->CastSpell(target, SPELL_MONK_WEAKENED_BLOWS, true);
                        caster->CastSpell(target, SPELL_MONK_DIZZYING_HAZE, true);
                    }
                }
            }

            void HandleAfterCast()
            {
                if (auto caster = GetCaster())
                    caster->CastSpell(caster, SPELL_MONK_KEG_SMASH_ENERGIZE, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_keg_smash_SpellScript::HandleOnHit);
                AfterCast += SpellCastFn(spell_monk_keg_smash_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_keg_smash_SpellScript();
        }
};

// Elusive Brew - 115308
class spell_monk_elusive_brew : public SpellScriptLoader
{
    public:
        spell_monk_elusive_brew() : SpellScriptLoader("spell_monk_elusive_brew") { }

        class spell_monk_elusive_brew_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_elusive_brew_SpellScript);

            void HandleOnHit()
            {
                int32 stackAmount = 0;

                if (Unit* caster = GetCaster())
                {
                    if (Player* _player = caster->ToPlayer())
                    {
                        if (AuraApplication* brewStacks = _player->GetAuraApplication(SPELL_MONK_ELUSIVE_BREW_STACKS))
                            stackAmount = brewStacks->GetBase()->GetStackAmount();

                        _player->AddAura(SPELL_MONK_ELUSIVE_BREW, _player);

                        if (AuraApplication* aura = _player->GetAuraApplication(SPELL_MONK_ELUSIVE_BREW))
                        {
                            Aura *elusiveBrew = aura->GetBase();
                            int32 maxDuration = elusiveBrew->GetMaxDuration();
                            int32 newDuration = stackAmount * 1000;
                            elusiveBrew->SetDuration(newDuration);

                            if (newDuration > maxDuration)
                                elusiveBrew->SetMaxDuration(newDuration);

                            _player->RemoveAura(SPELL_MONK_ELUSIVE_BREW_STACKS);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_elusive_brew_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_elusive_brew_SpellScript();
        }
};

// Breath of Fire - 115181
class spell_monk_breath_of_fire : public SpellScriptLoader
{
    public:
        spell_monk_breath_of_fire() : SpellScriptLoader("spell_monk_breath_of_fire") { }

        class spell_monk_breath_of_fire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_breath_of_fire_SpellScript);

            void HandleAfterHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* _player = caster->ToPlayer())
                    {
                        if (Unit* target = GetHitUnit())
                        {
                            // if Dizzying Haze is on the target, they will burn for an additionnal damage over 8s
                            if (target->HasAura(SPELL_MONK_DIZZYING_HAZE))
                            {
                                _player->CastSpell(target, SPELL_MONK_BREATH_OF_FIRE_DOT, true);

                                // Glyph of Breath of Fire
                                if (_player->HasAura(123394))
                                    _player->CastSpell(target, SPELL_MONK_BREATH_OF_FIRE_CONFUSED, true);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_monk_breath_of_fire_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_breath_of_fire_SpellScript();
        }
};

// Soothing Mist - 115175
class spell_monk_soothing_mist : public SpellScriptLoader
{
    public:
        spell_monk_soothing_mist() : SpellScriptLoader("spell_monk_soothing_mist") { }

        class spell_monk_soothing_mist_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_soothing_mist_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                auto caster = GetCaster();
                auto target = GetTarget();
                if (!caster || !target)
                    return;

                target->CastSpell(target, SPELL_MONK_SOOTHING_MIST_VISUAL, true);

                if (auto _player = caster->ToPlayer())
                {
                    std::list<Unit*> playerList;
                    std::list<Creature*> statueList;

                    _player->GetPartyMembers(playerList);
                    if (playerList.empty())
                        return;

                    if (playerList.size() > 1)
                    {
                        playerList.remove(target);
                        playerList.sort(Trinity::HealthPctOrderPred());
                        playerList.resize(1);
                    }

                    _player->GetCreatureListWithEntryInGrid(statueList, 60849, 100.0f);

                    // Remove other players jade statue
                    for (auto i = statueList.begin(); i != statueList.end();)
                    {
                        Unit* owner = (*i)->GetOwner();
                        if (owner && owner == _player && (*i)->IsSummon())
                            ++i;
                        else
                            i = statueList.erase(i);
                    }

                    auto playerTarget = *playerList.begin();
                    if (!statueList.empty())
                        if (auto statue = statueList.front())
                            if (statue->GetOwnerGUID() == _player->GetGUID())
                                statue->CastSpell(playerTarget, GetSpellInfo()->Id, true);
                }
            }

            void HandleEffectPeriodic(AuraEffect const * /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                    if (GetTarget() && roll_chance_i(30))
                        caster->CastSpell(caster, SPELL_MONK_SOOTHING_MIST_ENERGIZE, true);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    if (Unit* target = GetTarget())
                        target->RemoveAura(SPELL_MONK_SOOTHING_MIST_VISUAL);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_monk_soothing_mist_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_soothing_mist_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_monk_soothing_mist_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_soothing_mist_AuraScript();
        }
};

// Disable - 116095
class spell_monk_disable : public SpellScriptLoader
{
    public:
        spell_monk_disable() : SpellScriptLoader("spell_monk_disable") { }

        class aura_impl : public AuraScript
        {
            PrepareAuraScript(aura_impl);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                    if (caster && caster->getClass() == CLASS_MONK && caster->IsAlive())
                            if (aurEff->GetTickNumber() % aurEff->GetTotalTicks() == 0) // last tick of duration
                                if (GetOwner()->IsInRange(caster, 0, 10))
                                    if (Aura* aura = aurEff->GetBase())
                                        aura->RefreshDuration();
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(aura_impl::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new aura_impl();
        }

        class spell_impl : public SpellScript
        {
            PrepareSpellScript(spell_impl);

            bool snaredOnHit;

            SpellCastResult CheckCast()
            {
                snaredOnHit = false;

                if (GetCaster())
                    if (Unit* target = GetCaster()->GetVictim())
                        if (target->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED))
                            snaredOnHit = true;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                    if (Player* _player = caster->ToPlayer())
                        if (Unit* target = GetHitUnit())
                            if (snaredOnHit)
                                _player->CastSpell(target, SPELL_MONK_DISABLE_ROOT, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_impl::CheckCast);
                OnHit += SpellHitFn(spell_impl::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_impl();
        }
};

// Zen Pilgrimage - 126892 and Zen Pilgrimage : Return - 126895
class spell_monk_zen_pilgrimage : public SpellScriptLoader
{
    public:
        spell_monk_zen_pilgrimage() : SpellScriptLoader("spell_monk_zen_pilgrimage") { }

        class spell_monk_zen_pilgrimage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_zen_pilgrimage_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MONK_ZEN_PILGRIMAGE) || !sSpellMgr->GetSpellInfo(SPELL_MONK_ZEN_PILGRIMAGE_RETURN))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* _player = caster->ToPlayer())
                    {
                        if (GetSpellInfo()->Id == SPELL_MONK_ZEN_PILGRIMAGE)
                        {
                            _player->SaveRecallPosition();
                            _player->TeleportTo(870, 3818.55f, 1793.18f, 950.35f, _player->GetOrientation());
                        }
                        else if (GetSpellInfo()->Id == SPELL_MONK_ZEN_PILGRIMAGE_RETURN)
                        {
                            _player->TeleportTo(_player->m_recallMap, _player->m_recallX, _player->m_recallY, _player->m_recallZ, _player->m_recallO);
                            _player->RemoveAura(126896);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_zen_pilgrimage_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_zen_pilgrimage_SpellScript();
        }
};

// Blackout Kick - 100784
class spell_monk_blackout_kick : public SpellScriptLoader
{
    public:
        spell_monk_blackout_kick() : SpellScriptLoader("spell_monk_blackout_kick") { }

        class spell_monk_blackout_kick_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_blackout_kick_SpellScript);

            void afterHit()
            {
                auto caster = GetCaster();
                auto target = GetHitUnit();
                if (!caster || !target || caster == target)
                    return;

                uint32 damage = GetHitDamage();
                if (!damage)
                    return;

                if (auto player = caster->ToPlayer())
                {
                    // Second effect by spec : Instant heal or DoT
                    if (player->GetSpecializationId(player->GetActiveSpec()) == SPEC_MONK_WINDWALKER && player->HasAura(128595))
                    {
                        // Your Blackout Kick always deals 20% additional damage over 4 sec regardless of positioning but you're unable to trigger the healing effect.
                        if (caster->HasAura(SPELL_MONK_GLYPH_OF_BLACKOUT_KICK))
                        {
                            int32 bp = int32(damage * 0.2f) / 4;
                            if (auto eff = target->HasAura(SPELL_MONK_BLACKOUT_KICK_DOT, caster->GetGUID()))
                                bp += target->GetRemainingPeriodicAmount(caster->GetGUID(), SPELL_MONK_BLACKOUT_KICK_DOT, SPELL_AURA_PERIODIC_DAMAGE).perTick();
                            caster->CastCustomSpell(target, SPELL_MONK_BLACKOUT_KICK_DOT, &bp, NULL, NULL, true);
                        }
                        else
                        {
                            // If behind : 20% damage on DoT
                            if (target->isInBack(caster))
                            {
                                int32 bp = int32(damage * 0.2f) / 4;
                                if (auto eff = target->HasAura(SPELL_MONK_BLACKOUT_KICK_DOT, caster->GetGUID()))
                                    bp += target->GetRemainingPeriodicAmount(caster->GetGUID(), SPELL_MONK_BLACKOUT_KICK_DOT, SPELL_AURA_PERIODIC_DAMAGE).perTick();
                                caster->CastCustomSpell(target, SPELL_MONK_BLACKOUT_KICK_DOT, &bp, NULL, NULL, true);
                            }
                            // else : 20% damage on instant heal
                            else
                            {
                                int32 bp = int32(damage * 0.2f);
                                caster->CastCustomSpell(caster, SPELL_MONK_BLACKOUT_KICK_HEAL, &bp, NULL, NULL, true);
                            }
                        }
                    }
                    // Brewmaster : Training - you gain Shuffle, increasing parry chance and stagger amount by 20%
                    else if (player->GetSpecializationId(player->GetActiveSpec()) == SPEC_MONK_BREWMASTER)
                    {
                        if (Aura * const shuffle = caster->GetAura(SPELL_MONK_SHUFFLE))
                            shuffle->SetDuration(shuffle->GetDuration() + shuffle->GetMaxDuration());
                        else
                            caster->CastSpell(caster, SPELL_MONK_SHUFFLE, true);
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_monk_blackout_kick_SpellScript::afterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_blackout_kick_SpellScript();
        }
};

// Provoke - 115546
class spell_monk_provoke : public SpellScriptLoader
{
    public:
        spell_monk_provoke() : SpellScriptLoader("spell_monk_provoke") { }

        class spell_monk_provoke_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_provoke_SpellScript);

            enum
            {
                BLACK_OX_STATUE_ENTRY = 61146
            };

            SpellCastResult CheckCast()
            {
                Unit* target = GetExplTargetUnit();
                if (!target)
                    return SPELL_FAILED_NO_VALID_TARGETS;
                else if (target->GetTypeId() == TYPEID_PLAYER)
                    return SPELL_FAILED_TARGET_IS_PLAYER;
                else if (!target->IsWithinLOSInMap(GetCaster()))
                    return SPELL_FAILED_LINE_OF_SIGHT;
                else if (GetCaster()->IsFriendlyTo(target))
                {
                    if (target->GetTypeId() == TYPEID_UNIT && target->GetEntry() != BLACK_OX_STATUE_ENTRY)
                        return SPELL_FAILED_TARGET_FRIENDLY;
                }
                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->getClass() == CLASS_MONK && caster->GetTypeId() == TYPEID_PLAYER)
                        if (Unit* target = GetHitUnit())
                            caster->CastSpell(target, SPELL_MONK_PROVOKE, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_provoke_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_monk_provoke_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_provoke_SpellScript();
        }
};

// Paralysis - 115078
class spell_monk_paralysis : public SpellScriptLoader
{
    public:
        spell_monk_paralysis() : SpellScriptLoader("spell_monk_paralysis") { }

        class spell_monk_paralysis_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_paralysis_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->isInBack(caster))
                        {
                            if (AuraApplication* aura = target->GetAuraApplication(115078))
                            {
                                Aura *Paralysis = aura->GetBase();
                                int32 maxDuration = Paralysis->GetMaxDuration();
                                int32 newDuration = maxDuration * 2;
                                Paralysis->SetDuration(newDuration);

                                if (newDuration > maxDuration)
                                    Paralysis->SetMaxDuration(newDuration);
                            }
                        }

                        if (target->ToPlayer())
                        {
                            if (AuraApplication* aura = target->GetAuraApplication(115078))
                            {
                                Aura *Paralysis = aura->GetBase();
                                int32 maxDuration = Paralysis->GetMaxDuration();
                                int32 newDuration = maxDuration / 2;
                                Paralysis->SetDuration(newDuration);
                                Paralysis->SetMaxDuration(newDuration);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_monk_paralysis_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_paralysis_SpellScript();
        }
};

// Touch of Death - 115080
class spell_monk_touch_of_death : public SpellScriptLoader
{
    public:
        spell_monk_touch_of_death() : SpellScriptLoader("spell_monk_touch_of_death") { }

        class spell_monk_touch_of_death_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_touch_of_death_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit * const caster = GetCaster();
                Unit * const target = GetExplTargetUnit();
                if (caster && target)
                {
                    if (target->GetTypeId() == TYPEID_UNIT)
                    {
                        // Is boss, or target has more health
                        if (target->ToCreature()->IsDungeonBoss()
                            || target->GetHealth() > caster->GetHealth())
                        {
                            return SPELL_FAILED_BAD_TARGETS;
                        }
                    }

                    if (caster->HasAura(124490)) // Arena S14 4P bonus
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER && (target->GetHealthPct() > 10.0f))
                            return SPELL_FAILED_BAD_TARGETS;
                    }
                    else
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER || target->isPet())
                            return SPELL_FAILED_BAD_TARGETS;
                    }
                    return SPELL_CAST_OK;
                }
                return SPELL_FAILED_NO_VALID_TARGETS;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_touch_of_death_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_touch_of_death_SpellScript();
        }
};

// Fortifying brew - 115203
class spell_monk_fortifying_brew : public SpellScriptLoader
{
    public:
        spell_monk_fortifying_brew() : SpellScriptLoader("spell_monk_fortifying_brew") { }

        class spell_monk_fortifying_brew_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_fortifying_brew_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (caster && caster->GetTypeId() == TYPEID_PLAYER)
                    caster->CastSpell(caster, SPELL_MONK_FORTIFYING_BREW, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_fortifying_brew_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_fortifying_brew_SpellScript();
        }
};

// Legacy of the Emperor - 115921
class spell_monk_legacy_of_the_emperor : public SpellScriptLoader
{
    public:
        spell_monk_legacy_of_the_emperor() : SpellScriptLoader("spell_monk_legacy_of_the_emperor") { }

        class spell_monk_legacy_of_the_emperor_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_legacy_of_the_emperor_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* plr = GetCaster()->ToPlayer())
                {
                    std::list<Unit*> groupList;

                    plr->GetPartyMembers(groupList);
                    if (!groupList.empty())
                        for (auto itr : groupList)
                            plr->CastSpell(itr, SPELL_MONK_LEGACY_OF_THE_EMPEROR, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_legacy_of_the_emperor_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_legacy_of_the_emperor_SpellScript();
        }
};

// Roll - 109132 or Roll (3 charges) - 121827
class spell_monk_roll : public SpellScriptLoader
{
    public:
        spell_monk_roll() : SpellScriptLoader("spell_monk_roll") { }

        class spell_monk_roll_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_roll_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MONK_ROLL))
                    return false;
                return true;
            }

            SpellCastResult CheckCast()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (player->HasUnitState(UNIT_STATE_ROOT))
                        return SPELL_FAILED_ROOTED;

                return SPELL_CAST_OK;
            }

            void HandleBeforeCast()
            {
                Aura *aur = GetCaster()->AddAura(SPELL_MONK_ROLL_TRIGGER, GetCaster());
                if (!aur)
                    return;

                AuraApplication* app =  aur->GetApplicationOfTarget(GetCaster()->GetGUID());
                if (!app)
                    return;

                app->ClientUpdate();
            }

            void HandleAfterCast()
            {
                Unit* caster = GetCaster();
                if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                caster->CastSpell(caster, SPELL_MONK_ROLL_TRIGGER, true);

                if (caster->HasAura(SPELL_MONK_ITEM_PVP_GLOVES_BONUS))
                    caster->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_roll_SpellScript::CheckCast);
                BeforeCast += SpellCastFn(spell_monk_roll_SpellScript::HandleBeforeCast);
                AfterCast += SpellCastFn(spell_monk_roll_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_monk_roll_SpellScript();
        }
};

// Brewing : Tigereye Brew - 123980
class spell_monk_tigereye_brew_stacks : public SpellScriptLoader
{
    public:
        spell_monk_tigereye_brew_stacks() : SpellScriptLoader("spell_monk_tigereye_brew_stacks") { }

        class spell_monk_tigereye_brew_stacks_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_tigereye_brew_stacks_AuraScript);

            uint32 chiConsumed;

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                chiConsumed = 0;
            }

            void SetData(uint32 /*type*/, uint32 data)
            {
                Unit * const caster = GetCaster();
                if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                while ((chiConsumed += data) >= 4)
                {
                    chiConsumed = 0;
                    data = data > 4 ? data - 4: 0;

                    caster->CastSpell(caster, SPELL_MONK_TIGEREYE_BREW_STACKS, true);
                    // Mastery: Bottled Fury
                    if (AuraEffect * const mastery = caster->GetAuraEffect(115636, EFFECT_0))
                        if (roll_chance_f(mastery->GetFloatAmount()))
                            caster->CastSpell(GetCaster(), SPELL_MONK_TIGEREYE_BREW_STACKS, true);
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_monk_tigereye_brew_stacks_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_monk_tigereye_brew_stacks_AuraScript();
        }
};

// 137384 - Mastery : Combo Breaker
class spell_monk_combo_breaker : public SpellScriptLoader
{
    enum
    {
        SPELL_COMBO_BREAKER_PALM = 118864,
        SPELL_COMBO_BREAKER_KICK = 116768,
    };

public:
    spell_monk_combo_breaker() : SpellScriptLoader("spell_monk_combo_breaker") { }

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void OnProc(AuraEffect const * eff, ProcEventInfo &eventInfo)
        {
            PreventDefaultAction();
            auto caster = GetCaster();
            if (!caster || !(eventInfo.GetDamageInfo()->GetDamage()))
                return;

            if (roll_chance_i(eff->GetAmount()))
                caster->CastSpell(caster, SPELL_COMBO_BREAKER_KICK, true);

            if (roll_chance_i(eff->GetAmount()))
                caster->CastSpell(caster, SPELL_COMBO_BREAKER_PALM, true);
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

class spell_monk_sanctuary_of_the_ox final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            BLACK_OX_GUARD_AOE_SELECTOR = 118605
        };

        uint32 totalDamageDone_;

    public:
        script_impl()
            : totalDamageDone_()
        { }

    private:
        void onProc(AuraEffect const *, ProcEventInfo &eventInfo)
        {
            PreventDefaultAction();

            totalDamageDone_ += eventInfo.GetDamageInfo()->GetDamage();

            auto const monk = eventInfo.GetActor();

            uint32 const threshold = monk->GetTotalAttackPowerValue(BASE_ATTACK) * 16;
            if (totalDamageDone_ < threshold)
                return;

            totalDamageDone_ -= threshold;

            auto const statue = Unit::GetUnit(*monk, monk->m_SummonSlot[SUMMON_SLOT_TOTEM]);
            if (!statue)
                return;

            statue->CastSpell(statue, BLACK_OX_GUARD_AOE_SELECTOR, true, nullptr, nullptr, monk->GetGUID());
        }

        void Register() final
        {
            OnEffectProc += AuraEffectProcFn(script_impl::onProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_monk_sanctuary_of_the_ox()
        : SpellScriptLoader("spell_monk_sanctuary_of_the_ox")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

class spell_monk_black_ox_guard_aoe_selector final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            BLACK_OX_GUARD = 118604
        };

        void filterTargets(std::list<WorldObject*> &targets)
        {
            auto const monk = GetOriginalCaster();
            Unit *mostInjured = nullptr;

            for (auto &obj : targets)
            {
                if (obj->GetTypeId() != TYPEID_PLAYER || obj == monk)
                    continue;

                auto const unitTarget = obj->ToUnit();
                if (!monk->IsInRaidWith(unitTarget))
                    continue;

                if (!mostInjured || mostInjured->GetHealthPct() > unitTarget->GetHealthPct())
                    mostInjured = unitTarget;
            };

            targets.clear();

            if (mostInjured)
                targets.emplace_back(mostInjured);
        }

        void scriptEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            auto const monk = GetOriginalCaster();
            auto const target = GetHitUnit();

            monk->CastSpell(target, BLACK_OX_GUARD, true);
        }

        void Register() final
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::filterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            OnEffectHitTarget += SpellEffectFn(script_impl::scriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

public:
    spell_monk_black_ox_guard_aoe_selector()
        : SpellScriptLoader("spell_monk_black_ox_guard_aoe_selector")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

// 145640 - Chi Brew
class spell_monk_chi_brew : public SpellScriptLoader
{
public:
    spell_monk_chi_brew() : SpellScriptLoader("spell_monk_chi_brew") { }

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Player * const player = GetCaster()->ToPlayer();
            if (!player)
                return;

            uint32 brewId = 0;
            switch (player->GetSpecializationId(player->GetActiveSpec()))
            {
                case SPEC_MONK_BREWMASTER:
                    brewId = SPELL_MONK_ELUSIVE_BREW_STACKS;
                    break;
                case SPEC_MONK_MISTWEAVER:
                    brewId = SPELL_MONK_MANA_TEA_STACKS;
                    break;
                case SPEC_MONK_WINDWALKER:
                    brewId = SPELL_MONK_TIGEREYE_BREW_STACKS;
                    break;
                default:
                    break;
            }

            if (brewId)
                for (uint32 i = 0; i < 2; ++i)
                    player->CastSpell(player, brewId, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

// Muscle Memory (139598) - Called by Jab (all versions)
class spell_monk_muscle_memory : public SpellScriptLoader
{
public:
    spell_monk_muscle_memory() : SpellScriptLoader("spell_monk_muscle_memory") { }

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            Player * const player = GetCaster()->ToPlayer();
            if (!player)
                return;

            if (player->HasSpell(139598))
                player->CastSpell(player, 139597, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

// 116844 - Ring of Peace
class sat_monk_ring_of_peace : public SpellAreaTriggerScript
{
public:
    sat_monk_ring_of_peace() : SpellAreaTriggerScript("sat_monk_ring_of_peace") {}

    class sat_monk_ring_of_peace_interface : public IAreaTriggerAura
    {
        enum
        {
            SPELL_RING_OF_PEACE_AURA_DUMMY = 140023
        };

        bool CheckTriggering(WorldObject* triggering)
        {
            Unit* unit = triggering->ToUnit();
            if (!unit)
                return false;

            if (!m_target->IsWithinDistInMap(unit, m_range))
                return false;

            if (!m_caster->_IsValidAttackTarget(unit, m_spellInfo, m_target))
                return false;

            return true;
        }

        void OnTriggeringUpdate(WorldObject* triggering)
        {
            triggering->ToUnit()->AddAura(SPELL_RING_OF_PEACE_AURA_DUMMY, triggering->ToUnit());
        }

        void OnTriggeringApply(WorldObject* triggering)
        {
            triggering->ToUnit()->AddAura(SPELL_RING_OF_PEACE_AURA_DUMMY, triggering->ToUnit());
        }

        void OnTriggeringRemove(WorldObject* triggering)
        {
            triggering->ToUnit()->RemoveAurasDueToSpell(SPELL_RING_OF_PEACE_AURA_DUMMY);
        }
    };

    IAreaTrigger* GetInterface() const override
    {
        return new sat_monk_ring_of_peace_interface();
    }
};

// 140023 - Ring of Peace Dummy
class spell_monk_ring_of_peace_dummy_aura : public SpellScriptLoader
{
public:
    spell_monk_ring_of_peace_dummy_aura() : SpellScriptLoader("spell_monk_ring_of_peace_dummy_aura") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);
        enum
        {
            SPELL_MONK_RING_OF_PEACE_SILENCE = 137460,
            SPELL_MONK_RING_OF_PEACE_DISARM = 137461
        };

        void OnProc(AuraEffect const *aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            GetCaster()->AddAura(SPELL_MONK_RING_OF_PEACE_DISARM, GetCaster());
            GetCaster()->AddAura(SPELL_MONK_RING_OF_PEACE_SILENCE, GetCaster());
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

const uint32 spiritEntry[3] = { 69680, 69792, 69791 }; // Storm, Earth and Fire
const uint32 summonsMonk[3] = { 138122, 138121, 138123 }; // Storm, Earth and Fire
const uint32 visualMorph[3] = { 138080, 138083, 138081 }; // Storm, Earth and Fire

// Storm, Earth and Fire - 138130
class spell_monk_storm_earth_and_fire_stats : public SpellScriptLoader
{
public:
	spell_monk_storm_earth_and_fire_stats() : SpellScriptLoader("spell_monk_storm_earth_and_fire_stats") { }

	class spell_monk_storm_earth_and_fire_stats_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_monk_storm_earth_and_fire_stats_AuraScript);

		void OnUpdate(uint32 /*diff*/, AuraEffect* aurEff)
		{
			if (!GetCaster())
				return;

			if (Unit* caster = GetCaster()->GetOwner())
			{
				if (AuraEffect* stormAura = caster->GetAuraEffect(SPELL_MONK_STORM_EARTH_AND_FIRE, EFFECT_1))
				{
					if (aurEff->GetAmount() != stormAura->GetAmount())
						aurEff->ChangeAmount(stormAura->GetAmount());
				}
			}
		}

		void CalculateReducedDamage(const AuraEffect* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
		{
			if (!GetCaster() || !GetCaster()->GetOwner())
				return;

			if (Unit* owner = GetCaster()->GetOwner())
				if (AuraEffect* stormAura = owner->GetAuraEffect(SPELL_MONK_STORM_EARTH_AND_FIRE, EFFECT_1))
					amount = stormAura->GetAmount();
		}

		void CalculateHealing(const AuraEffect* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
		{
			if (!GetCaster() || !GetCaster()->GetOwner())
				return;

			if (Unit* owner = GetCaster()->GetOwner())
				amount = owner->GetTotalAuraModifier(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
		}

		void CalculateAttackPower(const AuraEffect* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
		{
			if (!GetCaster() || !GetCaster()->GetOwner())
				return;

			if (Unit* owner = GetCaster()->GetOwner())
				amount = owner->GetTotalAttackPowerValue(BASE_ATTACK);
		}

		void CalculateHaste(const AuraEffect* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
		{
			if (!GetCaster() || !GetCaster()->GetOwner())
				return;

			if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
			{
				if (Unit* caster = GetCaster())
				{
					// Convert Owner's haste into the Spirit haste
					float ownerHaste = 1.0f + owner->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HASTE_MELEE) *
						owner->GetRatingMultiplier(CR_HASTE_MELEE) / 100.0f;
					caster->ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED, ownerHaste, false);
					caster->ApplyPercentModFloatValue(UNIT_MOD_CAST_HASTE, ownerHaste, false);
					caster->ApplyPercentModFloatValue(UNIT_MOD_HASTE, ownerHaste, false);
				}
			}
		}

		void Register()
		{
			OnEffectUpdate += AuraEffectUpdateFn(spell_monk_storm_earth_and_fire_stats_AuraScript::OnUpdate, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_storm_earth_and_fire_stats_AuraScript::CalculateReducedDamage, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_storm_earth_and_fire_stats_AuraScript::CalculateHealing, EFFECT_3, SPELL_AURA_MOD_HEALING_DONE_PERCENT);
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_storm_earth_and_fire_stats_AuraScript::CalculateAttackPower, EFFECT_4, SPELL_AURA_MOD_ATTACK_POWER);
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_storm_earth_and_fire_stats_AuraScript::CalculateHaste, EFFECT_5, SPELL_AURA_MOD_MELEE_HASTE_3);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_monk_storm_earth_and_fire_stats_AuraScript();
	}
};

// Storm, Earth and Fire - 137639
class spell_monk_storm_earth_and_fire : public SpellScriptLoader
{
public:
	spell_monk_storm_earth_and_fire() : SpellScriptLoader("spell_monk_storm_earth_and_fire") { }

	class spell_monk_storm_earth_and_fire_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_monk_storm_earth_and_fire_SpellScript);

		uint8 summonsCount;
		uint8 firstSpirit;

		bool Load()
		{
			summonsCount = 0;
			firstSpirit = 3;
			return true;
		}

		void HandleDummy(SpellEffIndex effIndex)
		{
			if (Unit* caster = GetCaster())
			{
				if (Unit* target = GetHitUnit())
				{
					for (uint8 i = 0; i < 3; ++i)
					{
						for (Unit::ControlList::const_iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
						{
							if ((*itr)->GetEntry() == spiritEntry[i])
							{
								++summonsCount;

								if (firstSpirit == 3)
									firstSpirit = i;
							}
						}
					}

					if (!summonsCount)
						caster->CastSpell(caster, visualMorph[urand(0, 2)], true);

					++summonsCount;

					// Find summonned spirit
					for (Unit::ControlList::const_iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
					{
						if (Creature* spirit = (*itr)->ToCreature())
						{
							if (spirit->AI() && spirit->AI()->GetGUID() == target->GetGUID())
							{
								spirit->AI()->DoAction(0);
								return;
							}
						}
					}

					Unit* spiritRemoved = NULL;
					if (summonsCount > 2)
					{
						for (Unit::ControlList::const_iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
						{
							if ((*itr)->GetEntry() == spiritEntry[firstSpirit])
							{
								if (Creature* spirit = (*itr)->ToCreature())
								{
									spirit->GetMotionMaster()->Clear();
									spirit->GetMotionMaster()->MoveJump(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), 15.0f, 10.0f, spirit->GetOrientation(), 1);
									spiritRemoved = spirit->ToUnit();
									spirit->DespawnOrUnsummon();
									break;
								}
							}
						}
					}

					Unit* pPet = NULL;
					if (spiritRemoved)
					{
						for (uint8 i = 0; i < 3; ++i)
						{
							if (spiritEntry[i] == spiritRemoved->GetEntry())
							{
								caster->CastSpell(caster, summonsMonk[i], true);

								// Find summonned spirit
								for (Unit::ControlList::const_iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
								{
									if ((*itr)->GetEntry() == spiritEntry[i] && (*itr) != spiritRemoved)
									{
										pPet = *itr;
										break;
									}
								}

								if (pPet && pPet->GetAI())
									pPet->GetAI()->SetGUID(target->GetGUID());

								return;
							}
						}
					}

					for (uint8 i = 0; i < 3; ++i)
					{
						if (caster->HasAura(visualMorph[i]))
							continue;

						if (firstSpirit < 3 && firstSpirit == i)
							continue;

						caster->CastSpell(caster, summonsMonk[i], true);

						// Find summonned spirit
						for (Unit::ControlList::const_iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
						{
							if ((*itr)->GetEntry() == spiritEntry[i] && (*itr) != spiritRemoved)
							{
								pPet = *itr;
								break;
							}
						}

						if (pPet && pPet->GetAI())
							pPet->GetAI()->SetGUID(target->GetGUID());

						if (firstSpirit == 3)
							firstSpirit = i;

						break;
					}
				}
			}
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_monk_storm_earth_and_fire_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_monk_storm_earth_and_fire_SpellScript();
	}

	class spell_monk_storm_earth_and_fire_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_monk_storm_earth_and_fire_AuraScript);

		void OnUpdate(uint32 /*diff*/, AuraEffect* /*aurEff*/)
		{
			if (Unit* caster = GetCaster())
			{
				uint8 count = 0;
				for (Unit::ControlList::const_iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
					if ((*itr)->GetEntry() == 69680 || (*itr)->GetEntry() == 69792 || (*itr)->GetEntry() == 69791)
						++count;

				if (!count)
				{
					caster->RemoveAura(GetSpellInfo()->Id);
					return;
				}

				if (Aura* stormAura = caster->GetAura(GetSpellInfo()->Id))
				{
					if (count != stormAura->GetStackAmount())
						stormAura->SetStackAmount(count);

					if (stormAura->GetStackAmount() > 1)
					{
						stormAura->GetEffect(1)->ChangeAmount(-70);
						stormAura->GetEffect(2)->ChangeAmount(-70);
						stormAura->GetEffect(3)->ChangeAmount(-70);
						stormAura->GetEffect(4)->ChangeAmount(-70);
					}
					else
					{
						stormAura->GetEffect(1)->ChangeAmount(-30);
						stormAura->GetEffect(2)->ChangeAmount(-30);
						stormAura->GetEffect(3)->ChangeAmount(-30);
						stormAura->GetEffect(4)->ChangeAmount(-30);
					}
				}
			}
		}

		void OnRemove(const AuraEffect* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			if (Unit* caster = GetCaster())
			{
				for (uint8 i = 0; i < 3; ++i)
					caster->RemoveAura(visualMorph[i]);

				for (Unit::ControlList::const_iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
				{
					if ((*itr)->GetEntry() == 69680 || (*itr)->GetEntry() == 69792 || (*itr)->GetEntry() == 69791)
					{
						if (Creature* spirit = (*itr)->ToCreature())
						{
							spirit->GetMotionMaster()->Clear();
							spirit->GetMotionMaster()->MoveJump(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), 15.0f, 10.0f, spirit->GetOrientation(), 1);
							spirit->DespawnOrUnsummon();
						}
					}
				}
			}
		}

		void Register()
		{
			OnEffectUpdate += AuraEffectUpdateFn(spell_monk_storm_earth_and_fire_AuraScript::OnUpdate, EFFECT_2, SPELL_AURA_DUMMY);
			AfterEffectRemove += AuraEffectRemoveFn(spell_monk_storm_earth_and_fire_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_monk_storm_earth_and_fire_AuraScript();
	}
};

void AddSC_monk_spell_scripts()
{
    new spell_monk_fists_of_fury_stun();
    new spell_monk_expel_harm();
    new spell_monk_chi_wave_healing_bolt();
    new spell_monk_chi_wave_bolt();
    new spell_monk_chi_wave();
    new spell_monk_grapple_weapon();
    new spell_monk_transcendence_transfer();
    new spell_monk_transcendence();
    new spell_monk_serpents_zeal();
    new spell_monk_dampen_harm();
    new spell_monk_item_s12_4p_mistweaver();
    new spell_monk_diffuse_magic();
    new spell_monk_guard();
    new spell_monk_bear_hug();
    new spell_monk_zen_flight_check();
    new spell_monk_glyph_of_zen_flight();
    new spell_monk_power_strikes();
    new spell_monk_crackling_jade_lightning();
    new spell_monk_touch_of_karma();
    new spell_monk_spinning_fire_blossom_damage();
    new spell_monk_spinning_fire_blossom();
    new spell_monk_path_of_blossom();
    new spell_monk_thunder_focus_tea();
    new spell_monk_jade_serpent_statue();
    new spell_monk_teachings_of_the_monastery();
    new spell_monk_mana_tea();
    new spell_monk_mana_tea_stacks();
    new spell_monk_enveloping_mist();
    new spell_monk_surging_mist();
    new spell_monk_surging_mist_glyphed();
    new spell_monk_renewing_mist();
    new spell_monk_zen_sphere();
    new spell_monk_zen_sphere_hot();
    new spell_monk_chi_burst();
    new spell_monk_energizing_brew();
    new spell_monk_spear_hand_strike();
    new spell_monk_tigereye_brew();
    new spell_monk_tigers_lust();
    new spell_monk_flying_serpent_kick();
    new spell_monk_chi_torpedo();
    new spell_monk_purifying_brew();
    new spell_monk_clash();
    new spell_monk_keg_smash();
    new spell_monk_elusive_brew();
    new spell_monk_breath_of_fire();
    new spell_monk_soothing_mist();
    new spell_monk_disable();
    new spell_monk_zen_pilgrimage();
    new spell_monk_blackout_kick();
    new spell_monk_legacy_of_the_emperor();
    new spell_monk_fortifying_brew();
    new spell_monk_touch_of_death();
    new spell_monk_paralysis();
    new spell_monk_provoke();
    new spell_monk_roll();
    new spell_monk_tigereye_brew_stacks();
    new spell_monk_combo_breaker();
    new spell_monk_sanctuary_of_the_ox();
    new spell_monk_black_ox_guard_aoe_selector();
    new spell_monk_chi_brew();
    new spell_monk_muscle_memory();
    new sat_monk_ring_of_peace();
    new spell_monk_ring_of_peace_dummy_aura();
    new spell_monk_storm_earth_and_fire_stats();
    new spell_monk_storm_earth_and_fire();
}
