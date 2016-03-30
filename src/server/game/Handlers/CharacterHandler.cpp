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
#include "SystemConfig.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "Arena.h"
#include "Chat.h"
#include "Group.h"
#include "Guild.h"
#include "GuildFinderMgr.h"
#include "Language.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "PlayerDump.h"
#include "SharedDefines.h"
#include "SocialMgr.h"
#include "UpdateMask.h"
#include "Util.h"
#include "ScriptMgr.h"
#include "Battleground.h"
#include "AccountMgr.h"
#include "DBCStores.h"
#include "LFGMgr.h"
#include "DB2Stores.h"
#include "SpellAuraEffects.h"
#include "BattlePetMgr.h"

class CharLoginQueryHolder final : public SQLQueryHolder
{
    uint64 m_guid;
    uint32 m_accountId;

public:
    CharLoginQueryHolder(uint64 guid, uint32 accountId)
        : m_guid(guid)
        , m_accountId(accountId)
    { }

    uint64 GetGuid() const { return m_guid; }
    uint32 GetAccountId() const { return m_accountId; }
    bool Initialize();
};

class AuthLoginQueryHolder final : public SQLQueryHolder
{
    uint32 m_accountId;

public:
    AuthLoginQueryHolder(uint32 accountId)
        : m_accountId(accountId)
    { }

    uint32 GetAccountId() const { return m_accountId; }
    bool Initialize();
};

bool CharLoginQueryHolder::Initialize()
{
    SetSize(MAX_CHAR_LOGIN_QUERY);

    bool res = true;
    uint32 lowGuid = GUID_LOPART(m_guid);
    PreparedStatement* stmt = NULL;

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_FROM, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GROUP_MEMBER);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_GROUP, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_INSTANCE);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_BOUND_INSTANCES, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_AURAS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_AURAS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_AURAS_EFFECTS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_AURAS_EFFECTS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SPELL);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_SPELLS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_QUESTSTATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_QUEST_OBJECTIVE_STATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_QUEST_OBJECTIVE_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_DAILYQUESTSTATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_DAILY_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_WEEKLYQUESTSTATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_WEEKLY_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_MONTHLYQUESTSTATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_MONTHLY_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SEASONALQUESTSTATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_SEASONAL_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_REPUTATION);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_REPUTATION, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_INVENTORY);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_INVENTORY, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_VOID_STORAGE);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_VOID_STORAGE, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_ACTIONS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_ACTIONS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_MAILCOUNT);
    stmt->setUInt32(0, lowGuid);
    stmt->setUInt64(1, uint64(time(NULL)));
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_MAIL_COUNT, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_MAILDATE);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_MAIL_DATE, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SOCIALLIST);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_SOCIAL_LIST, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_HOMEBIND);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_HOMEBIND, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SPELLCOOLDOWNS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_SPELL_COOLDOWNS, stmt);

    if (sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_DECLINEDNAMES);
        stmt->setUInt32(0, lowGuid);
        res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_DECLINED_NAMES, stmt);
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_MEMBER);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_GUILD, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_ACHIEVEMENTS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_ACHIEVEMENTS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_CRITERIAPROGRESS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_CRITERIA_PROGRESS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_EQUIPMENTSETS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_EQUIPMENT_SETS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_ARENA_DATA);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_ARENA_DATA, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_BGDATA);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_BG_DATA, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_GLYPHS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_GLYPHS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_TALENTS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_TALENTS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PLAYER_ACCOUNT_DATA);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_ACCOUNT_DATA, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SKILLS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_SKILLS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_RANDOMBG);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_RANDOM_BG, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_BANNED);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_BANNED, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_QUESTSTATUSREW);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_QUEST_STATUS_REW, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_INSTANCELOCKTIMES);
    stmt->setUInt32(0, m_accountId);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_INSTANCE_LOCK_TIMES, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_CP_WEEK_CAP);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_CP_WEEK_CAP, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PLAYER_CURRENCY);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_CURRENCY, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_ARCHAEOLOGY);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_ARCHAEOLOGY, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CUF_PROFILE);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_CUF_PROFILES, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_KNOWN_TITLES);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_KNOWN_TITLES, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_RATED_BG_STATS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_RATED_BG_STATS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_LFR_LOOT_BOUND);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(CHAR_LOGIN_QUERY_LOAD_LFR_LOOT_BOUND, stmt);

    return res;
}

bool AuthLoginQueryHolder::Initialize()
{
    SetSize(MAX_AUTH_LOGIN_QUERY);

    bool res = true;
    PreparedStatement* stmt = NULL;

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_SPELL);
    stmt->setUInt32(0, GetAccountId());
    res &= SetPreparedQuery(AUTH_LOGIN_QUERY_LOAD_SPELLS, stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ACHIEVEMENTS);
    stmt->setUInt32(0, GetAccountId());
    res &= SetPreparedQuery(AUTH_LOGIN_QUERY_LOAD_ACHIEVEMENTS, stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_CRITERIAPROGRESS);
    stmt->setUInt32(0, m_accountId);
    res &= SetPreparedQuery(AUTH_LOGIN_QUERY_LOAD_CRITERIA_PROGRESS, stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BATTLE_PETS);
    stmt->setUInt32(0, GetAccountId());
    res &= SetPreparedQuery(AUTH_LOGIN_QUERY_LOAD_BATTLE_PETS, stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BATTLE_PET_SLOTS);
    stmt->setUInt32(0, GetAccountId());
    res &= SetPreparedQuery(AUTH_LOGIN_QUERY_LOAD_BATTLE_PET_SLOTS, stmt);

    return res;
}

void WorldSession::HandleCharEnum(PreparedQueryResult result)
{
    uint32 unkCount = 0;
    uint32 charCount = 0;
    ByteBuffer bitBuffer;
    ByteBuffer dataBuffer;

    bitBuffer.WriteBit(1); // Must send 1, else receive CHAR_LIST_FAILED error
    bitBuffer.WriteBits(unkCount, 21); // unk uint32 count

    if (result)
    {
        _legitCharacters.clear();

        charCount = uint32(result->GetRowCount());

        bitBuffer.WriteBits(charCount, 16);

        do
        {
            uint32 guidLow = (*result)[0].GetUInt32();

            TC_LOG_INFO("network", "Loading char guid %u from account %u.", guidLow, GetAccountId());

            Player::BuildEnumData(result, &dataBuffer, &bitBuffer);

            _legitCharacters.insert(guidLow);
        }
        while (result->NextRow());
    }
    else
        bitBuffer.WriteBits(0, 16);

    bitBuffer.FlushBits();

    WorldPacket data(SMSG_CHAR_ENUM, bitBuffer.size() + dataBuffer.size());
    data.append(bitBuffer);
    if (charCount)
        data.append(dataBuffer);

    SendPacket(&data);
}

void WorldSession::HandleCharEnumOpcode(WorldPacket& /*recvData*/)
{
    time_t now = time(NULL);
    if (ignoreNextCharEnumCheck)
        ignoreNextCharEnumCheck = false;
    else
    {
        if (now - timeCharEnumOpcode < 5)
            return;
        else
            timeCharEnumOpcode = now;
    }

    // remove expired bans
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_EXPIRED_BANS);
    CharacterDatabase.Execute(stmt);

    /// get all the data necessary for loading all characters (along with their pets) on the account
    if (sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ENUM_DECLINED_NAME);
    else
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ENUM);

    stmt->setUInt32(0, GetAccountId());

    _charEnumCallback = CharacterDatabase.AsyncQuery(stmt);
}

void WorldSession::HandleCharCreateOpcode(WorldPacket& recvData)
{
    std::string name;
    uint32 name_length = 0;
    uint8 race_, class_;
    // extract other data required for player creating
    uint8 gender, skin, face, hairStyle, hairColor, facialHair, outfitId;
    bool unkBit = false;
    uint32 unk32bits = 0;

    outfitId = 0;

    recvData >> face;
    recvData >> gender;
    recvData >> race_;
    recvData >> facialHair;
    recvData >> hairColor;
    recvData >> outfitId;
    recvData >> class_;
    recvData >> skin;
    recvData >> hairStyle;

    unkBit = recvData.ReadBit();

    name_length = recvData.ReadBits(6);
    name = recvData.ReadString(name_length);

    if (unkBit)
        recvData >> unk32bits;

    WorldPacket data(SMSG_CHAR_CREATE, 1);                  // returned with diff.values in all cases
    if (uint32 mask = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED))
    {
        bool disabled = false;

        uint32 team = Player::TeamForRace(race_);
        switch (team)
        {
            case ALLIANCE: disabled = mask & (1 << 0); break;
            case HORDE:    disabled = mask & (1 << 1); break;
        }

        if (disabled)
        {
            data << (uint8)CHAR_CREATE_DISABLED;
            SendPacket(&data);
            return;
        }
    }

    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(class_);
    if (!classEntry)
    {
        data << (uint8)CHAR_CREATE_FAILED;
        SendPacket(&data);
        TC_LOG_ERROR("network", "Class (%u) not found in DBC while creating new char for account (ID: %u): wrong DBC files or cheater?", class_, GetAccountId());
        return;
    }

    ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(race_);
    if (!raceEntry)
    {
        data << (uint8)CHAR_CREATE_FAILED;
        SendPacket(&data);
        TC_LOG_ERROR("network", "Race (%u) not found in DBC while creating new char for account (ID: %u): wrong DBC files or cheater?", race_, GetAccountId());
        return;
    }

    uint32 raceMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK);
    if ((1 << (race_ - 1)) & raceMaskDisabled)
    {
        data << uint8(CHAR_CREATE_DISABLED);
        SendPacket(&data);
        return;
    }

    uint32 classMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK);
    if ((1 << (class_ - 1)) & classMaskDisabled)
    {
        data << uint8(CHAR_CREATE_DISABLED);
        SendPacket(&data);
        return;
    }

    // prevent character creating with invalid name
    if (!normalizePlayerName(name))
    {
        data << (uint8)CHAR_NAME_NO_NAME;
        SendPacket(&data);
        TC_LOG_ERROR("network", "Account:[%d] but tried to Create character with empty [name] ", GetAccountId());
        return;
    }

    // check name limitations
    uint8 res = ObjectMgr::CheckPlayerName(name, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        data << uint8(res);
        SendPacket(&data);
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sObjectMgr->IsReservedName(name))
    {
        data << (uint8)CHAR_NAME_RESERVED;
        SendPacket(&data);
        return;
    }

    if (class_ == CLASS_DEATH_KNIGHT && AccountMgr::IsPlayerAccount(GetSecurity()))
    {
        // speedup check for heroic class disabled case
        uint32 heroic_free_slots = sWorld->getIntConfig(CONFIG_HEROIC_CHARACTERS_PER_REALM);
        if (heroic_free_slots == 0 && AccountMgr::IsPlayerAccount(GetSecurity()))
        {
            data << (uint8)CHAR_CREATE_UNIQUE_CLASS_LIMIT;
            SendPacket(&data);
            return;
        }

        // speedup check for heroic class disabled case
        uint32 req_level_for_heroic = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER);
        if (AccountMgr::IsPlayerAccount(GetSecurity()) && req_level_for_heroic > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            data << (uint8)CHAR_CREATE_LEVEL_REQUIREMENT;
            SendPacket(&data);
            return;
        }
    }

    delete _charCreateCallback.GetParam();  // Delete existing if any, to make the callback chain reset to stage 0
    _charCreateCallback.SetParam(new CharacterCreateInfo(name, race_, class_, gender, skin, face, hairStyle, hairColor, facialHair, outfitId, recvData));
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
    stmt->setString(0, name);
    _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleCharCreateCallback(PreparedQueryResult result, CharacterCreateInfo* createInfo)
{
    /** This is a series of callbacks executed consecutively as a result from the database becomes available.
        This is much more efficient than synchronous requests on packet handler, and much less DoS prone.
        It also prevents data syncrhonisation errors.
    */
    switch (_charCreateCallback.GetStage())
    {
        case 0:
        {
            if (result)
            {
                WorldPacket data(SMSG_CHAR_CREATE, 1);
                data << uint8(CHAR_CREATE_NAME_IN_USE);
                SendPacket(&data);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            ASSERT(_charCreateCallback.GetParam() == createInfo);

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_SUM_REALM_CHARACTERS);
            stmt->setUInt32(0, GetAccountId());

            _charCreateCallback.FreeResult();
            _charCreateCallback.SetFutureResult(LoginDatabase.AsyncQuery(stmt));
            _charCreateCallback.NextStage();
        }
        break;
        case 1:
        {
            uint16 acctCharCount = 0;
            if (result)
            {
                Field* fields = result->Fetch();
                // SELECT SUM(x) is MYSQL_TYPE_NEWDECIMAL - needs to be read as string
                const char* ch = fields[0].GetCString();
                if (ch)
                    acctCharCount = atoi(ch);
            }

            if (acctCharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_ACCOUNT))
            {
                WorldPacket data(SMSG_CHAR_CREATE, 1);
                data << uint8(CHAR_CREATE_ACCOUNT_LIMIT);
                SendPacket(&data);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }


            ASSERT(_charCreateCallback.GetParam() == createInfo);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_SUM_CHARS);
            stmt->setUInt32(0, GetAccountId());

            _charCreateCallback.FreeResult();
            _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
            _charCreateCallback.NextStage();
        }
        break;
        case 2:
        {
            if (result)
            {
                Field* fields = result->Fetch();
                createInfo->CharCount = uint8(fields[0].GetUInt64()); // SQL's COUNT() returns uint64 but it will always be less than uint8.Max

                if (createInfo->CharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
                {
                    WorldPacket data(SMSG_CHAR_CREATE, 1);
                    data << uint8(CHAR_CREATE_SERVER_LIMIT);
                    SendPacket(&data);
                    delete createInfo;
                    _charCreateCallback.Reset();
                    return;
                }
            }

            bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || !AccountMgr::IsPlayerAccount(GetSecurity());
            uint32 skipCinematics = sWorld->getIntConfig(CONFIG_SKIP_CINEMATICS);

            _charCreateCallback.FreeResult();

            if (!allowTwoSideAccounts || skipCinematics == 1 || createInfo->Class == CLASS_DEATH_KNIGHT)
            {
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_CREATE_INFO);
                stmt->setUInt32(0, GetAccountId());
                stmt->setUInt32(1, (skipCinematics == 1 || createInfo->Class == CLASS_DEATH_KNIGHT) ? 10 : 1);
                _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
                _charCreateCallback.NextStage();
                return;
            }

            _charCreateCallback.NextStage();
            HandleCharCreateCallback(PreparedQueryResult(NULL), createInfo);   // Will jump to case 3
        }
        break;
        case 3:
        {
            bool haveSameRace = false;
            uint32 heroicReqLevel = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER);
            bool hasHeroicReqLevel = (heroicReqLevel == 0);
            bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || !AccountMgr::IsPlayerAccount(GetSecurity());
            uint32 skipCinematics = sWorld->getIntConfig(CONFIG_SKIP_CINEMATICS);
            bool checkHeroicReqs = createInfo->Class == CLASS_DEATH_KNIGHT && AccountMgr::IsPlayerAccount(GetSecurity());

            if (result)
            {
                uint32 team = Player::TeamForRace(createInfo->Race);
                uint32 freeHeroicSlots = sWorld->getIntConfig(CONFIG_HEROIC_CHARACTERS_PER_REALM);

                Field* field = result->Fetch();
                uint8 accRace  = field[1].GetUInt8();

                if (checkHeroicReqs)
                {
                    uint8 accClass = field[2].GetUInt8();
                    if (accClass == CLASS_DEATH_KNIGHT)
                    {
                        if (freeHeroicSlots > 0)
                            --freeHeroicSlots;

                        if (freeHeroicSlots == 0)
                        {
                            WorldPacket data(SMSG_CHAR_CREATE, 1);
                            data << uint8(CHAR_CREATE_UNIQUE_CLASS_LIMIT);
                            SendPacket(&data);
                            delete createInfo;
                            _charCreateCallback.Reset();
                            return;
                        }
                    }

                    if (!hasHeroicReqLevel)
                    {
                        uint8 accLevel = field[0].GetUInt8();
                        if (accLevel >= heroicReqLevel)
                            hasHeroicReqLevel = true;
                    }
                }

                // need to check team only for first character
                // TODO: what to if account already has characters of both races?
                if (!allowTwoSideAccounts)
                {
                    uint32 accTeam = 0;
                    if (accRace > 0)
                        accTeam = Player::TeamForRace(accRace);

                    if (accTeam != team)
                    {
                        WorldPacket data(SMSG_CHAR_CREATE, 1);
                        data << uint8(CHAR_CREATE_PVP_TEAMS_VIOLATION);
                        SendPacket(&data);
                        delete createInfo;
                        _charCreateCallback.Reset();
                        return;
                    }
                }

                // search same race for cinematic or same class if need
                // TODO: check if cinematic already shown? (already logged in?; cinematic field)
                while ((skipCinematics == 1 && !haveSameRace) || createInfo->Class == CLASS_DEATH_KNIGHT)
                {
                    if (!result->NextRow())
                        break;

                    field = result->Fetch();
                    accRace = field[1].GetUInt8();

                    if (!haveSameRace)
                        haveSameRace = createInfo->Race == accRace;

                    if (checkHeroicReqs)
                    {
                        uint8 acc_class = field[2].GetUInt8();
                        if (acc_class == CLASS_DEATH_KNIGHT)
                        {
                            if (freeHeroicSlots > 0)
                                --freeHeroicSlots;

                            if (freeHeroicSlots == 0)
                            {
                                WorldPacket data(SMSG_CHAR_CREATE, 1);
                                data << uint8(CHAR_CREATE_UNIQUE_CLASS_LIMIT);
                                SendPacket(&data);
                                delete createInfo;
                                _charCreateCallback.Reset();
                                return;
                            }
                        }

                        if (!hasHeroicReqLevel)
                        {
                            uint8 acc_level = field[0].GetUInt8();
                            if (acc_level >= heroicReqLevel)
                                hasHeroicReqLevel = true;
                        }
                    }
                }
            }

            if (checkHeroicReqs && !hasHeroicReqLevel)
            {
                WorldPacket data(SMSG_CHAR_CREATE, 1);
                data << uint8(CHAR_CREATE_LEVEL_REQUIREMENT);
                SendPacket(&data);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            if (createInfo->Data.rpos() < createInfo->Data.wpos())
            {
                uint8 unk;
                createInfo->Data >> unk;
                TC_LOG_DEBUG("network", "Character creation %s (account %u) has unhandled tail data: [%u]", createInfo->Name.c_str(), GetAccountId(), unk);
            }

            Player newChar(this);
            newChar.GetMotionMaster()->Initialize();
            if (!newChar.Create(sObjectMgr->GenerateLowGuid(HIGHGUID_PLAYER), createInfo))
            {
                // Player not create (race/class/etc problem?)
                newChar.CleanupsBeforeDelete();

                WorldPacket data(SMSG_CHAR_CREATE, 1);
                data << uint8(CHAR_CREATE_ERROR);
                SendPacket(&data);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            if ((haveSameRace && skipCinematics == 1) || skipCinematics == 2)
                newChar.setCinematic(1);                          // not show intro

            newChar.SetAtLoginFlag(AT_LOGIN_FIRST);               // First login

            // Player created, save it now
            newChar.SaveToDB(true);
            createInfo->CharCount += 1;

            SQLTransaction trans = LoginDatabase.BeginTransaction();

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_REALM_CHARACTERS_BY_REALM);
            stmt->setUInt32(0, GetAccountId());
            stmt->setUInt32(1, realmID);
            trans->Append(stmt);

            stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS);
            stmt->setUInt32(0, createInfo->CharCount);
            stmt->setUInt32(1, GetAccountId());
            stmt->setUInt32(2, realmID);
            trans->Append(stmt);

            LoginDatabase.CommitTransaction(trans);

            WorldPacket data(SMSG_CHAR_CREATE, 1);
            data << uint8(CHAR_CREATE_SUCCESS);
            SendPacket(&data);

            std::string const &addr = GetRemoteAddress();
            TC_LOG_INFO("entities.player.character", "Account: %d (IP: %s) Create Character:[%s] (GUID: %u)", GetAccountId(), addr.c_str(), createInfo->Name.c_str(), newChar.GetGUIDLow());
            sScriptMgr->OnPlayerCreate(&newChar);

            newChar.CleanupsBeforeDelete();
            delete createInfo;
            _charCreateCallback.Reset();
        }
        break;
    }
}

void WorldSession::HandleCharDeleteOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<3, 5>(guid);

    recvData.ReadBit();

    recvData.ReadBitSeq<6, 4, 2, 7, 1, 0>(guid);

    recvData.ReadByteSeq<7, 5, 0, 1, 2, 4, 6, 3>(guid);

    if (!IS_PLAYER_GUID(guid) || !IsLegitCharacterForAccount(GUID_LOPART(guid)))
        return KickPlayer();

    // can't delete loaded character
    if (ObjectAccessor::FindPlayer(guid))
        return;

    uint32 accountId = 0;
    uint8 level = 0;
    std::string name;

    // is guild leader
    if (sGuildMgr->GetGuildByLeader(guid))
    {
        WorldPacket data(SMSG_CHAR_DELETE, 1);
        data << uint8(CHAR_DELETE_FAILED_GUILD_LEADER);
        SendPacket(&data);
        return;
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_NAME_BY_GUID);
    stmt->setUInt32(0, GUID_LOPART(guid));

    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();
        accountId = fields[0].GetUInt32();
        name = fields[1].GetString();
    }

    // prevent deleting other players' characters using cheating tools
    if (accountId != GetAccountId())
        return;

    std::string const &ipStr = GetRemoteAddress();
    TC_LOG_INFO("entities.player.character.delete", "Account: %u. IP: %s. Name: %s. GUID: %u. Level: %u",
                accountId, ipStr.c_str(), name.c_str(), GUID_LOPART(guid), level);

    sScriptMgr->OnPlayerDelete(guid);

    sGuildFinderMgr->RemoveAllMembershipRequestsFromPlayer(guid);
    Player::DeleteFromDB(guid, accountId);

    ignoreNextCharEnumCheck = true;

    WorldPacket data(SMSG_CHAR_DELETE, 1);
    data << uint8(CHAR_DELETE_SUCCESS);
    SendPacket(&data);
}

void WorldSession::HandlePlayerLoginOpcode(WorldPacket& recvData)
{
    if (PlayerLoading() || GetPlayer() != NULL)
    {
        TC_LOG_ERROR("network", "Player tries to login again, AccountId = %d", GetAccountId());
        return;
    }

    m_playerLoading = true;
    ObjectGuid playerGuid;

    TC_LOG_DEBUG("network", "WORLD: Recvd Player Logon Message");

    float farClip = 0.0f;
    recvData >> farClip;

    recvData.ReadBitSeq<2, 3, 7, 4, 0, 1, 5, 6>(playerGuid);
    recvData.ReadByteSeq<0, 1, 3, 4, 7, 6, 2, 5>(playerGuid);

    TC_LOG_DEBUG("network", "Character (Guid: %u) logging in", GUID_LOPART(playerGuid));

    if (!IsLegitCharacterForAccount(GUID_LOPART(playerGuid)))
    {
        TC_LOG_ERROR("network", "Account (%u) can't login with that character (%u).", GetAccountId(), GUID_LOPART(playerGuid));
        KickPlayer();
        return;
    }

    auto const charHolder = new CharLoginQueryHolder(playerGuid, GetAccountId());
    auto const authHolder = new AuthLoginQueryHolder(GetAccountId());

    if (!charHolder->Initialize() || !authHolder->Initialize())
    {
        delete charHolder;
        delete authHolder;
        m_playerLoading = false;
        return;
    }

    _charLoginCallback = CharacterDatabase.DelayQueryHolder(charHolder);
    _authLoginCallback = LoginDatabase.DelayQueryHolder(authHolder);
}

void WorldSession::HandleLoadScreenOpcode(WorldPacket& recvPacket)
{
    TC_LOG_INFO("misc", "WORLD: Recvd CMSG_LOAD_SCREEN");
    uint32 mapID;

    recvPacket >> mapID;
    recvPacket.ReadBit();

    if (auto const player = GetPlayer())
    {
        // Refresh spellmods for client
        // This is Hackypig fix : find a better way
        for (auto const &eff : player->GetAuraEffectsByType(SPELL_AURA_ADD_PCT_MODIFIER))
        {
            player->AddSpellMod(eff->GetSpellModifier(), false);
            player->AddSpellMod(eff->GetSpellModifier(), true);
        }
        for (auto const &eff : player->GetAuraEffectsByType(SPELL_AURA_ADD_FLAT_MODIFIER))
        {
            player->AddSpellMod(eff->GetSpellModifier(), false);
            player->AddSpellMod(eff->GetSpellModifier(), true);
        }

        if (player->hasForcedMovement())
            player->SendApplyMovementForce(false, *player);
    }
}

void WorldSession::HandlePlayerLogin(CharLoginQueryHolder *charHolder, AuthLoginQueryHolder *authHolder)
{
    uint64 playerGuid = charHolder->GetGuid();

    Player* pCurrChar = new Player(this);
     // for send server info and strings (config)
    ChatHandler chH = ChatHandler(pCurrChar);

    // "GetAccountId() == db stored account id" checked in LoadFromDB (prevent login not own character using cheating tools)
    if (!pCurrChar->LoadFromDB(GUID_LOPART(playerGuid), charHolder, authHolder))
    {
        SetPlayer(NULL);
        KickPlayer();                                       // disconnect client, player no set to session and it will not deleted or saved at kick
        delete pCurrChar;                                   // delete it manually
        delete charHolder;                                  // delete all unprocessed queries
        delete authHolder;
        m_playerLoading = false;
        return;
    }

    pCurrChar->GetMotionMaster()->Initialize();
    pCurrChar->SendDungeonDifficulty(false);

    WorldPacket data(SMSG_SUSPEND_TOKEN_RESPONSE, 5);
    data << uint8(0x80);
    data << uint32(0);
    SendPacket(&data);

    data.Initialize(SMSG_LOGIN_VERIFY_WORLD, 20);
    data << pCurrChar->GetPositionZ();
    data << pCurrChar->GetMapId();
    data << pCurrChar->GetPositionY();
    data << pCurrChar->GetPositionX();
    data << pCurrChar->GetOrientation();
    SendPacket(&data);

    // load player specific part before send times
    LoadAccountData(charHolder->GetPreparedResult(CHAR_LOGIN_QUERY_LOAD_ACCOUNT_DATA), PER_CHARACTER_CACHE_MASK);
    SendAccountDataTimes(PER_CHARACTER_CACHE_MASK);

    bool battlePayEnabled = false;
    bool sessionTimeAlert = false;
    bool europaStatus = true;

    data.Initialize(SMSG_FEATURE_SYSTEM_STATUS, 47);

    data << uint32(0);                  // ScrollOfResurrectionMaxRequestsPerDay
    data << uint32(realmID);            // CfgRealmID
    data << uint8(2);                   // ComplaintStatus
    data << uint32(0);                  // CfgRealmRecID
    data << uint32(0);                  // ScrollOfResurrectionRequestsRemaining

    data.WriteBit(0);                   // BpayStoreAvailable
    data.WriteBit(0);                   // CharUndeleteEnabled
    data.WriteBit(battlePayEnabled);    // BpayStoreEnabled
    data.WriteBit(sessionTimeAlert);    // SessionAlert
    data.WriteBit(0);                   // ScrollOfResurrectionEnabled
    data.WriteBit(europaStatus);        // EuropaTicketSystemStatus
    data.WriteBit(0);                   // VoiceEnabled
    data.WriteBit(0);                   // ItemRestorationButtonEnabled
    data.WriteBit(0);                   // BrowserEnabled
    data.FlushBits();

    if (sessionTimeAlert)
    {
        data << uint32(0);              // Delay
        data << uint32(0);              // Period
        data << uint32(0);              // DisplayTime
    }

    if (europaStatus)
    {
        data << uint32(60000);          // PerMilliseconds
        data << uint32(10);             // MaxTries
        data << uint32(0);              // LastResetTimeBeforeNow
        data << uint32(0);              // TryCount
    }

    SendPacket(&data);

    // Send MOTD
    {
        data.Initialize(SMSG_MOTD, 50);                     // new in 2.0.1

        ByteBuffer byteBuffer;

        data.WriteBits(sWorld->GetMotdLineCount(), 4);

        std::string str_motd = sWorld->GetMotd();
        std::string::size_type pos, nextpos;

        pos = 0;
        while ((nextpos = str_motd.find('@', pos)) != std::string::npos)
        {
            if (nextpos != pos)
            {
                byteBuffer.WriteString(str_motd.substr(pos, nextpos - pos));
                data.WriteBits(str_motd.substr(pos, nextpos-pos).size(), 7);
            }
            pos = nextpos + 1;
        }

        if (pos < str_motd.length())
        {
            byteBuffer.WriteString(str_motd.substr(pos));
            data.WriteBits(str_motd.substr(pos).size(), 7);
        }

        if (!byteBuffer.empty())
        {
            data.FlushBits();
            data.append(byteBuffer);
        }

        SendPacket(&data);
        TC_LOG_DEBUG("network", "WORLD: Sent motd (SMSG_MOTD)");

        // send server info
        if (sWorld->getIntConfig(CONFIG_ENABLE_SINFO_LOGIN) == 1)
            chH.PSendSysMessage(_FULLVERSION);

        TC_LOG_DEBUG("network", "WORLD: Sent server info");
    }

    static const std::string timeZoneName = "Europe/Paris";

    data.Initialize(SMSG_TIME_ZONE_INFORMATION, 26);
    data.WriteBits(timeZoneName.size(), 7);
    data.WriteBits(timeZoneName.size(), 7);
    data.FlushBits();
    data.append(timeZoneName.c_str(), timeZoneName.size());
    data.append(timeZoneName.c_str(), timeZoneName.size());
    SendPacket(&data);

    if (sWorld->getBoolConfig(CONFIG_ARENA_SEASON_IN_PROGRESS))
    {
        data.Initialize(SMSG_SET_ARENA_SEASON, 8);
        data << uint32(sWorld->getIntConfig(CONFIG_ARENA_SEASON_ID) - 1);
        data << uint32(sWorld->getIntConfig(CONFIG_ARENA_SEASON_ID));
        SendPacket(&data);
    }

    //QueryResult* result = CharacterDatabase.PQuery("SELECT guildid, rank FROM guild_member WHERE guid = '%u'", pCurrChar->GetGUIDLow());
    if (PreparedQueryResult resultGuild = charHolder->GetPreparedResult(CHAR_LOGIN_QUERY_LOAD_GUILD))
    {
        Field* fields = resultGuild->Fetch();
        pCurrChar->SetInGuild(fields[0].GetUInt32());
        pCurrChar->SetRank(fields[1].GetUInt8());
        if (Guild* guild = sGuildMgr->GetGuildById(pCurrChar->GetGuildId()))
            pCurrChar->SetGuildLevel(guild->GetLevel());
    }
    else if (pCurrChar->GetGuildId())                        // clear guild related fields in case wrong data about non existed membership
    {
        pCurrChar->SetInGuild(0);
        pCurrChar->SetRank(0);
        pCurrChar->SetGuildLevel(0);
    }

    data.Initialize(SMSG_LEARNED_DANCE_MOVES, 4+4);
    data << uint64(0);
    SendPacket(&data);

    HotfixData const &hotfixes = sObjectMgr->GetHotfixData();

    data.Initialize(SMSG_HOTFIX_INFO, 3 + hotfixes.size() * 4 * 3);
    data.WriteBits(hotfixes.size(), 20);
    data.FlushBits();
    for (auto const &hotfix : hotfixes)
    {
        data << uint32(hotfix.Entry);
        data << uint32(hotfix.Timestamp);
        data << uint32(hotfix.Type);
    }
    SendPacket(&data);

    // Send item extended costs hotfix
    for (auto const &id : sObjectMgr->GetOverwriteExtendedCosts())
    {
        auto const extendedCost = sItemExtendedCostStore.LookupEntry(id);
        if (!extendedCost)
            continue;

        WorldPacket data(SMSG_DB_REPLY);
        ByteBuffer buff;

        buff << uint32(extendedCost->ID);
        buff << uint32(0); // reqhonorpoints
        buff << uint32(0); // reqarenapoints
        buff << uint32(extendedCost->RequiredArenaSlot);

        for (auto const &req : extendedCost->RequiredItem)
            buff << req;

        for (auto const &req : extendedCost->RequiredItemCount)
            buff << req;

        buff << uint32(extendedCost->RequiredPersonalArenaRating);
        buff << uint32(0); // ItemPurchaseGroup

        for (auto const &req : extendedCost->RequiredCurrency)
            buff << req;

        for (auto const &req : extendedCost->RequiredCurrencyCount)
            buff << req;

        // Unk
        for (uint32 i = 0; i < MAX_ITEM_EXT_COST_CURRENCIES; i++)
            buff << uint32(0);

        data << uint32(buff.size());
        data.append(buff);

        data << uint32(DB2_REPLY_ITEM_EXTENDED_COST);
        data << uint32(sObjectMgr->GetHotfixDate(extendedCost->ID, DB2_REPLY_ITEM_EXTENDED_COST));
        data << uint32(extendedCost->ID);

        SendPacket(&data);

    }

    pCurrChar->SendInitialPacketsBeforeAddToMap();

    //Show cinematic at the first time that player login
    if (!pCurrChar->getCinematic())
    {
        pCurrChar->setCinematic(1);

        if (ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(pCurrChar->getClass()))
        {
            if (cEntry->CinematicSequence)
                pCurrChar->SendCinematicStart(cEntry->CinematicSequence);
            else if (ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(pCurrChar->getRace()))
                pCurrChar->SendCinematicStart(rEntry->CinematicSequence);

            // send new char string if not empty
            if (!sWorld->GetNewCharString().empty())
                chH.PSendSysMessage("%s", sWorld->GetNewCharString().c_str());
        }
    }

    if (Group* group = pCurrChar->GetGroup())
    {
        if (group->isLFGGroup())
        {
            LfgDungeonSet Dungeons;
            Dungeons.insert(sLFGMgr->GetDungeon(group->GetGUID()));
            sLFGMgr->SetSelectedDungeons(pCurrChar->GetGUID(), Dungeons);
            sLFGMgr->SetState(pCurrChar->GetGUID(), sLFGMgr->GetState(group->GetGUID()));
        }
    }

    if (!pCurrChar->CheckInstanceLoginValid() || !pCurrChar->GetMap()->AddPlayerToMap(pCurrChar))
    {
        AreaTriggerStruct const* at = sObjectMgr->GetGoBackTrigger(pCurrChar->GetMapId());
        if (at)
            pCurrChar->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, pCurrChar->GetOrientation());
        else
            pCurrChar->TeleportTo(pCurrChar->m_homebindMapId, pCurrChar->m_homebindX, pCurrChar->m_homebindY, pCurrChar->m_homebindZ, pCurrChar->GetOrientation());
    }

    sObjectAccessor->AddObject(pCurrChar);
    //TC_LOG_DEBUG("misc", "Player %s added to Map.", pCurrChar->GetName().c_str());

    if (pCurrChar->GetGuildId() != 0)
    {
        if (Guild* guild = sGuildMgr->GetGuildById(pCurrChar->GetGuildId()))
            guild->SendLoginInfo(this);
        else
        {
            // remove wrong guild data
            TC_LOG_ERROR("misc", "Player %s (GUID: %u) marked as member of not existing guild (id: %u), removing guild membership for player.",
                         pCurrChar->GetName().c_str(), pCurrChar->GetGUIDLow(), pCurrChar->GetGuildId());
            pCurrChar->SetInGuild(0);
        }
    }

    pCurrChar->SendInitialPacketsAfterAddToMap();

    CharacterDatabase.PExecute("UPDATE characters SET online = 1 WHERE guid = '%u'", pCurrChar->GetGUIDLow());
    LoginDatabase.PExecute("UPDATE account SET online = 1 WHERE id = '%u'", GetAccountId());
    pCurrChar->SetInGameTime(getMSTime());

    // announce group about member online (must be after add to player list to receive announce to self)
    if (Group* group = pCurrChar->GetGroup())
    {
        //pCurrChar->groupInfo.group->SendInit(this); // useless
        group->SendUpdate();
        group->ResetMaxEnchantingLevel();
    }

    // friend status
    sSocialMgr->SendFriendStatus(pCurrChar, FRIEND_ONLINE, pCurrChar->GetGUID(), true);

    // Place character in world (and load zone) before some object loading
    pCurrChar->LoadCorpse();

    // setting Ghost+speed if dead
    if (pCurrChar->m_deathState != ALIVE)
    {
        // not blizz like, we must correctly save and load player instead...
        if (pCurrChar->getRace() == RACE_NIGHTELF)
            pCurrChar->CastSpell(pCurrChar, 20584, true, 0);// auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
        pCurrChar->CastSpell(pCurrChar, 8326, true, 0);     // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

        pCurrChar->SendMovementSetWaterWalking(true);
    }

    pCurrChar->ContinueTaxiFlight();

    // Load pet if any (if player not alive and in taxi flight or another then pet will remember as temporary unsummoned)
    pCurrChar->LoadPet();

    // Set FFA PvP for non GM in non-rest mode
    if (sWorld->IsFFAPvPRealm() && !pCurrChar->isGameMaster() && !pCurrChar->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
        pCurrChar->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);

    if (pCurrChar->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
        pCurrChar->SetContestedPvP();

    // Apply at_login requests
    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_SPELLS))
    {
        pCurrChar->resetSpells();
        SendNotification(LANG_RESET_SPELLS);
    }

    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_TALENTS))
    {
        pCurrChar->ResetTalents(true);
        pCurrChar->SendTalentsInfoData(false);              // original talents send already in to SendInitialPacketsBeforeAddToMap, resend reset state
        SendNotification(LANG_RESET_TALENTS);
    }

    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_FIRST))
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_FIRST);

    // show time before shutdown if shutdown planned.
    if (sWorld->IsShuttingDown())
        sWorld->ShutdownMsg(true, pCurrChar);

    if (sWorld->getBoolConfig(CONFIG_ALL_TAXI_PATHS))
        pCurrChar->SetTaxiCheater(true);

    if (pCurrChar->isGameMaster())
        SendNotification(LANG_GM_ON);

    pCurrChar->SendCUFProfiles();

    // Hackfix Remove Talent spell - Remove Glyph spell
    pCurrChar->learnSpell(111621, false); // Reset Glyph
    pCurrChar->learnSpell(113873, false); // Reset Talent

    if (pCurrChar->GetBattlePetMgr().HasPendingPassiveLearn())
        pCurrChar->learnSpell(SPELL_BATTLE_PET_TRAINING_PASSIVE, false);

    std::string const &ip = GetRemoteAddress();
    TC_LOG_INFO("entities.player.character", "Account: %u (IP: %s) Login Character:[%s] (GUID: %u) Level: %d",
                GetAccountId(), ip.c_str(), pCurrChar->GetName().c_str(), pCurrChar->GetGUIDLow(), pCurrChar->getLevel());

    if (!pCurrChar->IsStandState() && !pCurrChar->HasUnitState(UNIT_STATE_STUNNED))
        pCurrChar->SetStandState(UNIT_STAND_STATE_STAND);

    m_playerLoading = false;

    sScriptMgr->OnPlayerLogin(pCurrChar);
    pCurrChar->FitPlayerInTeam(pCurrChar->GetBattleground() && !pCurrChar->GetBattleground()->isArena() ? true : false);

    delete charHolder;
    delete authHolder;
}

void WorldSession::HandleSetFactionAtWar(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_SET_FACTION_ATWAR");

    uint8 repListID;

    recvData >> repListID;

    GetPlayer()->GetReputationMgr().SetAtWar(repListID, true);
}

void WorldSession::HandleUnSetFactionAtWar(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_UNSET_FACTION_ATWAR");

    uint8 repListID;

    recvData >> repListID;

    GetPlayer()->GetReputationMgr().SetAtWar(repListID, false);
}

//I think this function is never used :/ I dunno, but i guess this opcode not exists
void WorldSession::HandleSetFactionCheat(WorldPacket& /*recvData*/)
{
    TC_LOG_ERROR("network", "WORLD SESSION: HandleSetFactionCheat, not expected call, please report.");
    GetPlayer()->GetReputationMgr().SendStates();
}

void WorldSession::HandleTutorialFlag(WorldPacket& recvData)
{
    uint32 data;
    recvData >> data;

    uint8 index = uint8(data / 32);
    if (index >= MAX_ACCOUNT_TUTORIAL_VALUES)
        return;

    uint32 value = (data % 32);

    uint32 flag = GetTutorialInt(index);
    flag |= (1 << value);
    SetTutorialInt(index, flag);
}

void WorldSession::HandleTutorialClear(WorldPacket& /*recvData*/)
{
    for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
        SetTutorialInt(i, 0xFFFFFFFF);
}

void WorldSession::HandleTutorialReset(WorldPacket& /*recvData*/)
{
    for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
        SetTutorialInt(i, 0x00000000);
}

void WorldSession::HandleSetWatchedFactionOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_SET_WATCHED_FACTION");
    uint32 fact;
    recvData >> fact;
    GetPlayer()->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fact);
}

void WorldSession::HandleSetFactionInactiveOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_SET_FACTION_INACTIVE");
    uint32 replistid;
    bool inactive;

    recvData >> replistid;
    inactive = recvData.ReadBit();

    _player->GetReputationMgr().SetInactive(replistid, inactive);
}

void WorldSession::HandleShowAccountAchievement(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_SHOW_ACCOUNT_ACHIEVEMENT for %s", _player->GetName().c_str());
    recvData.ReadBit();
}

void WorldSession::HandleShowingHelmOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_SHOWING_HELM for %s", _player->GetName().c_str());
    recvData.read_skip<uint8>(); // unknown, bool?
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
}

void WorldSession::HandleShowingCloakOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_SHOWING_CLOAK for %s", _player->GetName().c_str());
    recvData.read_skip<uint8>(); // unknown, bool?
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
}

void WorldSession::HandleCharRenameOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 nameLen = 0;
    std::string newName;

    recvData.ReadBitSeq<3, 5, 6>(guid);
    nameLen = recvData.ReadBits(6);
    recvData.ReadBitSeq<4, 2, 0, 7, 1>(guid);

    recvData.ReadByteSeq<6, 7, 5, 1, 4, 0, 2>(guid);
    newName = recvData.ReadString(nameLen);
    recvData.ReadByteSeq<3>(guid);

    // prevent character rename to invalid name
    if (!normalizePlayerName(newName))
    {
        WorldPacket data(SMSG_CHAR_RENAME, 1);
        data << uint8(CHAR_NAME_NO_NAME);
        SendPacket(&data);
        return;
    }

    uint8 res = ObjectMgr::CheckPlayerName(newName, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        WorldPacket data(SMSG_CHAR_RENAME, 1+8+(newName.size()+1));
        data << uint8(res);
        data << uint64(guid);
        data << newName;
        SendPacket(&data);
        return;
    }

    // check name limitations
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sObjectMgr->IsReservedName(newName))
    {
        WorldPacket data(SMSG_CHAR_RENAME, 1);
        data << uint8(CHAR_NAME_RESERVED);
        SendPacket(&data);
        return;
    }

    ignoreNextCharEnumCheck = true;

    // Ensure that the character belongs to the current account, that rename at login is enabled
    // and that there is no character with the desired new name
    _charRenameCallback.SetParam(newName);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_FREE_NAME);

    stmt->setUInt32(0, GUID_LOPART(guid));
    stmt->setUInt32(1, GetAccountId());
    stmt->setUInt16(2, AT_LOGIN_RENAME);
    stmt->setUInt16(3, AT_LOGIN_RENAME);
    stmt->setString(4, newName);

    _charRenameCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleChangePlayerNameOpcodeCallBack(PreparedQueryResult result, std::string newName)
{
    if (!result)
    {
        WorldPacket data(SMSG_CHAR_RENAME, 1);
        data << uint8(CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    Field* fields = result->Fetch();

    uint32 guidLow      = fields[0].GetUInt32();
    std::string oldName = fields[1].GetString();

    uint64 guid = MAKE_NEW_GUID(guidLow, 0, HIGHGUID_PLAYER);

    // Update name and at_login flag in the db
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_NAME);

    stmt->setString(0, newName);
    stmt->setUInt16(1, AT_LOGIN_RENAME);
    stmt->setUInt32(2, guidLow);

    CharacterDatabase.Execute(stmt);

    // Removed declined name from db
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
    stmt->setUInt32(0, guidLow);
    CharacterDatabase.Execute(stmt);

    TC_LOG_INFO("entities.player.character", "Account: %d (IP: %s) Character:[%s] (guid:%u) Changed name to: %s", GetAccountId(), GetRemoteAddress().c_str(), oldName.c_str(), guidLow, newName.c_str());

    WorldPacket data(SMSG_CHAR_RENAME, 1+8+(newName.size()+1));
    data << uint8(RESPONSE_SUCCESS);
    data << uint64(guid);
    data << newName;
    SendPacket(&data);

    sWorld->InvalidatePlayerData(guid);
}

void WorldSession::HandleSetPlayerDeclinedNames(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    // not accept declined names for unsupported languages
    std::string name;
    if (!sObjectMgr->GetPlayerNameByGUID(guid, name))
    {
        SendPlayerDeclinedNamesResult(guid, 1);
        return;
    }

    std::wstring wname;
    if (!Utf8toWStr(name, wname))
    {
        SendPlayerDeclinedNamesResult(guid, 1);
        return;
    }

    if (!isCyrillicCharacter(wname[0]))                      // name already stored as only single alphabet using
    {
        SendPlayerDeclinedNamesResult(guid, 1);
        return;
    }

    std::string name2;
    DeclinedName declinedname;

    recvData >> name2;

    if (name2 != name)                                       // character have different name
    {
        SendPlayerDeclinedNamesResult(guid, 1);
        return;
    }

    for (int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
    {
        recvData >> declinedname.name[i];
        if (!normalizePlayerName(declinedname.name[i]))
        {
            SendPlayerDeclinedNamesResult(guid, 1);
            return;
        }
    }

    if (!ObjectMgr::CheckDeclinedNames(wname, declinedname))
    {
        SendPlayerDeclinedNamesResult(guid, 1);
        return;
    }

    for (int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        CharacterDatabase.EscapeString(declinedname.name[i]);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
    stmt->setUInt32(0, GUID_LOPART(guid));
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_DECLINED_NAME);
    stmt->setUInt32(0, GUID_LOPART(guid));

    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; i++)
        stmt->setString(i+1, declinedname.name[i]);

    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    SendPlayerDeclinedNamesResult(guid, 0);
}

void WorldSession::SendPlayerDeclinedNamesResult(uint64 guid, uint32 result)
{
    WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
    data << uint32(result);
    data << uint64(guid);
    SendPacket(&data);
}

void WorldSession::HandleAlterAppearance(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_ALTER_APPEARANCE");

    uint32 Hair, Color, FacialHair, SkinColor;
    recvData >> Color >> FacialHair >> SkinColor >> Hair;

    BarberShopStyleEntry const* bs_hair = sBarberShopStyleStore.LookupEntry(Hair);
    if (!bs_hair || bs_hair->type != 0 || bs_hair->race != _player->getRace() || bs_hair->gender != _player->getGender())
        return;

    BarberShopStyleEntry const* bs_facialHair = sBarberShopStyleStore.LookupEntry(FacialHair);
    if (!bs_facialHair || bs_facialHair->type != 2 || bs_facialHair->race != _player->getRace() || bs_facialHair->gender != _player->getGender())
        return;

    BarberShopStyleEntry const* bs_skinColor = sBarberShopStyleStore.LookupEntry(SkinColor);
    if (bs_skinColor && (bs_skinColor->type != 3 || bs_skinColor->race != _player->getRace() || bs_skinColor->gender != _player->getGender()))
        return;

    GameObject* go = _player->FindNearestGameObjectOfType(GAMEOBJECT_TYPE_BARBER_CHAIR, 5.0f);
    if (!go)
    {
        WorldPacket data(SMSG_BARBER_SHOP_RESULT, 4);
        data << uint32(2);
        SendPacket(&data);
        return;
    }

    if (_player->getStandState() != UNIT_STAND_STATE_SIT_LOW_CHAIR + go->GetGOInfo()->barberChair.chairheight)
    {
        WorldPacket data(SMSG_BARBER_SHOP_RESULT, 4);
        data << uint32(2);
        SendPacket(&data);
        return;
    }

    uint32 cost = _player->GetBarberShopCost(bs_hair->hair_id, Color, bs_facialHair->hair_id, bs_skinColor);

    // 0 - ok
    // 1, 3 - not enough money
    // 2 - you have to sit on barber chair
    if (!_player->HasEnoughMoney((uint64)cost))
    {
        WorldPacket data(SMSG_BARBER_SHOP_RESULT, 4);
        data << uint32(1);                                  // no money
        SendPacket(&data);
        return;
    }
    else
    {
        WorldPacket data(SMSG_BARBER_SHOP_RESULT, 4);
        data << uint32(0);                                  // ok
        SendPacket(&data);
    }

    _player->ModifyMoney(-int64(cost));                     // it isn't free
    _player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER, cost);

    _player->SetByteValue(PLAYER_BYTES, 2, uint8(bs_hair->hair_id));
    _player->SetByteValue(PLAYER_BYTES, 3, uint8(Color));
    _player->SetByteValue(PLAYER_BYTES_2, 0, uint8(bs_facialHair->hair_id));
    if (bs_skinColor)
        _player->SetByteValue(PLAYER_BYTES, 0, uint8(bs_skinColor->hair_id));

    _player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP, 1);

    _player->SetStandState(0);                              // stand up
}

void WorldSession::HandleRemoveGlyph(WorldPacket& recvData)
{
    uint32 slot;
    recvData >> slot;

    if (slot >= MAX_GLYPH_SLOT_INDEX)
    {
        TC_LOG_DEBUG("network", "Client sent wrong glyph slot number in opcode CMSG_REMOVE_GLYPH %u", slot);
        return;
    }

    if (uint32 glyph = _player->GetGlyph(_player->GetActiveSpec(), slot))
    {
        if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyph))
        {
            _player->RemoveAurasDueToSpell(gp->SpellId);
            _player->SetGlyph(slot, 0);
            _player->SendTalentsInfoData(false);
        }
    }
}

void WorldSession::HandleCharCustomize(WorldPacket& recvData)
{
    ObjectGuid playerGuid;
    uint8 gender, skin, face, hairStyle, hairColor, facialHair;
    std::string newName;
    uint32 nameLen;

    recvData >> gender >> hairColor >> facialHair >> skin >> face >> hairStyle;

    recvData.ReadBitSeq<0, 3, 4, 5, 6>(playerGuid);
    nameLen = recvData.ReadBits(6);
    recvData.ReadBitSeq<2, 7, 1>(playerGuid);
    newName = recvData.ReadString(nameLen);

    recvData.ReadByteSeq<6, 3, 1, 4, 7, 2, 5, 0>(playerGuid);

    if (!IsLegitCharacterForAccount(GUID_LOPART(playerGuid)))
    {
        TC_LOG_ERROR("network", "Account %u, IP: %s tried to customise character %u, but it does not belong to their account!",
                     GetAccountId(), GetRemoteAddress().c_str(), GUID_LOPART(playerGuid));
        KickPlayer();
        return;
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_AT_LOGIN);
    stmt->setUInt32(0, GUID_LOPART(playerGuid));
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1 + 8 + 1);
        data.WriteBitSeq<0, 5, 2, 4, 6, 7, 3, 1>(playerGuid);
        data.WriteByteSeq<7, 1, 0, 5, 2>(playerGuid);
        data << uint8(CHAR_CREATE_ERROR);
        data.WriteByteSeq<6, 4, 3>(playerGuid);

        SendPacket(&data);
        return;
    }

    Field* fields = result->Fetch();
    uint32 at_loginFlags = fields[0].GetUInt16();

    if (!(at_loginFlags & AT_LOGIN_CUSTOMIZE))
    {
        WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1 + 8 + 1);
        data.WriteBitSeq<0, 5, 2, 4, 6, 7, 3, 1>(playerGuid);
        data.WriteByteSeq<7, 1, 0, 5, 2>(playerGuid);
        data << uint8(CHAR_CREATE_ERROR);
        data.WriteByteSeq<6, 4, 3>(playerGuid);

        SendPacket(&data);
        return;
    }

    // prevent character rename to invalid name
    if (!normalizePlayerName(newName))
    {
        WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1 + 8 + 1);
        data.WriteBitSeq<0, 5, 2, 4, 6, 7, 3, 1>(playerGuid);
        data.WriteByteSeq<7, 1, 0, 5, 2>(playerGuid);
        data << uint8(CHAR_NAME_NO_NAME);
        data.WriteByteSeq<6, 4, 3>(playerGuid);

        SendPacket(&data);
        return;
    }

    uint8 res = ObjectMgr::CheckPlayerName(newName, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1 + 8 + 1);
        data.WriteBitSeq<0, 5, 2, 4, 6, 7, 3, 1>(playerGuid);
        data.WriteByteSeq<7, 1, 0, 5, 2>(playerGuid);
        data << uint8(res);
        data.WriteByteSeq<6, 4, 3>(playerGuid);

        SendPacket(&data);
        return;
    }

    // check name limitations
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sObjectMgr->IsReservedName(newName))
    {
        WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1 + 8 + 1);
        data.WriteBitSeq<0, 5, 2, 4, 6, 7, 3, 1>(playerGuid);
        data.WriteByteSeq<7, 1, 0, 5, 2>(playerGuid);
        data << uint8(CHAR_NAME_RESERVED);
        data.WriteByteSeq<6, 4, 3>(playerGuid);

        SendPacket(&data);
        return;
    }

    // character with this name already exist
    if (uint64 newguid = sObjectMgr->GetPlayerGUIDByName(newName))
    {
        if (newguid != playerGuid)
        {
            WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1 + 8 + 1);
            data.WriteBitSeq<0, 5, 2, 4, 6, 7, 3, 1>(playerGuid);
            data.WriteByteSeq<7, 1, 0, 5, 2>(playerGuid);
            data << uint8(CHAR_CREATE_NAME_IN_USE);
            data.WriteByteSeq<6, 4, 3>(playerGuid);

            SendPacket(&data);
            return;
        }
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_NAME);
    stmt->setUInt32(0, GUID_LOPART(playerGuid));
    result = CharacterDatabase.Query(stmt);

    if (result)
    {
        std::string oldname = result->Fetch()[0].GetString();
        TC_LOG_INFO("entities.player.character", "Account: %d (IP: %s), Character[%s] (guid:%u) Customized to: %s",
                    GetAccountId(), GetRemoteAddress().c_str(), oldname.c_str(), GUID_LOPART(playerGuid), newName.c_str());
    }

    Player::Customize(playerGuid, gender, skin, face, hairStyle, hairColor, facialHair);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_NAME_AT_LOGIN);
    stmt->setString(0, newName);
    stmt->setUInt16(1, uint16(AT_LOGIN_CUSTOMIZE));
    stmt->setUInt32(2, GUID_LOPART(playerGuid));
    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
    stmt->setUInt32(0, GUID_LOPART(playerGuid));
    CharacterDatabase.Execute(stmt);

    WorldPacket data(SMSG_CHAR_CUSTOMIZE, 17 + newName.size());

    data.WriteBitSeq<0, 5, 2, 4, 6, 7, 3, 1>(playerGuid);
    data.WriteByteSeq<7, 1, 0, 5, 2>(playerGuid);
    data << uint8(RESPONSE_SUCCESS);
    data.WriteByteSeq<6, 4>(playerGuid);
    data << uint8(skin);
    data << uint8(hairColor);
    data << uint8(facialHair);
    data << uint8(face);
    data << uint8(hairStyle);
    data << uint8(gender);
    data.WriteByteSeq<3>(playerGuid);
    data.WriteBits(newName.size(), 6);
    data.FlushBits();
    data.append(newName.c_str(), newName.size());

    SendPacket(&data);

    sWorld->InvalidatePlayerData(playerGuid);
}

void WorldSession::HandleEquipmentSetSave(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_EQUIPMENT_SET_SAVE");

    ObjectGuid setGuid;
    uint32 index;
    EquipmentSet eqSet;
    uint8 iconNameLen, setNameLen;

    recvData >> index;

    if (index >= MAX_EQUIPMENT_SET_INDEX)                    // client set slots amount
        return;

    recvData.ReadBitSeq<7>(setGuid);

    ObjectGuid itemGuid[EQUIPMENT_SLOT_END];

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
        recvData.ReadBitSeq<4, 6, 0, 1, 3, 5, 2, 7>(itemGuid[i]);

    recvData.ReadBitSeq<4, 1, 5, 0, 3, 2>(setGuid);

    iconNameLen = 2 * recvData.ReadBits(8);
    bool pair = recvData.ReadBit();

    if (pair)
        iconNameLen++;

    setNameLen = recvData.ReadBits(8);
    recvData.ReadBitSeq<6>(setGuid);

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        recvData.ReadByteSeq<6, 1, 4, 0, 3, 5, 7, 2>(itemGuid[i]);

        // equipment manager sends "1" (as raw GUID) for slots set to "ignore" (don't touch slot at equip set)
        if (itemGuid[i] == 1)
        {
            // ignored slots saved as bit mask because we have no free special values for Items[i]
            eqSet.IgnoreMask |= 1 << i;
            continue;
        }

        Item* item = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (!item && itemGuid[i])                               // cheating check 1
            return;

        if (item && item->GetGUID() != itemGuid[i])             // cheating check 2
            return;

        eqSet.Items[i] = GUID_LOPART(itemGuid[i]);
    }

    recvData.ReadByteSeq<4, 3>(setGuid);

    std::string name, iconName;

    iconName = recvData.ReadString(iconNameLen);
    name = recvData.ReadString(setNameLen);

    if (name.empty() || iconName.empty())
        return;

    recvData.ReadByteSeq<5, 0, 1, 7, 6, 2>(setGuid);

    eqSet.Guid      = setGuid;
    eqSet.Name      = name;
    eqSet.IconName  = iconName;
    eqSet.state     = EQUIPMENT_SET_NEW;

    _player->SetEquipmentSet(index, eqSet);
}

void WorldSession::HandleEquipmentSetDelete(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_EQUIPMENT_SET_DELETE");

    ObjectGuid setGuid;

    recvData.ReadBitSeq<4, 1, 7, 0, 5, 6, 3, 2>(setGuid);
    recvData.ReadByteSeq<5, 1, 3, 4, 2, 0, 7, 6>(setGuid);

    _player->DeleteEquipmentSet(setGuid);
}

void WorldSession::HandleEquipmentSetUse(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_EQUIPMENT_SET_USE");

    uint8 srcbag[EQUIPMENT_SLOT_END];;
    uint8 srcslot[EQUIPMENT_SLOT_END];

    ObjectGuid itemGuid[EQUIPMENT_SLOT_END];

    EquipmentSlots startSlot = _player->IsInCombat() ? EQUIPMENT_SLOT_MAINHAND : EQUIPMENT_SLOT_START;

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
        recvData >> srcslot[i] >> srcbag[i];

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
        recvData.ReadBitSeq<2, 5, 0, 1, 3, 6, 4, 7>(itemGuid[i]);

    uint8 unkCounter = recvData.ReadBits(2);

    for (uint8 i = 0; i < unkCounter; i++)
    {
        recvData.ReadBit();
        recvData.ReadBit();
    }

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (i == 17)
            continue;

        recvData.ReadByteSeq<4, 1, 6, 5, 3, 2, 0, 7>(itemGuid[i]);

        if (i < uint32(startSlot))
            continue;

        // check if item slot is set to "ignored" (raw value == 1), must not be unequipped then
        if (itemGuid[i] == 1)
            continue;

        Item* item = _player->GetItemByGuid(itemGuid[i]);

        uint16 dstpos = i | (INVENTORY_SLOT_BAG_0 << 8);

        if (!item)
        {
            Item* uItem = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!uItem)
                continue;

            ItemPosCountVec sDest;
            InventoryResult msg = _player->CanStoreItem(NULL_BAG, NULL_SLOT, sDest, uItem, false);
            if (msg == EQUIP_ERR_OK)
            {
                _player->RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
                _player->StoreItem(sDest, uItem, true);
            }
            else
                _player->SendEquipError(msg, uItem, NULL);

            continue;
        }

        if (item->GetPos() == dstpos)
            continue;

        _player->SwapItem(item->GetPos(), dstpos);
    }

    for (uint8 i = 0; i < unkCounter; i++)
    {
        recvData.read_skip<uint8>();
        recvData.read_skip<uint8>();
    }

    WorldPacket data(SMSG_DUMP_OBJECTS_DATA);
    data << uint8(0);   // 4 - equipment swap failed - inventory is full
    SendPacket(&data);
}

void WorldSession::HandleCharFactionOrRaceChange(WorldPacket& recvData)
{
    ObjectGuid guid;
    std::string newname;
    bool hasSkin, hasFace, hasHairStyle, hasHairColor, hasFacialHair;
    uint8 gender, race;
    uint8 skin = 0;
    uint8 face = 0;
    uint8 hairStyle = 0;
    uint8 hairColor = 0;
    uint8 facialHair = 0;
    uint32 nameLen = 0;

    recvData >> gender;
    recvData >> race;
    recvData.ReadBit();
    hasFace = recvData.ReadBit();
    recvData.ReadBitSeq<7, 3>(guid);
    nameLen = recvData.ReadBits(6);
    hasHairStyle = recvData.ReadBit();
    recvData.ReadBitSeq<0>(guid);
    hasHairColor = recvData.ReadBit();
    recvData.ReadBitSeq<2, 5>(guid);
    hasFacialHair = recvData.ReadBit();
    recvData.ReadBitSeq<4, 1>(guid);
    hasSkin = recvData.ReadBit();
    recvData.ReadBitSeq<6>(guid);

    recvData.ReadByteSeq<0, 3, 7>(guid);
    newname = recvData.ReadString(nameLen);
    recvData.ReadByteSeq<5, 4, 6, 2, 1>(guid);

    if (hasHairStyle)
        recvData >> hairStyle;
    if (hasHairColor)
        recvData >> hairColor;
    if (hasFace)
        recvData >> face;
    if (hasSkin)
        recvData >> skin;
    if (hasFacialHair)
        recvData >> facialHair;

    uint32 const lowGuid = GUID_LOPART(guid);

    if (!IsLegitCharacterForAccount(lowGuid))
    {
        TC_LOG_ERROR("network", "Account %u, IP: %s tried to factionchange character %u, but it does not belong to their account!",
                     GetAccountId(), GetRemoteAddress().c_str(), lowGuid);
        KickPlayer();
        return;
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_FACTIONCHANGE_DATA);
    stmt->setUInt32(0, lowGuid);

    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
    {
        WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    Field const * const fields = result->Fetch();

    auto const oldRace       = fields[0].GetUInt8();
    auto const playerClass   = fields[1].GetUInt8();
    auto const level         = fields[2].GetUInt8();
    auto const at_loginFlags = fields[3].GetUInt16();

    if (!(at_loginFlags & (AT_LOGIN_CHANGE_FACTION | AT_LOGIN_CHANGE_RACE)))
    {
        WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    auto const bgTeamForRace = [](uint8 race) {
        switch (race) {
            case RACE_ORC:
            case RACE_GOBLIN:
            case RACE_TAUREN:
            case RACE_UNDEAD_PLAYER:
            case RACE_TROLL:
            case RACE_BLOODELF:
            case RACE_PANDAREN_HORDE:
                return BG_TEAM_HORDE;
            default:
                return BG_TEAM_ALLIANCE;
        }
    };

    BattlegroundTeamId const oldTeam = bgTeamForRace(oldRace);
    BattlegroundTeamId team;

    if (race != RACE_PANDAREN_NEUTRAL)
    {
        team = bgTeamForRace(race);
    }
    else
    {
        // We have to correct the race when it's Pandaren, as the client always
        // sends the neutral ID.
        if (at_loginFlags & AT_LOGIN_CHANGE_FACTION)
        {
            team = oldTeam == BG_TEAM_ALLIANCE ? BG_TEAM_HORDE : BG_TEAM_ALLIANCE;
            race = oldTeam == BG_TEAM_ALLIANCE ? RACE_PANDAREN_HORDE : RACE_PANDAREN_ALLI;
        }
        else
        {
            team = oldTeam;
            race = oldTeam == BG_TEAM_ALLIANCE ? RACE_PANDAREN_ALLI : RACE_PANDAREN_HORDE;
        }
    }

    if (!sObjectMgr->GetPlayerInfo(race, playerClass))
    {
        WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    uint32 raceMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK);
    if ((1 << (race - 1)) & raceMaskDisabled)
    {
        WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    // prevent character rename to invalid name
    if (!normalizePlayerName(newname))
    {
        WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
        data << uint8(CHAR_NAME_NO_NAME);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    uint8 res = ObjectMgr::CheckPlayerName(newname, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
        data << uint8(res);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    // check name limitations
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sObjectMgr->IsReservedName(newname))
    {
        WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
        data << uint8(CHAR_NAME_RESERVED);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    // A character with this name already exists.
    if (uint64 newguid = sObjectMgr->GetPlayerGUIDByName(newname))
    {
        if (newguid != guid)
        {
            WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
            data << uint8(CHAR_CREATE_NAME_IN_USE);
            data << uint64(guid);
            SendPacket(&data);
            return;
        }
    }

    CharacterDatabase.EscapeString(newname);
    Player::Customize(guid, gender, skin, face, hairStyle, hairColor, facialHair);
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    // As the client sends the same packet for both faction and race changes now,
    // we have to guess the player's intentions. Although it's easy when he
    // actually changes something, it's also possible to simply rename the
    // character. If it happens, we remove both flags to keep things simple.
    uint16 const used_loginFlag = (oldRace != race)
            ? ((oldTeam != team) ? AT_LOGIN_CHANGE_FACTION : AT_LOGIN_CHANGE_RACE)
            : (AT_LOGIN_CHANGE_FACTION | AT_LOGIN_CHANGE_RACE);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_FACTION_OR_RACE);
    stmt->setString(0, newname);
    stmt->setUInt8 (1, race);
    stmt->setUInt16(2, used_loginFlag);
    stmt->setUInt32(3, lowGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
    stmt->setUInt32(0, lowGuid);
    trans->Append(stmt);

    if (oldRace != race)
    {
        // Switch Languages
        // delete all languages first
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SKILL_LANGUAGES);
        stmt->setUInt32(0, lowGuid);
        trans->Append(stmt);

        // Now add them back
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SKILL_LANGUAGE);
        stmt->setUInt32(0, lowGuid);

        // Faction specific languages
        stmt->setUInt16(1, team == BG_TEAM_HORDE ? SKILL_LANG_ORCISH : SKILL_LANG_COMMON);

        trans->Append(stmt);

        // Race specific languages
        if (race != RACE_ORC && race != RACE_HUMAN)
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SKILL_LANGUAGE);
            stmt->setUInt32(0, lowGuid);

            switch (race)
            {
                case RACE_DWARF:
                    stmt->setUInt16(1, SKILL_LANG_DWARVEN);
                    break;
                case RACE_DRAENEI:
                    stmt->setUInt16(1, SKILL_LANG_DRAENEI);
                    break;
                case RACE_GNOME:
                    stmt->setUInt16(1, SKILL_LANG_GNOMISH);
                    break;
                case RACE_NIGHTELF:
                    stmt->setUInt16(1, SKILL_LANG_DARNASSIAN);
                    break;
                case RACE_WORGEN:
                    stmt->setUInt16(1, SKILL_LANG_WORGEN);
                    break;
                case RACE_UNDEAD_PLAYER:
                    stmt->setUInt16(1, SKILL_LANG_GUTTERSPEAK);
                    break;
                case RACE_TAUREN:
                    stmt->setUInt16(1, SKILL_LANG_TAURAHE);
                    break;
                case RACE_TROLL:
                    stmt->setUInt16(1, SKILL_LANG_TROLL);
                    break;
                case RACE_BLOODELF:
                    stmt->setUInt16(1, SKILL_LANG_THALASSIAN);
                    break;
                case RACE_GOBLIN:
                    stmt->setUInt16(1, SKILL_LANG_GOBLIN);
                    break;
                case RACE_PANDAREN_ALLI:
                    stmt->setUInt16(1, SKILL_LANG_PANDAREN_A);
                    break;
                case RACE_PANDAREN_HORDE:
                    stmt->setUInt16(1, SKILL_LANG_PANDAREN_H);
                    break;
                case RACE_PANDAREN_NEUTRAL:
                    stmt->setUInt16(1, SKILL_LANG_PANDAREN_N);
                    break;
            }

            trans->Append(stmt);
        }

        // Race traits that are not available at char creation
        {
            auto range = sObjectMgr->FactionChangeRewardedRacials.equal_range(oldRace);

            for (; range.first != range.second; ++range.first)
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SPELL_BY_SPELL);
                stmt->setUInt32(0, lowGuid);
                stmt->setUInt32(1, range.first->second);

                trans->Append(stmt);
            }

            range = sObjectMgr->FactionChangeRewardedRacials.equal_range(race);

            for (; range.first != range.second; ++range.first)
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SPELL);
                stmt->setUInt32(0, lowGuid);
                stmt->setUInt32(1, range.first->second);
                stmt->setUInt8(2, 1);
                stmt->setUInt8(3, 0);

                trans->Append(stmt);
            }
        }

        if (oldTeam != team)
        {
            // Delete all Flypaths
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_TAXI_PATH);
            stmt->setUInt32(0, lowGuid);
            trans->Append(stmt);

            if (level > 7)
            {
                // Update Taxi path
                // this doesn't seem to be 100% blizzlike... but it can't really be helped.
                std::ostringstream taximaskstream;
                uint32 numFullTaximasks = level / 7;
                if (numFullTaximasks > 11)
                    numFullTaximasks = 11;
                if (team == BG_TEAM_ALLIANCE)
                {
                    if (playerClass != CLASS_DEATH_KNIGHT)
                    {
                        for (uint8 i = 0; i < numFullTaximasks; ++i)
                            taximaskstream << uint32(sAllianceTaxiNodesMask[i]) << ' ';
                    }
                    else
                    {
                        for (uint8 i = 0; i < numFullTaximasks; ++i)
                            taximaskstream << uint32(sAllianceTaxiNodesMask[i] | sDeathKnightTaxiNodesMask[i]) << ' ';
                    }
                }
                else
                {
                    if (playerClass != CLASS_DEATH_KNIGHT)
                    {
                        for (uint8 i = 0; i < numFullTaximasks; ++i)
                            taximaskstream << uint32(sHordeTaxiNodesMask[i]) << ' ';
                    }
                    else
                    {
                        for (uint8 i = 0; i < numFullTaximasks; ++i)
                            taximaskstream << uint32(sHordeTaxiNodesMask[i] | sDeathKnightTaxiNodesMask[i]) << ' ';
                    }
                }

                uint32 numEmptyTaximasks = 11 - numFullTaximasks;
                for (uint8 i = 0; i < numEmptyTaximasks; ++i)
                    taximaskstream << "0 ";
                taximaskstream << '0';
                std::string taximask = taximaskstream.str();

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_TAXIMASK);
                stmt->setString(0, taximask);
                stmt->setUInt32(1, lowGuid);
                trans->Append(stmt);
            }

            if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD))
            {
                // Reset guild
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_MEMBER);

                stmt->setUInt32(0, lowGuid);

                PreparedQueryResult result = CharacterDatabase.Query(stmt);
                if (result)
                    if (Guild* guild = sGuildMgr->GetGuildById((result->Fetch()[0]).GetUInt32()))
                        guild->DeleteMember(MAKE_NEW_GUID(lowGuid, 0, HIGHGUID_PLAYER), false, false, true);
            }

            if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND))
            {
                // Delete Friend List
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SOCIAL_BY_GUID);
                stmt->setUInt32(0, lowGuid);
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SOCIAL_BY_FRIEND);
                stmt->setUInt32(0, lowGuid);
                trans->Append(stmt);
            }

            // Reset homebind and position
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PLAYER_HOMEBIND);
            stmt->setUInt32(0, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PLAYER_HOMEBIND);
            stmt->setUInt32(0, lowGuid);
            if (team == BG_TEAM_ALLIANCE)
            {
                stmt->setUInt16(1, 0);
                stmt->setUInt16(2, 1519);
                stmt->setFloat (3, -8867.68f);
                stmt->setFloat (4, 673.373f);
                stmt->setFloat (5, 97.9034f);
                Player::SavePositionInDB(0, -8867.68f, 673.373f, 97.9034f, 0.0f, 1519, lowGuid);
            }
            else
            {
                stmt->setUInt16(1, 1);
                stmt->setUInt16(2, 1637);
                stmt->setFloat (3, 1633.33f);
                stmt->setFloat (4, -4439.11f);
                stmt->setFloat (5, 15.7588f);
                Player::SavePositionInDB(1, 1633.33f, -4439.11f, 15.7588f, 0.0f, 1637, lowGuid);
            }

            trans->Append(stmt);

            // Achievement conversion
            for (auto it = sObjectMgr->FactionChangeAchievements.cbegin(); it != sObjectMgr->FactionChangeAchievements.cend(); ++it)
            {
                uint32 achiev_alliance = it->first;
                uint32 achiev_horde = it->second;

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_ACHIEVEMENT_BY_ACHIEVEMENT);
                stmt->setUInt32(0, lowGuid);
                stmt->setUInt16(1, uint16(team == BG_TEAM_ALLIANCE ? achiev_alliance : achiev_horde));
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_ACHIEVEMENT);
                stmt->setUInt16(0, uint16(team == BG_TEAM_ALLIANCE ? achiev_alliance : achiev_horde));
                stmt->setUInt32(1, lowGuid);
                stmt->setUInt16(2, uint16(team == BG_TEAM_ALLIANCE ? achiev_horde : achiev_alliance));
                trans->Append(stmt);
            }

            // Item conversion
            for (auto it = sObjectMgr->FactionChangeItems.cbegin(); it != sObjectMgr->FactionChangeItems.cend(); ++it)
            {
                uint32 item_alliance = it->first;
                uint32 item_horde = it->second;

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_INVENTORY_FACTION_CHANGE);
                stmt->setUInt32(0, (team == BG_TEAM_ALLIANCE ? item_alliance : item_horde));
                stmt->setUInt32(1, guid);
                stmt->setUInt32(2, (team == BG_TEAM_ALLIANCE ? item_horde : item_alliance));
                trans->Append(stmt);
            }

            // Delete all current quests
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_QUESTSTATUS);
            stmt->setUInt32(0, lowGuid);
            trans->Append(stmt);

            // Quest conversion
            for (auto it = sObjectMgr->FactionChangeQuests.cbegin(); it != sObjectMgr->FactionChangeQuests.cend(); ++it)
            {
                uint32 quest_alliance = it->first;
                uint32 quest_horde = it->second;

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_QUESTSTATUS_REWARDED_BY_QUEST);
                stmt->setUInt32(0, lowGuid);
                stmt->setUInt32(1, (team == BG_TEAM_ALLIANCE ? quest_alliance : quest_horde));
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_QUESTSTATUS_REWARDED_FACTION_CHANGE);
                stmt->setUInt32(0, (team == BG_TEAM_ALLIANCE ? quest_alliance : quest_horde));
                stmt->setUInt32(1, lowGuid);
                stmt->setUInt32(2, (team == BG_TEAM_ALLIANCE ? quest_horde : quest_alliance));
                trans->Append(stmt);
            }

            // Mark all rewarded quests as "active" (will count for completed quests achievements)
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_QUESTSTATUS_REWARDED_ACTIVE);
            stmt->setUInt32(0, lowGuid);
            trans->Append(stmt);

            // Disable all old-faction specific quests
            {
                ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
                for (auto iter = questTemplates.cbegin(); iter != questTemplates.cend(); ++iter)
                {
                    Quest const* quest = iter->second;
                    uint32 newRaceMask = (team == BG_TEAM_ALLIANCE) ? RACEMASK_ALLIANCE : RACEMASK_HORDE;
                    if (!(quest->GetRequiredRaces() & newRaceMask))
                    {
                        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_QUESTSTATUS_REWARDED_ACTIVE_BY_QUEST);
                        stmt->setUInt32(0, lowGuid);
                        stmt->setUInt32(1, quest->GetQuestId());
                        trans->Append(stmt);
                    }
                }
            }

            // Spell conversion
            for (auto it = sObjectMgr->FactionChangeSpells.cbegin(); it != sObjectMgr->FactionChangeSpells.cend(); ++it)
            {
                uint32 spell_alliance = it->first;
                uint32 spell_horde = it->second;

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SPELL_BY_SPELL);
                stmt->setUInt32(0, lowGuid);
                stmt->setUInt32(1, (team == BG_TEAM_ALLIANCE ? spell_alliance : spell_horde));
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_SPELL_FACTION_CHANGE);
                stmt->setUInt32(0, (team == BG_TEAM_ALLIANCE ? spell_alliance : spell_horde));
                stmt->setUInt32(1, lowGuid);
                stmt->setUInt32(2, (team == BG_TEAM_ALLIANCE ? spell_horde : spell_alliance));
                trans->Append(stmt);
            }

            // Reputation conversion
            for (auto it = sObjectMgr->FactionChangeReputation.cbegin(); it != sObjectMgr->FactionChangeReputation.cend(); ++it)
            {
                uint32 reputation_alliance = it->first;
                uint32 reputation_horde = it->second;
                uint32 newReputation = (team == BG_TEAM_ALLIANCE) ? reputation_alliance : reputation_horde;
                uint32 oldReputation = (team == BG_TEAM_ALLIANCE) ? reputation_horde : reputation_alliance;

                // select old standing set in db
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_REP_BY_FACTION);
                stmt->setUInt32(0, lowGuid);
                stmt->setUInt32(1, oldReputation);
                PreparedQueryResult result = CharacterDatabase.Query(stmt);

                if (!result)
                {
                    WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1);
                    data << uint8(CHAR_CREATE_ERROR);
                    data << uint64(guid);
                    SendPacket(&data);
                    return;
                }

                Field* fields = result->Fetch();
                int32 oldDBRep = fields[0].GetInt32();
                FactionEntry const* factionEntry = sFactionStore.LookupEntry(oldReputation);

                // old base reputation
                int32 oldBaseRep = sObjectMgr->GetBaseReputationOf(factionEntry, oldRace, playerClass);

                // new base reputation
                int32 newBaseRep = sObjectMgr->GetBaseReputationOf(sFactionStore.LookupEntry(newReputation), race, playerClass);

                // final reputation shouldnt change
                int32 finalRep = oldDBRep + oldBaseRep;
                int32 newDBRep = finalRep - newBaseRep;

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_REP_BY_FACTION);
                stmt->setUInt32(0, lowGuid);
                stmt->setUInt32(1, newReputation);
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_REP_FACTION_CHANGE);
                stmt->setUInt16(0, uint16(newReputation));
                stmt->setUInt32(1, lowGuid);
                stmt->setUInt16(2, uint16(oldReputation));
                trans->Append(stmt);
            }

            // Title conversion
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_KNOWN_TITLES);
            stmt->setUInt32(0, lowGuid);
            result = CharacterDatabase.Query(stmt);

            if (result)
            {
                uint32 knownTitles[KNOWN_TITLES_SIZE * 2] = { 0 };

                do {
                    Field const * const fields = result->Fetch();
                    knownTitles[fields[0].GetUInt8()] = fields[1].GetUInt32();
                } while (result->NextRow());

                for (auto it = sObjectMgr->FactionChangeTitles.cbegin(); it != sObjectMgr->FactionChangeTitles.cend(); ++it)
                {
                    uint32 title_alliance = it->first;
                    uint32 title_horde = it->second;

                    CharTitlesEntry const* atitleInfo = sCharTitlesStore.LookupEntry(title_alliance);
                    CharTitlesEntry const* htitleInfo = sCharTitlesStore.LookupEntry(title_horde);
                    // new team
                    if (team == BG_TEAM_ALLIANCE)
                    {
                        uint32 bitIndex = htitleInfo->bit_index;
                        uint32 index = bitIndex / 32;
                        uint32 old_flag = 1 << (bitIndex % 32);
                        uint32 new_flag = 1 << (atitleInfo->bit_index % 32);
                        if (knownTitles[index] & old_flag)
                        {
                            knownTitles[index] &= ~old_flag;
                            // use index of the new title
                            knownTitles[atitleInfo->bit_index / 32] |= new_flag;
                        }
                    }
                    else
                    {
                        uint32 bitIndex = atitleInfo->bit_index;
                        uint32 index = bitIndex / 32;
                        uint32 old_flag = 1 << (bitIndex % 32);
                        uint32 new_flag = 1 << (htitleInfo->bit_index % 32);
                        if (knownTitles[index] & old_flag)
                        {
                            knownTitles[index] &= ~old_flag;
                            // use index of the new title
                            knownTitles[htitleInfo->bit_index / 32] |= new_flag;
                        }
                    }

                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_KNOWN_TITLES);
                    stmt->setUInt32(0, lowGuid);
                    trans->Append(stmt);

                    for (uint8 i = 0; i < KNOWN_TITLES_SIZE * 2; ++i)
                    {
                        if (knownTitles[i])
                        {
                            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_KNOWN_TITLES);
                            stmt->setUInt32(0, lowGuid);
                            stmt->setUInt8(1, i);
                            stmt->setUInt32(2, knownTitles[i]);
                            trans->Append(stmt);
                        }
                    }

                    // unset any currently chosen title
                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_RES_CHAR_TITLES_FACTION_CHANGE);
                    stmt->setUInt32(0, lowGuid);
                    trans->Append(stmt);
                }
            }
        }
    }

    CharacterDatabase.DirectCommitTransaction(trans);

    std::string const &addr = GetRemoteAddress();
    TC_LOG_DEBUG("entities.player", "Account: %u (IP: %s), Character guid: %u changed race from %u to %u", GetAccountId(), addr.c_str(), lowGuid, oldRace, race);

    WorldPacket data(SMSG_CHAR_FACTION_OR_RACE_CHANGE, 1 + 8 + (newname.size() + 1) + 1 + 1 + 1 + 1 + 1 + 1 + 1);
    data << uint8(RESPONSE_SUCCESS);
    data << uint64(guid);
    data << newname;
    data << uint8(gender);
    data << uint8(skin);
    data << uint8(hairColor);
    data << uint8(hairStyle);
    data << uint8(facialHair);
    data << uint8(face);
    data << uint8(race);

    SendPacket(&data);

    sWorld->InvalidatePlayerData(guid);
}

void WorldSession::HandleRandomizeCharNameOpcode(WorldPacket& recvData)
{
    uint8 gender, race;

    recvData >> race;
    recvData >> gender;

    if (!Player::IsValidRace(race))
    {
        TC_LOG_ERROR("misc", "Invalid race (%u) sent by accountId: %u", race, GetAccountId());
        return;
    }

    if (!Player::IsValidGender(gender))
    {
        TC_LOG_ERROR("misc", "Invalid gender (%u) sent by accountId: %u", gender, GetAccountId());
        return;
    }

    std::string const* name = GetRandomCharacterName(race, gender);
    WorldPacket data(SMSG_RANDOMIZE_CHAR_NAME, 10);
    data.WriteBits(name->size(), 6);
    data.WriteBit(0); // unk
    data.WriteString(name->c_str());
    SendPacket(&data);
}

void WorldSession::HandleReorderCharacters(WorldPacket& recvData)
{
    uint32 charactersCount = recvData.ReadBits(9);

    std::vector<ObjectGuid> guids(charactersCount);
    uint8 position;

    for (uint8 i = 0; i < charactersCount; ++i)
        recvData.ReadBitSeq<7, 1, 5, 6, 4, 3, 0, 2>(guids[i]);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    for (uint8 i = 0; i < charactersCount; ++i)
    {
        recvData.ReadByteSeq<0>(guids[i]);

        recvData >> position;

        recvData.ReadByteSeq<6, 2, 3, 1, 7, 4, 5>(guids[i]);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_LIST_SLOT);
        stmt->setUInt8(0, position);
        stmt->setUInt32(1, GUID_LOPART(guids[i]));
        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);
}
