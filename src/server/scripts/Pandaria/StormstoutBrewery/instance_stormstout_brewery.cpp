/*
    Dungeon : Stormstout Brewery 85-87
    Instance General Script
*/

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "stormstout_brewery.h"

//sudsy brew wall of suds
//fizzy brew carbonation
//blackout brew blackout
//bloating brew bloat
//yeasty brew yeasty adds
//bubbling brew bubble shield

struct AddSpellPair
{
    uint32 uiEntry_1;
    uint32 uiEntry_2;
    uint32 uiSpell_1;
    uint32 uiSpell_2;
};

static const AddSpellPair aYanzhuPairs[3] =
{
    { NPC_YEASTY_ALEMENTAL, NPC_BUBBLING_ALEMENTAL, SPELL_YEASTY_BREW, SPELL_BUBBLING_BREW},
    { NPC_FIZZY_ALEMENTAL, NPC_SUDSY_ALEMENTAL, SPELL_FIZZY_BREW, SPELL_SUDSY_BREW},
    { NPC_BLOATED_ALEMENTAL, NPC_STOUT_ALEMENTAL, SPELL_BLOATING_BREW, SPELL_BLACKOUT_BREW}
};

class instance_stormstout_brewery : public InstanceMapScript
{
public:
    instance_stormstout_brewery() : InstanceMapScript("instance_stormstout_brewery", 961) { }

    InstanceScript* GetInstanceScript(InstanceMap* pMap) const
    {
        return new instance_stormstout_brewery_InstanceMapScript(pMap);
    }

    struct instance_stormstout_brewery_InstanceMapScript : public InstanceScript
    {
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string strSaveData;

        EntryGuidMap m_mNpcGuidStorage;
        EntryGuidMap m_mGoGuidStorage;
        EventMap m_mEvents;

        std::unordered_map<uint32, uint32> m_mYanzhuAuraMap;
        std::vector<uint64> m_vHozenGuids;
        std::vector<uint64> m_vBouncerGuids;

        uint32 m_uiHozenSlain;

        instance_stormstout_brewery_InstanceMapScript(Map* pMap) : InstanceScript(pMap)
        {
        }

        void Initialize()
        {
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

            InitializeYanzhuAdds(2);
            SetBossNumber(MAX_ENCOUNTER);

            m_uiHozenSlain = GetData(DATA_HOZEN_SLAIN);

            m_mEvents.ScheduleEvent(1, 10000);
            m_mEvents.ScheduleEvent(2, 10000);
        }

        bool InitializeYanzhuAdds(int n)
        {
            // Can never be higher than 2.
            if (n < 0)
                return false;

            uint32 m_uiAdd = urand(0, 1);
            SetData(n + 3, m_uiAdd);

            m_mYanzhuAuraMap.insert(std::make_pair(aYanzhuPairs[n].uiEntry_1, aYanzhuPairs[n].uiSpell_1));
            m_mYanzhuAuraMap.insert(std::make_pair(aYanzhuPairs[n].uiEntry_2, aYanzhuPairs[n].uiSpell_2));

            //sLog->outError(LOG_FILTER_TSCR, "Add entry #%i to be summoned = %u", n, GetAddToSummonEntry(n + 3));

            return InitializeYanzhuAdds(n - 1);
        }

        uint32 GetAddToSummonEntry(uint32 uiType)
        {
            return GetData(uiType) ? aYanzhuPairs[uiType - 3].uiEntry_2 : aYanzhuPairs[uiType - 3].uiEntry_1;
        }

        uint32 GetAffectedSpellToAdd(uint32 uiType)
        {
            std::unordered_map<uint32, uint32>::iterator find = m_mYanzhuAuraMap.find(GetAddToSummonEntry(uiType));
            if (find != m_mYanzhuAuraMap.cend())
                return find->second;

            return 0;
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
            case NPC_HOPTALLUS:
            case NPC_UNCLE_GAO:
            case NPC_OOK_OOK:
                pCreature->setActive(true);
                m_mNpcGuidStorage.insert(std::make_pair(pCreature->GetEntry(), pCreature->GetGUID()));      
                break;
            case NPC_CHEN_YANZHU:
                m_mNpcGuidStorage.insert(std::make_pair(pCreature->GetEntry(), pCreature->GetGUID()));
                break;
            case NPC_YAN_ZHU:
                // Sudsy brew currently disabled until targeting type is fixed
                for (int i = DATA_SMALL_ADDS; i < DATA_LARGE_ADDS + 1; ++i)
                    pCreature->AddAura(GetAffectedSpellToAdd(i) == SPELL_SUDSY_BREW ? SPELL_FIZZY_BREW : GetAffectedSpellToAdd(i), pCreature);
                m_mNpcGuidStorage.insert(std::make_pair(pCreature->GetEntry(), pCreature->GetGUID()));
                break;
            case NPC_HOZEN_CLINGER:
                pCreature->SetCanFly(true);
                break;
            case NPC_HOZEN_PARTY_ANIMAL:
            case NPC_HOZEN_PARTY_ANIMAL2:
            case NPC_HOZEN_PARTY_ANIMAL3:
                m_vHozenGuids.push_back(pCreature->GetGUID());
                break;
            case NPC_HOZEN_BOUNCER:
                m_vBouncerGuids.push_back(pCreature->GetGUID());
                break;
            case NPC_PURPOSE_BUNNY_GROUND:
                pCreature->RemoveAurasDueToSpell(128571);
                pCreature->AddAura(114380, pCreature);
                Creature* pClose = GetClosestCreatureWithEntry(pCreature, NPC_PURPOSE_BUNNY_FLYING, 20.f);
                if (pClose)
                    pCreature->CastSpell(pClose, SPELL_GUSHING_BREW, true);
                break;
            }
        }

        void OnUnitDeath(Unit* pUnit)
        {
            if (pUnit->ToCreature())
            {
                switch (pUnit->GetEntry())
                {
                case NPC_DRUNKEN_HOZEN_BRAWLER:
                case NPC_INFLAMED_HOZEN_BRAWLER:
                case NPC_SLEEPY_HOZEN_BRAWLER:
                case NPC_SODDEN_HOZEN_BRAWLER:
                case NPC_HOZEN_PARTY_ANIMAL:
                case NPC_HOZEN_PARTY_ANIMAL2:
                case NPC_HOZEN_PARTY_ANIMAL3:
                    ++m_uiHozenSlain;
                    break;
                }
            }
        }


        void OnGameObjectCreate(GameObject* pGo)
        {
            switch (pGo->GetEntry())
            {
            case GO_BREWERY_DOOR:
            case GO_INVIS_DOOR:
                m_mGoGuidStorage.insert(std::make_pair(pGo->GetEntry(), pGo->GetGUID()));
                break;
            case GO_OOK_DOOR:
                m_mGoGuidStorage[GO_OOK_DOOR] = pGo->GetGUID();

                if (GetData(DATA_OOK_OOK) == DONE)
                    pGo->AddObjectToRemoveList();
                break;
            case GO_SLIDING_DOOR:
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                break;
            }
        }

        void HandleOok()
        {

            if (m_uiHozenSlain > GetData(DATA_HOZEN_SLAIN))
                SetData(DATA_HOZEN_SLAIN, m_uiHozenSlain);

            if (m_uiHozenSlain >= 40 && GetData(DATA_OOK_OOK) <= SPECIAL)
            {
                m_mEvents.CancelEvent(1);

                if (Creature* pOok = instance->GetCreature(GetData64(NPC_OOK_OOK)))
                {
                    if (pOok->AI())
                    {
                        if (GetData(DATA_OOK_OOK) != DONE)
                        {
                            SetData(DATA_OOK_OOK, SPECIAL);
                            pOok->AI()->DoAction(0);

                            for (auto itr : m_vHozenGuids)
                            {
                                if (Creature* pCreature = instance->GetCreature(itr))
                                {
                                    if (pCreature->AI() && pCreature->IsAlive())
                                        pCreature->AI()->DoAction(0);
                                }
                            }
                        }
                    }
                }
            }
        }

        void HandleBouncers()
        {
            if (GetData(DATA_OOK_OOK) == DONE)
            {
                m_mEvents.CancelEvent(2);

                int32 m_number = 0;
                for (auto itr : m_vBouncerGuids)
                {
                    if (Creature* pCreature = instance->GetCreature(itr))
                    {
                        if (pCreature->AI())
                            pCreature->AI()->DoAction(m_number);

                        ++m_number;
                    }
                }
            }
        }
    
        void Update(uint32 uiDiff)
        {
            m_mEvents.Update(uiDiff);

            while (uint32 eventId = m_mEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                case 1:
                    m_mEvents.ScheduleEvent(1, 7000);
                    HandleOok();
                    break;
                case 2:
                    m_mEvents.ScheduleEvent(2, 3000);
                    HandleBouncers();
                    break;
                }
            }
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            if (uiType >= MAX_ENCOUNTER)
                return;

            switch (uiData)
            {
            case DATA_YAN_ZHU:
            case DATA_OOK_OOK:
            case DATA_HOZEN_SLAIN:
            case DATA_HOPTALLUS:
                break;
            }

            m_auiEncounter[uiType] = uiData;

            if (uiData >= DONE)
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << m_auiEncounter[0] << ' ' << m_auiEncounter[1] << ' ' << m_auiEncounter[2] << ' '
                    << m_auiEncounter[3] << ' ' << m_auiEncounter[4] << ' ' << m_auiEncounter[5] << ' ' << m_auiEncounter[6];

                strSaveData = saveStream.str();

                SaveToDB();
                OUT_SAVE_INST_DATA_COMPLETE;
            }
        }

        std::string GetSaveData()
        {
            return strSaveData;
        }

        uint32 GetData(uint32 uiType)
        {
            if (uiType >= MAX_ENCOUNTER)
                return 0;

            return m_auiEncounter[uiType];
        }

        uint64 GetGoData(uint32 uiEntry)
        {
            EntryGuidMap::iterator findGo = m_mGoGuidStorage.find(uiEntry);
            if (findGo != m_mGoGuidStorage.cend())
                return findGo->second;

            return 0;
        }

        uint64 GetData64(uint32 uiType)
        {
            switch (uiType)
            {
                case GO_BREWERY_DOOR:
                case GO_OOK_DOOR:
                    return GetGoData(uiType);
                case NPC_OOK_OOK:
                case NPC_HOPTALLUS:
                case NPC_YAN_ZHU:
                case NPC_UNCLE_GAO:
                case NPC_CHEN_YANZHU:
                    EntryGuidMap::iterator find = m_mNpcGuidStorage.find(uiType);
                    if (find != m_mNpcGuidStorage.cend())
                        return find->second;
                    break;
            }

            return 0;
        }

        void Load(const char* chrIn)
        {
            if (!chrIn)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(chrIn);
            std::istringstream loadStream(chrIn);

            loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
                >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6];
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
            if (m_auiEncounter[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                m_auiEncounter[i] = NOT_STARTED;

            m_uiHozenSlain = m_auiEncounter[6];

            if (m_auiEncounter[0] == DONE)
            {
                if (GameObject* pGo = instance->GetGameObject(GetData64(GO_OOK_DOOR)))
                    pGo->AddObjectToRemoveList();
            }

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };

};

// 7755 ( usually exit), might be 7998
class AreaTrigger_at_stormstout_intro : public AreaTriggerScript
{
public:
    AreaTrigger_at_stormstout_intro() : AreaTriggerScript("at_stormstout_intro")
    {
    }

    bool OnTrigger(Player* pPlayer, AreaTriggerEntry const* /*pTrigger*/)
    {
        if (Creature* pChen = GetClosestCreatureWithEntry(pPlayer, NPC_CHEN_STORMSTOUT, 20.f))
        {
            if (pChen->AI())
                pChen->AI()->DoAction(0);

            return true;
        }

        return false;
    }
};

//7781 (just after stairs, currently unknown)


//8366

class AreaTrigger_at_uncle_gao : public AreaTriggerScript
{
public:
    AreaTrigger_at_uncle_gao() : AreaTriggerScript("at_uncle_gao")
    {
    }

    bool OnTrigger(Player* pPlayer, AreaTriggerEntry const /*pTrigger*/)
    {
        if (Creature* pGao = GetClosestCreatureWithEntry(pPlayer, NPC_UNCLE_GAO, 42.f))
        {
            if (pGao->AI())
                pGao->AI()->DoAction(4);

            return true;
        }
        return false;
    }
};



void AddSC_instance_stormstout_brewery()
{
    new instance_stormstout_brewery();
    new AreaTrigger_at_stormstout_intro();
    //new AreaTrigger_at_uncle_gao();
}
