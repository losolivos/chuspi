/*
Echo Isles (Troll Starting Zone)
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "Player.h"
#include "Vehicle.h"
#include "Creature.h"

// Novice Darkspear npcs
class npc_generic_darkspear_novice : public CreatureScript
{
    enum
    {
        NPC_TIKI_TARGET             = 38038
    };

    struct npc_generic_darkspear_noviceAI : public ScriptedAI
    {
        npc_generic_darkspear_noviceAI(Creature* c) : ScriptedAI(c) { }

        void ResetData()
        {
            uint32 cd = 0;

            switch(me->getClass())
            {
            case CLASS_ROGUE:
            case CLASS_WARRIOR:
                meleeClass = true;
                break;
            case CLASS_DRUID:
                cd = 2000;
                break;
            case CLASS_PRIEST:
                cd = 2500;
                break;
            default:
                break;
            }

            cooldown = cd ? cd : 3000;
        }

        void Reset()
        {
            spellId = me->m_spells[0];
            meleeClass = false;
            ResetData();
            castTimer = urand(cooldown, 10000);
            targetTimer = 2000;
        }

        void AttackStart(Unit * who)
        {
            if (meleeClass)
                ScriptedAI::AttackStart(who);
            else
                ScriptedAI::AttackStartNoMove(who);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!me->GetVictim())
            {
                if (targetTimer <= diff)
                {
                    if (Creature * tikiTarget = me->FindNearestCreature(NPC_TIKI_TARGET, meleeClass ? 5.0f : 30.0f))
                    {
                        me->Attack(tikiTarget, meleeClass);
                        if (meleeClass)
                            DoStartMovement(tikiTarget);
                    }
                    else
                        EnterEvadeMode();
                    targetTimer = 2000;
                }
                else targetTimer -= diff;

                return;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
            {
                castTimer -= diff;
                return;
            }

            if (castTimer <= diff)
            {
                if (me->getPowerType() == POWER_MANA)
                    me->SetPower(POWER_MANA, me->GetMaxPower(POWER_MANA));
                DoCastVictim(spellId);
                castTimer = cooldown;
            }else castTimer -= diff;

            if (meleeClass)
                DoMeleeAttackIfReady();
        }

    private:
        uint32 spellId;
        uint32 castTimer;
        uint32 cooldown;
        bool meleeClass;
        uint32 targetTimer;
    };

public:
    npc_generic_darkspear_novice() : CreatureScript("npc_generic_darkspear_novice") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_generic_darkspear_noviceAI (creature);
    }
};

class npc_tiki_target_troll : public CreatureScript
{
    struct npc_tiki_target_trollAI : public ScriptedAI
    {
        npc_tiki_target_trollAI(Creature* c) : ScriptedAI(c)
        {
            SetCombatMovement(false);
            me->SetCorpseDelay(5);
        }

        void Reset()
        {
            resetTimer = 5000;
        }

        void DamageTaken(Unit * done_by, uint32& damage)
        {
            resetTimer = 5000;

            if (me->HealthBelowPctDamaged(75, damage) && done_by->GetTypeId() == TYPEID_UNIT && !done_by->isPet())
                damage = 0;
        }

        void UpdateAI(uint32 const diff)
        {
            if (me->GetVictim())
            {
                if (resetTimer <= diff)
                {
                    me->RemoveAllAuras();
                    me->DeleteThreatList();
                    me->CombatStop(true);
                    me->LoadCreaturesAddon();
                    resetTimer = 5000;
                }else resetTimer -= diff;
            }
        }

        void SpellHit(Unit * caster, const SpellInfo * spell)
        {
            if (spell->Id == 100 || spell->Id == 122 || spell->Id == 172 || spell->Id == 348 || spell->Id == 2098 || spell->Id == 5143 || spell->Id == 8921 ||
                spell->Id == 20271 || spell->Id == 56641 || spell->Id == 73899 || spell->Id == 100787)
                if (Player * pCaster = caster->ToPlayer())
                    pCaster->KilledMonsterCredit(44175);
        }

    private:
        uint32 resetTimer;
    };

public:
    npc_tiki_target_troll() : CreatureScript("npc_tiki_target_troll") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tiki_target_trollAI (creature);
    }
};

/*
Quest "Proving Pit"
*/

#define GOSSIP_ITEM_READY "I'm ready to face my challenge."

class npc_darkspear_jailor : public CreatureScript
{
    enum
    {
        GO_CAGE             = 201968,
        NPC_SCOUT           = 38142,
        GOSSIP_TEXT         = 15251,
        WP_THERE            = 3906201,
        WP_BACK             = 3906202,
    };

    struct npc_darkspear_jailorAI : public ScriptedAI
    {
        npc_darkspear_jailorAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            EventInProgress = false;
            emoteTimer = 2000;
            done = true;
            there = true;
        }

        void DoAction(const int32 action)
        {
            if (action == 1)
            {
                if (!EventInProgress)
                {
                    EventInProgress = true;
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    me->GetMotionMaster()->MovePath(WP_THERE, false);
                }
            }
            else
            {
                Reset();
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            }
        }

        void MovementInform(uint32 /*type*/, uint32 id)
        {
            if (there && id == 1)
            {
                me->HandleEmoteCommand(432);
                there = false;
                done = false;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!done)
            {
                if (emoteTimer <= diff)
                {
                    if (GameObject * cage = me->FindNearestGameObject(GO_CAGE, 10.0f))
                        cage->SetGoState(GO_STATE_ACTIVE);
                    if (Creature * Scout = me->FindNearestCreature(NPC_SCOUT, 10.0f))
                        Scout->AI()->DoAction(1);
                    me->GetMotionMaster()->MovePath(WP_BACK, false);
                    emoteTimer = 2000;
                    done = true;
                }emoteTimer -= diff;
            }
        }

    private:
        bool EventInProgress;
        bool done;
        bool there;
        uint32 emoteTimer;
    };

public:
    npc_darkspear_jailor() : CreatureScript("npc_darkspear_jailor") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_READY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(GOSSIP_TEXT, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            creature->AI()->Talk(1,player->GetGUID());
            creature->AI()->DoAction(1);
            player->KilledMonsterCredit(creature->GetEntry(), creature->GetGUID());
        }
        return true;
    }
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_darkspear_jailorAI (creature);
    }
};

class npc_captive_spitescale_scout : public CreatureScript
{
    enum
    {
        GO_CAGE                     = 201968,
        SPELL_FROST_SHOCK           = 15089,
        NPC_JAILOR                  = 39062,
    };

    struct npc_captive_spitescale_scoutAI : public ScriptedAI
    {
        npc_captive_spitescale_scoutAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            me->SetCorpseDelay(5);
            frostShockTimer = 2000;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }

        void JustDied(Unit * killer)
        {
            if (Creature * jailor = me->FindNearestCreature(NPC_JAILOR, 30.0f))
                jailor->AI()->Talk(2, killer->GetGUID());
        }

        void JustRespawned()
        {
            if (GameObject * go = me->FindNearestGameObject(GO_CAGE, 30.0f))
                go->SetGoState(GO_STATE_READY);
            if (Creature * jailor = me->FindNearestCreature(NPC_JAILOR, 30.0f))
                jailor->AI()->DoAction(2);
        }

        void DoAction(const int32 /*action*/)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
            Talk(1);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(0,  -1151.08f, -5526.25f, 8.11f);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (frostShockTimer <= diff)
            {
                DoCastVictim(SPELL_FROST_SHOCK);
                frostShockTimer = urand(5000, 8000);
            }else frostShockTimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        uint32 frostShockTimer;
    };

public:
    npc_captive_spitescale_scout() : CreatureScript("npc_captive_spitescale_scout") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_captive_spitescale_scoutAI (creature);
    }
};

/*
Quest: Saving the Young
*/
class npc_lost_bloodtalon_hatchling : public CreatureScript
{
    enum
    {
        SPELL_BLOODTALON_WHISTLE            = 70874,
    };

    struct npc_lost_bloodtalon_hatchlingAI : public ScriptedAI
    {
        npc_lost_bloodtalon_hatchlingAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            following = false;
            followTimer = 60000;
        }

        void SpellHit(Unit * caster, const SpellInfo * spell)
        {
            if (!following && followTimer && spell->Id == SPELL_BLOODTALON_WHISTLE)
            {
                if (!urand(0, 4))
                    Talk(1);
                me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveFollow(caster, PET_FOLLOW_DIST, (float)rand_norm()*static_cast<float>(M_PI));
                following = true;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (following)
            {
                if (followTimer <= diff)
                {
                    Position pos;
                    me->GetNearPosition(pos, 30.0f, me->GetFollowAngle());
                    me->GetMotionMaster()->MovePoint(0, pos);
                    me->DespawnOrUnsummon(3000);
                    following = false;
                    followTimer = 0;
                }else followTimer -= diff;
            }
        }
    private:
        uint32 followTimer;
        bool following;
    };

public:
    npc_lost_bloodtalon_hatchling() : CreatureScript("npc_lost_bloodtalon_hatchling") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lost_bloodtalon_hatchlingAI (creature);
    }
};

class spell_bloodtalon_whistle : public SpellScriptLoader
{
    class MovementCheck
    {
    public:
        MovementCheck() { }

        bool operator() (WorldObject* obj)
        {
            return obj->ToUnit()->HasUnitState(UNIT_STATE_FOLLOW);
        }
    };

    class spell_bloodtalon_whistle_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bloodtalon_whistle_SpellScript)

        void SelectTarget(std::list<WorldObject*> & targets)
        {
            targets.remove_if (MovementCheck());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_bloodtalon_whistle_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
        }
    };

public:
    spell_bloodtalon_whistle() : SpellScriptLoader("spell_bloodtalon_whistle") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_bloodtalon_whistle_SpellScript();
    }
};

/*
Quest: Breaking the Line
*/
static const Position jumpPos[2] =
{
    {-1202.38f, -5581.41f, 11.9828f, 0.0f},
    {-916.437f, -5542.72f, -2.21524f, 0.0f},
};

class npc_bloodtalon_thrasher : public CreatureScript
{
    enum
    {
        PATH_BREAKING_THE_LINE          = 3899101,
        SPELL_REMOVE_ALL_PASSENGERS     = 50630,
    };

    struct npc_bloodtalon_thrasherAI : public ScriptedAI
    {
        npc_bloodtalon_thrasherAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            jump = 0;
            jumping = false;
            nextPathTimer = 2500;
        }

        void MovementInform(uint32 /*type*/, uint32 id)
        {
            if (!jumping)
            {
                if ((jump == 0 && id == 4) || (jump == 1 && id == 13))
                {
                    nextPathTimer = 2500;
                    jumping = true;
                    me->GetMotionMaster()->MoveJump(jumpPos[jump].GetPositionX(), jumpPos[jump].GetPositionY(), jumpPos[jump].GetPositionZ(), 10.0f, 20.0f);
                    ++jump;
                }
                else if (jump == 2 && id == 8)
                    me->GetVehicleKit()->RemoveAllPassengers();
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (jumping)
            {
                if (nextPathTimer <= diff)
                {
                    me->GetMotionMaster()->UpdateMotion(10000); // hacky - needed for smooth movement without 2-3 sec delays
                    me->GetMotionMaster()->MovePath(PATH_BREAKING_THE_LINE + jump, false);
                    jumping = false;
                    nextPathTimer = 0;
                }nextPathTimer -= diff;
            }
        }
    private:
        int8 jump;
        bool jumping;
        uint32 nextPathTimer;
    };

public:
    npc_bloodtalon_thrasher() : CreatureScript("npc_bloodtalon_thrasher") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodtalon_thrasherAI (creature);
    }
};

/*class veh_bloodtalon_thrasher : public VehicleScript
{

public:
    veh_bloodtalon_thrasher() : VehicleScript("npc_bloodtalon_thrasher") { }

    void OnAddPassenger(Vehicle* veh, Unit* passenger, int8 seatI/)
    {
        veh->GetBase()->RemoveCharmedBy(passenger);
        veh->GetBase()->GetMotionMaster()->Clear();
        veh->GetBase()->GetMotionMaster()->MovePath(3899101, false);
    }
};*/

class npc_tortunga : public CreatureScript
{
    enum
    {
        NPC_JORNUN          = 38989,
    };

public:
    npc_tortunga() : CreatureScript("npc_tortunga") { }

    bool OnQuestAccept(Player* /*player*/, Creature* creature, Quest const* /*quest*/)
    {
        if (Creature * jornun = creature->FindNearestCreature(NPC_JORNUN, 20.0f))
            jornun->AI()->Talk(1);
        return true;
    }
};

/*
Quest: An Ancient Enemy -- needs some corrections
*/

static const Position ancientEnemyPos[3] =
{
    {-716.50f, -5597.82f, 25.56f, 1.23f}, // Vanira
    {-730.68f, -5592.52f, 25.5f, 0.52f}, // Zuni
    {-722.39f, -5597.65f, 25.5f, 0.89f}, // Vol'jin
};

enum ancientEnemy
{
        NPC_ZUNI                    = 38423,
        NPC_ZARJIRA                 = 38306,
        NPC_VANIRA                  = 38437,
        NPC_VOLJIN                  = 38225,
        EVENT_INTRO                 = 1,
        SPELL_FIRES                 = 72250,
        SPELL_FIRES_BEAM            = 73294,
};

#define VOLJIN_GOSSIP       "I am ready, Vol'jin."
static const uint32 npcId[3] = { NPC_ZUNI, NPC_VANIRA, NPC_VOLJIN };

class npc_voljin_ancient_enemy : public CreatureScript
{
    enum
    {
        QUEST_ANCIENT_ENEMY         = 24814,
        GOSSIP_VOLJIN               = 15318,

        SPELL_VOLJIN_CREDIT         = 73589,
        SPELL_SHADOW_SHOCK          = 73087,
        SPELL_SHOOT                 = 20463,

        EVENT_SHOOT                 = 2,
        EVENT_SHADOW_SHOCK          = 3,

        SAY_INTRO                   = 1,
        SAY_BRAZIERS                = 2,
        SAY_OUTRO_1                 = 3,
        SAY_OUTRO_2                 = 4,
        SAY_OUTRO_3                 = 5,
    };

    struct npc_voljin_ancient_enemyAI : public ScriptedAI
    {
        npc_voljin_ancient_enemyAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            EventInProgress = false;
            events.Reset();
        }

        void AttackStart(Unit * who)
        {
            ScriptedAI::AttackStartNoMove(who);
        }

        void DoAction(const int32 /*action*/)
        {
            if (EventInProgress)
                return;
            EventInProgress = true;

            for (int i=0; i<=2; ++i)
            {
                if (Creature * creature = i == 2 ? me : me->FindNearestCreature(i ? NPC_ZUNI : NPC_VANIRA, 20.0f))
                {
                    creature->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                    creature->SetReactState(REACT_PASSIVE);
                    creature->GetMotionMaster()->MovePoint(0, ancientEnemyPos[i]);
                }
            }

            events.ScheduleEvent(EVENT_INTRO, 10000);
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_SHOOT, 2000);
            events.ScheduleEvent(EVENT_SHADOW_SHOCK, 5000);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!EventInProgress)
                return;

            events.Update(diff);
            {
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                    case EVENT_INTRO:
                        Talk(SAY_INTRO);
                        if (Creature * zarjira = me->FindNearestCreature(NPC_ZARJIRA, 30.0f))
                            zarjira->AI()->DoAction(1);
                        break;
                    case EVENT_SHOOT:
                        DoCastVictim(SPELL_SHOOT);
                        events.ScheduleEvent(EVENT_SHOOT, urand(2000, 2500));
                        break;
                    case EVENT_SHADOW_SHOCK:
                        DoCastVictim(SPELL_SHADOW_SHOCK);
                        events.ScheduleEvent(EVENT_SHADOW_SHOCK, urand(8000, 10000));
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    private:
        bool EventInProgress;
        EventMap events;
    };

public:

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_ANCIENT_ENEMY) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, VOLJIN_GOSSIP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(GOSSIP_VOLJIN, creature->GetGUID());
        }
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
            creature->AI()->DoAction(1);
        return true;
    }

    npc_voljin_ancient_enemy() : CreatureScript("npc_voljin_ancient_enemy") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_voljin_ancient_enemyAI (creature);
    }
};

class npc_zarjira : public CreatureScript
{
    enum
    {
        NPC_FIRE_OF_THE_SEAS        = 38542,
        SPELL_FROSTBOLT             = 46987,
        SPELL_SUMMON_MANIFESTATION  = 73295,
        SPELL_DELUGE_OF_SHADOW      = 72044,

        EVENT_COMBAT                = 2,
        EVENT_FROSTBOLT             = 3,
        EVENT_MANIFESTATION         = 4,
        EVENT_FIRES                 = 5,

        SAY_INTRO                   = 1,
        SAY_FIRES                   = 2,
    };

    struct npc_zarjiraAI : public ScriptedAI
    {
        npc_zarjiraAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        void AttackStart(Unit * who)
        {
            ScriptedAI::AttackStartNoMove(who);
        }

        void DoAction(const int32 action)
        {
            if (action == 1)
                events.ScheduleEvent(EVENT_INTRO, 12000);
            else
            {
                Talk(SAY_FIRES);
                if (Creature * voljin = me->FindNearestCreature(NPC_VOLJIN, 30.0f))
                    voljin->RemoveAllAuras();
            }
        }

        void JustDied(Unit *)
        {
            for(int i=0;i<=2;++i)
                if (Creature * creature = me->FindNearestCreature(npcId[i], 30.0f))
                    creature->AI()->EnterEvadeMode();
        }

        void EnterCombat(Unit *)
        {
            events.ScheduleEvent(EVENT_FROSTBOLT, 3000);
            events.ScheduleEvent(EVENT_MANIFESTATION, 10000);
            events.ScheduleEvent(EVENT_FIRES, 60000);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                case EVENT_INTRO:
                    Talk(1);
                    events.ScheduleEvent(EVENT_COMBAT, 10000);
                    break;
                case EVENT_COMBAT:
                    {
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (Creature * creature = me->FindNearestCreature(NPC_VOLJIN, 30.0f))
                            creature->CastSpell(me, SPELL_DELUGE_OF_SHADOW, true);

                        for(int i=0;i<=2;++i)
                        {
                            if (Creature * creature = me->FindNearestCreature(npcId[i], 30.0f))
                            {
                                me->SetInCombatWith(creature);
                                if (i == 2)
                                {
                                    me->Attack(creature, false);
                                    creature->Attack(me, false);
                                }
                                else
                                {
                                    creature->Attack(me, true);
                                    creature->GetMotionMaster()->MoveChase(me);
                                }

                            }
                        }
                        me->SetReactState(REACT_PASSIVE);
                    }
                    break;
                case EVENT_FROSTBOLT:
                    if (me->GetPower(POWER_MANA) <= me->GetMaxPower(POWER_MANA) / 4)
                        me->SetPower(POWER_MANA, me->GetMaxPower(POWER_MANA));
                    DoCastVictim(SPELL_FROSTBOLT);
                    events.ScheduleEvent(EVENT_FROSTBOLT, urand(2500, 3000));
                    break;
                case EVENT_MANIFESTATION:
                    if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        target->CastSpell(target, SPELL_SUMMON_MANIFESTATION, true);
                    events.ScheduleEvent(EVENT_MANIFESTATION, urand(10000, 15000));
                    break;
                case EVENT_FIRES:
                    {
                        std::list<Creature*> cList;
                        me->GetCreatureListWithEntryInGrid(cList, NPC_FIRE_OF_THE_SEAS, 30.0f);
                        for(std::list<Creature*>::const_iterator i = cList.begin(); i != cList.end(); ++i)
                        {
                            (*i)->CastSpell((*i), SPELL_FIRES, true);
                            (*i)->CastSpell(me, SPELL_FIRES_BEAM, true);
                        }
                        events.ScheduleEvent(EVENT_FIRES, urand(60000, 80000));
                    }
                    break;
                default:
                    break;
                }
            }
        }
    private:
        EventMap events;
    };

public:
    npc_zarjira() : CreatureScript("npc_zarjira") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_zarjiraAI (creature);
    }
};

class npc_fire_of_the_seas : public CreatureScript
{
    enum
    {
        SPELL_STAMP_OUT_FIRES       = 73296,
    };

    struct npc_fire_of_the_seasAI : public ScriptedAI
    {
        npc_fire_of_the_seasAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void SpellHit(Unit * /*caster*/, const SpellInfo * spell)
        {
            if (spell->Id == SPELL_STAMP_OUT_FIRES)
            {
                me->RemoveAllAuras();
                me->InterruptNonMeleeSpells(true);
                if (Creature * zarjira = me->FindNearestCreature(NPC_ZARJIRA, 30.0f))
                    if (!zarjira->HasAura(SPELL_FIRES_BEAM))
                        zarjira->AI()->DoAction(2);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
        }
    private:
    };

public:
    npc_fire_of_the_seas() : CreatureScript("npc_fire_of_the_seas") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fire_of_the_seasAI (creature);
    }
};

void AddSC_echo_isles()
{
    new npc_generic_darkspear_novice();
    new npc_tiki_target_troll();
    new npc_darkspear_jailor();
    new npc_captive_spitescale_scout();
    new npc_lost_bloodtalon_hatchling();
    new spell_bloodtalon_whistle();
    new npc_bloodtalon_thrasher();
    //new veh_bloodtalon_thrasher();
    new npc_tortunga();
    new npc_voljin_ancient_enemy();
    new npc_zarjira();
    new npc_fire_of_the_seas();
}
