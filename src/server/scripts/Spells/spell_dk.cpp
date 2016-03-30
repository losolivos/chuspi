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
 * Scripts for spells with SPELLFAMILY_DEATHKNIGHT and SPELLFAMILY_GENERIC spells used by deathknight players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dk_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum DeathKnightSpells
{
    DK_SPELL_RUNIC_POWER_ENERGIZE               = 49088,
    DK_SPELL_ANTI_MAGIC_SHELL_TALENT            = 51052,
    DK_SPELL_CORPSE_EXPLOSION_TRIGGERED         = 43999,
    DK_SPELL_CORPSE_EXPLOSION_VISUAL            = 51270,
    DK_SPELL_GHOUL_EXPLODE                      = 47496,
    DK_SPELL_SCOURGE_STRIKE_TRIGGERED           = 70890,
    DK_SPELL_BLOOD_BOIL_TRIGGERED               = 65658,
    SPELL_DK_ITEM_T8_MELEE_4P_BONUS             = 64736,
    DK_SPELL_BLACK_ICE_R1                       = 49140,
    DK_SPELL_BLOOD_PLAGUE                       = 55078,
    DK_SPELL_FROST_FEVER                        = 55095,
    DK_SPELL_ROILING_BLOOD                      = 108170,
    DK_SPELL_PESTILENCE                         = 50842,
    DK_SPELL_CHILBLAINS                         = 50041,
    DK_SPELL_CHAINS_OF_ICE_ROOT                 = 53534,
    DK_SPELL_PLAGUE_LEECH                       = 123693,
    DK_SPELL_PERDITION                          = 123981,
    DK_SPELL_SHROUD_OF_PURGATORY                = 116888,
    DK_SPELL_PURGATORY_INSTAKILL                = 123982,
    DK_SPELL_BLOOD_RITES                        = 50034,
    DK_SPELL_DEATH_SIPHON_HEAL                  = 116783,
    DK_SPELL_BLOOD_CHARGE                       = 114851,
    DK_SPELL_PILLAR_OF_FROST                    = 51271,
    DK_SPELL_SOUL_REAPER_HASTE                  = 114868,
    DK_SPELL_SOUL_REAPER_DAMAGE                 = 114867,
    DK_SPELL_REMORSELESS_WINTER_STUN            = 115001,
    DK_SPELL_REMORSELESS_WINTER                 = 115000,
    DK_SPELL_CONVERSION                         = 119975,
    DK_SPELL_WEAKENED_BLOWS                     = 115798,
    DK_SPELL_SCARLET_FEVER                      = 81132,
    DK_SPELL_SCENT_OF_BLOOD_AURA                = 50421,
    DK_SPELL_CHAINS_OF_ICE                      = 45524,
    DK_SPELL_EBON_PLAGUEBRINGER                 = 51160,
    DK_SPELL_DESECRATED_GROUND                  = 118009,
    DK_SPELL_DESECRATED_GROUND_IMMUNE           = 115018,
    DK_SPELL_ASPHYXIATE                         = 108194,
    DK_SPELL_DARK_INFUSION_STACKS               = 91342,
    DK_SPELL_DARK_INFUSION_AURA                 = 93426,
    DK_NPC_WILD_MUSHROOM                        = 59172,
    DK_SPELL_RUNIC_CORRUPTION_REGEN             = 51460,
    DK_SPELL_RUNIC_EMPOWERMENT                  = 81229,
    DK_SPELL_GOREFIENDS_GRASP_GRIP_VISUAL       = 114869,
    DK_SPELL_DEATH_GRIP_ONLY_JUMP               = 49575,
    DK_SPELL_GLYPH_OF_CORPSE_EXPLOSION          = 127344,
    DK_SPELL_GLYPH_OF_HORN_OF_WINTER            = 58680,
	DK_SPELL_GLYPH_OF_DEATH_AND_DECAY           = 58629,
	DK_SPELL_DEATH_AND_DECAY_DECREASE_SPEED     = 143375,
    DK_SPELL_GLYPH_OF_HORN_OF_WINTER_EFFECT     = 121920
};

// Death and Decay - 43265
class spell_dk_death_and_decay : public SpellScriptLoader
{
    public:
        spell_dk_death_and_decay() : SpellScriptLoader("spell_dk_death_and_decay") { }

        class spell_dk_death_and_decay_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_and_decay_SpellScript);

            void HandleAfterCast()
            {
                WorldLocation* dest = const_cast<WorldLocation*>(GetExplTargetDest());
                if (dest && GetCaster()->HasAura(DK_SPELL_GLYPH_OF_DEATH_AND_DECAY))
                    GetCaster()->CastSpell(dest->GetPositionX(), dest->GetPositionY(), dest->GetPositionZ(), DK_SPELL_DEATH_AND_DECAY_DECREASE_SPEED, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_dk_death_and_decay_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_and_decay_SpellScript();
        }
};

// Gorefiend's Grasp - 108199
class spell_dk_gorefiends_grasp : public SpellScriptLoader
{
    public:
        spell_dk_gorefiends_grasp() : SpellScriptLoader("spell_dk_gorefiends_grasp") { }

        class spell_dk_gorefiends_grasp_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_gorefiends_grasp_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        std::list<Unit*> tempList;
                        std::list<Unit*> gripList;

                        target->GetAttackableUnitListInRange(tempList, 20.0f);

                        for (auto itr : tempList)
                        {
                            if (itr->GetGUID() == target->GetGUID())
                                continue;

                            if (!_player->IsValidAttackTarget(itr))
                                continue;

                            if (!itr->IsWithinLOSInMap(target))
                                continue;

                            gripList.push_back(itr);
                        }

                        for (auto itr : gripList)
                        {
                            itr->CastSpell(target, DK_SPELL_DEATH_GRIP_ONLY_JUMP, true);
                            target->CastSpell(itr, DK_SPELL_GOREFIENDS_GRASP_GRIP_VISUAL, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_gorefiends_grasp_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_gorefiends_grasp_SpellScript();
        }
};

// Called by Death Coil (damage) - 47632, Frost Strike - 49143 and Runic Strike - 56815
// Runic Empowerment - 81229
class spell_dk_runic_empowerment : public SpellScriptLoader
{
    public:
        spell_dk_runic_empowerment() : SpellScriptLoader("spell_dk_runic_empowerment") { }

        class spell_dk_runic_empowerment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_runic_empowerment_SpellScript);

            void HandleOnHit()
            {
                Player * const player = GetCaster()->ToPlayer();
                if (!player || !player->HasAura(DK_SPELL_RUNIC_EMPOWERMENT))
                    return;

                if (!roll_chance_i(45))
                    return;

                auto const baseCooldown = player->GetRuneBaseCooldown();

                uint8 runes[MAX_RUNES];
                uint8 *runesEnd = runes;

                for (uint8 i = 0; i < MAX_RUNES; ++i)
                    if (player->GetRuneCooldown(i) == baseCooldown)
                        *runesEnd++ = i;

                if (auto const count = std::distance(runes, runesEnd))
                {
                    uint8 const *itr = runes;
                    std::advance(itr, irand(0, count - 1));

                    player->SetRuneCooldown(*itr, 0);
                    player->ResyncRunes(MAX_RUNES);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_runic_empowerment_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_runic_empowerment_SpellScript();
        }
};

// Called by Death Coil (damage) - 47632, Frost Strike - 49143 and Runic Strike - 56815
// Runic Corruption - 51462
class spell_dk_runic_corruption : public SpellScriptLoader
{
    public:
        spell_dk_runic_corruption() : SpellScriptLoader("spell_dk_runic_corruption") { }

        class spell_dk_runic_corruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_runic_corruption_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (AuraEffect const *runicCorruption = _player->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 4068, 0))
                    {
                        if (roll_chance_i(45))
                        {
                            int32 basepoints0 = runicCorruption->GetAmount();
                            if (Aura *aur = _player->GetAura(DK_SPELL_RUNIC_CORRUPTION_REGEN))
                                aur->SetDuration(aur->GetDuration() + 3000);
                            else
                                _player->CastCustomSpell(_player, DK_SPELL_RUNIC_CORRUPTION_REGEN, &basepoints0, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_runic_corruption_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_runic_corruption_SpellScript();
        }
};

// Might of Ursoc - 113072
class spell_dk_might_of_ursoc : public SpellScriptLoader
{
    public:
        spell_dk_might_of_ursoc() : SpellScriptLoader("spell_dk_might_of_ursoc") { }

        class spell_dk_might_of_ursoc_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_might_of_ursoc_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetHealthPct() < 15.0f)
                        caster->SetHealth(caster->CountPctFromMaxHealth(15));
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dk_might_of_ursoc_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_might_of_ursoc_AuraScript();
        }
};

// Wild Mushroom : Plague - 113517
class spell_dk_wild_mushroom_plague : public SpellScriptLoader
{
    public:
        spell_dk_wild_mushroom_plague() : SpellScriptLoader("spell_dk_wild_mushroom_plague") { }

        class spell_dk_wild_mushroom_plague_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_wild_mushroom_plague_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (!GetCaster())
                    return;

                std::list<Creature*> tempList;
                std::list<Creature*> mushroomlist;
                std::list<Unit*> tempUnitList;
                std::list<Unit*> targetList;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    _player->GetCreatureListWithEntryInGrid(tempList, DK_NPC_WILD_MUSHROOM, 500.0f);

                    for (auto itr : tempList)
                        mushroomlist.push_back(itr);

                    // Remove other players mushrooms
                    for (std::list<Creature*>::iterator i = tempList.begin(); i != tempList.end(); ++i)
                    {
                        Unit* owner = (*i)->GetOwner();
                        if (owner && owner == _player && (*i)->IsSummon())
                            continue;

                        mushroomlist.remove((*i));
                    }

                    if (!mushroomlist.empty())
                    {
                        for (auto itr : mushroomlist)
                        {
                            itr->GetAttackableUnitListInRange(tempUnitList, 10.0f);

                            for (auto itr2 : tempUnitList)
                            {
                                if (itr2->GetGUID() == _player->GetGUID())
                                    continue;

                                if (itr2->GetGUID() == itr->GetGUID())
                                    continue;

                                if (!_player->IsValidAttackTarget(itr2))
                                    continue;

                                targetList.push_back(itr2);
                            }

                            for (auto itr2 : targetList)
                            {
                                _player->CastSpell(itr2, DK_SPELL_BLOOD_PLAGUE, true);
                                _player->CastSpell(itr2, DK_SPELL_FROST_FEVER, true);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_wild_mushroom_plague_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_wild_mushroom_plague_AuraScript();
        }
};

// Dark transformation - transform pet spell - 63560
class spell_dk_dark_transformation_form : public SpellScriptLoader
{
    public:
        spell_dk_dark_transformation_form() : SpellScriptLoader("spell_dk_dark_transformation_form") { }

        class spell_dk_dark_transformation_form_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_dark_transformation_form_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* pet = GetHitUnit())
                    {
                        if (pet->HasAura(DK_SPELL_DARK_INFUSION_STACKS))
                        {
                            _player->RemoveAura(DK_SPELL_DARK_INFUSION_STACKS);
                            pet->RemoveAura(DK_SPELL_DARK_INFUSION_STACKS);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_dark_transformation_form_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_dark_transformation_form_SpellScript();
        }
};

// Desecrated ground - 118009
class spell_dk_desecrated_ground : public SpellScriptLoader
{
    public:
        spell_dk_desecrated_ground() : SpellScriptLoader("spell_dk_desecrated_ground") { }

        class spell_dk_desecrated_ground_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_desecrated_ground_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (auto caster = GetCaster())
                    if (DynamicObject* dynObj = caster->GetDynObject(DK_SPELL_DESECRATED_GROUND))
                        if (caster->GetDistance(dynObj) <= 8.0f)
                            caster->CastSpell(caster, DK_SPELL_DESECRATED_GROUND_IMMUNE, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_desecrated_ground_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_desecrated_ground_AuraScript();
        }
};

// Necrotic Strike - 73975
class spell_dk_necrotic_strike : public SpellScriptLoader
{
    public:
        spell_dk_necrotic_strike() : SpellScriptLoader("spell_dk_necrotic_strike") { }

        class spell_dk_necrotic_strike_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_necrotic_strike_AuraScript);

            void CalculateAmount(AuraEffect const *aurEff, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (auto caster = GetCaster())
                {
                    int32 absorbAmount = caster->GetTotalAttackPowerValue(BASE_ATTACK) * 2.25f;
                    if (Unit* target = GetUnitOwner())
                        caster->ApplyResilience(target, &absorbAmount);
                    if (AuraEffect* necrotic = GetUnitOwner()->GetAuraEffect(73975, EFFECT_0, caster->GetGUID()))
                        absorbAmount += necrotic->GetAmount();
                    amount = int32(absorbAmount);
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_necrotic_strike_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_necrotic_strike_AuraScript();
        }
};

// Festering Strike - 85948
class spell_dk_festering_strike : public SpellScriptLoader
{
    public:
        spell_dk_festering_strike() : SpellScriptLoader("spell_dk_festering_strike") { }

        class spell_dk_festering_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_festering_strike_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (Aura *BP = target->GetAura(DK_SPELL_BLOOD_PLAGUE, _player->GetGUID()))
                        {
                            uint32 dur = BP->GetDuration() + 6000;
                            BP->SetDuration(dur);

                            if (dur > uint32(BP->GetMaxDuration()))
                                BP->SetMaxDuration(dur);

                            if (auto const PV = target->GetAura(81326, _player->GetGUID()))
                            {
                                PV->SetDuration(dur);
                                if (dur > uint32(PV->GetMaxDuration()))
                                    PV->SetMaxDuration(dur);
                            }
                        }
                        if (Aura *FF = target->GetAura(DK_SPELL_FROST_FEVER, _player->GetGUID()))
                        {
                            uint32 dur = FF->GetDuration() + 6000;
                            FF->SetDuration(dur);

                            if (dur > uint32(FF->GetMaxDuration()))
                                FF->SetMaxDuration(dur);
                        }
                        if (Aura *COI = target->GetAura(DK_SPELL_CHAINS_OF_ICE, _player->GetGUID()))
                        {
                            uint32 dur = COI->GetDuration() + 6000;
                            COI->SetDuration(dur);

                            if (dur > uint32(COI->GetMaxDuration()))
                                COI->SetMaxDuration(dur);
                        }
                    }
                }
            }
            void Register()
            {
                OnHit += SpellHitFn(spell_dk_festering_strike_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_festering_strike_SpellScript();
        }
};

// Death Strike heal - 45470
class spell_dk_death_strike_heal : public SpellScriptLoader
{
    public:
        spell_dk_death_strike_heal() : SpellScriptLoader("spell_dk_death_strike_heal") { }

        class spell_dk_death_strike_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_strike_heal_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        if (Aura *scentOfBlood = player->GetAura(DK_SPELL_SCENT_OF_BLOOD_AURA))
                        {
                            uint8 chg = scentOfBlood->GetStackAmount();
                            uint32 hl = GetHitHeal() * 0.2 * chg;
                            SetHitHeal(GetHitHeal() + hl);
                        }

                        player->RemoveAura(DK_SPELL_SCENT_OF_BLOOD_AURA);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_death_strike_heal_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_strike_heal_SpellScript();
        }
};

// Howling Blast - 49184
class spell_dk_howling_blast : public SpellScriptLoader
{
    public:
        spell_dk_howling_blast() : SpellScriptLoader("spell_dk_howling_blast") { }

        class spell_dk_howling_blast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_howling_blast_SpellScript);

            uint64 tar;

            void HandleBeforeCast()
            {
                Unit* target = GetExplTargetUnit();
                Unit* caster = GetCaster();

                if (!caster || !target)
                    return;

                tar = target->GetGUID();
            }

            void HandleOnHit()
            {
                Unit* target = GetHitUnit();
                Unit* caster = GetCaster();

                if (!caster || !target || !tar)
                    return;

                if (target->GetGUID() != tar)
                    SetHitDamage(GetHitDamage() / 2);

                caster->CastSpell(target, DK_SPELL_FROST_FEVER, true);
            }

            void Register()
            {
                BeforeCast += SpellCastFn(spell_dk_howling_blast_SpellScript::HandleBeforeCast);
                OnHit += SpellHitFn(spell_dk_howling_blast_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_howling_blast_SpellScript();
        }
};

// Conversion - 119975
class spell_dk_conversion : public SpellScriptLoader
{
    public:
        spell_dk_conversion() : SpellScriptLoader("spell_dk_conversion") { }

        class spell_dk_conversion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_conversion_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                {
                    // Drain 5 runic power to regen 3% of max health per second
                    int32 runicPower = GetCaster()->GetPower(POWER_RUNIC_POWER);

                    if (runicPower > 50)
                        GetCaster()->SetPower(POWER_RUNIC_POWER, GetCaster()->GetPower(POWER_RUNIC_POWER) - 50);
                    else if (runicPower > 0)
                    {
                        GetCaster()->SetPower(POWER_RUNIC_POWER, 0);
                        GetCaster()->RemoveAura(DK_SPELL_CONVERSION);
                    }
                    else if (runicPower == 0)
                        GetCaster()->RemoveAura(DK_SPELL_CONVERSION);
                 }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_conversion_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_conversion_AuraScript();
        }
};

// Remorseless Winter - 115000
class spell_dk_remorseless_winter : public SpellScriptLoader
{
    public:
        spell_dk_remorseless_winter() : SpellScriptLoader("spell_dk_remorseless_winter") { }

        class spell_dk_remorseless_winter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_remorseless_winter_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (Aura *remorselessWinter = target->GetAura(DK_SPELL_REMORSELESS_WINTER))
                            if (remorselessWinter->GetStackAmount() == 5 && !target->HasAura(DK_SPELL_REMORSELESS_WINTER_STUN))
                                _player->CastSpell(target, DK_SPELL_REMORSELESS_WINTER_STUN, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_remorseless_winter_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_remorseless_winter_SpellScript();
        }
};

// Soul Reaper - 130736 (unholy) or 130735 (frost) or 114866 (blood)
class spell_dk_soul_reaper : public SpellScriptLoader
{
    public:
        spell_dk_soul_reaper() : SpellScriptLoader("spell_dk_soul_reaper") { }

        class spell_dk_soul_reaper_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_soul_reaper_AuraScript);

            void HandleRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_DEATH)
                        GetCaster()->CastSpell(GetCaster(), DK_SPELL_SOUL_REAPER_HASTE, true);
                    else if (removeMode == AURA_REMOVE_BY_EXPIRE && GetTarget()->GetHealthPct() < 35.0f)
                        GetCaster()->CastSpell(GetTarget(), DK_SPELL_SOUL_REAPER_DAMAGE, true);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_dk_soul_reaper_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_soul_reaper_AuraScript();
        }
};

// Pillar of Frost - 51271
class spell_dk_pillar_of_frost : public SpellScriptLoader
{
    public:
        spell_dk_pillar_of_frost() : SpellScriptLoader("spell_dk_pillar_of_frost") { }

        class spell_dk_pillar_of_frost_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_pillar_of_frost_AuraScript);

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->ApplySpellImmune(DK_SPELL_PILLAR_OF_FROST, IMMUNITY_MECHANIC, MECHANIC_KNOCKOUT, false);
            }

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->ApplySpellImmune(DK_SPELL_PILLAR_OF_FROST, IMMUNITY_MECHANIC, MECHANIC_KNOCKOUT, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dk_pillar_of_frost_AuraScript::OnApply, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_dk_pillar_of_frost_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_pillar_of_frost_AuraScript();
        }
};

// Called by Death Coil - 47541, Rune Strike - 56815 and Frost Strike - 49143
// Blood Charges - 114851 for Blood Tap - 45529
class spell_dk_blood_charges : public SpellScriptLoader
{
    public:
        spell_dk_blood_charges() : SpellScriptLoader("spell_dk_blood_charges") { }

        class spell_dk_blood_charges_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_blood_charges_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        if (player->HasSpell(45529))
                        {
                            player->CastSpell(player, DK_SPELL_BLOOD_CHARGE, true);
                            player->CastSpell(player, DK_SPELL_BLOOD_CHARGE, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_blood_charges_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_blood_charges_SpellScript();
        }
};

// Blood Tap - 45529
class spell_dk_blood_tap : public SpellScriptLoader
{
    public:
        spell_dk_blood_tap() : SpellScriptLoader("spell_dk_blood_tap") { }

        class spell_dk_blood_tap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_blood_tap_SpellScript);

            SpellCastResult CheckBloodCharges()
            {
                if (GetCaster()->ToPlayer())
                {
                    if (Aura *bloodCharges = GetCaster()->ToPlayer()->GetAura(DK_SPELL_BLOOD_CHARGE))
                    {
                        if (bloodCharges->GetStackAmount() < 5)
                            return SPELL_FAILED_DONT_REPORT;
                    }
                    else
                        return SPELL_FAILED_DONT_REPORT;

                    bool cooldown = false;

                    for (uint8 i = 0; i < MAX_RUNES; ++i)
                    {
                        if (GetCaster()->ToPlayer()->GetCurrentRune(i) == RUNE_DEATH || !GetCaster()->ToPlayer()->GetRuneCooldown(i))
                            continue;

                        cooldown = true;
                    }

                    if (!cooldown)
                        return SPELL_FAILED_DONT_REPORT;
                }

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        if (Aura *bloodCharges = player->GetAura(DK_SPELL_BLOOD_CHARGE))
                        {
                            int32 newAmount = bloodCharges->GetStackAmount();

                            if ((newAmount - 5) <= 0)
                                player->RemoveAura(DK_SPELL_BLOOD_CHARGE);
                            else
                                bloodCharges->SetStackAmount(newAmount - 5);
                        }
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_blood_tap_SpellScript::CheckBloodCharges);
                OnHit += SpellHitFn(spell_dk_blood_tap_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_blood_tap_SpellScript();
        }
};

// Death Siphon - 108196
class spell_dk_death_siphon : public SpellScriptLoader
{
    public:
        spell_dk_death_siphon() : SpellScriptLoader("spell_dk_death_siphon") { }

        class spell_dk_death_siphon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_siphon_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        int32 bp = CalculatePct(GetHitDamage(), GetSpellInfo()->Effects[EFFECT_1].BasePoints);

                        player->CastCustomSpell(player, DK_SPELL_DEATH_SIPHON_HEAL, &bp, NULL, NULL, true);

                        uint32 const cooldown = player->GetRuneBaseCooldown();

                        for (uint32 i = 0; i < MAX_RUNES; ++i)
                        {
                            RuneType rune = player->GetCurrentRune(i);

                            if (!player->GetRuneCooldown(i) && rune == RUNE_DEATH)
                            {
                                player->SetRuneCooldown(i, cooldown);

                                bool takePower = true;
                                if (uint32 spell = player->GetRuneConvertSpell(i))
                                    takePower = spell != 54637;

                                // keep Death Rune type if player has Blood of the North
                                if (takePower)
                                {
                                    player->RestoreBaseRune(i);
                                    player->SetDeathRuneUsed(i, true);
                                }

                                break;
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_siphon_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_siphon_SpellScript();
        }
};

// Death Strike - 49998
class spell_dk_death_strike : public SpellScriptLoader
{
    public:
        spell_dk_death_strike() : SpellScriptLoader("spell_dk_death_strike") { }

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                int32 bp = 0;
                auto caster = GetCaster();
                if ((caster->CountPctFromMaxHealth(7)) > (20 * caster->GetDamageTakenInPastSecs(5) / 100))
                    bp = caster->CountPctFromMaxHealth(7);
                else
                    bp = (20 * caster->GetDamageTakenInPastSecs(5) / 100);

                // Item - Death Knight T14 Blood 4P bonus
                if (caster->HasAura(123080))
                    bp *= 1.1f;

                // Glyph of Dark Succor
                if (AuraEffect const *aurEff = caster->GetAuraEffect(101568, 0))
                    if (bp < int32(caster->CountPctFromMaxHealth(aurEff->GetAmount())))
                        if (caster->HasAura(48265) || caster->HasAura(48266)) // Only in frost/unholy presence
                            bp = caster->CountPctFromMaxHealth(aurEff->GetAmount());

                caster->CastCustomSpell(caster, 45470, &bp, NULL, NULL, false);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(script_impl::HandleDummy, EFFECT_2, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl();
        }
};

// Purgatory - 116888
class spell_dk_purgatory : public SpellScriptLoader
{
    public:
        spell_dk_purgatory() : SpellScriptLoader("spell_dk_purgatory") { }

        class spell_dk_purgatory_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_purgatory_AuraScript);

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_EXPIRE)
                        _player->CastSpell(_player, DK_SPELL_PURGATORY_INSTAKILL, true);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_dk_purgatory_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_purgatory_AuraScript();
        }
};

// Purgatory - 114556
class spell_dk_purgatory_absorb : public SpellScriptLoader
{
    public:
        spell_dk_purgatory_absorb() : SpellScriptLoader("spell_dk_purgatory_absorb") { }

        class spell_dk_purgatory_absorb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_purgatory_absorb_AuraScript);

            void CalculateAmount(AuraEffect const * /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect * /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                Unit* target = GetTarget();

                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                // No damage received under Shroud of Purgatory
                if (target->ToPlayer()->HasAura(DK_SPELL_SHROUD_OF_PURGATORY))
                {
                    absorbAmount = dmgInfo.GetDamage();
                    return;
                }

                if (target->ToPlayer()->HasAura(DK_SPELL_PERDITION))
                    return;

                int32 bp = dmgInfo.GetDamage();

                target->CastCustomSpell(target, DK_SPELL_SHROUD_OF_PURGATORY, &bp, NULL, NULL, true);
                target->CastSpell(target, DK_SPELL_PERDITION, true);
                target->SetHealth(1);
                absorbAmount = dmgInfo.GetDamage();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_purgatory_absorb_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_purgatory_absorb_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_purgatory_absorb_AuraScript();
        }
};

// Plague Leech - 123693
class spell_dk_plague_leech : public SpellScriptLoader
{
    public:
        spell_dk_plague_leech() : SpellScriptLoader("spell_dk_plague_leech") { }

        class spell_dk_plague_leech_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_plague_leech_SpellScript);

            SpellCastResult CheckRunes()
            {
                if (Unit* target = GetExplTargetUnit())
                {
                    uint8 diseases = target->GetDiseasesByCaster(GetCaster()->GetGUID());
                    uint8 requiredDiseases = 2;
                    if (diseases < requiredDiseases)
                    {
                        SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_BOTH_FROST_FEVER_BLOOD_PLAGUE);
                        return SPELL_FAILED_CUSTOM_ERROR;
                    }
                }

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                Player* player = GetCaster()->ToPlayer();
                if (!player)
                    return;

                std::vector<uint8> runes;
                for (uint8 i = 0; i < MAX_RUNES; ++i)
                    if (player->GetRuneCooldown(i) == player->GetRuneBaseCooldown())
                        runes.push_back(i);

                if (!runes.empty())
                {
                    for (uint8 i = 0; i < 2 && !runes.empty(); i++)
                    {
                        std::vector<uint8>::iterator itr = runes.begin();
                        std::advance(itr, urand(0, runes.size() - 1));
                        player->SetRuneCooldown((*itr), 0);
                        player->ConvertRune((*itr), RUNE_DEATH);
                        runes.erase(itr);
                    }

                    GetHitUnit()->GetDiseasesByCaster(GetCaster()->GetGUID(), true);
                    player->ResyncRunes(MAX_RUNES);
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_plague_leech_SpellScript::CheckRunes);
                OnHit += SpellHitFn(spell_dk_plague_leech_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_plague_leech_SpellScript();
        }
};

// Unholy Blight - 115994
class spell_dk_unholy_blight : public SpellScriptLoader
{
    public:
        spell_dk_unholy_blight() : SpellScriptLoader("spell_dk_unholy_blight") { }

        class spell_dk_unholy_blight_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_unholy_blight_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        _player->CastSpell(target, DK_SPELL_BLOOD_PLAGUE, true);
                        _player->CastSpell(target, DK_SPELL_FROST_FEVER, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_unholy_blight_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_unholy_blight_SpellScript();
        }
};

// Called by Chains of Ice - 45524
// Chilblains - 50041
class spell_dk_chilblains : public SpellScriptLoader
{
    public:
        spell_dk_chilblains() : SpellScriptLoader("spell_dk_chilblains") { }

        class spell_dk_chilblains_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_chilblains_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(DK_SPELL_CHILBLAINS))
                            _player->CastSpell(target, DK_SPELL_CHAINS_OF_ICE_ROOT, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_chilblains_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_chilblains_SpellScript();
        }
};

// Outbreak - 77575
class spell_dk_outbreak : public SpellScriptLoader
{
    public:
        spell_dk_outbreak() : SpellScriptLoader("spell_dk_outbreak") { }

        class spell_dk_outbreak_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_outbreak_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        _player->CastSpell(target, DK_SPELL_BLOOD_PLAGUE, true);
                        _player->CastSpell(target, DK_SPELL_FROST_FEVER, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_outbreak_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_outbreak_SpellScript();
        }
};

// 46584 - Raise Dead
class spell_dk_raise_dead : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl)

            enum
        {
            MASTER_OF_GHOULS = 52143
        };

        bool Load()
        {
            Unit * const caster = GetCaster();
            return caster && caster->GetTypeId() == TYPEID_PLAYER;
        }

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitEffect(effIndex);

            Unit * const caster = GetCaster();

            effIndex = caster->HasAura(MASTER_OF_GHOULS) ? EFFECT_1 : EFFECT_0;
            uint32 const spellId = GetSpellInfo()->Effects[effIndex].BasePoints;

            caster->ToPlayer()->RemoveSpellCooldown(spellId);
            caster->CastSpell(caster, spellId, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

public:
    spell_dk_raise_dead()
        : SpellScriptLoader("spell_dk_raise_dead")
    { }

    SpellScript * GetSpellScript() const
    {
        return new script_impl;
    }
};

// 50462 - Anti-Magic Shell (on raid member)
class spell_dk_anti_magic_shell_raid : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_shell_raid() : SpellScriptLoader("spell_dk_anti_magic_shell_raid") { }

        class spell_dk_anti_magic_shell_raid_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_shell_raid_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return true;
            }

            void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // TODO: this should absorb limited amount of damage, but no info on calculation formula
                amount = -1;
            }

            void Absorb(AuraEffect * /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                 absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_raid_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_raid_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_shell_raid_AuraScript();
        }
};

// 50462 - Anti-Magic Shell (on raid member)
class spell_dk_deaths_advance: public SpellScriptLoader
{
    public:
        spell_dk_deaths_advance() : SpellScriptLoader("spell_dk_deaths_advance") { }

        class spell_dk_deaths_advance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_deaths_advance_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return true;
            }

            void CalculateEffect(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (caster->HasAura(90259))
                    amount = 0;
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_deaths_advance_AuraScript::CalculateEffect, EFFECT_0, SPELL_AURA_MOD_SPEED_NOT_STACK);
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_deaths_advance_AuraScript::CalculateEffect, EFFECT_1, SPELL_AURA_MOD_MINIMUM_SPEED);
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_deaths_advance_AuraScript::CalculateEffect, EFFECT_1, SPELL_AURA_MOD_SPEED_NOT_STACK);
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_deaths_advance_AuraScript::CalculateEffect, EFFECT_0, SPELL_AURA_MOD_MINIMUM_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_deaths_advance_AuraScript();
        }

        class spell_dk_deaths_advance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_deaths_advance_SpellScript);

            SpellCastResult CheckClass()
            {
                if (GetSpellInfo()->Id == 96268)
                    return SPELL_CAST_OK;

                if (GetCaster()->HasAura(90259))
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_deaths_advance_SpellScript::CheckClass);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_deaths_advance_SpellScript();
        }
};

// 48707 - Anti-Magic Shell (on self)
class spell_dk_anti_magic_shell_self : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_shell_self() : SpellScriptLoader("spell_dk_anti_magic_shell_self") { }

        class spell_dk_anti_magic_shell_self_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_shell_self_AuraScript);

            uint32 absorbPct, hpPct;
            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                hpPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                return true;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(DK_SPELL_RUNIC_POWER_ENERGIZE))
                    return false;
                return true;
            }

            void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (GetCaster())
                    amount = GetCaster()->CountPctFromMaxHealth(hpPct);
            }

            void Absorb(AuraEffect * /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                absorbAmount = std::min(CalculatePct(dmgInfo.GetDamage(), absorbPct), GetTarget()->CountPctFromMaxHealth(hpPct));
            }

            void Trigger(AuraEffect *aurEff, DamageInfo & /*dmgInfo*/, uint32 & absorbAmount)
            {
                Unit* target = GetTarget();
                // damage absorbed by Anti-Magic Shell energizes the DK with additional runic power.
                // This, if I'm not mistaken, shows that we get back ~20% of the absorbed damage as runic power.
                int32 bp = absorbAmount * 2 / 10;
                target->CastCustomSpell(target, DK_SPELL_RUNIC_POWER_ENERGIZE, &bp, NULL, NULL, true, NULL, aurEff);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_self_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Absorb, EFFECT_0);
                 AfterEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Trigger, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_shell_self_AuraScript();
        }
};

// 50461 - Anti-Magic Zone
class spell_dk_anti_magic_zone : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_zone() : SpellScriptLoader("spell_dk_anti_magic_zone") { }

        class spell_dk_anti_magic_zone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_zone_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return true;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(DK_SPELL_ANTI_MAGIC_SHELL_TALENT))
                    return false;
                return true;
            }

            void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                amount = 136800;
                if (Player* player = GetCaster()->ToPlayer())
                     amount += int32(player->GetStat(STAT_STRENGTH) * 4);
            }

            void Absorb(AuraEffect * /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                 absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_zone_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_zone_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_zone_AuraScript();
        }
};

// 47496 - Explode, Ghoul spell for Corpse Explosion
class spell_dk_ghoul_explode : public SpellScriptLoader
{
    public:
        spell_dk_ghoul_explode() : SpellScriptLoader("spell_dk_ghoul_explode") { }

        class spell_dk_ghoul_explode_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_ghoul_explode_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(DK_SPELL_CORPSE_EXPLOSION_TRIGGERED))
                    return false;
                return true;
            }

            void Suicide(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                {
                    // Corpse Explosion (Suicide)
                    unitTarget->CastSpell(unitTarget, DK_SPELL_CORPSE_EXPLOSION_TRIGGERED, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_ghoul_explode_SpellScript::Suicide, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_ghoul_explode_SpellScript();
        }
};

// Death Gate - 53822
class spell_dk_death_gate_teleport : public SpellScriptLoader
{
    public:
        spell_dk_death_gate_teleport() : SpellScriptLoader("spell_dk_death_gate_teleport") {}

        class spell_dk_death_gate_teleport_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_gate_teleport_SpellScript);

            SpellCastResult CheckClass()
            {
                if (GetCaster()->getClass() != CLASS_DEATH_KNIGHT)
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }
                return SPELL_CAST_OK;
            }

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->TeleportTo(0, 2359.41f, -5662.084f, 382.259f, 0.60f);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_death_gate_teleport_SpellScript::CheckClass);
                AfterCast += SpellCastFn(spell_dk_death_gate_teleport_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_gate_teleport_SpellScript();
        }
};

// Death Gate - 52751
class spell_dk_death_gate : public SpellScriptLoader
{
    public:
        spell_dk_death_gate() : SpellScriptLoader("spell_dk_death_gate") {}

        class spell_dk_death_gate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_gate_SpellScript);

            SpellCastResult CheckClass()
            {
                if (GetCaster()->getClass() != CLASS_DEATH_KNIGHT)
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                return SPELL_CAST_OK;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Unit* target = GetHitUnit())
                    target->CastSpell(target, GetEffectValue(), false);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_death_gate_SpellScript::CheckClass);
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_gate_SpellScript();
        }
};

class spell_dk_death_pact : public SpellScriptLoader
{
    public:
        spell_dk_death_pact() : SpellScriptLoader("spell_dk_death_pact") { }

        class spell_dk_death_pact_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_pact_SpellScript);

            SpellCastResult CheckCast()
            {
                // Check if we have valid targets, otherwise skip spell casting here
                if (Player* player = GetCaster()->ToPlayer())
                    for (Unit::ControlList::const_iterator itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
                        if (Creature* undeadPet = (*itr)->ToCreature())
                            if (undeadPet->IsAlive() &&
                                undeadPet->GetOwnerGUID() == player->GetGUID() &&
                                undeadPet->GetCreatureType() == CREATURE_TYPE_UNDEAD &&
                                undeadPet->IsWithinDist(player, 100.0f, false))
                                return SPELL_CAST_OK;

                return SPELL_FAILED_NO_PET;
            }

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                Unit* unit_to_add = NULL;
                for (std::list<WorldObject*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr)
                {
                    if (Unit* unit = (*itr)->ToUnit())
                    if (unit->GetOwnerGUID() == GetCaster()->GetGUID() && unit->GetCreatureType() == CREATURE_TYPE_UNDEAD)
                    {
                        unit_to_add = unit;
                        break;
                    }
                }

                unitList.clear();
                if (unit_to_add)
                    unitList.push_back(unit_to_add);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_death_pact_SpellScript::CheckCast);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_death_pact_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_pact_SpellScript();
        }
};

// Scourge Strike - 55090
class spell_dk_scourge_strike : public SpellScriptLoader
{
    public:
        spell_dk_scourge_strike() : SpellScriptLoader("spell_dk_scourge_strike") { }

        class spell_dk_scourge_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_scourge_strike_SpellScript);
            float multiplier;

            bool Load()
            {
                multiplier = 1.0f;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    multiplier = (GetEffectValue() * unitTarget->GetDiseasesByCaster(caster->GetGUID()) / 100.f);
                    // Death Knight T8 Melee 4P Bonus
                    if (AuraEffect const *aurEff = caster->GetAuraEffect(SPELL_DK_ITEM_T8_MELEE_4P_BONUS, EFFECT_0))
                        AddPct(multiplier, aurEff->GetAmount());
                }
            }

            void HandleAfterHit()
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    int32 bp = GetHitDamage() * multiplier;

                    if (AuraEffect *aurEff = caster->GetAuraEffectOfRankedSpell(DK_SPELL_BLACK_ICE_R1, EFFECT_0))
                        AddPct(bp, aurEff->GetAmount());

                    // Mastery: Dreadblade
                    if (AuraEffect *aurEff = caster->GetAuraEffect(77515, EFFECT_0))
                        AddPct(bp, aurEff->GetAmount());

                    caster->CastCustomSpell(unitTarget, DK_SPELL_SCOURGE_STRIKE_TRIGGERED, &bp, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_scourge_strike_SpellScript::HandleDummy, EFFECT_2, SPELL_EFFECT_DUMMY);
                AfterHit += SpellHitFn(spell_dk_scourge_strike_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_scourge_strike_SpellScript();
        }
};

// Blood Boil - 48721
class spell_dk_blood_boil : public SpellScriptLoader
{
    public:
        spell_dk_blood_boil() : SpellScriptLoader("spell_dk_blood_boil") { }

        class spell_dk_blood_boil_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_blood_boil_SpellScript);

            bool runicGranted;
            bool Load()
            {
                runicGranted = false;
                return true;
            }

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (!runicGranted)
                        {
                            GetCaster()->CastSpell(GetCaster(), DK_SPELL_BLOOD_BOIL_TRIGGERED, true);
                            runicGranted = true;
                        }

                        if (player->HasAura(DK_SPELL_SCARLET_FEVER))
                        {
                            player->CastSpell(target, DK_SPELL_WEAKENED_BLOWS, true);

                            if (target->HasAura(DK_SPELL_BLOOD_PLAGUE))
                                if (auto const aura = target->GetAura(DK_SPELL_BLOOD_PLAGUE))
                                    aura->SetDuration(aura->GetMaxDuration());
                            if (target->HasAura(DK_SPELL_FROST_FEVER))
                                if (auto const aura = target->GetAura(DK_SPELL_FROST_FEVER))
                                    aura->SetDuration(aura->GetMaxDuration());
                        }
                        // Deals 50% additional damage to targets infected with Blood Plague or Frost Fever
                        if (target->GetAuraApplication(DK_SPELL_FROST_FEVER))
                        {
                            SetHitDamage(int32(GetHitDamage() * 1.5f));

                            // Roiling Blood
                            if (player->HasAura(DK_SPELL_ROILING_BLOOD))
                                player->CastSpell(target, DK_SPELL_PESTILENCE, true);
                        }
                        else if (target->GetAuraApplication(DK_SPELL_BLOOD_PLAGUE))
                        {
                            SetHitDamage(int32(GetHitDamage() * 1.5f));

                            // Roiling Blood
                            if (player->HasAura(DK_SPELL_ROILING_BLOOD))
                                player->CastSpell(target, DK_SPELL_PESTILENCE, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_blood_boil_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_blood_boil_SpellScript();
        }
};

// Death Grip - 49560
class spell_dk_death_grip : public SpellScriptLoader
{
    public:
        spell_dk_death_grip() : SpellScriptLoader("spell_dk_death_grip") { }

        class spell_dk_death_grip_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_grip_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                int32 damage = GetEffectValue();
                Position const* pos = GetExplTargetDest();
                if (Unit* target = GetHitUnit())
                {
                    if (!target->HasAuraType(SPELL_AURA_DEFLECT_SPELLS)) // Deterrence
                        target->CastSpell(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ(), damage, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_grip_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }

        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_grip_SpellScript();
        }
};

// Glyph of Corpse Explosion - 59336
class spell_dk_glyph_of_corpse_explosion : public SpellScriptLoader
{
    public:
        spell_dk_glyph_of_corpse_explosion() : SpellScriptLoader("spell_dk_glyph_of_corpse_explosion") { }

        class spell_dk_glyph_of_corpse_explosion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_glyph_of_corpse_explosion_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(DK_SPELL_GLYPH_OF_CORPSE_EXPLOSION, false);
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(DK_SPELL_GLYPH_OF_CORPSE_EXPLOSION))
                        _player->removeSpell(DK_SPELL_GLYPH_OF_CORPSE_EXPLOSION, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dk_glyph_of_corpse_explosion_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_dk_glyph_of_corpse_explosion_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_glyph_of_corpse_explosion_AuraScript();
        }
};

// Glyph of Horn of Winter - 58680
// Called by Horn of Winter - 57330
class spell_dk_glyph_of_horn_of_winter : public SpellScriptLoader
{
    public:
        spell_dk_glyph_of_horn_of_winter() : SpellScriptLoader("spell_dk_glyph_of_horn_of_winter") { }

        class spell_dk_glyph_of_horn_of_winter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_glyph_of_horn_of_winter_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (!_player->IsInCombat() && _player->HasAura(DK_SPELL_GLYPH_OF_HORN_OF_WINTER))
                        _player->CastSpell(_player, DK_SPELL_GLYPH_OF_HORN_OF_WINTER_EFFECT, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_dk_glyph_of_horn_of_winter_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_glyph_of_horn_of_winter_SpellScript();
        }
};

// 145677 - Riposte
class spell_dk_riposte final : public SpellScriptLoader
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

            if (Player const * const player = caster->ToPlayer())
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
    spell_dk_riposte()
        : SpellScriptLoader("spell_dk_riposte")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Plague Strike - 45462
class spell_dk_plague_strike : public SpellScriptLoader
{
public:
    spell_dk_plague_strike() : SpellScriptLoader("spell_dk_plague_strike") { }

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit * const caster = GetCaster();
            if (!caster || !caster->HasAura(51160))
                return;

            caster->CastSpell(GetHitUnit(), DK_SPELL_FROST_FEVER, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleDummy, EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
        }

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_dk_asphyxiate : public SpellScriptLoader
{
public:
    spell_dk_asphyxiate() : SpellScriptLoader("spell_dk_asphyxiate") { }

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleOnHit()
        {
            Spell* spell = GetSpell();
            Unit* target = GetHitUnit();
            if (!spell || !target)
                return;

            if (spell->GetDiminishingLevel() == DIMINISHING_LEVEL_IMMUNE)
                GetCaster()->CastSpell(target, 47476, true);
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

// Unholy Frenzy - 49016
class spell_dk_unholy_frenzy final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            GLYPH_OF_UNHOLY_FRENZY  = 58616
        };

        void onInitEffects(uint32 &effectMask)
        {
            auto const caster = GetCaster();
            if (caster && caster->HasAura(GLYPH_OF_UNHOLY_FRENZY))
                effectMask &= ~(1 << EFFECT_1);
        }

        void Register() final
        {
            OnInitEffects += AuraInitEffectsFn(script_impl::onInitEffects);
        }
    };

public:
    spell_dk_unholy_frenzy()
        : SpellScriptLoader("spell_dk_unholy_frenzy")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// Soul Reaper - 114868
class spell_dk_soul_reaper_effect final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            GLYPH_OF_SWIFT_DEATH  = 146645
        };

        void onInitEffects(uint32 &effectMask)
        {
            auto const caster = GetCaster();
            if (caster && !caster->HasAura(GLYPH_OF_SWIFT_DEATH))
                effectMask &= ~(1 << EFFECT_1);
        }

        void Register() final
        {
            OnInitEffects += AuraInitEffectsFn(script_impl::onInitEffects);
        }
    };

public:
    spell_dk_soul_reaper_effect()
        : SpellScriptLoader("spell_dk_soul_reaper_effect")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// 63560 - Dark Transformation
class spell_dk_dark_transformation: public SpellScriptLoader
{
    public:
        spell_dk_dark_transformation() : SpellScriptLoader("spell_dk_dark_transformation") { }

        enum
        {
            SPELL_DK_SHADOW_UNFUSION = 91342
        };

        class spell_dk_dark_transformation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_dark_transformation_SpellScript);

            SpellCastResult CheckClass()
            {
                if (Aura* aura = GetCaster()->GetAura(SPELL_DK_SHADOW_UNFUSION))
                    if (aura->GetStackAmount() == 5)
                        return SPELL_CAST_OK;

                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_dark_transformation_SpellScript::CheckClass);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_dark_transformation_SpellScript();
        }
};

// Shadow Infusion - 91342
class spell_dk_shadow_infusion : public SpellScriptLoader
{
    public:
        spell_dk_shadow_infusion() : SpellScriptLoader("spell_dk_shadow_infusion") { }

        class spell_dk_shadow_infusion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_shadow_infusion_AuraScript);

            enum
            {
                SPELL_DK_DARK_TRANS_DRIVER = 93426
            };

            void OnRemoveFusion(AuraEffect const* /*eff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster()->GetTypeId() != TYPEID_UNIT)
                    return;

                GetCaster()->GetCharmerOrOwnerOrSelf()->RemoveAurasDueToSpell(SPELL_DK_DARK_TRANS_DRIVER);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_dk_shadow_infusion_AuraScript::OnRemoveFusion, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_shadow_infusion_AuraScript();
        }
};

void AddSC_deathknight_spell_scripts()
{
    new spell_dk_shadow_infusion();
    new spell_dk_dark_transformation();
    new spell_dk_gorefiends_grasp();
    new spell_dk_runic_empowerment();
    new spell_dk_runic_corruption();
    new spell_dk_might_of_ursoc();
    new spell_dk_wild_mushroom_plague();
    new spell_dk_dark_transformation_form();
    new spell_dk_desecrated_ground();
    new spell_dk_necrotic_strike();
    new spell_dk_festering_strike();
    new spell_dk_death_strike_heal();
    new spell_dk_howling_blast();
    new spell_dk_conversion();
    new spell_dk_remorseless_winter();
    new spell_dk_soul_reaper();
    new spell_dk_pillar_of_frost();
    new spell_dk_blood_charges();
    new spell_dk_blood_tap();
    new spell_dk_death_siphon();
    new spell_dk_death_strike();
    new spell_dk_purgatory();
    new spell_dk_purgatory_absorb();
    new spell_dk_plague_leech();
    new spell_dk_unholy_blight();
    new spell_dk_chilblains();
    new spell_dk_outbreak();
    new spell_dk_raise_dead();
    new spell_dk_anti_magic_shell_raid();
    new spell_dk_anti_magic_shell_self();
    new spell_dk_anti_magic_zone();
    new spell_dk_ghoul_explode();
    new spell_dk_death_gate_teleport();
    new spell_dk_death_gate();
    new spell_dk_death_pact();
    new spell_dk_scourge_strike();
    new spell_dk_blood_boil();
    new spell_dk_death_grip();
    new spell_dk_riposte();
    new spell_dk_plague_strike();
    new spell_dk_unholy_frenzy();
    new spell_dk_glyph_of_horn_of_winter();
    new spell_dk_soul_reaper_effect();
    new spell_dk_asphyxiate();
    new spell_dk_deaths_advance();
	new spell_dk_death_and_decay();
}
