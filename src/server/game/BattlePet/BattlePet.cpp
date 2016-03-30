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

#include "AchievementMgr.h"
#include "BattlePet.h"
#include "BattlePetAbilityEffect.h"
#include "BattlePetSpawnMgr.h"
#include "DBCStores.h"
#include "DB2Enums.h"
#include "DB2Stores.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SharedDefines.h"
#include "Util.h"
#include "WorldSession.h"
#include <algorithm>

// -------------------------------------------------------------------------------
// Battle Pet
// -------------------------------------------------------------------------------

void BattlePet::Initialise(bool newBattlePet)
{
    memset(AbilitiyIds, 0, sizeof(AbilitiyIds));

    // existence is checked before this, no problem should arise
    m_npc = sBattlePetSpeciesStore.LookupEntry(m_species)->NpcId;
    m_displayId = sObjectMgr->GetCreatureTemplate(m_npc)->Modelid1;

    // setup initial battle pet states
    InitialiseStates(newBattlePet);
}

void BattlePet::InitialiseStates(bool newBattlePet)
{
    // initialise states
    memset(States, 0, sizeof(States));
    memset(m_mainStates, 0, sizeof(m_mainStates));

    States[BATTLE_PET_STATE_STAT_CRITICAL_CHANCE] = 5;
    States[BATTLE_PET_STATE_MOD_PET_TYPE_ID] = m_family;

    // set passive ability states
    static const uint32 familtyPassiveStates[BATTLE_PET_FAMILY_COUNT] =
    {
        BATTLE_PET_STATE_PASSIVE_HUMANOID,
        BATTLE_PET_STATE_PASSIVE_DRAGON,
        BATTLE_PET_STATE_PASSIVE_FLYING,
        BATTLE_PET_STATE_PASSIVE_UNDEAD,
        BATTLE_PET_STATE_PASSIVE_CRITTER,
        BATTLE_PET_STATE_PASSIVE_MAGIC,
        BATTLE_PET_STATE_PASSIVE_ELEMENTAL,
        BATTLE_PET_STATE_PASSIVE_BEAST,
        BATTLE_PET_STATE_PASSIVE_AQUATIC,
        BATTLE_PET_STATE_PASSIVE_MECHANICAL,
    };

    States[familtyPassiveStates[m_family]] = 1;

    // set breed specific states
    for (uint32 i = 0; i < sBattlePetBreedStateStore.GetNumRows(); i++)
    {
        auto breedStateEntry = sBattlePetBreedStateStore.LookupEntry(i);
        if (!breedStateEntry)
            continue;

        if (breedStateEntry->BreedId != m_breed)
            continue;

        // handle main stat states
        if (IsMainStatState(breedStateEntry->StateId))
            m_mainStates[breedStateEntry->StateId - BATTLE_PET_STATE_STAT_POWER] += breedStateEntry->Modifier;
        // other states
        else
            States[breedStateEntry->StateId] += breedStateEntry->Modifier;
    }

    // set species specific states
    for (uint32 i = 0; i < sBattlePetSpeciesStateStore.GetNumRows(); i++)
    {
        auto speciesStateEntry = sBattlePetSpeciesStateStore.LookupEntry(i);
        if (!speciesStateEntry)
            continue;

        if (speciesStateEntry->SpeciesId != m_species)
            continue;

        // handle main stat states
        if (IsMainStatState(speciesStateEntry->StateId))
            m_mainStates[speciesStateEntry->StateId - BATTLE_PET_STATE_STAT_POWER] += speciesStateEntry->Modifier;
        else
        // other states
            States[speciesStateEntry->StateId] += speciesStateEntry->Modifier;
    }

    // apply quality modifier
    for (uint8 i = 0; i < BATTLE_PET_MAX_MAIN_STATES; i++)
    {
        auto breedQualityEntry = sBattlePetBreedQualityStore.LookupEntry(7 + m_quality);
        ASSERT(breedQualityEntry);

        m_mainStates[i] *= breedQualityEntry->Multiplier;
    }

    // calculate main stats
    CalculateStats(newBattlePet);

    if (!m_curHealth)
        States[BATTLE_PET_STATE_IS_DEAD] = 1;
}

void BattlePet::InitialiseAbilities(bool wild)
{
    // calculate abilites for the battle pet
    for (uint32 i = 0; i < sBattlePetSpeciesXAbilityStore.GetNumRows(); i++)
    {
        auto abilityEntry = sBattlePetSpeciesXAbilityStore.LookupEntry(i);
        if (!abilityEntry)
            continue;

        if (abilityEntry->SpeciesId != m_species)
            continue;

        if (abilityEntry->RequiredLevel > m_level)
            continue;

        // first ability in the tier
        if (abilityEntry->RequiredLevel < 5)
            AbilitiyIds[abilityEntry->SlotId] = abilityEntry->AbilityId;
        // second ability in the tier
        else
        {
            // wild battle pets have their second ability decided by chance
            if (wild && urand(0, 1))
                AbilitiyIds[abilityEntry->SlotId] = abilityEntry->AbilityId;
            // player battle pets have their second ability decided by journal flags 
            else if (!wild)
            {
                switch (abilityEntry->SlotId)
                {
                    case 0:
                        if (HasFlag(BATTLE_PET_JOURNAL_FLAG_ABILITY_1))
                            AbilitiyIds[abilityEntry->SlotId] = abilityEntry->AbilityId;
                        break;
                    case 1:
                        if (HasFlag(BATTLE_PET_JOURNAL_FLAG_ABILITY_2))
                            AbilitiyIds[abilityEntry->SlotId] = abilityEntry->AbilityId;
                        break;
                    case 2:
                        if (HasFlag(BATTLE_PET_JOURNAL_FLAG_ABILITY_3))
                            AbilitiyIds[abilityEntry->SlotId] = abilityEntry->AbilityId;
                        break;
                }
            }
        }
    }
}

void BattlePet::SetBattleAbilities()
{
    // initialise abilities for battle
    for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
    {
        if (!AbilitiyIds[i])
        {
            Abilities[i] = nullptr;
            continue;
        }

        auto battlePetAbility = new BattlePetAbility();
        battlePetAbility->AbilityId  = AbilitiyIds[i];
        battlePetAbility->OnCooldown = false;
        battlePetAbility->Cooldown   = 0;
        battlePetAbility->Lockdown   = 0;

        Abilities[i] = battlePetAbility;
    }
}

void BattlePet::CalculateStats(bool currentHealth)
{
    // initial values
    for (uint8 i = 0; i < BATTLE_PET_MAX_MAIN_STATES; i++)
        States[BATTLE_PET_STATE_STAT_POWER + i] = m_mainStates[i];

    // apply level modifier
    States[BATTLE_PET_STATE_STAT_POWER] *= m_level;
    States[BATTLE_PET_STATE_STAT_SPEED] *= m_level;

    // calculate health
    m_maxHealth = (10000 + (States[BATTLE_PET_STATE_STAT_STAMINA] * 5) * m_level) / 100;

    if (currentHealth)
        m_curHealth = m_maxHealth;

    // flag battle pet for save
    m_dbState = BATTLE_PET_DB_STATE_SAVE;
}

void BattlePet::SetFlag(uint16 flag)
{
    if (HasFlag(flag))
        return;

    m_flags |= flag;
    m_dbState = BATTLE_PET_DB_STATE_SAVE;
}

void BattlePet::UnSetFlag(uint16 flag)
{
    if (!HasFlag(flag))
        return;

    m_flags &= ~flag;
    m_dbState = BATTLE_PET_DB_STATE_SAVE;
}

void BattlePet::SetCurrentHealth(uint32 health)
{
    if (health > m_maxHealth)
        health = m_maxHealth;

    if (!IsAlive() && health)
        States[BATTLE_PET_STATE_IS_DEAD] = 0;

    m_curHealth = health;
    m_dbState = BATTLE_PET_DB_STATE_SAVE;
}

void BattlePet::SetNickname(std::string nickname)
{
    m_nickname = nickname;
    m_dbState = BATTLE_PET_DB_STATE_SAVE;
}

void BattlePet::SetXP(uint16 xpGain)
{
    if (!xpGain)
        return;

    if (m_level == BATTLE_PET_MAX_LEVEL)
        return;

    uint16 baseValue = sGtBattlePetXpStore.LookupEntry(100 + (m_level - 1))->Value;
    uint16 multiplier = sGtBattlePetXpStore.LookupEntry(m_level - 1)->Value;

    uint16 maxXpForLevel = baseValue * multiplier;

    // battle pet has leveled
    if (m_xp + xpGain >= maxXpForLevel)
    {
        m_xp = (m_xp + xpGain) - maxXpForLevel;
        SetLevel(m_level + 1);
    }
    else
        m_xp += xpGain;

    m_dbState = BATTLE_PET_DB_STATE_SAVE;
}

void BattlePet::SetLevel(uint8 level)
{
    // make sure level is valid
    if (!level || level > BATTLE_PET_MAX_LEVEL || level == m_level)
        return;

    if (m_level == BATTLE_PET_MAX_LEVEL || level < m_level)
        m_xp = 0;

    m_level = level;

    // These achievements should be handled better?
    // Achievement: Newbie - Raise a pet to level 3.
    if (m_level >= 3 && !m_owner->GetAchievementMgr().HasAchieved(BATTLE_PET_ACHIEVEMENT_NEWBIE))
    {
        auto achievement = sAchievementMgr->GetAchievement(BATTLE_PET_ACHIEVEMENT_NEWBIE);
        m_owner->CompletedAchievement(achievement);

        // unlock second loadout slot
        m_owner->GetBattlePetMgr().UnlockLoadoutSlot(BATTLE_PET_LOADOUT_SLOT_2);
    }

    // Achievement: Just a Pup - Raise a pet to level 5.
    if (m_level >= 5 && !m_owner->GetAchievementMgr().HasAchieved(BATTLE_PET_ACHIEVEMENT_JUST_A_PUP))
    {
        auto achievement = sAchievementMgr->GetAchievement(BATTLE_PET_ACHIEVEMENT_JUST_A_PUP);
        m_owner->CompletedAchievement(achievement);

        // unlock third loadout slot
        m_owner->GetBattlePetMgr().UnlockLoadoutSlot(BATTLE_PET_LOADOUT_SLOT_3);
    }

    CalculateStats(true);
    InitialiseAbilities(false);

    // update world object if battle pet is currently summoned
    if (m_owner->GetBattlePetMgr().GetCurrentSummonId() == m_id)
    {
        auto worldPet = m_owner->GetBattlePetMgr().GetCurrentSummon();

        worldPet->SetUInt32Value(UNIT_FIELD_WILD_BATTLE_PET_LEVEL, m_level);
        worldPet->SetHealth(m_curHealth);
        worldPet->SetMaxHealth(m_maxHealth);
    }
}

uint16 BattlePet::GetSpeed() const
{
    int32 modPercent = States[BATTLE_PET_STATE_MOD_SPEED_PERCENT];

    // Flying (239)
    // Flying creatures gain 50% extra speed while above 50% health
    if (States[BATTLE_PET_STATE_PASSIVE_FLYING])
        if ((GetCurrentHealth() * 100 / GetMaxHealth()) > 50)
            modPercent += 50;

    return (States[BATTLE_PET_STATE_STAT_SPEED] + CalculatePct(States[BATTLE_PET_STATE_STAT_SPEED], modPercent)) / 100;
}

uint16 BattlePet::GetPower() const
{
    return (States[BATTLE_PET_STATE_STAT_POWER] + CalculatePct(States[BATTLE_PET_STATE_STAT_POWER], States[BATTLE_PET_STATE_MOD_DAMAGE_DEALT_PCT])) / 100;
}

void BattlePet::ResetMechanicStates()
{
    States[BATTLE_PET_STATE_MECHANIC_POISONED]  = 0;
    States[BATTLE_PET_STATE_MECHANIC_STUNNED]   = 0;
    States[BATTLE_PET_STATE_MOD_SPEED_PERCENT]  = 0;
    States[BATTLE_PET_STATE_UNTARGETABLE]       = 0;
    States[BATTLE_PET_STATE_UNDERGROUND]        = 0;
    States[BATTLE_PET_STATE_MECHANIC_FLYING]    = 0;
    States[BATTLE_PET_STATE_MECHANIC_BURNING]   = 0;
    States[BATTLE_PET_STATE_TURN_LOCK]          = 0;
    States[BATTLE_PET_STATE_SWAP_OUT_LOCK]      = 0;
    States[BATTLE_PET_STATE_MECHANIC_CHILLED]   = 0;
    States[BATTLE_PET_STATE_MECHANIC_WEBBED]    = 0;
    States[BATTLE_PET_STATE_MECHANIC_INVISIBLE] = 0;
    States[BATTLE_PET_STATE_UNKILLABLE]         = 0;
    States[BATTLE_PET_STATE_MECHANIC_BLEEDING]  = 0;
    States[BATTLE_PET_STATE_MECHANIC_BLIND]     = 0;
    States[BATTLE_PET_STATE_SWAP_IN_LOCK]       = 0;
    States[BATTLE_PET_STATE_MECHANIC_BOMB]      = 0;
    States[BATTLE_PET_STATE_RESILITANT]         = 0;
}

PetBattleAura* BattlePet::GetAura(uint32 abilityId)
{
    for (auto &aura : Auras)
        if (aura->GetAbility() == abilityId && !aura->HasExpired())
            return aura;

    return nullptr;
}

// -------------------------------------------------------------------------------
// Pet Battle
// -------------------------------------------------------------------------------

void PetBattleAura::OnApply()
{
    // apply any state modifiers for the aura
    for (uint32 i = 0; i < sBattlePetAbilityStateStore.GetNumRows(); i++)
    {
        auto stateEntry = sBattlePetAbilityStateStore.LookupEntry(i);
        if (!stateEntry)
            continue;

        if (stateEntry->AbilityId != m_ability)
            continue;

        // store state and modifier for removal
        if (m_auraStates.find(stateEntry->StateId) == m_auraStates.end())
            m_auraStates[stateEntry->StateId] = 0;

        int32 newValue = stateEntry->Value + m_target->States[stateEntry->StateId];
        m_auraStates[stateEntry->StateId] += newValue;

        // update state
        m_petBattle->UpdatePetState(m_caster, m_target, 0, stateEntry->StateId, newValue);
    }
}

void PetBattleAura::OnExpire()
{
    // remove any state modifiers for the aura
    for (auto stateEntry : m_auraStates)
    {
        int32 newValue = m_target->States[stateEntry.first] - stateEntry.second;
        m_petBattle->UpdatePetState(m_caster, m_target, 0, stateEntry.first, newValue);
    }

    m_auraStates.clear();
}

void PetBattleAura::Process()
{
    // expire aura if it has reached max duration
    if (m_duration != -1 && m_turn > m_maxDuration && !m_expired)
    {
        Expire();
        return;
    }

    // handle aura effects
    if (auto abilityEntry = sBattlePetAbilityStore.LookupEntry(m_ability))
    {
        uint32 turnCount = 0;
        uint32 topMaxTurnId = 0;

        // find longest duration effect
        for (uint32 i = 0; i < sBattlePetAbilityTurnStore.GetNumRows(); i++)
        {
            auto abilityTurnEntry = sBattlePetAbilityTurnStore.LookupEntry(i);
            if (!abilityTurnEntry || abilityTurnEntry->AbilityId != m_ability)
                continue;

            turnCount++;
            topMaxTurnId = std::max(topMaxTurnId, abilityTurnEntry->Duration);
        }

        for (uint32 i = 0; i < sBattlePetAbilityTurnStore.GetNumRows(); i++)
        {
            // make sure ability turn entry is for auras ability
            auto abilityTurnEntry = sBattlePetAbilityTurnStore.LookupEntry(i);
            if (!abilityTurnEntry || abilityTurnEntry->AbilityId != m_ability)
                continue;

            // make sure ability has reached duration required for effect
            if (abilityTurnEntry->Duration != m_turn && turnCount != 1 && topMaxTurnId != 1)
                continue;

            for (uint32 j = 0; j < sBattlePetAbilityEffectStore.GetNumRows(); j++)
            {
                auto abilityEffectEntry = sBattlePetAbilityEffectStore.LookupEntry(j);
                if (!abilityEffectEntry || abilityEffectEntry->AbilityTurnId != abilityTurnEntry->Id)
                    continue;

                // initialise ability effect
                BattlePetAbilityEffect abilityEffect;
                abilityEffect.SetAbilityInfo(m_ability, abilityEffectEntry, abilityEntry->FamilyId);
                abilityEffect.SetCaster(m_caster);
                abilityEffect.AddTarget(m_target);
                abilityEffect.SetParentBattle(m_petBattle);

                abilityEffect.Execute();
            }
        }
    }

    // notify client of aura update
    PetBattleEffect effect(PET_BATTLE_EFFECT_AURA_CHANGE, m_caster->GetGlobalIndex());
    effect.TargetUpdateAura(m_target->GetGlobalIndex(), m_id, m_ability, m_duration, m_turn);

    m_petBattle->Effects.push_back(effect);

    m_turn++;

    if (m_duration != -1)
        m_duration--;
}

void PetBattleAura::Expire()
{
    if (m_expired)
        return;

    m_expired = true;

    // notify client of aura removal
    PetBattleEffect effect(PET_BATTLE_EFFECT_AURA_REMOVE, m_caster->GetGlobalIndex(), 0, 0, 1, 0, 1);
    effect.TargetUpdateAura(m_target->GetGlobalIndex(), m_id, m_ability, 0, 0);

    m_petBattle->Effects.push_back(effect);

    // cast any removal procs the ability might have
    m_petBattle->Cast(m_target, m_ability, m_turn, PET_BATTLE_ABILITY_TURN0_PROC_ON_AURA_REMOVED);

    OnExpire();
}

// -------------------------------------------------------------------------------

void PetBattleTeam::AddPlayer(Player* player)
{
    m_owner = player;
    m_ownerGuid = player->GetGUID();

    // add player loadout battle pets to team
    uint8 localIndex = 0;
    for (uint8 i = 0; i < BATTLE_PET_MAX_LOADOUT_SLOTS; i++)
    {
        uint64 battlePetId = player->GetBattlePetMgr().GetLoadoutSlot(i);
        if (!battlePetId)
            continue;

        auto battlePet = player->GetBattlePetMgr().GetBattlePet(battlePetId);
        if (!battlePet->IsAlive())
            continue;

        // bind pet battle information to battle pet
        battlePet->SetBattleInfo(m_teamIndex, ConvertToGlobalIndex(localIndex++));
        battlePet->SetBattleAbilities();

        // in PvE matches, the first pet in loadout is initially set to active pet
        if (m_petBattle->GetType() == PET_BATTLE_TYPE_PVE && !m_activePet)
            m_activePet = battlePet;

        BattlePets.push_back(battlePet);
    }
}

void PetBattleTeam::AddWildBattlePet(Creature* creature)
{
    auto battlePet = sBattlePetSpawnMgr->GetWildBattlePet(creature);
    ASSERT(battlePet);

    battlePet->SetBattleInfo(m_teamIndex, ConvertToGlobalIndex(BattlePets.size()));
    battlePet->SetBattleAbilities();

    BattlePets.push_back(battlePet);

    // only update creature if it's the first to the team
    if (!m_wildBattlePet)
    {
        m_wildBattlePet = creature;
        m_activePet = battlePet;
    }
}

typedef std::vector<uint32> AvaliableAbilities;

void PetBattleTeam::Update()
{
    if (m_ready)
        return;

    if (!m_activePet->IsAlive())
    {
        BattlePetStore avaliablePets;
        GetAvaliablePets(avaliablePets);

        // team is dead, finish battle
        if (!avaliablePets.size())
        {
            m_petBattle->FinishBattle(this, false);
            return;
        }

        // final pet, auto swap
        if (avaliablePets.size() == 1)
            m_petBattle->SwapActivePet(avaliablePets[0]);
    }
    else
    {
        // team has no move if multiple turn ability active
        if (HasMultipleTurnAbility())
        {
            m_ready = true;
            return;
        }

        // wild battle pets currently choose a random ability that isn't on cooldown, no other AI
        if (m_petBattle->GetType() == PET_BATTLE_TYPE_PVE && m_teamIndex == PET_BATTLE_OPPONENT_TEAM)
        {
            AvaliableAbilities avaliableAbilities;
            for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
                if (m_activePet->Abilities[i])
                    if (CanActivePetCast(m_activePet->Abilities[i]->AbilityId))
                        avaliableAbilities.push_back(m_activePet->Abilities[i]->AbilityId);

            // cast ability
            if (avaliableAbilities.size())
                SetPendingMove(PET_BATTLE_MOVE_TYPE_CAST, avaliableAbilities[rand() % avaliableAbilities.size()], nullptr);
            // skip turn
            else
                SetPendingMove(PET_BATTLE_MOVE_TYPE_SWAP_OR_PASS, 0, m_activePet);
        }
    }
}

void PetBattleTeam::ActivePetPrepareCast(uint32 abilityId)
{
    if (!abilityId)
        return;

    // find the longest duration effect for ability
    uint8 duration = 0;
    for (uint32 i = 0; i < sBattlePetAbilityStore.GetNumRows(); i++)
    {
        auto abilityTurnEntry = sBattlePetAbilityTurnStore.LookupEntry(i);
        if (!abilityTurnEntry || abilityTurnEntry->AbilityId != abilityId)
            continue;

        if (abilityTurnEntry->Duration >= duration)
            duration = abilityTurnEntry->Duration;
    }

    ActiveAbility.AbilityId   = abilityId;
    ActiveAbility.TurnsPassed = 1;
    ActiveAbility.TurnsTotal  = duration;
}

bool PetBattleTeam::CanActivePetCast(uint32 abilityId) const
{
    if (HasMultipleTurnAbility())
        return false;

    if (!sBattlePetAbilityStore.LookupEntry(abilityId))
        return false;

    // make sure active pet has ability and it isn't on cooldown
    for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
        if (m_activePet->Abilities[i] && m_activePet->Abilities[i]->AbilityId == abilityId && !m_activePet->Abilities[i]->OnCooldown)
            return true;

    return false;
}

uint8 PetBattleTeam::GetTrapStatus() const
{
    if (m_petBattle->GetType() != PET_BATTLE_TYPE_PVE)
        return PET_BATTLE_TRAP_STATUS_NOT_CAPTURABLE;

    if (m_teamIndex == PET_BATTLE_OPPONENT_TEAM)
        return PET_BATTLE_TRAP_STATUS_DISABLED;

    auto &battlePetMgr = m_owner->GetBattlePetMgr();

    // player needs to have a trap ability
    if (!battlePetMgr.GetTrapAbility())
        return PET_BATTLE_TRAP_STATUS_DISABLED;

    // player can only catch a single pet per battle
    if (m_petBattle->GetCagedPet())
        return PET_BATTLE_TRAP_STATUS_ALREADY_TRAPPED;

    auto team = m_petBattle->Teams[PET_BATTLE_OPPONENT_TEAM];

    if (!team->GetActivePet()->IsAlive())
        return PET_BATTLE_TRAP_STATUS_CANT_TRAP_DEAD_PET;

    // make sure player can store more battle pets
    if (!battlePetMgr.CanStoreBattlePet(team->GetActivePet()->GetSpecies()))
        return PET_BATTLE_TRAP_STATUS_TOO_MANY_PETs;

    // check if pet is below 35% health
    if (team->GetActivePet()->GetCurrentHealth() > CalculatePct(team->GetActivePet()->GetMaxHealth(), 35))
        return PET_BATTLE_TRAP_STATUS_HEALTH_TOO_HIGH;

    return PET_BATTLE_TRAP_STATUS_ENABLED;
}

bool PetBattleTeam::CanSwap(BattlePet* battlePet, bool ignoreAlive) const
{
    if (HasMultipleTurnAbility())
        return false;

    if (!m_activePet->IsAlive() && !ignoreAlive)
        return false;

    // used for abilities such as Sticky Web (339)
    if (m_activePet->States[BATTLE_PET_STATE_SWAP_OUT_LOCK])
        return false;

    if (battlePet)
    {
        if (!battlePet->IsAlive())
            return false;

        // used for abilities such as Banished (717)
        if (battlePet->States[BATTLE_PET_STATE_SWAP_IN_LOCK])
            return false;
    }

    return true;
}

bool PetBattleTeam::HasMultipleTurnAbility() const
{
    if (ActiveAbility.AbilityId && ActiveAbility.TurnsTotal != 1 && ActiveAbility.TurnsPassed <= ActiveAbility.TurnsTotal)
        return true;

    return false;
}

void PetBattleTeam::DoCasts(int8 procType)
{
    if (!m_activePet->IsAlive())
        return;

    if (!m_activePet->CanAttack())
        return;

    bool noProc = procType == PET_BATTLE_ABILITY_TURN0_PROC_ON_NONE;
    m_petBattle->Cast(m_activePet, ActiveAbility.AbilityId, noProc ? ActiveAbility.TurnsPassed : 0, procType);
}

void PetBattleTeam::GetAvaliablePets(BattlePetStore &avaliablePets) const
{
    for (auto battlePet : BattlePets)
    {
        if (!battlePet->IsAlive())
            continue;

        // make sure local pet can be swapped with active
        if (!CanSwap(battlePet, true))
            continue;

        avaliablePets.push_back(battlePet);
    }
}

// checks if a battle pet belongs to a pet battle team
bool PetBattleTeam::IsValidBattlePet(BattlePet* battlePet) const
{
    bool isValid = false;
    for (auto battlePetTeam : BattlePets)
        if (battlePetTeam == battlePet)
            isValid = true;

    return isValid;
}

uint8 PetBattleTeam::GetInputStatusFlags() const
{
    uint8 flags = PET_BATTLE_TEAM_INPUT_FLAG_NONE;

    // TODO: more checks probably required
    if (HasMultipleTurnAbility())
        flags |= (PET_BATTLE_TEAM_INPUT_FLAG_LOCK_ABILITY_1 | PET_BATTLE_TEAM_INPUT_FLAG_LOCK_ABILITY_2);

    if (!CanSwap())
        flags |= PET_BATTLE_TEAM_INPUT_FLAG_LOCK_PET_SWAP;

    if (!m_activePet->IsAlive())
    {
        BattlePetStore avaliablePets;
        GetAvaliablePets(avaliablePets);

        if (avaliablePets.size())
            flags |= PET_BATTLE_TEAM_INPUT_FLAG_SELECT_NEW_PET;
        else
            flags |= PET_BATTLE_TEAM_INPUT_FLAG_LOCK_PET_SWAP;
    }

    return flags;
}

void PetBattleTeam::ResetActiveAbility()
{
    ActiveAbility.AbilityId   = 0;
    ActiveAbility.TurnsPassed = 0;
    ActiveAbility.TurnsTotal  = 0;
}

void PetBattleTeam::ProcessAuras()
{
    // process any active auras
    for (auto battlePet : BattlePets)
    {
        for (auto aura : battlePet->Auras)
        {
            if (!aura->HasExpired())
                aura->Process();

            // 543 - Prowl
            if (aura->GetAbility() == 543
                && aura->GetTurn() > 2
                && m_activePet->States[BATTLE_PET_STATE_CONDITION_DID_DAMAGE_THIS_ROUND])
                aura->Expire();

            // remove next attack auras (Attack Reduction)
            if (aura->GetDuration() == -1
                && aura->GetTurn() > 2
                && m_activePet->States[BATTLE_PET_STATE_CONDITION_DID_DAMAGE_THIS_ROUND])
                aura->Expire();
        }
    }
}

typedef std::vector<PetBattleAura*> AuraRemovalStore;

void PetBattleTeam::RemoveExpiredAuras()
{
    // find any expired auras that need removing
    AuraRemovalStore removalList;
    for (auto battlePet : BattlePets)
    {
        for (auto &aura : battlePet->Auras)
            if (aura->HasExpired())
                removalList.push_back(aura);

        // delete expired auras
        for (auto &aura : removalList)
        {
            battlePet->Auras.remove(aura);
            delete aura;
        }

        removalList.clear();
    }
}

void PetBattleTeam::SetPendingMove(uint8 moveType, uint32 abilityId, BattlePet* newActivePet)
{
    m_pendingMove.MoveType  = moveType;
    m_pendingMove.AbilityId = abilityId;
    m_pendingMove.battlePet = newActivePet;
    m_ready = true;
}

// -------------------------------------------------------------------------------

uint8 PetBattleTeam::ConvertToGlobalIndex(uint8 localPetIndex) const
{
    ASSERT(localPetIndex < PET_BATTLE_MAX_TEAM_PETS);
    return localPetIndex + (m_teamIndex == PET_BATTLE_CHALLANGER_TEAM ? 0 : PET_BATTLE_MAX_TEAM_PETS);
}

uint8 PetBattleTeam::ConvertToLocalIndex(uint8 globalPetIndex) const
{
    ASSERT(globalPetIndex < (PET_BATTLE_MAX_TEAM_PETS * PET_BATTLE_MAX_TEAMS));
    return globalPetIndex - (m_teamIndex == PET_BATTLE_CHALLANGER_TEAM ? 0 : PET_BATTLE_MAX_TEAM_PETS);
}

// -------------------------------------------------------------------------------

PetBattle::~PetBattle()
{
    // clean up teams
    for (uint8 i = 0; i < PET_BATTLE_MAX_TEAMS; i++)
        delete Teams[i];
}

void PetBattle::StartBattle()
{
    // in PvE matches the initial active pet is locked
    for (auto &team : Teams)
        SwapActivePet(team->GetActivePet());

    for (auto &team : Teams)
    {
        // initial team setup only required for players
        if (team->GetTeamIndex() == PET_BATTLE_OPPONENT_TEAM && m_type == PET_BATTLE_TYPE_PVE)
            continue;

        auto owner = Teams[team->GetTeamIndex()]->GetOwner();
        owner->GetBattlePetMgr().UnSummonCurrentBattlePet(true);

        // send initial packet update
        owner->GetSession()->SendPetBattleInitialUpdate(this);
        owner->GetSession()->SendPetBattleFirstRound(this);
    }

    Effects.clear();

    // round setup
    m_phase = PET_BATTLE_PHASE_IN_PROGRESS;
    m_round++;
}

void PetBattle::FinishBattle(PetBattleTeam* lostTeam, bool forfeit)
{
    // if no winning team is specified, battle was forcefully ended
    m_winningTeam = nullptr;
    if (lostTeam)
        m_winningTeam = Teams[!lostTeam->GetTeamIndex()];

    for (auto team : Teams)
    {
        for (auto battlePet : team->BattlePets)
        {
            // remove abilities
            for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
            {
                if (auto ability = battlePet->Abilities[i])
                {
                    delete ability;
                    battlePet->Abilities[i] = nullptr;
                }
            }

            // remove auras
            for (auto &aura : battlePet->Auras)
            {
                aura->OnExpire();
                delete aura;
            }

            battlePet->Auras.clear();

            // remove any remaining mechanic states not cleaned up on aura removal
            battlePet->ResetMechanicStates();
        }

        if (auto player = team->GetOwner())
        {
            for (auto battlePet : team->BattlePets)
            {
                // make sure battle wasn't forcefully ended
                if (m_winningTeam)
                {
                    // remove 10% health on forfeit
                    if (team != m_winningTeam && forfeit)
                    {
                        uint16 reduction = CalculatePct(battlePet->GetCurrentHealth(), 10);
                        battlePet->SetCurrentHealth(battlePet->GetCurrentHealth() - reduction);
                    }

                    // award XP to battle pets that actively participated in the battle
                    if (team->SeenAction.find(battlePet) != team->SeenAction.end()
                        && GetType() == PET_BATTLE_TYPE_PVE
                        && m_winningTeam == team
                        && battlePet->GetLevel() != BATTLE_PET_MAX_LEVEL
                        && battlePet->IsAlive())
                    {
                        // XP gain formular from: http://www.wowwiki.com/Pet_Battle_System#Calculating_experience
                        uint16 xp = 0;
                        for (auto opponentBattlePet : Teams[!team->GetTeamIndex()]->BattlePets)
                        {
                            int levelDifference = opponentBattlePet->GetLevel() - battlePet->GetLevel();

                            // level difference roof capped at +2
                            if (levelDifference > 2)
                                levelDifference = 2;
                            // level difference floor capped at -4
                            else if (levelDifference < -4)
                                levelDifference = -4;

                            xp += ((opponentBattlePet->GetLevel() + 9) * (levelDifference + 5)) / team->SeenAction.size();
                        }

                        battlePet->SetXP(xp);
                    }

                    // update battle pet clientside
                    player->GetBattlePetMgr().SendBattlePetUpdate(battlePet, false);
                }
            }

            // make sure battle wasn't forcefully ended
            if (m_winningTeam)
            {
                // save captured pets
                if (m_winningTeam == team && GetType() == PET_BATTLE_TYPE_PVE)
                {
                    if (auto battlePet = GetCagedPet())
                    {
                        auto &battlePetMgr = player->GetBattlePetMgr();
                        if (!battlePetMgr.CanStoreBattlePet(battlePet->GetSpecies()))
                            continue;

                        uint8 level = battlePet->GetLevel();

                        // level 16-20 pets lose 1 level when caught
                        if (level >= 16 && level <= 20)
                            level--;
                        // level 21-25 pets lose 2 levels when caught
                        else if (level >= 21 && level <= 25)
                            level -= 2;

                        battlePetMgr.Create(battlePet->GetSpecies(), level, battlePet->GetBreed(), battlePet->GetQuality());
                    }
                }
            }

            // achievements
            // TODO...

            player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC);
            player->SetControlled(false, UNIT_STATE_ROOT);

            // alert client of pet battle end
            if (!m_winningTeam)
                // instant battle end
                player->GetSession()->SendPetBattleFinished();
            else
                // delayed battle end and statistics are displayed
                player->GetSession()->SendPetBatatleFinalRound(this);
        }

        if (auto creature = team->GetWildBattlePet())
            sBattlePetSpawnMgr->LeftBattle(creature, team == lostTeam);
    }

    m_phase = PET_BATTLE_PHASE_FINISHED;
}

void PetBattle::HandleRound()
{
    TurnInstance = 1;
    uint32 OldStates[PET_BATTLE_MAX_TEAMS][BATTLE_PET_MAX_STATES] = { };

    // reset state conditions
    for (auto &team : Teams)
    {
        for (auto &battlePet : team->BattlePets)
        {
            battlePet->States[BATTLE_PET_STATE_CONDITION_WAS_DAMAGED_THIS_ROUND] = 0;
            battlePet->States[BATTLE_PET_STATE_CONDITION_DID_DAMAGE_THIS_ROUND] = 0;
        }

        // store main stat states before they are modified during the round
        OldStates[team->GetTeamIndex()][BATTLE_PET_STATE_STAT_SPEED] = team->GetActivePet()->GetSpeed();
        OldStates[team->GetTeamIndex()][BATTLE_PET_STATE_STAT_POWER] = team->GetActivePet()->GetPower();
    }

    uint8 firstTeam = GetFirstAttackingTeam();

    // cast abilities that have round start proc type (example: Deflection(490))
    Teams[firstTeam]->DoCasts(PET_BATTLE_ABILITY_TURN0_PROC_ON_ROUND_START);
    Teams[!firstTeam]->DoCasts(PET_BATTLE_ABILITY_TURN0_PROC_ON_ROUND_START);

    // cast non proc abilities
    Teams[firstTeam]->DoCasts();
    Teams[!firstTeam]->DoCasts();

    // -------------------------------------------------------------------------------

    PetBattleEffect auraStartEffect(PET_BATTLE_EFFECT_AURA_PROCESSING_BEGIN);
    auraStartEffect.TargetActivePet();

    // process active battle pet auras
    Teams[firstTeam]->ProcessAuras();
    Teams[!firstTeam]->ProcessAuras();

    PetBattleEffect auraEndEffect(PET_BATTLE_EFFECT_AURA_PROCESSING_END);
    auraEndEffect.TargetActivePet();

    Effects.push_back(auraStartEffect);
    Effects.push_back(auraEndEffect);

    // -------------------------------------------------------------------------------

    // cast abilities that have round end proc type
    Teams[firstTeam]->DoCasts(PET_BATTLE_ABILITY_TURN0_PROC_ON_ROUND_END);
    Teams[!firstTeam]->DoCasts(PET_BATTLE_ABILITY_TURN0_PROC_ON_ROUND_END);

    // send stat updates if they changed this round
    for (auto &team : Teams)
    {
        auto activePet = team->GetActivePet();
        if (OldStates[team->GetTeamIndex()][BATTLE_PET_STATE_STAT_SPEED] != activePet->GetSpeed())
        {
            // alert client of speed update
            PetBattleEffect speedEffect(PET_BATTLE_EFFECT_SET_SPEED);
            speedEffect.TargetUpdateStat(activePet->GetGlobalIndex(), activePet->GetSpeed());

            Effects.push_back(speedEffect);
        }

        if (OldStates[team->GetTeamIndex()][BATTLE_PET_STATE_STAT_POWER] != activePet->GetPower())
        {
            // alert client of power update
            PetBattleEffect powerEffect(PET_BATTLE_EFFECT_SET_POWER);
            powerEffect.TargetUpdateStat(activePet->GetGlobalIndex(), activePet->GetPower());

            Effects.push_back(powerEffect);
        }
    }

    // must be incremented before sending round result
    for (auto &team : Teams)
        team->ActiveAbility.TurnsPassed++;

    // send round result to players
    if (m_roundResult == PET_BATTLE_ROUND_RESULT_NONE)
        m_roundResult = PET_BATTLE_ROUND_RESULT_NORMAL;

    for (auto &team : Teams)
        if (auto player = team->GetOwner())
            player->GetSession()->SendPetBattleRoundResult(this);

    // initialise data for the next round
    for (auto &team : Teams)
    {
        team->RemoveExpiredAuras();

        // reduce ability cooldowns
        for (auto &battlePet : team->BattlePets)
            for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
                if (battlePet->Abilities[i] && battlePet->Abilities[i]->Cooldown)
                    battlePet->Abilities[i]->Cooldown--;
    }

    m_round++;
    m_roundResult = PET_BATTLE_ROUND_RESULT_NONE;
}

void PetBattle::Update(uint32 diff)
{
    if (m_phase != PET_BATTLE_PHASE_IN_PROGRESS)
        return;

    m_timeSinceLastRound += diff;

    // check if all teams are ready to progress round
    bool progressRound = true;
    for (auto &team : Teams)
    {
        team->Update();

        if (!team->IsReady())
            progressRound = false;
    }

    if (progressRound)
    {
        // make sure next round timer has elapsed
        if (m_timeSinceLastRound >= m_nextRoundTimer)
        {
            m_timeSinceLastRound = 0;

            // execute pending moves before handling the round
            for (auto &team : Teams)
            {
                auto pendingMove = team->GetPendingMove();
                switch (pendingMove.MoveType)
                {
                    case PET_BATTLE_MOVE_TYPE_CAST:
                    {
                        if (team->CanActivePetCast(pendingMove.AbilityId))
                            team->ActivePetPrepareCast(pendingMove.AbilityId);
                        break;
                    }
                    case PET_BATTLE_MOVE_TYPE_CATCH:
                    {
                        if (team->GetTrapStatus() == PET_BATTLE_TRAP_STATUS_ENABLED)
                            team->ActivePetPrepareCast(team->GetOwner()->GetBattlePetMgr().GetTrapAbility());
                        break;
                    }
                    case PET_BATTLE_MOVE_TYPE_SWAP_OR_PASS:
                    {
                        if (team->CanSwap(pendingMove.battlePet))
                            SwapActivePet(pendingMove.battlePet);
                        break;
                    }
                }
            }

            HandleRound();

            // round timer is 1 second for every event (expect aura processing) in the round up to 4 seconds
            m_nextRoundTimer = std::min(uint32((Effects.size() - 2) * IN_MILLISECONDS), 4000u);
            Effects.clear();

            for (auto &team : Teams)
                team->IsReady(false);
        }
    }
}

bool PetBattle::Cast(BattlePet* caster, uint32 abilityId, uint8 turn, int8 procType)
{
    // make sure the ability exists
    auto abilityEntry = sBattlePetAbilityStore.LookupEntry(abilityId);
    if (!abilityEntry)
        return false;

    // prevent states being updated every tick for multiple turn abilities
    if (!turn)
    {
        // update any states the ability modifies
        for (uint32 i = 0; i != sBattlePetAbilityStateStore.GetNumRows(); i++)
        {
            auto abilityStateEntry = sBattlePetAbilityStateStore.LookupEntry(i);
            if (!abilityStateEntry)
                continue;

            if (abilityStateEntry->AbilityId != abilityId)
                continue;

            // update battle pet state
            UpdatePetState(caster, caster, 0, abilityStateEntry->StateId, caster->States[abilityStateEntry->StateId] + abilityStateEntry->Value);
        }
    }

    // handle ability effects
    for (uint32 i = 0; i < sBattlePetAbilityTurnStore.GetNumRows(); i++)
    {
        auto abilityTurnEntry = sBattlePetAbilityTurnStore.LookupEntry(i);
        if (!abilityTurnEntry)
            continue;

        if (abilityTurnEntry->AbilityId != abilityId)
            continue;

        // make sure multiple turn ability has done it's full duration
        if (abilityTurnEntry->Duration > 1 && abilityTurnEntry->Duration != turn)
            continue;

        if (abilityTurnEntry->ProcType != procType)
            continue;

        for (uint32 j = 0; j < sBattlePetAbilityEffectStore.GetNumRows(); j++)
        {
            auto abilityEffectEntry = sBattlePetAbilityEffectStore.LookupEntry(j);
            if (!abilityEffectEntry)
                continue;

            if (abilityEffectEntry->AbilityTurnId != abilityTurnEntry->Id)
                continue;

            // initialise ability effect
            BattlePetAbilityEffect abilityEffect;
            abilityEffect.SetAbilityInfo(abilityId, abilityEffectEntry, abilityEntry->FamilyId);
            abilityEffect.SetCaster(caster);
            abilityEffect.SetParentBattle(this);

            abilityEffect.AddTargets();
            abilityEffect.Execute();
        }
    }

    // update ability cooldown
    for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
    {
        if (caster->Abilities[i] && caster->Abilities[i]->AbilityId == abilityEntry->Id)
        {
            // make sure ability has a cooldown
            if (uint32 cooldown = abilityEntry->Cooldown)
            {
                caster->Abilities[i]->Cooldown   = cooldown;
                caster->Abilities[i]->OnCooldown = true;
            }
        }
    }

    return true;
}

void PetBattle::SwapActivePet(BattlePet* battlePet)
{
    auto team = Teams[battlePet->GetTeamIndex()];
    if (!team->IsValidBattlePet(battlePet))
        return;

    if (!team->CanSwap())
        return;

    // update team information
    team->ResetActiveAbility();
    team->SetActivePet(battlePet);
    team->SeenAction.insert(battlePet);

    // alert client of active pet swap
    PetBattleEffect effect(PET_BATTLE_EFFECT_ACTIVE_PET, battlePet->GetGlobalIndex());
    effect.TargetActivePet(battlePet->GetGlobalIndex());

    Effects.push_back(effect);
}

void PetBattle::UpdatePetState(BattlePet* source, BattlePet* target, uint32 abilityEffect, uint32 state, int32 value, uint16 flags)
{
    if (!sBattlePetStateStore.LookupEntry(state))
        return;

    uint8 modifyType = BattlePetStateModifyType[state];
    if (!modifyType)
        return;

    // make sure value is within allowed bounds
    if (modifyType == BATTLE_PET_STATE_MODIFY_TYPE_BOOL)
        value = value >= 1 ? 1 : 0;

    if (target->States[state] == value)
        return;

    target->States[state] = value;

    // notify client of state change
    PetBattleEffect effect(PET_BATTLE_EFFECT_SET_STATE, source->GetGlobalIndex(), flags, abilityEffect, TurnInstance++, 0, 1);
    effect.TargetUpdateState(target->GetGlobalIndex(), state, value);

    Effects.push_back(effect);
}

void PetBattle::AddAura(BattlePet* source, BattlePet* target, uint32 ability, uint32 abilityEffect, int32 duration, uint16 flags, uint8 maxAllowed)
{
    // count current auras for ability
    uint32 id = 0;
    uint32 auraCount = 0;

    for (auto aura : target->Auras)
    {
        if (aura->GetId() > id)
            id = aura->GetId();

        if (!aura->HasExpired() && aura->GetAbility() == ability)
            auraCount++;
    }

    id++;

    if (!duration && duration != -1)
        duration++;

    // notify client of aura update
    PetBattleEffect effect(PET_BATTLE_EFFECT_AURA_APPLY, source->GetGlobalIndex(), flags, abilityEffect, TurnInstance++, 0, 1);
    effect.TargetUpdateAura(target->GetGlobalIndex(), id, ability, duration, 0);

    Effects.push_back(effect);

    if (flags)
        return;

    // expire auras above the allowed count
    if (maxAllowed && auraCount >= maxAllowed)
    {
        uint8 removeCount = 1 + auraCount - maxAllowed;
        for (auto aura : target->Auras)
        {
            if (!removeCount)
                continue;

            if (!aura->HasExpired() && aura->GetAbility() == ability)
            {
                aura->Expire();
                removeCount--;
            }
        }
    }

    // create and apply aura
    auto aura = new PetBattleAura(this, id, ability, abilityEffect, source, target, duration);
    target->Auras.push_back(aura);

    aura->OnApply();
}

void PetBattle::Kill(BattlePet* killer, BattlePet* victim, uint32 abilityEffect, uint16 flags)
{
    auto victimTeam = Teams[victim->GetTeamIndex()];
    if (victimTeam->GetActivePet() == victim)
        victimTeam->ResetActiveAbility();

    // remove any auras the victim has
    for (auto aura : victim->Auras)
        aura->Expire();

    UpdatePetState(killer, victim, abilityEffect, BATTLE_PET_STATE_IS_DEAD, 1, flags);

    m_roundResult = PET_BATTLE_ROUND_RESULT_CATCH_OR_KILL;
}

void PetBattle::Catch(BattlePet* source, BattlePet* target, uint32 abilityEffect)
{
    Kill(source, target, abilityEffect, 0);
    m_cagedPet = target;

    m_roundResult = PET_BATTLE_ROUND_RESULT_CATCH_OR_KILL;
}

typedef std::vector<BattlePet*> ActivePetStore;

uint8 PetBattle::GetFirstAttackingTeam()
{
    auto challangerPet = Teams[PET_BATTLE_CHALLANGER_TEAM]->GetActivePet();
    auto opponentPet = Teams[PET_BATTLE_OPPONENT_TEAM]->GetActivePet();

    // return random team if active pet speed is the same
    if (challangerPet->GetSpeed() == opponentPet->GetSpeed())
        return urand(PET_BATTLE_CHALLANGER_TEAM, PET_BATTLE_OPPONENT_TEAM);

    return (challangerPet->GetSpeed() < opponentPet->GetSpeed());
}

PetBattleTeam* PetBattle::GetTeam(uint64 guid) const
{
    for (auto team : Teams)
    {
        // opponent team in PvE should always be a wild battle pet
        if (team->GetTeamIndex() == PET_BATTLE_OPPONENT_TEAM
            && GetType() == PET_BATTLE_TYPE_PVE)
            return nullptr;

        if (team->GetOwner()->GetGUID() == guid)
            return team;
    }

    return nullptr;
}

// -------------------------------------------------------------------------------

void PetBattleEffect::TargetActivePet(int8 targetPet)
{
    PetBattleEffectTarget target;
    target.Type   = PET_BATTLE_EFFECT_TARGET_ACTIVE_PET;
    target.Target = targetPet;

    Targets.push_back(target);
}

void PetBattleEffect::TargetUpdateHealth(int8 targetPet, uint32 health)
{
    PetBattleEffectTarget target;
    target.Type   = PET_BATTLE_EFFECT_TARGET_UPDATE_HEALTH;
    target.Target = targetPet;
    target.Health = health;

    Targets.push_back(target);
}

void PetBattleEffect::TargetUpdateState(int8 targetPet, uint32 state, uint32 value)
{
    PetBattleEffectTarget target;
    target.Type          = PET_BATTLE_EFFECT_TARGET_UPDATE_STATE;
    target.Target        = targetPet;
    target.State.StateId = state;
    target.State.Value   = value;

    Targets.push_back(target);
}

void PetBattleEffect::TargetUpdateAura(int8 targetPet, uint32 auraInstance, uint32 ability, int32 duration, uint32 turn)
{
    PetBattleEffectTarget target;
    target.Type                = PET_BATTLE_EFFECT_TARGET_UPDATE_AURA;
    target.Target              = targetPet;
    target.Aura.AuraInstanceId = auraInstance;
    target.Aura.AbilityId      = ability;
    target.Aura.CurrentRound   = turn;
    target.Aura.RoundsRemaing  = duration;

    Targets.push_back(target);
}

void PetBattleEffect::TargetUpdateStat(int8 targetPet, uint32 value)
{
    PetBattleEffectTarget target;
    target.Type      = PET_BATTLE_EFFECT_TARGET_UPDATE_STAT;
    target.Target    = targetPet;
    target.StatValue = value;

    Targets.push_back(target);
}

// -------------------------------------------------------------------------------

void PetBattleSystem::Update(uint32 diff)
{
    m_updatePetBattlesTimer += diff;
    m_removePetBattlesTimer += diff;

    // remove all pending deletion pet battles
    if (m_removePetBattlesTimer >= PET_BATTLE_SYSTEM_REMOVE_BATTLE_TIMER)
    {
        m_removePetBattlesTimer = 0;

        for (auto &petBattleToRemove : m_petBattlesToRemove)
            Remove(petBattleToRemove);

        m_petBattlesToRemove.clear();
    }

    // update all pet battles
    if (m_updatePetBattlesTimer >= PET_BATTLE_SYSTEM_UPDATE_BATTLE_TIMER)
    {
        for (auto &petBattleSet : m_petBattles)
        {
            auto petBattle = petBattleSet.second;
            switch (petBattle->GetPhase())
            {
                // update in progress pet battles
                case PET_BATTLE_PHASE_IN_PROGRESS:
                    petBattle->Update(diff + m_updatePetBattlesTimer);
                    break;
                // mark pet battle for deletion
                case PET_BATTLE_PHASE_FINISHED:
                    m_petBattlesToRemove.insert(petBattle);
                    break;
            }
        }

        m_updatePetBattlesTimer = 0;
    }
}

PetBattle* PetBattleSystem::Create(WorldObject* challenger, WorldObject* opponent, uint8 type)
{
    uint32 battleId = m_globalPetBattleId++;
    auto petBattle = new PetBattle(battleId, time(nullptr), type);

    // challenger will should only be a player
    auto challengerTeam = new PetBattleTeam(petBattle, PET_BATTLE_CHALLANGER_TEAM);
    challengerTeam->AddPlayer(challenger->ToPlayer());
    challengerTeam->ResetActiveAbility();

    petBattle->Teams.push_back(challengerTeam);

    // opponent can be another player or a wild battle pet
    auto opponentTeam = new PetBattleTeam(petBattle, PET_BATTLE_OPPONENT_TEAM);
    opponentTeam->ResetActiveAbility();

    if (type == PET_BATTLE_TYPE_PVE)
    {
        opponentTeam->AddWildBattlePet(opponent->ToCreature());

        // TODO: nearby wild battle pets should join the pet battle as well
        // ...
    }
    else
        opponentTeam->AddPlayer(opponent->ToPlayer());

    petBattle->Teams.push_back(opponentTeam);

    // add players to system
    m_playerPetBattles[challenger->GetGUID()] = battleId;
    if (type != PET_BATTLE_TYPE_PVE)
        m_playerPetBattles[opponent->GetGUID()] = battleId;

    // add pet battle to system
    m_petBattles[battleId] = petBattle;

    return petBattle;
}

void PetBattleSystem::Remove(PetBattle* petBattle)
{
    if (!petBattle)
        return;

    // remove players from the system
    for (auto team : petBattle->Teams)
        if (uint64 guid = team->GetOwnerGuid())
            m_playerPetBattles.erase(guid);

    m_petBattles.erase(petBattle->GetId());
    delete petBattle;
}

void PetBattleSystem::ForfietBattle(uint64 guid)
{
    // make sure player is currently in a pet battle
    auto petBattle = GetPlayerPetBattle(guid);
    if (!petBattle)
        return;

    auto team = petBattle->GetTeam(guid);
    ASSERT(team);

    ForfietBattle(petBattle, team);
}

void PetBattleSystem::ForfietBattle(PetBattle* petBattle, PetBattleTeam* team)
{
    if (!petBattle || !team)
        return;

    petBattle->FinishBattle(team, true);
}

PetBattle* PetBattleSystem::GetPlayerPetBattle(uint64 guid)
{
    // find players pet battle id
    auto playerPetBattle = m_playerPetBattles.find(guid);
    if (playerPetBattle == m_playerPetBattles.end())
        return nullptr;

    // find players pet battle instance
    auto petBattle = m_petBattles.find(playerPetBattle->second);
    if (petBattle == m_petBattles.end())
        return nullptr;

    return petBattle->second;
}
