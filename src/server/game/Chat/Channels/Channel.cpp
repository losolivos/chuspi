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

#include "Channel.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "SocialMgr.h"
#include "World.h"
#include "DatabaseEnv.h"
#include "AccountMgr.h"
#include "Player.h"
#include "LexicalCast.h"

Channel::Channel(const std::string& name, uint32 channel_id, uint32 Team)
    : m_announce(true)
    , m_ownership(true)
    , m_IsSaved(false)
    , m_flags(0)
    , m_channelId(channel_id)
    , m_Team(Team)
    , m_ownerGUID(0)
    , m_name(name)
    , m_password("")
{
    // set special flags if built-in channel
    if (ChatChannelsEntry const* ch = sChatChannelsStore.LookupEntry(channel_id)) // check whether it's a built-in channel
    {
        m_announce = false;                                 // no join/leave announces
        m_ownership = false;                                // no ownership handout

        m_flags |= CHANNEL_FLAG_GENERAL;                    // for all built-in channels

        if (ch->flags & CHANNEL_DBC_FLAG_TRADE)             // for trade channel
            m_flags |= CHANNEL_FLAG_TRADE;

        if (ch->flags & CHANNEL_DBC_FLAG_CITY_ONLY2)        // for city only channels
            m_flags |= CHANNEL_FLAG_CITY;

        if (ch->flags & CHANNEL_DBC_FLAG_LFG)               // for LFG channel
            m_flags |= CHANNEL_FLAG_LFG;
        else                                                // for all other channels
            m_flags |= CHANNEL_FLAG_NOT_LFG;
    }
    else if (!stricmp(m_name.c_str(), "global"))
    {
        m_announce = false;
        m_flags |= CHANNEL_FLAG_GENERAL;
        m_ownership = false;                                // no ownership handout
    }
    else                                                    // it's custom channel
    {
        m_flags |= CHANNEL_FLAG_CUSTOM;

        // If storing custom channels in the db is enabled either load or save the channel
        if (sWorld->getBoolConfig(CONFIG_PRESERVE_CUSTOM_CHANNELS))
        {
            PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHANNEL);
            stmt->setString(0, name);
            stmt->setUInt32(1, m_Team);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (result) //load
            {
                Field* fields = result->Fetch();
                m_announce = fields[0].GetBool();
                m_ownership = fields[1].GetBool();
                m_password  = fields[2].GetString();
                const char* db_BannedList = fields[3].GetCString();

                if (db_BannedList)
                {
                    Tokenizer tokens(db_BannedList, ' ');
                    Tokenizer::const_iterator iter;
                    for (auto i = tokens.begin(); i != tokens.end(); ++i)
                    {
                        uint64 banned_guid = Trinity::lexicalCast<uint64>(*i);
                        if (banned_guid)
                        {
                            TC_LOG_DEBUG("chat.system", "Channel(%s) loaded banned guid:" UI64FMTD "", name.c_str(), banned_guid);
                            bannedStore.insert(banned_guid);
                        }
                    }
                }
            }
            else // save
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHANNEL);
                stmt->setString(0, name);
                stmt->setUInt32(1, m_Team);
                CharacterDatabase.Execute(stmt);
                TC_LOG_DEBUG("chat.system", "Channel(%s) saved in database", name.c_str());
            }

            m_IsSaved = true;
        }
    }
}

void Channel::UpdateChannelInDB() const
{
    if (m_IsSaved)
    {
        std::ostringstream banlist;
        for (auto iter = bannedStore.begin(); iter != bannedStore.end(); ++iter)
            banlist << (*iter) << ' ';

        std::string banListStr = banlist.str();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHANNEL);
        stmt->setBool(0, m_announce);
        stmt->setBool(1, m_ownership);
        stmt->setString(2, m_password);
        stmt->setString(3, banListStr);
        stmt->setString(4, m_name);
        stmt->setUInt32(5, m_Team);
        CharacterDatabase.Execute(stmt);

        TC_LOG_DEBUG("chat.system", "Channel(%s) updated in database", m_name.c_str());
    }

}

void Channel::UpdateChannelUseageInDB() const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHANNEL_USAGE);
    stmt->setString(0, m_name);
    stmt->setUInt32(1, m_Team);
    CharacterDatabase.Execute(stmt);
}

void Channel::CleanOldChannelsInDB()
{
    if (sWorld->getIntConfig(CONFIG_PRESERVE_CUSTOM_CHANNEL_DURATION) > 0)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_OLD_CHANNELS);
        stmt->setUInt32(0, sWorld->getIntConfig(CONFIG_PRESERVE_CUSTOM_CHANNEL_DURATION) * DAY);
        CharacterDatabase.Execute(stmt);

        TC_LOG_DEBUG("chat.system", "Cleaned out unused custom chat channels.");
    }
}

void Channel::JoinChannel(Player* player, std::string const& pass)
{
    uint64 guid = player->GetGUID();
    if (IsOn(guid))
    {
        // Do not send error message for built-in channels
        if (!IsConstant())
        {
            WorldPacket data;
            MakePlayerAlreadyMember(&data, guid);
            SendToOne(&data, guid);
        }
        return;
    }

    if (IsBanned(guid))
    {
        WorldPacket data;
        MakeBanned(&data);
        SendToOne(&data, guid);
        return;
    }

    if (!m_password.empty() && pass != m_password)
    {
        WorldPacket data;
        MakeWrongPassword(&data);
        SendToOne(&data, guid);
        return;
    }

    if (HasFlag(CHANNEL_FLAG_LFG) &&
        sWorld->getBoolConfig(CONFIG_RESTRICTED_LFG_CHANNEL) &&
        AccountMgr::IsPlayerAccount(player->GetSession()->GetSecurity()) && //FIXME: Move to RBAC
        player->GetGroup())
    {
        WorldPacket data;
        MakeNotInLfg(&data);
        SendToOne(&data, guid);
        return;
    }

    player->JoinedChannel(this);

    if (m_announce && (!AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()) || !sWorld->getBoolConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)))
    {
        WorldPacket data;
        MakeJoined(&data, guid);
        SendToAll(&data);
    }

    PlayerInfo pinfo;
    pinfo.player = guid;
    pinfo.flags = MEMBER_FLAG_NONE;
    playersStore[guid] = pinfo;

    WorldPacket data;
    MakeYouJoined(&data);
    SendToOne(&data, guid);

    JoinNotify(guid);

    // Custom channel handling
    if (!IsConstant())
    {
        // Update last_used timestamp in db
        if (!playersStore.empty())
            UpdateChannelUseageInDB();

        // If the channel has no owner yet and ownership is allowed, set the new owner.
        if (!m_ownerGUID && m_ownership)
        {
            SetOwner(guid, playersStore.size() > 1);
            playersStore[guid].SetModerator(true);
        }
    }
}

void Channel::LeaveChannel(Player* player, bool send)
{
    uint64 guid = player->GetGUID();
    if (!IsOn(guid))
    {
        if (send)
        {
            WorldPacket data;
            MakeNotMember(&data);
            SendToOne(&data, guid);
        }
        return;
    }

    if (send)
    {
        WorldPacket data;
        MakeYouLeft(&data);
        SendToOne(&data, guid);
        player->LeftChannel(this);
    }

    bool changeowner = playersStore[guid].IsOwner();

    playersStore.erase(guid);

    if (m_announce && (!AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()) || !sWorld->getBoolConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)))
    {
        WorldPacket data;
        MakeLeft(&data, guid);
        SendToAll(&data);
    }

    LeaveNotify(guid);

    if (!IsConstant())
    {
        // Update last_used timestamp in db
        UpdateChannelUseageInDB();

        // If the channel owner left and there are still players inside, pick a new owner
        if (changeowner && m_ownership && !playersStore.empty())
        {
            uint64 newowner = playersStore.begin()->second.player;
            playersStore[newowner].SetModerator(true);
            SetOwner(newowner);
        }
    }
}

void Channel::KickOrBan(Player const* player, std::string const& badname, bool ban)
{
    uint64 good = player->GetGUID();

    if (!IsOn(good))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, good);
        return;
    }

    AccountTypes sec = SEC_PLAYER;
    Player* gplr = ObjectAccessor::FindPlayer(good);
    if (gplr)
        sec = gplr->GetSession()->GetSecurity();
    else if (!playersStore[good].IsModerator() && !AccountMgr::IsModeratorAccount(sec))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, good);
        return;
    }

    Player* bad = sObjectAccessor->FindPlayerByName(badname);
    uint64 victim = bad ? bad->GetGUID() : 0;
    if (!victim || !IsOn(victim))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, badname);
        SendToOne(&data, good);
        return;
    }

    bool changeowner = m_ownerGUID == victim;
    if (!(AccountMgr::IsModeratorAccount(sec) && sWorld->getBoolConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)) && changeowner && good != m_ownerGUID)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data, good);
        return;
    }

    if (ban && !IsBanned(bad->GetGUID()))
    {
        bannedStore.insert(bad->GetGUID());
        UpdateChannelInDB();

        if (AccountMgr::IsModeratorAccount(sec))
        {
            WorldPacket data;
            MakePlayerBanned(&data, victim, good);
            SendToAll(&data);
        }
    }
    else if (!AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()) || !sWorld->getBoolConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL))
    {
        WorldPacket data;
        MakePlayerKicked(&data, victim, good);
        SendToAll(&data);
    }

    playersStore.erase(victim);
    bad->LeftChannel(this);

    if (changeowner && m_ownership && !playersStore.empty())
    {
        uint64 newowner = good;
        playersStore[newowner].SetModerator(true);
        SetOwner(newowner);
    }
}

void Channel::UnBan(Player const* player, std::string const& badname)
{
    uint64 good = player->GetGUID();

    if (!IsOn(good))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, good);
        return;
    }

    AccountTypes sec = SEC_PLAYER;
    Player* gplr = ObjectAccessor::FindPlayer(good);
    if (gplr)
        sec = gplr->GetSession()->GetSecurity();

    if (!playersStore[good].IsModerator() && !AccountMgr::IsModeratorAccount(sec))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, good);
        return;
    }

    Player* bad = sObjectAccessor->FindPlayerByName(badname);
    uint64 victim = bad ? bad->GetGUID(): 0;

    if (!victim || !IsBanned(victim))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, badname);
        SendToOne(&data, good);
        return;
    }

    bannedStore.erase(victim);

    WorldPacket data;
    MakePlayerUnbanned(&data, victim, good);
    SendToAll(&data);

    UpdateChannelInDB();
}

void Channel::Password(Player const* player, std::string const& pass)
{
    uint64 guid = player->GetGUID();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    AccountTypes sec = SEC_PLAYER;
    Player* gplr = ObjectAccessor::FindPlayer(guid);
    if (gplr)
        sec = gplr->GetSession()->GetSecurity();

    if (!playersStore[guid].IsModerator() && !AccountMgr::IsModeratorAccount(sec))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, guid);
        return;
    }

    m_password = pass;

    WorldPacket data;
    MakePasswordChanged(&data, guid);
    SendToAll(&data);

    UpdateChannelInDB();
}

void Channel::SetMode(Player const* player, std::string const& p2n, bool mod, bool set)
{
    uint64 guid = player->GetGUID();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    AccountTypes sec = SEC_PLAYER;
    Player* gplr = ObjectAccessor::FindPlayer(guid);
    if (gplr)
        sec = gplr->GetSession()->GetSecurity();

    if (!playersStore[guid].IsModerator() && !AccountMgr::IsModeratorAccount(sec))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, guid);
        return;
    }

    if (guid == m_ownerGUID && p2n == player->GetName() && mod)
        return;

    Player* newp = sObjectAccessor->FindPlayerByName(p2n);
    uint64 victim = newp ? newp->GetGUID() : 0;

    if (!victim || !IsOn(victim) ||
        (player->GetTeam() != newp->GetTeam() && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL) &&
        (!AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()) || !AccountMgr::IsModeratorAccount(newp->GetSession()->GetSecurity()))))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, p2n);
        SendToOne(&data, guid);
        return;
    }

    if (m_ownerGUID == victim && m_ownerGUID != guid)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data, guid);
        return;
    }

    if (mod)
        SetModerator(newp->GetGUID(), set);
    else
        SetMute(newp->GetGUID(), set);
}

void Channel::SetOwner(Player const* player, std::string const& newname)
{
    uint64 guid = player->GetGUID();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    AccountTypes sec = SEC_PLAYER;
    Player* gplr = ObjectAccessor::FindPlayer(guid);
    if (gplr)
        sec = gplr->GetSession()->GetSecurity();

    if (!AccountMgr::IsModeratorAccount(sec) && guid != m_ownerGUID)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data, guid);
        return;
    }

    Player* newp = sObjectAccessor->FindPlayerByName(newname);
    uint64 victim = newp ? newp->GetGUID() : 0;

    if (!victim || !IsOn(victim) ||
        (player->GetTeam() != newp->GetTeam() &&!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL)))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, newname);
        SendToOne(&data, guid);
        return;
    }

    playersStore[victim].SetModerator(true);
    SetOwner(victim);
}

void Channel::SendWhoOwner(uint64 guid)
{
    WorldPacket data;
    if (IsOn(guid))
        MakeChannelOwner(&data);
    else
        MakeNotMember(&data);
    SendToOne(&data, guid);
}

void Channel::List(Player const* player)
{
    uint64 guid = player->GetGUID();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    TC_LOG_DEBUG("chat.system", "SMSG_CHANNEL_LIST %s Channel: %s",
                 player->GetSession()->GetPlayerName().c_str(), GetName().c_str());

    WorldPacket data(SMSG_CHANNEL_LIST, 1+(GetName().size()+1)+1+4+playersStore.size()*(8+1));
    data << uint8(1);                                   // channel type?
    data << GetName();                                  // channel name
    data << uint8(GetFlags());                          // channel flags?

    size_t pos = data.wpos();
    data << uint32(0);                                  // size of list, placeholder

    uint32 gmLevelInWhoList = sWorld->getIntConfig(CONFIG_GM_LEVEL_IN_WHO_LIST);

    uint32 count  = 0;
    for (PlayerContainer::const_iterator i = playersStore.begin(); i != playersStore.end(); ++i)
    {
        Player* member = ObjectAccessor::FindPlayer(i->first);

        // PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
        // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
        if (member
                && (!AccountMgr::IsPlayerAccount(player->GetSession()->GetSecurity()) || member->GetSession()->GetSecurity() <= AccountTypes(gmLevelInWhoList))
                && member->IsVisibleGloballyFor(player))
        {
            data << uint64(i->first);
            data << uint8(i->second.flags);             // flags seems to be changed...
            ++count;

            if (++count == sWorld->getIntConfig(CONFIG_MAX_WHO))
                break;
        }
    }

    data.put<uint32>(pos, count);

    SendToOne(&data, guid);
}

void Channel::Announce(Player const* player)
{
    uint64 guid = player->GetGUID();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    AccountTypes sec = SEC_PLAYER;
    Player* gplr = ObjectAccessor::FindPlayer(guid);
    if (gplr)
        sec = gplr->GetSession()->GetSecurity();

    if (!playersStore[guid].IsModerator() && !AccountMgr::IsModeratorAccount(sec))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, guid);
        return;
    }

    m_announce = !m_announce;

    WorldPacket data;
    if (m_announce)
        MakeAnnouncementsOn(&data, guid);
    else
        MakeAnnouncementsOff(&data, guid);
    SendToAll(&data);

    UpdateChannelInDB();
}

void Channel::Say(uint64 guid, std::string const& what, uint32 lang)
{
    if (what.empty())
        return;

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)
        return;

    // TODO: Add proper RBAC check
    if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
        lang = LANG_UNIVERSAL;

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    if (playersStore[guid].IsMuted())
    {
        WorldPacket data;
        MakeMuted(&data);
        SendToOne(&data, guid);
        return;
    }

    WorldPacket data;
    player->BuildPlayerChat(&data, CHAT_MSG_CHANNEL, what, lang, NULL, m_name);
    SendToAll(&data, !playersStore[guid].IsModerator() ? guid : false);
}

void Channel::Invite(Player const* player, std::string const& newname)
{
    uint64 guid = player->GetGUID();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    Player* newp = sObjectAccessor->FindPlayerByName(newname);
    if (!newp || !newp->isGMVisible())
    {
        WorldPacket data;
        MakePlayerNotFound(&data, newname);
        SendToOne(&data, guid);
        return;
    }

    if (IsBanned(newp->GetGUID()))
    {
        WorldPacket data;
        MakePlayerInviteBanned(&data, newname);
        SendToOne(&data, guid);
        return;
    }

    if (newp->GetTeam() != player->GetTeam() && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        WorldPacket data;
        MakeInviteWrongFaction(&data);
        SendToOne(&data, guid);
        return;
    }

    if (IsOn(newp->GetGUID()))
    {
        WorldPacket data;
        MakePlayerAlreadyMember(&data, newp->GetGUID());
        SendToOne(&data, guid);
        return;
    }

    if (!newp->GetSocial()->HasIgnore(GUID_LOPART(guid)))
    {
        WorldPacket data;
        MakeInvite(&data, guid);
        SendToOne(&data, newp->GetGUID());
    }

    WorldPacket data;
    MakePlayerInvited(&data, newp->GetName());
    SendToOne(&data, guid);
}

void Channel::SetOwner(uint64 guid, bool exclaim)
{
    if (m_ownerGUID)
    {
        // [] will re-add player after it possible removed
        PlayerContainer::iterator p_itr = playersStore.find(m_ownerGUID);
        if (p_itr != playersStore.end())
            p_itr->second.SetOwner(false);
    }

    m_ownerGUID = guid;
    if (m_ownerGUID)
    {
        uint8 oldFlag = GetPlayerFlags(m_ownerGUID);
        playersStore[m_ownerGUID].SetModerator(true);
        playersStore[m_ownerGUID].SetOwner(true);

        WorldPacket data;
        MakeModeChange(&data, m_ownerGUID, oldFlag);
        SendToAll(&data);

        if (exclaim)
        {
            MakeOwnerChanged(&data, m_ownerGUID);
            SendToAll(&data);
        }

        UpdateChannelInDB();
    }
}

void Channel::SendToAll(WorldPacket* data, uint64 guid)
{
    for (PlayerContainer::const_iterator i = playersStore.begin(); i != playersStore.end(); ++i)
        if (Player* player = ObjectAccessor::FindPlayer(i->first))
            if (!guid || !player->GetSocial()->HasIgnore(GUID_LOPART(guid)))
                player->SendDirectMessage(data);
}

void Channel::SendToAllButOne(WorldPacket* data, uint64 who)
{
    for (PlayerContainer::const_iterator i = playersStore.begin(); i != playersStore.end(); ++i)
        if (i->first != who)
            if (Player* player = ObjectAccessor::FindPlayer(i->first))
                player->SendDirectMessage(data);
}

void Channel::SendToOne(WorldPacket* data, uint64 who)
{
    if (Player* player = ObjectAccessor::FindPlayer(who))
        player->SendDirectMessage(data);
}

void Channel::Voice(uint64 /*guid1*/, uint64 /*guid2*/)
{

}

void Channel::DeVoice(uint64 /*guid1*/, uint64 /*guid2*/)
{

}

void Channel::MakeNotifyPacket(WorldPacket* data, uint8 notify_type)
{
    data->Initialize(SMSG_CHANNEL_NOTIFY, 1+m_name.size()+1);
    *data << uint8(notify_type);
    *data << m_name;
}

void Channel::MakeJoined(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_JOINED_NOTICE);
    *data << uint64(guid);
}

void Channel::MakeLeft(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_LEFT_NOTICE);
    *data << uint64(guid);
}

void Channel::MakeYouJoined(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_YOU_JOINED_NOTICE);
    *data << uint8(GetFlags());
    *data << uint32(GetChannelId());
    *data << uint32(0);
}

void Channel::MakeYouLeft(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_YOU_LEFT_NOTICE);
    *data << uint32(GetChannelId());
    *data << uint8(IsConstant());
}

void Channel::MakeWrongPassword(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_WRONG_PASSWORD_NOTICE);
}

void Channel::MakeNotMember(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_MEMBER_NOTICE);
}

void Channel::MakeNotModerator(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_MODERATOR_NOTICE);
}

void Channel::MakePasswordChanged(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PASSWORD_CHANGED_NOTICE);
    *data << uint64(guid);
}

void Channel::MakeOwnerChanged(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_OWNER_CHANGED_NOTICE);
    *data << uint64(guid);
}

void Channel::MakePlayerNotFound(WorldPacket* data, const std::string& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_NOT_FOUND_NOTICE);
    *data << name;
}

void Channel::MakeNotOwner(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_OWNER_NOTICE);
}

void Channel::MakeChannelOwner(WorldPacket* data)
{
    std::string name = "";

    if (!sObjectMgr->GetPlayerNameByGUID(m_ownerGUID, name) || name.empty())
        name = "PLAYER_NOT_FOUND";

    MakeNotifyPacket(data, CHAT_CHANNEL_OWNER_NOTICE);
    *data << ((IsConstant() || !m_ownerGUID) ? "Nobody" : name);
}

void Channel::MakeModeChange(WorldPacket* data, uint64 guid, uint8 oldflags)
{
    MakeNotifyPacket(data, CHAT_MODE_CHANGE_NOTICE);
    *data << uint64(guid);
    *data << uint8(oldflags);
    *data << uint8(GetPlayerFlags(guid));
}

void Channel::MakeAnnouncementsOn(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_ANNOUNCEMENTS_ON_NOTICE);
    *data << uint64(guid);
}

void Channel::MakeAnnouncementsOff(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_ANNOUNCEMENTS_OFF_NOTICE);
    *data << uint64(guid);
}

void Channel::MakeMuted(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_MUTED_NOTICE);
}

void Channel::MakePlayerKicked(WorldPacket* data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_KICKED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

void Channel::MakeBanned(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_BANNED_NOTICE);
}

void Channel::MakePlayerBanned(WorldPacket* data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_BANNED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

void Channel::MakePlayerUnbanned(WorldPacket* data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_UNBANNED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

void Channel::MakePlayerNotBanned(WorldPacket* data, const std::string &name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_NOT_BANNED_NOTICE);
    *data << name;
}

void Channel::MakePlayerAlreadyMember(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PLAYER_ALREADY_MEMBER_NOTICE);
    *data << uint64(guid);
}

void Channel::MakeInvite(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_INVITE_NOTICE);
    *data << uint64(guid);
}

void Channel::MakeInviteWrongFaction(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_INVITE_WRONG_FACTION_NOTICE);
}

void Channel::MakeWrongFaction(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_WRONG_FACTION_NOTICE);
}

void Channel::MakeInvalidName(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_INVALID_NAME_NOTICE);
}

void Channel::MakeNotModerated(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_MODERATED_NOTICE);
}

void Channel::MakePlayerInvited(WorldPacket* data, const std::string& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_INVITED_NOTICE);
    *data << name;
}

void Channel::MakePlayerInviteBanned(WorldPacket* data, const std::string& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_INVITE_BANNED_NOTICE);
    *data << name;
}

void Channel::MakeThrottled(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_THROTTLED_NOTICE);
}

void Channel::MakeNotInArea(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_IN_AREA_NOTICE);
}

void Channel::MakeNotInLfg(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_IN_LFG_NOTICE);
}

void Channel::MakeVoiceOn(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_VOICE_ON_NOTICE);
    *data << uint64(guid);
}

void Channel::MakeVoiceOff(WorldPacket* data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_VOICE_OFF_NOTICE);
    *data << uint64(guid);
}

void Channel::JoinNotify(uint64 /*guid*/)
{
#if 0
    ObjectGuid playerGuid = guid;
    WorldPacket data(SMSG_USERLIST_ADD);

    data.WriteBitSeq<3, 1, 2, 5, 7, 6, 0>(playerGuid);
    data.WriteBits(GetName().size(), 7);
    data.WriteBitSeq<4>(playerGuid);

    data.WriteByteSeq<5>(playerGuid);

    if (GetName().size())
    {
        data.FlushBits();
        data.append(GetName().c_str(), GetName().size());
    }

    data << uint8(GetPlayerFlags(guid));
    data.WriteByteSeq<3, 7, 0, 1, 4, 6>(playerGuid);
    data << uint32(GetNumPlayers());
    data.WriteByteSeq<2>(playerGuid);

    SendToAllButOne(&data, guid);
#endif
}

void Channel::LeaveNotify(uint64 /*guid*/)
{
#if 0
    WorldPacket data(SMSG_USERLIST_REMOVE);
    ObjectGuid playerGuid = guid;

    data.WriteBitSeq<2>(playerGuid);
    data.WriteBits(GetName().size(), 7);
    data.WriteBitSeq<6, 3, 7, 0, 5, 1, 4>(playerGuid);

    data << uint8(GetFlags());
    data.WriteByteSeq<6>(playerGuid);
    data << uint8(GetPlayerFlags(guid));
    data.WriteByteSeq<0>(playerGuid);

    if (GetName().size())
        data.append(GetName().c_str(), GetName().size());

    data << uint32(GetNumPlayers());
    data.WriteByteSeq<3, 2, 4, 5, 1, 7>(playerGuid);

    if (IsConstant())
        SendToAllButOne(&data, guid);
    else
        SendToAll(&data);
#endif
}
