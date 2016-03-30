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

#ifndef TRINITY_WAYPOINTMANAGER_H
#define TRINITY_WAYPOINTMANAGER_H

#include <ace/Singleton.h>
#include <ace/Null_Mutex.h>

#include <vector>
#include <unordered_map>

struct WaypointData
{
    uint32 id;
    float x, y, z, orientation;
    bool run;
    uint32 delay;
    uint32 event_id;
    uint8 event_chance;
};

typedef std::vector<WaypointData*> WaypointPath;
typedef std::unordered_map<uint32, WaypointPath> WaypointPathContainer;

struct SplineWaypointData
{
    uint8 wp_id;
    float x, y, z;
};

typedef std::vector<SplineWaypointData> SplineWaypointPath;
typedef std::unordered_map<uint8, SplineWaypointPath> SplineWaypointPathContainer;
typedef std::unordered_map<uint32, SplineWaypointPathContainer> CreatureSplineWaypointPathContainer;

struct WaypointAddon
{
    bool fly;
    bool walk;
    bool catmullRom;
    float speed;
};

typedef std::unordered_map<uint32, WaypointAddon> WaypointAddonContainer;

class WaypointMgr
{
        friend class ACE_Singleton<WaypointMgr, ACE_Null_Mutex>;

    public:
        // Attempts to reload a single path from database
        void ReloadPath(uint32 id);

        // Loads all paths from database, should only run on startup
        void Load();

        // Returns the path from a given id
        WaypointPath const* GetPath(uint32 id) const
        {
            WaypointPathContainer::const_iterator itr = m_waypointStore.find(id);
            if (itr != m_waypointStore.end())
                return &itr->second;

            return NULL;
        }

        SplineWaypointPath const* GetSplinePath(uint32 c_entry, uint8 path_id) const
        {
            CreatureSplineWaypointPathContainer::const_iterator itr = m_splineWaypointStore.find(c_entry);

            if (itr != m_splineWaypointStore.end())
            {
                SplineWaypointPathContainer::const_iterator jitr = (*itr).second.find(path_id);

                if (jitr != (*itr).second.end())
                    return &jitr->second;
            }

            return NULL;
        }

        WaypointAddon const* GetWaypointAddon(uint32 path_id) const
        {
            WaypointAddonContainer::const_iterator itr = m_waypointAddonStore.find(path_id);

            if (itr != m_waypointAddonStore.end())
                return &itr->second;

            return NULL;
        }


    private:
        // Only allow instantiation from ACE_Singleton
        WaypointMgr();
        ~WaypointMgr();

        WaypointPathContainer m_waypointStore;
        CreatureSplineWaypointPathContainer m_splineWaypointStore;
        WaypointAddonContainer m_waypointAddonStore;
};

#define sWaypointMgr ACE_Singleton<WaypointMgr, ACE_Null_Mutex>::instance()

#endif

