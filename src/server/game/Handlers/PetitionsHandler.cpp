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
#include "Language.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "Arena.h"
#include "GossipDef.h"
#include "SocialMgr.h"

#define CHARTER_DISPLAY_ID 16161

/*enum PetitionType // dbc data
{
    PETITION_TYPE_GUILD      = 1,
    PETITION_TYPE_ARENA_TEAM = 3
};*/

// Charters ID in item_template
enum CharterItemIDs
{
    GUILD_CHARTER                                 = 5863,
    ARENA_TEAM_CHARTER_2v2                        = 23560,
    ARENA_TEAM_CHARTER_3v3                        = 23561,
    ARENA_TEAM_CHARTER_5v5                        = 23562
};

enum CharterCosts
{
    GUILD_CHARTER_COST                            = 1000,
    ARENA_TEAM_CHARTER_2v2_COST                   = 800000,
    ARENA_TEAM_CHARTER_3v3_COST                   = 1200000,
    ARENA_TEAM_CHARTER_5v5_COST                   = 2000000
};

void WorldSession::HandlePetitionBuyOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_PETITION_BUY");
    ObjectGuid npcGuid;
    uint32 nameLen = 0;
    std::string name;

    recvData.ReadBitSeq<3>(npcGuid);
    nameLen = recvData.ReadBits(7);
    recvData.ReadBitSeq<4, 7, 1, 5, 2, 6, 0>(npcGuid);

    recvData.ReadByteSeq<6>(npcGuid);
    name = recvData.ReadString(nameLen);
    recvData.ReadByteSeq<1, 7, 0, 5, 3, 2, 4>(npcGuid);

    TC_LOG_DEBUG("network", "Petitioner with GUID %u tried sell petition: name %s", GUID_LOPART(npcGuid), name.c_str());

    // prevent cheating
    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(npcGuid, UNIT_NPC_FLAG_PETITIONER);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandlePetitionBuyOpcode - Unit (GUID: %u) not found or you can't interact with him.", GUID_LOPART(npcGuid));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    uint32 charterid = 0;
    uint32 cost = 0;
    uint32 type = 0;
    if (creature->IsTabardDesigner())
    {
        // if tabard designer, then trying to buy a guild charter.
        // do not let if already in guild.
        if (_player->GetGuildId())
            return;

        charterid = GUILD_CHARTER;
        cost = GUILD_CHARTER_COST;
        type = GUILD_CHARTER_TYPE;
    }

    if (type == GUILD_CHARTER_TYPE)
    {
        if (sGuildMgr->GetGuildByName(name))
        {
            Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_EXISTS_S, name);
            return;
        }
        if (sObjectMgr->IsReservedName(name) || !ObjectMgr::IsValidCharterName(name))
        {
            Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_INVALID, name);
            return;
        }
    }

    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(charterid);
    if (!pProto)
    {
        _player->SendBuyError(BUY_ERR_CANT_FIND_ITEM, NULL, charterid, 0);
        return;
    }

    if (!_player->HasEnoughMoney(uint64(cost)))
    {                                                       //player hasn't got enough money
        _player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, creature, charterid, 0);
        return;
    }

    ItemPosCountVec dest;
    InventoryResult msg = _player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, charterid, pProto->BuyCount);
    if (msg != EQUIP_ERR_OK)
    {
        _player->SendEquipError(msg, NULL, NULL, charterid);
        return;
    }

    _player->ModifyMoney(-(int32)cost);
    Item* charter = _player->StoreNewItem(dest, charterid, true);
    if (!charter)
        return;

    charter->SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1, charter->GetGUIDLow());
    // ITEM_FIELD_ENCHANTMENT_1_1 is guild/arenateam id
    // ITEM_FIELD_ENCHANTMENT_1_1+1 is current signatures count (showed on item)
    charter->SetState(ITEM_CHANGED, _player);
    _player->SendNewItem(charter, 1, true, false);

    // a petition is invalid, if both the owner and the type matches
    // we checked above, if this player is in an arenateam, so this must be
    // datacorruption
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_BY_OWNER);
    stmt->setUInt32(0, _player->GetGUIDLow());
    stmt->setUInt8(1, type);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    std::ostringstream ssInvalidPetitionGUIDs;

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            ssInvalidPetitionGUIDs << '\'' << fields[0].GetUInt32() << "', ";
        }
        while (result->NextRow());
    }

    // delete petitions with the same guid as this one
    ssInvalidPetitionGUIDs << '\'' << charter->GetGUIDLow() << '\'';

    TC_LOG_DEBUG("network", "Invalid petition GUIDs: %s", ssInvalidPetitionGUIDs.str().c_str());
    CharacterDatabase.EscapeString(name);
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM petition WHERE petitionguid IN (%s)",  ssInvalidPetitionGUIDs.str().c_str());
    trans->PAppend("DELETE FROM petition_sign WHERE petitionguid IN (%s)", ssInvalidPetitionGUIDs.str().c_str());

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PETITION);
    stmt->setUInt32(0, _player->GetGUIDLow());
    stmt->setUInt32(1, charter->GetGUIDLow());
    stmt->setString(2, name);
    stmt->setUInt8(3, uint8(type));
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void WorldSession::HandlePetitionShowSignOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_PETITION_SHOW_SIGNATURES");

    uint8 signs = 0;
    ObjectGuid petitionguid;

    recvData.ReadBitSeq<4, 2, 5, 1, 0, 7, 6, 3>(petitionguid);
    recvData.ReadByteSeq<7, 5, 3, 0, 6, 1, 4, 2>(petitionguid);

    // solve (possible) some strange compile problems with explicit use GUID_LOPART(petitionguid) at some GCC versions (wrong code optimization in compiler?)
    uint32 petitionGuidLow = GUID_LOPART(petitionguid);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_TYPE);

    stmt->setUInt32(0, petitionGuidLow);

    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_DEBUG("entities.player.items", "Petition %u is not found for player %u %s",
                     GUID_LOPART(petitionguid), GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str());
        return;
    }
    Field* fields = result->Fetch();
    uint32 type = fields[0].GetUInt8();

    // if guild petition and has guild => error, return;
    if (type == GUILD_CHARTER_TYPE && _player->GetGuildId())
        return;

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURE);

    stmt->setUInt32(0, petitionGuidLow);

    result = CharacterDatabase.Query(stmt);

    // result == NULL also correct in case no sign yet
    if (result)
        signs = uint8(result->GetRowCount());

    TC_LOG_DEBUG("network", "CMSG_PETITION_SHOW_SIGNATURES petition entry: '%u'", petitionGuidLow);

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES);
    ByteBuffer signsBuffer;
    ObjectGuid playerGuid = _player->GetGUID();

    data.WriteBitSeq<5>(playerGuid);
    data.WriteBitSeq<0, 4, 7>(petitionguid);
    data.WriteBitSeq<6>(playerGuid);
    data.WriteBitSeq<1, 2, 5, 3>(petitionguid);
    data.WriteBitSeq<2, 4, 0, 3>(playerGuid);
    data.WriteBitSeq<6>(petitionguid);
    data.WriteBitSeq<1>(playerGuid);
    data.WriteBits(signs, 21);

    for (uint8 i = 1; i <= signs; ++i)
    {
        Field* fields2 = result->Fetch();
        uint32 lowGuid = fields2[0].GetUInt32();

        ObjectGuid signerGuid = MAKE_NEW_GUID(lowGuid, 0, HIGHGUID_PLAYER);

        data.WriteBitSeq<0, 4, 2, 6, 3, 7, 5, 1>(signerGuid);

        signsBuffer.WriteByteSeq<4, 6, 0, 5, 7, 2, 1>(signerGuid);
        signsBuffer << uint32(0);
        signsBuffer.WriteByteSeq<3>(signerGuid);

        result->NextRow();
    }

    data.WriteBitSeq<7>(playerGuid);
    data.FlushBits();

    if (signsBuffer.size())
        data.append(signsBuffer);

    data.WriteByteSeq<7, 5, 3>(playerGuid);
    data << uint32(petitionGuidLow);
    data.WriteByteSeq<4>(petitionguid);
    data.WriteByteSeq<2>(playerGuid);
    data.WriteByteSeq<5, 1>(petitionguid);
    data.WriteByteSeq<4>(playerGuid);
    data.WriteByteSeq<0>(petitionguid);
    data.WriteByteSeq<0>(playerGuid);
    data.WriteByteSeq<7>(petitionguid);
    data.WriteByteSeq<6, 1>(playerGuid);
    data.WriteByteSeq<3, 6, 2>(petitionguid);

    SendPacket(&data);
}

void WorldSession::HandlePetitionQueryOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_PETITION_QUERY");   // ok

    uint32 guildId;
    ObjectGuid petitionGuid;

    recvData >> guildId;

    recvData.ReadBitSeq<7, 5, 4, 1, 3, 6, 0, 2>(petitionGuid);
    recvData.ReadByteSeq<4, 3, 5, 6, 1, 7, 0, 2>(petitionGuid);

    TC_LOG_DEBUG("network", "CMSG_PETITION_QUERY Petition GUID %u Guild GUID %u", GUID_LOPART(petitionGuid), guildId);

    SendPetitionQueryOpcode(uint64(guildId));
}

void WorldSession::SendPetitionQueryOpcode(uint64 petitionguid)
{
    uint64 ownerguid = 0;
    std::string name = "NO_NAME_FOR_GUID";

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION);

    stmt->setUInt32(0, GUID_LOPART(petitionguid));

    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        Field* fields = result->Fetch();
        ownerguid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        name      = fields[1].GetString();
    }
    else
    {
        TC_LOG_DEBUG("network", "CMSG_PETITION_QUERY failed for petition (GUID: %u)", GUID_LOPART(petitionguid));
        return;
    }

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE);
    ObjectGuid ownerGuid = ownerguid;

    data.WriteBit(1);

    data.WriteBitSeq<7, 5>(ownerGuid);
    data.WriteBits(name.size(), 7);
    data.WriteBitSeq<1>(ownerGuid);
    data.WriteBits(0, 12);                  // Unk String
    data.WriteBitSeq<4, 6>(ownerGuid);

    for (int i = 0; i < 10; ++i)
        data.WriteBits(0, 6);               // Unk Strings

    data.WriteBitSeq<3, 0, 2>(ownerGuid);

    data << uint32(0);
    data << uint32(0);
    data.WriteByteSeq<0, 2>(ownerGuid);
    data << uint32(GUID_LOPART(petitionguid));
    data.WriteByteSeq<1>(ownerGuid);
    data << uint32(4);
    data.WriteByteSeq<3, 4>(ownerGuid);
    data << uint32(0);

    if (name.size())
        data.append(name.c_str(), name.size());

    data << uint16(0);
    data << uint32(0);
    data << uint32(0);
    data.WriteByteSeq<7>(ownerGuid);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data.WriteByteSeq<6, 5>(ownerGuid);
    data << uint32(4);
    data << uint32(0);

    data << uint32(GUID_LOPART(petitionguid));

    SendPacket(&data);
}

void WorldSession::HandlePetitionRenameOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_PETITION_RENAME");

    ObjectGuid petitionGuid;
    ObjectGuid playerGuid;
    std::string newName;
    uint32 nameLen = 0;

    nameLen = recvData.ReadBits(7);

    recvData.ReadBitSeq<1, 4, 3, 5, 7, 2, 0, 6>(playerGuid);
    recvData.ReadByteSeq<1, 7, 2, 5, 4, 3, 6>(playerGuid);
    newName = recvData.ReadString(nameLen);
    recvData.ReadByteSeq<0>(playerGuid);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_BY_OWNER);
    stmt->setUInt32(0, GUID_LOPART(playerGuid));
    stmt->setUInt8(1, GUILD_CHARTER_TYPE);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_DEBUG("network", "CMSG_PETITION_QUERY failed for player (GUID: %u)", GUID_LOPART(playerGuid));
        return;
    }

    Field* fields = result->Fetch();
    petitionGuid = MAKE_NEW_GUID(fields[0].GetInt32(), 0, HIGHGUID_ITEM);

    Item* item = _player->GetItemByGuid(petitionGuid);
    if (!item)
    {
        TC_LOG_DEBUG("network", "CMSG_PETITION_QUERY failed for petition (GUID: %u)", GUID_LOPART(petitionGuid));
        return;
    }

    if (sGuildMgr->GetGuildByName(newName))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_EXISTS_S, newName);
        return;
    }
    if (sObjectMgr->IsReservedName(newName) || !ObjectMgr::IsValidCharterName(newName))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_INVALID, newName);
        return;
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PETITION_NAME);
    stmt->setString(0, newName);
    stmt->setUInt32(1, GUID_LOPART(petitionGuid));
    CharacterDatabase.Execute(stmt);

    TC_LOG_DEBUG("network", "Petition (GUID: %u) renamed to '%s'", GUID_LOPART(petitionGuid), newName.c_str());

    WorldPacket data(SMSG_PETITION_RENAME);

    data.WriteBitSeq<5, 0, 1, 6, 7>(petitionGuid);
    data.WriteBits(nameLen, 7);
    data.WriteBitSeq<4, 3, 2>(petitionGuid);

    data.WriteByteSeq<7, 4>(petitionGuid);
    if (nameLen)
        data.append(newName.c_str(), nameLen);
    data.WriteByteSeq<0, 3, 6, 1, 2, 5>(petitionGuid);

    SendPacket(&data);
}

void WorldSession::HandlePetitionSignOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_PETITION_SIGN");

    Field* fields;
    ObjectGuid ownerGuid;
    ObjectGuid petitionGuid;
    uint8 unk;

    recvData >> unk;

    recvData.ReadBitSeq<5, 2, 6, 7, 1, 0, 3, 4>(ownerGuid);
    recvData.ReadByteSeq<4, 7, 6, 3, 0, 2, 5, 1>(ownerGuid);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_BY_OWNER);
    stmt->setUInt32(0, GUID_LOPART(ownerGuid));
    stmt->setUInt8(1, GUILD_CHARTER_TYPE);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    fields = result->Fetch();
    petitionGuid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_ITEM);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURES);
    stmt->setUInt32(0, GUID_LOPART(petitionGuid));
    stmt->setUInt32(1, GUID_LOPART(petitionGuid));
    result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_ERROR("network", "Petition %u is not found for player %u %s",
                     GUID_LOPART(petitionGuid), GetPlayer()->GetGUIDLow(), GetPlayer()->GetName().c_str());
        return;
    }

    fields = result->Fetch();
    uint64 signs = fields[1].GetUInt64();
    uint8 type = fields[2].GetUInt8();

    uint32 playerGuid = _player->GetGUIDLow();
    if (GUID_LOPART(ownerGuid) == playerGuid)
        return;

    // not let enemies sign guild charter
    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && GetPlayer()->GetTeam() != sObjectMgr->GetPlayerTeamByGUID(ownerGuid))
    {
        if (type == GUILD_CHARTER_TYPE)
            Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NOT_ALLIED);
        return;
    }

    if (type == GUILD_CHARTER_TYPE)
    {
        if (_player->GetGuildId())
        {
            Guild::SendCommandResult(this, GUILD_INVITE_S, ERR_ALREADY_IN_GUILD_S, _player->GetName());
            return;
        }
        if (_player->GetGuildIdInvited())
        {
            Guild::SendCommandResult(this, GUILD_INVITE_S, ERR_ALREADY_INVITED_TO_GUILD_S, _player->GetName());
            return;
        }
    }

    if (++signs > type)                                        // client signs maximum
        return;

    // Client doesn't allow to sign petition two times by one character, but not check sign by another character from same account
    // not allow sign another player from already sign player account
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIG_BY_ACCOUNT);

    stmt->setUInt32(0, GetAccountId());
    stmt->setUInt32(1, GUID_LOPART(petitionGuid));

    result = CharacterDatabase.Query(stmt);

    if (result)
    {
        SendPetitionSignResult(_player->GetGUID(), petitionGuid, PETITION_SIGN_ALREADY_SIGNED);
        return;
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PETITION_SIGNATURE);

    stmt->setUInt32(0, GUID_LOPART(ownerGuid));
    stmt->setUInt32(1, GUID_LOPART(petitionGuid));
    stmt->setUInt32(2, playerGuid);
    stmt->setUInt32(3, GetAccountId());

    CharacterDatabase.Execute(stmt);

    TC_LOG_DEBUG("network", "PETITION SIGN: GUID %u by player: %s (GUID: %u Account: %u)",
                 GUID_LOPART(petitionGuid), _player->GetName().c_str(), playerGuid, GetAccountId());

    SendPetitionSignResult(_player->GetGUID(), petitionGuid, PETITION_SIGN_OK);

    Player* owner = ObjectAccessor::FindPlayer(ownerGuid);
    if (owner)
        owner->GetSession()->SendPetitionSignResult(_player->GetGUID(), petitionGuid, PETITION_SIGN_OK);
}

void WorldSession::HandlePetitionDeclineOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_PETITION_DECLINE");

    ObjectGuid petitionGuid;
    ObjectGuid ownerGuid;

    recvData.ReadBitSeq<7, 5, 6, 2, 4, 3, 1, 0>(ownerGuid);
    recvData.ReadByteSeq<1, 0, 6, 4, 7, 5, 3, 2>(ownerGuid);

    TC_LOG_DEBUG("network", "Petition %u declined by %u", GUID_LOPART(petitionGuid), _player->GetGUIDLow());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_BY_OWNER);

    stmt->setUInt32(0, GUID_LOPART(ownerGuid));
    stmt->setUInt8(1, GUILD_CHARTER_TYPE);

    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    Field* fields = result->Fetch();
    petitionGuid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_ITEM);

    Player* owner = ObjectAccessor::FindPlayer(ownerGuid);
    if (owner)
        owner->GetSession()->SendPetitionSignResult(ownerGuid, petitionGuid, PETITION_SIGN_DECLINED);
}

void WorldSession::HandleOfferPetitionOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_OFFER_PETITION");

    uint8 signs = 0;
    ObjectGuid ownerGuid, playerGuid;
    uint32 type, junk;
    uint32 petitionGuidLow = 0;
    Player* player;

    recvData >> junk;                                      // this is not petition type!

    recvData.ReadBitSeq<5, 0>(playerGuid);
    recvData.ReadBitSeq<6, 3, 5, 7>(ownerGuid);
    recvData.ReadBitSeq<3>(playerGuid);
    recvData.ReadBitSeq<4>(ownerGuid);
    recvData.ReadBitSeq<6, 4, 2>(playerGuid);
    recvData.ReadBitSeq<2>(ownerGuid);
    recvData.ReadBitSeq<1>(playerGuid);
    recvData.ReadBitSeq<1, 0>(ownerGuid);
    recvData.ReadBitSeq<7>(playerGuid);

    recvData.ReadByteSeq<0>(ownerGuid);
    recvData.ReadByteSeq<2, 4>(playerGuid);
    recvData.ReadByteSeq<2>(ownerGuid);
    recvData.ReadByteSeq<1, 6>(playerGuid);
    recvData.ReadByteSeq<7, 3>(ownerGuid);
    recvData.ReadByteSeq<5>(playerGuid);
    recvData.ReadByteSeq<6, 1>(ownerGuid);
    recvData.ReadByteSeq<7, 0, 3>(playerGuid);
    recvData.ReadByteSeq<5, 4>(ownerGuid);

    player = ObjectAccessor::FindPlayer(playerGuid);
    if (!player)
        return;

    type = GUILD_CHARTER_TYPE;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_BY_OWNER);

    stmt->setUInt32(0, GUID_LOPART(ownerGuid));
    stmt->setUInt8(1, type);

    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    Field* fields = result->Fetch();
    petitionGuidLow = fields[0].GetUInt32();

    TC_LOG_DEBUG("network", "OFFER PETITION: type %u, GUID1 %u, to player id: %u", type, petitionGuidLow, GUID_LOPART(playerGuid));

    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && GetPlayer()->GetTeam() != player->GetTeam())
    {
        if (type == GUILD_CHARTER_TYPE)
            Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NOT_ALLIED);
        return;
    }

    if (type == GUILD_CHARTER_TYPE)
    {
        if (player->GetGuildIdInvited())
        {
            SendPetitionSignResult(_player->GetGUID(), MAKE_NEW_GUID(petitionGuidLow, 0, HIGHGUID_ITEM), PETITION_SIGN_ALREADY_SIGNED_OTHER);
            return;
        }

        if (player->GetGuildId())
        {
            SendPetitionSignResult(_player->GetGUID(), MAKE_NEW_GUID(petitionGuidLow, 0, HIGHGUID_ITEM), PETITION_SIGN_ALREADY_IN_GUILD);
            return;
        }
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURE);
    stmt->setUInt32(0, petitionGuidLow);
    result = CharacterDatabase.Query(stmt);

    std::list<uint32> playerGuids;

    // result == NULL also correct charter without signs
    if (result)
    {
        signs = uint8(result->GetRowCount());

        do
        {
            fields = result->Fetch();
            playerGuids.push_back(fields[0].GetUInt32());
        }
        while (result->NextRow());
    }

    for (auto itr : playerGuids)
    {
        if (GUID_LOPART(playerGuid) == itr)
        {
            player->GetSession()->SendAlreadySigned(playerGuid);
            return;
        }
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURE);
    stmt->setUInt32(0, petitionGuidLow);
    result = CharacterDatabase.Query(stmt);

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES);
    ByteBuffer signsBuffer;
    ObjectGuid petitionGuid = MAKE_NEW_GUID(petitionGuidLow, 0, HIGHGUID_ITEM);
    ObjectGuid masterGuid = _player->GetGUID();

    data.WriteBitSeq<5>(masterGuid);
    data.WriteBitSeq<0, 4, 7>(petitionGuid);
    data.WriteBitSeq<6>(masterGuid);
    data.WriteBitSeq<1, 2, 5, 3>(petitionGuid);
    data.WriteBitSeq<2, 4, 0, 3>(masterGuid);
    data.WriteBitSeq<6>(petitionGuid);
    data.WriteBitSeq<1>(masterGuid);
    data.WriteBits(signs, 21);

    for (uint8 i = 1; i <= signs; ++i)
    {
        Field* fields2 = result->Fetch();
        uint32 lowGuid = fields2[0].GetUInt32();

        ObjectGuid signerGuid = MAKE_NEW_GUID(lowGuid, 0, HIGHGUID_PLAYER);

        data.WriteBitSeq<0, 4, 2, 6, 3, 7, 5, 1>(signerGuid);

        signsBuffer.WriteByteSeq<4, 6, 0, 5, 7, 2, 1>(signerGuid);
        signsBuffer << uint32(0);
        signsBuffer.WriteByteSeq<3>(signerGuid);

        result->NextRow();
    }

    data.WriteBitSeq<7>(masterGuid);
    data.FlushBits();

    if (signsBuffer.size())
        data.append(signsBuffer);

    data.WriteByteSeq<7, 5, 3>(masterGuid);
    data << uint32(petitionGuidLow);
    data.WriteByteSeq<4>(petitionGuid);
    data.WriteByteSeq<2>(masterGuid);
    data.WriteByteSeq<5, 1>(petitionGuid);
    data.WriteByteSeq<4>(masterGuid);
    data.WriteByteSeq<0>(petitionGuid);
    data.WriteByteSeq<0>(masterGuid);
    data.WriteByteSeq<7>(petitionGuid);
    data.WriteByteSeq<6, 1>(masterGuid);
    data.WriteByteSeq<3, 6, 2>(petitionGuid);

    player->GetSession()->SendPacket(&data);
}

void WorldSession::HandleTurnInPetitionOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_TURN_IN_PETITION");

    // Get petition guid from packet
    WorldPacket data;
    ObjectGuid petitionGuid;

    recvData.ReadBitSeq<2, 0, 1, 4, 6, 5, 3, 7>(petitionGuid);
    recvData.ReadByteSeq<3, 6, 7, 0, 1, 2, 5, 4>(petitionGuid);

    // Check if player really has the required petition charter
    Item* item = _player->GetItemByGuid(petitionGuid);
    if (!item)
        return;

    TC_LOG_DEBUG("network", "Petition %u turned in by %u", GUID_LOPART(petitionGuid), _player->GetGUIDLow());

    // Get petition data from db
    uint32 ownerguidlo;
    std::string name;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION);
    stmt->setUInt32(0, GUID_LOPART(petitionGuid));
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        Field* fields = result->Fetch();
        ownerguidlo = fields[0].GetUInt32();
        name = fields[1].GetString();
    }
    else
    {
        TC_LOG_ERROR("network", "Player %s (guid: %u) tried to turn in petition (guid: %u) that is not present in the database",
                     _player->GetName().c_str(), _player->GetGUIDLow(), GUID_LOPART(petitionGuid));
        return;
    }

    // Only the petition owner can turn in the petition
    if (_player->GetGUIDLow() != ownerguidlo)
        return;

    // Check if player is already in a guild
    if (_player->GetGuildId())
    {
        data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
        data.WriteBits(PETITION_TURN_ALREADY_IN_GUILD, 4);
        data.FlushBits();
        SendPacket(&data);
        return;
    }

    // Check if guild name is already taken
    if (sGuildMgr->GetGuildByName(name))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_EXISTS_S, name);
        return;
    }

    // Get petition signatures from db
    uint8 signatures;

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURE);
    stmt->setUInt32(0, GUID_LOPART(petitionGuid));
    result = CharacterDatabase.Query(stmt);

    if (result)
        signatures = uint8(result->GetRowCount());
    else
        signatures = 0;

    uint32 requiredSignatures;
    requiredSignatures = sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS);

    // Notify player if signatures are missing
    if (signatures < requiredSignatures)
    {
        data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
        data.WriteBits(PETITION_TURN_NEED_MORE_SIGNATURES, 4);
        data.FlushBits();
        SendPacket(&data);
        return;
    }

    // Proceed with guild/arena team creation

    // Delete charter item
    _player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);

    // Create guild
    Guild* guild = new Guild;

    if (!guild->Create(_player, name))
    {
        delete guild;
        return;
    }

    // Register guild and add guild master
    sGuildMgr->AddGuild(guild);

    // Add members from signatures
    for (uint8 i = 0; i < signatures; ++i)
    {
        Field* fields = result->Fetch();
        guild->AddMember(MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER));
        result->NextRow();
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PETITION_BY_GUID);
    stmt->setUInt32(0, GUID_LOPART(petitionGuid));
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PETITION_SIGNATURE_BY_GUID);
    stmt->setUInt32(0, GUID_LOPART(petitionGuid));
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    // created
    TC_LOG_DEBUG("network", "TURN IN PETITION GUID %u", GUID_LOPART(petitionGuid));

    data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
    data.WriteBits(PETITION_TURN_OK, 4);
    data.FlushBits();
    SendPacket(&data);
}

void WorldSession::HandlePetitionShowListOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received CMSG_PETITION_SHOWLIST");

    uint64 guid;
    recvData >> guid;

    SendPetitionShowList(guid);
}

void WorldSession::SendPetitionShowList(uint64 guid)
{
    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_PETITIONER);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandlePetitionShowListOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    WorldPacket data(SMSG_PETITION_SHOW_LIST, 8 + 1 + 4 * 6);
    ObjectGuid npcGuid = guid;

    data.WriteBitSeq<7, 2, 0, 1, 5, 3, 6, 4>(npcGuid);

    data.WriteByteSeq<2, 3, 7, 5, 4, 0, 6>(npcGuid);
    data << uint32(GUILD_CHARTER_COST);                 // charter cost
    data.WriteByteSeq<1>(npcGuid);

    SendPacket(&data);
    TC_LOG_DEBUG("network", "Sent SMSG_PETITION_SHOW_LIST");
}

void WorldSession::SendPetitionSignResult(ObjectGuid ownerGuid, ObjectGuid petitionGuid, uint8 result)
{
    WorldPacket data(SMSG_PETITION_DECLINED);

    data.WriteBitSeq<5>(ownerGuid);
    data.WriteBitSeq<7, 4>(petitionGuid);
    data.WriteBitSeq<3, 1>(ownerGuid);
    data.WriteBitSeq<6, 1, 3>(petitionGuid);
    data.WriteBitSeq<7>(ownerGuid);
    data.WriteBits(PetitionSigns(result), 4);
    data.WriteBitSeq<0>(petitionGuid);
    data.WriteBitSeq<2>(ownerGuid);
    data.WriteBitSeq<5>(petitionGuid);
    data.WriteBitSeq<4, 0, 6>(ownerGuid);
    data.WriteBitSeq<2>(petitionGuid);

    data.WriteByteSeq<2, 4, 7, 3>(petitionGuid);
    data.WriteByteSeq<7, 2, 3>(ownerGuid);
    data.WriteByteSeq<6, 5>(petitionGuid);
    data.WriteByteSeq<4, 0>(ownerGuid);
    data.WriteByteSeq<0>(petitionGuid);
    data.WriteByteSeq<5, 6, 1>(ownerGuid);
    data.WriteByteSeq<1>(petitionGuid);

    SendPacket(&data);
}

void WorldSession::SendAlreadySigned(ObjectGuid playerGuid)
{
    WorldPacket data(SMSG_PETITION_ALREADY_SIGNED);

    data.WriteBitSeq<4, 0, 7, 1, 5, 6, 2, 3>(playerGuid);
    data.WriteByteSeq<6, 0, 2, 4, 3, 1, 5, 7>(playerGuid);

    SendPacket(&data);
}
