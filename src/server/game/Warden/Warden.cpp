/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#include "Warden.h"
#include "AccountMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "Opcodes.h"
#include "ByteBuffer.h"
#include "World.h"
#include "Player.h"
#include "Util.h"
#include "Language.h"

#include <openssl/md5.h>
#include <openssl/sha.h>

#include <cstring>

Warden::Warden()
    : _session(NULL)
    , _inputCrypto(MD5_DIGEST_LENGTH)
    , _outputCrypto(MD5_DIGEST_LENGTH)
    , _checkTimer(10 * IN_MILLISECONDS)
    , _clientResponseTimer(0)
    , _dataSent(false)
    , _previousTimestamp(0)
    , _initialized(false)
{ }

void Warden::banAccountIfNeeded() const
{
    if (sWorld->getBoolConfig(CONFIG_WARDEN_ENABLED))
    {
        std::string username;
        AccountMgr::GetName(_session->GetAccountId(), username);
        AccountMgr::normalizeString(username);

        std::string reason("Sentinel Anti-Cheat");
        sWorld->BanAccount(BAN_ACCOUNT, username, "3d", reason, "Sentinel");
        sWorld->SendWorldText(LANG_ACC_BANNED, username.c_str(), "3 days", reason.c_str());
    }
}

// common function for packet sending
void Warden::sendPacket(uint32 opcode, void const *data, size_t dataSize)
{
    WorldPacket packet(opcode, sizeof(uint32) + dataSize);
    packet << uint32(dataSize);
    packet.append(static_cast<uint8 const *>(data), dataSize);

    _session->SendPacket(&packet);
}

void Warden::DecryptData(uint8* buffer, uint32 length)
{
    _inputCrypto.UpdateData(length, buffer);
}

void Warden::EncryptData(uint8* buffer, uint32 length)
{
    _outputCrypto.UpdateData(length, buffer);
}

bool Warden::IsValidCheckSum(uint32 checksum, const uint8* data, const uint16 length)
{
    return BuildChecksum(data, length) == checksum;
}

uint32 Warden::BuildChecksum(const uint8* data, uint32 length)
{
    struct KeyData final
    {
        union
        {
            uint8 bytes[20];
            uint32 ints[5];
        };
    };

    KeyData hash;
    SHA1(data, length, hash.bytes);
    uint32 checkSum = 0;
    for (uint8 i = 0; i < 5; ++i)
        checkSum = checkSum ^ hash.ints[i];

    return checkSum;
}

void Warden::Update()
{
    if (_initialized)
    {
        uint32 currentTimestamp = getMSTime();
        uint32 diff = currentTimestamp - _previousTimestamp;
        _previousTimestamp = currentTimestamp;

        if (_dataSent)
        {
            uint32 maxClientResponseDelay = sWorld->getIntConfig(CONFIG_WARDEN_CLIENT_RESPONSE_DELAY);

            if (maxClientResponseDelay > 0)
            {
                // Kick player if client response delays more than set in config
                if (_clientResponseTimer > maxClientResponseDelay * IN_MILLISECONDS)
                {
                    TC_LOG_INFO("warden", "%u - exceeded Warden module response delay for more than %s, disconnecting client",
                                _session->GetAccountId(), secsToTimeString(maxClientResponseDelay, true).c_str());
                    _session->KickPlayer();
                }
                else
                    _clientResponseTimer += diff;
            }
        }
        else
        {
            if (diff >= _checkTimer)
            {
                RequestData();
                // 25-35 seconds
                _checkTimer = irand(25, 35) * IN_MILLISECONDS;
            }
            else
                _checkTimer -= diff;
        }
    }
}

void WorldSession::HandleWardenDataOpcode(WorldPacket& recvData)
{
    if (!_warden || recvData.empty())
        return;

    /// [CHANGES:KVaks] now protocol have size of data before content
    uint32 dataSize;
    recvData >> dataSize;

    _warden->DecryptData(recvData.contents() + recvData.rpos(), dataSize);

    uint8 opcode;
    recvData >> opcode;
    TC_LOG_DEBUG("warden", "%u - Got packet, opcode %02X, size %u", GetAccountId(), opcode, uint32(recvData.size()));
    TC_LOG_TRACE("warden", "%s", recvData.hexlike().c_str());

    switch (opcode)
    {
        case WARDEN_CMSG_MODULE_MISSING:
            _warden->SendModuleToClient();
            break;
        case WARDEN_CMSG_MODULE_OK:
            _warden->RequestHash();
            break;
        case WARDEN_CMSG_CHEAT_CHECKS_RESULT:
            _warden->HandleData(recvData);
            break;
        case WARDEN_CMSG_MEM_CHECKS_RESULT:
            TC_LOG_DEBUG("warden", "%u - NYI WARDEN_CMSG_MEM_CHECKS_RESULT received!", GetAccountId());
            break;
        case WARDEN_CMSG_HASH_RESULT:
            _warden->HandleHashResult(recvData);
            break;
        case WARDEN_CMSG_MODULE_FAILED:
            TC_LOG_DEBUG("warden", "%u - NYI WARDEN_CMSG_MODULE_FAILED received!", GetAccountId());
            break;
        default:
            TC_LOG_DEBUG("warden", "%u - Got unknown warden opcode %02X of size %u.", GetAccountId(), opcode, uint32(recvData.size() - 1));
            break;
    }
}

// [CHANGED]
void Warden::RequestModule()
{
    TC_LOG_DEBUG("warden", "%u - Request module", _session->GetAccountId());

    // Create packet structure
    WardenModuleUse request;
    request.Command = WARDEN_SMSG_MODULE_USE;
    memcpy(request.ModuleHash, _module.hash.first, _module.hash.second);
    memcpy(request.ModuleKey, _module.key.first, _module.key.second);
    request.Size = _module.data.second;

    // Encrypt with warden RC4 key.
    EncryptData((uint8 *)&request, sizeof(WardenModuleUse));

    sendPacket(SMSG_WARDEN_DATA, &request, sizeof(request));
}

// [CHANGED]
void Warden::SendModuleToClient()
{
    TC_LOG_DEBUG("warden", "%u - Send module to client", _session->GetAccountId());

    // Create packet structure
    WardenModuleTransfer packet;

    size_t sizeLeft = _module.data.second;
    size_t pos = 0;
    uint16 burstSize;

    while (sizeLeft > 0)
    {
        burstSize = sizeLeft < 500 ? sizeLeft : 500;
        packet.Command = WARDEN_SMSG_MODULE_CACHE;
        packet.DataSize = burstSize;
        memcpy(packet.Data, &_module.data.first[pos], burstSize);
        sizeLeft -= burstSize;
        pos += burstSize;

        EncryptData((uint8*)&packet, burstSize + 3);

        sendPacket(SMSG_WARDEN_DATA, &packet, burstSize + 3);
    }
}
