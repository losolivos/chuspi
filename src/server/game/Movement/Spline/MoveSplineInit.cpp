/*
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

#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "MovementPacketBuilder.h"
#include "Unit.h"
#include "Transport.h"
#include "Vehicle.h"
#include "WorldPacket.h"
#include "Opcodes.h"

namespace {

template<typename T, T limit>
class counter
{
public:
    counter()
        : m_counter()
    { }

    T NewId() { Increase(); return m_counter; }

    T getCurrent() const { return m_counter; }

private:
    void Increase()
    {
        if (m_counter == limit)
            m_counter = T();
        else
            ++m_counter;
    }

    T m_counter;
};

typedef counter<uint32, 0xFFFFFFFF> UInt32Counter;

UInt32Counter splineIdGen;

} // namespace

namespace Movement
{
    UnitMoveType SelectSpeedType(uint32 moveFlags)
    {
        /*! Not sure about MOVEMENTFLAG_CAN_FLY here - do creatures that can fly
            but are on ground right now also have it? If yes, this needs a more
            dynamic check, such as is flying now
        */
        if (moveFlags & (MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY))
        {
            if (moveFlags & MOVEMENTFLAG_BACKWARD /*&& speed_obj.flight >= speed_obj.flight_back*/)
                return MOVE_FLIGHT_BACK;
            else
                return MOVE_FLIGHT;
        }
        else if (moveFlags & MOVEMENTFLAG_SWIMMING)
        {
            if (moveFlags & MOVEMENTFLAG_BACKWARD /*&& speed_obj.swim >= speed_obj.swim_back*/)
                return MOVE_SWIM_BACK;
            else
                return MOVE_SWIM;
        }
        else if (moveFlags & MOVEMENTFLAG_WALKING)
        {
            //if (speed_obj.run > speed_obj.walk)
            return MOVE_WALK;
        }
        else if (moveFlags & MOVEMENTFLAG_BACKWARD /*&& speed_obj.run >= speed_obj.run_back*/)
            return MOVE_RUN_BACK;

        return MOVE_RUN;
    }

    enum MonsterMoveType
    {
        MonsterMoveNormal       = 0,
        MonsterMoveStop         = 1,
        MonsterMoveFacingSpot   = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle  = 4
    };

    void MoveSplineInit::Launch()
    {
        MoveSpline& move_spline = *unit->movespline;

        Location real_position(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZMinusOffset(), unit->GetOrientation());
        // Elevators also use MOVEMENTFLAG_ONTRANSPORT but we do not keep track of their position changes
        if (unit->GetTransGUID())
        {
            real_position.x = unit->GetTransOffsetX();
            real_position.y = unit->GetTransOffsetY();
            real_position.z = unit->GetTransOffsetZ();
            real_position.orientation = unit->GetTransOffsetO();
        }

        // there is a big chance that current position is unknown if current state is not finalized, need compute it
        // this also allows calculate spline position and update map position in much greater intervals
        // Don't compute for transport movement if the unit is in a motion between two transports
        if (!move_spline.Finalized() && move_spline.onTransport == (unit->GetTransGUID() != 0))
            real_position = move_spline.ComputePosition();

        // should i do the things that user should do? - no.
        if (args.path.empty())
            return;

        // correct first vertex
        args.path[0] = real_position;
        args.initialOrientation = real_position.orientation;
        move_spline.onTransport = (unit->GetTransGUID() != 0);

        uint32 moveFlags = unit->m_movementInfo.GetMovementFlags();
        moveFlags |= MOVEMENTFLAG_FORWARD;

        if (moveFlags & MOVEMENTFLAG_ROOT)
            moveFlags &= ~MOVEMENTFLAG_MASK_MOVING;

        if (!args.HasVelocity)
        {
            // If spline is initialized with SetWalk method it only means we need to select
            // walk move speed for it but not add walk flag to unit
            uint32 moveFlagsForSpeed = moveFlags;
            if (args.flags.walkmode)
                moveFlagsForSpeed |= MOVEMENTFLAG_WALKING;
            else
                moveFlagsForSpeed &= ~MOVEMENTFLAG_WALKING;

            args.velocity = unit->GetSpeed(SelectSpeedType(moveFlagsForSpeed));
        }

        if (!args.Validate(unit))
            return;

        unit->m_movementInfo.SetMovementFlags(moveFlags);
        move_spline.Initialize(args);

        WorldPacket data(SMSG_MONSTER_MOVE, 64);
        PacketBuilder::BuildLaunchPacket(data, unit);
        unit->SendMessageToSet(&data, true);
    }

    void MoveSplineInit::LaunchTeleport()
    {
        if (args.path.size() != 2)
            return;

        args.path[0] = args.path[1];
        args.flags = 0;
        args.velocity = unit->GetSpeed(MOVE_RUN);

        MoveSpline& moveSpline = *unit->movespline;
        moveSpline.onTransport = (unit->GetTransGUID() != 0);
        moveSpline.Initialize(args);

        WorldPacket data(SMSG_MONSTER_MOVE, 64);
        PacketBuilder::BuildLaunchPacket(data, unit);
        unit->SendMessageToSet(&data, true);
    }

    void MoveSplineInit::Stop()
    {
        MoveSpline& move_spline = *unit->movespline;

        // No need to stop if we are not moving
        bool stopped = move_spline.Finalized();

        Location loc = (!stopped)
                ? move_spline.ComputePosition()
                : Location();

        args.flags = MoveSplineFlag::Done;
        unit->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FORWARD);
        move_spline.Initialize(args);

        if (stopped)
            return;

        WorldPacket data(SMSG_MONSTER_MOVE, 64);
        ObjectGuid moverGUID = unit->GetGUID();
        ObjectGuid transportGUID = unit->GetTransGUID();

        int8 seat = unit->GetTransSeat();

        bool hasUnk1 = false;
        bool hasUnk2 = false;
        bool hasUnk3 = false;
        bool unk4 = false;
        uint32 unkCounter = 0;
        uint32 packedWPcount = 0;
        uint32 WPcount = 0;
        uint8 splineType = MonsterMoveStop;

        // Writes bits
        data.WriteBit(hasUnk1);                     // unk, has counter + 2 bits & somes uint16/float
        data.WriteBitSeq<5>(moverGUID);
        data.WriteBit(true);                        // !hasAnimationTime
        data.WriteBit(true);                        // has duration
        data.WriteBitSeq<4, 3>(moverGUID);
        data.WriteBit(true);                        // !unk, send uint32
        data.WriteBit(seat == -1);                  // !has seat
        data.WriteBitSeq<2>(moverGUID);
        data.WriteBit(true);        // !hasFlags
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

        data.WriteBit(true);      // animation state
        data.WriteBit(true);      // !hasParabolicTime
        data.WriteBit(true);      // !hasParabolicSpeed
        data.WriteBitSeq<6>(moverGUID);
        data.WriteBits(WPcount, 20);
        data.WriteBitSeq<1>(moverGUID);
        data.WriteBit(!hasUnk3);                    // !hasUnk3
        data.WriteBits(splineType, 3);              // splineType

        data.WriteBit(unk4);                        // unk bit 38

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

        data << float(loc.y);
        data.WriteByteSeq<7>(moverGUID);

        data << float(0.0f);                    // unk float
        data << float(loc.z);
        data << float(0.0f);                    // unk float

        data.WriteByteSeq<5>(moverGUID);
        data << uint32(getMSTime());            // Move Ticks

        data.WriteByteSeq<0>(moverGUID);

        if (hasUnk2)
            data << uint8(0);                   // unk byte

        data << float(loc.x);

        data.WriteByteSeq<4>(moverGUID);

        if (seat != -1)
            data << int8(seat);

        if (hasUnk3)
            data << uint8(0);                   // unk byte

        data << float(0.0f);                    // unk float

        if (false)
            data << uint32(0);                  // unk uint32

        data.WriteByteSeq<6, 2, 3, 1>(moverGUID);

        unit->SendMessageToSet(&data, true);
    }

    MoveSplineInit::MoveSplineInit(Unit* m) : unit(m)
    {
        args.splineId = splineIdGen.NewId();
        // Elevators also use MOVEMENTFLAG_ONTRANSPORT but we do not keep track of their position changes
        args.TransformForTransport = unit->GetTransGUID();
        // mix existing state into new
        args.flags.walkmode = unit->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING);
        args.flags.flying = unit->m_movementInfo.HasMovementFlag(MovementFlags(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY));
        args.flags.smoothGroundPath = true; // enabled by default, CatmullRom mode or client config "pathSmoothing" will disable this
    }

    void MoveSplineInit::SetFacing(const Unit* target)
    {
        args.flags.EnableFacingTarget();
        args.facing.target = target->GetGUID();
    }

    void MoveSplineInit::SetFacing(float angle)
    {
        if (args.TransformForTransport)
        {
            if (Unit* vehicle = unit->GetVehicleBase())
                angle -= vehicle->GetOrientation();
            else if (Transport* transport = unit->GetTransport())
                angle -= transport->GetOrientation();
        }

        args.facing.angle = G3D::wrap(angle, 0.f, (float)G3D::twoPi());
        args.flags.EnableFacingAngle();
    }

    void MoveSplineInit::MoveTo(const G3D::Vector3& dest)
    {
        args.path_Idx_offset = 0;
        args.path.resize(2);
        TransportPathTransform transform(unit, args.TransformForTransport);
        args.path[1] = transform(dest);
    }

    void MoveSplineInit::SetFall()
    {
        args.flags.EnableFalling();
        args.flags.fallingSlow = unit->HasUnitMovementFlag(MOVEMENTFLAG_FALLING_SLOW);
    }

    G3D::Vector3 TransportPathTransform::operator()(G3D::Vector3 const & input)
    {
        if (!_transformForTransport)
            return input;

        TransportBase const *transport = _owner->GetDirectTransport();
        if (!transport)
            return input;

        G3D::Vector3 ret(input);
        float o;
        transport->CalculatePassengerOffset(ret.x, ret.y, ret.z, o);
        return ret;
    }
}
