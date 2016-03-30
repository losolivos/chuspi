/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "Corpse.h"
#include "Player.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "MapManager.h"
#include "Transport.h"
#include "Battleground.h"
#include "WaypointMovementGenerator.h"
#include "InstanceSaveMgr.h"
#include "ObjectMgr.h"
#include "MovementStructures.h"
#include "MoveSplineInit.h"
#include "AnticheatMgr.h"

namespace {

MovementStatusElements const * GetMovementStatusElementsSequence(Opcodes opcode)
{
    switch (opcode)
    {
        case CMSG_CAST_SPELL:       // Cast spell has movement data part when castflags & 0x10, patched ClientSide to have same data of CMSG_PLAYER_MOVE
        case CMSG_PLAYER_MOVE:
            return MovementStartForwardSequence;
        case SMSG_MOVE_UPDATE:
            return PlayerMoveSequence;
        default:
            break;
    }

    return NULL;
}

} // namespace

void WorldSession::checkMoveCheat(uint16 opcode, MovementInfo const &newMovementInfo)
{
    // This packet may be spammed by the client under unknown circumstances
    if (opcode == MSG_MOVE_SET_FACING || !sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
        return;

    auto player = GetPlayer();

    // Completely ignore transition from land to transport or any movement on transport.
    if (player->GetTransport() || newMovementInfo.bits[MovementInfo::Bit::TransportData])
        return;

    auto const &oldMovementInfo = player->m_movementInfo;
    auto const distance = oldMovementInfo.pos.GetExactDist2d(&newMovementInfo.pos);

    if (distance > sWorld->getFloatConfig(CONFIG_CHEAT_MOVING_TELEPORT_DISTANCE_DETECT)
            && !player->IsBeingTeleported())
    {
        TC_LOG_DEBUG("entities.player.cheat", "%u (%s) - 0x%04X - 0x%X - teleport hack used",
                     GetAccountId(), player->GetName().c_str(), opcode,
                     newMovementInfo.GetMovementFlags());
        return KickPlayer();
    }

    auto const moveType = Movement::SelectSpeedType(newMovementInfo.GetMovementFlags());
    auto const maxSpeed = player->GetSpeed(moveType);

    auto const wasMoving = oldMovementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING_CHEAT);
    auto const isMoving = newMovementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING_CHEAT);

    if (!(wasMoving && isMoving && maxSpeed > 0.01f))
    {
        player->m_averageSpeed *= 0.5f;
        return;
    }

    if (player->IsBeingTeleported() || player->IsFalling())
        return;

    auto const oldMoveTime = oldMovementInfo.time;
    auto const newMoveTime = newMovementInfo.time;

    // time in seconds
    auto const timeDiff = (newMoveTime - oldMoveTime) * (1.0f / 1000.0f);
    if (timeDiff <= 0.0001f)
        return;

    auto const speed = distance / timeDiff;

    player->m_averageSpeed = (player->m_averageSpeed != 0.0f)
            ? ((player->m_averageSpeed + speed) * 0.5f)
            : speed;

    if (player->m_averageSpeed / maxSpeed < sWorld->getFloatConfig(CONFIG_CHEAT_MOVING_MAX_SPEED_MULTIPLIER))
    {
        player->m_numSpeedChecks = 0;
        return;
    }

    TC_LOG_DEBUG("entities.player.cheat", "%u (%s) - 0x%04X - 0x%X - move (%f) (%f, %f, %d)",
                 GetAccountId(), player->GetName().c_str(), opcode, newMovementInfo.GetMovementFlags(),
                 maxSpeed, timeDiff, player->m_averageSpeed, player->m_numSpeedChecks);

    if (++player->m_numSpeedChecks > sWorld->getIntConfig(CONFIG_CHEAT_MOVING_MAX_FAILED_SPEED_CHECKS))
    {
        TC_LOG_DEBUG("entities.player.cheat", "%u (%s) - 0x%04X - 0x%X - speed hack used",
                     GetAccountId(), player->GetName().c_str(), opcode,
                     newMovementInfo.GetMovementFlags());
        KickPlayer();
    }
}

void WorldSession::HandleMoveWorldportAckOpcode(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: got MSG_MOVE_WORLDPORT_ACK.");
    HandleMoveWorldportAckOpcode();
}

void WorldSession::HandleMoveWorldportAckOpcode()
{
    // ignore unexpected far teleports
    if (!GetPlayer()->IsBeingTeleportedFar())
        return;

    GetPlayer()->SetSemaphoreTeleportFar(false);
    GetPlayer()->SetIgnoreMovementCount(5);

    // get the teleport destination
    WorldLocation const loc = GetPlayer()->GetTeleportDest();

    // possible errors in the coordinate validity check
    if (!MapManager::IsValidMapCoord(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetOrientation()))
    {
        LogoutPlayer(false);
        return;
    }

    // get the destination map entry, not the current one, this will fix homebind and reset greeting
    MapEntry const* mEntry = sMapStore.LookupEntry(loc.GetMapId());
    InstanceTemplate const* mInstance = sObjectMgr->GetInstanceTemplate(loc.GetMapId());

    // reset instance validity, except if going to an instance inside an instance
    if (GetPlayer()->m_InstanceValid == false && !mInstance)
        GetPlayer()->m_InstanceValid = true;

    Map* oldMap = GetPlayer()->GetMap();
    if (GetPlayer()->IsInWorld())
    {
        TC_LOG_ERROR("network", "Player (Name %s) is still in world when teleported from map %u to new map %u",
                     GetPlayer()->GetName().c_str(), oldMap->GetId(), loc.GetMapId());
        oldMap->RemovePlayerFromMap(GetPlayer(), false);
    }

    // relocate the player to the teleport destination
    Map* newMap = sMapMgr->CreateMap(loc.GetMapId(), GetPlayer());
    // the CanEnter checks are done in TeleporTo but conditions may change
    // while the player is in transit, for example the map may get full
    if (!newMap || !newMap->CanEnter(GetPlayer()))
    {
        TC_LOG_ERROR("network", "Map %d could not be created for player %d, porting player to homebind", loc.GetMapId(), GetPlayer()->GetGUIDLow());
        GetPlayer()->TeleportTo(GetPlayer()->m_homebindMapId, GetPlayer()->m_homebindX, GetPlayer()->m_homebindY, GetPlayer()->m_homebindZ, GetPlayer()->GetOrientation());
        return;
    }

    GetPlayer()->Relocate(&loc);
    GetPlayer()->m_movementInfo.pos.Relocate(GetPlayer());

    GetPlayer()->ResetMap();
    GetPlayer()->SetMap(newMap);

    GetPlayer()->SendInitialPacketsBeforeAddToMap();
    if (!GetPlayer()->GetMap()->AddPlayerToMap(GetPlayer()))
    {
        TC_LOG_ERROR("network", "WORLD: failed to teleport player %s (%d) to map %d (%s) because of unknown reason!",
            GetPlayer()->GetName().c_str(), GetPlayer()->GetGUIDLow(), loc.GetMapId(), newMap ? newMap->GetMapName() : "Unknown");
        GetPlayer()->ResetMap();
        GetPlayer()->SetMap(oldMap);
        GetPlayer()->TeleportTo(GetPlayer()->m_homebindMapId, GetPlayer()->m_homebindX, GetPlayer()->m_homebindY, GetPlayer()->m_homebindZ, GetPlayer()->GetOrientation());
        return;
    }

    // battleground state prepare (in case join to BG), at relogin/tele player not invited
    // only add to bg group and object, if the player was invited (else he entered through command)
    if (_player->InBattleground())
    {
        // cleanup setting if outdated
        if (!mEntry->IsBattlegroundOrArena())
        {
            // We're not in BG
            _player->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE);
            // reset destination bg team
            _player->SetBGTeam(0);
        }
        // join to bg case
        else if (Battleground* bg = _player->GetBattleground())
        {
            if (_player->IsInvitedForBattlegroundInstance(_player->GetBattlegroundId()))
                bg->AddPlayer(_player);
        }
    }

    GetPlayer()->SendInitialPacketsAfterAddToMap();

    // Update position client-side to avoid undermap
    WorldPacket data(SMSG_MOVE_UPDATE);
    _player->m_movementInfo.time = getMSTime();
    _player->m_movementInfo.pos.m_positionX = loc.m_positionX;
    _player->m_movementInfo.pos.m_positionY = loc.m_positionY;
    _player->m_movementInfo.pos.m_positionZ = loc.m_positionZ;
    WorldSession::WriteMovementInfo(data, &_player->m_movementInfo);
    _player->GetSession()->SendPacket(&data);

    // flight fast teleport case
    if (GetPlayer()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
    {
        if (!_player->InBattleground())
        {
            // short preparations to continue flight
            FlightPathMovementGenerator* flight = (FlightPathMovementGenerator*)(GetPlayer()->GetMotionMaster()->top());
            flight->Initialize(GetPlayer());
            return;
        }

        // battleground state prepare, stop flight
        GetPlayer()->GetMotionMaster()->MovementExpired();
        GetPlayer()->CleanupAfterTaxiFlight();
    }

    // resurrect character at enter into instance where his corpse exist after add to map
    Corpse* corpse = GetPlayer()->GetCorpse();
    if (corpse && corpse->GetType() != CORPSE_BONES && corpse->GetMapId() == GetPlayer()->GetMapId())
    {
        if (mEntry->IsDungeon())
        {
            GetPlayer()->ResurrectPlayer(0.5f, false);
            GetPlayer()->SpawnCorpseBones();
        }
    }

    bool allowMount = !mEntry->IsDungeon() || mEntry->IsBattlegroundOrArena();
    if (mInstance)
    {
        Difficulty diff = GetPlayer()->GetDifficulty(mEntry->IsRaid());
        if (MapDifficulty const* mapDiff = GetMapDifficultyData(mEntry->MapID, diff))
        {
            if (mapDiff->resetTime)
            {
                if (time_t timeReset = sInstanceSaveMgr->GetResetTimeFor(mEntry->MapID, diff))
                {
                    uint32 timeleft = uint32(timeReset - time(NULL));
                    GetPlayer()->SendInstanceResetWarning(mEntry->MapID, diff, timeleft);
                }
            }
        }
        allowMount = mInstance->AllowMount;
    }

    // mount allow check
    if (!allowMount)
        _player->RemoveAurasByType(SPELL_AURA_MOUNTED);

    // update zone immediately, otherwise leave channel will cause crash in mtmap
    uint32 newzone, newarea;
    GetPlayer()->GetZoneAndAreaId(newzone, newarea);
    GetPlayer()->UpdateZone(newzone, newarea);

    for (uint8 i = 0; i < 9; ++i)
        GetPlayer()->UpdateSpeed(UnitMoveType(i), true);

    // honorless target
    if (GetPlayer()->pvpInfo.inHostileArea)
        GetPlayer()->CastSpell(GetPlayer(), 2479, true);

    // in friendly area
    else if (GetPlayer()->IsPvP() && !GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
        GetPlayer()->UpdatePvP(false, false);

    // resummon pet
    GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();

    // resummon temporarily unsummoned battle pet
    GetPlayer()->GetBattlePetMgr().ResummonLastBattlePet();

    //lets process all delayed operations on successful teleport
    GetPlayer()->ProcessDelayedOperations();

    Unit::VisibleAuraMap const* visibleAuras = GetPlayer()->GetVisibleAuras();
    for (Unit::VisibleAuraMap::const_iterator itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
    {
        for (uint8 i = 0; i < itr->second->GetBase()->GetSpellInfo()->Effects.size(); ++i)
        {
            if (AuraEffect* eff = itr->second->GetBase()->GetEffect(i))
            {
                eff->ApplySpellMod(GetPlayer(), false);
                eff->ApplySpellMod(GetPlayer(), true);
            }
        }
    }
}

void WorldSession::HandleMoveTeleportAck(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "MSG_MOVE_TELEPORT_ACK");

    ObjectGuid guid;
    uint32 flags, time;
    recvPacket >> flags >> time;

    recvPacket.ReadBitSeq<3, 4, 7, 2, 5, 1, 6, 0>(guid);
    recvPacket.ReadByteSeq<0, 6, 5, 3, 7, 1, 2, 4>(guid);

    TC_LOG_DEBUG("network", "Guid " UI64FMTD, uint64(guid));
    TC_LOG_DEBUG("network", "Flags %u, time %u", flags, time/IN_MILLISECONDS);

    Player* plMover = _player->m_mover->ToPlayer();

    if (!plMover || !plMover->IsBeingTeleportedNear())
        return;

    if (guid != plMover->GetGUID())
        return;

    plMover->SetSemaphoreTeleportNear(false);
    plMover->SetIgnoreMovementCount(5);

    uint32 old_zone = plMover->GetZoneId();

    WorldLocation const& dest = plMover->GetTeleportDest();

    plMover->UpdatePosition(dest, true);

    uint32 newzone, newarea;
    plMover->GetZoneAndAreaId(newzone, newarea);
    plMover->UpdateZone(newzone, newarea);

    // new zone
    if (old_zone != newzone)
    {
        // honorless target
        if (plMover->pvpInfo.inHostileArea)
            plMover->CastSpell(plMover, 2479, true);

        // in friendly area
        else if (plMover->IsPvP() && !plMover->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
            plMover->UpdatePvP(false, false);
    }

    // resummon pet
    plMover->ResummonPetTemporaryUnSummonedIfAny();

    // resummon temporarily unsummoned battle pet
    GetPlayer()->GetBattlePetMgr().ResummonLastBattlePet();

    //lets process all delayed operations on successful teleport
    plMover->ProcessDelayedOperations();
}

void RemoveDanceEmote(Player* player)
{
    if (uint32 emote = player->GetUInt32Value(UNIT_NPC_EMOTESTATE))
        player->SetUInt32Value(UNIT_NPC_EMOTESTATE, emote - EMOTE_STATE_DANCE);
}

void WorldSession::HandleMovementOpcodes(WorldPacket& recvPacket)
{
    uint16 opcode = recvPacket.GetOpcode();

    Unit* mover = _player->m_mover;

    ASSERT(mover != NULL);                      // there must always be a mover

    Player* plrMover = mover->ToPlayer();

    // ignore, waiting processing in WorldSession::HandleMoveWorldportAckOpcode and WorldSession::HandleMoveTeleportAck
    if (plrMover && plrMover->IsBeingTeleported())
    {
        recvPacket.rfinish();                     // prevent warnings spam
        return;
    }

    // Sometime, client send movement packet after teleporation with position before teleportation, so we ignore 3 first movement packet after teleporation
    // TODO: find a better way to check that, may be a new CMSG send by client ?
    if (plrMover && plrMover->GetIgnoreMovementCount() && opcode != CMSG_CAST_SPELL)
    {
        plrMover->SetIgnoreMovementCount(plrMover->GetIgnoreMovementCount() - 1);
        recvPacket.rfinish();                     // prevent warnings spam
        return;
    }

    /* extract packet */
    MovementInfo movementInfo;
    ReadMovementInfo(recvPacket, &movementInfo);

    if (plrMover && plrMover->HasUnitState(UNIT_STATE_FALLING) && !movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING))
        plrMover->ClearUnitState(UNIT_STATE_FALLING);

    // prevent tampered movement data
    if (movementInfo.guid != mover->GetGUID())
    {
        TC_LOG_DEBUG("network", "HandleMovementOpcodes: guid error");
        return;
    }
    if (!movementInfo.pos.IsPositionValid())
    {
        TC_LOG_DEBUG("network", "HandleMovementOpcodes: Invalid Position");
        return;
    }

    movementInfo.alive32 = movementInfo.time; // hack, but it's work in 505 in this way ...
    movementInfo.time = getMSTime();

    if (GetSecurity() < SEC_GAMEMASTER && GetPlayer() == mover)
        checkMoveCheat(opcode, movementInfo);

    /* handle special cases */
    if (movementInfo.t_guid)
    {
        // transports size limited
        // (also received at zeppelin leave by some reason with t_* as absolute in continent coordinates, can be safely skipped)
        if (movementInfo.t_pos.GetPositionX() > 50 || movementInfo.t_pos.GetPositionY() > 50 || movementInfo.t_pos.GetPositionZ() > 50)
        {
            recvPacket.rfinish();                 // prevent warnings spam
            return;
        }

        if (!Trinity::IsValidMapCoord(movementInfo.pos.GetPositionX() + movementInfo.t_pos.GetPositionX(), movementInfo.pos.GetPositionY() + movementInfo.t_pos.GetPositionY(),
            movementInfo.pos.GetPositionZ() + movementInfo.t_pos.GetPositionZ(), movementInfo.pos.GetOrientation() + movementInfo.t_pos.GetOrientation()))
        {
            recvPacket.rfinish();                 // prevent warnings spam
            return;
        }

        // if we boarded a transport, add us to it
        if (plrMover)
        {
            if (!plrMover->GetTransport())
            {
                // elevators also cause the client to send MOVEMENTFLAG_ONTRANSPORT - just dismount if the guid can be found in the transport list
                for (MapManager::TransportSet::const_iterator iter = sMapMgr->m_Transports.begin(); iter != sMapMgr->m_Transports.end(); ++iter)
                {
                    if ((*iter)->GetGUID() == movementInfo.t_guid)
                    {
                        plrMover->m_transport = *iter;
                        (*iter)->AddPassenger(plrMover);
                        break;
                    }
                }
            }
            else if (plrMover->GetTransport()->GetGUID() != movementInfo.t_guid)
            {
                bool foundNewTransport = false;
                plrMover->m_transport->RemovePassenger(plrMover);
                for (MapManager::TransportSet::const_iterator iter = sMapMgr->m_Transports.begin(); iter != sMapMgr->m_Transports.end(); ++iter)
                {
                    if ((*iter)->GetGUID() == movementInfo.t_guid)
                    {
                        foundNewTransport = true;
                        plrMover->m_transport = *iter;
                        (*iter)->AddPassenger(plrMover);
                        break;
                    }
                }

                if (!foundNewTransport)
                {
                    plrMover->m_transport = NULL;
                    movementInfo.t_pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
                    movementInfo.t_time = 0;
                    movementInfo.t_seat = -1;
                }
            }
        }

        if (!mover->GetTransport() && !mover->GetVehicle())
        {
            GameObject* go = mover->GetMap()->GetGameObject(movementInfo.t_guid);
            if (!go || go->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT)
                movementInfo.t_guid = 0;
        }
    }
    else if (plrMover && plrMover->GetTransport())                // if we were on a transport, leave
    {
        plrMover->m_transport->RemovePassenger(plrMover);
        plrMover->m_transport = NULL;
        movementInfo.t_pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        movementInfo.t_time = 0;
        movementInfo.t_seat = -1;
    }

    // fall damage generation (ignore in flight case that can be triggered also at lags in moment teleportation to another map).
    if (plrMover && plrMover->m_movementInfo.GetMovementFlags() & MOVEMENTFLAG_FALLING && (movementInfo.GetMovementFlags() & MOVEMENTFLAG_FALLING) == 0 && (movementInfo.GetMovementFlags() & MOVEMENTFLAG_SWIMMING) == 0 && !plrMover->isInFlight())
    {
        plrMover->HandleFall(movementInfo);
    }

    if (plrMover && ((movementInfo.flags & MOVEMENTFLAG_SWIMMING) != 0) != plrMover->IsInWater())
    {
        // now client not include swimming flag in case jumping under water
        plrMover->SetInWater(!plrMover->IsInWater() || plrMover->GetBaseMap()->IsUnderWater(movementInfo.pos.GetPositionX(), movementInfo.pos.GetPositionY(), movementInfo.pos.GetPositionZ()));
    }

    if (plrMover)
        sAnticheatMgr->StartHackDetection(plrMover, movementInfo, opcode);
	
    /* process position-change */
    WorldPacket data(SMSG_MOVE_UPDATE, recvPacket.size());
    movementInfo.guid = mover->GetGUID();
    WorldSession::WriteMovementInfo(data, &movementInfo);
    mover->SendMessageToSet(&data, _player);

    mover->m_movementInfo = movementInfo;

    // this is almost never true (not sure why it is sometimes, but it is), normally use mover->IsVehicle()
    if (mover->GetVehicle())
    {
        mover->SetOrientation(movementInfo.pos.GetOrientation());
        return;
    }

    mover->UpdatePosition(movementInfo.pos);

    if (plrMover)                                            // nothing is charmed, or player charmed
    {
        // trigger PROC_FLAG_JUMPING (25)
        if (movementInfo.flags & MOVEMENTFLAG_FALLING 
            && movementInfo.flags2 & MOVEMENTFLAG2_INTERPOLATED_PITCHING
            && movementInfo.j_zspeed < 0.0f)
            plrMover->ProcDamageAndSpell(NULL, PROC_FLAG_JUMPING, PROC_FLAG_NONE, PROC_EX_NONE, 0, 0, BASE_ATTACK, NULL);

        plrMover->UpdateFallInformationIfNeed(movementInfo, opcode);

        float underMapValueZ, upperLimitValueZ;
        bool check = false;
        switch (plrMover->GetMapId())
        {
            case 617: // Dalaran Arena
                underMapValueZ = 3.0f;
                upperLimitValueZ = 30.0f;
                break;
            case 562: // Blades Edge Arena
                underMapValueZ = -1.0f;
                upperLimitValueZ = 22.0f;
                break;
            case 559: // Nagrand Arena
                underMapValueZ = -1.0f;
                upperLimitValueZ = 21.0f;
                break;
            case 572: // Ruins of Lordaeron
                underMapValueZ = -1.0f;
                upperLimitValueZ = 45.0f;
                break;
            case 618: // Ring of Valor
                underMapValueZ = 28.0f;
                upperLimitValueZ = 60.0f;
                break;
            case 566: // Eye of the storm
                underMapValueZ = 1000.0f;
                upperLimitValueZ = MAX_HEIGHT;
                break;
            case 726: // Twin Peaks
                underMapValueZ = -180.0f;
                upperLimitValueZ = MAX_HEIGHT;
                break;
            default:
                AreaTableEntry const* zone = GetAreaEntryByAreaID(plrMover->GetAreaId());
                underMapValueZ = zone ? zone->MaxDepth : -500.0f;
                upperLimitValueZ = MAX_HEIGHT;
                break;
        }

        check = movementInfo.pos.GetPositionZ() < underMapValueZ || movementInfo.pos.GetPositionZ() > upperLimitValueZ;
        if (check && !(plrMover->GetBattleground() && plrMover->GetBattleground()->HandlePlayerUnderMap(_player)))
        {
            // NOTE: this is actually called many times while falling
            // even after the player has been teleported away
            // TODO: discard movement packets after the player is rooted
            if (plrMover->IsAlive())
            {
                plrMover->EnvironmentalDamage(DAMAGE_FALL_TO_VOID, GetPlayer()->GetMaxHealth());
                // player can be alive if GM/etc
                // change the death state to CORPSE to prevent the death timer from
                // starting in the next player update
                if (!plrMover->IsAlive())
                    plrMover->KillPlayer();
            }
        }
    }
}

void WorldSession::HandleForceSpeedChangeAck(WorldPacket &recvData)
{
    /* extract packet */
    uint64 guid;
    uint32 unk1;
    float  newspeed;

    recvData.readPackGUID(guid);

    // now can skip not our packet
    if (_player->GetGUID() != guid)
    {
        recvData.rfinish();                   // prevent warnings spam
        return;
    }

    // continue parse packet

    recvData >> unk1;                                      // counter or moveEvent

    MovementInfo movementInfo;
    movementInfo.guid = guid;
    ReadMovementInfo(recvData, &movementInfo);

    recvData >> newspeed;
    /*----------------*/

    // client ACK send one packet for mounted/run case and need skip all except last from its
    // in other cases anti-cheat check can be fail in false case
    UnitMoveType move_type       = MOVE_WALK;
    UnitMoveType force_move_type = MOVE_WALK;

    static char const* move_type_name[MAX_MOVE_TYPE] = {  "Walk", "Run", "RunBack", "Swim", "SwimBack", "TurnRate", "Flight", "FlightBack", "PitchRate" };

    // skip all forced speed changes except last and unexpected
    // in run/mounted case used one ACK and it must be skipped.m_forced_speed_changes[MOVE_RUN} store both.
    if (_player->m_forced_speed_changes[force_move_type] > 0)
    {
        --_player->m_forced_speed_changes[force_move_type];
        if (_player->m_forced_speed_changes[force_move_type] > 0)
            return;
    }

    if (!_player->GetTransport() && fabs(_player->GetSpeed(move_type) - newspeed) > 0.01f)
    {
        if (_player->GetSpeed(move_type) > newspeed)         // must be greater - just correct
        {
            TC_LOG_ERROR("network", "%sSpeedChange player %s is NOT correct (must be %f instead %f), force set to correct value",
                move_type_name[move_type], _player->GetName().c_str(), _player->GetSpeed(move_type), newspeed);
            _player->SetSpeed(move_type, _player->GetSpeedRate(move_type), true);
        }
    }
}

void WorldSession::HandleSetActiveMoverOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_SET_ACTIVE_MOVER");

    ObjectGuid guid;

    recvPacket.ReadBit(); //unk

    recvPacket.ReadBitSeq<4, 7, 6, 0, 5, 3, 1, 2>(guid);
    recvPacket.ReadByteSeq<7, 0, 3, 6, 5, 2, 4, 1>(guid);

    if (GetPlayer()->IsInWorld())
    {
        if (_player->m_mover->GetGUID() != guid)
            TC_LOG_ERROR("network", "HandleSetActiveMoverOpcode: incorrect mover guid: mover is " UI64FMTD " (%s - Entry: %u) and should be " UI64FMTD, uint64(guid), GetLogNameForGuid(guid), GUID_ENPART(guid), _player->m_mover->GetGUID());
    }
}

void WorldSession::HandleMoveNotActiveMover(WorldPacket &recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_MOVE_NOT_ACTIVE_MOVER");

    uint64 old_mover_guid;
    recvData.readPackGUID(old_mover_guid);

    MovementInfo mi;
    ReadMovementInfo(recvData, &mi);

    mi.guid = old_mover_guid;

    _player->m_movementInfo = mi;
}

void WorldSession::HandleMountSpecialAnimOpcode(WorldPacket& /*recvData*/)
{
    WorldPacket data(SMSG_MOUNT_SPECIAL_ANIM, 8);
    data << uint64(GetPlayer()->GetGUID());

    GetPlayer()->SendMessageToSet(&data, false);
}

void WorldSession::HandleMoveKnockBackAck(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "CMSG_MOVE_KNOCK_BACK_ACK");

    uint64 guid;
    recvData.readPackGUID(guid);

    if (_player->m_mover->GetGUID() != guid)
        return;

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);

    _player->m_movementInfo = movementInfo;

    WorldPacket data(SMSG_MOVE_UPDATE_KNOCK_BACK, 66);
    data.appendPackGUID(guid);
    _player->BuildMovementPacket(&data);

    // knockback specific info
    data << movementInfo.j_sinAngle;
    data << movementInfo.j_cosAngle;
    data << movementInfo.j_xyspeed;
    data << movementInfo.j_zspeed;

    _player->SendMessageToSet(&data, false);
}

void WorldSession::HandleMoveHoverAck(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_MOVE_HOVER_ACK");

    uint64 guid;                                            // guid - unused
    recvData.readPackGUID(guid);

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);

    recvData.read_skip<uint32>();                          // unk2
}

void WorldSession::HandleSummonResponseOpcode(WorldPacket& recvData)
{
    if (!_player->IsAlive() || _player->IsInCombat())
        return;

    ObjectGuid summonerGuid;
    bool agree;

    recvData.ReadBitSeq<7, 3, 6>(summonerGuid);

    agree = recvData.ReadBit();

    recvData.ReadBitSeq<4, 5, 1, 0, 2>(summonerGuid);
    recvData.ReadByteSeq<4, 2, 6, 1, 7, 3, 0, 5>(summonerGuid);

    _player->SummonIfPossible(agree);
}

void WorldSession::ReadMovementInfo(WorldPacket& data, MovementInfo* mi)
{
    MovementInfo::BitSet::reference hasMovementFlags = mi->bits[MovementInfo::Bit::MovementFlags];
    MovementInfo::BitSet::reference hasMovementFlags2 = mi->bits[MovementInfo::Bit::MovementFlags2];
    MovementInfo::BitSet::reference hasTimestamp = mi->bits[MovementInfo::Bit::Timestamp];
    MovementInfo::BitSet::reference hasOrientation = mi->bits[MovementInfo::Bit::Orientation];
    MovementInfo::BitSet::reference hasTransportData = mi->bits[MovementInfo::Bit::TransportData];
    MovementInfo::BitSet::reference hasTransportTime2 = mi->bits[MovementInfo::Bit::TransportTime2];
    MovementInfo::BitSet::reference hasTransportTime3 = mi->bits[MovementInfo::Bit::TransportTime3];
    MovementInfo::BitSet::reference hasAlive32 = mi->bits[MovementInfo::Bit::Alive32];
    MovementInfo::BitSet::reference hasPitch = mi->bits[MovementInfo::Bit::Pitch];
    MovementInfo::BitSet::reference hasFallData = mi->bits[MovementInfo::Bit::FallData];
    MovementInfo::BitSet::reference hasFallDirection = mi->bits[MovementInfo::Bit::FallDirection];
    MovementInfo::BitSet::reference hasSplineElevation = mi->bits[MovementInfo::Bit::SplineElevation];

    uint32 bitcounterLoop = 0;

    MovementStatusElements const * const sequence = GetMovementStatusElementsSequence(data.GetOpcode());
    if (sequence == NULL)
    {
        TC_LOG_ERROR("network", "WorldSession::ReadMovementInfo: No movement sequence found for opcode 0x%04X", uint32(data.GetOpcode()));
        return;
    }

    ObjectGuid guid;
    ObjectGuid tguid;

    for (uint32 i = 0; i < MSE_COUNT; ++i)
    {
        MovementStatusElements element = sequence[i];
        if (element == MSEEnd)
            break;

        if (element >= MSEHasGuidByte0 && element <= MSEHasGuidByte7)
        {
            guid[element - MSEHasGuidByte0] = data.ReadBit();
            continue;
        }

        if (element >= MSEHasTransportGuidByte0 &&
            element <= MSEHasTransportGuidByte7)
        {
            if (hasTransportData)
                tguid[element - MSEHasTransportGuidByte0] = data.ReadBit();
            continue;
        }

        if (element >= MSEGuidByte0 && element <= MSEGuidByte7)
        {
            data.ReadByte(guid[element - MSEGuidByte0]);
            continue;
        }

        if (element >= MSETransportGuidByte0 &&
            element <= MSETransportGuidByte7)
        {
            if (hasTransportData)
                data.ReadByte(tguid[element - MSETransportGuidByte0]);
            continue;
        }

        switch (element)
        {
            case MSEBitCounter1:
                bitcounterLoop = data.ReadBits(22);
                break;
            case MSEBitCounterLoop1:
                for (uint32 i = 0; i < bitcounterLoop; ++i)
                    data.read_skip<uint32>();
                break;
            case MSEFlushBits:
                break;
            case MSEHasMovementFlags:
                hasMovementFlags = !data.ReadBit();
                break;
            case MSEHasMovementFlags2:
                hasMovementFlags2 = !data.ReadBit();
                break;
            case MSEHasTimestamp:
                hasTimestamp = !data.ReadBit();
                break;
            case MSEHasOrientation:
                hasOrientation = !data.ReadBit();
                break;
            case MSEHasTransportData:
                hasTransportData = data.ReadBit();
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    hasTransportTime2 = data.ReadBit();
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    hasTransportTime3 = data.ReadBit();
                break;
            case MSEHasPitch:
                hasPitch = !data.ReadBit();
                break;
            case MSEHasFallData:
                hasFallData = data.ReadBit();
                break;
            case MSEHasFallDirection:
                if (hasFallData)
                    hasFallDirection = data.ReadBit();
                break;
            case MSEHasSplineElevation:
                hasSplineElevation = !data.ReadBit();
                break;
            case MSEHasSpline:
                data.ReadBit();
                break;
            case MSEMovementFlags:
                if (hasMovementFlags)
                    mi->flags = data.ReadBits(30);
                break;
            case MSEMovementFlags2:
                if (hasMovementFlags2)
                    mi->flags2 = data.ReadBits(13);
                break;
            case MSETimestamp:
                if (hasTimestamp)
                    data >> mi->time;
                break;
            case MSEPositionX:
                data >> mi->pos.m_positionX;
                break;
            case MSEPositionY:
                data >> mi->pos.m_positionY;
                break;
            case MSEPositionZ:
                data >> mi->pos.m_positionZ;
                break;
            case MSEOrientation:
                if (hasOrientation)
                    mi->pos.SetOrientation(data.read<float>());
                break;
            case MSETransportPositionX:
                if (hasTransportData)
                    data >> mi->t_pos.m_positionX;
                break;
            case MSETransportPositionY:
                if (hasTransportData)
                    data >> mi->t_pos.m_positionY;
                break;
            case MSETransportPositionZ:
                if (hasTransportData)
                    data >> mi->t_pos.m_positionZ;
                break;
            case MSETransportOrientation:
                if (hasTransportData)
                    mi->t_pos.SetOrientation(data.read<float>());
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data >> mi->t_seat;
                break;
            case MSETransportTime:
                if (hasTransportData)
                    data >> mi->t_time;
                break;
            case MSETransportTime2:
                if (hasTransportData && hasTransportTime2)
                    data >> mi->t_time2;
                break;
            case MSETransportTime3:
                if (hasTransportData && hasTransportTime3)
                    data >> mi->t_time3;
                break;
            case MSEPitch:
                if (hasPitch)
                    data >> mi->pitch;
                break;
            case MSEFallTime:
                if (hasFallData)
                    data >> mi->fallTime;
                break;
            case MSEFallVerticalSpeed:
                if (hasFallData)
                    data >> mi->j_zspeed;
                break;
            case MSEFallCosAngle:
                if (hasFallData && hasFallDirection)
                    data >> mi->j_cosAngle;
                break;
            case MSEFallSinAngle:
                if (hasFallData && hasFallDirection)
                    data >> mi->j_sinAngle;
                break;
            case MSEFallHorizontalSpeed:
                if (hasFallData && hasFallDirection)
                    data >> mi->j_xyspeed;
                break;
            case MSESplineElevation:
                if (hasSplineElevation)
                    data >> mi->splineElevation;
                break;
            case MSEZeroBit:
            case MSEOneBit:
                data.ReadBit();
                break;
            case MSEHasAlive32:
                hasAlive32 = !data.ReadBit();
                break;
            case MSEAlive32:
                if (hasAlive32)
                    data >> mi->alive32;
                break;
            default:
                ASSERT(false && "Incorrect sequence element detected at ReadMovementInfo");
                break;
        }
    }

    if (hasMovementFlags)
        RemoveDanceEmote(GetPlayer());

    mi->guid = guid;
    mi->t_guid = tguid;

   if (hasTransportData && mi->pos.m_positionX != mi->t_pos.m_positionX)
       if (GetPlayer()->GetTransport())
           GetPlayer()->GetTransport()->UpdatePosition(mi);

    //! Anti-cheat checks. Please keep them in seperate if() blocks to maintain a clear overview.
    //! Might be subject to latency, so just remove improper flags.
    #ifdef TRINITY_DEBUG
    #define REMOVE_VIOLATING_FLAGS(check, maskToRemove) \
    { \
        if (check) \
        { \
            TC_LOG_DEBUG("entities.unit", "WorldSession::ReadMovementInfo: Violation of MovementFlags found (%s). " \
                "MovementFlags: %u, MovementFlags2: %u for player GUID: %u. Mask %u will be removed.", \
                STRINGIZE(check), mi->GetMovementFlags(), mi->GetExtraMovementFlags(), GetPlayer()->GetGUIDLow(), maskToRemove); \
            mi->RemoveMovementFlag((maskToRemove)); \
        } \
    }
    #else
    #define REMOVE_VIOLATING_FLAGS(check, maskToRemove) \
        if (check) \
            mi->RemoveMovementFlag((maskToRemove));
    #endif


    /*! This must be a packet spoofing attempt. MOVEMENTFLAG_ROOT sent from the client is not valid
        in conjunction with any of the moving movement flags such as MOVEMENTFLAG_FORWARD.
        It will freeze clients that receive this player's movement info.*/

    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_ROOT),
        MOVEMENTFLAG_ROOT);

    //! Cannot hover without SPELL_AURA_HOVER
    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_HOVER) && !GetPlayer()->HasAuraType(SPELL_AURA_HOVER),
        MOVEMENTFLAG_HOVER);

    //! Cannot ascend and descend at the same time
    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_ASCENDING) && mi->HasMovementFlag(MOVEMENTFLAG_DESCENDING),
        MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING);

    //! Cannot move left and right at the same time
    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_LEFT) && mi->HasMovementFlag(MOVEMENTFLAG_RIGHT),
        MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT);

    //! Cannot strafe left and right at the same time
    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_STRAFE_LEFT) && mi->HasMovementFlag(MOVEMENTFLAG_STRAFE_RIGHT),
        MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT);

    //! Cannot pitch up and down at the same time
    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_PITCH_UP) && mi->HasMovementFlag(MOVEMENTFLAG_PITCH_DOWN),
        MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN);

    //! Cannot move forwards and backwards at the same time
    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_FORWARD) && mi->HasMovementFlag(MOVEMENTFLAG_BACKWARD),
        MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD);

    //! Cannot walk on water without SPELL_AURA_WATER_WALK
    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_WATERWALKING) && !GetPlayer()->HasAuraType(SPELL_AURA_WATER_WALK),
        MOVEMENTFLAG_WATERWALKING);

    //! Cannot feather fall without SPELL_AURA_FEATHER_FALL
    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_FALLING_SLOW) && !GetPlayer()->HasAuraType(SPELL_AURA_FEATHER_FALL),
        MOVEMENTFLAG_FALLING_SLOW);

    /*! Cannot fly if no fly auras present. Exception is being a GM.
        Note that we check for account level instead of Player::isGameMaster() because in some
        situations it may be feasable to use .gm fly on as a GM without having .gm on,
        e.g. aerial combat.
    */

    REMOVE_VIOLATING_FLAGS(mi->HasMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY) && GetSecurity() == SEC_PLAYER &&
        !GetPlayer()->m_mover->HasAuraType(SPELL_AURA_FLY) &&
        !GetPlayer()->m_mover->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED),
        MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY);

    #undef REMOVE_VIOLATING_FLAGS
}

void WorldSession::WriteMovementInfo(WorldPacket &data, MovementInfo const *mi)
{
    MovementStatusElements const * const sequence = GetMovementStatusElementsSequence(data.GetOpcode());
    if (!sequence)
    {
        //TC_LOG_ERROR("network", "WorldSession::WriteMovementInfo: No movement sequence found for opcode 0x%04X", uint32(data.GetOpcode()));
        return;
    }

    bool hasMovementFlags = mi->GetMovementFlags() != 0;
    bool hasMovementFlags2 = mi->GetExtraMovementFlags() != 0;
    bool hasOrientation = !G3D::fuzzyEq(mi->pos.GetOrientation(), 0.0f);
    bool hasTransportData = mi->t_guid != 0;

    ObjectGuid guid = mi->guid;
    ObjectGuid tguid = mi->t_guid;

    const_cast<MovementInfo *>(mi)->Normalize();

    for (uint32 i = 0; i < MSE_COUNT; ++i)
    {
        MovementStatusElements element = sequence[i];
        if (element == MSEEnd)
            break;

        if (element >= MSEHasGuidByte0 && element <= MSEHasGuidByte7)
        {
            data.WriteBit(guid[element - MSEHasGuidByte0]);
            continue;
        }

        if (element >= MSEHasTransportGuidByte0 &&
            element <= MSEHasTransportGuidByte7)
        {
            if (hasTransportData)
                data.WriteBit(tguid[element - MSEHasTransportGuidByte0]);
            continue;
        }

        if (element >= MSEGuidByte0 && element <= MSEGuidByte7)
        {
            data.WriteByte(guid[element - MSEGuidByte0]);
            continue;
        }

        if (element >= MSETransportGuidByte0 &&
            element <= MSETransportGuidByte7)
        {
            if (hasTransportData)
                data.WriteByte(tguid[element - MSETransportGuidByte0]);
            continue;
        }

        switch (element)
        {
            case MSEBitCounter1:
                data.WriteBits(0, 22);
                break;
            case MSEFlushBits:
                data.FlushBits();
                break;
            case MSEHasMovementFlags:
                data.WriteBit(!hasMovementFlags);
                break;
            case MSEHasMovementFlags2:
                data.WriteBit(!hasMovementFlags2);
                break;
            case MSEHasTimestamp:
                data.WriteBit(!mi->time);
                break;
            case MSEHasOrientation:
                data.WriteBit(!hasOrientation);
                break;
            case MSEHasTransportData:
                data.WriteBit(hasTransportData);
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    data.WriteBit(mi->t_time2);
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    data.WriteBit(mi->t_time3);
                break;
            case MSEHasPitch:
                data.WriteBit(!mi->bits[MovementInfo::Bit::Pitch]);
                break;
            case MSEHasFallData:
                data.WriteBit(mi->bits[MovementInfo::Bit::FallData]);
                break;
            case MSEHasFallDirection:
                if (mi->bits[MovementInfo::Bit::FallData])
                    data.WriteBit(mi->bits[MovementInfo::Bit::FallDirection]);
                break;
            case MSEHasSplineElevation:
                data.WriteBit(!mi->bits[MovementInfo::Bit::SplineElevation]);
                break;
            case MSEHasSpline:
                data.WriteBit(false);
                break;
            case MSEMovementFlags:
                if (hasMovementFlags)
                    data.WriteBits(mi->flags, 30);
                break;
            case MSEMovementFlags2:
                if (hasMovementFlags2)
                    data.WriteBits(mi->flags2, 13);
                break;
            case MSETimestamp:
                if (mi->time)
                    data << mi->time;
                break;
            case MSEPositionX:
                data << mi->pos.m_positionX;
                break;
            case MSEPositionY:
                data << mi->pos.m_positionY;
                break;
            case MSEPositionZ:
                data << mi->pos.m_positionZ;
                break;
            case MSEOrientation:
                if (hasOrientation)
                    data << mi->pos.GetOrientation();
                break;
            case MSETransportPositionX:
                if (hasTransportData)
                    data << mi->t_pos.m_positionX;
                break;
            case MSETransportPositionY:
                if (hasTransportData)
                    data << mi->t_pos.m_positionY;
                break;
            case MSETransportPositionZ:
                if (hasTransportData)
                    data << mi->t_pos.m_positionZ;
                break;
            case MSETransportOrientation:
                if (hasTransportData)
                    data << mi->t_pos.GetOrientation();
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data << mi->t_seat;
                break;
            case MSETransportTime:
                if (hasTransportData)
                    data << mi->t_time;
                break;
            case MSETransportTime2:
                if (hasTransportData && mi->t_time2)
                    data << mi->t_time2;
                break;
            case MSETransportTime3:
                if (hasTransportData && mi->t_time3)
                    data << mi->t_time3;
                break;
            case MSEPitch:
                if (mi->bits[MovementInfo::Bit::Pitch])
                    data << mi->pitch;
                break;
            case MSEFallTime:
                if (mi->bits[MovementInfo::Bit::FallData])
                    data << mi->fallTime;
                break;
            case MSEFallVerticalSpeed:
                if (mi->bits[MovementInfo::Bit::FallData])
                    data << mi->j_zspeed;
                break;
            case MSEFallCosAngle:
                if (mi->bits[MovementInfo::Bit::FallData] && mi->bits[MovementInfo::Bit::FallDirection])
                    data << mi->j_cosAngle;
                break;
            case MSEFallSinAngle:
                if (mi->bits[MovementInfo::Bit::FallData] && mi->bits[MovementInfo::Bit::FallDirection])
                    data << mi->j_sinAngle;
                break;
            case MSEFallHorizontalSpeed:
                if (mi->bits[MovementInfo::Bit::FallData] && mi->bits[MovementInfo::Bit::FallDirection])
                    data << mi->j_xyspeed;
                break;
            case MSESplineElevation:
                if (mi->bits[MovementInfo::Bit::SplineElevation])
                    data << mi->splineElevation;
                break;
            case MSEZeroBit:
                data.WriteBit(0);
                break;
            case MSEOneBit:
                data.WriteBit(1);
                break;
            case MSEHasAlive32:
                data.WriteBit(!mi->alive32);
                break;
            case MSEAlive32:
                if (mi->alive32)
                    data << mi->alive32;
                break;
            default:
                ASSERT(false && "Incorrect sequence element detected at ReadMovementInfo");
                break;
        }
    }
}
