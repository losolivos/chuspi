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
 * Scripts for spells with SPELLFAMILY_WARLOCK and SPELLFAMILY_GENERIC spells used by warlock players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_warl_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum WarlockSpells
{
    WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS    = 54435,
    WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER  = 54443,
    WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD    = 54508,
    WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER   = 54509,
    WARLOCK_DEMONIC_EMPOWERMENT_IMP         = 54444,
    WARLOCK_DEMONIC_CIRCLE_SUMMON           = 48018,
    WARLOCK_DEMONIC_CIRCLE_TELEPORT         = 48020,
    WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST       = 62388,
    WARLOCK_UNSTABLE_AFFLICTION_DISPEL      = 31117,
    WARLOCK_GLYPH_OF_FEAR                   = 56244,
    WARLOCK_FEAR_EFFECT                     = 118699,
    WARLOCK_GLYPH_OF_FEAR_EFFECT            = 130616,
    WARLOCK_CREATE_HEALTHSTONE              = 23517,
    WARLOCK_HARVEST_LIFE_HEAL               = 125314,
    WARLOCK_DRAIN_LIFE_HEAL                 = 89653,
    WARLOCK_SOULBURN_AURA                   = 74434,
    WARLOCK_CORRUPTION_SPELL                = 172,
    WARLOCK_CORRUPTION_DAMAGE_AURA          = 146739,
    WARLOCK_AGONY                           = 980,
    WARLOCK_DOOM                            = 603,
    WARLOCK_UNSTABLE_AFFLICTION             = 30108,
    WARLOCK_IMMOLATE                        = 348,
    WARLOCK_SHADOWBURN_ENERGIZE             = 125882,
    WARLOCK_CONFLAGRATE                     = 17962,
    WARLOCK_CONFLAGRATE_FIRE_AND_BRIMSTONE  = 108685,
    WARLOCK_IMMOLATE_FIRE_AND_BRIMSTONE     = 108686,
    WARLOCK_FIRE_AND_BRIMSTONE              = 108683,
    WARLOCK_RAIN_OF_FIRE                    = 104232,
    WARLOCK_RAIN_OF_FIRE_TRIGGERED          = 42223,
    WARLOCK_SPAWN_PURPLE_DEMONIC_GATEWAY    = 113890,
    WARLOCK_DEMONIC_GATEWAY_TELEPORT_GREEN  = 113896,
    WARLOCK_DEMONIC_GATEWAY_TELEPORT_PURPLE = 120729,
    WARLOCK_DEMONIC_GATEWAY_PERIODIC_CHARGE = 113901,
    WARLOCK_NIGHTFALL                       = 108558,
    SPELL_WARLOCK_SOUL_SWAP_AURA            = 86211,
    SPELL_WARLOCK_SOUL_SWAP_CD_MARKER       = 94229,
    SPELL_WARLOCK_SOUL_SWAP_DOT_MARKER      = 92795,
    SPELL_WARLOCK_SOUL_SWAP_MOD_COST        = 92794,
    SPELL_WARLOCK_SOUL_SWAP_OVERRIDE        = 86211,
    SPELL_WARLOCK_SOUL_SWAP_VISUAL          = 92795,
    WARLOCK_GRIMOIRE_OF_SACRIFICE           = 108503,
    WARLOCK_METAMORPHOSIS                   = 103958,
    WARLOCK_DEMONIC_LEAP_JUMP               = 109163,
    WARLOCK_ITEM_S12_TIER_4                 = 131632,
    WARLOCK_TWILIGHT_WARD_S12               = 131623,
    WARLOCK_TWILIGHT_WARD_METAMORPHOSIS_S12 = 131624,
    WARLOCK_SHADOWFLAME                     = 47960,
    WARLOCK_SOUL_LEECH_ABSORB               = 108366,
    WARLOCK_DARK_REGENERATION               = 108359,
    WARLOCK_DARK_BARGAIN_DOT                = 110914,
    WARLOCK_MOLTEN_CORE                     = 122355,
    WARLOCK_MOLTEN_CORE_AURA                = 122351,
    WARLOCK_WILD_IMP_SUMMON                 = 104317,
    WARLOCK_DEMONIC_CALL                    = 114925,
    WARLOCK_DECIMATE_AURA                   = 108869,
    WARLOCK_SOUL_LEECH_AURA                 = 108370,
    WARLOCK_ARCHIMONDES_VENGEANCE_COOLDOWN  = 116405,
    WARLOCK_ARCHIMONDES_VENGEANCE_DAMAGE    = 124051,
    WARLOCK_ARCHIMONDES_VENGEANCE_PASSIVE   = 116403,
    WARLOCK_SOUL_LINK_DUMMY_AURA            = 108446,
    WARLOCK_GLYPH_OF_CONFLAGRATE            = 56235,
    WARLOCK_SHIELD_OF_SHADOW                = 115232,
    WARLOCK_THREATENING_PRESENCE            = 112042,
    WARLOCK_SUPPLANT_DEMONIC_COMMAND        = 119904,
    WARLOCK_KIL_JAEDENS_CUNNING_PASSIVE     = 108507,
    WARLOCK_AFTERMATH_STUN                  = 85387,
    WARLOCK_IMP_SWARM                       = 104316,
    WARLOCK_DISRUPTED_NETHER                = 114736,
    WARLOCK_GLYPH_OF_SIPHON_LIFE            = 56218,
    WARLOCK_GLYPH_OF_SOULWELL               = 58094,
    WARLOCK_GLYPH_OF_SOULWELL_VISUAL        = 34145,
    WARLOCK_SOULBURN_OVERRIDE_1             = 93312,
    WARLOCK_SOULBURN_OVERRIDE_2             = 93313,
    WARLOCK_SOULBURN_OVERRIDE_3             = 104245,
    WARLOCK_SOULBURN_OVERRIDE_4             = 104249,
    WARLOCK_SOULBURN_OVERRIDE_5             = 104250,
    WARLOCK_SOULBURN_OVERRIDE_6             = 104251,
    WARLOCK_SOULBURN_OVERRIDE_7             = 114787,
    WARLOCK_SEED_OF_CORRUPTION_DUMMY        = 86664,
    WARLOCK_SOULBURN_DEMONIC_CIRCLE_TELE    = 114794,
};

// Soulburn : Health Funnel - 755
class spell_warl_health_funnel : public SpellScriptLoader
{
    public:
        spell_warl_health_funnel() : SpellScriptLoader("spell_warl_health_funnel") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (player->HasAura(WARLOCK_SOULBURN_AURA))
                    {
                        player->CastSpell(player, 104220, true);
                        player->RemoveAurasDueToSpell(WARLOCK_SOULBURN_AURA);
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

// Soulburn : Seed of Corruption - Damage - 87385
class spell_warl_soulburn_seed_of_corruption_damage : public SpellScriptLoader
{
    public:
        spell_warl_soulburn_seed_of_corruption_damage() : SpellScriptLoader("spell_warl_soulburn_seed_of_corruption_damage") { }

        class spell_warl_soulburn_seed_of_corruption_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulburn_seed_of_corruption_damage_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (GetExplTargetUnit())
                    targets.remove(GetExplTargetUnit());
            }

            void HandleScript()
            {
                auto caster = GetCaster();
                if (!caster)
                    return;

                // Remove Soul Burn aura
                if (caster->HasAura(WARLOCK_SEED_OF_CORRUPTION_DUMMY))
                    caster->RemoveAurasDueToSpell(WARLOCK_SEED_OF_CORRUPTION_DUMMY);
            }

            void HandleAfterCast()
            {
                auto caster = GetCaster();
                if (!caster)
                    return;

                // Give Soul Shard
                if (caster->HasAura(WARLOCK_SEED_OF_CORRUPTION_DUMMY))
                    caster->SetPower(POWER_SOUL_SHARDS, caster->GetPower(POWER_SOUL_SHARDS) + 100);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_soulburn_seed_of_corruption_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                AfterHit += SpellHitFn(spell_warl_soulburn_seed_of_corruption_damage_SpellScript::HandleScript);
                OnCast += SpellCastFn(spell_warl_soulburn_seed_of_corruption_damage_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulburn_seed_of_corruption_damage_SpellScript();
        }
};

// Called by Soulburn : Seed of Corruption - 114790
class spell_warl_soulburn_seed_of_corruption : public SpellScriptLoader
{
    public:
        spell_warl_soulburn_seed_of_corruption() : SpellScriptLoader("spell_warl_soulburn_seed_of_corruption") { }

        class spell_warl_soulburn_seed_of_corruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulburn_seed_of_corruption_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    player->CastSpell(player, WARLOCK_SEED_OF_CORRUPTION_DUMMY, true);

                    if (player->HasAura(WARLOCK_SOULBURN_AURA))
                        player->RemoveAurasDueToSpell(WARLOCK_SOULBURN_AURA);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_soulburn_seed_of_corruption_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulburn_seed_of_corruption_SpellScript();
        }
};

// Called by Soulburn : Curse of Enfeeblement - 109468, Soulburn : Curse of Elements - 104225 and Soulburn : Curse of Exhaustion - 104223
// Soulburn aura - Remove - 74434
class spell_warl_soulburn_remove : public SpellScriptLoader
{
    public:
        spell_warl_soulburn_remove() : SpellScriptLoader("spell_warl_soulburn_remove") { }

        class spell_warl_soulburn_remove_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulburn_remove_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (player->HasAura(WARLOCK_SOULBURN_AURA))
                        player->RemoveAurasDueToSpell(WARLOCK_SOULBURN_AURA);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_soulburn_remove_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulburn_remove_SpellScript();
        }
};

// Soulburn - 74434
class spell_warl_soulburn_override : public SpellScriptLoader
{
    public:
        spell_warl_soulburn_override() : SpellScriptLoader("spell_warl_soulburn_override") { }

        class spell_warl_soulburn_override_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soulburn_override_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* player = GetCaster()->ToPlayer())
                {
                    // Overrides Drain Life, Undending Breath and Harvest Life
                    player->CastSpell(player, WARLOCK_SOULBURN_OVERRIDE_1, true);
                    // Overrides Seed of Corruption
                    player->CastSpell(player, WARLOCK_SOULBURN_OVERRIDE_2, true);
                    // Overrides Curse of Enfeeblement and Curse of Elements
                    player->CastSpell(player, WARLOCK_SOULBURN_OVERRIDE_3, true);
                    // Overrides Demonic Circle : Teleport
                    player->CastSpell(player, WARLOCK_SOULBURN_OVERRIDE_4, true);
                    // Overrides Curse of Exhaustion
                    player->CastSpell(player, WARLOCK_SOULBURN_OVERRIDE_5, true);
                    // Overrides Soul Swap
                    player->CastSpell(player, WARLOCK_SOULBURN_OVERRIDE_6, true);
                    // Overrides Health Funnel
                    //player->CastSpell(player, WARLOCK_SOULBURN_OVERRIDE_7, true);
                }
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* player = GetCaster()->ToPlayer())
                {
                    // Overrides Drain Life, Undending Breath and Harvest Life
                    player->RemoveAura(WARLOCK_SOULBURN_OVERRIDE_1);
                    // Overrides Seed of Corruption
                    player->RemoveAura(WARLOCK_SOULBURN_OVERRIDE_2);
                    // Overrides Curse of Enfeeblement and Curse of Elements
                    player->RemoveAura(WARLOCK_SOULBURN_OVERRIDE_3);
                    // Overrides Demonic Circle : Teleport
                    player->RemoveAura(WARLOCK_SOULBURN_OVERRIDE_4);
                    // Overrides Curse of Exhaustion
                    player->RemoveAura(WARLOCK_SOULBURN_OVERRIDE_5);
                    // Overrides Soul Swap
                    player->RemoveAura(WARLOCK_SOULBURN_OVERRIDE_6);
                    // Overrides Health Funnel
                    //player->RemoveAura(WARLOCK_SOULBURN_OVERRIDE_7);
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_soulburn_override_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_soulburn_override_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soulburn_override_AuraScript();
        }
};

// Called by Create Soulwell - 29893
// Glyph of Soulwell - 58094
class spell_warl_glyph_of_soulwell : public SpellScriptLoader
{
    public:
        spell_warl_glyph_of_soulwell() : SpellScriptLoader("spell_warl_glyph_of_soulwell") { }

        class spell_warl_glyph_of_soulwell_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_glyph_of_soulwell_SpellScript);

            void HandleAfterCast()
            {
                if (!GetCaster())
                    return;

                if (!GetExplTargetDest())
                    return;

                if (!GetCaster()->HasAura(WARLOCK_GLYPH_OF_SOULWELL))
                    return;

                GetCaster()->CastSpell(GetExplTargetDest()->GetPositionX(), GetExplTargetDest()->GetPositionY(), GetExplTargetDest()->GetPositionZ(), WARLOCK_GLYPH_OF_SOULWELL_VISUAL, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_warl_glyph_of_soulwell_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_glyph_of_soulwell_SpellScript();
        }
};

// Imp Swarm - 104316
class spell_warl_imp_swarm : public SpellScriptLoader
{
    public:
        spell_warl_imp_swarm() : SpellScriptLoader("spell_warl_imp_swarm") { }

        class spell_warl_imp_swarm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_imp_swarm_SpellScript);

            SpellCastResult CheckSpec()
            {
                if (!GetCaster())
                    return SPELL_FAILED_DONT_REPORT;

                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_DONT_REPORT;

                Player* _plr = GetCaster()->ToPlayer();
                if (_plr->GetSpecializationId(_plr->GetActiveSpec()) != SPEC_WARLOCK_DEMONOLOGY)
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* _plr = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        _plr->AddAura(WARLOCK_DEMONIC_CALL, _plr);
                        _plr->CastSpell(target, WARLOCK_WILD_IMP_SUMMON, true);
                        _plr->AddAura(WARLOCK_DEMONIC_CALL, _plr);
                        _plr->CastSpell(target, WARLOCK_WILD_IMP_SUMMON, true);
                        _plr->AddAura(WARLOCK_DEMONIC_CALL, _plr);
                        _plr->CastSpell(target, WARLOCK_WILD_IMP_SUMMON, true);
                        _plr->AddAura(WARLOCK_DEMONIC_CALL, _plr);
                        _plr->CastSpell(target, WARLOCK_WILD_IMP_SUMMON, true);
                        _plr->AddAura(WARLOCK_DEMONIC_CALL, _plr);
                        _plr->CastSpell(target, WARLOCK_WILD_IMP_SUMMON, true);
                        _plr->RemoveAura(WARLOCK_DEMONIC_CALL);
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_imp_swarm_SpellScript::CheckSpec);
                OnEffectHitTarget += SpellEffectFn(spell_warl_imp_swarm_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_imp_swarm_SpellScript();
        }
};

// Singe Magic
class spell_warl_singe_magic : public SpellScriptLoader
{
    public:
        spell_warl_singe_magic() : SpellScriptLoader("spell_warl_singe_magic") { }

        class spell_warl_singe_magic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_singe_magic_SpellScript);

            SpellCastResult CheckAutoCast()
            {
                if (!GetCaster())
                    return SPELL_FAILED_DONT_REPORT;

                Unit* owner = GetCaster()->GetOwner();
                if (!owner)
                    return SPELL_FAILED_DONT_REPORT;

                if (!owner->HasCrowdControlAura(NULL, DISPEL_MAGIC))
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckAutoCast += SpellCheckCastFn(spell_warl_singe_magic_SpellScript::CheckAutoCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_singe_magic_SpellScript();
        }
};

// Glyph of Imp Swarm - 56242
class spell_warl_glyph_of_imp_swarm : public SpellScriptLoader
{
    public:
        spell_warl_glyph_of_imp_swarm() : SpellScriptLoader("spell_warl_glyph_of_imp_swarm") { }

        class spell_warl_glyph_of_imp_swarm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_glyph_of_imp_swarm_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(WARLOCK_IMP_SWARM, false);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(WARLOCK_IMP_SWARM))
                        _player->removeSpell(WARLOCK_IMP_SWARM, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_glyph_of_imp_swarm_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_glyph_of_imp_swarm_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_glyph_of_imp_swarm_AuraScript();
        }
};

// Unbound Will - 108482
class spell_warl_unbound_will : public SpellScriptLoader
{
    public:
        spell_warl_unbound_will() : SpellScriptLoader("spell_warl_unbound_will") { }

        class spell_warl_unbound_will_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_unbound_will_SpellScript);

            SpellCastResult CheckHealth()
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->GetHealthPct() <= 20.0f)
                    {
                        SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_NOT_ENOUGH_HEALTH);
                        return SPELL_FAILED_CUSTOM_ERROR;
                    }
                    else
                        return SPELL_CAST_OK;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    player->ModifyHealth(-int32(player->CountPctFromMaxHealth(20)));
                    player->RemoveMovementImpairingAuras();
                    player->RemoveAurasWithMechanic(IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK, AURA_REMOVE_BY_DEFAULT, GetSpellInfo()->Id);
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_unbound_will_SpellScript::CheckHealth);
                OnHit += SpellHitFn(spell_warl_unbound_will_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_unbound_will_SpellScript();
        }
};

// Kil'Jaeden's Cunning (passive with cooldown) - 119048
class spell_warl_kil_jaedens_cunning : public SpellScriptLoader
{
    public:
        spell_warl_kil_jaedens_cunning() : SpellScriptLoader("spell_warl_kil_jaedens_cunning") { }

        class spell_warl_kil_jaedens_cunning_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_kil_jaedens_cunning_AuraScript);

            void HandleApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    GetCaster()->RemoveAura(WARLOCK_KIL_JAEDENS_CUNNING_PASSIVE);
            }

            void HandleRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    GetCaster()->CastSpell(GetCaster(), WARLOCK_KIL_JAEDENS_CUNNING_PASSIVE, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_kil_jaedens_cunning_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_kil_jaedens_cunning_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_kil_jaedens_cunning_AuraScript();
        }
};

// Voidwalker : Shield of Shadow - 103130
class spell_warl_shield_of_shadow : public SpellScriptLoader
{
    public:
        spell_warl_shield_of_shadow() : SpellScriptLoader("spell_warl_shield_of_shadow") { }

        class spell_warl_shield_of_shadow_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_shield_of_shadow_AuraScript);

            void OnUpdate(uint32 /*diff*/, AuraEffect * /*aurEff*/)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (Pet* pet = player->GetPet())
                    {
                        if (pet->GetEntry() == 1860) // Voidwalker
                        {
                            if (!pet->HasSpell(WARLOCK_SHIELD_OF_SHADOW))
                                pet->addSpell(WARLOCK_SHIELD_OF_SHADOW);
                            if (!pet->HasSpell(WARLOCK_THREATENING_PRESENCE))
                                pet->addSpell(WARLOCK_THREATENING_PRESENCE);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectUpdate += AuraEffectUpdateFn(spell_warl_shield_of_shadow_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_shield_of_shadow_AuraScript();
        }
};

// Agony - 980
class spell_warl_agony : public SpellScriptLoader
{
    public:
        spell_warl_agony() : SpellScriptLoader("spell_warl_agony") { }

        class spell_warl_agony_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_agony_AuraScript);

            void OnTick(AuraEffect const *aurEff)
            {
                if (GetCaster())
                    if (GetTarget())
                        if (Aura *agony = GetTarget()->GetAura(aurEff->GetSpellInfo()->Id, GetCaster()->GetGUID()))
                            agony->ModStackAmount(1);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_agony_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_agony_AuraScript();
        }
};

// Grimoire of Sacrifice - 108503
class spell_warl_grimoire_of_sacrifice : public SpellScriptLoader
{
    public:
        spell_warl_grimoire_of_sacrifice() : SpellScriptLoader("spell_warl_grimoire_of_sacrifice") { }

        class spell_warl_grimoire_of_sacrifice_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_grimoire_of_sacrifice_SpellScript);

            void HandleAfterCast()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (Pet* pet = player->GetPet())
                    {
                        // Supplant Command Demon
                        if (player->getLevel() >= 56)
                        {
                            int32 bp = 0;

                            player->RemoveAura(WARLOCK_SUPPLANT_DEMONIC_COMMAND);

                            switch (pet->GetEntry())
                            {
                                case ENTRY_IMP:
                                    bp = 132411;// Single Magic
                                    break;
                                case ENTRY_VOIDWALKER:
                                    bp = 132413;// Shadow Bulwark
                                    break;
                                case ENTRY_SUCCUBUS:
                                    bp = 137706;// Whiplash
                                    break;
                                case ENTRY_FELHUNTER:
                                    bp = 132409;// Spell Lock
                                    break;
                                case ENTRY_FELGUARD:
                                    bp = 132410;// Pursuit
                                    break;
                                default:
                                    break;
                            }

                            if (bp)
                                player->CastCustomSpell(player, WARLOCK_SUPPLANT_DEMONIC_COMMAND, &bp, NULL, NULL, true);
                        }
                    }
                }
            }

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    // EFFECT_0 : Instakill
                    // EFFECT_1 : 2% health every 5s
                    // EFFECT_2 : +50% DOT damage for Malefic Grasp, Drain Life and Drain Soul
                    // EFFECT_3 : +30% damage for Shadow Bolt, Hand of Gul'Dan, Soul Fire, Wild Imps and Fel Flame
                    // EFFECT_4 : +25% damage for Incinerate, Conflagrate, Chaos Bolt, Shadowburn and Fel Flame
                    // EFFECT_5 : +50% damage for Fel Flame
                    // EFFECT_6 : +20% Health if Soul Link talent is also chosen
                    // EFFECT_7 : +50% on EFFECT_2 of Malefic Grasp
                    // EFFECT_8 : +50% on EFFECT_4 and EFFECT_5 of Drain Soul -> Always set to 0
                    // EFFECT_9 : Always set to 0
                    // EFFECT_10 : Always set to 0
                    if (Aura *grimoireOfSacrifice = player->GetAura(WARLOCK_GRIMOIRE_OF_SACRIFICE))
                    {
                        if (grimoireOfSacrifice->GetEffect(EFFECT_10))
                            grimoireOfSacrifice->GetEffect(EFFECT_10)->SetAmount(0);
                        if (grimoireOfSacrifice->GetEffect(EFFECT_9))
                            grimoireOfSacrifice->GetEffect(EFFECT_9)->SetAmount(0);
                        if (grimoireOfSacrifice->GetEffect(EFFECT_8))
                            grimoireOfSacrifice->GetEffect(EFFECT_8)->SetAmount(0);

                        switch (player->GetSpecializationId(player->GetActiveSpec()))
                        {
                            case SPEC_WARLOCK_AFFLICTION:
                                if (grimoireOfSacrifice->GetEffect(EFFECT_3))
                                    grimoireOfSacrifice->GetEffect(EFFECT_3)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_4))
                                    grimoireOfSacrifice->GetEffect(EFFECT_4)->SetAmount(0);
                                break;
                            case SPEC_WARLOCK_DEMONOLOGY:
                                if (grimoireOfSacrifice->GetEffect(EFFECT_2))
                                    grimoireOfSacrifice->GetEffect(EFFECT_2)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_4))
                                    grimoireOfSacrifice->GetEffect(EFFECT_4)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_5))
                                    grimoireOfSacrifice->GetEffect(EFFECT_5)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_7))
                                    grimoireOfSacrifice->GetEffect(EFFECT_7)->SetAmount(0);
                                break;
                            case SPEC_WARLOCK_DESTRUCTION:
                                if (grimoireOfSacrifice->GetEffect(EFFECT_2))
                                    grimoireOfSacrifice->GetEffect(EFFECT_2)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_3))
                                    grimoireOfSacrifice->GetEffect(EFFECT_3)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_5))
                                    grimoireOfSacrifice->GetEffect(EFFECT_5)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_7))
                                    grimoireOfSacrifice->GetEffect(EFFECT_7)->SetAmount(0);
                                break;
                            case SPEC_NONE:
                                if (grimoireOfSacrifice->GetEffect(EFFECT_2))
                                    grimoireOfSacrifice->GetEffect(EFFECT_2)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_3))
                                    grimoireOfSacrifice->GetEffect(EFFECT_3)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_4))
                                    grimoireOfSacrifice->GetEffect(EFFECT_4)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_5))
                                    grimoireOfSacrifice->GetEffect(EFFECT_5)->SetAmount(0);
                                if (grimoireOfSacrifice->GetEffect(EFFECT_7))
                                    grimoireOfSacrifice->GetEffect(EFFECT_7)->SetAmount(0);
                            default:
                                break;
                        }
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_warl_grimoire_of_sacrifice_SpellScript::HandleAfterCast);
                OnHit += SpellHitFn(spell_warl_grimoire_of_sacrifice_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_grimoire_of_sacrifice_SpellScript();
        }

        class spell_warl_grimoire_of_sacrifice_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_grimoire_of_sacrifice_AuraScript);

            void HandleRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                if (Player* player = GetTarget()->ToPlayer())
                    player->RemoveAura(WARLOCK_SUPPLANT_DEMONIC_COMMAND);
            }

            void CalculateAmount(const AuraEffect* , int32 & amount, bool & )
            {
                if (Unit* caster = GetCaster())
                    // Need soul link
                    if (!caster->HasSpell(108415))
                        amount = 0;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_grimoire_of_sacrifice_AuraScript::CalculateAmount, EFFECT_6, SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT);
                OnEffectRemove += AuraEffectApplyFn(spell_warl_grimoire_of_sacrifice_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_grimoire_of_sacrifice_AuraScript();
        }
};

// Flames of Xoroth - 120451
class spell_warl_flames_of_xoroth : public SpellScriptLoader
{
    enum NPCEntries
    {
        NPC_FELGUARD    = 17252,
        NPC_WRATHGUARD  = 58965,
        NPC_DOOMGUARD   = 11859,
        NPC_INFERNAL    = 89,
        NPC_IMP         = 416,
    };

    public:
        spell_warl_flames_of_xoroth() : SpellScriptLoader("spell_warl_flames_of_xoroth") { }

        class spell_warl_flames_of_xoroth_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_flames_of_xoroth_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            SpellCastResult CheckPet()
            {
                if (!GetCaster())
                    return SPELL_FAILED_DONT_REPORT;

                auto const player = GetCaster()->ToPlayer();
                if (!player)
                    return SPELL_FAILED_DONT_REPORT;

                if (player->GetPet())
                    return SPELL_FAILED_ALREADY_HAVE_PET;

                if (!player->GetLastPetNumber(true))
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                auto player = GetCaster()->ToPlayer();

                if (uint32 petId = player->GetLastPetNumber(true))
                {
                    if (Pet* newPet = new Pet(player, SUMMON_PET))
                    {
                        if (newPet->LoadPetFromDB(PET_LOAD_BY_ID, petId))
                        {
                            // revive the pet if it is dead
                            if (newPet->getDeathState() == DEAD || newPet->getDeathState() == CORPSE)
                                newPet->setDeathState(ALIVE);

                            newPet->ClearUnitState(uint32(UNIT_STATE_ALL_STATE));
                            newPet->SetFullHealth();
                            newPet->SetPower(newPet->getPowerType(), newPet->GetMaxPower(newPet->getPowerType()));

                            bool needResummon = false;
                            switch (newPet->GetEntry())
                            {
                                case NPC_DOOMGUARD:
                                case NPC_INFERNAL:
                                    needResummon = true;
                                    break;
                                case NPC_FELGUARD:
                                case NPC_WRATHGUARD:
                                    if (player->GetSpecializationId(player->GetActiveSpec()) != SPEC_WARLOCK_DEMONOLOGY)
                                        needResummon = true;
                                    break;
                                default:
                                    break;
                            }
                            if (needResummon)
                            {
                                newPet->UnSummon();
                                player->SummonPet(NPC_IMP, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), 0);
                            }
                        }
                        else
                            delete newPet;
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_flames_of_xoroth_SpellScript::CheckPet);
                OnEffectHitTarget += SpellEffectFn(spell_warl_flames_of_xoroth_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_flames_of_xoroth_SpellScript();
        }
};

// Called by Shadowflame - 47960
// Molten Core - 122351
class spell_warl_molten_core_dot : public SpellScriptLoader
{
    public:
        spell_warl_molten_core_dot() : SpellScriptLoader("spell_warl_molten_core_dot") { }

        class spell_warl_molten_core_dot_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_molten_core_dot_AuraScript);

            void OnTick(AuraEffect const *aurEff)
            {
                if (GetCaster())
                {
                    if (GetCaster()->HasAura(WARLOCK_MOLTEN_CORE_AURA))
                        if (roll_chance_i(8))
                            GetCaster()->CastSpell(GetCaster(), WARLOCK_MOLTEN_CORE, true);

                    GetCaster()->EnergizeBySpell(GetCaster(), aurEff->GetSpellInfo()->Id, 2 * aurEff->GetBase()->GetStackAmount(), POWER_DEMONIC_FURY);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_molten_core_dot_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_molten_core_dot_AuraScript();
        }
};

// Called by Shadow Bolt - 686 and Soul Fire - 6353
// Decimate - 108869
class spell_warl_decimate : public SpellScriptLoader
{
    public:
        spell_warl_decimate() : SpellScriptLoader("spell_warl_decimate") { }

        class spell_warl_decimate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_decimate_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(WARLOCK_DECIMATE_AURA))
                            if (target->GetHealthPct() < 25.0f)
                                _player->CastSpell(_player, WARLOCK_MOLTEN_CORE, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_decimate_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_decimate_SpellScript();
        }
};

// Called by Shadow Bolt - 686, Soul Fire - 6353 and Touch of Chaos - 103964
// Demonic Call - 114925
class spell_warl_demonic_call : public SpellScriptLoader
{
    public:
        spell_warl_demonic_call() : SpellScriptLoader("spell_warl_demonic_call") { }

        class spell_warl_demonic_call_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_call_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        if (player->HasAura(WARLOCK_DEMONIC_CALL) && !player->HasAura(WARLOCK_DISRUPTED_NETHER))
                        {
                            player->CastSpell(player, WARLOCK_WILD_IMP_SUMMON, true);
                            player->RemoveAura(WARLOCK_DEMONIC_CALL);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_demonic_call_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_call_SpellScript();
        }
};

// Void Ray - 115422 and Touch of Chaos - 103964
class spell_warl_void_ray : public SpellScriptLoader
{
    public:
        spell_warl_void_ray() : SpellScriptLoader("spell_warl_void_ray") { }

        class spell_warl_void_ray_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_void_ray_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (Aura *corruption = target->GetAura(WARLOCK_CORRUPTION_DAMAGE_AURA, _player->GetGUID()))
                        {
                            corruption->SetDuration(corruption->GetDuration() + 4000);
                            corruption->SetNeedClientUpdateForTargets();
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_void_ray_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_void_ray_SpellScript();
        }
};

// Chaos Wave - 124916
class spell_warl_chaos_wave : public SpellScriptLoader
{
    public:
        spell_warl_chaos_wave() : SpellScriptLoader("spell_warl_chaos_wave") { }

        class spell_warl_chaos_wave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_chaos_wave_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->CastSpell(_player, WARLOCK_MOLTEN_CORE, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_warl_chaos_wave_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_chaos_wave_SpellScript();
        }
};

// Metamorphosis - 103958
class spell_warl_metamorphosis_cost : public SpellScriptLoader
{
    public:
        spell_warl_metamorphosis_cost() : SpellScriptLoader("spell_warl_metamorphosis_cost") { }

        class spell_warl_metamorphosis_cost_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_metamorphosis_cost_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                    GetCaster()->EnergizeBySpell(GetCaster(), WARLOCK_METAMORPHOSIS, -6, POWER_DEMONIC_FURY);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_metamorphosis_cost_AuraScript::OnTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_metamorphosis_cost_AuraScript();
        }
};

// Immolation Aura - 104025
class spell_warl_immolation_aura : public SpellScriptLoader
{
    public:
        spell_warl_immolation_aura() : SpellScriptLoader("spell_warl_immolation_aura") { }

        class spell_warl_immolation_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_immolation_aura_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                    GetCaster()->EnergizeBySpell(GetCaster(), GetSpellInfo()->Id, -25, POWER_DEMONIC_FURY);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_immolation_aura_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_immolation_aura_AuraScript();
        }
};

// Dark Bargain - 110013
class spell_warl_dark_bargain_on_absorb : public SpellScriptLoader
{
    public:
        spell_warl_dark_bargain_on_absorb() : SpellScriptLoader("spell_warl_dark_bargain_on_absorb") { }

        class spell_warl_dark_bargain_on_absorb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_dark_bargain_on_absorb_AuraScript);

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                PreventDefaultAction();
                int32 damage = dmgInfo.GetDamage();
                dmgInfo.AbsorbDamage(dmgInfo.GetDamage());
                aurEff->SetAmount(aurEff->GetAmount() + damage);
            }

            void OnRemove(AuraEffect const * aurEff, AuraEffectHandleModes /*mode*/)
            {
                // (totalAbsorbAmount / 16) it's for totalAbsorbAmount 50% & totalAbsorbAmount / 8 (for each tick of custom spell)
                if (Unit* caster = GetCaster())
                {
                    SpellInfo const* triggerInfo = sSpellMgr->GetSpellInfo(WARLOCK_DARK_BARGAIN_DOT);
                    int32 basePoints = aurEff->GetAmount() * GetSpellInfo()->Effects[EFFECT_1].CalcValue() / 100.0f / triggerInfo->GetMaxTicks();
                    caster->CastCustomSpell(WARLOCK_DARK_BARGAIN_DOT, SPELLVALUE_BASE_POINT0, basePoints, caster, true);
                }
            }

            void Register()
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_warl_dark_bargain_on_absorb_AuraScript::Absorb, EFFECT_0);
                AfterEffectRemove += AuraEffectRemoveFn(spell_warl_dark_bargain_on_absorb_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_dark_bargain_on_absorb_AuraScript();
        }
};


// Metamorphosis overrides
class spell_warl_metamorphosis_overrides : public SpellScriptLoader
{
    public:
        spell_warl_metamorphosis_overrides() : SpellScriptLoader("spell_warl_metamorphosis_overrides") { }

        class spell_warl_metamorphosis_overrides_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_metamorphosis_overrides_AuraScript);

            void CheckTouchOfChaos(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HasSpell(112089))
                        amount = 0;
                }
            }
            void CheckChaosWave(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HasSpell(124917))
                        amount = 0;
                }
            }
            void CheckCursedAuras(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HasSpell(116208))
                        amount = 0;
                }
            }
            void CheckSleep(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HasAura(114168))
                        amount = 0;
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_metamorphosis_overrides_AuraScript::CheckTouchOfChaos, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_metamorphosis_overrides_AuraScript::CheckChaosWave, EFFECT_3, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_metamorphosis_overrides_AuraScript::CheckCursedAuras, EFFECT_6, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_metamorphosis_overrides_AuraScript::CheckSleep, EFFECT_7, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_metamorphosis_overrides_AuraScript();
        }
};

// Dark Regeneration - 108359
class spell_warl_dark_regeneration : public SpellScriptLoader
{
    public:
        spell_warl_dark_regeneration() : SpellScriptLoader("spell_warl_dark_regeneration") { }

        class spell_warl_dark_regeneration_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_dark_regeneration_AuraScript);

            void HandleApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget())
                    if (Guardian* pet = GetTarget()->GetGuardianPet())
                        pet->CastSpell(pet, WARLOCK_DARK_REGENERATION, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_dark_regeneration_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_dark_regeneration_AuraScript();
        }
};

// Called by Haunt - 48181, Fel Flame - 77799, Shadow Bolt - 686, Incinerate - 29722, Chaos Bolt - 116858
// Touch of Chaos - 103964, Demonic Slash - 114175, Soul Fire - 6353, Soul Fire (Metamorphosis) - 104027
// Void Ray - 115422 and Shadow Burn - 17877
// Soul Leech - 108370
class spell_warl_soul_leech : public SpellScriptLoader
{
    public:
        spell_warl_soul_leech() : SpellScriptLoader("spell_warl_soul_leech") { }

        class spell_warl_soul_leech_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_leech_SpellScript);

            void HandleAfterHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        if (player->HasAura(WARLOCK_SOUL_LEECH_AURA))
                        {
                            uint32 absorbPct = 20;
                            if (player->GetSpecializationId(player->GetActiveSpec()) != SPEC_WARLOCK_AFFLICTION)
                                absorbPct = 10;

                            int32 bp = CalculatePct(GetHitDamage(), absorbPct);

                            if (!bp)
                                return;

                            if (auto soulLeech = player->GetAuraEffect(WARLOCK_SOUL_LEECH_ABSORB, EFFECT_0))
                                bp += soulLeech->GetAmount();
                            // Calculate amount cap at 25% of caster health
                            bp = std::min(bp, (int32)player->CountPctFromMaxHealth(25));

                            player->CastCustomSpell(player, WARLOCK_SOUL_LEECH_ABSORB, &bp, NULL, NULL, true);

                            if (Pet* pet = player->GetPet())
                                player->CastCustomSpell(pet, WARLOCK_SOUL_LEECH_ABSORB, &bp, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_warl_soul_leech_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soul_leech_SpellScript();
        }
};

// Sacrificial Pact - 108416
class spell_warl_sacrificial_pact : public SpellScriptLoader
{
    public:
        spell_warl_sacrificial_pact() : SpellScriptLoader("spell_warl_sacrificial_pact") { }

        class spell_warl_sacrificial_pact_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_sacrificial_pact_AuraScript);

            void CalculateAmount(AuraEffect const *, int32 & amount, bool & )
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->GetGuardianPet())
                    {
                        int32 sacrifiedHealth = caster->CountPctFromCurHealth(GetSpellInfo()->Effects[EFFECT_1].BasePoints);
                        caster->ModifyHealth(-sacrifiedHealth);
                        amount = CalculatePct(sacrifiedHealth, amount);
                    }
                    else if (caster->GetGuardianPet())
                    {
                        int32 sacrifiedHealth = caster->GetGuardianPet()->CountPctFromCurHealth(GetSpellInfo()->Effects[EFFECT_1].BasePoints);
                        caster->GetGuardianPet()->ModifyHealth(-sacrifiedHealth);
                        amount = CalculatePct(sacrifiedHealth, amount);
                    }
                }
            }
            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_sacrificial_pact_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_sacrificial_pact_AuraScript();
        }
};

// Hand of Gul'Dan - 105174, 86040
class spell_warl_hand_of_guldan : public SpellScriptLoader
{
    public:
        spell_warl_hand_of_guldan() : SpellScriptLoader("spell_warl_hand_of_guldan") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                auto _player = GetCaster()->ToPlayer();
                auto target = GetHitUnit();
                if (!target || !_player)
                    return;

                if (GetSpellInfo()->Id == 86040)
                    _player->CastSpell(target, WARLOCK_SHADOWFLAME, true);
                else
                    _player->CastSpell(GetExplTargetDest()->GetPositionX(), GetExplTargetDest()->GetPositionY(), GetExplTargetDest()->GetPositionZ(), 86040, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(script_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
                OnEffectHitTarget += SpellEffectFn(script_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

// Twilight Ward - 6229 and Twilight Ward (Metamorphosis) - 104048
class spell_warl_twilight_ward_s12 : public SpellScriptLoader
{
    public:
        spell_warl_twilight_ward_s12() : SpellScriptLoader("spell_warl_twilight_ward_s12") { }

        class spell_warl_twilight_ward_s12_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_twilight_ward_s12_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(WARLOCK_ITEM_S12_TIER_4))
                    {
                        if (GetSpellInfo()->Id == 6229)
                        {
                            if (_player->HasAura(GetSpellInfo()->Id))
                                _player->RemoveAura(GetSpellInfo()->Id);

                            _player->CastSpell(_player, WARLOCK_TWILIGHT_WARD_S12, true);
                        }
                        else if (GetSpellInfo()->Id == 104048)
                        {
                            if (_player->HasAura(GetSpellInfo()->Id))
                                _player->RemoveAura(GetSpellInfo()->Id);

                            _player->CastSpell(_player, WARLOCK_TWILIGHT_WARD_METAMORPHOSIS_S12, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_twilight_ward_s12_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_twilight_ward_s12_SpellScript();
        }
};

// Hellfire - 5857
class spell_warl_hellfire : public SpellScriptLoader
{
    public:
        spell_warl_hellfire() : SpellScriptLoader("spell_warl_hellfire") { }

        class spell_warl_hellfire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_hellfire_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (GetHitUnit())
                        player->EnergizeBySpell(player, GetSpellInfo()->Id, 4, POWER_DEMONIC_FURY);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_hellfire_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_hellfire_SpellScript();
        }
};

// Demonic Leap (jump) - 109163
class spell_warl_demonic_leap_jump : public SpellScriptLoader
{
    public:
        spell_warl_demonic_leap_jump() : SpellScriptLoader("spell_warl_demonic_leap_jump") { }

        class spell_warl_demonic_leap_jump_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_leap_jump_SpellScript);

            enum
            {
                SPELL_WARLOCK_GLYPH_OF_FALLING_METEOR = 56247,
                SPELL_WARLOCK_METEOR_SLAM = 124315
            };

            SpellCastResult CheckElevation()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return SPELL_FAILED_DONT_REPORT;

                if (caster->HasAuraType(SPELL_AURA_MOD_ROOT))
                {
                    caster->RemoveAura(WARLOCK_METAMORPHOSIS);
                    return SPELL_FAILED_ROOTED;
                }

                return SPELL_CAST_OK;
            }

            void HandleGlyph()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_WARLOCK_GLYPH_OF_FALLING_METEOR) && caster->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
                        caster->CastSpell(caster, SPELL_WARLOCK_METEOR_SLAM, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_warl_demonic_leap_jump_SpellScript::HandleGlyph);
                OnCheckCast += SpellCheckCastFn(spell_warl_demonic_leap_jump_SpellScript::CheckElevation);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_leap_jump_SpellScript();
        }
};

// Demonic Leap - 109151
class spell_warl_demonic_leap : public SpellScriptLoader
{
    public:
        spell_warl_demonic_leap() : SpellScriptLoader("spell_warl_demonic_leap") { }

        class spell_warl_demonic_leap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_leap_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    _player->CastSpell(_player, WARLOCK_METAMORPHOSIS, true);
                    _player->CastSpell(_player, WARLOCK_DEMONIC_LEAP_JUMP, true);
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_warl_demonic_leap_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_leap_SpellScript();
        }
};

// Burning Rush - 111400
class spell_warl_burning_rush : public SpellScriptLoader
{
    public:
        spell_warl_burning_rush() : SpellScriptLoader("spell_warl_burning_rush") { }

        class spell_warl_burning_rush_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_burning_rush_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                {
                    // Drain 4% of health every second
                    int32 basepoints = GetCaster()->CountPctFromMaxHealth(4);

                    GetCaster()->DealDamage(GetCaster(), basepoints, NULL, NODAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_burning_rush_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_burning_rush_AuraScript();
        }
};

// Soul Swap : Soulburn - 119678
class spell_warl_soul_swap_soulburn : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap_soulburn() : SpellScriptLoader("spell_warl_soul_swap_soulburn") { }

        class spell_warl_soul_swap_soulburn_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_swap_soulburn_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // Apply instantly corruption, unstable affliction and agony on the target
                        _player->AddAura(WARLOCK_CORRUPTION_DAMAGE_AURA, target);
                        _player->AddAura(WARLOCK_UNSTABLE_AFFLICTION, target);
                        _player->AddAura(WARLOCK_AGONY, target);
                        _player->RemoveAura(WARLOCK_SOULBURN_AURA);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_soul_swap_soulburn_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soul_swap_soulburn_SpellScript();
        }
};

// 86211 - Soul Swap Override - Also acts as a dot container
class spell_warl_soul_swap_override : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap_override() : SpellScriptLoader("spell_warl_soul_swap_override") { }

        class spell_warl_soul_swap_override_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_swap_override_AuraScript);

            bool Load()
            {
                _swapCaster = 0;
                return true;
            }

            //! Forced to, pure virtual functions must have a body when linking
            void Register() { }

        public:
            void AddDot(Aura* aur)
            {
                EssentialAuraInfo eAura;
                for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
                    if (aur->HasEffect(i))
                        eAura.amount[i] = aur->GetEffect(i)->GetFixedDamageInfo().GetFixedDamage();

                eAura.duration = aur->GetDuration();
                eAura.maxDuration = aur->GetMaxDuration();
                eAura.effectMask = aur->GetEffectMask();
                eAura.spellId = aur->GetId();
                eAura.stackAmount = aur->GetStackAmount();
                _dotList.push_back(eAura);
            }
            std::list<EssentialAuraInfo> const GetDotList() const       { return _dotList; }
            uint64 GetOriginalSwapSource() const                        { return _swapCaster; }
            void SetOriginalSwapSource(uint64 victimGuid)               { _swapCaster = victimGuid; }

        private:
            std::list<EssentialAuraInfo> _dotList;
            uint64 _swapCaster;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_soul_swap_override_AuraScript();
        }
};

typedef spell_warl_soul_swap_override::spell_warl_soul_swap_override_AuraScript SoulSwapOverrideAuraScript;


// Soul Swap - 86121 or Soul Swap : Exhale - 86213
class spell_warl_soul_swap : public SpellScriptLoader
{
public:
    spell_warl_soul_swap() : SpellScriptLoader("spell_warl_soul_swap") { }

    class spell_warl_soul_swap_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warl_soul_swap_SpellScript);

        SpellCastResult CheckCast()
        {
            if (Unit* currentTarget = GetExplTargetUnit())
            {
                bool hasDots = false;
                SpellInfo const* flagInfo = sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_DOT_MARKER);
                for (Unit::AuraApplicationMap::const_iterator itr = currentTarget->GetAppliedAuras().begin(); itr != currentTarget->GetAppliedAuras().end(); ++itr)
                {
                    SpellInfo const* spellProto = itr->second->GetBase()->GetSpellInfo();
                    if (itr->second->GetBase()->GetCaster() == GetCaster())
                        if (spellProto->SpellFamilyName == SPELLFAMILY_WARLOCK && (spellProto->SpellFamilyFlags & flagInfo->Effects[0].SpellClassMask))
                        {
                            hasDots = true;
                            break;
                        }
                }

                if (!hasDots)
                    return SPELL_FAILED_BAD_TARGETS;
            }

            return SPELL_CAST_OK;
        }

        void HandleOnHit()
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetHitUnit())
                {
                    // Soul Swap override spell
                    caster->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_AURA, true);
                    target->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_DOT_MARKER, true);

                    if (Aura* swapOverrideAura = caster->GetAura(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
                        if (SoulSwapOverrideAuraScript* swapScript = dynamic_cast<SoulSwapOverrideAuraScript*>(swapOverrideAura->GetScriptByName("spell_warl_soul_swap_override")))
                            swapScript->SetOriginalSwapSource(target->GetGUID());
                }
            }
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_warl_soul_swap_SpellScript::CheckCast);
            OnHit += SpellHitFn(spell_warl_soul_swap_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_warl_soul_swap_SpellScript();
    }
};

//! Soul Swap Copy Spells - 92795 - Simply copies spell IDs.
class spell_warl_soul_swap_dot_marker : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap_dot_marker() : SpellScriptLoader("spell_warl_soul_swap_dot_marker") { }

        class spell_warl_soul_swap_dot_marker_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_swap_dot_marker_SpellScript);

            void HandleAfterCast()
            {
                Unit* swapVictim = GetCaster();
                Unit* warlock    = GetExplTargetUnit();
                if (!warlock || !swapVictim)
                    return;

                Trinity::Flag128 classMask = GetSpellInfo()->Effects[EFFECT_0].SpellClassMask;
                Unit::AuraApplicationMap const& appliedAuras = swapVictim->GetAppliedAuras();
                SoulSwapOverrideAuraScript* swapSpellScript = NULL;
                if (Aura* swapOverrideAura = warlock->GetAura(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
                    swapSpellScript = dynamic_cast<SoulSwapOverrideAuraScript*>(swapOverrideAura->GetScriptByName("spell_warl_soul_swap_override"));

                if (swapSpellScript == NULL)
                    return;

                for (Unit::AuraApplicationMap::const_iterator itr = appliedAuras.begin(); itr != appliedAuras.end(); ++itr)
                {
                    SpellInfo const* spellProto = itr->second->GetBase()->GetSpellInfo();
                    if (itr->second->GetBase()->GetCaster() == warlock)
                        if (spellProto->SpellFamilyName == SPELLFAMILY_WARLOCK && (spellProto->SpellFamilyFlags & classMask))
                            swapSpellScript->AddDot(itr->second->GetBase());
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_warl_soul_swap_dot_marker_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_soul_swap_dot_marker_SpellScript();
        }
};

// 86213 - Soul Swap Exhale
class spell_warl_soul_swap_exhale : public SpellScriptLoader
{
public:
    spell_warl_soul_swap_exhale() : SpellScriptLoader("spell_warl_soul_swap_exhale") { }

    class spell_warl_soul_swap_exhale_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warl_soul_swap_exhale_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_MOD_COST) ||
                !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
                return false;
            return true;
        }

        SpellCastResult CheckCast()
        {
            Unit* currentTarget = GetExplTargetUnit();
            uint64 swapTargetGuid = 0;
            if (Aura const* swapOverride = GetCaster()->GetAura(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
                if (SoulSwapOverrideAuraScript* swapScript = dynamic_cast<SoulSwapOverrideAuraScript*>(swapOverride->GetScriptByName("spell_warl_soul_swap_override")))
                    swapTargetGuid = swapScript->GetOriginalSwapSource();

            // Soul Swap Exhale can't be cast on the same target than Soul Swap
            if (swapTargetGuid && currentTarget && swapTargetGuid == currentTarget->GetGUID())
                return SPELL_FAILED_BAD_TARGETS;

            return SPELL_CAST_OK;
        }

        void OnEffectHit(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_SOUL_SWAP_MOD_COST, true);
            std::list<EssentialAuraInfo> dotList;
            if (Aura const* swapOverride = GetCaster()->GetAura(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
            {
                SoulSwapOverrideAuraScript* swapScript = dynamic_cast<SoulSwapOverrideAuraScript*>(swapOverride->GetScriptByName("spell_warl_soul_swap_override"));
                if (!swapScript)
                    return;
                dotList = swapScript->GetDotList();
            }

            if (dotList.empty())
                return;

            for (std::list<EssentialAuraInfo>::const_iterator itr = dotList.begin(); itr != dotList.end(); ++itr)
            {
                if (Aura* newAura = GetCaster()->AddAura(sSpellMgr->GetSpellInfo(itr->spellId), itr->effectMask, GetHitUnit()))
                {
                    newAura->SetMaxDuration(itr->duration);
                    newAura->SetDuration(itr->duration);
                    newAura->SetStackAmount(itr->stackAmount);
                    for (int i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        if (itr->effectMask & (1 << i) && newAura->GetEffectMask() & (1 << i))
                            newAura->GetEffect(i)->GetFixedDamageInfo().SetFixedDamage(itr->amount[i]);
                }
            }

            // Remove Soul Swap Exhale buff
            GetCaster()->RemoveAurasDueToSpell(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE);
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_warl_soul_swap_exhale_SpellScript::CheckCast);
            OnEffectLaunchTarget += SpellEffectFn(spell_warl_soul_swap_exhale_SpellScript::OnEffectHit, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_warl_soul_swap_exhale_SpellScript();
    }
};

// Called by Corruption - 172
// Nightfall - 108558
class spell_warl_nightfall : public SpellScriptLoader
{
    public:
        spell_warl_nightfall() : SpellScriptLoader("spell_warl_nightfall") { }

        class spell_warl_nightfall_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_nightfall_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (!GetCaster())
                    return;

                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (player->HasAura(WARLOCK_NIGHTFALL))
                        if (roll_chance_i(5))
                            player->SetPower(POWER_SOUL_SHARDS, player->GetPower(POWER_SOUL_SHARDS) + 100);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_nightfall_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_nightfall_AuraScript();
        }
};

// Drain Soul - 1120
class spell_warl_drain_soul : public SpellScriptLoader
{
    public:
        spell_warl_drain_soul() : SpellScriptLoader("spell_warl_drain_soul") { }

        class spell_warl_drain_soul_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_drain_soul_AuraScript);

            void HandleRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_DEATH)
                        GetCaster()->SetPower(POWER_SOUL_SHARDS, GetCaster()->GetPower(POWER_SOUL_SHARDS) + 300);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_warl_drain_soul_AuraScript::HandleRemove, EFFECT_4, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_drain_soul_AuraScript();
        }
};

// Demonic Gateway - 111771
class spell_warl_demonic_gateway : public SpellScriptLoader
{
    public:
        spell_warl_demonic_gateway() : SpellScriptLoader("spell_warl_demonic_gateway") { }

        class spell_warl_demonic_gateway_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_gateway_SpellScript);

            SpellCastResult HandleCheck()
            {
                if (Position* pos = GetHitDest())
                    if (GetCaster()->GetDistanceZ(pos) > 3.5f)
                        return SPELL_FAILED_LINE_OF_SIGHT;

                return SPELL_CAST_OK;
            }

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->CastSpell(_player, WARLOCK_SPAWN_PURPLE_DEMONIC_GATEWAY, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_demonic_gateway_SpellScript::HandleCheck);
                AfterCast += SpellCastFn(spell_warl_demonic_gateway_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_gateway_SpellScript();
        }
};

// Rain of Fire - 104232
class spell_warl_rain_of_fire : public SpellScriptLoader
{
    public:
        spell_warl_rain_of_fire() : SpellScriptLoader("spell_warl_rain_of_fire") { }

        class spell_warl_rain_of_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_rain_of_fire_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                {
                    std::list<DynamicObject*> list;
                    GetCaster()->GetDynObjectList(list, WARLOCK_RAIN_OF_FIRE);
                    if (list.empty())
                        return;

                    for (auto const &dynObj : list)
                        GetCaster()->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), WARLOCK_RAIN_OF_FIRE_TRIGGERED, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_rain_of_fire_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_rain_of_fire_AuraScript();
        }
};

// Called By : Incinerate (Fire and Brimstone) - 114654, Conflagrate (Fire and Brimstone) - 108685
// Curse of the Elements (Fire and Brimstone) - 104225, Curse of Enfeeblement (Fire and Brimstone) - 109468
// Immolate (Fire and Brimstone) - 108686
// Fire and Brimstone - 108683
class spell_warl_fire_and_brimstone : public SpellScriptLoader
{
    public:
        spell_warl_fire_and_brimstone() : SpellScriptLoader("spell_warl_fire_and_brimstone") { }

        class spell_warl_fire_and_brimstone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_fire_and_brimstone_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (GetHitUnit() && player->GetPower(POWER_BURNING_EMBERS) < 10)
                        player->RemoveAura(WARLOCK_FIRE_AND_BRIMSTONE);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_fire_and_brimstone_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_fire_and_brimstone_SpellScript();
        }
};

// Conflagrate - 17962 and Conflagrate (Fire and Brimstone) - 108685
class spell_warl_conflagrate_aura : public SpellScriptLoader
{
    public:
        spell_warl_conflagrate_aura() : SpellScriptLoader("spell_warl_conflagrate_aura") { }

        class spell_warl_conflagrate_aura_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_conflagrate_aura_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (!target->HasAura(WARLOCK_IMMOLATE) && !player->HasAura(WARLOCK_GLYPH_OF_CONFLAGRATE))
                            target->RemoveAura(WARLOCK_CONFLAGRATE);

                        if (!target->HasAura(WARLOCK_IMMOLATE_FIRE_AND_BRIMSTONE))
                            target->RemoveAura(WARLOCK_CONFLAGRATE_FIRE_AND_BRIMSTONE);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_conflagrate_aura_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_conflagrate_aura_SpellScript();
        }
};

// Shadowburn - 29341
class spell_warl_shadowburn : public SpellScriptLoader
{
    public:
        spell_warl_shadowburn() : SpellScriptLoader("spell_warl_shadowburn") { }

        class spell_warl_shadowburn_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_shadowburn_AuraScript);

            void HandleRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_DEATH)
                        GetCaster()->SetPower(POWER_BURNING_EMBERS, GetCaster()->GetPower(POWER_BURNING_EMBERS) + 20); // Give 2 Burning Embers
                    else if (removeMode == AURA_REMOVE_BY_EXPIRE)
                        GetCaster()->CastSpell(GetCaster(), WARLOCK_SHADOWBURN_ENERGIZE, true);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_warl_shadowburn_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_shadowburn_AuraScript();
        }
};

// Drain Life - 689
class spell_warl_drain_life : public SpellScriptLoader
{
    public:
        spell_warl_drain_life() : SpellScriptLoader("spell_warl_drain_life") { }

        class spell_warl_drain_life_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_drain_life_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetCaster()->RemoveAurasDueToSpell(WARLOCK_SOULBURN_AURA);
            }

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                {
                    Player* player = caster->ToPlayer();
                    if (!player)
                        return;

                    // Restoring 2 (3 if Soulburn's) % of the caster's total health every 1s
                    int32 basepoints = player->CountPctFromMaxHealth(GetId() == 89420 ? 3 : 2);

                    // Harvest Life
                    if (Aura const * const harvest = player->GetAura(108371))
                        AddPct(basepoints, harvest->GetSpellInfo()->Effects[EFFECT_1].BasePoints);

                    // In Demonology spec : Generates 10 Demonic Fury per second
                    if (player->GetSpecializationId(player->GetActiveSpec()) == SPEC_WARLOCK_DEMONOLOGY)
                        player->EnergizeBySpell(player, 689, 10, POWER_DEMONIC_FURY);

                    player->CastCustomSpell(player, WARLOCK_DRAIN_LIFE_HEAL, &basepoints, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_drain_life_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                AfterEffectApply += AuraEffectApplyFn(spell_warl_drain_life_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_drain_life_AuraScript();
        }
};

// Soul Harvest - 101976
class spell_warl_soul_harverst : public SpellScriptLoader
{
    public:
        spell_warl_soul_harverst() : SpellScriptLoader("spell_warl_soul_harverst") { }

        class spell_warl_soul_harverst_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_harverst_AuraScript);

            uint32 update;

            bool Validate(SpellInfo const* /*spell*/)
            {
                update = 0;

                if (!sSpellMgr->GetSpellInfo(101976))
                    return false;
                return true;
            }

            void OnUpdate(uint32 diff)
            {
                update += diff;

                if (update >= 1000)
                {
                    if (Player* _player = GetCaster()->ToPlayer())
                    {
                        if (!_player->IsInCombat() && !_player->InArena() && _player->IsAlive())
                        {
                            _player->SetHealth(_player->GetHealth() + int32(_player->GetMaxHealth() / 50));

                            if (Pet* pet = _player->GetPet())
                                pet->SetHealth(pet->GetHealth() + int32(pet->GetMaxHealth() / 50));
                        }
                    }

                    update = 0;
                }
            }

            void Register()
            {
                OnAuraUpdate += AuraUpdateFn(spell_warl_soul_harverst_AuraScript::OnUpdate);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soul_harverst_AuraScript();
        }
};

// Life Tap - 1454
class spell_warl_life_tap : public SpellScriptLoader
{
    public:
        spell_warl_life_tap() : SpellScriptLoader("spell_warl_life_tap") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            SpellCastResult CheckLife()
            {
                if (GetCaster()->GetHealthPct() > 15.0f)
                    return SPELL_CAST_OK;
                return SPELL_FAILED_FIZZLE;
            }

            void HandleOnHit(SpellEffIndex effIndex)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    PreventHitEffect(effIndex);
                    int32 healthCost = int32(_player->GetMaxHealth() * 0.15f);

                    _player->SetHealth(_player->GetHealth() - healthCost);
                    _player->EnergizeBySpell(_player, 1454, healthCost, POWER_MANA);
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(script_impl::CheckLife);
                OnEffectHitTarget += SpellEffectFn(script_impl::HandleOnHit, EFFECT_0, SPELL_EFFECT_ENERGIZE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

// Fear - 5782 and Bloody Fear - 111397
class spell_warl_fear : public SpellScriptLoader
{
    public:
        spell_warl_fear() : SpellScriptLoader("spell_warl_fear") { }

        class spell_warl_fear_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_fear_SpellScript);

            void HandleAfterHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (player->HasAura(WARLOCK_GLYPH_OF_FEAR))
                            player->CastSpell(target, WARLOCK_GLYPH_OF_FEAR_EFFECT, true);
                        else
                            player->CastSpell(target, WARLOCK_FEAR_EFFECT, true);
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_warl_fear_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_fear_SpellScript();
        }
};

// Updated 4.3.4
class spell_warl_banish : public SpellScriptLoader
{
    public:
        spell_warl_banish() : SpellScriptLoader("spell_warl_banish") { }

        class spell_warl_banish_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_banish_SpellScript);

            bool Load()
            {
                _removed = false;
                return true;
            }

            void HandleBanish()
            {
                // Casting Banish on a banished target will cancel the effect
                // Check if the target already has Banish, if so, do nothing.

                if (Unit* target = GetHitUnit())
                {
                    Trinity::Flag128 const flags(0, 0x08000000);
                    if (target->GetAuraEffect(SPELL_AURA_SCHOOL_IMMUNITY, SPELLFAMILY_WARLOCK, flags))
                    {
                        // No need to remove old aura since its removed due to not stack by current Banish aura
                        PreventHitDefaultEffect(EFFECT_0);
                        PreventHitDefaultEffect(EFFECT_1);
                        PreventHitDefaultEffect(EFFECT_2);
                        _removed = true;
                    }
                }
            }

            void RemoveAura()
            {
                if (_removed)
                    PreventHitAura();
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_warl_banish_SpellScript::HandleBanish);
                AfterHit += SpellHitFn(spell_warl_banish_SpellScript::RemoveAura);
            }

            bool _removed;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_banish_SpellScript();
        }
};

// Create Healthstone - 6201
class spell_warl_create_healthstone : public SpellScriptLoader
{
    public:
        spell_warl_create_healthstone() : SpellScriptLoader("spell_warl_create_healthstone") { }

        class spell_warl_create_healthstone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_create_healthstone_SpellScript);

            void HandleAfterCast()
            {
                GetCaster()->CastSpell(GetCaster(), WARLOCK_CREATE_HEALTHSTONE, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_warl_create_healthstone_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_create_healthstone_SpellScript();
        }
};

class spell_warl_seed_of_corruption : public SpellScriptLoader
{
    public:
        spell_warl_seed_of_corruption() : SpellScriptLoader("spell_warl_seed_of_corruption") { }

        class spell_warl_seed_of_corruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_seed_of_corruption_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (GetExplTargetUnit())
                    targets.remove(GetExplTargetUnit());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_seed_of_corruption_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_seed_of_corruption_SpellScript();
        }
};

enum Soulshatter
{
    SPELL_SOULSHATTER   = 32835,
};

class spell_warl_soulshatter : public SpellScriptLoader
{
    public:
        spell_warl_soulshatter() : SpellScriptLoader("spell_warl_soulshatter") { }

        class spell_warl_soulshatter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulshatter_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SOULSHATTER))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    if (target->CanHaveThreatList() && target->getThreatManager().getThreat(caster) > 0.0f)
                        caster->CastSpell(target, SPELL_SOULSHATTER, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_soulshatter_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulshatter_SpellScript();
        }
};

// Demonic Circle : Summon - 48018
class spell_warl_demonic_circle_summon : public SpellScriptLoader
{
    public:
        spell_warl_demonic_circle_summon() : SpellScriptLoader("spell_warl_demonic_circle_summon") { }

        class spell_warl_demonic_circle_summon_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_circle_summon_AuraScript);

            void HandleRemove(AuraEffect const *aurEff, AuraEffectHandleModes mode)
            {
                if (GetTarget())
                {
                    // If effect is removed by expire remove the summoned demonic circle too.
                    if (!(mode & AURA_EFFECT_HANDLE_REAPPLY))
                        GetTarget()->RemoveGameObject(GetId(), true);

                    if (GetTarget()->GetAuraApplication(aurEff->GetSpellInfo()->Id, GetTarget()->GetGUID()))
                        GetTarget()->GetAuraApplication(aurEff->GetSpellInfo()->Id, GetTarget()->GetGUID())->SendFakeAuraUpdate(WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST, true);
                }
            }

            void HandleDummyTick(AuraEffect const *aurEff)
            {
                if (GetTarget())
                {
                    if (GameObject* circle = GetTarget()->GetGameObject(GetId()))
                    {
                        // Here we check if player is in demonic circle teleport range, if so add
                        // WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST; allowing him to cast the WARLOCK_DEMONIC_CIRCLE_TELEPORT.
                        // If not in range remove the WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST.

                        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(WARLOCK_DEMONIC_CIRCLE_TELEPORT);

                        if (GetTarget()->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
                            GetTarget()->GetAuraApplication(aurEff->GetSpellInfo()->Id, GetTarget()->GetGUID())->SendFakeAuraUpdate(WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST, false);
                        else
                            GetTarget()->GetAuraApplication(aurEff->GetSpellInfo()->Id, GetTarget()->GetGUID())->SendFakeAuraUpdate(WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST, true);
                    }
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_warl_demonic_circle_summon_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_demonic_circle_summon_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_demonic_circle_summon_AuraScript();
        }
};

// Demonic Circle : Teleport - 48020 and Soulburn : Demonic Circle : Teleport - 114794
class spell_warl_demonic_circle_teleport : public SpellScriptLoader
{
    public:
        spell_warl_demonic_circle_teleport() : SpellScriptLoader("spell_warl_demonic_circle_teleport") { }

        class spell_warl_demonic_circle_teleport_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_circle_teleport_AuraScript);

            void HandleTeleport(AuraEffect const *aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* player = GetTarget()->ToPlayer())
                {
                    if (GameObject* circle = player->GetGameObject(WARLOCK_DEMONIC_CIRCLE_SUMMON))
                    {
                        player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                        player->RemoveMovementImpairingAuras();
                        player->Relocate(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());

                        if (aurEff->GetSpellInfo()->Id == WARLOCK_SOULBURN_DEMONIC_CIRCLE_TELE)
                            if (player->HasAura(WARLOCK_SOULBURN_AURA))
                                player->RemoveAurasDueToSpell(WARLOCK_SOULBURN_AURA);

                        WorldPacket data(SMSG_MOVE_UPDATE);
                        player->m_movementInfo.pos.m_positionX = player->GetPositionX();
                        player->m_movementInfo.pos.m_positionY = player->GetPositionY();
                        player->m_movementInfo.pos.m_positionZ = player->GetPositionZ();
                        player->m_movementInfo.pos.m_orientation = player->GetOrientation();
                        WorldSession::WriteMovementInfo(data, &player->m_movementInfo);
                        player->SendMessageToSet(&data, player);
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_demonic_circle_teleport_AuraScript::HandleTeleport, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_demonic_circle_teleport_AuraScript();
        }
};

class spell_warl_unstable_affliction : public SpellScriptLoader
{
    public:
        spell_warl_unstable_affliction() : SpellScriptLoader("spell_warl_unstable_affliction") { }

        class spell_warl_unstable_affliction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_unstable_affliction_AuraScript);

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const *aurEff = GetEffect(EFFECT_0))
                    {
                        int32 damage = aurEff->GetAmount() * 8;
                        // backfire damage and silence
                        caster->CastCustomSpell(dispelInfo->GetDispeller(), WARLOCK_UNSTABLE_AFFLICTION_DISPEL, &damage, NULL, NULL, true, NULL, aurEff);
                    }
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_warl_unstable_affliction_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_unstable_affliction_AuraScript();
        }
};

class spell_warl_haunt : public SpellScriptLoader
{
    public:
        spell_warl_haunt() : SpellScriptLoader("spell_warl_haunt") { }

        class spell_warl_haunt_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_haunt_AuraScript);

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster, 145159, true);
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_warl_haunt_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_haunt_AuraScript();
        }
};

// Backlash - 108563
class spell_warl_backlash : public SpellScriptLoader
{
public:
    spell_warl_backlash() : SpellScriptLoader("spell_warl_backlash") { }

    class spell_warl_backlash_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_backlash_AuraScript);

        bool CheckProc(ProcEventInfo &)
        {
            Player const * const player = GetCaster()->ToPlayer();
            if (!player)
                return false;

            return !player->HasSpellCooldown(std::numeric_limits<uint32>::max() - GetId());
        }

        void OnProc(AuraEffect const *, ProcEventInfo &)
        {
            PreventDefaultAction();
            GetCaster()->CastSpell(GetCaster(), 34936, true);
            GetCaster()->ToPlayer()->AddSpellCooldown(std::numeric_limits<uint32>::max() - GetId(), 0, 8 * IN_MILLISECONDS);
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_warl_backlash_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_warl_backlash_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_backlash_AuraScript();
    }
};

// 111397 Blood Horror
class spell_warl_blood_horror : public SpellScriptLoader
{
    enum
    {
        SPELL_BLOOD_HORROR_EFFECT       = 137143,
        HEALTH_COST_PCT                 = 5
    };

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        // HACK This spell has no entry in SpellPower.dbc (5.4.17399)
        // spellscript for power check and consumption

        SpellCastResult CheckSpec()
        {
            if (Unit *caster = GetCaster())
                if (caster->GetHealthPct() < HEALTH_COST_PCT)
                    return SPELL_FAILED_FIZZLE;

            return SPELL_CAST_OK;
        }

        void HandleOnHit()
        {
            if (Unit * caster = GetCaster())
                caster->ModifyHealth(-int32(caster->CountPctFromMaxHealth((int32)HEALTH_COST_PCT)));
        }
        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_impl::CheckSpec);
            OnHit += SpellHitFn(spell_impl::HandleOnHit);
        }
    };

public:
    spell_warl_blood_horror() : SpellScriptLoader("spell_warl_blood_horror") {}

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

// 108647 - Burning Embers
class spell_warl_burning_embers : public SpellScriptLoader
{
    enum
    {
        SPELL_CONFLAGRATE           = 17962,
        SPELL_INCINERATE            = 29722,
        SPELL_IMMOLATE              = 348,
        SPELL_FEL_FLAME             = 77799,
        SPELL_RAIN_OF_FIRE          = 42223,

        SPELL_CONFLAGRATE_FB        = 108685,
        SPELL_INCINERATE_FB         = 114654,
        SPELL_IMMOLATE_FB           = 108686,

        MAX_BURNING_EMBERS          = 4, // as of 5.4.0 17399
        EMBER_CHARGE                = 10 // 10 power points = 1 ember
    };

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);

        void OnProc(AuraEffect const *, ProcEventInfo & event)
        {
            PreventDefaultAction();

            const SpellInfo * spell = event.GetSpellInfo();
            Unit * caster = GetCaster();


            if (!spell || !caster)
                return;

            bool isCrit = event.GetHitMask() & PROC_EX_CRITICAL_HIT;
            int8 embersCnt = -1;

            switch (spell->Id)
            {
                case SPELL_CONFLAGRATE:
                case SPELL_CONFLAGRATE_FB:
                case SPELL_INCINERATE:
                case SPELL_INCINERATE_FB:
                case SPELL_FEL_FLAME:
                    embersCnt = isCrit ? 2 : 1;
                    break;
                case SPELL_RAIN_OF_FIRE:
                    if (!urand(0, 10))
                        embersCnt = 1;
                    break;
                case SPELL_IMMOLATE:
                case SPELL_IMMOLATE_FB:
                    if (isCrit)
                        embersCnt = 1;
                    break;
                default:
                    return;
            }

            if (embersCnt != -1)
            {
                caster->EnergizeBySpell(caster, spell->Id, embersCnt, POWER_BURNING_EMBERS);

                TC_LOG_DEBUG("network.opcode", "Burning Embers: generated %d embers from %u (%s)", embersCnt, spell->Id, spell->SpellName);
            }

        }

        void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit * caster = GetCaster())
            {
                caster->SetMaxPower(POWER_BURNING_EMBERS, MAX_BURNING_EMBERS * EMBER_CHARGE);
                caster->SetPower(POWER_BURNING_EMBERS, EMBER_CHARGE);
            }
        }

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit * caster = GetCaster())
                caster->SetMaxPower(POWER_BURNING_EMBERS, 0);
        }


        void Register()
        {
            OnEffectProc += AuraEffectProcFn(aura_impl::OnProc, EFFECT_0, SPELL_AURA_MOD_POWER_REGEN_PERCENT);
            OnEffectApply += AuraEffectApplyFn(aura_impl::OnApply, EFFECT_0, SPELL_AURA_MOD_POWER_REGEN_PERCENT, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(aura_impl::OnRemove, EFFECT_0, SPELL_AURA_MOD_POWER_REGEN_PERCENT, AURA_EFFECT_HANDLE_REAL);
        }
    };

public:
    spell_warl_burning_embers() : SpellScriptLoader("spell_warl_burning_embers") {}

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};

class spell_warl_backdraft final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            PYROCLASM  = 123686,
        };

        void onInitEffects(uint32 &effectMask)
        {
            auto const caster = GetCaster();
            if (!caster || !caster->HasAura(PYROCLASM))
                effectMask &= ~(1 << EFFECT_2);
        }

        void onRefreshCharges(uint8 &newCharges)
        {
            // Retail applies 3 charges first, and 3 more at refresh
            uint8 const maxCharges = GetSpellInfo()->ProcCharges * 2;
            newCharges = std::min<uint8>(maxCharges, GetCharges() + newCharges);

            // We should enable Chaos Bolt modifier again at refresh
            if (HasEffect(EFFECT_2) && GetCharges() < maxCharges / 2)
                GetEffect(EFFECT_2)->ApplySpellMod(GetCaster(), true);
        }

        void onIncinerate(AuraEffect *)
        {
            // If Pyroclasm is active and aura charges will drop below threshold,
            // Chaos Bolt can no longer benefit from Backdraft
            int32 const threshold = GetSpellInfo()->ProcCharges;
            if (HasEffect(EFFECT_2) && GetCharges() - 1 < threshold)
                GetEffect(EFFECT_2)->ApplySpellMod(GetCaster(), false);
        }

        void onChaosBolt(AuraEffect *)
        {
            // Drop 2 charges only, one more will be consumed after this hook
            int32 const threshold = GetSpellInfo()->ProcCharges;
            ModCharges(-(threshold - 1), AURA_REMOVE_BY_EXPIRE);
        }

        void Register() final
        {
            OnInitEffects += AuraInitEffectsFn(script_impl::onInitEffects);
            OnRefreshCharges += AuraRefreshChargesFn(script_impl::onRefreshCharges);
            OnEffectDropModCharge += AuraEffectDropModChargeFn(script_impl::onIncinerate, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER);
            OnEffectDropModCharge += AuraEffectDropModChargeFn(script_impl::onChaosBolt, EFFECT_2, SPELL_AURA_ADD_PCT_MODIFIER);
        }
    };

public:
    spell_warl_backdraft()
        : SpellScriptLoader("spell_warl_backdraft")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

class spell_warl_molten_core final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            CHARGES_CAP = 10
        };

        void onRefreshCharges(uint8 &newCharges)
        {
            // Caps at 10 charges, +1 at refresh
            newCharges = std::min<uint8>(CHARGES_CAP, GetCharges() + 1);
        }

        void Register() final
        {
            OnRefreshCharges += AuraRefreshChargesFn(script_impl::onRefreshCharges);
        }
    };

public:
    spell_warl_molten_core()
        : SpellScriptLoader("spell_warl_molten_core")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

class spell_warl_glyph_of_siphon_life : public SpellScriptLoader
{
public:
    spell_warl_glyph_of_siphon_life() : SpellScriptLoader("spell_warl_glyph_of_siphon_life") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Unit * caster = GetCaster())
            {
                // 0.5% (5.4.0 17399)
                int32 healing = caster->CountPctFromMaxHealth(1) * 0.5f;
                SetHitHeal(healing);
                SetHitDamage(0);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleEffect, EFFECT_0, SPELL_EFFECT_HEAL_PCT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_warl_demonic_fury : public SpellScriptLoader
{
public:
    spell_warl_demonic_fury() : SpellScriptLoader("spell_warl_demonic_fury") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            auto owner = GetCaster()->GetOwner();
            if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
                return;

            auto player = owner->ToPlayer();
            if (player->GetSpecializationId(player->GetActiveSpec()) == SPEC_WARLOCK_DEMONOLOGY)
                player->EnergizeBySpell(player, GetSpellInfo()->Id, GetSpellInfo()->Effects[EFFECT_2].BasePoints, POWER_DEMONIC_FURY);
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

class spell_warl_havoc : public SpellScriptLoader
{
    public:
        spell_warl_havoc() : SpellScriptLoader("spell_warl_havoc") { }

        class spell_warl_havoc_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_havoc_AuraScript);

            void HandleApply(const AuraEffect* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    GetAura()->SetStackAmount(GetAura()->CalcMaxStacks(caster));
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_havoc_AuraScript::HandleApply, EFFECT_1, SPELL_AURA_ADD_FLAT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_havoc_AuraScript();
        }
};

class spell_warl_soul_link : public SpellScriptLoader
{
    public:
        spell_warl_soul_link() : SpellScriptLoader("spell_warl_soul_link") { }

        class spell_warl_soul_link_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_link_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (Pet* pet = caster->ToPlayer()->GetPet())
                    pet->CastSpell(pet, 108446, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_soul_link_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soul_link_SpellScript();
        }
};

class spell_warl_ember_consumers : public SpellScriptLoader
{
    public:
        spell_warl_ember_consumers() : SpellScriptLoader("spell_warl_ember_consumers") { }

        class spell_warl_ember_consumers_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_ember_consumers_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                int32 damage = GetHitDamage();
                switch (GetSpellInfo()->Id)
                {
                    case 108686:
                    case 108685:
                    case 114654:
                        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(108683))
                            damage = CalculatePct(damage, info->Effects[EFFECT_5].CalcValue());
                        break;
                }

                if (AuraEffect* mastery = caster->GetAuraEffect(77220, EFFECT_0))
                {
                    AddPct(damage, mastery->GetFloatAmount());
                    SetHitDamage(damage);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_ember_consumers_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_ember_consumers_SpellScript();
        }
};

class spell_warl_glyph_of_shadow_bolt : public SpellScriptLoader
{
    public:
        spell_warl_glyph_of_shadow_bolt() : SpellScriptLoader("spell_warl_glyph_of_shadow_bolt") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void HandleApply(const AuraEffect* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    caster->learnSpell(aurEff->GetAmount(), false);
            }

            void HandleRemove(const AuraEffect* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    caster->removeSpell(aurEff->GetAmount());
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(script_impl::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(script_impl::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new script_impl();
        }
};

enum demonHunting
{
    SPELL_WARLOCK_DARK_APOTHESIS = 114168
};

class spell_warl_glyph_of_demon_hunting : public SpellScriptLoader
{
    public:
        spell_warl_glyph_of_demon_hunting() : SpellScriptLoader("spell_warl_glyph_of_demon_hunting") { }

        class script_impl : public AuraScript
        {
            PrepareAuraScript(script_impl);

            void HandleApply(const AuraEffect* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    caster->learnSpell(SPELL_WARLOCK_DARK_APOTHESIS, false);
            }

            void HandleRemove(const AuraEffect* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    caster->removeSpell(SPELL_WARLOCK_DARK_APOTHESIS);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(script_impl::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(script_impl::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new script_impl();
        }
};

enum emberTap
{
    SPELL_WARLOCK_EMBER_TAP_GLYPH_PASSIVE = 63304
};

class spell_warl_glyph_of_ember_tap : public SpellScriptLoader
{
public:
    spell_warl_glyph_of_ember_tap() : SpellScriptLoader("spell_warl_glyph_of_ember_tap") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleEffect(SpellEffIndex effIndex)
        {
            if (GetCaster()->HasAura(SPELL_WARLOCK_EMBER_TAP_GLYPH_PASSIVE))
            {
                PreventHitDefaultEffect(effIndex);
                return;
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleEffect, EFFECT_0, SPELL_EFFECT_HEAL_PCT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_warl_glyph_of_nightmares : public SpellScriptLoader
{
public:
    spell_warl_glyph_of_nightmares() : SpellScriptLoader("spell_warl_glyph_of_nightmares") {}

    enum
    {
        SPELL_WARL_GLYPH_OF_NIGHTMARES_PASSIVE = 143314
    };

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void HandleApply(const AuraEffect* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Player* caster = GetCaster()->ToPlayer())
                caster->AddAura(SPELL_WARL_GLYPH_OF_NIGHTMARES_PASSIVE, caster);
        }

        void HandleRemove(const AuraEffect* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Player* caster = GetCaster()->ToPlayer())
                caster->RemoveAurasDueToSpell(SPELL_WARL_GLYPH_OF_NIGHTMARES_PASSIVE);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(script_impl::HandleApply, EFFECT_0, SPELL_AURA_MOUNTED, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(script_impl::HandleRemove, EFFECT_0, SPELL_AURA_MOUNTED, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new script_impl();
    }
};

// Dark Intent - 109773
class spell_warl_dark_intent : public SpellScriptLoader
{
public:
    spell_warl_dark_intent() : SpellScriptLoader("spell_warl_dark_intent") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleEffect(SpellEffIndex effIndex)
        {
            if (GetHitUnit()->GetTypeId() != TYPEID_PLAYER || GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Player* caster = GetCaster()->ToPlayer())
            {
                if (Player* target = GetHitUnit()->ToPlayer())
                {
                    if (target->IsInPartyWith(caster))
                    {
                        std::list<Unit*> memberList;
                        Player* plr = caster->ToPlayer();
                        plr->GetPartyMembers(memberList);

                        for (auto itr : memberList)
                            caster->AddAura(GetSpellInfo()->Id, (itr));
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleEffect, EFFECT_0, SPELL_AURA_MOD_SPELL_POWER_PCT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

void AddSC_warlock_spell_scripts()
{
    new spell_warl_health_funnel();
    new spell_warl_soulburn_seed_of_corruption_damage();
    new spell_warl_soulburn_seed_of_corruption();
    new spell_warl_soulburn_remove();
    new spell_warl_soulburn_override();
    new spell_warl_glyph_of_soulwell();
    new spell_warl_imp_swarm();
    new spell_warl_glyph_of_imp_swarm();
    new spell_warl_unbound_will();
    new spell_warl_kil_jaedens_cunning();
    new spell_warl_shield_of_shadow();
    new spell_warl_agony();
    new spell_warl_grimoire_of_sacrifice();
    new spell_warl_flames_of_xoroth();
    new spell_warl_soul_link();
    new spell_warl_molten_core_dot();
    new spell_warl_decimate();
    new spell_warl_demonic_call();
    new spell_warl_void_ray();
    new spell_warl_chaos_wave();
    new spell_warl_metamorphosis_cost();
    new spell_warl_immolation_aura();
    new spell_warl_dark_bargain_on_absorb();
    new spell_warl_dark_regeneration();
    new spell_warl_soul_leech();
    new spell_warl_sacrificial_pact();
    new spell_warl_hand_of_guldan();
    new spell_warl_twilight_ward_s12();
    new spell_warl_hellfire();
    new spell_warl_demonic_leap_jump();
    new spell_warl_demonic_leap();
    new spell_warl_burning_rush();
    new spell_warl_soul_swap();
    new spell_warl_soul_swap_dot_marker();
    new spell_warl_soul_swap_exhale();
    new spell_warl_soul_swap_override();
    new spell_warl_soul_swap_soulburn();
    new spell_warl_nightfall();
    new spell_warl_drain_soul();
    new spell_warl_demonic_gateway();
    new spell_warl_rain_of_fire();
    new spell_warl_fire_and_brimstone();
    new spell_warl_conflagrate_aura();
    new spell_warl_shadowburn();
    new spell_warl_burning_embers();
    new spell_warl_drain_life();
    new spell_warl_soul_harverst();
    new spell_warl_life_tap();
    new spell_warl_fear();
    new spell_warl_banish();
    new spell_warl_create_healthstone();
    new spell_warl_seed_of_corruption();
    new spell_warl_soulshatter();
    new spell_warl_demonic_circle_summon();
    new spell_warl_demonic_circle_teleport();
    new spell_warl_unstable_affliction();
    new spell_warl_backlash();
    new spell_warl_blood_horror();
    new spell_warl_backdraft();
    new spell_warl_molten_core();
    new spell_warl_glyph_of_siphon_life();
    new spell_warl_demonic_fury();
    new spell_warl_havoc();
    new spell_warl_metamorphosis_overrides();
    new spell_warl_haunt();
    new spell_warl_ember_consumers();
    new spell_warl_singe_magic();
    new spell_warl_glyph_of_shadow_bolt();
    new spell_warl_glyph_of_demon_hunting();
    new spell_warl_glyph_of_ember_tap();
    new spell_warl_glyph_of_nightmares();
    new spell_warl_dark_intent();
}
