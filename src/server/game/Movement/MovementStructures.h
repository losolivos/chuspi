/*
* Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#ifndef TRINITY_MOVEMENT_STRUCTURES_H
#define TRINITY_MOVEMENT_STRUCTURES_H

enum MovementStatusElements
{
    MSEHasGuidByte0,
    MSEHasGuidByte1,
    MSEHasGuidByte2,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte7,
    MSEHasMovementFlags,
    MSEHasMovementFlags2,
    MSEHasTimestamp,
    MSEHasOrientation,
    MSEHasTransportData,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte7,
    MSEHasTransportTime2,
    MSEHasTransportTime3,
    MSEHasPitch,
    MSEHasFallData,
    MSEHasFallDirection,
    MSEHasSplineElevation,
    MSEHasSpline,
    MSEHasAlive32,

    MSEGuidByte0,
    MSEGuidByte1,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte7,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSETimestamp,
    MSEPositionX,
    MSEPositionY,
    MSEPositionZ,
    MSEOrientation,
    MSETransportGuidByte0,
    MSETransportGuidByte1,
    MSETransportGuidByte2,
    MSETransportGuidByte3,
    MSETransportGuidByte4,
    MSETransportGuidByte5,
    MSETransportGuidByte6,
    MSETransportGuidByte7,
    MSETransportPositionX,
    MSETransportPositionY,
    MSETransportPositionZ,
    MSETransportOrientation,
    MSETransportSeat,
    MSETransportTime,
    MSETransportTime2,
    MSETransportTime3,
    MSEPitch,
    MSEFallTime,
    MSEFallVerticalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSESplineElevation,
    MSEBitCounter1,
    MSEBitCounterLoop1,
    MSEAlive32,
    // Special
    MSEFlushBits,   //FlushBits()
    MSEZeroBit, // writes bit value 1 or skips read bit
    MSEOneBit,  // writes bit value 0 or skips read bit
    MSEEnd,     // marks end of parsing
    MSE_COUNT
};

//5.0.5 16048 //TODO FIX ME unkvalues
MovementStatusElements const MovementStartForwardSequence[] =
{
    MSEPositionZ,
    MSEPositionX,
    MSEPositionY,
    MSEHasGuidByte1, //guid1
    MSEHasGuidByte3, //guid3
    MSEHasMovementFlags, //MoveFlags
    MSEHasGuidByte0,//guid0
    MSEHasOrientation,     //_AH & 0x44
    MSEHasGuidByte4,//guid4
    MSEHasPitch,      //_AH & 0x44
    MSEHasGuidByte2,//guid2
    MSEZeroBit,
    MSEZeroBit,
    MSEBitCounter1,
    MSEHasFallData,     //IsInterpolated
    MSEHasGuidByte5,    //guid5
    MSEHasGuidByte7,    //guid7
    MSEHasSplineElevation,      //_AH & 0x44
    MSEHasTransportData,    //isTransport
    MSEZeroBit,      //isAlive_unk1
    MSEHasAlive32,
    MSEHasTimestamp,     // not sure, maybe Unk
    MSEHasMovementFlags2,//MoveFlags2
    MSEHasGuidByte6,    //guid6

    MSEHasTransportGuidByte1,//transport guid 1
    MSEHasTransportGuidByte2,//transport guid 2
    MSEHasTransportGuidByte6,//transport guid 6
    MSEHasTransportTime3,     //transport_unk2
    MSEHasTransportGuidByte3,//transport guid 3
    MSEHasTransportTime2,         //transport_unk1
    MSEHasTransportGuidByte0,//transport guid 0
    MSEHasTransportGuidByte7,//transport guid 7
    MSEHasTransportGuidByte4,//transport guid 4
    MSEHasTransportGuidByte5,//transport guid 5

    MSEMovementFlags,
    MSEMovementFlags2,
    MSEHasFallDirection,           //isFalling

    MSEFlushBits,

    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEBitCounterLoop1,
    MSEGuidByte6,

    MSETransportGuidByte6,
    MSETransportGuidByte2,
    MSETransportPositionY,
    MSETransportGuidByte5,
    MSETransportTime,
    MSETransportPositionX,
    MSETransportGuidByte1,
    MSETransportGuidByte3,
    MSETransportTime3,
    MSETransportGuidByte4,
    MSETransportPositionZ,
    MSETransportGuidByte0,
    MSETransportGuidByte7,
    MSETransportOrientation,
    MSETransportTime2,
    MSETransportSeat,

    MSESplineElevation,
    MSEFallTime,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallVerticalSpeed,

    MSEPitch,
    MSETimestamp,
    MSEAlive32,
    MSEOrientation,
    MSEEnd,
};

//5.0.5 16048
MovementStatusElements const PlayerMoveSequence[] =
{
    MSEZeroBit,             //isAlive_unk1
    MSEHasPitch,            //Pitch
    MSEHasGuidByte4,        //guid4
    MSEHasGuidByte2,        //guid2
    MSEZeroBit,             //unk
    MSEHasFallData,         //IsInterpolated
    MSEHasGuidByte7,        //guid7
    MSEBitCounter1,         //unk bit counter
    MSEHasGuidByte5,        //guid5
    MSEHasGuidByte3,        //guid3
    MSEHasAlive32,          //isAlive32

    MSEHasTransportData,    //isTransport
    MSEHasTransportGuidByte1,//TransGuid1
    MSEHasTransportGuidByte2,//TransGuid2
    MSEHasTransportGuidByte3,//TransGuid3
    MSEHasTransportGuidByte4,//TransGuid4
    MSEHasTransportGuidByte5,//TransGuid5
    MSEHasTransportTime3,    //transport_unk2
    MSEHasTransportTime3,    //transport_unk2
    MSEHasTransportGuidByte0,//TransGuid0
    MSEHasTransportGuidByte7,//TransGuid7
    MSEHasTransportGuidByte6,//TransGuid6

    MSEHasMovementFlags,    //!MoveFlags
    MSEMovementFlags,       //MoveFlags 30bits
    MSEHasOrientation,      //MSEHasOrientation
    MSEHasTimestamp,        //timestamp
    MSEHasFallDirection,    //isFalling
    MSEHasMovementFlags2,    //MoveFlags2
    MSEHasGuidByte6,        //guid6
    MSEHasGuidByte0,        //guid0
    MSEHasGuidByte1,        //guid1
    MSEZeroBit,             //unk
    MSEMovementFlags2,      //MoveFlags2
    MSEHasSplineElevation,  //SplineElevation inverse
    MSEFlushBits,

    MSEPositionX,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallTime,
    MSEFallVerticalSpeed,
    MSEGuidByte3,    //guid3

    MSETransportGuidByte2,  //transguid2
    MSETransportGuidByte0,  //transguid0
    MSETransportGuidByte5,  //transguid5
    MSETransportSeat,       //transSeat
    MSETransportGuidByte4,  //transguid4
    MSETransportGuidByte3,  //transguid3
    MSETransportTime2,      //time2
    MSETransportGuidByte6,  //transguid6
    MSETransportGuidByte7,  //transguid7
    MSETransportPositionX,  //transX
    MSETransportTime3,      //time3
    MSETransportTime,       //TransTime
    MSETransportPositionZ,  //transZ
    MSETransportGuidByte1,  //transguid1
    MSETransportPositionY,  //transY
    MSETransportOrientation,    //TransOrientation
    MSEGuidByte2,
    MSEGuidByte6,
    // Unk counter
    MSEGuidByte1,
    MSEPitch,

    MSEPositionY,
    MSEPositionZ,
    MSEGuidByte4,
    MSETimestamp,
    MSESplineElevation,     //SplineElevation
    MSEAlive32,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEOrientation,
    MSEEnd,
};

//5.0.5 16048
MovementStatusElements const MovementStopSequence[] =
{
    MSEPositionY,
    MSEPositionZ,
    MSEPositionX,

    MSEHasFallData,     //IsInterpolated
    MSEZeroBit,         //isAlive32
    MSEZeroBit,         //isAlive_unk1
    MSEHasOrientation,  //unk _AH & 0x44
    MSEHasGuidByte2,    //Guid2
    MSEHasGuidByte1,    //Guid1
    MSEZeroBit,         //!isAlive_unk3
    MSEHasGuidByte4,    //Guid4

    MSEHasSplineElevation,  //unk _AH & 0x44
    MSEHasGuidByte6,        //Guid6
    MSEHasMovementFlags2,   //MoveFlags2
    MSEZeroBit,             //isAlive_unk4
    MSEHasMovementFlags,    //MoveFlags
    MSEHasGuidByte5,        //Guid5
    MSEHasGuidByte0,        //Guid0
    MSEHasGuidByte3,        //Guid3

    //begin BitCounter1 values on 24bits
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    MSEZeroBit, //unk
    //end bitCounter1 values

    MSEZeroBit,                 //isAlive_unk2
    MSEHasGuidByte7,            //Guid7
    MSEHasTransportData,        //isTransport
    MSEZeroBit,                 //_AH & 0x44
    MSEHasTransportGuidByte6,   //TransGuid6
    MSEHasTransportGuidByte1,   //TransGuid1
    MSEHasTransportGuidByte1,   //TransGuid4
    MSEZeroBit,                 //transport_unk2
    MSEHasTransportGuidByte0,   //TransGuid0
    MSEHasTransportGuidByte7,   //TransGuid7

    MSEZeroBit,                 //transport_unk1
    MSEHasTransportGuidByte5,   //Guid5
    MSEHasTransportGuidByte2,   //Guid2
    MSEHasTransportGuidByte3,   //Guid3
    MSEHasFallDirection,        //IsInterpolated
    MSEMovementFlags,           //MoveFlags
    MSEMovementFlags2,          //MoveFlags2

    //do uint32 unk while BitCounter1 > 0

    MSEGuidByte6,   //GuidByte6
    MSEGuidByte5,   //GuidByte5
    MSEGuidByte1,   //GuidByte1
    MSEGuidByte7,   //GuidByte7
    MSEGuidByte3,   //GuidByte3
    MSEGuidByte2,   //GuidByte2
    MSEGuidByte0,   //GuidByte0
    MSEGuidByte4,   //GuidByte4

    MSETransportPositionZ,  //TransportZ
    MSETransportGuidByte2,  //TransGuidByte2
    MSETransportPositionY,  //TransportY
    MSETransportGuidByte0,  //TransGuidByte0
    MSETransportGuidByte3,  //TransGuidByte3
    MSETransportPositionX,  //TransportX
    MSETransportSeat,       //TransportSeat
    MSETransportOrientation,//TransportO
    MSETransportGuidByte5,  //TransGuidByte5
    MSETransportTime,       //TransportTime
    MSETransportGuidByte6,  //TransGuidByte6
    MSETransportGuidByte7,  //TransGuidByte7
    MSETransportGuidByte4,  //TransGuidByte4
    MSETransportTime2,      //transport_unk2
    MSETransportGuidByte1,  //TransGuidByte1
    MSETransportTime,       //transport_unk1

    MSEPitch,               //Pitch
    MSETimestamp,           //isAlive32

    MSEFallHorizontalSpeed, //xySpeed
    MSEFallSinAngle,        //sinAngle
    MSEFallCosAngle,        //cosAngle
    MSEFallVerticalSpeed,   //zSpeed
    MSEOrientation,         //Orientation
    MSESplineElevation,     //SplineElevation

    //nisAlive_unk3
    MSEZeroBit,
    MSEZeroBit,
    MSEZeroBit,
    MSEZeroBit,

    MSEEnd,
};

#endif // TRINITY_MOVEMENT_STRUCTURES_H
