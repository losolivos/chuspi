/*
*    Dungeon : Scarlet monastery 31-90
*    Instance General Script
*/

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "VMapFactory.h"
#include "scarlet_monastery.h"
#include "Log.h"
#include "Containers.h"


DoorData const doorData[] =
{
    { GO_KORLOFF_EXIT, BOSS_BROTHER_KORLOFF, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_WHITEMANE_ENTRANCE,BOSS_WHITEMANE, DOOR_TYPE_SPAWN_HOLE, BOUNDARY_NONE}
};

class instance_scarlet_monastery : public InstanceMapScript
{
public:
    instance_scarlet_monastery() : InstanceMapScript("instance_scarlet_monastery", 1004) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_scarlet_monastery_InstanceMapScript(map);
    }

    struct instance_scarlet_monastery_InstanceMapScript : public InstanceScript
    {
        instance_scarlet_monastery_InstanceMapScript(Map* map) : InstanceScript(map) {}
        std::set<ObjectGuid> guids;
        uint32 m_auiEncounter[MAX_TYPES];

        void Initialize()
        {
            SetBossNumber(encounternumber);
            LoadDoorData(doorData);
            guids.clear();
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
            case THALNOS_THE_SOULRENDER:
                ThalnosGUID = creature->GetGUID();
                break;
            case BROTHER_KORLOFF:
                KorloffGUID = creature->GetGUID();
                break;
            case COMMANDER_DURAND:
                DurandGUID = creature->GetGUID();
                break;
            case HIGH_INQUISITOR_WHITEMANE:
                WhitemaneGUID = creature->GetGUID();
                break;
            case FALLEN_CRUSADER:
                FallenCrusaderGUID = creature->GetGUID();
                guids.insert(FallenCrusaderGUID);
                break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
            case GO_KORLOFF_EXIT:
            case GO_WHITEMANE_ENTRANCE:
                AddDoor(go, true);
                break;
            }
        }

        void OnUnitDeath(Unit* unit) override
        {
            Creature* creature = unit->ToCreature();
            if (!creature)
                return;
            if (creature->GetEntry()==FALLEN_CRUSADER)
                guids.erase(creature->GetGUID());
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            switch (type)
            {
            case BOSS_THALNOS_THE_SOULRENDER:
                return ThalnosGUID;
            case BOSS_BROTHER_KORLOFF:
                return KorloffGUID;
            case BOSS_DURAND:
                return DurandGUID;
            case BOSS_WHITEMANE:
                return WhitemaneGUID;
            default:
                break;
            }
            return emptyGUID;
        }

        uint64 GetData64(uint32 type)
        {
            switch (type)
            {
            case BOSS_THALNOS_THE_SOULRENDER:
                return ThalnosGUID;
            case BOSS_BROTHER_KORLOFF:
                return KorloffGUID;
            case BOSS_WHITEMANE:
                return WhitemaneGUID;
            case BOSS_DURAND:
                return KorloffGUID;
            }

            return 0;
        }

        std::string GetSaveData()
        {
            std::ostringstream saveStream;
            saveStream << m_auiEncounter[0] << ' ' << m_auiEncounter[1] << ' ' << m_auiEncounter[2] << ' '
                << m_auiEncounter[3] << ' ' << m_auiEncounter[4] << ' ' << m_auiEncounter[5] << ' '
                << m_auiEncounter[6] << ' ' << m_auiEncounter[7] << ' ' << m_auiEncounter[8] << ' '
                << m_auiEncounter[9] << ' ' << m_auiEncounter[10] << ' ' << m_auiEncounter[11] << ' '
                << m_auiEncounter[12] << ' ' << m_auiEncounter[13];
            return saveStream.str();
        }

        bool isWipe()
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();

            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                {
                    if (Player* plr = i->GetSource())
                        if (plr->IsAlive() && !plr->isGameMaster())
                            return false;
                }
            }
            return true;
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;
            if (type==BOSS_DURAND)
                if (state == SPECIAL)
                {
                    if (Creature* Durand = instance->GetCreature(DurandGUID))
                    {
                        Durand->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        Durand->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        Durand->SetReactState(REACT_AGGRESSIVE);
                        Durand->SetStandState(UNIT_STAND_STATE_STAND);
                        Durand->SetFullHealth();
                        if (Player* _player = Durand->FindNearestPlayer(VISIBLE_RANGE))
                            Durand->CombatStart(_player);
                    }
                }
            return true;
        }
    protected:
        ObjectGuid ThalnosGUID;
        ObjectGuid KorloffGUID;
        ObjectGuid DurandGUID;
        ObjectGuid WhitemaneGUID;
        ObjectGuid emptyGUID;
        ObjectGuid FallenCrusaderGUID;
        ObjectGuid emptyCreature;

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
                >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
                >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11]
                >> m_auiEncounter[12] >> m_auiEncounter[13];

            // Do not load an encounter as "In Progress" - reset it instead.
            for (uint8 i = 0; i < MAX_TYPES; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS)
                    m_auiEncounter[i] = NOT_STARTED;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

    };
};

enum flamethower_events
{
    EVENT_FLAMETHOWER_NONCOMBAT = 1,
    EVENT_FLAMETHOWER_COMBAT = 2
};

enum flamethower_spells
{
    SPELL_FLAMETHOWER = 115506
};
class npc_scarlet_flamethower : public CreatureScript
{
public:
    npc_scarlet_flamethower() : CreatureScript("npc_scarlet_flamethower")
    {
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_flamethowerAI(creature);
    }

    struct npc_scarlet_flamethowerAI : public ScriptedAI
    {
        npc_scarlet_flamethowerAI(Creature* creature) : ScriptedAI(creature) { nonCombatEvents.ScheduleEvent(EVENT_FLAMETHOWER_NONCOMBAT, urand(1500, 2000)); }

        EventMap events, nonCombatEvents;

        void Reset()
        {
            events.Reset();
        }


        void EnterCombat(Unit* /*who*/) { nonCombatEvents.Reset(); events.ScheduleEvent(EVENT_FLAMETHOWER_COMBAT, urand(1500, 2000)); }

        void JustDied(Unit* /*killer*/) {}

        void UpdateAI(const uint32 diff)
        {

            events.Update(diff);
            nonCombatEvents.Update(diff);

            if (uint32 eventId = nonCombatEvents.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FLAMETHOWER_NONCOMBAT:
                    DoCastAOE(SPELL_FLAMETHOWER);
                    nonCombatEvents.ScheduleEvent(EVENT_FLAMETHOWER_NONCOMBAT, urand(6000, 8000));
                    break;
                default:
                    break;
                }
            }
            if (!UpdateVictim())
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FLAMETHOWER_COMBAT:
                    DoCast(SelectTarget(SELECT_TARGET_TOPAGGRO), SPELL_FLAMETHOWER);
                    events.ScheduleEvent(EVENT_FLAMETHOWER_COMBAT, urand(6000, 8000));
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};
// ###60033###
class npc_frenzied_spirit : public CreatureScript
{
public:
    npc_frenzied_spirit() : CreatureScript("npc_frenzied_spirit")
    {
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frenzied_spiritAI(creature);
    }

    struct npc_frenzied_spiritAI : public ScriptedAI
    {
        npc_frenzied_spiritAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
            frenzitimer = 1000;
            me->RemoveAllAuras();
        }
        uint32 frenzitimer;

        void EnterCombat(Unit* /*who*/) { frenzitimer = 1000; }

        void JustDied(Unit* /*killer*/) {}

        void UpdateAI(const uint32 diff)
        {

            events.Update(diff);

            if (!UpdateVictim())
                return;
            if (frenzitimer <= diff)
            {
                me->CastSpell(me, 115524, false);
                frenzitimer = urand(1000, 1500);
            }
            else frenzitimer -= diff;
            DoMeleeAttackIfReady();
        }
    };
};
// ###58555###
class npc_scarlet_fanatic : public CreatureScript
{
public:
    npc_scarlet_fanatic() : CreatureScript("npc_scarlet_fanatic")
    {
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_fanaticAI(creature);
    }

    struct npc_scarlet_fanaticAI : public ScriptedAI
    {
        npc_scarlet_fanaticAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
            puritytimer = 1000;
            fanaticstriketimer = 2000;
            me->RemoveAllAuras();
        }
        uint32 puritytimer;
        uint32 fanaticstriketimer;

        void EnterCombat(Unit* /*who*/) { puritytimer = 1000; fanaticstriketimer = 2000; me->HandleEmoteCommand(0); }

        void JustDied(Unit* /*killer*/) {}

        void UpdateAI(const uint32 diff)
        {

            events.Update(diff);

            if (!UpdateVictim())
                return;

            if (puritytimer <= diff)
            {
                me->CastSpell(me, 110954, false);
                puritytimer = 15000;
            }
            else puritytimer -= diff;

            if (fanaticstriketimer <= diff)
            {
                me->CastSpell(SelectTarget(SELECT_TARGET_TOPAGGRO), 110956,false);
                fanaticstriketimer = urand(5000, 6000);
            }
            else fanaticstriketimer -= diff;
            DoMeleeAttackIfReady();
        }
    };
};

// ###59722###
class npc_pile_corpses : public CreatureScript
{
public:
    npc_pile_corpses() : CreatureScript("npc_pile_corpses")
    {
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_pile_corpsesAI(creature);
    }

    struct npc_pile_corpsesAI : public ScriptedAI
    {
        npc_pile_corpsesAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset()
        {
            events.Reset();
            me->RemoveAllAuras();
            me->ClearUnitState(UNIT_STATE_ROOT);
            summons.DespawnAll();
        }
        uint32 frenzitimer;

        void EnterCombat(Unit* /*who*/) { me->SetReactState(REACT_PASSIVE); me->AddUnitState(UNIT_STATE_ROOT); me->CastSpell(me, 114951, false);}

        void JustDied(Unit* /*killer*/) { summons.DespawnAll(); }

        void UpdateAI(const uint32 diff)
        {

            events.Update(diff);

            if (!UpdateVictim())
                return;
        }
    };
};

void AddSC_instance_scarlet_monastery()
{
    new instance_scarlet_monastery();
    new npc_scarlet_flamethower();
    new npc_frenzied_spirit();
    new npc_scarlet_fanatic();
    new npc_pile_corpses();
}
