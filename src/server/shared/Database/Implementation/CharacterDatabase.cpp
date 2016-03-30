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

#include "CharacterDatabase.h"
#include "MySQLConnection.h"

namespace CharacterDatabaseConnection {

void DoPrepareStatements(MySQLConnection &conn)
{
    if (!conn.reconnecting())
        conn.setMaxPreparedStatements(MAX_CHARACTERDATABASE_STATEMENTS);

    conn.prepareStatement(CHAR_DEL_QUEST_POOL_SAVE, "DELETE FROM pool_quest_save WHERE pool_id = ?");
    conn.prepareStatement(CHAR_INS_QUEST_POOL_SAVE, "INSERT INTO pool_quest_save (pool_id, quest_id) VALUES (?, ?)");
    conn.prepareStatement(CHAR_DEL_NONEXISTENT_GUILD_BANK_ITEM, "DELETE FROM guild_bank_item WHERE guildid = ? AND TabId = ? AND SlotId = ?");
    conn.prepareStatement(CHAR_DEL_EXPIRED_BANS, "UPDATE character_banned SET active = 0 WHERE unbandate <= UNIX_TIMESTAMP() AND unbandate <> bandate");
    conn.prepareStatement(CHAR_SEL_GUID_BY_NAME, "SELECT guid FROM characters WHERE name = ?");
    conn.prepareStatement(CHAR_SEL_CHECK_NAME, "SELECT 1 FROM characters WHERE name = ?");
    conn.prepareStatement(CHAR_SEL_CHECK_GUID, "SELECT 1 FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_SUM_CHARS, "SELECT COUNT(guid) FROM characters WHERE account = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_CREATE_INFO, "SELECT level, race, class FROM characters WHERE account = ? LIMIT 0, ?");
    conn.prepareStatement(CHAR_INS_CHARACTER_BAN, "INSERT INTO character_banned VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?, 1)");
    conn.prepareStatement(CHAR_UPD_CHARACTER_BAN, "UPDATE character_banned SET active = 0 WHERE guid = ? AND active != 0");
    conn.prepareStatement(CHAR_DEL_CHARACTER_BAN, "DELETE cb FROM character_banned cb INNER JOIN characters c ON c.guid = cb.guid WHERE c.account = ?");
    conn.prepareStatement(CHAR_SEL_BANINFO, "SELECT FROM_UNIXTIME(bandate), unbandate-bandate, active, unbandate, banreason, bannedby FROM character_banned WHERE guid = ? ORDER BY bandate ASC");
    conn.prepareStatement(CHAR_SEL_GUID_BY_NAME_FILTER, "SELECT guid, name FROM characters WHERE name LIKE CONCAT('%%', ?, '%%')");
    conn.prepareStatement(CHAR_SEL_BANINFO_LIST, "SELECT bandate, unbandate, bannedby, banreason FROM character_banned WHERE guid = ? ORDER BY unbandate");
    conn.prepareStatement(CHAR_SEL_BANNED_NAME, "SELECT characters.name FROM characters, character_banned WHERE character_banned.guid = ? AND character_banned.guid = characters.guid");
    conn.prepareStatement(CHAR_SEL_MAIL_LIST_COUNT, "SELECT COUNT(id) FROM mail WHERE receiver = ? ");
    conn.prepareStatement(CHAR_SEL_MAIL_LIST_INFO, "SELECT id, sender, (SELECT name FROM characters WHERE guid = sender) AS sendername, receiver, (SELECT name FROM characters WHERE guid = receiver) AS receivername,"
                     "SUBJECT, deliver_time, expire_time, money, has_items FROM mail WHERE receiver = ? ");
    conn.prepareStatement(CHAR_SEL_MAIL_LIST_ITEMS, "SELECT itemEntry,count FROM item_instance WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_ENUM, "SELECT c.guid, c.name, c.race, c.class, c.gender, c.playerBytes, c.playerBytes2, c.level, c.zone, c.map, c.position_x, c.position_y, c.position_z, "
                          "gm.guildid, c.playerFlags, c.at_login, p.entry, p.model_id, p.level, c.equipmentCache, cb.guid, c.slot "
                          "FROM characters AS c "
                          "LEFT JOIN character_pet AS cp ON c.guid = cp.guid "
                          "LEFT JOIN pet_info AS p ON cp.pet_id = p.id "
                          "LEFT JOIN guild_member AS gm ON c.guid = gm.guid "
                          "LEFT JOIN character_banned AS cb ON c.guid = cb.guid AND cb.active = 1 "
                          "WHERE c.account = ? AND c.deleteInfos_Name IS NULL AND (at_login & 512) <> 512");
    conn.prepareStatement(CHAR_SEL_ENUM_DECLINED_NAME, "SELECT c.guid, c.name, c.race, c.class, c.gender, c.playerBytes, c.playerBytes2, c.level, c.zone, c.map, "
                          "c.position_x, c.position_y, c.position_z, gm.guildid, c.playerFlags, c.at_login, p.entry, p.model_id, p.level, c.equipmentCache, "
                          "cb.guid, c.slot, cd.genitive "
                          "FROM characters AS c "
                          "LEFT JOIN character_pet AS cp ON c.guid = cp.guid "
                          "LEFT JOIN pet_info AS p ON cp.pet_id = p.id "
                          "LEFT JOIN character_declinedname AS cd ON c.guid = cd.guid "
                          "LEFT JOIN guild_member AS gm ON c.guid = gm.guid "
                          "LEFT JOIN character_banned AS cb ON c.guid = cb.guid AND cb.active = 1 "
                          "WHERE c.account = ? AND c.deleteInfos_Name IS NULL");
    conn.prepareStatement(CHAR_SEL_FREE_NAME, "SELECT guid, name FROM characters WHERE guid = ? AND account = ? AND (at_login & ?) = ? AND NOT EXISTS (SELECT NULL FROM characters WHERE name = ?)");
    conn.prepareStatement(CHAR_SEL_GUID_RACE_ACC_BY_NAME, "SELECT guid, race, account FROM characters WHERE name = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_RACE, "SELECT race FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_LEVEL, "SELECT level FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_ZONE, "SELECT zone FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_POSITION_XYZ, "SELECT map, position_x, position_y, position_z FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_POSITION, "SELECT position_x, position_y, position_z, orientation, map, taxi_path FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_QUEST_STATUS_DAILY, "DELETE FROM character_queststatus_daily");
    conn.prepareStatement(CHAR_DEL_QUEST_STATUS_WEEKLY, "DELETE FROM character_queststatus_weekly");
    conn.prepareStatement(CHAR_DEL_QUEST_STATUS_MONTHLY, "DELETE FROM character_queststatus_monthly");
    conn.prepareStatement(CHAR_DEL_QUEST_STATUS_SEASONAL, "DELETE FROM character_queststatus_seasonal WHERE event = ?");
    conn.prepareStatement(CHAR_DEL_QUEST_STATUS_DAILY_CHAR, "DELETE FROM character_queststatus_daily WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_QUEST_STATUS_WEEKLY_CHAR, "DELETE FROM character_queststatus_weekly WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_QUEST_STATUS_MONTHLY_CHAR, "DELETE FROM character_queststatus_monthly WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_QUEST_STATUS_SEASONAL_CHAR, "DELETE FROM character_queststatus_seasonal WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_BATTLEGROUND_RANDOM, "DELETE FROM character_battleground_random");
    conn.prepareStatement(CHAR_INS_BATTLEGROUND_RANDOM, "INSERT INTO character_battleground_random (guid) VALUES (?)");

    // Start LoginQueryHolder content
    conn.prepareStatement(CHAR_SEL_CHARACTER, "SELECT c.guid, account, name, race, class, gender, level, xp, money, playerBytes, playerBytes2, playerFlags, "
                      "position_x, position_y, position_z, map, orientation, taximask, cinematic, totaltime, leveltime, rest_bonus, logout_time, is_logout_resting, resettalents_cost, "
                      "resettalents_time, trans_x, trans_y, trans_z, trans_o, transguid, extra_flags, at_login, death_expire_time, taxi_path, instance_mode_mask, "
                      "totalKills, todayKills, yesterdayKills, chosenTitle, watchedFaction, drunk, health, power1, power2, power3, power4, power5, instance_id, speccount, activespec, "
                      "specialization1, specialization2, exploredZones, actionBars, grantableLevels, resetspecialization_cost, resetspecialization_time, cp.pet_id "
                      "FROM characters AS c LEFT JOIN character_pet AS cp ON c.guid = cp.guid WHERE c.guid = ?");

    conn.prepareStatement(CHAR_SEL_GROUP_MEMBER, "SELECT guid FROM group_member WHERE memberGuid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_INSTANCE, "SELECT id, permanent, map, difficulty, resettime FROM character_instance LEFT JOIN instance ON instance = id WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_AURAS, "SELECT caster_guid, slot, spell, effect_mask, recalculate_mask, stackcount, maxduration, remaintime, remaincharges FROM character_aura WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_AURAS_EFFECTS, "SELECT slot, effect, baseamount, amount FROM character_aura_effect WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_SPELL, "SELECT spell, active, disabled FROM character_spell WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_QUESTSTATUS, "SELECT quest, status, explored, timer FROM character_queststatus WHERE guid = ? AND status <> 0");
    conn.prepareStatement(CHAR_SEL_CHARACTER_QUEST_OBJECTIVE_STATUS, "SELECT `objectiveId`, `amount` FROM `character_queststatus_objective` WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_DAILYQUESTSTATUS, "SELECT quest, time FROM character_queststatus_daily WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_WEEKLYQUESTSTATUS, "SELECT quest FROM character_queststatus_weekly WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_MONTHLYQUESTSTATUS, "SELECT quest FROM character_queststatus_monthly WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_SEASONALQUESTSTATUS, "SELECT quest, event FROM character_queststatus_seasonal WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_CHARACTER_DAILYQUESTSTATUS, "INSERT INTO character_queststatus_daily (guid, quest, time) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_INS_CHARACTER_WEEKLYQUESTSTATUS, "INSERT INTO character_queststatus_weekly (guid, quest) VALUES (?, ?)");
    conn.prepareStatement(CHAR_INS_CHARACTER_MONTHLYQUESTSTATUS, "INSERT INTO character_queststatus_monthly (guid, quest) VALUES (?, ?)");
    conn.prepareStatement(CHAR_INS_CHARACTER_SEASONALQUESTSTATUS, "INSERT INTO character_queststatus_seasonal (guid, quest, event) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_SEL_CHARACTER_REPUTATION, "SELECT faction, standing, flags FROM character_reputation WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_INVENTORY, "SELECT creatorGuid, giftCreatorGuid, count, duration, charges, flags, enchantments, randomPropertyId, reforgeId, transmogrifyId, upgradeId, durability, playedTime, text, bag, slot, "
    "item, itemEntry FROM character_inventory ci JOIN item_instance ii ON ci.item = ii.guid WHERE ci.guid = ? ORDER BY bag, slot");
    conn.prepareStatement(CHAR_SEL_CHARACTER_ACTIONS, "SELECT a.button, a.action, a.type FROM character_action as a, characters as c WHERE a.guid = c.guid AND a.spec = c.activespec AND a.guid = ? ORDER BY button");
    conn.prepareStatement(CHAR_SEL_CHARACTER_MAILCOUNT, "SELECT COUNT(id) FROM mail WHERE receiver = ? AND (checked & 1) = 0 AND deliver_time <= ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_MAILDATE, "SELECT MIN(deliver_time) FROM mail WHERE receiver = ? AND (checked & 1) = 0");
    conn.prepareStatement(CHAR_SEL_MAIL_COUNT, "SELECT COUNT(*) FROM mail WHERE receiver = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_SOCIALLIST, "SELECT friend, flags, note FROM character_social JOIN characters ON characters.guid = character_social.friend WHERE character_social.guid = ? AND deleteinfos_name IS NULL LIMIT 255");
    conn.prepareStatement(CHAR_SEL_CHARACTER_HOMEBIND, "SELECT mapId, zoneId, posX, posY, posZ FROM character_homebind WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_SPELLCOOLDOWNS, "SELECT spell, item, startTime, delay FROM character_spell_cooldown WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_DECLINEDNAMES, "SELECT genitive, dative, accusative, instrumental, prepositional FROM character_declinedname WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_GUILD, "SELECT g.guildid, g.name, g.leaderguid, g.EmblemStyle, g.EmblemColor, g.BorderStyle, g.BorderColor, g.BackgroundColor, g.info, g.motd, "
                          "g.createdate, g.BankMoney, g.level, g.experience, g.todayExperience, COUNT(gbt.guildid) FROM guild g LEFT JOIN guild_bank_tab gbt ON g.guildid = gbt.guildid "
                          "GROUP BY g.guildid, g.name, g.leaderguid, g.EmblemStyle, g.EmblemColor, g.BorderStyle, g.BorderColor, g.BackgroundColor, g.info, g.motd, g.createdate, "
                          "g.BankMoney, g.level, g.experience, g.todayExperience ORDER BY g.guildid ASC");
    conn.prepareStatement(CHAR_SEL_GUILD_MEMBER, "SELECT guildid, rank FROM guild_member WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_GUILD_MEMBER_EXTENDED, "SELECT g.guildid, g.name, gr.rname, gm.pnote, gm.offnote "
                          "FROM guild g JOIN guild_member gm ON g.guildid = gm.guildid "
                          "JOIN guild_rank gr ON g.guildid = gr.guildid AND gm.rank = gr.rid WHERE gm.guid = ?");

    conn.prepareStatement(CHAR_SEL_CHARACTER_ACHIEVEMENTS, "SELECT achievement, guid, date FROM character_achievement WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_CRITERIAPROGRESS, "SELECT criteria, counter, date FROM character_achievement_progress WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_EQUIPMENTSETS, "SELECT setguid, setindex, name, iconname, ignore_mask, item0, item1, item2, item3, item4, item5, item6, item7, item8, "
    "item9, item10, item11, item12, item13, item14, item15, item16, item17, item18 FROM character_equipmentsets WHERE name <> '' AND iconname <> '' AND guid = ? ORDER BY setindex");
    conn.prepareStatement(CHAR_SEL_CHARACTER_BGDATA, "SELECT instanceId, team, joinX, joinY, joinZ, joinO, joinMapId, taxiStart, taxiEnd, mountSpell FROM character_battleground_data WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_GLYPHS, "SELECT spec, glyph1, glyph2, glyph3, glyph4, glyph5, glyph6 FROM character_glyphs WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_TALENTS, "SELECT spell, spec FROM character_talent WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_SKILLS, "SELECT skill, value, max FROM character_skills WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_RANDOMBG, "SELECT guid FROM character_battleground_random WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_BANNED, "SELECT guid FROM character_banned WHERE guid = ? AND active = 1");
    conn.prepareStatement(CHAR_SEL_CHARACTER_QUESTSTATUSREW, "SELECT quest FROM character_queststatus_rewarded WHERE guid = ? AND active = 1");
    conn.prepareStatement(CHAR_SEL_ACCOUNT_INSTANCELOCKTIMES, "SELECT instanceId, releaseTime FROM account_instance_times WHERE accountId = ?");
    // End LoginQueryHolder content

    conn.prepareStatement(CHAR_SEL_CHARACTER_ACTIONS_SPEC, "SELECT button, action, type FROM character_action WHERE guid = ? AND spec = ? ORDER BY button");
    conn.prepareStatement(CHAR_SEL_MAILITEMS,
                          "SELECT creatorGuid, giftCreatorGuid, count, duration, charges, flags, enchantments,"
                          " randomPropertyId, reforgeId, transmogrifyId, upgradeId, durability, playedTime, text, item_guid,"
                          " itemEntry, owner_guid FROM mail_items mi JOIN item_instance ii ON mi.item_guid = ii.guid WHERE mail_id = ?");

    conn.prepareStatement(CHAR_SEL_AUCTION_ITEMS, "SELECT creatorGuid, giftCreatorGuid, count, duration, charges, flags, enchantments, randomPropertyId, reforgeId, transmogrifyId, upgradeId, durability, playedTime, text, itemguid, itemEntry FROM auctionhouse ah JOIN item_instance ii ON ah.itemguid = ii.guid");
    conn.prepareStatement(CHAR_SEL_AUCTIONS, "SELECT id, auctionHouseId, itemguid, itemEntry, count, itemowner, buyoutprice, time, buyguid, lastbid, startbid, deposit FROM auctionhouse ah INNER JOIN item_instance ii ON ii.guid = ah.itemguid");
    conn.prepareStatement(CHAR_INS_AUCTION, "INSERT INTO auctionhouse (id, auctionHouseId, itemguid, itemowner, buyoutprice, time, buyguid, lastbid, startbid, deposit) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_AUCTION, "DELETE FROM auctionhouse WHERE id = ?");
    conn.prepareStatement(CHAR_SEL_AUCTION_BY_TIME, "SELECT id FROM auctionhouse WHERE time <= ? ORDER BY TIME ASC");
    conn.prepareStatement(CHAR_UPD_AUCTION_BID, "UPDATE auctionhouse SET buyguid = ?, lastbid = ? WHERE id = ?");
    conn.prepareStatement(CHAR_INS_MAIL, "INSERT INTO mail(id, messageType, stationery, mailTemplateId, sender, receiver, subject, body, has_items, expire_time, deliver_time, money, cod, checked) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_MAIL_BY_ID, "DELETE FROM mail WHERE id = ?");
    conn.prepareStatement(CHAR_INS_MAIL_ITEM, "INSERT INTO mail_items(mail_id, item_guid) VALUES (?, ?)");
    conn.prepareStatement(CHAR_DEL_MAIL_ITEM, "DELETE FROM mail_items WHERE item_guid = ?");
    conn.prepareStatement(CHAR_DEL_INVALID_MAIL_ITEM, "DELETE FROM mail_items WHERE item_guid = ?");
    conn.prepareStatement(CHAR_DEL_EMPTY_EXPIRED_MAIL, "DELETE FROM mail WHERE expire_time < ? AND has_items = 0 AND body = ''");
    conn.prepareStatement(CHAR_SEL_EXPIRED_MAIL, "SELECT id, messageType, sender, receiver, has_items, expire_time, cod, checked, mailTemplateId FROM mail WHERE expire_time < ?");
    conn.prepareStatement(CHAR_SEL_EXPIRED_MAIL_ITEMS, "SELECT item_guid, itemEntry, mail_id FROM mail_items mi INNER JOIN item_instance ii ON ii.guid = mi.item_guid LEFT JOIN mail mm ON mi.mail_id = mm.id WHERE mm.id IS NOT NULL AND mm.expire_time < ?");
    conn.prepareStatement(CHAR_UPD_MAIL_RETURNED, "UPDATE mail SET sender = ?, receiver = ?, expire_time = ?, deliver_time = ?, cod = 0, checked = ? WHERE id = ?");

    conn.prepareStatement(CHAR_SEL_ITEM_REFUNDS, "SELECT player_guid, paidMoney, paidExtendedCost FROM item_refund_instance WHERE item_guid = ? AND player_guid = ?");
    conn.prepareStatement(CHAR_SEL_ITEM_BOP_TRADE, "SELECT allowedPlayers FROM item_soulbound_trade_data WHERE itemGuid = ?");
    conn.prepareStatement(CHAR_DEL_ITEM_BOP_TRADE, "DELETE FROM item_soulbound_trade_data WHERE itemGuid = ?");
    conn.prepareStatement(CHAR_INS_ITEM_BOP_TRADE, "INSERT INTO item_soulbound_trade_data VALUES (?, ?)");
    conn.prepareStatement(CHAR_REP_INVENTORY_ITEM, "REPLACE INTO character_inventory (guid, bag, slot, item) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_REP_ITEM_INSTANCE,
                          "INSERT INTO item_instance (itemEntry, owner_guid, creatorGuid, giftCreatorGuid,"
                          " count, duration, charges, flags, enchantments, randomPropertyId, reforgeId,"
                          " transmogrifyId, upgradeId, durability, playedTime, text, guid)"
                          " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                          " ON DUPLICATE KEY UPDATE itemEntry = VALUES(itemEntry), owner_guid = VALUES(owner_guid),"
                          " creatorGuid = VALUES(creatorGuid), giftCreatorGuid = VALUES(giftCreatorGuid),"
                          " count = VALUES(count), duration = VALUES(duration), charges = VALUES(charges),"
                          " flags = VALUES(flags), enchantments = VALUES(enchantments),"
                          " randomPropertyId = VALUES(randomPropertyId), reforgeId = VALUES(reforgeId),"
                          " transmogrifyId = VALUES(transmogrifyId), upgradeId = VALUES(upgradeId),"
                          " durability = VALUES(durability), playedTime = VALUES(playedTime),"
                          " text = VALUES(text)");
    conn.prepareStatement(CHAR_UPD_ITEM_INSTANCE_ON_LOAD, "UPDATE item_instance SET duration = ?, flags = ?, durability = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_ITEM_INSTANCE, "DELETE FROM item_instance WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_GIFT_OWNER, "UPDATE character_gifts SET guid = ? WHERE item_guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_GIFT_BY_ITEM, "SELECT entry, flags FROM character_gifts WHERE item_guid = ?");
    conn.prepareStatement(CHAR_SEL_ACCOUNT_BY_NAME, "SELECT account FROM characters WHERE name = ?");
    conn.prepareStatement(CHAR_SEL_ACCOUNT_BY_GUID, "SELECT account FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_ACCOUNT_NAME_BY_GUID, "SELECT account, name FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_ACCOUNT_INSTANCE_LOCK_TIMES, "DELETE FROM account_instance_times WHERE accountId = ?");
    conn.prepareStatement(CHAR_INS_ACCOUNT_INSTANCE_LOCK_TIMES, "INSERT INTO account_instance_times (accountId, instanceId, releaseTime) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_SEL_CHARACTER_NAME_CLASS, "SELECT name, class FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_NAME, "SELECT name FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_COUNT, "SELECT account, COUNT(guid) FROM characters WHERE account = ? GROUP BY account");
    conn.prepareStatement(CHAR_UPD_NAME, "UPDATE characters set name = ?, at_login = at_login & ~ ? WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_NAME_BY_GUID, "UPDATE characters SET name = ? WHERE guid = ?");

    // Guild handling
    // 0: uint32, 1: string, 2: uint32, 3: string, 4: string, 5: uint64, 6-10: uint32, 11: uint64
    conn.prepareStatement(CHAR_INS_GUILD, "INSERT INTO guild (guildid, name, leaderguid, info, motd, createdate, EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor, BankMoney) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD, "DELETE FROM guild WHERE guildid = ?"); // 0: uint32
    // 0: string, 1: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_NAME, "UPDATE guild SET name = ? WHERE guildid = ?");
    // 0: uint32, 1: uint32, 2: uint8, 4: string, 5: string
    conn.prepareStatement(CHAR_INS_GUILD_MEMBER, "INSERT INTO guild_member (guildid, guid, rank, pnote, offnote, weekActivity, totalActivity, weekReputation, totalReputation) VALUES (?, ?, ?, ?, ?, 0, 0, 0, 0)");
    conn.prepareStatement(CHAR_DEL_GUILD_MEMBER, "DELETE FROM guild_member WHERE guid = ?"); // 0: uint32
    conn.prepareStatement(CHAR_DEL_GUILD_MEMBERS, "DELETE FROM guild_member WHERE guildid = ?"); // 0: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBERS_RANK, "UPDATE guild_member SET rank = ? WHERE guildid = ? AND rank = ?"); // 0: uint32
    // 0: uint32, 1: uint8, 3: string, 4: uint32
    conn.prepareStatement(CHAR_INS_GUILD_RANK, "INSERT INTO guild_rank (guildid, rid, rname, rights) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD_RANKS, "DELETE FROM guild_rank WHERE guildid = ?"); // 0: uint32
    conn.prepareStatement(CHAR_DEL_GUILD_RANK, "DELETE FROM guild_rank WHERE guildid = ? AND rid = ?"); // 0: uint32, 1: uint8
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_ID, "UPDATE guild_rank SET rid = ? WHERE guildid = ? AND rid = ?"); // 0: uint32, 1: uint8
    conn.prepareStatement(CHAR_INS_GUILD_BANK_TAB, "INSERT INTO guild_bank_tab (guildid, TabId) VALUES (?, ?)"); // 0: uint32, 1: uint8
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_TAB, "DELETE FROM guild_bank_tab WHERE guildid = ? AND TabId = ?"); // 0: uint32, 1: uint8
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_TABS, "DELETE FROM guild_bank_tab WHERE guildid = ?"); // 0: uint32
    // 0: uint32, 1: uint8, 2: uint8, 3: uint32, 4: uint32
    conn.prepareStatement(CHAR_INS_GUILD_BANK_ITEM, "INSERT INTO guild_bank_item (guildid, TabId, SlotId, item_guid) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_ITEM, "DELETE FROM guild_bank_item WHERE item_guid = ?"); // 0: uint32, 1: uint8, 2: uint8
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_ITEMS, "DELETE FROM guild_bank_item WHERE guildid = ?"); // 0: uint32
    conn.prepareStatement(CHAR_INS_GUILD_BANK_RIGHT_DEFAULT, "INSERT INTO guild_bank_right (guildid, TabId, rid) VALUES (?, ?, ?)"); // 0: uint32, 1: uint8, 2: uint8
    // 0: uint32, 1: uint8, 2: uint8, 3: uint8, 4: uint32
    conn.prepareStatement(CHAR_INS_GUILD_BANK_RIGHT, "INSERT INTO guild_bank_right (guildid, TabId, rid, gbright, SlotPerDay) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_RIGHT, "DELETE FROM guild_bank_right WHERE guildid = ? AND TabId = ? AND rid = ?"); // 0: uint32, 1: uint8, 2: uint8
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_RIGHTS, "DELETE FROM guild_bank_right WHERE guildid = ?"); // 0: uint32
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_RIGHTS_FOR_RANK, "DELETE FROM guild_bank_right WHERE guildid = ? AND rid = ?"); // 0: uint32, 1: uint8
    conn.prepareStatement(CHAR_UPD_GUILD_BANK_RIGHTS_ID, "UPDATE guild_bank_right SET rid = ? WHERE guildid = ? AND rid = ?"); // 0: uint32, 1: uint8
    // 0-1: uint32, 2-3: uint8, 4-5: uint32, 6: uint16, 7: uint8, 8: uint64
    conn.prepareStatement(CHAR_INS_GUILD_BANK_EVENTLOG, "INSERT INTO guild_bank_eventlog (guildid, LogGuid, TabId, EventType, PlayerGuid, ItemOrMoney, ItemStackCount, DestTabId, TimeStamp) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_EVENTLOG, "DELETE FROM guild_bank_eventlog WHERE guildid = ? AND LogGuid = ? AND TabId = ?"); // 0: uint32, 1: uint32, 2: uint8
    conn.prepareStatement(CHAR_DEL_GUILD_BANK_EVENTLOGS, "DELETE FROM guild_bank_eventlog WHERE guildid = ?"); // 0: uint32
    // 0-1: uint32, 2: uint8, 3-4: uint32, 5: uint8, 6: uint64
    conn.prepareStatement(CHAR_INS_GUILD_EVENTLOG, "INSERT INTO guild_eventlog (guildid, LogGuid, EventType, PlayerGuid1, PlayerGuid2, NewRank, TimeStamp) VALUES (?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD_EVENTLOG, "DELETE FROM guild_eventlog WHERE guildid = ? AND LogGuid = ?"); // 0: uint32, 1: uint32
    conn.prepareStatement(CHAR_DEL_GUILD_EVENTLOGS, "DELETE FROM guild_eventlog WHERE guildid = ?"); // 0: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_PNOTE, "UPDATE guild_member SET pnote = ? WHERE guid = ?"); // 0: string, 1: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_OFFNOTE, "UPDATE guild_member SET offnote = ? WHERE guid = ?"); // 0: string, 1: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_RANK, "UPDATE guild_member SET rank = ? WHERE guid = ?"); // 0: uint8, 1: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_ROSTER_INFO, "UPDATE guild_member SET firstSkillId = ?, firstSkillValue = ?, firstSkillRank = ?, secondSkillId = ?, secondSkillValue = ?, secondSkillRank = ?, achievementPoints = ? WHERE guid = ?"); // 0: uint32, 1: uint16, 2: uint16, 3: uint8, 4: uint16, 5: uint16, 6: uint8, 7: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MOTD, "UPDATE guild SET motd = ? WHERE guildid = ?"); // 0: string, 1: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_INFO, "UPDATE guild SET info = ? WHERE guildid = ?"); // 0: string, 1: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_LEADER, "UPDATE guild SET leaderguid = ? WHERE guildid = ?"); // 0: uint32, 1: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_NAME, "UPDATE guild_rank SET rname = ? WHERE rid = ? AND guildid = ?"); // 0: string, 1: uint8, 2: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_RIGHTS, "UPDATE guild_rank SET rights = ? WHERE rid = ? AND guildid = ?"); // 0: uint32, 1: uint8, 2: uint32
    // 0-5: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_EMBLEM_INFO, "UPDATE guild SET EmblemStyle = ?, EmblemColor = ?, BorderStyle = ?, BorderColor = ?, BackgroundColor = ? WHERE guildid = ?");
    // 0: string, 1: string, 2: uint32, 3: uint8
    conn.prepareStatement(CHAR_UPD_GUILD_BANK_TAB_INFO, "UPDATE guild_bank_tab SET TabName = ?, TabIcon = ? WHERE guildid = ? AND TabId = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_BANK_MONEY, "UPDATE guild SET BankMoney = ? WHERE guildid = ?"); // 0: uint64, 1: uint32
    // 0: uint8, 1: uint32, 2: uint8, 3: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_BANK_EVENTLOG_TAB, "UPDATE guild_bank_eventlog SET TabId = ? WHERE guildid = ? AND TabId = ? AND LogGuid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_MONEY, "UPDATE guild_member SET BankRemMoney = ? WHERE guildid = ? AND guid = ?"); // 0: uint32, 1: uint32, 2: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_MONEY, "UPDATE guild_member SET BankResetTimeMoney = ?, BankRemMoney = ? WHERE guildid = ? AND guid = ?"); // 0: uint32, 1: uint32, 2: uint32, 3: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_RESET_TIME, "UPDATE guild_member SET BankResetTimeMoney = 0 WHERE guildid = ? AND rank = ?"); // 0: uint32, 1: uint8
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_MONEY, "UPDATE guild_rank SET BankMoneyPerDay = ? WHERE rid = ? AND guildid = ?"); // 0: uint32, 1: uint8, 2: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_BANK_TAB_TEXT, "UPDATE guild_bank_tab SET TabText = ? WHERE guildid = ? AND TabId = ?"); // 0: string, 1: uint32, 2: uint8
    // 0: uint32, 1: uint8
    conn.prepareStatement(CHAR_UPD_GUILD_BANK_RANK_RIGHTS, "UPDATE guild_bank_right Set rid = rid - 1 WHERE guildid = ? AND rid > ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RANKS, "UPDATE guild_rank Set rid = rid - 1 WHERE guildid = ? AND rid > ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_RANKS, "UPDATE guild_member Set rank = rank - 1 WHERE guildid = ? AND rank > ?");
    // 0: uint32, 1: uint32, 2: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS0, "UPDATE guild_member SET BankRemSlotsTab0 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS1, "UPDATE guild_member SET BankRemSlotsTab1 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS2, "UPDATE guild_member SET BankRemSlotsTab2 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS3, "UPDATE guild_member SET BankRemSlotsTab3 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS4, "UPDATE guild_member SET BankRemSlotsTab4 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS5, "UPDATE guild_member SET BankRemSlotsTab5 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS6, "UPDATE guild_member SET BankRemSlotsTab6 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS7, "UPDATE guild_member SET BankRemSlotsTab7 = ? WHERE guildid = ? AND guid = ?");
    // 0: uint32, 1: uint32, 2: uint32, 3: uint32
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS0, "UPDATE guild_member SET BankResetTimeTab0 = ?, BankRemSlotsTab0 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS1, "UPDATE guild_member SET BankResetTimeTab1 = ?, BankRemSlotsTab1 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS2, "UPDATE guild_member SET BankResetTimeTab2 = ?, BankRemSlotsTab2 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS3, "UPDATE guild_member SET BankResetTimeTab3 = ?, BankRemSlotsTab3 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS4, "UPDATE guild_member SET BankResetTimeTab4 = ?, BankRemSlotsTab4 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS5, "UPDATE guild_member SET BankResetTimeTab5 = ?, BankRemSlotsTab5 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS6, "UPDATE guild_member SET BankResetTimeTab6 = ?, BankRemSlotsTab6 = ? WHERE guildid = ? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS7, "UPDATE guild_member SET BankResetTimeTab7 = ?, BankRemSlotsTab7 = ? WHERE guildid = ? AND guid = ?");
    // 0: uint32, 1: uint8
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME0, "UPDATE guild_member SET BankResetTimeTab0 = 0 WHERE guildid = ? AND rank = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME1, "UPDATE guild_member SET BankResetTimeTab1 = 0 WHERE guildid = ? AND rank = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME2, "UPDATE guild_member SET BankResetTimeTab2 = 0 WHERE guildid = ? AND rank = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME3, "UPDATE guild_member SET BankResetTimeTab3 = 0 WHERE guildid = ? AND rank = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME4, "UPDATE guild_member SET BankResetTimeTab4 = 0 WHERE guildid = ? AND rank = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME5, "UPDATE guild_member SET BankResetTimeTab5 = 0 WHERE guildid = ? AND rank = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME6, "UPDATE guild_member SET BankResetTimeTab6 = 0 WHERE guildid = ? AND rank = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME7, "UPDATE guild_member SET BankResetTimeTab7 = 0 WHERE guildid = ? AND rank = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_DATA_FOR_GUILD, "SELECT name, level, class, zone, account FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_GUILD_ACHIEVEMENT, "DELETE FROM guild_achievement WHERE guildId = ? AND achievement = ?");
    conn.prepareStatement(CHAR_INS_GUILD_ACHIEVEMENT, "INSERT INTO guild_achievement (guildId, achievement, date, guids) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD_ACHIEVEMENT_CRITERIA, "DELETE FROM guild_achievement_progress WHERE guildId = ? AND criteria = ?");
    conn.prepareStatement(CHAR_INS_GUILD_ACHIEVEMENT_CRITERIA, "INSERT INTO guild_achievement_progress (guildId, criteria, counter, date, completedGuid) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENTS, "DELETE FROM guild_achievement WHERE guildId = ?");
    conn.prepareStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENT_CRITERIA, "DELETE FROM guild_achievement_progress WHERE guildId = ?");
    conn.prepareStatement(CHAR_SEL_GUILD_ACHIEVEMENT, "SELECT achievement, date, guids FROM guild_achievement WHERE guildId = ?");
    conn.prepareStatement(CHAR_SEL_GUILD_ACHIEVEMENT_CRITERIA, "SELECT criteria, counter, date, completedGuid FROM guild_achievement_progress WHERE guildId = ?");
    conn.prepareStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA_GUILD, "DELETE FROM guild_achievement_progress WHERE criteria = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_EXPERIENCE, "UPDATE guild SET level = ?, experience = ?, todayExperience = ? WHERE guildId = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_RESET_TODAY_EXPERIENCE, "UPDATE guild SET todayExperience = 0");
    conn.prepareStatement(CHAR_LOAD_GUILD_NEWS, "SELECT id, eventType, playerGuid, data, flags, date FROM guild_news_log WHERE guild = ? ORDER BY guild ASC, id ASC");
    conn.prepareStatement(CHAR_SAVE_GUILD_NEWS, "INSERT INTO guild_news_log (guild, id, eventType, playerGuid, data, flags, date) VALUES (?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_RESET_WEEK_ACTIVITY, "UPDATE guild_member SET weekActivity = 0, weekReputation = 0");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_REPUTATION, "UPDATE guild_member SET weekReputation = ?, totalReputation = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_GUILD_MEMBER_ACTIVITY, "UPDATE guild_member SET weekActivity = ?, totalActivity = ? WHERE guid = ?");

    // Chat channel handling
    conn.prepareStatement(CHAR_SEL_CHANNEL, "SELECT announce, ownership, password, bannedList FROM channels WHERE name = ? AND team = ?");
    conn.prepareStatement(CHAR_INS_CHANNEL, "INSERT INTO channels(name, team, lastUsed) VALUES (?, ?, UNIX_TIMESTAMP())");
    conn.prepareStatement(CHAR_UPD_CHANNEL, "UPDATE channels SET announce = ?, ownership = ?, password = ?, bannedList = ?, lastUsed = UNIX_TIMESTAMP() WHERE name = ? AND team = ?");
    conn.prepareStatement(CHAR_UPD_CHANNEL_USAGE, "UPDATE channels SET lastUsed = UNIX_TIMESTAMP() WHERE name = ? AND team = ?");
    conn.prepareStatement(CHAR_UPD_CHANNEL_OWNERSHIP, "UPDATE channels SET ownership = ? WHERE name LIKE ?");
    conn.prepareStatement(CHAR_DEL_OLD_CHANNELS, "DELETE FROM channels WHERE ownership = 1 AND lastUsed + ? < UNIX_TIMESTAMP()");

    // Equipment sets
    conn.prepareStatement(CHAR_UPD_EQUIP_SET, "UPDATE character_equipmentsets SET name=?, iconname=?, ignore_mask=?, item0=?, item1=?, item2=?, item3=?, item4=?, item5=?, item6=?, item7=?, item8=?, item9=?, item10=?, item11=?, item12=?, item13=?, item14=?, item15=?, item16=?, item17=?, item18=? WHERE guid=? AND setguid=? AND setindex=?");
    conn.prepareStatement(CHAR_INS_EQUIP_SET, "INSERT INTO character_equipmentsets (guid, setguid, setindex, name, iconname, ignore_mask, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13, item14, item15, item16, item17, item18) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_EQUIP_SET, "DELETE FROM character_equipmentsets WHERE setguid=?");

    // Auras
    conn.prepareStatement(CHAR_INS_AURA, "INSERT INTO character_aura (guid, slot, caster_guid, item_guid, spell, effect_mask, recalculate_mask, stackcount, maxduration, remaintime, remaincharges) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    conn.prepareStatement(CHAR_INS_AURA_EFFECT, "INSERT INTO character_aura_effect (guid, slot, effect, baseamount, amount) "
    "VALUES (?, ?, ?, ?, ?)");

    // Archaeology
    conn.prepareStatement(CHAR_SEL_CHAR_ARCHAEOLOGY, "SELECT sites0, sites1, sites2, sites3, counts, projects, completed FROM character_archaeology WHERE guid = ?");

    conn.prepareStatement(CHAR_DEL_CUF_PROFILE, "DELETE FROM character_cuf_profiles WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_CUF_PROFILE, "INSERT INTO character_cuf_profiles (guid, id, name, unk0, unk1, frameWidth, frameHeight, unk4, unk5, unk6, unk7, sortType, healthText, bits) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_SEL_CUF_PROFILE, "SELECT id, name, unk0, unk1, frameWidth, frameHeight, unk4, unk5, unk6, unk7, sortType, healthText, bits FROM character_cuf_profiles WHERE guid = ?");

    // Currency
    conn.prepareStatement(CHAR_SEL_PLAYER_CURRENCY, "SELECT currency, season_count, week_count, current_count, flags FROM character_currency WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_PLAYER_CURRENCY, "UPDATE character_currency SET season_count = ?, week_count = ?, current_count = ?, flags = ? WHERE guid = ? AND currency = ?");
    conn.prepareStatement(CHAR_REP_PLAYER_CURRENCY, "REPLACE INTO character_currency (guid, currency, season_count, week_count, current_count, flags) VALUES (?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_SEL_CHARACTER_CP_WEEK_CAP, "SELECT max_personal_rating, arena_cap, max_rated_bg_rating, rated_bg_cap FROM character_cp_weekcap WHERE guid = ?");
    conn.prepareStatement(CHAR_REP_CHARACTER_CP_WEEK_CAP, "REPLACE INTO character_cp_weekcap (guid, max_personal_rating, arena_cap, max_rated_bg_rating, rated_bg_cap) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_CHARACTER_CP_WEEK_CAP, "UPDATE character_cp_weekcap SET arena_cap = CP_CAP_FROM_RATING(max_personal_rating), max_personal_rating = 0, "
                          "rated_bg_cap = CONVERT(CP_CAP_FROM_RATING(max_rated_bg_rating) * 1.222 + 0.5, UNSIGNED INTEGER), max_rated_bg_rating = 0");
    conn.prepareStatement(CHAR_UPD_CURRENCY_WEEK_COUNT, "UPDATE character_currency SET week_count = 0 WHERE currency NOT IN (390, 483, 484, 692)");
    conn.prepareStatement(CHAR_UPD_CONQUEST_WEEK_COUNT, "UPDATE character_currency SET week_count = 0 WHERE currency IN (390, 483, 484, 692)");
    conn.prepareStatement(CHAR_UPD_ARENA_DATA, "UPDATE character_arena_data SET prevWeekWins = weekWins, weekWins = 0, weekGames = 0, bestRatingOfWeek = 0");
    conn.prepareStatement(CHAR_UPD_RATED_BG_STATS, "UPDATE character_rated_bg_stats SET prevWeekWins = thisWeekWins, thisWeekWins = 0, weekGames = 0, bestWeekRating = 0");

    // Account data
    conn.prepareStatement(CHAR_SEL_ACCOUNT_DATA, "SELECT type, time, data FROM account_data WHERE accountId = ?");
    conn.prepareStatement(CHAR_REP_ACCOUNT_DATA, "REPLACE INTO account_data (accountId, type, time, data) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_ACCOUNT_DATA, "DELETE FROM account_data WHERE accountId = ?");
    conn.prepareStatement(CHAR_SEL_PLAYER_ACCOUNT_DATA, "SELECT type, time, data FROM character_account_data WHERE guid = ?");
    conn.prepareStatement(CHAR_REP_PLAYER_ACCOUNT_DATA, "REPLACE INTO character_account_data(guid, type, time, data) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_PLAYER_ACCOUNT_DATA, "DELETE FROM character_account_data WHERE guid = ?");

    // Tutorials
    conn.prepareStatement(CHAR_SEL_TUTORIALS, "SELECT tut0, tut1, tut2, tut3, tut4, tut5, tut6, tut7 FROM account_tutorial WHERE accountId = ?");
    conn.prepareStatement(CHAR_SEL_HAS_TUTORIALS, "SELECT 1 FROM account_tutorial WHERE accountId = ?");
    conn.prepareStatement(CHAR_INS_TUTORIALS, "INSERT INTO account_tutorial(tut0, tut1, tut2, tut3, tut4, tut5, tut6, tut7, accountId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_TUTORIALS, "UPDATE account_tutorial SET tut0 = ?, tut1 = ?, tut2 = ?, tut3 = ?, tut4 = ?, tut5 = ?, tut6 = ?, tut7 = ? WHERE accountId = ?");
    conn.prepareStatement(CHAR_DEL_TUTORIALS, "DELETE FROM account_tutorial WHERE accountId = ?");

    // Instance saves
    conn.prepareStatement(CHAR_INS_INSTANCE_SAVE, "INSERT INTO instance (id, map, resettime, difficulty, completedEncounters, data) VALUES (?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_INSTANCE_DATA, "UPDATE instance SET completedEncounters=?, data=? WHERE id=?");

    // Game event saves
    conn.prepareStatement(CHAR_DEL_GAME_EVENT_SAVE, "DELETE FROM game_event_save WHERE eventEntry = ?");
    conn.prepareStatement(CHAR_INS_GAME_EVENT_SAVE, "INSERT INTO game_event_save (eventEntry, state, next_start) VALUES (?, ?, ?)");

    // Game event condition saves
    conn.prepareStatement(CHAR_DEL_ALL_GAME_EVENT_CONDITION_SAVE, "DELETE FROM game_event_condition_save WHERE eventEntry = ?");
    conn.prepareStatement(CHAR_DEL_GAME_EVENT_CONDITION_SAVE, "DELETE FROM game_event_condition_save WHERE eventEntry = ? AND condition_id = ?");
    conn.prepareStatement(CHAR_INS_GAME_EVENT_CONDITION_SAVE, "INSERT INTO game_event_condition_save (eventEntry, condition_id, done) VALUES (?, ?, ?)");

    // Petitions
    conn.prepareStatement(CHAR_SEL_PETITION, "SELECT ownerguid, name FROM petition WHERE petitionguid = ?");
    conn.prepareStatement(CHAR_SEL_PETITION_SIGNATURE, "SELECT playerguid FROM petition_sign WHERE petitionguid = ?");
    conn.prepareStatement(CHAR_DEL_ALL_PETITION_SIGNATURES, "DELETE FROM petition_sign WHERE playerguid = ?");
    conn.prepareStatement(CHAR_DEL_PETITION_SIGNATURE, "DELETE FROM petition_sign WHERE playerguid = ? AND type = ?");
    conn.prepareStatement(CHAR_SEL_PETITION_BY_OWNER, "SELECT petitionguid FROM petition WHERE ownerguid = ? AND type = ?");
    conn.prepareStatement(CHAR_SEL_PETITION_TYPE, "SELECT type FROM petition WHERE petitionguid = ?");
    conn.prepareStatement(CHAR_SEL_PETITION_SIGNATURES, "SELECT ownerguid, (SELECT COUNT(playerguid) FROM petition_sign WHERE petition_sign.petitionguid = ?) AS signs, type FROM petition WHERE petitionguid = ?");
    conn.prepareStatement(CHAR_SEL_PETITION_SIG_BY_ACCOUNT, "SELECT playerguid FROM petition_sign WHERE player_account = ? AND petitionguid = ?");
    conn.prepareStatement(CHAR_SEL_PETITION_OWNER_BY_GUID, "SELECT ownerguid FROM petition WHERE petitionguid = ?");
    conn.prepareStatement(CHAR_SEL_PETITION_SIG_BY_GUID, "SELECT ownerguid, petitionguid FROM petition_sign WHERE playerguid = ?");
    conn.prepareStatement(CHAR_SEL_PETITION_SIG_BY_GUID_TYPE, "SELECT ownerguid, petitionguid FROM petition_sign WHERE playerguid = ? AND type = ?");

    conn.prepareStatement(CHAR_SEL_RATED_BG_STATS, "SELECT personalRating, matchmakerRating, seasonGames, weekGames,"
                          " thisWeekWins, prevWeekWins, bestWeekRating, bestSeasonRating FROM character_rated_bg_stats WHERE guid = ?");
    conn.prepareStatement(CHAR_REP_RATED_BG_STATS, "REPLACE INTO character_rated_bg_stats (guid, personalRating, matchmakerRating,"
                          " seasonGames, weekGames, thisWeekWins, prevWeekWins, bestWeekRating, bestSeasonRating) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

    // Character arena data
    conn.prepareStatement(CHAR_SEL_CHARACTER_ARENA_DATA, "SELECT slot, rating, bestRatingOfWeek, bestRatingOfSeason, matchMakerRating, weekGames, weekWins, prevWeekWins, seasonGames, seasonWins FROM character_arena_data WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_CHARACTER_ARENA_DATA, "INSERT INTO character_arena_data (guid, slot, rating, bestRatingOfWeek, bestRatingOfSeason, matchMakerRating, weekGames, weekWins, prevWeekWins, seasonGames, seasonWins) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHARACTER_ARENA_DATA, "DELETE FROM character_arena_data WHERE guid = ?");

    // Character battleground data
    conn.prepareStatement(CHAR_INS_PLAYER_BGDATA, "INSERT INTO character_battleground_data (guid, instanceId, team, joinX, joinY, joinZ, joinO, joinMapId, taxiStart, taxiEnd, mountSpell) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_PLAYER_BGDATA, "DELETE FROM character_battleground_data WHERE guid = ?");

    // Character homebind
    conn.prepareStatement(CHAR_INS_PLAYER_HOMEBIND, "INSERT INTO character_homebind (guid, mapId, zoneId, posX, posY, posZ) VALUES (?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_PLAYER_HOMEBIND, "UPDATE character_homebind SET mapId = ?, zoneId = ?, posX = ?, posY = ?, posZ = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_PLAYER_HOMEBIND, "DELETE FROM character_homebind WHERE guid = ?");

    // Corpse
    conn.prepareStatement(CHAR_SEL_CORPSES, "SELECT posX, posY, posZ, orientation, mapId, displayId, itemCache, bytes1, bytes2, flags, dynFlags, time, corpseType, instanceId, phaseMask, corpseGuid, guid FROM corpse WHERE corpseType <> 0");
    conn.prepareStatement(CHAR_INS_CORPSE, "INSERT INTO corpse (corpseGuid, guid, posX, posY, posZ, orientation, mapId, displayId, itemCache, bytes1, bytes2, flags, dynFlags, time, corpseType, instanceId, phaseMask) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CORPSE, "DELETE FROM corpse WHERE corpseGuid = ?");
    conn.prepareStatement(CHAR_DEL_PLAYER_CORPSES, "DELETE FROM corpse WHERE guid = ? AND corpseType <> 0");
    conn.prepareStatement(CHAR_DEL_OLD_CORPSES, "DELETE FROM corpse WHERE corpseType = 0 OR time < (UNIX_TIMESTAMP(NOW()) - ?)");

    // Creature respawn
    conn.prepareStatement(CHAR_SEL_CREATURE_RESPAWNS, "SELECT guid, respawnTime FROM creature_respawn WHERE mapId = ? AND instanceId = ?");
    conn.prepareStatement(CHAR_REP_CREATURE_RESPAWN, "REPLACE INTO creature_respawn (guid, respawnTime, mapId, instanceId) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CREATURE_RESPAWN, "DELETE FROM creature_respawn WHERE guid = ? AND mapId = ? AND instanceId = ?");
    conn.prepareStatement(CHAR_DEL_CREATURE_RESPAWN_BY_INSTANCE, "DELETE FROM creature_respawn WHERE mapId = ? AND instanceId = ?");
    conn.prepareStatement(CHAR_SEL_MAX_CREATURE_RESPAWNS, "SELECT MAX(respawnTime), instanceId FROM creature_respawn WHERE instanceId > 0 GROUP BY instanceId");

    // Gameobject respawn
    conn.prepareStatement(CHAR_SEL_GO_RESPAWNS, "SELECT guid, respawnTime FROM gameobject_respawn WHERE mapId = ? AND instanceId = ?");
    conn.prepareStatement(CHAR_REP_GO_RESPAWN, "REPLACE INTO gameobject_respawn (guid, respawnTime, mapId, instanceId) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GO_RESPAWN, "DELETE FROM gameobject_respawn WHERE guid = ? AND mapId = ? AND instanceId = ?");
    conn.prepareStatement(CHAR_DEL_GO_RESPAWN_BY_INSTANCE, "DELETE FROM gameobject_respawn WHERE mapId = ? AND instanceId = ?");

    // GM Tickets
    conn.prepareStatement(CHAR_SEL_GM_TICKETS,
                          "SELECT ticketId, guid, name, message, createTime, mapId, posX, posY, posZ,"
                          " lastModifiedTime, closedBy, assignedTo, comment, completed, escalated,"
                          " viewed, haveTicket FROM gm_tickets");

    conn.prepareStatement(CHAR_REP_GM_TICKET,
                          "REPLACE INTO gm_tickets (ticketId, guid, name, message, createTime, mapId, posX, posY, posZ,"
                          " lastModifiedTime, closedBy, assignedTo, comment, completed, escalated,"
                          " viewed, haveTicket) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    conn.prepareStatement(CHAR_DEL_GM_TICKET, "DELETE FROM gm_tickets WHERE ticketId = ?");

    // GM Survey/subsurvey/lag report
    conn.prepareStatement(CHAR_INS_GM_SURVEY, "INSERT INTO gm_surveys (guid, surveyId, mainSurvey, overallComment, createTime) VALUES (?, ?, ?, ?, UNIX_TIMESTAMP(NOW()))");
    conn.prepareStatement(CHAR_INS_GM_SUBSURVEY, "INSERT INTO gm_subsurveys (surveyId, subsurveyId, rank, comment) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_INS_LAG_REPORT, "INSERT INTO lag_reports (guid, lagType, mapId, posX, posY, posZ, latency, createTime) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    //  For loading and deleting expired auctions at startup
    conn.prepareStatement(CHAR_SEL_EXPIRED_AUCTIONS, "SELECT id, auctionHouseId, itemguid, itemEntry, count, itemowner, buyoutprice, time, buyguid, lastbid, startbid, deposit FROM auctionhouse ah INNER JOIN item_instance ii ON ii.guid = ah.itemguid WHERE ah.time <= ?");

    // LFG Data
    conn.prepareStatement(CHAR_INS_LFG_DATA, "INSERT INTO lfg_data (guid, dungeon, state) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_LFG_DATA, "DELETE FROM lfg_data WHERE guid = ?");

    // LFR
    conn.prepareStatement(CHAR_SEL_LFR_LOOT_BOUND, "SELECT id FROM character_lfr_loot_bound WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_LFR_LOOT_BOUND, "DELETE FROM character_lfr_loot_bound");
    conn.prepareStatement(CHAR_DEL_LFR_LOOT_BOUND_PLR, "DELETE FROM character_lfr_loot_bound WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_LFR_LOOT_BOUND, "INSERT INTO character_lfr_loot_bound (guid, id) VALUES (?, ?)");

    // Player saving
    conn.prepareStatement(CHAR_INS_CHARACTER, "INSERT INTO characters (guid, account, name, race, class, gender, level, xp, money, playerBytes, playerBytes2, playerFlags,"
                          " map, instance_id, instance_mode_mask, position_x, position_y, position_z, orientation, taximask, cinematic, totaltime, leveltime, rest_bonus,"
                          " logout_time, is_logout_resting, resettalents_cost, resettalents_time, extra_flags, at_login, zone, death_expire_time, taxi_path, totalKills,"
                          " todayKills, yesterdayKills, chosenTitle, watchedFaction, drunk, health, power1, power2, power3, power4, power5, latency, speccount, activespec,"
                          " specialization1, specialization2, exploredZones, equipmentCache, actionBars, grantableLevels) VALUES"
                          " (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");

    conn.prepareStatement(CHAR_UPD_CHARACTER, "UPDATE characters SET name=?,race=?,class=?,gender=?,level=?,xp=?,money=?,playerBytes=?,playerBytes2=?,playerFlags=?,"
                          " map=?,instance_id=?,instance_mode_mask=?,position_x=?,position_y=?,position_z=?,orientation=?,trans_x=?,trans_y=?,trans_z=?,trans_o=?,transguid=?,"
                          " taximask=?,cinematic=?,totaltime=?,leveltime=?,rest_bonus=?,logout_time=?,is_logout_resting=?,resettalents_cost=?,resettalents_time=?,extra_flags=?,"
                          " at_login=?,zone=?,death_expire_time=?,taxi_path=?,totalKills=?,todayKills=?,yesterdayKills=?,chosenTitle=?,watchedFaction=?,drunk=?,health=?,power1=?,"
                          " power2=?,power3=?,power4=?,power5=?,latency=?,speccount=?,activespec=?,specialization1=?,specialization2=?,exploredZones=?,equipmentCache=?,"
                          " actionBars=?,grantableLevels=?,online=?,resetspecialization_cost=?,resetspecialization_time=? WHERE guid=?");

    conn.prepareStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG, "UPDATE characters SET at_login = at_login | ? WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_REM_AT_LOGIN_FLAG, "UPDATE characters set at_login = at_login & ~ ? WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_BUG_REPORT, "INSERT INTO bugreport (type, content) VALUES(?, ?)");
    conn.prepareStatement(CHAR_UPD_PETITION_NAME, "UPDATE petition SET name = ? WHERE petitionguid = ?");
    conn.prepareStatement(CHAR_INS_PETITION_SIGNATURE, "INSERT INTO petition_sign (ownerguid, petitionguid, playerguid, player_account) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_ACCOUNT_ONLINE, "UPDATE characters SET online = 0 WHERE account = ?");
    conn.prepareStatement(CHAR_INS_GROUP, "INSERT INTO groups (guid, leaderGuid, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, groupType, difficulty, raiddifficulty) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_INS_GROUP_MEMBER, "INSERT INTO group_member (guid, memberGuid, memberFlags, subgroup, roles) VALUES(?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GROUP_MEMBER, "DELETE FROM group_member WHERE memberGuid = ?");
    conn.prepareStatement(CHAR_DEL_GROUP_INSTANCE_PERM_BINDING, "DELETE FROM group_instance WHERE guid = ? AND instance = ?");
    conn.prepareStatement(CHAR_UPD_GROUP_LEADER, "UPDATE groups SET leaderGuid = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_GROUP_TYPE, "UPDATE groups SET groupType = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_GROUP_MEMBER_SUBGROUP, "UPDATE group_member SET subgroup = ? WHERE memberGuid = ?");
    conn.prepareStatement(CHAR_UPD_GROUP_MEMBER_FLAG, "UPDATE group_member SET memberFlags = ? WHERE memberGuid = ?");
    conn.prepareStatement(CHAR_UPD_GROUP_MEMBER_ROLE, "UPDATE group_member SET roles = ? WHERE memberGuid = ?");
    conn.prepareStatement(CHAR_UPD_GROUP_DIFFICULTY, "UPDATE groups SET difficulty = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_GROUP_RAID_DIFFICULTY, "UPDATE groups SET raiddifficulty = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_ALL_GM_TICKETS, "TRUNCATE TABLE gm_tickets");
    conn.prepareStatement(CHAR_DEL_INVALID_SPELL, "DELETE FROM character_talent WHERE spell = ?");
    conn.prepareStatement(CHAR_UPD_DELETE_INFO, "UPDATE characters SET deleteInfos_Name = name, deleteInfos_Account = account, deleteDate = UNIX_TIMESTAMP(), name = '', account = 0 WHERE guid = ?");
    conn.prepareStatement(CHAR_UDP_RESTORE_DELETE_INFO, "UPDATE characters SET name = ?, account = ?, deleteDate = NULL, deleteInfos_Name = NULL, deleteInfos_Account = NULL WHERE deleteDate IS NOT NULL AND guid = ?");
    conn.prepareStatement(CHAR_UPD_ZONE, "UPDATE characters SET zone = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_LEVEL, "UPDATE characters SET level = ?, xp = 0 WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA, "DELETE FROM character_achievement_progress WHERE criteria = ?");
    conn.prepareStatement(CHAR_DEL_INVALID_ACHIEVMENT, "DELETE FROM character_achievement WHERE achievement = ?");
    conn.prepareStatement(CHAR_INS_ADDON, "INSERT INTO addons (name, crc) VALUES (?, ?)");
    conn.prepareStatement(CHAR_DEL_GROUP_INSTANCE_BY_INSTANCE, "DELETE FROM group_instance WHERE instance = ?");
    conn.prepareStatement(CHAR_DEL_GROUP_INSTANCE_BY_GUID, "DELETE FROM group_instance WHERE guid = ? AND instance = ?");
    conn.prepareStatement(CHAR_REP_GROUP_INSTANCE, "REPLACE INTO group_instance (guid, instance, permanent) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_INSTANCE_RESETTIME, "UPDATE instance SET resettime = ? WHERE id = ?");
    conn.prepareStatement(CHAR_UPD_GLOBAL_INSTANCE_RESETTIME, "UPDATE instance_reset SET resettime = ? WHERE mapid = ? AND difficulty = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_ONLINE, "UPDATE characters SET online = 1 WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_NAME_AT_LOGIN, "UPDATE characters set name = ?, at_login = at_login & ~ ? WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_WORLDSTATE, "UPDATE worldstates SET value = ? WHERE entry = ?");
    conn.prepareStatement(CHAR_INS_WORLDSTATE, "INSERT INTO worldstates (entry, value) VALUES (?, ?)");
    conn.prepareStatement(CHAR_DEL_CHAR_INSTANCE_BY_INSTANCE_GUID, "DELETE FROM character_instance WHERE guid = ? AND instance = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_INSTANCE, "UPDATE character_instance SET instance = ?, permanent = ? WHERE guid = ? AND instance = ?");
    conn.prepareStatement(CHAR_INS_CHAR_INSTANCE, "INSERT INTO character_instance (guid, instance, permanent) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_GENDER_PLAYERBYTES, "UPDATE characters SET gender = ?, playerBytes = ?, playerBytes2 = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHARACTER_SKILL, "DELETE FROM character_skills WHERE guid = ? AND skill = ?");
    conn.prepareStatement(CHAR_UPD_ADD_CHARACTER_SOCIAL_FLAGS, "UPDATE character_social SET flags = flags | ? WHERE guid = ? AND friend = ?");
    conn.prepareStatement(CHAR_UPD_REM_CHARACTER_SOCIAL_FLAGS, "UPDATE character_social SET flags = flags & ~ ? WHERE guid = ? AND friend = ?");
    conn.prepareStatement(CHAR_INS_CHARACTER_SOCIAL, "INSERT INTO character_social (guid, friend, flags) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHARACTER_SOCIAL, "DELETE FROM character_social WHERE guid = ? AND friend = ?");
    conn.prepareStatement(CHAR_UPD_CHARACTER_SOCIAL_NOTE, "UPDATE character_social SET note = ? WHERE guid = ? AND friend = ?");
    conn.prepareStatement(CHAR_UPD_CHARACTER_POSITION, "UPDATE characters SET position_x = ?, position_y = ?, position_z = ?, orientation = ?, map = ?, zone = ?, trans_x = 0, trans_y = 0, trans_z = 0, transguid = 0, taxi_path = '' WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_AURA_FROZEN, "SELECT characters.name FROM characters LEFT JOIN character_aura ON (characters.guid = character_aura.guid) WHERE character_aura.spell = 9454");
    conn.prepareStatement(CHAR_SEL_CHARACTER_ONLINE, "SELECT name, account, map, zone FROM characters WHERE online > 0");
    conn.prepareStatement(CHAR_SEL_CHAR_DEL_INFO_BY_GUID, "SELECT guid, deleteInfos_Name, deleteInfos_Account, deleteDate FROM characters WHERE deleteDate IS NOT NULL AND guid = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_DEL_INFO_BY_NAME, "SELECT guid, deleteInfos_Name, deleteInfos_Account, deleteDate FROM characters WHERE deleteDate IS NOT NULL AND deleteInfos_Name LIKE CONCAT('%%', ?, '%%')");
    conn.prepareStatement(CHAR_SEL_CHAR_DEL_INFO, "SELECT guid, deleteInfos_Name, deleteInfos_Account, deleteDate FROM characters WHERE deleteDate IS NOT NULL");
    conn.prepareStatement(CHAR_SEL_CHARS_BY_ACCOUNT_ID, "SELECT guid FROM characters WHERE account = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_PINFO, "SELECT totaltime, level, money, account, race, class, map, zone, gender, health, playerFlags FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_PINFO_BANS, "SELECT unbandate, bandate = unbandate, bannedby, banreason FROM character_banned WHERE guid = ? AND active ORDER BY bandate ASC LIMIT 1");
    conn.prepareStatement(CHAR_SEL_PINFO_MAILS, "SELECT CAST(SUM(CASE WHEN (checked & 1) THEN 1 ELSE 0 END) AS UNSIGNED) AS `readmail`, COUNT(*) AS `totalmail` FROM mail WHERE `receiver` = ?");
    conn.prepareStatement(CHAR_SEL_PINFO_XP, "SELECT xp FROM characters WHERE guid = ?");

    conn.prepareStatement(CHAR_SEL_CHAR_GUID_NAME_BY_ACC, "SELECT guid, name FROM characters WHERE account = ?");
    conn.prepareStatement(CHAR_SEL_POOL_QUEST_SAVE, "SELECT quest_id FROM pool_quest_save WHERE pool_id = ?");
    conn.prepareStatement(CHAR_SEL_CHARACTER_AT_LOGIN, "SELECT at_login FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_FACTIONCHANGE_DATA, "SELECT race, class, level, at_login FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_INSTANCE, "SELECT data, completedEncounters FROM instance WHERE map = ? AND id = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_COD_ITEM_MAIL, "SELECT id, messageType, mailTemplateId, sender, subject, body, money, has_items FROM mail WHERE receiver = ? AND has_items <> 0 AND cod <> 0");
    conn.prepareStatement(CHAR_SEL_CHAR_SOCIAL, "SELECT DISTINCT guid FROM character_social WHERE friend = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_OLD_CHARS, "SELECT guid, deleteInfos_Account FROM characters WHERE deleteDate IS NOT NULL AND deleteDate < ?");
    conn.prepareStatement(CHAR_SEL_MAIL, "SELECT id, messageType, sender, receiver, subject, body, has_items, expire_time, deliver_time, money, cod, checked, stationery, mailTemplateId FROM mail WHERE receiver = ? ORDER BY id DESC");
    conn.prepareStatement(CHAR_SEL_CHAR_PLAYERBYTES2, "SELECT playerBytes2 FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_GUID_BY_NAME, "SELECT guid FROM characters WHERE name = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_AURA_FROZEN, "DELETE FROM character_aura WHERE guid = ? AND spell = 9454");
    conn.prepareStatement(CHAR_SEL_CHAR_INVENTORY_COUNT_ITEM, "SELECT COUNT(itemEntry) FROM character_inventory ci INNER JOIN item_instance ii ON ii.guid = ci.item WHERE itemEntry = ?");
    conn.prepareStatement(CHAR_SEL_MAIL_COUNT_ITEM, "SELECT COUNT(itemEntry) FROM mail_items mi INNER JOIN item_instance ii ON ii.guid = mi.item_guid WHERE itemEntry = ?");
    conn.prepareStatement(CHAR_SEL_AUCTIONHOUSE_COUNT_ITEM,"SELECT COUNT(itemEntry) FROM auctionhouse ah INNER JOIN item_instance ii ON ii.guid = ah.itemguid WHERE itemEntry = ?");
    conn.prepareStatement(CHAR_SEL_GUILD_BANK_COUNT_ITEM, "SELECT COUNT(itemEntry) FROM guild_bank_item gbi INNER JOIN item_instance ii ON ii.guid = gbi.item_guid WHERE itemEntry = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_INVENTORY_ITEM_BY_ENTRY, "SELECT ci.item, cb.slot AS bag, ci.slot, ci.guid, c.account, c.name FROM characters c INNER JOIN character_inventory ci ON ci.guid = c.guid INNER JOIN item_instance ii ON ii.guid = ci.item LEFT JOIN character_inventory cb ON cb.item = ci.bag WHERE ii.itemEntry = ? LIMIT ?");
    conn.prepareStatement(CHAR_SEL_MAIL_ITEMS_BY_ENTRY, "SELECT mi.item_guid, m.sender, m.receiver, cs.account, cs.name, cr.account, cr.name FROM mail m INNER JOIN mail_items mi ON mi.mail_id = m.id INNER JOIN item_instance ii ON ii.guid = mi.item_guid INNER JOIN characters cs ON cs.guid = m.sender INNER JOIN characters cr ON cr.guid = m.receiver WHERE ii.itemEntry = ? LIMIT ?");
    conn.prepareStatement(CHAR_SEL_AUCTIONHOUSE_ITEM_BY_ENTRY, "SELECT  ah.itemguid, ah.itemowner, c.account, c.name FROM auctionhouse ah INNER JOIN characters c ON c.guid = ah.itemowner INNER JOIN item_instance ii ON ii.guid = ah.itemguid WHERE ii.itemEntry = ? LIMIT ?");
    conn.prepareStatement(CHAR_SEL_GUILD_BANK_ITEM_BY_ENTRY, "SELECT gi.item_guid, gi.guildid, g.name FROM guild_bank_item gi INNER JOIN guild g ON g.guildid = gi.guildid INNER JOIN item_instance ii ON ii.guid = gi.item_guid WHERE ii.itemEntry = ? LIMIT ?");
    conn.prepareStatement(CHAR_DEL_CHAR_ACHIEVEMENT, "DELETE FROM character_achievement WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_ACHIEVEMENT_PROGRESS_BY_CRITERIA, "DELETE FROM character_achievement_progress WHERE guid = ? AND criteria = ?");
    conn.prepareStatement(CHAR_INS_CHAR_ACHIEVEMENT_PROGRESS, "INSERT INTO character_achievement_progress (guid, criteria, counter, date) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHAR_REPUTATION_BY_FACTION, "DELETE FROM character_reputation WHERE guid = ? AND faction = ?");
    conn.prepareStatement(CHAR_INS_CHAR_REPUTATION_BY_FACTION, "INSERT INTO character_reputation (guid, faction, standing, flags) VALUES (?, ?, ? , ?)");
    conn.prepareStatement(CHAR_DEL_ITEM_REFUND_INSTANCE, "DELETE FROM item_refund_instance WHERE item_guid = ?");
    conn.prepareStatement(CHAR_INS_ITEM_REFUND_INSTANCE, "INSERT INTO item_refund_instance (item_guid, player_guid, paidMoney, paidExtendedCost) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GROUP, "DELETE FROM groups WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_CHAR_GIFT, "INSERT INTO character_gifts (guid, item_guid, entry, flags) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_INSTANCE_BY_INSTANCE, "DELETE FROM instance WHERE id = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_INSTANCE_BY_INSTANCE, "DELETE FROM character_instance WHERE instance = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_INSTANCE_BY_MAP_DIFF, "DELETE FROM character_instance USING character_instance LEFT JOIN instance ON character_instance.instance = id WHERE map = ? and difficulty = ?");
    conn.prepareStatement(CHAR_DEL_GROUP_INSTANCE_BY_MAP_DIFF, "DELETE FROM group_instance USING group_instance LEFT JOIN instance ON group_instance.instance = id WHERE map = ? and difficulty = ?");
    conn.prepareStatement(CHAR_DEL_INSTANCE_BY_MAP_DIFF, "DELETE FROM instance WHERE map = ? and difficulty = ?");
    conn.prepareStatement(CHAR_INS_PETITION, "INSERT INTO petition (ownerguid, petitionguid, name, type) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_PETITION_BY_GUID, "DELETE FROM petition WHERE petitionguid = ?");
    conn.prepareStatement(CHAR_DEL_PETITION_SIGNATURE_BY_GUID, "DELETE FROM petition_sign WHERE petitionguid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_DECLINED_NAME, "DELETE FROM character_declinedname WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_CHAR_DECLINED_NAME, "INSERT INTO character_declinedname (guid, genitive, dative, accusative, instrumental, prepositional) VALUES (?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_FACTION_OR_RACE, "UPDATE characters SET name = ?, race = ?, at_login = at_login & ~ ? WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_SKILL_LANGUAGES, "DELETE FROM character_skills WHERE skill IN (98, 113, 759, 111, 313, 109, 115, 315, 673, 137, 905, 906, 907, 791, 792) AND guid = ?");
    conn.prepareStatement(CHAR_INS_CHAR_SKILL_LANGUAGE, "INSERT INTO `character_skills` (guid, skill, value, max) VALUES (?, ?, 300, 300)");
    conn.prepareStatement(CHAR_UPD_CHAR_TAXI_PATH, "UPDATE characters SET taxi_path = '' WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_TAXIMASK, "UPDATE characters SET taximask = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_QUESTSTATUS, "DELETE FROM character_queststatus WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_SOCIAL_BY_GUID, "DELETE FROM character_social WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_SOCIAL_BY_FRIEND, "DELETE FROM character_social WHERE friend = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_ACHIEVEMENT_BY_ACHIEVEMENT, "DELETE FROM character_achievement WHERE guid = ? AND achievement = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_ACHIEVEMENT, "UPDATE character_achievement SET achievement = ? WHERE guid = ? AND achievement = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_INVENTORY_FACTION_CHANGE, "UPDATE item_instance SET itemEntry = ? WHERE owner_guid = ? AND itemEntry = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_SPELL_BY_SPELL, "DELETE FROM character_spell WHERE guid = ? AND spell = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_SPELL_FACTION_CHANGE, "UPDATE character_spell SET spell = ? WHERE guid = ? AND spell = ?");
    conn.prepareStatement(CHAR_SEL_CHAR_REP_BY_FACTION, "SELECT standing FROM character_reputation WHERE guid = ? AND faction = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_REP_BY_FACTION, "DELETE FROM character_reputation WHERE guid = ? AND faction = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_REP_FACTION_CHANGE, "UPDATE character_reputation SET faction = ? where guid = ? AND faction = ?");
    conn.prepareStatement(CHAR_RES_CHAR_TITLES_FACTION_CHANGE, "UPDATE characters SET chosenTitle = 0 WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_SPELL_COOLDOWN, "DELETE FROM character_spell_cooldown WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHARACTER, "DELETE FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_AURA, "DELETE FROM character_aura WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_AURA_EFFECT, "DELETE FROM character_aura_effect WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_GIFT, "DELETE FROM character_gifts WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_INSTANCE, "DELETE FROM character_instance WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_GLYPHS, "DELETE FROM character_glyphs WHERE guid = ?");
    conn.prepareStatement(CHAR_UDP_CHAR_MONEY, "UPDATE characters SET money = ? WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_CHAR_ACTION, "INSERT INTO character_action (guid, spec, button, action, type) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_CHAR_ACTION, "UPDATE character_action SET action = ?, type = ? WHERE guid = ? AND button = ? AND spec = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_ACTION_BY_BUTTON_SPEC, "DELETE FROM character_action WHERE guid = ? and button = ? and spec = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_INVENTORY_BY_ITEM, "DELETE FROM character_inventory WHERE item = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_INVENTORY_BY_BAG_SLOT, "DELETE FROM character_inventory WHERE guid = ? AND bag = ? AND slot = ?");
    conn.prepareStatement(CHAR_UPD_MAIL, "UPDATE mail SET has_items = ?, expire_time = ?, deliver_time = ?, money = ?, cod = ?, checked = ? WHERE id = ?");
    conn.prepareStatement(CHAR_REP_CHAR_QUESTSTATUS, "REPLACE INTO character_queststatus (guid, quest, status, explored, timer) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_BY_QUEST, "DELETE FROM character_queststatus WHERE guid = ? AND quest = ?");
    conn.prepareStatement(CHAR_REP_CHAR_QUESTSTATUS_OBJECTIVE, "REPLACE INTO `character_queststatus_objective` (`guid`, `objectiveId`, `amount`) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_OBJECTIVE, "DELETE FROM `character_queststatus_objective` WHERE `guid` = ? AND `objectiveId` = ?");
    conn.prepareStatement(CHAR_REP_CHAR_QUESTSTATUS_REWARDED, "REPLACE INTO character_queststatus_rewarded (guid, quest, active) VALUES (?, ?, 1)");
    conn.prepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_REWARDED_BY_QUEST, "DELETE FROM character_queststatus_rewarded WHERE guid = ? AND quest = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_QUESTSTATUS_REWARDED_FACTION_CHANGE, "UPDATE character_queststatus_rewarded SET quest = ? WHERE guid = ? AND quest = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_QUESTSTATUS_REWARDED_ACTIVE, "UPDATE character_queststatus_rewarded SET active = 1 WHERE guid = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_QUESTSTATUS_REWARDED_ACTIVE_BY_QUEST, "UPDATE character_queststatus_rewarded SET active = 0 WHERE guid = ? AND quest = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_SKILL_BY_SKILL, "DELETE FROM character_skills WHERE guid = ? AND skill = ?");
    conn.prepareStatement(CHAR_INS_CHAR_SKILLS, "INSERT INTO character_skills (guid, skill, value, max) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UDP_CHAR_SKILLS, "UPDATE character_skills SET value = ?, max = ? WHERE guid = ? AND skill = ?");
    conn.prepareStatement(CHAR_INS_CHAR_SPELL, "INSERT INTO character_spell (guid, spell, active, disabled) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHAR_STATS, "DELETE FROM character_stats WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_CHAR_STATS, "INSERT INTO character_stats (guid, maxhealth, maxpower1, maxpower2, maxpower3, maxpower4, maxpower5, strength, agility, stamina, intellect, spirit, armor, resHoly, resFire, resNature, resFrost, resShadow, resArcane, blockPct, dodgePct, parryPct, critPct, rangedCritPct, spellCritPct, attackPower, rangedAttackPower, spellPower, resilience) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_PETITION_BY_OWNER, "DELETE FROM petition WHERE ownerguid = ?");
    conn.prepareStatement(CHAR_DEL_PETITION_SIGNATURE_BY_OWNER, "DELETE FROM petition_sign WHERE ownerguid = ?");
    conn.prepareStatement(CHAR_DEL_PETITION_BY_OWNER_AND_TYPE, "DELETE FROM petition WHERE ownerguid = ? AND type = ?");
    conn.prepareStatement(CHAR_DEL_PETITION_SIGNATURE_BY_OWNER_AND_TYPE, "DELETE FROM petition_sign WHERE ownerguid = ? AND type = ?");
    conn.prepareStatement(CHAR_INS_CHAR_GLYPHS, "INSERT INTO character_glyphs (guid, spec, glyph1, glyph2, glyph3, glyph4, glyph5, glyph6) VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHAR_TALENT_BY_SPELL_SPEC, "DELETE FROM character_talent WHERE guid = ? and spell = ? and spec = ?");
    conn.prepareStatement(CHAR_INS_CHAR_TALENT, "INSERT INTO character_talent (guid, spell, spec) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHAR_ACTION_EXCEPT_SPEC, "DELETE FROM character_action WHERE spec<>? AND guid = ?");
    conn.prepareStatement(CHAR_UPD_CHAR_LIST_SLOT, "UPDATE characters SET slot = ? WHERE guid = ?");

    conn.prepareStatement(CHAR_SEL_CHAR_VOID_STORAGE, "SELECT itemId, itemEntry, slot, creatorGuid, randomProperty, suffixFactor FROM character_void_storage WHERE playerGuid = ?");
    conn.prepareStatement(CHAR_REP_CHAR_VOID_STORAGE_ITEM, "REPLACE INTO character_void_storage (itemId, playerGuid, itemEntry, slot, creatorGuid, randomProperty, suffixFactor) VALUES (?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_CHAR_VOID_STORAGE_ITEM_BY_SLOT, "DELETE FROM character_void_storage WHERE playerGuid = ? AND slot = ?");

    // Guild Finder
    conn.prepareStatement(CHAR_REP_GUILD_FINDER_APPLICANT, "REPLACE INTO guild_finder_applicant (guildId, playerGuid, availability, classRole, interests, comment, submitTime) VALUES(?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD_FINDER_APPLICANT, "DELETE FROM guild_finder_applicant WHERE guildId = ? AND playerGuid = ?");
    conn.prepareStatement(CHAR_REP_GUILD_FINDER_GUILD_SETTINGS, "REPLACE INTO guild_finder_guild_settings (guildId, availability, classRoles, interests, level, listed, comment) VALUES(?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_GUILD_FINDER_GUILD_SETTINGS, "DELETE FROM guild_finder_guild_settings WHERE guildId = ?");

    // Pet
    conn.prepareStatement(CHAR_SEL_PET_INFO_BY_ID, "SELECT pet_info.id, entry, model_id, level, experience, react_state, name, renamed, cur_health, cur_mana, save_time, create_spell_id, type, specialization, slot FROM pet_info INNER JOIN pet_slot on pet_info.id = pet_slot.id WHERE pet_info.id = ?");
    conn.prepareStatement(CHAR_SEL_PET_INFO_BY_ENTRY, "SELECT pet_info.id, entry, model_id, level, experience, react_state, name, renamed, cur_health, cur_mana, save_time, create_spell_id, type, specialization, slot FROM pet_info INNER JOIN pet_slot on pet_info.id = pet_slot.id WHERE owner = ? AND entry = ?");
    conn.prepareStatement(CHAR_SEL_PET_INFO_BY_SLOT, "SELECT t1.id, t1.entry, t1.model_id, t1.level, t1.experience, t1.react_state, t1.name, t1.renamed, t1.cur_health, t1.cur_mana, t1.save_time, t1.create_spell_id, t1.type, t1.specialization "
                      "FROM pet_info AS t1 JOIN pet_slot AS t2 ON t1.id = t2.id WHERE t1.owner = ? AND t2.slot = ?");
    conn.prepareStatement(CHAR_SEL_PET_STAMPEDE, "SELECT t1.entry FROM pet_info AS t1 JOIN pet_slot AS t2 ON t1.id = t2.id WHERE t1.owner = ? AND t2.slot != ?");
    conn.prepareStatement(CHAR_DEL_PET_DECLINED_NAME, "DELETE FROM pet_declined_name WHERE id = ?");
    conn.prepareStatement(CHAR_ADD_PET_DECLINED_NAME, "INSERT INTO pet_declined_name (id, genitive, dative, accusative, instrumental, prepositional) VALUES (?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_SEL_PET_AURA, "SELECT slot, caster_guid, spell, effect_mask, recalculate_mask, stackcount, maxduration, remaintime, remaincharges FROM pet_aura WHERE id = ?");
    conn.prepareStatement(CHAR_SEL_PET_AURA_EFFECT, "SELECT slot, effect, amount, baseamount FROM pet_aura_effect WHERE id = ?");
    conn.prepareStatement(CHAR_SEL_PET_SPELL, "SELECT spell, active FROM pet_spell WHERE id = ?");
    conn.prepareStatement(CHAR_SEL_PET_SPELL_COOLDOWN, "SELECT spell, time FROM pet_spell_cooldown WHERE id = ?");
    conn.prepareStatement(CHAR_SEL_PET_DECLINED_NAME, "SELECT genitive, dative, accusative, instrumental, prepositional FROM pet_declined_name WHERE id = ?");
    conn.prepareStatement(CHAR_DEL_PET_AURAS, "DELETE FROM pet_aura WHERE id = ?");
    conn.prepareStatement(CHAR_DEL_PET_AURA_EFFECTS, "DELETE FROM pet_aura_effect WHERE id = ?");
    conn.prepareStatement(CHAR_DEL_PET_SPELL_COOLDOWNS, "DELETE FROM pet_spell_cooldown WHERE id = ?");
    conn.prepareStatement(CHAR_INS_PET_SPELL_COOLDOWN, "INSERT INTO pet_spell_cooldown (id, spell, time) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_PET_SPELL_BY_SPELL, "DELETE FROM pet_spell WHERE id = ? and spell = ?");
    conn.prepareStatement(CHAR_DEL_INVALID_PET_SPELL, "DELETE FROM pet_spell WHERE spell = ?");
    conn.prepareStatement(CHAR_INS_PET_SPELL, "INSERT INTO pet_spell (id, spell, active) VALUES (?, ?, ?)");
    conn.prepareStatement(CHAR_INS_PET_AURA, "INSERT INTO pet_aura (id, slot, caster_guid, spell, effect_mask, recalculate_mask, stackcount, maxduration, remaintime, remaincharges) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_INS_PET_AURA_EFFECT, "INSERT INTO pet_aura_effect (id, slot, effect, amount, baseamount) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_SEL_PET_ACTION_BAR, "SELECT `index`, `type`, `action` FROM `pet_action_bar` WHERE `id` = ?");
    conn.prepareStatement(CHAR_INS_PET_ACTION_BAR, "INSERT INTO `pet_action_bar` (`id`, `index`, `type`, `action`) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_PET_ACTION_BAR, "UPDATE `pet_action_bar` SET `type`=?, `action`=? WHERE `id`=? AND `index`=?");
    conn.prepareStatement(CHAR_INS_CURRENT_PET_ID, "INSERT INTO character_pet (guid, pet_id) VALUES (?, ?) "
                      "ON DUPLICATE KEY UPDATE pet_id = VALUES(pet_id)");
    conn.prepareStatement(CHAR_DEL_CURRENT_PET_ID, "DELETE FROM character_pet WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_PET_INFO, "INSERT INTO pet_info (id, `owner`, entry, model_id, create_spell_id, `type`, `level`, experience, react_state, `name`, renamed, cur_health, cur_mana, save_time, specialization)"
                      " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_UPD_PET_INFO, "UPDATE pet_info SET `owner`=?, entry=?, model_id=?, create_spell_id=?, `type`=?, `level`=?, experience=?, react_state=?, `name`=?, renamed=?, cur_health=?,"
                      "cur_mana=?, save_time=?, specialization=? WHERE id=?");
    conn.prepareStatement(CHAR_SEL_PET_INFO_EXCEPT, "SELECT id FROM pet_info WHERE `owner` = ? AND id <> ?");
    conn.prepareStatement(CHAR_SEL_PET_INFO_FOR_SLOT_CHANGE, "SELECT t2.slot, t1.entry, t1.id FROM pet_info AS t1 JOIN pet_slot AS t2 ON t1.id = t2.id WHERE t1.id = ?");
    conn.prepareStatement(CHAR_SEL_NEXT_PET_NUMBER, "SELECT MAX(id) + 1 FROM pet_info");
    conn.prepareStatement(CHAR_DEL_PET_INFO_BY_ID, "DELETE FROM pet_info WHERE id = ?");
    conn.prepareStatement(CHAR_SEL_PET_SPELL_EXCEPT, "SELECT DISTINCT t1.spell FROM pet_spell AS t1 JOIN pet_info AS t2 ON t2.id = t1.id WHERE t2.owner = ? AND t2.id <> ?");
    conn.prepareStatement(CHAR_SEL_PET_INFO_LIST, "SELECT t2.slot, t1.id, t1.entry, t1.level, t1.name FROM pet_info AS t1 "
                      "JOIN pet_slot AS t2 ON t1.id = t2.id WHERE t1.owner = ? AND t2.slot BETWEEN ? AND ?");
    conn.prepareStatement(CHAR_SEL_PET_SLOT_LIST, "SELECT t1.slot, t1.id FROM pet_slot as t1 JOIN pet_info as t2 on t1.id = t2.id WHERE t2.owner = ? AND t1.slot BETWEEN ? AND ?");
    conn.prepareStatement(CHAR_DEL_PET_SLOT_BY_OWNER, "DELETE t1 FROM pet_slot AS t1 JOIN pet_info as t2 on t1.id = t2.id WHERE t2.owner = ? AND t1.slot = ?");
    conn.prepareStatement(CHAR_DEL_PET_SLOT_BY_PET_ID, "DELETE FROM pet_slot WHERE id = ?");
    conn.prepareStatement(CHAR_INS_PET_SLOT, "INSERT INTO pet_slot (id, slot) VALUES (?, ?)");

    // Titles
    conn.prepareStatement(CHAR_SEL_CHAR_KNOWN_TITLES, "SELECT offset, value from character_known_titles WHERE guid = ?");
    conn.prepareStatement(CHAR_DEL_CHAR_KNOWN_TITLES, "DELETE FROM character_known_titles WHERE guid = ?");
    conn.prepareStatement(CHAR_INS_CHAR_KNOWN_TITLES, "INSERT INTO character_known_titles (guid, offset, value) VALUES (?, ?, ?)");

    conn.prepareStatement(CHAR_SEL_NAME_QUERY_SIMPLE, "SELECT name, race, gender, class, level FROM characters WHERE guid = ?");
    conn.prepareStatement(CHAR_SEL_NAME_QUERY_DECLINED, "SELECT name, race, gender, class, level, genitive, dative, accusative, instrumental, prepositional "
                          "FROM characters LEFT JOIN character_declinedname ON characters.guid = character_declinedname.guid "
                          "WHERE characters.guid = ?");

    // Black Market
    conn.prepareStatement(CHAR_INS_BLACKMARKET_AUCTION, "INSERT INTO blackmarket (id, templateId, startTime, bid, bidder, bidderCount) VALUES (?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_DEL_BLACKMARKET_AUCTION, "DELETE FROM blackmarket WHERE id = ?");
    conn.prepareStatement(CHAR_UPD_BLACKMARKET_AUCTION, "UPDATE blackmarket SET bid = ?, bidder = ?, bidderCount = ? WHERE id = ?");
    conn.prepareStatement(CHAR_SEL_BLACKMARKET_AUCTIONS, "SELECT id, templateId, startTime, bid, bidder, bidderCount FROM blackmarket");

    // Items that hold loot or money
    conn.prepareStatement(CHAR_SEL_ITEMCONTAINER_ITEMS, "SELECT item_id, item_count, follow_rules, ffa, blocked, counted, under_threshold, needs_quest, rnd_prop, rnd_suffix FROM item_loot_items WHERE container_id = ?");
    conn.prepareStatement(CHAR_DEL_ITEMCONTAINER_ITEMS, "DELETE FROM item_loot_items WHERE container_id = ?");
    conn.prepareStatement(CHAR_DEL_ITEMCONTAINER_ITEM, "DELETE FROM item_loot_items WHERE container_id = ? AND item_id = ?");
    conn.prepareStatement(CHAR_INS_ITEMCONTAINER_ITEMS, "INSERT INTO item_loot_items (container_id, item_id, item_count, follow_rules, ffa, blocked, counted, under_threshold, needs_quest, rnd_prop, rnd_suffix) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(CHAR_SEL_ITEMCONTAINER_MONEY, "SELECT money FROM item_loot_money WHERE container_id = ?");
    conn.prepareStatement(CHAR_DEL_ITEMCONTAINER_MONEY, "DELETE FROM item_loot_money WHERE container_id = ?");
    conn.prepareStatement(CHAR_INS_ITEMCONTAINER_MONEY, "INSERT INTO item_loot_money (container_id, money) VALUES (?, ?)");

    // Realm transfer
    conn.prepareStatement(CHAR_UPD_TRANSFER_DATA, "INSERT INTO transfer_outcoming_queue (char_guid, comp_char_dump, orig_dump_size) VALUES(?, ?, ?)");
    conn.prepareStatement(CHAR_SEL_TRANSFER_DATA, "SELECT id, acc_guid, comp_char_dump, LENGTH(comp_char_dump), orig_dump_size FROM transfer_incoming_queue WHERE new_char_guid = 0");
}

} // namespace CharacterDatabaseConnection
