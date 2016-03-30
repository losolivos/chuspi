/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "Logger.h"

Logger::Logger(): name_(""), level_(LOG_LEVEL_DISABLED) { }

void Logger::Create(std::string const& name, LogLevel level)
{
    name_ = name;
    level_ = level;
}

std::string const& Logger::getName() const
{
    return name_;
}

LogLevel Logger::getLogLevel() const
{
    return level_;
}

void Logger::addAppender(uint8 id, Appender* appender)
{
    appenders_[id] = appender;
}

void Logger::delAppender(uint8 id)
{
    appenders_.erase(id);
}

void Logger::setLogLevel(LogLevel level)
{
    level_ = level;
}

void Logger::write(LogMessage& message) const
{
    if (!level_ || level_ > message.level || message.text.empty())
    {
        //fprintf(stderr, "Logger::write: Logger %s, Level %u. Msg %s Level %u WRONG LEVEL MASK OR EMPTY MSG\n", getName().c_str(), getLogLevel(), message.text.c_str(), message.level);
        return;
    }

    for (AppenderMap::const_iterator it = appenders_.begin(); it != appenders_.end(); ++it)
        if (it->second)
            it->second->write(message);
}
