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
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectDefines.h"
#include "Vehicle.h"
#include "VehicleDefines.h"

void WorldSession::HandleAttackSwingOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<7, 6, 4, 3, 5, 0, 2, 1>(guid);
    recvData.ReadByteSeq<6, 3, 2, 5, 4, 7, 1, 0>(guid);

    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_ATTACKSWING Message guidlow:%u guidhigh:%u", GUID_LOPART(guid), GUID_HIPART(guid));

    Unit* pEnemy = ObjectAccessor::GetUnit(*_player, guid);

    if (!pEnemy)
    {
        // stop attack state at client
        SendAttackStop(NULL);
        return;
    }

    if (!_player->IsValidAttackTarget(pEnemy))
    {
        // stop attack state at client
        SendAttackStop(pEnemy);
        return;
    }

    //! Client explicitly checks the following before sending CMSG_ATTACKSWING packet,
    //! so we'll place the same check here. Note that it might be possible to reuse this snippet
    //! in other places as well.
    if (Vehicle* vehicle = _player->GetVehicle())
    {
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(_player);
        ASSERT(seat);
        if (!(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_ATTACK))
        {
            SendAttackStop(pEnemy);
            return;
        }
    }

    _player->Attack(pEnemy, true);
}

void WorldSession::HandleAttackStopOpcode(WorldPacket & /*recvData*/)
{
    GetPlayer()->AttackStop();
}

void WorldSession::HandleSetSheathedOpcode(WorldPacket& recvData)
{
    uint32 sheathed;

    recvData >> sheathed;
    recvData.ReadBit();

    //TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "WORLD: Recvd CMSG_SETSHEATHED Message guidlow:%u value1:%u", GetPlayer()->GetGUIDLow(), sheathed);

    if (sheathed >= MAX_SHEATH_STATE)
    {
        TC_LOG_ERROR("network", "Unknown sheath state %u ??", sheathed);
        return;
    }

    GetPlayer()->SetSheath(SheathState(sheathed));
}

void WorldSession::SendAttackStop(Unit const* enemy)
{
    WorldPacket data(SMSG_ATTACK_STOP);

    ObjectGuid victimGUID = enemy ? enemy->GetGUID() : 0;
    ObjectGuid attackerGUID = GetPlayer()->GetGUID();

    data.WriteBitSeq<0>(victimGUID);
    data.WriteBitSeq<4>(attackerGUID);
    data.WriteBitSeq<1>(victimGUID);
    data.WriteBitSeq<7>(attackerGUID);
    data.WriteBitSeq<6, 3>(victimGUID);

    data.WriteBit(0);                   // Unk bit - updating rotation ?

    data.WriteBitSeq<5>(victimGUID);
    data.WriteBitSeq<1, 0>(attackerGUID);
    data.WriteBitSeq<7>(victimGUID);
    data.WriteBitSeq<6>(attackerGUID);
    data.WriteBitSeq<4, 2>(victimGUID);
    data.WriteBitSeq<3, 2, 5>(attackerGUID);

    data.WriteByteSeq<2, 7>(attackerGUID);
    data.WriteByteSeq<0>(victimGUID);
    data.WriteByteSeq<5>(attackerGUID);
    data.WriteByteSeq<5>(victimGUID);
    data.WriteByteSeq<3>(attackerGUID);
    data.WriteByteSeq<7, 1, 3>(victimGUID);
    data.WriteByteSeq<0>(attackerGUID);
    data.WriteByteSeq<4, 6>(victimGUID);
    data.WriteByteSeq<1, 6>(attackerGUID);
    data.WriteByteSeq<2>(victimGUID);
    data.WriteByteSeq<4>(attackerGUID);
    SendPacket(&data);
}
