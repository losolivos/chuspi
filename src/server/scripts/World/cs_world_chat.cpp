#include "ScriptPCH.h"
#include "Chat.h"

#define MSG_COLOR_BLUEVIOLET "|cFF8A2BE2"
const char* CLASS_ICON;

#define FACTION_SPECIFIC 0

std::string GetNameLink(Player* player)
{
        std::string name = player->GetName();
        std::string color;
        switch(player->getClass())
        {
        case CLASS_DEATH_KNIGHT:
                color = "|cffC41F3B";
				CLASS_ICON = "|TInterface\\icons\\Spell_Deathknight_ClassIcon:15|t|r";
                break;
        case CLASS_DRUID:
                color = "|cffFF7D0A";
				CLASS_ICON = "|TInterface\\icons\\Ability_Druid_Maul:15|t|r";
                break;
        case CLASS_HUNTER:
                color = "|cffABD473";
				CLASS_ICON = "|TInterface\\icons\\INV_Weapon_Bow_07:15|t|r";
                break;
        case CLASS_MAGE:
                color = "|cff69CCF0";
				CLASS_ICON = "|TInterface\\icons\\INV_Staff_13:15|t|r";
                break;
        case CLASS_PALADIN:
                color = "|cffF58CBA";
				CLASS_ICON = "|TInterface\\icons\\INV_Hammer_01:15|t|r";
                break;
        case CLASS_PRIEST:
                color = "|cffFFFFFF";
				CLASS_ICON = "|TInterface\\icons\\INV_Staff_30:15|t|r";
                break;
        case CLASS_ROGUE:
                color = "|cffFFF569";
				CLASS_ICON = "|TInterface\\icons\\INV_ThrowingKnife_04:15|t|r";
                break;
        case CLASS_SHAMAN:
                color = "|cff0070DE";
				CLASS_ICON = "|TInterface\\icons\\Spell_Nature_BloodLust:15|t|r";
                break;
        case CLASS_WARLOCK:
                color = "|cff9482C9";
				CLASS_ICON = "|TInterface\\icons\\Spell_Nature_FaerieFire:15|t|r";
                break;
        case CLASS_WARRIOR:
                color = "|cffC79C6E";
				CLASS_ICON = "|TInterface\\icons\\INV_Sword_27:15|t|r";
                break;
        case CLASS_MONK:
                color = "|cff0fe384";
				CLASS_ICON = "|TInterface\\icons\\ClassIcon_Monk:15|t|r";
                break;
        }
        return "|Hplayer:" + name + "|h" + CLASS_ICON + "|cffFFFFFF[" + color + name + "|cffFFFFFF]|h|r";
}

class cs_world_chat : public CommandScript
{
        public:
                cs_world_chat() : CommandScript("cs_world_chat"){}
 
        ChatCommand * GetCommands() const
        {
                static ChatCommand WorldChatCommandTable[] =
                {
                        {"chat",        SEC_PLAYER,             true,           &HandleWorldChatCommand,        "", NULL},
                        {NULL,          0,                              false,          NULL,                                           "", NULL}
                };
 
                return WorldChatCommandTable;
        }

        static bool HandleWorldChatCommand(ChatHandler * handler, const char * args)
        {
                if (!args)
                        return false;

                std::string msg = "";
                Player * player = handler->GetSession()->GetPlayer();

                switch(player->GetSession()->GetSecurity())
                {
                        // Player
                        case SEC_PLAYER:
                                if (player->GetTeam() == ALLIANCE)
                                {
                                        msg += "|cff00ff00[World]";
                                        msg += "|cff0000ff|TInterface\\pvpframe\\PVPCurrency-Honor-Alliance:25|t";
                                        msg += GetNameLink(player);
                                        msg += " |cfffaeb00";
                                }
 
                                else
                                {
                                        msg += "|cff00ff00[World]";
                                        msg += "|cffff0000|TInterface\\pvpframe\\PVPCurrency-Honor-Horde:25|t";
                                        msg += GetNameLink(player);
                                        msg += " |cfffaeb00";
                                }
                                break;
                        // Moderator/trial
                        case SEC_MODERATOR:
                                msg += "|cff00ff00[World] ";
                                msg += "|cffff8a00[Mod]";
                                msg += "|TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t";
                                msg += GetNameLink(player);
                                msg += " |cfffaeb00";
                                break;
                        // GM
                        case SEC_GAMEMASTER:
                                msg += "|cff00ff00[World] ";
                                msg += "|cff00ffff[GM]";
                                msg += "|TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t";
                                msg += GetNameLink(player);
                                msg += " |cfffaeb00";
                                break;
                        // Senior GM
                        case SEC_ADMINISTRATOR:
                                msg += "|cff00ff00[World] ";
                                msg += "|cff00ffff[Senior GM]";
                                msg += "|TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t";
                                msg += GetNameLink(player);
                                msg += " |cfffaeb00";
                                break;
                        // Consola
                        case SEC_CONSOLE:
                                msg += "|cff00ff00[World] ";
                                msg += "|cff00ffff[Senior GM]";
                                msg += "|TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t";
                                msg += GetNameLink(player);
                                msg += " |cfffaeb00";
                                break;
                        // Admin
                        case 5:
                                msg += "|cff00ff00[World] ";
                                msg += "|cfffa9900[Admin]";
                                msg += "|TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t";
                                msg += GetNameLink(player);
                                msg += " |cfffaeb00";
                                break;
                        // Dev
                        case 6:
                                msg += "|cff00ff00[World] ";
                                msg += "|cffff6666[Dev]";
                                msg += "|TInterface\\icons\\Achievement_Boss_Mutanus_the_Devourer:15|t";
                                msg += GetNameLink(player);
                                msg += " |cfffaeb00";
                                break;
 
                }

                msg += args;
                if (FACTION_SPECIFIC)
                {
                        SessionMap sessions = sWorld->GetAllSessions();
                        for (SessionMap::iterator itr = sessions.begin(); itr != sessions.end(); ++itr)
                                if (Player* plr = itr->second->GetPlayer())
                                        if (plr->GetTeam() == player->GetTeam())
                                                sWorld->SendServerMessage(SERVER_MSG_STRING, msg.c_str(), plr);
                }
                else
                        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.c_str(), 0);  

                return true;
        }
};

void AddSC_cs_world_chat()
{
    new cs_world_chat();
}