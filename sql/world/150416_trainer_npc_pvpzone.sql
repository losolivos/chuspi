/*correct trainer npc para luego mejor
DELETE FROM `creature` WHERE `id`=55184 and `map`=1;
DELETE FROM `creature` WHERE `id`=53714 and `map`=1;
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'53714','1','0','0','1','1','0','0','16214.9','16253.4','13.5315','1.78202','300','0','0','84','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'53714','1','0','0','1','1','0','0','3311.07','-4601.4','266.614','1.16288','300','0','0','84','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'53714','1','0','0','1','1','0','0','3308.68','-4600.02','266.771','0.872281','300','0','0','84','0','0','0','0','0','2048','0','0',NULL);
*/

-- tradear item q da 150honor a otra pj de la misma cuenta item 44115/costo 200honor
insert into `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `type`) values('811112','0','44115','0','0','3372','1');
insert into `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `type`) values('811116','0','44115','0','0','3372','1');
insert into `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `type`) values('811111','0','44115','0','0','3372','1');
insert into `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `type`) values('811113','0','44115','0','0','3372','1');
