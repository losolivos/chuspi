/*
 * Copyright (C) 2008-2014 MoltenCore <http://www.molten-wow.com/>
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
#include "siege_of_niuzao_temple.h"


DoorData const doorData[] =
{
    {GO_TEMPLE_INVIS_DOOR,          BOSS_PAVALAK,          DOOR_TYPE_PASSAGE,       BOUNDARY_NONE   },
    {GO_WIND_WALL,                  BOSS_PAVALAK,          DOOR_TYPE_PASSAGE,       BOUNDARY_NONE   },
    {GO_FIRE_WALL,                  BOSS_VOJAK,            DOOR_TYPE_PASSAGE,       BOUNDARY_NONE   },
    {0,                                      0,            DOOR_TYPE_ROOM,          BOUNDARY_NONE   },// END
};

class instance_siege_of_niuzao_temple : public InstanceMapScript
{
    struct instance_impl : public InstanceScript
    {
        instance_impl(Map* map) : InstanceScript(map) {}


        void Initialize()
        {
            invisDoorGUID[0] = 0;
            invisDoorGUID[1] = 0;
            vojakDoorGUID = 0;
            jinbakDoorGUID = 0;
            SetBossNumber(TOTAL_ENCOUNTERS);
            LoadDoorData(doorData);
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_TEMPLE_INVIS_DOOR:
                    invisDoorGUID[invisDoorGUID[0] != 0] = go->GetGUID();
                    break;
                case GO_WIND_WALL:
                case GO_FIRE_WALL:
                    AddDoor(go, true);
                    break;
                case GO_DOOR:
                    vojakDoorGUID = go->GetGUID();
                    break;
                case GO_HARDENED_RESIN:
                    jinbakDoorGUID = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void OnGameObjectRemove(GameObject *go) final
        {
            switch (go->GetEntry())
            {
                case GO_TEMPLE_INVIS_DOOR:
                case GO_WIND_WALL:
                case GO_FIRE_WALL:
                    AddDoor(go, false);
                    break;
            }
        }

        bool SetBossState(uint32 id, EncounterState state)
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case BOSS_JINBAK:
                    if (state == DONE)
                        if (GameObject * go = instance->GetGameObject(jinbakDoorGUID))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                case BOSS_PAVALAK:
                    if (state == DONE)
                        for (int i = 0; i < 2; ++i)
                            if (GameObject * go = instance->GetGameObject(invisDoorGUID[i]))
                                go->Delete();
                    break;
                default:
                    break;
            }

            return true;
        }

        void SetData(uint32 type, uint32 )
        {
            if (type == DATA_VOJAK_DOOR)
                if (GameObject * go = instance->GetGameObject(vojakDoorGUID))
                    go->UseDoorOrButton(DAY, false);
        }

        bool CheckRequiredBosses(uint32 bossId, Player const* player = NULL) const
        {
            if (!InstanceScript::CheckRequiredBosses(bossId, player))
                return false;

            return true;
        }

        void ProcessEvent(WorldObject* /*source*/, uint32 eventId)
        {

        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "N T " << GetBossSaveData();

            OUT_SAVE_INST_DATA_COMPLETE;
            return saveStream.str();
        }

        void Load(const char* str)
        {
            if (!str)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(str);

            char dataHead1, dataHead2;

            std::istringstream loadStream(str);
            loadStream >> dataHead1 >> dataHead2;

            if (dataHead1 == 'N' && dataHead2 == 'T')
            {
                for (uint32 i = 0; i < TOTAL_ENCOUNTERS; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                    SetBossState(i, EncounterState(tmpState));
                }
            } else
                OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    private:
        uint64 vojakDoorGUID;
        uint64 jinbakDoorGUID;
        uint64 invisDoorGUID[2];

    };

public:
    instance_siege_of_niuzao_temple() : InstanceMapScript("instance_siege_of_niuzao_temple", 1011) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_impl(map);
    }
};

void AddSC_instance_siege_of_niuzao_temple()
{
    new instance_siege_of_niuzao_temple();
}