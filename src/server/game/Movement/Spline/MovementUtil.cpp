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

#include "MoveSplineFlag.h"
#include <math.h>
#include <string>

namespace Movement
{
    double const gravity = 19.29110527038574;

    /// Velocity bounds that makes fall speed limited
    float terminalVelocity = 60.148003f;
    float terminalSafefallVelocity = 7.0f;

    const float terminal_length = float(terminalVelocity * terminalVelocity) / (2.0f * gravity);
    const float terminal_safeFall_length = (terminalSafefallVelocity * terminalSafefallVelocity) / (2.0f * gravity);
    const float terminal_fallTime = float(terminalVelocity / gravity); // the time that needed to reach terminalVelocity
    const float terminal_safeFall_fallTime = float(terminalSafefallVelocity / gravity); // the time that needed to reach terminalVelocity with safefall

    float computeFallTime(float path_length, bool isSafeFall)
    {
        if (path_length < 0.0f)
            return 0.0f;

        float time;
        if (isSafeFall)
        {
            if (path_length >= terminal_safeFall_length)
                time = (path_length - terminal_safeFall_length) / terminalSafefallVelocity + terminal_safeFall_fallTime;
            else
                time = sqrtf(2.0f * path_length / gravity);
        }
        else
        {
            if (path_length >= terminal_length)
                time = (path_length - terminal_length) / terminalVelocity + terminal_fallTime;
            else
                time = sqrtf(2.0f * path_length / gravity);
        }

        return time;
    }

    float computeFallElevation(float t_passed, bool isSafeFall, float start_velocity /*= 0.0f*/)
    {
        float termVel;
        float result;

        if (isSafeFall)
            termVel = terminalSafefallVelocity;
        else
            termVel = terminalVelocity;

        if (start_velocity > termVel)
            start_velocity = termVel;

        float terminal_time = terminal_fallTime - start_velocity / gravity; // the time that needed to reach terminalVelocity

        if (t_passed > terminal_time)
        {
            result = terminalVelocity * (t_passed - terminal_time) +
                start_velocity * terminal_time +
                gravity * terminal_time * terminal_time*0.5f;
        }
        else
            result = t_passed * (start_velocity + t_passed * gravity * 0.5f);

        return result;
    }

    #define STR(x) #x

    char const* g_SplineFlag_names[32] =
    {
        STR(AnimBit1           ),// 0x00000001,
        STR(AnimBit2           ),// 0x00000002,
        STR(AnimBit3           ),// 0x00000004,
        STR(Unknown0           ),// 0x00000008,
        STR(FallingSlow        ),// 0x00000010,
        STR(Done               ),// 0x00000020,
        STR(Falling            ),// 0x00000040,           // Not Compartible With Trajectory Movement
        STR(No_Spline          ),// 0x00000080,
        STR(Unknown2           ),// 0x00000100,
        STR(Flying             ),// 0x00000200,           // Smooth Movement(Catmullrom Interpolation Mode), Flying Animation
        STR(OrientationFixed   ),// 0x00000400,           // Model Orientation Fixed
        STR(Catmullrom         ),// 0x00000800,           // Used Catmullrom Interpolation Mode
        STR(Cyclic             ),// 0x00001000,           // Movement By Cycled Spline
        STR(Enter_Cycle        ),// 0x00002000,           // Everytime Appears With Cyclic Flag In Monster Move Packet
        STR(Frozen             ),// 0x00004000,
        STR(TransportEnter     ),// 0x00008000
        STR(TransportExit      ),// 0x00010000
        STR(Unknown3           ),// 0x00020000,
        STR(Unknown4           ),// 0x00040000,
        STR(OrientationInversed),// 0x00080000,           // Appears With Runmode Flag, Nodes ),// 1, Handles Orientation
        STR(SmoothGroundPath   ),// 0x00100000,
        STR(Walkmode           ),// 0x00200000,
        STR(UncompressedPath   ),// 0x00400000,
        STR(Unknown6           ),// 0x00800000,
        STR(Animation          ),// 0x01000000,           // Animationid (0...3), Uint32 Time, Not Compartible With Trajectory And Fall Movement
        STR(Parabolic          ),// 0x02000000,           // Not Compartible With Fall Movement
        STR(Final_Point        ),// 0x04000000,
        STR(Final_Target       ),// 0x08000000,
        STR(Final_Angle        ),// 0x10000000,
        STR(Unknown7           ),// 0x20000000,
        STR(Unknown8           ),// 0x40000000,
        STR(Unknown9           ),// 0x80000000,
    };

    template<class Flags, int N>
    void print_flags(Flags t, char const* (&names)[N], std::string& str)
    {
        for (int i = 0; i < N; ++i)
        {
            if ((t & Flags(1 << i)) && names[i] != NULL)
                str.append(" ").append(names[i]);
        }
    }

    std::string MoveSplineFlag::ToString() const
    {
        std::string str;
        print_flags(raw(), g_SplineFlag_names, str);
        return str;
    }
}
