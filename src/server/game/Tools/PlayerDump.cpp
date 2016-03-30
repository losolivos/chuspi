/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#include "PlayerDump.h"
#include "DatabaseEnv.h"
#include "UpdateFields.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "World.h"
#include "ArraySize.h"
#include "LexicalCast.h"

#include <sstream>
#include <cstdio>
#include <cstring>

namespace {

enum class GuidType
{
    Item,
    Mail
};

struct DumpTable final
{
    char const* name;
    DumpTableType type;
};

DumpTable const dumpTables[] =
{
    { "characters",                       DTT_CHARACTER   },
    { "character_account_data",           DTT_CHAR_TABLE  },
    { "character_achievement",            DTT_CHAR_TABLE  },
    { "character_achievement_progress",   DTT_CHAR_TABLE  },
    { "character_action",                 DTT_CHAR_TABLE  },
    { "character_archaeology",            DTT_CHAR_TABLE  },
    { "character_aura",                   DTT_CHAR_TABLE  },
    { "character_aura_effect",            DTT_CHAR_TABLE  },
    { "character_cuf_profiles",           DTT_CHAR_TABLE  },
    { "character_currency",               DTT_CHAR_TABLE  },
    { "character_declinedname",           DTT_CHAR_TABLE  },
    { "character_gifts",                  DTT_ITEM_GIFT   },
    { "character_glyphs",                 DTT_CHAR_TABLE  },
    { "character_homebind",               DTT_CHAR_TABLE  },
    { "character_inventory",              DTT_INVENTORY   },
    { "character_known_titles",           DTT_CHAR_TABLE  },
    { "character_queststatus",            DTT_CHAR_TABLE  },
    { "character_queststatus_daily",      DTT_CHAR_TABLE  },
    { "character_queststatus_monthly",    DTT_CHAR_TABLE  },
    { "character_queststatus_rewarded",   DTT_CHAR_TABLE  },
    { "character_queststatus_seasonal",   DTT_CHAR_TABLE  },
    { "character_queststatus_weekly",     DTT_CHAR_TABLE  },
    { "character_reputation",             DTT_CHAR_TABLE  },
    { "character_skills",                 DTT_CHAR_TABLE  },
    { "character_spell",                  DTT_CHAR_TABLE  },
    { "character_spell_cooldown",         DTT_CHAR_TABLE  },
    { "character_talent",                 DTT_CHAR_TABLE  },
    { "character_void_storage",           DTT_ITEM_VOID   },
    { "mail",                             DTT_MAIL        },
    { "mail_items",                       DTT_MAIL_ITEM   },
    { "item_instance",                    DTT_ITEM        },
    { "pet_info",                         DTT_PET         },
    { "pet_action_bar",                   DTT_PET_TABLE   },
    { "pet_declined_name",                DTT_PET_TABLE   },
    { "pet_slot",                         DTT_PET_TABLE   },
    { "character_pet",                    DTT_CHAR_PET    },
};

// Low level functions
bool findnth(std::string &str, int n, std::string::size_type &s, std::string::size_type &e)
{
    s = str.find("VALUES ('")+9;
    if (s == std::string::npos) return false;

    do
    {
        e = str.find('\'', s);
        if (e == std::string::npos) return false;
    } while (str[e-1] == '\\');

    for (int i = 1; i < n; ++i)
    {
        do
        {
            s = e+4;
            e = str.find('\'', s);
            if (e == std::string::npos) return false;
        } while (str[e-1] == '\\');
    }
    return true;
}

std::string gettablename(std::string &str)
{
    std::string::size_type s = 13;
    std::string::size_type e = str.find('`', s);
    if (e == std::string::npos)
        return "";

    return str.substr(s, e-s);
}

bool changenth(std::string &str, int n, char const* with, bool insert = false, bool nonzero = false)
{
    std::string::size_type s, e;
    if (!findnth(str, n, s, e))
        return false;

    if (nonzero && str.substr(s, e-s) == "0")
        return true;                                        // not an error
    if (!insert)
        str.replace(s, e-s, with);
    else
        str.insert(s, with);

    return true;
}

std::string getnth(std::string &str, int n)
{
    std::string::size_type s, e;
    if (!findnth(str, n, s, e))
        return "";

    return str.substr(s, e-s);
}

uint32 registerNewGuid(uint32 oldGuid, std::map<uint32, uint32> &guidMap, GuidType guidType)
{
    std::map<uint32, uint32>::const_iterator itr = guidMap.find(oldGuid);
    if (itr != guidMap.end())
        return itr->second;

    uint32 &newguid = guidMap[oldGuid];
    switch (guidType)
    {
        case GuidType::Item:
            newguid = sObjectMgr->GenerateLowGuid(HIGHGUID_ITEM);
            break;
        case GuidType::Mail:
            newguid = sObjectMgr->GenerateMailID();
            break;
    }

    return newguid;
}

bool changeGuid(std::string &str, int n, std::map<uint32, uint32> &guidMap, GuidType guidType, bool nonzero = false)
{
    char chritem[20];
    uint32 oldGuid = Trinity::lexicalCast<uint32>(getnth(str, n).c_str());
    if (nonzero && oldGuid == 0)
        return true;                                        // not an error

    uint32 newGuid = registerNewGuid(oldGuid, guidMap, guidType);
    snprintf(chritem, 20, "%u", newGuid);

    return changenth(str, n, chritem, false, nonzero);
}

std::string CreateDumpString(char const* tableName, QueryResult result)
{
    if (!tableName || !result)
        return "";

    Field* fields = result->Fetch();

    std::ostringstream ss;

    ss << "INSERT INTO `" << tableName << "` VALUES (";
    for (uint32 i = 0; i < result->GetFieldCount(); ++i)
    {
        if (i == 0)
            ss << '\'';
        else
            ss << ", '";

        std::string s = fields[i].GetString();
        CharacterDatabase.EscapeString(s);

        ss << s << '\'';
    }
    ss << ");";

    return ss.str();
}

void fixNULLfields(std::string &line)
{
    std::string nullString("'NULL'");
    size_t pos = line.find(nullString);
    while (pos != std::string::npos)
    {
        line.replace(pos, nullString.length(), "NULL");
        pos = line.find(nullString);
    }
}

void StoreGUID(QueryResult result, uint32 field, std::set<uint32>& guids)
{
    Field* fields = result->Fetch();
    uint32 guid = fields[field].GetUInt32();
    if (guid)
        guids.insert(guid);
}

} // namespace

std::string PlayerDumpWriter::GenerateWhereStr(char const* field, uint32 guid)
{
    std::ostringstream wherestr;
    wherestr << field << " = '" << guid << '\'';
    return wherestr.str();
}

std::string PlayerDumpWriter::GenerateWhereStr(char const* field, GuidSet const& guids, GuidSet::const_iterator& itr)
{
    std::ostringstream wherestr;
    wherestr << field << " IN ('";
    for (; itr != guids.end(); ++itr)
    {
        wherestr << *itr;

        if (wherestr.str().size() > MAX_QUERY_LEN - 50)      // near to max query
        {
            ++itr;
            break;
        }

        GuidSet::const_iterator itr2 = itr;
        if (++itr2 != guids.end())
            wherestr << "', '";
    }
    wherestr << "')";
    return wherestr.str();
}

// Writing - High-level functions
bool PlayerDumpWriter::DumpTable(std::ostringstream &ss, uint32 guid, char const *tableFrom, char const *tableTo, DumpTableType type)
{
    GuidSet const* guids = NULL;
    char const* fieldname = NULL;

    switch (type)
    {
        case DTT_ITEM:
        case DTT_ITEM_TABLE:
            fieldname = "guid";
            guids = &items;
            break;
        case DTT_ITEM_GIFT:
            fieldname = "item_guid";
            guids = &items;
            break;
        case DTT_PET:
            fieldname = "owner";
            break;
        case DTT_PET_TABLE:
            fieldname = "id";
            guids = &pets;
            break;
        case DTT_MAIL:
            fieldname = "receiver";
            break;
        case DTT_MAIL_ITEM:
            fieldname = "mail_id";
            guids = &mails;
            break;
        case DTT_ITEM_VOID:
            fieldname = "playerGuid";
            break;
        default:
            fieldname = "guid";
            break;
    }

    // for guid set stop if set is empty
    if (guids && guids->empty())
        return true;                                        // nothing to do

    // setup for guids case start position
    GuidSet::const_iterator guids_itr;
    if (guids)
        guids_itr = guids->begin();

    do
    {
        std::string wherestr;

        if (guids)                                           // set case, get next guids string
            wherestr = GenerateWhereStr(fieldname, *guids, guids_itr);
        else                                                // not set case, get single guid string
            wherestr = GenerateWhereStr(fieldname, guid);

        QueryResult result = CharacterDatabase.PQuery("SELECT * FROM %s WHERE %s", tableFrom, wherestr.c_str());
        if (!result)
            return true;

        do
        {
            // collect guids
            switch (type)
            {
                case DTT_INVENTORY:
                    StoreGUID(result, 3, items);                // item guid collection (character_inventory.item)
                    break;
                case DTT_PET:
                    StoreGUID(result, 0, pets);                 // pet petnumber collection (character_pet.id)
                    break;
                case DTT_MAIL:
                    StoreGUID(result, 0, mails);                // mail id collection (mail.id)
                    break;
                case DTT_MAIL_ITEM:
                    StoreGUID(result, 1, items);                // item guid collection (mail_items.item_guid)
                    break;
                case DTT_CHARACTER:
                {
                    if (result->GetFieldCount() <= 63)          // avoid crashes on next check
                    {
                        TC_LOG_FATAL("misc", "PlayerDumpWriter::DumpTable - Trying to access non-existing or wrong positioned field (`deleteInfos_Account`) in `characters` table.");
                        return false;
                    }

                    if (result->Fetch()[63].GetUInt32())        // characters.deleteInfos_Account - if filled error
                        return false;
                    break;
                }
                default:
                    break;
            }

            ss << CreateDumpString(tableTo, result) << '\n';
        }
        while (result->NextRow());
    }
    while (guids && guids_itr != guids->end());              // not set case iterate single time, set case iterate for all guids

    return true;
}

bool PlayerDumpWriter::GetDump(uint32 guid, std::string &dump)
{
    size_t const dumpTableSize = TC_ARRAY_SIZE(dumpTables);

    // Required for cases when PlayerDumpWriter is reused.
    items.clear();
    mails.clear();
    pets.clear();

    std::ostringstream tableStream[dumpTableSize];

    for (size_t i = 0; i < dumpTableSize; ++i)
        if (!DumpTable(tableStream[i], guid, dumpTables[i].name, dumpTables[i].name, dumpTables[i].type))
            return false;

    std::ostringstream ss;

    ss << "-- THIS DUMPFILE IS MADE FOR USE WITH THE 'PDUMP' COMMAND ONLY - EITHER THROUGH INGAME CHAT OR ON CONSOLE!\n"
    << "-- DO NOT apply it directly - it will irreversibly DAMAGE and CORRUPT your database! You have been warned!\n\n";

    // character and item_instance data must be saved before anything else due to FK constraints
    ss << tableStream[0].str() << tableStream[30].str();

    for (size_t i = 1; i < 30; ++i)
        ss << tableStream[i].str();
    for (size_t i = 31; i < dumpTableSize; ++i)
        ss << tableStream[i].str();

    dump = ss.str();
    return true;
}

DumpReturn PlayerDumpWriter::WriteDump(const std::string& file, uint32 guid)
{
    if (sWorld->getBoolConfig(CONFIG_PDUMP_NO_PATHS))
        if (std::strstr(file.c_str(), "\\") || std::strstr(file.c_str(), "/"))
            return DUMP_FILE_OPEN_ERROR;

    if (sWorld->getBoolConfig(CONFIG_PDUMP_NO_OVERWRITE))
        if (FILE* f = std::fopen(file.c_str(), "rb"))
        {
            std::fclose(f);
            return DUMP_FILE_OPEN_ERROR;
        }

    FILE* fout = std::fopen(file.c_str(), "wb");
    if (!fout)
        return DUMP_FILE_OPEN_ERROR;

    DumpReturn ret = DUMP_SUCCESS;
    std::string dump;
    if (!GetDump(guid, dump))
        ret = DUMP_CHARACTER_DELETED;

    std::fwrite(dump.c_str(), sizeof(char), dump.size() + 1, fout);
    std::fclose(fout);

    return ret;
}

std::pair<DumpReturn, uint32> PlayerDumpReader::LoadDump(char const *dump, uint32 account, std::string name)
{
    uint32 charcount = AccountMgr::GetCharactersCount(account);
    if (charcount >= 10)
        return std::make_pair(DUMP_TOO_MANY_CHARS, 0u);

    // normalize the name if specified and check if it exists
    if (!normalizePlayerName(name))
        name = "";

    if (ObjectMgr::CheckPlayerName(name, true) == CHAR_NAME_SUCCESS)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->setString(0, name);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
            name = "";                                      // use the one from the dump
    }
    else
        name = "";

    uint32 const guid = sObjectMgr->GenerateLowGuid(HIGHGUID_PLAYER);

    // name encoded or empty
    char newguid[20], chraccount[20], newpetid[20], currpetid[20], lastpetid[20];

    snprintf(newguid, 20, "%u", guid);
    snprintf(chraccount, 20, "%u", account);
    snprintf(newpetid, 20, "%u", sObjectMgr->GeneratePetNumber());
    snprintf(lastpetid, 20, "%s", "");

    std::map<uint32, uint32> items;
    std::map<uint32, uint32> mails;

    typedef std::map<uint32, uint32> PetIds;                // old->new petid relation
    PetIds petids;

    std::istringstream dumpStream(dump);
    std::string line;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    while (std::getline(dumpStream, line))
    {
        // skip empty strings
        size_t nw_pos = line.find_first_not_of(" \t\n\r\7");
        if (nw_pos == std::string::npos)
            continue;

        // skip logfile-side dump start notice, comment lines and dump end notices
        if (line[nw_pos] == '=' || line[nw_pos] == '-')
            continue;

        // determine table name and load type
        std::string tn = gettablename(line);
        if (tn.empty())
        {
            TC_LOG_ERROR("misc", "LoadPlayerDump: Can't extract table name from line: '%s'!", line.c_str());
            return std::make_pair(DUMP_FILE_BROKEN, 0u);
        }

        DumpTableType type = DumpTableType(0);
        uint8 i;

        size_t const dumpTableCount = TC_ARRAY_SIZE(dumpTables);

        for (i = 0; i < dumpTableCount; ++i)
        {
            if (tn == dumpTables[i].name)
            {
                type = dumpTables[i].type;
                break;
            }
        }

        if (i == dumpTableCount)
        {
            TC_LOG_ERROR("misc", "LoadPlayerDump: Unknown table: '%s'!", tn.c_str());
            return std::make_pair(DUMP_FILE_BROKEN, 0u);
        }

        // change the data to server values
        switch (type)
        {
            case DTT_CHARACTER:
            {
                if (!changenth(line, 1, newguid))           // characters.guid update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                if (!changenth(line, 2, chraccount))        // characters.account update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                if (name.empty())
                {
                    // check if the original name already exists
                    name = getnth(line, 3);

                    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
                    stmt->setString(0, name);
                    PreparedQueryResult result = CharacterDatabase.Query(stmt);

                    if (result)
                        if (!changenth(line, 39, "1"))       // characters.at_login set to "rename on login"
                            return std::make_pair(DUMP_FILE_BROKEN, 0u);
                }
                else if (!changenth(line, 3, name.c_str())) // characters.name
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                const char null[] = "NULL";
                if (!changenth(line, 64, null))             // characters.deleteInfos_Account
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                if (!changenth(line, 65, null))             // characters.deleteInfos_Name
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                if (!changenth(line, 66, null))             // characters.deleteDate
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                break;
            }
            case DTT_CHAR_TABLE:
            {
                if (!changenth(line, 1, newguid))           // character_*.guid update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                break;
            }
            case DTT_INVENTORY:
            {
                if (!changenth(line, 1, newguid))           // character_inventory.guid update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                if (!changeGuid(line, 2, items, GuidType::Item, true))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);             // character_inventory.bag update
                if (!changeGuid(line, 4, items, GuidType::Item))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);             // character_inventory.item update
                break;
            }
            case DTT_MAIL:                                  // mail
            {
                if (!changeGuid(line, 1, mails, GuidType::Mail))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);             // mail.id update
                if (!changenth(line, 6, newguid))           // mail.receiver update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                break;
            }
            case DTT_MAIL_ITEM:                             // mail_items
            {
                if (!changeGuid(line, 1, mails, GuidType::Mail))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);             // mail_items.id
                if (!changeGuid(line, 2, items, GuidType::Item))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);             // mail_items.item_guid
                break;
            }
            case DTT_ITEM:
            {
                // item, owner, data field:item, owner guid
                if (!changeGuid(line, 1, items, GuidType::Item))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);                 // item_instance.guid update
                if (!changenth(line, 3, newguid))           // item_instance.owner_guid update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                char const null[] = "NULL";
                if (!changenth(line, 4, null))              // item_instance.creatorGuid
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                if (!changenth(line, 5, null))              // item_instance.giftCreatorGuid
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                break;
            }
            case DTT_ITEM_TABLE:
            {
                if (!changeGuid(line, 1, items, GuidType::Item))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                break;
            }
            case DTT_ITEM_GIFT:
            {
                if (!changenth(line, 1, newguid))           // character_gifts.guid update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                if (!changeGuid(line, 2, items, GuidType::Item))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);             // character_gifts.item_guid update
                break;
            }
            case DTT_PET:
            {
                //store a map of old pet id to new inserted pet id for use by type 5 tables
                snprintf(currpetid, 20, "%s", getnth(line, 1).c_str());
                if (*lastpetid == '\0')
                    snprintf(lastpetid, 20, "%s", currpetid);
                if (strcmp(lastpetid, currpetid) != 0)
                {
                    snprintf(newpetid, 20, "%u", sObjectMgr->GeneratePetNumber());
                    snprintf(lastpetid, 20, "%s", currpetid);
                }

                if (petids.find(atoi(currpetid)) == petids.end())
                    petids.insert(PetIds::value_type(atoi(currpetid), atoi(newpetid)));

                if (!changenth(line, 1, newpetid))          // pet_info.id update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                if (!changenth(line, 2, newguid))           // pet_info.owner update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                break;
            }
            case DTT_PET_TABLE:                             // pet_aura, pet_spell, pet_spell_cooldown
            {
                snprintf(currpetid, 20, "%s", getnth(line, 1).c_str());

                // lookup currpetid and match to new inserted pet id
                PetIds::const_iterator petids_iter = petids.find(atoi(currpetid));
                if (petids_iter == petids.end())             // couldn't find new inserted id
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                snprintf(newpetid, 20, "%u", petids_iter->second);

                if (!changenth(line, 1, newpetid))
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                break;
            }
            case DTT_CHAR_PET:
            {
                snprintf(currpetid, 20, "%s", getnth(line, 2).c_str());

                // lookup currpetid and match to new inserted pet id
                PetIds::const_iterator petids_iter = petids.find(atoi(currpetid));
                if (petids_iter == petids.end())             // couldn't find new inserted id
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                snprintf(newpetid, 20, "%u", petids_iter->second);

                if (!changenth(line, 1, newguid))           // character_pet.guid update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                if (!changenth(line, 2, newpetid))          // character_pet.pet_id update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                break;
            }
            case DTT_ITEM_VOID:
            {
                char voidStorageId[21];
                snprintf(voidStorageId, 21, UI64FMTD, sObjectMgr->GenerateVoidStorageItemId());

                if (!changenth(line, 1, voidStorageId))     // character_void_storage.id update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);
                if (!changenth(line, 2, newguid))           // character_void_storage.playerGuid update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                char const zero[] = "0";
                if (!changenth(line, 5, zero))              // character_void_storage.creatorGuid update
                    return std::make_pair(DUMP_FILE_BROKEN, 0u);

                break;
            }
            default:
                TC_LOG_ERROR("misc", "Unknown dump table type: %u", type);
                break;
        }

        fixNULLfields(line);

        trans->Append(line.c_str());
    }

    CharacterDatabase.CommitTransaction(trans);

    return std::make_pair(DUMP_SUCCESS, guid);
}
