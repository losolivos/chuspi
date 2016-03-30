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

#include "LoginDatabase.h"
#include "MySQLConnection.h"

namespace LoginDatabaseConnection {

void DoPrepareStatements(MySQLConnection &conn)
{
    if (!conn.reconnecting())
        conn.setMaxPreparedStatements(MAX_LOGINDATABASE_STATEMENTS);

    conn.prepareStatement(LOGIN_SEL_REALMLIST, "SELECT id, name, address, port, icon, flag, timezone, allowedSecurityLevel, population, gamebuild FROM realmlist WHERE flag <> 3 ORDER BY name");
    conn.prepareStatement(LOGIN_DEL_EXPIRED_IP_BANS, "DELETE FROM ip_banned WHERE unbandate<>bandate AND unbandate<=UNIX_TIMESTAMP()");
    conn.prepareStatement(LOGIN_UPD_EXPIRED_ACCOUNT_BANS, "UPDATE account_banned SET active = 0 WHERE active = 1 AND unbandate<>bandate AND unbandate<=UNIX_TIMESTAMP()");
    conn.prepareStatement(LOGIN_SEL_IP_BANNED, "SELECT * FROM ip_banned WHERE ip = ?");
    conn.prepareStatement(LOGIN_INS_IP_AUTO_BANNED, "INSERT INTO ip_banned (ip, bandate, unbandate, bannedby, banreason) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, 'Trinity realmd', 'Failed login autoban')");
    conn.prepareStatement(LOGIN_SEL_IP_BANNED_ALL, "SELECT ip, bandate, unbandate, bannedby, banreason FROM ip_banned WHERE (bandate = unbandate OR unbandate > UNIX_TIMESTAMP()) ORDER BY unbandate");
    conn.prepareStatement(LOGIN_SEL_IP_BANNED_BY_IP, "SELECT ip, bandate, unbandate, bannedby, banreason FROM ip_banned WHERE (bandate = unbandate OR unbandate > UNIX_TIMESTAMP()) AND ip LIKE CONCAT('%%', ?, '%%') ORDER BY unbandate");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_BANNED, "SELECT bandate, unbandate FROM account_banned WHERE id = ? AND active = 1");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_BANNED_ALL, "SELECT account.id, username FROM account, account_banned WHERE account.id = account_banned.id AND active = 1 GROUP BY account.id, username");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_BANNED_BY_USERNAME, "SELECT account.id, username FROM account, account_banned WHERE account.id = account_banned.id AND active = 1 AND username LIKE CONCAT('%%', ?, '%%') GROUP BY account.id, username");
    conn.prepareStatement(LOGIN_INS_ACCOUNT_AUTO_BANNED, "INSERT INTO account_banned VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, 'Trinity realmd', 'Failed login autoban', 1)");
    conn.prepareStatement(LOGIN_DEL_ACCOUNT_BANNED, "DELETE FROM account_banned WHERE id = ?");
    conn.prepareStatement(LOGIN_SEL_SESSIONKEY, "SELECT a.sessionkey, a.id, aa.gmlevel  FROM account a LEFT JOIN account_access aa ON (a.id = aa.id) WHERE username = ?");
    conn.prepareStatement(LOGIN_UPD_VS, "UPDATE account SET v = ?, s = ? WHERE username = ?");
    conn.prepareStatement(LOGIN_UPD_LOGONPROOF, "UPDATE account SET sessionkey = ?, last_ip = ?, last_login = NOW(), locale = ?, failed_logins = 0, os = ? WHERE username = ?");
    conn.prepareStatement(LOGIN_SEL_LOGONCHALLENGE, "SELECT a.sha_pass_hash, a.id, a.locked, a.last_ip, aa.gmlevel, a.v, a.s, a.token_key FROM account a LEFT JOIN account_access aa ON (a.id = aa.id) WHERE a.username = ?");
    conn.prepareStatement(LOGIN_UPD_FAILEDLOGINS, "UPDATE account SET failed_logins = failed_logins + 1 WHERE username = ?");
    conn.prepareStatement(LOGIN_SEL_FAILEDLOGINS, "SELECT id, failed_logins FROM account WHERE username = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_ID_BY_NAME, "SELECT id FROM account WHERE username = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_LIST_BY_NAME, "SELECT id, username FROM account WHERE username = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_INFO_BY_NAME, "SELECT id, sessionkey, last_ip, locked, v, s, expansion, mutetime, locale, recruiter, os FROM account WHERE username = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_LIST_BY_EMAIL, "SELECT id, username FROM account WHERE email = ?");
    conn.prepareStatement(LOGIN_SEL_NUM_CHARS_ON_REALM, "SELECT numchars FROM realmcharacters WHERE realmid = ? AND acctid= ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_BY_IP, "SELECT id, username FROM account WHERE last_ip = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_BY_ID, "SELECT 1 FROM account WHERE id = ?");
    conn.prepareStatement(LOGIN_INS_IP_BANNED, "INSERT INTO ip_banned (ip, bandate, unbandate, bannedby, banreason) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?)");
    conn.prepareStatement(LOGIN_DEL_IP_NOT_BANNED, "DELETE FROM ip_banned WHERE ip = ?");
    conn.prepareStatement(LOGIN_INS_ACCOUNT_BANNED, "INSERT INTO account_banned VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?, 1)");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_BANNED, "SELECT 1 FROM account_banned WHERE id = ? AND active = 1 AND bandate = unbandate");
    conn.prepareStatement(LOGIN_DEL_REALM_CHARACTERS_BY_REALM, "DELETE FROM realmcharacters WHERE acctid = ? AND realmid = ?");
    conn.prepareStatement(LOGIN_DEL_REALM_CHARACTERS, "DELETE FROM realmcharacters WHERE acctid = ?");
    conn.prepareStatement(LOGIN_INS_REALM_CHARACTERS, "INSERT INTO realmcharacters (numchars, acctid, realmid) VALUES (?, ?, ?)");
    conn.prepareStatement(LOGIN_UPD_REALM_CHARACTERS, "UPDATE realmcharacters SET numchars = ? WHERE acctid = ? AND realmid = ?");
    conn.prepareStatement(LOGIN_SEL_SUM_REALM_CHARACTERS, "SELECT SUM(numchars) FROM realmcharacters WHERE acctid = ?");
    conn.prepareStatement(LOGIN_INS_ACCOUNT, "INSERT INTO account(username, sha_pass_hash, reg_mail, email, joindate) VALUES(?, ?, ?, ?, NOW())");
    conn.prepareStatement(LOGIN_INS_REALM_CHARACTERS_INIT, "INSERT INTO realmcharacters (realmid, acctid, numchars) SELECT realmlist.id, account.id, 0 FROM realmlist, account LEFT JOIN realmcharacters ON acctid=account.id WHERE acctid IS NULL");
    conn.prepareStatement(LOGIN_UPD_EXPANSION, "UPDATE account SET expansion = ? WHERE id = ?");
    conn.prepareStatement(LOGIN_UPD_ACCOUNT_LOCK, "UPDATE account SET locked = ? WHERE id = ?");
    conn.prepareStatement(LOGIN_INS_LOG, "INSERT INTO logs (time, realm, type, level, string) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(LOGIN_UPD_USERNAME, "UPDATE account SET v = '', s = '', username = ?, sha_pass_hash = ? WHERE id = ?");
    conn.prepareStatement(LOGIN_UPD_PASSWORD, "UPDATE account SET v = '', s = '', sha_pass_hash = ? WHERE id = ?");
    conn.prepareStatement(LOGIN_UPD_EMAIL, "UPDATE account SET email = ? WHERE id = ?");
    conn.prepareStatement(LOGIN_UPD_REG_EMAIL, "UPDATE account SET reg_mail = ? WHERE id = ?");
    conn.prepareStatement(LOGIN_UPD_MUTE_TIME, "UPDATE account SET mutetime = ? , mutereason = ? , muteby = ? WHERE id = ?");
    conn.prepareStatement(LOGIN_UPD_MUTE_TIME_LOGIN, "UPDATE account SET mutetime = ? WHERE id = ?");
    conn.prepareStatement(LOGIN_UPD_LAST_IP, "UPDATE account SET last_ip = ? WHERE username = ?");
    conn.prepareStatement(LOGIN_UPD_ACCOUNT_ONLINE, "UPDATE account SET online = 1 WHERE id = ?");
    conn.prepareStatement(LOGIN_UPD_UPTIME_PLAYERS, "UPDATE uptime SET uptime = ?, maxplayers = ? WHERE realmid = ? AND starttime = ?");
    conn.prepareStatement(LOGIN_DEL_OLD_LOGS, "DELETE FROM logs WHERE (time + ?) < ?");
    conn.prepareStatement(LOGIN_DEL_ACCOUNT_ACCESS, "DELETE FROM account_access WHERE id = ?");
    conn.prepareStatement(LOGIN_DEL_ACCOUNT_ACCESS_BY_REALM, "DELETE FROM account_access WHERE id = ? AND (RealmID = ? OR RealmID = -1)");
    conn.prepareStatement(LOGIN_INS_ACCOUNT_ACCESS, "INSERT INTO account_access (id,gmlevel,RealmID) VALUES (?, ?, ?)");
    conn.prepareStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME, "SELECT id FROM account WHERE username = ?");
    conn.prepareStatement(LOGIN_GET_ACCOUNT_ACCESS_GMLEVEL, "SELECT gmlevel FROM account_access WHERE id = ?");
    conn.prepareStatement(LOGIN_GET_GMLEVEL_BY_REALMID, "SELECT gmlevel FROM account_access WHERE id = ? AND (RealmID = ? OR RealmID = -1)");
    conn.prepareStatement(LOGIN_GET_USERNAME_BY_ID, "SELECT username FROM account WHERE id = ?");
    conn.prepareStatement(LOGIN_SEL_CHECK_PASSWORD, "SELECT 1 FROM account WHERE id = ? AND sha_pass_hash = ?");
    conn.prepareStatement(LOGIN_SEL_CHECK_PASSWORD_BY_NAME, "SELECT 1 FROM account WHERE username = ? AND sha_pass_hash = ?");
    conn.prepareStatement(LOGIN_SEL_PINFO, "SELECT a.username, aa.gmlevel, a.email, a.reg_mail, a.last_ip, DATE_FORMAT(a.last_login, '%Y-%m-%d %T'), a.mutetime, a.mutereason, a.muteby, a.failed_logins, a.locked, a.OS FROM account a LEFT JOIN account_access aa ON (a.id = aa.id AND (aa.RealmID = ? OR aa.RealmID = -1)) WHERE a.id = ?");
    conn.prepareStatement(LOGIN_SEL_PINFO_BANS, "SELECT unbandate, bandate = unbandate, bannedby, banreason FROM account_banned WHERE id = ? AND active ORDER BY bandate ASC LIMIT 1");
    conn.prepareStatement(LOGIN_SEL_GM_ACCOUNTS, "SELECT a.username, aa.gmlevel FROM account a, account_access aa WHERE a.id=aa.id AND aa.gmlevel >= ? AND (aa.realmid = -1 OR aa.realmid = ?)");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_INFO, "SELECT a.username, a.last_ip, aa.gmlevel, a.expansion FROM account a LEFT JOIN account_access aa ON (a.id = aa.id) WHERE a.id = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_ACCESS_GMLEVEL_TEST, "SELECT 1 FROM account_access WHERE id = ? AND gmlevel > ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_ACCESS, "SELECT a.id, aa.gmlevel, aa.RealmID FROM account a LEFT JOIN account_access aa ON (a.id = aa.id) WHERE a.username = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_RECRUITER, "SELECT 1 FROM account WHERE recruiter = ?");
    conn.prepareStatement(LOGIN_SEL_BANS, "SELECT 1 FROM account_banned WHERE id = ? AND active = 1 UNION SELECT 1 FROM ip_banned WHERE ip = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_WHOIS, "SELECT username, email, last_ip FROM account WHERE id = ?");
    conn.prepareStatement(LOGIN_SEL_REALMLIST_SECURITY_LEVEL, "SELECT allowedSecurityLevel from realmlist WHERE id = ?");
    conn.prepareStatement(LOGIN_DEL_ACCOUNT, "DELETE FROM account WHERE id = ?");
    conn.prepareStatement(LOGIN_GET_EMAIL_BY_ID, "SELECT email FROM account WHERE id = ?");

    conn.prepareStatement(LOGIN_SEL_ACCOUNT_ACHIEVEMENTS, "SELECT first_guid, achievement, date FROM account_achievement WHERE account = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_CRITERIAPROGRESS, "SELECT criteria, counter, date FROM account_achievement_progress WHERE account = ?");
    conn.prepareStatement(LOGIN_SEL_ACCOUNT_SPELL, "SELECT spell, active, disabled FROM account_spell WHERE accountId = ?");
    conn.prepareStatement(LOGIN_INS_ACCOUNT_SPELL, "INSERT INTO account_spell (accountId, spell, active, disabled) VALUES (?, ?, ?, ?)");
    conn.prepareStatement(LOGIN_DEL_ACCOUNT_SPELL_BY_SPELL, "DELETE FROM account_spell WHERE accountId = ? AND spell = ?");

    conn.prepareStatement(LOGIN_SEL_ACCOUNT_BATTLE_PETS, "SELECT id, species, nickname, timestamp, level, xp, health, quality, breed, flags FROM account_battle_pet WHERE accountId = ?");
    conn.prepareStatement(LOGIN_INS_ACCOUNT_BATTLE_PET, "INSERT INTO account_battle_pet (accountId, id, species, nickname, timestamp, level, xp, health, quality, breed, flags) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    conn.prepareStatement(LOGIN_DEL_ACCOUNT_BATTLE_PET, "DELETE FROM account_battle_pet WHERE accountId = ? AND id = ?");

    conn.prepareStatement(LOGIN_SEL_ACCOUNT_BATTLE_PET_SLOTS, "SELECT slot1, slot2, slot3, flags FROM account_battle_pet_slot WHERE accountId = ?");
    conn.prepareStatement(LOGIN_INS_ACCOUNT_BATTLE_PET_SLOTS, "INSERT INTO account_battle_pet_slot (accountId, slot1, slot2, slot3, flags) VALUES (?, ?, ?, ?, ?)");
    conn.prepareStatement(LOGIN_DEL_ACCOUNT_BATTLE_PET_SLOTS, "DELETE FROM account_battle_pet_slot WHERE accountId = ?");
}

} // namespace LoginDatabaseConnection
