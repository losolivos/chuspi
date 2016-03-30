#include "ScriptInfo.hpp"

#include <sstream>
#include <stdexcept>

#include <cstdio>

char const * GetScriptsTableNameByType(ScriptsType type)
{
    switch (type)
    {
        case SCRIPTS_QUEST_END:
            return "quest_end_scripts";
        case SCRIPTS_QUEST_START:
            return "quest_start_scripts";
        case SCRIPTS_SPELL:
            return "spell_scripts";
        case SCRIPTS_GAMEOBJECT:
            return "gameobject_scripts";
        case SCRIPTS_EVENT:
            return "event_scripts";
        case SCRIPTS_WAYPOINT:
            return "waypoint_scripts";
    }

    std::ostringstream ss;
    ss << "GetScriptsTableNameByType: unknown script type " << static_cast<int>(type);

    throw std::logic_error(ss.str());
}

char const * GetScriptCommandName(ScriptCommands command)
{
    switch (command)
    {
        case SCRIPT_COMMAND_TALK:
            return "SCRIPT_COMMAND_TALK";
        case SCRIPT_COMMAND_EMOTE:
            return "SCRIPT_COMMAND_EMOTE";
        case SCRIPT_COMMAND_FIELD_SET:
            return "SCRIPT_COMMAND_FIELD_SET";
        case SCRIPT_COMMAND_MOVE_TO:
            return "SCRIPT_COMMAND_MOVE_TO";
        case SCRIPT_COMMAND_FLAG_SET:
            return "SCRIPT_COMMAND_FLAG_SET";
        case SCRIPT_COMMAND_FLAG_REMOVE:
            return "SCRIPT_COMMAND_FLAG_REMOVE";
        case SCRIPT_COMMAND_TELEPORT_TO:
            return "SCRIPT_COMMAND_TELEPORT_TO";
        case SCRIPT_COMMAND_QUEST_EXPLORED:
            return "SCRIPT_COMMAND_QUEST_EXPLORED";
        case SCRIPT_COMMAND_KILL_CREDIT:
            return "SCRIPT_COMMAND_KILL_CREDIT";
        case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            return "SCRIPT_COMMAND_RESPAWN_GAMEOBJECT";
        case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
            return "SCRIPT_COMMAND_TEMP_SUMMON_CREATURE";
        case SCRIPT_COMMAND_OPEN_DOOR:
            return "SCRIPT_COMMAND_OPEN_DOOR";
        case SCRIPT_COMMAND_CLOSE_DOOR:
            return "SCRIPT_COMMAND_CLOSE_DOOR";
        case SCRIPT_COMMAND_ACTIVATE_OBJECT:
            return "SCRIPT_COMMAND_ACTIVATE_OBJECT";
        case SCRIPT_COMMAND_REMOVE_AURA:
            return "SCRIPT_COMMAND_REMOVE_AURA";
        case SCRIPT_COMMAND_CAST_SPELL:
            return "SCRIPT_COMMAND_CAST_SPELL";
        case SCRIPT_COMMAND_PLAY_SOUND:
            return "SCRIPT_COMMAND_PLAY_SOUND";
        case SCRIPT_COMMAND_CREATE_ITEM:
            return "SCRIPT_COMMAND_CREATE_ITEM";
        case SCRIPT_COMMAND_DESPAWN_SELF:
            return "SCRIPT_COMMAND_DESPAWN_SELF";
        case SCRIPT_COMMAND_LOAD_PATH:
            return "SCRIPT_COMMAND_LOAD_PATH";
        case SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT:
            return "SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT";
        case SCRIPT_COMMAND_KILL:
            return "SCRIPT_COMMAND_KILL";
        case SCRIPT_COMMAND_ORIENTATION:
            return "SCRIPT_COMMAND_ORIENTATION";
        case SCRIPT_COMMAND_EQUIP:
            return "SCRIPT_COMMAND_EQUIP";
        case SCRIPT_COMMAND_MODEL:
            return "SCRIPT_COMMAND_MODEL";
        case SCRIPT_COMMAND_CLOSE_GOSSIP:
            return "SCRIPT_COMMAND_CLOSE_GOSSIP";
        case SCRIPT_COMMAND_PLAYMOVIE:
            return "SCRIPT_COMMAND_PLAYMOVIE";
    }

    std::ostringstream ss;
    ss << "GetScriptCommandName: unknown script command " << static_cast<int>(command);

    throw std::logic_error(ss.str());
}

std::string ScriptInfo::GetDebugInfo() const
{
    char sz[256];
    std::sprintf(sz, "%s ('%s' script id: %u)", GetScriptCommandName(command), GetScriptsTableNameByType(type), id);
    return std::string(sz);
}
