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
#include "Opcodes.h"
#include "Log.h"
#include "CUFProfiles.h"
#include "Player.h"
#include "GossipDef.h"
#include "World.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "WorldSession.h"
#include "BigNumber.h"
#include "SHA1.h"
#include "UpdateData.h"
#include "LootMgr.h"
#include "Chat.h"
#include "zlib.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "Battleground.h"
#include "OutdoorPvP.h"
#include "Pet.h"
#include "SocialMgr.h"
#include "CellImpl.h"
#include "AccountMgr.h"
#include "Vehicle.h"
#include "CreatureAI.h"
#include "DBCEnums.h"
#include "ScriptMgr.h"
#include "MapManager.h"
#include "InstanceScript.h"
#include "GameObjectAI.h"
#include "Group.h"
#include "AccountMgr.h"
#include "Spell.h"
#include "BattlegroundMgr.h"
#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "TicketMgr.h"
#include "SpellAuraEffects.h"

void WorldSession::HandleRepopRequestOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_REPOP_REQUEST Message");

    recvData.ReadBit();

    if (GetPlayer()->IsAlive() || GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    if (GetPlayer()->HasAuraType(SPELL_AURA_PREVENT_RESURRECTION))
        return; // silently return, client should display the error by itself

    // the world update order is sessions, players, creatures
    // the netcode runs in parallel with all of these
    // creatures can kill players
    // so if the server is lagging enough the player can
    // release spirit after he's killed but before he is updated
    if (GetPlayer()->getDeathState() == JUST_DIED)
    {
        TC_LOG_DEBUG("network", "HandleRepopRequestOpcode: got request after player %s(%d) was killed and before he was updated",
                     GetPlayer()->GetName().c_str(), GetPlayer()->GetGUIDLow());
        GetPlayer()->KillPlayer();
    }

    //this is spirit release confirm?
    GetPlayer()->setDeathState(DEAD);
    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}

void WorldSession::HandleGossipSelectOptionOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_GOSSIP_SELECT_OPTION");

    uint32 gossipListId;
    uint32 textId;
    ObjectGuid guid;
    uint32 codeLen = 0;
    std::string code = "";

    recvData >> gossipListId >> textId;

    recvData.ReadBitSeq<7, 6, 1>(guid);
    codeLen = recvData.ReadBits(8);
    recvData.ReadBitSeq<5, 2, 4, 3, 0>(guid);

    recvData.ReadByteSeq<1, 0, 6, 3, 7, 5, 2>(guid);
    code = recvData.ReadString(codeLen);
    recvData.ReadByteSeq<4>(guid);

    Creature* unit = NULL;
    GameObject* go = NULL;
    if (IS_CRE_OR_VEH_GUID(guid))
    {
        unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
        if (!unit)
        {
            TC_LOG_DEBUG("network", "WORLD: HandleGossipSelectOptionOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else if (IS_GAMEOBJECT_GUID(guid))
    {
        go = _player->GetMap()->GetGameObject(guid);
        if (!go)
        {
            TC_LOG_DEBUG("network", "WORLD: HandleGossipSelectOptionOpcode - GameObject (GUID: %u) not found.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGossipSelectOptionOpcode - unsupported GUID type for highguid %u. lowpart %u.", uint32(GUID_HIPART(guid)), uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if ((unit && unit->GetCreatureTemplate()->ScriptID != unit->LastUsedScriptID) || (go && go->GetGOInfo()->ScriptId != go->LastUsedScriptID))
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGossipSelectOptionOpcode - Script reloaded while in use, ignoring and set new scipt id");
        if (unit)
            unit->LastUsedScriptID = unit->GetCreatureTemplate()->ScriptID;
        if (go)
            go->LastUsedScriptID = go->GetGOInfo()->ScriptId;
        _player->PlayerTalkClass->SendCloseGossip();
        return;
    }

    uint32 menuId = _player->PlayerTalkClass->GetGossipMenu().GetMenuId();

    if (!code.empty())
    {
        if (unit)
        {
            unit->AI()->sGossipSelectCode(_player, menuId, gossipListId, code.c_str());
            if (!sScriptMgr->OnGossipSelectCode(_player, unit, _player->PlayerTalkClass->GetGossipOptionSender(gossipListId), _player->PlayerTalkClass->GetGossipOptionAction(gossipListId), code.c_str()))
                _player->OnGossipSelect(unit, gossipListId, menuId);
        }
        else
        {
            go->AI()->GossipSelectCode(_player, menuId, gossipListId, code.c_str());
            sScriptMgr->OnGossipSelectCode(_player, go, _player->PlayerTalkClass->GetGossipOptionSender(gossipListId), _player->PlayerTalkClass->GetGossipOptionAction(gossipListId), code.c_str());
        }
    }
    else
    {
        if (unit)
        {
            unit->AI()->sGossipSelect(_player, menuId, gossipListId);
            if (!sScriptMgr->OnGossipSelect(_player, unit, _player->PlayerTalkClass->GetGossipOptionSender(gossipListId), _player->PlayerTalkClass->GetGossipOptionAction(gossipListId)))
                _player->OnGossipSelect(unit, gossipListId, menuId);
        }
        else
        {
            go->AI()->GossipSelect(_player, menuId, gossipListId);
            if (!sScriptMgr->OnGossipSelect(_player, go, _player->PlayerTalkClass->GetGossipOptionSender(gossipListId), _player->PlayerTalkClass->GetGossipOptionAction(gossipListId)))
                _player->OnGossipSelect(go, gossipListId, menuId);
        }
    }
}

void WorldSession::HandleWhoOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_WHO Message");

    if (!AccountMgr::IsAdminAccount(GetSecurity()))
    {
        time_t now = time(NULL);
        if (now - timeLastWhoCommand < sWorld->getIntConfig(CONFIG_WHO_OPCODE_INTERVAL))
            return;
        else
            timeLastWhoCommand = now;
    }

    uint32 matchcount = 0;
    uint32 level_min, level_max, racemask, classmask, zones_count, str_count;
    uint32 zoneids[10]; // 10 is client limit
    bool bit740;
    uint8 playerLen = 0, guildLen = 0;
    uint8 unkLen2, unkLen3;
    std::string player_name, guild_name;
    recvData >> classmask; // class mask
    recvData >> level_max; // minimal player level, default 100 (MAX_LEVEL)
    recvData >> level_min; // maximal player level, default 0
    recvData >> racemask; // race mask
    guildLen = recvData.ReadBits(7);
    playerLen = recvData.ReadBits(6);
    str_count = recvData.ReadBits(3);
    if (str_count > 4) // can't be received from real client or broken packet
        return;
    recvData.ReadBit();
    recvData.ReadBit();
    zones_count = recvData.ReadBits(4); // zones count, client limit = 10 (2.0.10)
    if (zones_count > 10) // can't be received from real client or broken packet
        return;
    unkLen2 = recvData.ReadBits(9);
    unkLen3 = recvData.ReadBits(9);
    recvData.ReadBit();

    std::vector<uint8> unkLens(str_count);
    for (uint8 i = 0; i < str_count; i++)
        unkLens[i] = recvData.ReadBits(7);

    // std::vector<std::string> unkStrings(str_count);

    bit740 = recvData.ReadBit();

    if (playerLen > 0)
        player_name = recvData.ReadString(playerLen); // player name, case sensitive...

    std::wstring str[4];                                    // 4 is client limit
    for (uint32 i = 0; i < str_count; ++i)
    {
        std::string temp = recvData.ReadString(unkLens[i]); // user entered string, it used as universal search pattern(guild+player name)?
        if (!Utf8toWStr(temp, str[i]))
            continue;

        wstrToLower(str[i]);
        TC_LOG_DEBUG("network", "String %u: %s", i, temp.c_str());
    }

    if (guildLen > 0)
        guild_name = recvData.ReadString(guildLen); // guild name, case sensitive...

    for (uint32 i = 0; i < zones_count; ++i)
    {
        uint32 temp;
        recvData >> temp; // zone id, 0 if zone is unknown...
        zoneids[i] = temp;
        TC_LOG_DEBUG("network", "Zone %u: %u", i, zoneids[i]);
    }
    if (unkLen3 > 0)
    {
        std::string unkString = recvData.ReadString(unkLen3);
    }
    if (unkLen2 > 0)
    {
        std::string unkString = recvData.ReadString(unkLen2);
    }
    if (bit740)
    {
        uint32 unk1, unk2, unk3;
        recvData >> unk1 >> unk2 >> unk3;
    }
    TC_LOG_DEBUG("network", "Minlvl %u, maxlvl %u, name %s, guild %s, racemask %u, classmask %u, zones %u, strings %u", level_min, level_max, player_name.c_str(), guild_name.c_str(), racemask, classmask, zones_count, str_count);
    std::wstring wplayer_name;
    std::wstring wguild_name;
    if (!(Utf8toWStr(player_name, wplayer_name) && Utf8toWStr(guild_name, wguild_name)))
        return;

    wstrToLower(wplayer_name);
    wstrToLower(wguild_name);
    // client send in case not set max level value 100 but Trinity supports 255 max level,
    // update it to show GMs with characters after 100 level
    if (level_max >= MAX_LEVEL)
        level_max = STRONG_MAX_LEVEL;

    uint32 team = _player->GetTeam();
    uint32 security = GetSecurity();
    bool allowTwoSideWhoList = sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
    uint32 gmLevelInWhoList = sWorld->getIntConfig(CONFIG_GM_LEVEL_IN_WHO_LIST);
    uint8 displaycount = 0;

    ByteBuffer bitsData;
    ByteBuffer bytesData;

    bitsData.WriteBits(0, 6);

    HashMapHolder<Player>::ReadGuardType guard(HashMapHolder<Player>::GetLock());

    for (auto const &kvPair : sObjectAccessor->GetPlayers())
    {
        Player const * const target = kvPair.second;

        if (AccountMgr::IsPlayerAccount(security))
        {
            // player can see member of other team only if CONFIG_ALLOW_TWO_SIDE_WHO_LIST
            if (target->GetTeam() != team && !allowTwoSideWhoList)
                continue;

            // player can see MODERATOR, GAME MASTER, ADMINISTRATOR only if CONFIG_GM_IN_WHO_LIST
            if (target->GetSession()->GetSecurity() > AccountTypes(gmLevelInWhoList))
                continue;
        }

        //do not process players which are not in world
        if (!target->IsInWorld())
            continue;

        // check if target is globally visible for player
        if (!target->IsVisibleGloballyFor(_player))
            continue;

        // check if target's level is in level range
        uint8 lvl = target->getLevel();
        if (lvl < level_min || lvl > level_max)
            continue;

        // check if class matches classmask
        uint8 class_ = target->getClass();
        if (!(classmask & (1 << class_)))
            continue;

        // check if race matches racemask
        uint32 race = target->getRace();
        if (!(racemask & (1 << race)))
            continue;

        uint32 pzoneid = target->GetZoneId();
        uint8 gender = target->getGender();

        bool z_show = true;
        for (uint32 i = 0; i < zones_count; ++i)
        {
            if (zoneids[i] == pzoneid)
            {
                z_show = true;
                break;
            }

            z_show = false;
        }
        if (!z_show)
            continue;

        std::string pname = target->GetName();
        std::wstring wpname;
        if (!Utf8toWStr(pname, wpname))
            continue;
        wstrToLower(wpname);

        if (!(wplayer_name.empty() || wpname.find(wplayer_name) != std::wstring::npos))
            continue;

        std::string gname = sGuildMgr->GetGuildNameById(target->GetGuildId());
        std::wstring wgname;
        if (!Utf8toWStr(gname, wgname))
            continue;
        wstrToLower(wgname);

        if (!(wguild_name.empty() || wgname.find(wguild_name) != std::wstring::npos))
            continue;

        std::string aname;
        if (AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(pzoneid))
            aname = areaEntry->area_name[GetSessionDbcLocale()];

        bool s_show = true;
        for (uint32 i = 0; i < str_count; ++i)
        {
            if (!str[i].empty())
            {
                if (wgname.find(str[i]) != std::wstring::npos ||
                    wpname.find(str[i]) != std::wstring::npos ||
                    Utf8FitTo(aname, str[i]))
                {
                    s_show = true;
                    break;
                }
                s_show = false;
            }
        }
        if (!s_show)
            continue;

        // 49 is maximum player count sent to client - can be overridden
        // through config, but is unstable
        if (matchcount++ >= sWorld->getIntConfig(CONFIG_MAX_WHO))
            break;

        ObjectGuid playerGuid = target->GetGUID();
        ObjectGuid unkGuid = 0;
        ObjectGuid guildGuid = target->GetGuild() ? target->GetGuild()->GetGUID() : 0;

        bitsData.WriteBitSeq<4>(guildGuid);

        if (DeclinedName const* names = target->GetDeclinedNames())
        {
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                bitsData.WriteBits(names->name[i].size(), 7);
        }
        else
        {
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                bitsData.WriteBits(0, 7);
        }

        bitsData.WriteBits(gname.size(), 7);
        bitsData.WriteBitSeq<3>(unkGuid);
        bitsData.WriteBitSeq<1>(playerGuid);
        bitsData.WriteBits(pname.size(), 6);
        bitsData.WriteBitSeq<2>(guildGuid);
        bitsData.WriteBitSeq<7>(playerGuid);
        bitsData.WriteBitSeq<2>(unkGuid);
        bitsData.WriteBitSeq<6, 7>(guildGuid);
        bitsData.WriteBitSeq<7>(unkGuid);
        bitsData.WriteBitSeq<3>(playerGuid);
        bitsData.WriteBitSeq<4>(unkGuid);
        bitsData.WriteBit(0); // 0
        bitsData.WriteBitSeq<1, 0>(unkGuid);
        bitsData.WriteBitSeq<4>(playerGuid);
        bitsData.WriteBit(0); // 532
        bitsData.WriteBitSeq<1, 3>(guildGuid);
        bitsData.WriteBitSeq<6>(unkGuid);
        bitsData.WriteBitSeq<0>(guildGuid);
        bitsData.WriteBitSeq<2, 5, 6>(playerGuid);
        bitsData.WriteBitSeq<5>(guildGuid);
        bitsData.WriteBitSeq<5>(unkGuid);
        bitsData.WriteBitSeq<0>(playerGuid);

        bytesData.WriteByteSeq<7>(playerGuid);
        bytesData.WriteByteSeq<4, 1>(guildGuid);
        bytesData << uint32(38297239);
        bytesData.WriteByteSeq<0>(playerGuid);
        bytesData.WriteString(pname);
        bytesData.WriteByteSeq<0>(unkGuid);
        bytesData.WriteByteSeq<6>(guildGuid);
        bytesData << uint32(pzoneid);
        bytesData.WriteByteSeq<7, 5>(unkGuid);
        bytesData.WriteByteSeq<3, 6>(playerGuid);
        bytesData << uint32(realmID);
        bytesData << uint8(race);
        bytesData << uint8(lvl);
        bytesData.WriteByteSeq<2, 1>(playerGuid);
        bytesData << uint8(gender);
        bytesData << uint32(realmID);
        bytesData.WriteByteSeq<0, 5, 7>(guildGuid);

        if (DeclinedName const* names = target->GetDeclinedNames())
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                bytesData.WriteString(names->name[i]);

        bytesData.WriteByteSeq<5>(playerGuid);
        bytesData.WriteByteSeq<3>(guildGuid);
        bytesData.WriteByteSeq<4>(playerGuid);
        bytesData.WriteString(gname);

        bytesData << uint8(class_);
        bytesData.WriteByteSeq<4>(unkGuid);
        bytesData.WriteByteSeq<2>(guildGuid);
        bytesData.WriteByteSeq<3, 6, 2, 1>(unkGuid);
        ++displaycount;
    }

    WorldPacket data(SMSG_WHO);
    if (displaycount != 0)
    {
        bitsData.FlushBits();
        uint8 firstByte = bitsData.contents()[0];
        data << uint8((displaycount << 2) | (firstByte & 0x3));
        for (size_t i = 1; i < bitsData.size(); i++)
            data << uint8(bitsData.contents()[i]);
        data.append(bytesData);
    }
    else
    {
        data.WriteBits(0, 6);
        data.FlushBits();
    }

    SendPacket(&data);

    TC_LOG_DEBUG("network", "WORLD: Send SMSG_WHO Message");
}
void WorldSession::HandleLogoutRequestOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_LOGOUT_REQUEST Message, security - %u", GetSecurity());

    if (uint64 lguid = GetPlayer()->GetLootGUID())
        DoLootRelease(lguid);

    bool instantLogout = (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) && !GetPlayer()->IsInCombat()) ||
            GetPlayer()->isInFlight() || GetSecurity() >= AccountTypes(sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT));

    /// TODO: Possibly add RBAC permission to log out in combat
    bool canLogoutInCombat = GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);

    uint32 reason = 0;
    if (GetPlayer()->IsInCombat() && !canLogoutInCombat)
        reason = 1;
    else if (GetPlayer()->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR))
        reason = 3;                                         // is jumping or falling
    else if (GetPlayer()->duel || GetPlayer()->HasAura(9454)) // is dueling or frozen by GM via freeze command
        reason = 2;                                         // FIXME - Need the correct value

    WorldPacket data(SMSG_LOGOUT_RESPONSE, 1+4);
    data << uint32(reason);
    data.WriteBit(instantLogout);
    data.FlushBits();
    SendPacket(&data);

    if (reason)
    {
        LogoutRequest(0);
        return;
    }

    //instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in worldserver.conf
    if (instantLogout)
    {
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if (GetPlayer()->CanFreeMove())
    {
        GetPlayer()->SetStandState(UNIT_STAND_STATE_SIT);
        GetPlayer()->SetRooted(true);
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    LogoutRequest(time(NULL));
}

void WorldSession::HandlePlayerLogoutOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_PLAYER_LOGOUT Message");
}

void WorldSession::HandleLogoutCancelOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_LOGOUT_CANCEL Message");

    // Player have already logged out serverside, too late to cancel
    if (!GetPlayer())
        return;

    LogoutRequest(0);

    WorldPacket data(SMSG_LOGOUT_CANCEL_ACK, 0);
    SendPacket(&data);

    // not remove flags if can't free move - its not set in Logout request code.
    if (GetPlayer()->CanFreeMove())
    {
        //!we can move again
        GetPlayer()->SetRooted(false);

        //! Stand Up
        GetPlayer()->SetStandState(UNIT_STAND_STATE_STAND);

        //! DISABLE_ROTATE
        GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    GetPlayer()->PetSpellInitialize();

    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_LOGOUT_CANCEL_ACK Message");
}

void WorldSession::HandleTogglePvP(WorldPacket& recvData)
{
    // this opcode can be used in two ways: Either set explicit new status or toggle old status
    if (recvData.size() == 1)
    {
        bool newPvPStatus = recvData.ReadBit();

        GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP, newPvPStatus);
        GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_PVP_TIMER, !newPvPStatus);
    }
    else
    {
        GetPlayer()->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);
        GetPlayer()->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_PVP_TIMER);
    }

    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
    {
        if (!GetPlayer()->IsPvP() || GetPlayer()->pvpInfo.endTimer != 0)
            GetPlayer()->UpdatePvP(true, true);
    }
    else
    {
        if (!GetPlayer()->pvpInfo.inHostileArea && GetPlayer()->IsPvP())
            GetPlayer()->pvpInfo.endTimer = time(NULL);     // start toggle-off
    }

    //if (OutdoorPvP* pvp = _player->GetOutdoorPvP())
    //    pvp->HandlePlayerActivityChanged(_player);
}

void WorldSession::HandleZoneUpdateOpcode(WorldPacket& recvData)
{
    uint32 newZone;
    recvData >> newZone;

    TC_LOG_DEBUG("network", "WORLD: Recvd ZONE_UPDATE: %u", newZone);

    // use server size data
    uint32 newzone, newarea;
    GetPlayer()->GetZoneAndAreaId(newzone, newarea);
    GetPlayer()->UpdateZone(newzone, newarea);
    //GetPlayer()->SendInitWorldStates(true, newZone);
}

void WorldSession::HandleReturnToGraveyard(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->IsAlive() || !GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;
    //TODO: unk32, unk32
    GetPlayer()->RepopAtGraveyard();
}

void WorldSession::HandleSetSelectionOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<1, 3, 4, 6, 0, 5, 7, 2>(guid);
    recvData.ReadByteSeq<7, 6, 0, 2, 3, 1, 4, 5>(guid);

    _player->SetSelection(guid);
}

void WorldSession::HandleStandStateChangeOpcode(WorldPacket& recvData)
{
    // TC_LOG_DEBUG("network", "WORLD: Received CMSG_STANDSTATECHANGE"); -- too many spam in log at lags/debug stop
    uint32 animstate;
    recvData >> animstate;

    _player->SetStandState(animstate);
}

void WorldSession::HandleContactListOpcode(WorldPacket& recvData)
{
    recvData.read_skip<uint32>(); // always 1
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_CONTACT_LIST");
    _player->GetSocial()->SendSocialList(_player);
}

void WorldSession::HandleAddFriendOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_ADD_FRIEND");

    std::string friendName = GetTrinityString(LANG_FRIEND_IGNORE_UNKNOWN);
    std::string friendNote;

    recvData >> friendName;

    recvData >> friendNote;

    if (!normalizePlayerName(friendName))
        return;

    TC_LOG_DEBUG("network", "WORLD: %s asked to add friend : '%s'", GetPlayer()->GetName().c_str(), friendName.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUID_RACE_ACC_BY_NAME);

    stmt->setString(0, friendName);

    _addFriendCallback.SetParam(friendNote);
    _addFriendCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleAddFriendOpcodeCallBack(PreparedQueryResult result, std::string friendNote)
{
    if (!GetPlayer())
        return;

    uint64 friendGuid;
    uint32 friendAccountId;
    uint32 team;
    FriendsResult friendResult;

    friendResult = FRIEND_NOT_FOUND;
    friendGuid = 0;

    if (result)
    {
        Field* fields = result->Fetch();

        uint32 lowGuid = fields[0].GetUInt32();
        friendGuid = MAKE_NEW_GUID(lowGuid, 0, HIGHGUID_PLAYER);

        team = Player::TeamForRace(fields[1].GetUInt8());
        friendAccountId = fields[2].GetUInt32();

        if (!AccountMgr::IsPlayerAccount(GetSecurity()) || sWorld->getBoolConfig(CONFIG_ALLOW_GM_FRIEND) || AccountMgr::IsPlayerAccount(AccountMgr::GetSecurity(friendAccountId, realmID)))
        {
            if (friendGuid)
            {
                if (friendGuid == GetPlayer()->GetGUID())
                    friendResult = FRIEND_SELF;
                else if (GetPlayer()->GetTeam() != team && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND) && AccountMgr::IsPlayerAccount(GetSecurity()))
                    friendResult = FRIEND_ENEMY;
                else if (GetPlayer()->GetSocial()->HasFriend(lowGuid))
                    friendResult = FRIEND_ALREADY;
                else
                {
                    Player* pFriend = ObjectAccessor::FindPlayer(friendGuid);
                    if (pFriend && pFriend->IsInWorld() && pFriend->IsVisibleGloballyFor(GetPlayer()))
                        friendResult = FRIEND_ADDED_ONLINE;
                    else
                        friendResult = FRIEND_ADDED_OFFLINE;
                    if (!GetPlayer()->GetSocial()->AddToSocialList(lowGuid, false))
                    {
                        friendResult = FRIEND_LIST_FULL;
                        TC_LOG_DEBUG("network", "WORLD: %s's friend list is full.", GetPlayer()->GetName().c_str());
                    }
                }
                GetPlayer()->GetSocial()->SetFriendNote(lowGuid, friendNote);
            }
        }
    }

    sSocialMgr->SendFriendStatus(GetPlayer(), friendResult, friendGuid, false);

    TC_LOG_DEBUG("network", "WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelFriendOpcode(WorldPacket& recvData)
{
    uint64 FriendGUID;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_DEL_FRIEND");

    recvData >> FriendGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(FriendGUID), false);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_REMOVED, FriendGUID, false);

    TC_LOG_DEBUG("network", "WORLD: Sent motd (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleAddIgnoreOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_ADD_IGNORE");

    std::string ignoreName = GetTrinityString(LANG_FRIEND_IGNORE_UNKNOWN);

    recvData >> ignoreName;

    if (!normalizePlayerName(ignoreName))
        return;

    TC_LOG_DEBUG("network", "WORLD: %s asked to Ignore: '%s'", GetPlayer()->GetName().c_str(), ignoreName.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUID_BY_NAME);

    stmt->setString(0, ignoreName);

    _addIgnoreCallback = CharacterDatabase.AsyncQuery(stmt);
}

void WorldSession::HandleAddIgnoreOpcodeCallBack(PreparedQueryResult result)
{
    if (!GetPlayer())
        return;

    uint64 IgnoreGuid;
    FriendsResult ignoreResult;

    ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    IgnoreGuid = 0;

    if (result)
    {
        uint32 lowGuid = (*result)[0].GetUInt32();
        IgnoreGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);

        if (IgnoreGuid)
        {
            if (IgnoreGuid == GetPlayer()->GetGUID())              //not add yourself
                ignoreResult = FRIEND_IGNORE_SELF;
            else if (GetPlayer()->GetSocial()->HasIgnore(lowGuid))
                ignoreResult = FRIEND_IGNORE_ALREADY;
            else
            {
                ignoreResult = FRIEND_IGNORE_ADDED;

                // ignore list full
                if (!GetPlayer()->GetSocial()->AddToSocialList(lowGuid, true))
                    ignoreResult = FRIEND_IGNORE_FULL;
            }
        }
    }

    sSocialMgr->SendFriendStatus(GetPlayer(), ignoreResult, IgnoreGuid, false);

    TC_LOG_DEBUG("network", "WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelIgnoreOpcode(WorldPacket& recvData)
{
    uint64 IgnoreGUID;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_DEL_IGNORE");

    recvData >> IgnoreGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(IgnoreGUID), true);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_IGNORE_REMOVED, IgnoreGUID, false);

    TC_LOG_DEBUG("network", "WORLD: Sent motd (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleSetContactNotesOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_SET_CONTACT_NOTES");
    uint64 guid;
    std::string note;
    recvData >> guid >> note;
    _player->GetSocial()->SetFriendNote(GUID_LOPART(guid), note);
}

void WorldSession::HandleReportBugOpcode(WorldPacket& recvData)
{
    float posX, posY, posZ, orientation;
    uint32 contentlen, mapId;
    std::string content;

    recvData >> posX >> posY >> orientation >> posZ;
    recvData >> mapId;

    contentlen = recvData.ReadBits(10);
    recvData.FlushBits();
    content = recvData.ReadString(contentlen);

    TC_LOG_DEBUG("network", "%s", content.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_BUG_REPORT);
    stmt->setString(0, "Bug");
    stmt->setString(1, content);
    CharacterDatabase.Execute(stmt);
}

void WorldSession::HandleReportSuggestionOpcode(WorldPacket& recvData)
{
    float posX, posY, posZ, orientation;
    uint32 contentlen, mapId;
    std::string content;

    recvData >> mapId;
    recvData >> posZ >> orientation >> posY >> posX;

    contentlen = recvData.ReadBits(10);
    recvData.FlushBits();
    content = recvData.ReadString(contentlen);

    TC_LOG_DEBUG("network", "%s", content.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_BUG_REPORT);
    stmt->setString(0, "Suggestion");
    stmt->setString(1, content);
    CharacterDatabase.Execute(stmt);
}

void WorldSession::HandleRequestBattlePetJournal(WorldPacket& /*recvPacket*/)
{
    GetPlayer()->GetBattlePetMgr().SendBattlePetJournal();
}

void WorldSession::HandleRequestGmTicket(WorldPacket& /*recvPakcet*/)
{
    // Notify player if he has a ticket in progress
    if (GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        if (ticket->IsCompleted())
            ticket->SendResponse(this);
        else
            sTicketMgr->SendTicket(this, ticket);
    }
}

void WorldSession::HandleReclaimCorpseOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_RECLAIM_CORPSE");

    ObjectGuid guid;

    recvData.ReadBitSeq<0, 1, 2, 6, 5, 7, 3, 4>(guid);
    recvData.ReadByteSeq<4, 1, 6, 7, 2, 3, 5, 0>(guid);

    if (GetPlayer()->IsAlive())
        return;

    // do not allow corpse reclaim in arena
    if (GetPlayer()->InArena())
        return;

    // body not released yet
    if (!GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    Corpse* corpse = GetPlayer()->GetCorpse();

    if (!corpse)
        return;

    // prevent resurrect before 30-sec delay after body release not finished
    if (time_t(corpse->GetGhostTime() + GetPlayer()->GetCorpseReclaimDelay(corpse->GetType() == CORPSE_RESURRECTABLE_PVP)) > time_t(time(NULL)))
        return;

    if (!corpse->IsWithinDistInMap(GetPlayer(), CORPSE_RECLAIM_RADIUS, true))
        return;

    // resurrect
    GetPlayer()->ResurrectPlayer(GetPlayer()->InBattleground() ? 1.0f : 0.5f);

    // spawn bones
    GetPlayer()->SpawnCorpseBones();
}

void WorldSession::HandleResurrectResponseOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_RESURRECT_RESPONSE");

    uint32 status;
    recvData >> status;

    ObjectGuid guid;

    recvData.ReadBitSeq<3, 4, 1, 5, 2, 0, 7, 6>(guid);
    recvData.ReadByteSeq<0, 6, 4, 5, 3, 1, 2, 7>(guid);

    if (GetPlayer()->IsAlive())
        return;

    if (status == 1)
    {
        GetPlayer()->ClearResurrectRequestData();           // reject
        return;
    }

    if (!GetPlayer()->IsRessurectRequestedBy(guid))
        return;

    GetPlayer()->ResurectUsingRequestData();
}

void WorldSession::SendAreaTriggerMessage(const char* Text, ...)
{
    va_list ap;
    char szStr [1024];
    szStr[0] = '\0';

    va_start(ap, Text);
    vsnprintf(szStr, 1024, Text, ap);
    va_end(ap);

    std::stringstream lel;
    lel << "|cffffff00" << szStr;
    SendNotification("%s", lel.str().c_str());
    /*uint32 length = strlen(szStr)+1;
    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 4+length);
    data << length;
    data << szStr;
    SendPacket(&data);*/
}

void WorldSession::HandleAreaTriggerOpcode(WorldPacket& recvData)
{
    uint32 triggerId;

    recvData >> triggerId;
    recvData.ReadBit();
    recvData.ReadBit();

    TC_LOG_DEBUG("network", "CMSG_AREATRIGGER. Trigger ID: %u", triggerId);

    Player* player = GetPlayer();
    if (player->isInFlight())
    {
        TC_LOG_DEBUG("network", "HandleAreaTriggerOpcode: Player '%s' (GUID: %u) in flight, ignore Area Trigger ID:%u",
            player->GetName().c_str(), player->GetGUIDLow(), triggerId);
        return;
    }

    AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(triggerId);
    if (!atEntry)
    {
        TC_LOG_DEBUG("network", "HandleAreaTriggerOpcode: Player '%s' (GUID: %u) send unknown (by DBC) Area Trigger ID:%u",
            player->GetName().c_str(), player->GetGUIDLow(), triggerId);
        return;
    }

    if (player->GetMapId() != atEntry->mapid)
    {
        TC_LOG_DEBUG("network", "HandleAreaTriggerOpcode: Player '%s' (GUID: %u) too far (trigger map: %u player map: %u), ignore Area Trigger ID: %u",
            player->GetName().c_str(), atEntry->mapid, player->GetMapId(), player->GetGUIDLow(), triggerId);
        return;
    }

    // delta is safe radius
    const float delta = 5.0f;

    if (atEntry->radius > 0)
    {
        // if we have radius check it
        float dist = player->GetDistance(atEntry->x, atEntry->y, atEntry->z);
        if (dist > atEntry->radius + delta)
        {
            TC_LOG_DEBUG("network", "HandleAreaTriggerOpcode: Player '%s' (GUID: %u) too far (radius: %f distance: %f), ignore Area Trigger ID: %u",
                player->GetName().c_str(), player->GetGUIDLow(), atEntry->radius, dist, triggerId);
            return;
        }
    }
    else
    {
        // we have only extent

        // rotate the players position instead of rotating the whole cube, that way we can make a simplified
        // is-in-cube check and we have to calculate only one point instead of 4

        // 2PI = 360°, keep in mind that ingame orientation is counter-clockwise
        double rotation = 2 * M_PI - atEntry->box_orientation;
        double sinVal = std::sin(rotation);
        double cosVal = std::cos(rotation);

        float playerBoxDistX = player->GetPositionX() - atEntry->x;
        float playerBoxDistY = player->GetPositionY() - atEntry->y;

        float rotPlayerX = float(atEntry->x + playerBoxDistX * cosVal - playerBoxDistY*sinVal);
        float rotPlayerY = float(atEntry->y + playerBoxDistY * cosVal + playerBoxDistX*sinVal);

        // box edges are parallel to coordiante axis, so we can treat every dimension independently :D
        float dz = player->GetPositionZ() - atEntry->z;
        float dx = rotPlayerX - atEntry->x;
        float dy = rotPlayerY - atEntry->y;
        if ((fabs(dx) > atEntry->box_x / 2 + delta) ||
            (fabs(dy) > atEntry->box_y / 2 + delta) ||
            (fabs(dz) > atEntry->box_z / 2 + delta))
        {
            TC_LOG_DEBUG("network", "HandleAreaTriggerOpcode: Player '%s' (GUID: %u) too far (1/2 box X: %f 1/2 box Y: %f 1/2 box Z: %f rotatedPlayerX: %f rotatedPlayerY: %f dZ:%f), ignore Area Trigger ID: %u",
                player->GetName().c_str(), player->GetGUIDLow(), atEntry->box_x/2, atEntry->box_y/2, atEntry->box_z/2, rotPlayerX, rotPlayerY, dz, triggerId);
            return;
        }
    }

    if (player->isDebugAreaTriggers)
        ChatHandler(player).PSendSysMessage(LANG_DEBUG_AREATRIGGER_REACHED, triggerId);

    if (sScriptMgr->OnAreaTrigger(player, atEntry))
        return;

    if (player->IsAlive())
        if (uint32 questId = sObjectMgr->GetQuestForAreaTrigger(triggerId))
            if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(questId);

    if (sObjectMgr->IsTavernAreaTrigger(triggerId))
    {
        // set resting flag we are in the inn
        player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        player->InnEnter(time(NULL), atEntry->mapid, atEntry->x, atEntry->y, atEntry->z);
        player->SetRestType(REST_TYPE_IN_TAVERN);

        if (sWorld->IsFFAPvPRealm())
            player->RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);

        return;
    }

    if (Battleground* bg = player->GetBattleground())
        if (bg->GetStatus() == STATUS_IN_PROGRESS)
        {
            bg->HandleAreaTrigger(player, triggerId);
            return;
        }

        if (OutdoorPvP* pvp = player->GetOutdoorPvP())
            if (pvp->HandleAreaTrigger(_player, triggerId))
                return;

        AreaTriggerStruct const* at = sObjectMgr->GetAreaTrigger(triggerId);
        if (!at)
            return;

        bool teleported = false;
        if (player->GetMapId() != at->target_mapId)
        {
            if (!sMapMgr->CanPlayerEnter(at->target_mapId, player, false))
                return;

            if (Group* group = player->GetGroup())
                if (group->isLFGGroup() && player->GetMap()->IsDungeon())
                    teleported = player->TeleportToBGEntryPoint();
        }

        if (!teleported)
            player->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation, TELE_TO_NOT_LEAVE_TRANSPORT);
}

void WorldSession::HandleUpdateAccountData(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_UPDATE_ACCOUNT_DATA");

    uint32 type, timestamp, decompressedSize, compressedSize;
    recvData >> decompressedSize >> timestamp >> compressedSize;

    type = uint8(recvData.contents()[recvData.size()-1]) >> 5;

    TC_LOG_DEBUG("network", "UAD: type %u, time %u, decompressedSize %u", type, timestamp, decompressedSize);

    if (decompressedSize == 0)                               // erase
    {
        SetAccountData(AccountDataType(type), 0, "");
        return;
    }

    if (decompressedSize > 0xFFFF)
    {
        recvData.rfinish();                   // unnneded warning spam in this case
        TC_LOG_ERROR("network", "UAD: Account data packet too big, size %u", decompressedSize);
        return;
    }

    ByteBuffer dest;
    dest.resize(decompressedSize);

    uLongf realSize = decompressedSize;
    if (uncompress(dest.contents(), &realSize, recvData.contents() + recvData.rpos(), recvData.size() - recvData.rpos()) != Z_OK)
    {
        recvData.rfinish();                   // unnneded warning spam in this case
        TC_LOG_ERROR("network", "UAD: Failed to decompress account data");
        return;
    }

    recvData.rfinish();                       // uncompress read (recvData.size() - recvData.rpos())

    std::string adata = dest.ReadString(decompressedSize);

    SetAccountData(AccountDataType(type), timestamp, adata);
}

void WorldSession::HandleRequestAccountData(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");

    uint32 type = recvData.ReadBits(3);

    TC_LOG_DEBUG("network", "RAD: type %u", type);

    if (type > NUM_ACCOUNT_DATA_TYPES)
        return;

    AccountData* adata = GetAccountData(AccountDataType(type));

    uint32 size = adata->Data.size();

    uLongf destSize = compressBound(size);

    ByteBuffer dest;
    dest.resize(destSize);

    if (size && compress(dest.contents(), &destSize, (uint8*)adata->Data.c_str(), size) != Z_OK)
    {
        TC_LOG_DEBUG("network", "RAD: Failed to compress account data");
        return;
    }

    dest.resize(destSize);

    WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA, 4+4+4+3+3+5+8+destSize);
    ObjectGuid playerGuid = _player ? _player->GetGUID() : 0;

    data << uint32(size);                                   // decompressed length
    data << uint32(adata->Time);                            // unix time
    data << uint32(destSize);                               // compressed length
    data.append(dest);                                      // compressed data

    data.WriteBitSeq<4, 2, 0>(playerGuid);
    data.WriteBits(type, 3);
    data.WriteBitSeq<7, 5, 1, 3, 6>(playerGuid);
    data.FlushBits();

    data.WriteByteSeq<4, 2, 7, 5, 3, 1, 6, 0>(playerGuid);

    SendPacket(&data);
}

int32 WorldSession::HandleEnableNagleAlgorithm()
{
    // Instructs the server we wish to receive few amounts of large packets (SMSG_MULTIPLE_PACKETS?)
    // instead of large amount of small packets
    return 0;
}

void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_SET_ACTION_BUTTON");
    uint8 button;

    recvData >> button;

    ObjectGuid packetData;

    recvData.ReadBitSeq<1, 7, 6, 5, 2, 4, 0, 3>(packetData);
    recvData.ReadByteSeq<2, 7, 1, 4, 0, 5, 3, 6>(packetData);

    uint32 action = uint64(packetData) & 0xFFFFFFFF;
    uint8  type   = (uint64(packetData) & 0xFF00000000000000) >> 56;

    TC_LOG_INFO("network", "BUTTON: %u ACTION: %u TYPE: %u", button, action, type);
    if (!packetData)
    {
        TC_LOG_INFO("network", "MISC: Remove action from button %u", button);
        GetPlayer()->removeActionButton(button);
    }
    else
    {
        switch (type)
        {
            case ACTION_BUTTON_MACRO:
            case ACTION_BUTTON_CMACRO:
                TC_LOG_INFO("network", "MISC: Added Macro %u into button %u", action, button);
                break;
            case ACTION_BUTTON_EQSET:
                TC_LOG_INFO("network", "MISC: Added EquipmentSet %u into button %u", action, button);
                break;
            case ACTION_BUTTON_SPELL:
                TC_LOG_INFO("network", "MISC: Added Spell %u into button %u", action, button);
                break;
            case ACTION_BUTTON_SUB_BUTTON:
                TC_LOG_INFO("network", "MISC: Added sub buttons %u into button %u", action, button);
                break;
            case ACTION_BUTTON_BATTLEPET:
                TC_LOG_INFO("network", "MISC: Added Battle Pet %u into button %u", action, button);
                break;
            case ACTION_BUTTON_ITEM:
                TC_LOG_INFO("network", "MISC: Added Item %u into button %u", action, button);
                break;
            default:
                TC_LOG_ERROR("network", "MISC: Unknown action button type %u for action %u into button %u for player %s (GUID: %u)",
                             type, action, button, _player->GetName().c_str(), _player->GetGUIDLow());
                return;
        }
        GetPlayer()->addActionButton(button, action, type);
    }
}

void WorldSession::HandleCompleteCinematic(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_COMPLETE_CINEMATIC");
}

void WorldSession::HandleNextCinematicCamera(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_NEXT_CINEMATIC_CAMERA");
}

void WorldSession::HandleMoveTimeSkippedOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_MOVE_TIME_SKIPPED");

    ObjectGuid guid;
    uint32 time;
    recvData >> time;

    recvData.ReadBitSeq<3, 0, 5, 1, 7, 6, 4, 2>(guid);
    recvData.ReadByteSeq<1, 6, 0, 5, 3, 4, 2, 7>(guid);

    //TODO!

    /*
    uint64 guid;
    uint32 time_skipped;
    recvData >> guid;
    recvData >> time_skipped;
    TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "WORLD: CMSG_MOVE_TIME_SKIPPED");

    /// TODO
    must be need use in Trinity
    We substract server Lags to move time (AntiLags)
    for exmaple
    GetPlayer()->ModifyLastMoveTime(-int32(time_skipped));
    */
}

void WorldSession::HandleFeatherFallAck(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_MOVE_FEATHER_FALL_ACK");

    // no used
    recvData.rfinish();                       // prevent warnings spam
}

void WorldSession::HandleMoveUnRootAck(WorldPacket& recvData)
{
    // no used
    recvData.rfinish();                       // prevent warnings spam
    /*
    uint64 guid;
    recvData >> guid;

    // now can skip not our packet
    if (_player->GetGUID() != guid)
    {
    recvData.rfinish();                   // prevent warnings spam
    return;
    }

    TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "WORLD: CMSG_FORCE_MOVE_UNROOT_ACK");

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    movementInfo.guid = guid;
    ReadMovementInfo(recvData, &movementInfo);
    recvData.read_skip<float>();                           // unk2
    */
}

void WorldSession::HandleMoveRootAck(WorldPacket& recvData)
{
    // no used
    recvData.rfinish();                       // prevent warnings spam
    /*
    uint64 guid;
    recvData >> guid;

    // now can skip not our packet
    if (_player->GetGUID() != guid)
    {
    recvData.rfinish();                   // prevent warnings spam
    return;
    }

    TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "WORLD: CMSG_FORCE_MOVE_ROOT_ACK");

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);
    */
}

void WorldSession::HandleSetActionBarToggles(WorldPacket& recvData)
{
    uint8 actionBar;

    recvData >> actionBar;

    if (!GetPlayer())                                        // ignore until not logged (check needed because STATUS_AUTHED)
    {
        if (actionBar != 0)
            TC_LOG_ERROR("network", "WorldSession::HandleSetActionBarToggles in not logged state with value: %u, ignored", uint32(actionBar));
        return;
    }

    GetPlayer()->SetByteValue(PLAYER_FIELD_BYTES, 2, actionBar);
}

void WorldSession::HandlePlayedTime(WorldPacket& recvData)
{
    bool unk1 = recvData.ReadBit();                 // 0 or 1 expected

    WorldPacket data(SMSG_PLAYED_TIME, 4 + 4 + 1);
    data << uint32(_player->GetLevelPlayedTime());
    data << uint32(_player->GetTotalPlayedTime());
    data.WriteBit(unk1);                            // 0 - will not show in chat frame
    data.FlushBits();
    SendPacket(&data);
}

void WorldSession::HandleInspectOpcode(WorldPacket& recvData)
{
    ObjectGuid playerGuid;

    recvData.ReadBitSeq<2, 5, 1, 6, 7, 4, 0, 3>(playerGuid);
    recvData.ReadByteSeq<0, 1, 5, 6, 4, 2, 3, 7>(playerGuid);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_INSPECT");

    _player->SetSelection(playerGuid);

    Player* player = ObjectAccessor::FindPlayer(playerGuid);
    if (!player)
    {
        TC_LOG_DEBUG("network", "CMSG_INSPECT: No player found from GUID: " UI64FMTD, uint64(playerGuid));
        return;
    }

    WorldPacket data(SMSG_INSPECT_TALENT);

    ByteBuffer talentData;
    ByteBuffer glyphData;

    uint32 talentCount = 0;
    uint32 glyphCount = 0;
    uint32 equipmentCount = 0;

    data.WriteBitSeq<7, 3>(playerGuid);

    Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId());
    data.WriteBit(guild != nullptr);

    if (guild != nullptr)
    {
        ObjectGuid guildGuid = guild->GetGUID();

        data.WriteBitSeq<6, 7, 4, 5, 2, 3, 1, 0>(guildGuid);
    }

    for (auto const &kvPair : player->GetTalentMap(player->GetActiveSpec()))
    {
        auto const spell = sSpellMgr->GetSpellInfo(kvPair.first);
        if (spell && spell->talentId)
        {
            talentData << uint16(spell->talentId);
            ++talentCount;
        }
    }

    for (uint8 i = 0; i < MAX_GLYPH_SLOT_INDEX; ++i)
    {
        if (player->GetGlyph(0, i) == 0)
            continue;

        glyphData << uint16(player->GetGlyph(0, i));               // GlyphProperties.dbc
        ++glyphCount;
    }

    data.WriteBits(talentCount, 23);
    data.WriteBits(glyphCount, 23);
    data.WriteBitSeq<5, 2, 6>(playerGuid);

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!item)
            continue;

        ++equipmentCount;
    }

    data.WriteBits(equipmentCount, 20);

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!item)
            continue;

        ObjectGuid itemCreator = item->GetUInt64Value(ITEM_FIELD_CREATOR);

        data.WriteBitSeq<0>(itemCreator);
        data.WriteBit(0);               // unk bit 32
        data.WriteBitSeq<6, 7>(itemCreator);
        data.WriteBit(0);               // unk bit 16
        data.WriteBitSeq<3, 2, 1>(itemCreator);

        uint32 enchantmentCount = 0;

        for (uint32 j = 0; j < MAX_ENCHANTMENT_SLOT; ++j)
        {
            uint32 enchId = item->GetEnchantmentId(EnchantmentSlot(j));
            if (!enchId)
                continue;

            ++enchantmentCount;
        }

        data.WriteBits(enchantmentCount, 21);
        data.WriteBit(0);               // unk bit
        data.WriteBitSeq<5, 4>(itemCreator);
    }

    data.WriteBitSeq<4, 1, 0>(playerGuid);

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!item)
            continue;

        ObjectGuid itemCreator = item->GetUInt64Value(ITEM_FIELD_CREATOR);

        // related to random stats
        // if (unkBit)
        //     data << uint16(UNK);

        data.WriteByteSeq<3, 4>(itemCreator);

        for (uint32 j = 0; j < MAX_ENCHANTMENT_SLOT; ++j)
        {
            uint32 enchId = item->GetEnchantmentId(EnchantmentSlot(j));
            if (!enchId)
                continue;

            data << uint8(j);
            data << uint32(enchId);
        }

        // if (unkBit)
        //     data << uint32(UNK);

        data.WriteByteSeq<0, 2, 6>(itemCreator);

        uint32 mask = 0;
        uint32 modifiers = 0;
        if (item->GetDynamicUInt32Value(ITEM_DYNAMIC_MODIFIERS, 0))
        {
            ++modifiers;
            mask |= 0x1;
        }
        if (item->GetDynamicUInt32Value(ITEM_DYNAMIC_MODIFIERS, 1))
        {
            ++modifiers;
            mask |= 0x2;
        }
        if (item->GetDynamicUInt32Value(ITEM_DYNAMIC_MODIFIERS, 2))
        {
            ++modifiers;
            mask |= 0x4;
        }
        data << uint32(modifiers == 0 ? 0 : ((modifiers*4) + 4));
        if (modifiers > 0)
        {
            data << uint32(mask);
            if (uint32 reforge = item->GetDynamicUInt32Value(ITEM_DYNAMIC_MODIFIERS, 0))
                data << uint32(reforge);
            if (uint32 transmogrification = item->GetDynamicUInt32Value(ITEM_DYNAMIC_MODIFIERS, 1))
                data << uint32(transmogrification);
            if (uint32 upgrade = item->GetDynamicUInt32Value(ITEM_DYNAMIC_MODIFIERS, 2))
                data << uint32(upgrade);
        }

        data.WriteByteSeq<1, 7, 5>(itemCreator);
        data << uint8(i);
        data << uint32(item->GetEntry());
    }

    data.WriteByteSeq<7, 1, 5, 0>(playerGuid);

    data << uint32(player->GetSpecializationId(player->GetActiveSpec()));

    if (guild != nullptr)
    {
        ObjectGuid guildGuid = guild->GetGUID();

        data << uint32(guild->GetLevel());

        data.WriteByteSeq<1, 3>(guildGuid);

        data << uint32(guild->GetMembersCount());

        data.WriteByteSeq<6, 2, 5, 4, 0, 7>(guildGuid);

        data << uint64(guild->GetExperience());
    }

    data.WriteByteSeq<6, 4, 2, 3>(playerGuid);

    data.append(talentData);
    data.append(glyphData);

    SendPacket(&data);
}

void WorldSession::HandleInspectHonorStatsOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<0, 3, 2, 4, 6, 7, 5, 1>(guid);
    recvData.ReadByteSeq<5, 0, 2, 7, 6, 3, 1, 4>(guid);

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)
    {
        //TC_LOG_DEBUG("network", "CMSG_INSPECT_HONOR_STATS: No player found from GUID: " UI64FMTD, guid);
        return;
    }

    ObjectGuid playerGuid = guid;
    WorldPacket data(SMSG_INSPECT_HONOR_STATS);

    data.WriteBitSeq<5, 3, 7, 2, 1, 6, 0, 4>(playerGuid);

    const uint32 cycleCount = 3; // MAX_ARENA_SLOT
    data.WriteBits(cycleCount, 3);

    data.WriteByteSeq<7>(playerGuid);

    for (size_t i = 0; i < cycleCount; ++i)
    {
        // Client display only this two fields

        data << uint32(player->GetSeasonWins(i));
        data << uint32(0);
        data << uint32(0);

        data << uint8(i);

        data << uint32(0);
        data << uint32(0);
        data << uint32(player->GetArenaPersonalRating(i));
        data << uint32(0);
    }

    data.WriteByteSeq<1, 5, 0, 3, 2, 6, 4>(playerGuid);

    SendPacket(&data);
}

void WorldSession::HandleInspectRatedBGStatsOpcode(WorldPacket& recvData)
{
    uint32 unk;
    ObjectGuid guid;

    recvData >> unk;

    recvData.ReadBitSeq<1, 3, 5, 2, 6, 7, 0, 4>(guid);
    recvData.ReadByteSeq<4, 7, 0, 3, 5, 2, 6, 1>(guid);

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)
    {
        //TC_LOG_DEBUG("network", "CMSG_REQUEST_INSPECT_RATED_BG_STATS: No player found from GUID: " UI64FMTD, guid);
        return;
    }

    return;

    // TODO //
    WorldPacket data(SMSG_INSPECT_RATED_BG_STATS);

    ObjectGuid gguid = guid;
    data << uint32(0); //SeasonWin
    data << uint32(0); //SeasonPlayed
    data << uint32(0); //Rating

    data.WriteBitSeq<5, 7, 2, 3, 4, 6, 0, 1>(gguid);
    data.WriteByteSeq<6, 2, 3, 1, 7, 5, 4, 0>(gguid);

    SendPacket(&data);
}

void WorldSession::HandleWorldTeleportOpcode(WorldPacket& recvData)
{
    uint32 time;
    uint32 mapid;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recvData >> time;                                      // time in m.sec.
    recvData >> mapid;
    recvData >> PositionX;
    recvData >> Orientation;
    recvData >> PositionY;
    recvData >> PositionZ;                          // o (3.141593 = 180 degrees)

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_WORLD_TELEPORT");

    if (GetPlayer()->isInFlight())
    {
        TC_LOG_DEBUG("network", "Player '%s' (GUID: %u) in flight, ignore worldport command.",
                     GetPlayer()->GetName().c_str(), GetPlayer()->GetGUIDLow());
        return;
    }

    TC_LOG_DEBUG("network", "CMSG_WORLD_TELEPORT: Player = %s, Time = %u, map = %u, x = %f, y = %f, z = %f, o = %f",
                 GetPlayer()->GetName().c_str(), time, mapid, PositionX, PositionY, PositionZ, Orientation);

    if (AccountMgr::IsAdminAccount(GetSecurity()))
        GetPlayer()->TeleportTo(mapid, PositionX, PositionY, PositionZ, Orientation);
    else
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
}

void WorldSession::HandleWhoisOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "Received opcode CMSG_WHOIS");
    uint32 textLength = recvData.ReadBits(6);
    std::string charname = recvData.ReadString(textLength);

    if (!AccountMgr::IsAdminAccount(GetSecurity()))
    {
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
        return;
    }

    if (charname.empty() || !normalizePlayerName (charname))
    {
        SendNotification(LANG_NEED_CHARACTER_NAME);
        return;
    }

    Player* player = sObjectAccessor->FindPlayerByName(charname);

    if (!player)
    {
        SendNotification(LANG_PLAYER_NOT_EXIST_OR_OFFLINE, charname.c_str());
        return;
    }

    uint32 accid = player->GetSession()->GetAccountId();

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_WHOIS);

    stmt->setUInt32(0, accid);

    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (!result)
    {
        SendNotification(LANG_ACCOUNT_FOR_PLAYER_NOT_FOUND, charname.c_str());
        return;
    }

    Field* fields = result->Fetch();
    std::string acc = fields[0].GetString();
    if (acc.empty())
        acc = "Unknown";
    std::string email = fields[1].GetString();
    if (email.empty())
        email = "Unknown";
    std::string lastip = fields[2].GetString();
    if (lastip.empty())
        lastip = "Unknown";

    std::string msg = charname + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastip;

    WorldPacket data(SMSG_WHOIS, msg.size()+1);
    data.WriteBits(msg.size(), 11);

    data.FlushBits();
    if (msg.size())
        data.append(msg.c_str(), msg.size());

    SendPacket(&data);

    TC_LOG_DEBUG("network", "Received whois command from player %s for character %s",
                 GetPlayer()->GetName().c_str(), charname.c_str());
}

void WorldSession::HandleComplainOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_COMPLAIN");

    // recvData is not empty, but all data are unused in core
    // NOTE: all chat messages from this spammer automatically ignored by spam reporter until logout in case chat spam.
    // if it's mail spam - ALL mails from this spammer automatically removed by client

    // Complaint Received message
    WorldPacket data(SMSG_COMPLAIN_RESULT, 2);
    data << uint8(0);   // value 1 resets CGChat::m_complaintsSystemStatus in client. (unused?)
    data << uint32(0);  // value 0xC generates a "CalendarError" in client.
    SendPacket(&data);
}

void WorldSession::HandleRealmSplitOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_REALM_SPLIT");

    uint32 unk;
    std::string split_date = "01/01/01";
    recvData >> unk;

    WorldPacket data(SMSG_REALM_SPLIT);
    data.WriteBits(split_date.size(), 7);
    data << unk;
    data << uint32(0x00000000);                             // realm split state
    data << split_date;
    SendPacket(&data);
}

void WorldSession::HandleRealmQueryNameOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_REALM_QUERY_NAME");

    uint32 realmId = recvData.read<uint32>();

    if (realmId != realmID)
        return; // Cheater ?

    std::string realmName = sWorld->GetRealmName();

    WorldPacket data(SMSG_REALM_QUERY_RESPONSE);
    // 0 : OK, 1 : Error, 2 : Retry, 3 : Show '?'
    data << uint8(0); // ok, realmId exist server-side
    data << realmID;
    data.WriteBits(realmName.size(), 8);
    data.WriteBit(1); // unk, if it's main realm ?
    data.WriteBits(realmName.size(), 8);
    data.FlushBits();
    data.append(realmName.c_str(), realmName.size());
    data.append(realmName.c_str(), realmName.size());

    SendPacket(&data);
}

void WorldSession::HandleFarSightOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_FAR_SIGHT");

    bool apply = recvData.ReadBit();

    if (apply)
    {
        TC_LOG_DEBUG("network", "Added FarSight " UI64FMTD " to player %u", _player->GetUInt64Value(PLAYER_FARSIGHT), _player->GetGUIDLow());
        if (WorldObject* target = _player->GetViewpoint())
            _player->SetSeer(target);
        else
            TC_LOG_ERROR("network", "Player %s requests non-existing seer " UI64FMTD, _player->GetName().c_str(), _player->GetUInt64Value(PLAYER_FARSIGHT));
    }
    else
    {
        TC_LOG_DEBUG("network", "Player %u set vision to self", _player->GetGUIDLow());
        _player->SetSeer(_player);
    }

    GetPlayer()->UpdateVisibilityForPlayer();
}

void WorldSession::HandleSetTitleOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_SET_TITLE");

    uint32 title;
    recvData >> title;

    // -1 at none
    if (title > 0 && title < MAX_TITLE_INDEX)
    {
        if (!GetPlayer()->HasTitle(title))
            return;
    }
    else
        title = 0;

    GetPlayer()->SetUInt32Value(PLAYER_CHOSEN_TITLE, title);
}

void WorldSession::HandleTimeSyncResp(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_TIME_SYNC_RESP");

    if (Player* player = GetPlayer())
    {
        if (Battleground* bg = GetPlayer()->GetBattleground())
        {
            if (player->ShouldForgetBGPlayers())
            {
                player->DoForgetPlayersInBG(bg);
                player->SetForgetBGPlayers(false);
            }
        }
        else if (player->ShouldForgetInListPlayers())
        {
            player->DoForgetPlayersInList();
            player->SetForgetInListPlayers(false);
        }
    }
    uint32 counter, clientTicks;
    recvData >> counter >> clientTicks;

    if (counter != _player->m_timeSyncCounter - 1)
        TC_LOG_DEBUG("network", "Wrong time sync counter from player %s (cheater?)", _player->GetName().c_str());

    TC_LOG_DEBUG("network", "Time sync received: counter %u, client ticks %u, time since last sync %u", counter, clientTicks, clientTicks - _player->m_timeSyncClient);

    uint32 ourTicks = clientTicks + (getMSTime() - _player->m_timeSyncServer);

    // diff should be small
    TC_LOG_DEBUG("network", "Our ticks: %u, diff %u, latency %u", ourTicks, ourTicks - clientTicks, GetLatency());

    _player->m_timeSyncClient = clientTicks;
}

void WorldSession::HandleResetInstancesOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_RESET_INSTANCES");

    if (Group* group = _player->GetGroup())
    {
        if (group->IsLeader(_player->GetGUID()))
            group->ResetInstances(INSTANCE_RESET_ALL, false, _player);
    }
    else
        _player->ResetInstances(INSTANCE_RESET_ALL, false);
}

void WorldSession::HandleResetChallengeModeOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_RESET_CHALLENGE_MODE");
    // TODO: Do something about challenge mode ...
}

void WorldSession::HandleSetDungeonDifficultyOpcode(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "MSG_SET_DUNGEON_DIFFICULTY");

    uint32 mode;
    recvData >> mode;

    if (mode != CHALLENGE_MODE_DIFFICULTY && mode >= MAX_DUNGEON_DIFFICULTY)
    {
        TC_LOG_ERROR("network", "WorldSession::HandleSetDungeonDifficultyOpcode: player %d sent an invalid instance mode %d!", _player->GetGUIDLow(), mode);
        return;
    }

    if (Difficulty(mode) == _player->GetDungeonDifficulty())
        return;

    // cannot reset while in an instance
    Map* map = _player->FindMap();
    if (map && map->IsDungeon())
    {
        TC_LOG_ERROR("network", "WorldSession::HandleSetDungeonDifficultyOpcode: player (Name: %s, GUID: %u)"
                     " tried to reset the instance while player is inside!", _player->GetName().c_str(), _player->GetGUIDLow());
        return;
    }

    Group* group = _player->GetGroup();
    if (group)
    {
        if (group->IsLeader(_player->GetGUID()))
        {
            if (group->IsLFGRestricted())
                return;

            for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* groupGuy = itr->GetSource();
                if (!groupGuy)
                    continue;

                // FIXME obj1 IsInmap obj1
                if (!groupGuy->IsInMap(groupGuy))
                    return;

                if (groupGuy->GetMap()->IsNonRaidDungeon())
                {
                    TC_LOG_ERROR("network", "WorldSession::HandleSetDungeonDifficultyOpcode: player %d tried"
                                 " to reset the instance while group member (Name: %s, GUID: %u) is inside!",
                                 _player->GetGUIDLow(), groupGuy->GetName().c_str(), groupGuy->GetGUIDLow());
                    return;
                }
            }
            // the difficulty is set even if the instances can't be reset
            //_player->SendDungeonDifficulty(true);
            group->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, false, _player);
            group->SetDungeonDifficulty(Difficulty(mode));
            _player->SendDungeonDifficulty(true);
        }
    }
    else
    {
        _player->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, false);
        _player->SetDungeonDifficulty(Difficulty(mode));
        _player->SendDungeonDifficulty(false);
    }
}

void WorldSession::HandleSetRaidDifficultyOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "MSG_SET_RAID_DIFFICULTY");

    uint32 mode;
    recvData >> mode;

    if (mode >= MAX_RAID_DIFFICULTY)
    {
        TC_LOG_ERROR("network", "WorldSession::HandleSetRaidDifficultyOpcode: player %d sent an invalid instance mode %d!", _player->GetGUIDLow(), mode);
        return;
    }

    // cannot reset while in an instance
    Map* map = _player->FindMap();
    if (map && map->IsDungeon())
    {
        TC_LOG_ERROR("network", "WorldSession::HandleSetRaidDifficultyOpcode: player %d tried to reset the instance while inside!", _player->GetGUIDLow());
        return;
    }

    if (Difficulty(mode) == _player->GetRaidDifficulty())
        return;

    Group* group = _player->GetGroup();
    if (group)
    {
        if (group->IsLeader(_player->GetGUID()))
        {
            if (group->IsLFGRestricted())
                return;

            for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* groupGuy = itr->GetSource();
                if (!groupGuy)
                    continue;

                // FIXME obj1 IsInmap obj1
                if (!groupGuy->IsInMap(groupGuy))
                    return;

                if (groupGuy->GetMap()->IsRaid())
                {
                    TC_LOG_ERROR("network", "WorldSession::HandleSetRaidDifficultyOpcode: player %d tried to reset the instance while inside!", _player->GetGUIDLow());
                    return;
                }
            }
            // the difficulty is set even if the instances can't be reset
            //_player->SendDungeonDifficulty(true);
            group->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, true, _player);
            group->SetRaidDifficulty(Difficulty(mode));
            _player->SendRaidDifficulty(true);
        }
    }
    else
    {
        _player->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, true);
        _player->SetRaidDifficulty(Difficulty(mode));
        _player->SendRaidDifficulty(false);
    }
}

void WorldSession::HandleCancelMountAuraOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_CANCEL_MOUNT_AURA");

    //If player is not mounted, so go out :)
    if (!_player->IsMounted())                              // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANG_CHAR_NON_MOUNTED);
        return;
    }

    if (_player->isInFlight())                               // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANG_YOU_IN_FLIGHT);
        return;
    }

    _player->Dismount();
    _player->RemoveAurasByType(SPELL_AURA_MOUNTED);
}

void WorldSession::HandleRequestPetInfoOpcode(WorldPacket& /*recvData */)
{
    /*
    TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "WORLD: CMSG_REQUEST_PET_INFO");
    TC_LOG_TRACE("network", "%s", recvData.hexlike().c_str());
    */
}

void WorldSession::HandleSetTaxiBenchmarkOpcode(WorldPacket& recvData)
{
    uint8 mode;
    recvData >> mode;

    TC_LOG_DEBUG("network", "Client used \"/timetest %d\" command", mode);
}

void WorldSession::HandleQueryInspectAchievements(WorldPacket& recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<7, 4, 0, 5, 2, 1, 3, 6>(guid);
    recvData.ReadByteSeq<3, 2, 4, 6, 1, 7, 5, 0>(guid);

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)
        return;

    player->GetAchievementMgr().SendAchievementInfo(_player);
}

void WorldSession::HandleGuildAchievementProgressQuery(WorldPacket& recvData)
{
    uint32 achievementId;
    recvData >> achievementId;

    if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
        guild->GetAchievementMgr().SendAchievementInfo(_player, achievementId);
}

void WorldSession::HandleWorldStateUITimerUpdate(WorldPacket& /*recvData*/)
{
    // empty opcode
    TC_LOG_DEBUG("network", "WORLD: CMSG_WORLD_STATE_UI_TIMER_UPDATE");

    WorldPacket data(SMSG_WORLD_STATE_UI_TIMER_UPDATE, 4);
    data << uint32(time(NULL));
    SendPacket(&data);
}

void WorldSession::HandleReadyForAccountDataTimes(WorldPacket& /*recvData*/)
{
    // empty opcode
    TC_LOG_DEBUG("network", "WORLD: CMSG_READY_FOR_ACCOUNT_DATA_TIMES");

    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::SendSetPhaseShift(PhaseShiftSet const& phaseIds, PhaseShiftSet const& terrainswaps, PhaseShiftSet const& worldMapAreaIds)
{
    ObjectGuid guid = _player->GetGUID();

    WorldPacket data(SMSG_SET_PHASE_SHIFT, 4 + 4 + 2 * terrainswaps.size() + 4 + 2 * phaseIds.size() + 4 + 2 * worldMapAreaIds.size() + 4 + 8);

    // 0x8 or 0x10 is related to areatrigger, if we send flags 0x00 areatrigger doesn't work in some case
    data << uint32(0x8);                        // flags (not phasemask)

    // Active terrain swaps
    data << uint32(terrainswaps.size() * 2);    // Map.dbc Ids
    for (auto const &id : terrainswaps)
        data << uint16(id);

    // WorldMapArea.dbc id (controls map display)
    data << uint32(worldMapAreaIds.size() * 2); // WorldMapArea.dbc Ids
    for (auto const &id : worldMapAreaIds)
        data << uint16(id);

    data << uint32(phaseIds.size() * 2);        // Phase.dbc ids
    for (auto const &id : phaseIds)
        data << uint16(id);

    // Inactive terrain swaps count
    // Purpose unknown : terrain switch works without it
    data << uint32(0); // Map.dbc Ids ?

    data.WriteBitSeq<0, 2, 1, 5, 3, 7, 4, 6>(guid);
    data.WriteByteSeq<0, 5, 4, 7, 6, 2, 1, 3>(guid);

    SendPacket(&data);
}

// Battlefield and Battleground
void WorldSession::HandleAreaSpiritHealerQueryOpcode(WorldPacket& recv_data)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_AREA_SPIRIT_HEALER_QUERY");

    Battleground* bg = _player->GetBattleground();

    ObjectGuid guid;

    recv_data.ReadBitSeq<5, 3, 0, 6, 1, 7, 4, 2>(guid);
    recv_data.ReadByteSeq<6, 7, 3, 0, 5, 4, 2, 1>(guid);

    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isSpiritService())                            // it's not spirit service
        return;

    if (bg)
        sBattlegroundMgr->SendAreaSpiritHealerQueryOpcode(_player, bg, guid);

    if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(_player->GetZoneId()))
        bf->SendAreaSpiritHealerQueryOpcode(_player,guid);
}

void WorldSession::HandleAreaSpiritHealerQueueOpcode(WorldPacket& recv_data)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_AREA_SPIRIT_HEALER_QUEUE");

    Battleground* bg = _player->GetBattleground();
    ObjectGuid npcGuid;

    recv_data.ReadBitSeq<6, 2, 0, 3, 5, 4, 1, 7>(npcGuid);
    recv_data.ReadByteSeq<4, 2, 6, 0, 1, 7, 3, 5>(npcGuid);

    Creature* unit = GetPlayer()->GetMap()->GetCreature(npcGuid);
    if (!unit)
        return;

    if (!unit->isSpiritService())                            // it's not spirit service
        return;

    if (bg)
        bg->AddPlayerToResurrectQueue(npcGuid, _player->GetGUID());

    if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(_player->GetZoneId()))
        bf->AddPlayerToResurrectQueue(npcGuid, _player->GetGUID());
}

void WorldSession::HandleHearthAndResurrect(WorldPacket& /*recvData*/)
{
    if (_player->isInFlight())
        return;

    if (/*Battlefield* bf =*/ sBattlefieldMgr->GetBattlefieldToZoneId(_player->GetZoneId()))
    {
        // bf->PlayerAskToLeave(_player);                   //@todo: FIXME
        return;
    }

    AreaTableEntry const* atEntry = GetAreaEntryByAreaID(_player->GetAreaId());
    if (!atEntry || !(atEntry->flags & AREA_FLAG_WINTERGRASP_2))
        return;

    _player->BuildPlayerRepop();
    _player->ResurrectPlayer(100);
    _player->TeleportTo(_player->m_homebindMapId, _player->m_homebindX, _player->m_homebindY, _player->m_homebindZ, _player->GetOrientation());
}

void WorldSession::HandleInstanceLockResponse(WorldPacket& recvPacket)
{
    uint8 accept;
    recvPacket >> accept;

    if (!_player->HasPendingBind())
    {
        TC_LOG_INFO("network", "InstanceLockResponse: Player %s (guid %u) tried to bind himself/teleport to graveyard without a pending bind!",
                    _player->GetName().c_str(), _player->GetGUIDLow());
        return;
    }

    if (accept)
        _player->BindToInstance();
    else
        _player->RepopAtGraveyard();

    _player->SetPendingBind(0, 0);
}

void WorldSession::HandleMovieComplete(WorldPacket& /*recvData*/)
{
    _player->SetCurrentMovieId(0);
}

void WorldSession::HandleRequestHotfix(WorldPacket& recvPacket)
{
    uint32 type, count;
    recvPacket >> type;

    count = recvPacket.ReadBits(21);

    std::vector<ObjectGuid> guids(count);

    for (uint32 i = 0; i < count; ++i)
        recvPacket.ReadBitSeq<3, 4, 7, 2, 5, 1, 6, 0>(guids[i]);

    uint32 entry;

    for (uint32 i = 0; i < count; ++i)
    {
        recvPacket.ReadByteSeq<6, 1, 2>(guids[i]);
        recvPacket >> entry;
        recvPacket.ReadByteSeq<4, 5, 7, 0, 3>(guids[i]);

        switch (type)
        {
            case DB2_REPLY_ITEM:
                SendItemDb2Reply(entry);
                break;
            case DB2_REPLY_SPARSE:
                SendItemSparseDb2Reply(entry);
                break;
            // TODO
            case DB2_REPLY_BATTLE_PET_EFFECT_PROPERTIES:
            case DB2_REPLY_SCENE_SCRIPT:
                break;
            case DB2_REPLY_BROADCAST_TEXT:
                SendBroadcastTextDb2Reply(entry);
                break;
            default:
                TC_LOG_ERROR("network", "CMSG_REQUEST_HOTFIX: Received unknown hotfix type: %u", type);
                recvPacket.rfinish();
                return;
        }
    }
}

void WorldSession::HandleUpdateMissileTrajectory(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_UPDATE_MISSILE_TRAJECTORY");

    uint64 guid;
    uint32 spellId;
    float elevation, speed;
    float curX, curY, curZ;
    float targetX, targetY, targetZ;
    uint8 moveStop;

    recvPacket >> guid >> spellId >> elevation >> speed;
    recvPacket >> curX >> curY >> curZ;
    recvPacket >> targetX >> targetY >> targetZ;
    recvPacket >> moveStop;

    Unit* caster = ObjectAccessor::GetUnit(*_player, guid);
    Spell* spell = caster ? caster->GetCurrentSpell(CURRENT_GENERIC_SPELL) : NULL;
    if (!spell || spell->m_spellInfo->Id != spellId || !spell->m_targets.HasDst() || !spell->m_targets.HasSrc())
    {
        recvPacket.rfinish();
        return;
    }

    Position pos = *spell->m_targets.GetSrcPos();
    pos.Relocate(curX, curY, curZ);
    spell->m_targets.ModSrc(pos);

    pos = *spell->m_targets.GetDstPos();
    pos.Relocate(targetX, targetY, targetZ);
    spell->m_targets.ModDst(pos);

    spell->m_targets.SetElevation(elevation);
    spell->m_targets.SetSpeed(speed);

    if (moveStop)
    {
        uint32 opcode;
        recvPacket >> opcode;
        recvPacket.SetOpcode(MSG_MOVE_STOP); // always set to MSG_MOVE_STOP in client SetOpcode
        HandleMovementOpcodes(recvPacket);
    }
}

void WorldSession::HandleViolenceLevel(WorldPacket& recvPacket)
{
    uint8 violenceLevel;
    recvPacket >> violenceLevel;

    // do something?
}

void WorldSession::HandleObjectUpdateFailedOpcode(WorldPacket& recvPacket)
{
    ObjectGuid guid;

    recvPacket.ReadBitSeq<2, 1, 0, 6, 3, 7, 4, 5>(guid);
    recvPacket.ReadByteSeq<7, 5, 1, 6, 4, 2, 3, 0>(guid);

    WorldObject* obj = ObjectAccessor::GetWorldObject(*GetPlayer(), guid);
    if (obj)
        obj->SendUpdateToPlayer(GetPlayer());

    TC_LOG_ERROR("network", "Object update failed for object " UI64FMTD " (%s) for player %s (%u)",
                 uint64(guid), obj ? obj->GetName().c_str() : "object-not-found", GetPlayerName().c_str(), GetGuidLow());
}

// DestrinyFrame.xml : lua function NeutralPlayerSelectFaction
#define JOIN_THE_ALLIANCE 1
#define JOIN_THE_HORDE    0

void WorldSession::HandleSetFactionOpcode(WorldPacket& recvPacket)
{
    uint32 choice = recvPacket.read<uint32>();

    if (_player->getRace() != RACE_PANDAREN_NEUTRAL)
        return;

    if (choice == JOIN_THE_HORDE)
    {
        _player->SetByteValue(UNIT_FIELD_BYTES_0, 0, RACE_PANDAREN_HORDE);
        _player->setORace();
        _player->setFactionForRace(RACE_PANDAREN_HORDE);
        _player->SaveToDB();
        WorldLocation location(1, 1366.730f, -4371.248f, 26.070f, 3.1266f);
        _player->TeleportTo(location);
        _player->SetHomebind(location, 363);
        _player->learnSpell(669, false); // Language Orcish
        _player->learnSpell(108127, false); // Language Pandaren
    }
    else if (choice == JOIN_THE_ALLIANCE)
    {
        _player->SetByteValue(UNIT_FIELD_BYTES_0, 0, RACE_PANDAREN_ALLI);
        _player->setORace();
        _player->setFactionForRace(RACE_PANDAREN_ALLI);
        _player->SaveToDB();
        WorldLocation location(0, -9096.236f, 411.380f, 92.257f, 3.649f);
        _player->TeleportTo(location);
        _player->SetHomebind(location, 9);
        _player->learnSpell(668, false); // Language Common
        _player->learnSpell(108127, false); // Language Pandaren
    }

    if (_player->GetQuestStatus(31450) == QUEST_STATUS_INCOMPLETE)
        _player->KilledMonsterCredit(64594);

    _player->SendMovieStart(116);
}

void WorldSession::HandleCategoryCooldownOpcode(WorldPacket& /*recvPacket*/)
{
    Unit::AuraEffectList const& list = GetPlayer()->GetAuraEffectsByType(SPELL_AURA_MOD_SPELL_CATEGORY_COOLDOWN);

    WorldPacket data(SMSG_SPELL_CATEGORY_COOLDOWN, 4 + (list.size() * 8));

    data.WriteBits<uint32>(list.size(), 21);
    data.FlushBits();

    for (Unit::AuraEffectList::const_iterator itr = list.begin(); itr != list.end(); ++itr)
    {
        AuraEffect *effect = *itr;
        if (!effect)
            continue;

        data << uint32(effect->GetMiscValue());
        data << int32(-effect->GetAmount());
    }

    SendPacket(&data);
}

void WorldSession::HandleTradeInfo(WorldPacket& recvPacket)
{
    uint32 skillId = recvPacket.read<uint32>();
    uint32 spellId = recvPacket.read<uint32>();

    ObjectGuid guid;

    recvPacket.ReadBitSeq<5, 4, 7, 1, 3, 6, 0, 2>(guid);
    recvPacket.ReadByteSeq<7, 3, 4, 6, 1, 5, 0, 2>(guid);

    Player* plr = sObjectAccessor->FindPlayer(guid);
    if (!plr || !plr->HasSkill(skillId) || !plr->HasSpell(spellId))
        return;

    uint32 spellSkillCount = 0;
    ByteBuffer buff(sizeof(uint32)*32);
    for (auto itr : plr->GetSpellMap())
    {
        SpellInfo const* spell = sSpellMgr->GetSpellInfo(itr.first);
        if (!spell)
            continue;

        if (!spell->IsAbilityOfSkillType(skillId))
            continue;

        if (!(spell->Attributes & SPELL_ATTR0_TRADESPELL))
            continue;

        buff.append(itr.first);
        ++spellSkillCount;
    }
    WorldPacket data(SMSG_TRADE_INFO);
    data.WriteBitSeq<2, 6, 7>(guid);
    data.WriteBits(spellSkillCount, 22);
    data.WriteBitSeq<5, 1, 4>(guid);
    data.WriteBits(1, 22); // skill value count
    data.WriteBits(1, 22); // skill id count
    data.WriteBits(1, 22); // skill max value
    data.WriteBitSeq<3, 0>(guid);
    data.FlushBits();

    data << uint32(plr->GetSkillValue(skillId));
    data.WriteByteSeq<0>(guid);
    data << uint32(skillId);
    data.WriteByteSeq<1>(guid);
    data << uint32(spellId);

    data.append(buff);

    data.WriteByteSeq<3, 5, 6, 4, 7, 2>(guid);
    data << uint32(plr->GetMaxSkillValue(skillId));
    SendPacket(&data);
}

void WorldSession::HandleSaveCUFProfiles(WorldPacket& recvPacket)
{
    uint32 const count = recvPacket.ReadBits(19);
    if (count > CufProfile::MaxProfiles)
    {
        recvPacket.rfinish();
        return;
    }

    auto &profiles = GetPlayer()->m_cufProfiles;
    profiles.resize(count);

    uint32 nameLengths[CufProfile::MaxProfiles];
    auto nameLengthsItr = std::begin(nameLengths);

    for (CufProfile &profile : profiles)
    {
        profile.bits.reset();
        profile.bits[CufProfile::DisplayMainTankAndAssistant] = recvPacket.ReadBit();
        profile.bits[CufProfile::KeepGroupsTogether] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivate5Players] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivate15Players] = recvPacket.ReadBit();
        profile.bits[CufProfile::DisplayPowerBar] = recvPacket.ReadBit();
        profile.bits[CufProfile::DisplayBorder] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivatePvp] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivate40Players] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivateSpec1] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivate3Players] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivate2Players] = recvPacket.ReadBit();
        profile.bits[CufProfile::UseClassColors] = recvPacket.ReadBit();
        profile.bits[CufProfile::Unk13] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivateSpec2] = recvPacket.ReadBit();
        profile.bits[CufProfile::HorizontalGroups] = recvPacket.ReadBit();
        profile.bits[CufProfile::Unk16] = recvPacket.ReadBit();
        profile.bits[CufProfile::DisplayOnlyDispellableDebuffs] = recvPacket.ReadBit();

        *nameLengthsItr = recvPacket.ReadBits(7);
        if (*nameLengthsItr++ > CufProfile::MaxNameLength)
        {
            recvPacket.rfinish();
            return;
        }

        profile.bits[CufProfile::DisplayNonBossDebuffs] = recvPacket.ReadBit();
        profile.bits[CufProfile::DisplayPets] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivate25Players] = recvPacket.ReadBit();
        profile.bits[CufProfile::DisplayHealPrediction] = recvPacket.ReadBit();
        profile.bits[CufProfile::DisplayAggroHighlight] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivate10Players] = recvPacket.ReadBit();
        profile.bits[CufProfile::Unk24] = recvPacket.ReadBit();
        profile.bits[CufProfile::AutoActivatePve] = recvPacket.ReadBit();
    }

    nameLengthsItr = std::begin(nameLengths);

    for (CufProfile &profile : profiles)
    {
        recvPacket >> profile.unk5 >> profile.unk6 >> profile.unk0
                   >> profile.unk7 >> profile.unk1 >> profile.sortType
                   >> profile.frameWidth >> profile.healthText
                   >> profile.frameHeight;

        profile.name = recvPacket.ReadString(*nameLengthsItr++);

        recvPacket >> profile.unk4;
    }

    _player->SendCUFProfiles();

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CUF_PROFILE);
    stmt->setUInt32(0, GetPlayer()->GetGUIDLow());
    trans->Append(stmt);

    for (uint32 i = 0; i < count; ++i)
    {
        CufProfile const &profile = profiles[i];

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CUF_PROFILE);
        stmt->setUInt32(0, GetPlayer()->GetGUIDLow());
        stmt->setUInt8(1, i);
        stmt->setString(2, profile.name);
        stmt->setUInt16(3, profile.unk0);
        stmt->setUInt16(4, profile.unk1);
        stmt->setUInt16(5, profile.frameWidth);
        stmt->setUInt16(6, profile.frameHeight);
        stmt->setUInt16(7, profile.unk4);
        stmt->setUInt8(8, profile.unk5);
        stmt->setUInt8(9, profile.unk6);
        stmt->setUInt8(10, profile.unk7);
        stmt->setUInt8(11, profile.sortType);
        stmt->setUInt8(12, profile.healthText);
        stmt->setUInt32(13, profile.bits.to_ulong());

        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);
}
