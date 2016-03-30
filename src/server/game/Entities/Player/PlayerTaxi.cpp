#include "PlayerTaxi.hpp"
#include "ByteBuffer.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Util.h"
#include "LexicalCast.h"

namespace Trinity {

PlayerTaxi::LevelupTaxiNodes PlayerTaxi::s_levelupNodes[2][DEFAULT_MAX_LEVEL];

PlayerTaxi::PlayerTaxi()
    : m_mask()
{ }

void PlayerTaxi::CacheLevelupNodes()
{
    std::map<uint8, std::map<uint8, uint32>> nodeMap[2];

    // Add Taxi Nodes availables from player level
    for (uint32 i = 0; i < sTaxiNodesStore.GetNumRows(); i++)
    {
        auto const node = sTaxiNodesStore.LookupEntry(i);
        if (!node || (node->MountCreatureID[0] == 0 && node->MountCreatureID[1] == 0))
            continue;

        auto const gridX = int(32 - node->x / SIZE_OF_GRIDS);
        auto const gridY = int(32 - node->y / SIZE_OF_GRIDS);

        // Bad positions
        if (gridX < 0 || gridY < 0)
            continue;

        // Bad map id
        if (!sMapStore.LookupEntry(node->map_id))
            continue;

        auto const zoneId = sMapMgr->GetZoneId(node->map_id, node->x, node->y, node->z);
        if (zoneId == 0)
            continue;

        auto const worldMapArea = sWorldMapAreaStore.LookupEntry(zoneId);
        if (!worldMapArea || worldMapArea->minRecommendedLevel == 0)
            continue;

        // Hackfix for TwilightHighlands map swapping
        auto const minLevel = (worldMapArea->area_id != 4922)
                ? worldMapArea->minRecommendedLevel
                : 84u;

        uint8 const field = uint8((node->ID - 1) / 8);
        uint32 const submask = 1 << ((node->ID - 1) % 8);

        if (node->MountCreatureID[0] != 0)
            nodeMap[0][minLevel - 1][field] |= submask;

        if (node->MountCreatureID[1] != 0)
            nodeMap[1][minLevel - 1][field] |= submask;
    }

    for (size_t i = 0; i < 2; ++i)
    {
        auto &nodes = s_levelupNodes[i];

        for (auto const &j : nodeMap[i])
        {
            nodes[j.first].reserve(j.second.size());
            for (auto const &k : j.second)
                nodes[j.first].emplace_back(k.first, k.second);
        }
    }
}

void PlayerTaxi::InitTaxiNodesForLevel(uint8 race, uint8 chrClass, uint8 oldLevel, uint8 newLevel)
{
    // class specific initial known nodes
    switch (chrClass)
    {
        case CLASS_DEATH_KNIGHT:
        {
            for (uint8 i = 0; i < TaxiMaskSize; ++i)
                m_mask[i] |= sOldContinentsNodesMask[i];
            break;
        }
    }

    // race specific initial known nodes: capital and taxi hub masks
    // @TODO MISSING PANDAREN HORDE/ALLIANCE WORGEN and GOBLIN
    switch (race)
    {
        case RACE_HUMAN:
            SetTaximaskNode(2);
            break;
        case RACE_ORC:
            SetTaximaskNode(23);
            break;
        case RACE_DWARF:
            SetTaximaskNode(6);
            break;
        case RACE_NIGHTELF:
            SetTaximaskNode(26);
            SetTaximaskNode(27);
            break;
        case RACE_UNDEAD_PLAYER:
            SetTaximaskNode(11);
            break;
        case RACE_TAUREN:
            SetTaximaskNode(22);
            break;
        case RACE_GNOME:
            SetTaximaskNode(6);
            break;
        case RACE_TROLL:
            SetTaximaskNode(23);
            break;
        case RACE_BLOODELF:
            SetTaximaskNode(82);
            break;
        case RACE_DRAENEI:
            SetTaximaskNode(94);
            break;
    }

    auto const team = Player::TeamForRace(race);

    // New continent starting masks (It will be accessible only at new map)
    switch (team)
    {
        case HORDE:
            SetTaximaskNode(99);
            break;
        case ALLIANCE:
            SetTaximaskNode(100);
            break;
        case PANDAREN_NEUTRAL:
            return;
    }

    // Level dependent taxi hubs
    if (newLevel >= 68) {
        // Shattered Sun Staging Area
        SetTaximaskNode(213);
    }

    auto const maxLevel = std::min<size_t>(newLevel, DEFAULT_MAX_LEVEL);

    // Add Taxi Nodes available from player level
    for (size_t i = oldLevel; i < maxLevel; ++i)
        for (auto const &node : s_levelupNodes[team == HORDE ? 0 : 1][i])
            SetTaximaskNode(node.first, node.second);
}

void PlayerTaxi::LoadTaxiMask(std::string const &data)
{
    Tokenizer tokens(data, ' ');

    uint8 index = 0;
    for (auto iter = tokens.begin(); index < TaxiMaskSize && iter != tokens.end(); ++iter, ++index)
    {
        // load and set bits only for existing taxi nodes
        m_mask[index] = sTaxiNodesMask[index] & Trinity::lexicalCast<uint32>(*iter);
    }
}

void PlayerTaxi::AppendTaximaskTo(ByteBuffer &data, ByteBuffer &dataBuffer, bool all)
{
    data.WriteBits(TaxiMaskSize, 24);

    if (all)
    {
        // all existing nodes
        for (uint8 i = 0; i < TaxiMaskSize; ++i)
            dataBuffer << uint8(sTaxiNodesMask[i]);
    }
    else
    {
        // known nodes
        for (uint8 i = 0; i < TaxiMaskSize; ++i)
            dataBuffer << uint8(m_mask[i]);
    }
}

bool PlayerTaxi::LoadTaxiDestinationsFromString(std::string const &values, uint32 team)
{
    ClearTaxiDestinations();

    for (auto const &token : Tokenizer(values, ' '))
        AddTaxiDestination(Trinity::lexicalCast<uint32>(token));

    if (m_destinations.empty())
        return true;

    // Check integrity
    if (m_destinations.size() < 2)
        return false;

    for (size_t i = 1; i < m_destinations.size(); ++i)
    {
        uint32 cost, path;
        sObjectMgr->GetTaxiPath(m_destinations[i - 1], m_destinations[i], path, cost);
        if (path == 0)
            return false;
    }

    // can't load taxi path without mount set (quest taxi path?)
    if (!sObjectMgr->GetTaxiMountDisplayId(GetTaxiSource(), team, true))
        return false;

    return true;
}

std::string PlayerTaxi::SaveTaxiDestinationsToString() const
{
    if (m_destinations.empty())
        return "";

    std::ostringstream ss;

    for (auto const &dest : m_destinations)
        ss << dest << ' ';

    return ss.str();
}

uint32 PlayerTaxi::GetCurrentTaxiPath() const
{
    if (m_destinations.size() < 2)
        return 0;

    uint32 path, cost;
    sObjectMgr->GetTaxiPath(m_destinations[0], m_destinations[1], path, cost);
    return path;
}

std::ostringstream & operator<<(std::ostringstream &ss, PlayerTaxi const &taxi)
{
    for (auto const &mask : taxi.m_mask)
        ss << static_cast<uint32>(mask) << ' ';
    return ss;
}

} // namespace Trinity
