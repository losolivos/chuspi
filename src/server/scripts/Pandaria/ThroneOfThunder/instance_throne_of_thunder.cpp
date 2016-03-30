#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "throne_of_thunder.h"

DoorData const doorData[] =
{
    { GOB_JIN_ROKH_ENTRANCE, DATA_JINROKH, DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GOB_JIN_ROKH_EXIT, DATA_JINROKH, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    { GOB_HORRIDON_ENTRANCE, DATA_HORRIDON, DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GOB_HORRIDON_EXIT, DATA_HORRIDON, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    { GOB_COUNCIL_ENTRANCE1, DATA_COUNCIL_OF_ELDERS, DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GOB_COUNCIL_ENTRANCE2, DATA_COUNCIL_OF_ELDERS, DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GOB_COUNCIL_EXIT, DATA_COUNCIL_OF_ELDERS, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    { GOB_TORTOS_DOOR, DATA_TORTOS, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    { GOB_TORTOS_COLLISION, DATA_TORTOS, DOOR_TYPE_SPAWN_HOLE, BOUNDARY_NONE },
    { GOB_MEGAERA_EXIT, DATA_MEGAERA, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    { GOB_PRIMORDIUS_ENTRANCE, DATA_PRIMORDIUS, DOOR_TYPE_ROOM, BOUNDARY_S},
    { GOB_PRIMORDIUS_EXIT, DATA_PRIMORDIUS, DOOR_TYPE_PASSAGE, BOUNDARY_NONE }
};

typedef std::unordered_map<uint32, uint64> EntryGuidMap;

class instance_throne_of_thunder : public InstanceMapScript
{
public:
    instance_throne_of_thunder() : InstanceMapScript("instance_throne_of_thunder", 1098) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_throne_of_thunder_InstanceScript(map);
    }

    struct instance_throne_of_thunder_InstanceScript : public InstanceScript
    {
        EntryGuidMap m_mNpcGuidStorage;
        EntryGuidMap m_mGoGuidStorage;
        EventMap m_mEvents;

        uint64 horridonHelperGuid;
        uint64 megaeraChestGuid;

        uint32 m_auiEncounter[MAX_TYPES];
        std::string strSaveData;
        std::list<uint64> m_lMoguBellGuids;
        
        typedef std::list<Creature*> golems;
        uint8 golemsCount[2];
        bool golemsSetup;
        golems animusGolems;
        golems largeAnimusGolems;

        instance_throne_of_thunder_InstanceScript(Map* map) : InstanceScript(map) {}

        void Initialize()
        {
            SetBossNumber(MAX_DATA);
            LoadDoorData(doorData);

            golemsCount[0] = 0;
            golemsCount[1] = 0;
            golemsSetup = false;

            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
        }

        void SetupGolems()
        {
            for (uint8 x = 0; x < A_G_BROKEN_MAX;)
            {
                if (Creature* golem = Trinity::Containers::SelectRandomContainerElement(animusGolems))
                {
                    if (!golem->HasAura(SPELL_CRITICALLY_DAMAGED))
                    {
                        golem->AddAura(SPELL_CRITICALLY_DAMAGED, golem);
                        golem->DisableHealthRegen();
                        golem->SetHealth(CalculatePct(golem->GetMaxHealth(), 5.0f));
                        x++;
                    }
                }
            }

            for (uint8 x = 0; x < L_A_G_BROKEN_MAX;)
            {
                if (Creature* golem = Trinity::Containers::SelectRandomContainerElement(largeAnimusGolems))
                {
                    if (!golem->HasAura(SPELL_CRITICALLY_DAMAGED))
                    {
                        golem->AddAura(SPELL_CRITICALLY_DAMAGED, golem);
                        golem->SetHealth(CalculatePct(golem->GetMaxHealth(), 5.0f));
                        x++;
                    }
                }
            }

            golemsSetup = true;
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
                case BOSS_JINROKH:
                case BOSS_HORRIDON:
                case BOSS_COUNCIL_KAZRAJIN:
                case BOSS_COUNCIL_SUL_THE_SANDCRAWLER:
                case BOSS_COUNCIL_FROST_KING_MALAKK:
                case BOSS_COUNCIL_HIGH_PRIESTESS_MARLI:
                case BOSS_TORTOS:   
                case BOSS_MEGAERA:       
                case BOSS_JI_KUN:          
                case BOSS_DURUMU_THE_FORGOTTEN: 
                case BOSS_PRIMORDIUS:         
                case BOSS_DARK_ANIMUS:        
                case BOSS_IRON_QON:           
                case BOSS_LULIN:
                case BOSS_SUEN:
                case BOSS_LEI_SHEN:             
                case BOSS_RA_DEN:              
                case MOB_WAR_GOD_JALAK:                    
                    m_mNpcGuidStorage.insert(std::make_pair(pCreature->GetEntry(), pCreature->GetGUID()));
                    break;
                case NPC_JINROKH_STATUE:
                    pCreature->SetCanFly(true);
                    pCreature->SetHover(true);
                    pCreature->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY);
                    break;
                case MOB_GARA_JAL:
                    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);
                    pCreature->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_STAND_FLAGS_CREEP);
                    m_mNpcGuidStorage.insert(std::make_pair(pCreature->GetEntry(), pCreature->GetGUID()));
                    break;
                case NPC_HORRIDON_EVENT_HELPER:
                    horridonHelperGuid = pCreature->GetGUID();
                    break;
                case NPC_ANIMUS_GOLEM:
                    animusGolems.push_back(pCreature);
                    pCreature->RemoveAurasDueToSpell(SPELL_CRITICALLY_DAMAGED);
                    golemsCount[0]++;
                    break;
                case NPC_LARGE_ANIMA_GOLEM:
                    largeAnimusGolems.push_back(pCreature);
                    pCreature->RemoveAurasDueToSpell(SPELL_CRITICALLY_DAMAGED);
                    golemsCount[1]++;
                    break;
                default:
                    break;
            }

            if (golemsCount[0] == ANIMUS_GOLEMS_COUNT && golemsCount[1] == LARGE_ANIMUS_GOLEMS_COUNT && !golemsSetup)
                SetupGolems();
        }

        void OnGameObjectCreate(GameObject* pGo)
        {
            switch (pGo->GetEntry())
            {
                case GOB_JIN_ROKH_ENTRANCE:
                case GOB_JIN_ROKH_EXIT: 
                case GOB_HORRIDON_ENTRANCE: 
                case GOB_HORRIDON_EXIT:    
                case GOB_COUNCIL_ENTRANCE1:
                case GOB_COUNCIL_ENTRANCE2:
                case GOB_COUNCIL_EXIT:
                case GOB_TORTOS_DOOR:
                case GOB_TORTOS_COLLISION:
                case GOB_MEGAERA_EXIT:
                case GOB_PRIMORDIUS_ENTRANCE:
                case GOB_PRIMORDIUS_EXIT:
                    AddDoor(pGo, true);
                    m_mGoGuidStorage.insert(std::make_pair(pGo->GetEntry(), pGo->GetGUID()));
                    break;
                case GOB_TRIBAL_DOOR_FARRAKI:
                case GOB_TRIBAL_DOOR_GURUBASHI:
                case GOB_TRIBAL_DOOR_DRAKKARI: 
                case GOB_TRIBAL_DOOR_AMANI:
                case GOB_MOGU_STATUE_1:
                case GOB_MOGU_STATUE_2:
                case GOB_MOGU_STATUE_3:
                case GOB_MOGU_STATUE_4:
                case GOB_JIKUN_FEATHER:
                case GOB_JIN_ROKH_PREDOOR:
                    m_mGoGuidStorage.insert(std::make_pair(pGo->GetEntry(), pGo->GetGUID()));
                    break;
                case 218805: // Megaera chests
                case 218806:
                case 218807:
                case 218808:
                    megaeraChestGuid = pGo->GetGUID();
                    break;
                case GOB_MOGU_BELL:
                    m_lMoguBellGuids.push_back(pGo->GetGUID());
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 uiId, EncounterState eState)
        {
            TC_LOG_ERROR("scripts", "SetBossState called type%u, state %u, instance id %u", uiId, (uint32)eState, instance->GetInstanceId());
            if (!InstanceScript::SetBossState(uiId, eState))
                return false;

            if (uiId >= MAX_DATA)
                return false;

            switch (uiId)
            {
                case DATA_JINROKH:
                case DATA_HORRIDON:
                case DATA_COUNCIL_OF_ELDERS:
                case DATA_TORTOS:
                case DATA_MEGAERA:
                case DATA_JI_KUN:
                case DATA_DURUMU_THE_FORGOTTEN:
                case DATA_PRIMORDIUS:
                case DATA_DARK_ANIMUS:
                case DATA_IRON_QON:
                case DATA_TWIN_CONSORTS:
                case DATA_LEI_SHEN:
                    SetData(uiId, (uint32)eState);
                    break;
                default:
                    break;
            }

            return true;
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            // Don't set the same data twice.
            if (m_auiEncounter[uiType] == uiData)
                return;

            switch (uiType)
            {             
                case TYPE_JINROKH_INTRO:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                        SaveInstance();
                    if (Creature* pJinRokh = instance->GetCreature(GetData64(BOSS_JINROKH)))
                    {
                        if (pJinRokh->AI())
                            pJinRokh->AI()->DoAction(ACTION_START_INTRO);
                    }
                    HandleGameObject(GetData64(GOB_JIN_ROKH_PREDOOR), true);
                    break;
                case TYPE_JINROKH:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_JINROKH, instance->GetCreature(GetData64(BOSS_JINROKH)));
                        SaveInstance();
                        HandleGameObject(GetData64(GOB_HORRIDON_PREDOOR), true);
                    }
                    break;
                case TYPE_HORRIDON:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_HORRIDON, instance->GetCreature(GetData64(BOSS_HORRIDON)));
                        SaveInstance();
                    }
                    break;
                case TYPE_TORTOS:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_TORTOS, instance->GetCreature(GetData64(BOSS_TORTOS)));
                        SaveInstance();
                    }
                    break;
                case TYPE_MEGAERA:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_MEGAERA, instance->GetCreature(GetData64(BOSS_MEGAERA)));
                        SaveInstance();
                    }
                    break;
                case TYPE_DURUMU:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_DURUMU_THE_FORGOTTEN, instance->GetCreature(GetData64(BOSS_DURUMU_THE_FORGOTTEN)));
                        SaveInstance();
                    }
                    break;
                case TYPE_JI_KUN:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_JI_KUN, instance->GetCreature(GetData64(BOSS_JI_KUN)));
                        SaveInstance();
                    }
                    break;
                case TYPE_PRIMORDIUS:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_PRIMORDIUS, instance->GetCreature(GetData64(BOSS_PRIMORDIUS)));
                        SaveInstance();
                    }
                    break;
                case TYPE_DARK_ANIMUS:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_DARK_ANIMUS, instance->GetCreature(GetData64(BOSS_DARK_ANIMUS)));
                        SaveInstance();
                    }
                    break;
                case TYPE_IRON_QON:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_IRON_QON, instance->GetCreature(GetData64(BOSS_IRON_QON)));
                        SaveInstance();
                    }
                    break;
                case TYPE_LEI_SHEN:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_LEI_SHEN, instance->GetCreature(GetData64(BOSS_LEI_SHEN)));
                        SaveInstance();
                    }
                    break;
                case TYPE_RA_DEN:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                    {
                        UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, BOSS_RA_DEN, instance->GetCreature(GetData64(BOSS_RA_DEN)));
                        SaveInstance();
                    }
                    break;
                case TYPE_BELLS_RUNG:
                    m_auiEncounter[uiType] = uiData;
                    SaveInstance();
                    break;
                case TYPE_TORTOS_INTRO:
                case TYPE_PRIMORDIUS_INTRO:
                    // Council and Twin Consorts are handled in scripts
                case TYPE_TWIN_CONSORTS:
                case TYPE_COUNCIL:
                    m_auiEncounter[uiType] = uiData;
                    if (uiData >= DONE)
                        SaveInstance();
                    break;
            }
        }

        void SetData64(uint32 uiType, uint64 uiData)
        {
            switch (uiType)
            {
                case MOB_GARA_JALS_SOUL:
                    m_mNpcGuidStorage[MOB_GARA_JALS_SOUL] = uiData;
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 uiType)
        {
            if (uiType >= MAX_TYPES)
            {
                TC_LOG_ERROR("scripts", "ToT instance script requested data > MAX_TYPES, aborting");
                return 0;
            }

            return m_auiEncounter[uiType];
        }

        uint64 GetData64(uint32 uiType)
        {
            switch (uiType)
            {
                // Creatures here
                case BOSS_JINROKH:
                case BOSS_HORRIDON:
                case BOSS_COUNCIL_KAZRAJIN:
                case BOSS_COUNCIL_SUL_THE_SANDCRAWLER:
                case BOSS_COUNCIL_FROST_KING_MALAKK:
                case BOSS_COUNCIL_HIGH_PRIESTESS_MARLI:
                case BOSS_TORTOS:
                case BOSS_MEGAERA:
                case BOSS_JI_KUN:
                case BOSS_DURUMU_THE_FORGOTTEN:
                case BOSS_PRIMORDIUS:
                case BOSS_DARK_ANIMUS:
                case BOSS_IRON_QON: 
                case BOSS_LULIN:
                case BOSS_SUEN:
                case BOSS_LEI_SHEN:
                case BOSS_RA_DEN:
                case MOB_GARA_JAL:          
                case MOB_GARA_JALS_SOUL:      
                case MOB_WAR_GOD_JALAK:
                {
                    EntryGuidMap::const_iterator find = m_mNpcGuidStorage.find(uiType);
                    if (find != m_mNpcGuidStorage.cend())
                        return find->second;
                    return 0;
                }
                case NPC_HORRIDON_EVENT_HELPER:
                    return horridonHelperGuid;
                // Gameobjects below here #####
                // ############################
                // ############################
                case GOB_JIN_ROKH_ENTRANCE:
                case GOB_JIN_ROKH_PREDOOR:
                case GOB_JIN_ROKH_EXIT:
                case GOB_HORRIDON_ENTRANCE:
                case GOB_HORRIDON_EXIT:
                case GOB_COUNCIL_ENTRANCE1:
                case GOB_COUNCIL_ENTRANCE2:
                case GOB_COUNCIL_EXIT:
                case GOB_TORTOS_DOOR:
                case GOB_TORTOS_COLLISION:
                case GOB_MEGAERA_EXIT:
                case GOB_TRIBAL_DOOR_FARRAKI:
                case GOB_TRIBAL_DOOR_GURUBASHI:
                case GOB_TRIBAL_DOOR_DRAKKARI: 
                case GOB_TRIBAL_DOOR_AMANI:
                case GOB_MOGU_STATUE_1:
                case GOB_MOGU_STATUE_2:
                case GOB_MOGU_STATUE_3:
                case GOB_MOGU_STATUE_4:
                case GOB_JIKUN_FEATHER:
                case GOB_PRIMORDIUS_ENTRANCE:
                case GOB_PRIMORDIUS_EXIT:
                {                                          
                    EntryGuidMap::const_iterator find = m_mGoGuidStorage.find(uiType);
                    if (find != m_mGoGuidStorage.cend())
                        return find->second;
                    return 0;
                }
                case GOB_MEGAERA_CHEST:
                    return megaeraChestGuid;
                default:
                    return 0;
            }

            return 0;
        }

        std::string GetSaveData()
        {
            return strSaveData;
        }

        void Update(uint32 uiDiff)
        {
            m_mEvents.Update(uiDiff);

            while (uint32 eventId = m_mEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_JINROKH_DOOR:
                        HandleGameObject(GetData64(GOB_JIN_ROKH_PREDOOR), true);
                        break;
                    case EVENT_MOGU_BELLS:
                        for (std::list<uint64>::const_iterator itr = m_lMoguBellGuids.cbegin(); itr != m_lMoguBellGuids.cend(); ++itr)
                        {                          
                            if (GameObject* pGo = instance->GetGameObject(*itr))
                            {
                                HandleGameObject(0, true, pGo);
                                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                            }
                        }
                        break;
                    case EVENT_PRIMORDIUS_INTRO:
                        if (Creature* pBoss = instance->GetCreature(GetData64(BOSS_PRIMORDIUS)))
                            if (pBoss->AI())
                                pBoss->AI()->DoAction(ACTION_START_INTRO);
                        break;
                }
            }
        }

        void SaveInstance()
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << m_auiEncounter[0] << ' ' << m_auiEncounter[1] << ' ' << m_auiEncounter[2] << ' '
                << m_auiEncounter[3] << ' ' << m_auiEncounter[4] << ' ' << m_auiEncounter[5] << ' ' << m_auiEncounter[6]
                << ' ' << m_auiEncounter[7] << ' ' << m_auiEncounter[8] << ' ' << m_auiEncounter[9] << ' ' << m_auiEncounter[10]
                << ' ' << m_auiEncounter[11] << ' ' << m_auiEncounter[12] << ' ' << m_auiEncounter[13] << ' ' << m_auiEncounter[14] 
                << ' ' << m_auiEncounter[15] << ' ' << m_auiEncounter[16];

            strSaveData = saveStream.str();

            SaveToDB();
            OUT_SAVE_INST_DATA_COMPLETE;

        }

        void Load(char const* chrIn)
        {
            if (!chrIn)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(chrIn);
            std::istringstream loadStream(chrIn);

            loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3] >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6]
                >> m_auiEncounter[7] >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11] >> m_auiEncounter[12] >> m_auiEncounter[13]
                >> m_auiEncounter[14] >> m_auiEncounter[15] >> m_auiEncounter[16];
            for (int i = 0; i < MAX_TYPES; ++i)
            {
                // For storage purposes
                if (i != TYPE_BELLS_RUNG)
                {
                    if (m_auiEncounter[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                        m_auiEncounter[i] = NOT_STARTED;
                }

                if (m_auiEncounter[i] == DONE)
                {
                    if (i < MAX_DATA)
                        SetBossState(i, (EncounterState)DONE);

                    switch (i)
                    {
                    case TYPE_JINROKH_INTRO:
                        m_mEvents.ScheduleEvent(EVENT_JINROKH_DOOR, 100);
                        break;
                    case TYPE_BELLS_RUNG:
                        m_mEvents.ScheduleEvent(EVENT_MOGU_BELLS, 100);
                        break;
                    case TYPE_PRIMORDIUS_INTRO:
                        m_mEvents.ScheduleEvent(EVENT_PRIMORDIUS_INTRO, 500);
                        break;
                    }
                }
            }

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

void AddSC_instance_throne_of_thunder()
{
    new instance_throne_of_thunder();
}