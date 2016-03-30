/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "MovementPacketBuilder.h"
#include "MoveSpline.h"
#include "WorldPacket.h"
#include "Unit.h"

#include <G3D/Vector3.h>

namespace Movement
{
    inline void operator << (ByteBuffer& b, const G3D::Vector3& v)
    {
        b << v.x << v.y << v.z;
    }

    inline void operator >> (ByteBuffer& b, G3D::Vector3& v)
    {
        b >> v.x >> v.y >> v.z;
    }

    void WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 last_idx = spline.getPointCount() - 3;
        const G3D::Vector3 * real_path = &spline.getPoint(1);

        data << real_path[last_idx];   // destination
        if (last_idx > 1)
        {
            G3D::Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
            G3D::Vector3 offset;
            // first and last points already appended
            for (uint32 i = 1; i < last_idx; ++i)
            {
                offset = middle - real_path[i];
                data.appendPackXYZ(offset.x, offset.y, offset.z);
            }
        }
    }

    void WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 2;

        for (uint32 i = 0; i < count; i++)
            data << spline.getPoint(i+2).y << spline.getPoint(i+2).z << spline.getPoint(i+2).x;

        //data.append<Vector3>(&spline.getPoint(2), count);
    }

    void WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 2;
        data << spline.getPoint(1).y << spline.getPoint(1).z << spline.getPoint(1).x ; // fake point, client will erase it from the spline after first cycle done
        for (uint32 i = 0; i < count; i++)
            data << spline.getPoint(i+1).y << spline.getPoint(i+1).z << spline.getPoint(i+1).x;
        //data.append<Vector3>(&spline.getPoint(1), count);
    }

    enum MonsterMoveType
    {
        MonsterMoveNormal       = 0,
        MonsterMoveStop         = 1,
        MonsterMoveFacingSpot   = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle  = 4
    };

        void PacketBuilder::BuildLaunchPacket(WorldPacket &data, Unit const *unit)
    {
        MoveSpline const &move_spline = *unit->movespline;

        ObjectGuid moverGUID = unit->GetGUID();
        ObjectGuid transportGUID = unit->GetTransGUID();
        MoveSplineFlag splineflags =  move_spline.splineflags;
        splineflags.enter_cycle = move_spline.isCyclic();
        uint32 sendSplineFlags = splineflags & ~MoveSplineFlag::Mask_No_Monster_Move;
        int8 seat = unit->GetTransSeat();

        bool hasUnk1 = false;
        bool hasUnk2 = false;
        bool hasUnk3 = false;
        bool unk4 = false;
        uint32 unkCounter = 0;
        uint32 packedWPcount = splineflags & MoveSplineFlag::UncompressedPath ? 0 : move_spline.spline.getPointCount() - 3;
        uint32 WPcount = !packedWPcount ? move_spline.spline.getPointCount() - 2 : 1;
        uint8 splineType = 0;

        switch (splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
            case MoveSplineFlag::Final_Target:
                splineType = MonsterMoveFacingTarget;
                break;
            case MoveSplineFlag::Final_Angle:
                splineType = MonsterMoveFacingAngle;
                break;
            case MoveSplineFlag::Final_Point:
                splineType = MonsterMoveFacingSpot;
                break;
            default:
                splineType = MonsterMoveNormal;
                break;
        }

        // Writes bits
        data.WriteBit(hasUnk1);                     // unk, has counter + 2 bits & somes uint16/float
        data.WriteBitSeq<5>(moverGUID);
        data.WriteBit(!splineflags.animation);      // !hasAnimationTime
        data.WriteBit(false);                       // !has duration
        data.WriteBitSeq<4, 3>(moverGUID);
        data.WriteBit(true);                        // !unk, send uint32
        data.WriteBit(seat == -1);                  // !has seat
        data.WriteBitSeq<2>(moverGUID);
        data.WriteBit(sendSplineFlags == 0);        // !hasFlags
        data.WriteBitSeq<0>(moverGUID);

        if (hasUnk1)
        {
            data.WriteBits(0, 2);
            data.WriteBits(unkCounter, 22);
        }

        data.WriteBits(packedWPcount, 22);          // packed waypoint count
        data.WriteBit(!hasUnk2);                    // !hasUnk2, unk byte
        data.WriteBitSeq<7>(moverGUID);
        data.WriteBit(false);                       // fake bit

        data.WriteBitSeq<5, 3, 4, 6, 2, 1, 7, 0>(transportGUID);

        data.WriteBit(!splineflags.animation);      // animation state
        data.WriteBit(!splineflags.parabolic);      // !hasParabolicTime
        data.WriteBit(!splineflags.parabolic);      // !hasParabolicSpeed
        data.WriteBitSeq<6>(moverGUID);
        data.WriteBits(WPcount, 20);
        data.WriteBitSeq<1>(moverGUID);
        data.WriteBit(!hasUnk3);                    // !hasUnk3
        data.WriteBits(splineType, 3);              // splineType

        if (splineType == MonsterMoveFacingTarget)
        {
            ObjectGuid facingTargetGUID = move_spline.facing.target;
            data.WriteBitSeq<4, 6, 5, 1, 0, 7, 3, 2>(facingTargetGUID);
        }

        data.WriteBit(unk4);                       // unk bit 38

        data.FlushBits();

        // Write bytes
        if (hasUnk1)
        {
            data << float(0.0f);
            data << float(0.0f);
            data << uint16(0);

            for (uint32 i = 0; i < unkCounter; i++)
            {
                data << uint16(0);
                data << uint16(0);
            }

            data << uint16(0);
        }

        data.WriteByteSeq<0, 1, 2, 7, 3, 4, 6, 5>(transportGUID);

        if (splineType == MonsterMoveFacingTarget)
        {
            ObjectGuid facingTargetGUID = move_spline.facing.target;
            data.WriteByteSeq<2, 1, 7, 0, 5, 3, 4, 6>(facingTargetGUID);
        }

        data << float(move_spline.spline.getPoint(move_spline.spline.first()).y);
        data.WriteByteSeq<7>(moverGUID);

        if (splineflags.animation)
            data << int32(move_spline.effect_start_time);

        if ((splineflags & MoveSplineFlag::UncompressedPath) == 0)
        {
            uint32 last_idx = move_spline.spline.getPointCount() - 3;
            const G3D::Vector3 * real_path = &move_spline.spline.getPoint(1);
            if (last_idx > 0)
            {
                G3D::Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
                G3D::Vector3 offset;
                // end points already appended
                for (uint32 i = 0; i < last_idx; ++i)
                {
                    offset = middle - real_path[i];
                    data.appendPackXYZ(offset.x, offset.y, offset.z);
                }
            }
        }
         //   WriteLinearPath(move_spline.spline, data);

        data << float(0.0f);                    // unk float
        data << float(move_spline.spline.getPoint(move_spline.spline.first()).z);
        data << float(0.0f);                    // unk float

        if (splineType == MonsterMoveFacingSpot)
            data << move_spline.facing.f.z << move_spline.facing.f.x << move_spline.facing.f.y;

        if (splineflags & MoveSplineFlag::UncompressedPath)
        {
            if (splineflags.cyclic)
                WriteCatmullRomCyclicPath(move_spline.spline, data);
            else
                WriteCatmullRomPath(move_spline.spline, data);
        }
        // Append end point for packed waypoints
        else
        {
            uint32 last_idx = move_spline.spline.getPointCount() - 2;
            const G3D::Vector3 * real_path = &move_spline.spline.getPoint(1);
            data << real_path[last_idx].y << real_path[last_idx].z << real_path[last_idx].x; // destination
        }

        data.WriteByteSeq<5>(moverGUID);
        data << uint32(getMSTime());            // Move Ticks

        if (sendSplineFlags)
            data << uint32(sendSplineFlags);

        if (splineflags.animation)
            data << uint8(splineflags.getAnimationId());

        data.WriteByteSeq<0>(moverGUID);

        if (hasUnk2)
            data << uint8(0);                   // unk byte

        if (splineflags.parabolic)
            data << move_spline.effect_start_time;

        data << float(move_spline.spline.getPoint(move_spline.spline.first()).x);
        data << move_spline.Duration();
        data.WriteByteSeq<4>(moverGUID);

        if (splineflags.parabolic)
            data << move_spline.vertical_acceleration;

        if (seat != -1)
            data << int8(seat);

        if (splineType == MonsterMoveFacingAngle)
            data << move_spline.facing.angle;

        if (hasUnk3)
            data << uint8(0);                   // unk byte

        data << float(0.0f);                    // unk float

        if (false)
            data << uint32(0);                  // unk uint32

        data.WriteByteSeq<6, 2, 3, 1>(moverGUID);
    }

    void PacketBuilder::WriteCreateBits(MoveSpline const& moveSpline, ByteBuffer& data)
    {
        ASSERT(!moveSpline.Finalized());

        MoveSplineFlag flags = moveSpline.splineflags;

        data.WriteBit(true);
        data.WriteBit(flags.parabolic || flags.animation);

        data.WriteBits(uint8(moveSpline.spline.mode()), 2);
        data.WriteBits(flags.raw(), 25);
        data.WriteBit(false);
        if (false)
        {
            data.WriteBits(0, 21);
            data.WriteBits(0, 2);
        }
        data.WriteBits(moveSpline.getPath().size(), 20);
        data.WriteBit(flags.parabolic);
    }

    void PacketBuilder::WriteCreateData(MoveSpline const& moveSpline, ByteBuffer& data)
    {
        if (/*!moveSpline.Finalized()*/true)
        {
            MoveSplineFlag splineFlags = moveSpline.splineflags;

            uint8 splineType = 0;
            switch (splineFlags & MoveSplineFlag::Mask_Final_Facing)
            {
                case MoveSplineFlag::Final_Target:
                    splineType = MonsterMoveFacingTarget;
                    break;
                case MoveSplineFlag::Final_Angle:
                    splineType = MonsterMoveFacingAngle;
                    break;
                case MoveSplineFlag::Final_Point:
                    splineType = MonsterMoveFacingSpot;
                    break;
                default:
                    splineType = MonsterMoveNormal;
                    break;
            }

            data << uint8(splineType);
            data << float(1.0f);                             // splineInfo.duration_mod; added in 3.1

            uint32 nodes = moveSpline.getPath().size();
            for (uint32 i = 0; i < nodes; ++i)
            {
                data << float(moveSpline.getPath()[i].z);
                data << float(moveSpline.getPath()[i].x);
                data << float(moveSpline.getPath()[i].y);
            }

            data << float(1.0f);                             // splineInfo.duration_mod_next; added in 3.1

            if (splineFlags.final_point)
                data << moveSpline.facing.f.z << moveSpline.facing.f.y << moveSpline.facing.f.x;

            if (splineFlags.final_angle)
                data << moveSpline.facing.angle;

            if (splineFlags.parabolic)
                data << moveSpline.vertical_acceleration;   // added in 3.1

            if (splineFlags.parabolic || splineFlags.animation)
                data << moveSpline.effect_start_time;       // added in 3.1

            data << moveSpline.timePassed();
            data << moveSpline.Duration();
        }

        if (!moveSpline.isCyclic())
        {
            G3D::Vector3 dest = moveSpline.FinalDestination();
            data << moveSpline.GetId();
            data << float(dest.z);
            data << float(dest.x);
            data << float(dest.y);
        }
        else
        {
            data << moveSpline.GetId();
            data << float(0.0f);
            data << float(0.0f);
            data << float(0.0f);
        }
    }

    void PacketBuilder::WriteCreateGuid(MoveSpline const& moveSpline, ByteBuffer& data)
    {
        if ((moveSpline.splineflags & MoveSplineFlag::Mask_Final_Facing) == MoveSplineFlag::Final_Target)
        {
            ObjectGuid facingGuid = moveSpline.facing.target;

            data.WriteBitSeq<3, 1, 0, 7, 6, 4, 5, 2>(facingGuid);
            data.WriteByteSeq<3, 0, 4, 6, 1, 5, 2, 7>(facingGuid);
        }
    }
}
