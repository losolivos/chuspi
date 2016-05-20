-- quest [Limpiar la casa] ID: 30078
DELETE FROM `creature` WHERE `id` in (58014,58015,58017);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58014','870','0','0','1','1','0','0','-754.946','1351.14','119.305','4.93928','120','5','0','1','0','1','0','0','0','0','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58014','870','0','0','1','65535','0','0','-699.33','1266.04','162.796','4.99594','300','0','0','1941090','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58017','870','0','0','1','1','0','0','-726.781','1157.3','139.262','0.349066','120','0','0','1','0','0','0','0','0','0','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58017','870','0','0','1','65535','0','0','-649.259','1189.08','139.155','0.314957','300','0','0','5823270','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58015','870','0','0','1','1','0','0','-747.986','1321.48','146.797','1.78024','120','5','0','1','0','1','0','0','0','0','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58015','870','0','0','1','65535','0','0','-655.24','1250.15','154.813','2.18028','300','0','0','679382','0','0','0','0','0','2048','0','0',NULL);

-- [Legado] ID: 29949
DELETE FROM `quest_poi` WHERE `questid` IN (29949);
INSERT INTO `quest_poi` (`questid`, `id`, `objIndex`, `mapid`, `WorldMapAreaId`, `unk2`, `unk3`, `unk4`, `FloorId`) VALUES
(29949, 0, -1, 870, 5949, 0, 0, 0, 0);

DELETE FROM `quest_poi_points` WHERE `questid` IN (29949);
INSERT INTO `quest_poi_points` (`questId`, `id`, `idx`, `x`, `y`) VALUES
(29949, 0, 0, 159, -278);

-- [La desesperación de Zhu] ID: 30090
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58410','870','0','0','1','65535','0','0','-384.258','-636.834','116.774','0.27644','300','0','0','156000','0','0','0','0','0','2048','0','0',NULL);

insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-348.209','-651.177','122.348','6.12153','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-343.507','-644.582','120.762','0.945759','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-336.497','-651.053','123.022','3.47867','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-345.595','-659.458','123.667','2.83857','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-357.907','-655.384','120.274','2.52912','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-353.589','-649.536','121.148','2.77809','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-339.385','-669.016','125.287','5.34242','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-329.233','-648.609','122.181','1.06435','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-350.133','-654.636','122.37','3.79361','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-352.645','-667.906','122.421','0.286808','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-343.629','-665.247','124.451','0.286808','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-353.305','-594.017','114.899','2.74511','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-354.621','-602.172','116.492','5.49871','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'58409','870','0','0','1','65535','0','0','-347.818','-597.024','115.223','4.98035','300','0','0','92175','0','0','0','0','0','2048','0','0',NULL);


UPDATE `creature_template` SET `faction_A`=14, `faction_H`=14 WHERE `entry`=58409;
UPDATE `creature_template` SET `faction_A`=14, `faction_H`=14 WHERE `entry`=58360;

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=58410;

DELETE FROM `smart_scripts` WHERE `entryorguid`=58410;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES 
(58410,0,0,0,11,0,100,0,0,0,0,0,11,108450,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spyglass on Spawn'),
(58410,0,1,0,8,0,100,0,110697,0,0,0,33,58410,0,0,0,0,0,7,0,0,0,0,0,0,0,'On Spell Hit - Quest Credit'),
(58410,0,2,0,8,0,100,0,110697,5,0,0,12,58360,5,0,0,0,0,7,0,0,0,0,0,0,0,' spawn');

-- [Pasaje peligroso] ID:30269
UPDATE `creature_template` SET `gossip_menu_id`=30269, `AIName`='SmartAI' WHERE `entry`=58547;

INSERT INTO `gossip_menu_option` VALUES (30269, 0, 0, "Estoy listo, Koro.", 1, 1, 0, 0, 0, 0, '');

DELETE FROM `smart_scripts` WHERE `entryorguid`=58547;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(58547,0,0,1,62,0,100,0,30269,0,0,0,33,58946,0,0,0,0,0,7,0,0,0,0,0,0,0, 'Instructor Windblade - al seleccionar gossip - killcredit');
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(58547,0,1,2,61,0,100,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0, 'Instructor Windblade - al seleccionar gossip - cerrar gossip');

INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`ErrorTextId`,`ScriptName`,`COMMENT`) VALUES
(15,30269,0,0,9,30269,0,0,0,'',"Solo muestra gossip si player tiene quest 30269 ");

-- npc text 
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES
(30269, 6502); 
INSERT INTO `npc_text` (`ID`,`text0_0`,`prob0`,`WDBVerified`) VALUES
(6502,"Estas listo para irnos. Creo que estamos casi acabados por aqui.",100,15595);

DELETE FROM `quest_poi` WHERE `questid` IN (30269);
INSERT INTO `quest_poi` (`questid`, `id`, `objIndex`, `mapid`, `WorldMapAreaId`, `unk2`, `unk3`, `unk4`, `FloorId`) VALUES
(30269, 0, -1, 870, 6049, 0, 0, 0, 0);

DELETE FROM `quest_poi_points` WHERE `questid` IN (30269);
INSERT INTO `quest_poi_points` (`questId`, `id`, `idx`, `x`, `y`) VALUES
(30269, 0, 0, -1163, 1044);

-- [Té de loto] ID: 30351
INSERT INTO `quest_poi` (`questid`, `id`, `objIndex`, `mapid`, `WorldMapAreaId`, `unk2`, `unk3`, `unk4`, `FloorId`) VALUES
(30351, 3, 0, 870, 6008, 0, 0, 0, 0);

INSERT INTO `quest_poi_points` (`questId`, `id`, `idx`, `x`, `y`) VALUES
(30351, 3, 0, -1107, 103);

insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1124.05','129.949','6.48218','3.18727','0','0','0.999739','-0.0228382','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1103.95','89.0864','6.94577','1.03449','0','0','0.49449','0.869184','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1086.89','92.0138','7.39052','5.0361','0','0','0.583914','-0.811815','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1077.73','99.2006','7.13542','0.484717','0','0','0.239993','0.970775','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1068.02','118.426','8.96008','2.40109','0','0','0.932236','0.361851','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1065.81','128.045','13.4154','1.32509','0','0','0.615126','0.788429','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1048.42','134.92','15.231','0.983444','0','0','0.472145','0.881521','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1078.1','147.02','10.497','2.59744','0','0','0.963215','0.268734','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1098.71','145.295','6.54686','5.00861','0','0','0.595019','-0.803712','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1133.59','164.557','11.919','2.75451','0','0','0.98133','0.192333','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1145.14','110.019','12.0122','4.97326','0','0','0.609129','-0.793071','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1133.28','109.919','9.9313','5.14212','0','0','0.540081','-0.841613','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1129.27','96.4883','12.0402','5.16568','0','0','0.53013','-0.847916','300','0','1','0',NULL);
insert into `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) values(NULL,'210209','870','0','0','1','65535','-1106.88','121.313','6.53081','4.05827','0','0','0.896788','-0.442459','300','0','1','0',NULL);

-- [La cerveza del principiante] ID: 31534

--primeranpc Pez Junco Tolado
UPDATE `creature_template` SET `gossip_menu_id`=70000, `AIName`='SmartAI' WHERE `entry`=58705;

INSERT INTO `gossip_menu_option` VALUES (70000, 0, 0, "Tienes algunas Picotas?", 1, 1, 0, 0, 0, 0, '');

DELETE FROM `smart_scripts` WHERE `entryorguid`=58705;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(58705,0,0,1,62,0,100,0,70000,0,0,0,56,87556,1,0,0,0,0,7,0,0,0,0,0,0,0, 'al seleccionar gossip additem'),
(58705,0,1,2,61,0,100,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0, 'al seleccionar gossip - cerrar gossip'),
(58705,0,2,0,61,0,100,0,18000,18000,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,"Say Line 0");


INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`ErrorTextId`,`ScriptName`,`COMMENT`) VALUES
(15,70000,0,0,9,31534,0,0,0,'',"Solo muestra gossip si player tiene quest 31534 ");

-- npc text 
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES
(70000, 900000); 
INSERT INTO `npc_text` (`ID`,`text0_0`,`prob0`,`WDBVerified`) VALUES
(900000,"Hola amigo $n. Supremamente increible verte denuevo.|r",100,15595);

DELETE FROM `quest_poi` WHERE `questid` IN (31534);
INSERT INTO `quest_poi` (`questid`, `id`, `objIndex`, `mapid`, `WorldMapAreaId`, `unk2`, `unk3`, `unk4`, `FloorId`) VALUES
(31534, 3, 0, 870, 5981, 0, 0, 0, 0);

DELETE FROM `quest_poi_points` WHERE `questid` IN (31534);
INSERT INTO `quest_poi_points` (`questId`, `id`, `idx`, `x`, `y`) VALUES
(31534, 3, 0, -263, 605);


DELETE FROM `creature_text` WHERE `entry`=58705;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(58705,0,0,"Awww ¡Cáscaras!. Yo me iba a comer eso! Bueno, si Ella los nesecita, no lo hare, no mas. Aca, cojelos!",12,0,100,0,0,0,"fish");


--SegundoNpc Haoan Zarpa
UPDATE `creature_template` SET `gossip_menu_id`=70001, `AIName`='SmartAI' WHERE `entry`=57402;

INSERT INTO `gossip_menu_option` VALUES (70001, 0, 0, "Tienes una Badea dulce sobrante?", 1, 1, 0, 0, 0, 0, '');

DELETE FROM `smart_scripts` WHERE `entryorguid`=57402;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(57402,0,0,1,62,0,100,0,70001,0,0,0,56,87554,1,0,0,0,0,7,0,0,0,0,0,0,0, 'al seleccionar gossip additem'),
(57402,0,1,2,61,0,100,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0, 'al seleccionar gossip - cerrar gossip'),
(57402,0,2,0,61,0,100,0,18000,18000,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,"Say Line 0");


INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`ErrorTextId`,`ScriptName`,`COMMENT`) VALUES
(15,70001,0,0,9,31534,0,0,0,'',"Solo muestra gossip si player tiene quest 31534 ");

-- npc text 
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES
(70001, 900001); 
INSERT INTO `npc_text` (`ID`,`text0_0`,`prob0`,`WDBVerified`) VALUES
(900001,"Hola vecino $n. Como esta esa bestia Mushan funcionando para ti?. Es una Belleza cierto?|r",100,15595);

DELETE FROM `creature_text` WHERE `entry`=57402;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(57402,0,0,"La pequeña Ella esta metiendo sus manos en la elaboracion de cervezas? Bueno, lo hare! le deseo suerte. La cervezeria no es algo que un novato pueda cojer asi nomas, ya sabes. Como sea, puedo darte una Badea dulce sobrante para ella.",12,0,100,0,0,0,"fish");

--tercernpc Granjero Fung
UPDATE `creature_template` SET `gossip_menu_id`=70002, `AIName`='SmartAI' WHERE `entry`=57298;

INSERT INTO `gossip_menu_option` VALUES (70002, 0, 0, "Puedo tener un Rábano Rojo?", 1, 1, 0, 0, 0, 0, '');

DELETE FROM `smart_scripts` WHERE `entryorguid`=57298;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(57298,0,0,1,62,0,100,0,70002,0,0,0,56,87553,1,0,0,0,0,7,0,0,0,0,0,0,0, 'al seleccionar gossip additem'),
(57298,0,1,2,61,0,100,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0, 'al seleccionar gossip - cerrar gossip'),
(57298,0,2,0,61,0,100,0,18000,18000,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,"Say Line 0");


INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`ErrorTextId`,`ScriptName`,`COMMENT`) VALUES
(15,70002,0,0,9,31534,0,0,0,'',"Solo muestra gossip si player tiene quest 31534 ");

-- npc text 
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES
(70002, 900002); 
INSERT INTO `npc_text` (`ID`,`text0_0`,`prob0`,`WDBVerified`) VALUES
(900002,"Hola vecino $n. Esos vegetales se ven muy bien.|r",100,15595);

DELETE FROM `creature_text` WHERE `entry`=57298;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(57298,0,0,"Un Rábano para la pequeña Ella? Bueno, claro que puedo prescindir de uno, aca tienes.",12,0,100,0,0,0,"fish");

--cuartoNPC Gina Zarpa
UPDATE `creature_template` SET `gossip_menu_id`=70003, `AIName`='SmartAI' WHERE `entry`=58706;

INSERT INTO `gossip_menu_option` VALUES (70003, 0, 0, "Puedo tener un Melocotón aterciopelado para Ella?", 1, 1, 0, 0, 0, 0, '');

DELETE FROM `smart_scripts` WHERE `entryorguid`=58706;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(58706,0,0,1,62,0,100,0,70003,0,0,0,56,87555,1,0,0,0,0,7,0,0,0,0,0,0,0, 'al seleccionar gossip additem'),
(58706,0,1,2,61,0,100,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0, 'al seleccionar gossip - cerrar gossip'),
(58706,0,2,0,61,0,100,0,18000,18000,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,"Say Line 0");


INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`ErrorTextId`,`ScriptName`,`COMMENT`) VALUES
(15,70003,0,0,9,31534,0,0,0,'',"Solo muestra gossip si player tiene quest 31534 ");

-- npc text 
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES
(70003, 900003); 
INSERT INTO `npc_text` (`ID`,`text0_0`,`prob0`,`WDBVerified`) VALUES
(900003,"Como va ese buzón funcionando para ti.|rBuena idea cierto?|r",100,15595);

DELETE FROM `creature_text` WHERE `entry`=58706;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(58706,0,0,"Te iba a cambiar un pequeño centavo por ese melocotón, pero Ella es una buena clienta en mi mercado, esta va por la casa. Solo esta vez oiste?!",12,0,100,0,0,0,"fish");

-- [El día libre de Li Li] ID: 29950
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=56138;

DELETE FROM `smart_scripts` WHERE `entryorguid`=56138;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(56138,0,0,1,19,0,100,0,29950,0,0,0,33,56546,0,0,0,0,0,7,0,0,0,0,0,0,0, 'killcredit on quest accept'),
(56138,0,1,2,61,0,100,0,29950,0,0,0,33,56547,0,0,0,0,0,7,0,0,0,0,0,0,0, 'killcredit on quest accept'),
(56138,0,2,0,61,0,100,0,29950,0,0,0,33,56548,0,0,0,0,0,7,0,0,0,0,0,0,0, 'killcredit on quest accept');

-- [Ovejita perdida] ID: 31338 y [Ovejita perdida… otra vez] ID: 31339

insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'64385','870','0','0','1','1','0','0','81.8919','1301.46','218.584','4.23541','300','0','0','393941','0','0','0','0','0','2048','0','0',NULL);
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'64386','870','0','0','1','1','0','0','112.744','812.016','160.928','1.82816','300','0','0','393941','0','0','0','0','0','2048','0','0',NULL);

UPDATE `creature_template` SET `npcflag`=1, `AIName`='SmartAI' WHERE `entry`=64385;
UPDATE `creature_template` SET `npcflag`=1, `AIName`='SmartAI' WHERE `entry`=64386;

DELETE FROM `smart_scripts` WHERE `entryorguid`=64386;
insert into `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) values
('64386','0','0','0','64','0','100','0','0','0','0','0','56','86446','1','0','0','0','0','7','0','0','0','0','0','0','0','NPC 64386');

DELETE FROM `smart_scripts` WHERE `entryorguid`=64385;
insert into `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) values
('64385','0','0','0','64','0','100','0','0','0','0','0','56','86446','1','0','0','0','0','7','0','0','0','0','0','0','0','NPC 64385');

-- [Un lobo con piel de cordero] ID: 31341
insert into `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `isActive`, `protec_anti_doublet`) values(NULL,'64389','870','0','0','1','1','0','0','42.3597','1515.74','385.421','3.38607','300','0','0','393941','0','0','0','0','0','0','0','0',NULL);

-- [La jardinera Frany y la regadera] ID: 30050
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=62377;

DELETE FROM `smart_scripts` WHERE `entryorguid`=62377;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(62377,0,0,1,19,0,100,0,30050,0,0,0,33,57284,0,0,0,0,0,7,0,0,0,0,0,0,0, 'killcredit on quest accept');

UPDATE `quest_template` SET `RequiredNpcOrGoCount1`='1' WHERE `id`=30050;

-- [Gran vehículo para la salvación] ID: 31082
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=62771;

DELETE FROM `smart_scripts` WHERE `entryorguid`=62771;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(62771,0,0,1,19,0,100,0,31082,0,0,0,33,62601,0,0,0,0,0,7,0,0,0,0,0,0,0, 'killcredit on quest accept');

UPDATE `quest_template` SET `RequiredNpcOrGoCount1`='1' WHERE `id`=31082;

-- [El consejo Klaxxi] ID: 31006
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=62202;

DELETE FROM `smart_scripts` WHERE `entryorguid`=62202;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES 
(62202,0,0,1,19,0,100,0,31006,0,0,0,33,62538,0,0,0,0,0,7,0,0,0,0,0,0,0, 'killcredit on quest accept');

-- -[Ligada con sombra] ID: 31069
UPDATE `creature_template` SET `KillCredit1`='62817' WHERE `entry`=62751;
UPDATE `creature_template` SET `KillCredit1`='62817' WHERE `entry`=65996;

