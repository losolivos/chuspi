/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#include "PacketLog.h"
#include "WorldPacket.h"
#include "Config.h"

#include <ctime>

PacketLog::PacketLog()
    : file_(NULL)
{ }

PacketLog::~PacketLog()
{
    if (file_)
        std::fclose(file_);
}

void PacketLog::Initialize(std::string const &logFileName)
{
    if (logFileName.empty())
        return;

    std::string logsDir = sConfigMgr->GetStringDefault("LogsDir", "");
    if (!logsDir.empty() && logsDir.back() != '/' && logsDir.back() != '\\')
        logsDir.push_back('/');

    file_ = std::fopen((logsDir + logFileName).c_str(), "wb");
}

void PacketLog::LogPacket(WorldPacket const &packet, Direction direction)
{
    ByteBuffer data(4 + 4 + 4 + 1 + packet.size());

    data << uint32(packet.GetOpcode())
         << uint32(packet.size())
         << uint32(std::time(NULL))
         << uint8(direction);

    data.append(packet);

    std::fwrite(data.contents(), 1, data.size(), file_);
    std::fflush(file_);
}
