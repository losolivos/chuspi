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
 * Scripts for spells realted to Cooking and Banquets
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_cooking_"
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SkillDiscovery.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "Group.h"
#include "LFGMgr.h"

// Called By Seafood Magnifique Feast - 87806
// Seafood Magnifique Feast - 87644
class spell_cooking_seafood_magnifique_feast : public SpellScriptLoader
{
public: 
    spell_cooking_seafood_magnifique_feast() : SpellScriptLoader("spell_cooking_seafood_magnifique_feast") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH_AND_STAMINA = 87545, // 89 strength and 89 stamina
            SPELL_STAT_AGILITY_AND_STAMINA = 87546, // 89 agility and 89 stamina
            SPELL_STAT_INTELECT_AND_STAMINA = 87547, // 89 intelect and 89 stamina
            SPELL_STAT_SPIRIT_AND_STAMINA = 87548, // 89 spirit and 89 stamina
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH_AND_STAMINA;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH_AND_STAMINA;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY_AND_STAMINA;
                        break;
                    case STAT_STAMINA:
                    {
                        switch (player->GetSpecializationId(player->GetActiveSpec()))
                        {
                           case SPEC_DRUID_GUARDIAN:
                           case SPEC_MONK_BREWMASTER:
                               spellId = SPELL_STAT_AGILITY_AND_STAMINA;
                               break;
                           case SPEC_PALADIN_PROTECTION:
                           case SPEC_WARRIOR_PROTECTION:
                           case SPEC_DK_BLOOD:
                               spellId = SPELL_STAT_STRENGTH_AND_STAMINA;
                               break;
                           default:
                               spellId = SPELL_STAT_STRENGTH_AND_STAMINA;
                               break;
                        }
                    }
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT_AND_STAMINA;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT_AND_STAMINA;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH_AND_STAMINA;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

// Called By Broiled Dragon Feast - 87763
// Broiled Dragon Feast - 87643
class spell_cooking_broiled_dragon_feast : public SpellScriptLoader
{
public: 
    spell_cooking_broiled_dragon_feast() : SpellScriptLoader("spell_cooking_broiled_dragon_feast") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH_AND_STAMINA = 87556, // 59 strength and 59 stamina
            SPELL_STAT_AGILITY_AND_STAMINA = 87557, // 59 agility and 59 stamina
            SPELL_STAT_INTELECT_AND_STAMINA = 87558, // 59 intelect and 59 stamina
            SPELL_STAT_SPIRIT_AND_STAMINA = 87559, // 59 spirit and 59 stamina
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH_AND_STAMINA;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH_AND_STAMINA;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY_AND_STAMINA;
                        break;
                    case STAT_STAMINA:
                    {
                        switch (player->GetSpecializationId(player->GetActiveSpec()))
                        {
                           case SPEC_DRUID_GUARDIAN:
                           case SPEC_MONK_BREWMASTER:
                               spellId = SPELL_STAT_AGILITY_AND_STAMINA;
                               break;
                           case SPEC_PALADIN_PROTECTION:
                           case SPEC_WARRIOR_PROTECTION:
                           case SPEC_DK_BLOOD:
                               spellId = SPELL_STAT_STRENGTH_AND_STAMINA;
                               break;
                           default:
                               spellId = SPELL_STAT_STRENGTH_AND_STAMINA;
                               break;
                        }
                    }
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT_AND_STAMINA;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT_AND_STAMINA;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH_AND_STAMINA;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

/*
each banquet(except Brew and Pandaren) has an associated stat : 
    Oven : Stamina
    Grill : Strength
    Wok : Agility
    Steamer : Spirit
    Pot : Intellect
The feast will give + 275 of its associated stat, or + 250 of any other stat, according to the usual rules of smart food.The Banquest of the Brew gives + 250 to each stat, and the Pandaren Banquet gives + 275 to each stat.
For stamina, the numbers are + 375 and + 415.
*/

// Called By Banquet of the wok - 126541
// Banquet of the Wok - 126495 and Great Banquet of the Wok - 126496
class spell_cooking_banquet_of_the_wok : public SpellScriptLoader
{
public: 
    spell_cooking_banquet_of_the_wok() : SpellScriptLoader("spell_cooking_banquet_of_the_wok") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH = 104267, // 249 strength
            SPELL_STAT_AGILITY = 104274, // 274 agility
            SPELL_STAT_INTELECT = 104264, // 249 intelect
            SPELL_STAT_STAMINA = 104281, // 375 stamina
            SPELL_STAT_SPIRIT = 104278, // 249 spirit
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY;
                        break;
                    case STAT_STAMINA:
                        spellId = SPELL_STAT_STAMINA;
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

// Called By Banquet of the Oven - 126544
// Banquet of the Oven - 125600 and Great Banquet of the Oven - 125601
class spell_cooking_banquet_of_the_oven : public SpellScriptLoader
{
public: 
    spell_cooking_banquet_of_the_oven() : SpellScriptLoader("spell_cooking_banquet_of_the_oven") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH = 104267, // 249 strength
            SPELL_STAT_AGILITY = 104273, // 249 agility
            SPELL_STAT_INTELECT = 104264, // 249 intelect
            SPELL_STAT_STAMINA = 104282, // 414 stamina
            SPELL_STAT_SPIRIT = 104278, // 249 spirit
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY;
                        break;
                    case STAT_STAMINA:
                        spellId = SPELL_STAT_STAMINA;
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

// Called By Banquet of the Grill - 126532
// Banquet of the Grill	- 126492 and Great Banquet of the Grill - 126494
class spell_cooking_banquet_of_the_grill : public SpellScriptLoader
{
public: 
    spell_cooking_banquet_of_the_grill() : SpellScriptLoader("spell_cooking_banquet_of_the_grill") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH = 104271, // 274 strength
            SPELL_STAT_AGILITY = 104273, // 249 agility
            SPELL_STAT_INTELECT = 104264, // 249 intelect
            SPELL_STAT_STAMINA = 104281, // 375 stamina
            SPELL_STAT_SPIRIT = 104278, // 249 spirit
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY;
                        break;
                    case STAT_STAMINA:
                        spellId = SPELL_STAT_STAMINA;
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

// Called By Banquet of the Pot	- 126542
// Banquet of the Pot - 126497 and Banquet of the Pot - 126498
class spell_cooking_banquet_of_the_pot : public SpellScriptLoader
{
public: 
    spell_cooking_banquet_of_the_pot() : SpellScriptLoader("spell_cooking_banquet_of_the_pot") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH = 104267, // 249 strength
            SPELL_STAT_AGILITY = 104273, // 249 agility
            SPELL_STAT_INTELECT = 104276, // 274 intelect
            SPELL_STAT_STAMINA = 104281, // 375 stamina
            SPELL_STAT_SPIRIT = 104278, // 249 spirit
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY;
                        break;
                    case STAT_STAMINA:
                        spellId = SPELL_STAT_STAMINA;
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

// Called By Banquet of the Steamer - 126543
// Banquet of the Steamer - 125598 and Great Banquet of the Steamer - 125599
class spell_cooking_banquet_of_the_steamer : public SpellScriptLoader
{
public: 
    spell_cooking_banquet_of_the_steamer() : SpellScriptLoader("spell_cooking_banquet_of_the_steamer") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH = 104267, // 249 strength
            SPELL_STAT_AGILITY = 104273, // 249 agility
            SPELL_STAT_INTELECT = 104264, // 249 intelect
            SPELL_STAT_STAMINA = 104281, // 375 stamina
            SPELL_STAT_SPIRIT = 104279, // 274 spirit
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY;
                        break;
                    case STAT_STAMINA:
                        spellId = SPELL_STAT_STAMINA;
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

// Called By Banquet of the Brew - 126545
// Banquet of the Brew - 126503 and Great Banquet of the Brew - 126504
class spell_cooking_banquet_of_the_brew : public SpellScriptLoader
{
public: 
    spell_cooking_banquet_of_the_brew() : SpellScriptLoader("spell_cooking_banquet_of_the_brew") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH = 104267, // 249 strength
            SPELL_STAT_AGILITY = 104273, // 249 agility
            SPELL_STAT_INTELECT = 104264, // 249 intelect
            SPELL_STAT_STAMINA = 104281, // 375 stamina
            SPELL_STAT_SPIRIT = 104278, // 249 spirit
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY;
                        break;
                    case STAT_STAMINA:
                        spellId = SPELL_STAT_STAMINA;
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

// Called By Pandaren Banquet - 104924
// Pandaren Banquet - 104958 and Great Pandaren Banquet - 105193
class spell_cooking_pandarian_banquet : public SpellScriptLoader
{
public: 
    spell_cooking_pandarian_banquet() : SpellScriptLoader("spell_cooking_pandarian_banquet") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl)

        enum
        {
            SPELL_STAT_STRENGTH = 104271, // 274 strength
            SPELL_STAT_AGILITY = 104274, // 274 agility
            SPELL_STAT_INTELECT = 104276, // 274 intelect
            SPELL_STAT_STAMINA = 104282, // 414 stamina
            SPELL_STAT_SPIRIT = 104279, // 274 spirit
        };

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();

            if (aurEff->GetTickNumber() > 1)
                return;

            if (Player* player = GetCaster()->ToPlayer())
            {
                uint32 spellId = SPELL_STAT_STRENGTH;

                switch (player->GetStatForSpec(player->getClass()))
                {
                    case STAT_STRENGTH:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                    case STAT_AGILITY:
                        spellId = SPELL_STAT_AGILITY;
                        break;
                    case STAT_STAMINA:
                        spellId = SPELL_STAT_STAMINA;
                        break;
                    case STAT_INTELLECT:
                        spellId = SPELL_STAT_INTELECT;
                        break;
                    case STAT_SPIRIT:
                        spellId = SPELL_STAT_SPIRIT;
                        break;
                    default:
                        spellId = SPELL_STAT_STRENGTH;
                        break;
                }

                player->CastSpell(player, spellId, true);
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

void AddSC_cooking_spell_scripts()
{
    // cata
    new spell_cooking_seafood_magnifique_feast();
    new spell_cooking_broiled_dragon_feast();
    // panda
    new spell_cooking_banquet_of_the_wok();
    new spell_cooking_banquet_of_the_oven();
    new spell_cooking_banquet_of_the_grill();
    new spell_cooking_banquet_of_the_pot();
    new spell_cooking_banquet_of_the_steamer();
    new spell_cooking_banquet_of_the_brew();
    new spell_cooking_pandarian_banquet();
}