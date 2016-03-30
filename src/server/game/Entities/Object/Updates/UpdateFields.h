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

#ifndef _UPDATEFIELDS_H
#define _UPDATEFIELDS_H

// Updatefields auto generated for version 5.0.4.16016
enum EObjectFields
{
    OBJECT_FIELD_GUID                                = 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    OBJECT_FIELD_DATA                                = 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    OBJECT_FIELD_TYPE                                = 0x0004, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    OBJECT_FIELD_ENTRY                               = 0x0005, // Size: 1, Type: INT, Flags: PUBLIC
    OBJECT_FIELD_DYNAMIC_FLAGS                       = 0x0006, // Size: 1
    OBJECT_FIELD_SCALE_X                             = 0x0007, // Size: 1, Type: FLOAT, Flags: PUBLIC
    OBJECT_END                                       = 0x0008
};

enum EItemFields
{
    ITEM_FIELD_OWNER                                 = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_CONTAINED                             = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_CREATOR                               = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_GIFTCREATOR                           = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_STACK_COUNT                           = OBJECT_END + 0x0008, // Size: 1, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_DURATION                              = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_SPELL_CHARGES                         = OBJECT_END + 0x000A, // Size: 5, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_FLAGS                                 = OBJECT_END + 0x000F, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_1_1                       = OBJECT_END + 0x0010, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_1_3                       = OBJECT_END + 0x0012, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_2_1                       = OBJECT_END + 0x0013, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_2_3                       = OBJECT_END + 0x0015, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_3_1                       = OBJECT_END + 0x0016, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_3_3                       = OBJECT_END + 0x0018, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_4_1                       = OBJECT_END + 0x0019, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_4_3                       = OBJECT_END + 0x001B, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_5_1                       = OBJECT_END + 0x001C, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_5_3                       = OBJECT_END + 0x001E, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_6_1                       = OBJECT_END + 0x001F, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_6_3                       = OBJECT_END + 0x0021, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_7_1                       = OBJECT_END + 0x0022, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_7_3                       = OBJECT_END + 0x0024, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_8_1                       = OBJECT_END + 0x0025, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_8_3                       = OBJECT_END + 0x0027, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_9_1                       = OBJECT_END + 0x0028, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_9_3                       = OBJECT_END + 0x002A, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_10_1                      = OBJECT_END + 0x002B, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_10_3                      = OBJECT_END + 0x002D, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_11_1                      = OBJECT_END + 0x002E, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_11_3                      = OBJECT_END + 0x0030, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_12_1                      = OBJECT_END + 0x0031, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_12_3                      = OBJECT_END + 0x0033, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_13_1                      = OBJECT_END + 0x0034, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_13_3                      = OBJECT_END + 0x0036, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_PROPERTY_SEED                         = OBJECT_END + 0x0037, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_RANDOM_PROPERTIES_ID                  = OBJECT_END + 0x0038, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_DURABILITY                            = OBJECT_END + 0x0039, // Size: 1, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_MAXDURABILITY                         = OBJECT_END + 0x003A, // Size: 1, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_CREATE_PLAYED_TIME                    = OBJECT_END + 0x003B, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_MODIFIERS_MASK                        = OBJECT_END + 0x003C,
    ITEM_END                                         = OBJECT_END + 0x003D,
};

enum EItemDynamicFields
{
    ITEM_DYNAMIC_MODIFIERS                           = 0x0000,
    ITEM_DYNAMIC_END                                 = 0x0048
};

enum EContainerFields
{
    CONTAINER_FIELD_SLOT_1                           = ITEM_END + 0x0000, // Size: 72, Type: LONG, Flags: PUBLIC
    CONTAINER_FIELD_NUM_SLOTS                        = ITEM_END + 0x0048, // Size: 1, Type: INT, Flags: PUBLIC
    CONTAINER_END                                    = ITEM_END + 0x0049
};

enum EUnitFields
{
    UNIT_FIELD_CHARM                                 = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_SUMMON                                = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CRITTER                               = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PRIVATE
    UNIT_FIELD_CHARMEDBY                             = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_SUMMONEDBY                            = OBJECT_END + 0x0008, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CREATEDBY                             = OBJECT_END + 0x000A, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_DEMON_CREATOR                         = OBJECT_END + 0x000C, // Size: 2,  Type: LONG, Flags: PUBLIC
    UNIT_FIELD_TARGET                                = OBJECT_END + 0x000E,
    UNIT_FIELD_BATTLEPETCOMPANIONGUID                = OBJECT_END + 0x0010, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CHANNEL_OBJECT                        = OBJECT_END + 0x0012, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_CHANNEL_SPELL                               = OBJECT_END + 0x0014, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FILED_SUMMONED_BY_HOME_REALM                = OBJECT_END + 0x0015, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BYTES_0                               = OBJECT_END + 0x0016, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_DISPLAY_POWER                         = OBJECT_END + 0x0017, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_OVERRIDE_DISPLAY_POWER_ID                   = OBJECT_END + 0x0018, // Size: 1, Type: Flags PUBLIC
    UNIT_FIELD_HEALTH                                = OBJECT_END + 0x0019, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER1                                = OBJECT_END + 0x001A, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER2                                = OBJECT_END + 0x001B, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER3                                = OBJECT_END + 0x001C, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER4                                = OBJECT_END + 0x001D, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER5                                = OBJECT_END + 0x001E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXHEALTH                             = OBJECT_END + 0x001F, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER1                             = OBJECT_END + 0x0020, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER2                             = OBJECT_END + 0x0021, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER3                             = OBJECT_END + 0x0022, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER4                             = OBJECT_END + 0x0023, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER5                             = OBJECT_END + 0x0024, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER             = OBJECT_END + 0x0025, // Size: 5, Type: FLOAT, Flags: PRIVATE, OWNER, UNUSED2
    UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER = OBJECT_END + 0x002A, // Size: 5, Type: FLOAT, Flags: PRIVATE, OWNER, UNUSED2
    UNIT_FIELD_LEVEL                                 = OBJECT_END + 0x002F, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_EFFECTIVE_LEVEL                       = OBJECT_END + 0x0030, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FACTIONTEMPLATE                       = OBJECT_END + 0x0031, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_VIRTUAL_ITEM_SLOT_ID                        = OBJECT_END + 0x0032, // Size: 3, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FLAGS                                 = OBJECT_END + 0x0035, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FLAGS_2                               = OBJECT_END + 0x0036, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_AURASTATE                             = OBJECT_END + 0x0037, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BASEATTACKTIME                        = OBJECT_END + 0x0038, // Size: 2, Type: INT, Flags: PUBLIC
    UNIT_FIELD_RANGEDATTACKTIME                      = OBJECT_END + 0x003A, // Size: 1, Type: INT, Flags: PRIVATE
    UNIT_FIELD_BOUNDING_RADIUS                       = OBJECT_END + 0x003B, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_COMBAT_REACH                           = OBJECT_END + 0x003C, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_DISPLAYID                             = OBJECT_END + 0x003D, // Size: 1, Type: INT, Flags: DYNAMIC
    UNIT_FIELD_NATIVEDISPLAYID                       = OBJECT_END + 0x003E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MOUNTDISPLAYID                        = OBJECT_END + 0x003F, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MINDAMAGE                             = OBJECT_END + 0x0040, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_MAXDAMAGE                             = OBJECT_END + 0x0041, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_MINOFFHANDDAMAGE                      = OBJECT_END + 0x0042, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_MAXOFFHANDDAMAGE                      = OBJECT_END + 0x0043, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_BYTES_1                               = OBJECT_END + 0x0044, // Size: 1, Type: BYTES, Flags: PUBLIC  UNIT_FIELD_BYTES_1 ?/
    UNIT_FIELD_PETNUMBER                             = OBJECT_END + 0x0045, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PET_NAME_TIMESTAMP                    = OBJECT_END + 0x0046, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PETEXPERIENCE                         = OBJECT_END + 0x0047, // Size: 1, Type: INT, Flags: OWNER
    UNIT_FIELD_PETNEXTLEVELEXP                       = OBJECT_END + 0x0048, // Size: 1, Type: INT, Flags: OWNER
    UNIT_MOD_CAST_SPEED                              = OBJECT_END + 0x0049, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_MOD_CAST_HASTE                              = OBJECT_END + 0x004A, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_MOD_HASTE                                   = OBJECT_END + 0x004B,
    UNIT_FIELD_MOD_RANGED_HASTE                      = OBJECT_END + 0x004C, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_MOD_HASTE_REGEN                             = OBJECT_END + 0x004D,
    UNIT_CREATED_BY_SPELL                            = OBJECT_END + 0x004E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_NPC_FLAGS                                   = OBJECT_END + 0x004F, // Size: 2, Type: INT, Flags: DYNAMIC
    UNIT_NPC_EMOTESTATE                              = OBJECT_END + 0x0051, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_STAT0                                 = OBJECT_END + 0x0052, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_STAT1                                 = OBJECT_END + 0x0053, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_STAT2                                 = OBJECT_END + 0x0054, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_STAT3                                 = OBJECT_END + 0x0055, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_STAT4                                 = OBJECT_END + 0x0056, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POSSTAT0                              = OBJECT_END + 0x0057, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POSSTAT1                              = OBJECT_END + 0x0058, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POSSTAT2                              = OBJECT_END + 0x0059, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POSSTAT3                              = OBJECT_END + 0x005A, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POSSTAT4                              = OBJECT_END + 0x005B, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_NEGSTAT0                              = OBJECT_END + 0x005C, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_NEGSTAT1                              = OBJECT_END + 0x005D, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_NEGSTAT2                              = OBJECT_END + 0x005E, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_NEGSTAT3                              = OBJECT_END + 0x005F, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_NEGSTAT4                              = OBJECT_END + 0x0060, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RESISTANCES                           = OBJECT_END + 0x0061, // Size: 7, Type: INT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE            = OBJECT_END + 0x0068, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE            = OBJECT_END + 0x006F, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_BASE_MANA                             = OBJECT_END + 0x0076, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BASE_HEALTH                           = OBJECT_END + 0x0077, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_BYTES_2                               = OBJECT_END + 0x0078, // Size: 1, Type: BYTES, Flags: PUBLIC      UNIT_FIELD_SHAPESHIFT_FORM??
    UNIT_FIELD_ATTACK_POWER                          = OBJECT_END + 0x0079, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MOD_POS                  = OBJECT_END + 0x007A, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MOD_NEG                  = OBJECT_END + 0x007B, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MULTIPLIER               = OBJECT_END + 0x007C, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER                   = OBJECT_END + 0x007D, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS           = OBJECT_END + 0x007E, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG           = OBJECT_END + 0x007F, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER        = OBJECT_END + 0x0080, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MINRANGEDDAMAGE                       = OBJECT_END + 0x0081, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MAXRANGEDDAMAGE                       = OBJECT_END + 0x0082, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POWER_COST_MODIFIER                   = OBJECT_END + 0x0083, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POWER_COST_MULTIPLIER                 = OBJECT_END + 0x008A, // Size: 7, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MAXHEALTHMODIFIER                     = OBJECT_END + 0x0091, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_HOVERHEIGHT                           = OBJECT_END + 0x0092, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_MIN_ITEM_LEVEL                        = OBJECT_END + 0x0093,
    UNIT_FIELD_MAXITEMLEVEL                          = OBJECT_END + 0x0094, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_WILD_BATTLE_PET_LEVEL                 = OBJECT_END + 0x0095,
    UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP   = OBJECT_END + 0x0096, // size 1, flags MIRROR_ALL
    UNIT_FIELD_INTERACT_SPELL_ID                     = OBJECT_END + 0x0097, // size 1, flags MIRROR_ALL
    UNIT_END                                         = OBJECT_END + 0x0098
};

enum EUnitDynamicField
{
    UNIT_DYNAMIC_FIELD_PASSIVE_SPELLS                = 0x0000,
    UNIT_DYNAMIC_END                                 = 0x0101
};

enum EPlayerFields
{
    PLAYER_DUEL_ARBITER                              = UNIT_END + 0x0000, // SIZE:2
    PLAYER_FLAGS                                     = UNIT_END + 0x0002, // SIZE:1
    PLAYER_GUILDRANK                                 = UNIT_END + 0x0003, // SIZE:1
    PLAYER_GUILDDELETE_DATE                          = UNIT_END + 0x0004, // SIZE:1
    PLAYER_GUILDLEVEL                                = UNIT_END + 0x0005, // SIZE:1
    PLAYER_BYTES                                     = UNIT_END + 0x0006, // SIZE:1
    PLAYER_BYTES_2                                   = UNIT_END + 0x0007, // SIZE:1
    PLAYER_BYTES_3                                   = UNIT_END + 0x0008, // SIZE:1
    PLAYER_DUEL_TEAM                                 = UNIT_END + 0x0009, // SIZE:1
    PLAYER_GUILD_TIMESTAMP                           = UNIT_END + 0x000A, // SIZE:1
    PLAYER_QUEST_LOG_1_1                             = UNIT_END + 0x000B, // SIZE:750
    PLAYER_VISIBLE_ITEM_1_ENTRYID                    = UNIT_END + 0x02F9,
    PLAYER_VISIBLE_ITEM_1_ENCHANTMENT                = UNIT_END + 0x02FA,
    PLAYER_VISIBLE_ITEM_2_ENTRYID                    = UNIT_END + 0x02FB,
    PLAYER_VISIBLE_ITEM_2_ENCHANTMENT                = UNIT_END + 0x02FC,
    PLAYER_VISIBLE_ITEM_3_ENTRYID                    = UNIT_END + 0x02FD,
    PLAYER_VISIBLE_ITEM_3_ENCHANTMENT                = UNIT_END + 0x02FE,
    PLAYER_VISIBLE_ITEM_4_ENTRYID                    = UNIT_END + 0x02FF,
    PLAYER_VISIBLE_ITEM_4_ENCHANTMENT                = UNIT_END + 0x0300,
    PLAYER_VISIBLE_ITEM_5_ENTRYID                    = UNIT_END + 0x0301,
    PLAYER_VISIBLE_ITEM_5_ENCHANTMENT                = UNIT_END + 0x0302,
    PLAYER_VISIBLE_ITEM_6_ENTRYID                    = UNIT_END + 0x0303,
    PLAYER_VISIBLE_ITEM_6_ENCHANTMENT                = UNIT_END + 0x0304,
    PLAYER_VISIBLE_ITEM_7_ENTRYID                    = UNIT_END + 0x0305,
    PLAYER_VISIBLE_ITEM_7_ENCHANTMENT                = UNIT_END + 0x0306,
    PLAYER_VISIBLE_ITEM_8_ENTRYID                    = UNIT_END + 0x0307,
    PLAYER_VISIBLE_ITEM_8_ENCHANTMENT                = UNIT_END + 0x0308,
    PLAYER_VISIBLE_ITEM_9_ENTRYID                    = UNIT_END + 0x0309,
    PLAYER_VISIBLE_ITEM_9_ENCHANTMENT                = UNIT_END + 0x030A,
    PLAYER_VISIBLE_ITEM_10_ENTRYID                   = UNIT_END + 0x030B,
    PLAYER_VISIBLE_ITEM_10_ENCHANTMENT               = UNIT_END + 0x030C,
    PLAYER_VISIBLE_ITEM_11_ENTRYID                   = UNIT_END + 0x030D,
    PLAYER_VISIBLE_ITEM_11_ENCHANTMENT               = UNIT_END + 0x030E,
    PLAYER_VISIBLE_ITEM_12_ENTRYID                   = UNIT_END + 0x030F,
    PLAYER_VISIBLE_ITEM_12_ENCHANTMENT               = UNIT_END + 0x0310,
    PLAYER_VISIBLE_ITEM_13_ENTRYID                   = UNIT_END + 0x0311,
    PLAYER_VISIBLE_ITEM_13_ENCHANTMENT               = UNIT_END + 0x0312,
    PLAYER_VISIBLE_ITEM_14_ENTRYID                   = UNIT_END + 0x0313,
    PLAYER_VISIBLE_ITEM_14_ENCHANTMENT               = UNIT_END + 0x0314,
    PLAYER_VISIBLE_ITEM_15_ENTRYID                   = UNIT_END + 0x0315,
    PLAYER_VISIBLE_ITEM_15_ENCHANTMENT               = UNIT_END + 0x0316,
    PLAYER_VISIBLE_ITEM_16_ENTRYID                   = UNIT_END + 0x0317,
    PLAYER_VISIBLE_ITEM_16_ENCHANTMENT               = UNIT_END + 0x0318,
    PLAYER_VISIBLE_ITEM_17_ENTRYID                   = UNIT_END + 0x0319,
    PLAYER_VISIBLE_ITEM_17_ENCHANTMENT               = UNIT_END + 0x031A,
    PLAYER_VISIBLE_ITEM_18_ENTRYID                   = UNIT_END + 0x031B,
    PLAYER_VISIBLE_ITEM_18_ENCHANTMENT               = UNIT_END + 0x031C,
    PLAYER_VISIBLE_ITEM_19_ENTRYID                   = UNIT_END + 0x031D,
    PLAYER_VISIBLE_ITEM_19_ENCHANTMENT               = UNIT_END + 0x031E,
    PLAYER_CHOSEN_TITLE                              = UNIT_END + 0x031F, // SIZE:1
    PLAYER_FAKE_INEBRIATION                          = UNIT_END + 0x0320, // SIZE:1
    PLAYER_FIELD_VIRTUAL_PLAYER_REALM                = UNIT_END + 0x0321, // size 1
    PLAYER_CURRENT_SPEC_ID                           = UNIT_END + 0x0322, // SIZE:1
    PLAYER_FIELD_TAXI_MOUNT_ANIM_KIT_ID              = UNIT_END + 0x0323, // SIZE:1
    PLAYER_FIELD_CURRENT_BATTLE_PET_BREED_QUALITY    = UNIT_END + 0x0324, // SIZE:1
    PLAYER_FIELD_INV_SLOT_HEAD                       = UNIT_END + 0x0325, // SIZE:172
    PLAYER_FIELD_VENDORBUYBACK_SLOT_1                = UNIT_END + 0x03B9,
    PLAYER_FARSIGHT                                  = UNIT_END + 0x03D1, // SIZE:2
    PLAYER_FIELD_KNOWN_TITLES                        = UNIT_END + 0x03D3, // SIZE:10
    PLAYER_FIELD_COINAGE                             = UNIT_END + 0x03DD, // SIZE:2
    PLAYER_XP                                        = UNIT_END + 0x03DF, // SIZE:1
    PLAYER_NEXT_LEVEL_XP                             = UNIT_END + 0x03E0, // SIZE:1
    PLAYER_FIELD_SKILL                               = UNIT_END + 0x03E1, // SIZE:64
    PLAYER_CHARACTER_POINTS                          = UNIT_END + 0x05A1, // SIZE:1
    PLAYER_MAX_TALENT_TIERS                          = 1602, // SIZE:1
    PLAYER_TRACK_CREATURES                           = 1603, // SIZE:1
    PLAYER_TRACK_RESOURCES                           = 1604, // SIZE:1
    PLAYER_EXPERTISE                                 = 1605, // SIZE:1
    PLAYER_OFFHAND_EXPERTISE                         = 1606, // SIZE:1
    PLAYER_RANGED_EXPERTISE                          = 1607, // SIZE:1
    PLAYER_COMBAT_RATING_EXPERTISE                   = 1608, // SIZE:1
    PLAYER_BLOCK_PERCENTAGE                          = 1609, // SIZE:1
    PLAYER_DODGE_PERCENTAGE                          = 1610, // SIZE:1
    PLAYER_PARRY_PERCENTAGE                          = 1611, // SIZE:1
    PLAYER_CRIT_PERCENTAGE                           = 1612, // SIZE:1
    PLAYER_RANGED_CRIT_PERCENTAGE                    = 1613, // SIZE:1
    PLAYER_OFFHAND_CRIT_PERCENTAGE                   = 1614, // SIZE:1
    PLAYER_SPELL_CRIT_PERCENTAGE1                    = 1615, // SIZE:7
    PLAYER_SHIELD_BLOCK                              = 1622, // SIZE:1
    PLAYER_SHIELD_BLOCK_CRIT_PERCENTAGE              = 1623, // SIZE:1
    PLAYER_MASTERY                                   = 1624, // SIZE:1
    PLAYER_FIELD_PVP_POWER_DAMAGE                    = 1625, // SIZE:1
    PLAYER_FIELD_PVP_POWER_HEALING                   = 1626, // SIZE:1
    PLAYER_EXPLORED_ZONES_1                          = 1627, // SIZE:200
    PLAYER_FIELD_REST_STATE_BONUS_POOL               = 1827, // SIZE:1
    PLAYER_FIELD_MOD_DAMAGE_DONE_POS                 = 1828, // SIZE:7
    PLAYER_FIELD_MOD_DAMAGE_DONE_NEG                 = 1835, // SIZE:7
    PLAYER_FIELD_MOD_DAMAGE_DONE_PCT                 = 1842, // SIZE:7
    PLAYER_FIELD_MOD_HEALING_DONE_POS                = 1849, // SIZE:1
    PLAYER_FIELD_MOD_HEALING_PCT                     = 1850, // SIZE:1
    PLAYER_FIELD_MOD_HEALING_DONE_PCT                = 1851, // SIZE:1
    PLAYER_FIELD_MOD_PERIODIC_HEALING_DONE_PERCENT   = 1852, // SIZE:1
    PLAYER_FIELD_WEAPON_DMG_MULTIPLIERS              = 1853, // SIZE:3
    PLAYER_FIELD_MOD_SPELL_POWER_PCT                 = 1856, // SIZE:1
    PLAYER_FIELD_MOD_RESILIENCE_PCT                  = 1857, // SIZE:1
    PLAYER_FIELD_OVERRIDE_SPELL_POWER_BY_AP_PCT      = 1858, // SIZE:1
    PLAYER_FIELD_OVERRIDE_AP_BY_SPELL_POWER_PCT      = 1859, // SIZE:1
    PLAYER_FIELD_MOD_TARGET_RESISTANCE               = 1860, // SIZE:1
    PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE      = 1861, // SIZE:1
    PLAYER_FIELD_BYTES                               = 1862, // SIZE:1
    PLAYER_SELF_RES_SPELL                            = 1863, // SIZE:1
    PLAYER_FIELD_PVP_MEDALS                          = 1864, // SIZE:1
    PLAYER_FIELD_BUYBACK_PRICE_1                     = 1865, // SIZE:12
    PLAYER_FIELD_BUYBACK_TIMESTAMP_1                 = 1877, // SIZE:12
    PLAYER_FIELD_KILLS                               = 1889, // SIZE:1
    PLAYER_FIELD_LIFETIME_HONORABLE_KILLS            = 1890, // SIZE:1
    PLAYER_FIELD_WATCHED_FACTION_INDEX               = 1891, // SIZE:1
    PLAYER_FIELD_COMBAT_RATING_1                     = 1892, // SIZE:27
    PLAYER_FIELD_PVP_INFO                            = 1919, // SIZE:24
    PLAYER_FIELD_MAX_LEVEL                           = 1943, // SIZE:1
    PLAYER_RUNE_REGEN_1                              = 1944, // SIZE:4
    PLAYER_NO_REAGENT_COST_1                         = 1948, // SIZE:4
    PLAYER_FIELD_GLYPH_SLOTS_1                       = 1952, // SIZE:6
    PLAYER_FIELD_GLYPHS_1                            = 1958, // SIZE:6
    PLAYER_GLYPHS_ENABLED                            = 1964, // SIZE:1
    PLAYER_PET_SPELL_POWER                           = 1965, // SIZE:1
    PLAYER_FIELD_RESEARCHING_1                       = 1966, // SIZE:8
    PLAYER_PROFESSION_SKILL_LINE_1                   = 1974, // SIZE:2
    PLAYER_FIELD_UI_HIT_MODIFIER                     = 1976, // SIZE:1
    PLAYER_FIELD_UI_SPELL_HIT_MODIFIER               = 1977, // SIZE:1
    PLAYER_FIELD_HOME_REALM_TIME_OFFSET              = 1978, // SIZE:1
    PLAYER_FIELD_MOD_PET_HASTE                       = 1979, // SIZE:1
    PLAYER_FIELD_SUMMONED_BATTLE_PET_GUID            = 1980, // SIZE:2
    PLAYER_FIELD_OVERRIDE_SPELLS_ID                  = 1982, // SIZE:1
    PLAYER_FIELD_LFG_BONUS_FACTION_ID                = 1983, // SIZE:1
    PLAYER_FIELD_LOOT_SPEC_ID                        = 1984, // SIZE:1
    PLAYER_FIELD_OVERRIDE_ZONE_PVP_TYPE              = 1985, // SIZE:1
    PLAYER_FIELD_ITEM_LEVEL_DELTA                    = 1986, // SIZE:1
    PLAYER_END                                       = 1987
};

enum EPlayerDynamicFields
{
    PLAYER_DYNAMIC_RESEARCH_SITES                    = PLAYER_END + 0x0000,
    PLAYER_DYNAMIC_DAILY_QUESTS_COMPLETED            = PLAYER_END + 0x0002,
    PLAYER_DYNAMIC_END                               = PLAYER_END + 0x0004
};

enum EGameObjectFields
{
    OBJECT_FIELD_CREATED_BY                          = OBJECT_END + 0x0,
    GAMEOBJECT_DISPLAYID                             = OBJECT_END + 0x2,
    GAMEOBJECT_FLAGS                                 = OBJECT_END + 0x3,
    GAMEOBJECT_PARENTROTATION                        = OBJECT_END + 0x4,
    GAMEOBJECT_FACTION                               = OBJECT_END + 0x8,
    GAMEOBJECT_LEVEL                                 = OBJECT_END + 0x9,
    GAMEOBJECT_BYTES_1                               = OBJECT_END + 0xA,
    GAMEOBJECT_FIELD_ANIM_PROGRESS                   = OBJECT_END + 0xB,
    GAMEOBJECT_END                                   = OBJECT_END + 0xC
};

enum EDynamicObjectFields
{
    DYNAMICOBJECT_CASTER                             = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    DYNAMICOBJECT_BYTES                              = OBJECT_END + 0x0002, // Size: 1, Type: INT, Flags: DYNAMIC //DYNAMICOBJECT_BYTES
    DYNAMICOBJECT_SPELLID                            = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    DYNAMICOBJECT_RADIUS                             = OBJECT_END + 0x0004, // Size: 1, Type: FLOAT, Flags: PUBLIC
    DYNAMICOBJECT_CASTTIME                           = OBJECT_END + 0x0005, // Size: 1, Type: INT, Flags: PUBLIC
    DYNAMICOBJECT_END                                = OBJECT_END + 0x0006
};

enum ECorpseFields
{
    CORPSE_FIELD_OWNER                               = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    CORPSE_FIELD_PARTY                               = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    CORPSE_FIELD_DISPLAY_ID                          = OBJECT_END + 0x0004, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_ITEM                                = OBJECT_END + 0x0005, // Size: 19, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_BYTES_1                             = OBJECT_END + 0x0018, // Size: 1, Type: BYTES, Flags: PUBLIC //CORPSE_FIELD_SKINID
    CORPSE_FIELD_BYTES_2                             = OBJECT_END + 0x0019, // Size: 1, Type: BYTES, Flags: PUBLIC //CORPSE_FIELD_FACIAL_HAIR_STYLE_ID
    CORPSE_FIELD_FLAGS                               = OBJECT_END + 0x001A, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_DYNAMIC_FLAGS                       = OBJECT_END + 0x001B, // Size: 1, Type: INT, Flags: DYNAMIC
    CORPSE_END                                       = OBJECT_END + 0x001C
};

enum EAreaTriggerFields
{
    AREATRIGGER_CASTER                               = OBJECT_END + 0x0000,
    AREATRIGGER_DURATION                             = OBJECT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    AREATRIGGER_SPELLID                              = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    AREATRIGGER_SPELLVISUALID                        = OBJECT_END + 0x0004, // Size: 1, Type: INT, Flags: PUBLIC
    AREATRIGGER_FIELD_EXPLICIT_SCALE                 = OBJECT_END + 0x0005, // Size: 1, Type: INT, Flags: PUBLIC
    AREATRIGGER_END                                  = OBJECT_END + 0x0006
};

enum ESceneObjectFields
{
    SCENE_SCRIPT_PACKAGE_ID                          = OBJECT_END + 0x0000, // SIZE:1
    SCENE_RND_SEED_VAL                               = OBJECT_END + 0x0001, // SIZE:1
    SCENE_CREATE_BY                                  = OBJECT_END + 0x0002, // SIZE:2
    SCENE_TYPE                                       = OBJECT_END + 0x0004, // SIZE:1
    SCENE_END                                        = OBJECT_END + 0x0005
};

#endif // _UPDATEFIELDS_H
