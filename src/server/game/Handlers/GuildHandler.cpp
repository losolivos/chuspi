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
#include "World.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "GossipDef.h"
#include "SocialMgr.h"

// Helper for getting guild object of session's player.
// If guild does not exist, sends error (if necessary).
inline Guild* _GetPlayerGuild(WorldSession* session, bool sendError = false)
{
    if (uint32 guildId = session->GetPlayer()->GetGuildId())    // If guild id = 0, player is not in guild
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))   // Find guild by id
            return guild;
    if (sendError)
        Guild::SendCommandResult(session, GUILD_CREATE_S, ERR_GUILD_PLAYER_NOT_IN_GUILD);
    return NULL;
}

void WorldSession::HandleGuildQueryOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_QUERY");

    ObjectGuid guildGuid;
    ObjectGuid playerGuid;

    recvPacket.ReadBitSeq<2>(playerGuid);
    recvPacket.ReadBitSeq<7>(guildGuid);
    recvPacket.ReadBitSeq<6, 1>(playerGuid);
    recvPacket.ReadBitSeq<5, 4, 6, 1>(guildGuid);
    recvPacket.ReadBitSeq<3>(playerGuid);
    recvPacket.ReadBitSeq<2>(guildGuid);
    recvPacket.ReadBitSeq<0>(playerGuid);
    recvPacket.ReadBitSeq<0>(guildGuid);
    recvPacket.ReadBitSeq<4>(playerGuid);
    recvPacket.ReadBitSeq<3>(guildGuid);
    recvPacket.ReadBitSeq<5, 7>(playerGuid);


    recvPacket.ReadByteSeq<0, 2, 7>(playerGuid);
    recvPacket.ReadByteSeq<7, 2>(guildGuid);
    recvPacket.ReadByteSeq<3>(playerGuid);
    recvPacket.ReadByteSeq<3, 6, 0>(guildGuid);
    recvPacket.ReadByteSeq<4, 6>(playerGuid);
    recvPacket.ReadByteSeq<1, 5, 4>(guildGuid);
    recvPacket.ReadByteSeq<5, 1>(playerGuid);

    // If guild doesn't exist or player is not part of the guild send error
    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        if (guild->IsMember(playerGuid))
        {
            guild->HandleQuery(this);
            return;
        }

    Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_PLAYER_NOT_IN_GUILD);
}

void WorldSession::HandleGuildInviteOpcode(WorldPacket& recvPacket)
{
    time_t now = time(NULL);
    if (now - timeLastGuildInviteCommand < 5)
        return;
    else
       timeLastGuildInviteCommand = now;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_INVITE");

    uint32 nameLength = recvPacket.ReadBits(9);

    std::string invitedName = recvPacket.ReadString(nameLength);

    if (normalizePlayerName(invitedName))
        if (Guild* guild = _GetPlayerGuild(this, true))
            guild->HandleInviteMember(this, invitedName);
}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_REMOVE");

    ObjectGuid playerGuid;

    recvPacket.ReadBitSeq<7, 0, 6, 3, 1, 2, 4, 5>(playerGuid);
    recvPacket.ReadByteSeq<7, 4, 3, 0, 2, 6, 5, 1>(playerGuid);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleRemoveMember(this, playerGuid);
}

void WorldSession::HandleGuildMasterReplaceOpcode(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_REPLACE_GUILD_MASTER");

    Guild* guild = _GetPlayerGuild(this, true);

    if (!guild)
        return; // Cheat

    uint32 logoutTime = guild->GetMemberLogoutTime(guild->GetLeaderGUID());

    if (!logoutTime)
        return;

    time_t now = time(NULL);

    if (time_t(logoutTime + 3 * MONTH) > now)
        return; // Cheat

    guild->SwitchGuildLeader(GetPlayer()->GetGUID());
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_ACCEPT");
    // Player cannot be in guild
    if (!GetPlayer()->GetGuildId())
        // Guild where player was invited must exist
        if (Guild* guild = sGuildMgr->GetGuildById(GetPlayer()->GetGuildIdInvited()))
            guild->HandleAcceptMember(this);
}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_DECLINE");

    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);
}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_ROSTER");

    recvPacket.rfinish();

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleRoster(this);
}

void WorldSession::HandleGuildAssignRankOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_ASSIGN_MEMBER_RANK");

    ObjectGuid targetGuid;

    uint32 rankId;
    recvPacket >> rankId;

    recvPacket.ReadBitSeq<7, 2, 5, 6, 0, 4, 3, 1>(targetGuid);
    recvPacket.ReadByteSeq<0, 6, 5, 2, 4, 1, 3, 7>(targetGuid);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMemberRank(this, targetGuid, GetPlayer()->GetGUID(), rankId);
}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_LEAVE");

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleLeaveMember(this);
}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_DISBAND");

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleDisband(this);
}

void WorldSession::HandleGuildLeaderOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_LEADER");

    std::string name;
    std::string realName;
    uint32 len = recvPacket.ReadBits(9);
    name = recvPacket.ReadString(len);
    realName.resize(name.size());

    size_t pos = name.find('-');
    if (pos > 0)
    {
        for (size_t i = 0; i < name.size(); i++)
            if (i <= pos)
                realName[i] = name[i];

        if (normalizePlayerName(name))
            if (Guild* guild = _GetPlayerGuild(this, true))
                guild->HandleSetLeader(this, realName);
    }
    else
    {
        if (normalizePlayerName(name))
            if (Guild* guild = _GetPlayerGuild(this, true))
                guild->HandleSetLeader(this, name);
    }
}

void WorldSession::HandleGuildMOTDOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_MOTD");

    uint32 motdLength = recvPacket.ReadBits(10);
    std::string motd = recvPacket.ReadString(motdLength);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMOTD(this, motd);
}

void WorldSession::HandleSwapRanks(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_SWITCH_RANK");

    uint32 id = 0;
    bool up = false;

    recvPacket >> id;
    up = recvPacket.ReadBit();
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSwapRanks(this, id, up);
}

void WorldSession::HandleGuildSetNoteOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_SET_NOTE");

    ObjectGuid playerGuid;
    recvPacket.ReadBitSeq<6, 5, 0, 1, 2, 3>(playerGuid);
    bool type = recvPacket.ReadBit();      // 0 == Officer, 1 == Public
    recvPacket.ReadBitSeq<4>(playerGuid);
    uint32 noteLength = recvPacket.ReadBits(8);
    recvPacket.ReadBitSeq<7>(playerGuid);

    recvPacket.ReadByteSeq<5, 7, 2, 3, 4>(playerGuid);

    std::string note = recvPacket.ReadString(noteLength);

    recvPacket.ReadByteSeq<1, 0, 6>(playerGuid);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMemberNote(this, note, playerGuid, type);
}

void WorldSession::HandleGuildQueryRanksOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_QUERY_RANKS");

    ObjectGuid guildGuid;

    recvData.ReadBitSeq<4, 1, 6, 7, 5, 3, 0, 2>(guildGuid);
    recvData.ReadByteSeq<0, 3, 4, 5, 6, 1, 2, 7>(guildGuid);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        if (guild->IsMember(_player->GetGUID()))
            guild->HandleGuildRanks(this);
}

void WorldSession::HandleGuildAddRankOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_ADD_RANK");

    uint32 rankId;
    recvPacket >> rankId;

    uint32 length = recvPacket.ReadBits(7);

    std::string rankName = recvPacket.ReadString(length);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleAddNewRank(this, rankName); //, rankId);
}

void WorldSession::HandleGuildDelRankOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_DEL_RANK");

    uint32 rankId;
    recvPacket >> rankId;

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleRemoveRank(this, rankId);
}

void WorldSession::HandleGuildChangeInfoTextOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_INFO_TEXT");

    uint32 length = recvPacket.ReadBits(12);

    recvPacket.FlushBits();

    std::string info = recvPacket.ReadString(length / 2);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetInfo(this, info);
}

void WorldSession::HandleSaveGuildEmblemOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received MSG_SAVE_GUILD_EMBLEM");

    EmblemInfo emblemInfo;
    emblemInfo.ReadPacket(recvPacket);

    ObjectGuid playerGuid;

    recvPacket.ReadBitSeq<6, 4, 0, 7, 5, 2, 1, 3>(playerGuid);
    recvPacket.ReadByteSeq<5, 1, 0, 7, 4, 3, 6, 2>(playerGuid);

    Player* player = ObjectAccessor::FindPlayer(playerGuid);
    if (!player)
        return;

    if (GetPlayer()->GetGUID() != player->GetGUID())
        return;

    // Remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (Guild* guild = _GetPlayerGuild(this))
        guild->HandleSetEmblem(this, emblemInfo);
    else
        // "You are not part of a guild!";
        Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_NOGUILD);
}

void WorldSession::HandleGuildEventLogQueryOpcode(WorldPacket& /* recvPacket */)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_EVENT_LOG_QUERY)");

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendEventLog(this);
}

void WorldSession::HandleGuildBankMoneyWithdrawn(WorldPacket& /* recvData */)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_BANK_MONEY_WITHDRAWN_QUERY)");

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendMoneyInfo(this);
}

void WorldSession::HandleGuildPermissions(WorldPacket& /* recvData */)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_PERMISSIONS)");

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendPermissions(this);
}

// Called when clicking on Guild bank gameobject
void WorldSession::HandleGuildBankerActivate(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_BANKER_ACTIVATE)");

    ObjectGuid goGuid;

    recvData.ReadBitSeq<1, 7, 6, 3, 5, 0>(goGuid);
    recvData.ReadBit(); // 0 = only slots updated in last operation are shown. 1 = all slots updated
    recvData.ReadBitSeq<4, 2>(goGuid);
    recvData.ReadByteSeq<0, 2, 6, 5, 3, 7, 4, 1>(goGuid);

    if (GetPlayer()->GetGameObjectIfCanInteractWith(goGuid, GAMEOBJECT_TYPE_GUILD_BANK))
    {
        if (Guild* guild = _GetPlayerGuild(this))
            guild->SendBankList(this, 0, true, true);
        else
            Guild::SendCommandResult(this, GUILD_BANK, ERR_GUILD_PLAYER_NOT_IN_GUILD);
    }
}

// Called when opening guild bank tab only (first one)
void WorldSession::HandleGuildBankQueryTab(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_BANK_QUERY_TAB)");

    ObjectGuid goGuid;
    uint8 tabId;

    recvData >> tabId;

    recvData.ReadBitSeq<3, 1, 4, 0, 6, 5, 7>(goGuid);
    recvData.ReadBit(); // 0 = only slots updated in last operation are shown. 1 = all slots updated
    recvData.ReadBitSeq<2>(goGuid);

    recvData.ReadByteSeq<2, 5, 6, 0, 1, 4, 7, 3>(goGuid);

    if (GetPlayer()->GetGameObjectIfCanInteractWith(goGuid, GAMEOBJECT_TYPE_GUILD_BANK))
        if (Guild* guild = _GetPlayerGuild(this))
            guild->SendBankList(this, tabId, true, false);
}

void WorldSession::HandleGuildBankDepositMoney(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_BANK_DEPOSIT_MONEY)");

    ObjectGuid goGuid;
    uint64 money;

    recvData >> money;

    recvData.ReadBitSeq<4, 3, 5, 6, 1, 0, 2, 7>(goGuid);
    recvData.ReadByteSeq<1, 0, 4, 5, 2, 7, 3, 6>(goGuid);

    if (GetPlayer()->GetGameObjectIfCanInteractWith(goGuid, GAMEOBJECT_TYPE_GUILD_BANK))
    {
        if (money && GetPlayer()->HasEnoughMoney(money))
        {
            if (Guild* guild = _GetPlayerGuild(this))
            {
                uint64 amount = guild->GetBankMoney();
                if ((amount + money) > MAX_MONEY_AMOUNT)
                    guild->SendCommandResult(this, GUILD_BANK, ERR_GUILD_TOO_MUCH_MONEY);
                else
                    guild->HandleMemberDepositMoney(this, money);
            }
        }
    }
}

void WorldSession::HandleGuildBankWithdrawMoney(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_BANK_WITHDRAW_MONEY)");

    uint64 money;
    ObjectGuid goGuid;

    recvData >> money;

    recvData.ReadBitSeq<7, 6, 5, 0, 4, 3, 1, 2>(goGuid);
    recvData.ReadByteSeq<2, 4, 6, 7, 3, 1, 0, 5>(goGuid);

    if (money && GetPlayer()->GetGameObjectIfCanInteractWith(goGuid, GAMEOBJECT_TYPE_GUILD_BANK))
        if (Guild* guild = _GetPlayerGuild(this))
            guild->HandleMemberWithdrawMoney(this, money);
}

void WorldSession::HandleGuildBankSwapItems(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_BANK_SWAP_ITEMS)");

    ObjectGuid goGuid;
    uint32 amountSplited = 0;
    uint32 originalItemId = 0;
    uint32 itemId = 0;
    uint32 autostoreCount = 0;
    uint8 srcTabSlot = 0;
    uint8 toChar = 0;
    uint8 srcTabId = 0;
    uint8 dstTabId = 0;
    uint8 dstTabSlot = 0;
    uint8 plrSlot = NULL_SLOT;
    uint8 plrBag = NULL_BAG;
    bool hasDstTab = false;
    bool bankToBank = false;
    bool hasSrcTabSlot = false;
    bool hasPlrSlot = false;
    bool hasItemId = false;
    bool hasPlrBag = false;
    bool autostore = false;

    recvData >> amountSplited;
    recvData >> dstTabSlot >> toChar;
    recvData >> originalItemId;
    recvData >> srcTabId;

    recvData.ReadBit();
    recvData.ReadBitSeq<3>(goGuid);
    hasDstTab = !recvData.ReadBit();
    bankToBank = recvData.ReadBit();
    hasSrcTabSlot = !recvData.ReadBit();
    recvData.ReadBitSeq<1>(goGuid);
    hasPlrSlot = !recvData.ReadBit();
    recvData.ReadBitSeq<4>(goGuid);
    hasItemId = !recvData.ReadBit();
    recvData.ReadBitSeq<0>(goGuid);
    hasPlrBag = !recvData.ReadBit();
    recvData.ReadBitSeq<2, 7>(goGuid);
    autostore = !recvData.ReadBit();
    recvData.ReadBitSeq<6, 5>(goGuid);

    recvData.ReadByteSeq<7, 3, 1, 6, 5, 4, 2, 0>(goGuid);

    if (hasItemId)
        recvData >> itemId;

    if (hasPlrSlot)
        recvData >> plrSlot;

    if (autostore)
        recvData >> autostoreCount;

    if (hasDstTab)
        recvData >> dstTabId;

    if (hasPlrBag)
        recvData >> plrBag;

    if (hasSrcTabSlot)
        recvData >> srcTabSlot;

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(goGuid, GAMEOBJECT_TYPE_GUILD_BANK))
    {
        recvData.rfinish();                   // Prevent additional spam at rejected packet
        return;
    }

    Guild* guild = _GetPlayerGuild(this);
    if (!guild)
    {
        recvData.rfinish();                   // Prevent additional spam at rejected packet
        return;
    }

    if (bankToBank)
        guild->SwapItems(GetPlayer(), dstTabId, srcTabSlot, srcTabId, dstTabSlot, amountSplited);
    else
    {
        // allows depositing of items in the first slot of a bag that isn't the backpack
        if (!autostore && plrBag != 255 && plrSlot == NULL_SLOT)
            plrSlot = 0;

        // Player <-> Bank
        // Allow to work with inventory only
        if (!Player::IsInventoryPos(plrBag, plrSlot) && !(plrBag == NULL_BAG && plrSlot == NULL_SLOT))
            GetPlayer()->SendEquipError(EQUIP_ERR_INTERNAL_BAG_ERROR, NULL);
        else
            guild->SwapItemsWithInventory(GetPlayer(), toChar, srcTabId, dstTabSlot, plrBag, plrSlot, amountSplited);
    }
}

void WorldSession::HandleGuildBankBuyTab(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_BANK_BUY_TAB)");

    uint8 tabId;
    ObjectGuid goGuid;
    recvData >> tabId;

    recvData.ReadBitSeq<1, 3, 7, 2, 4, 0, 5, 6>(goGuid);
    recvData.ReadByteSeq<3, 1, 6, 5, 4, 2, 7, 0>(goGuid);

    // Only for SPELL_EFFECT_UNLOCK_GUILD_VAULT_TAB, this prevent cheating
    if (tabId > 5)
        return;

    if (!goGuid || GetPlayer()->GetGameObjectIfCanInteractWith(goGuid, GAMEOBJECT_TYPE_GUILD_BANK))
        if (Guild* guild = _GetPlayerGuild(this))
            guild->HandleBuyBankTab(this, tabId);
}

void WorldSession::HandleGuildBankUpdateTab(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received (CMSG_GUILD_BANK_UPDATE_TAB)");

    uint8 tabId;
    ObjectGuid goGuid;
    uint32 nameLen, iconLen;
    std::string name;
    std::string icon;

    recvData >> tabId;

    nameLen = recvData.ReadBits(7);
    iconLen = recvData.ReadBits(9);

    recvData.ReadBitSeq<0, 2, 6, 7, 3, 4, 5, 1>(goGuid);
    recvData.ReadByteSeq<6, 4>(goGuid);
    icon = recvData.ReadString(iconLen);
    recvData.ReadByteSeq<5, 0, 7, 1>(goGuid);
    name = recvData.ReadString(nameLen);
    recvData.ReadByteSeq<2, 3>(goGuid);

    if (!name.empty() && !icon.empty())
        if (GetPlayer()->GetGameObjectIfCanInteractWith(goGuid, GAMEOBJECT_TYPE_GUILD_BANK))
            if (Guild* guild = _GetPlayerGuild(this))
                guild->HandleSetBankTabInfo(this, tabId, name, icon);
}

void WorldSession::HandleGuildBankLogQuery(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received (MSG_GUILD_BANK_LOG_QUERY)");

    uint32 tabId;
    recvData >> tabId;

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendBankLog(this, tabId);
}

void WorldSession::HandleQueryGuildBankTabText(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_BANK_QUERY_TEXT");

    uint32 tabId;
    recvData >> tabId;

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendBankTabText(this, tabId);
}

void WorldSession::HandleSetGuildBankTabText(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_SET_GUILD_BANK_TEXT");

    uint32 tabId;
    recvData >> tabId;

    uint32 textLen = recvData.ReadBits(14);

    std::string text = recvData.ReadString(textLen);

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SetBankTabText(tabId, text);
}

void WorldSession::HandleGuildQueryXPOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUERY_GUILD_XP");

    ObjectGuid guildGuid;

    recvPacket.ReadBitSeq<2, 1, 6, 4, 3, 7, 0, 5>(guildGuid);
    recvPacket.ReadByteSeq<6, 0, 1, 3, 4, 7, 5, 2>(guildGuid);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildXP(this);
}

void WorldSession::HandleGuildSetRankPermissionsOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_SET_RANK_PERMISSIONS");

    Guild* guild = _GetPlayerGuild(this, true);
    if (!guild)
    {
        recvPacket.rfinish();
        return;
    }

    uint32 unk;
    uint32 rankId;
    uint32 oldRights;
    uint32 newRights;
    uint32 moneyPerDay;

    recvPacket >> rankId;
    recvPacket >> newRights;
    recvPacket >> moneyPerDay;

    GuildBankRightsAndSlotsVec rightsAndSlots(GUILD_BANK_MAX_TABS);
    for (uint8 tabId = 0; tabId < GUILD_BANK_MAX_TABS; ++tabId)
    {
        uint32 bankRights;
        uint32 slots;

        recvPacket >> bankRights;
        recvPacket >> slots;

        rightsAndSlots[tabId] = GuildBankRightsAndSlots(uint8(bankRights), slots);
    }

    recvPacket >> unk;
    recvPacket >> oldRights;
    uint32 nameLength = recvPacket.ReadBits(7);
    std::string rankName = recvPacket.ReadString(nameLength);

    guild->HandleSetRankInfo(this, rankId, rankName, newRights, moneyPerDay * GOLD, rightsAndSlots);
}

void WorldSession::HandleGuildRequestPartyState(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_REQUEST_PARTY_STATE");

    ObjectGuid guildGuid;

    recvData.ReadBitSeq<0, 1, 2, 6, 5, 7, 3, 4>(guildGuid);
    recvData.ReadByteSeq<4, 1, 6, 7, 2, 3, 5, 0>(guildGuid);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        guild->HandleGuildPartyRequest(this);
}

void WorldSession::HandleGuildRequestMaxDailyXP(WorldPacket& recvPacket)
{
    ObjectGuid guildGuid;

    recvPacket.ReadBitSeq<2, 5, 3, 7, 4, 1, 0, 6>(guildGuid);
    recvPacket.ReadByteSeq<7, 3, 2, 1, 0, 5, 6, 4>(guildGuid);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
    {
        if (guild->IsMember(_player->GetGUID()))
        {
            WorldPacket data(SMSG_GUILD_MAX_DAILY_XP, 8);
            data << uint64(sWorld->getIntConfig(CONFIG_GUILD_DAILY_XP_CAP));
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleAutoDeclineGuildInvites(WorldPacket& recvPacket)
{
    bool enable;
    enable = recvPacket.ReadBit();

    GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_AUTO_DECLINE_GUILD, enable);
}

void WorldSession::HandleGuildRewardsQueryOpcode(WorldPacket& recvPacket)
{
    uint32 unk = 0;
    recvPacket >> unk;

    if (sGuildMgr->GetGuildById(_player->GetGuildId()))
    {
        std::vector<GuildReward> const& rewards = sGuildMgr->GetGuildRewards();

        WorldPacket data(SMSG_GUILD_REWARDS_LIST);
        ByteBuffer dataBuffer;

        data << uint32(time(NULL));
        data.WriteBits(rewards.size(), 19);

        for (uint32 i = 0; i < rewards.size(); i++)
        {
            data.WriteBits(rewards[i].AchievementId > 0 ? 1 : 0, 22);

            dataBuffer << uint32(rewards[i].Entry);
            dataBuffer << uint64(rewards[i].Price);
            dataBuffer << uint32(rewards[i].Racemask);

            if (rewards[i].AchievementId)
                dataBuffer << uint32(rewards[i].AchievementId);

            dataBuffer << uint32(rewards[i].Standing);
            dataBuffer << uint32(0);
        }

        data.FlushBits();
        data.append(dataBuffer);
        SendPacket(&data);
    }
}

void WorldSession::HandleGuildQueryNewsOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_QUERY_NEWS");

    ObjectGuid guildGuid;

    recvPacket.ReadBitSeq<7, 3, 4, 1, 0, 6, 2, 5>(guildGuid);
    recvPacket.ReadByteSeq<2, 7, 6, 4, 3, 1, 0, 5>(guildGuid);

    auto const guild = sGuildMgr->GetGuildByGuid(guildGuid);
    if (guild && guild->IsMember(_player->GetGUID()))
    {
        WorldPacket data;
        guild->GetNewsLog().BuildNewsData(data);
        SendPacket(&data);
    }
}

void WorldSession::HandleGuildNewsUpdateStickyOpcode(WorldPacket& recvPacket)
{
    uint32 newsId;
    bool sticky;
    ObjectGuid guid;

    recvPacket >> newsId;

    recvPacket.ReadBitSeq<6, 7, 1>(guid);
    sticky = recvPacket.ReadBit();
    recvPacket.ReadBitSeq<2, 5, 0, 3, 4>(guid);

    recvPacket.ReadByteSeq<0, 7, 2, 3, 6, 5, 1, 4>(guid);

    if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
    {
        if (GuildNewsEntry* newsEntry = guild->GetNewsLog().GetNewsById(newsId))
        {
            if (sticky)
                newsEntry->Flags |= 1;
            else
                newsEntry->Flags &= ~1;
            WorldPacket data;
            guild->GetNewsLog().BuildNewsData(newsId, *newsEntry, data);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleGuildRequestChallengeUpdate(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_REQUEST_CHALLENGE_UPDATE");

    GuildChallengeRewardData const& reward = sObjectMgr->GetGuildChallengeRewardData();

    WorldPacket data(SMSG_GUILD_CHALLENGE_UPDATED, 5*6*4);

    for (uint8 i = 0; i < CHALLENGE_MAX; i++)
        data << uint32(reward[i].ChallengeCount);

    for (uint8 i = 0; i < CHALLENGE_MAX; i++)
        data << uint32(0);                      // Current count : @TODO : New system ! Guild challenge

    for (uint8 i = 0; i < CHALLENGE_MAX; i++)
        data << uint32(reward[i].Experience);

    for (uint8 i = 0; i < CHALLENGE_MAX; i++)
        data << uint32(reward[i].Gold2);

    for (uint8 i = 0; i < CHALLENGE_MAX; i++)
        data << uint32(reward[i].Gold);

    SendPacket(&data);
}

void WorldSession::HandleGuildRequestGuildRecipes(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_REQUEST_CHALLENGE_UPDATE");

    ObjectGuid guildGuid;

    recvPacket.ReadBitSeq<1, 7, 4, 5, 2, 6, 0, 3>(guildGuid);
    recvPacket.ReadByteSeq<7, 0, 2, 3, 1, 5, 4, 6>(guildGuid);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildRecipes(this);
}
