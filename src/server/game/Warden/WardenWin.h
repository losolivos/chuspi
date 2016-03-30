/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#ifndef TRINITY_GAME_WARDEN_WIN_H
#define TRINITY_GAME_WARDEN_WIN_H

#include "Warden.h"

#include <vector>

class BigNumber;
class ByteBuffer;
class WorldSession;

class WardenWin : public Warden
{
public:
    WardenWin();
    ~WardenWin();

    void Init(WorldSession* session, BigNumber* K);
    void RequestHash();
    void HandleHashResult(ByteBuffer &buff);
    void RequestData();
    void HandleData(ByteBuffer &buff);

private:
    uint32 _serverTicks;
    std::vector<uint16> _otherChecksTodo;
    std::vector<uint16> _memChecksTodo;
    std::vector<uint16> _currentChecks;
};

#endif // TRINITY_GAME_WARDEN_WIN_H
