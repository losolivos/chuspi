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
 * Scripts for spells with SPELLFAMILY_PRIEST and SPELLFAMILY_GENERIC spells used by priest players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pri_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"

enum PriestSpells
{
    PRIEST_SPELL_GUARDIAN_SPIRIT_HEAL               = 48153,
    PRIEST_SPELL_PENANCE                            = 47540,
    PRIEST_SPELL_PENANCE_DAMAGE                     = 47758,
    PRIEST_SPELL_PENANCE_HEAL                       = 47757,
    PRIEST_SHADOW_WORD_DEATH                        = 32409,
    PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH          = 107903,
    PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH             = 107904,
    PRIEST_GLYPH_OF_SHADOW                          = 107906,
    PRIEST_VOID_SHIFT                               = 108968,
    PRIEST_LEAP_OF_FAITH                            = 73325,
    PRIEST_LEAP_OF_FAITH_JUMP                       = 110726,
    PRIEST_INNER_WILL                               = 73413,
    PRIEST_INNER_FIRE                               = 588,
    PRIEST_NPC_SHADOWY_APPARITION                   = 46954,
    PRIEST_SPELL_HALO_EFFECT_SHADOW                 = 120696,
    PRIEST_SPELL_HALO_EFFECT_HOLY                   = 120692,

    // Cascade
    PRIEST_CASCADE_HOLY_DAMAGE                      = 120785,
    PRIEST_CASCADE_HOLY_TRIGGER                     = 120786,
    PRIEST_CASCADE_INVISIBLE_AURA                   = 120840,
    PRIEST_CASCADE_HOLY_TALENT                      = 121135,
    PRIEST_CASCADE_HOLY_MISSILE                     = 121146,
    PRIEST_CASCADE_HOLY_HEAL                        = 121148,
    PRIEST_CASCADE_SHADOW_MISSILE                   = 127627,
    PRIEST_CASCADE_SHADOW_DAMAGE                    = 127628,
    PRIEST_CASCADE_SHADOW_HEAL                      = 127629,
    PRIEST_CASCADE_DAMAGE_TRIGGER                   = 127630,
    PRIEST_CASCADE_INVISIBLE_AURA_2                 = 127631,
    PRIEST_CASCADE_SHADOW_TALENT                    = 127632,

    PRIEST_SHADOWFORM_STANCE                        = 15473,
    PRIEST_SHADOW_WORD_PAIN                         = 589,
    PRIEST_DEVOURING_PLAGUE                         = 2944,
    PRIEST_DEVOURING_PLAGUE_HEAL                    = 127626,
    PRIEST_VAMPIRIC_TOUCH                           = 34914,
    PRIEST_PHANTASM_AURA                            = 108942,
    PRIEST_PHANTASM_PROC                            = 114239,
    PRIEST_SPIRIT_SHELL_AURA                        = 109964,
    PRIEST_SPIRIT_SHELL_ABSORPTION                  = 114908,
    PRIEST_ATONEMENT_AURA                           = 81749,
    PRIEST_ATONEMENT_HEAL                           = 81751,
    PRIEST_INNER_FOCUS                              = 89485,
    PRIEST_GRACE_AURA                               = 47517,
    PRIEST_GRACE_PROC                               = 77613,
    PRIEST_STRENGTH_OF_SOUL_AURA                    = 89488,
    PRIEST_STRENGTH_OF_SOUL_REDUCE_TIME             = 89490,
    PRIEST_WEAKENED_SOUL                            = 6788,
    PRIEST_STRENGTH_OF_SOUL                         = 89488,
    PRIEST_EVANGELISM_AURA                          = 81662,
    PRIEST_EVANGELISM_STACK                         = 81661,
    PRIEST_ARCHANGEL                                = 81700,
    LIGHTWELL_CHARGES                               = 59907,
    LIGHTSPRING_RENEW                               = 126154,
    PRIEST_SMITE                                    = 585,
    PRIEST_HOLY_WORD_CHASTISE                       = 88625,
    PRIEST_HOLY_WORD_SANCTUARY_AREA                 = 88685,
    PRIEST_HOLY_WORD_SANCTUARY_HEAL                 = 88686,
    PRIEST_SPELL_DIVINE_INSIGHT_TALENT              = 109175,
    PRIEST_SPELL_DIVINE_INSIGHT_DISCIPLINE          = 123266,
    PRIEST_SPELL_DIVINE_INSIGHT_HOLY                = 123267,
    PRIEST_PRAYER_OF_MENDING                        = 33076,
    PRIEST_PRAYER_OF_MENDING_HEAL                   = 33110,
    PRIEST_PRAYER_OF_MENDING_RADIUS                 = 123262,
    PRIEST_BODY_AND_SOUL_AURA                       = 64129,
    PRIEST_BODY_AND_SOUL_INCREASE_SPEED             = 65081,
    PRIEST_FROM_DARKNESS_COMES_LIGHT_AURA           = 109186,
    PRIEST_SURGE_OF_LIGHT                           = 114255,
    PRIEST_SURGE_OF_DARKNESS                        = 87160,
    PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST       = 130733,
    PRIEST_SHADOW_WORD_INSANITY_DAMAGE              = 129249,
    PRIEST_SPELL_MIND_BLAST                         = 8092,
    PRIEST_SPELL_2P_S12_SHADOW                      = 92711,
    PRIEST_SPELL_DISPERSION_SPRINT                  = 129960,
    PRIEST_SPELL_4P_S12_SHADOW                      = 131556,
    PRIEST_SPELL_SIN_AND_PUNISHMENT                 = 87204,
    PRIEST_SPELL_2P_S12_HEAL                        = 33333,
    PRIEST_SPELL_SOUL_OF_DIAMOND                    = 96219,
    PRIEST_SPELL_4P_S12_HEAL                        = 131566,
    PRIEST_SPELL_HOLY_SPARK                         = 131567,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_IMMUNITY      = 62371,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM          = 27795,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_TALENT        = 20711,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_ROOT          = 27792,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_SHAPESHIFT    = 27827,
    PRIEST_SPELL_LEVITATE                           = 111758,
    PRIEST_SPELL_VOID_TENDRILS_SUMMON               = 127665,
    PRIEST_NPC_VOID_TENDRILS                        = 65282,
    PRIEST_NPC_PSYFIEND                             = 59190,
    PRIEST_SPELL_SPECTRAL_GUISE_CHARGES             = 119030,
    PRIEST_SPELL_POWER_WORD_SHIELD                  = 17,
    SPELL_PRIEST_DIVINE_AEGIS                       = 47753,
    SPELL_PRIEST_SHIELD_DISCIPLINE                  = 77484,
    SPELL_PRIEST_RAPID_RENEWAL                      = 95649,
    SPELL_PRIEST_RAPID_RENEWAL_HEAL                 = 63544,
    SPELL_PRIEST_ECHO_OF_LIGHT_HEAL                 = 77489,
};

// Power Word : Fortitude - 21562
class spell_pri_power_word_fortitude : public SpellScriptLoader
{
    public:
        spell_pri_power_word_fortitude() : SpellScriptLoader("spell_pri_power_word_fortitude") { }

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

// Spectral Guise Charges - 119030
class spell_pri_spectral_guise_charges : public SpellScriptLoader
{
    public:
        spell_pri_spectral_guise_charges() : SpellScriptLoader("spell_pri_spectral_guise_charges") { }

        class spell_pri_spectral_guise_charges_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_spectral_guise_charges_AuraScript);

            void OnProc(AuraEffect const * /*aurEff*/, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                Unit* attacker = eventInfo.GetActor();
                if (!attacker)
                    return;

                if (eventInfo.GetActor()->GetGUID() != GetCaster()->GetGUID())
                    return;

                if (Unit* spectralGuise = GetCaster())
                    if (eventInfo.GetDamageInfo()->GetDamageType() == SPELL_DIRECT_DAMAGE || eventInfo.GetDamageInfo()->GetDamageType() == DIRECT_DAMAGE)
                        if (Aura *spectralGuiseCharges = spectralGuise->GetAura(PRIEST_SPELL_SPECTRAL_GUISE_CHARGES))
                            spectralGuiseCharges->DropCharge();
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->ToCreature())
                        caster->ToCreature()->DespawnOrUnsummon();
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_pri_spectral_guise_charges_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_spectral_guise_charges_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_spectral_guise_charges_AuraScript();
        }
};

class spell_pri_psychic_terror : public SpellScriptLoader {
public:
    spell_pri_psychic_terror() : SpellScriptLoader("spell_pri_psychic_terror") { }
    
    class spell_pri_psychic_terror_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_psychic_terror_SpellScript);
        
        void SelectTarget(std::list<WorldObject*>& targets)
        {
            // 1. Clear all the targets : we only chose one
            std::list<WorldObject*> preClearTargets = targets;
            targets.clear();
            
            // The caster is a mob, whom we need to check the owner
            Unit* caster = GetCaster();
            if (!caster)
                return;
            Unit* owner = caster->GetOwner();
            if (!owner)
                return;
            
            // 2. First check for a victim
            if (AuraEffect* driver = owner->GetAuraEffect(114164, EFFECT_0))
            { 
                if (Unit* target = sObjectAccessor->GetUnit(*owner, driver->GetAmount()))
                {
                    if (!target->HasAura(113792))
                    {
                        driver->SetAmount(0);
                        targets.push_back(target); 
                        return;
                    }
                }
            }
            
            // 3. Loop over the threat list : it is supposed to be ordered by threat level
            if (caster) 
            {
                std::list<HostileReference*> const& threatList = caster->getThreatManager().getThreatList();
                for (std::list<HostileReference*>::const_iterator iter = threatList.begin(); iter != threatList.end(); ++iter)
                {
                    if (HostileReference *ref = (*iter))
                    {
                        if (Unit *target = ref->getTarget())
                        {
                            if (!target->HasAura(113792))
                            {
                                targets.push_back(target);
                                return;
                            }
                        }
                    }
                }
                // 4. No target found with threat list : select a random inside it, instead of starting a new fight
                if (!threatList.empty())
                {
                    HostileReference* ref = Trinity::Containers::SelectRandomContainerElement(threatList);
                    targets.push_back(ref->getTarget());
                }
                else if (!preClearTargets.empty())
                {
                    for (std::list<WorldObject*>::iterator itr = preClearTargets.begin(); itr != preClearTargets.end(); itr++)
                        if (Unit* target = (*itr)->ToUnit())
                            if (!target->HasAura(113792))
                            {
                                targets.push_back(target);
                                return;
                            }
                }
            }
        }


        void HandleOnHit()
        {
            if (!GetHitUnit())
                return;

            bool glyph = GetCaster()->HasAura(55676);
            if (Aura* debuff = GetHitAura())
            {
                if (debuff->GetApplicationOfTarget(GetHitUnit()->GetGUID()))
                {
                    // Remove fear aspect
                    if (AuraEffect* fear = debuff->GetEffect(EFFECT_0))
                    {
                        if (glyph)
                            fear->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                    }

                    // Remove root aspect
                    if (AuraEffect* root = debuff->GetEffect(EFFECT_2))
                    {
                        if (!glyph)
                            root->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                    }
                }
            }
        }
        
        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_psychic_terror_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_psychic_terror_SpellScript::SelectTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_psychic_terror_SpellScript::SelectTarget, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_pri_psychic_terror_SpellScript::HandleOnHit);
        }
    };
    
    SpellScript *GetSpellScript() const
    {
        return new spell_pri_psychic_terror_SpellScript();
    }
};

// Void Tendrils - 108920
class spell_pri_void_tendrils : public SpellScriptLoader
{
    public:
        spell_pri_void_tendrils() : SpellScriptLoader("spell_pri_void_tendrils") { }

        class spell_pri_void_tendrils_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_void_tendrils_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, PRIEST_SPELL_VOID_TENDRILS_SUMMON, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_void_tendrils_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_void_tendrils_SpellScript();
        }
};

// Phantasm (proc) - 114239
class spell_pri_phantasm_proc : public SpellScriptLoader
{
    public:
        spell_pri_phantasm_proc() : SpellScriptLoader("spell_pri_phantasm_proc") { }

        class spell_pri_phantasm_proc_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_phantasm_proc_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->RemoveMovementImpairingAuras();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_phantasm_proc_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_phantasm_proc_SpellScript();
        }
};

// Spirit of Redemption (Shapeshift) - 27827
class spell_pri_spirit_of_redemption_form : public SpellScriptLoader
{
    public:
        spell_pri_spirit_of_redemption_form() : SpellScriptLoader("spell_pri_spirit_of_redemption_form") { }

        class spell_pri_spirit_of_redemption_form_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_spirit_of_redemption_form_AuraScript);

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->RemoveAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_IMMUNITY);
                    caster->RemoveAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM);
                    caster->RemoveAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_ROOT);
                    caster->setDeathState(JUST_DIED);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_spirit_of_redemption_form_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_WATER_BREATHING, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_spirit_of_redemption_form_AuraScript();
        }
};

// Spirit of Redemption - 20711
class spell_pri_spirit_of_redemption : public SpellScriptLoader
{
    public:
        spell_pri_spirit_of_redemption() : SpellScriptLoader("spell_pri_spirit_of_redemption") { }

        class spell_pri_spirit_of_redemption_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_spirit_of_redemption_AuraScript);

            void CalculateAmount(AuraEffect const * /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect * /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->ToPlayer())
                        return;

                    if (dmgInfo.GetDamage() < caster->GetHealth())
                        return;

                    if (caster->ToPlayer()->HasSpellCooldown(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_TALENT))
                        return;

                    caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM, true);
                    caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_IMMUNITY, true);
                    caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_ROOT, true);
                    caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_SHAPESHIFT, true);
                    caster->ToPlayer()->AddSpellCooldown(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_TALENT, 0, 90 * IN_MILLISECONDS);

                    absorbAmount = caster->GetHealth() - 1;
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_spirit_of_redemption_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_spirit_of_redemption_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_spirit_of_redemption_AuraScript();
        }
};

// Called by Prayer of Mending - 33076
// Item : S12 4P bonus - Heal
class spell_pri_item_s12_4p_heal : public SpellScriptLoader
{
    public:
        spell_pri_item_s12_4p_heal() : SpellScriptLoader("spell_pri_item_s12_4p_heal") { }

        class spell_pri_item_s12_4p_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_item_s12_4p_heal_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(PRIEST_SPELL_4P_S12_HEAL))
                            _player->CastSpell(target, PRIEST_SPELL_HOLY_SPARK, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_item_s12_4p_heal_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_item_s12_4p_heal_SpellScript();
        }
};

// Called by Dispersion - 47585
// Item : S12 2P bonus - Shadow
class spell_pri_item_s12_2p_shadow : public SpellScriptLoader
{
    public:
        spell_pri_item_s12_2p_shadow() : SpellScriptLoader("spell_pri_item_s12_2p_shadow") { }

        class spell_pri_item_s12_2p_shadow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_item_s12_2p_shadow_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PRIEST_SPELL_2P_S12_SHADOW))
                        _player->CastSpell(_player, PRIEST_SPELL_DISPERSION_SPRINT, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_item_s12_2p_shadow_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_item_s12_2p_shadow_SpellScript();
        }
};

// Divine Insight - 124430
class spell_pri_divine_insight_shadow : public SpellScriptLoader
{
    public:
        spell_pri_divine_insight_shadow() : SpellScriptLoader("spell_pri_divine_insight_shadow") { }

        class spell_pri_divine_insight_shadow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_divine_insight_shadow_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasSpellCooldown(PRIEST_SPELL_MIND_BLAST))
                        _player->RemoveSpellCooldown(PRIEST_SPELL_MIND_BLAST, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_divine_insight_shadow_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_divine_insight_shadow_SpellScript();
        }
};

// Power Word - Insanity - 129249
class spell_pri_power_word_insanity : public SpellScriptLoader
{
    public:
        spell_pri_power_word_insanity() : SpellScriptLoader("spell_pri_power_word_insanity") { }

        class spell_pri_power_word_insanity_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_power_word_insanity_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->HasAura(PRIEST_SHADOW_WORD_PAIN, _player->GetGUID()))
                            target->RemoveAura(PRIEST_SHADOW_WORD_PAIN, _player->GetGUID());

                        if (target->HasAura(PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST, _player->GetGUID()))
                            target->RemoveAura(PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST, _player->GetGUID());
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_power_word_insanity_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_power_word_insanity_SpellScript();
        }
};

// Power Word - Solace - 129250
class spell_pri_power_word_solace : public SpellScriptLoader
{
    public:
        spell_pri_power_word_solace() : SpellScriptLoader("spell_pri_power_word_solace") { }

        class spell_pri_power_word_solace_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_power_word_solace_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (GetHitUnit())
                        player->CastSpell(player, 129253, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_power_word_solace_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_power_word_solace_SpellScript();
        }
};

// Called by Shadow Word : Pain - 589
// Shadow Word : Insanity (allowing cast) - 130733
class spell_pri_shadow_word_insanity_allowing : public SpellScriptLoader
{
    public:
        spell_pri_shadow_word_insanity_allowing() : SpellScriptLoader("spell_pri_shadow_word_insanity_allowing") { }

        class spell_pri_shadow_word_insanity_allowing_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_shadow_word_insanity_allowing_AuraScript);

            std::list<Unit*> targetList;

            void OnUpdate(uint32 /*diff*/, AuraEffect *aurEff)
            {
                aurEff->GetTargetList(targetList);

                for (auto itr : targetList)
                {
                    if (Unit* caster = GetCaster())
                    {
                        if (Aura *shadowWordPain = itr->GetAura(PRIEST_SHADOW_WORD_PAIN, caster->GetGUID()))
                        {
                            if (shadowWordPain->GetDuration() <= (shadowWordPain->GetEffect(0)->GetAmplitude() * 2))
                                caster->CastSpell(itr, PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST, true);
                            else
                                itr->RemoveAura(PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST);
                        }
                    }
                }

                targetList.clear();
            }

            void Register()
            {
                OnEffectUpdate += AuraEffectUpdateFn(spell_pri_shadow_word_insanity_allowing_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_shadow_word_insanity_allowing_AuraScript();
        }
};

// Shadowfiend - 34433 or Mindbender - 123040
class spell_pri_shadowfiend : public SpellScriptLoader
{
    public:
        spell_pri_shadowfiend() : SpellScriptLoader("spell_pri_shadowfiend") { }

        class spell_pri_shadowfiend_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_shadowfiend_SpellScript);

            void HandleAfterHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetExplTargetUnit())
                    {
                        if (Guardian* pet = _player->GetGuardianPet())
                        {
                            pet->InitCharmInfo();
                            pet->SetReactState(REACT_DEFENSIVE);
                            pet->ToCreature()->AI()->AttackStart(target);
                        }
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_pri_shadowfiend_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_shadowfiend_SpellScript();
        }
};

// Called by Smite - 585, Heal - 2050, Flash Heal - 2061, Binding Heal - 32546 and Greater Heal - 2060 (Surge of Darkness)
// From Darkness, Comes Light - 109186
class spell_pri_from_darkness_comes_light : public SpellScriptLoader
{
    public:
        spell_pri_from_darkness_comes_light() : SpellScriptLoader("spell_pri_from_darkness_comes_light") { }

        class spell_pri_from_darkness_comes_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_from_darkness_comes_light_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PRIEST_FROM_DARKNESS_COMES_LIGHT_AURA) && _player->GetSpecializationId(_player->GetActiveSpec()) != SPEC_PRIEST_SHADOW)
                        if (roll_chance_i(15))
                            _player->CastSpell(_player, PRIEST_SURGE_OF_LIGHT, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_from_darkness_comes_light_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_from_darkness_comes_light_SpellScript();
        }
};

// Called by Leap of Faith - 73325 and Power Word : Shield - 17
// Body and Soul - 64129
class spell_pri_body_and_soul : public SpellScriptLoader
{
    public:
        spell_pri_body_and_soul() : SpellScriptLoader("spell_pri_body_and_soul") { }

        class spell_pri_body_and_soul_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_body_and_soul_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(PRIEST_BODY_AND_SOUL_AURA))
                            _player->CastSpell(target, PRIEST_BODY_AND_SOUL_INCREASE_SPEED, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_body_and_soul_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_body_and_soul_SpellScript();
        }
};

// Prayer of Mending (Divine Insight) - 123259
class spell_pri_prayer_of_mending_divine_insight : public SpellScriptLoader
{
    public:
        spell_pri_prayer_of_mending_divine_insight() : SpellScriptLoader("spell_pri_prayer_of_mending_divine_insight") { }

        class spell_pri_prayer_of_mending_divine_insight_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_prayer_of_mending_divine_insight_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (Aura *prayerOfMending = target->GetAura(PRIEST_PRAYER_OF_MENDING_RADIUS, _player->GetGUID()))
                        {
                            int32 value = prayerOfMending->GetEffect(0)->GetAmount();

                            if (_player->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_HOLY))
                                _player->RemoveAura(PRIEST_SPELL_DIVINE_INSIGHT_HOLY);

                            target->CastCustomSpell(target, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());
                            if (target->HasAura(GetSpellInfo()->Id))
                                target->RemoveAura(GetSpellInfo()->Id);

                            float radius = sSpellMgr->GetSpellInfo(PRIEST_PRAYER_OF_MENDING_RADIUS)->Effects[0].CalcRadius(_player);

                            if (Unit* secondTarget = target->GetNextRandomRaidMemberOrPet(radius))
                            {
                                target->CastCustomSpell(secondTarget, PRIEST_PRAYER_OF_MENDING, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());
                                if (secondTarget->HasAura(PRIEST_PRAYER_OF_MENDING))
                                    secondTarget->RemoveAura(PRIEST_PRAYER_OF_MENDING);

                                secondTarget->CastCustomSpell(secondTarget, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());

                                if (Unit* thirdTarget = target->GetNextRandomRaidMemberOrPet(radius))
                                {
                                    secondTarget->CastCustomSpell(thirdTarget, PRIEST_PRAYER_OF_MENDING, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());
                                    if (thirdTarget->HasAura(PRIEST_PRAYER_OF_MENDING))
                                        thirdTarget->RemoveAura(PRIEST_PRAYER_OF_MENDING);

                                    thirdTarget->CastCustomSpell(thirdTarget, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());

                                    if (Unit* fourthTarget = target->GetNextRandomRaidMemberOrPet(radius))
                                    {
                                        thirdTarget->CastCustomSpell(fourthTarget, PRIEST_PRAYER_OF_MENDING, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());
                                        if (fourthTarget->HasAura(PRIEST_PRAYER_OF_MENDING))
                                            fourthTarget->RemoveAura(PRIEST_PRAYER_OF_MENDING);

                                        fourthTarget->CastCustomSpell(fourthTarget, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());

                                        if (Unit* fifthTarget = target->GetNextRandomRaidMemberOrPet(radius))
                                        {
                                            fourthTarget->CastCustomSpell(fifthTarget, PRIEST_PRAYER_OF_MENDING, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());
                                            if (fifthTarget->HasAura(PRIEST_PRAYER_OF_MENDING))
                                                fifthTarget->RemoveAura(PRIEST_PRAYER_OF_MENDING);

                                            fifthTarget->CastCustomSpell(fifthTarget, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, NULL, _player->GetGUID());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_prayer_of_mending_divine_insight_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_prayer_of_mending_divine_insight_SpellScript();
        }
};

// Called by Greater Heal - 2060 and Prayer of Healing - 596
// Divine Insight (Holy) - 109175
class spell_pri_divine_insight_holy : public SpellScriptLoader
{
    public:
        spell_pri_divine_insight_holy() : SpellScriptLoader("spell_pri_divine_insight_holy") { }

        class spell_pri_divine_insight_holy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_divine_insight_holy_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_TALENT))
                        if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_PRIEST_HOLY)
                            if (roll_chance_i(40))
                                _player->CastSpell(_player, PRIEST_SPELL_DIVINE_INSIGHT_HOLY, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_divine_insight_holy_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_divine_insight_holy_SpellScript();
        }
};

// Called by Power Word : Shield (Divine Insight) - 123258
// Divine Insight (Discipline) - 123266
class spell_pri_divine_insight_discipline : public SpellScriptLoader
{
    public:
        spell_pri_divine_insight_discipline() : SpellScriptLoader("spell_pri_divine_insight_discipline") { }

        class spell_pri_divine_insight_discipline_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_divine_insight_discipline_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_DISCIPLINE))
                        _player->RemoveAura(PRIEST_SPELL_DIVINE_INSIGHT_DISCIPLINE);

                    if (Unit* target = GetHitUnit())
                    {
                        if (target->HasAura(PRIEST_SPELL_POWER_WORD_SHIELD))
                            target->RemoveAura(PRIEST_SPELL_POWER_WORD_SHIELD);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_divine_insight_discipline_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_divine_insight_discipline_SpellScript();
        }
};

// Holy Word : Sanctuary - 88685
class spell_pri_holy_word_sanctuary : public SpellScriptLoader
{
    public:
        spell_pri_holy_word_sanctuary() : SpellScriptLoader("spell_pri_holy_word_sanctuary") { }

        class spell_pri_holy_word_sanctuary_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_holy_word_sanctuary_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                auto const caster = GetCaster();
                if (!caster)
                    return;

                if (DynamicObject* dynObj = caster->GetDynObject(PRIEST_HOLY_WORD_SANCTUARY_AREA))
                    caster->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), PRIEST_HOLY_WORD_SANCTUARY_HEAL, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_holy_word_sanctuary_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_holy_word_sanctuary_AuraScript();
        }
};

// Called by Smite - 585
// Chakra : Chastise - 81209
class spell_pri_chakra_chastise : public SpellScriptLoader
{
    public:
        spell_pri_chakra_chastise() : SpellScriptLoader("spell_pri_chakra_chastise") { }

        class spell_pri_chakra_chastise_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_chakra_chastise_SpellScript);

            void HandleOnHit()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (GetHitUnit() && roll_chance_i(10))
                        if (player->HasSpellCooldown(PRIEST_HOLY_WORD_CHASTISE))
                            player->RemoveSpellCooldown(PRIEST_HOLY_WORD_CHASTISE, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_chakra_chastise_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_chakra_chastise_SpellScript();
        }
};

// Lightwell Renew - 60123
class spell_pri_lightwell_renew : public SpellScriptLoader
{
    public:
        spell_pri_lightwell_renew() : SpellScriptLoader("spell_pri_lightwell_renew") { }

        class spell_pri_lightwell_renew_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_lightwell_renew_SpellScript);

            void HandleOnHit()
            {
                if (Unit* m_caster = GetCaster())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        if (m_caster->GetTypeId() != TYPEID_UNIT || !m_caster->ToCreature()->IsSummon())
                            return;

                        // proc a spellcast
                        if (Aura *chargesAura = m_caster->GetAura(LIGHTWELL_CHARGES))
                        {
                            m_caster->CastSpell(unitTarget, LIGHTSPRING_RENEW, true, NULL, NULL, m_caster->ToTempSummon()->GetSummonerGUID());
                            if (chargesAura->ModCharges(-1))
                                m_caster->ToTempSummon()->UnSummon();

                            chargesAura->SetNeedClientUpdateForTargets();
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_lightwell_renew_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_lightwell_renew_SpellScript();
        }
};

// Called by Heal - 2050, Greater Heal - 2060 and Flash Heal - 2061
// Strength of Soul - 89488
class spell_pri_strength_of_soul : public SpellScriptLoader
{
    public:
        spell_pri_strength_of_soul() : SpellScriptLoader("spell_pri_strength_of_soul") { }

        class spell_pri_strength_of_soul_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_strength_of_soul_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(PRIEST_STRENGTH_OF_SOUL))
                        {
                            if (Aura *weakenedSoul = target->GetAura(PRIEST_WEAKENED_SOUL, _player->GetGUID()))
                            {
                                if (weakenedSoul->GetDuration() > 2000)
                                    weakenedSoul->SetDuration(weakenedSoul->GetDuration() - 2000);
                                else
                                    target->RemoveAura(PRIEST_WEAKENED_SOUL);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_strength_of_soul_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_strength_of_soul_SpellScript();
        }
};

// Called by Heal - 2050
// Grace - 47517
class spell_pri_grace : public SpellScriptLoader
{
    public:
        spell_pri_grace() : SpellScriptLoader("spell_pri_grace") { }

        class spell_pri_grace_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_grace_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(PRIEST_GRACE_AURA))
                            _player->CastSpell(target, PRIEST_GRACE_PROC, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_grace_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_grace_SpellScript();
        }
};

// Atonement - 81749
class spell_pri_atonement : public SpellScriptLoader
{
public:
    spell_pri_atonement() : SpellScriptLoader("spell_pri_atonement") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(PRIEST_ATONEMENT_HEAL))
                return false;
            return true;
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetProcTarget() && eventInfo.GetSpellInfo();
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            auto const target = eventInfo.GetProcTarget();
            auto const player = GetTarget()->ToPlayer();
            if (!target || !player || !player->IsAlive())
                return;

            std::list<Unit*> groupList;
            player->GetPartyMembers(groupList);

            if (groupList.empty())
                return;

            groupList.sort(Trinity::HealthPctOrderPred());
            auto const healTarget = groupList.front();

            int32 bp = eventInfo.GetDamageInfo()->GetDamage();
            if (healTarget->GetGUID() == player->GetGUID())
                bp /= 2;

            player->CastCustomSpell(healTarget, PRIEST_ATONEMENT_HEAL, &bp, NULL, NULL, true);
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(script_impl::CheckProc);
            OnEffectProc += AuraEffectProcFn(script_impl::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Called by Heal - 2050, Flash Heal - 2061, Greater Heal - 2060 and Prayer of Healing - 596
// Spirit Shell - 109964
class spell_pri_spirit_shell : public SpellScriptLoader
{
    public:
        spell_pri_spirit_shell() : SpellScriptLoader("spell_pri_spirit_shell") { }

        class spell_pri_spirit_shell_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_spirit_shell_SpellScript);

            void HandleOnHit()
            {
                if (Player* const _player = GetCaster()->ToPlayer())
                {
                    if (Unit* const target = GetHitUnit())
                    {
                        if (_player->HasAura(PRIEST_SPIRIT_SHELL_AURA))
                        {
                            int32 absorb = GetHitHeal();

                            SetHitHeal(0);

                            // Multiple effects stack, so let's try to find this aura.
                            if (AuraEffect const* const shell = target->GetAuraEffect(PRIEST_SPIRIT_SHELL_ABSORPTION, EFFECT_0, _player->GetGUID()))
                                absorb += shell->GetAmount();

                            absorb = std::min<int32>(absorb, target->CountPctFromMaxHealth(60));

                            _player->CastCustomSpell(target, PRIEST_SPIRIT_SHELL_ABSORPTION, &absorb, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_spirit_shell_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_spirit_shell_SpellScript();
        }
};

// Devouring Plague - 2944
class spell_pri_devouring_plague : public SpellScriptLoader
{
    public:
        spell_pri_devouring_plague() : SpellScriptLoader("spell_pri_devouring_plague") { }

        class spell_pri_devouring_plague_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_devouring_plague_SpellScript);

            void HandleDamage(SpellEffIndex /*eff*/)
            {
                auto player = GetCaster()->ToPlayer();
                if (player && player->GetSpecializationId(player->GetActiveSpec()) == SPEC_PRIEST_SHADOW)
                {
                    // Shadow Orb visual
                    if (player->HasAura(77487))
                        player->RemoveAura(77487);
                    // Glyph of Shadow Ravens
                    else if (player->HasAura(127850))
                        player->RemoveAura(127850);

                    // Instant damage equal to amount of shadow orb
                    int32 damage = GetHitDamage();
                    // First orb is consumed in spell-cast
                    uint8 powerUsed = GetCaster()->GetPower(POWER_SHADOW_ORB) + 1;
                    player->SetPower(POWER_SHADOW_ORB, 0);

                    damage *= powerUsed;
                    SetHitDamage(damage);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_devouring_plague_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_devouring_plague_SpellScript;
        }

        class spell_pri_devouring_plague_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_devouring_plague_AuraScript);

            void CalculatePowerUsed(AuraEffect const * auraEffect, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (!GetCaster())
                    return;

                // First orb is consumed in spell-cast
                const_cast<AuraEffect*>(auraEffect)->SetUserData(GetCaster()->GetPower(POWER_SHADOW_ORB) + 1);
            }

            void OnTick(AuraEffect const *aurEff)
            {
                if (!GetCaster())
                    return;
                // Power is saved in calculation of periodic
                int32 bp = aurEff->GetUserData();
                GetCaster()->CastCustomSpell(GetCaster(), PRIEST_DEVOURING_PLAGUE_HEAL, &bp, NULL, NULL, true);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_devouring_plague_AuraScript::CalculatePowerUsed, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_devouring_plague_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_devouring_plague_AuraScript();
        }
};

// Called by Fade - 586
// Phantasm - 108942
class spell_pri_phantasm : public SpellScriptLoader
{
    public:
        spell_pri_phantasm() : SpellScriptLoader("spell_pri_phantasm") { }

        class spell_pri_phantasm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_phantasm_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PRIEST_PHANTASM_AURA))
                        _player->CastSpell(_player, PRIEST_PHANTASM_PROC, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_phantasm_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_phantasm_SpellScript;
        }
};

// Mind Spike - 73510
class spell_pri_mind_spike : public SpellScriptLoader
{
    public:
        spell_pri_mind_spike() : SpellScriptLoader("spell_pri_mind_spike") { }

        class spell_pri_mind_spike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mind_spike_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // Surge of Darkness - Your next Mind Spike will not consume your damage-over-time effects ...
                        if (!_player->HasAura(PRIEST_SURGE_OF_DARKNESS))
                        {
                            // Mind Spike remove all DoT on the target's
                            if (target->HasAura(PRIEST_SHADOW_WORD_PAIN, _player->GetGUID()))
                                target->RemoveAura(PRIEST_SHADOW_WORD_PAIN, _player->GetGUID());
                            if (target->HasAura(PRIEST_DEVOURING_PLAGUE, _player->GetGUID()))
                                target->RemoveAura(PRIEST_DEVOURING_PLAGUE, _player->GetGUID());
                            if (target->HasAura(PRIEST_VAMPIRIC_TOUCH, _player->GetGUID()))
                                target->RemoveAura(PRIEST_VAMPIRIC_TOUCH, _player->GetGUID());
                        }
                        // ... and deals 50% additional damage.
                        else if (Aura *surgeOfDarkness = _player->GetAura(PRIEST_SURGE_OF_DARKNESS))
                            SetHitDamage(int32(GetHitDamage() * 1.5f));
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_mind_spike_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_mind_spike_SpellScript;
        }
};

// Called by Holy Fire - 14914, Smite - 585 and Penance - 47666
// Evangelism - 81662
class spell_pri_evangelism : public SpellScriptLoader
{
    public:
        spell_pri_evangelism() : SpellScriptLoader("spell_pri_evangelism") { }

        class spell_pri_evangelism_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_evangelism_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PRIEST_EVANGELISM_AURA))
                        if (GetHitDamage())
                            _player->CastSpell(_player, PRIEST_EVANGELISM_STACK, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_evangelism_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_evangelism_SpellScript;
        }
};

// Archangel - 81700
class spell_pri_archangel : public SpellScriptLoader
{
    public:
        spell_pri_archangel() : SpellScriptLoader("spell_pri_archangel") { }

        class spell_pri_archangel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_archangel_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    int stackNumber = _player->GetAura(PRIEST_EVANGELISM_STACK)->GetStackAmount();
                    if (!(stackNumber > 0))
                        return;

                    if (Aura *archangel = _player->GetAura(GetSpellInfo()->Id))
                    {
                        if (archangel->GetEffect(0))
                        {
                            archangel->GetEffect(0)->ChangeAmount(archangel->GetEffect(0)->GetAmount() * stackNumber);
                            _player->RemoveAura(PRIEST_EVANGELISM_STACK);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_archangel_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_archangel_SpellScript;
        }
};

// Cascade - 127630 (damage trigger) or Cascade - 120786 (heal trigger)
class spell_pri_cascade_second : public SpellScriptLoader
{
    public:
        spell_pri_cascade_second() : SpellScriptLoader("spell_pri_cascade_second") { }

        class spell_pri_cascade_second_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_cascade_second_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        std::list<Unit*> checkAuras;
                        std::list<Unit*> targetList;
                        int32 affectedUnits = 0;

                        _player->GetAttackableUnitListInRange(targetList, 40.0f);

                        for (auto itr : targetList)
                        {
                            if (itr->HasAura(PRIEST_CASCADE_INVISIBLE_AURA))
                                if (Unit* caster = itr->GetAura(PRIEST_CASCADE_INVISIBLE_AURA)->GetCaster())
                                    if (caster->GetGUID() == _player->GetGUID())
                                        affectedUnits++;
                        }

                        // Stop the script if the max targets is reached ...
                        if (affectedUnits >= 15)
                            return;

                        if (Aura *boundNumber = _player->GetAura(PRIEST_CASCADE_INVISIBLE_AURA_2))
                            if (boundNumber->GetCharges() >= 4)
                                return;

                        for (auto itr : targetList)
                            checkAuras.push_back(itr);

                        for (auto itr : checkAuras)
                        {
                            if (itr->HasAura(PRIEST_CASCADE_INVISIBLE_AURA))
                                if (Unit* caster = itr->GetAura(PRIEST_CASCADE_INVISIBLE_AURA)->GetCaster())
                                    if (caster->GetGUID() == _player->GetGUID())
                                        targetList.remove(itr);

                            if (!itr->IsWithinLOSInMap(_player))
                                targetList.remove(itr);

                            if (!itr->isInFront(_player))
                                targetList.remove(itr);

                            if (itr->GetGUID() == _player->GetGUID())
                                targetList.remove(itr);

                            // damage
                            if (GetSpellInfo()->Id == 127630)
                                if (!_player->IsValidAttackTarget(itr))
                                    targetList.remove(itr);

                            // heal
                            if (GetSpellInfo()->Id == 120786)
                                if (_player->IsValidAttackTarget(itr))
                                    targetList.remove(itr);
                        }

                        // ... or if there are no targets reachable
                        if (targetList.size() == 0)
                            return;

                        // Each bound hit twice more targets up to 8 for the same bound
                        Trinity::Containers::RandomResizeList(targetList, (affectedUnits * 2));

                        for (auto itr : targetList)
                        {
                            if (_player->HasAura(PRIEST_SHADOWFORM_STANCE))
                            {
                                switch (GetSpellInfo()->Id)
                                {
                                    // damage
                                    case 127630:
                                        target->CastSpell(itr, PRIEST_CASCADE_SHADOW_DAMAGE, true, 0, NULL, _player->GetGUID());
                                        break;
                                    // heal
                                    case 120786:
                                        target->CastSpell(itr, PRIEST_CASCADE_SHADOW_MISSILE, true, 0, NULL, _player->GetGUID());
                                        target->CastSpell(itr, PRIEST_CASCADE_SHADOW_HEAL, true, 0, NULL, _player->GetGUID());
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                            {
                                switch (GetSpellInfo()->Id)
                                {
                                    // damage
                                    case 127630:
                                        target->CastSpell(itr, PRIEST_CASCADE_HOLY_DAMAGE, true, 0, NULL, _player->GetGUID());
                                        break;
                                    // heal
                                    case 120786:
                                        target->CastSpell(itr, PRIEST_CASCADE_HOLY_MISSILE, true, 0, NULL, _player->GetGUID());
                                        target->CastSpell(itr, PRIEST_CASCADE_HOLY_HEAL, true, 0, NULL, _player->GetGUID());
                                        break;
                                    default:
                                        break;
                                }
                            }

                            _player->CastSpell(itr, PRIEST_CASCADE_INVISIBLE_AURA, true);
                        }

                        if (Aura *boundNumber = _player->GetAura(PRIEST_CASCADE_INVISIBLE_AURA_2))
                        {
                            boundNumber->RefreshDuration();
                            boundNumber->SetCharges(boundNumber->GetCharges() + 1);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_cascade_second_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_cascade_second_SpellScript;
        }
};

// Cascade - 120785 (holy damage) or Cascade - 127628 (shadow damage) or Cascade - 127627 (shadow missile) or Cascade - 121146 (holy missile)
class spell_pri_cascade_trigger : public SpellScriptLoader
{
    public:
        spell_pri_cascade_trigger() : SpellScriptLoader("spell_pri_cascade_trigger") { }

        class spell_pri_cascade_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_cascade_trigger_SpellScript);

            void HandleOnHit()
            {
                if (GetOriginalCaster())
                {
                    if (Player* _player = GetOriginalCaster()->ToPlayer())
                    {
                        if (Unit* target = GetHitUnit())
                        {
                            // Trigger for SpellScript
                            if (_player->IsValidAttackTarget(target))
                                _player->CastSpell(target, PRIEST_CASCADE_DAMAGE_TRIGGER, true); // Only damage
                            else
                                _player->CastSpell(target, PRIEST_CASCADE_HOLY_TRIGGER, true); // Only heal
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_cascade_trigger_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_cascade_trigger_SpellScript;
        }
};

// Cascade (shadow) - 127632 and Cascade - 121135
class spell_pri_cascade_first : public SpellScriptLoader
{
    public:
        spell_pri_cascade_first() : SpellScriptLoader("spell_pri_cascade_first") { }

        class spell_pri_cascade_first_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_cascade_first_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        switch (GetSpellInfo()->Id)
                        {
                            case 127632:
                            {
                                // First missile
                                if (_player->IsValidAttackTarget(target))
                                    _player->CastSpell(target, PRIEST_CASCADE_SHADOW_DAMAGE, true, 0, NULL, _player->GetGUID());
                                else
                                {
                                    _player->CastSpell(target, PRIEST_CASCADE_SHADOW_MISSILE, true, 0, NULL, _player->GetGUID());
                                    _player->CastSpell(target, PRIEST_CASCADE_SHADOW_HEAL, true, 0, NULL, _player->GetGUID());
                                }

                                break;
                            }
                            case 121135:
                            {
                                // First missile
                                if (_player->IsValidAttackTarget(target))
                                    _player->CastSpell(target, PRIEST_CASCADE_HOLY_DAMAGE, true, 0, NULL, _player->GetGUID());
                                else
                                {
                                    _player->CastSpell(target, PRIEST_CASCADE_HOLY_MISSILE, true, 0, NULL, _player->GetGUID());
                                    _player->CastSpell(target, PRIEST_CASCADE_HOLY_HEAL, true, 0, NULL, _player->GetGUID());
                                }

                                break;
                            }
                        }

                        // Invisible aura : Each target cannot be hit more than once time [...]
                        _player->CastSpell(target, PRIEST_CASCADE_INVISIBLE_AURA, true);
                        // Invisible aura 2 : [...] or Cascade can bound three times
                        _player->CastSpell(_player, PRIEST_CASCADE_INVISIBLE_AURA_2, true); // First bound

                        if (Aura *boundNumber = _player->GetAura(PRIEST_CASCADE_INVISIBLE_AURA_2))
                            boundNumber->SetCharges(1);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_cascade_first_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_cascade_first_SpellScript;
        }
};

// Halo (shadow) - 120696 and Halo - 120692
class spell_pri_halo_heal : public SpellScriptLoader
{
    public:
        spell_pri_halo_heal() : SpellScriptLoader("spell_pri_halo_heal") { }

        class spell_pri_halo_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_halo_heal_SpellScript);

            void HandleHeal(SpellEffIndex eff)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        int32 heal = GetHitHeal();

                        float Distance = _player->GetDistance(target);
                        float pct = Distance / 25.0f;
                        heal = int32(heal * pct);

                        SetHitHeal(heal);
                    }
                }
            }

            void HandleDamage(SpellEffIndex eff)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        int32 dmg = GetHitDamage();

                        float Distance = _player->GetDistance(target);
                        float pct = Distance / 25.0f;
                        dmg = int32(dmg * pct);

                        SetHitDamage(dmg);
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_halo_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
                OnEffectHitTarget += SpellEffectFn(spell_pri_halo_heal_SpellScript::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_halo_heal_SpellScript;
        }
};

// Inner Fire - 588 or Inner Will - 73413
class spell_pri_inner_fire_or_will : public SpellScriptLoader
{
    public:
        spell_pri_inner_fire_or_will() : SpellScriptLoader("spell_pri_inner_fire_or_will") { }

        class spell_pri_inner_fire_or_will_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_inner_fire_or_will_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_INNER_FIRE) || !sSpellMgr->GetSpellInfo(PRIEST_INNER_WILL))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (GetSpellInfo()->Id == PRIEST_INNER_FIRE)
                    {
                        if (_player->HasAura(PRIEST_INNER_WILL))
                            _player->RemoveAura(PRIEST_INNER_WILL);
                    }
                    else if (GetSpellInfo()->Id == PRIEST_INNER_WILL)
                    {
                        if (_player->HasAura(PRIEST_INNER_FIRE))
                            _player->RemoveAura(PRIEST_INNER_FIRE);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_inner_fire_or_will_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_inner_fire_or_will_SpellScript;
        }
};

// Leap of Faith - 73325 and Leap of Faith - 110718 (Symbiosis)
class spell_pri_leap_of_faith : public SpellScriptLoader
{
    public:
        spell_pri_leap_of_faith() : SpellScriptLoader("spell_pri_leap_of_faith") { }

        class spell_pri_leap_of_faith_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_leap_of_faith_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        target->CastSpell(_player, PRIEST_LEAP_OF_FAITH_JUMP, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_leap_of_faith_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_leap_of_faith_SpellScript;
        }
};

// Void Shift - 108968
class spell_pri_void_shift : public SpellScriptLoader
{
    public:
        spell_pri_void_shift() : SpellScriptLoader("spell_pri_void_shift") { }

        class spell_pri_void_shift_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_void_shift_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_VOID_SHIFT))
                    return false;
                return true;
            }

            SpellCastResult CheckTarget()
            {
                if (GetExplTargetUnit())
                    if (GetExplTargetUnit()->GetTypeId() != TYPEID_PLAYER)
                        return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        float playerPct;
                        float targetPct;

                        playerPct = _player->GetHealthPct();
                        targetPct = target->GetHealthPct();

                        if (playerPct < 25.0f)
                            playerPct = 25.0f;
                        if (targetPct < 25.0f)
                            targetPct = 25.0f;

                        playerPct /= 100.0f;
                        targetPct /= 100.0f;

                        _player->SetHealth(_player->GetMaxHealth() * targetPct);
                        target->SetHealth(target->GetMaxHealth() * playerPct);
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pri_void_shift_SpellScript::CheckTarget);
                OnEffectHitTarget += SpellEffectFn(spell_pri_void_shift_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_void_shift_SpellScript;
        }
};

// Psychic Horror - 64044
class spell_pri_psychic_horror : public SpellScriptLoader
{
    public:
        spell_pri_psychic_horror() : SpellScriptLoader("spell_pri_psychic_horror") { }

        class spell_pri_psychic_horror_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_psychic_horror_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (caster->ToPlayer() && caster->ToPlayer()->GetSpecializationId(caster->ToPlayer()->GetActiveSpec()) == SPEC_PRIEST_SHADOW)
                        {
                            int32 currentPower = caster->GetPower(POWER_SHADOW_ORB);
                            caster->ModifyPower(POWER_SHADOW_ORB, -currentPower);

                            // +1s per Shadow Orb consumed
                            if (Aura *psychicHorror = target->GetAura(64044))
                            {
                                int32 maxDuration = psychicHorror->GetMaxDuration();
                                int32 newDuration = maxDuration + currentPower * IN_MILLISECONDS;
                                psychicHorror->SetDuration(newDuration);

                                if (newDuration > maxDuration)
                                    psychicHorror->SetMaxDuration(newDuration);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_psychic_horror_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_psychic_horror_SpellScript();
        }
};

// Guardian Spirit - 47788
class spell_pri_guardian_spirit : public SpellScriptLoader
{
    public:
        spell_pri_guardian_spirit() : SpellScriptLoader("spell_pri_guardian_spirit") { }

        class spell_pri_guardian_spirit_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_guardian_spirit_AuraScript);

            uint32 healPct;

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_SPELL_GUARDIAN_SPIRIT_HEAL))
                    return false;
                return true;
            }

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                return true;
            }

            void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect * /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* target = GetTarget();
                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                int32 healAmount = int32(target->CountPctFromMaxHealth(healPct));
                // remove the aura now, we don't want 40% healing bonus
                Remove(AURA_REMOVE_BY_ENEMY_SPELL);
                target->CastCustomSpell(target, PRIEST_SPELL_GUARDIAN_SPIRIT_HEAL, &healAmount, NULL, NULL, true);
                absorbAmount = dmgInfo.GetDamage();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_guardian_spirit_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_guardian_spirit_AuraScript::Absorb, EFFECT_2);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_guardian_spirit_AuraScript();
        }
};

// Penance - 47540
class spell_pri_penance : public SpellScriptLoader
{
    public:
        spell_pri_penance() : SpellScriptLoader("spell_pri_penance") { }

        class spell_pri_penance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_penance_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* spellEntry)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_SPELL_PENANCE))
                    return false;
                // can't use other spell than this penance due to spell_ranks dependency
                if (sSpellMgr->GetFirstSpellInChain(PRIEST_SPELL_PENANCE) != sSpellMgr->GetFirstSpellInChain(spellEntry->Id))
                    return false;

                uint8 rank = sSpellMgr->GetSpellRank(spellEntry->Id);
                if (!sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_DAMAGE, rank, true))
                    return false;
                if (!sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_HEAL, rank, true))
                    return false;

                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        if (!unitTarget->IsAlive())
                            return;

                        uint8 rank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);

                        if (_player->IsFriendlyTo(unitTarget))
                            _player->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_HEAL, rank), false, 0);
                        else
                            _player->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_DAMAGE, rank), false, 0);

                        // Divine Insight (Discipline)
                        if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_PRIEST_DISCIPLINE)
                            if (_player->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_TALENT))
                                _player->CastSpell(_player, PRIEST_SPELL_DIVINE_INSIGHT_DISCIPLINE, true);
                    }
                }
            }

            SpellCastResult CheckCast()
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Unit* target = GetExplTargetUnit())
                    if (!caster->IsFriendlyTo(target) && !caster->IsValidAttackTarget(target))
                        return SPELL_FAILED_BAD_TARGETS;
                return SPELL_CAST_OK;
            }

            void Register()
            {
                // add dummy effect spell handler to Penance
                OnEffectHitTarget += SpellEffectFn(spell_pri_penance_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_pri_penance_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_penance_SpellScript;
        }
};

enum PrayerOfMending
{
    SPELL_T9_HEALING_2_PIECE = 67201,
};

// Prayer of Mending Heal
class spell_pri_prayer_of_mending_heal : public SpellScriptLoader
{
    public:
        spell_pri_prayer_of_mending_heal() : SpellScriptLoader("spell_pri_prayer_of_mending_heal") { }

        class spell_pri_prayer_of_mending_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_prayer_of_mending_heal_SpellScript);

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetOriginalCaster())
                {
                    if (AuraEffect *aurEff = caster->GetAuraEffect(SPELL_T9_HEALING_2_PIECE, EFFECT_0))
                    {
                        int32 heal = GetHitHeal();
                        AddPct(heal, aurEff->GetAmount());
                        SetHitHeal(heal);
                    }
                }

            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_prayer_of_mending_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_prayer_of_mending_heal_SpellScript();
        }
};

// Vampiric Touch - 34914
class spell_pri_vampiric_touch : public SpellScriptLoader
{
    public:
        spell_pri_vampiric_touch() : SpellScriptLoader("spell_pri_vampiric_touch") { }

        class spell_pri_vampiric_touch_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_vampiric_touch_AuraScript);

            void OnTick(AuraEffect const * /*aurEff*/)
            {
                if (GetCaster())
                {
                    GetCaster()->EnergizeBySpell(GetCaster(), GetSpellInfo()->Id, GetCaster()->CountPctFromMaxMana(2), POWER_MANA);

                    // From Darkness, Comes Light
                    if (GetCaster()->HasAura(PRIEST_FROM_DARKNESS_COMES_LIGHT_AURA))
                        if (roll_chance_i(15))
                            GetCaster()->CastSpell(GetCaster(), PRIEST_SURGE_OF_DARKNESS, true);
                }
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* dispeller = dispelInfo->GetDispeller())
                        if (caster->HasAura(PRIEST_SPELL_4P_S12_SHADOW))
                            dispeller->CastSpell(dispeller, PRIEST_SPELL_SIN_AND_PUNISHMENT, true, 0, NULL, caster->GetGUID());
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_vampiric_touch_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
                AfterDispel += AuraDispelFn(spell_pri_vampiric_touch_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_vampiric_touch_AuraScript();
        }
};

// Called by Shadow Form - 15473
// Glyph of Shadow - 107906
class spell_pri_shadowform : public SpellScriptLoader
{
    public:
        spell_pri_shadowform() : SpellScriptLoader("spell_pri_shadowform") { }

        class spell_pri_shadowform_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_shadowform_AuraScript);

            bool Validate(SpellInfo const* /*entry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH) ||
                    !sSpellMgr->GetSpellInfo(PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH))
                    return false;
                return true;
            }

            void HandleEffectApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->CastSpell(GetTarget(), GetTarget()->HasAura(PRIEST_GLYPH_OF_SHADOW) ? PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH : PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH, true);
            }

            void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(GetTarget()->HasAura(PRIEST_GLYPH_OF_SHADOW) ? PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH : PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pri_shadowform_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pri_shadowform_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_shadowform_AuraScript();
        }
};

// Levitate - 1706
class spell_pri_levitate : public SpellScriptLoader
{
    public:
        spell_pri_levitate() : SpellScriptLoader("spell_pri_levitate") { }

        class spell_pri_levitate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_levitate_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (GetCaster())
                    if (GetHitUnit())
                        GetCaster()->CastSpell(GetHitUnit(), PRIEST_SPELL_LEVITATE, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_levitate_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_levitate_SpellScript;
        }
};

class spell_pri_binding_heal : public SpellScriptLoader
{
public:
    spell_pri_binding_heal() : SpellScriptLoader("spell_pri_binding_heal") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { SPELL_GLYPH_OF_BINDING_HEAL = 63248 };

        void RemoveInvalidTargets(std::list<WorldObject*>& targets)
        {
            auto explicitTarget = GetExplTargetUnit();
            if (!GetCaster()->HasAura(SPELL_GLYPH_OF_BINDING_HEAL))
            {
                targets.clear();
                targets.push_back(explicitTarget);
                return;
            }

            targets.remove_if([explicitTarget](WorldObject const *obj) {
                return !obj->isType(TYPEMASK_UNIT) || obj == explicitTarget;
            });

            targets.sort([] ( WorldObject * a, WorldObject * b)
            {
                return (a->ToUnit()->GetHealth() < b->ToUnit()->GetHealth());
            });

            if (targets.size() > 1)
                targets.resize(1);
            targets.push_back(GetExplTargetUnit());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// 47515 - Divine Aegis
class spell_pri_divine_aegis : public SpellScriptLoader
{
public:
    spell_pri_divine_aegis() : SpellScriptLoader("spell_pri_divine_aegis") { }

    class spell_pri_divine_aegis_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_divine_aegis_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_DIVINE_AEGIS))
                return false;
            return true;
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetProcTarget();
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            if (!eventInfo.GetActor() || !eventInfo.GetSpellInfo() || (eventInfo.GetHitMask() & PROC_EX_NORMAL_HIT))
                return;

            Unit * caster = GetTarget();

            int32 absorb = CalculatePct(int32(eventInfo.GetHealInfo()->GetHeal()), aurEff->GetAmount());

            // Multiple effects stack, so let's try to find this aura.
            if (AuraEffect const* aegis = eventInfo.GetProcTarget()->GetAuraEffect(SPELL_PRIEST_DIVINE_AEGIS, EFFECT_0, eventInfo.GetActor()->GetGUID()))
                absorb += aegis->GetAmount();

            absorb = std::min<int32>(absorb, eventInfo.GetActor()->CountPctFromMaxHealth(60));

            GetTarget()->CastCustomSpell(SPELL_PRIEST_DIVINE_AEGIS, SPELLVALUE_BASE_POINT0, absorb, eventInfo.GetProcTarget(), true, NULL, aurEff, eventInfo.GetActor()->GetGUID());
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_pri_divine_aegis_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_pri_divine_aegis_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pri_divine_aegis_AuraScript();
    }
};

// 139 - Renew
class spell_pri_renew : public SpellScriptLoader
{
public:
    spell_pri_renew() : SpellScriptLoader("spell_priest_renew") { }

    class spell_pri_renew_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_renew_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_RAPID_RENEWAL_HEAL))
                return false;
            return true;
        }

        bool Load()
        {
            return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                // Rapid Renewal
                if (AuraEffect const* empoweredRenewAurEff = caster->GetAuraEffect(SPELL_PRIEST_RAPID_RENEWAL, EFFECT_2))
                {
                    uint32 heal = caster->SpellHealingBonusDone(GetTarget(), GetSpellInfo(), aurEff->GetEffIndex(), aurEff->GetAmount(), DOT);
                    heal = GetTarget()->SpellHealingBonusTaken(caster, GetSpellInfo(), aurEff->GetEffIndex(), heal, DOT);
                    int32 basepoints0 = CalculatePct(int32(heal) * aurEff->GetTotalTicks(), empoweredRenewAurEff->GetAmount());
                    caster->CastCustomSpell(GetTarget(), SPELL_PRIEST_RAPID_RENEWAL_HEAL, &basepoints0, NULL, NULL, true, NULL, aurEff);
                }
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_pri_renew_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_renew_AuraScript();
    }
};

// 17 - Power Word: Shield, 123258 - Power Word : Shield (Divine Insight)
class spell_pri_power_word_shield : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            GLYPH_OF_REFLECTIVE_SHIELD  = 33202,
            REFLECTIVE_SHIELD_TRIGGERED = 33619,

            RAPTURE_TALENT              = 47536,
            RAPTURE_ENERGIZE            = 47755,
            RAPTURE_MARKER              = 63853,

            MASTERY_SPELL_DISCIPLINE_SHIELD     = 77484,
        };

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return sSpellMgr->GetSpellInfo(REFLECTIVE_SHIELD_TRIGGERED)
                && sSpellMgr->GetSpellInfo(GLYPH_OF_REFLECTIVE_SHIELD);
        }

        void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit * const caster = GetCaster();
            if (caster && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
            {
                // Rapture
                if (AuraEffect const * const raptureEff = caster->GetAuraEffectOfRankedSpell(RAPTURE_TALENT, EFFECT_0))
                {
                    if (!caster->HasAura(RAPTURE_MARKER))
                    {
                        int32 basepoints0 = this->GetSpellInfo()->CalcPowerCost(caster, GetSpellInfo()->GetSchoolMask(), &GetSpellInfo()->spellPower, false);
                        // Priest T13 Healer 4P Bonus - Increase mana granted from Rapture by 100%
                        if (AuraEffect const * const eff = caster->GetAuraEffect(105832, EFFECT_1))
                            AddPct(basepoints0, eff->GetAmount());

                        caster->CastCustomSpell(caster, RAPTURE_ENERGIZE, &basepoints0, NULL, NULL, true);
                        caster->CastSpell(caster, RAPTURE_MARKER, true);
                    }
                }
            }
        }

        void ReflectDamage(AuraEffect *aurEff, DamageInfo &dmgInfo, uint32 &absorbAmount)
        {
            Unit * const target = GetTarget();
            Unit * const attacker = dmgInfo.GetAttacker();

            if (!attacker || attacker == target || (dmgInfo.GetSpellInfo() && dmgInfo.GetSpellInfo()->Id == REFLECTIVE_SHIELD_TRIGGERED))
                return;

            Unit * const caster = GetCaster();
            if (!caster)
                return;

            if (AuraEffect const * talentAurEff = caster->GetAuraEffect(GLYPH_OF_REFLECTIVE_SHIELD, EFFECT_0))
            {
                int32 bp = CalculatePct(absorbAmount, talentAurEff->GetAmount());
                target->CastCustomSpell(attacker, REFLECTIVE_SHIELD_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
            }
        }

        void CalculateAmount(AuraEffect const *, int32 & amount, bool & )
        {
            Unit * const caster = GetCaster();
            if (!caster)
                return;

            if (Player const * const player = caster->ToPlayer())
            {
                // Divine Aegis
                if (AuraEffect const * const aegis = player->GetAuraEffect(47515, EFFECT_0))
                    if (roll_chance_f(player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + GetFirstSchoolInMask(GetSpellInfo()->GetSchoolMask()))))
                        amount *= 2;
            }
        }

        void Register()
        {
            AfterEffectAbsorb += AuraEffectAbsorbFn(script_impl::ReflectDamage, EFFECT_0);
            AfterEffectRemove += AuraEffectRemoveFn(script_impl::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        }
    };

public:
    spell_pri_power_word_shield()
        : SpellScriptLoader("spell_pri_power_word_shield")
    { }

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

class npc_pri_divine_star : public CreatureScript
{
public:
    npc_pri_divine_star() : CreatureScript("npc_pri_divine_star") { }

    struct npc_pri_divine_starAI : public PassiveAI
    {
        npc_pri_divine_starAI(Creature* c) : PassiveAI(c) { }

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        std::set<uint64> alreadyHit;

        void IsSummonedBy(Unit* owner)
        {
            if (owner && owner->GetTypeId() == TYPEID_PLAYER)
            {
                me->SetLevel(owner->getLevel());
                me->setFaction(owner->getFaction());
                me->SetOwnerGUID(owner->GetGUID());
            }
            else
                me->DespawnOrUnsummon();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                if (Player * const owner = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                {
                    // Move back to owner
                    alreadyHit.clear();
                    me->CastSpell(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), me->HasAura(58880) ? 58880 : 122127, true);
                    me->GetMotionMaster()->MoveFollow(owner, 0, M_PI);
                }
            }
        }

    public:
        std::set<uint64> & getHitTargets() { return alreadyHit; }

    };

    CreatureAI* GetAI(Creature *creature) const
    {
        return new npc_pri_divine_starAI(creature);
    }
};

// 58880, 122127 - Divine Star dummy
class spell_pri_divine_star_dummy : public SpellScriptLoader
{
public:
    spell_pri_divine_star_dummy() : SpellScriptLoader("spell_pri_divine_star_dummy") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum divineStar
        {
            NPC_DIVINE_STAR         = 73692,
        };

        void HandleDummy(SpellEffIndex f)
        {
            if (Unit* caster = GetCaster())
            {
                Position casterPos;
                Position destPos;
                caster->GetPosition(&destPos);
                caster->GetPosition(&casterPos);

                caster->MovePosition(destPos, 25.f, caster->GetTypeId() == TYPEID_PLAYER ? 0 : M_PI);

                // needed for NPC height increase
                destPos.m_positionZ += 1.5;

                const_cast<WorldLocation*>(GetExplTargetDest())->Relocate(destPos);
                GetHitDest()->Relocate(destPos);

                // Triggered dummy is casted back by creature - don't spawn another star
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

               if (Creature * const divineStar = caster->SummonCreature(NPC_DIVINE_STAR, casterPos, TEMPSUMMON_TIMED_DESPAWN, 5000))
               {
                   caster->AddAura(GetSpellInfo()->Id == 58880 ? 110744 : 122121, divineStar);
                   // Move ~24 yards forward call point-reach and move back to caster
                   divineStar->SetSpeed(MOVE_RUN, 2.7f, true);
                   divineStar->SetSpeed(MOVE_FLIGHT, 2.7f, true);
                   divineStar->GetMotionMaster()->MovePoint(1, destPos);
               }
            }
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(script_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// 110745, 122128 - Divine Star damage and heal
class spell_pri_divine_star_damage_heal : public SpellScriptLoader
{
public:
    spell_pri_divine_star_damage_heal() : SpellScriptLoader("spell_pri_divine_star_damage_heal") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum divineStar
        {
            NPC_DIVINE_STAR         = 73692,
        };

        void RemoveInvalidTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            Creature * divineStar = GetCaster()->ToCreature();
            if (!divineStar)
            {
                targets.clear();
                return;
            }

            // Remove self
            targets.remove(divineStar);

            if (npc_pri_divine_star::npc_pri_divine_starAI * divineAI = CAST_AI(npc_pri_divine_star::npc_pri_divine_starAI, divineStar->GetAI()))
            {
                auto & proceeded = divineAI->getHitTargets();
                if (!proceeded.empty())
                    targets.remove_if([&proceeded](WorldObject *u) { return proceeded.find(u->GetGUID()) != proceeded.end(); });

                for (auto const obj : targets)
                    proceeded.insert(obj->GetGUID());
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Mind Flay - 15407, 129197
class spell_pri_mind_flay final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            GLYPH_OF_MIND_FLAY          = 120585,
        };

        void initEffects(uint32 &effectMask)
        {
            // Glyph of Mind Flay - Remove slow effect
            auto const caster = GetCaster();
            if (caster && caster->HasAura(GLYPH_OF_MIND_FLAY))
                effectMask &= ~(1 << EFFECT_1);
        }

        void onTick(AuraEffect const * /*aurEff*/)
        {
            if (auto caster = GetCaster())
                if (caster->HasAura(GLYPH_OF_MIND_FLAY))
                    caster->CastSpell(caster, 120587, true);
        }

        void Register() final
        {
            OnInitEffects += AuraInitEffectsFn(script_impl::initEffects);
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::onTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

public:
    spell_pri_mind_flay()
        : SpellScriptLoader("spell_pri_mind_flay")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

// 129176 - Shadow Word Death (Glyphed)
class spell_pri_shadow_word_death_glyphed : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            SPELL_SHADOW_WORD_DEATH                 = 32379,
            SPELL_GLYPH_OF_SHADOW_WORD_DEATH        = 55682,
            SPELL_GLYPH_OF_SHADOW_WORD_DEATH_MARKER = 95652,
            SPELL_BACKFIRE_DAMAGE                   = 32409,
        };

        bool m_damageIncreased;

    public:
        script_impl()
            : m_damageIncreased(false)
        { }

    private:
        void HandleBeforeHit()
        {
            Unit const * const caster = GetCaster();
            Unit const * const target = GetHitUnit();
            if (!caster || !target)
                return;

            if (target->HealthBelowPct(20))
            {
                m_damageIncreased = true;
                SetHitDamage(GetHitDamage() * 4.0f);
            }
        }

        void HandleAfterHit()
        {
            Unit * const caster = GetCaster();
            Unit * const target = GetHitUnit();
            if (!caster || !target || !target->IsAlive() || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            int32 backfireDamage = GetHitDamage();

            // Priest T13 Shadow 2P Bonus
            if (AuraEffect const * const eff = caster->GetAuraEffect(105843, EFFECT_1))
                backfireDamage -= CalculatePct(backfireDamage, eff->GetAmount());

            caster->CastCustomSpell(SPELL_BACKFIRE_DAMAGE, SPELLVALUE_BASE_POINT0, backfireDamage, caster, true);

            if (m_damageIncreased && !caster->HasAura(SPELL_GLYPH_OF_SHADOW_WORD_DEATH_MARKER))
            {
                caster->ToPlayer()->RemoveSpellCooldown(SPELL_SHADOW_WORD_DEATH, true);
                caster->CastSpell(caster, SPELL_GLYPH_OF_SHADOW_WORD_DEATH_MARKER, true);
            }
        }

        void Register()
        {
            BeforeHit += SpellHitFn(script_impl::HandleBeforeHit);
            AfterHit += SpellHitFn(script_impl::HandleAfterHit);
        }
    };

public:
    spell_pri_shadow_word_death_glyphed()
        : SpellScriptLoader("spell_pri_shadow_word_death_glyphed")
    { }

    SpellScript * GetSpellScript() const
    {
        return new script_impl;
    }
};

// 32592 - Mass Dispel
class spell_pri_mass_dispel : public SpellScriptLoader
{
    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        bool _hasImmunity;

        bool Load()
        {
            _hasImmunity = false;
            return true;
        }

        void CheckAuras()
        {
            // Check if Immunity aura is on target
            if (Unit* target = GetHitUnit())
                if (target->HasAuraWithMechanic(1 << MECHANIC_IMMUNE_SHIELD))
                    _hasImmunity = true;
        }

        void HandleScript(SpellEffIndex effIndex)
        {
            // Glyph of Mass Dispel - Prevent dispel effect when target has immunity and no glyph is applied
            if (_hasImmunity)
                if (!GetCaster()->HasAura(55691))
                    PreventHitEffect(effIndex);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleScript, EFFECT_0, SPELL_EFFECT_DISPEL);
            BeforeHit += SpellHitFn(spell_impl::CheckAuras);
        }
    };

public:
    spell_pri_mass_dispel() : SpellScriptLoader("spell_pri_mass_dispel") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

// Mind Blast - 8092
class spell_pri_mind_blast : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            SPELL_DIVINE_INSIGHT_PROC = 124430,
        };

        void HandleAfterHit()
        {
            Unit * const caster = GetCaster();
            if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            // Handle Divine Insight if it procced while casting current Mind Blast
            Spell::UsedSpellMods const &mods = appliedSpellMods();
            AuraEffect * const aurEff = caster->GetAuraEffect(SPELL_DIVINE_INSIGHT_PROC, EFFECT_0);

            if (!aurEff || mods.find(aurEff->GetSpellModifier()) != mods.end())
                return;

            caster->ToPlayer()->RemoveSpellCooldown(GetSpellInfo()->Id, true);
        }

        void Register()
        {
            AfterHit += SpellHitFn(script_impl::HandleAfterHit);
        }
    };

public:
    spell_pri_mind_blast() : SpellScriptLoader("spell_pri_mind_blast")
    { }

    SpellScript * GetSpellScript() const
    {
        return new script_impl();
    }
};

// Confession - 126123
class spell_pri_confession : public SpellScriptLoader
{
public:
    spell_pri_confession() : SpellScriptLoader("spell_pri_confession") { }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            std::string confessions[40] =
            {
                "For a long time, I thought the plural of anecdote WAS data.",
                "I always forget to gem my gear.",
                "I always wanted to be a paladin.",
                "I ask for the Light to give me strength, but I'm not sure it really does.",
                "I asked a friend for gold to buy my first mount.",
                "I dabble in archaeology, but I'm just not that interested in history.",
                "I died to an elevator once.Maybe more than once.",
                "I don't know if Milhouse is a good guy or not.",
                "I don't really have a clue who the Sin'dorei are.",
                "I don't really remember you in the mountains.",
                "I don't treat all of my mounts equally.",
                "I fell off of Dalaran.",
                "I find all these names with so many apostrophes so confusing.",
                "I forgot the Sunwell.",
                "I go into dungeons not to make Azeroth a better place, but just for loot.",
                "I have \"borrowed\" things from my guild bank.",
                "I have stood in the fire.",
                "I haven't been in a barber shop in months. Goblins with scissors. Shudder.",
                "I know he's a jerk, but there's something about Garrosh...",
                "I light things on fire and yell BY FIRE BE PURGED when nobody is looking.",
                "I never use the lightwell.",
                "I once punched a gnome.No reason.I was just having a bad day.",
                "I once took a bow that a hunter wanted.",
                "I outbid a friend on an auction for something I didn't really want.",
                "I really wasn't prepared. Who knew?",
                "I said I had been in the dungeon before, but i had no idea what I was doing.It was embarassing.",
                "I saw a mage cast a spell once and my jaw really did drop at the damage.",
                "I sometimes forget if Northrend is north or south of here.",
                "I sometimes use my mount to travel really short distances.I mean REALLY short.",
                "I sometimes wonder if tauren taste like... you know.",
                "I spent six months chasing the Time - Lost Proto - Drake.",
                "I thought pandaren were a type of furbolg.",
                "I told my raid leader that I was ready, but I wasn't really ready.",
                "I wasn't really at the opening of Ahn'Qiraj, I just read about it. (thanks Stonehearth)",
                "I went into Alterac Valley and didn't help my team at all.",
                "Oh, I took the candle.",
                "Sometimes I ask for a warlock to summon me when I'm really not that far away.",
                "Sometimes when I'm questing, I want to be alone, so I pretend I can't hear my friends.",
                "This is just a setback.",
                "Troll toes sort of creep me out."
            };
            auto target = GetHitUnit();
            if (GetCaster() != target)
                target->MonsterTextEmote(target->GetName() + " confesses: " + confessions[urand(0, 39)], 0);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl;
    }
};

class spell_pri_echo_of_light : public SpellScriptLoader
{
    public:
        spell_pri_echo_of_light() : SpellScriptLoader("spell_pri_echo_of_light") { }


        class spell_pri_echo_of_light_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_echo_of_light_AuraScript);

            void HandleProc(ProcEventInfo& eventInfo)
            {
                Unit* target = eventInfo.GetActionTarget();
                Unit* caster = GetCaster();
                int32 heal = eventInfo.GetHealInfo()->GetHeal();
                if (!caster || !target || !heal)
                    return;
                
                if (Aura* aur = GetAura())
                {
                    if (AuraEffect* eff = aur->GetEffect(0))
                    {
                        int32 amount = heal * eff->GetFloatAmount() / 100;
                        amount /= 6; // tick count

                        auto const remaining = target->GetRemainingPeriodicAmount(caster->GetGUID(), 77489, SPELL_AURA_PERIODIC_HEAL);
                        amount += remaining.perTick();

                        if (amount)
                            caster->CastCustomSpell(target, SPELL_PRIEST_ECHO_OF_LIGHT_HEAL, &amount, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                OnProc += AuraProcFn(spell_pri_echo_of_light_AuraScript::HandleProc);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_echo_of_light_AuraScript();
        }
};

// Halo (120517, 120644)
class spell_pri_halo : public SpellScriptLoader
{
    public:
        spell_pri_halo() : SpellScriptLoader("spell_pri_halo") { }

        enum eAreatrigger
        {
            HALO_AREATRIGGER_SHADOW = 657,
            HALO_AREATRIGGER_HOLY = 658
        };
        
        enum eSpells
        {
            HALO_SPELL_HOLY = 120692,
            HALO_SPELL_SHADOW = 120696
        };

        class script_impl : public SpellScript
        {
            PrepareSpellScript(script_impl);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                GetCaster()->CastSpell(GetCaster(), GetSpellInfo()->Effects[EFFECT_0].MiscValue == HALO_AREATRIGGER_HOLY ? HALO_SPELL_HOLY : HALO_SPELL_SHADOW);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(script_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new script_impl;
        }
};

// Called by Prayer of Mending - 33076
class spell_pri_4p_holy_spark : public SpellScriptLoader
{
    public:
        spell_pri_4p_holy_spark() : SpellScriptLoader("spell_pri_4p_holy_spark") { }

        enum Spells
        {
            PRIEST_SPELL_HOLY_SPARK = 131567,
            PRIEST_SPELL_FLASH_HEAL = 2061,
            PRIEST_SPELL_GREATER_HEAL = 2060,
            PRIEST_SPELL_HOLY_WORD_SERENITY = 88684,
            PRIEST_SPELL_P_F_M = 33076,
            PRIEST_SPELL_HOLY_SPARK_PASSIVE = 33333
        };

        class spell_impl : public SpellScript
        {
            PrepareSpellScript(spell_impl);

            void HandleDrop(SpellEffIndex /*effIndex*/)
            {
                if (GetSpellInfo()->Id != PRIEST_SPELL_FLASH_HEAL 
                    && GetSpellInfo()->Id != PRIEST_SPELL_GREATER_HEAL 
                    && GetSpellInfo()->Id != PRIEST_SPELL_HOLY_WORD_SERENITY)
                    return;

                if (Aura* aura = GetHitUnit()->GetAura(PRIEST_SPELL_HOLY_SPARK))
                    aura->DropCharge();
            }

            void HandleSpark(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster()->HasAura(PRIEST_SPELL_HOLY_SPARK_PASSIVE))
                    return;

                if (GetSpellInfo()->Id != PRIEST_SPELL_P_F_M)
                    return;

                GetCaster()->CastSpell(GetHitUnit(), PRIEST_SPELL_HOLY_SPARK, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_impl::HandleDrop, EFFECT_0, SPELL_EFFECT_HEAL);
                OnEffectHitTarget += SpellEffectFn(spell_impl::HandleSpark, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_impl;
        }
};

void AddSC_priest_spell_scripts()
{
    new spell_pri_power_word_fortitude();
    new spell_pri_spectral_guise_charges();
    new spell_pri_void_tendrils();
    new spell_pri_phantasm_proc();
    new spell_pri_spirit_of_redemption_form();
    new spell_pri_spirit_of_redemption();
    new spell_pri_item_s12_4p_heal();
    new spell_pri_item_s12_2p_shadow();
    new spell_pri_divine_insight_shadow();
    new spell_pri_power_word_insanity();
    new spell_pri_power_word_solace();
    new spell_pri_shadow_word_insanity_allowing();
    new spell_pri_shadowfiend();
    new spell_pri_from_darkness_comes_light();
    new spell_pri_body_and_soul();
    new spell_pri_prayer_of_mending_divine_insight();
    new spell_pri_divine_insight_holy();
    new spell_pri_divine_insight_discipline();
    new spell_pri_holy_word_sanctuary();
    new spell_pri_chakra_chastise();
    new spell_pri_lightwell_renew();
    new spell_pri_strength_of_soul();
    new spell_pri_grace();
    new spell_pri_atonement();
    new spell_pri_spirit_shell();
    new spell_pri_devouring_plague();
    new spell_pri_phantasm();
    new spell_pri_mind_spike();
    new spell_pri_cascade_second();
    new spell_pri_cascade_trigger();
    new spell_pri_cascade_first();
    new spell_pri_halo_heal();
    new spell_pri_inner_fire_or_will();
    new spell_pri_leap_of_faith();
    new spell_pri_void_shift();
    new spell_pri_psychic_horror();
    new spell_pri_guardian_spirit();
    new spell_pri_penance();
    new spell_pri_prayer_of_mending_heal();
    new spell_pri_vampiric_touch();
    new spell_pri_renew();
    new spell_pri_shadowform();
    new spell_pri_evangelism();
    new spell_pri_archangel();
    new spell_pri_levitate();
    new spell_pri_binding_heal();
    new spell_pri_divine_aegis();
    new spell_pri_power_word_shield();
    new spell_pri_divine_star_dummy();
    new npc_pri_divine_star();
    new spell_pri_divine_star_damage_heal();
    new spell_pri_mind_flay();
    new spell_pri_shadow_word_death_glyphed();
    new spell_pri_mass_dispel();
    new spell_pri_mind_blast();
    new spell_pri_confession();
    new spell_pri_echo_of_light();
    new spell_pri_psychic_terror();
    new spell_pri_halo();
    new spell_pri_4p_holy_spark();
}
