/*
* Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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


#ifndef SHADOPAN_MONASTERY_H_
#define SHADOPAN_MONASTERY_H_

#include "SpellScript.h"
#include "Map.h"
#include "Creature.h"
#include "CreatureAIImpl.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"

uint32 const EncounterCount = 5;

#define MAX_NOVICE              24

enum DataTypes
{
    DATA_GU_CLOUDSTRIKE         = 0,
    DATA_MASTER_SNOWDRIFT       = 1,
    DATA_SHA_VIOLENCE           = 2,
    DATA_TARAN_ZHU              = 3,
    DATA_SNOWDRIFT_STATE        = 4,
    DATA_RANDOM_FIRST_POS       = 5,
    DATA_RANDOM_SECOND_POS      = 6,
    DATA_RANDOM_MINIBOSS_POS    = 7,
    DATA_DEFEATED_NOVICE        = 8,
    DATA_DEFEATED_MINIBOSS      = 9,
    DATA_ARCHERY                = 10,
    DATA_FIRE_ARCHERY           = 11,
    DATA_AZURE_SERPENT          = 12,
    DATA_ARCHERY_TARGET         = 13,
    DATA_PANDAREN_REFEREE       = 14,
    DATA_TARAN_ZHU_CACHE        = 15,
    DATA_POSSESSIONS            = 16,
    MAX_DATA
};

enum CreaturesIds
{
    NPC_GU_CLOUDSTRIKE          = 56747,
    NPC_MASTER_SNOWDRIFT        = 56541,
    NPC_SHA_VIOLENCE            = 56719,
    NPC_TARAN_ZHU               = 56884,
    // Gu Cloudstrike
    NPC_AZURE_SERPENT           = 56754,
    NPC_GUARDIAN                = 59741,
    // Master Snowdrift
    NPC_NOVICE                  = 56395,
    NPC_SNOWDRIFT_POSITION      = 56397,
    NPC_FLAGRANT_LOTUS          = 56472,
    NPC_FLYING_SNOW             = 56473,
    NPC_SNOWDRIFT_CLONE         = 56713,
    NPC_PANDAREEN_REFEREE       = 56505,
    NPC_LOTUS_STAFF             = 56678,
    // Sha of Violence
    NPC_LESSER_VOLATILE_ENERGY  = 66652,
    // Trashs
    NPC_ARCHERY_FIRST           = 64549,
    NPC_ARCHERY_SECOND          = 56767,
    NPC_ARCHERY_TARGET          = 64550,
    NPC_RESIDUAL_OF_HATRED      = 58803,
    NPC_VESTIGE_OF_HATRED       = 58807,
    NPC_FRAGMENT_OF_HATRED      = 58810,
    NPC_SHADO_PAN_AMBUSHER      = 59752,
    NPC_SHADO_PAN_WARDEN        = 59751,
    NPC_UNSTABLE_ENERGY         = 59811,
    NPC_TRAINING_TARGET         = 60162
};

enum ObjectsIds
{
    GO_CLOUDSTRIKE_ENTRANCE     = 210863,
    GO_CLOUDSTRIKE_EXIT         = 210864,
    GO_SNOWDRIFT_ENTRANCE       = 213194,
    GO_SNOWDRIFT_FIRE_WALL      = 212908,
    GO_SNOWDRIFT_DOJO_DOOR      = 210800,
    GO_SNOWDRIFT_EXIT           = 210862,
    GO_SNOWDRIFT_POSSESSIONS    = 214519,
    GO_SHA_ENTRANCE             = 210867,
    GO_SHA_EXIT                 = 210866,
    GO_TARAN_ZHU_CACHE          = 213888,
    GO_SNOWDRIFT_POSSISIONS_H   = 530301,
    GO_TARAN_ZHU_CACHE_H        = 530300
};

enum SharedSpells
{
    SPELL_HATE                  = 107085,
    SPELL_HAZE_OF_HATE          = 107087,
    SPELL_HAZE_OF_HATE_VISUAL   = 107217,
};

Position const IceArchersPosition[3] =
{
    {3670.379f, 2974.310f, 874.172f, 4.673f},
    {3657.360f, 2996.389f, 863.440f, 4.952f},
    {3673.500f, 2998.620f, 863.341f, 4.877f}
};

Position const FireArchersPosition[3] =
{
    {3978.959f, 2977.770f, 832.538f, 1.84f},
    {3984.889f, 2979.379f, 832.414f, 1.84f},
    {3971.669f, 2976.040f, 832.411f, 1.84f}
};

#endif // SHADOPAN_MONASTERY_H_
