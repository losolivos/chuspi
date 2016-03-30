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

#include "BattlePet.h"
#include "BattlePetMgr.h"
#include "BattlePetSpawnMgr.h"
#include "Common.h"
#include "DB2Enums.h"
#include "DB2Stores.h"
#include "Log.h"
#include "WorldSession.h"
#include "WorldPacket.h"

// -------------------------------------------------------------------------------
// Battle Pet
// -------------------------------------------------------------------------------

#define BATTLE_PET_MAX_DECLINED_NAMES 5

void WorldSession::HandleBattlePetModifyName(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PET_MODIFY_NAME");

    ObjectGuid petEntry;
    uint8 nicknameLen;
    std::string nickname;
    bool hasDeclinedNames;

    // TODO: Finish declined names
    uint8 declinedNameLen[BATTLE_PET_MAX_DECLINED_NAMES];
    std::string declinedNames[BATTLE_PET_MAX_DECLINED_NAMES];

    recvData.ReadBitSeq<3, 5, 1, 4, 0, 6>(petEntry);
    nicknameLen = recvData.ReadBits(7);
    recvData.ReadBitSeq<2, 7>(petEntry);
    hasDeclinedNames = recvData.ReadBit();

    if (hasDeclinedNames)
        for (uint8 i = 0; i < BATTLE_PET_MAX_DECLINED_NAMES; i++)
            declinedNameLen[i] = recvData.ReadBits(7);

    recvData.ReadByteSeq<1, 7, 3, 4, 0, 5, 2, 6>(petEntry);

    if (hasDeclinedNames)
        for (uint8 i = 0; i < BATTLE_PET_MAX_DECLINED_NAMES; i++)
            declinedNames[i] = recvData.ReadString(declinedNameLen[i]);

    nickname = recvData.ReadString(nicknameLen);

    // check if player owns the battle pet
    auto &battlePetMgr = GetPlayer()->GetBattlePetMgr();
    BattlePet* battlePet = battlePetMgr.GetBattlePet(petEntry);
    if (!battlePet)
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_MODIFY_NAME - Player %u tryed to set the name for Battle Pet " UI64FMTD " which it doesn't own!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    // check if nickname is a valid length
    if (nickname.size() > BATTLE_PET_MAX_NAME_LENGTH)
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_MODIFY_NAME - Player %u tryed to set the name for Battle Pet " UI64FMTD " with an invalid length!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    battlePet->SetNickname(nickname);
    battlePet->SetTimestamp((uint32)time(nullptr));

    // update current summoned battle pet timestamp, this makes the client request the name again
    if (auto summon = battlePetMgr.GetCurrentSummon())
        summon->SetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP, battlePet->GetTimestamp());
}

void WorldSession::HandleBattlePetRelease(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PET_DELETE");

    ObjectGuid petEntry;

    recvData.ReadBitSeq<0, 4, 2, 6, 3, 7, 5, 1>(petEntry);
    recvData.ReadByteSeq<4, 2, 7, 1, 5, 0, 3, 6>(petEntry);

    BattlePet* battlePet = GetPlayer()->GetBattlePetMgr().GetBattlePet(petEntry);
    if (!battlePet)
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_DELETE - Player %u tryed to release Battle Pet " UI64FMTD " which it doesn't own!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    if (!HasBattlePetSpeciesFlag(battlePet->GetSpecies(), BATTLE_PET_FLAG_RELEASABLE))
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_DELETE - Player %u tryed to release Battle Pet " UI64FMTD " which isn't releasable!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    GetPlayer()->GetBattlePetMgr().Delete(battlePet);
}

enum BattlePetFlagModes
{
    BATTLE_PET_FLAG_MODE_SET         = 0,
    BATTLE_PET_FLAG_MODE_TOGGLE      = 1,
    BATTLE_PET_FLAG_MODE_UNSET       = 2
};

void WorldSession::HandleBattlePetSetBattleSlot(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PET_SET_BATTLE_SLOT");

    uint8 slot;
    ObjectGuid petEntry;

    recvData >> slot;
    recvData.ReadBitSeq<7, 0, 6, 5, 2, 3, 4, 1>(petEntry);
    recvData.ReadByteSeq<3, 2, 4, 7, 5, 1, 6, 0>(petEntry);

    auto &battlePetMgr = GetPlayer()->GetBattlePetMgr();

    BattlePet* battlePet = battlePetMgr.GetBattlePet(petEntry);
    if (!battlePet)
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_SET_BATTLE_SLOT - Player %u tryed to add Battle Pet " UI64FMTD " to loadout which it doesn't own!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    if (!battlePetMgr.HasLoadoutSlot(slot))
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_SET_BATTLE_SLOT - Player %u tryed to add Battle Pet " UI64FMTD " into slot %u which is locked!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry, slot);
        return;
    }

    // this check is also done clientside
    if (HasBattlePetSpeciesFlag(battlePet->GetSpecies(), BATTLE_PET_FLAG_COMPANION))
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_SET_BATTLE_SLOT - Player %u tryed to add a compainion Battle Pet " UI64FMTD " into slot %u!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry, slot);
        return;
    }

    // sever handles slot swapping, find source slot and replace it with the destination slot
    uint8 srcSlot = battlePetMgr.GetLoadoutSlotForBattlePet(petEntry);
    if (srcSlot != BATTLE_PET_LOADOUT_SLOT_NONE)
        battlePetMgr.SetLoadoutSlot(srcSlot, battlePetMgr.GetLoadoutSlot(slot), true);

    battlePetMgr.SetLoadoutSlot(slot, petEntry, true);
}

void WorldSession::HandleBattlePetSetFlags(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PET_SET_FLAGS");

    ObjectGuid petEntry;
    uint32 flag;
    uint8 mode;

    recvData >> flag;

    recvData.ReadBitSeq<0>(petEntry);
    mode = recvData.ReadBits(2);
    recvData.ReadBitSeq<7, 3, 5, 1, 2, 4, 6>(petEntry);

    recvData.ReadByteSeq<3, 4, 2, 5, 6, 0, 7, 1>(petEntry);

    // check if player owns the battle pet
    auto battlePet = GetPlayer()->GetBattlePetMgr().GetBattlePet(petEntry);
    if (!battlePet)
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_SET_FLAGS - Player %u tryed to set the flags for Battle Pet " UI64FMTD " which it doesn't own!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    // list of flags the client can currently change
    if (flag != BATTLE_PET_JOURNAL_FLAG_FAVORITES
        && flag != BATTLE_PET_JOURNAL_FLAG_ABILITY_1
        && flag != BATTLE_PET_JOURNAL_FLAG_ABILITY_2
        && flag != BATTLE_PET_JOURNAL_FLAG_ABILITY_3)
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_SET_FLAGS - Player %u tryed to set an invalid Battle Pet flag %u!",
            GetPlayer()->GetGUIDLow(), flag);
        return;
    }
    
    // handle flag
    switch (mode)
    {
        case BATTLE_PET_FLAG_MODE_SET:
            battlePet->SetFlag(flag);
            break;
        case BATTLE_PET_FLAG_MODE_TOGGLE:
            battlePet->HasFlag(flag) ? battlePet->UnSetFlag(flag) : battlePet->SetFlag(flag);
            break;
        case BATTLE_PET_FLAG_MODE_UNSET:
            battlePet->UnSetFlag(flag);
            break;
    }

    battlePet->InitialiseAbilities(false);
}

void WorldSession::HandleQueryBattlePetName(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUERY_BATTLE_PET_NAME");

    ObjectGuid petEntry, petGuid;

    recvData.ReadBitSeq<6, 3, 1>(petEntry);
    recvData.ReadBitSeq<0, 1>(petGuid);
    recvData.ReadBitSeq<4, 2>(petEntry);
    recvData.ReadBitSeq<3, 7, 4>(petGuid);
    recvData.ReadBitSeq<5>(petEntry);
    recvData.ReadBitSeq<6, 5>(petGuid);
    recvData.ReadBitSeq<0>(petEntry);
    recvData.ReadBitSeq<2>(petGuid);
    recvData.ReadBitSeq<7>(petEntry);

    recvData.ReadByteSeq<4, 6, 5>(petEntry);
    recvData.ReadByteSeq<0, 5>(petGuid);
    recvData.ReadByteSeq<3>(petEntry);
    recvData.ReadByteSeq<4>(petGuid);
    recvData.ReadByteSeq<2, 1>(petEntry);
    recvData.ReadByteSeq<6>(petGuid);
    recvData.ReadByteSeq<0>(petEntry);
    recvData.ReadByteSeq<2, 7, 1, 3>(petGuid);
    recvData.ReadByteSeq<7>(petEntry);

    Unit* petUnit = sObjectAccessor->FindUnit(petGuid);
    if (!petUnit)
    {
        TC_LOG_DEBUG("network", "CMSG_BATTLE_PET_QUERY_NAME - Player %u queried the name of Battle Pet " UI64FMTD " which doesnt't exist in world!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    Unit* ownerUnit = petUnit->ToTempSummon()->GetSummoner();
    if (!ownerUnit)
        return;

    auto &battlePetMgr = ownerUnit->ToPlayer()->GetBattlePetMgr();
    BattlePet* battlePet = battlePetMgr.GetBattlePet(battlePetMgr.GetCurrentSummonId());
    if (!battlePet)
        return;

    BattlePetSpeciesEntry const* speciesEntry = sBattlePetSpeciesStore.LookupEntry(battlePet->GetSpecies());

    WorldPacket data(SMSG_QUERY_BATTLEPET_NAME_RESPONSE, 8 + 4 + 4 + 5 + battlePet->GetNickname().size());
    data.WriteBit(1);                                   // HasDeclined
    data.WriteBits(battlePet->GetNickname().size(), 8);

    for (uint8 i = 0; i < BATTLE_PET_MAX_DECLINED_NAMES; i++)
        data.WriteBits(0, 7);

    data.WriteBit(0);                                   // Allow
    data.FlushBits();

    data.WriteString(battlePet->GetNickname());
    data << uint64(petEntry);
    data << uint32(speciesEntry->NpcId);
    data << uint32(battlePet->GetTimestamp());

    SendPacket(&data);
}

void WorldSession::HandleSummonBattlePetCompanion(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_SUMMON_BATTLEPET_COMPANION");

    ObjectGuid petEntry;

    recvData.ReadBitSeq<7, 5, 0, 2, 4, 6, 3, 1>(petEntry);
    recvData.ReadByteSeq<4, 1, 0, 2, 6, 3, 7, 5>(petEntry);

    auto &battlePetMgr = GetPlayer()->GetBattlePetMgr();

    // check if player owns the battle pet
    BattlePet* battlePet = battlePetMgr.GetBattlePet(petEntry);
    if (!battlePet)
    {
        TC_LOG_DEBUG("network", "CMSG_SUMMON_BATTLE_PET_COMPANION - Player %u tryed to summon battle pet companion " UI64FMTD " which it doesn't own!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    // make sure the battle pet is alive
    if (!battlePet->GetCurrentHealth())
    {
        TC_LOG_DEBUG("network", "CMSG_SUMMON_BATTLE_PET_COMPANION - Player %u tryed to summon battle pet companion " UI64FMTD " which is dead!",
            GetPlayer()->GetGUIDLow(), (uint64)petEntry);
        return;
    }

    // player wants to unsummon the current battle pet
    if (battlePetMgr.GetCurrentSummonId() == petEntry)
        battlePetMgr.UnSummonCurrentBattlePet(false);
    else
    {
        // player wants to summon a new battle pet
        battlePetMgr.SetCurrentSummonId(petEntry);
        GetPlayer()->CastSpell(GetPlayer(), SPELL_BATTLE_PET_SUMMON, false);
    }
}

// -------------------------------------------------------------------------------
// Pet Battle
// -------------------------------------------------------------------------------

void WorldSession::HandlePetBattleInput(WorldPacket& recvData)
{
    bool hasByte13 = !recvData.ReadBit();
    bool hasIgnoreAbandonPenalty = recvData.ReadBit();
    bool hasAbilityId = !recvData.ReadBit();
    bool hasNewFrontPet = !recvData.ReadBit();
    bool hasByte12 = !recvData.ReadBit();
    bool hasMoveType = !recvData.ReadBit();
    bool hasRound = !recvData.ReadBit();

    uint8 moveType = 0;
    uint8 newFrontPet = 0;
    uint8 byte12 = 0;                               // DebugFlags, use unknown
    uint8 byte13 = 0;                               // BattleInterrupted, use unknown
    uint32 abilityId = 0;
    uint32 round = 0;

    if (hasByte13)
        recvData >> byte13;

    if (hasNewFrontPet)
        recvData >> newFrontPet;

    if (hasRound)
        recvData >> round;

    if (hasAbilityId)
        recvData >> abilityId;

    if (hasByte12)
        recvData >> byte12;

    if (hasMoveType)
        recvData >> moveType;

    // make sure player is in a pet battle
    auto petBattle = sPetBattleSystem->GetPlayerPetBattle(_player->GetGUID());
    if (!petBattle)
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_INPUT - Player %u(%s) tried to make a pet battle move while not in battle!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str());
        return;
    }

    // make sure the pet battle is in progress
    if (petBattle->GetPhase() != PET_BATTLE_PHASE_IN_PROGRESS)
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_INPUT - Player %u(%s) tried to make a move in a pet battle that isn't in progress!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str());
        return;
    }

    if (hasAbilityId)
    {
        // make sure ability is valid
        if (!sBattlePetAbilityStore.LookupEntry(abilityId))
        {
            TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_INPUT - Player %u(%s) tried use an invalid battle pet ability %u!",
                GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str(), abilityId);
            return;
        }
    }

    if (hasNewFrontPet)
    {
        // make sure active pet is valid
        if (newFrontPet >= PET_BATTLE_MAX_TEAM_PETS)
        {
            TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_INPUT - Player %u(%s) tried to switch to invalid front pet %u!",
                GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str(), newFrontPet);
            return;
        }
    }

    auto team = petBattle->GetTeam(_player->GetGUID());
    ASSERT(team);

    // make sure players team hasn't already made a move this round
    if (team->IsReady())
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_INPUT - Player %u(%s) tried use an invalid battle pet ability %u!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str(), abilityId);
        return;
    }

    // make sure new active pet is valid
    BattlePet* battlePet = nullptr;
    if (hasNewFrontPet)
    {
        battlePet = team->BattlePets[team->ConvertToLocalIndex(newFrontPet)];
        if (!battlePet)
        {
            TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_INPUT - Player %u(%s) tried to swap to invalid team battle pet %u!",
                GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str(), newFrontPet);
            return;
        }
    }

    // make sure move type is valid
    if (moveType >= PET_BATTLE_MOVE_TYPE_COUNT)
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_INPUT - Player %u(%s) tried to do invalid move %u!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str(), moveType);
        return;
    }

    if (moveType == PET_BATTLE_MOVE_TYPE_REQUEST_LEAVE)
    {
        sPetBattleSystem->ForfietBattle(petBattle, team);
        return;
    }

    team->SetPendingMove(moveType, abilityId, battlePet);
}

void WorldSession::HandlePetBattleRequestWild(WorldPacket& recvData)
{
    PetBattleRequest petBattleRequest;

    bool hasLocationResult, hasBattleFacing;
    ObjectGuid wildBattlePetGuid;

    recvData >> petBattleRequest.BattleOrigin[1] >> petBattleRequest.BattleOrigin[2];

    for (uint8 i = 0; i < PET_BATTLE_MAX_TEAMS; i++)
        recvData >> petBattleRequest.TeamPositions[i][2] >> petBattleRequest.TeamPositions[i][0] >> petBattleRequest.TeamPositions[i][1];

    recvData >> petBattleRequest.BattleOrigin[0];

    recvData.ReadBitSeq<4, 1, 0, 5>(wildBattlePetGuid);
    hasLocationResult = !recvData.ReadBit();
    hasBattleFacing = !recvData.ReadBit();
    recvData.ReadBitSeq<7, 6, 2, 3>(wildBattlePetGuid);

    recvData.ReadByteSeq<3, 2, 6, 1, 7, 5, 4, 0>(wildBattlePetGuid);

    if (hasLocationResult)
        recvData >> petBattleRequest.LocationResult;

    if (hasBattleFacing)
        recvData >> petBattleRequest.BattleFacing;

    petBattleRequest.OpponentGuid = wildBattlePetGuid;

    // check if player is dead
    if (!GetPlayer()->IsAlive())
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_REQUEST_WILD - Player %u(%s) tried to initiate a wild pet battle while dead!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str());

        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_DEAD);
        return;
    }

    // check if player is in combat
    if (GetPlayer()->IsInCombat())
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_REQUEST_WILD - Player %u(%s) tried to initiate a wild pet battle while in combat!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str());

        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_ALREADY_IN_COMBAT);
        return;
    }

    // check if player isn't already in a battle
    if (sPetBattleSystem->GetPlayerPetBattle(GetPlayer()->GetGUID()))
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_REQUEST_WILD - Player %u(%s) tried to initiate a new wild pet battle while still in an old pet battle!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str());

        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_ALREADY_IN_PETBATTLE);
        return;
    }

    Creature* wildBattlePet = ObjectAccessor::GetCreatureOrPetOrVehicle(*GetPlayer(), petBattleRequest.OpponentGuid);
    if (!wildBattlePet)
    {
        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_INVALID_TARGET);
        return;
    }

    // player must be able to interact with the creature
    if (!wildBattlePet->IsWithinDistInMap(GetPlayer(), PETBATTLE_INTERACTION_DIST))
    {
        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_TOO_FAR);
        return;
    }

    if (!GetPlayer()->GetNPCIfCanInteractWith(petBattleRequest.OpponentGuid, UNIT_NPC_FLAG_PETBATTLE))
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_REQUEST_WILD - Player %u(%s) tried to initiate a wild pet battle but can't interact with opponent %u!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str(), wildBattlePet->GetGUIDLow());

        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_NOT_VALID_TARGET);
        return;
    }

    // check if creature is a wild battle pet
    if (!sBattlePetSpawnMgr->GetWildBattlePet(wildBattlePet))
    {
        TC_LOG_DEBUG("network", "CMSG_PET_BATTLE_REQUEST_WILD - Player %u(%s) tried to initiate a wild pet battle but creature %u isn't a wild battle pet!",
            GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str(), wildBattlePet->GetGUIDLow());

        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_INVALID_TARGET);
        return;
    }

    auto &battlePetMgr = GetPlayer()->GetBattlePetMgr();

    // player needs to have at least one battle pet loadout slot populated
    if (!battlePetMgr.GetLoadoutSlot(0))
    {
        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_NEED_AT_LEAST_1_PET_IN_SLOT);
        return;
    }

    // make sure all of the players loadout pets aren't dead
    bool allDead = true;
    for (uint8 i = 0; i < BATTLE_PET_MAX_LOADOUT_SLOTS; i++)
        if (auto battlePet = battlePetMgr.GetBattlePet(battlePetMgr.GetLoadoutSlot(i)))
            if (battlePet->IsAlive())
            {
                allDead = false;
                break;
            }

    if (allDead)
    {
        SendPetBattleRequestFailed(PET_BATTLE_REQUEST_PET_ALL_DEAD);
        return;
    }

    // make sure there is enough room for the pet battle
    for (uint8 i = 0; i < PET_BATTLE_MAX_TEAMS; i++)
    {
        if (_player->GetMap()->getObjectHitPos(_player->GetPhaseMask(), petBattleRequest.BattleOrigin[0], petBattleRequest.BattleOrigin[1], petBattleRequest.BattleOrigin[2],
            petBattleRequest.TeamPositions[i][0], petBattleRequest.TeamPositions[i][1], petBattleRequest.TeamPositions[i][2],
            petBattleRequest.TeamPositions[i][0], petBattleRequest.TeamPositions[i][1], petBattleRequest.TeamPositions[i][2], 0.0f))
        {
            SendPetBattleRequestFailed(PET_BATTLE_REQUEST_GROUND_NOT_ENOUGHT_SMOOTH);
            return;
        }
    }

    SendPetBattleFinaliseLocation(petBattleRequest);

    // setup player
    _player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC);
    _player->SetTarget(wildBattlePet->GetGUID());
    _player->SetFacingTo(_player->GetAngle(petBattleRequest.TeamPositions[PET_BATTLE_OPPONENT_TEAM][0], petBattleRequest.TeamPositions[PET_BATTLE_OPPONENT_TEAM][1]));
    _player->SetRooted(true);

    // setup wild battle pet
    wildBattlePet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_PC);
    wildBattlePet->SetTarget(GetPlayer()->GetGUID());
    wildBattlePet->SetControlled(true, UNIT_STATE_ROOT);

    sBattlePetSpawnMgr->EnteredBattle(wildBattlePet);

    // create and start pet battle
    sPetBattleSystem->Create(GetPlayer(), wildBattlePet, PET_BATTLE_TYPE_PVE)->StartBattle();
}

// -------------------------------------------------------------------------------

void WorldSession::SendPetBattleFinaliseLocation(PetBattleRequest& request)
{
    WorldPacket data(SMSG_PET_BATTLE_FINALISE_LOCATION, 2 * (4 + 4 + 4) + 4 + 4 + 4 + 4 + 4);
    data << request.BattleOrigin[0];

    for (uint8 i = 0; i < PET_BATTLE_MAX_TEAMS; i++)
        data << request.TeamPositions[i][0] << request.TeamPositions[i][1] << request.TeamPositions[i][2];

    data << request.BattleOrigin[1] << request.BattleOrigin[2];

    data.WriteBit(1);                               // HasLocationResult
    data.WriteBit(1);                               // HasBattleFacing
    data.FlushBits();

    SendPacket(&data);
}

void WorldSession::SendPetBattleFirstRound(PetBattle* petBattle)
{
    ByteBuffer effectData;
    WorldPacket data(SMSG_PET_BATTLE_FIRST_ROUND, 100);
    data << uint32(petBattle->GetRound());

    for (auto &team : petBattle->Teams)
    {
        data << uint16(0);                          // RoundTimeSecs
        data << uint8(team->GetInputStatusFlags());
        data << uint8(team->GetTrapStatus());
    }

    data.WriteBits(petBattle->Effects.size(), 22);

    for (auto &effect : petBattle->Effects)
    {
        // since this is the first round, only active pet should be updated?
        uint8 effectType = effect.GetType();
        ASSERT(effectType == PET_BATTLE_EFFECT_ACTIVE_PET);

        data.WriteBit(!effect.GetSource());
        data.WriteBits(effect.Targets.size(), 25);
        data.WriteBit(!false);
        data.WriteBit(!false);
        data.WriteBit(!effect.GetType());
        data.WriteBit(!false);

        for (auto &target : effect.Targets)
        {
            data.WriteBit(!true);
            data.WriteBits(target.Type, 3);
        }

        data.WriteBit(!false);
        data.WriteBit(!false);

        for (auto &target : effect.Targets)
            effectData << uint8(target.Target);

        if (uint8 effectType = effect.GetType())
            effectData << uint8(effectType);

        /*if (abilityEffect)
            effectData << uint32(0);*/              // AbilityEffectId

        if (uint8 source = effect.GetSource())
            effectData << uint8(source);

        /*if (hasStackDepth)
            effectData << uint8(0);                 // StackDepth

        if (true)
            effectData << uint16(0);

        if (true)
            effectData << uint16(0);

        if (true)
            effectData << uint16(0);*/
    }

    data.WriteBits(0, 20);
    data.WriteBit(!true);                           // NextPetBattleState
    data.WriteBits(0, 3);

    /*for (uint8 i = 0; i < cooldownCount; i++)
    {
    }*/

    data.FlushBits();

    /*for (uint8 i = 0; i < cooldownCount; i++)
    {
    }*/

    data.append(effectData);

    // always send next pet battle state
    if (true)
        data << uint8(PET_BATTLE_ROUND_RESULT_NORMAL);

    SendPacket(&data);
}

void WorldSession::SendPetBattleInitialUpdate(PetBattle* petBattle)
{
    bool hasWatingForFrontPetsMaxSecs = true;
    bool hasPvPMaxRoundTime = true;
    bool hasForfietPenalty = true;
    bool hasCreatureId = false;
    bool hasDisplayId = false;

    // only set wild battle pet guid if you are in a pve match
    ObjectGuid wildBattlePetGuid = 0;
    if (petBattle->GetType() == PET_BATTLE_TYPE_PVE)
        wildBattlePetGuid = petBattle->Teams[PET_BATTLE_OPPONENT_TEAM]->GetWildBattlePet()->GetGUID();

    ByteBuffer playerUpdateData;
    WorldPacket data(SMSG_PET_BATTLE_INITIAL_UPDATE, 1000);

    for (auto &team : petBattle->Teams)
    {
        bool hasRoundTimeSec = false;
        bool hasFrontPet = true;

        // no character if opponent is a wild battle pet
        ObjectGuid characterGuid = 0;
        if ((petBattle->GetType() != PET_BATTLE_TYPE_PVE || team->GetTeamIndex() == PET_BATTLE_CHALLANGER_TEAM))
            characterGuid = team->GetOwner()->GetGUID();

        data.WriteBitSeq<5, 6, 3>(characterGuid);
        data.WriteBit(!hasFrontPet);
        data.WriteBit(!true);                       // always send trap status
        data.WriteBitSeq<2, 7, 4>(characterGuid);
        data.WriteBits(team->BattlePets.size(), 2);

        for (auto &battlePet : team->BattlePets)
        {
            uint8 abilityCount = 0;
            for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
                if (battlePet->Abilities[i])
                    abilityCount++;

            ObjectGuid petEntry = battlePet->GetId();

            data.WriteBitSeq<7>(petEntry);
            data.WriteBits(0, 21);                  // StateCount
            data.WriteBitSeq<5>(petEntry);
            data.WriteBits(abilityCount, 20);
            data.WriteBit(!true);
            data.WriteBitSeq<3, 0, 1, 6>(petEntry);

            for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
                if (battlePet->Abilities[i])
                    data.WriteBit(!true);

            data.WriteBitSeq<4>(petEntry);
            data.WriteBits(0, 21);                  // AuraCount

            /*for (uint8 i = 0; i < auraCount; i++)
            {
            }*/

            data.WriteBits(battlePet->GetNickname().size(), 7);
            data.WriteBitSeq<2>(petEntry);
            data.WriteBit(!battlePet->GetFlags());
        }

        data.WriteBitSeq<1>(characterGuid);
        data.WriteBit(!hasRoundTimeSec);            // hasRoundTimeSec or hasTrapAbilityId
        data.WriteBitSeq<0>(characterGuid);

        playerUpdateData.WriteByteSeq<1, 6>(characterGuid);

        uint8 battlePetTeamIndex = 0;
        for (auto &battlePet : team->BattlePets)
        {
            ObjectGuid petEntry = battlePet->GetId();

            playerUpdateData << uint32(0);
            playerUpdateData.WriteByteSeq<4>(petEntry);
            playerUpdateData << uint32(0);

            /*for (uint8 i = 0; i < auraCount; i++)
            {
            }*/

            if (uint16 flags = battlePet->GetFlags())
                playerUpdateData << uint16(flags);

            playerUpdateData.WriteByteSeq<7>(petEntry);

            /*for (uint8 i = 0; i < stateCount; i++)
            {
            }*/

            playerUpdateData << uint32(0);

            for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
            {
                if (!battlePet->Abilities[i])
                    continue;

                playerUpdateData << uint32(battlePet->Abilities[i]->AbilityId);
                playerUpdateData << uint16(battlePet->Abilities[i]->Lockdown);
                playerUpdateData << uint16(battlePet->Abilities[i]->Cooldown);
                playerUpdateData << uint8(i);

                // always send global index
                if (true)
                    playerUpdateData << uint8(battlePet->GetGlobalIndex());
            }

            playerUpdateData << uint16(battlePet->GetQuality());
            playerUpdateData << uint32(battlePet->GetSpecies());
            playerUpdateData.WriteByteSeq<3, 1, 2, 0, 5>(petEntry);
            playerUpdateData.WriteString(battlePet->GetNickname());
            playerUpdateData << uint32(battlePet->GetSpeed());
            playerUpdateData << uint16(battlePet->GetLevel());
            playerUpdateData.WriteByteSeq<6>(petEntry);
            playerUpdateData << uint32(battlePet->GetCurrentHealth());
            playerUpdateData << uint32(battlePet->GetMaxHealth());
            playerUpdateData << uint16(battlePet->GetXp());
            playerUpdateData << uint32(battlePet->GetPower());

            // always send local index
            if (true)
                playerUpdateData << uint8(battlePetTeamIndex++);
        }

        playerUpdateData.WriteByteSeq<0>(characterGuid);

        if (hasFrontPet)
            playerUpdateData << uint8(team->ConvertToLocalIndex(team->GetActivePet()->GetGlobalIndex()));

        playerUpdateData << uint32(team->GetOwner() ? team->GetOwner()->GetBattlePetMgr().GetTrapAbility() : 0);

        if (true)
            playerUpdateData << uint32(team->GetTrapStatus());

        playerUpdateData.WriteByteSeq<4>(characterGuid);

        if (hasRoundTimeSec)
            playerUpdateData << uint16(0);

        playerUpdateData.WriteByteSeq<7>(characterGuid);
        playerUpdateData << uint8(6);               // InputFlags?
        playerUpdateData.WriteByteSeq<2, 3, 5>(characterGuid);
    }

    data.WriteBit(!hasPvPMaxRoundTime);

    // enviroment update (bits)
    for (uint8 i = 0; i < 3; i++)
    {
        data.WriteBits(0, 21);                      // AuraCount
        data.WriteBits(0, 21);                      // StateCount
    }

    data.WriteBit(!wildBattlePetGuid);              // Fake?
    data.WriteBitSeq<3, 0, 2, 5, 7, 1, 4, 6>(wildBattlePetGuid);
    data.WriteBit(false);                           // CanAwardXP?
    data.WriteBit(!true);                           // CurPetBattleState
    data.WriteBit(false);                           // IsPvP?
    data.WriteBit(!hasCreatureId);
    data.WriteBit(!hasDisplayId);
    data.WriteBit(!hasWatingForFrontPetsMaxSecs);
    data.WriteBit(!hasForfietPenalty);
    data.FlushBits();

    // always send initial pet battle state
    if (true)
        data << uint8(PET_BATTLE_ROUND_RESULT_INITIALISE);

    data.append(playerUpdateData);
    data << uint32(petBattle->GetRound());
    data.WriteByteSeq<5, 4, 0, 1, 2, 3, 6, 7>(wildBattlePetGuid);

    // enviroment update (bytes)
    for (uint8 i = 0; i < 3; i++)
    {
        /*for (uint8 i = 0; i < auraCount; i++)
        {
        }*/

        /*for (uint8 i = 0; i < stateCount; i++)
        {
        }*/
    }

    if (hasForfietPenalty)
        data << uint8(10);

    // or hasPvPMaxRoundTime
    if (hasWatingForFrontPetsMaxSecs)
        data << uint16(30);

    if (hasDisplayId)
        data << uint32(0);

    // or hasWatingForFrontPetsMaxSecs
    if (hasPvPMaxRoundTime)
        data << uint16(30);

    if (hasCreatureId)
        data << uint32(0);

    SendPacket(&data);
}

void WorldSession::SendPetBattleRequestFailed(uint8 reason)
{
    WorldPacket data(SMSG_PET_BATTLE_REQUEST_FAILED, 2);
    data.WriteBit(0);
    data.FlushBits();

    data << uint8(reason);
    SendPacket(&data);
}

void WorldSession::SendPetBattleRoundResult(PetBattle* petBattle)
{
    // get players team
    auto team = petBattle->Teams[PET_BATTLE_CHALLANGER_TEAM];
    if (petBattle->GetType() != PET_BATTLE_TYPE_PVE && petBattle->Teams[PET_BATTLE_OPPONENT_TEAM]->GetOwner() == _player)
        team = petBattle->Teams[PET_BATTLE_OPPONENT_TEAM];

    ByteBuffer effectData, cooldownData;
    WorldPacket data(SMSG_PET_BATTLE_ROUND_RESULT, 300);

    for (auto &team : petBattle->Teams)
    {
        data << uint8(team->GetTrapStatus());
        data << uint8(team->GetInputStatusFlags());
        data << uint16(0);                          // RoundTimeSecs
    }

    data << uint32(petBattle->GetRound());

    data.WriteBit(!petBattle->GetRoundResult());
    data.WriteBits(0, 3);
    data.WriteBits(petBattle->Effects.size(), 22);

    for (auto &effect : petBattle->Effects)
    {
        bool unkWord1 = true;
        bool unkWord3 = true;

        data.WriteBit(!effect.GetSource());
        data.WriteBit(!effect.GetAbilityEffect());
        data.WriteBit(!unkWord1);
        data.WriteBits(effect.Targets.size(), 25);
        data.WriteBit(!effect.GetStackDepth());
        data.WriteBit(!effect.GetFlags());
        data.WriteBit(!effect.GetType());

        for (auto &target : effect.Targets)
        {
            data.WriteBits(target.Type, 3);

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UPDATE_HEALTH)
                data.WriteBit(!target.Health);

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UPDATE_AURA)
            {
                data.WriteBit(!target.Aura.AuraInstanceId);
                data.WriteBit(!target.Aura.AbilityId);
                data.WriteBit(!target.Aura.CurrentRound);
                data.WriteBit(!target.Aura.RoundsRemaing);
            }

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UNK5)
            {
                data.WriteBit(!false);
                data.WriteBit(!false);
                data.WriteBit(!false);
            }

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UNK0)
                data.WriteBit(!false);

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UPDATE_STATE)
            {
                data.WriteBit(!target.State.Value);
                data.WriteBit(!target.State.StateId);
            }

            data.WriteBit(!(target.Target != PET_BATTLE_NULL_PET_INDEX));

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UPDATE_STAT)
                data.WriteBit(!target.StatValue);

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UNK1)
                data.WriteBit(!false);
        }

        data.WriteBit(!unkWord3);

        for (auto &target : effect.Targets)
        {
            if (target.Type == PET_BATTLE_EFFECT_TARGET_UPDATE_STATE)
            {
                if (uint32 value = target.State.Value)
                    effectData << uint32(value);

                if (uint32 stateId = target.State.StateId)
                    effectData << uint32(stateId);
            }

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UNK5)
            {
                if (false)
                    effectData << uint32(0);

                if (false)
                    effectData << uint32(0);

                if (false)
                    effectData << uint32(0);
            }

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UPDATE_AURA)
            {
                if (uint32 ability = target.Aura.AbilityId)
                    effectData << uint32(ability);

                if (uint32 roundsRemaining = target.Aura.RoundsRemaing)
                    effectData << uint32(roundsRemaining);

                if (uint32 currentRound = target.Aura.CurrentRound)
                    effectData << uint32(currentRound);

                if (uint32 auraInstanceId = target.Aura.AuraInstanceId)
                    effectData << uint32(auraInstanceId);
            }

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UNK0)
                effectData << uint32(0);

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UPDATE_STAT)
                if (uint32 stateValue = target.StatValue)
                    effectData << uint32(stateValue);

            if (target.Target != PET_BATTLE_NULL_PET_INDEX)
                effectData << uint8(target.Target);

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UNK1)
                effectData << uint32(0);

            if (target.Type == PET_BATTLE_EFFECT_TARGET_UPDATE_HEALTH)
                if (uint32 health = target.Health)
                    effectData << uint32(health);
        }

        if (uint8 stackDepth = effect.GetStackDepth())
            effectData << uint8(stackDepth);

        if (unkWord3)
            effectData << uint16(0);

        if (uint16 flags = effect.GetFlags())
            effectData << uint16(flags);

        if (uint8 source = effect.GetSource())
            effectData << uint8(source);

        if (uint8 type = effect.GetType())
            effectData << uint8(type);

        if (unkWord1)
            effectData << uint16(0);

        if (uint32 abilityEffect = effect.GetAbilityEffect())
            effectData << uint32(abilityEffect);
    }

    uint32 cooldownCount = 0;

    size_t cooldownPos = data.bitwpos();
    data.WriteBits(cooldownCount, 20);

    for (auto battlePet : team->BattlePets)
    {
        for (uint8 i = 0; i < BATTLE_PET_MAX_ABILITIES; i++)
        {
            if (!battlePet->Abilities[i])
                continue;

            if (!battlePet->Abilities[i]->OnCooldown)
                continue;

            if (!battlePet->Abilities[i]->Cooldown)
                battlePet->Abilities[i]->OnCooldown = false;

            data.WriteBit(!true);

            // always send global index
            if (true)
                cooldownData << uint8(battlePet->GetGlobalIndex());

            cooldownData << uint32(battlePet->Abilities[i]->AbilityId);
            cooldownData << uint8(i);
            cooldownData << uint16(battlePet->Abilities[i]->Lockdown);
            cooldownData << uint16(battlePet->Abilities[i]->Cooldown);

            cooldownCount++;
        }
    }

    data.FlushBits();

    data.PutBits(cooldownPos, cooldownCount, 20);

    data.append(cooldownData);
    data.append(effectData);

    if (uint8 roundResult = petBattle->GetRoundResult())
        data << uint8(roundResult);

    SendPacket(&data);
}

void WorldSession::SendPetBatatleFinalRound(PetBattle* petBattle)
{
    uint8 battlePetCount = 0;

    ByteBuffer battlePetData;
    WorldPacket data(SMSG_PET_BATTLE_FINAL_ROUND, 100);

    for (auto team : petBattle->Teams)
        data.WriteBit(petBattle->GetWinningTeam() == team);

    data.WriteBit(true);                            // Abandoned (hackfix! client gets stuck in battle if this is false)
    data.WriteBit(false);                           // PvpBattle

    size_t battlePetCountPos = data.bitwpos();
    data.WriteBits(battlePetCount, 20);

    for (auto team : petBattle->Teams)
    {
        for (auto battlePet : team->BattlePets)
        {
            bool caged = petBattle->GetCagedPet() == battlePet;

            // always send XP and level information
            data.WriteBit(!true);
            data.WriteBit(!true);
            data.WriteBit(!true);

            // if captured = false && caged = true - "Your trapped pet was lost. You must win the battle to claim your trapped pet."
            data.WriteBit(caged && petBattle->GetWinningTeam() != team);                    // Captured
            data.WriteBit(team->SeenAction.find(battlePet) != team->SeenAction.end());      // AwardedXP
            data.WriteBit(caged);                                                           // Caged
            data.WriteBit(team->SeenAction.find(battlePet) != team->SeenAction.end());      // SeenAction

            battlePetData << uint8(battlePet->GetGlobalIndex());
            battlePetData << uint32(battlePet->GetCurrentHealth());

            if (true)
                battlePetData << uint16(battlePet->GetXp());

            battlePetData << uint32(battlePet->GetMaxHealth());

            if (true)
                battlePetData << uint16(battlePet->GetOldLevel());

            if (true)
                battlePetData << uint16(battlePet->GetLevel());

            battlePetCount++;
        }
    }

    data.FlushBits();

    data.PutBits(battlePetCountPos, battlePetCount, 20);
    data.append(battlePetData);

    for (auto team : petBattle->Teams)
        data << uint32(0);                          // NpcCreatureID

    SendPacket(&data);
}

void WorldSession::SendPetBattleFinished()
{
    WorldPacket data(SMSG_PET_BATTLE_FINISHED, 0);
    SendPacket(&data);
}
