#ifndef TRINITY_GAME_PLAYER_TAXI_HPP
#define TRINITY_GAME_PLAYER_TAXI_HPP

#include "Define.h"
#include "DBCStores.h"

#include <deque>
#include <string>
#include <vector>

class ByteBuffer;

namespace Trinity {

class PlayerTaxi
{
    friend std::ostringstream & operator<<(std::ostringstream &, PlayerTaxi const &);

    typedef std::deque<uint32> DestinationQueue;

    typedef std::vector<std::pair<uint8, uint32>> LevelupTaxiNodes;

public:
    PlayerTaxi();

    static void CacheLevelupNodes();

    // Nodes
    void InitTaxiNodesForLevel(uint8 race, uint8 chrClass, uint8 oldLevel, uint8 newLevel);

    void LoadTaxiMask(std::string const &data);

    bool IsTaximaskNodeKnown(uint32 nodeidx) const
    {
        uint8 field = uint8((nodeidx - 1) / 8);
        uint32 submask = 1 << ((nodeidx - 1) % 8);
        return (m_mask[field] & submask) == submask;
    }

    bool SetTaximaskNode(uint32 nodeidx)
    {
        uint8 field = uint8((nodeidx - 1) / 8);
        uint32 submask = 1 << ((nodeidx - 1) % 8);
        return SetTaximaskNode(field, submask);
    }

    bool SetTaximaskNode(uint8 field, uint32 submask)
    {
        if ((m_mask[field] & submask) != submask)
        {
            m_mask[field] |= submask;
            return true;
        }

        return false;
    }

    void AppendTaximaskTo(ByteBuffer &data, ByteBuffer &dataBuffer, bool all);

    // Destinations
    bool LoadTaxiDestinationsFromString(const std::string& values, uint32 team);

    std::string SaveTaxiDestinationsToString() const;

    void ClearTaxiDestinations() { m_destinations.clear(); }

    void AddTaxiDestination(uint32 dest) { m_destinations.push_back(dest); }

    uint32 GetTaxiSource() const { return m_destinations.empty() ? 0 : m_destinations.front(); }

    uint32 GetTaxiDestination() const { return m_destinations.size() < 2 ? 0 : m_destinations[1]; }

    uint32 GetCurrentTaxiPath() const;

    uint32 NextTaxiDestination()
    {
        m_destinations.pop_front();
        return GetTaxiDestination();
    }

    bool empty() const { return m_destinations.empty(); }

private:
    TaxiMask m_mask;
    DestinationQueue m_destinations;

    static LevelupTaxiNodes s_levelupNodes[2][DEFAULT_MAX_LEVEL];
};

std::ostringstream & operator<<(std::ostringstream &ss, PlayerTaxi const &taxi);

} // namespace Trinity

#endif // TRINITY_GAME_PLAYER_TAXI_HPP
