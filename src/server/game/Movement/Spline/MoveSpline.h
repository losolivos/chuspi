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

#ifndef TRINITYSERVER_MOVEPLINE_H
#define TRINITYSERVER_MOVEPLINE_H

#include "Spline.h"
#include "MoveSplineInitArgs.h"

namespace Movement
{
    struct Location : public G3D::Vector3
    {
        Location() : orientation(0) { }
        Location(float x, float y, float z, float o) : G3D::Vector3(x, y, z), orientation(o) { }
        Location(const G3D::Vector3& v) : G3D::Vector3(v), orientation(0) { }
        Location(const G3D::Vector3& v, float o) : G3D::Vector3(v), orientation(o) { }

        float orientation;
    };

    // MoveSpline represents smooth catmullrom or linear curve and point that moves belong it
    // curve can be cyclic - in this case movement will be cyclic
    // point can have vertical acceleration motion componemt(used in fall, parabolic movement)
    class MoveSpline
    {
    public:
        typedef Spline<int32> MySpline;

        enum UpdateResult
        {
            Result_None         = 0x01,
            Result_Arrived      = 0x02,
            Result_NextCycle    = 0x04,
            Result_NextSegment  = 0x08
        };

        friend struct PacketBuilder;

        MoveSpline();

        MySpline::LengthType Duration() const { return spline.length(); }
        void Interrupt() { splineflags.done = true;}

        void Initialize(const MoveSplineInitArgs&);
        bool Initialized() const { return !spline.empty(); }

        template<class UpdateHandler>
        void updateState(int32 difftime, UpdateHandler& handler)
        {
            ASSERT(Initialized());

            do {
                handler(_updateState(difftime));
            } while (difftime > 0);
        }

        void updateState(int32 difftime)
        {
            ASSERT(Initialized());

            do {
                _updateState(difftime);
            } while (difftime > 0);
        }

        Location ComputePosition() const;

        uint32 GetId() const { return m_Id; }
        bool Finalized() const { return splineflags.done; }
        bool isCyclic() const { return splineflags.cyclic; }
        bool isFalling() const { return splineflags.falling; }
        const G3D::Vector3 & FinalDestination() const { return Initialized() ? spline.getPoint(spline.last()) : G3D::Vector3::zero(); }
        const G3D::Vector3 & CurrentDestination() const { return Initialized() ? spline.getPoint(point_Idx+1) : G3D::Vector3::zero(); }
        int32 currentPathIdx() const;

        bool onTransport;
        std::string ToString() const;

    private:
        MySpline spline;

        FacingInfo facing;

        uint32 m_Id;

        MoveSplineFlag splineflags;

        MySpline::LengthType time_passed;
        MySpline::LengthType effect_start_time;

        // currently duration mods are unused, but its _currently_
        //float duration_mod;
        //float duration_mod_next;

        float vertical_acceleration;
        float initialOrientation;
        size_t point_Idx;
        size_t point_Idx_offset;

        void init_spline(const MoveSplineInitArgs& args);

        void _Finalize();

        const MySpline::ControlArray& getPath() const { return spline.getPoints(); }
        void computeParabolicElevation(float& el) const;
        void computeFallElevation(float& el) const;

        UpdateResult _updateState(int32& ms_time_diff);
        MySpline::LengthType next_timestamp() const { return spline.length(point_Idx + 1); }
        MySpline::LengthType segment_time_elapsed() const { return next_timestamp() - time_passed; }
        MySpline::LengthType timePassed() const { return time_passed; }
    };
}

#endif // TRINITYSERVER_MOVEPLINE_H
