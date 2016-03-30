/*
    Dungeon : Stormstout Brewery 85-87
    Instance General Script
*/

#ifndef STORMSTOUT_BREWERY_H_
#define STORMSTOUT_BREWERY_H_

#include "SpellScript.h"
#include "Map.h"
#include "Creature.h"
#include "CreatureAIImpl.h"

#define SBScriptName "instance_stormstout_brewery"

typedef std::unordered_map<uint32, uint64> EntryGuidMap;

enum DataTypes : uint32
{
    DATA_OOK_OOK        = 0,
    DATA_HOPTALLUS      = 1,
    DATA_YAN_ZHU        = 2,
    
    DATA_SMALL_ADDS     = 3,
    DATA_MIDDLE_ADDS    = 4,
    DATA_LARGE_ADDS     = 5,

    DATA_HOZEN_SLAIN    = 6
};

#define MAX_ENCOUNTER 7

enum CreaturesIds : uint32
{
    NPC_OOK_OOK                  = 56637,
    NPC_HOPTALLUS                = 56717,
    NPC_YAN_ZHU                  = 59479,
    NPC_UNCLE_GAO                = 59074,
    NPC_CHEN_YANZHU              = 64361,
    NPC_CHEN_STORMSTOUT          = 59704,
    NPC_AUNTIE_STORMSTOUT        = 59822,

    // Yanzhu mobs
    NPC_BLOATED_ALEMENTAL        = 59518,
    NPC_STOUT_ALEMENTAL          = 59519,
    NPC_FIZZY_ALEMENTAL          = 59520,
    NPC_BUBBLING_ALEMENTAL       = 59521,
    NPC_YEASTY_ALEMENTAL         = 59494,
    NPC_SUDSY_ALEMENTAL          = 59522,

    NPC_WALL_OF_SUDS             = 59512,
    NPC_FIZZY_BUBBLE             = 59799,

    NPC_HOZEN_PARTY_ANIMAL       = 59684,
    NPC_HOZEN_PARTY_ANIMAL2      = 57097,
    NPC_HOZEN_PARTY_ANIMAL3      = 56927,
    NPC_DRUNKEN_HOZEN_BRAWLER    = 56862,
    NPC_SLEEPY_HOZEN_BRAWLER     = 56863,
    NPC_INFLAMED_HOZEN_BRAWLER   = 56924,
    NPC_SODDEN_HOZEN_BRAWLER     = 59605,

    NPC_HOZEN_CLINGER            = 60276,
    NPC_HOZEN_BOUNCER            = 56849,

    NPC_PURPOSE_BUNNY_FLYING     = 54020,
    NPC_PURPOSE_BUNNY_GROUND     = 59394,
    
    NPC_PANDA_BREWMASTER         = 65375

};

enum GlobalSpells : uint32
{
    SPELL_BLOATING_BREW          = 114929,
    SPELL_BLACKOUT_BREW          = 114930,
    SPELL_BUBBLING_BREW          = 114931,
    SPELL_YEASTY_BREW            = 114932,
    SPELL_SUDSY_BREW             = 114933,
    SPELL_FIZZY_BREW             = 114934,

    SPELL_GUSHING_BREW           = 114379,
    SPELL_MOONBEAM_VISUAL        = 128281,
    
    SPELL_NIBBLER_COSMETIC       = 128188,
    
};

enum GoIds : uint32
{
    GO_BREWERY_DOOR          = 209938,
    GO_INVIS_DOOR            = 207997,
    GO_SLIDING_DOOR          = 211981,
    GO_OOK_DOOR              = 211127
};

#endif // STORMSTOUT_BREWERY_H_

// SPELL_WASTED 114730
// SPELL_BLAZING_SPARK 107071
// ANCESTRAL_BREWMASTER_COSMETIC 113124
// SPELL_FERMENT 106859

// GO_TAP_TOOL = 211326
// GO_BREW_TAP = 211314