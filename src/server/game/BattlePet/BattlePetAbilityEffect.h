/*
 * Copyright (C) 2015 Warmane <http://www.warmane.com/>
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

#ifndef BATTLE_PET_ABILITY_EFFECT_H
#define BATTLE_PET_ABILITY_EFFECT_H

#include "Common.h"
#include "BattlePet.h"
#include "DB2Stores.h"

static const uint32 FailFlags = PET_BATTLE_EFFECT_FLAG_MISS | PET_BATTLE_EFFECT_FLAG_BLOCKED | PET_BATTLE_EFFECT_FLAG_DODGE | PET_BATTLE_EFFECT_FLAG_REFLECT | PET_BATTLE_EFFECT_FLAG_IMMUNE;

enum PetBattleAbilityTarget
{
    PET_BATTLE_ABILITY_TARGET_NONE                      = 0,
    PET_BATTLE_ABILITY_TARGET_CASTER                    = 1,
    PET_BATTLE_ABILITY_TARGET_TARGET                    = 2,
    // PET_BATTLE_ABILITY_TARGET_CASTER_TEAM               = 3,
    // PET_BATTLE_ABILITY_TARGET_CASTER_TEAM_0             = 4,
    // PET_BATTLE_ABILITY_TARGET_CASTER_TEAM_1             = 5,
    // PET_BATTLE_ABILITY_TARGET_CASTER_TEAM_2             = 6,
    // PET_BATTLE_ABILITY_TARGET_TARGET_TEAM               = 7,
    // PET_BATTLE_ABILITY_TARGET_TARGET_TEAM_0             = 8,
    // PET_BATTLE_ABILITY_TARGET_TARGET_TEAM_1             = 9,
    // PET_BATTLE_ABILITY_TARGET_TARGET_TEAM_2             = 10,
    PET_BATTLE_ABILITY_TARGET_ALL                       = 11,
    PET_BATTLE_ABILITY_TARGET_CASTER_TEAM_UNCONDITIONAL = 12, 
    PET_BATTLE_ABILITY_TARGET_HEAD                      = 13
};

#define PET_BATTLE_TOTAL_ABILITY_EFFECTS 198

typedef std::vector<BattlePet*> TargetStore;

class BattlePetAbilityEffect
{
public:
    BattlePetAbilityEffect()
        : m_flags(0), m_reportFailAsImmune(0), m_chainFailure(false) { }

    void SetAbilityInfo(uint32 ability, BattlePetAbilityEffectEntry const* effectEntry, uint8 family);
    void SetCaster(BattlePet* caster) { m_caster = caster; }
    void SetParentBattle(PetBattle* petBattle) { m_petBattle = petBattle; }

    BattlePet* GetActiveOpponent();
    void AddTargets();
    void AddTarget(BattlePet* target);

    uint32 CalculateDamage(uint32 damage);
    uint32 CalculateHeal(uint32 heal);
    void CalculateHit(int32 accuracy);

    void Damage(BattlePet* target, uint32 damage);
    void Heal(BattlePet* target, uint32 heal);

    void SetHealth(BattlePet* target, uint32 value);

    bool Execute();
    void HandleNull() { }

    void HandleHeal();
    void HandleDamage();
    void HandleCatch();
    void HandlePositiveAura();
    void HandleRampingDamage();
    void HandleSetState();
    void HandleHealPctDealt();
    void HandleNegativeAura();
    void HandlePeriodicTrigger();
    void HandlePeriodicPositiveTrigger();
    void HandleDamageToggleAura();
    void HandleDamageHitState();
    void HandleExtraAttackFirst();
    void HandleHealState();
    void HandleDamageNonLethal();
    void HandlePowerlessAura();

private:
    uint32 m_abilityId;
    uint8 m_abilityFamily;
    uint16 m_flags;

    BattlePet* m_caster;
    BattlePet* m_target;                                        // current target being executed on
    TargetStore m_targets;                                      // set of targets to execute effect on

    BattlePetAbilityEffectEntry const* m_effectEntry;
    BattlePetAbilityTurnEntry* m_turnEntry;
    PetBattle* m_petBattle;

    bool m_reportFailAsImmune;
    bool m_chainFailure;
};

#endif
