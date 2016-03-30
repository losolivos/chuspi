/*
* Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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
#include "CreatureTextMgr.h"
#include "shadopan_monastery.h"

Position snowdriftCenterPos = {3659.08f, 3015.38f, 804.74f};

DoorData const doorData[] =
{
    {GO_CLOUDSTRIKE_ENTRANCE, DATA_GU_CLOUDSTRIKE,   DOOR_TYPE_ROOM,    BOUNDARY_NONE},
    {GO_CLOUDSTRIKE_EXIT,     DATA_GU_CLOUDSTRIKE,   DOOR_TYPE_PASSAGE, BOUNDARY_NONE},
    {GO_SNOWDRIFT_DOJO_DOOR,  DATA_MASTER_SNOWDRIFT, DOOR_TYPE_ROOM,    BOUNDARY_NONE},
    {GO_SNOWDRIFT_EXIT,       DATA_MASTER_SNOWDRIFT, DOOR_TYPE_PASSAGE, BOUNDARY_NONE},
    {GO_SHA_ENTRANCE,         DATA_SHA_VIOLENCE,     DOOR_TYPE_ROOM,    BOUNDARY_NONE},
    {GO_SHA_EXIT,             DATA_SHA_VIOLENCE,     DOOR_TYPE_PASSAGE, BOUNDARY_NONE},
    {GO_SNOWDRIFT_ENTRANCE,   DATA_SNOWDRIFT_STATE , DOOR_TYPE_ROOM,    BOUNDARY_NONE},
    {GO_SNOWDRIFT_FIRE_WALL,  DATA_SNOWDRIFT_STATE,  DOOR_TYPE_PASSAGE, BOUNDARY_NONE},
    {0,                       0,                     DOOR_TYPE_ROOM,    BOUNDARY_NONE},// END
};

class instance_shadopan_monastery : public InstanceMapScript
{
    public: instance_shadopan_monastery() : InstanceMapScript("instance_shadopan_monastery", 959) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_shadopan_monastery_InstanceMapScript(map);
    }

    struct instance_shadopan_monastery_InstanceMapScript : public InstanceScript
    {
        instance_shadopan_monastery_InstanceMapScript(Map* map) : InstanceScript(map) {}

        std::list<uint64> firstArcherySet;
        std::list<uint64> secondArcherySet;
        std::list<uint64> archeryTargetGuids;
        std::list<uint64> minibossPositionsGuid;
        std::list<uint64> minibossPositionsGuidSave;
        std::list<uint64> firstDefeatedNovicePositionsGuid;
        std::list<uint64> secondDefeatedNovicePositionsGuid;
        std::list<uint64> firstDefeatedNovicePositionsGuidSave;     
        std::list<uint64> secondDefeatedNovicePositionsGuidSave;       

        uint8 aliveNoviceCount;
        uint8 aliveMinibossCount;
        uint32 dataStorage[MAX_DATA];
        uint32 iceArchersState;
        uint32 fireArchersState;
        uint32 snowdriftState;
        uint64 guCloudstikeGUID;
        uint64 masterSnowdriftGUID;
        uint64 shaViolenceGuid;
        uint64 taranZhuGuid;
        uint64 azureSerpentGUID;
        uint64 snowdriftRefereeGUID;
        uint64 taranZhuCache;
        uint64 snowdriftPossesions;
        uint64 taranZhuCacheH;
        uint64 snowdriftPossesionsH;

        void Initialize()
        {
            SetBossNumber(EncounterCount);
            LoadDoorData(doorData);
            memset(dataStorage, 0, MAX_DATA * sizeof(uint32));
            iceArchersState      = NOT_STARTED;
            fireArchersState     = NOT_STARTED;
            snowdriftState       = NOT_STARTED;
            aliveNoviceCount     = MAX_NOVICE;
            aliveMinibossCount   = 2;
            guCloudstikeGUID     = 0;
            masterSnowdriftGUID  = 0;
            shaViolenceGuid      = 0;
            taranZhuGuid         = 0;
            azureSerpentGUID     = 0;
            snowdriftRefereeGUID = 0;
            taranZhuCache        = 0;
            snowdriftPossesions  = 0;
            taranZhuCacheH       = 0;
            snowdriftPossesionsH = 0;
            firstArcherySet.clear();
            secondArcherySet.clear(); 
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch(creature->GetEntry())
            {
                case NPC_GU_CLOUDSTRIKE:    
                    guCloudstikeGUID = creature->GetGUID();
                    break;
                case NPC_MASTER_SNOWDRIFT: 
                    masterSnowdriftGUID = creature->GetGUID();
                    break;
                case NPC_SHA_VIOLENCE:    
                    shaViolenceGuid = creature->GetGUID();   
                    break;
                case NPC_TARAN_ZHU:       
                    taranZhuGuid = creature->GetGUID();     
                    break;
                case NPC_AZURE_SERPENT:   
                    azureSerpentGUID = creature->GetGUID();
                    break;
                case NPC_PANDAREEN_REFEREE:
                    snowdriftRefereeGUID = creature->GetGUID();
                    break;
                case NPC_ARCHERY_TARGET:  
                    archeryTargetGuids.push_back(creature->GetGUID()); 
                    break;
                case NPC_SNOWDRIFT_POSITION:
               {
                   if (creature->GetDistance(snowdriftCenterPos) > 5.0f && creature->GetDistance(snowdriftCenterPos) < 15.0f)
                   {
                      minibossPositionsGuid.push_back(creature->GetGUID());
                      minibossPositionsGuidSave.push_back(creature->GetGUID());
                   }
                   else if (creature->GetDistance(snowdriftCenterPos) > 15.0f  && creature->GetDistance(snowdriftCenterPos) < 18.5f)
                   {
                      firstDefeatedNovicePositionsGuid.push_back(creature->GetGUID());
                      firstDefeatedNovicePositionsGuidSave.push_back(creature->GetGUID());
                   }
                   else if (creature->GetDistance(snowdriftCenterPos) > 15.0f && creature->GetDistance(snowdriftCenterPos) < 25.0f)
                   {
                      secondDefeatedNovicePositionsGuid.push_back(creature->GetGUID());
                      secondDefeatedNovicePositionsGuidSave.push_back(creature->GetGUID());
                   }
               }
               break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch(go->GetEntry())
            {
                case GO_TARAN_ZHU_CACHE:
                    taranZhuCache = go->GetGUID();
                    break;
                case GO_SNOWDRIFT_POSSESSIONS:
                    snowdriftPossesions = go->GetGUID();
                    break;
                case GO_SNOWDRIFT_POSSISIONS_H:
                    snowdriftPossesionsH = go->GetGUID();
                    break;
                case GO_TARAN_ZHU_CACHE_H:
                    taranZhuCacheH = go->GetGUID();
                    break;
                case GO_CLOUDSTRIKE_ENTRANCE:
                case GO_CLOUDSTRIKE_EXIT:
                case GO_SNOWDRIFT_ENTRANCE:
                case GO_SNOWDRIFT_FIRE_WALL:
                case GO_SNOWDRIFT_DOJO_DOOR:
                case GO_SNOWDRIFT_EXIT:
                case GO_SHA_ENTRANCE:
                case GO_SHA_EXIT:
                    AddDoor(go, true);
                    break;
            }
        }

        bool SetBossState(uint32 id, EncounterState state)
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch(id)
            {
                case DATA_MASTER_SNOWDRIFT:
                {
                    if (state == DONE)
                        if (GameObject* cache = instance->GetGameObject(instance->IsHeroic() ? snowdriftPossesionsH : snowdriftPossesions))
                            cache->SetPhaseMask(1, true);

                    break;
                }
                case DATA_TARAN_ZHU:
                {
                    switch(state)
                    {
                         case IN_PROGRESS:
                             DoAddAuraOnPlayers(SPELL_HATE);
                             break;
                         case FAIL:
                         case NOT_STARTED:
                         case SPECIAL:
                             {
                                   Map::PlayerList const &PlayerList = instance->GetPlayers();
                                   if (!PlayerList.isEmpty())
                                       for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                           if (Player* player = i->GetSource())
                                           {
                                                player->RemoveAurasDueToSpell(SPELL_HATE);
                                                player->RemoveAurasDueToSpell(SPELL_HAZE_OF_HATE);
                                                player->RemoveAurasDueToSpell(SPELL_HAZE_OF_HATE_VISUAL);
                                           }
                             }
                             break;
                         case DONE:
                         {
                             Map::PlayerList const &PlayerList = instance->GetPlayers();
                             if (!PlayerList.isEmpty())
                                for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                if (Player* player = i->GetSource())
                                {
                                   player->RemoveAurasDueToSpell(SPELL_HATE);
                                   player->RemoveAurasDueToSpell(SPELL_HAZE_OF_HATE);
                                   player->RemoveAurasDueToSpell(SPELL_HAZE_OF_HATE_VISUAL);
                                }

                              if (GameObject* cache = instance->GetGameObject(instance->IsHeroic() ? taranZhuCacheH: taranZhuCache))
                                  cache->SetPhaseMask(1, true);
                         }
                         break;
                    }
                    break;
                }
            }
            return true;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch(type)
            {
                case DATA_DEFEATED_NOVICE:
                {
                    if (!--aliveNoviceCount)
                       if (Creature* referee = instance->GetCreature(snowdriftRefereeGUID))
                          if (referee->IsAIEnabled)
                              referee->AI()->DoAction(1); // ACTION_NOVICE_DONE
                    break;
                }
                case DATA_DEFEATED_MINIBOSS:
                {
                    if (!--aliveMinibossCount)
                    {
                        if (Creature* referee = instance->GetCreature(snowdriftRefereeGUID))
                           if (referee->IsAIEnabled)
                               referee->AI()->DoAction(2); // ACTION_MINIBOSS_DONE
                    }
                    break;
                }
                case DATA_ARCHERY:
                {
                    iceArchersState = data;
                    switch(data)
                    {
                        case IN_PROGRESS:
                        {
                            for(int i = 0; i < 3; i++)
                            {
                                if (Creature* archer = instance->SummonCreature(NPC_ARCHERY_FIRST, IceArchersPosition[i]))
                                    firstArcherySet.push_back(archer->GetGUID());
                            }

                            if (Creature* archer = instance->SummonCreature(NPC_ARCHERY_FIRST, IceArchersPosition[2]))
                            {
                                firstArcherySet.push_back(archer->GetGUID());
                                sCreatureTextMgr->SendChat(archer, 0, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
                            }
                            break;
                        }
                        case DONE:
                        {
                            if (Creature* archer = instance->GetCreature(firstArcherySet.back()))
                                sCreatureTextMgr->SendChat(archer, 1, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                            for(auto itr : firstArcherySet)
                            {
                                if (auto const archer = instance->GetCreature(itr))
                                    archer->DespawnOrUnsummon();
                            }
                            break;
                        }
                    }
                    break;
                }
                case DATA_FIRE_ARCHERY:
                {
                    fireArchersState = data;
                    switch(data)
                    {
                        case IN_PROGRESS:
                        {
                            for(int i = 0; i < 3; i++)
                            {
                                if (Creature* archer = instance->SummonCreature(NPC_ARCHERY_SECOND, FireArchersPosition[i]))
                                    secondArcherySet.push_back(archer->GetGUID());
                            }

                            if (Creature* archer = instance->SummonCreature(NPC_ARCHERY_SECOND, FireArchersPosition[2]))
                            {
                                secondArcherySet.push_back(archer->GetGUID());
                                sCreatureTextMgr->SendChat(archer, 0, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
                            }
                            break;
                        }
                        case DONE:
                        {
                            if (Creature* archer = instance->GetCreature(secondArcherySet.back()))
                                sCreatureTextMgr->SendChat(archer, 1, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                            for(auto itr : secondArcherySet)
                            {
                                if (auto const archer = instance->GetCreature(itr))
                                    archer->DespawnOrUnsummon();
                            }
                            break;
                        }
                    }
                    break;
                }
                case DATA_SNOWDRIFT_STATE:
                    snowdriftState = data;
                    switch(data)
                    {
                        case IN_PROGRESS:
                        case FAIL:
                            aliveNoviceCount = MAX_NOVICE;
                            aliveMinibossCount = 2;
                            minibossPositionsGuid = minibossPositionsGuidSave;
                            firstDefeatedNovicePositionsGuid = firstDefeatedNovicePositionsGuidSave;
                            secondDefeatedNovicePositionsGuid = secondDefeatedNovicePositionsGuidSave;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    if (type < MAX_DATA)
                        dataStorage[type] = data;
                    break;
            }
        }

        uint32 GetData(uint32 type)
        {
            switch(type)
            {
                case DATA_ARCHERY:
                    return iceArchersState;
                case DATA_SNOWDRIFT_STATE:
                    return snowdriftState;
                case DATA_FIRE_ARCHERY:
                    return fireArchersState;
                default:
                {
                    if (type < MAX_DATA)
                        return dataStorage[type];
             
                }break;
            }

            return 0;
        }

        uint64 GetData64(uint32 type)
        {
            switch(type)
            {
                case DATA_GU_CLOUDSTRIKE:        
                    return guCloudstikeGUID;
                case DATA_MASTER_SNOWDRIFT:
                    return masterSnowdriftGUID;
                case NPC_SHA_VIOLENCE:          
                    return shaViolenceGuid;
                case NPC_TARAN_ZHU:             
                    return taranZhuGuid;
                case DATA_AZURE_SERPENT:         
                    return azureSerpentGUID;
                case DATA_POSSESSIONS:
                    return instance->IsHeroic() ? snowdriftPossesionsH : snowdriftPossesions;
                case DATA_TARAN_ZHU_CACHE:
                    return instance->IsHeroic() ? taranZhuCacheH : taranZhuCache;
                case DATA_PANDAREN_REFEREE:
                    return snowdriftRefereeGUID;
                case DATA_ARCHERY_TARGET:
                    return Trinity::Containers::SelectRandomContainerElement(archeryTargetGuids);
                case DATA_RANDOM_FIRST_POS:
                {
                    if (firstDefeatedNovicePositionsGuid.empty())
                        return 0;

                    if (uint64 guid = Trinity::Containers::SelectRandomContainerElement(firstDefeatedNovicePositionsGuid))
                    {
                        firstDefeatedNovicePositionsGuid.remove(guid);
                        return guid;
                    }
                    break;
                }
                
                case DATA_RANDOM_SECOND_POS:
                {
                     if (secondDefeatedNovicePositionsGuid.empty())
                        return 0;

                     if (uint64 guid = Trinity::Containers::SelectRandomContainerElement(secondDefeatedNovicePositionsGuid))
                     {
                         secondDefeatedNovicePositionsGuid.remove(guid);
                         return guid;
                     }
                     break;
                }
                case DATA_RANDOM_MINIBOSS_POS:
                {
                    if (minibossPositionsGuid.empty())
                       return 0;

                    if (uint64 guid = Trinity::Containers::SelectRandomContainerElement(minibossPositionsGuid))
                    {
                        minibossPositionsGuid.remove(guid);
                        return guid;
                    }
                    break;
                }
            }

            return 0;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "S P M " << GetBossSaveData() << iceArchersState << " " << fireArchersState;

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

            if (dataHead1 == 'S' && dataHead2 == 'P' && dataHead3 == 'M')
            {
                for(uint8 i = 0; i < EncounterCount; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;

                    SetBossState(i, EncounterState(tmpState));
                }

                uint32 temp = 0;
                loadStream >> temp;
                iceArchersState = temp ? DONE : NOT_STARTED;

                loadStream >> temp;
                fireArchersState = temp ? DONE : NOT_STARTED;
            }
            else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

void AddSC_instance_shadopan_monastery()
{
    new instance_shadopan_monastery();
}
