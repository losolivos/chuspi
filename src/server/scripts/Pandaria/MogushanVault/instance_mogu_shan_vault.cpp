/*
 * Copyright (C) 2008-20XX Trinity <http://www.pandashan.com>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "VMapFactory.h"
#include "mogu_shan_vault.h"

DoorData const doorData[] =
{
    {GOB_STONE_GUARD_DOOR_ENTRANCE,          DATA_STONE_GUARD,          DOOR_TYPE_ROOM,       BOUNDARY_E   },
    {GOB_STONE_GUARD_DOOR_EXIT,              DATA_STONE_GUARD,          DOOR_TYPE_PASSAGE,    BOUNDARY_W   },
    {GOB_FENG_DOOR_FENCE,                    DATA_FENG,                 DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GOB_FENG_DOOR_EXIT,                     DATA_FENG,                 DOOR_TYPE_PASSAGE,    BOUNDARY_N   },
    {GOB_GARAJAL_FENCE,                      DATA_GARAJAL,              DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GOB_GARAJAL_EXIT,                       DATA_GARAJAL,              DOOR_TYPE_PASSAGE,    BOUNDARY_W   },
    {GOB_SPIRIT_KINGS_WIND_WALL,             DATA_SPIRIT_KINGS,         DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GOB_SPIRIT_KINGS_EXIT,                  DATA_SPIRIT_KINGS,         DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GOB_CELESTIAL_DOOR,                     DATA_ELEGON,               DOOR_TYPE_ROOM,       BOUNDARY_E   },
    {GOB_ELEGON_DOOR_ENTRANCE,               DATA_SPIRIT_KINGS,         DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GOB_ELEGON_CELESTIAL_DOOR,              DATA_ELEGON,               DOOR_TYPE_ROOM,       BOUNDARY_E   },
    {GOB_WILL_OF_EMPEROR_ENTRANCE,           DATA_ELEGON,               DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GOB_TERRACOTTA_DOOR,                    DATA_WILL_OF_EMPEROR,      DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {0,                                      0,                         DOOR_TYPE_ROOM,       BOUNDARY_NONE},// END
};

#define DIST_BETWEEN_TWO_Z      32.39f
#define ACHIEVEMENT_SHOWMOVES   6455

class instance_mogu_shan_vault : public InstanceMapScript
{
public:
    instance_mogu_shan_vault() : InstanceMapScript("instance_mogu_shan_vault", 1008) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_mogu_shan_vault_InstanceMapScript(map);
    }

    struct instance_mogu_shan_vault_InstanceMapScript : public InstanceScript
    {
        instance_mogu_shan_vault_InstanceMapScript(Map* map) : InstanceScript(map) {}

        EventMap events;

        uint32 stoneGuardiansState;
        uint32 actualPetrifierEntry;

        uint64 cursedMogu1Guid;
        uint64 cursedMogu2Guid;
        uint64 ghostEssenceGuid;

        uint64 stoneGuardControllerGuid;
        uint64 fengGuid;
        uint64 siphonShieldGuid;
        uint64 spiritKingsControllerGuid;
        uint64 elegonGuid;
        uint64 infiniteEnergyGuid;

        uint64 inversionGobGuid;
        uint64 stoneGuardExit;
        uint64 cancelGobGuid;
        uint64 ancientMoguDoorGuid;
        uint64 emperorsDoorGuid;
        uint64 celestialCommandGuid;

        uint64 energyPlatformGuid;
        uint64 titanDiskGuid;
        uint64 janxiGuid;
        uint64 qinxiGuid;
        uint64 ancienControlCondoleGUID;

        std::vector<uint64> stoneGuardGUIDs;
        std::vector<uint64> fengStatuesGUIDs;
        std::vector<uint64> spiritKingsGUIDs;
        std::vector<uint64> titanCirclesGuids;
        std::vector<uint32> achievementGuids;

        void Initialize()
        {
            SetBossNumber(ENCOUNTERS);
            LoadDoorData(doorData);
            stoneGuardiansState             = NOT_STARTED;
            stoneGuardControllerGuid        = 0;
            fengGuid                        = 0;
            siphonShieldGuid                = 0;
            inversionGobGuid                = 0;
            cancelGobGuid                   = 0;
            ancientMoguDoorGuid             = 0;
            emperorsDoorGuid                = 0;
            celestialCommandGuid            = 0;
            energyPlatformGuid              = 0;
            titanDiskGuid                   = 0;
            cursedMogu1Guid                 = 0;
            cursedMogu2Guid                 = 0;
            ghostEssenceGuid                = 0;
            inversionGobGuid                = 0;
            cancelGobGuid                   = 0;
            spiritKingsControllerGuid       = 0;
            qinxiGuid                       = 0;
            janxiGuid                       = 0;
            ancienControlCondoleGUID        = 0;

            stoneGuardGUIDs.clear();
            fengStatuesGUIDs.clear();
            spiritKingsGUIDs.clear();
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case NPC_STONE_GUARD_CONTROLLER:
                    stoneGuardControllerGuid = creature->GetGUID();
                    break;
                case NPC_JASPER:
                case NPC_JADE:
                case NPC_AMETHYST:
                case NPC_COBALT:
                {
                    stoneGuardGUIDs.push_back(creature->GetGUID());

                    auto const difficulty = instance->GetSpawnMode();
                    auto const turnOver = difficulty == MAN10_DIFFICULTY || difficulty == MAN10_HEROIC_DIFFICULTY || difficulty == RAID_TOOL_DIFFICULTY;

                    if (stoneGuardGUIDs.size() >= 4 && GetData(DATA_STONE_GUARD_STATE) == NOT_STARTED && GetBossState(DATA_STONE_GUARD) != DONE && turnOver)
                    {
                        SetData(DATA_STONE_GUARD_STATE, DONE);

                        std::random_shuffle(stoneGuardGUIDs.begin(), stoneGuardGUIDs.end());

                        uint64 const toDespawn = stoneGuardGUIDs.back();

                        Creature* stoneGuard = (toDespawn != creature->GetGUID())
                                ? instance->GetCreature(toDespawn)
                                : creature;

                        if (stoneGuard)
                        {
                            stoneGuard->DespawnOrUnsummon();
                            stoneGuardGUIDs.pop_back();
                        }
                    }
                    break;
                }
                case NPC_CURSED_MOGU_SCULPTURE_2:
                    if (!cursedMogu1Guid)
                        cursedMogu1Guid = creature->GetGUID();
                    else
                        cursedMogu2Guid = creature->GetGUID();
                    break;
                case NPC_GHOST_ESSENCE:
                    ghostEssenceGuid = creature->GetGUID();
                    break;
                case NPC_FENG:
                    fengGuid = creature->GetGUID();
                    break;
                case NPC_SIPHONING_SHIELD:
                    siphonShieldGuid = creature->GetGUID();
                    break;
                case NPC_SPIRIT_GUID_CONTROLLER:
                    spiritKingsControllerGuid = creature->GetGUID();
                    break;
                case NPC_ZIAN:
                case NPC_MENG:
                case NPC_QIANG:
                case NPC_SUBETAI:
                    spiritKingsGUIDs.push_back(creature->GetGUID());
                    break;
                case NPC_ELEGON:
                    elegonGuid = creature->GetGUID();
                    break;
                case NPC_INFINITE_ENERGY:
                    infiniteEnergyGuid = creature->GetGUID();
                    break;
                case NPC_QIN_XI:
                    qinxiGuid = creature->GetGUID();
                    break;
                case NPC_JAN_XI:
                    janxiGuid = creature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GOB_STONE_GUARD_DOOR_ENTRANCE:
                case GOB_FENG_DOOR_FENCE:
                case GOB_FENG_DOOR_EXIT:
                case GOB_GARAJAL_FENCE:
                case GOB_GARAJAL_EXIT:
                case GOB_SPIRIT_KINGS_WIND_WALL:
                case GOB_SPIRIT_KINGS_EXIT:
                case GOB_CELESTIAL_DOOR:
                case GOB_TERRACOTTA_DOOR:
                    AddDoor(go, true);
                    break;
                    // Feng
                case GOB_SPEAR_STATUE:
                case GOB_FIST_STATUE:
                case GOB_STAFF_STATUE:
                    fengStatuesGUIDs.push_back(go->GetGUID());
                    break;
                case GOB_SHIELD_STATUE:
                {
                    if (!instance->IsHeroic())
                        go->SetObjectScale(0.001f);
                    else
                        fengStatuesGUIDs.push_back(go->GetGUID());
                    break;
                }
                case GOB_STONE_GUARD_DOOR_EXIT:
                    AddDoor(go, true);
                    stoneGuardExit = go->GetGUID();
                    break;
                case GOB_INVERSION:
                    inversionGobGuid = go->GetGUID();
                    break;
                case GOB_CANCEL:
                    cancelGobGuid = go->GetGUID();
                    break;
                case GOB_ENERGY_PLATFORM:
                    energyPlatformGuid = go->GetGUID();
                    go->SetGoState(GO_STATE_ACTIVE);
                    break;
                case GOB_ELEGON_DOOR_ENTRANCE:
                    ancientMoguDoorGuid = go->GetGUID();
                    break;
                case GOB_WILL_OF_EMPEROR_ENTRANCE:
                    emperorsDoorGuid = go->GetGUID();
                    AddDoor(go, true);
                    break;
                case GOB_ENERGY_TITAN_DISK:
                    titanDiskGuid = go->GetGUID();
                    break;
                case GOB_ENERGY_TITAN_CIRCLE_1:
                case GOB_ENERGY_TITAN_CIRCLE_2:
                case GOB_ENERGY_TITAN_CIRCLE_3:
                    go->SetGoState(GO_STATE_ACTIVE);
                    titanCirclesGuids.push_back(go->GetGUID());
                    break;
                case GOB_CELESTIAL_COMMAND:
                    celestialCommandGuid = go->GetGUID();
                    break;
                case GOB_ANCIEN_CONTROL_CONSOLE:
                    ancienControlCondoleGUID = go->GetGUID();
                    break;
            }
        }

        void OnGameObjectRemove(GameObject *go) final
        {
            switch(go->GetEntry())
            {
                case GOB_STONE_GUARD_DOOR_ENTRANCE:
                case GOB_FENG_DOOR_FENCE:
                case GOB_FENG_DOOR_EXIT:
                case GOB_GARAJAL_FENCE:
                case GOB_GARAJAL_EXIT:
                case GOB_SPIRIT_KINGS_WIND_WALL:
                case GOB_SPIRIT_KINGS_EXIT:
                case GOB_CELESTIAL_DOOR:
                case GOB_STONE_GUARD_DOOR_EXIT:
                case GOB_TERRACOTTA_DOOR:
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
                case DATA_STONE_GUARD:
                {
                    switch (state)
                    {
                        case IN_PROGRESS:
                            if (Creature* stoneGuardController = instance->GetCreature(stoneGuardControllerGuid))
                                stoneGuardController->AI()->DoAction(ACTION_ENTER_COMBAT);

                            for (auto stoneGuardGuid : stoneGuardGUIDs)
                                if (Creature* stoneGuard = instance->GetCreature(stoneGuardGuid))
                                    stoneGuard->AI()->DoAction(ACTION_ENTER_COMBAT);
                                break;
                        case DONE:
                            if (Creature* stoneGuardController = instance->GetCreature(stoneGuardControllerGuid))
                                stoneGuardController->CastSpell(stoneGuardController, ACHIEVEMENT_STONE_GUARD_KILL, true);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case DATA_SPIRIT_KINGS:
                {
                    switch (state)
                    {
                        case IN_PROGRESS:
                        {
                            if (Creature* spiritKingsController = instance->GetCreature(spiritKingsControllerGuid))
                                spiritKingsController->AI()->DoAction(ACTION_ENTER_COMBAT);
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
                case DATA_ELEGON:
                {
                    switch (state)
                    {
                        case TO_BE_DECIDED:
                        {
                            if (GameObject* titanDisk = instance->GetGameObject(titanDiskGuid))
                                titanDisk->SetGoState(GO_STATE_ACTIVE);

                            if (GameObject* energyPlatform = instance->GetGameObject(energyPlatformGuid))
                                energyPlatform->SetGoState(GO_STATE_ACTIVE);

                            for (auto guid: titanCirclesGuids)
                                if (GameObject* titanCircles = instance->GetGameObject(guid))
                                    titanCircles->SetGoState(GO_STATE_ACTIVE);

                            break;
                        }
                        case FAIL:
                        {
                            if (GameObject* titanDisk = instance->GetGameObject(titanDiskGuid))
                                titanDisk->SetGoState(GO_STATE_READY);

                            if (GameObject* energyPlatform = instance->GetGameObject(energyPlatformGuid))
                                energyPlatform->SetGoState(GO_STATE_READY);

                            for (auto guid: titanCirclesGuids)
                                if (GameObject* titanCircles = instance->GetGameObject(guid))
                                    titanCircles->SetGoState(GO_STATE_READY);

                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
                case DATA_WILL_OF_EMPEROR:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                        case FAIL:
                            events.ScheduleEvent(EVENT_RESET_WOE_CONSOLE, 35 * IN_MILLISECONDS);
                            break;
                        default:
                            break;
                    }
                }
            }

            return true;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case ACHIEVEMENT_SHOWMOVES:
                    SetAchievementValid(ACHIEVEMENT_SHOWMOVES);
                    break;
                case DATA_STONE_GUARD_STATE:
                    stoneGuardiansState = data;
                    break;
            }

            if (data == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 type)
        {
            switch (type)
            {
                case ACHIEVEMENT_SHOWMOVES:
                    return IsAchievementValid(ACHIEVEMENT_SHOWMOVES);
                case DATA_STONE_GUARD_STATE:
                    return stoneGuardiansState;
            }

            return 0;
        }

        uint64 GetData64(uint32 type)
        {
            switch (type)
            {
                // Creature
                // Stone Guard
                case NPC_STONE_GUARD_CONTROLLER:
                    return stoneGuardControllerGuid;
                case NPC_CURSED_MOGU_SCULPTURE_1:
                    return cursedMogu1Guid;
                case NPC_CURSED_MOGU_SCULPTURE_2:
                    return cursedMogu2Guid;
                case NPC_GHOST_ESSENCE:
                    return ghostEssenceGuid;
                case NPC_JASPER:
                case NPC_JADE:
                case NPC_AMETHYST:
                case NPC_COBALT:
                {
                    for (auto guid: stoneGuardGUIDs)
                        if (Creature* stoneGuard = instance->GetCreature(guid))
                            if (stoneGuard->GetEntry() == type)
                                return guid;
                            break;
                }
                // Feng
                case NPC_FENG:
                    return fengGuid;
                    // SiphonShield
                case NPC_SIPHONING_SHIELD:
                    return siphonShieldGuid;
                    // Spirit Kings
                case NPC_SPIRIT_GUID_CONTROLLER:
                    return spiritKingsControllerGuid;
                case NPC_ZIAN:
                case NPC_MENG:
                case NPC_QIANG:
                case NPC_SUBETAI:
                {
                    for (auto guid: spiritKingsGUIDs)
                        if (Creature* spiritKing = instance->GetCreature(guid))
                            if (spiritKing->GetEntry() == type)
                                return guid;
                            break;
                }
                // Elegon
                case NPC_ELEGON:
                    return elegonGuid;
                case NPC_INFINITE_ENERGY:
                    return infiniteEnergyGuid;
                    // Will of Emperor
                case NPC_QIN_XI:
                    return qinxiGuid;
                case NPC_JAN_XI:
                    return janxiGuid;
                    // Gameobject
                case GOB_SPEAR_STATUE:
                case GOB_FIST_STATUE:
                case GOB_SHIELD_STATUE:
                case GOB_STAFF_STATUE:
                {
                    for (auto guid: fengStatuesGUIDs)
                        if (GameObject* fengStatue = instance->GetGameObject(guid))
                            if (fengStatue->GetEntry() == type)
                                return guid;
                            break;
                }
                case GOB_STONE_GUARD_DOOR_EXIT:
                    return stoneGuardExit;
                case GOB_INVERSION:
                    return inversionGobGuid;
                case GOB_CANCEL:
                    return cancelGobGuid;
                case GOB_ENERGY_PLATFORM:
                    return energyPlatformGuid;
                case GOB_ENERGY_TITAN_DISK:
                    return titanDiskGuid;
                case GOB_ELEGON_DOOR_ENTRANCE:
                    return ancientMoguDoorGuid;
                case GOB_WILL_OF_EMPEROR_ENTRANCE:
                    return emperorsDoorGuid;
                case GOB_CELESTIAL_COMMAND:
                    return celestialCommandGuid;
                case GOB_ANCIEN_CONTROL_CONSOLE:
                    return ancienControlCondoleGUID;
            }

            return 0;
        }

        bool IsWipe()
        {
            Map::PlayerList const &playerList = instance->GetPlayers();

            for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
                Player* player = itr->GetSource();
                if (!player)
                    continue;

                if (player->IsAlive() && !player->isGameMaster() && !player->HasAura(115877)) // Aura 115877 = Totaly Petrified
                    return false;
            }

            return true;
        }

        void Update(uint32 diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_RESET_WOE_CONSOLE)
            {
                if (GameObject* console = instance->GetGameObject(ancienControlCondoleGUID))
                {
                    console->SetGoState(GO_STATE_READY);
                    console->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE | GO_FLAG_IN_USE);
                }
            }
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "M S V " << GetBossSaveData() << stoneGuardiansState;

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

            char dataHead1, dataHead2, dataHead3;

            std::istringstream loadStream(str);
            loadStream >> dataHead1 >> dataHead2 >> dataHead3;

            if (dataHead1 == 'M' && dataHead2 == 'S' && dataHead3 == 'V')
            {
                for (uint8 i = 0; i < ENCOUNTERS; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;

                    SetBossState(i, EncounterState(tmpState));
                }

                uint32 temp = 0;
                loadStream >> temp;
                stoneGuardiansState = temp ? DONE : NOT_STARTED;
            }
            else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        bool CheckRequiredBosses(uint32 bossId, Player const* player = NULL) const
        {
            if (!InstanceScript::CheckRequiredBosses(bossId, player))
                return false;

            switch (bossId)
            {
                case DATA_WILL_OF_EMPEROR:
                case DATA_ELEGON:
                case DATA_SPIRIT_KINGS:
                case DATA_GARAJAL:
                case DATA_FENG:
                    if (GetBossState(bossId - 1) != DONE)
                        return false;
            }

            return true;
        }

        bool IsAchievementValid(uint32 id) const
        {
            if (achievementGuids[id])
                return true;

            return false;
        }

        void SetAchievementValid(uint32 id)
        {
            if (achievementGuids[id])
                return;

            achievementGuids.push_back(id);
            return;
        }
    };
};

void AddSC_instance_mogu_shan_vault()
{
    new instance_mogu_shan_vault();
}
