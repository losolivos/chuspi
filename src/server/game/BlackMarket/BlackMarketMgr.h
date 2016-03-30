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

#ifndef TRINITY_GAME_BLACK_MARKET_MGR_H
#define TRINITY_GAME_BLACK_MARKET_MGR_H

#include "Define.h"
#include "MySQLPtrTypesFwd.h"

#include <ace/Singleton.h>

#include <map>
#include <string>

class Item;
class Player;
class WorldPacket;

enum BMMailAuctionAnswers
{
    BM_AUCTION_OUTBIDDED           = 0,
    BM_AUCTION_WON                 = 1
};

#define BLACKMARKET_AUCTION_HOUSE 7

struct BMAuctionTemplate
{
    uint32 id;
    uint32 itemEntry;
    uint32 itemCount;
    uint32 seller;
    uint32 duration;
    uint64 startBid;
    uint32 chance;
};

struct BMAuctionEntry
{
    uint32 id;
    uint32 startTime;
    uint64 bid;
    uint32 bidder;
    uint32 bidderCount;
    BMAuctionTemplate const *bmTemplate;

    // helpers
    void DeleteFromDB(SQLTransaction& trans);
    void SaveToDB(SQLTransaction& trans);
    bool LoadFromDB(Field const *fields);
    void UpdateToDB(SQLTransaction& trans);

    uint32 EndTime() const { return startTime + bmTemplate->duration; }
    uint32 TimeLeft() const;
    bool IsActive() const;
    bool IsExpired() const;

    std::string BuildAuctionMailSubject(BMMailAuctionAnswers response) const;
    std::string BuildAuctionMailBody(uint32 lowGuid) const;
};

class BlackMarketMgr
{
    friend class ACE_Singleton<BlackMarketMgr, ACE_Null_Mutex>;

    typedef std::map<uint32, BMAuctionTemplate> BMAuctionTemplateMap;
    typedef std::map<uint32, BMAuctionEntry*> BMAuctionEntryMap;

private:
    BlackMarketMgr();
    ~BlackMarketMgr();

public:

    BMAuctionTemplate * GetTemplate(uint32 id)
    {
        auto itr = m_bmTemplatesMap.find(id);
        return itr != m_bmTemplatesMap.end() ? &itr->second : NULL;
    }

    BMAuctionTemplate const * GetTemplate(uint32 id) const
    {
        auto itr = m_bmTemplatesMap.find(id);
        return itr != m_bmTemplatesMap.end() ? &itr->second : NULL;
    }

    uint32 GetTemplatesCount() const { return m_bmTemplatesMap.size(); }

    BMAuctionEntry * GetAuction(uint32 id)
    {
        auto itr = m_bmAuctionsMap.find(id);
        return itr != m_bmAuctionsMap.end() ? itr->second : NULL;
    }

    BMAuctionEntry const * GetAuction(uint32 id) const
    {
        auto itr = m_bmAuctionsMap.find(id);
        return itr != m_bmAuctionsMap.end() ? itr->second : NULL;
    }

    uint32 GetAuctionCount() const { return m_bmAuctionsMap.size(); }

    // Auction messages
    void SendAuctionWon(BMAuctionEntry* auction, SQLTransaction& trans);
    void SendAuctionOutbidded(BMAuctionEntry* auction, SQLTransaction& trans);

    void LoadTemplates();
    void LoadAuctions();

    uint32 GetNewAuctionId();
    uint32 GetAuctionOutBid(uint32 bid);
    void CreateAuctions(uint32 number, SQLTransaction& trans);
    void UpdateAuction(BMAuctionEntry* auction, uint64 newPrice, Player* newBidder);

    void Update();

    void BuildBlackMarketAuctionsPacket(WorldPacket& data, uint32 guidLow);

private:
    BMAuctionTemplateMap m_bmTemplatesMap;
    BMAuctionEntryMap m_bmAuctionsMap;
};

#define sBlackMarketMgr ACE_Singleton<BlackMarketMgr, ACE_Null_Mutex>::instance()

#endif // TRINITY_GAME_BLACK_MARKET_MGR_H
