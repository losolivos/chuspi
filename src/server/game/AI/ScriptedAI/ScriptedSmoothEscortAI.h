#ifndef SC_SMOOTH_ESCORTAI_H
#define SC_SMOOTH_ESCORTAI_H

#include "ScriptedCreature.h"

enum eSmoothEscortState
{
    ESCORT_STATE_NONE           = 0x01,
    ESCORT_STATE_ESCORTING      = 0x02,
    ESCORT_STATE_PAUSED         = 0x04,
    ESCORT_STATE_WAITING        = 0x08,
    ESCORT_STATE_COMBAT         = 0x10,
};

struct SmoothEscortAI : public ScriptedAI
{
    struct VehicleEscortWaypoint
    {
        VehicleEscortWaypoint(uint32 _id, float _x, float _y, float _z,
                              uint32 _w, bool _j) :
            id(_id), x(_x), y(_y), z(_z), waitTimeMs(_w), jump(_j)
        { }

        uint32 id;
        float x;
        float y;
        float z;
        uint32 waitTimeMs;
        bool jump;
    };

public:
    SmoothEscortAI(Creature* creature);

    virtual ~SmoothEscortAI() { }

    void AttackStart(Unit* who);

    void MoveInLineOfSight(Unit* who);

    void EnterEvadeMode();

    void UpdateAI(uint32 const diff);

    void AddWaypoint(uint32 id, float x, float y, float z, uint32 waitTimeMs = 0, bool jump = false)
    {
        VehicleEscortWaypoint node(id, x, y, z, waitTimeMs, jump);
        waypointList.push_back(node);
    }

    virtual void WaypointReached(uint32 /*uiPointId*/) { };

    virtual void FinishEscort() { }

    virtual void StartMoveTo(float /*x*/, float /*y*/, float /*z*/) { }

    void JustRespawned();

    void Start(bool run = false, bool repeat = false);

    void StartMove();

    void SetRun(bool bRun) { isRunning = bRun; }

    void SetRepeat(bool value) { repeat = value; }

    void SetEscortPaused(bool paused)
    {
        if (!paused)
        {
            if (HasEscortState(ESCORT_STATE_PAUSED))
            {
                RemoveEscortState(ESCORT_STATE_PAUSED);
                StartMove();
            }
        }
        else
            AddEscortState(ESCORT_STATE_PAUSED);
    }

    bool HasEscortState(uint32 uiEscortState) { return (escortState & uiEscortState); }

    void SetSpeedXY(float speed){ speedXY = speed; }

    void SetSpeedZ(float speed){ speedZ = speed; }

    void AddEscortState(uint32 uiEscortState) { escortState |= uiEscortState; }

    void RemoveEscortState(uint32 uiEscortState) { escortState &= ~uiEscortState; }

    MovementGeneratorType GetMovementGeneratorType() { return WAYPOINT_MOTION_TYPE; }

    void SetDespawnAtEnd(bool Despawn) { despawnAtEnd = Despawn; }

    void SetTimeDiff(int32 diff) { timeDiff = diff; }

private:
    TimeTrackerSmall nextMoveTime;

    int32 timeDiff;

    uint32 escortState;
    uint32 currentWPId;

    float speedXY;
    float speedZ;
    bool isRunning;
    bool repeat;
    bool despawnAtEnd;

    std::list<VehicleEscortWaypoint> waypointList;
    std::list<VehicleEscortWaypoint>::iterator currentWP;
};

#endif
