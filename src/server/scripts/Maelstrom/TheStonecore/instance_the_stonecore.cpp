/*
 * Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
 *
 * Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2006-2011 ScriptDev2 <http://www.scriptdev2.com/>
 *
 * Copyright (C) 2010-2011 Project Trinity <http://www.projecttrinity.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "the_stonecore.h"
#include "InstanceScript.h"
#include "ScriptMgr.h"

DoorData const doorData[] =
{
    {GO_ROCK_WALL,  DATA_SLABHIDE, DOOR_TYPE_ROOM,  BOUNDARY_NONE},
    {0,                  0,             DOOR_TYPE_ROOM,  BOUNDARY_NONE} // END
};

class instance_the_stonecore : public InstanceMapScript
{
    public:
        instance_the_stonecore() : InstanceMapScript("instance_the_stonecore", 725) { }

        struct instance_the_stonecore_InstanceMapScript : public InstanceScript
        {
        private:
            uint64 _teleporters[2];
            uint64 _corborusWallGUID;
            uint64 _corborusGUID;
            uint64 _slabhideGUID;
            uint64 _millhouseGUID;
            std::vector<uint64> stalactiteTriggers;
            bool _manastormStatus;
            bool _slabhideTrapStatus;

        public:

            instance_the_stonecore_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetBossNumber(TOTAL_ENCOUNTERS);
                LoadDoorData(doorData);

                memset(_teleporters, 0, sizeof(uint64) * 2);
                _corborusWallGUID = 0;
                _corborusGUID = 0;
                _slabhideGUID = 0;
                _millhouseGUID = 0;
                _manastormStatus = false;
                _slabhideTrapStatus = false;
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch(go->GetEntry())
                {
                case GO_STALACTITE:
                    go->EnableCollision(false); // hack
                    break;
                case GO_ROCKDOOR_BREAK:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    _corborusWallGUID = go->GetGUID();
                    if (_manastormStatus)
                        go->Delete();
                    break;
                case GO_ROCK_WALL:
                    AddDoor(go, true);
                    break;
                }
            }

            void OnGameObjectRemove(GameObject* go)
            {
                switch (go->GetEntry())
                {
                case GO_ROCK_WALL:
                    AddDoor(go, false);
                    break;
                default:
                    break;
                }
            }

            void OnCreatureCreate(Creature * creature)
            {
                switch(creature->GetEntry())
                {
                case NPC_STONECORE_TELEPORTER1:
                    _teleporters[0] = creature->GetGUID();
                    break;
                case NPC_STONECORE_TELEPORTER2:
                    _teleporters[1] = creature->GetGUID();
                    SetTeleporters(true);
                    break;
                case NPC_STONECORE_BRUISER:
                case NPC_STONECORE_SENTRY:
                    creature->SetSheath(SHEATH_STATE_MELEE);
                    break;
                case NPC_MILLHOUSE_MANASTORM:
                    _millhouseGUID = creature->GetGUID();
                    break;
                case BOSS_CORBORUS:
                    if (_manastormStatus)
                    {
                        creature->SetHomePosition(1154.55f, 878.843f, 286.963f, M_PI);
                        creature->Relocate(creature->GetHomePosition());
                    }
                    break;
                case NPC_STALACTITE_TRIGGER_TRASH:
                    stalactiteTriggers.push_back(creature->GetGUID());
                    break;
                case BOSS_SLABHIDE:
                    _slabhideGUID = creature->GetGUID();
                    break;
                default:
                    break;
                }
            }

            uint64 GetData64(uint32 data)
            {
                if (data == DATA_SLABHIDE)
                    return _slabhideGUID;
                return 0;
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                if (state != TO_BE_DECIDED)
                    SetTeleporters(state != IN_PROGRESS);

                 return true;
            }

            void SetTeleporters(bool enabled)
            {
                for (int i=0; i<2; ++i)
                    if (Creature * teleporter = instance->GetCreature(_teleporters[i]))
                    {
                        if (enabled)
                        {
                            teleporter->CastSpell(teleporter, SPELL_TELEPORTER_ACTIVE);
                            teleporter->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        }
                        else
                        {
                            teleporter->RemoveAurasDueToSpell(SPELL_TELEPORTER_ACTIVE);
                            teleporter->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        }
                    }
            }

            void SetData(uint32 type, uint32 data)
            {
                if (type == DATA_MILLHOUSE_EVENT)
                {
                    if (data == DONE)
                    {
                        if (GameObject * go = instance->GetGameObject(_corborusWallGUID))
                            go->UseDoorOrButton();
                    }
                    else if (data == SPECIAL)
                    {
                        _manastormStatus = true;
                        SaveToDB();
                    }
                }
                else if (!_manastormStatus && type == DATA_CORBORUS_AREATRIGGER && data == DONE)
                {
                    if (Creature * millhouse = instance->GetCreature(_millhouseGUID))
                        millhouse->AI()->DoAction(ACTION_MILLHOUSE_DEMISE);
                }
                else if (!_slabhideTrapStatus && type == DATA_SLABHIDE_AREATRIGGER && data == DONE)
                {
                    _slabhideTrapStatus = true;
                    for (std::vector<uint64>::const_iterator itr = stalactiteTriggers.begin(); itr != stalactiteTriggers.end(); ++itr)
                        if (Creature * stalactite = instance->GetCreature(*itr))
                            stalactite->DespawnOrUnsummon();
                    stalactiteTriggers.clear();
                    if (Creature * slabhide = instance->GetCreature(_slabhideGUID))
                        slabhide->AI()->DoAction(ACTION_SLABHIDE_END_INTRO);
                }
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << "S C " << GetBossSaveData() << _manastormStatus << " ";

                OUT_SAVE_INST_DATA_COMPLETE;
                return saveStream.str();
            }

            void Load(char const* str)
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

                if (dataHead1 == 'S' && dataHead2 == 'C')
                {
                    for (uint8 i = 0; i < TOTAL_ENCOUNTERS; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }
                     loadStream >> _manastormStatus;
                } else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_the_stonecore_InstanceMapScript(map);
        }
};

void AddSC_instance_the_stonecore()
{
    new instance_the_stonecore();
}
