/*
 * Copyright (C) 2011 TrintiyCore <http://www.trinitycore.org/>
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

#include "BattlePet.h"
#include "DB2Stores.h"
#include "Log.h"
#include "SharedDefines.h"
#include "SpellMgr.h"
#include "DB2fmt.h"

#include <list>
#include <string>
#include <unordered_map>

namespace {

QuestPackageItemMap sQuestPackageItemMap;
BattlePetSpellStore sBattlePetSpellXSpeciesStore;

typedef std::list<std::string> StoreProblemList1;

uint32 DB2FilesCount = 0;

bool LoadDB2_assert_print(uint32 fsize,uint32 rsize, const std::string& filename)
{
    TC_LOG_ERROR("misc", "Size of '%s' set by format string (%u) is not equal to size of C++ structure (%u).", filename.c_str(), fsize, rsize);

    // ASSERT must fail after function call
    return false;
}

} // namespace

DB2Storage<BattlePetAbilityEntry> sBattlePetAbilityStore(BattlePetAbilityfmt);
DB2Storage<BattlePetAbilityStateEntry> sBattlePetAbilityStateStore(BattlePetAbilityStatefmt);
DB2Storage<BattlePetAbilityEffectEntry> sBattlePetAbilityEffectStore(BattlePetAbilityEffectfmt);
DB2Storage<BattlePetAbilityTurnEntry> sBattlePetAbilityTurnStore(BattlePetAbilityTurnfmt);
DB2Storage<BattlePetBreedQualityEntry> sBattlePetBreedQualityStore(BattlePetBreedQualityfmt);
DB2Storage<BattlePetBreedStateEntry> sBattlePetBreedStateStore(BattlePetBreedStatefmt);
DB2Storage<BattlePetEffectPropertiesEntry> sBattlePetEffectPropertiesStore(BattlePetEffectPropertiesfmt);
DB2Storage<BattlePetSpeciesEntry> sBattlePetSpeciesStore(BattlePetSpeciesfmt);
DB2Storage<BattlePetSpeciesStateEntry> sBattlePetSpeciesStateStore(BattlePetSpeciesStatefmt);
DB2Storage<BattlePetSpeciesXAbilityEntry> sBattlePetSpeciesXAbilityStore(BattlePetSpeciesXAbilityfmt);
DB2Storage<BattlePetState> sBattlePetStateStore(sBattlePetStatefmt);

DB2Storage<ItemEntry> sItemStore(Itemfmt);
DB2Storage<ItemCurrencyCostEntry> sItemCurrencyCostStore(ItemCurrencyCostfmt);
DB2Storage<ItemExtendedCostEntry> sItemExtendedCostStore(ItemExtendedCostEntryfmt);
DB2Storage<ItemSparseEntry> sItemSparseStore (ItemSparsefmt);
DB2Storage<SpellReagentsEntry> sSpellReagentsStore(SpellReagentsEntryfmt);
DB2Storage<ItemUpgradeEntry> sItemUpgradeStore(ItemUpgradeEntryfmt);
DB2Storage<RulesetItemUpgradeEntry> sRulesetItemUpgradeStore(RulesetItemUpgradeEntryfmt);
DB2Storage<QuestPackageItemEntry> sQuestPackageItemStore(QuestPackageItemEntryfmt);

template<class T>
inline void LoadDB2(StoreProblemList1& errlist, DB2Storage<T>& storage, const std::string& db2_path, const std::string& filename)
{
    // compatibility format and C++ structure sizes
    ASSERT(DB2FileLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadDB2_assert_print(DB2FileLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), filename));

    ++DB2FilesCount;

    std::string db2_filename = db2_path + filename;
    if (!storage.Load(db2_filename.c_str()))
    {
        // sort problematic db2 to (1) non compatible and (2) nonexistent
        if (FILE * f = fopen(db2_filename.c_str(), "rb"))
        {
            char buf[100];
            snprintf(buf, 100,"(exist, but have %d fields instead " SIZEFMTD ") Wrong client version DBC file?", storage.GetFieldCount(), strlen(storage.GetFormat()));
            errlist.push_back(db2_filename + buf);
            fclose(f);
        }
        else
            errlist.push_back(db2_filename);
    }
}

void LoadDB2Stores(const std::string& dataPath)
{
    std::string db2Path = dataPath + "dbc/";

    StoreProblemList1 bad_db2_files;

    LoadDB2(bad_db2_files, sBattlePetAbilityStore, db2Path, "BattlePetAbility.db2");
    LoadDB2(bad_db2_files, sBattlePetAbilityEffectStore, db2Path, "BattlePetAbilityEffect.db2");
    LoadDB2(bad_db2_files, sBattlePetAbilityStateStore, db2Path, "BattlePetAbilityState.db2");
    LoadDB2(bad_db2_files, sBattlePetAbilityTurnStore, db2Path, "BattlePetAbilityTurn.db2");
    LoadDB2(bad_db2_files, sBattlePetBreedQualityStore, db2Path, "BattlePetBreedQuality.db2");
    LoadDB2(bad_db2_files, sBattlePetBreedStateStore, db2Path, "BattlePetBreedState.db2");
    LoadDB2(bad_db2_files, sBattlePetEffectPropertiesStore, db2Path, "BattlePetEffectProperties.db2");
    LoadDB2(bad_db2_files, sBattlePetSpeciesStore, db2Path, "BattlePetSpecies.db2");

    for (uint32 i = 0; i < sBattlePetSpeciesStore.GetNumRows(); i++)
    {
        auto speciesEntry = sBattlePetSpeciesStore.LookupEntry(i);
        if (!speciesEntry)
            continue;

        sBattlePetSpellXSpeciesStore[speciesEntry->SpellId] = speciesEntry->Id;
    }

    LoadDB2(bad_db2_files, sBattlePetSpeciesStateStore, db2Path, "BattlePetSpeciesState.db2");
    LoadDB2(bad_db2_files, sBattlePetSpeciesXAbilityStore, db2Path, "BattlePetSpeciesXAbility.db2");
    LoadDB2(bad_db2_files, sBattlePetStateStore, db2Path, "BattlePetState.db2");

    LoadDB2(bad_db2_files, sItemStore, db2Path, "Item.db2");
    LoadDB2(bad_db2_files, sItemCurrencyCostStore, db2Path, "ItemCurrencyCost.db2");
    LoadDB2(bad_db2_files, sItemSparseStore, db2Path, "Item-sparse.db2");
    LoadDB2(bad_db2_files, sItemExtendedCostStore, db2Path, "ItemExtendedCost.db2");
    LoadDB2(bad_db2_files, sSpellReagentsStore, db2Path, "SpellReagents.db2");                                                 // 17399
    LoadDB2(bad_db2_files, sItemUpgradeStore, db2Path, "ItemUpgrade.db2");
    LoadDB2(bad_db2_files, sRulesetItemUpgradeStore, db2Path, "RulesetItemUpgrade.db2");
    LoadDB2(bad_db2_files, sQuestPackageItemStore, db2Path, "QuestPackageItem.db2");

    // error checks
    if (bad_db2_files.size() >= DB2FilesCount)
    {
        TC_LOG_ERROR("misc", "\nIncorrect DataDir value in worldserver.conf or ALL required *.db2 files (%d) not found by path: %sdb2", DB2FilesCount, dataPath.c_str());
        exit(1);
    }
    else if (!bad_db2_files.empty())
    {
        std::string str;
        for (std::list<std::string>::iterator i = bad_db2_files.begin(); i != bad_db2_files.end(); ++i)
            str += *i + "\n";

        TC_LOG_ERROR("misc", "\nSome required *.db2 files (%u from %d) not found or not compatible:\n%s", (uint32)bad_db2_files.size(), DB2FilesCount,str.c_str());
        exit(1);
    }

    // Check loaded DB2 files proper version
    if (!sItemStore.LookupEntry(106130)             ||      // last item added in 5.4 (17371)
        !sItemExtendedCostStore.LookupEntry(5268)  )        // last item extended cost added in 5.4 (17371)
    {
        TC_LOG_ERROR("misc", "Please extract correct db2 files from client 5.4 17371.");
        exit(1);
    }

    for (size_t i = 1; i < sQuestPackageItemStore.GetNumRows(); ++i)
    {
        auto const entry = sQuestPackageItemStore.LookupEntry(i);
        if (!entry)
            continue;

        sQuestPackageItemMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(entry->Package),
            std::forward_as_tuple(entry->Item, entry->Count));
    }

    TC_LOG_INFO("misc", ">> Initialized %d DB2 data stores.", DB2FilesCount);
}

std::pair<QuestPackageItemMap::const_iterator, QuestPackageItemMap::const_iterator>
GetQuestPackageItems(uint32 packageId)
{
    return sQuestPackageItemMap.equal_range(packageId);
}

bool HasBattlePetSpeciesFlag(uint16 species, uint16 flag)
{
    auto speciesEntry = sBattlePetSpeciesStore.LookupEntry(species);
    if (!speciesEntry)
        return false;

    return (speciesEntry->Flags & flag) != 0;
}

uint32 GetBattlePetSummonSpell(uint16 species)
{
    auto speciesEntry = sBattlePetSpeciesStore.LookupEntry(species);
    if (!speciesEntry)
        return 0;

    return speciesEntry->SpellId;
}

uint16 GetBattlePetSpeciesFromSpell(uint32 spellId)
{
    auto spellSpeciesEntry = sBattlePetSpellXSpeciesStore.find(spellId);
    if (spellSpeciesEntry == sBattlePetSpellXSpeciesStore.end())
        return 0;

    return spellSpeciesEntry->second;
}
