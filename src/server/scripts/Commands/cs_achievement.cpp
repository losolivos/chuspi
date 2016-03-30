/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

/* ScriptData
Name: achievement_commandscript
%Complete: 100
Comment: All achievement related commands
Category: commandscripts
EndScriptData */

#include "AchievementMgr.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "ScriptMgr.h"

class achievement_commandscript : public CommandScript
{
public:
    achievement_commandscript() : CommandScript("achievement_commandscript") { }

    ChatCommand* GetCommands() const override
    {
        static ChatCommand achievementCommandTable[] =
        {
            { "criteria", SEC_GAMEMASTER, false, &HandleAchievementCriteriaCommand, "", NULL },
            { "check",    SEC_GAMEMASTER, false, &HandleAchievementCheckCommand,    "", NULL },
            { "start",    SEC_GAMEMASTER, false, &HandleAchievementStartTimedCriteriaCommand, "", NULL },
            { "add", SEC_ADMINISTRATOR, false, &HandleAchievementAddCommand, "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };
        static ChatCommand commandTable[] =
        {
            { "achievement", SEC_ADMINISTRATOR,  false, NULL, "", achievementCommandTable },
            { NULL, 0, false, NULL, "", NULL }
        };
        return commandTable;
    }

    static bool HandleAchievementCheckCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->CheckAllAchievementCriteria();
        return true;
    }

    static bool HandleAchievementStartTimedCriteriaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* ctype = strtok((char*)args, " ");
        uint32 type = ctype ? atoi(ctype) : 0;
        uint32 entry = 0;

        ctype = strtok(NULL, " ");
        if (ctype)
            entry = atoi(ctype);

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->GetAchievementMgr().StartTimedAchievement(AchievementCriteriaTimedTypes(type), entry);
        return true;
    }

    static bool HandleAchievementCriteriaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* ctype = strtok((char*)args, " ");
        uint32 type = ctype ? atoi(ctype) : 0;
        uint64 val1 = 0;
        uint64 val2 = 0;
        uint64 val3 = 0;

        ctype = strtok(NULL, " ");
        if (ctype)
            val1 = atoi(ctype);

        ctype = strtok(NULL, " ");
        if (ctype)
            val2 = atoi(ctype);

        ctype = strtok(NULL, " ");
        if (ctype)
            val3 = atoi(ctype);

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->UpdateAchievementCriteria(AchievementCriteriaTypes(type), val1, val2, val3, target->GetSelectedUnit());
        return true;
    }

    static bool HandleAchievementAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 achievementId = atoi((char*)args);
        if (!achievementId)
        {
            if (char* id = handler->extractKeyFromLink((char*)args, "Hachievement"))
                achievementId = atoi(id);
            if (!achievementId)
                return false;
        }

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (AchievementEntry const* achievementEntry = sAchievementMgr->GetAchievement(achievementId))
            target->CompletedAchievement(achievementEntry);

        return true;
    }
};

void AddSC_achievement_commandscript()
{
    new achievement_commandscript();
}
