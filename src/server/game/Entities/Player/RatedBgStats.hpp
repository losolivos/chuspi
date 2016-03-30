#ifndef TRINITY_GAME_RATED_BG_STATS_HPP
#define TRINITY_GAME_RATED_BG_STATS_HPP

#include "Define.h"

class Field;

namespace Trinity {

class RatedBgStats final
{
public:
    RatedBgStats();

    void loadFromDB(Field const *fields);

    uint32 personalRating() const { return m_personalRating; }
    uint32 matchmakerRating() const { return m_matchmakerRating; }

    uint32 seasonGames() const { return m_seasonGames; }
    uint32 weekGames() const { return m_weekGames; }

    uint32 thisWeekWins() const { return m_thisWeekWins; }
    uint32 prevWeekWins() const { return m_prevWeekWins; }

    uint32 bestWeekRating() const { return m_bestWeekRating; }
    uint32 bestSeasonRating() const { return m_bestSeasonRating; }

    void modifyDueToWin(int32 personalMod, int32 mmrMod);
    void modifyDueToLoss(int32 personalMod, int32 mmrMod);

    void finishWeek();

private:
    // FIXME may be PLAYER_FIELD_PVP_INFO can be used instead.
    uint32 m_personalRating;
    uint32 m_matchmakerRating;
    uint32 m_seasonGames;
    uint32 m_weekGames;
    uint32 m_thisWeekWins;
    uint32 m_prevWeekWins;
    uint32 m_bestWeekRating;
    uint32 m_bestSeasonRating;
};

} // namespace Trinity

#endif // TRINITY_GAME_RATED_BG_STATS_HPP
