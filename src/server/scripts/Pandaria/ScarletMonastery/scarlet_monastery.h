/*
*    Dungeon : Scarlet monastery 31-90
*    Instance General Script
*/

#ifndef SCARLET_MONASTERY_H_
#define SCARLET_MONASTERY_H_
#define DataHeader "SM"
#define scarlet_monastery_scriptname "instance_scarlet_monastery"
uint32 const encounternumber = 4;

#include "SpellScript.h"
#include "Map.h"
#include "Creature.h"
#include "CreatureAIImpl.h"

enum eData
{
    BOSS_THALNOS_THE_SOULRENDER = 0,
    BOSS_BROTHER_KORLOFF = 1,
    BOSS_WHITEMANE = 2,
    BOSS_DURAND = 3,
    NPC_FALLEN_CRUSADER = 4
};

enum eObjects
{
    GO_KORLOFF_EXIT = 210564,
    GO_WHITEMANE_ENTRANCE = 210563
};

enum eCreature
{
    BROTHER_KORLOFF = 59223,
    COMMANDER_DURAND = 60040,
    HIGH_INQUISITOR_WHITEMANE = 3977,
    THALNOS_THE_SOULRENDER = 59789,
    SCARLET_FANATIC = 58555, // trash
    SCARLET_CENTURION = 59746,
    SCARLET_FLAMETHOWER = 59705,
    SCARLET_JUDICATOR = 58605,
    SCARLET_PURIFIER = 58569,
    FALLEN_CRUSADER = 59884, // Thalnos
    EVICTED_SOUL = 59974,
    EMPOWERING_SPIRIT = 59893,
    EMPOWERED_ZOMBIE = 59930,
    EMPOWERED_ZOMBIE_HEROIC = 599300
};

#define MAX_TYPES 14
#endif // SCARLET_MONASTERY_H_