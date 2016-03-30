/*
 * Copyright (C) 2015 Warmane <http://www.warmane.com/>
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
#include "BattlePetMgr.h"
#include "BattlePetSpawnMgr.h"
#include "Chat.h"
#include "Common.h"
#include "DB2Stores.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Unit.h"

class battlepet_commandscript : public CommandScript
{
public:
    battlepet_commandscript() : CommandScript("battlepet_commandscript") { }

    ChatCommand* GetCommands() const override
    {
        static ChatCommand battlePetCommandTable[] =
        {
            { "level",          SEC_ADMINISTRATOR,    false, &HandleBattlePetLevel,        "", nullptr },
            { "learnall",       SEC_ADMINISTRATOR,   false, &HandleBattlePetLearnAll,     "", nullptr },
            { nullptr,          0,                                              false, nullptr,                      "", nullptr }
        };

        static ChatCommand commandTable[] =
        {
            { "battlepet",      SEC_ADMINISTRATOR,             false, nullptr,                     "", battlePetCommandTable },
            { nullptr,          0,                                              false, nullptr,                     "", nullptr }
        };

        return commandTable;
    }

    static bool HandleBattlePetLevel(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint8 newLevel = atoi((char*)args);
        if (!newLevel)
            return false;

        // make sure a creature is selected
        auto creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);

            return false;
        }

        // creature must be a battle pet that belongs to a player
        uint64 battlePetId = creature->GetUInt64Value(UNIT_FIELD_BATTLEPETCOMPANIONGUID);
        if (!battlePetId)
        {
            handler->PSendSysMessage(LANG_BATTLE_PET_NOT_PLAYER_OWNED);
            handler->SetSentErrorMessage(true);

            return false;
        }

        // if this fails there is a major problem elsewhere
        auto player = sObjectMgr->GetPlayerByLowGUID(GUID_LOPART(creature->GetUInt64Value(UNIT_FIELD_CREATEDBY)));
        ASSERT(player);

        auto &battlePetMgr = player->GetBattlePetMgr();

        // update battle pet level
        auto battlePet = battlePetMgr.GetBattlePet(battlePetId);
        battlePet->SetLevel(newLevel);

        // alert client of change
        player->GetBattlePetMgr().SendBattlePetUpdate(battlePet, false);

        handler->PSendSysMessage(LANG_BATTLE_PET_TARGET_LEVELED, newLevel);
        return true;
    }

    static bool HandleBattlePetLearnAll(ChatHandler* handler, char const* args)
    {
        Player* target = handler->getSelectedPlayer();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        auto &battlePetMgr = target->GetBattlePetMgr();
        for (uint32 i = 0; i < sBattlePetSpeciesStore.GetNumRows(); i++)
        {
            auto battlePetSpeciesEntry = sBattlePetSpeciesStore.LookupEntry(i);
            if (!battlePetSpeciesEntry)
                continue;

            // make sure player doesn't already have a battle pet of this species
            uint16 species = battlePetSpeciesEntry->Id;
            if (battlePetMgr.GetBattlePetCount(species))
                continue;

            // add battle pet to journal
            battlePetMgr.Create(battlePetSpeciesEntry->Id, 25, sObjectMgr->BattlePetGetRandomBreed(species), sObjectMgr->BattlePetGetRandomQuality(species), true);
        }

        return true;
    }
};

void AddSC_battlepet_commandscript()
{
    new battlepet_commandscript();
}
