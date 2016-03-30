#include "RatedBgStats.hpp"
#include "Field.h"
#include "World.h"

namespace Trinity {

RatedBgStats::RatedBgStats()
    : m_personalRating()
    , m_matchmakerRating(sWorld->getIntConfig(CONFIG_ARENA_START_MATCHMAKER_RATING))
    , m_seasonGames()
    , m_weekGames()
    , m_thisWeekWins()
    , m_prevWeekWins()
    , m_bestWeekRating()
    , m_bestSeasonRating()
{ }

void RatedBgStats::loadFromDB(Field const *fields)
{
    m_personalRating = fields[0].GetUInt32();
    m_matchmakerRating = fields[1].GetUInt32();
    m_seasonGames = fields[2].GetUInt32();
    m_weekGames = fields[3].GetUInt32();
    m_thisWeekWins = fields[4].GetUInt32();
    m_prevWeekWins = fields[5].GetUInt32();
    m_bestWeekRating = fields[6].GetUInt32();
    m_bestSeasonRating = fields[7].GetUInt32();
}

void RatedBgStats::modifyDueToWin(int32 personalMod, int32 mmrMod)
{
    modifyDueToLoss(personalMod, mmrMod);

    ++m_thisWeekWins;

    if (m_personalRating > m_bestWeekRating)
        m_bestWeekRating = m_personalRating;

    if (m_personalRating > m_bestSeasonRating)
        m_bestSeasonRating = m_personalRating;
}

void RatedBgStats::modifyDueToLoss(int32 personalMod, int32 mmrMod)
{
    if (int32(m_personalRating) + personalMod < 0)
        m_personalRating = 0;
    else
        m_personalRating += personalMod;

    if (int32(m_matchmakerRating) + mmrMod < 0)
        m_matchmakerRating = 0;
    else
        m_matchmakerRating += mmrMod;

    ++m_seasonGames;
    ++m_weekGames;
}

void RatedBgStats::finishWeek()
{
    m_prevWeekWins = m_thisWeekWins;
    m_thisWeekWins = m_weekGames = m_bestWeekRating = 0;
}

} // namespace Trinity
