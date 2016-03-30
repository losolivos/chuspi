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

#include "WorldPacket.h"
#include "Log.h"
#include "World.h"

#include <zlib.h>

namespace {

void DoCompress(z_stream* compressionStream, void* dst, uint32 *dst_size, const void* src, int src_size)
{
    compressionStream->next_out = (Bytef*)dst;
    compressionStream->avail_out = *dst_size;
    compressionStream->next_in = (Bytef*)src;
    compressionStream->avail_in = (uInt)src_size;

    int32 z_res = deflate(compressionStream, Z_SYNC_FLUSH);
    if (z_res != Z_OK)
    {
        TC_LOG_ERROR("network", "Can't compress packet (zlib: deflate) Error code: %i (%s, msg: %s)", z_res, zError(z_res), compressionStream->msg);
        *dst_size = 0;
        return;
    }

    if (compressionStream->avail_in != 0)
    {
        TC_LOG_ERROR("network", "Can't compress packet (zlib: deflate not greedy)");
        *dst_size = 0;
        return;
    }

    *dst_size -= compressionStream->avail_out;
}

} // namespace

//! Compresses packet in place
void WorldPacket::Compress(z_stream* compressionStream)
{
    Opcodes uncompressedOpcode = GetOpcode();
    if (uncompressedOpcode & COMPRESSED_OPCODE_MASK)
    {
        TC_LOG_ERROR("network", "Packet with opcode 0x%04X is already compressed!", uncompressedOpcode);
        return;
    }

    Opcodes opcode = Opcodes(uncompressedOpcode | COMPRESSED_OPCODE_MASK);
    uint32 size = wpos();
    uint32 destsize = compressBound(size);

    std::vector<uint8> storage(sizeof(uint32) + destsize);

    DoCompress(compressionStream, &storage[0] + sizeof(uint32), &destsize, contents(), size);
    if (destsize == 0)
        return;

    std::swap(storage, _storage);

    resize(sizeof(uint32) + destsize);

    put<uint32>(0, size);
    SetOpcode(opcode);

    TC_LOG_INFO("network", "Successfully compressed opcode %u (len %u) to %u (len %u)", uncompressedOpcode, size, opcode, destsize);
}

//! Compresses another packet and stores it in self (source left intact)
void WorldPacket::Compress(z_stream* compressionStream, WorldPacket const* source)
{
    ASSERT(source != this);

    Opcodes uncompressedOpcode = source->GetOpcode();
    if (uncompressedOpcode & COMPRESSED_OPCODE_MASK)
    {
        TC_LOG_ERROR("network", "Packet with opcode 0x%04X is already compressed!", uncompressedOpcode);
        return;
    }

    Opcodes opcode = Opcodes(uncompressedOpcode | COMPRESSED_OPCODE_MASK);
    uint32 size = source->size();
    uint32 destsize = compressBound(size);

    resize(sizeof(uint32) + destsize);

    DoCompress(compressionStream, &_storage[0] + sizeof(uint32), &destsize, source->contents(), size);
    if (destsize == 0)
        return;

    resize(sizeof(uint32) + destsize);

    put<uint32>(0, size);
    SetOpcode(opcode);

    TC_LOG_INFO("network", "Successfully compressed opcode %u (len %u) to %u (len %u)", uncompressedOpcode, size, opcode, destsize);
}
