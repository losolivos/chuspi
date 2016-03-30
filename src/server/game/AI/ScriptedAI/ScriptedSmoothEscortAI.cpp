#include "ScriptedSmoothEscortAI.h"
#include "MoveSplineInit.h"

SmoothEscortAI::SmoothEscortAI(Creature* creature)
    : ScriptedAI(creature)
    , nextMoveTime(0)
    , timeDiff(250)
    , escortState(ESCORT_STATE_NONE)
    , currentWPId(0)
    , speedXY(2.5f)
    , speedZ(10.0f)
    , isRunning(false)
    , repeat(false)
    , despawnAtEnd(false)
{ }

void SmoothEscortAI::AttackStart(Unit* who)
{
    if (!who)
        return;

    if (me->Attack(who, true))
    {
        if (!HasEscortState(ESCORT_STATE_COMBAT))
        {
            AddEscortState(ESCORT_STATE_COMBAT);
            me->StopMoving();
            Position pos;
            me->GetPosition(&pos);
            me->SetHomePosition(pos);
        }

        if (IsCombatMovementAllowed())
            me->GetMotionMaster()->MoveChase(who);
    }
}

void SmoothEscortAI::MoveInLineOfSight(Unit* who)
{
    if (!me->HasUnitState(UNIT_STATE_STUNNED) && who->isTargetableForAttack() && who->isInAccessiblePlaceFor(me))
    {
        if (!me->CanFly() && me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
            return;

        if (me->IsHostileTo(who))
        {
            float fAttackRadius = me->GetAttackDistance(who);

            if (me->IsWithinDistInMap(who, fAttackRadius) && me->IsWithinLOSInMap(who))
            {
                if (who->GetTypeId() == TYPEID_UNIT && !who->GetVictim())
                    who->Attack(me, false);

                if (!me->GetVictim())
                {
                    who->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                    AttackStart(who);
                }
                else
                {
                    me->CombatStart(who);
                    me->AddThreat(who, 0.0f);
                }
            }
        }
    }
}

void SmoothEscortAI::EnterEvadeMode()
{
    if (HasEscortState(ESCORT_STATE_COMBAT))
    {
        Reset();
        RemoveEscortState(ESCORT_STATE_COMBAT);
        me->RemoveAllAuras();
        me->DeleteThreatList();
        me->CombatStop(true);
        me->SetLootRecipient(NULL);
        me->LoadCreaturesAddon();
        me->StopMoving();

        if (HasEscortState(ESCORT_STATE_ESCORTING))
            StartMove();
        else
            me->GetMotionMaster()->MoveTargetedHome();
    }
}

void SmoothEscortAI::UpdateAI(uint32 diff)
{
    if (HasEscortState(ESCORT_STATE_COMBAT) && !me->IsInCombat())
        EnterEvadeMode();

    if (HasEscortState(ESCORT_STATE_PAUSED) || HasEscortState(ESCORT_STATE_NONE) || HasEscortState(ESCORT_STATE_COMBAT))
        return;

    if (me->HasUnitState(UNIT_STATE_ROOT) || me->HasUnitState(UNIT_STATE_STUNNED) || me->HasUnitState(UNIT_STATE_DISTRACTED))
        return;

    nextMoveTime.Update(diff);

    if (nextMoveTime.GetExpiry() <= timeDiff)
    {
        if (waypointList.empty())
        {
            RemoveEscortState(ESCORT_STATE_ESCORTING);
            AddEscortState(ESCORT_STATE_NONE);
            return;
        }

        if (HasEscortState(ESCORT_STATE_WAITING))
        {
            RemoveEscortState(ESCORT_STATE_WAITING);
            StartMove();
            return;
        }

        WaypointReached(currentWP->id);

        if (currentWP->waitTimeMs)
        {
            nextMoveTime.Reset(currentWP->waitTimeMs);
            AddEscortState(ESCORT_STATE_WAITING);
            return;
        }

        if (currentWPId == waypointList.size() - 1)
        {
            if (!repeat)
            {
                me->SetHomePosition(currentWP->x, currentWP->y, currentWP->z, me->GetOrientation());
                me->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
                me->GetMotionMaster()->Initialize();
                waypointList.clear();
                FinishEscort();
                RemoveEscortState(ESCORT_STATE_ESCORTING);
                AddEscortState(ESCORT_STATE_NONE);

                if (despawnAtEnd)
                    me->DespawnOrUnsummon();

                return;
            }

            currentWP = waypointList.begin();
            currentWPId = 0;
        }
        else
        {
            ++currentWP;
            ++currentWPId;
        }

        StartMove();
    }
}

void SmoothEscortAI::JustRespawned()
{
    float x, y, z, o;
    me->GetRespawnPosition(x, y, z, &o);
    me->SetHomePosition(x, y, z, o);
    me->Relocate(x, y, z, o);
}

void SmoothEscortAI::Start(bool run, bool shouldRepeat)
{
    run ? me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING) : me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
    me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
    me->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    me->StopMoving();
    isRunning = run;
    repeat = shouldRepeat;
    escortState = ESCORT_STATE_ESCORTING;
    me->AddUnitState(UNIT_STATE_ROAMING_MOVE);

    if (!waypointList.empty())
    {
        currentWP = waypointList.begin();
        currentWPId = 0;
    }

    StartMove();
}

void SmoothEscortAI::StartMove()
{
    if (HasEscortState(ESCORT_STATE_PAUSED) || HasEscortState(ESCORT_STATE_NONE) || HasEscortState(ESCORT_STATE_COMBAT))
        return;

    if (currentWP->jump)
    {
        float moveTimeHalf = speedZ / Movement::gravity;
        float max_height = -Movement::computeFallElevation(moveTimeHalf,false,-speedZ);
        Movement::MoveSplineInit init(me);
        init.MoveTo(currentWP->x, currentWP->y, currentWP->z);
        init.SetParabolic(max_height,0);
        init.SetVelocity(speedXY);
        init.Launch();
    }
    else
    {
        Movement::MoveSplineInit init(me);
        init.MoveTo(currentWP->x, currentWP->y, currentWP->z);
        init.SetWalk(!isRunning);
        init.Launch();
    }

    nextMoveTime.Reset(me->GetSplineDuration());
    StartMoveTo(currentWP->x, currentWP->y, currentWP->z);
}
