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

#include "BlackMarketMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "Item.h"
#include "Language.h"
#include "Log.h"
#include "Util.h"

#include <vector>

// BMAuctionEntry
bool BMAuctionEntry::LoadFromDB(Field const *fields)
{
    id          = fields[0].GetUInt32();
    startTime   = fields[2].GetUInt32();
    bid         = fields[3].GetUInt64();
    bidder      = fields[4].GetUInt32();
    bidderCount = fields[5].GetUInt32();

    bmTemplate = sBlackMarketMgr->GetTemplate(fields[1].GetUInt32());
    if (!bmTemplate)
        return false;

    return true;
}

void BMAuctionEntry::SaveToDB(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_BLACKMARKET_AUCTION);
    stmt->setUInt32(0, id);
    stmt->setUInt32(1, bmTemplate->id);
    stmt->setUInt32(2, startTime);
    stmt->setUInt64(3, bid);
    stmt->setUInt32(4, bidder);
    stmt->setUInt32(5, bidderCount);
    trans->Append(stmt);
}

void BMAuctionEntry::DeleteFromDB(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_BLACKMARKET_AUCTION);
    stmt->setUInt32(0, id);
    trans->Append(stmt);
}

void BMAuctionEntry::UpdateToDB(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_BLACKMARKET_AUCTION);
    stmt->setUInt64(0, bid);
    stmt->setUInt32(1, bidder);
    stmt->setUInt32(2, bidderCount);
    stmt->setUInt32(3, id);
    trans->Append(stmt);
}

uint32 BMAuctionEntry::TimeLeft() const
{
    uint32 endTime = startTime + bmTemplate->duration;
    uint32 curTime = std::time(NULL);
    return (endTime >= curTime) ? endTime - curTime : 0;
}

bool BMAuctionEntry::IsActive() const
{
    return std::time(NULL) > startTime;
}

bool BMAuctionEntry::IsExpired() const
{
    return EndTime() <= std::time(NULL);
}

BlackMarketMgr::BlackMarketMgr() { }

BlackMarketMgr::~BlackMarketMgr()
{
    for (auto &kvPair : m_bmAuctionsMap)
        delete kvPair.second;
}

void BlackMarketMgr::LoadTemplates()
{
    uint32 oldMSTime = getMSTime();

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_BLACKMARKET_TEMPLATE);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 BlackMarket templates. DB table `blackmarket_template` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        auto &bm_template = m_bmTemplatesMap[fields[0].GetUInt32()];

        bm_template.id         = fields[0].GetUInt32();
        bm_template.itemEntry  = fields[1].GetUInt32();
        bm_template.itemCount  = fields[2].GetUInt32();
        bm_template.seller     = fields[3].GetUInt32();
        bm_template.startBid   = fields[4].GetUInt64();
        bm_template.duration   = fields[5].GetUInt32();
        bm_template.chance     = fields[6].GetUInt32();

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u BlackMarket templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BlackMarketMgr::LoadAuctions()
{
    uint32 oldMSTime = getMSTime();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_BLACKMARKET_AUCTIONS);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 BlackMarket Auctions. DB table `blackmarket` is empty.");
        return;
    }

    uint32 count = 0;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    do
    {
        Field* fields = result->Fetch();

        BMAuctionEntry* auction = new BMAuctionEntry();
        if (!auction->LoadFromDB(fields))
        {
            auction->DeleteFromDB(trans);
            delete auction;
            continue;
        }

        m_bmAuctionsMap[auction->id] = auction;

        ++count;
    }
    while (result->NextRow());

    CharacterDatabase.CommitTransaction(trans);

    TC_LOG_INFO("server.loading", ">> Loaded %u BlackMarket Auctions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BlackMarketMgr::Update()
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    // Delete expired auctions
    for (auto itr = m_bmAuctionsMap.begin(); itr != m_bmAuctionsMap.end();)
    {
        BMAuctionEntry* auction = itr->second;
        if (auction->IsExpired())
        {
            if (auction->bidder)
                SendAuctionWon(auction, trans);
            auction->DeleteFromDB(trans);
            itr = m_bmAuctionsMap.erase(itr);
        }
        else
            ++itr;
    }

    // Add New Auctions
    int32 add = sWorld->getIntConfig(CONFIG_BLACKMARKET_MAX_AUCTIONS) - m_bmAuctionsMap.size();
    if (add > 0)
        CreateAuctions(add, trans);

    CharacterDatabase.CommitTransaction(trans);
}

uint32 BlackMarketMgr::GetNewAuctionId()
{
    uint32 newId = 1;
    while (GetAuction(newId))
        ++newId;
    return newId;
}

uint32 BlackMarketMgr::GetAuctionOutBid(uint32 bid)
{
    // The / 10000 and * 10000 prevent sending price with silver or copper
    // Because the client allow only golds for Blackmarket price
    uint32 outbid = CalculatePct((bid / 10000), 5) * 10000;
    return outbid ? outbid : 1;
}

void BlackMarketMgr::CreateAuctions(uint32 number, SQLTransaction& trans)
{
    if (m_bmTemplatesMap.empty())
        return;

    for (uint32 i = 0; i < number; ++i)
    {
        // Select a template
        std::vector<uint32> templateList;
        uint32 rand = urand(1, 100);

        for (auto const &kvPair : m_bmTemplatesMap)
            if (kvPair.second.chance >= rand)
                templateList.push_back(kvPair.first);

        for (auto const &kvPair : m_bmAuctionsMap)
            templateList.erase(std::remove(templateList.begin(), templateList.end(), kvPair.second->bmTemplate->id), templateList.end());

        if (templateList.empty())
            continue;

        BMAuctionTemplate* selTemplate = GetTemplate(templateList[urand(0, (templateList.size() - 1))]);
        if (!selTemplate)
            continue;

        BMAuctionEntry* auction = new BMAuctionEntry;
        auction->id = GetNewAuctionId();
        auction->bid = selTemplate->startBid;
        auction->bidder = 0;
        auction->bidderCount = 0;
        auction->startTime = std::time(NULL)
                + sWorld->getIntConfig(CONFIG_BLACKMARKET_AUCTION_DELAY)
                + urand(0, sWorld->getIntConfig(CONFIG_BLACKMARKET_AUCTION_DELAY_MOD));

        auction->bmTemplate = selTemplate;

        m_bmAuctionsMap[auction->id] = auction;
        auction->SaveToDB(trans);
    }
}

void BlackMarketMgr::BuildBlackMarketAuctionsPacket(WorldPacket& data, uint32 guidLow)
{
    uint32 count = 0;

    data << uint32(time(NULL));

    for (auto const &kvPair : m_bmAuctionsMap)
        if (kvPair.second->IsActive())
            ++count;

    data.WriteBits(count, 18);

    ByteBuffer datas;

    for (auto const &kvPair : m_bmAuctionsMap)
    {
        auto const auction = kvPair.second;
        if (!auction->IsActive())
            continue;

        // Is owner
        data.WriteBit(guidLow == auction->bidder);

        uint64 currentBid = auction->bidder ? auction->bid : 0;
        uint64 nextBidPrice = auction->bidder ? auction->bid + GetAuctionOutBid(auction->bid) : auction->bid;
        uint64 upPrice = auction->bidder ? nextBidPrice - currentBid : 1;

        datas << uint32(auction->bmTemplate->itemEntry);
        datas << uint32(auction->bmTemplate->seller);
        datas << uint32(auction->id);
        datas << uint32(auction->bidderCount);
        datas << uint32(auction->TimeLeft());
        datas << uint64(currentBid);
        datas << uint32(auction->bmTemplate->itemCount);
        datas << uint32(0);                                 // Unk
        datas << uint64(nextBidPrice);
        datas << uint64(upPrice);
    }

    data.FlushBits();

    if (datas.size())
        data.append(datas);

    TC_LOG_DEBUG("network", ">> Sent %u BlackMarket Auctions", count);
}

void BlackMarketMgr::UpdateAuction(BMAuctionEntry* auction, uint64 newPrice, Player* newBidder)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    if (auction->bidder)
        SendAuctionOutbidded(auction, trans);

    auction->bid = newPrice;
    auction->bidder = newBidder->GetGUIDLow();
    ++auction->bidderCount;

    auction->UpdateToDB(trans);

    CharacterDatabase.CommitTransaction(trans);
}

std::string BMAuctionEntry::BuildAuctionMailSubject(BMMailAuctionAnswers response) const
{
    std::ostringstream strm;
    strm << bmTemplate->itemEntry << ":0:" << response << ':' << id << ':' << bmTemplate->itemCount;
    return strm.str();
}

std::string BMAuctionEntry::BuildAuctionMailBody(uint32 lowGuid) const
{
    std::ostringstream strm;
    strm.width(16);
    strm << std::right << std::hex << MAKE_NEW_GUID(lowGuid, 0, HIGHGUID_PLAYER);   // HIGHGUID_PLAYER always present, even for empty guids
    strm << std::dec << ':' << bid << ':' << 0;
    strm << ':' << 0 << ':' << 0;
    return strm.str();
}

void BlackMarketMgr::SendAuctionOutbidded(BMAuctionEntry* auction, SQLTransaction& trans)
{
    Player* bidder = sObjectAccessor->FindPlayer(MAKE_NEW_GUID(auction->bidder, 0, HIGHGUID_PLAYER));
    if (bidder)
    {
        WorldPacket data(SMSG_BLACK_MARKET_OUT_BID, 12);
        data << uint32(auction->bmTemplate->itemEntry);
        data << uint32(1);
        data << uint32(1);
        bidder->GetSession()->SendPacket(&data);
    }

    MailDraft(auction->BuildAuctionMailSubject(BM_AUCTION_OUTBIDDED), auction->BuildAuctionMailBody(auction->bmTemplate->seller))
        .AddMoney(auction->bid)
        .SendMailTo(trans, MailReceiver(bidder, auction->bidder), auction, MAIL_CHECK_MASK_COPIED);
}

void BlackMarketMgr::SendAuctionWon(BMAuctionEntry* auction, SQLTransaction& trans)
{
    Player* bidder = sObjectAccessor->FindPlayer(MAKE_NEW_GUID(auction->bidder, 0, HIGHGUID_PLAYER));
    if (bidder)
    {
        WorldPacket data(SMSG_BLACK_MARKET_WON, 12);
        data << uint32(1);
        data << uint32(1);
        data << uint32(auction->bmTemplate->itemEntry);
        bidder->GetSession()->SendPacket(&data);
    }

    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(auction->bmTemplate->itemEntry);
    if (!itemTemplate)
        return;

    Item* item = Item::CreateItem(auction->bmTemplate->itemEntry, auction->bmTemplate->itemCount);
    if (!item)
        return;

    item->SaveToDB(trans);

    MailDraft(auction->BuildAuctionMailSubject(BM_AUCTION_WON), auction->BuildAuctionMailBody(auction->bidder))
        .AddItem(item)
        .SendMailTo(trans, MailReceiver(bidder, auction->bidder), MailSender(auction), MAIL_CHECK_MASK_COPIED);
}
