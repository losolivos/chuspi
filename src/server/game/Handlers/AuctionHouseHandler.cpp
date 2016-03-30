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

#include "ObjectMgr.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include "AuctionHouseMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "UpdateMask.h"
#include "Util.h"
#include "AccountMgr.h"

#include <unordered_set>

//void called when player click on auctioneer npc
void WorldSession::HandleAuctionHelloOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<6, 5, 7, 2, 4, 0, 1, 3>(guid);
    recvData.ReadByteSeq<1, 5, 0, 6, 4, 2, 3, 7>(guid);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleAuctionHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendAuctionHello((ObjectGuid)guid, unit);
}

//this void causes that auction window is opened
void WorldSession::SendAuctionHello(ObjectGuid guid, Creature* unit)
{
    if (GetPlayer()->getLevel() < sWorld->getIntConfig(CONFIG_AUCTION_LEVEL_REQ))
    {
        SendNotification(GetTrinityString(LANG_AUCTION_REQ), sWorld->getIntConfig(CONFIG_AUCTION_LEVEL_REQ));
        return;
    }

    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntry(unit->getFaction());
    if (!ahEntry)
        return;

    WorldPacket data(SMSG_AUCTION_HELLO_RESPONSE, 12);
    data.WriteBitSeq<6, 1, 4, 0, 2>(guid);
    data.WriteBit(1); // 3.3.3: 1 - AH enabled, 0 - AH disabled
    data.WriteBitSeq<3, 5, 7>(guid);
    data.FlushBits();

    data.WriteByteSeq<2, 0, 5, 3, 6, 1, 7>(guid);
    data << uint32(ahEntry->houseId); // dword18
    data.WriteByteSeq<4>(guid);
    SendPacket(&data);
}

//call this method when player bids, creates, or deletes auction
void WorldSession::SendAuctionCommandResult(AuctionEntry* auction, uint32 action, uint32 errorCode, uint32 bidError)
{
    ObjectGuid guid = 0;

    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT);
    data << uint32(action);
    data << uint32(errorCode);
    data << uint32(auction ? auction->Id : 0);
    data << uint32(bidError);

    data.WriteBit(!(action == AUCTION_PLACE_BID || action == ERR_AUCTION_HIGHER_BID));
    data.WriteBit(!false);
    data.WriteBitSeq<6, 4, 1, 5, 0, 3, 7, 2>(guid);
    data.WriteBit(!(action == ERR_AUCTION_HIGHER_BID));
    data.FlushBits();

    data.WriteByteSeq<0, 6, 7, 1, 5, 4, 3, 2>(guid);

    if (action == AUCTION_PLACE_BID || action == ERR_AUCTION_HIGHER_BID)
        data << uint64(auction ? (auction->bid ? auction->GetAuctionOutBid() : 0) : 0);

    if (action == ERR_AUCTION_HIGHER_BID)
        data << uint64(auction ? (auction->bid ? auction->GetAuctionOutBid() : 0) : 0);

    SendPacket(&data);
}

//this function sends notification, if bidder is online
void WorldSession::SendAuctionBidderNotification(uint32 location, uint32 auctionId, uint64 bidder, uint64 bidSum, uint64 diff, uint32 itemEntry)
{
    ObjectGuid bidderGuid = bidder;

    // Buyout
    if (!bidSum && !diff)
    {
        WorldPacket data(SMSG_AUCTION_BUYOUT_NOTIFICATION, 5 * 4 + 1 + 8);
        data << uint32(itemEntry);
        data << uint32(location);
        data << uint32(bidSum);
        data << uint32(auctionId);
        data << uint32(diff);
        data.WriteBitSeq<4, 3, 6, 2, 7, 0, 5, 1>(bidderGuid);
        data.WriteByteSeq<3, 1, 4, 6, 5, 7, 0, 2>(bidderGuid);

        SendPacket(&data);
    }
    else
    {
        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 5 * 4 + 2 * 8 + 1 + 8);
        data.WriteBitSeq<3, 6, 2, 0, 4, 1, 5, 7>(bidderGuid);
        data << uint32(location);
        data << uint32(auctionId);
        data.WriteByteSeq<3, 2, 1>(bidderGuid);
        data << uint32(itemEntry);
        data << uint32(0);
        data.WriteByteSeq<4>(bidderGuid);
        data << uint64(diff);
        data.WriteByteSeq<0, 5>(bidderGuid);
        data << uint64(bidSum);
        data << uint32(0);
        data.WriteByteSeq<7, 6>(bidderGuid);

        SendPacket(&data);
    }
}

// this void causes on client to display: "Your auction sold"
void WorldSession::SendAuctionOwnerNotification(AuctionEntry* auction)
{
    WorldPacket data(SMSG_AUCTION_OWNER_BID_NOTIFICATION, 40);

    data << uint64(auction->bid);
    data << uint32(0);
    data << uint32(0);                      // Unk
    data << float(3600);                    // Time in seconds before money received
    data << uint32(auction->Id);
    data << uint32(auction->itemEntry);
    data.WriteBit(true);
    data.FlushBits();

    SendPacket(&data);
}

void WorldSession::SendAuctionRemovedNotification(uint32 /*auctionId*/, uint32 /*itemEntry*/, int32 /*randomPropertyId*/)
{
    // No more sended on 5.4.x official
    /*WorldPacket data(SMSG_AUCTION_REMOVED_NOTIFICATION, (4+4+4));
    data << uint32(auctionId);
    data << uint32(itemEntry);
    data << uint32(randomPropertyId);
    SendPacket(&data);*/
}

//this void creates new auction and adds auction to some auctionhouse
void WorldSession::HandleAuctionSellItem(WorldPacket & recvData)
{
    ObjectGuid auctioneer;
    uint64 bid, buyout;
    uint32 itemsCount, etime;

    recvData >> bid >> buyout >> etime;
    recvData.ReadBitSeq<3, 0, 7, 2, 6, 5>(auctioneer);
    itemsCount = recvData.ReadBits(5);

    if (itemsCount > MAX_AUCTION_ITEMS)
    {
        SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
        return;
    }

    ObjectGuid itemGUIDs[MAX_AUCTION_ITEMS]; // 160 slot = 4x 36 slot bag + backpack 16 slot
    uint32 count[MAX_AUCTION_ITEMS];

    for (uint8 i = 0; i < itemsCount; i++)
    {
        itemGUIDs[i][0] = recvData.ReadBit();
        itemGUIDs[i][2] = recvData.ReadBit();
        itemGUIDs[i][7] = recvData.ReadBit();
        itemGUIDs[i][4] = recvData.ReadBit();
        itemGUIDs[i][6] = recvData.ReadBit();
        itemGUIDs[i][3] = recvData.ReadBit();
        itemGUIDs[i][5] = recvData.ReadBit();
        itemGUIDs[i][1] = recvData.ReadBit();
    }

    recvData.ReadBitSeq<4, 1>(auctioneer);

    {
        // client can not send same GUID multiple times in single packet
        std::unordered_set<uint64> uniqueGuids;

        for (uint8 i = 0; i < itemsCount; i++)
        {
            recvData.ReadByteSeq<6, 7, 4, 1, 0, 2>(itemGUIDs[i]);
            recvData >> count[i];
            recvData.ReadByteSeq<3, 5>(itemGUIDs[i]);

            if (!itemGUIDs[i] || !count[i] || count[i] > 1000 || !uniqueGuids.insert(itemGUIDs[i]).second)
            {
                SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
                recvData.rfinish();
                return;
            }
        }
    }

    recvData.ReadByteSeq<7, 1, 2, 5, 6, 4, 0, 3>(auctioneer);

    if (!bid || !etime)
        return;

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(auctioneer, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleAuctionSellItem - Unit (GUID: %u) not found or you can't interact with him.", GUID_LOPART(auctioneer));
        return;
    }

    AuctionHouseEntry const* auctionHouseEntry = AuctionHouseMgr::GetAuctionHouseEntry(creature->getFaction());
    if (!auctionHouseEntry)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleAuctionSellItem - Unit (GUID: %u) has wrong faction.", GUID_LOPART(auctioneer));
        return;
    }

    etime *= MINUTE;

    switch (etime)
    {
        case 1*MIN_AUCTION_TIME:
        case 2*MIN_AUCTION_TIME:
        case 4*MIN_AUCTION_TIME:
            break;
        default:
            return;
    }

    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    Item* items[MAX_AUCTION_ITEMS];

    uint32 finalCount = 0;
    uint32 itemEntry = 0;

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = _player->GetItemByGuid(itemGUIDs[i]);

        if (!item)
        {
            SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_ITEM_NOT_FOUND);
            return;
        }

        // It is not possible to sell items with different entries.
        if (itemEntry == 0)
            itemEntry = item->GetEntry();

        if (sAuctionMgr->GetAItem(item->GetGUIDLow())
                || !item->CanBeTraded()
                || item->IsNotEmptyBag()
                || item->GetTemplate()->Flags & ITEM_PROTO_FLAG_CONJURED
                || item->GetUInt32Value(ITEM_FIELD_DURATION)
                || item->GetCount() < count[i]
                || itemEntry != item->GetEntry())
        {
            SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
            return;
        }

        items[i] = item;
        finalCount += count[i];
    }

    if (!finalCount)
    {
        SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
        return;
    }

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        if (item->GetMaxStackCount() < finalCount)
        {
            SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
            return;
        }
    }

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        uint32 auctionTime = uint32(etime * sWorld->getRate(RATE_AUCTION_TIME));
        AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMapByFaction(creature->getFaction());

        uint32 deposit = sAuctionMgr->GetAuctionDeposit(auctionHouseEntry, etime, item, finalCount);
        if (!_player->HasEnoughMoney((uint64)deposit))
        {
            SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_NOT_ENOUGHT_MONEY);
            return;
        }

        _player->ModifyMoney(-int64(deposit));

        AuctionEntry* AH = new AuctionEntry;
        AH->Id = sObjectMgr->GenerateAuctionID();

        // Required stack size of auction matches to current item stack size, just move item to auctionhouse
        if (itemsCount == 1 && item->GetCount() == count[i])
        {
            if (GetSecurity() > SEC_PLAYER && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
            {
                sLog->outCommand(GetAccountId(), "GM %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
                                 GetPlayerName().c_str(), GetAccountId(), item->GetTemplate()->Name1.c_str(), item->GetEntry(), item->GetCount());
            }

            AH->itemGUIDLow = item->GetGUIDLow();
            AH->itemEntry = item->GetEntry();
            AH->itemCount = item->GetCount();
            AH->owner = _player->GetGUID();
            AH->startbid = bid;
            AH->bidder = 0;
            AH->bid = 0;
            AH->buyout = buyout;
            AH->expire_time = time(NULL) + auctionTime;
            AH->deposit = deposit;
            AH->auctionHouseEntry = auctionHouseEntry;

            TC_LOG_INFO("network", "CMSG_AUCTION_SELL_ITEM: Player %s (guid %u) is selling item %s entry"
                          " %u (guid %u) with count %u with initial bid " UI64FMTD " with buyout " UI64FMTD
                          " and with time %u (in sec) in auctionhouse %u",
                          _player->GetName().c_str(), _player->GetGUIDLow(), item->GetTemplate()->Name1.c_str(), item->GetEntry(),
                          item->GetGUIDLow(), item->GetCount(), bid, buyout, auctionTime, AH->GetHouseId());

            sAuctionMgr->AddAItem(item);
            auctionHouse->AddAuction(AH);

            _player->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);

            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            item->DeleteFromInventoryDB(trans);
            item->SaveToDB(trans);
            AH->SaveToDB(trans);
            _player->SaveInventoryAndGoldToDB(trans);
            CharacterDatabase.CommitTransaction(trans);

            SendAuctionCommandResult(AH, AUCTION_SELL_ITEM, ERR_AUCTION_OK);

            GetPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION, 1);
            return;
        }
        else // Required stack size of auction does not match to current item stack size, clone item and set correct stack size
        {
            Item* newItem = item->CloneItem(finalCount, _player);
            if (!newItem)
            {
                TC_LOG_ERROR("network", "CMSG_AUCTION_SELL_ITEM: Could not create clone of item %u", item->GetEntry());
                SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
                return;
            }

            if (GetSecurity() > SEC_PLAYER && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
            {
                sLog->outCommand(GetAccountId(), "GM %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
                                 GetPlayerName().c_str(), GetAccountId(), newItem->GetTemplate()->Name1.c_str(), newItem->GetEntry(), newItem->GetCount());
            }

            // New item should not belong to anyone
            newItem->SetOwnerGUID(0);

            AH->itemGUIDLow = newItem->GetGUIDLow();
            AH->itemEntry = newItem->GetEntry();
            AH->itemCount = newItem->GetCount();
            AH->owner = _player->GetGUID();
            AH->startbid = bid;
            AH->bidder = 0;
            AH->bid = 0;
            AH->buyout = buyout;
            AH->expire_time = time(NULL) + auctionTime;
            AH->deposit = deposit;
            AH->auctionHouseEntry = auctionHouseEntry;

            TC_LOG_INFO("network", "CMSG_AUCTION_SELL_ITEM: Player %s (guid %u) is selling item %s entry %u (guid %u)"
                        " with count %u with initial bid " UI64FMTD " with buyout " UI64FMTD " and with time %u (in sec) in auctionhouse %u",
                        _player->GetName().c_str(), _player->GetGUIDLow(), newItem->GetTemplate()->Name1.c_str(), newItem->GetEntry(),
                        newItem->GetGUIDLow(), newItem->GetCount(), bid, buyout, auctionTime, AH->GetHouseId());

            sAuctionMgr->AddAItem(newItem);
            auctionHouse->AddAuction(AH);

            for (uint32 j = 0; j < itemsCount; ++j)
            {
                Item* item2 = items[j];

                // Item stack count equals required count, ready to delete item - cloned item will be used for auction
                if (item2->GetCount() == count[j])
                {
                    _player->MoveItemFromInventory(item2->GetBagSlot(), item2->GetSlot(), true);

                    SQLTransaction trans = CharacterDatabase.BeginTransaction();
                    item2->DeleteFromInventoryDB(trans);
                    item2->DeleteFromDB(trans);
                    CharacterDatabase.CommitTransaction(trans);
                }
                else // Item stack count is bigger than required count, update item stack count and save to database - cloned item will be used for auction
                {
                    item2->SetCount(item2->GetCount() - count[j]);
                    item2->SetState(ITEM_CHANGED, _player);
                    _player->ItemRemovedQuestCheck(item2->GetEntry(), count[j]);
                    item2->SendUpdateToPlayer(_player);

                    SQLTransaction trans = CharacterDatabase.BeginTransaction();
                    item2->SaveToDB(trans);
                    CharacterDatabase.CommitTransaction(trans);
                }
            }

            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            newItem->SaveToDB(trans);
            AH->SaveToDB(trans);
            _player->SaveInventoryAndGoldToDB(trans);
            CharacterDatabase.CommitTransaction(trans);

            SendAuctionCommandResult(AH, AUCTION_SELL_ITEM, ERR_AUCTION_OK);

            GetPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION, 1);
            return;
        }
    }
}

// this function is called when client bids or buys out auction
void WorldSession::HandleAuctionPlaceBid(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_AUCTION_PLACE_BID");

    ObjectGuid auctioneer;
    uint32 auctionId;
    uint64 price;

    recvData >> auctionId;
    recvData >> price;

    recvData.ReadBitSeq<6, 7, 3, 5, 4, 1, 0, 2>(auctioneer);
    recvData.ReadByteSeq<2, 5, 1, 3, 4, 7, 0, 6>(auctioneer);

    if (!auctionId || !price)
        return;                                             // check for cheaters

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(auctioneer, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleAuctionPlaceBid - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(auctioneer)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMapByFaction(creature->getFaction());

    AuctionEntry* auction = auctionHouse->GetAuction(auctionId);
    Player* player = GetPlayer();

    if (!auction || auction->owner == player->GetGUID())
    {
        //you cannot bid your own auction:
        SendAuctionCommandResult(NULL, AUCTION_PLACE_BID, ERR_AUCTION_BID_OWN);
        return;
    }

    // impossible have online own another character (use this for speedup check in case online owner)
    /*Player* auction_owner = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(auction->owner, 0, HIGHGUID_PLAYER));
    if (!auction_owner && sObjectMgr->GetPlayerAccountIdByGUID(MAKE_NEW_GUID(auction->owner, 0, HIGHGUID_PLAYER)) == player->GetSession()->GetAccountId())
    {
        //you cannot bid your another character auction:
        SendAuctionCommandResult(NULL, AUCTION_PLACE_BID, ERR_AUCTION_BID_OWN);
        return;
    }*/

    // cheating
    if (price <= auction->bid || price < auction->startbid)
        return;

    // price too low for next bid if not buyout
    if ((price < auction->buyout || auction->buyout == 0)
            && price < auction->bid + auction->GetAuctionOutBid())
    {
        // client already test it but just in case ...
        SendAuctionCommandResult(auction, AUCTION_PLACE_BID, ERR_AUCTION_HIGHER_BID);
        return;
    }

    // In case of buyout client sends full buyout price, even when previous bid
    // was done by the same player
    uint64 const moneyReq = (player->GetGUID() == auction->bidder && price == auction->buyout)
            ? price - auction->bid
            : price;

    if (!player->HasEnoughMoney(moneyReq))
    {
        // client already test it but just in case ...
        SendAuctionCommandResult(auction, AUCTION_PLACE_BID, ERR_AUCTION_NOT_ENOUGHT_MONEY);
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    if (price < auction->buyout || auction->buyout == 0)
    {
        if (auction->bidder > 0)
        {
            if (auction->bidder == player->GetGUID())
                player->ModifyMoney(-int64(price - auction->bid));
            else
            {
                // mail to last bidder and return money
                sAuctionMgr->SendAuctionOutbiddedMail(auction, price, player->GetGUID(), trans);
                player->ModifyMoney(-int64(price));
            }
        }
        else
            player->ModifyMoney(-int64(price));

        auction->bidder = player->GetGUID();
        auction->bid = price;
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID, price);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_AUCTION_BID);
        stmt->setUInt64(0, auction->bidder);
        stmt->setUInt64(1, auction->bid);
        stmt->setUInt32(2, auction->Id);
        trans->Append(stmt);

        SendAuctionCommandResult(auction, AUCTION_PLACE_BID, ERR_AUCTION_OK);
    }
    else
    {
        //buyout:
        if (player->GetGUID() == auction->bidder)
            player->ModifyMoney(-int64(auction->buyout - auction->bid));
        else
        {
            player->ModifyMoney(-int64(auction->buyout));
            if (auction->bidder)                          //buyout for bidded auction ..
                sAuctionMgr->SendAuctionOutbiddedMail(auction, auction->buyout, player->GetGUID(), trans);
        }
        auction->bidder = player->GetGUID();
        auction->bid = auction->buyout;
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID, auction->buyout);

        //- Mails must be under transaction control too to prevent data loss
        sAuctionMgr->SendAuctionSalePendingMail(auction, trans);
        sAuctionMgr->SendAuctionSuccessfulMail(auction, trans);
        sAuctionMgr->SendAuctionWonMail(auction, trans);

        SendAuctionCommandResult(auction, AUCTION_PLACE_BID, ERR_AUCTION_OK);

        auction->DeleteFromDB(trans);

        uint32 itemEntry = auction->itemEntry;
        sAuctionMgr->RemoveAItem(auction->itemGUIDLow);
        auctionHouse->RemoveAuction(auction, itemEntry);
    }
    player->SaveInventoryAndGoldToDB(trans);
    CharacterDatabase.CommitTransaction(trans);
}

//this void is called when auction_owner cancels his auction
void WorldSession::HandleAuctionRemoveItem(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_AUCTION_REMOVE_ITEM");

    ObjectGuid auctioneer;
    uint32 auctionId;

    recvData >> auctionId;

    recvData.ReadBitSeq<1, 5, 4, 2, 7, 6, 0, 3>(auctioneer);
    recvData.ReadByteSeq<3, 5, 2, 7, 1, 0, 6, 4>(auctioneer);

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(auctioneer, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleAuctionRemoveItem - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(auctioneer)));
        return;
    }

    m_auctionsToRemove.emplace_back(creature->getFaction(), auctionId);
}

//called when player lists his bids
void WorldSession::HandleAuctionListBidderItems(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_AUCTION_LIST_BIDDER_ITEMS");

    ObjectGuid guid;                                        //NPC guid
    uint32 listfrom;                                        //page of auctions
    uint32 outbiddedCount;                                  //count of outbidded auctions
    std::list<uint32> outbiddedAuctionIds;

    recvData >> listfrom;                                   // not used in fact (this list not have page control in client)
    outbiddedCount = recvData.ReadBits(7);

    recvData.ReadBitSeq<6, 7, 4, 3, 2, 5, 0, 1>(guid);
    recvData.ReadByteSeq<1, 4, 5, 6>(guid);

    uint32 t;
    for (uint8 i = 0; i < outbiddedCount; i++)
    {
        recvData >> t;
        outbiddedAuctionIds.push_back(t);
    }

    recvData.ReadByteSeq<0, 2, 3, 7>(guid);

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleAuctionListBidderItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMapByFaction(creature->getFaction());

    WorldPacket data(SMSG_AUCTION_BIDDER_LIST_RESULT, (4+4+4));
    Player* player = GetPlayer();
    data << uint32(0);                                     //add 0 as count
    uint32 count = 0;
    uint32 totalcount = 0;

    for (std::list<uint32>::iterator itr = outbiddedAuctionIds.begin(); itr != outbiddedAuctionIds.end(); ++itr)
    {
        AuctionEntry* auction = auctionHouse->GetAuction(*itr);
        if (auction && auction->BuildAuctionInfo(data))
        {
            ++totalcount;
            ++count;
        }
    }

    auctionHouse->BuildListBidderItems(data, player, count, totalcount);
    data.put<uint32>(0, count);                           // add count to placeholder
    data << totalcount;
    data << uint32(300);                                  //unk 2.3.0
    SendPacket(&data);
}

//this void sends player info about his auctions
void WorldSession::HandleAuctionListOwnerItems(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_AUCTION_LIST_OWNER_ITEMS");

    uint32 listfrom;
    ObjectGuid guid;

    recvData >> listfrom;                                  // not used in fact (this list not have page control in client)

    recvData.ReadBitSeq<5, 7, 3, 0, 2, 6, 1, 4>(guid);
    recvData.ReadByteSeq<4, 3, 5, 7, 6, 0, 2, 1>(guid);

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleAuctionListOwnerItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMapByFaction(creature->getFaction());

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, (4+4+4));
    data << uint32(0);                                     // amount place holder

    uint32 count = 0;
    uint32 totalcount = 0;

    auctionHouse->BuildListOwnerItems(data, _player, count, totalcount);
    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(0);
    SendPacket(&data);
}

//this void is called when player clicks on search button
void WorldSession::HandleAuctionListItems(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_AUCTION_LIST_ITEMS");

    std::string searchedname;
    uint8 levelmin, levelmax, usable, searchedname_len;
    uint32 listfrom, auctionSlotID, auctionMainCategory, auctionSubCategory, quality, dword148;
    ObjectGuid guid;

    recvData.read_skip<uint8>(); // f18_0
    recvData >> quality >> auctionSubCategory >> auctionSlotID >> auctionMainCategory;
    recvData >> levelmax >> levelmin >> listfrom >> dword148;

    // this block looks like it uses some lame byte packing or similar...
    for (uint32 i = 0; i < dword148; i++)
        recvData.read_skip<uint8>();

    searchedname_len = recvData.ReadBits(8);
    recvData.ReadBitSeq<2, 5, 6, 4>(guid);
    usable = recvData.ReadBit();
    recvData.ReadBitSeq<1, 7, 3, 0>(guid);
    recvData.ReadBit(); // byte135

    recvData.ReadByteSeq<4, 6, 2, 7, 1, 3, 5, 0>(guid);
    searchedname = recvData.ReadString(searchedname_len);

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleAuctionListItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMapByFaction(creature->getFaction());

    //TC_LOG_DEBUG("misc", "Auctionhouse search (GUID: %u TypeId: %u)",, list from: %u, searchedname: %s, levelmin: %u, levelmax: %u, auctionSlotID: %u, auctionMainCategory: %u, auctionSubCategory: %u, quality: %u, usable: %u",
    //  GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)), listfrom, searchedname.c_str(), levelmin, levelmax, auctionSlotID, auctionMainCategory, auctionSubCategory, quality, usable);

    WorldPacket data(SMSG_AUCTION_LIST_RESULT, (4+4+4));
    uint32 count = 0;
    uint32 totalcount = 0;
    data << uint32(0);

    // converting string that we try to find to lower case
    std::wstring wsearchedname;
    if (!Utf8toWStr(searchedname, wsearchedname))
        return;

    wstrToLower(wsearchedname);

    auctionHouse->BuildListAuctionItems(data, _player,
        wsearchedname, listfrom, levelmin, levelmax, usable,
        auctionSlotID, auctionMainCategory, auctionSubCategory, quality,
        count, totalcount);

    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(300);                                  //unk 2.3.0
    SendPacket(&data);
}

void WorldSession::HandleAuctionListPendingSales(WorldPacket & /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_AUCTION_LIST_PENDING_SALES");

    uint32 count = 0;

    WorldPacket data(SMSG_AUCTION_LIST_PENDING_SALES, 4);
    data << uint32(count);                                  // count
    /*for (uint32 i = 0; i < count; ++i)
    {
        data << "";                                         // string
        data << "";                                         // string
        data << uint64(0);
        data << uint32(0);
        data << float(0);
    }*/
    SendPacket(&data);
}

void WorldSession::processAuctionsToRemove()
{
    if (m_auctionsToRemove.empty())
        return;

    auto toRemove(std::move(m_auctionsToRemove));
    m_auctionsToRemove.clear();

    auto const player = GetPlayer();

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    auto trans = CharacterDatabase.BeginTransaction();

    for (auto const &pair : toRemove)
    {
        auto const &faction = pair.first;
        auto const &auctionId = pair.second;

        auto const auctionHouse = sAuctionMgr->GetAuctionsMapByFaction(faction);

        auto const auction = auctionHouse->GetAuction(auctionId);
        if (!auction || auction->owner != player->GetGUID())
        {
            SendAuctionCommandResult(auction, AUCTION_CANCEL, ERR_AUCTION_ITEM_NOT_FOUND);
            continue;
        }

        auto const item = sAuctionMgr->GetAItem(auction->itemGUIDLow);
        if (!item)
        {
            TC_LOG_ERROR("network", "Auction id: %u has non existing item (item guid: %u)!", auction->Id, auction->itemGUIDLow);
            SendAuctionCommandResult(auction, AUCTION_CANCEL, ERR_AUCTION_DATABASE_ERROR);
            continue;
        }

        // If we have a bidder, we have to send him the money he paid
        if (auction->bidder != 0)
        {
            auto const auctionCut = auction->GetAuctionCut();

            // player doesn't have enough money, maybe message needed
            if (!player->HasEnoughMoney(auctionCut))
                continue;

            sAuctionMgr->SendAuctionCancelledToBidderMail(auction, trans, item);
            player->ModifyMoney(-int64(auctionCut));
        }

        // item will be deleted or added to received mail list
        MailDraft(auction->BuildAuctionMailSubject(AUCTION_CANCELED),
                  AuctionEntry::BuildAuctionMailBody(0, 0, auction->buyout, auction->deposit, 0))
        .AddItem(item)
        .SendMailTo(trans, player, auction, MAIL_CHECK_MASK_COPIED);

        // inform player that auction is removed
        SendAuctionCommandResult(auction, AUCTION_CANCEL, ERR_AUCTION_OK);

        // Now remove the auction
        auction->DeleteFromDB(trans);

        auto const itemEntry = auction->itemEntry;
        sAuctionMgr->RemoveAItem(auction->itemGUIDLow);
        auctionHouse->RemoveAuction(auction, itemEntry);
    }

    player->SaveInventoryAndGoldToDB(trans);

    CharacterDatabase.CommitTransaction(trans);
}
