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
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"

#include "CellImpl.h"
#include "Chat.h"
#include "ChannelMgr.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Guild.h"
#include "Language.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "Util.h"
#include "ScriptMgr.h"
#include "AccountMgr.h"

bool WorldSession::processChatmessageFurtherAfterSecurityChecks(std::string& msg, uint32 lang)
{
    if (lang != LANG_ADDON)
    {
        // strip invisible characters for non-addon messages
        if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
            stripLineInvisibleChars(msg);

        if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) && AccountMgr::IsPlayerAccount(GetSecurity())
                && !ChatHandler(this).isValidChatMessage(msg.c_str()))
        {
            TC_LOG_ERROR("network", "Player %s (GUID: %u) sent a chatmessage with an invalid link: %s", GetPlayer()->GetName().c_str(),
                    GetPlayer()->GetGUIDLow(), msg.c_str());
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            return false;
        }
    }

    return true;
}

void WorldSession::HandleMessagechatOpcode(WorldPacket& recvData)
{
    uint32 type = 0;
    uint32 lang;

    switch (recvData.GetOpcode())
    {
        case CMSG_MESSAGECHAT_SAY:
            type = CHAT_MSG_SAY;
            break;
        case CMSG_MESSAGECHAT_YELL:
            type = CHAT_MSG_YELL;
            break;
        case CMSG_MESSAGECHAT_CHANNEL:
            type = CHAT_MSG_CHANNEL;
            break;
        case CMSG_MESSAGECHAT_WHISPER:
            type = CHAT_MSG_WHISPER;
            break;
        case CMSG_MESSAGECHAT_GUILD:
            type = CHAT_MSG_GUILD;
            break;
        case CMSG_MESSAGECHAT_OFFICER:
            type = CHAT_MSG_OFFICER;
            break;
        case CMSG_MESSAGECHAT_AFK:
            type = CHAT_MSG_AFK;
            break;
        case CMSG_MESSAGECHAT_DND:
            type = CHAT_MSG_DND;
            break;
        case CMSG_MESSAGECHAT_EMOTE:
            type = CHAT_MSG_EMOTE;
            break;
        case CMSG_MESSAGECHAT_PARTY:
            type = CHAT_MSG_PARTY;
            break;
        case CMSG_MESSAGECHAT_RAID:
            type = CHAT_MSG_RAID;
            break;
        case CMSG_MESSAGECHAT_BATTLEGROUND:
            type = CHAT_MSG_INSTANCE_CHAT;
            break;
        case CMSG_MESSAGECHAT_RAID_WARNING:
            type = CHAT_MSG_RAID_WARNING;
            break;
        default:
            TC_LOG_ERROR("network", "HandleMessagechatOpcode : Unknown chat opcode (%u)", recvData.GetOpcode());
            TC_LOG_TRACE("network", "%s", recvData.hexlike().c_str());
            return;
    }

    if (type >= MAX_CHAT_MSG_TYPE)
    {
        TC_LOG_ERROR("network", "CHAT: Wrong message type received: %u", type);
        recvData.rfinish();
        return;
    }

    Player* sender = GetPlayer();

    //TC_LOG_DEBUG("misc", "CHAT: packet received. type %u, lang %u", type, lang);

    // no language sent with emote packet.
    if (type != CHAT_MSG_EMOTE && type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
    {
        recvData >> lang;

        if (sWorld->getBoolConfig(BATTLEGROUND_CROSSFACTION_ENABLED) && lang != LANG_ADDON)
        {
            switch (type)
            {
                case CHAT_MSG_INSTANCE_CHAT:
	    		case CHAT_MSG_INSTANCE_CHAT_LEADER:
                case CHAT_MSG_RAID:
                case CHAT_MSG_RAID_LEADER:
	    		    lang = LANG_UNIVERSAL;
                    break;
	    		default:
	    		    break;
            }
        }

        // prevent talking at unknown language (cheating)
        LanguageDesc const* langDesc = GetLanguageDescByID(lang);
        if (!langDesc)
        {
            SendNotification(LANG_UNKNOWN_LANGUAGE);
            recvData.rfinish();
            return;
        }
        if (langDesc->skill_id != 0 && !sender->HasSkill(langDesc->skill_id))
        {
            // also check SPELL_AURA_COMPREHEND_LANGUAGE (client offers option to speak in that language)
            Unit::AuraEffectList const& langAuras = sender->GetAuraEffectsByType(SPELL_AURA_COMPREHEND_LANGUAGE);
            bool foundAura = false;
            for (Unit::AuraEffectList::const_iterator i = langAuras.begin(); i != langAuras.end(); ++i)
            {
                if ((*i)->GetMiscValue() == int32(lang))
                {
                    foundAura = true;
                    break;
                }
            }
            if (!foundAura)
            {
                SendNotification(LANG_NOT_LEARNED_LANGUAGE);
                recvData.rfinish();
                return;
            }
        }

        if (lang == LANG_ADDON)
        {
            if (sWorld->getBoolConfig(CONFIG_CHATLOG_ADDON))
            {
                std::string msg;
                recvData >> msg;

                if (msg.empty())
                    return;

                sScriptMgr->OnPlayerChat(sender, uint32(CHAT_MSG_ADDON), lang, msg);
            }

            if (type == CHAT_MSG_WHISPER)
            {
                if (!sender->UpdatePmChatTime())
                {
                    SendNotification("You have sent too many whisper messages in a short time interval.");
                    recvData.rfinish();
                    return;
                }
            }

            // Disabled addon channel?
            if (!sWorld->getBoolConfig(CONFIG_ADDON_CHANNEL))
                return;
        }
        // LANG_ADDON should not be changed nor be affected by flood control
        else
        {
            // send in universal language if player in .gm on mode (ignore spell effects)
            if (sender->isGameMaster())
                lang = LANG_UNIVERSAL;
            else
            {
                // send in universal language in two side iteration allowed mode
                if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT))
                    lang = LANG_UNIVERSAL;
                else
                {
                    switch (type)
                    {
                        case CHAT_MSG_PARTY:
                        case CHAT_MSG_PARTY_LEADER:
                        case CHAT_MSG_RAID:
                        case CHAT_MSG_RAID_LEADER:
                        case CHAT_MSG_RAID_WARNING:
                            // allow two side chat at group channel if two side group allowed
                            if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
                                lang = LANG_UNIVERSAL;
                            break;
                        case CHAT_MSG_GUILD:
                        case CHAT_MSG_OFFICER:
                            // allow two side chat at guild channel if two side guild allowed
                            if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD))
                                lang = LANG_UNIVERSAL;
                            break;
                    }
                }

                // but overwrite it by SPELL_AURA_MOD_LANGUAGE auras (only single case used)
                Unit::AuraEffectList const& ModLangAuras = sender->GetAuraEffectsByType(SPELL_AURA_MOD_LANGUAGE);
                if (!ModLangAuras.empty())
                    lang = ModLangAuras.front()->GetMiscValue();
            }

            if (!sender->CanSpeak())
            {
                std::string timeStr = secsToTimeString(m_muteTime - time(NULL));
                SendNotification(GetTrinityString(LANG_WAIT_BEFORE_SPEAKING), timeStr.c_str());
                recvData.rfinish(); // Prevent warnings
                return;
            }

            if (type == CHAT_MSG_CHANNEL || type == CHAT_MSG_YELL)
                sender->UpdateSpeakTime();
        }
    }
    else
        lang = LANG_UNIVERSAL;

    if (sender->HasAura(1852) && type != CHAT_MSG_WHISPER)
    {
        recvData.rfinish();
        SendNotification(GetTrinityString(LANG_GM_SILENCE), sender->GetName().c_str());
        return;
    }

    uint32 textLength = 0;
    uint32 receiverLength = 0;
    std::string to, channel, msg;
    bool ignoreChecks = false;
    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_YELL:
        case CHAT_MSG_PARTY:
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_WARNING:
        case CHAT_MSG_INSTANCE_CHAT:
            textLength = recvData.ReadBits(8);
            msg = recvData.ReadString(textLength);
            break;
        case CHAT_MSG_WHISPER:
            receiverLength = recvData.ReadBits(9);
            textLength = recvData.ReadBits(8);
            to = recvData.ReadString(receiverLength);
            msg = recvData.ReadString(textLength);
            break;
        case CHAT_MSG_CHANNEL:
            textLength = recvData.ReadBits(8);
            receiverLength = 2 * recvData.ReadBits(8);
            receiverLength += recvData.ReadBit();
            msg = recvData.ReadString(textLength);
            channel = recvData.ReadString(receiverLength);
            break;
        case CHAT_MSG_AFK:
        case CHAT_MSG_DND:
            textLength = recvData.ReadBits(8);
            msg = recvData.ReadString(textLength);
            ignoreChecks = true;
            break;
    }

    if (!ignoreChecks)
    {
        if (msg.empty())
            return;

        if (ChatHandler(this).ParseCommands(msg.c_str()))
            return;

        if (!processChatmessageFurtherAfterSecurityChecks(msg, lang))
            return;

        if (msg.empty())
            return;
    }

    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_YELL:
        {
            if (lang != LANG_ADDON)
            {
                if (sender->getLevel() < sWorld->getIntConfig(CONFIG_CHAT_SAY_LEVEL_REQ))
                {
                    SendNotification(GetTrinityString(LANG_SAY_REQ), sWorld->getIntConfig(CONFIG_CHAT_SAY_LEVEL_REQ));
                    return;
                }
                if (type == CHAT_MSG_YELL)
                {
                    if (uint32 minPlayedTime = sWorld->getIntConfig(CONFIG_CHAT_YELL_PLAYED_TIME_REQ))
                    {
                        if (sender->GetTotalPlayedTime() < minPlayedTime * HOUR)
                        {
                            SendNotification(GetTrinityString(LANG_YELL_PLAYED_TIME_REQ), minPlayedTime);
                            return;
                        }
                    }
                }
            }
            if (type == CHAT_MSG_SAY)
                sender->Say(msg, lang);
            else if (type == CHAT_MSG_EMOTE)
                sender->TextEmote(msg);
            else if (type == CHAT_MSG_YELL)
                sender->Yell(msg, lang);

            break;
        }
        case CHAT_MSG_WHISPER:
        {
            if (!normalizePlayerName(to))
            {
                SendPlayerNotFoundNotice(to);
                break;
            }

            Player * const receiver = sObjectAccessor->FindPlayerByName(to);
            if (!receiver)
            {
                SendPlayerNotFoundNotice(to);
                break;
            }

            // Whitelist sender who tries to whisper to GM (if GM accepts whispers)
            bool senderIsPlayer = AccountMgr::IsPlayerAccount(GetSecurity());
            bool receiverIsPlayer = AccountMgr::IsPlayerAccount(receiver ? receiver->GetSession()->GetSecurity() : SEC_PLAYER);
            if (!receiver || (senderIsPlayer && !receiverIsPlayer && !receiver->acceptsWhispers() && !receiver->HasInWhisperWhiteList(sender->GetGUID())))
            {
                if (!receiver->acceptsWhispers())
                {
                    SendPlayerNotFoundNotice(to);
                    return;
                }

                receiver->AddWhisperWhiteList(sender->GetGUID());
            }

            if (lang != LANG_ADDON && !sender->isGameMaster() && !receiver->HasInWhisperWhiteList(sender->GetGUID()))
            {
                if (sender->getLevel() < sWorld->getIntConfig(CONFIG_CHAT_WHISPER_LEVEL_REQ))
                {
                    SendNotification(GetTrinityString(LANG_WHISPER_REQ), sWorld->getIntConfig(CONFIG_CHAT_WHISPER_LEVEL_REQ));
                    return;
                }

                if (uint32 minPlayedTime = sWorld->getIntConfig(CONFIG_CHAT_WHISPER_PLAYED_TIME_REQ))
                {
                    if (sender->GetTotalPlayedTime() < minPlayedTime * HOUR)
                    {
                        SendNotification(GetTrinityString(LANG_WHISPER_PLAYED_TIME_REQ), minPlayedTime);
                        return;
                    }
                }
            }

            if (GetPlayer()->GetTeam() != receiver->GetTeam()
                    && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT))
            {
                SendWrongFactionNotice();
                return;
            }

            if (GetPlayer()->HasAura(1852) && !receiver->isGameMaster())
            {
                SendNotification(GetTrinityString(LANG_GM_SILENCE), GetPlayer()->GetName().c_str());
                return;
            }

            // If player is a Gamemaster and doesn't accept whisper, we auto-whitelist every player that the Gamemaster is talking to
            // We also do that if a player is under the required level for whispers.
            if (receiver->getLevel() < sWorld->getIntConfig(CONFIG_CHAT_WHISPER_LEVEL_REQ) && !sender->acceptsWhispers() && !sender->HasInWhisperWhiteList(receiver->GetGUID()))
            {
                sender->AddWhisperWhiteList(receiver->GetGUID());
            }

            GetPlayer()->Whisper(msg, lang, receiver->GetGUID());

            break;
        }
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        {
            // if player is in battleground, he cannot say to battleground members by /p
            Group* group = GetPlayer()->GetOriginalGroup();
            if (!group)
            {
                group = _player->GetGroup();
                if (!group || group->isBGGroup())
                    return;
            }

            if (group->IsLeader(GetPlayer()->GetGUID()))
                type = CHAT_MSG_PARTY_LEADER;

            sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, uint8(type), lang, NULL, 0, msg.c_str(), NULL);
            group->BroadcastPacket(&data, false, group->GetMemberGroup(GetPlayer()->GetGUID()));

            break;
        }
        case CHAT_MSG_GUILD:
        {
            if (GetPlayer()->GetGuildId())
            {
                if (Guild* guild = sGuildMgr->GetGuildById(GetPlayer()->GetGuildId()))
                {
                    sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, guild);

                    guild->BroadcastToGuild(this, false, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL);
                }
            }

            break;
        }
        case CHAT_MSG_OFFICER:
        {
            if (GetPlayer()->GetGuildId())
            {
                if (Guild* guild = sGuildMgr->GetGuildById(GetPlayer()->GetGuildId()))
                {
                    sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, guild);

                    guild->BroadcastToGuild(this, true, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL);
                }
            }

            break;
        }
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_LEADER:
        {
            // if player is in battleground, he cannot say to battleground members by /ra
            Group* group = GetPlayer()->GetOriginalGroup();
            if (!group)
            {
                group = GetPlayer()->GetGroup();
                if (!group || group->isBGGroup() || !group->isRaidGroup())
                    return;
            }

            if (group->IsLeader(GetPlayer()->GetGUID()))
                type = CHAT_MSG_RAID_LEADER;

            sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, uint8(type), lang, "", 0, msg.c_str(), NULL);
            group->BroadcastPacket(&data, false);

            break;
        }
        case CHAT_MSG_RAID_WARNING:
        {
            Group* group = GetPlayer()->GetGroup();
            if (!group || !group->isRaidGroup() || !(group->IsLeader(GetPlayer()->GetGUID()) || group->IsAssistant(GetPlayer()->GetGUID()) || group->GetGroupType() & GROUPTYPE_EVERYONE_IS_ASSISTANT) || group->isBGGroup())
                return;

            sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);

            WorldPacket data;
            //in battleground, raid warning is sent only to players in battleground - code is ok
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_RAID_WARNING, lang, "", 0, msg.c_str(), NULL);
            group->BroadcastPacket(&data, false);

            break;
        }
        case CHAT_MSG_INSTANCE_CHAT:
        case CHAT_MSG_INSTANCE_CHAT_LEADER:
        {
            // battleground raid is always in Player->GetGroup(), never in GetOriginalGroup()
            Group* group = GetPlayer()->GetGroup();
            if (!group)
                return;

            if (!group->isBGGroup())
            {
                type = CHAT_MSG_RAID;

                if (group->IsLeader(GetPlayer()->GetGUID()))
                    type = CHAT_MSG_RAID_LEADER;
            }
            else if (group->IsLeader(GetPlayer()->GetGUID()))
                type = CHAT_MSG_INSTANCE_CHAT_LEADER;

            sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, uint8(type), lang, "", 0, msg.c_str(), NULL);
            group->BroadcastPacket(&data, false);

            break;
        }
        case CHAT_MSG_CHANNEL:
        {
            if (lang != LANG_ADDON)
            {
                if (_player->getLevel() < sWorld->getIntConfig(CONFIG_CHAT_CHANNEL_LEVEL_REQ))
                {
                    SendNotification(GetTrinityString(LANG_CHANNEL_REQ), sWorld->getIntConfig(CONFIG_CHAT_CHANNEL_LEVEL_REQ));
                    return;
                }
                if (uint32 minPlayedTime = sWorld->getIntConfig(CONFIG_CHAT_CHANNEL_PLAYED_TIME_REQ))
                {
                    if (_player->GetTotalPlayedTime() < minPlayedTime * HOUR)
                    {
                        SendNotification(GetTrinityString(LANG_CHANNEL_PLAYED_TIME_REQ), minPlayedTime);
                        return;
                    }
                }
            }

            if (ChannelMgr* cMgr = ChannelMgr::forTeam(_player->GetTeam()))
            {
                if (Channel* chn = cMgr->GetChannel(channel, _player))
                {
                    sScriptMgr->OnPlayerChat(_player, type, lang, msg, chn);
                    chn->Say(_player->GetGUID(), msg, lang);
                }
            }

            break;
        }
        case CHAT_MSG_AFK:
        {
            if ((msg.empty() || !_player->isAFK()) && !_player->IsInCombat())
            {
                if (!_player->isAFK())
                {
                    if (msg.empty())
                        msg  = GetTrinityString(LANG_PLAYER_AFK_DEFAULT);
                    _player->afkMsg = msg;
                }

                sScriptMgr->OnPlayerChat(_player, type, lang, msg);

                _player->ToggleAFK();
                if (_player->isAFK() && _player->isDND())
                    _player->ToggleDND();
            }

            break;
        }
        case CHAT_MSG_DND:
        {
            if (msg.empty() || !_player->isDND())
            {
                if (!_player->isDND())
                {
                    if (msg.empty())
                        msg = GetTrinityString(LANG_PLAYER_DND_DEFAULT);
                    _player->dndMsg = msg;
                }

                sScriptMgr->OnPlayerChat(_player, type, lang, msg);

                _player->ToggleDND();
                if (_player->isDND() && _player->isAFK())
                    _player->ToggleAFK();
            }

            break;
        }
        default:
            TC_LOG_ERROR("network", "CHAT: unknown message type %u, lang: %u", type, lang);
            break;
    }
}

void WorldSession::HandleAddonMessagechatOpcode(WorldPacket& recvData)
{
    Player* sender = GetPlayer();
    ChatMsg type;

    switch (recvData.GetOpcode())
    {
        case CMSG_MESSAGECHAT_ADDON_BATTLEGROUND:
            type = CHAT_MSG_INSTANCE_CHAT;
            break;
        case CMSG_MESSAGECHAT_ADDON_GUILD:
            type = CHAT_MSG_GUILD;
            break;
        case CMSG_MESSAGECHAT_ADDON_OFFICER:
            type = CHAT_MSG_OFFICER;
            break;
        case CMSG_MESSAGECHAT_ADDON_PARTY:
            type = CHAT_MSG_PARTY;
            break;
        case CMSG_MESSAGECHAT_ADDON_RAID:
            type = CHAT_MSG_RAID;
            break;
        case CMSG_MESSAGECHAT_ADDON_WHISPER:
            type = CHAT_MSG_WHISPER;
            break;
        default:
            TC_LOG_ERROR("network", "HandleAddonMessagechatOpcode: Unknown addon chat opcode (%u)", recvData.GetOpcode());
            TC_LOG_TRACE("network", "%s", recvData.hexlike().c_str());
            return;
    }

    std::string message;
    std::string prefix;
    std::string targetName;

    switch (type)
    {
        case CHAT_MSG_WHISPER:
        {
            uint32 msgLen = recvData.ReadBits(9);
            uint32 prefixLen = recvData.ReadBits(5);
            uint32 targetLen = recvData.ReadBits(8);
            targetName = recvData.ReadString(targetLen);
            message = recvData.ReadString(msgLen);
            prefix = recvData.ReadString(prefixLen);
            break;
        }
        case CHAT_MSG_PARTY:
        {
            uint32 prefixLen = recvData.ReadBits(5);
            uint32 msgLen = recvData.ReadBits(8);
            prefix = recvData.ReadString(prefixLen);
            message = recvData.ReadString(msgLen);
            break;
        }
        case CHAT_MSG_RAID:
        {
            uint32 msgLen = recvData.ReadBits(8);
            uint32 prefixLen = recvData.ReadBits(5);
            prefix = recvData.ReadString(prefixLen);
            message = recvData.ReadString(msgLen);
            break;
        }
        case CHAT_MSG_OFFICER:
        case CHAT_MSG_GUILD:
        case CHAT_MSG_INSTANCE_CHAT:
        {
            uint32 msgLen = recvData.ReadBits(8);
            uint32 prefixLen = recvData.ReadBits(5);
            prefix = recvData.ReadString(prefixLen);
            message = recvData.ReadString(msgLen);
            break;
        }
        default:
            break;
    }

    // Logging enabled?
    if (sWorld->getBoolConfig(CONFIG_CHATLOG_ADDON))
    {
        if (message.empty())
            return;

        // Weird way to log stuff...
        sScriptMgr->OnPlayerChat(sender, CHAT_MSG_ADDON, LANG_ADDON, message);
    }

    // Disabled addon channel?
    if (!sWorld->getBoolConfig(CONFIG_ADDON_CHANNEL))
        return;

    switch (type)
    {
        case CHAT_MSG_INSTANCE_CHAT:
        {
            Group* group = sender->GetGroup();
            if (!group || !group->isBGGroup())
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, type, LANG_ADDON, "", 0, message.c_str(), NULL);
            group->BroadcastAddonMessagePacket(&data, prefix, false);
            break;
        }
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        {
            if (sender->GetGuildId())
                if (Guild* guild = sGuildMgr->GetGuildById(sender->GetGuildId()))
                    guild->BroadcastAddonToGuild(this, type == CHAT_MSG_OFFICER, message, prefix);
            break;
        }
        case CHAT_MSG_WHISPER:
        {
            if (!normalizePlayerName(targetName))
                break;
            Player* receiver = sObjectAccessor->FindPlayerByName(targetName);
            if (!receiver)
                break;

            sender->WhisperAddon(message, prefix, receiver);
            break;
        }
        // Messages sent to "RAID" while in a party will get delivered to "PARTY"
        case CHAT_MSG_PARTY:
        case CHAT_MSG_RAID:
        {

            Group* group = sender->GetGroup();
            if (!group || group->isBGGroup())
                break;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, type, LANG_ADDON, "", 0, message.c_str(), NULL, prefix.c_str());
            group->BroadcastAddonMessagePacket(&data, prefix, true, -1, group->GetMemberGroup(sender->GetGUID()));
            break;
        }
        default:
        {
            TC_LOG_ERROR("misc", "HandleAddonMessagechatOpcode: unknown addon message type %u", type);
            break;
        }
    }
}

void WorldSession::HandleEmoteOpcode(WorldPacket & recvData)
{
    if (!GetPlayer()->IsAlive() || GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        return;

    if (!GetPlayer()->CanSpeak())
    {
        std::string timeStr = secsToTimeString(m_muteTime - time(NULL));
        SendNotification(GetTrinityString(LANG_WAIT_BEFORE_SPEAKING), timeStr.c_str());
        recvData.rfinish(); // Prevent warnings
        return;
    }

    uint32 emote;
    recvData >> emote;

    if (emote != EMOTE_ONESHOT_NONE)
        GetPlayer()->UpdateSpeakTime();

    sScriptMgr->OnPlayerEmote(GetPlayer(), emote);
    GetPlayer()->HandleEmoteCommand(emote);
}

namespace Trinity
{
    class EmoteChatBuilder
    {
        public:
            EmoteChatBuilder(Player const& player, uint32 text_emote, uint32 emote_num, Unit const* target)
                : i_player(player), i_text_emote(text_emote), i_emote_num(emote_num), i_target(target) {}

            void operator()(WorldPacket& data, LocaleConstant /*loc_idx*/)
            {
                ObjectGuid playerGuid = i_player.GetGUID();
                ObjectGuid targetGuid = i_target ? i_target->GetGUID() : 0;

                data.Initialize(SMSG_TEXT_EMOTE);
                data.WriteBitSeq<0>(playerGuid);
                data.WriteBitSeq<3, 4>(targetGuid);
                data.WriteBitSeq<6, 7, 3>(playerGuid);
                data.WriteBitSeq<6, 7>(targetGuid);
                data.WriteBitSeq<5, 2, 1>(playerGuid);
                data.WriteBitSeq<0>(targetGuid);
                data.WriteBitSeq<4>(playerGuid);
                data.WriteBitSeq<1, 5, 2>(targetGuid);

                data.WriteByteSeq<4, 5, 1>(playerGuid);
                data.WriteByteSeq<6>(targetGuid);
                data << int32(i_text_emote);
                data.WriteByteSeq<7, 1, 4>(targetGuid);
                data << uint32(i_emote_num);
                data.WriteByteSeq<0>(targetGuid);
                data.WriteByteSeq<7, 3>(playerGuid);
                data.WriteByteSeq<2>(targetGuid);
                data.WriteByteSeq<6, 2>(playerGuid);
                data.WriteByteSeq<5>(targetGuid);
                data.WriteByteSeq<0>(playerGuid);
                data.WriteByteSeq<3>(targetGuid);
            }

        private:
            Player const& i_player;
            uint32        i_text_emote;
            uint32        i_emote_num;
            Unit const*   i_target;
    };
}                                                           // namespace Trinity

void WorldSession::HandleTextEmoteOpcode(WorldPacket & recvData)
{
    if (!GetPlayer()->IsAlive())
        return;

    if (!GetPlayer()->CanSpeak())
    {
        std::string timeStr = secsToTimeString(m_muteTime - time(NULL));
        SendNotification(GetTrinityString(LANG_WAIT_BEFORE_SPEAKING), timeStr.c_str());
        return;
    }


    uint32 text_emote, emoteNum;
    ObjectGuid guid;

    recvData >> emoteNum;
    recvData >> text_emote;

    // these emotes have no text entries
    if (text_emote != TEXT_EMOTE_READ && text_emote != TEXT_EMOTE_MOUNT_SPECIAL)
        GetPlayer()->UpdateSpeakTime();

    recvData.ReadBitSeq<4, 2, 5, 6, 0, 3, 7, 1>(guid);
    recvData.ReadByteSeq<4, 1, 5, 2, 3, 0, 6, 7>(guid);

    sScriptMgr->OnPlayerTextEmote(GetPlayer(), text_emote, emoteNum, guid);

    EmotesTextEntry const* em = sEmotesTextStore.LookupEntry(text_emote);
    if (!em)
        return;

    uint32 emote_anim = em->textid;

    switch (emote_anim)
    {
        case EMOTE_STATE_SLEEP:
            GetPlayer()->SetStandState(UNIT_STAND_STATE_SLEEP);
            break;
        case EMOTE_STATE_SIT:
            GetPlayer()->SetStandState(UNIT_STAND_STATE_SIT);
            break;
        case EMOTE_STATE_KNEEL:
            GetPlayer()->SetStandState(UNIT_STAND_STATE_KNEEL);
            break;
        case EMOTE_ONESHOT_NONE:
            break;
        case EMOTE_STATE_READ:
            GetPlayer()->SetUInt32Value(UNIT_NPC_EMOTESTATE, emote_anim);
            break;
        default:
            // Only allow text-emotes for "dead" entities (feign death included)
            if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
                break;
            GetPlayer()->HandleEmoteCommand(emote_anim);
            break;
    }

    Unit* unit = ObjectAccessor::GetUnit(*_player, guid);

    CellCoord p = Trinity::ComputeCellCoord(GetPlayer()->GetPositionX(), GetPlayer()->GetPositionY());

    Cell cell(p);
    cell.SetNoCreate();

    Trinity::EmoteChatBuilder emote_builder(*GetPlayer(), emote_anim, text_emote, unit);
    Trinity::LocalizedPacketDo<Trinity::EmoteChatBuilder > emote_do(emote_builder);
    Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::EmoteChatBuilder > > emote_worker(GetPlayer(), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), emote_do);

    cell.Visit(p, Trinity::makeWorldVisitor(emote_worker), *GetPlayer()->GetMap(), *GetPlayer(), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE));

    GetPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE, text_emote, 0, 0, unit);

    //Send scripted event call
    if (unit && unit->GetTypeId() == TYPEID_UNIT && ((Creature*)unit)->AI())
        ((Creature*)unit)->AI()->ReceiveEmote(GetPlayer(), text_emote);
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint8 unk;
    //TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "WORLD: Received CMSG_CHAT_IGNORED");

    recvData >> unk;                                       // probably related to spam reporting

    recvData.ReadBitSeq<5, 7, 3, 1, 4, 0, 6, 2>(guid);
    recvData.ReadByteSeq<3, 7, 0, 5, 2, 6, 1, 4>(guid);

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player || !player->GetSession())
        return;

    WorldPacket data;
    ChatHandler::FillMessageData(&data, this, CHAT_MSG_IGNORED, LANG_UNIVERSAL, NULL, GetPlayer()->GetGUID(), GetPlayer()->GetName().c_str(), NULL);
    player->GetSession()->SendPacket(&data);
}

void WorldSession::HandleChannelDeclineInvite(WorldPacket &recvPacket)
{
    TC_LOG_DEBUG("network", "Opcode %u", recvPacket.GetOpcode());
}

void WorldSession::SendPlayerNotFoundNotice(std::string const &name)
{
    WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, name.size()+1);

    data.WriteBits((name.size() - (name.size() % 2)) / 2, 8);
    data.WriteBit((name.size() % 2));
    data.FlushBits();
    data.append(name.c_str(), name.size());
    SendPacket(&data);
}

void WorldSession::SendPlayerAmbiguousNotice(std::string const &name)
{
    WorldPacket data(SMSG_CHAT_PLAYER_AMBIGUOUS, name.size()+1);
    data << name;
    SendPacket(&data);
}

void WorldSession::SendWrongFactionNotice()
{
    WorldPacket data(SMSG_CHAT_WRONG_FACTION, 0);
    SendPacket(&data);
}

void WorldSession::SendChatRestrictedNotice(ChatRestrictionType restriction)
{
    WorldPacket data(SMSG_CHAT_RESTRICTED, 1);
    data << uint8(restriction);
    SendPacket(&data);
}
