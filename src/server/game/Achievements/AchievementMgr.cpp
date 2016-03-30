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
#include "DBCEnums.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "DatabaseEnv.h"
#include "AchievementMgr.h"
#include "Arena.h"
#include "CellImpl.h"
#include "GameEventMgr.h"
#include "GridNotifiersImpl.h"
#include "Guild.h"
#include "Language.h"
#include "Player.h"
#include "SpellMgr.h"
#include "DisableMgr.h"
#include "ScriptMgr.h"
#include "MapManager.h"
#include "Battleground.h"
#include "BattlegroundAB.h"
#include "Map.h"
#include "InstanceScript.h"
#include "Group.h"
#include "Chat.h"
#include "LexicalCast.h"

namespace Trinity
{
    class AchievementChatBuilder
    {
        public:
            AchievementChatBuilder(Player const& player, ChatMsg msgtype, int32 textId, uint32 ach_id)
                : i_player(player), i_msgtype(msgtype), i_textId(textId), i_achievementId(ach_id) {}
            void operator()(WorldPacket& data, LocaleConstant loc_idx)
            {
                auto const &text = sObjectMgr->GetTrinityString(i_textId, loc_idx);
                ChatHandler::FillMessageData(&data, i_player.GetSession(), i_msgtype, LANG_UNIVERSAL, NULL, i_player.GetGUID(), text, NULL, NULL, i_achievementId);
            }

        private:
            Player const& i_player;
            ChatMsg i_msgtype;
            int32 i_textId;
            uint32 i_achievementId;
    };
}                                                           // namespace Trinity

bool AchievementCriteriaData::IsValid(CriteriaEntry const* criteria)
{
    if (dataType >= MAX_ACHIEVEMENT_CRITERIA_DATA_TYPE)
    {
        TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` for criteria (Entry: %u) has wrong data type (%u), ignored.", criteria->ID, dataType);
        return false;
    }

    switch (criteria->type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:          // Only hardcoded list
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:    // Only Children's Week achievements
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:                // Only Children's Week achievements
        case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            break;
        default:
            if (dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` has data for non-supported criteria type (Entry: %u Type: %u), ignored.", criteria->ID, criteria->type);
                return false;
            }
            break;
    }

    switch (dataType)
    {
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE:
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_VALUE:
        case ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT:
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_CREATURE:
            if (!creature.id || !sObjectMgr->GetCreatureTemplate(creature.id))
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_CREATURE (%u) has non-existing creature id in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, creature.id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE:
            if (!classRace.class_id && !classRace.race_id)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE (%u) must not have 0 in either value field, ignored.",
                    criteria->ID, criteria->type, dataType);
                return false;
            }
            if (classRace.class_id && ((1 << (classRace.class_id-1)) & CLASSMASK_ALL_PLAYABLE) == 0)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE (%u) has non-existing class in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, classRace.class_id);
                return false;
            }
            if (classRace.race_id && ((1 << (classRace.race_id-1)) & RACEMASK_ALL_PLAYABLE) == 0)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE (%u) has non-existing race in value2 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, classRace.race_id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_LESS_HEALTH:
            if (health.percent < 1 || health.percent > 100)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_PLAYER_LESS_HEALTH (%u) has wrong percent value in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, health.percent);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA:
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA:
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(aura.spell_id);
            if (!spellEntry)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has wrong spell id in value1 (%u), ignored.",
                    criteria->ID, criteria->type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA?"ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA":"ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.spell_id);
                return false;
            }
            if (aura.effect_idx >= 3)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has wrong spell effect index in value2 (%u), ignored.",
                    criteria->ID, criteria->type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA?"ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA":"ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.effect_idx);
                return false;
            }
            if (!spellEntry->Effects[aura.effect_idx].ApplyAuraName)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has non-aura spell effect (ID: %u Effect: %u), ignores.",
                    criteria->ID, criteria->type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA?"ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA":"ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.spell_id, aura.effect_idx);
                return false;
            }
            return true;
        }
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL:
            if (level.minlevel > STRONG_MAX_LEVEL)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL (%u) has wrong minlevel in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, level.minlevel);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER:
            if (gender.gender > GENDER_NONE)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER (%u) has wrong gender in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, gender.gender);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT:
            if (!ScriptId)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT (%u) does not have ScriptName set, ignored.",
                    criteria->ID, criteria->type, dataType);
                return false;
            }
            return true;
        /*
        @Todo:
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_DIFFICULTY:
        */
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT:
            if (map_players.maxcount <= 0)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT (%u) has wrong max players count in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, map_players.maxcount);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM:
            if (team.team != ALLIANCE && team.team != HORDE)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM (%u) has unknown team in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, team.team);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK:
            if (drunk.state >= MAX_DRUNKEN)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK (%u) has unknown drunken state in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, drunk.state);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY:
            if (!sHolidaysStore.LookupEntry(holiday.id))
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY (%u) has unknown holiday in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, holiday.id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_BG_LOSS_TEAM_SCORE:
            return true;                                    // Not check correctness node indexes
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_EQUIPED_ITEM:
            if (equipped_item.item_quality >= MAX_ITEM_QUALITY)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_requirement` (Entry: %u Type: %u) for requirement ACHIEVEMENT_CRITERIA_REQUIRE_S_EQUIPED_ITEM (%u) has unknown quality state in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, equipped_item.item_quality);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE:
            if (!classRace.class_id && !classRace.race_id)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) must not have 0 in either value field, ignored.",
                    criteria->ID, criteria->type, dataType);
                return false;
            }
            if (classRace.class_id && ((1 << (classRace.class_id-1)) & CLASSMASK_ALL_PLAYABLE) == 0)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) has non-existing class in value1 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, classRace.class_id);
                return false;
            }
            if (classRace.race_id && ((1 << (classRace.race_id-1)) & RACEMASK_ALL_PLAYABLE) == 0)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) has non-existing race in value2 (%u), ignored.",
                    criteria->ID, criteria->type, dataType, classRace.race_id);
                return false;
            }
            return true;
        default:
            TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` (Entry: %u Type: %u) has data for non-supported data type (%u), ignored.", criteria->ID, criteria->type, dataType);
            return false;
    }
}

bool AchievementCriteriaData::Meets(uint32 criteria_id, Player const* source, Unit const* target, uint64 miscValue1 /*= 0*/) const
{
    switch (dataType)
    {
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE:
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_CREATURE:
            if (!target || target->GetTypeId() != TYPEID_UNIT)
                return false;
            return target->GetEntry() == creature.id;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE:
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return false;
            if (classRace.class_id && classRace.class_id != target->ToPlayer()->getClass())
                return false;
            if (classRace.race_id && classRace.race_id != target->ToPlayer()->getRace())
                return false;
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE:
            if (!source || source->GetTypeId() != TYPEID_PLAYER)
                return false;
            if (classRace.class_id && classRace.class_id != source->ToPlayer()->getClass())
                return false;
            if (classRace.race_id && classRace.race_id != source->ToPlayer()->getRace())
                return false;
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_LESS_HEALTH:
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return false;
            return !target->HealthAbovePct(health.percent);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA:
            return source->HasAuraEffect(aura.spell_id, aura.effect_idx);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA:
            return target && target->HasAuraEffect(aura.spell_id, aura.effect_idx);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_VALUE:
           return miscValue1 >= value.minvalue;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL:
            if (!target)
                return false;
            return target->getLevel() >= level.minlevel;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER:
            if (!target)
                return false;
            return target->getGender() == gender.gender;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT:
            return sScriptMgr->OnCriteriaCheck(ScriptId, criteria_id, miscValue1, const_cast<Player*>(source), const_cast<Unit*>(target));
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT:
            return source->GetMap()->GetPlayersCountExceptGMs() <= map_players.maxcount;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM:
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return false;
            return target->ToPlayer()->GetTeam() == team.team;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK:
            return Player::GetDrunkenstateByValue(source->GetDrunkValue()) >= DrunkenState(drunk.state);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY:
            return IsHolidayActive(HolidayIds(holiday.id));
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_BG_LOSS_TEAM_SCORE:
        {
            Battleground* bg = source->GetBattleground();
            if (!bg)
                return false;

            uint16 winnerTeamScore = 0;
            switch(bg->GetTypeID(true))
            {
                case BATTLEGROUND_WS:
                    winnerTeamScore = 3;
                    break;
                case BATTLEGROUND_AB:
                case BATTLEGROUND_EY:
                    winnerTeamScore = 1600;
                    break;
                default:
                    break;
            }
            if (winnerTeamScore > 0 && !bg->IsTeamScoreInRange(source->GetTeam(), winnerTeamScore, winnerTeamScore))
                return false;

            return bg->IsTeamScoreInRange(source->GetTeam() == ALLIANCE ? HORDE : ALLIANCE, bg_loss_team_score.min_score, bg_loss_team_score.max_score);
        }
        case ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT:
        {
            if (!source->IsInWorld())
                return false;
            Map* map = source->GetMap();
            if (!map->IsDungeon())
            {
                TC_LOG_ERROR("achievement", "Achievement system call ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT (%u) for achievement criteria %u for non-dungeon/non-raid map %u",
                    ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT, criteria_id, map->GetId());
                return false;
            }
            InstanceScript* instance = ((InstanceMap*)map)->GetInstanceScript();
            if (!instance)
            {
                TC_LOG_ERROR("achievement", "Achievement system call ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT (%u) for achievement criteria %u for map %u but map does not have a instance script",
                    ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT, criteria_id, map->GetId());
                return false;
            }
            return instance->CheckAchievementCriteriaMeet(criteria_id, source, target, miscValue1);
        }
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_EQUIPED_ITEM:
        {
            ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(miscValue1);
            if (!pProto)
                return false;
            return pProto->ItemLevel >= equipped_item.item_level && pProto->Quality >= equipped_item.item_quality;
        }
        default:
            break;
    }
    return false;
}

bool AchievementCriteriaDataSet::Meets(Player const* source, Unit const* target, uint64 miscvalue /*= 0*/) const
{
    for (Storage::const_iterator itr = storage.begin(); itr != storage.end(); ++itr)
        if (!itr->Meets(criteria_id, source, target, miscvalue))
            return false;

    return true;
}

template<class T>
AchievementMgr<T>::AchievementMgr(T* owner): _owner(owner), _achievementPoints(0)
{
}

template<class T>
AchievementMgr<T>::~AchievementMgr()
{
}

template<class T>
void AchievementMgr<T>::SendPacket(WorldPacket* /*data*/) const
{
}

template<>
void AchievementMgr<Guild>::SendPacket(WorldPacket* data) const
{
    GetOwner()->BroadcastPacket(data);
}

template<>
void AchievementMgr<Player>::SendPacket(WorldPacket* data) const
{
    GetOwner()->GetSession()->SendPacket(data);
}

template<class T>
void AchievementMgr<T>::RemoveCriteriaProgress(const CriteriaEntry* entry)
{
    CriteriaProgressMap &progressMap = GetCriteriaProgressMap();

    auto const itr = progressMap.find(entry->ID);
    if (itr == progressMap.end())
        return;

    WorldPacket data(SMSG_CRITERIA_DELETED, 4);
    data << uint32(entry->ID);
    SendPacket(&data);

    progressMap.erase(itr);
}

template<>
void AchievementMgr<Guild>::RemoveCriteriaProgress(const CriteriaEntry* entry)
{
    CriteriaProgressMap &progressMap = GetCriteriaProgressMap();

    auto const itr = progressMap.find(entry->ID);
    if (itr == progressMap.end())
        return;

    ObjectGuid guid = GetOwner()->GetGUID();

    WorldPacket data(SMSG_GUILD_CRITERIA_DELETED, 4 + 8);

    data.WriteBitSeq<7, 3, 4, 2, 1, 5, 6, 0>(guid);

    data.WriteByteSeq<5, 7, 4, 6, 2, 1, 3>(guid);
    data << uint32(entry->ID);
    data.WriteByteSeq<0>(guid);

    SendPacket(&data);

    progressMap.erase(itr);
}

template<class T>
void AchievementMgr<T>::ResetAchievementCriteria(AchievementCriteriaTypes type, uint64 miscValue1, uint64 miscValue2, bool evenIfCriteriaComplete)
{
    // Disable for gamemasters with GM-mode enabled
    if (GetOwner()->isGameMaster())
        return;

    AchievementCriteriaEntryList const& achievementCriteriaList = sAchievementMgr->GetAchievementCriteriaByType(type);
    for (AchievementCriteriaEntryList::const_iterator i = achievementCriteriaList.begin(); i != achievementCriteriaList.end(); ++i)
    {
        CriteriaEntry const* achievementCriteria = (*i);
        // don't update already completed criteria if not forced or achievement already complete
        if (!IsCompletedCriteria(achievementCriteria))
            RemoveCriteriaProgress(achievementCriteria);
    }
}

template<>
void AchievementMgr<Guild>::ResetAchievementCriteria(AchievementCriteriaTypes /*type*/, uint64 /*miscValue1*/, uint64 /*miscValue2*/, bool /*evenIfCriteriaComplete*/)
{
    // Not needed
}

template<class T>
void AchievementMgr<T>::DeleteFromDB(uint32 /*lowguid*/, uint32 /*accountId*/)
{
}

template<>
void AchievementMgr<Player>::DeleteFromDB(uint32 lowguid, uint32 /*accountId*/)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_ACHIEVEMENT);
    stmt->setUInt32(0, lowguid);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

template<>
void AchievementMgr<Guild>::DeleteFromDB(uint32 lowguid, uint32 /*accountId*/)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENTS);
    stmt->setUInt32(0, lowguid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENT_CRITERIA);
    stmt->setUInt32(0, lowguid);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

template<class T>
void AchievementMgr<T>::SaveToDB(SQLTransaction& /*charTrans*/, SQLTransaction& /*authTrans*/)
{
}

template<>
void AchievementMgr<Player>::SaveToDB(SQLTransaction& charTrans, SQLTransaction& authTrans)
{
    uint32 const guidLow = GetOwner()->GetGUIDLow();
    uint32 const accountId = GetOwner()->GetSession()->GetAccountId();

    if (!m_completedAchievements.empty())
    {
        bool needCharacterExecute = false;
        bool needAccountExecute = false;

        std::ostringstream ssAccDel;
        std::ostringstream ssAccIns;

        std::ostringstream ssCharDel;
        std::ostringstream ssCharIns;
        std::ostringstream sscount;

        for (auto &kvPair : m_completedAchievements)
        {
            auto const achievementId = kvPair.first;
            auto &completionData = kvPair.second;

            if (!completionData.changed)
                continue;

            AchievementEntry const* ach = sAchievementMgr->GetAchievement(achievementId);
            if (!ach)
                continue;

            bool isAccountAchievement = ach->flags & ACHIEVEMENT_FLAG_ACCOUNT;
            bool const mustSaveForCharacter = completionData.completedByThisCharacter;
            if (!needAccountExecute)
            {
                if (isAccountAchievement)
                {
                    // First new/changed record prefix
                    ssAccDel << "DELETE FROM account_achievement WHERE account = " << accountId << " AND achievement IN (";
                    ssAccIns << "INSERT INTO account_achievement (account, first_guid, achievement, date) VALUES ";
                    needAccountExecute = true;
                }
            }
            else
            {
                if (isAccountAchievement)
                {
                    // Next new/changed record prefix
                    ssAccDel << ',';
                    ssAccIns << ',';
                }
            }

            if (!needCharacterExecute)
            {
                if (mustSaveForCharacter || !isAccountAchievement)
                {
                    ssCharDel << "DELETE FROM character_achievement WHERE guid = " << guidLow << " AND achievement IN (";
                    ssCharIns << "INSERT INTO character_achievement (guid, achievement, date) VALUES ";
                    needCharacterExecute = true;
                }
            }
            else
            {
                if (mustSaveForCharacter || !isAccountAchievement)
                {
                    ssCharDel << ',';
                    ssCharIns << ',';
                }
            }

            if (isAccountAchievement)
            {
                // New/changed record data
                ssAccDel << achievementId;
                ssAccIns << '(' << accountId << ',' << completionData.first_guid << ',' << achievementId << ',' << completionData.date << ')';
            }

            if (mustSaveForCharacter || !isAccountAchievement)
            {
                ssCharDel << achievementId;
                ssCharIns << '(' << guidLow << ',' << achievementId << "," << completionData.date << ')';
            }

            /// Mark as saved in db
            completionData.changed = false;
        }

        if (needAccountExecute)
        {
            ssAccDel << ')';

            authTrans->Append(ssAccDel.str().c_str());
            authTrans->Append(ssAccIns.str().c_str());
        }

        if (needCharacterExecute)
        {
            ssCharDel << ')';

            charTrans->Append(ssCharDel.str().c_str());
            charTrans->Append(ssCharIns.str().c_str());
        }
    }

    CriteriaProgressMap &progressMap = GetCriteriaProgressMap();
    if (progressMap.empty())
        return;

    // Prepare deleting and insert
    bool needExecuteDel     = false;
    bool needExecuteIns     = false;
    bool needExecuteAccount = false;

    bool isAccountAchievement = false;

    bool alreadyOneCharDelLine = false;
    bool alreadyOneAccDelLine  = false;
    bool alreadyOneCharInsLine = false;
    bool alreadyOneAccInsLine  = false;

    std::ostringstream ssAccDel;
    std::ostringstream ssAccIns;
    std::ostringstream ssCharDel;
    std::ostringstream ssCharIns;

    for (auto &kvPair : progressMap)
    {
        auto const criteriaId = kvPair.first;
        auto &progressData = kvPair.second;

        if (!progressData.changed)
            continue;

        auto const criteria = sAchievementMgr->GetAchievementCriteria(criteriaId);
        if (!criteria)
            continue;

        AchievementEntry const* achievement;
        AchievementCriteriaTreeList const& criteriaTreeList = sAchievementMgr->GetAchievementCriteriaTreeList(criteria);
        for (AchievementCriteriaTreeList::const_iterator iter = criteriaTreeList.begin(); iter != criteriaTreeList.end(); iter++)
        {
            CriteriaTreeEntry const* criteriaTree = *iter;
            achievement = sAchievementMgr->GetAchievementEntryByCriteriaTree(criteriaTree);
        }

            if (!achievement)
            continue;

        if (achievement->flags & ACHIEVEMENT_FLAG_ACCOUNT)
        {
            isAccountAchievement = true;
            needExecuteAccount = true;
        }
        else
            isAccountAchievement = false;

        // Deleted data (including 0 progress state)
        if (!needExecuteDel)
        {
            // First new/changed record prefix (for any counter value)
            ssAccDel << "DELETE FROM account_achievement_progress WHERE account = " << accountId << " AND criteria IN (";
            ssCharDel << "DELETE FROM character_achievement_progress WHERE guid = " << guidLow << " AND criteria IN (";
            needExecuteDel = true;
        }
        else
        {
            // Next new/changed record prefix
            if (isAccountAchievement)
            {
                if (alreadyOneAccDelLine)
                    ssAccDel << ',';
            }
            else
            {
                if (alreadyOneCharDelLine)
                    ssCharDel << ',';
            }
        }

        // New/changed record data
        if (isAccountAchievement)
        {
            ssAccDel << criteriaId;
            alreadyOneAccDelLine = true;
        }
        else
        {
            ssCharDel << criteriaId;
            alreadyOneCharDelLine = true;
        }

        // Store data only for real progress
        if (progressData.counter != 0)
        {
            if (!needExecuteIns)
            {
                // First new/changed record prefix
                ssAccIns << "INSERT INTO account_achievement_progress (account, criteria, counter, date) VALUES ";
                ssCharIns << "INSERT INTO character_achievement_progress (guid, criteria, counter, date) VALUES ";
                needExecuteIns = true;
            }
            else
            {
                // Next new/changed record prefix
                if (isAccountAchievement)
                {
                    if (alreadyOneAccInsLine)
                        ssAccIns << ',';
                }
                else
                {
                    if (alreadyOneCharInsLine)
                        ssCharIns << ',';
                }
            }

            // New/changed record data
            if (isAccountAchievement)
            {
                ssAccIns << '(' << accountId << ',' << criteriaId << ',' << progressData.counter << ',' << progressData.date << ')';
                alreadyOneAccInsLine = true;
            }
            else
            {
                ssCharIns << '(' << guidLow << ',' << criteriaId << ',' << progressData.counter << ',' << progressData.date << ')';
                alreadyOneCharInsLine = true;
            }
        }

        // Mark as updated in db
        progressData.changed = false;
    }

    // DELETE ... IN (.... _)_
    if (needExecuteDel)
    {
        ssAccDel << ')';
        ssCharDel << ')';
    }

    if (needExecuteDel || needExecuteIns)
    {
        if (needExecuteDel)
        {
            if (needExecuteAccount && alreadyOneAccDelLine)
                authTrans->Append(ssAccDel.str().c_str());

            if (alreadyOneCharDelLine)
                charTrans->Append(ssCharDel.str().c_str());
        }

        if (needExecuteIns)
        {
            if (needExecuteAccount && alreadyOneAccInsLine)
                authTrans->Append(ssAccIns.str().c_str());

            if (alreadyOneCharInsLine)
                charTrans->Append(ssCharIns.str().c_str());
        }
    }
}

template<>
void AchievementMgr<Guild>::SaveToDB(SQLTransaction& trans, SQLTransaction& /*trans*/)
{
    PreparedStatement* stmt;
    std::ostringstream guidstr;

    for (auto &kvPair : m_completedAchievements)
    {
        auto const achievementId = kvPair.first;
        auto &completionData = kvPair.second;

        if (!completionData.changed)
            continue;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_ACHIEVEMENT);
        stmt->setUInt32(0, GetOwner()->GetId());
        stmt->setUInt16(1, achievementId);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_ACHIEVEMENT);
        stmt->setUInt32(0, GetOwner()->GetId());
        stmt->setUInt16(1, achievementId);
        stmt->setUInt32(2, completionData.date);
        for (auto const &guid : completionData.guids)
            guidstr << GUID_LOPART(guid) << ',';
        stmt->setString(3, guidstr.str());
        trans->Append(stmt);

        guidstr.str("");

        completionData.changed = false;
    }

    for (auto &kvPair : GetCriteriaProgressMap())
    {
        auto const criteriaId = kvPair.first;
        auto &progressData = kvPair.second;

        if (!progressData.changed)
            continue;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_ACHIEVEMENT_CRITERIA);
        stmt->setUInt32(0, GetOwner()->GetId());
        stmt->setUInt16(1, criteriaId);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_ACHIEVEMENT_CRITERIA);
        stmt->setUInt32(0, GetOwner()->GetId());
        stmt->setUInt16(1, criteriaId);
        stmt->setUInt64(2, progressData.counter);
        stmt->setUInt32(3, progressData.date);
        stmt->setUInt32(4, GUID_LOPART(progressData.CompletedGUID));
        trans->Append(stmt);

        progressData.changed = false;
    }
}

template<class T>
void AchievementMgr<T>::LoadFromDB(PreparedQueryResult /*achievementResult*/, PreparedQueryResult /*criteriaResult*/, PreparedQueryResult /*achievementAccountResult*/, PreparedQueryResult /*criteriaAccountResult*/)
{
}

template<>
void AchievementMgr<Player>::LoadFromDB(PreparedQueryResult achievementResult, PreparedQueryResult criteriaResult, PreparedQueryResult achievementAccountResult, PreparedQueryResult criteriaAccountResult)
{
    if (achievementAccountResult)
    {
        do
        {
            Field* fields = achievementAccountResult->Fetch();
            uint32 first_guid    = fields[0].GetUInt32();
            uint32 achievementid = fields[1].GetUInt16();

            // Must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(achievementid);
            if (!achievement)
                continue;

            if (achievement->flags & ACHIEVEMENT_FLAG_GUILD)
                continue;

            CompletedAchievementData& ca = m_completedAchievements[achievementid];
            ca.date = time_t(fields[2].GetUInt32());
            ca.changed = false;
            ca.first_guid = MAKE_NEW_GUID(first_guid, 0, HIGHGUID_PLAYER);
            ca.completedByThisCharacter = false;

            _achievementPoints += achievement->points;

            // Title achievement rewards are retroactive
            if (AchievementReward const* reward = sAchievementMgr->GetAchievementReward(achievement))
                if (uint32 titleId = reward->titleId[Player::TeamForRace(GetOwner()->getRace()) == ALLIANCE ? 0 : 1])
                    if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId))
                        GetOwner()->SetTitle(titleEntry);

        }
        while (achievementAccountResult->NextRow());
    }

    if (criteriaResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint64 counter = fields[1].GetUInt64();
            time_t date    = time_t(fields[2].GetUInt32());

            CriteriaEntry const* criteria = sAchievementMgr->GetAchievementCriteria(id);
            if (!criteria)
            {
                // We will remove not existed criteria for all characters
                TC_LOG_ERROR("achievement", "Non-existing achievement criteria %u data removed from table `character_achievement_progress`.", id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt16(0, uint16(id));
                CharacterDatabase.Execute(stmt);

                continue;
            }

            if (criteria->timeLimit && time_t(date + criteria->timeLimit) < now)
                continue;

            CriteriaProgress& progress = m_criteriaProgress[id];
            progress.counter = counter;
            progress.date    = date;
            progress.changed = false;
        }
        while (criteriaResult->NextRow());
    }

    if (achievementResult)
    {
        do
        {
            Field* fields = achievementResult->Fetch();
            uint32 achievementid = fields[0].GetUInt16();
            uint32 guid = fields[1].GetUInt32();

            // Must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(achievementid);
            if (!achievement)
                continue;

            if (achievement->flags & ACHIEVEMENT_FLAG_GUILD)
                continue;

            // Achievement in character_achievement but not in account_achievement, there is a problem.
            if (m_completedAchievements.find(achievementid) == m_completedAchievements.end() && achievement->flags & ACHIEVEMENT_FLAG_ACCOUNT)
            {
                TC_LOG_ERROR("achievement", "Account-wide achievement '%u' in character_achievement but not in account_achievement, there is a problem.", achievementid);
                continue;
            }

            CompletedAchievementData& ca = m_completedAchievements[achievementid];
            ca.completedByThisCharacter = true;
            ca.date = time_t(fields[2].GetUInt32());
            ca.changed = false;
            ca.first_guid = MAKE_NEW_GUID(guid, 0, HIGHGUID_PLAYER);
            _achievementPoints += achievement->points;
        }
        while (achievementResult->NextRow());
    }

    if (criteriaAccountResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaAccountResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint64 counter = fields[1].GetUInt64();
            time_t date    = time_t(fields[2].GetUInt32());

            CriteriaEntry const* criteria = sAchievementMgr->GetAchievementCriteria(id);
            if (!criteria)
            {
                // We will remove not existed criteria for all characters
                TC_LOG_ERROR("achievement", "Non-existing achievement criteria %u data removed from table `character_achievement_progress`.", id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt16(0, uint16(id));
                CharacterDatabase.Execute(stmt);

                continue;
            }

            if (criteria->timeLimit && time_t(date + criteria->timeLimit) < now)
                continue;

            CriteriaProgressMap &progressMap = GetCriteriaProgressMap();

            // Achievement in both account & characters achievement_progress, problem
            if (progressMap.find(id) != progressMap.end())
            {
                TC_LOG_ERROR("achievement", "Achievement '%u' in both account & characters achievement_progress", id);
                continue;
            }

            CriteriaProgress& progress = progressMap[id];
            progress.counter = counter;
            progress.date    = date;
            progress.changed = false;
        }
        while (criteriaAccountResult->NextRow());
    }
}

template<>
void AchievementMgr<Guild>::LoadFromDB(PreparedQueryResult achievementResult, PreparedQueryResult criteriaResult, PreparedQueryResult /*achievementAccountResult*/, PreparedQueryResult /*criteriaAccountResult*/)
{
    if (achievementResult)
    {
        do
        {
            Field* fields = achievementResult->Fetch();
            uint32 achievementid = fields[0].GetUInt16();

            // Must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(achievementid);
            if (!achievement)
                continue;

            if (!(achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            CompletedAchievementData& ca = m_completedAchievements[achievementid];
            ca.date = time_t(fields[1].GetUInt32());
            Tokenizer guids(fields[2].GetString(), ' ');
            for (uint32 i = 0; i < guids.size(); ++i)
                ca.guids.insert(MAKE_NEW_GUID(Trinity::lexicalCast<uint32>(guids[i]), 0, HIGHGUID_PLAYER));

            ca.changed = false;
            _achievementPoints += achievement->points;

        }
        while (achievementResult->NextRow());
    }

    if (criteriaResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint64 counter = fields[1].GetUInt64();
            time_t date    = time_t(fields[2].GetUInt32());
            uint64 guid    = MAKE_NEW_GUID(fields[3].GetUInt32(), 0, HIGHGUID_PLAYER);

            CriteriaEntry const* criteria = sAchievementMgr->GetAchievementCriteria(id);
            if (!criteria)
            {
                // We will remove not existed criteria for all guilds
                TC_LOG_ERROR("achievement", "Non-existing achievement criteria %u data removed from table `guild_achievement_progress`.", id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA_GUILD);
                stmt->setUInt16(0, uint16(id));
                CharacterDatabase.Execute(stmt);
                continue;
            }

            if (criteria->timeLimit && time_t(date + criteria->timeLimit) < now)
                continue;

            CriteriaProgress& progress = m_criteriaProgress[id];
            progress.counter = counter;
            progress.date    = date;
            progress.CompletedGUID = MAKE_NEW_GUID(guid, 0, HIGHGUID_PLAYER);
            progress.changed = false;
        }
        while (criteriaResult->NextRow());
    }
}

template<class T>
void AchievementMgr<T>::Reset()
{
}

template<>
void AchievementMgr<Player>::Reset()
{
    for (auto const &kvPair : m_completedAchievements)
    {
        WorldPacket data(SMSG_ACHIEVEMENT_DELETED, 4);
        data << uint32(kvPair.first);
        SendPacket(&data);
    }
    m_completedAchievements.clear();

    for (auto const &kvPair : GetCriteriaProgressMap())
    {
        WorldPacket data(SMSG_CRITERIA_DELETED, 4);
        data << uint32(kvPair.first);
        SendPacket(&data);
    }
    GetCriteriaProgressMap().clear();

    _achievementPoints = 0;
    DeleteFromDB(GetOwner()->GetGUIDLow());

    // re-fill data
    GetOwner()->CheckAllAchievementCriteria();
}

template<>
void AchievementMgr<Guild>::Reset()
{
    ObjectGuid const guid = GetOwner()->GetGUID();

    for (auto const &kvPair : m_completedAchievements)
    {
        WorldPacket data(SMSG_GUILD_ACHIEVEMENT_DELETED, 4);

        data.WriteBitSeq<4, 1, 2, 3, 0, 7, 5, 6>(guid);
        data << uint32(kvPair.first);
        data.WriteByteSeq<5, 1, 3, 6, 0, 7>(guid);
        data << uint32(secsToTimeBitFields(kvPair.second.date));
        data.WriteByteSeq<4, 2>(guid);

        SendPacket(&data);
    }
    m_completedAchievements.clear();

    for (auto const &kvPair : GetCriteriaProgressMap())
    {
        WorldPacket data(SMSG_GUILD_CRITERIA_DELETED, 4 + 8);

        data.WriteBitSeq<7, 3, 4, 2, 1, 5, 6, 0>(guid);
        data.WriteByteSeq<5, 7, 4, 6, 2, 1, 3>(guid);
        data << uint32(kvPair.first);
        data.WriteByteSeq<0>(guid);

        SendPacket(&data);
    }
    GetCriteriaProgressMap().clear();

    _achievementPoints = 0;
    DeleteFromDB(GetOwner()->GetId());
}

template<class T>
void AchievementMgr<T>::SendAchievementEarned(AchievementEntry const* achievement) const
{
    // Don't send for achievements with ACHIEVEMENT_FLAG_HIDDEN
    if (achievement->flags & ACHIEVEMENT_FLAG_HIDDEN)
        return;

    if (Guild* guild = sGuildMgr->GetGuildById(GetOwner()->GetGuildId()))
    {
        Trinity::AchievementChatBuilder say_builder(*GetOwner(), CHAT_MSG_GUILD_ACHIEVEMENT, LANG_ACHIEVEMENT_EARNED, achievement->ID);
        Trinity::LocalizedPacketDo<Trinity::AchievementChatBuilder> say_do(say_builder);
        guild->BroadcastWorker(say_do);
    }

    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_REACH))
    {
        if (WorldSession* session = GetOwner()->GetSession())
        {
            WorldPacket data;
            auto const &text = session->GetTrinityString(LANG_ACHIEVEMENT_EARNED);
            ChatHandler::FillMessageData(&data, session, CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, NULL, session->GetPlayer()->GetGUID(), text, NULL, NULL, achievement->ID);
            sWorld->SendGlobalMessage(&data);
        }
        // Broadcast realm first reached
        /*WorldPacket data(SMSG_SERVER_FIRST_ACHIEVEMENT, GetOwner()->GetName().length() + 1 + 8 + 4 + 4);
        data << GetOwner()->GetName();
        data << uint64(GetOwner()->GetGUID());
        data << uint32(achievement->ID);
        data << uint32(0);                                  // 1=link supplied string as player name, 0=display plain string
        sWorld->SendGlobalMessage(&data);*/
    }
    // If player is in world he can tell his friends about new achievement
    else if (GetOwner()->IsInWorld())
    {
        CellCoord p = Trinity::ComputeCellCoord(GetOwner()->GetPositionX(), GetOwner()->GetPositionY());

        Cell cell(p);
        cell.SetNoCreate();

        Trinity::AchievementChatBuilder say_builder(*GetOwner(), CHAT_MSG_ACHIEVEMENT, LANG_ACHIEVEMENT_EARNED, achievement->ID);
        Trinity::LocalizedPacketDo<Trinity::AchievementChatBuilder> say_do(say_builder);
        Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::AchievementChatBuilder> > say_worker(GetOwner(), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), say_do);

        cell.Visit(p, Trinity::makeWorldVisitor(say_worker), *GetOwner()->GetMap(), *GetOwner(), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY));
    }

    WorldPacket data(SMSG_ACHIEVEMENT_EARNED);

    ObjectGuid thisPlayerGuid = GetOwner()->GetGUID();
    ObjectGuid firstPlayerOnAccountGuid = GetOwner()->GetGUID();

    if (HasAccountAchieved(achievement->ID))
        firstPlayerOnAccountGuid = GetFirstAchievedCharacterOnAccount(achievement->ID);

    data << uint32(secsToTimeBitFields(time(NULL)));
    data << uint32(50724905);
    data << uint32(achievement->ID);
    data << uint32(50724905);
    data.WriteBitSeq<6>(thisPlayerGuid);
    data.WriteBitSeq<5, 3, 0, 1, 6>(firstPlayerOnAccountGuid);
    data.WriteBitSeq<2>(thisPlayerGuid);
    data.WriteBitSeq<2>(firstPlayerOnAccountGuid);
    data.WriteBitSeq<1, 4, 5, 7>(thisPlayerGuid);
    data.WriteBitSeq<7>(firstPlayerOnAccountGuid);
    data.WriteBitSeq<0>(thisPlayerGuid);
    data.WriteBitSeq<4>(firstPlayerOnAccountGuid);
    data.WriteBit(0);
    data.WriteBitSeq<3>(thisPlayerGuid);

    data.WriteByteSeq<4>(firstPlayerOnAccountGuid);
    data.WriteByteSeq<0, 2, 6, 1>(thisPlayerGuid);
    data.WriteByteSeq<1, 5, 0>(firstPlayerOnAccountGuid);
    data.WriteByteSeq<7, 3, 5>(thisPlayerGuid);
    data.WriteByteSeq<6>(firstPlayerOnAccountGuid);
    data.WriteByteSeq<4>(thisPlayerGuid);
    data.WriteByteSeq<3, 7, 2>(firstPlayerOnAccountGuid);

    GetOwner()->SendMessageToSetInRange(&data, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), true);
}

template<>
void AchievementMgr<Guild>::SendAchievementEarned(AchievementEntry const* achievement) const
{
    ObjectGuid guid = GetOwner()->GetGUID();

    WorldPacket data(SMSG_GUILD_ACHIEVEMENT_EARNED, 8+4+8);

    data.WriteBitSeq<6, 1, 5, 0, 3, 4, 2, 7>(guid);

    data.WriteByteSeq<7>(guid);
    data << uint32(achievement->ID);
    data.WriteByteSeq<4, 3, 1, 6, 5>(guid);
    data << uint32(secsToTimeBitFields(time(NULL)));
    data.WriteByteSeq<0, 2>(guid);

    SendPacket(&data);
}

template<class T>
void AchievementMgr<T>::SendCriteriaUpdate(CriteriaEntry const* /*entry*/, CriteriaProgress const* /*progress*/, uint32 /*timeElapsed*/, bool /*timedCompleted*/) const
{
}

template<>
void AchievementMgr<Player>::SendCriteriaUpdate(CriteriaEntry const* entry, CriteriaProgress const* progress, uint32 timeElapsed, bool timedCompleted) const
{
    WorldPacket data(SMSG_CRITERIA_UPDATE, 4 + 4 + 4 + 4 + 8 + 4);
    ObjectGuid playerGuid = GetOwner()->GetGUID();

    data << uint32(entry->ID);

    // This are some flags, 1 is for keeping the counter at 0 in client
    if (!entry->timeLimit)
        data << uint32(0);
    else
        data << uint32(timedCompleted ? 0 : 1);

    data << uint32(timeElapsed);                        // Time elapsed in seconds
    data.AppendPackedTime(progress->date);
    data << (uint64)progress->counter;
    data << uint32(timeElapsed);

    data.WriteBitSeq<2, 4, 1, 5, 3, 6, 7, 0>(playerGuid);
    data.WriteByteSeq<7, 0, 6, 5, 2, 1, 4, 3>(playerGuid);

    SendPacket(&data);
}

template<>
void AchievementMgr<Guild>::SendCriteriaUpdate(CriteriaEntry const* entry, CriteriaProgress const* progress, uint32 /*timeElapsed*/, bool /*timedCompleted*/) const
{
    // Will send response to criteria progress request
    WorldPacket data(SMSG_GUILD_CRITERIA_DATA);

    ObjectGuid counter = progress->counter; // For accessing every byte individually
    ObjectGuid guid = progress->CompletedGUID;

    data.WriteBits(1, 19);
    data.WriteBitSeq<3, 6>(counter);
    data.WriteBitSeq<5, 4>(guid);
    data.WriteBitSeq<2>(counter);
    data.WriteBitSeq<0>(guid);
    data.WriteBitSeq<7>(counter);
    data.WriteBitSeq<6, 7, 3>(guid);
    data.WriteBitSeq<5, 0>(counter);
    data.WriteBitSeq<2>(guid);
    data.WriteBitSeq<1, 4>(counter);
    data.WriteBitSeq<1>(guid);

    data.FlushBits();

    data.WriteByteSeq<4>(guid);
    data << uint32(progress->date);      // Unknown date
    data.WriteByteSeq<2, 0>(counter);
    data << uint32(progress->changed);
    data.WriteByteSeq<7, 1>(guid);
    data << uint32(entry->ID);
    data << uint32(progress->date);      // Last update time (not packed!)
    data.WriteByteSeq<6>(guid);
    data.WriteByteSeq<1>(counter);
    data.WriteByteSeq<3>(guid);
    data.WriteByteSeq<4>(counter);
    data << uint32(progress->date);      // Unknown date
    data.WriteByteSeq<5, 3>(counter);
    data.WriteByteSeq<0>(guid);
    data.WriteByteSeq<6, 7>(counter);
    data.WriteByteSeq<2, 5>(guid);

    SendPacket(&data);
}

template<class T>
CriteriaProgressMap & AchievementMgr<T>::GetCriteriaProgressMap()
{
    return m_criteriaProgress;
}

static const uint32 achievIdByArenaSlot[MAX_ARENA_SLOT] = {1057, 1107, 1108};
static const uint32 achievIdForDungeon[][4] =
{
    // ach_cr_id, is_dungeon, is_raid, is_heroic_dungeon
    { 321,       true,      true,   true  },
    { 916,       false,     true,   false },
    { 917,       false,     true,   false },
    { 918,       true,      false,  false },
    { 2219,      false,     false,  true  },
    { 0,         false,     false,  false }
};

// Helper function to avoid having to specialize template for a 800 line long function
template <typename T> static bool IsGuild() { return false; }
template<> bool IsGuild<Guild>() { return true; }

/**
 * This function will be called whenever the user might have done a criteria relevant action
 */
template<class T>
void AchievementMgr<T>::UpdateAchievementCriteria(AchievementCriteriaTypes type, uint64 miscValue1 /*= 0*/, uint64 miscValue2 /*= 0*/, uint64 miscValue3 /*= 0*/, Unit const* unit /*= NULL*/, Player* referencePlayer /*= NULL*/)
{
    if (type >= ACHIEVEMENT_CRITERIA_TYPE_TOTAL)
        return;

    if (!IsGuild<T>() && !referencePlayer)
        return;

    // disable for gamemasters with GM-mode enabled
    if (referencePlayer)
        if (referencePlayer->isGameMaster())
            return;

     // Lua_GetGuildLevelEnabled() is checked in achievement UI to display guild tab
    if (IsGuild<T>() && !sWorld->getBoolConfig(CONFIG_GUILD_LEVELING_ENABLED))
        return;

    AchievementCriteriaEntryList const& achievementCriteriaList = sAchievementMgr->GetAchievementCriteriaByType(type, IsGuild<T>());
    for (AchievementCriteriaEntryList::const_iterator i = achievementCriteriaList.begin(); i != achievementCriteriaList.end(); ++i)
    {
        CriteriaEntry const* achievementCriteria = (*i);
        if (!CanUpdateCriteria(achievementCriteria, miscValue1, miscValue2, miscValue3, unit, referencePlayer))
            continue;

        // Requirements not found in the dbc
        if (AchievementCriteriaDataSet const* data = sAchievementMgr->GetCriteriaDataSet(achievementCriteria))
            if (!data->Meets(referencePlayer, unit, miscValue1))
                continue;

        switch (type)
        {
            // std. case: increment at 1
            case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
            case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
            case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
            case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:    // @TODO : for online player only currently
            case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
            case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
            case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
            case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
            case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
            case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
            case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
            case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
            case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
            case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
            case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
            case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
            case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
            case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
            case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
            case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
            case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
            case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
            case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
            case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
            case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
            case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
            case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:
            case ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA:
            case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_TYPE:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE:
                SetCriteriaProgress(achievementCriteria, 1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            // std case: increment at miscValue1
            case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
            case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
            case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
            case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
            case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
            case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
            case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
            case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:     //@TODO : for online player only currently
            case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
            case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
            case ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
            case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
            case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE:
            case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE:
            case ACHIEVEMENT_CRITERIA_TYPE_CATCH_FROM_POOL:
            case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
            case ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
            case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_TABARD:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_GUILD:
            case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
                SetCriteriaProgress(achievementCriteria, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            // std case: increment at miscValue2
            case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
            case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY:
                SetCriteriaProgress(achievementCriteria, miscValue2, referencePlayer, PROGRESS_ACCUMULATE);
                break;
                // std case: high value at miscValue1
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:        //@TODO : for online player only currently
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_DEALT:
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
                SetCriteriaProgress(achievementCriteria, miscValue1, referencePlayer, PROGRESS_HIGHEST);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
                SetCriteriaProgress(achievementCriteria, referencePlayer->getLevel(), referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
                if (uint32 skillvalue = referencePlayer->GetBaseSkillValue(achievementCriteria->reach_skill_level.skillID))
                    SetCriteriaProgress(achievementCriteria, skillvalue, referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
                if (uint32 maxSkillvalue = referencePlayer->GetPureMaxSkillValue(achievementCriteria->learn_skill_level.skillID))
                    SetCriteriaProgress(achievementCriteria, maxSkillvalue, referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
                SetCriteriaProgress(achievementCriteria, referencePlayer->GetRewardedQuestCount(), referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
            {
                time_t nextDailyResetTime = sWorld->GetNextDailyQuestsResetTime();
                CriteriaProgress *progress = GetCriteriaProgress(achievementCriteria);

                if (!miscValue1) // Login case.
                {
                    // Reset if player missed one day.
                    if (progress && progress->date < (nextDailyResetTime - 2 * DAY))
                        SetCriteriaProgress(achievementCriteria, 0, referencePlayer, PROGRESS_SET);
                    continue;
                }

                ProgressType progressType;
                if (!progress)
                    // 1st time. Start count.
                    progressType = PROGRESS_SET;
                else if (progress->date < (nextDailyResetTime - 2 * DAY))
                   // Last progress is older than 2 days. Player missed 1 day => Restart count.
                    progressType = PROGRESS_SET;
                else if (progress->date < (nextDailyResetTime - DAY))
                    // Last progress is between 1 and 2 days. => 1st time of the day.
                    progressType = PROGRESS_ACCUMULATE;
                else
                    // Last progress is within the day before the reset => Already counted today.
                    continue;

                SetCriteriaProgress(achievementCriteria, 1, referencePlayer, progressType);
                break;
            }
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            {
                uint32 counter = 0;

                const RewardedQuestSet &rewQuests = referencePlayer->getRewardedQuests();
                for (RewardedQuestSet::const_iterator itr = rewQuests.begin(); itr != rewQuests.end(); ++itr)
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(*itr);
                    if (quest && quest->GetZoneOrSort() >= 0 && uint32(quest->GetZoneOrSort()) == achievementCriteria->complete_quests_in_zone.zoneID)
                        ++counter;
                }
                SetCriteriaProgress(achievementCriteria, counter, referencePlayer);
                break;
            }
            case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
                // miscValue1 is the ingame fallheight*100 as stored in dbc
                SetCriteriaProgress(achievementCriteria, miscValue1, referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
            case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
                SetCriteriaProgress(achievementCriteria, 1, referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
                SetCriteriaProgress(achievementCriteria, referencePlayer->GetBankBagSlotCount(), referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            {
                int32 reputation = referencePlayer->GetReputationMgr().GetReputation(achievementCriteria->gain_reputation.factionID);
                if (reputation > 0)
                    SetCriteriaProgress(achievementCriteria, reputation, referencePlayer);
                break;
            }
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
                SetCriteriaProgress(achievementCriteria, referencePlayer->GetReputationMgr().GetExaltedFactionCount(), referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
            {
                uint32 spellCount = 0;
                for (auto const &kvPair : referencePlayer->GetSpellMap())
                {
                    SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(kvPair.first);
                    for (auto skillIter = bounds.first; skillIter != bounds.second; ++skillIter)
                    {
                        if (skillIter->second->skillId == achievementCriteria->learn_skillline_spell.skillLine)
                            ++spellCount;
                    }
                }
                SetCriteriaProgress(achievementCriteria, spellCount, referencePlayer);
                break;
            }
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
                SetCriteriaProgress(achievementCriteria, referencePlayer->GetReputationMgr().GetReveredFactionCount(), referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
                SetCriteriaProgress(achievementCriteria, referencePlayer->GetReputationMgr().GetHonoredFactionCount(), referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
                SetCriteriaProgress(achievementCriteria, referencePlayer->GetReputationMgr().GetVisibleFactionCount(), referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE:
            {
                uint32 spellCount = 0;
                for (auto const &kvPair : referencePlayer->GetSpellMap())
                {
                    SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(kvPair.first);
                    for (auto skillIter = bounds.first; skillIter != bounds.second; ++skillIter)
                        if (skillIter->second->skillId == achievementCriteria->learn_skill_line.skillLine)
                            ++spellCount;
                }
                SetCriteriaProgress(achievementCriteria, spellCount, referencePlayer);
                break;
            }
            case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
                SetCriteriaProgress(achievementCriteria, referencePlayer->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS), referencePlayer);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
                SetCriteriaProgress(achievementCriteria, referencePlayer->GetMoney(), referencePlayer, PROGRESS_HIGHEST);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
                if (!miscValue1)
                    SetCriteriaProgress(achievementCriteria, _achievementPoints, referencePlayer, PROGRESS_SET);
                else
                    SetCriteriaProgress(achievementCriteria, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING:
                {
                    uint32 reqTeamType = achievementCriteria->highest_team_rating.teamtype;

                    if (miscValue1)
                    {
                        if (miscValue2 != reqTeamType)
                            continue;
                        SetCriteriaProgress(achievementCriteria, miscValue1, referencePlayer, PROGRESS_HIGHEST);
                    }
                    else // Login case
                    {
                        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                            SetCriteriaProgress(achievementCriteria, referencePlayer->GetArenaPersonalRating(arena_slot), referencePlayer, PROGRESS_HIGHEST);
                    }
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
            {
                uint32 reqTeamType = achievementCriteria->highest_personal_rating.teamtype;

                if (miscValue1)
                {
                    if (miscValue2 != reqTeamType)
                        continue;

                    SetCriteriaProgress(achievementCriteria, miscValue1, referencePlayer, PROGRESS_HIGHEST);
                }

                else // Login case
                {
                    for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                        SetCriteriaProgress(achievementCriteria, referencePlayer->GetArenaPersonalRating(arena_slot), referencePlayer, PROGRESS_HIGHEST);
                }

                break;
            }
            case ACHIEVEMENT_CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
            {
                if (!miscValue1)
                    continue;

                SetCriteriaProgress(achievementCriteria, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            }
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL:
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING:
            {
                if (!miscValue1)
                    continue;

                SetCriteriaProgress(achievementCriteria, miscValue1, referencePlayer);
                break;
            }
            case ACHIEVEMENT_CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
            {
                if (!miscValue1 || !miscValue2)
                    continue;

                SetCriteriaProgress(achievementCriteria, miscValue2, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            }
            // FIXME: not triggered in code as result, need to implement
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID:
            case ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA:
            case ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK:
            case ACHIEVEMENT_CRITERIA_TYPE_EARNED_PVP_TITLE:
            case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
                break;                                   // Not implemented yet :(
            default:
                break;
        }

        std::list<uint32> achivEntries;
        AchievementCriteriaTreeList achievementCriteriaTreeList = sAchievementMgr->GetAchievementCriteriaTreeList(achievementCriteria);
        for (AchievementCriteriaTreeList::const_iterator iter = achievementCriteriaTreeList.begin(); iter != achievementCriteriaTreeList.end(); iter++)
        {
            AchievementEntry const* achievement = sAchievementMgr->GetAchievementEntryByCriteriaTree(*iter);
            if (!achievement)
                continue;

            if ((IsGuild<T>() && !(achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                || (achievement->flags & ACHIEVEMENT_FLAG_GUILD && !IsGuild<T>()))
            {
                TC_LOG_DEBUG("achievement", "CanUpdateCriteria: (Id: %u Type %s Achievement %u) Non-guild achievement",
                    achievementCriteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(achievementCriteria->type), achievement->ID, achievement->parentAchievement);
                continue;
            }

            if (achievement->mapID != -1 && referencePlayer->GetMapId() != uint32(achievement->mapID))
            {
                if (type == ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS)
                {
                    TC_LOG_DEBUG("achievement",  "CanUpdateCriteria: (Id: %u Type %s Achievement %u) Wrong map",
                        achievementCriteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(achievementCriteria->type), achievement->ID);
                    continue;
                }
            }
            if ((achievement->requiredFaction == ACHIEVEMENT_FACTION_HORDE    && referencePlayer->GetTeam() != HORDE) ||
                (achievement->requiredFaction == ACHIEVEMENT_FACTION_ALLIANCE && referencePlayer->GetTeam() != ALLIANCE))
            {
                TC_LOG_DEBUG("achievement", "CanUpdateCriteria: (Id: %u Type %s Achievement %u) Wrong faction",
                    achievementCriteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(achievementCriteria->type), achievement->ID);
                continue;
            }

            achivEntries.push_back(achievement->ID);
        }

        achivEntries.sort();
        for (auto x : achivEntries)
        {
            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(x);
            if (!achievement)
                continue;

            if (referencePlayer)
            {
                if (achievement->parentAchievement != 0 &&
                    ((!(achievement->flags & ACHIEVEMENT_FLAG_ACCOUNT) && !HasAchieved(achievement->parentAchievement)) 
                    || (achievement->flags & ACHIEVEMENT_FLAG_ACCOUNT && !HasAccountAchieved(achievement->parentAchievement))))
                {
                    TC_LOG_DEBUG("achievement",  "CanUpdateCriteria: (Id: %u Type %s Achievement %u) Not completed parent achievement %u (Guild: %u)",
                        achievementCriteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(achievementCriteria->type), achievement->ID, achievement->parentAchievement, IsGuild<T>() ? 1 : 0);
                    continue;
                }
            }

            if (IsCompletedCriteriaForAchievement(achievementCriteria, achievement))
                CompletedCriteriaFor(achievement, referencePlayer);

            // check again the completeness for SUMM and REQ COUNT achievements,
            // as they don't depend on the completed criteria but on the sum of the progress of each individual criteria
            if (achievement->flags & ACHIEVEMENT_FLAG_SUMM)
                if (IsCompletedAchievement(achievement))
                        CompletedAchievement(achievement, referencePlayer);

            if (AchievementEntryList const* achRefList = sAchievementMgr->GetAchievementByReferencedId(achievement->ID))
                for (AchievementEntryList::const_iterator itr = achRefList->begin(); itr != achRefList->end(); ++itr)
                    if (IsCompletedAchievement(*itr))
                        CompletedAchievement(*itr, referencePlayer);
        }
    }
}

template<class T>
bool AchievementMgr<T>::CanCompleteCriteria(CriteriaEntry const* /*achievementCriteria*/, AchievementEntry const* /*achievement*/)
{
    return true;
}

template<>
bool AchievementMgr<Player>::CanCompleteCriteria(CriteriaEntry const* /*achievementCriteria*/, AchievementEntry const* achievement)
{
    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
    {
        // Someone on this realm has already completed that achievement
        if (sAchievementMgr->IsRealmCompleted(achievement))
            return false;

        if (GetOwner())
            if (GetOwner()->GetSession())
                if (GetOwner()->GetSession()->GetSecurity())
                    return false;
    }

    return true;
}

template<class T>
bool AchievementMgr<T>::IsCompletedCriteria(CriteriaEntry const* criteria)
{
    AchievementCriteriaTreeList const& criteriaTreeList = sAchievementMgr->GetAchievementCriteriaTreeList(criteria);
    for (AchievementCriteriaTreeList::const_iterator iter = criteriaTreeList.begin(); iter != criteriaTreeList.end(); iter++)
    {
        CriteriaTreeEntry const* criteriaTree = *iter;
        AchievementEntry const* achievement = sAchievementMgr->GetAchievementEntryByCriteriaTree(criteriaTree);

        if (!achievement)
            return false;

        if (CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(criteriaTree->criteriaID))
            if (!IsCompletedCriteriaForAchievement(criteria, achievement))
                return false;
    }

    return true;
}

template<class T>
bool AchievementMgr<T>::IsCompletedCriteriaForAchievement(CriteriaEntry const* criteria, AchievementEntry const* achievement)
{
    if (!criteria || !achievement)
        return false;

    CriteriaTreeEntry const* criteriaTree = NULL;
    AchievementCriteriaTreeList const* treeList = sAchievementMgr->GetSubCriteriaTreeById(achievement->criteriaTreeID);
    if (treeList)
    {
        for (AchievementCriteriaTreeList::const_iterator iter = treeList->begin(); iter != treeList->end(); iter++)
        {
            if ((*iter)->criteriaID == criteria->ID)
            {
                criteriaTree = *iter;
                break;
            }
        }
    }
    if (!criteriaTree)
        return false;

    // counter can never complete
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
        return false;

    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
    {
        // someone on this realm has already completed that achievement
        if (sAchievementMgr->IsRealmCompleted(achievement))
            return false;
    }

    CriteriaProgress const* progress = GetCriteriaProgress(criteria);
    if (!progress)
        return false;

    switch (AchievementCriteriaTypes(criteria->type))
    {
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
        case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE:
        case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
        case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE:
        case ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
        case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
        case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
        case ACHIEVEMENT_CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
            return progress->counter >= criteriaTree->criteriaCount;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_TABARD:
            return progress->counter >= 1;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            return progress->counter >= (criteriaTree->criteriaCount * 75);
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
            return progress->counter >= 9000;
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
            return progress->counter >= 2000;
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA:
            return criteriaTree->criteriaCount && progress->counter >= criteriaTree->criteriaCount;
        // handle all statistic-only criteria here
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING:
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
        case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
        case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
        case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
        case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
        default:
            return false;
    }
    return false;
}

template<class T>
void AchievementMgr<T>::CompletedCriteriaFor(AchievementEntry const* achievement, Player* referencePlayer)
{
    // Counter can never complete
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
        return;

    // Already completed and stored
    if (HasAchieved(achievement->ID))
        return;

    if (IsCompletedAchievement(achievement))
        CompletedAchievement(achievement, referencePlayer);
}

template<class T>
bool AchievementMgr<T>::IsCompletedAchievement(AchievementEntry const* entry)
{
    // Counter can never complete
    if (entry->flags & ACHIEVEMENT_FLAG_COUNTER)
        return false;

    // For achievement with referenced achievement criterias get from referenced and counter from self
    uint32 achievementForTestId = entry->refAchievement ? entry->refAchievement : entry->ID;
    uint32 achievementForTestCount = entry->count;

    if (CriteriaTreeEntry const* criteriaTree = sCriteriaTreeStore.LookupEntry(entry->criteriaTreeID))
        achievementForTestCount = criteriaTree->criteriaCount;

    AchievementCriteriaEntryList cList;
    AchievementCriteriaTreeList const* list = sAchievementMgr->GetSubCriteriaTreeById(entry->criteriaTreeID);
    for (AchievementCriteriaTreeList::const_iterator iter = list->begin(); iter != list->end(); iter++)
    {
        CriteriaTreeEntry const* criteriaTree = *iter;
        if (CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(criteriaTree->criteriaID))
            cList.push_back(criteria);
    }

    if (!cList.size())
        return false;

    uint64 count = 0;

    // For SUMM achievements, we have to count the progress of each criteria of the achievement.
    // Oddly, the target count is NOT contained in the achievement, but in each individual criteria
    if (entry->flags & ACHIEVEMENT_FLAG_SUMM)
    {
        for (AchievementCriteriaEntryList::const_iterator itr = cList.begin(); itr != cList.end(); ++itr)
        {
            CriteriaEntry const* criteria = *itr;

            CriteriaProgress const* progress = GetCriteriaProgress(criteria);
            if (!progress)
                continue;

            count += progress->counter;

            // For counters, field4 contains the main count requirement
            if (count >= sCriteriaTreeStore.LookupEntry(entry->criteriaTreeID)->criteriaCount)
                return true;
        }
        return false;
    }

    // Default case - need complete all or
    bool completed_all = true;
    for (AchievementCriteriaEntryList::const_iterator itr = cList.begin(); itr != cList.end(); ++itr)
    {
        CriteriaEntry const* criteria = *itr;

        bool completed = IsCompletedCriteriaForAchievement(criteria, entry);

        // Found an uncompleted criteria, but DONT return false yet - there might be a completed criteria with ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_ALL
        if (completed)
            ++count;
        else
            completed_all = false;

        // Completed as have req. count of completed criterias
        if (achievementForTestCount > 0 && achievementForTestCount <= count)
            return true;
    }

    // All criterias completed requirement
    if (completed_all && achievementForTestCount == 0)
        return true;

    return false;
}

template<class T>
CriteriaProgress* AchievementMgr<T>::GetCriteriaProgress(uint32 criteriaId)
{
    CriteriaProgressMap::iterator iter = m_criteriaProgress.find(criteriaId);
    if (iter == m_criteriaProgress.end())
        return NULL;

    return &(iter->second);
}

template<class T>
void AchievementMgr<T>::SetCriteriaProgress(CriteriaEntry const* entry, uint64 changeValue, Player* referencePlayer, ProgressType ptype)
{
    // Don't allow to cheat - doing timed achievements without timer active
    TimedAchievementMap::iterator timedIter = m_timedAchievements.find(entry->ID);
    if (entry->timeLimit && timedIter == m_timedAchievements.end())
        return;

    TC_LOG_DEBUG("achievement", "SetCriteriaProgress(%u, " UI64FMTD ") for (%s GUID: %u)",
        entry->ID, changeValue, GetLogNameForGuid(GetOwner()->GetGUID()), GUID_LOPART(GetOwner()->GetGUID()));

    CriteriaProgress* progress = GetCriteriaProgress(entry);
    if (!progress)
    {
        // Not create record for 0 counter but allow it for timed achievements
        // We will need to send 0 progress to client to start the timer
        if (changeValue == 0 && !entry->timeLimit)
            return;

        progress = &m_criteriaProgress[entry->ID];
        progress->counter = changeValue;
    }
    else
    {
        uint64 newValue = 0;
        switch (ptype)
        {
            case PROGRESS_SET:
                newValue = changeValue;
                break;
            case PROGRESS_ACCUMULATE:
            {
                // Avoid overflow
                uint64 max_value = std::numeric_limits<uint64>::max();
                newValue = max_value - progress->counter > changeValue ? progress->counter + changeValue : max_value;
                break;
            }
            case PROGRESS_HIGHEST:
                newValue = progress->counter < changeValue ? changeValue : progress->counter;
                break;
        }

        // Not update (not mark as changed) if counter will have same value
        if (progress->counter == newValue && !entry->timeLimit && (!IsGuild<T>() || !IsCompletedCriteria(entry) || progress->CompletedGUID))
            return;

        progress->counter = newValue;
    }

    progress->changed = true;
    progress->date = time(NULL); // Set the date to the latest update.
    uint32 timeElapsed = 0;

    AchievementCriteriaTreeList criteriaList = sAchievementMgr->GetAchievementCriteriaTreeList(entry);
    for (AchievementCriteriaTreeList::const_iterator iter = criteriaList.begin(); iter != criteriaList.end(); iter++)
    {
        AchievementEntry const* achievement = sAchievementMgr->GetAchievementEntryByCriteriaTree(*iter);
        if (!achievement)
            continue;

        bool criteriaComplete = IsCompletedCriteriaForAchievement(entry, achievement);
        if (entry->timeLimit)
        {
            // Client expects this in packet
            timeElapsed = entry->timeLimit - (timedIter->second / IN_MILLISECONDS);

            // Remove the timer, we wont need it anymore
            if (criteriaComplete)
                m_timedAchievements.erase(timedIter);
        }

        if (criteriaComplete && achievement->flags & ACHIEVEMENT_FLAG_SHOW_CRITERIA_MEMBERS && !progress->CompletedGUID)
            progress->CompletedGUID = referencePlayer->GetGUID();

        if (achievement->flags & ACHIEVEMENT_FLAG_ACCOUNT)
        {
            WorldPacket data(SMSG_ACCOUNT_CRITERIA_UPDATE, 4 + 4 + 4 + 4 + 8 + 4);
            ObjectGuid guid = GetOwner()->GetGUID();
            ObjectGuid counter = progress->counter;

            data.WriteBitSeq<5>(counter);

            data.WriteBits(0, 4);

            data.WriteBitSeq<3, 1>(counter);
            data.WriteBitSeq<0, 1>(guid);
            data.WriteBitSeq<4, 2>(counter);
            data.WriteBitSeq<3, 7, 4>(guid);
            data.WriteBitSeq<6>(counter);
            data.WriteBitSeq<6, 5>(guid);
            data.WriteBitSeq<0>(counter);
            data.WriteBitSeq<2>(guid);
            data.WriteBitSeq<7>(counter);

            data.FlushBits();

            data.WriteByteSeq<5, 6>(guid);
            data.WriteByteSeq<6>(counter);
            data.WriteByteSeq<1>(guid);

            data << uint32(timeElapsed);

            data.WriteByteSeq<2>(counter);

            data << uint32(entry->ID);

            data.WriteByteSeq<2, 0>(guid);
            data.WriteByteSeq<0, 5>(counter);

            data.AppendPackedTime(progress->date);

            data.WriteByteSeq<4>(counter);
            data.WriteByteSeq<3, 7>(guid);

            data << uint32(timeElapsed);

            data.WriteByteSeq<4>(guid);
            data.WriteByteSeq<7, 3, 1>(counter);

            SendPacket(&data);
            return;
        }
    }

    SendCriteriaUpdate(entry, progress, timeElapsed, false);
}

template<class T>
void AchievementMgr<T>::UpdateTimedAchievements(uint32 timeDiff)
{
    if (!m_timedAchievements.empty())
    {
        for (TimedAchievementMap::iterator itr = m_timedAchievements.begin(); itr != m_timedAchievements.end();)
        {
            // Time is up, remove timer and reset progress
            if (itr->second <= timeDiff)
            {
                CriteriaEntry const* entry = sAchievementMgr->GetAchievementCriteria(itr->first);
                RemoveCriteriaProgress(entry);
                m_timedAchievements.erase(itr++);
            }
            else
            {
                itr->second -= timeDiff;
                ++itr;
            }
        }
    }
}

template<class T>
void AchievementMgr<T>::StartTimedAchievement(AchievementCriteriaTimedTypes /*type*/, uint32 /*entry*/, uint32 /*timeLost = 0*/)
{
}

template<>
void AchievementMgr<Player>::StartTimedAchievement(AchievementCriteriaTimedTypes type, uint32 entry, uint32 timeLost /* = 0 */)
{
    AchievementCriteriaEntryList const& achievementCriteriaList = sAchievementMgr->GetTimedAchievementCriteriaByType(type);
    for (AchievementCriteriaEntryList::const_iterator i = achievementCriteriaList.begin(); i != achievementCriteriaList.end(); ++i)
    {
        if ((*i)->timedCriteriaMiscId != entry)
            continue;

        AchievementEntry const* achievement = sAchievementMgr->GetAchievement(entry);
        if (m_timedAchievements.find((*i)->ID) == m_timedAchievements.end() && !IsCompletedCriteriaForAchievement(*i, achievement))
        {
            // Start the timer
            if ((*i)->timeLimit * IN_MILLISECONDS > timeLost)
            {
                m_timedAchievements[(*i)->ID] = (*i)->timeLimit * IN_MILLISECONDS - timeLost;

                // And at client too
                SetCriteriaProgress(*i, 0, GetOwner(), PROGRESS_SET);
            }
        }
    }
}

template<class T>
void AchievementMgr<T>::RemoveTimedAchievement(AchievementCriteriaTimedTypes type, uint32 entry)
{
    AchievementCriteriaEntryList const& achievementCriteriaList = sAchievementMgr->GetTimedAchievementCriteriaByType(type);
    for (AchievementCriteriaEntryList::const_iterator i = achievementCriteriaList.begin(); i != achievementCriteriaList.end(); ++i)
    {
        if ((*i)->timedCriteriaMiscId != entry)
            continue;

        TimedAchievementMap::iterator timedIter = m_timedAchievements.find((*i)->ID);
        // We don't have timer for this achievement
        if (timedIter == m_timedAchievements.end())
            continue;

        // Remove progress
        RemoveCriteriaProgress(*i);

        // Remove the timer
        m_timedAchievements.erase(timedIter);
    }
}

template<class T>
void AchievementMgr<T>::CompletedAchievement(AchievementEntry const* achievement, Player* referencePlayer)
{
    // Disable for gamemasters with GM-mode enabled
    if (GetOwner()->isGameMaster())
        return;

    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER || HasAchieved(achievement->ID))
        return;

    /*if (achievement->flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_NEWS)
        if (Guild* guild = sGuildMgr->GetGuildById(referencePlayer->GetGuildId()))
            guild->GetNewsLog().AddNewEvent(GUILD_NEWS_PLAYER_ACHIEVEMENT, time(NULL), referencePlayer->GetGUID(), achievement->flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_HEADER, achievement->ID);*/

    if (HasAccountAchieved(achievement->ID))
    {
        CompletedAchievementData& ca = m_completedAchievements[achievement->ID];
        ca.completedByThisCharacter = true;
        return;
    }

    if (!GetOwner()->GetSession()->PlayerLoading())
        SendAchievementEarned(achievement);

    CompletedAchievementData& ca = m_completedAchievements[achievement->ID];
    ca.completedByThisCharacter = true;
    ca.date = time(NULL);
    ca.first_guid = GetOwner()->GetGUIDLow();
    ca.changed = true;

    // Don't insert for ACHIEVEMENT_FLAG_REALM_FIRST_KILL since otherwise only the first group member would reach that achievement
    // @TODO: where do set this instead?
    if (!(achievement->flags & ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
        sAchievementMgr->SetRealmCompleted(achievement);

    _achievementPoints += achievement->points;

    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT, 0, 0, 0, NULL, referencePlayer);
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS, achievement->points, 0, 0, NULL, referencePlayer);

    // Reward items and titles if any
    AchievementReward const* reward = sAchievementMgr->GetAchievementReward(achievement);

    // No rewards
    if (!reward)
        return;

    // Titles
    //! Currently there's only one achievement that deals with gender-specific titles.
    //! Since no common attributes were found, (not even in titleRewardFlags field)
    //! we explicitly check by ID. Maybe in the future we could move the achievement_reward
    //! condition fields to the condition system.
    if (uint32 titleId = reward->titleId[achievement->ID == 1793 ? GetOwner()->getGender() : (GetOwner()->GetTeam() == ALLIANCE ? 0 : 1)])
        if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId))
            GetOwner()->SetTitle(titleEntry);

    // Mail
    if (reward->sender)
    {
        Item* item = reward->itemId ? Item::CreateItem(reward->itemId, 1, GetOwner()) : NULL;

        int loc_idx = GetOwner()->GetSession()->GetSessionDbLocaleIndex();

        // Subject and text
        std::string subject = reward->subject;
        std::string text = reward->text;
        if (loc_idx >= 0)
        {
            if (AchievementRewardLocale const* loc = sAchievementMgr->GetAchievementRewardLocale(achievement))
            {
                ObjectMgr::GetLocaleString(loc->subject, loc_idx, subject);
                ObjectMgr::GetLocaleString(loc->text, loc_idx, text);
            }
        }

        MailDraft draft(subject, text);

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        if (item)
        {
            item->SetOwnerGUID(0);
            // Save new item before send
            item->SaveToDB(trans);                               // Save for prevent lost at next mail load, if send fail then item will deleted

            // Item
            draft.AddItem(item);
        }

        draft.SendMailTo(trans, GetOwner(), MailSender(MAIL_CREATURE, reward->sender));
        CharacterDatabase.CommitTransaction(trans);
    }
}

template<>
void AchievementMgr<Guild>::CompletedAchievement(AchievementEntry const* achievement, Player* referencePlayer)
{
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER || HasAchieved(achievement->ID))
        return;

    /*if (achievement->flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_NEWS)
        if (Guild* guild = sGuildMgr->GetGuildById(referencePlayer->GetGuildId()))
            guild->GetNewsLog().AddNewEvent(GUILD_NEWS_GUILD_ACHIEVEMENT, time(NULL), 0, achievement->flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_HEADER, achievement->ID);*/

    SendAchievementEarned(achievement);
    CompletedAchievementData& ca = m_completedAchievements[achievement->ID];
    ca.date = time(NULL);
    ca.changed = true;

    if (achievement->flags & ACHIEVEMENT_FLAG_SHOW_GUILD_MEMBERS)
    {
        if (referencePlayer->GetGuildId() == GetOwner()->GetId())
            ca.guids.insert(referencePlayer->GetGUID());

        if (Group const* group = referencePlayer->GetGroup())
            for (GroupReference const* ref = group->GetFirstMember(); ref != NULL; ref = ref->next())
                if (Player const* groupMember = ref->GetSource())
                    if (groupMember->GetGuildId() == GetOwner()->GetId())
                        ca.guids.insert(groupMember->GetGUID());
    }

    sAchievementMgr->SetRealmCompleted(achievement);

    _achievementPoints += achievement->points;

    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT, 0, 0, 0, NULL, referencePlayer);
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS, achievement->points, 0, 0, NULL, referencePlayer);
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS, achievement->points, 0, 0, NULL, referencePlayer);
}

struct VisibleAchievementPred final
{
    bool operator()(CompletedAchievementMap::value_type const& val) const
    {
        AchievementEntry const* achievement = sAchievementMgr->GetAchievement(val.first);
        return achievement && !(achievement->flags & ACHIEVEMENT_FLAG_HIDDEN);
    }
};

template<class T>
void AchievementMgr<T>::SendAllAchievementData(Player* /*receiver*/)
{
    CriteriaProgressMap const &progressMap = GetCriteriaProgressMap();

    VisibleAchievementPred isVisible;
    size_t numCriteria = progressMap.size();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);
    ObjectGuid guid = GetOwner()->GetGUID();
    ObjectGuid counter;

    WorldPacket data(SMSG_ALL_ACHIEVEMENT_DATA);

    data.WriteBits(numAchievements, 20);

    for (auto const &kvPair : m_completedAchievements)
    {
        if (!isVisible(kvPair))
            continue;

        ObjectGuid const firstAccountGuid = kvPair.second.first_guid;
        data.WriteBitSeq<6, 1, 5, 3, 2, 7, 0, 4>(firstAccountGuid);
    }

    data.WriteBits(numCriteria, 19);

    for (auto const &kvPair : progressMap)
    {
        counter = uint64(kvPair.second.counter);

        data.WriteBitSeq<5>(counter);
        data.WriteBitSeq<2, 4>(guid);
        data.WriteBitSeq<1, 7>(counter);
        data.WriteBitSeq<0, 1>(guid);
        data.WriteBitSeq<3>(counter);
        data.WriteBitSeq<5>(guid);
        data.WriteBits(0, 4);
        data.WriteBitSeq<0, 6, 4>(counter);
        data.WriteBitSeq<7, 3, 6>(guid);
        data.WriteBitSeq<2>(counter);
    }

    data.FlushBits();

    for (auto const &kvPair : progressMap)
    {
        counter = uint64(kvPair.second.counter);

        data.WriteByteSeq<1>(guid);
        data << uint32(kvPair.first);   // Criteria id
        data.WriteByteSeq<7>(guid);
        data.WriteByteSeq<5>(counter);
        data << uint32(0);              // Unk value from 5.4.0, always 0
        data.WriteByteSeq<6>(guid);
        data.WriteByteSeq<0>(counter);
        data << uint32(227545947);      // Unk value from 5.4.0, always 227545947
        data.WriteByteSeq<2>(guid);
        data.WriteByteSeq<2, 1>(counter);
        data << uint32(0);              // Unk value from 5.4.0, always 0
        data.WriteByteSeq<3, 5>(guid);
        data.WriteByteSeq<3>(counter);
        data.WriteByteSeq<4>(guid);
        data.WriteByteSeq<4, 7, 6>(counter);
        data.WriteByteSeq<0>(guid);
    }

    for (auto const &kvPair : m_completedAchievements)
    {
        if (!isVisible(kvPair))
            continue;

        ObjectGuid firstAccountGuid = kvPair.second.first_guid;

        data.WriteByteSeq<4, 6, 3, 0>(firstAccountGuid);
        data << uint32(realmID);                                   // Unk timer from 5.4.0 17399, sometimes 50724907
        data.WriteByteSeq<1, 2>(firstAccountGuid);
        data << uint32(secsToTimeBitFields(kvPair.second.date));
        data.WriteByteSeq<7, 5>(firstAccountGuid);
        data << uint32(realmID);                                   // Unk timer from 5.4.0 17399, sometimes 50724907
        data << uint32(kvPair.first);
    }
    
    SendPacket(&data);
    
    WorldPacket data2(SMSG_ACCOUNT_CRITERIA_UPDATE_ALL);
    ObjectGuid acc = GetOwner()->GetSession()->GetAccountId();
    data2.WriteBits(numCriteria, 19);

    for (auto const &kvPair : progressMap)
    {
        counter = uint64(kvPair.second.counter);

        data2.WriteBitSeq<6, 3>(counter);
        data2.WriteBitSeq<6, 1, 2, 3>(acc);
        data2.WriteBits(0, 4);
        data2.WriteBitSeq<5, 0, 7>(acc);
        data2.WriteBitSeq<0, 7, 5>(counter);
        data2.WriteBitSeq<4>(acc);
        data2.WriteBitSeq<1, 4, 2>(counter);
    }

    data2.FlushBits();

    for (auto const &kvPair : progressMap)
    {
        counter = uint64(kvPair.second.counter);

        data2.WriteByteSeq<1>(acc);

        data2 << uint32(0);

        data2.WriteByteSeq<2>(acc);
        data2.WriteByteSeq<6>(counter);
        data2.WriteByteSeq<4>(acc);
        data2.WriteByteSeq<7>(counter);
        data2.WriteByteSeq<6, 7>(acc);

        data2 << uint32(0);
        
        data2.WriteByteSeq<5>(counter);
        data2.WriteByteSeq<5>(acc);
        data2.WriteByteSeq<2>(counter);

        data2 << uint32(227545947);
        data2 << uint32(kvPair.first);

        data2.WriteByteSeq<0>(counter);
        data2.WriteByteSeq<3>(acc);
        data2.WriteByteSeq<1>(counter);
        data2.WriteByteSeq<0>(acc);
        data2.WriteByteSeq<4, 3>(counter);
    }

    SendPacket(&data2);
}

template<>
void AchievementMgr<Guild>::SendAllAchievementData(Player* receiver)
{
    WorldPacket data(SMSG_GUILD_ACHIEVEMENT_DATA, m_completedAchievements.size() * (4 + 4) + 3);

    data.WriteBits(m_completedAchievements.size(), 20);

    ObjectGuid achievementee;

    for (CompletedAchievementMap::const_iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
    }

    data.FlushBits();

    for (CompletedAchievementMap::const_iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        data.WriteByte(0);

        data << uint32(itr->first);
        data << uint32(0); //0 or time ?

        data.WriteByte(0);
        data.WriteByte(0);
        data.WriteByte(0);
        data.WriteByte(0);

        data << uint32(0);

        data.WriteByte(0);
        data.WriteByte(0);

        data.AppendPackedTime(itr->second.date);

        data.WriteByte(0);
    }

    receiver->GetSession()->SendPacket(&data);
}

template<>
void AchievementMgr<Player>::SendAchievementInfo(Player* receiver, uint32 /*achievementId = 0 */)
{
    CriteriaProgressMap const &progressMap = GetCriteriaProgressMap();

    ObjectGuid guid = GetOwner()->GetGUID();
    ObjectGuid counter;

    VisibleAchievementPred isVisible;
    size_t numCriteria = progressMap.size();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);

    WorldPacket data(SMSG_RESPOND_INSPECT_ACHIEVEMENTS);

    data.WriteBits(numCriteria, 19);
    data.WriteBits(numAchievements, 20);

    for (auto const &kvPair : progressMap)
    {
        counter = kvPair.second.counter;

        data.WriteBitSeq<4>(guid);
        data.WriteBitSeq<2>(counter);
        data.WriteBitSeq<0>(guid);
        data.WriteBitSeq<4, 5, 1, 0>(counter);
        data.WriteBitSeq<5>(guid);
        data.WriteBitSeq<6, 3>(counter);
        data.WriteBits(1, 4);           // Criteria progress flags
        data.WriteBitSeq<1, 7, 2, 3>(guid);
        data.WriteBitSeq<7>(counter);
        data.WriteBitSeq<6>(guid);
    }

    data.WriteBitSeq<6, 2>(guid);

    for (auto const &kvPair : m_completedAchievements)
    {
        if (!isVisible(kvPair))
            continue;

        ObjectGuid const firstGuid = kvPair.second.first_guid;
        data.WriteBitSeq<2, 3, 7, 5, 4, 6, 0, 1>(firstGuid);
    }

    data.WriteBitSeq<4, 7, 3, 0, 5, 1>(guid);
    data.FlushBits();

    for (auto const &kvPair : progressMap)
    {
        counter = kvPair.second.counter;

        data.WriteByteSeq<5>(counter);
        data.WriteByteSeq<0, 1>(guid);
        data.WriteByteSeq<6>(counter);
        data.WriteByteSeq<4>(guid);
        data.WriteByteSeq<7, 0, 1>(counter);
        data << uint32(0);
        data.WriteByteSeq<4, 3>(counter);
        data << uint32(kvPair.first);     // criteria id
        data << uint32(secsToTimeBitFields(kvPair.second.date));
        data << uint32(227545947);
        data.WriteByteSeq<2, 5, 6>(guid);
        data.WriteByteSeq<2>(counter);
        data.WriteByteSeq<7, 3>(guid);
    }

    data.WriteByteSeq<5, 3>(guid);

    for (auto const &kvPair : m_completedAchievements)
    {
        if (!isVisible(kvPair))
            continue;

        ObjectGuid const firstGuid = kvPair.second.first_guid;

        data.WriteByteSeq<0, 7, 6>(firstGuid);
        data << uint32(0);
        data << uint32(kvPair.first);
        data << uint32(50397223);
        data << uint32(secsToTimeBitFields(kvPair.second.date));
        data.WriteByteSeq<4, 5, 1, 3, 2>(firstGuid);
    }

    data.WriteByteSeq<6, 7, 2, 1, 4, 0>(guid);

    receiver->GetSession()->SendPacket(&data);
}

template<>
void AchievementMgr<Guild>::SendAchievementInfo(Player* receiver, uint32 achievementId /*= 0*/)
{
    // Will send response to criteria progress request
    AchievementCriteriaEntryList const* criteria = sAchievementMgr->GetAchievementCriteriaByAchievement(achievementId);
    if (!criteria)
    {
        // Send empty packet
        WorldPacket data(SMSG_GUILD_CRITERIA_DATA, 3);
        data.WriteBits(0, 19);
        data.FlushBits();
        receiver->SendDirectMessage(&data);
        return;
    }

    ObjectGuid counter, guid;
    ByteBuffer criteriaData;

    CriteriaProgressMap const &progressMap = GetCriteriaProgressMap();

    WorldPacket data(SMSG_GUILD_CRITERIA_DATA, 3 + (2 + 8 + 5 * 4) * progressMap.size());

    data.WriteBits(progressMap.size(), 19);

    for (auto const &kvPair : progressMap)
    {
        auto const &criteriaId = kvPair.first;
        auto const &progressData = kvPair.second;

        counter = progressData.counter;
        guid = progressData.CompletedGUID;

        data.WriteBitSeq<3, 6>(counter);
        data.WriteBitSeq<5, 4>(guid);
        data.WriteBitSeq<2>(counter);
        data.WriteBitSeq<0>(guid);
        data.WriteBitSeq<7>(counter);
        data.WriteBitSeq<6, 7, 3>(guid);
        data.WriteBitSeq<5, 0>(counter);
        data.WriteBitSeq<2>(guid);
        data.WriteBitSeq<1, 4>(counter);
        data.WriteBitSeq<1>(guid);

        criteriaData.WriteByteSeq<4>(guid);
        criteriaData << uint32(progressData.date);      // Unknown date
        criteriaData.WriteByteSeq<2, 0>(counter);
        criteriaData << uint32(progressData.changed);
        criteriaData.WriteByteSeq<7, 1>(guid);
        criteriaData << uint32(criteriaId);
        criteriaData << uint32(progressData.date);      // Last update time (not packed!)
        criteriaData.WriteByteSeq<6>(guid);
        criteriaData.WriteByteSeq<1>(counter);
        criteriaData.WriteByteSeq<3>(guid);
        criteriaData.WriteByteSeq<4>(counter);
        criteriaData << uint32(progressData.date);      // Unknown date
        criteriaData.WriteByteSeq<5, 3>(counter);
        criteriaData.WriteByteSeq<0>(guid);
        criteriaData.WriteByteSeq<6, 7>(counter);
        criteriaData.WriteByteSeq<2, 5>(guid);
    }

    data.FlushBits();
    data.append(criteriaData);

    receiver->SendDirectMessage(&data);
}

template<class T>
bool AchievementMgr<T>::HasAchieved(uint32 achievementId) const
{
    return m_completedAchievements.find(achievementId) != m_completedAchievements.end();
}

template<>
bool AchievementMgr<Player>::HasAchieved(uint32 achievementId) const
{
    CompletedAchievementMap::const_iterator itr = m_completedAchievements.find(achievementId);

    if (itr == m_completedAchievements.end())
        return false;

    return (*itr).second.completedByThisCharacter;
}

template<class T>
bool AchievementMgr<T>::HasAccountAchieved(uint32 achievementId) const
{
    return m_completedAchievements.find(achievementId) != m_completedAchievements.end();
}

template<class T>
uint64 AchievementMgr<T>::GetFirstAchievedCharacterOnAccount(uint32 achievementId) const
{
    CompletedAchievementMap::const_iterator itr = m_completedAchievements.find(achievementId);

    if (itr == m_completedAchievements.end())
        return 0LL;

    return (*itr).second.first_guid;
}

template<class T>
bool AchievementMgr<T>::CanUpdateCriteria(CriteriaEntry const* criteria, uint64 miscValue1, uint64 miscValue2, uint64 miscValue3, Unit const* unit, Player* referencePlayer)
{
    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_ACHIEVEMENT_CRITERIA, criteria->ID, NULL))
    {
        TC_LOG_TRACE("achievement", "CanUpdateCriteria: (Id: %u Type %s) Disabled",
            criteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(criteria->type));
        return false;
    }

    if (IsCompletedCriteria(criteria))
    {
        TC_LOG_TRACE("achievement", "CanUpdateCriteria: (Id: %u Type %s) Is Completed",
            criteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(criteria->type));
        return false;
    }

    if (!RequirementsSatisfied(criteria, miscValue1, miscValue2, miscValue3, unit, referencePlayer))
    {
        TC_LOG_TRACE("achievement", "CanUpdateCriteria: (Id: %u Type %s) Requirements not satisfied",
                     criteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(criteria->type));
        return false;
    }

    if (!AdditionalRequirementsSatisfied(criteria, miscValue1, miscValue2, unit, referencePlayer))
    {
        TC_LOG_TRACE("achievement", "CanUpdateCriteria: (Id: %u Type %s) Additional requirements not satisfied",
            criteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(criteria->type));
        return false;
    }

    if (!ConditionsSatisfied(criteria, referencePlayer))
    {
        TC_LOG_TRACE("achievement", "CanUpdateCriteria: (Id: %u Type %s) Conditions not satisfied",
            criteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(criteria->type));
        return false;
    }

    return true;
}

template<class T>
bool AchievementMgr<T>::ConditionsSatisfied(CriteriaEntry const *criteria, Player* referencePlayer) const
{
    /*
    for (uint32 i = 0; i < MAX_CRITERIA_REQUIREMENTS; ++i)
    {
        if (!criteria->additionalRequirements[i].additionalRequirement_type)
            continue;

        switch (criteria->additionalRequirements[i].additionalRequirement_type)
        {
            case ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP:
                if (referencePlayer->GetMapId() != criteria->additionalRequirements[i].additionalRequirement_value)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_CONDITION_NOT_IN_GROUP:
                if (referencePlayer->GetGroup())
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_CONDITION_SERVER_SIDE_CHECK:
                // FIXME: return criteria->scriptId != 0;
            default:
                break;
        }
    }
    */
    return true;
}

template<class T>
bool AchievementMgr<T>::RequirementsSatisfied(CriteriaEntry const *achievementCriteria, uint64 miscValue1, uint64 miscValue2, uint64 miscValue3, Unit const *unit, Player* referencePlayer) const
{
    switch (AchievementCriteriaTypes(achievementCriteria->type))
    {
        case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
        case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_DEALT:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
        case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:
        case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:
            if (!miscValue1)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_EARNED_PVP_TITLE:
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING:
        case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            if (m_completedAchievements.find(achievementCriteria->complete_achievement.linkedAchievement) == m_completedAchievements.end())
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
            if (!miscValue1 || !referencePlayer || achievementCriteria->win_bg.bgMapID != referencePlayer->GetMapId())
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            if (!miscValue1 || achievementCriteria->kill_creature.creatureID != miscValue1)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            // update at loading or specific skill update
            if (miscValue1 && miscValue1 != achievementCriteria->reach_skill_level.skillID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            // update at loading or specific skill update
            if (miscValue1 && miscValue1 != achievementCriteria->learn_skill_level.skillID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            if (miscValue1 && miscValue1 != achievementCriteria->complete_quests_in_zone.zoneID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
            if (!miscValue1 || !referencePlayer || referencePlayer->GetMapId() != achievementCriteria->complete_battleground.mapID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            if (!miscValue1 || !referencePlayer ||referencePlayer->GetMapId() != achievementCriteria->death_at_map.mapID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
        {
            if (!miscValue1)
                return false;
            /*if (!referencePlayer)
                return false;
            // skip wrong arena achievements, if not achievIdByArenaSlot then normal total death counter
            bool notfit = false;
            for (int j = 0; j < MAX_ARENA_SLOT; ++j)
            {
                if (achievIdByArenaSlot[j] == achievementCriteria->achievement)
                {
                    Battleground* bg = referencePlayer->GetBattleground();
                    if (!bg || !bg->isArena() || Arena::GetSlotByType(bg->GetArenaType()) != j)
                        notfit = true;
                    break;
                }
            }
            if (notfit)
                return false;*/
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
        {
            if (!miscValue1)
                return false;

            /*
            if (!referencePlayer)
                return false;

            Map const* map = referencePlayer->IsInWorld() ? referencePlayer->GetMap() : sMapMgr->FindMap(referencePlayer->GetMapId(), referencePlayer->GetInstanceId());
            if (!map || !map->IsDungeon())
                return false;

            // search case
            bool found = false;
            for (int j = 0; achievIdForDungeon[j][0]; ++j)
            {
                if (achievIdForDungeon[j][0] == achievementCriteria->achievement)
                {
                    if (map->IsRaid())
                    {
                        // if raid accepted (ignore difficulty)
                        if (!achievIdForDungeon[j][2])
                            break;                      // for
                    }
                    else if (referencePlayer->GetDungeonDifficulty() == REGULAR_DIFFICULTY)
                    {
                        // dungeon in normal mode accepted
                        if (!achievIdForDungeon[j][1])
                            break;                      // for
                    }
                    else
                    {
                        // dungeon in heroic mode accepted
                        if (!achievIdForDungeon[j][3])
                            break;                      // for
                    }

                    found = true;
                    break;                              // for
                }
            }
            if (!found)
                return false;

            //FIXME: work only for instances where max == min for players
            if (((InstanceMap*)map)->GetMaxPlayers() != achievementCriteria->death_in_dungeon.manLimit)
                return false;*/
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
            if (!miscValue1 || miscValue1 != achievementCriteria->killed_by_creature.creatureEntry)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
            if (!miscValue1 || !unit || unit->GetTypeId() != TYPEID_PLAYER)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
            if (!miscValue1 || miscValue2 != achievementCriteria->death_from.type)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        {
            // if miscValues != 0, it contains the questID.
            if (miscValue1)
            {
                if (miscValue1 != achievementCriteria->complete_quest.questID)
                    return false;
            }
            else
            {
                // login case.
                if (!referencePlayer || !referencePlayer->GetQuestRewardStatus(achievementCriteria->complete_quest.questID))
                    return false;
            }

            if (AchievementCriteriaDataSet const* data = sAchievementMgr->GetCriteriaDataSet(achievementCriteria))
                if (!data->Meets(referencePlayer, unit))
                    return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            if (!miscValue1 || miscValue1 != achievementCriteria->be_spell_target.spellID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
            if (!miscValue1 || miscValue1 != achievementCriteria->cast_spell.spellID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            if (miscValue1 && miscValue1 != achievementCriteria->learn_spell.spellID)
                return false;

            if (!referencePlayer || !referencePlayer->HasSpell(achievementCriteria->learn_spell.spellID))
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
            // miscValue1 = itemId - miscValue2 = count of item loot
            // miscValue3 = loot_type (note: 0 = LOOT_CORPSE and then it ignored)
            if (!miscValue1 || !miscValue2 || !miscValue3 || miscValue3 != achievementCriteria->loot_type.lootType)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            if (miscValue1 && achievementCriteria->own_item.itemID != miscValue1)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            if (!miscValue1 || achievementCriteria->use_item.itemID != miscValue1)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
            if (!miscValue1 || miscValue1 != achievementCriteria->own_item.itemID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        {
            if (!referencePlayer)
                return false;

            WorldMapOverlayEntry const* worldOverlayEntry = sWorldMapOverlayStore.LookupEntry(achievementCriteria->explore_area.areaReference);
            if (!worldOverlayEntry)
                break;

            bool matchFound = false;
            for (int j = 0; j < MAX_WORLD_MAP_OVERLAY_AREA_IDX; ++j)
            {
                uint32 area_id = worldOverlayEntry->areatableID[j];
                if (!area_id)                            // array have 0 only in empty tail
                    break;

                int32 exploreFlag = GetAreaFlagByAreaID(area_id);

                // Hack: Explore Southern Barrens
                if (achievementCriteria->explore_area.areaReference == 3009)
                    exploreFlag = 515;

                if (exploreFlag < 0)
                    continue;

                uint32 playerIndexOffset = uint32(exploreFlag) / 32;
                uint32 mask = 1 << (uint32(exploreFlag) % 32);

                if (referencePlayer->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + playerIndexOffset) & mask)
                {
                    matchFound = true;
                    break;
                }
            }

            if (!matchFound)
                return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            if (miscValue1 && miscValue1 != achievementCriteria->gain_reputation.factionID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            // miscValue1 = itemid miscValue2 = itemSlot
            if (!miscValue1 || miscValue2 != achievementCriteria->equip_epic_item.itemSlot)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        {
            // miscValue1 = itemid miscValue2 = diced value
            if (!miscValue1 || miscValue2 != achievementCriteria->roll_greed_on_loot.rollValue)
                return false;

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(uint32(miscValue1));
            if (!proto)
                return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
            if (!miscValue1 || miscValue1 != achievementCriteria->do_emote.emoteID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE:
        case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE:
            if (!miscValue1)
                return false;

            /*if (!referencePlayer)
                return false;

            if (achievementCriteria->additionalRequirements[0].additionalRequirement_type == ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP)
            {
                if (referencePlayer->GetMapId() != achievementCriteria->additionalRequirements[0].additionalRequirement_value)
                    return false;
            */
                // map specific case (BG in fact) expected player targeted damage/heal
                if (!unit || unit->GetTypeId() != TYPEID_PLAYER)
                    return false;
            //}
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
            // miscValue1 = item_id
            if (!miscValue1 || miscValue1 != achievementCriteria->equip_item.itemID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            if (!miscValue1 || miscValue1 != achievementCriteria->use_gameobject.goEntry)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
            if (!miscValue1 || miscValue1 != achievementCriteria->fish_in_gameobject.goEntry)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
            if (miscValue1 && miscValue1 != achievementCriteria->learn_skillline_spell.skillLine)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
        {
            if (!miscValue1)
                return false;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(uint32(miscValue1));
            if (!proto)
                return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE:
            if (miscValue1 && miscValue1 != achievementCriteria->learn_skill_line.skillLine)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            if (!miscValue1 || miscValue1 != achievementCriteria->hk_class.classID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            if (!miscValue1 || miscValue1 != achievementCriteria->hk_race.raceID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
            if (!miscValue1 || miscValue1 != achievementCriteria->bg_objective.objectiveId)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            if (!miscValue1 || miscValue1 != achievementCriteria->honorable_kill_at_area.areaID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY:
            if (!miscValue1 || !miscValue2 || int64(miscValue2) < 0
                || miscValue1 != achievementCriteria->currencyGain.currency)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA:
            if (miscValue1 != achievementCriteria->win_arena.mapID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_TYPE:
            //if (miscValue1 != achievementCriteria->guild_challenge_complete_type.challenge_type)
            //    return false;
            break;
        default:
            break;
    }
    return true;
}

template<class T>
bool AchievementMgr<T>::AdditionalRequirementsSatisfied(CriteriaEntry const *criteria, uint64 miscValue1, uint64 /*miscValue2*/, Unit const* unit, Player* referencePlayer) const
{
    ModifierTreeEntry const* condition = sModifierTreeStore.LookupEntry(criteria->modifierTreeId);
    if (!condition)
        return true;

    ModifierTreeEntryList const* list = sAchievementMgr->GetModifierTreeByModifierId(condition->ID);
    if (!list)
        return true;

    for (ModifierTreeEntryList::const_iterator iter = list->begin(); iter != list->end(); iter++)
    {
        ModifierTreeEntry const* tree = (*iter);
        uint32 reqType = tree->conditionType;
        uint32 reqValue = tree->conditionValue[0];

        switch (AchievementCriteriaAdditionalCondition(reqType))
        {
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_DRUNK_VALUE: // 1
            {
                if (!referencePlayer || (referencePlayer->GetDrunkValue() < reqValue))
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SERVER_SIDE_CHECK:
            {
#if 0
                if (!referencePlayer)
                    return criteria->scriptId != 0;
#endif
                switch (reqValue)
                {
                    case 924: // Alliance faction
                    case 11835: // Alliance faction
                        return referencePlayer->GetTeam() == ALLIANCE;
                    case 923: // Horde faction
                    case 11836: // Horde faction
                        return referencePlayer->GetTeam() == HORDE;
#if 0
                    default:
                        return criteria->scriptId != 0;
#endif
                }
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_LEVEL: // 3
            {
                ItemTemplate const* pItem = sObjectMgr->GetItemTemplate(miscValue1);
                if (!pItem)
                    return false;

                if (pItem->ItemLevel < reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_CREATURE_ENTRY: // 4
                if (!unit || unit->GetEntry() != reqValue)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_PLAYER: // 5
                if (!unit || !unit->IsInWorld() || unit->GetTypeId() != TYPEID_PLAYER)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_DEAD: // 6
                if (!unit || !unit->IsInWorld() || unit->IsAlive())
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_ENEMY: // 7
                if (!referencePlayer)
                    return false;
                if (!unit || !unit->IsInWorld() || !referencePlayer->IsHostileTo(unit))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_HAS_AURA: // 8
                if (!referencePlayer->HasAura(reqValue))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_HAS_AURA: // 10
                if (!unit || !unit->HasAura(reqValue))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_HAS_AURA_TYPE: // 11
                if (!unit || !unit->HasAuraType(AuraType(reqValue)))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_QUALITY_MIN: // 14
            {
                // miscValue1 is itemid
                ItemTemplate const * const item = sObjectMgr->GetItemTemplate(uint32(miscValue1));
                if (!item || item->Quality < reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_QUALITY_EQUALS: // 15
            {
                // miscValue1 is itemid
                ItemTemplate const * const item = sObjectMgr->GetItemTemplate(uint32(miscValue1));
                if (!item || item->Quality < reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_AREA_OR_ZONE: // 17
            {
                if (!referencePlayer)
                    return false;

                uint32 zoneId, areaId;
                referencePlayer->GetZoneAndAreaId(zoneId, areaId);
                if (zoneId != reqValue && areaId != reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_AREA_OR_ZONE: // 18
            {
                if (!unit)
                    return false;
                uint32 zoneId, areaId;
                unit->GetZoneAndAreaId(zoneId, areaId);
                if (zoneId != reqValue && areaId != reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_MAP_DIFFICULTY: // 20
            {
                if (!referencePlayer)
                    return false;

                if (Map* pMap = referencePlayer->GetMap())
                {
                    if (pMap->IsNonRaidDungeon()
                        || pMap->IsRaid())
                    {
                        if (pMap->GetDifficulty() < Difficulty(reqValue))
                            return false;
                    }
                    else
                        return false;
                }
                else
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_CREATURE_YIELDS_XP: // 21
                if (!unit || unit->GetTypeId() != TYPEID_UNIT || !referencePlayer || !referencePlayer->isHonorOrXPTarget(unit))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ARENA_TYPE: // 24
            {
                if (!referencePlayer)
                    return false;

                Battleground const * const bg = referencePlayer->GetBattleground();
                if (!bg || !bg->isArena() || bg->GetArenaType() != reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_RACE: // 25
                if (!referencePlayer || (referencePlayer->getRace() != reqValue))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_CLASS: // 26
                if (!referencePlayer || (referencePlayer->getClass() != reqValue))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_RACE: // 27
                if (!unit || !unit->IsInWorld() || unit->GetTypeId() != TYPEID_PLAYER || unit->getRace() != reqValue)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_CLASS: // 28
                if (!unit || !unit->IsInWorld() || unit->GetTypeId() != TYPEID_PLAYER || unit->getClass() != reqValue)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_MAX_GROUP_MEMBERS: // 29
                if (!referencePlayer || (referencePlayer->GetGroup() && referencePlayer->GetGroup()->GetMembersCount() >= reqValue))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_CREATURE_TYPE: // 30
            {
                if (!unit)
                    return false;
                Creature const * const creature = unit->ToCreature();
                if (!creature || creature->GetCreatureType() != reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_MAP: // 32
                if (!referencePlayer || (referencePlayer->GetMapId() != reqValue))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_CLASS: // 33
            {
                ItemTemplate const* pItem = sObjectMgr->GetItemTemplate(miscValue1);
                if (!pItem)
                    return false;

                if (pItem->Class != reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_SUBCLASS: // 34
            {
                ItemTemplate const* pItem = sObjectMgr->GetItemTemplate(miscValue1);
                if (!pItem)
                    return false;

                if (pItem->SubClass != reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_COMPLETE_QUEST_NOT_IN_GROUP: // 35
                // miscValue is questid
                if (!referencePlayer || !referencePlayer->IsQuestRewarded(uint32(miscValue1)) || referencePlayer->GetGroup())
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_MIN_PERSONAL_RATING: // 37
            {
                if (miscValue1 <= reqValue)
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TITLE_BIT_INDEX: // 38
                // miscValue1 is title's bit index
                if (miscValue1 != reqValue)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_LEVEL: // 39
                if (!referencePlayer || (referencePlayer->getLevel() != reqValue))
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_LEVEL: // 40
                if (!unit || !unit->IsInWorld() || unit->getLevel() != reqValue)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_ZONE: // 41
                if (!referencePlayer || referencePlayer->GetZoneId() != reqValue)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_HEALTH_PERCENT_BELOW: // 46
                if (!unit || !unit->IsInWorld() || unit->GetHealthPct() >= reqValue)
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_MIN_ACHIEVEMENT_POINTS: // 56
                if (!referencePlayer || (referencePlayer->GetAchievementMgr().GetAchievementPoints() < reqValue))
                    return false;
                break;
#if 0
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_REQUIRES_LFG_GROUP: // 58
                if (!referencePlayer || !referencePlayer->inRandomLfgDungeon())
                    return false;
                break;
#endif
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_REQUIRES_GUILD_GROUP: // 61
            {
                if (!referencePlayer)
                    return false;

                Group* pGroup = referencePlayer->GetGroup();
                if (!pGroup)
                    return false;

                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_GUILD_REPUTATION: // 62
            {
                if (!referencePlayer)
                    return false;

                if (uint32(referencePlayer->GetReputationMgr().GetReputation(1168)) < reqValue) // 1168 = Guild faction
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_RATED_BATTLEGROUND: // 63
            {
                if (!referencePlayer)
                    return false;

                Battleground const * const bg = referencePlayer->GetBattleground();
                if (!bg || !bg->isRated())
                    return false;
                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_PROJECT_RARITY: // 65
            {
                if (!miscValue1)
                    return false;

                break;
            }
            case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_PROJECT_RACE: // 66
            {
                if (!miscValue1)
                    return false;

                break;
            }
            default:
                break;
        }
    }
    return true;
}

char const* AchievementGlobalMgr::GetCriteriaTypeString(uint32 type)
{
    return GetCriteriaTypeString(AchievementCriteriaTypes(type));
}

char const* AchievementGlobalMgr::GetCriteriaTypeString(AchievementCriteriaTypes type)
{
    switch (type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            return "KILL_CREATURE";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
            return "TYPE_WIN_BG";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS:
            return "COMPLETE_RESEARCH";
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            return "REACH_LEVEL";
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            return "REACH_SKILL_LEVEL";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            return "COMPLETE_ACHIEVEMENT";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
            return "COMPLETE_QUEST_COUNT";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
            return "COMPLETE_DAILY_QUEST_DAILY";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            return "COMPLETE_QUESTS_IN_ZONE";
        case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY:
            return "CURRENCY";
        case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE:
            return "DAMAGE_DONE";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
            return "COMPLETE_DAILY_QUEST";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
            return "COMPLETE_BATTLEGROUND";
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            return "DEATH_AT_MAP";
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
            return "DEATH";
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
            return "DEATH_IN_DUNGEON";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID:
            return "COMPLETE_RAID";
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
            return "KILLED_BY_CREATURE";
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
            return "KILLED_BY_PLAYER";
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
            return "FALL_WITHOUT_DYING";
        case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
            return "DEATHS_FROM";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            return "COMPLETE_QUEST";
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
            return "BE_SPELL_TARGET";
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
            return "CAST_SPELL";
        case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
            return "BG_OBJECTIVE_CAPTURE";
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            return "HONORABLE_KILL_AT_AREA";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA:
            return "WIN_ARENA";
        case ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA:
            return "PLAY_ARENA";
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            return "LEARN_SPELL";
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
            return "HONORABLE_KILL";
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            return "OWN_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
            return "WIN_RATED_ARENA";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING:
            return "HIGHEST_TEAM_RATING";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
            return "HIGHEST_PERSONAL_RATING";
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            return "LEARN_SKILL_LEVEL";
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            return "USE_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
            return "LOOT_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            return "EXPLORE_AREA";
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK:
            return "OWN_RANK";
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            return "BUY_BANK_SLOT";
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            return "GAIN_REPUTATION";
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
            return "GAIN_EXALTED_REPUTATION";
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
            return "VISIT_BARBER_SHOP";
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            return "EQUIP_EPIC_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
            return "ROLL_NEED_ON_LOOT";
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            return "GREED_ON_LOOT";
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            return "HK_CLASS";
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            return "HK_RACE";
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
            return "DO_EMOTE";
        case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE:
            return "HEALING_DONE";
        case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
            return "GET_KILLING_BLOWS";
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
            return "EQUIP_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
            return "MONEY_FROM_VENDORS";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
            return "GOLD_SPENT_FOR_TALENTS";
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
            return "NUMBER_OF_TALENT_RESETS";
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
            return "MONEY_FROM_QUEST_REWARD";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
            return "GOLD_SPENT_FOR_TRAVELLING";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
            return "GOLD_SPENT_AT_BARBER";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
            return "GOLD_SPENT_FOR_MAIL";
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
            return "LOOT_MONEY";
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            return "USE_GAMEOBJECT";
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            return "BE_SPELL_TARGET2";
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
            return "SPECIAL_PVP_KILL";
        case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
            return "FISH_IN_GAMEOBJECT";
        case ACHIEVEMENT_CRITERIA_TYPE_EARNED_PVP_TITLE:
            return "EARNED_PVP_TITLE";
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
            return "LEARN_SKILLLINE_SPELLS";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
            return "WIN_DUEL";
        case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
            return "LOSE_DUEL";
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
            return "KILL_CREATURE_TYPE";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
            return "GOLD_EARNED_BY_AUCTIONS";
        case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
            return "CREATE_AUCTION";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
            return "HIGHEST_AUCTION_BID";
        case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:
            return "WON_AUCTIONS";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
            return "HIGHEST_AUCTION_SOLD";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
            return "HIGHEST_GOLD_VALUE_OWNED";
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
            return "GAIN_REVERED_REPUTATION";
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
            return "GAIN_HONORED_REPUTATION";
        case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
            return "KNOWN_FACTIONS";
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
            return "LOOT_EPIC_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
            return "RECEIVE_EPIC_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
            return "ROLL_NEED";
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
            return "ROLL_GREED";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_DEALT:
            return "HIT_DEALT";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
            return "HIT_RECEIVED";
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
            return "TOTAL_DAMAGE_RECEIVED";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
            return "HIGHEST_HEAL_CASTED";
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
            return "TOTAL_HEALING_RECEIVED";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
            return "HIGHEST_HEALING_RECEIVED";
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
            return "QUEST_ABANDONED";
        case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
            return "FLIGHT_PATHS_TAKEN";
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
            return "LOOT_TYPE";
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
            return "CAST_SPELL2";
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE:
            return "LEARN_SKILL_LINE";
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
            return "EARN_HONORABLE_KILL";
        case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
            return "ACCEPTED_SUMMONINGS";
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
            return "EARN_ACHIEVEMENT_POINTS";
        case ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
            return "USE_LFD_TO_GROUP_WITH_PLAYERS";
        case ACHIEVEMENT_CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
            return "SPENT_GOLD_GUILD_REPAIRS";
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL:
            return "REACH_GUILD_LEVEL";
        case ACHIEVEMENT_CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
            return "CRAFT_ITEMS_GUILD";
        case ACHIEVEMENT_CRITERIA_TYPE_CATCH_FROM_POOL:
            return "CATCH_FROM_POOL";
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
            return "BUY_GUILD_BANK_SLOTS";
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
            return "EARN_GUILD_ACHIEVEMENT_POINTS";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:
            return "WIN_RATED_BATTLEGROUND";
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING:
            return "REACH_BG_RATING";
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_TABARD:
            return "BUY_GUILD_TABARD";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_GUILD:
            return "COMPLETE_QUESTS_GUILD";
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
            return "HONORABLE_KILLS_GUILD";
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
            return "KILL_CREATURE_TYPE_GUILD";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_TYPE:
            return "GUILD_CHALLENGE_TYPE";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE:
            return "GUILD_CHALLENGE";
        default:
            return "MISSING_TYPE";
    }
}

template class AchievementMgr<Guild>;
template class AchievementMgr<Player>;

//==========================================================
void AchievementGlobalMgr::LoadAchievementCriteriaList()
{
    uint32 oldMSTime = getMSTime();
    if (sCriteriaStore.GetNumRows() == 0)
    {
        TC_LOG_ERROR("server.loading", ">> Loaded 0 achievement criteria.");
        return;
    }

    for (uint32 entryId = 0; entryId < sCriteriaTreeStore.GetNumRows(); entryId++)
    {
        CriteriaTreeEntry const* criteriaTree = sCriteriaTreeStore.LookupEntry(entryId);
        if (!criteriaTree)
            continue;

        if (CriteriaTreeEntry const* parent = sCriteriaTreeStore.LookupEntry(criteriaTree->parentID))
            m_SubCriteriaTreeListById[parent->ID].push_back(criteriaTree);

        ModifierTreeEntryByTreeId m_ModifierTreeEntryByTreeId;
        CriteriaEntry const* criteriaEntry = sCriteriaStore.LookupEntry(criteriaTree->criteriaID);

        if (!criteriaEntry)
            continue;

        m_AchievementCriteriaTreeByCriteriaId[criteriaEntry->ID].push_back(criteriaTree);
    }

    for (uint32 entryId = 0; entryId < sModifierTreeStore.GetNumRows(); entryId++)
    {
        ModifierTreeEntry const* modifier = sModifierTreeStore.LookupEntry(entryId);

        if (!modifier)
            continue;

        if (modifier->parent)
            m_ModifierTreeEntryByTreeId[modifier->parent].push_back(modifier);
    }

    uint32 criterias = 0;
    for (uint32 entryId = 0; entryId < sCriteriaStore.GetNumRows(); entryId++)
    {
        CriteriaEntry const* criteria = sAchievementMgr->GetAchievementCriteria(entryId);
        if (!criteria)
            continue;

        m_AchievementCriteriasByType[criteria->type].push_back(criteria);
        if (criteria->timeLimit)
            m_AchievementCriteriasByTimedType[criteria->timedCriteriaStartType].push_back(criteria);

        ++criterias;
    }

    for (uint32 entryId = 0; entryId < sCriteriaStore.GetNumRows(); entryId++)
    {
        CriteriaEntry const* criteria = sAchievementMgr->GetAchievementCriteria(entryId);
        if (!criteria)
            continue;

        AchievementCriteriaTreeList achievementCriteriaTreeList = sAchievementMgr->GetAchievementCriteriaTreeList(criteria);
        for (AchievementCriteriaTreeList::const_iterator iter = achievementCriteriaTreeList.begin(); iter != achievementCriteriaTreeList.end(); iter++)
        {
            AchievementEntry const* achievement = sAchievementMgr->GetAchievementEntryByCriteriaTree(*iter);
            if (!achievement)
                continue;

            if (!(achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            m_GuildAchievementCriteriasByType[criteria->type].push_back(criteria);
            ++criterias;
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u achievement criteria in %u ms", criterias, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadAchievementReferenceList()
{
    uint32 oldMSTime = getMSTime();
    if (sAchievementStore.GetNumRows() == 0)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 achievement references.");
        return;
    }

    uint32 count = 0;
    for (uint32 entryId = 0; entryId < sAchievementStore.GetNumRows(); ++entryId)
    {
        AchievementEntry const* achievement = sAchievementMgr->GetAchievement(entryId);
        if (!achievement)
            continue;

        m_AchievementEntryByCriteriaTree[achievement->criteriaTreeID] = achievement;
        if (!achievement->refAchievement)
            continue;

        m_AchievementListByReferencedId[achievement->refAchievement].push_back(achievement);
        ++count;
    }

    // Once Bitten, Twice Shy (10 player) - Icecrown Citadel
    if (AchievementEntry const* achievement = sAchievementMgr->GetAchievement(4539))
        const_cast<AchievementEntry*>(achievement)->mapID = 631;    // Correct map requirement (currently has Ulduar)

    TC_LOG_INFO("server.loading", ">> Loaded %u achievement references in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadAchievementCriteriaData()
{
    uint32 oldMSTime = getMSTime();
    m_criteriaDataMap.clear();                              // Need for reload case

    QueryResult result = WorldDatabase.Query("SELECT criteria_id, type, value1, value2, ScriptName FROM achievement_criteria_data");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 additional achievement criteria data. DB table `achievement_criteria_data` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 criteria_id = fields[0].GetUInt32();

        CriteriaEntry const* criteria = sAchievementMgr->GetAchievementCriteria(criteria_id);
        if (!criteria)
        {
            TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` has data for non-existing criteria (Entry: %u), ignore.", criteria_id);
            continue;
        }

        uint32 dataType = fields[1].GetUInt8();
        const char* scriptName = fields[4].GetCString();
        uint32 scriptId = 0;
        if (strcmp(scriptName, "")) // Not empty
        {
            if (dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT)
                TC_LOG_ERROR("sql.sql", "Table `achievement_criteria_data` has ScriptName set for non-scripted data type (Entry: %u, type %u), useless data.", criteria_id, dataType);
            else
                scriptId = sObjectMgr->GetScriptId(scriptName);
        }

        AchievementCriteriaData data(dataType, fields[2].GetUInt32(), fields[3].GetUInt32(), scriptId);

        if (!data.IsValid(criteria))
            continue;

        // This will allocate empty data set storage
        AchievementCriteriaDataSet& dataSet = m_criteriaDataMap[criteria_id];
        dataSet.SetCriteriaId(criteria_id);

        // Add real data only for not NONE data types
        if (data.dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE)
            dataSet.Add(data);

        // Counting data by and data types
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u additional achievement criteria data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadCompletedAchievements()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = CharacterDatabase.Query("SELECT achievement FROM character_achievement GROUP BY achievement");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 completed achievements. DB table `character_achievement` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint16 achievementId = fields[0].GetUInt16();
        const AchievementEntry* achievement = sAchievementMgr->GetAchievement(achievementId);
        if (!achievement)
        {
            // Remove non existent achievements from all characters
            TC_LOG_ERROR("achievement", "Non-existing achievement %u data removed from table `character_achievement`.", achievementId);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEVMENT);

            stmt->setUInt16(0, uint16(achievementId));

            CharacterDatabase.Execute(stmt);

            continue;
        }
        else if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
            m_allCompletedAchievements.insert(achievementId);
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %lu completed achievements in %u ms", (unsigned long)m_allCompletedAchievements.size(), GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadRewards()
{
    uint32 oldMSTime = getMSTime();

    m_achievementRewards.clear();                           // Need for reload case

    //                                               0      1        2        3     4       5        6
    QueryResult result = WorldDatabase.Query("SELECT entry, title_A, title_H, item, sender, subject, text FROM achievement_reward");

    if (!result)
    {
        TC_LOG_ERROR("server.loading", ">> Loaded 0 achievement rewards. DB table `achievement_reward` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        const AchievementEntry* pAchievement = GetAchievement(entry);
        if (!pAchievement)
        {
            TC_LOG_ERROR("sql.sql", "Table `achievement_reward` has wrong achievement (Entry: %u), ignored.", entry);
            continue;
        }

        AchievementReward reward;
        reward.titleId[0] = fields[1].GetUInt32();
        reward.titleId[1] = fields[2].GetUInt32();
        reward.itemId     = fields[3].GetUInt32();
        reward.sender     = fields[4].GetUInt32();
        reward.subject    = fields[5].GetString();
        reward.text       = fields[6].GetString();

        // Must be title or mail at least
        if (!reward.titleId[0] && !reward.titleId[1] && !reward.sender)
        {
            TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) does not have title or item reward data, ignored.", entry);
            continue;
        }

        if (pAchievement->requiredFaction == ACHIEVEMENT_FACTION_ANY && ((reward.titleId[0] == 0) != (reward.titleId[1] == 0)))
            TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) has title (A: %u H: %u) for only one team.", entry, reward.titleId[0], reward.titleId[1]);

        if (reward.titleId[0])
        {
            CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(reward.titleId[0]);
            if (!titleEntry)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) has invalid title id (%u) in `title_A`, set to 0", entry, reward.titleId[0]);
                reward.titleId[0] = 0;
            }
        }

        if (reward.titleId[1])
        {
            CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(reward.titleId[1]);
            if (!titleEntry)
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) has invalid title id (%u) in `title_H`, set to 0", entry, reward.titleId[1]);
                reward.titleId[1] = 0;
            }
        }

        // Check mail data before item for report including wrong item case
        if (reward.sender)
        {
            if (!sObjectMgr->GetCreatureTemplate(reward.sender))
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) has invalid creature entry %u as sender, mail reward skipped.", entry, reward.sender);
                reward.sender = 0;
            }
        }
        else
        {
            if (reward.itemId)
                TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) does not have sender data but has item reward, item will not be rewarded.", entry);

            if (!reward.subject.empty())
                TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) does not have sender data but has mail subject.", entry);

            if (!reward.text.empty())
                TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) does not have sender data but has mail text.", entry);
        }

        if (reward.itemId)
        {
            if (!sObjectMgr->GetItemTemplate(reward.itemId))
            {
                TC_LOG_ERROR("sql.sql", "Table `achievement_reward` (Entry: %u) has invalid item id %u, reward mail will not contain item.", entry, reward.itemId);
                reward.itemId = 0;
            }
        }

        m_achievementRewards[entry] = reward;
        ++count;

    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u achievement rewards in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadRewardLocales()
{
    uint32 oldMSTime = getMSTime();

    m_achievementRewardLocales.clear();                       // Need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry, subject_loc1, text_loc1, subject_loc2, text_loc2, subject_loc3, text_loc3, subject_loc4, text_loc4, "
                                             "subject_loc5, text_loc5, subject_loc6, text_loc6, subject_loc7, text_loc7, subject_loc8, text_loc8, subject_loc9, text_loc9,"
                                             "subject_loc10, text_loc10 FROM locales_achievement_reward");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 achievement reward locale strings.  DB table `locales_achievement_reward` is empty");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (m_achievementRewards.find(entry) == m_achievementRewards.end())
        {
            TC_LOG_ERROR("sql.sql", "Table `locales_achievement_reward` (Entry: %u) has locale strings for non-existing achievement reward.", entry);
            continue;
        }

        AchievementRewardLocale& data = m_achievementRewardLocales[entry];

        for (int i = 1; i < TOTAL_LOCALES; ++i)
        {
            LocaleConstant locale = (LocaleConstant) i;
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1)].GetString(), locale, data.subject);
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1) + 1].GetString(), locale, data.text);
        }
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %lu achievement reward locale strings in %u ms", (unsigned long)m_achievementRewardLocales.size(), GetMSTimeDiffToNow(oldMSTime));
}

AchievementEntry const* AchievementGlobalMgr::GetAchievement(uint32 achievementId) const
{
    return sAchievementStore.LookupEntry(achievementId);
}

CriteriaEntry const* AchievementGlobalMgr::GetAchievementCriteria(uint32 criteriaId) const
{
    return sCriteriaStore.LookupEntry(criteriaId);
}
