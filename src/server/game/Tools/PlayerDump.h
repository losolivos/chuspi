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

#ifndef _PLAYER_DUMP_H
#define _PLAYER_DUMP_H

#include "Define.h"

#include <iosfwd>
#include <string>
#include <set>
#include <utility>

enum DumpTableType
{
    DTT_CHARACTER,
    DTT_CHAR_TABLE,
    DTT_INVENTORY,
    DTT_MAIL,
    DTT_MAIL_ITEM,
    DTT_ITEM,
    DTT_ITEM_GIFT,
    DTT_ITEM_TABLE,
    DTT_PET,
    DTT_PET_TABLE,
    DTT_CHAR_PET,
    DTT_ITEM_VOID
};

enum DumpReturn
{
    DUMP_SUCCESS,
    DUMP_FILE_OPEN_ERROR,
    DUMP_TOO_MANY_CHARS,
    DUMP_UNEXPECTED_END,
    DUMP_FILE_BROKEN,
    DUMP_CHARACTER_DELETED
};

class PlayerDumpWriter final
{
public:
    bool GetDump(uint32 guid, std::string& dump);
    DumpReturn WriteDump(std::string const& file, uint32 guid);

private:
    typedef std::set<uint32> GuidSet;

    bool DumpTable(std::ostringstream& ss, uint32 guid, char const*tableFrom, char const*tableTo, DumpTableType type);
    std::string GenerateWhereStr(char const* field, GuidSet const& guids, GuidSet::const_iterator& itr);
    std::string GenerateWhereStr(char const* field, uint32 guid);

    GuidSet pets;
    GuidSet mails;
    GuidSet items;
};

struct PlayerDumpReader final
{
    static std::pair<DumpReturn, uint32> LoadDump(char const *dump, uint32 account, std::string name);
};

#endif
