/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#ifndef TRINITY_COMPRESS_HPP
#define TRINITY_COMPRESS_HPP

#include "Define.h"

namespace zlib {

enum class level
{
    none = 0,
    fast = 1,
    good = 6,
    best = 9
};

size_t max_compressed_size(size_t initial_size);
bool compress(uint8 *dst, size_t &dst_size, const uint8 *src, size_t src_size, level l);
bool decompress(uint8 *dst, size_t &dst_size, const uint8 *src, size_t src_size);

} // namespace zlib

#endif // TRINITY_COMPRESS_HPP
