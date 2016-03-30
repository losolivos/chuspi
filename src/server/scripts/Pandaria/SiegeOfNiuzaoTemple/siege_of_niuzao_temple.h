/*
 * Copyright (C) 2008-2014 MoltenCore <http://www.molten-wow.com/>
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

#ifndef SIEGE_OF_NIUZAO_H
#define SIEGE_OF_NIUZAO_H

enum Bosses
{
    BOSS_JINBAK,
    BOSS_VOJAK,
    BOSS_PAVALAK,
    BOSS_NERONOK,
    TOTAL_ENCOUNTERS,

    DATA_VOJAK_DOOR
};

enum Creatures
{
    /* BOSSES */
    NPC_JINBAK          = 61567,
    NPC_VOJAK           = 61634,
    NPC_PAVALAK         = 61485,
    NPC_NERONOK         = 62205,
    NPC_MANTID_CATAPULT = 63565
};

enum GameObjects
{
    GO_HARDENED_RESIN       = 213174, // after jinbak
    GO_DOOR                 = 212921, // after vojak

    GO_TEMPLE_INVIS_DOOR    = 213251, // before neronok bridge
    GO_WIND_WALL            = 214548, // before neronok bridge
    GO_FIRE_WALL            = 210097,
};

enum WMOAreaEntries
{
    WMO_REAR_STAGING_AREA       = 59479
};

#endif