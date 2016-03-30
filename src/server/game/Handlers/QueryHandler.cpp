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
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "NPCHandler.h"
#include "Pet.h"
#include "MapManager.h"

void WorldSession::SendNameQueryOpcode(Player const *player)
{
    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, 8 + 1 + 1 + 1 + 1 + 1 + 10);

    ObjectGuid playerGuid = player->GetGUID();
    ObjectGuid unkGuid = 0;

    data.WriteBitSeq<5, 7, 3, 0, 4, 1, 6, 2>(playerGuid);
    data.FlushBits();

    data.WriteByteSeq<7, 4, 3>(playerGuid);

    data << uint8(0);
    data << uint32(0);
    data << uint8(player->getRace());
    data << uint8(!player->getGender());
    data << uint8(player->getLevel());
    data << uint8(player->getClass());
    data << uint32(realmID);

    data.WriteByteSeq<1, 5, 0, 6, 2>(playerGuid);

    data.WriteBitSeq<6>(playerGuid);
    data.WriteBitSeq<7>(unkGuid);
    data.WriteBits(player->GetName().length(), 6);
    data.WriteBitSeq<1, 7, 2>(playerGuid);
    data.WriteBitSeq<4>(unkGuid);
    data.WriteBitSeq<4, 0>(playerGuid);
    data.WriteBitSeq<1>(unkGuid);

    auto const &declinedNames = player->GetDeclinedNames();

    if (declinedNames)
    {
        for (auto const &name : declinedNames->name)
            data.WriteBits(name.size(), 7);
    }
    else
    {
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data.WriteBits(0, 7);
    }

    data.WriteBitSeq<3>(unkGuid);
    data.WriteBitSeq<3>(playerGuid);
    data.WriteBitSeq<5, 0>(unkGuid);
    data.WriteBitSeq<5>(playerGuid);
    data.WriteBit(false); // unk
    data.WriteBitSeq<2, 6>(unkGuid);
    data.FlushBits();

    data.WriteString(player->GetName());
    data.WriteByteSeq<4>(playerGuid);
    data.WriteByteSeq<3>(unkGuid);
    data.WriteByteSeq<6>(playerGuid);
    data.WriteByteSeq<2, 4>(unkGuid);
    data.WriteByteSeq<5, 1, 7>(playerGuid);

    if (declinedNames)
    {
        for (auto const &name : declinedNames->name)
            data.WriteString(name);
    }

    data.WriteByteSeq<3>(playerGuid);
    data.WriteByteSeq<7, 1, 6>(unkGuid);
    data.WriteByteSeq<0>(playerGuid);
    data.WriteByteSeq<0>(unkGuid);
    data.WriteByteSeq<2>(playerGuid);
    data.WriteByteSeq<5>(unkGuid);

    SendPacket(&data);
}

void WorldSession::SendNameQueryOpcode(uint64 guid)
{
    uint32 statementId = sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED)
            ? CHAR_SEL_NAME_QUERY_DECLINED
            : CHAR_SEL_NAME_QUERY_SIMPLE;

    auto stmt = CharacterDatabase.GetPreparedStatement(statementId);
    stmt->setUInt32(0, GUID_LOPART(guid));

    PreparedQueryResultFuture result = CharacterDatabase.AsyncQuery(stmt);

    ACE_Guard<ACE_Thread_Mutex> guard(_nameQueryCallbacksLock);
    _nameQueryCallbacks.emplace_back(guid, result);
}

void WorldSession::SendNameQueryOpcodeCallBack(uint64 guid, PreparedQueryResult result)
{
    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, 8 + 1 + 1 + 1 + 1 + 1 + 10);

    ObjectGuid playerGuid = guid;
    ObjectGuid unkGuid = 0;

    data.WriteBitSeq<5, 7, 3, 0, 4, 1, 6, 2>(playerGuid);
    data.FlushBits();

    data.WriteByteSeq<7, 4, 3>(playerGuid);

    if (!result)
    {
        data << uint8(1);
        data.WriteByteSeq<1, 5, 0, 6, 2>(playerGuid);
        SendPacket(&data);
        return;
    }

    auto const fields = result->Fetch();

    auto const playerName = fields[0].GetString();
    auto const race = fields[1].GetUInt8();
    auto const gender = fields[2].GetUInt8();
    auto const playerClass = fields[3].GetUInt8();
    auto const level = fields[4].GetUInt8();
    auto const hasDeclinedName = sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED) && *fields[5].GetCString() != '\0';

    data << uint8(0);
    data << uint32(0);
    data << uint8(GetPlayer() ? GetPlayer()->getRace() : race);
    data << gender;
    data << level;
    data << playerClass;
    data << uint32(realmID);

    data.WriteByteSeq<1, 5, 0, 6, 2>(playerGuid);

    data.WriteBitSeq<6>(playerGuid);
    data.WriteBitSeq<7>(unkGuid);
    data.WriteBits(playerName.size(), 6);
    data.WriteBitSeq<1, 7, 2>(playerGuid);
    data.WriteBitSeq<4>(unkGuid);
    data.WriteBitSeq<4, 0>(playerGuid);
    data.WriteBitSeq<1>(unkGuid);

    if (hasDeclinedName)
    {
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data.WriteBits(fields[5 + i].GetString().size(), 7);
    }
    else
    {
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data.WriteBits(0, 7);
    }

    data.WriteBitSeq<3>(unkGuid);
    data.WriteBitSeq<3>(playerGuid);
    data.WriteBitSeq<5, 0>(unkGuid);
    data.WriteBitSeq<5>(playerGuid);
    data.WriteBit(false); // unk
    data.WriteBitSeq<2, 6>(unkGuid);
    data.FlushBits();

    data.WriteString(playerName);
    data.WriteByteSeq<4>(playerGuid);
    data.WriteByteSeq<3>(unkGuid);
    data.WriteByteSeq<6>(playerGuid);
    data.WriteByteSeq<2, 4>(unkGuid);
    data.WriteByteSeq<5, 1, 7>(playerGuid);

    if (hasDeclinedName)
    {
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data.WriteString(fields[5 + i].GetString());
    }

    data.WriteByteSeq<3>(playerGuid);
    data.WriteByteSeq<7, 1, 6>(unkGuid);
    data.WriteByteSeq<0>(playerGuid);
    data.WriteByteSeq<0>(unkGuid);
    data.WriteByteSeq<2>(playerGuid);
    data.WriteByteSeq<5>(unkGuid);

    SendPacket(&data);
}

void WorldSession::HandleNameQueryOpcode(WorldPacket& recvData)
{
    /*
        CMSG_NAME_QUERY 5.4.0 17371
        7D -> 0 1 1 1 1 1 0 1
        80 -> 1 0 0 0 0 0 0 0
        10 // guid part
        41 // guid part
        05 // guid part
        81 // guid part
        1D // guid part
        00 // guid part
        27000103 // unk bit 1, realm id / flags ?
    */

    ObjectGuid guid;

    recvData.ReadBitSeq<5, 7, 0, 1>(guid);
    bool hasUnkBit1 = recvData.ReadBit();
    recvData.ReadBitSeq<6>(guid);
    bool hasUnkBit2 = recvData.ReadBit();
    recvData.ReadBitSeq<3, 2, 4>(guid);
    recvData.ReadByteSeq<0, 1, 3, 4, 6, 5, 2, 7>(guid);

    if (hasUnkBit2)
    {
        uint32 unk = recvData.read<uint32>();
        TC_LOG_DEBUG("network", "CMSG_NAME_QUERY uint32 unk : %u\r\n", unk);
    }

    if (hasUnkBit1)
    {
        uint32 unk1 = recvData.read<uint32>();
        TC_LOG_DEBUG("network", "CMSG_NAME_QUERY uint32 unk1 (realm flags / id ?) : %u\r\n", unk1);
    }

    if (Player const * const player = ObjectAccessor::FindPlayer(guid))
        SendNameQueryOpcode(player);
    else
        SendNameQueryOpcode(guid);
}

void WorldSession::HandleQueryTimeOpcode(WorldPacket& /*recvData*/)
{
    SendQueryTimeResponse();
}

void WorldSession::SendQueryTimeResponse()
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4+4);
    data << uint32(sWorld->GetNextDailyQuestsResetTime() - time(NULL));
    data << uint32(time(NULL));
    SendPacket(&data);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleCreatureQueryOpcode(WorldPacket& recvData)
{
    uint32 entry;
    recvData >> entry;

    if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(entry))
    {
        std::string Name = ci->Name;
        std::string SubName = ci->SubName;

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (CreatureLocale const* cl = sObjectMgr->GetCreatureLocale(entry))
            {
                ObjectMgr::GetLocaleString(cl->Name, loc_idx, Name);
                ObjectMgr::GetLocaleString(cl->SubName, loc_idx, SubName);
            }
        }
        TC_LOG_DEBUG("network", "WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u.", ci->Name.c_str(), entry);

        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE);

        data << uint32(entry);                                                          // creature entry
        data.WriteBit(1);                                                               // has valid data
        data.WriteBits(Name.size() ? Name.size() + 1 : 0, 11);                          // Male

        for (int i = 0; i < 7; i++)
            data.WriteBits(0, 11);                          // Female and other Names - Never send it

        uint8 itemCount = 0;
        for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
            if (ci->questItems[i])
                ++itemCount;                                // itemId[6], quest drop

        data.WriteBits(itemCount, 22);
        data.WriteBits(ci->IconName.size() ? ci->IconName.size() + 1 : 0, 6);
        data.WriteBits(SubName.size() ? SubName.size() + 1 : 0, 11);
        data.WriteBits(0, 11);                              // Unk 505 string
        data.WriteBit(ci->RacialLeader);                    // isRacialLeader

        data << uint32(ci->type);                           // CreatureType.dbc
        data << uint32(ci->KillCredit[1]);                  // new in 3.1, kill credit
        data << uint32(ci->Modelid4);                       // Modelid4
        data << uint32(ci->Modelid3);                       // Modelid3

        for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS && itemCount > 0; ++i)
        {
            if (ci->questItems[i])
            {
                data << uint32(ci->questItems[i]);
                --itemCount;
            }
        }

        data << uint32(ci->expansionUnknown);               // unknown meaning

        if (ci->Name.size())
            data << Name;                                   // Name

        data << float(ci->ModMana);                         // dmg/mana modifier
        data << uint32(ci->Modelid1);                       // Modelid1

        if (ci->IconName.size())
            data << ci->IconName;                           // Icon Name

        data << uint32(ci->KillCredit[0]);                  // new in 3.1, kill credit
        data << uint32(ci->Modelid2);                       // Modelid2

        if (SubName.size())
            data << SubName;                                // Sub Name

        data << uint32(ci->type_flags);                     // flags
        data << uint32(ci->type_flags2);                    // unknown meaning
        data << float(ci->ModHealth);                       // dmg/hp modifier
        data << uint32(ci->family);                         // CreatureFamily.dbc
        data << uint32(ci->rank);                           // Creature Rank (elite, boss, etc)
        data << uint32(ci->movementId);                     // CreatureMovementInfo.dbc

        SendPacket(&data);
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
    else
    {
        TC_LOG_DEBUG("network", "WORLD: CMSG_CREATURE_QUERY - NO CREATURE INFO! (ENTRY: %u)", entry);
        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 4);
        data << uint32(entry | 0x80000000);
        data.WriteBit(0); // has no valid data
        SendPacket(&data);
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode(WorldPacket& recvData)
{
    uint32 entry;
    recvData >> entry;
    ObjectGuid guid;

    recvData.ReadBitSeq<2, 4, 3, 7, 0, 6, 1, 5>(guid);
    recvData.ReadByteSeq<1, 7, 2, 3, 6, 5, 4, 0>(guid);

    if (const GameObjectTemplate* info = sObjectMgr->GetGameObjectTemplate(entry))
    {
        std::string Name;
        std::string IconName;
        std::string CastBarCaption;

        Name = info->name;
        IconName = info->IconName;
        CastBarCaption = info->castBarCaption;

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (GameObjectLocale const* gl = sObjectMgr->GetGameObjectLocale(entry))
            {
                ObjectMgr::GetLocaleString(gl->Name, loc_idx, Name);
                ObjectMgr::GetLocaleString(gl->CastBarCaption, loc_idx, CastBarCaption);
            }
        }

        TC_LOG_DEBUG("network", "WORLD: CMSG_GAMEOBJECT_QUERY '%s' - Entry: %u. ", info->name.c_str(), entry);
        WorldPacket data (SMSG_GAMEOBJECT_QUERY_RESPONSE);
        ByteBuffer byteBuffer;

        data.WriteBit(1);                                               // Always 1, from sniffs
        data.FlushBits();

        data << uint32(entry);

        {
            byteBuffer << uint32(info->type);
            byteBuffer << uint32(info->displayId);
            byteBuffer << Name;
            byteBuffer << uint32(0);                                    // unk
            byteBuffer << CastBarCaption;                               // 2.0.3, string. Text will appear in Cast Bar when using GO (ex: "Collecting")
            byteBuffer << IconName;                                     // 2.0.3, string. Icon name to use instead of default icon for go's (ex: "Attack" makes sword)

            for (int i = 0; i < 32; i++)
                byteBuffer << uint32(info->raw.data[i]);

            byteBuffer << float(info->size);                            // go size

            uint8 questItemCount = 0;
            for (uint32 i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; ++i)
                if (info->questItems[i])
                    questItemCount++;

            byteBuffer << uint8(questItemCount);

            for (int i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS && questItemCount > 0; i++)
            {
                if (info->questItems[i])
                {
                    byteBuffer << uint32(info->questItems[i]);          // itemId[6], quest drop
                    questItemCount--;
                }
            }

            byteBuffer << uint32(info->unkInt32);                       // 4.x, unknown
        }

        data << uint32(byteBuffer.size());
        data.append(byteBuffer);

        SendPacket(&data);
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
    else
    {
        TC_LOG_DEBUG("network", "WORLD: CMSG_GAMEOBJECT_QUERY - Missing gameobject info for (GUID: %u, ENTRY: %u)",
            GUID_LOPART(guid), entry);
        WorldPacket data (SMSG_GAMEOBJECT_QUERY_RESPONSE, 4);
        data << uint32(entry | 0x80000000);
        SendPacket(&data);
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
}

void WorldSession::HandleCorpseQueryOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_CORPSE_QUERY");

    Corpse* corpse = GetPlayer()->GetCorpse();

    if (!corpse)
    {
        WorldPacket data(SMSG_CORPSE_QUERY);

        data << uint32(0);
        data << float(0);
        data << uint32(0);
        data << float(0);
        data << float(0);

        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.FlushBits();

        SendPacket(&data);
        return;
    }

    uint32 mapid = corpse->GetMapId();
    float x = corpse->GetPositionX();
    float y = corpse->GetPositionY();
    float z = corpse->GetPositionZ();
    uint32 corpsemapid = mapid;

    // if corpse at different map
    if (mapid != _player->GetMapId())
    {
        // search entrance map for proper show entrance
        if (MapEntry const* corpseMapEntry = sMapStore.LookupEntry(mapid))
        {
            if (corpseMapEntry->IsDungeon() && corpseMapEntry->entrance_map >= 0)
            {
                // if corpse map have entrance
                if (Map const* entranceMap = sMapMgr->CreateBaseMap(corpseMapEntry->entrance_map))
                {
                    mapid = corpseMapEntry->entrance_map;
                    x = corpseMapEntry->entrance_x;
                    y = corpseMapEntry->entrance_y;
                    z = entranceMap->GetHeight(GetPlayer()->GetPhaseMask(), x, y, MAX_HEIGHT);
                }
            }
        }
    }
    ObjectGuid guid = corpse->GetGUID();

    WorldPacket data(SMSG_CORPSE_QUERY);

    data << uint32(mapid);
    data << float(x);
    data << uint32(corpsemapid);
    data << float(y);
    data << float(z);

    data.WriteBitSeq<5, 3, 4, 2, 6, 0, 7, 1>(guid);
    data.WriteBit(1);
    data.FlushBits();

    data.WriteByteSeq<4, 5, 2, 0, 1, 6, 7, 3>(guid);

    SendPacket(&data);
}

void WorldSession::HandleCemeteryListOpcode(WorldPacket& /*recvData*/)
{
    GetPlayer()->SendCemeteryList(false);
}

void WorldSession::HandleForcedReactionsOpcode(WorldPacket& /*recvData*/)
{
    GetPlayer()->GetReputationMgr().SendForceReactions();
}

void WorldSession::HandleNpcTextQueryOpcode(WorldPacket& recvData)
{
    uint32 textID;
    ObjectGuid guid;

    recvData >> textID;
    TC_LOG_DEBUG("network", "WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID);

    recvData.ReadBitSeq<7, 3, 1, 5, 6, 4, 0, 2>(guid);
    recvData.ReadByteSeq<1, 5, 2, 7, 3, 6, 4, 0>(guid);

    GetPlayer()->SetSelection(guid);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 100);          // guess size
    data << textID;
    data << uint32(0x40);       // size of packet
    data << uint32(0x3F800000); // unk flags 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << textID;
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data << uint32(0x00); // unk 5.0.5
    data.WriteBit(1);     // unk bit (true on retail sniff)
    data.FlushBits();

    /*if (!pGossip)
    {
        for (uint32 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << float(0);
            data << "Greetings $N";
            data << "Greetings $N";
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
        }
    }
    else
    {
        std::string Text_0[MAX_LOCALES], Text_1[MAX_LOCALES];
        for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            Text_0[i]=pGossip->Options[i].Text_0;
            Text_1[i]=pGossip->Options[i].Text_1;
        }

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (NpcTextLocale const* nl = sObjectMgr->GetNpcTextLocale(textID))
            {
                for (int i = 0; i < MAX_LOCALES; ++i)
                {
                    ObjectMgr::GetLocaleString(nl->Text_0[i], loc_idx, Text_0[i]);
                    ObjectMgr::GetLocaleString(nl->Text_1[i], loc_idx, Text_1[i]);
                }
            }
        }

        for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << pGossip->Options[i].Probability;

            if (Text_0[i].empty())
                data << Text_1[i];
            else
                data << Text_0[i];

            if (Text_1[i].empty())
                data << Text_0[i];
            else
                data << Text_1[i];

            data << pGossip->Options[i].Language;

            for (int j = 0; j < MAX_GOSSIP_TEXT_EMOTES; ++j)
            {
                data << pGossip->Options[i].Emotes[j]._Delay;
                data << pGossip->Options[i].Emotes[j]._Emote;
            }
        }
    }*/

    SendPacket(&data);

    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_NPC_TEXT_UPDATE");
}

#define DEFAULT_GREETINGS_GOSSIP      68

void WorldSession::SendBroadcastTextDb2Reply(uint32 entry)
{
    ByteBuffer buff;
    WorldPacket data(SMSG_DB_REPLY);

    GossipText const* pGossip = sObjectMgr->GetGossipText(entry);
    if (!pGossip)
        pGossip = sObjectMgr->GetGossipText(DEFAULT_GREETINGS_GOSSIP);

    std::string text = "Greetings, $n";
    std::string text1 = pGossip ? pGossip->Options[0].Text_0 : text;
    std::string text2 = pGossip ? pGossip->Options[0].Text_1 : text;

    uint16 size1 = text1.size();
    uint16 size2 = text2.size();

    int loc_idx = GetSessionDbLocaleIndex();
    if (loc_idx >= 0 && loc_idx <= MAX_LOCALES)
    {
        NpcTextLocale const* nl = sObjectMgr->GetNpcTextLocale(entry);
        if (!nl)
            nl = sObjectMgr->GetNpcTextLocale(DEFAULT_GREETINGS_GOSSIP);

        ObjectMgr::GetLocaleString(nl->Text_0[loc_idx], loc_idx, text1);
        ObjectMgr::GetLocaleString(nl->Text_1[loc_idx], loc_idx, text2);
        size1 = text1.size();
        size2 = text2.size();
    }

    buff << uint32(entry);
    buff << uint32(0); // unk
    buff << uint16(size1);
    if (size1)
        buff << std::string(text1);
    buff << uint16(size2);
    if (size2)
        buff << std::string(text2);

    buff << uint32(0);
    buff << uint32(0);
    buff << uint32(0);

    buff << uint32(0);
    buff << uint32(0);
    buff << uint32(0);

    buff << uint32(0); // sound Id
    buff << uint32(pGossip ? pGossip->Options[0].Emotes[0]._Delay : 0); // Delay
    buff << uint32(pGossip ? pGossip->Options[0].Emotes[0]._Emote : 0); // Emote

    data << uint32(buff.size());
    data.append(buff);

    data << uint32(DB2_REPLY_BROADCAST_TEXT);
    data << uint32(sObjectMgr->GetHotfixDate(entry, DB2_REPLY_BROADCAST_TEXT));
    data << uint32(entry);

    SendPacket(&data);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandlePageTextQueryOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_PAGE_TEXT_QUERY");

    ObjectGuid objectGuid;
    uint32 pageID;
    recvData >> pageID;

    recvData.ReadBitSeq<0, 7, 5, 2, 1, 3, 4, 6>(objectGuid);
    recvData.ReadByteSeq<7, 4, 6, 5, 2, 3, 0, 1>(objectGuid);

    if (IS_UNIT_GUID(objectGuid))
    {
        if (Unit* unit = Unit::GetUnit(*(GetPlayer()), objectGuid))
            TC_LOG_INFO("server.loading", "Received CMSG_PAGE_TEXT_QUERY. Unit Entry: %u", unit->GetEntry());
    }
    else if (IS_GAMEOBJECT_GUID(objectGuid))
    {
        if (GameObject* go = GetPlayer()->GetMap()->GetGameObject(objectGuid))
            TC_LOG_INFO("server.loading", "Received CMSG_PAGE_TEXT_QUERY. Gameobject Entry: %u", go->GetEntry());
    }

    while (pageID)
    {
        PageText const* pageText = sObjectMgr->GetPageText(pageID);

        WorldPacket data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 50);

        data.WriteBit(pageText != NULL);

        if (pageText)
        {
            std::string Text = pageText->Text;

            int loc_idx = GetSessionDbLocaleIndex();
            if (loc_idx >= 0)
                if (PageTextLocale const* player = sObjectMgr->GetPageTextLocale(pageID))
                    ObjectMgr::GetLocaleString(player->Text, loc_idx, Text);

            data.WriteBits(Text.size(), 12);

            data.FlushBits();
            if (Text.size())
                data.append(Text.c_str(), Text.size());

            data << uint32(pageID);
            data << uint32(pageText->NextPage);
        }

        data << uint32(pageID);

        pageID = pageText ? pageText->NextPage : 0u;

        SendPacket(&data);

        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_PAGE_TEXT_QUERY_RESPONSE");
    }
}

void WorldSession::HandleCorpseMapPositionQuery(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Recv CMSG_CORPSE_MAP_POSITION_QUERY");

    // Read guid, useless
    recvData.rfinish();

    WorldPacket data(SMSG_CORPSE_MAP_POSITION_QUERY_RESPONSE, 4+4+4+4);
    data << float(0);
    data << float(0);
    data << float(0);
    data << float(0);
    SendPacket(&data);
}

void WorldSession::HandleQuestPOIQuery(WorldPacket& recvData)
{
    uint32 count;
    count = recvData.ReadBits(22);

    if (count > MAX_QUEST_LOG_SIZE)
    {
        recvData.rfinish();
        return;
    }

    std::list<uint32> questList;
    for (uint32 i = 0; i < count; ++i)
        questList.push_back(recvData.read<uint32>());


    WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4+(4+4)*count);
    data << uint32(count); // count
    data.WriteBits(count, 20);

    // Write bits
    for (auto itr : questList)
    {
        uint32 questId = itr;
        bool questOk = false;
        uint16 questSlot = _player->FindQuestSlot(questId);

        if (questSlot != MAX_QUEST_LOG_SIZE)
            questOk =_player->GetQuestSlotQuestId(questSlot) == questId;

        if (questOk)
        {
            QuestPOIVector const* POI = sObjectMgr->GetQuestPOIVector(questId);

            if (POI)
            {
                data.WriteBits(POI->size(), 18);
                for (QuestPOIVector::const_iterator itr = POI->begin(); itr != POI->end(); ++itr)
                    data.WriteBits(itr->points.size(), 21);
            }
            else
                data.WriteBits(0, 18);
        }
        else
            data.WriteBits(0, 18);
    }

    // Write bytes
    for (auto itr : questList)
    {
        uint32 questId = itr;
        bool questOk = false;
        uint16 questSlot = _player->FindQuestSlot(questId);

        if (questSlot != MAX_QUEST_LOG_SIZE)
            questOk =_player->GetQuestSlotQuestId(questSlot) == questId;

        if (questOk)
        {
            QuestPOIVector const* POI = sObjectMgr->GetQuestPOIVector(questId);

            if (POI)
            {
                for (QuestPOIVector::const_iterator itr = POI->begin(); itr != POI->end(); ++itr)
                {
                    data << uint32(itr->Unk4);
                    data << uint32(itr->Unk3);
                    data << uint32(itr->Unk2);
                    data << uint32(itr->points.size());
                    data << uint32(itr->AreaId);

                    for (std::vector<QuestPOIPoint>::const_iterator itr2 = itr->points.begin(); itr2 != itr->points.end(); ++itr2)
                    {
                        data << int32(itr2->y); // POI point y
                        data << int32(itr2->x); // POI point x
                    }

                    data << int32(itr->ObjectiveIndex);
                    data << uint32(0);
                    data << uint32(itr->MapId);
                    data << uint32(0);
                    data << uint32(0);
                    data << uint32(itr->Id);
                }
                data << uint32(questId);
                data << uint32(POI->size());
            }
            else
            {
                data << uint32(questId); // quest ID
                data << uint32(0); // POI count
            }
        }
        else
        {
                data << uint32(questId); // quest ID
                data << uint32(0); // POI count
        }
    }

   /* for (uint32 i = 0; i < count; ++i)
    {
        uint32 questId;

        bool questOk = false;

        uint16 questSlot = _player->FindQuestSlot(questId);

        if (questSlot != MAX_QUEST_LOG_SIZE)
            questOk =_player->GetQuestSlotQuestId(questSlot) == questId;

        if (questOk)
        {
            QuestPOIVector const* POI = sObjectMgr->GetQuestPOIVector(questId);

            if (POI)
            {
                data << uint32(questId); // quest ID
                data << uint32(POI->size()); // POI count

                for (QuestPOIVector::const_iterator itr = POI->begin(); itr != POI->end(); ++itr)
                {
                    data << uint32(itr->Id);                // POI index
                    data << int32(itr->ObjectiveIndex);     // objective index
                    data << uint32(0);
                    data << uint32(itr->MapId);             // mapid
                    data << uint32(itr->AreaId);            // areaid
                    data << uint32(itr->Unk2);              // unknown
                    data << uint32(itr->Unk3);              // unknown
                    data << uint32(itr->Unk4);              // unknown
                    data << uint32(0);
                    data << uint32(0);
                    data << uint32(itr->points.size());     // POI points count

                    for (std::vector<QuestPOIPoint>::const_iterator itr2 = itr->points.begin(); itr2 != itr->points.end(); ++itr2)
                    {
                        data << int32(itr2->x); // POI point x
                        data << int32(itr2->y); // POI point y
                    }
                }
            }
            else
            {
                data << uint32(questId); // quest ID
                data << uint32(0); // POI count
            }
        }
        else
        {
            data << uint32(questId); // quest ID
            data << uint32(0); // POI count
        }
    }*/

    SendPacket(&data);
}
