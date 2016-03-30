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
#include "ScriptedCreature.h"
#include "MoveSplineInit.h"
#include "temple_of_the_jade_serpent.h"

class boss_sha_of_doubt : public CreatureScript
{
public:
    boss_sha_of_doubt() : CreatureScript("boss_sha_of_doubt") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_sha_of_doubt_AI(creature);
    }

    enum eSpells
    {
        SPELL_WITHER_WILL            = 106736,
        SPELL_TOUCH_OF_NOTHINGNESS   = 106113,
        SPELL_BOUNDS_OF_REALITY      = 117665,
        SPELL_FIGMENT_OF_DOUBT_2     = 106936,
        SPELL_INVISIBILITY_DETECTION = 126839
    };

    enum eEvents
    {
        EVENT_WITHER_WILL          = 1,
        EVENT_TOUCH_OF_NOTHINGNESS = 2,
        EVENT_BOUNDS_OF_REALITY    = 3,
        EVENT_CLOSE_DOOR           = 4,
        EVENT_OPEN_DOOR            = 5
    };

    enum eActions
    {
        ACTION_FIGMENT_DIE = 0
    };

    enum eTalks
    {
        TALK_AGGRO   = 0,
        TALK_DEATH   = 1,
        TALK_FIGMENT = 2,
        TALK_RESET   = 3,
        TALK_SLAY    = 4
    };

    struct boss_sha_of_doubt_AI : public BossAI
    {
        boss_sha_of_doubt_AI(Creature* creature) : BossAI(creature, DATA_SHA_OF_DOUBT) {}

        uint32 figmentsCount, figmentsDie;

        EventMap events;
        EventMap nonCombatEvents;

        void InitializeAI()
        {
            me->CastSpell(me, SPELL_INVISIBILITY_DETECTION, false);
            
            if (instance)
            {
                instance->SetData(DATA_SHA_OF_DOUBT, NOT_STARTED);
                if (instance->GetBossState(DATA_LIU) == DONE)
                {
                    nonCombatEvents.ScheduleEvent(EVENT_OPEN_DOOR, 1 * IN_MILLISECONDS);
                    for(int i = 0; i < 2; i++)
                    {
                        if (Creature* sha = me->SummonCreature(NPC_MINION_OF_DOUBTS, ShaSummonPosition[i]))
                        {
                            float x, y, z;
                            x = sha->GetPositionX();
                            y = sha->GetPositionY();
                            z = sha->GetPositionZ();
                            GetPositionWithDistInOrientation(sha, 18.0f, sha->GetOrientation(), x, y);
                            Movement::MoveSplineInit init(sha);
                            init.MoveTo(x, y, z);
                            init.SetVelocity(9.0f);
                            init.Launch();
                        }
                    }
                }
                else
                {
                    nonCombatEvents.ScheduleEvent(EVENT_CLOSE_DOOR, 1 * IN_MILLISECONDS);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                }
            }

            Reset();
        }

        void Reset()
        {
            figmentsCount = 0;
            figmentsDie   = 0;
            events.Reset();
            Talk(TALK_RESET);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_FIGMENT_DIE)
            {
                ++figmentsDie;
                if (figmentsDie == figmentsCount)
                {
                    figmentsDie = 0;
                    figmentsCount = 0;
                    me->RemoveAura(SPELL_BOUNDS_OF_REALITY);
                }
            }
        }

        void KilledUnit(Unit* unit)
        {
            if (unit->GetTypeId() == TYPEID_PLAYER)
                Talk(TALK_SLAY);
        }

        void JustDied(Unit* /*unit*/)
        {
            _JustDied();
            Talk(TALK_DEATH);

            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_SHA_OF_DOUBT, DONE);
            }

            std::list<Creature*> figments;
            me->GetCreatureListWithEntryInGrid(figments, NPC_FIGMENT_OF_DOUBT, 100.0f);
            if (!figments.empty())
            {
                for(auto creature : figments)
                    creature->DespawnOrUnsummon();
            }
        }

        void EnterCombat(Unit* /*unit*/)
        {
            _EnterCombat();
            Talk(TALK_AGGRO);
            events.ScheduleEvent(EVENT_WITHER_WILL, 5 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_TOUCH_OF_NOTHINGNESS, 1 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_BOUNDS_OF_REALITY, urand(8, 12) * IN_MILLISECONDS);

            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                instance->SetData(DATA_SHA_OF_DOUBT, IN_PROGRESS);
            }
        }

        void EnterEvadeMode()
        {
            BossAI::EnterEvadeMode();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_SHA_OF_DOUBT, FAIL);
                instance->SetBossState(DATA_SHA_OF_DOUBT, FAIL);
            }

            std::list<Creature*> figments;
            me->GetCreatureListWithEntryInGrid(figments, NPC_FIGMENT_OF_DOUBT, 100.0f);
            if (!figments.empty())
            {
                for(auto creature : figments)
                    creature->DespawnOrUnsummon();
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (uint32 eventId = nonCombatEvents.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_CLOSE_DOOR:
                        if (GameObject* door = me->FindNearestGameObject(GAMEOBJECT_DOOR_SHA_OF_DOUBT, 70.0f))
                            door->SetGoState(GO_STATE_READY);
                        break;
                    case EVENT_OPEN_DOOR:
                        if (GameObject* door = me->FindNearestGameObject(GAMEOBJECT_DOOR_SHA_OF_DOUBT, 70.0f))
                            door->SetGoState(GO_STATE_ACTIVE);
                        break;
                }
            }

            events.Update(diff);
            nonCombatEvents.Update(diff);

            if (!UpdateVictim())
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_WITHER_WILL:
                        if (!me->HasAura(SPELL_BOUNDS_OF_REALITY))
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            {
                                me->CastSpell(target, SPELL_WITHER_WILL, false);
                                me->CastSpell(target, SPELL_WITHER_WILL, false);
                            }
                        }
                        events.ScheduleEvent(EVENT_WITHER_WILL, urand(4, 7) * IN_MILLISECONDS);
                        break;
                    case EVENT_TOUCH_OF_NOTHINGNESS:
                        if (!me->HasAura(SPELL_BOUNDS_OF_REALITY))
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            me->CastSpell(target, SPELL_TOUCH_OF_NOTHINGNESS, false);
                        events.ScheduleEvent(EVENT_TOUCH_OF_NOTHINGNESS, 30 * IN_MILLISECONDS);
                        break;
                    case EVENT_BOUNDS_OF_REALITY:
                    {
                        Talk(TALK_FIGMENT);
                        me->CastSpell(me, SPELL_BOUNDS_OF_REALITY, false);

                        Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
                        if (!playerList.isEmpty())
                        {
                           figmentsCount = playerList.getSize();
                           for(Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                           {
                               if (Player* plr = i->GetSource())
                                   plr->CastSpell(plr, SPELL_FIGMENT_OF_DOUBT_2, false);
                           }
                        }
                        events.ScheduleEvent(EVENT_BOUNDS_OF_REALITY, 60 * IN_MILLISECONDS);
                        break;
                    }
                }
            }

            if (!me->HasAura(SPELL_BOUNDS_OF_REALITY))
                DoMeleeAttackIfReady();
        }
    };
};

class mob_figment_of_doubt : public CreatureScript
{
public:
    mob_figment_of_doubt() : CreatureScript("mob_figment_of_doubt") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_figment_of_doubt_AI(creature);
    }

    enum eEvents
    {
        EVENT_GATHERING_DOUBT      = 1,
        EVENT_SPELL_PHANTOM_STRIKE = 2,
        EVENT_SPELL_ARMOR_BUFF     = 3,
        EVENT_SPELL_FURY           = 4,
        EVENT_SANCTUARY            = 5,
        EVENT_SIPHON_ESSENCE       = 6,
        EVENT_STUN                 = 7,
        EVENT_BLADE_SONG           = 8,
        EVENT_UNTAMED_FURY         = 9,
        EVENT_GLIMPSE_OF_MADNESS   = 10
    };

    enum eActions
    {
        ACTION_FIGMENT_DIE = 0
    };

    enum eSpells
    {
        SPELL_SHADOWFORM        = 41408,
        SPELL_GROW              = 104921,
        SPELL_DROWNED_STATE     = 117509,
        SPELL_DRAW_DOUBT        = 106290,
        SPELL_KNOCK_BACK_SELF   = 117525,
        SPELL_GATHERING_DOUBT   = 117570,
        SPELL_GATHERING_DOUBT_2 = 117571,
        SPELL_WEAKENED_BLOWS    = 115798,
        SPELL_RELEASE_DOUBT     = 106112,
        SPELL_QUIET_SUICIDE     = 115372
    };

    struct mob_figment_of_doubt_AI : public ScriptedAI
    {
        mob_figment_of_doubt_AI(Creature* creature) : ScriptedAI(creature) {}

        InstanceScript* instance;

        void InitializeAI()
        {
            instance = me->GetInstanceScript();
            me->CastSpell(me, SPELL_SHADOWFORM, false);
            me->CastSpell(me, SPELL_GROW, false);

            if (me->ToTempSummon())
                me->CastSpell((Unit*)NULL, SPELL_DRAW_DOUBT, false);
        }

        void EnterCombat(Unit* u)
        {
            me->CastSpell(me, SPELL_GATHERING_DOUBT, false);
            events.ScheduleEvent(EVENT_GATHERING_DOUBT, 1 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SIPHON_ESSENCE, 8 * IN_MILLISECONDS);

            if (me->ToTempSummon())
            {
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                {
                    if (summoner->GetTypeId() == TYPEID_PLAYER)
                    {
                        switch(summoner->ToPlayer()->GetRoleForGroup(summoner->ToPlayer()->GetSpecializationId(summoner->ToPlayer()->GetActiveSpec())))
                        {
                            case ROLES_HEALER:
                                events.ScheduleEvent(EVENT_SPELL_PHANTOM_STRIKE, 20 * IN_MILLISECONDS);
                                events.ScheduleEvent(EVENT_STUN, 7 * IN_MILLISECONDS);
                                break;
                            case ROLES_DPS:
                                events.ScheduleEvent(EVENT_SPELL_FURY, 5 * IN_MILLISECONDS);
                                events.ScheduleEvent(EVENT_BLADE_SONG, 13 * IN_MILLISECONDS);
                                events.ScheduleEvent(EVENT_GLIMPSE_OF_MADNESS, 8 * IN_MILLISECONDS);
                                break;
                            case ROLES_TANK:
                                events.ScheduleEvent(EVENT_SPELL_ARMOR_BUFF, 10 * IN_MILLISECONDS);
                                events.ScheduleEvent(EVENT_SANCTUARY, urand(8, 12) * IN_MILLISECONDS);
                                events.ScheduleEvent(EVENT_STUN, 7 * IN_MILLISECONDS);
                                break;
                        }
                    }
                }
            }
        }

        void JustDied(Unit* killer)
        {
            me->CastSpell(me, SPELL_DROWNED_STATE, false);
            me->RemoveAura(SPELL_GATHERING_DOUBT);
            me->DespawnOrUnsummon(5 * IN_MILLISECONDS);

            if (me->ToTempSummon())
            {
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    summoner->RemoveAura(SPELL_DRAW_DOUBT);
            }

            if (Creature* sha = Unit::GetCreature(*me, instance->GetData64(DATA_SHA_OF_DOUBT)))
            {
                if (sha->IsAIEnabled)
                    sha->AI()->DoAction(ACTION_FIGMENT_DIE);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_GATHERING_DOUBT:
                        if (me->GetAuraCount(SPELL_GATHERING_DOUBT_2) == 30)
                        {
                            me->RemoveAura(SPELL_GATHERING_DOUBT);
                            me->CastSpell(me, SPELL_RELEASE_DOUBT, false); 
                            me->CastSpell(me, SPELL_QUIET_SUICIDE, false);
                        }
                        else
                            events.ScheduleEvent(EVENT_GATHERING_DOUBT, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_SPELL_PHANTOM_STRIKE:
                        if (me->GetVictim())
                            me->CastSpell(me->GetVictim(), 9806, false);
                        events.ScheduleEvent(EVENT_SPELL_PHANTOM_STRIKE, 20 * IN_MILLISECONDS);
                        break;
                    case EVENT_SPELL_ARMOR_BUFF:
                        me->CastSpell(me, 34199, false);
                        events.ScheduleEvent(EVENT_SPELL_ARMOR_BUFF, 10 * IN_MILLISECONDS);
                        break;
                    case EVENT_SPELL_FURY:
                        me->CastSpell(me, 15494, false);
                        events.ScheduleEvent(EVENT_SPELL_FURY, 5 * IN_MILLISECONDS);
                        break;
                    case EVENT_SANCTUARY:
                        me->CastSpell(me, 69207, false);
                        events.ScheduleEvent(EVENT_SANCTUARY, 10 * IN_MILLISECONDS);
                        break;
                    case EVENT_SIPHON_ESSENCE:
                        me->CastSpell(me, 40291, false);
                        events.ScheduleEvent(EVENT_SIPHON_ESSENCE, 8 * IN_MILLISECONDS);
                        break;
                    case EVENT_STUN:
                        if (me->GetVictim())
                            me->CastSpell(me->GetVictim(), 23454, false);
                        events.ScheduleEvent(EVENT_STUN, 7 * IN_MILLISECONDS);
                        break;
                    case EVENT_BLADE_SONG:
                        me->CastSpell(me, 38282, false);
                        events.ScheduleEvent(EVENT_BLADE_SONG, 13 * IN_MILLISECONDS);
                        break;
                    case EVENT_UNTAMED_FURY:
                        me->CastSpell(me, 23719, false);
                        events.ScheduleEvent(EVENT_UNTAMED_FURY, 9 * IN_MILLISECONDS);
                        break;
                    case EVENT_GLIMPSE_OF_MADNESS:
                        if (me->GetVictim())
                            me->CastSpell(me->GetVictim(), 26108, false);
                        events.ScheduleEvent(EVENT_GLIMPSE_OF_MADNESS, 8 * IN_MILLISECONDS);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class PlayerTargetSelector
{
public:
    PlayerTargetSelector() { }

    bool operator()(WorldObject* object)
    {
        if (object->GetTypeId() == TYPEID_PLAYER)
            return false;

        return true;
    }
};

class ShaTargetSelector
{
public:
    ShaTargetSelector() { }

    bool operator()(WorldObject* object)
    {
        if (Creature* creature = object->ToCreature())
           if (creature->GetEntry() == NPC_SHA_OF_DOUBT)
              return false;

        return true;
    }
};

class spell_sod_release_doubt : public SpellScriptLoader
{
public:
    spell_sod_release_doubt() : SpellScriptLoader("spell_sod_release_doubt") { }

    class spell_sod_release_doubt_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sod_release_doubt_SpellScript);

        void FilterPlayerTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(PlayerTargetSelector());
        }

        void FilterSha(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ShaTargetSelector());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sod_release_doubt_SpellScript::FilterPlayerTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sod_release_doubt_SpellScript::FilterSha, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_sod_release_doubt_SpellScript();
    }
};


void AddSC_boss_sha_of_doubt()
{
    new boss_sha_of_doubt();
    new mob_figment_of_doubt();
    new spell_sod_release_doubt();
}