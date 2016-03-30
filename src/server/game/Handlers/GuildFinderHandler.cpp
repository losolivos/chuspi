/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "WorldSession.h"
#include "WorldPacket.h"
#include "Object.h"
#include "SharedDefines.h"
#include "GuildFinderMgr.h"
#include "GuildMgr.h"

void WorldSession::HandleGuildFinderAddRecruit(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_LF_GUILD_ADD_RECRUIT");

    if (sGuildFinderMgr->GetAllMembershipRequestsForPlayer(GetPlayer()->GetGUIDLow()).size() == 10)
        return;

    uint32 classRoles = 0;
    uint32 availability = 0;
    uint32 guildInterests = 0;

    recvPacket >> classRoles >> availability >> guildInterests;

    ObjectGuid guid;

    recvPacket.ReadBitSeq<1, 5, 2, 7>(guid);

    uint16 commentLength = recvPacket.ReadBits(10);

    recvPacket.ReadBitSeq<0, 6, 4, 3>(guid);
    recvPacket.ReadByteSeq<2, 5, 3, 7, 1, 4, 0, 6>(guid);

    std::string comment = recvPacket.ReadString(commentLength);

    uint32 guildLowGuid = GUID_LOPART(uint64(guid));

    if (!IS_GUILD_GUID(guid))
        return;
    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        return;
    if (!(availability & ALL_WEEK) || availability > ALL_WEEK)
        return;
    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;

    MembershipRequest request = MembershipRequest(GetPlayer()->GetGUIDLow(), guildLowGuid, availability, classRoles, guildInterests, comment, time(NULL));
    sGuildFinderMgr->AddMembershipRequest(guildLowGuid, request);
}

void WorldSession::HandleGuildFinderBrowse(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_LF_GUILD_BROWSE");

    uint32 classRoles = 0;
    uint32 availability = 0;
    uint32 guildInterests = 0;
    uint32 playerLevel = 0;

    recvPacket >> playerLevel >> guildInterests >> availability >> classRoles;

    Player* player = GetPlayer();

    LFGuildPlayer settings(player->GetGUIDLow(), classRoles, availability, guildInterests, ANY_FINDER_LEVEL);
    LFGuildStore guildList = sGuildFinderMgr->GetGuildsMatchingSetting(settings, player->GetTeamId());
    uint32 guildCount = guildList.size();

    if (guildCount == 0)
    {
        WorldPacket packet(SMSG_LF_GUILD_BROWSE_UPDATED);
        packet.WriteBits(0, 18);
        packet.FlushBits();
        player->SendDirectMessage(&packet);
        return;
    }

    bool returned = false;

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        returned = true;
    if (!(availability & ALL_WEEK) || availability > ALL_WEEK)
        returned = true;
    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        returned = true;
    if (playerLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) || playerLevel < 1)
        returned = true;

    if (returned)
    {
        WorldPacket packet(SMSG_LF_GUILD_BROWSE_UPDATED);
        packet.WriteBits(0, 18);
        packet.FlushBits();
        player->SendDirectMessage(&packet);
        return;
    }

    ByteBuffer bufferData;
    WorldPacket data(SMSG_LF_GUILD_BROWSE_UPDATED);

    data.WriteBits(guildCount, 18);

    for (LFGuildStore::const_iterator itr = guildList.begin(); itr != guildList.end(); ++itr)
    {
        LFGuildSettings guildSettings = itr->second;
        Guild* guild = sGuildMgr->GetGuildById(itr->first);

        ObjectGuid guildGUID = ObjectGuid(guild->GetGUID());

        data.WriteBitSeq<4>(guildGUID);
        data.WriteBits(guild->GetName().size(), 7);
        data.WriteBitSeq<0, 1>(guildGUID);
        data.WriteBits(guildSettings.GetComment().size(), 10);
        data.WriteBitSeq<2, 7, 3, 6, 5>(guildGUID);

        bufferData << uint8(sGuildFinderMgr->HasRequest(player->GetGUIDLow(), guild->GetGUID()));
        bufferData << uint32(guild->GetLevel());
        bufferData << uint32(guildSettings.GetInterests());
        bufferData << uint32(0);                                                                    // Unk
        bufferData << uint32(guild->GetEmblemInfo().GetBorderStyle());
        bufferData.WriteByteSeq<1, 4>(guildGUID);
        bufferData << uint32(guild->GetEmblemInfo().GetStyle());
        bufferData.WriteByteSeq<3>(guildGUID);
        bufferData << uint32(50397223);                                                             // Unk Flags
        bufferData << uint32(guild->GetEmblemInfo().GetColor());
        bufferData.WriteString(guild->GetName());
        bufferData << uint32(guildSettings.GetClassRoles());
        bufferData << uint32(guildSettings.GetAvailability());
        bufferData.WriteString(guildSettings.GetComment());
        bufferData.WriteByteSeq<6, 7>(guildGUID);
        bufferData << uint8(0);                                                                     // Cached
        bufferData.WriteByteSeq<5, 0>(guildGUID);
        bufferData << uint32(guild->GetEmblemInfo().GetBorderColor());
        bufferData << uint32(guild->GetMembersCount());
        bufferData << uint32(guild->GetAchievementMgr().GetAchievementPoints());
        bufferData.WriteByteSeq<2>(guildGUID);
        bufferData << int32(guild->GetEmblemInfo().GetBackgroundColor());
    }

    data.FlushBits();
    data.append(bufferData);

    player->SendDirectMessage(&data);
}

void WorldSession::HandleGuildFinderDeclineRecruit(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_LF_GUILD_DECLINE_RECRUIT");

    ObjectGuid playerGuid;

    recvPacket.ReadBitSeq<3, 4, 0, 1, 6, 2, 5, 7>(playerGuid);
    recvPacket.ReadByteSeq<2, 6, 5, 1, 7, 0, 4, 3>(playerGuid);

    if (!IS_PLAYER_GUID(playerGuid))
        return;

    sGuildFinderMgr->RemoveMembershipRequest(GUID_LOPART(playerGuid), GetPlayer()->GetGuildId());
}

void WorldSession::HandleGuildFinderGetApplications(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_LF_GUILD_GET_APPLICATIONS");

    std::list<MembershipRequest> applicatedGuilds = sGuildFinderMgr->GetAllMembershipRequestsForPlayer(GetPlayer()->GetGUIDLow());
    uint32 applicationsCount = applicatedGuilds.size();

    WorldPacket data(SMSG_LF_GUILD_MEMBERSHIP_LIST_UPDATED);

    data << uint32(10 - sGuildFinderMgr->CountRequestsFromPlayer(GetPlayer()->GetGUIDLow())); // Applications count left

    data.WriteBits(applicationsCount, 19);

    if (applicationsCount > 0)
    {
        ByteBuffer bufferData;
        for (std::list<MembershipRequest>::const_iterator itr = applicatedGuilds.begin(); itr != applicatedGuilds.end(); ++itr)
        {
            Guild* guild = sGuildMgr->GetGuildById(itr->GetGuildId());

            if (!guild)
                continue;

            LFGuildSettings guildSettings = sGuildFinderMgr->GetGuildSettings(itr->GetGuildId());
            MembershipRequest request = *itr;

            ObjectGuid guildGuid = ObjectGuid(guild->GetGUID());

            data.WriteBits(request.GetComment().size(), 10);
            data.WriteBits(guild->GetName().size(), 7);
            data.WriteBitSeq<2, 1, 4, 0, 6, 3, 5, 7>(guildGuid);

            bufferData << uint32(50397223);                             // unk Flags
            bufferData.WriteByteSeq<1, 5, 6>(guildGuid);

            bufferData.WriteString(request.GetComment());

            bufferData.WriteByteSeq<0, 2>(guildGuid);
            bufferData << uint32(guildSettings.GetClassRoles());
            bufferData.WriteByteSeq<4>(guildGuid);
            bufferData << uint32(guildSettings.GetAvailability());

            bufferData.WriteString(guild->GetName());

            bufferData << uint32(time(NULL) - request.GetSubmitTime()); // Time since application (seconds)
            bufferData << uint32(guildSettings.GetInterests());
            bufferData << uint32(request.GetExpiryTime() - time(NULL)); // Time left to application expiry (seconds)
            bufferData.WriteByteSeq<7, 3>(guildGuid);
        }

        data.FlushBits();
        data.append(bufferData);
    }
    else
    {
        data.FlushBits();
    }

    GetPlayer()->SendDirectMessage(&data);
}

// Lists all recruits for a guild - Misses times
void WorldSession::HandleGuildFinderGetRecruits(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_LF_GUILD_GET_RECRUITS");

    uint32 unkTime = 0;
    recvPacket >> unkTime;

    Player* player = GetPlayer();
    if (!player->GetGuildId())
        return;

    std::vector<MembershipRequest> recruitsList = sGuildFinderMgr->GetAllMembershipRequestsForGuild(player->GetGuildId());
    uint32 recruitCount = recruitsList.size();

    ByteBuffer dataBuffer;
    WorldPacket data(SMSG_LF_GUILD_RECRUIT_LIST_UPDATED);

    data << uint32(time(NULL)); // Unk time
    data.WriteBits(recruitCount, 18);

    for (std::vector<MembershipRequest>::const_iterator itr = recruitsList.begin(); itr != recruitsList.end(); ++itr)
    {
        MembershipRequest request = *itr;
        ObjectGuid playerGuid(MAKE_NEW_GUID(request.GetPlayerGUID(), 0, HIGHGUID_PLAYER));

        uint8 _class, _level = 0;
        std::string name;
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_NAME_QUERY_SIMPLE);
        stmt->setUInt32(0, request.GetPlayerGUID());
        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            //name, race, gender, class, level
            Field* fields = result->Fetch();
            name = fields[0].GetString();
            _class = fields[3].GetUInt8();
            _level = fields[4].GetUInt8();
        }

        data.WriteBits(request.GetComment().size(), 10);
        data.WriteBitSeq<6, 3, 1, 4, 0, 5, 2>(playerGuid);
        data.WriteBits(name.size(), 6);
        data.WriteBitSeq<7>(playerGuid);

        dataBuffer << uint32(_class);
        dataBuffer << uint32(_level);
        dataBuffer << uint32(0);
        dataBuffer << uint32(time(NULL) <= request.GetExpiryTime());
        dataBuffer << uint32(request.GetAvailability());
        dataBuffer.WriteByteSeq<7>(playerGuid);
        dataBuffer << uint32(time(NULL) - request.GetSubmitTime()); // Time in seconds since application submitted.

        if (name.size() > 0)
            dataBuffer.append(name.c_str(), name.size());

        dataBuffer.WriteByteSeq<6>(playerGuid);

        if (name.size() > 0)
            dataBuffer.append(name.c_str(), name.size());

        dataBuffer << uint32(request.GetClassRoles());
        dataBuffer.WriteByteSeq<1>(playerGuid);
        dataBuffer << uint32(request.GetInterests());
        dataBuffer << uint32(request.GetExpiryTime() - time(NULL)); // TIme in seconds until application expires.
        dataBuffer.WriteByteSeq<2, 5, 0, 4, 3>(playerGuid);
    }

    data.FlushBits();
    data.append(dataBuffer);

    player->SendDirectMessage(&data);
}

void WorldSession::HandleGuildFinderPostRequest(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_LF_GUILD_POST_REQUEST"); // Empty opcode

    Player* player = GetPlayer();

    if (!player->GetGuildId()) // Player must be in guild
        return;

    bool isGuildMaster = true;
    if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
        if (guild->GetLeaderGUID() != player->GetGUID())
            isGuildMaster = false;

    LFGuildSettings settings = sGuildFinderMgr->GetGuildSettings(player->GetGuildId());

    WorldPacket data(SMSG_LF_GUILD_POST_UPDATED);
    data.WriteBit(isGuildMaster);

    if (isGuildMaster)
    {
        data.WriteBit(settings.IsListed());
        data.WriteBits(settings.GetComment().size(), 10);

        data << uint32(0); // Unk Int32

        if (settings.GetComment().size() > 0)
            data.append(settings.GetComment().c_str(), settings.GetComment().size());

        data << uint32(settings.GetAvailability());
        data << uint32(settings.GetClassRoles());
        data << uint32(settings.GetLevel());
        data << uint32(settings.GetInterests());
    }
    else
        data.FlushBits();

    player->SendDirectMessage(&data);
}

void WorldSession::HandleGuildFinderRemoveRecruit(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_LF_GUILD_REMOVE_RECRUIT");

    ObjectGuid guildGuid;

    recvPacket.ReadBitSeq<3, 1, 0, 7, 5, 4, 6, 2>(guildGuid);
    recvPacket.ReadByteSeq<2, 4, 0, 7, 6, 5, 1, 3>(guildGuid);

    if (!IS_GUILD_GUID(guildGuid))
        return;

    sGuildFinderMgr->RemoveMembershipRequest(GetPlayer()->GetGUIDLow(), GUID_LOPART(guildGuid));
}

// Sent any time a guild master sets an option in the interface and when listing / unlisting his guild
void WorldSession::HandleGuildFinderSetGuildPost(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_LF_GUILD_SET_GUILD_POST");

    uint32 classRoles = 0;
    uint32 availability = 0;
    uint32 guildInterests =  0;
    uint32 level = 0;

    recvPacket >> classRoles >> availability >> guildInterests >> level;
    bool listed = recvPacket.ReadBit();
    // Level sent is zero if untouched, force to any (from interface). Idk why
    if (!level)
        level = ANY_FINDER_LEVEL;

    uint16 length = recvPacket.ReadBits(10);
    std::string comment = recvPacket.ReadString(length);

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        return;
    if (!(availability & ALL_WEEK) || availability > ALL_WEEK)
        return;
    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;
    if (!(level & ALL_GUILDFINDER_LEVELS) || level > ALL_GUILDFINDER_LEVELS)
        return;

    Player* player = GetPlayer();

    if (!player->GetGuildId()) // Player must be in guild
        return;

    if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId())) // Player must be guild master
        if (guild->GetLeaderGUID() != player->GetGUID())
            return;

    LFGuildSettings settings(listed, player->GetTeamId(), player->GetGuildId(), classRoles, availability, guildInterests, level, comment);
    sGuildFinderMgr->SetGuildSettings(player->GetGuildId(), settings);
}