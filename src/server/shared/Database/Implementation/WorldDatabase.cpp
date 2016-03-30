/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "WorldDatabase.h"
#include "MySQLConnection.h"

namespace WorldDatabaseConnection {

void DoPrepareStatements(MySQLConnection &conn)
{
    if (!conn.reconnecting())
        conn.setMaxPreparedStatements(MAX_WORLDDATABASE_STATEMENTS);

    conn.prepareStatement(WORLD_SEL_QUEST_POOLS, "SELECT entry, pool_entry FROM pool_quest");
    conn.prepareStatement(WORLD_DEL_CRELINKED_RESPAWN, "DELETE FROM linked_respawn WHERE guid = ?");
    conn.prepareStatement(WORLD_REP_CREATURE_LINKED_RESPAWN, "REPLACE INTO linked_respawn (guid, linkedGuid) VALUES (?, ?)");
    conn.prepareStatement(WORLD_SEL_CREATURE_TEXT, "SELECT entry, groupid, id, text, type, language, probability, emote, duration, sound FROM creature_text");
    conn.prepareStatement(WORLD_SEL_SMART_SCRIPTS, "SELECT entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o FROM smart_scripts ORDER BY entryorguid, source_type, id, link");
    conn.prepareStatement(WORLD_SEL_SMARTAI_WP, "SELECT entry, pointid, position_x, position_y, position_z FROM waypoints ORDER BY entry, pointid");
    conn.prepareStatement(WORLD_DEL_GAMEOBJECT, "DELETE FROM gameobject WHERE guid = ?");
    conn.prepareStatement(WORLD_DEL_EVENT_GAMEOBJECT, "DELETE FROM game_event_gameobject WHERE guid = ?");
    conn.prepareStatement(WORLD_INS_GRAVEYARD_ZONE, "INSERT INTO game_graveyard_zone (id, ghost_zone, faction) VALUES (?, ?, ?)");
    conn.prepareStatement(WORLD_DEL_GRAVEYARD_ZONE, "DELETE FROM game_graveyard_zone WHERE id = ? AND ghost_zone = ? AND faction = ?");
    conn.prepareStatement(WORLD_INS_GAME_TELE, "INSERT INTO game_tele (id, position_x, position_y, position_z, orientation, map, name) VALUES (?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(WORLD_DEL_GAME_TELE, "DELETE FROM game_tele WHERE name = ?");
    conn.prepareStatement(WORLD_INS_NPC_VENDOR, "INSERT INTO npc_vendor (entry, item, maxcount, incrtime, extendedcost, type) VALUES(?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(WORLD_DEL_NPC_VENDOR, "DELETE FROM npc_vendor WHERE entry = ? AND item = ? AND type = ?");
    conn.prepareStatement(WORLD_SEL_NPC_VENDOR_REF, "SELECT item, maxcount, incrtime, ExtendedCost, type FROM npc_vendor WHERE entry = ? AND type = ? ORDER BY slot ASC");
    conn.prepareStatement(WORLD_UPD_CREATURE_MOVEMENT_TYPE, "UPDATE creature SET MovementType = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_UPD_CREATURE_FACTION, "UPDATE creature_template SET faction_A = ?, faction_H = ? WHERE entry = ?");
    conn.prepareStatement(WORLD_UPD_CREATURE_NPCFLAG, "UPDATE creature_template SET npcflag = ? WHERE entry = ?");
    conn.prepareStatement(WORLD_UPD_CREATURE_NPCFLAG2, "UPDATE creature_template SET npcflag2 = ? WHERE entry = ?");
    conn.prepareStatement(WORLD_UPD_CREATURE_POSITION, "UPDATE creature SET position_x = ?, position_y = ?, position_z = ?, orientation = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_UPD_CREATURE_SPAWN_DISTANCE, "UPDATE creature SET spawndist = ?, MovementType = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_UPD_CREATURE_SPAWN_TIME_SECS, "UPDATE creature SET spawntimesecs = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_INS_CREATURE_FORMATION, "INSERT INTO creature_formations (leaderGUID, memberGUID, dist, angle, groupAI) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(WORLD_INS_WAYPOINT_DATA, "INSERT INTO waypoint_data (id, point, position_x, position_y, position_z) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(WORLD_DEL_WAYPOINT_DATA, "DELETE FROM waypoint_data WHERE id = ? AND point = ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_DATA_POINT, "UPDATE waypoint_data SET point = point - 1 WHERE id = ? AND point > ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_DATA_POSITION, "UPDATE waypoint_data SET position_x = ?, position_y = ?, position_z = ? where id = ? AND point = ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_DATA_WPGUID, "UPDATE waypoint_data SET wpguid = ? WHERE id = ? and point = ?");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_MAX_ID, "SELECT MAX(id) FROM waypoint_data");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_MAX_POINT, "SELECT MAX(point) FROM waypoint_data WHERE id = ?");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_ID, "SELECT point, position_x, position_y, position_z, orientation, move_flag, delay, action, action_chance FROM waypoint_data WHERE id = ? ORDER BY point");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_BY_ID, "SELECT point, position_x, position_y, position_z FROM waypoint_data WHERE id = ?");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_FIRST_BY_ID, "SELECT position_x, position_y, position_z FROM waypoint_data WHERE point = 1 AND id = ?");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_LAST_BY_ID, "SELECT position_x, position_y, position_z, orientation FROM waypoint_data WHERE id = ? ORDER BY point DESC LIMIT 1");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_WPGUID, "SELECT id, point FROM waypoint_data WHERE wpguid = ?");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_ALL_BY_WPGUID, "SELECT id, point, delay, move_flag, action, action_chance FROM waypoint_data WHERE wpguid = ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_DATA_ALL_WPGUID, "UPDATE waypoint_data SET wpguid = 0");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_POS, "SELECT id, point FROM waypoint_data WHERE (abs(position_x - ?) <= ?) and (abs(position_y - ?) <= ?) and (abs(position_z - ?) <= ?)");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_WPGUID_BY_ID, "SELECT wpguid FROM waypoint_data WHERE id = ? and wpguid <> 0");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_DATA_ACTION, "SELECT DISTINCT action FROM waypoint_data");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_SCRIPTS_MAX_ID, "SELECT MAX(guid) FROM waypoint_scripts");
    conn.prepareStatement(WORLD_INS_CREATURE_PATH, "INSERT INTO creature_path(guid, path) VALUES (?, ?)");
    conn.prepareStatement(WORLD_UPD_CREATURE_PATH, "UPDATE creature_path SET path = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_DEL_CREATURE_PATH, "DELETE FROM creature_path WHERE guid = ?");
    conn.prepareStatement(WORLD_SEL_CREATURE_PATH, "SELECT 1 FROM creature_path WHERE guid = ?");
    conn.prepareStatement(WORLD_INS_WAYPOINT_SCRIPT, "INSERT INTO waypoint_scripts (guid) VALUES (?)");
    conn.prepareStatement(WORLD_DEL_WAYPOINT_SCRIPT, "DELETE FROM waypoint_scripts WHERE guid = ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_ID, "UPDATE waypoint_scripts SET id = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_X, "UPDATE waypoint_scripts SET x = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_Y, "UPDATE waypoint_scripts SET y = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_Z, "UPDATE waypoint_scripts SET z = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_O, "UPDATE waypoint_scripts SET o = ? WHERE guid = ?");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_SCRIPT_ID_BY_GUID, "SELECT id FROM waypoint_scripts WHERE guid = ?");
    conn.prepareStatement(WORLD_DEL_CREATURE, "DELETE FROM creature WHERE guid = ?");
    conn.prepareStatement(WORLD_INS_CREATURE_TRANSPORT, "INSERT INTO creature_transport (npc_entry, transport_entry,  TransOffsetX, TransOffsetY, TransOffsetZ, TransOffsetO) values (?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(WORLD_SEL_COMMANDS, "SELECT name, security, help FROM command");
    conn.prepareStatement(WORLD_SEL_CREATURE_TEMPLATE,
                          "SELECT difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, difficulty_entry_4, difficulty_entry_5, difficulty_entry_6, "
                          "difficulty_entry_7, difficulty_entry_8, difficulty_entry_9, difficulty_entry_10, difficulty_entry_11, difficulty_entry_12, "
                          "difficulty_entry_13, difficulty_entry_14, difficulty_entry_15, KillCredit1, KillCredit2, modelid1, modelid2, modelid3, "
                          "modelid4, name, subname, IconName, gossip_menu_id, minlevel, maxlevel, exp, exp_unk, faction_A, faction_H, npcflag, npcflag2, speed_walk, speed_run, "
                          "speed_fly, scale, rank, mindmg, maxdmg, dmgschool, attackpower, dmg_multiplier, baseattacktime, rangeattacktime, unit_class, unit_flags, unit_flags2, "
                          "dynamicflags, family, trainer_type, trainer_spell, trainer_class, trainer_race, minrangedmg, maxrangedmg, rangedattackpower, type, "
                          "type_flags, type_flags2, lootid, pickpocketloot, skinloot, resistance1, resistance2, resistance3, resistance4, resistance5, resistance6, "
                          "spell1, spell2, spell3, spell4, spell5, spell6, spell7, spell8, PetSpellDataId, VehicleId, mingold, maxgold, AIName, MovementType, "
                          "InhabitType, HoverHeight, Health_mod, Mana_mod, Mana_mod_extra, Armor_mod, RacialLeader, questItem1, questItem2, questItem3, questItem4, questItem5, "
                          "questItem6, movementId, RegenHealth, equipment_id, mechanic_immune_mask, flags_extra, ScriptName "
                          "FROM creature_template WHERE entry = ?");
    conn.prepareStatement(WORLD_SEL_WAYPOINT_SCRIPT_BY_ID, "SELECT guid, delay, command, datalong, datalong2, dataint, x, y, z, o FROM waypoint_scripts WHERE id = ?");
    conn.prepareStatement(WORLD_SEL_ITEM_TEMPLATE_BY_NAME, "SELECT entry FROM item_template WHERE name = ?");
    conn.prepareStatement(WORLD_SEL_CREATURE_BY_ID, "SELECT guid FROM creature WHERE id = ?");
    conn.prepareStatement(WORLD_SEL_GAMEOBJECT_NEAREST, "SELECT guid, id, position_x, position_y, position_z, map, (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) AS order_ FROM gameobject WHERE map = ? AND (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) <= ? ORDER BY order_");
    conn.prepareStatement(WORLD_SEL_CREATURE_NEAREST,   "SELECT guid, id, position_x, position_y, position_z, map, (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) AS order_ FROM creature WHERE map = ? AND (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) <= ? ORDER BY order_");
    conn.prepareStatement(WORLD_INS_CREATURE, "INSERT INTO creature (guid, id, map, spawnMask, phaseMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, npcflag, npcflag2, unit_flags, unit_flags2, dynamicflags, isActive) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(WORLD_DEL_GAME_EVENT_CREATURE, "DELETE FROM game_event_creature WHERE guid = ?");
    conn.prepareStatement(WORLD_DEL_GAME_EVENT_MODEL_EQUIP, "DELETE FROM game_event_model_equip WHERE guid = ?");
    conn.prepareStatement(WORLD_INS_GAMEOBJECT, "INSERT INTO gameobject (guid, id, map, spawnMask, phaseMask, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state, isActive) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(WORLD_INS_DISABLES, "INSERT INTO disables (entry, sourceType, flags, comment) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(WORLD_SEL_DISABLES, "SELECT entry FROM disables WHERE entry = ? AND sourceType = ?");
    conn.prepareStatement(WORLD_DEL_DISABLES, "DELETE FROM disables WHERE entry = ? AND sourceType = ?");
    conn.prepareStatement(WORLD_SEL_BLACKMARKET_TEMPLATE, "SELECT id, itemEntry, itemCount, seller, startBid, duration, chance FROM blackmarket_template");

    conn.prepareStatement(WORLD_SEL_REQ_XP, "SELECT xp_for_next_level FROM player_xp_for_level WHERE lvl = ?");
}

} // namespace WorldDatabaseConnection
