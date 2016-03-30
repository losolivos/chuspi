/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Transport.h"
#include "GridDefines.h"
#include "MapInstanced.h"
#include "InstanceScript.h"
#include "Config.h"
#include "World.h"
#include "CellImpl.h"
#include "Corpse.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "WorldPacket.h"
#include "Group.h"
#include "ThreadPoolMgr.hpp"
#include "Profiler/ProbePoint.hpp"

MapManager::MapManager()
{
    i_gridCleanUpDelay = sWorld->getIntConfig(CONFIG_INTERVAL_GRIDCLEAN);
    i_timer.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_MAPUPDATE));
}

MapManager::~MapManager()
{
}

void MapManager::Initialize()
{

}

void MapManager::InitializeVisibilityDistanceInfo()
{
    for (MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        (*iter).second->InitVisibilityDistance();
}

Map* MapManager::CreateBaseMap(uint32 id)
{
    Map* map = FindBaseMap(id);

    if (map == NULL)
    {
        GuardType guard(i_lock);

        MapEntry const* entry = sMapStore.LookupEntry(id);
        ASSERT(entry);

        if (entry->Instanceable())
            map = new MapInstanced(id, i_gridCleanUpDelay);
        else
        {
            map = new Map(id, i_gridCleanUpDelay, 0, NONE_DIFFICULTY);
            map->LoadRespawnTimes();
        }

        i_maps[id] = map;
    }

    ASSERT(map);
    return map;
}

Map* MapManager::FindBaseNonInstanceMap(uint32 mapId) const
{
    Map* map = FindBaseMap(mapId);
    if (map && map->Instanceable())
        return NULL;
    return map;
}

Map* MapManager::CreateMap(uint32 id, Player* player)
{
    Map* m = CreateBaseMap(id);

    if (m && m->Instanceable())
        m = ((MapInstanced*)m)->CreateInstanceForPlayer(id, player);

    return m;
}

Map* MapManager::FindMap(uint32 mapid, uint32 instanceId) const
{
    Map* map = FindBaseMap(mapid);
    if (!map)
        return NULL;

    if (!map->Instanceable())
        return instanceId == 0 ? map : NULL;

    return ((MapInstanced*)map)->FindInstanceMap(instanceId);
}

bool MapManager::CanPlayerEnter(uint32 mapid, Player* player, bool loginCheck)
{
    MapEntry const* entry = sMapStore.LookupEntry(mapid);
    if (!entry)
       return false;

    // Molten Front
    // That's not an instance, so check it here
    if (entry->MapID == 861)
        if (player->getLevel() < 85)
            return false;

    if (!entry->IsDungeon() || (entry->IsDungeon() && entry->Expansion() < 4))
        return true;

    InstanceTemplate const* instance = sObjectMgr->GetInstanceTemplate(mapid);
    if (!instance)
        return false;

    Difficulty targetDifficulty = player->GetDifficulty(entry->IsRaid());
    //The player has a heroic mode and tries to enter into instance which has no a heroic mode
    MapDifficulty const* mapDiff = GetMapDifficultyData(entry->MapID, targetDifficulty);
    if (!mapDiff)
    {
        // Send aborted message for dungeons
        if (entry->IsNonRaidDungeon())
        {
            player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY, player->GetDungeonDifficulty());
            return false;
        }
        else    // attempt to downscale
            mapDiff = GetDownscaledMapDifficultyData(entry->MapID, targetDifficulty);
    }

    //Bypass checks for GMs
    if (player->isGameMaster())
        return true;

    char const* mapName = entry->name;

    if (!player->IsAlive())
    {
        if (Corpse* corpse = player->GetCorpse())
        {
            // let enter in ghost mode in instance that connected to inner instance with corpse
            uint32 corpseMap = corpse->GetMapId();
            do
            {
                if (corpseMap == mapid)
                    break;

                InstanceTemplate const* corpseInstance = sObjectMgr->GetInstanceTemplate(corpseMap);
                corpseMap = corpseInstance ? corpseInstance->Parent : 0;
            }
            while (corpseMap);

            if (!corpseMap)
            {
                WorldPacket data(SMSG_CORPSE_NOT_IN_INSTANCE);
                player->GetSession()->SendPacket(&data);
                TC_LOG_DEBUG("maps", "MAP: Player '%s' does not have a corpse in instance '%s' and cannot enter.", player->GetName().c_str(), mapName);
                return false;
            }
            TC_LOG_DEBUG("maps", "MAP: Player '%s' has corpse in instance '%s' and can enter.", player->GetName().c_str(), mapName);
            player->ResurrectPlayer(0.5f, false);
            player->SpawnCorpseBones();
        }
        else
            TC_LOG_DEBUG("maps", "Map::CanPlayerEnter - player '%s' is dead but does not have a corpse!", player->GetName().c_str());
    }

    Group* group = player->GetGroup();
    //Get instance where player's group is bound & its map
    if (group)
    {
        InstanceGroupBind* boundInstance = group->GetBoundInstance(entry);
        if (boundInstance && boundInstance->save)
            if (Map* boundMap = sMapMgr->FindMap(mapid, boundInstance->save->GetInstanceId()))
                if (!loginCheck && !boundMap->CanEnter(player))
                    return false;
            /*
                This check has to be moved to InstanceMap::CanEnter()
                // Player permanently bounded to different instance than groups one
                InstancePlayerBind* playerBoundedInstance = player->GetBoundInstance(mapid, player->GetDifficulty(entry->IsRaid()));
                if (playerBoundedInstance && playerBoundedInstance->perm && playerBoundedInstance->save &&
                    boundedInstance->save->GetInstanceId() != playerBoundedInstance->save->GetInstanceId())
                {
                    //TODO: send some kind of error message to the player
                    return false;
                }*/
    }

    // players are only allowed to enter 5 instances per hour
    if (entry->IsDungeon() && (!player->GetGroup() || (player->GetGroup() && !player->GetGroup()->isLFGGroup())))
    {
        uint32 instaceIdToCheck = 0;
        if (InstanceSave* save = player->GetInstanceSave(mapid, entry->IsRaid()))
            instaceIdToCheck = save->GetInstanceId();

        // instanceId can never be 0 - will not be found
        if (!player->CheckInstanceCount(instaceIdToCheck) && !player->isDead())
        {
            player->SendTransferAborted(mapid, TRANSFER_ABORT_TOO_MANY_INSTANCES);
            return false;
        }
    }

    //Other requirements
    return player->Satisfy(sObjectMgr->GetAccessRequirement(mapid, targetDifficulty), mapid, true);
}

void MapManager::Update(uint32 diff)
{
    i_timer.Update(diff);
    if (!i_timer.Passed())
        return;

    uint32 curr = uint32(i_timer.GetCurrent());
    i_timer.SetCurrent(0);

    for (MapMapType::iterator i = i_maps.begin(); i != i_maps.end(); ++i) {
        Map * const map = i->second;
        sThreadPoolMgr->schedule([map, curr] { map->Update(curr); });
    }
    sThreadPoolMgr->wait();

    for (MapMapType::iterator i = i_maps.begin(); i != i_maps.end(); ++i) {
        Map * const map = i->second;
        sThreadPoolMgr->schedule([map, curr] { map->DelayedUpdate(curr); });
    }
    sThreadPoolMgr->wait();

    sObjectAccessor->Update(curr);

    for (auto itr = m_Transports.begin(); itr != m_Transports.end(); ++itr)
        (*itr)->Update(curr);

}

void MapManager::DoDelayedMovesAndRemoves()
{
}

bool MapManager::ExistMapAndVMap(uint32 mapid, float x, float y)
{
    GridCoord p = Trinity::ComputeGridCoord(x, y);

    int gx=63-p.x_coord;
    int gy=63-p.y_coord;

    return Map::ExistMap(mapid, gx, gy) && Map::ExistVMap(mapid, gx, gy);
}

bool MapManager::IsValidMAP(uint32 mapid, bool startUp)
{
    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);

    if (startUp)
        return mEntry ? true : false;
    else
        return mEntry && (!mEntry->IsDungeon() || sObjectMgr->GetInstanceTemplate(mapid));

    // TODO: add check for battleground template
}

void MapManager::UnloadAll()
{
    for (TransportSet::iterator i = m_Transports.begin(); i != m_Transports.end(); ++i)
    {
        (*i)->RemoveFromWorld();
        delete *i;
    }

    for (MapMapType::iterator iter = i_maps.begin(); iter != i_maps.end();)
    {
        iter->second->UnloadAll();
        delete iter->second;
        i_maps.erase(iter++);
    }
}

uint32 MapManager::GetNumInstances()
{
    GuardType guard(i_lock);

    uint32 ret = 0;
    for (MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map* map = itr->second;
        if (!map->Instanceable())
            continue;
        MapInstanced::InstancedMaps &maps = ((MapInstanced*)map)->GetInstancedMaps();
        for (MapInstanced::InstancedMaps::iterator mitr = maps.begin(); mitr != maps.end(); ++mitr)
            if (mitr->second->IsDungeon()) ret++;
    }
    return ret;
}

uint32 MapManager::GetNumPlayersInInstances()
{
    GuardType guard(i_lock);

    uint32 ret = 0;
    for (MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map* map = itr->second;
        if (!map->Instanceable())
            continue;
        MapInstanced::InstancedMaps &maps = ((MapInstanced*)map)->GetInstancedMaps();
        for (MapInstanced::InstancedMaps::iterator mitr = maps.begin(); mitr != maps.end(); ++mitr)
            if (mitr->second->IsDungeon())
                ret += ((InstanceMap*)mitr->second)->GetPlayers().getSize();
    }
    return ret;
}

void MapManager::InitInstanceIds()
{
    _nextInstanceId = 1;

    QueryResult result = CharacterDatabase.Query("SELECT MAX(id) FROM instance");
    if (result)
    {
        uint32 maxId = (*result)[0].GetUInt32();

        // Resize to multiples of 32 (vector<bool> allocates memory the same way)
        _instanceIds.resize((maxId / 32) * 32 + (maxId % 32 > 0 ? 32 : 0));
    }
}

void MapManager::RegisterInstanceId(uint32 instanceId)
{
    // Allocation and sizing was done in InitInstanceIds()
    _instanceIds[instanceId] = true;
}

uint32 MapManager::GenerateInstanceId()
{
    uint32 newInstanceId = _nextInstanceId;

    // Find the lowest available id starting from the current NextInstanceId (which should be the lowest according to the logic in FreeInstanceId()
    for (uint32 i = ++_nextInstanceId; i < 0xFFFFFFFF; ++i)
    {
        if ((i < _instanceIds.size() && !_instanceIds[i]) || i >= _instanceIds.size())
        {
            _nextInstanceId = i;
            break;
        }
    }

    if (newInstanceId == _nextInstanceId)
    {
        TC_LOG_ERROR("maps", "Instance ID overflow!! Can't continue, shutting down server. ");
        sWorld->StopNow(ERROR_EXIT_CODE);
    }

    // Allocate space if necessary
    if (newInstanceId >= uint32(_instanceIds.size()))
    {
        // Due to the odd memory allocation behavior of vector<bool> we match size to capacity before triggering a new allocation
        if (_instanceIds.size() < _instanceIds.capacity())
        {
            _instanceIds.resize(_instanceIds.capacity());
        }
        else
            _instanceIds.resize((newInstanceId / 32) * 32 + (newInstanceId % 32 > 0 ? 32 : 0));
    }

    _instanceIds[newInstanceId] = true;

    return newInstanceId;
}

void MapManager::FreeInstanceId(uint32 instanceId)
{
    // If freed instance id is lower than the next id available for new instances, use the freed one instead
    if (instanceId < _nextInstanceId)
        SetNextInstanceId(instanceId);

    _instanceIds[instanceId] = false;
}

uint32 MapManager::GetLoadedGrids()
{
    uint32 count = 0;
    for (auto itr: i_maps)
        count += itr.second->GetGridCount();

    return count;
}

Transport* MapManager::LoadTransportInMap(Map* instance, uint32 goEntry, uint32 period)
{
    const GameObjectTemplate* goInfo = sObjectMgr->GetGameObjectTemplate(goEntry);

    if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        return NULL;

    Transport* Ship = new Transport(period, goInfo->ScriptId);
    std::set<uint32> mapsUsed;
    if (!Ship->GenerateWaypoints(goInfo->moTransport.taxiPathId, mapsUsed))
    {
        delete Ship;
        return NULL;
    }
    uint32 transportLowGuid = sObjectMgr->GenerateLowGuid(HIGHGUID_MO_TRANSPORT);

    if (!Ship->Create(transportLowGuid, goEntry, Ship->m_WayPoints[0].mapid, Ship->m_WayPoints[0].x, Ship->m_WayPoints[0].y, Ship->m_WayPoints[0].z-10, 0.0f, 0, 0))
    {
        delete Ship;
        return NULL;
    }

    m_Transports.insert(Ship);
    m_TransportsByInstanceIdMap[instance->GetInstanceId()].insert(Ship);
    Ship->SetMap(instance);
    Ship->AddToWorld();

    return Ship;
}

void MapManager::UnLoadTransportFromMap(Transport* t)
{
    Map* map = t->GetMap();

    for (Transport::CreatureSet::iterator itr = t->m_NPCPassengerSet.begin(); itr != t->m_NPCPassengerSet.end();)
    {
        if (Creature* npc = *itr)
        {
            npc->SetTransport(NULL);
            npc->setActive(false);
            npc->RemoveFromWorld();
        }
        ++itr;
    }

    UpdateData transData (t->GetMapId());
    t->BuildOutOfRangeUpdateBlock(&transData);
    WorldPacket out_packet;

    if (transData.BuildPacket(&out_packet))
        for (Map::PlayerList::const_iterator itr = map->GetPlayers().begin(); itr != map->GetPlayers().end(); ++itr)
            if (t != itr->GetSource()->GetTransport())
                itr->GetSource()->SendDirectMessage(&out_packet);

    t->m_NPCPassengerSet.clear();
    m_TransportsByInstanceIdMap[t->GetInstanceId()].erase(t);
    m_Transports.erase(t);
    t->m_WayPoints.clear();
    t->RemoveFromWorld();

}

void MapManager::LoadTransportForPlayers(Player* player)
{
    MapManager::TransportMap& tmap = sMapMgr->m_TransportsByInstanceIdMap;

    UpdateData transData (player->GetMapId());

    MapManager::TransportSet& tset = tmap[player->GetInstanceId()];

    for (MapManager::TransportSet::const_iterator i = tset.begin(); i != tset.end(); ++i)
    {
        (*i)->BuildCreateUpdateBlockForPlayer(&transData, player);
    }

    WorldPacket packet;
    if (transData.BuildPacket(&packet))
        player->SendDirectMessage(&packet);
}

void MapManager::UnLoadTransportForPlayers(Player* player)
{
    MapManager::TransportMap& tmap = sMapMgr->m_TransportsByInstanceIdMap;

    UpdateData transData(player->GetMapId());

    MapManager::TransportSet& tset = tmap[player->GetInstanceId()];

    for (MapManager::TransportSet::const_iterator i = tset.begin(); i != tset.end(); ++i)
    {
        for (Transport::CreatureSet::iterator itr = (*i)->m_NPCPassengerSet.begin(); itr != (*i)->m_NPCPassengerSet.end();)
        {
            if (Creature* npc = *itr)
            {
                npc->SetTransport(NULL);
                npc->setActive(false);
                npc->RemoveFromWorld();
            }
            ++itr;
        }

        (*i)->BuildOutOfRangeUpdateBlock(&transData);
    }

    WorldPacket packet;
    if (transData.BuildPacket(&packet))
        player->SendDirectMessage(&packet);
}

void MapManager::LoadTransports()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT guid, entry, name, period, ScriptName FROM transports");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 transports. DB table `transports` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {

        Field* fields = result->Fetch();
        uint32 lowguid = fields[0].GetUInt32();
        uint32 entry = fields[1].GetUInt32();
        std::string name = fields[2].GetString();
        uint32 period = fields[3].GetUInt32();
        uint32 scriptId = sObjectMgr->GetScriptId(fields[4].GetCString());

        GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);

        if (!goinfo)
        {
            TC_LOG_ERROR("sql.sql", "Transport ID:%u, Name: %s, will not be loaded, gameobject_template missing", entry, name.c_str());
            continue;
        }

        if (goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            TC_LOG_ERROR("sql.sql", "Transport ID:%u, Name: %s, will not be loaded, gameobject_template type wrong", entry, name.c_str());
            continue;
        }

        // TC_LOG_INFO("server.loading", "Loading transport %d between %s, %s", entry, name.c_str(), goinfo->name);

        std::set<uint32> mapsUsed;

        Transport* t = new Transport(period, scriptId);
        if (!t->GenerateWaypoints(goinfo->moTransport.taxiPathId, mapsUsed))
            // skip transports with empty waypoints list
        {
            TC_LOG_ERROR("sql.sql", "Transport (path id %u) path size = 0. Transport ignored, check DBC files or transport GO data0 field.", goinfo->moTransport.taxiPathId);
            delete t;
            continue;
        }

        float x = t->m_WayPoints[0].x;
        float y = t->m_WayPoints[0].y;
        float z = t->m_WayPoints[0].z;
        uint32 mapid = t->m_WayPoints[0].mapid;
        float o = 1.0f;

         // creates the Gameobject -- Gunship
        if (!t->Create(lowguid, entry, mapid, x, y, z, o, 100, 0))
        {
            delete t;
            continue;
        }

        m_Transports.insert(t);

        for (std::set<uint32>::const_iterator i = mapsUsed.begin(); i != mapsUsed.end(); ++i)
            m_TransportsByMap[*i].insert(t);

        //If we someday decide to use the grid to track transports, here:
        t->SetMap(sMapMgr->CreateBaseMap(mapid));
        t->AddToWorld();

        ++count;
    }
    while (result->NextRow());

    // check transport data DB integrity
    result = WorldDatabase.Query("SELECT gameobject.guid, gameobject.id, transports.name FROM gameobject, transports WHERE gameobject.id = transports.entry");
    if (result)                                              // wrong data found
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 guid  = fields[0].GetUInt32();
            uint32 entry = fields[1].GetUInt32();
            std::string name = fields[2].GetString();
            TC_LOG_ERROR("sql.sql", "Transport %u '%s' have record (GUID: %u) in `gameobject`. Transports must not have any records in `gameobject` or its behavior will be unpredictable/bugged.", entry, name.c_str(), guid);
        }
        while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u transports in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void MapManager::LoadTransportNPCs()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0       1            2                3             4             5             6        7
    QueryResult result = WorldDatabase.Query("SELECT guid, npc_entry, transport_entry, TransOffsetX, TransOffsetY, TransOffsetZ, TransOffsetO, emote FROM creature_transport");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 transport NPCs. DB table `creature_transport` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[1].GetInt32();
        uint32 transportEntry = fields[2].GetInt32();
        float tX = fields[3].GetFloat();
        float tY = fields[4].GetFloat();
        float tZ = fields[5].GetFloat();
        float tO = fields[6].GetFloat();
        uint32 anim = fields[7].GetInt32();

        for (MapManager::TransportSet::iterator itr = m_Transports.begin(); itr != m_Transports.end(); ++itr)
        {
            if ((*itr)->GetEntry() == transportEntry)
            {
                (*itr)->AddNPCPassenger(entry, tX, tY, tZ, tO, anim);
                break;
            }
        }

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u transport npcs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
