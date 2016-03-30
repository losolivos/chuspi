#ifndef __BATTLEGROUNDTA_H
#define __BATTLEGROUNDTA_H

#include "Battleground.h"

enum BattlegroundTAObjectTypes
{
    BG_TA_OBJECT_DOOR_1         = 0,
    BG_TA_OBJECT_DOOR_2         = 1,
    BG_TA_OBJECT_BUFF_1         = 2,
    BG_TA_OBJECT_BUFF_2         = 3,
    BG_TA_OBJECT_MAX            = 4
};

enum BattlegroundTAObjects
{
    BG_TA_OBJECT_TYPE_DOOR_1    = 213196,
    BG_TA_OBJECT_TYPE_DOOR_2    = 213197,
    BG_TA_OBJECT_TYPE_BUFF_1    = 184663,
    BG_TA_OBJECT_TYPE_BUFF_2    = 184664
};

class BattlegroundTAScore : public BattlegroundScore
{
    public:
        BattlegroundTAScore() {};
        virtual ~BattlegroundTAScore() {};
};

class BattlegroundTA : public Battleground
{
    public:
        BattlegroundTA();
        ~BattlegroundTA();

        /* inherited from BattlegroundClass */
        void AddPlayer(Player* player);
        void StartingEventCloseDoors();
        void StartingEventOpenDoors();
        void StartingEventDespawnDoors();
        void RemovePlayer(Player* player, uint64 guid, uint32 team);
        void HandleAreaTrigger(Player* player, uint32 Trigger);
        bool SetupBattleground();
        void Reset();
        void FillInitialWorldStates(WorldPacket &d);
        void HandleKillPlayer(Player* player, Player* killer);

        bool HandlePlayerUnderMap(Player* player);
};
#endif
