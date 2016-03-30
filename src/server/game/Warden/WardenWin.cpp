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

#include "WardenWin.h"
#include "Cryptography/HMACSHA1.h"
#include "Cryptography/WardenKeyGeneration.h"
#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "Opcodes.h"
#include "ByteBuffer.h"
#include "Database/DatabaseEnv.h"
#include "World.h"
#include "Player.h"
#include "Util.h"
#include "WardenModuleWin.h"
#include "WardenCheckMgr.h"
#include "AccountMgr.h"

#include <openssl/md5.h>

namespace {

/// decode from stream uint64 value
uint64 decode64(ByteBuffer& buffer)
{
    uint8 values[8];

    uint8 mask;
    buffer >> mask;
    buffer >> values[0];

    uint8 fill = (mask & 1) ? 0xff : 0x00;
    mask >>= 1;
    for (uint32 n = 1; n < 8; n++,mask >>= 1)
    {
        if ((mask & 1) != 0)
            buffer >> values[n];
        else
            values[n] = fill;
    }

    return *reinterpret_cast<uint64 const *>(values);
}

} // namespace

WardenWin::WardenWin() : Warden(), _serverTicks(0) {}

WardenWin::~WardenWin() { }

void WardenWin::Init(WorldSession* session, BigNumber* k)
{
    _session = session;
    // Generate Warden Key
    SHA1Randx WK(k->AsByteArray().get(), k->GetNumBytes());

    // Initial encryption is done with session key
    uint8 inputKey[MD5_DIGEST_LENGTH];
    uint8 outputKey[MD5_DIGEST_LENGTH];

    auto const inputKeySize = sizeof(inputKey);
    auto const outputKeySize = sizeof(outputKey);

    WK.Generate(inputKey, inputKeySize);
    WK.Generate(outputKey, outputKeySize);

    _inputCrypto.Init(inputKey);
    _outputCrypto.Init(outputKey);

    _module.data = std::make_pair(winWardenModule.Module, sizeof(winWardenModule.Module));
    _module.key = std::make_pair(winWardenModule.ModuleKey, sizeof(winWardenModule.ModuleKey));
    _module.seed = std::make_pair(winWardenModule.Seed, sizeof(winWardenModule.Seed));
    _module.hash = std::make_pair(winWardenModule.Hash, sizeof(winWardenModule.Hash));
    _module.serverKeySeed = std::make_pair(winWardenModule.ServerKeySeed, sizeof(winWardenModule.ServerKeySeed));
    _module.clientKeySeed = std::make_pair(winWardenModule.ClientKeySeed, sizeof(winWardenModule.ClientKeySeed));
    _module.clientKeySeedHash = std::make_pair(winWardenModule.ClientKeySeedHash, sizeof(winWardenModule.ClientKeySeedHash));

    TC_LOG_DEBUG("warden", "%u - Server side warden initializing...", session->GetAccountId());
    TC_LOG_DEBUG("warden", "%u - C->S Key: %s", session->GetAccountId(), ByteArrayToHexStr(inputKey, inputKeySize).c_str());
    TC_LOG_DEBUG("warden", "%u - S->C Key: %s", session->GetAccountId(), ByteArrayToHexStr(outputKey, outputKeySize).c_str());
    TC_LOG_DEBUG("warden", "%u - Module Key: %s", session->GetAccountId(), ByteArrayToHexStr(_module.key.first, _module.key.second).c_str());
    TC_LOG_DEBUG("warden", "%u - Module Hash: %s", session->GetAccountId(), ByteArrayToHexStr(_module.hash.first, _module.hash.second).c_str());

    RequestModule();
}

void WardenWin::RequestData()
{
    TC_LOG_DEBUG("warden", "%u - Request data", _session->GetAccountId());

    // If all checks were done, fill the todo list again
    if (_memChecksTodo.empty())
        _memChecksTodo.assign(sWardenCheckMgr->MemChecksIdPool.begin(), sWardenCheckMgr->MemChecksIdPool.end());

    if (_otherChecksTodo.empty())
        _otherChecksTodo.assign(sWardenCheckMgr->OtherChecksIdPool.begin(), sWardenCheckMgr->OtherChecksIdPool.end());

    _serverTicks = getMSTime();

    uint16 id;
    WardenCheck const *wd;
    _currentChecks.clear();

    // Build check request
    for (uint32 i = 0; i < sWorld->getIntConfig(CONFIG_WARDEN_NUM_MEM_CHECKS); ++i)
    {
        // If todo list is done break loop (will be filled on next Update() run)
        if (_memChecksTodo.empty())
            break;

        // Get check id from the end and remove it from todo
        id = _memChecksTodo.back();
        _memChecksTodo.pop_back();

        // Add the id to the list sent in this cycle
        _currentChecks.push_back(id);
    }

    ByteBuffer buff;
    buff << uint8(WARDEN_SMSG_CHEAT_CHECKS_REQUEST);

    // always "wow.exe"
    std::string exename = "@exe";
    buff << uint8(exename.size());
    buff.append(exename.c_str(), exename.size());

    WardenCheckMgr::ReadGuardType g(sWardenCheckMgr->_checkStoreLock);

    for (uint32 i = 0; i < sWorld->getIntConfig(CONFIG_WARDEN_NUM_OTHER_CHECKS); ++i)
    {
        // If todo list is done break loop (will be filled on next Update() run)
        if (_otherChecksTodo.empty())
            break;

        // Get check id from the end and remove it from todo
        id = _otherChecksTodo.back();
        _otherChecksTodo.pop_back();

        // Add the id to the list sent in this cycle
        _currentChecks.push_back(id);

        wd = sWardenCheckMgr->GetWardenDataById(id);
        switch (wd->Type)
        {
            case MPQ_CHECK:
            case LUA_STR_CHECK:
            case DRIVER_CHECK:
                buff << uint8(wd->Str.size());
                buff.append(wd->Str.c_str(), wd->Str.size());
                break;
            default:
                break;
        }
    }

    // string with len 0, finish of string checks
    buff << uint8(0x00);

    uint8 const xorByte = _module.clientKeySeed.first[0];

    // Add TIMING_CHECK
    buff << uint8(TIMING_CHECK ^ xorByte);

    for (auto itr = _currentChecks.begin(); itr != _currentChecks.end(); ++itr)
    {
        wd = sWardenCheckMgr->GetWardenDataById(*itr);

        switch (auto const type = wd->Type)
        {
            case MEM_CHECK:
            {
                buff << uint8(type ^ xorByte);
                buff << uint8(0x01);

                buff << uint8(0x0E);
                for (uint32 i = 0; i < 4; ++i)
                    buff << uint8((wd->Address >> (i * 8)) & 0xFF);

                buff << uint8(wd->Length);
                break;
            }
            case PAGE_CHECK_A:
            case PAGE_CHECK_B:
            {
                buff << uint8(type ^ xorByte);
                buff.append(wd->Data.AsByteArray(0, false).get(), wd->Data.GetNumBytes());
                buff << uint32(wd->Address);
                buff << uint8(wd->Length);
                break;
            }
            case MODULE_CHECK:
            {
                buff << uint8(type ^ xorByte);
                uint32 seed = rand32();
                buff << uint32(seed);
                HmacHash hmac(&seed, sizeof(seed));
                hmac.UpdateData(wd->Str);
                hmac.Finalize();
                buff.append(hmac.GetDigest(), hmac.GetLength());
                break;
            }
#if 0
            case MPQ_CHECK:
            case LUA_STR_CHECK:
            {
                buff << uint8(index++);
                break;
            }
            case DRIVER_CHECK:
            {
                buff.append(wd->Data.AsByteArray(0, false).get(), wd->Data.GetNumBytes());
                buff << uint8(index++);
                break;
            }
            case PROC_CHECK:
            {
                buff.append(wd->i.AsByteArray(0, false).get(), wd->i.GetNumBytes());
                buff << uint8(index++);
                buff << uint8(index++);
                buff << uint32(wd->Address);
                buff << uint8(wd->Length);
                break;
            }
#endif
            default:
                break;                                      // Should never happen
        }
    }

    buff << uint8(CT_END ^ xorByte);

    TC_LOG_TRACE("warden", "%s", buff.hexlike().c_str());

    // Encrypt with warden RC4 key
    EncryptData(buff.contents(), buff.size());

    sendPacket(SMSG_WARDEN_DATA, buff.contents(), buff.size());

    _dataSent = true;

    std::stringstream stream;
    stream << "Sent check id's: ";
    for (auto itr = _currentChecks.begin(); itr != _currentChecks.end(); ++itr)
        stream << *itr << " ";

    TC_LOG_DEBUG("warden", "%u - %s", _session->GetAccountId(), stream.str().c_str());
}

void WardenWin::HandleData(ByteBuffer &buff)
{
    TC_LOG_DEBUG("warden", "%u - Handle data", _session->GetAccountId());

    _dataSent = false;
    _clientResponseTimer = 0;

    uint16 length;
    uint32 checksum;

    buff >> length >> checksum;

    if (buff.wpos() - buff.rpos() < length)
    {
        TC_LOG_INFO("warden", "%u - Length is too large", _session->GetAccountId());
        TC_LOG_INFO("warden", "%u - %s", _session->GetAccountId(), buff.hexlike().c_str());

        buff.rpos(buff.wpos());
        banAccountIfNeeded();
        return;
    }

    if (!IsValidCheckSum(checksum, buff.contents() + buff.rpos(), length))
    {
        TC_LOG_INFO("warden", "%u - Checksum is invalid", _session->GetAccountId());
        TC_LOG_INFO("warden", "%u - %s", _session->GetAccountId(), buff.hexlike().c_str());

        buff.rpos(buff.wpos());
        banAccountIfNeeded();
        return;
    }

    TC_LOG_DEBUG("warden", "%u - Checksum is valid", _session->GetAccountId());

    if (length == 0)
        return;

    // TIMING_CHECK
    {
        uint8 result;
        buff >> result;
        /// @todo test it.
        if (result == 0x00)
        {
            TC_LOG_INFO("warden", "%u - failed timing check", _session->GetAccountId());
            TC_LOG_INFO("warden", "%u - %s", _session->GetAccountId(), buff.hexlike().c_str());

            _session->KickPlayer();
            return;
        }

        uint32 clientTicks;
        buff >> clientTicks;

        uint32 const ticksNow = getMSTime();
        uint32 const ourTicks = clientTicks + (ticksNow - _serverTicks);

        TC_LOG_DEBUG("warden", "%u - ServerTicks %u", _session->GetAccountId(), ticksNow);          // Now
        TC_LOG_DEBUG("warden", "%u - RequestTicks %u", _session->GetAccountId(), _serverTicks);     // At request
        TC_LOG_DEBUG("warden", "%u - Ticks %u", _session->GetAccountId(), clientTicks);             // At response
        TC_LOG_DEBUG("warden", "%u - Ticks diff %u", _session->GetAccountId(), ourTicks - clientTicks);
    }

#if 0
    // WOWWARDEN_INFO
    auto const ticks0 = decode64(buff); // address of wow.exe
    auto const ticks1 = decode64(buff); // address of warden
    auto const ticks2 = decode64(buff); // address of main warden struct
#endif

    WardenCheckResult const *rs;
    WardenCheck const *rd;
    uint8 type;
    bool checkFailed = false;

    WardenCheckMgr::ReadGuardType g(sWardenCheckMgr->_checkStoreLock);

    for (auto itr = _currentChecks.begin(); itr != _currentChecks.end() && !checkFailed; ++itr)
    {
        rd = sWardenCheckMgr->GetWardenDataById(*itr);
        rs = sWardenCheckMgr->GetWardenResultById(*itr);

        type = rd->Type;

        switch (type)
        {
            case MEM_CHECK:
            {
                uint8 Mem_Result;
                buff >> Mem_Result;

                if (Mem_Result != 0)
                {
                    TC_LOG_INFO("warden", "%u - RESULT MEM_CHECK not 0x00, CheckId %u", _session->GetAccountId(), *itr);
                    checkFailed = true;
                    continue;
                }

                if (buff.wpos() - buff.rpos() < rd->Length)
                {
                    TC_LOG_INFO("warden", "%u - RESULT MEM_CHECK truncated packet CheckId %u", _session->GetAccountId(), *itr);
                    checkFailed = true;
                    continue;
                }

                if (memcmp(buff.contents() + buff.rpos(), rs->Result.AsByteArray(0, false).get(), rd->Length) != 0)
                {
                    TC_LOG_INFO("warden", "%u - RESULT MEM_CHECK fail CheckId %u", _session->GetAccountId(), *itr);
                    checkFailed = true;
                    continue;
                }

                buff.rpos(buff.rpos() + rd->Length);
                TC_LOG_DEBUG("warden", "%u - RESULT MEM_CHECK passed CheckId %u", _session->GetAccountId(), *itr);
                break;
            }
            case PAGE_CHECK_A:
            case PAGE_CHECK_B:
            case DRIVER_CHECK:
            {
                uint8 const validMarker = 0x4A;
                if (buff.read<uint8>() != validMarker)
                {
                    if (type == PAGE_CHECK_A || type == PAGE_CHECK_B)
                        TC_LOG_INFO("warden", "%u - RESULT PAGE_CHECK fail, CheckId %u", _session->GetAccountId(), *itr);
                    else if (type == DRIVER_CHECK)
                        TC_LOG_INFO("warden", "%u - RESULT DRIVER_CHECK fail, CheckId %u", _session->GetAccountId(), *itr);

                    checkFailed = true;
                    continue;
                }

                if (type == PAGE_CHECK_A || type == PAGE_CHECK_B)
                    TC_LOG_DEBUG("warden", "%u - RESULT PAGE_CHECK passed CheckId %u", _session->GetAccountId(), *itr);
                else if (type == DRIVER_CHECK)
                    TC_LOG_DEBUG("warden", "%u - RESULT DRIVER_CHECK passed CheckId %u", _session->GetAccountId(), *itr);

                break;
            }
            case MODULE_CHECK:
            {
                uint8 const modulePresentMarker = 0x4A;
                if (buff.read<uint8>() == modulePresentMarker)
                {
                    TC_LOG_INFO("warden", "%u - RESULT MODULE_CHECK fail, CheckId %u", _session->GetAccountId(), *itr);
                    checkFailed = true;
                    continue;
                }

                TC_LOG_DEBUG("warden", "%u - RESULT MODULE_CHECK passed CheckId %u", _session->GetAccountId(), *itr);
                break;
            }
            case LUA_STR_CHECK:
            {
                uint8 Lua_Result;
                buff >> Lua_Result;

                if (Lua_Result != 0)
                {
                    TC_LOG_INFO("warden", "%u - RESULT LUA_STR_CHECK fail, CheckId %u", _session->GetAccountId(), *itr);
                    checkFailed = true;
                    continue;
                }

                uint8 luaStrLen;
                buff >> luaStrLen;

                if (luaStrLen != 0)
                {
                    std::unique_ptr<char[]> str(new char[luaStrLen + 1]());
                    memcpy(str.get(), buff.contents() + buff.rpos(), luaStrLen);
                    TC_LOG_DEBUG("warden", "%u - Lua string: %s", _session->GetAccountId(), str.get());
                }
                buff.rpos(buff.rpos() + luaStrLen);         // Skip string
                TC_LOG_DEBUG("warden", "%u - RESULT LUA_STR_CHECK passed, CheckId %u", _session->GetAccountId(), *itr);
                break;
            }
            case MPQ_CHECK:
            {
                uint8 Mpq_Result;
                buff >> Mpq_Result;

                if (Mpq_Result != 0)
                {
                    TC_LOG_INFO("warden", "%u - RESULT MPQ_CHECK not 0x00", _session->GetAccountId());
                    checkFailed = true;
                    continue;
                }

                if (memcmp(buff.contents() + buff.rpos(), rs->Result.AsByteArray(0, false).get(), 20) != 0) // SHA1
                {
                    TC_LOG_INFO("warden", "%u - RESULT MPQ_CHECK fail, CheckId %u", _session->GetAccountId(), *itr);
                    checkFailed = true;
                    buff.rpos(buff.rpos() + 20);            // 20 bytes SHA1
                    continue;
                }

                buff.rpos(buff.rpos() + 20);                // 20 bytes SHA1
                TC_LOG_DEBUG("warden", "%u - RESULT MPQ_CHECK passed, CheckId %u", _session->GetAccountId(), *itr);
                break;
            }
            default:                                        // Should never happen
                break;
        }
    }

    if (checkFailed)
    {
        TC_LOG_INFO("warden", "%u - %s", _session->GetAccountId(), buff.hexlike().c_str());
        banAccountIfNeeded();
    }

    /// TODO [KVaks] unknown tail
    buff.rpos(buff.wpos());
}

/// [CHANGED] KVaks
void WardenWin::RequestHash()
{
    TC_LOG_DEBUG("warden", "%u - Request hash", _session->GetAccountId());

    // Create packet structure
    WardenHashRequest request;
    request.Command = WARDEN_SMSG_HASH_REQUEST;
    memcpy(request.Seed, _module.seed.first, _module.seed.second);

    // Encrypt with warden RC4 key.
    EncryptData((uint8*)&request, sizeof(WardenHashRequest));

    // Send packet
    sendPacket(SMSG_WARDEN_DATA, &request, sizeof(WardenHashRequest));
}

/// [CHANGED] KVaks
void WardenWin::HandleHashResult(ByteBuffer &buff)
{
    // Verify length
    if (buff.wpos() - buff.rpos() < _module.clientKeySeedHash.second)
    {
        TC_LOG_INFO("warden", "%u - Request hash truncated packet", _session->GetAccountId());
        TC_LOG_INFO("warden", "%u - %s", _session->GetAccountId(), buff.hexlike().c_str());

        banAccountIfNeeded();
        return;
    }

    // Verify key
    if (std::memcmp(buff.contents() + buff.rpos(), _module.clientKeySeedHash.first, _module.clientKeySeedHash.second) != 0)
    {
        TC_LOG_INFO("warden", "%u - Request hash reply failed", _session->GetAccountId());
        TC_LOG_INFO("warden", "%u - %s", _session->GetAccountId(), buff.hexlike().c_str());

        banAccountIfNeeded();
        return;
    }

    buff.rpos(buff.wpos());

    TC_LOG_DEBUG("warden", "%u - Request hash reply: succeed", _session->GetAccountId());

    // reinit crypto keys
    _inputCrypto.Init(_module.clientKeySeed.first);
    _outputCrypto.Init(_module.serverKeySeed.first);

    _initialized = true;
    _previousTimestamp = getMSTime();
}
