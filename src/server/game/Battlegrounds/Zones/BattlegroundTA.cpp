#include "BattlegroundTA.h"
#include "Language.h"
#include "Object.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldPacket.h"

BattlegroundTA::BattlegroundTA()
{
    BgObjects.resize(BG_TA_OBJECT_MAX);

    StartDelayTimes[BG_STARTING_EVENT_FIRST]  = BG_START_DELAY_1M;
    StartDelayTimes[BG_STARTING_EVENT_SECOND] = BG_START_DELAY_30S;
    StartDelayTimes[BG_STARTING_EVENT_THIRD]  = BG_START_DELAY_15S;
    StartDelayTimes[BG_STARTING_EVENT_FOURTH] = BG_START_DELAY_NONE;
    //we must set messageIds
    StartMessageIds[BG_STARTING_EVENT_FIRST]  = LANG_ARENA_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_ARENA_THIRTY_SECONDS;
    StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_ARENA_FIFTEEN_SECONDS;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_ARENA_HAS_BEGUN;
}

BattlegroundTA::~BattlegroundTA() { }

void BattlegroundTA::StartingEventCloseDoors()
{
    for (uint32 i = BG_TA_OBJECT_DOOR_1; i <= BG_TA_OBJECT_DOOR_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
}

void BattlegroundTA::StartingEventOpenDoors()
{
    for (uint32 i = BG_TA_OBJECT_DOOR_1; i <= BG_TA_OBJECT_DOOR_2; ++i)
        DoorOpen(i);
// 
//     for (uint32 i = BG_TA_OBJECT_BUFF_1; i <= BG_TA_OBJECT_BUFF_2; ++i)
//         SpawnBGObject(i, 60);
}

void BattlegroundTA::StartingEventDespawnDoors()
{
    for (uint32 i = BG_TA_OBJECT_DOOR_1; i <= BG_TA_OBJECT_DOOR_2; ++i)
        DoorDespawn(i);
}

void BattlegroundTA::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    //create score and add it to map, default values are set in constructor
    BattlegroundTAScore* sc = new BattlegroundTAScore;

    PlayerScores[player->GetGUID()] = sc;

    UpdateArenaWorldState();
}

void BattlegroundTA::RemovePlayer(Player* /*player*/, uint64 /*guid*/, uint32 /*team*/)
{
    if (GetStatus() == STATUS_WAIT_LEAVE)
        return;

    UpdateArenaWorldState();
    CheckArenaWinConditions();
}

bool BattlegroundTA::HandlePlayerUnderMap(Player* player)
{
    if (GetStatus() == STATUS_WAIT_JOIN)
        return true;

    player->TeleportTo(GetMapId(), -10721.909f, 428.273f, 24.5f, 2.422f);
    return true;
}

void BattlegroundTA::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (!killer)
    {
        TC_LOG_ERROR("bg.battleground", "BattlegroundTA: Killer player not found");
        return;
    }

    Battleground::HandleKillPlayer(player, killer);

    UpdateArenaWorldState();
    CheckArenaWinConditions();
}

void BattlegroundTA::HandleAreaTrigger(Player* player, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch (trigger)
    {
        case 8005: //buff next to scarab
        case 8006: //buff next to scarab
        case 8007: //center of map
        case 8009: //center of map
        case 8010: //center of map
        case 8451: //OR spawn
        case 8452: //GREEN spawn
            break;
        default:
            Battleground::HandleAreaTrigger(player, trigger);
            break;
    }
}

void BattlegroundTA::FillInitialWorldStates(WorldPacket &data)
{
    UpdateArenaWorldState();
}

void BattlegroundTA::Reset()
{
    //call parent's class reset
    Battleground::Reset();
}

bool BattlegroundTA::SetupBattleground()
{
    // gates
    if (!AddObject(BG_TA_OBJECT_DOOR_1, BG_TA_OBJECT_TYPE_DOOR_1, -10654.3f, 428.3047f, 23.54276f, -M_PI, 0.f, 0.f, -1.f, 0.f, RESPAWN_IMMEDIATELY)
     || !AddObject(BG_TA_OBJECT_DOOR_2, BG_TA_OBJECT_TYPE_DOOR_2, -10774.61f, 431.2383f, 23.54276f, 0.f, 0.f, 0.f, 0.f, 1.f, RESPAWN_IMMEDIATELY))
    {
         TC_LOG_ERROR("sql.sql", "BatteGroundTA: Failed to spawn some object!");
         return false;
    }
//     // buffs
//         || !AddObject(BG_TA_OBJECT_BUFF_1, BG_TA_OBJECT_TYPE_BUFF_1, 4009.189941f, 2895.250000f, 13.052700f, -1.448624f, 0, 0, 0.6626201f, -0.7489557f, 120)
//         || !AddObject(BG_TA_OBJECT_BUFF_2, BG_TA_OBJECT_TYPE_BUFF_2, 4103.330078f, 2946.350098f, 13.051300f, -0.06981307f, 0, 0, 0.03489945f, -0.9993908f, 120))
//     {
//         TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundTA: Failed to spawn some object!");
//         return false;
//     }

    return true;
}
