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

#ifndef TRINITY_ARRAY_SIZE_H
#define TRINITY_ARRAY_SIZE_H

#include <cstddef>

namespace Trinity {
namespace Detail {

template <std::size_t N>
struct TypeOfSize
{
    typedef char type[N];
};

template <typename T, std::size_t Size>
typename TypeOfSize<Size>::type & ArraySizeHelper(T(&)[Size]);

} // namespace Detail
} // namespace Trinity

#define TC_ARRAY_SIZE(arr__) sizeof(Trinity::Detail::ArraySizeHelper(arr__))

#endif
