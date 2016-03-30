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

#ifndef DEF_DEADMINES_H
#define DEF_DEADMINES_H

enum Data
{
    DATA_GLUBTOK                            = 0,
    DATA_HELIX_GEARBREAKER                  = 1,
    DATA_FOE_REAPER_5000                    = 2,
    DATA_ADMIRA_RIPSNARL                    = 3,
    DATA_CAPTAIN_COOKIE                     = 4,
    DATA_VANESSA_VANCLEEF                   = 5,
    MAX_BOSS_ENCOUNTER                      = 6,

    DATA_VANESSA_EVENT_GLUBTOK              = 0,
    DATA_VANESSA_EVENT_HELIX_GEARBREAKER    = 1,
    DATA_VANESSA_EVENT_FOE_REAPER_5000      = 2,
    DATA_VANESSA_EVENT_ADMIRA_RIPSNARL      = 3,
    MAX_ENCOUNTER                           = 4,

    DATA_VALVE_ACTIVATED                    = 5,
    DATA_EMME_SAVED                         = 6,
    DATA_ERIK_SAVED                         = 7,
    DATA_NOTE_USED                          = 8,
    DATA_ENGINEER_RUN                       = 9,
};

enum VanessaEventData
{

};

enum Creatures
{
    BOSS_GLUBTOK                = 47162,
    BOSS_HELIX                  = 47296,
    BOSS_FOE_REAPER_5000        = 43778,
    BOSS_CAPTAIN_COOKIE         = 47739,
};

enum GameObjects
{
};

#endif
