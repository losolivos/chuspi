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

#include "DatabaseEnv.h"
#include "GridDefines.h"
#include "WaypointManager.h"
#include "MapManager.h"
#include "Log.h"

WaypointMgr::WaypointMgr()
{
}

WaypointMgr::~WaypointMgr()
{
    for (WaypointPathContainer::iterator itr = m_waypointStore.begin(); itr != m_waypointStore.end(); ++itr)
    {
        for (WaypointPath::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;

        itr->second.clear();
    }

    m_waypointStore.clear();
}

void WaypointMgr::Load()
{
    uint32 oldMSTime = getMSTime();

    //                                                0    1         2           3          4            5           6        7      8           9
    QueryResult result = WorldDatabase.Query("SELECT id, point, position_x, position_y, position_z, orientation, move_flag, delay, action, action_chance FROM waypoint_data ORDER BY id, point");

    if (!result)
    {
        TC_LOG_ERROR("server.loading", ">> Loaded 0 waypoints. DB table `waypoint_data` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        uint32 pathId = fields[0].GetUInt32();
        WaypointPath& path = m_waypointStore[pathId];

        float x = fields[2].GetFloat();
        float y = fields[3].GetFloat();
        float z = fields[4].GetFloat();
        float o = fields[5].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        wp->id = fields[1].GetUInt32();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->run = fields[6].GetBool();
        wp->delay = fields[7].GetUInt32();
        wp->event_id = fields[8].GetUInt32();
        wp->event_chance = fields[9].GetInt16();

        path.push_back(wp);
        ++count;
    }
    while (result->NextRow());

    result = WorldDatabase.Query("SELECT c_entry, path_id, wp_id, position_x, position_y, position_z FROM waypoint_spline_data ORDER BY c_entry, path_id, wp_id");

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 c_entry = fields[0].GetUInt32();
            uint8 path_id = fields[1].GetUInt8();

            SplineWaypointPathContainer& c_paths = m_splineWaypointStore[c_entry];
            SplineWaypointPath& path = c_paths[path_id];

            SplineWaypointData wp;

            wp.wp_id = fields[2].GetUInt8();
            wp.x = fields[3].GetFloat();
            wp.y = fields[4].GetFloat();
            wp.z = fields[5].GetFloat();

            Trinity::NormalizeMapCoord(wp.x);
            Trinity::NormalizeMapCoord(wp.y);

            path.push_back(wp);
        }
        while (result->NextRow());
    }
    else
        TC_LOG_ERROR("server.loading", ">> Loaded 0 spline waypoints. DB table `waypoint_spline_data` is empty!");

    result = WorldDatabase.Query("SELECT path_id, fly, walk, catmullRom, speed FROM waypoint_data_addon ORDER BY path_id");

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 path_id = fields[0].GetUInt32();
            WaypointAddon& addon = m_waypointAddonStore[path_id];

            addon.fly = fields[1].GetBool();
            addon.walk = fields[2].GetBool();
            addon.catmullRom = fields[3].GetBool();
            addon.speed = fields[4].GetFloat();
        }
        while (result->NextRow());
    }
    else
        TC_LOG_ERROR("server.loading", ">> Loaded 0 waypoint's addons. DB table `waypoint_data_addon` is empty!");

    TC_LOG_INFO("server.loading", ">> Loaded %u waypoints in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void WaypointMgr::ReloadPath(uint32 id)
{
    WaypointPathContainer::iterator itr = m_waypointStore.find(id);
    if (itr != m_waypointStore.end())
    {
        for (WaypointPath::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;

        m_waypointStore.erase(itr);
    }

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_DATA_BY_ID);

    stmt->setUInt32(0, id);

    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (!result)
        return;

    WaypointPath& path = m_waypointStore[id];

    do
    {
        Field* fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        float x = fields[1].GetFloat();
        float y = fields[2].GetFloat();
        float z = fields[3].GetFloat();
        float o = fields[4].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        wp->id = fields[0].GetUInt32();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->run = fields[5].GetBool();
        wp->delay = fields[6].GetUInt32();
        wp->event_id = fields[7].GetUInt32();
        wp->event_chance = fields[8].GetUInt8();

        path.push_back(wp);

    }
    while (result->NextRow());
}
