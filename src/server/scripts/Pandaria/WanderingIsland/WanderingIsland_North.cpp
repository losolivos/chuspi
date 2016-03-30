#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "SpellScript.h"

class mob_master_shang_xi : public CreatureScript
{
    enum master_shang
    {

        SPELL_MASTERS_FLAME         = 114610,
        SPELL_CREATE_MASTERS_FLAME  = 114611,
        SPELL_SNATCH_MASTERS_FLAME  = 114746,

        ITEM_MASTERS_FLAME          = 80212,

        QUEST_LESSONS_OF_BURNING_SCROLL = 29408,
    };

public:
    mob_master_shang_xi() : CreatureScript("mob_master_shang_xi") { }

    bool OnQuestAccept(Player* /*player*/, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_LESSONS_OF_BURNING_SCROLL) // The Lesson of the Burning Scroll
        {
            creature->AddAura(SPELL_MASTERS_FLAME, creature);
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_master_shang_xi_AI(creature);
    }

    struct mob_master_shang_xi_AI : public ScriptedAI
    {
        mob_master_shang_xi_AI(Creature* creature) : ScriptedAI(creature)
        {
            resetTimer = 10000;
        }

        uint32 resetTimer;

        void SpellHit(Unit* caster, const SpellInfo* pSpell)
        {
            if (pSpell->Id == SPELL_SNATCH_MASTERS_FLAME) // Snatch Master's Flame
            {
                if (caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if (caster->ToPlayer()->GetQuestStatus(QUEST_LESSONS_OF_BURNING_SCROLL) == QUEST_STATUS_INCOMPLETE)
                    {
                        me->CastSpell(caster, SPELL_CREATE_MASTERS_FLAME, true);
                        me->RemoveAurasDueToSpell(SPELL_MASTERS_FLAME);
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                        Talk(0);
                    }
                }
            }
        }

        void MoveInLineOfSight(Unit * who)
        {
            Player * const player = who->ToPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(QUEST_LESSONS_OF_BURNING_SCROLL) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(ITEM_MASTERS_FLAME) && !me->HasAura(SPELL_MASTERS_FLAME))
                me->AddAura(SPELL_MASTERS_FLAME, me);
        }

        void UpdateAI(uint32 const diff)
        {
            // In case noone used spellclick - reset questgiver flag in periodic way
            if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
                return;

            if (resetTimer <= diff)
            {
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                resetTimer = 10000;
            }
            else
                resetTimer -= diff;
        }
    };
};

class go_wandering_weapon_rack : public GameObjectScript
{
public:
    go_wandering_weapon_rack() : GameObjectScript("go_wandering_weapon_rack") { }

    bool OnGossipHello(Player* player, GameObject* /*go*/)
    {
        if (player->GetQuestStatus(30027) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(73209))
        {
            player->AddItem(73209, 1);
        }
        else if (player->GetQuestStatus(30033) == QUEST_STATUS_INCOMPLETE && (!player->HasItemCount(76392) || !player->HasItemCount(76390)))
        {
            player->AddItem(76392, 1);
            player->AddItem(76390, 1);
        }
        else if (player->GetQuestStatus(30034) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(73211))
        {
            player->AddItem(73211, 1);
        }
        else if (player->GetQuestStatus(30035) == QUEST_STATUS_INCOMPLETE && (!player->HasItemCount(76393) || !player->HasItemCount(73207)))
        {
            player->AddItem(76393, 1);
            player->AddItem(73207, 1);
        }
        else if (player->GetQuestStatus(30036) == QUEST_STATUS_INCOMPLETE && (!player->HasItemCount(73212) || !player->HasItemCount(73208)))
        {
            player->AddItem(73212, 1);
            player->AddItem(73208, 1);
        }
        else if (player->GetQuestStatus(30037) == QUEST_STATUS_INCOMPLETE && (!player->HasItemCount(73213) || !player->HasItemCount(76391)))
        {
            player->AddItem(73213, 1);
            player->AddItem(76391, 1);
        }
        else if (player->GetQuestStatus(30038) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(73210))
        {
            player->AddItem(73210, 1);
        }

        return true;
    }
};

class mob_training_target : public CreatureScript
{
public:
    mob_training_target() : CreatureScript("mob_training_target") { }
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_training_targetAI(creature);
    }

    struct mob_training_targetAI : public ScriptedAI
    {
        uint32 resetTimer;

        mob_training_targetAI(Creature* creature) : ScriptedAI(creature)
        {
            SetCombatMovement(false);
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            // disable rotate
            me->SetControlled(true, UNIT_STATE_STUNNED);
            // imune to knock aways like blast wave
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);

            resetTimer = 5000;
        }

        void EnterEvadeMode()
        {
            if (_EnterEvadeMode())
                Reset();
        }

        void DamageTaken(Unit* /*doneBy*/, uint32& /*damage*/)
        {
            resetTimer = 5000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (!me->HasUnitState(UNIT_STATE_STUNNED))
                me->SetControlled(true, UNIT_STATE_STUNNED);

            if (resetTimer <= diff)
            {
                EnterEvadeMode();
                resetTimer = 5000;
            }
            else
                resetTimer -= diff;
        }

        void MoveInLineOfSight(Unit* /*who*/) { }
    };
};

class mob_tushui_trainee : public CreatureScript
{
    public:
        mob_tushui_trainee() : CreatureScript("mob_tushui_trainee") { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_tushui_trainee_AI(creature);
        }

        struct mob_tushui_trainee_AI : public ScriptedAI
        {
            mob_tushui_trainee_AI(Creature* creature) : ScriptedAI(creature) {}

            bool isInCombat;
            uint32 punch1;
            uint32 punch2;
            uint32 punch3;

            void Reset()
            {
                punch1 = 1000;
                punch2 = 3500;
                punch3 = 6000;
                isInCombat = false;

                me->SetReactState(REACT_DEFENSIVE);
                me->setFaction(2357);
                me->SetFullHealth();
            }

            void DamageTaken(Unit* attacker, uint32& damage)
            {
                if (me->HealthBelowPctDamaged(16, damage))
                {
                    me->setFaction(35);

                    if (attacker && attacker->GetTypeId() == TYPEID_PLAYER)
                        attacker->ToPlayer()->KilledMonsterCredit(54586, 0);

                    Talk(0);

                    damage = 0;

                    me->DeleteThreatList();
                    me->CombatStop();
                    isInCombat = false;

                    me->SetFullHealth();
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                    me->DespawnOrUnsummon(3000);
                }
            }

            void EnterCombat(Unit* /*unit*/)
            {
                isInCombat = true;
            }

            void JustRespawned()
            {
                Reset();
            }

            void UpdateAI(const uint32 diff)
            {
                if (isInCombat)
                {
                    if (!UpdateVictim())
                        return;

                    DoMeleeAttackIfReady();
                }
                else
                {
                    if (punch1 <= diff)
                    {
                        me->HandleEmoteCommand(35);
                        punch1 = 7500;
                    }
                    else
                        punch1 -= diff;

                    if (punch2 <= diff)
                    {
                        me->HandleEmoteCommand(36);
                        punch2 = 7500;
                    }
                    else
                        punch2 -= diff;

                    if (punch3 <= diff)
                    {
                        me->HandleEmoteCommand(37);
                        punch3 = 7500;
                    }
                    else
                        punch3 -= diff;
                }
            }
        };
};

class mob_huojin_trainee : public CreatureScript
{
    public:
        mob_huojin_trainee() : CreatureScript("mob_huojin_trainee") { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_huojin_traineeAI(creature);
        }

        struct mob_huojin_traineeAI : public ScriptedAI
        {
            mob_huojin_traineeAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            uint8 punch;
            bool isInCombat;

            void Reset()
            {
                punch = urand(500, 3000);
                isInCombat = false;

                me->SetReactState(REACT_DEFENSIVE);
                me->setFaction(2357);
                me->SetFullHealth();
            }

            void DamageTaken(Unit* attacker, uint32& damage)
            {
                if (me->HealthBelowPctDamaged(16, damage))
                {
                    me->setFaction(35);

                    if (attacker && attacker->GetTypeId() == TYPEID_PLAYER)
                        attacker->ToPlayer()->KilledMonsterCredit(54586, 0);

                    Talk(0);

                    damage = 0;

                    me->DeleteThreatList();
                    me->CombatStop();
                    isInCombat = false;

                    me->SetFullHealth();
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                    me->DespawnOrUnsummon(3000);
                }
            }

            void EnterCombat(Unit* /*unit*/)
            {
                isInCombat = true;
            }

            void JustRespawned()
            {
                Reset();
            }

            void UpdateAI(const uint32 diff)
            {
                if (isInCombat)
                {
                    if (!UpdateVictim())
                        return;

                    DoMeleeAttackIfReady();
                }
                else
                {
                    if (punch <= diff)
                    {
                        me->HandleEmoteCommand(35);
                        punch = urand(500, 3000);
                    }
                    else
                        punch -= diff;
                }
            }
        };
};

class boss_jaomin_ro : public CreatureScript
{
public:
    boss_jaomin_ro() : CreatureScript("boss_jaomin_ro") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_jaomin_roAI(creature);
    }

    struct boss_jaomin_roAI : public ScriptedAI
    {
        boss_jaomin_roAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        enum eEvents
        {
            EVENT_JAOMIN_JUMP   = 1,
            EVENT_HIT_CIRCLE    = 2,
            EVENT_FALCON        = 3,
            EVENT_RESET         = 4,
            EVENT_CHECK_AREA    = 5,
        };

        EventMap events;
        bool isInFalcon;

        void EnterCombat(Unit* /*unit*/)
        {
            events.ScheduleEvent(EVENT_JAOMIN_JUMP, 1000);
            events.ScheduleEvent(EVENT_HIT_CIRCLE, 2000);
            events.ScheduleEvent(EVENT_CHECK_AREA, 2500);
        }

        void Reset()
        {
            isInFalcon = false;
            me->SetDisplayId(39755);
            me->setFaction(2357); //mechant!
        }

        void MoveInLineOfSight(Unit * who)
        {
            Player * const player = who->ToPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(29409) != QUEST_STATUS_INCOMPLETE)
                return;

            if (who->GetDistance(me) < 15.f)
            {
                if (me->getStandState() != UNIT_STAND_STATE_STAND)
                {
                    Talk(0);
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                }
            }
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            if (me->HealthBelowPctDamaged(30, damage) && !isInFalcon)
            {
                isInFalcon = true;
                me->SetDisplayId(39796); //faucon
                events.ScheduleEvent(EVENT_FALCON, 1000);
                events.CancelEvent(EVENT_JAOMIN_JUMP);
                events.CancelEvent(EVENT_HIT_CIRCLE);
            }

            if (me->HealthBelowPctDamaged(5, damage))
            {
                me->SetDisplayId(39755);

                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 10.0f);
                for (auto player: playerList)
                    player->KilledMonsterCredit(me->GetEntry(), 0);

                Talk(1);
                EnterEvadeMode();
                me->setFaction(35);
                me->SetFullHealth();
                me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                events.Reset();
                events.ScheduleEvent(EVENT_RESET, 5000);
                damage = 0;
            }

            if (damage > me->GetHealth())
                damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_JAOMIN_JUMP: //on monte
                        if (me->GetVictim())
                            me->CastSpell(me->GetVictim(), 108938, true);
                        events.ScheduleEvent(EVENT_JAOMIN_JUMP, 30000);
                        break;
                    case EVENT_HIT_CIRCLE: //baffe
                        if (me->GetVictim())
                            me->CastSpell(me->GetVictim(), 119301, true);

                        events.ScheduleEvent(EVENT_HIT_CIRCLE, 3000);
                        break;
                    case EVENT_FALCON: //attaque du faucon
                        if (me->GetVictim())
                            me->CastSpell(me->GetVictim(), 108935, true);

                        events.ScheduleEvent(EVENT_FALCON, 4000);
                        break;
                    case EVENT_RESET: //remechant
                        Reset();
                        break;
                    case EVENT_CHECK_AREA:
                        if (me->GetAreaId() != 5843) // Grotte Paisible
                            Reset();
                        break;
                }
            }

            if (UpdateVictim())
                DoMeleeAttackIfReady();
        }
    };
};

class mob_attacker_dimwind : public CreatureScript
{
public:
    mob_attacker_dimwind() : CreatureScript("mob_attacker_dimwind") { }
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_attacker_dimwindAI(creature);
    }

    struct mob_attacker_dimwindAI : public ScriptedAI
    {
        mob_attacker_dimwindAI(Creature* creature) : ScriptedAI(creature) {}

        void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
        {
            if (me->GetHealthPct() < 90 && pDoneBy && pDoneBy->ToCreature() && pDoneBy->ToCreature()->GetEntry() == 54785)
                uiDamage = 0;
        }
    };
};

class mob_min_dimwind : public CreatureScript
{
public:
    mob_min_dimwind() : CreatureScript("mob_min_dimwind") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_min_dimwindAI(creature);
    }

    struct mob_min_dimwindAI : public ScriptedAI
    {
        EventMap events;
        SummonList summons;

        enum eEvents
        {
            EVENT_RESET    = 1,
        };

        mob_min_dimwindAI(Creature* creature) : ScriptedAI(creature), summons(creature) { }

        void Reset()
        {
            ResetMobs();
            me->HandleEmoteCommand(EMOTE_STATE_READY2H);
        }

        void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
        {
            if (me->GetHealthPct() < 25 && pDoneBy && pDoneBy->ToCreature() && pDoneBy->ToCreature()->GetEntry() == 54130)
                uiDamage = 0;
        }

        void JustSummoned(Creature* summoned)
        {
            if (summoned->GetEntry() == 54130)
            {
                summons.Summon(summoned);

                summoned->SetFacingToObject(me);
                summoned->HandleEmoteCommand(EMOTE_STATE_READY2H);
            }
        }

        void ResetMobs()
        {
            events.CancelEvent(EVENT_RESET);
            me->HandleEmoteCommand(EMOTE_STATE_READY2H);

            summons.DespawnAll();

            for(int i = 0; i < 4; ++i)
                me->SummonCreature(54130, me->GetPositionX()-3+rand()%6, me->GetPositionY() + 4 + rand()%4, me->GetPositionZ()+2, 4.9f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
        }

        void MoveInLineOfSight(Unit* who)
        {
            Player * player = who->ToPlayer();
            if (!player || player->GetQuestStatus(29419) != QUEST_STATUS_INCOMPLETE)
                return;

            if (me->GetDistance(who) < 15.f)
            {
                me->HandleEmoteCommand(EMOTE_STATE_STAND);
                Talk(0);
                player->KilledMonsterCredit(54855);
                events.ScheduleEvent(EVENT_RESET, 6000);

                for (auto guid : summons)
                    if (Creature * const creature = Unit::GetCreature(*me, guid))
                        creature->GetMotionMaster()->MoveFleeing(me, 5000);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RESET:
                    {
                        ResetMobs();
                    }
                }
            }
        }
    };
};

// NPC 56661 - The Way of the Tushui
class mob_aysa_lake_escort : public CreatureScript
{
public:
    mob_aysa_lake_escort() : CreatureScript("mob_aysa_lake_escort") { }

    struct mob_aysa_lake_escortAI : public npc_escortAI
    {
        mob_aysa_lake_escortAI(Creature* creature) : npc_escortAI(creature)
        {}

        uint32 IntroTimer;

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            IntroTimer = 2500;
        }

        void MovementInform(uint32 uiType, uint32 uiId)
        {
            npc_escortAI::MovementInform(uiType, uiId);

            if (uiType != POINT_MOTION_TYPE && uiType != EFFECT_MOTION_TYPE)
                return;

            switch (uiId)
            {
                case 10:
                    me->GetMotionMaster()->MoveJump(1227.11f, 3489.73f, 100.37f, 10, 20, 11);
                    break;
                case 11:
                    me->GetMotionMaster()->MoveJump(1236.68f, 3456.68f, 102.58f, 10, 20, 12);
                    break;
                case 12:
                    Start(false, true);
                    break;
                default:
                    break;
            }
        }

        void WaypointReached(uint32 waypointId)
        {
            if (waypointId == 4)
                me->DespawnOrUnsummon(500);
        }

        void UpdateAI(const uint32 diff)
        {
            if (IntroTimer)
            {
                if (IntroTimer <= diff)
                {
                    Talk(0);
                    IntroTimer = 0;
                    me->GetMotionMaster()->MoveJump(1216.78f, 3499.44f, 91.15f, 10, 20, 10);
                }
                else
                    IntroTimer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aysa_lake_escortAI(creature);
    }

};

// NPC 54567
class mob_aysa : public CreatureScript
{
    enum
    {
        QUEST_WAY_OF_THE_TUSHUI = 29414,
        NPC_AYSA_LAKE_ESCORT    = 56661,
        NPC_MASTER_LI_FEI       = 54856,
        SPELL_MEDITATION_BAR    = 116421,
        NPC_TROUBLEMAKER        = 59637,

        SAY_INTRO               = 0,
        SAY_END
    };

    enum eEvents
    {
        EVENT_START = 1,
        EVENT_SPAWN_MOBS = 2,
        EVENT_PROGRESS = 3,
        EVENT_END = 4,
    };

public:
    mob_aysa() : CreatureScript("mob_aysa") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_WAY_OF_THE_TUSHUI)
            if (Creature* tempSummon = creature->SummonCreature(NPC_AYSA_LAKE_ESCORT, *creature, TEMPSUMMON_MANUAL_DESPAWN, 0, player->GetGUID()))
                tempSummon->SetPhaseMask(1, true);

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aysaAI(creature);
    }

    struct mob_aysaAI : public ScriptedAI
    {
        EventMap events;
        std::vector<Player*> playersInvolved;

        uint64 lifeiGUID;

        bool inCombat;
        uint32 timer;

        mob_aysaAI(Creature* creature) : ScriptedAI(creature)
        {
            if (me->GetAreaId() == 5848) // Cave of Meditation
                events.ScheduleEvent(EVENT_START, 600); //Begin script
            inCombat = false;
            timer = 0;
            lifeiGUID = 0;
            me->SetReactState(REACT_DEFENSIVE);
            me->setFaction(2263);
        }

        void DamageTaken(Unit* /*pDoneBy*/, uint32 &uiDamage)
        {
            if (me->HealthBelowPctDamaged(5, uiDamage))
            {
                DespawnLifei();
                uiDamage = 0;
                me->MonsterSay("I can't meditate!", LANG_UNIVERSAL, 0);
                me->SetFullHealth();
                me->SetReactState(REACT_DEFENSIVE);

                std::list<Creature*> unitlist;
                GetCreatureListWithEntryInGrid(unitlist, me, NPC_TROUBLEMAKER, 50.0f);
                for (auto creature: unitlist)
                    me->Kill(creature);

                events.ScheduleEvent(EVENT_START, 20000);
                events.CancelEvent(EVENT_SPAWN_MOBS);
                events.CancelEvent(EVENT_PROGRESS);
                events.CancelEvent(EVENT_END);
            }
        }

        void updatePlayerList()
        {
            playersInvolved.clear();

            std::list<Player*> PlayerList;
            GetPlayerListInGrid(PlayerList, me, 20.0f);

            for (auto player: PlayerList)
                if (!player->isGameMaster() && player->GetQuestStatus(QUEST_WAY_OF_THE_TUSHUI) == QUEST_STATUS_INCOMPLETE)
                    playersInvolved.push_back(player);
        }

        Creature * GetLifei()
        {
            if (lifeiGUID != 0)
                return Creature::GetCreature(*me, lifeiGUID);
            else
            {
                TempSummon * temp = me->SummonCreature(NPC_MASTER_LI_FEI, 1130.162231f, 3435.905518f, 105.496597f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN);
                temp->GetMotionMaster()->Clear();
                temp->GetMotionMaster()->MoveRandom(10.0f);
                lifeiGUID = temp->GetGUID();
                return temp;
            }
        }

        void DespawnLifei()
        {
            if (lifeiGUID)
            {
                if (Creature * lifei = GetLifei())
                {
                    lifei->DespawnOrUnsummon();
                    lifeiGUID = 0;
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_START: //Begin script if playersInvolved is not empty
                    {
                        updatePlayerList();
                        if (playersInvolved.empty())
                            events.ScheduleEvent(EVENT_START, 600); // reschedule
                        else
                        {
                            Talk(SAY_INTRO);
                            me->SetReactState(REACT_PASSIVE);
                            timer = 0;
                            events.ScheduleEvent(EVENT_SPAWN_MOBS, 5000); //spawn mobs
                            events.ScheduleEvent(EVENT_PROGRESS, 1000); //update time
                            events.ScheduleEvent(EVENT_END, 90000); //end quest
                        }
                        break;
                    }
                    case EVENT_SPAWN_MOBS: //Spawn 3 mobs
                    {
                        updatePlayerList();

                        auto const maxSize = std::min<size_t>(playersInvolved.size(), 5);
                        auto const maxSpawns = std::max<size_t>(maxSize * 3, 3);

                        for(size_t i = 0; i < maxSpawns; ++i)
                            if (TempSummon* temp = me->SummonCreature(NPC_TROUBLEMAKER, 1171.71f, 3443.82f, 104.20f, 3.3f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000))
                            {
                                temp->AddThreat(me, 250.0f);

                                temp->SetInCombatWith(me);
                                me->SetInCombatWith(me);
                                temp->GetMotionMaster()->MoveChase(me);
                            }

                        events.ScheduleEvent(EVENT_SPAWN_MOBS, 20000); //spawn mobs
                        break;
                    }
                    case EVENT_PROGRESS: //update energy
                    {
                        timer++;

                        uint8 lafeiTalkThreshold[7] = { 25, 30, 42, 54, 66, 78, 85 };

                        for (int i = 0; i < 7; ++i)
                            if (timer == lafeiTalkThreshold[i])
                            {
                                Creature * lifei = GetLifei();
                                lifei->AI()->Talk(i);

                                if (i == 6)
                                {
                                    lifei->DespawnOrUnsummon(500);
                                    lifeiGUID = 0;
                                }
                                break;
                            }

                        updatePlayerList();
                        for (auto player: playersInvolved)
                        {
                            if (!player->HasAura(SPELL_MEDITATION_BAR))
                                player->CastSpell(player, SPELL_MEDITATION_BAR);

                            player->SetPower(POWER_ALTERNATE_POWER, timer);
                            player->SetMaxPower(POWER_ALTERNATE_POWER, 90);
                        }

                        events.ScheduleEvent(EVENT_PROGRESS, 1000);
                        break;
                    }
                    case EVENT_END: //script end
                    {
                        if (Creature * lifei = GetLifei())
                        {
                            lifei->DespawnOrUnsummon();
                            lifeiGUID = 0;
                        }

                        events.ScheduleEvent(EVENT_START, 10000);
                        events.CancelEvent(EVENT_SPAWN_MOBS);
                        events.CancelEvent(EVENT_PROGRESS);
                        Talk(SAY_END);
                        me->SetReactState(REACT_DEFENSIVE);

                        updatePlayerList();
                        for(auto player: playersInvolved)
                        {
                            player->KilledMonsterCredit(NPC_MASTER_LI_FEI, 0);
                            player->RemoveAura(SPELL_MEDITATION_BAR);
                        }
                        break;
                    }
                }
            }
        }
    };
};

class boss_living_air : public CreatureScript
{
public:
    boss_living_air() : CreatureScript("boss_living_air") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_living_airAI(creature);
    }

    struct boss_living_airAI : public ScriptedAI
    {
        boss_living_airAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        EventMap events;

        void EnterCombat(Unit* /*unit*/)
        {
            events.ScheduleEvent(1, 3000);
            events.ScheduleEvent(2, 5000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case 1:
                        me->CastSpell(me->GetVictim(), 108693);
                        break;
                    case 2:
                        me->CastSpell(me->GetVictim(), 73212);
                        events.ScheduleEvent(2, 5000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class boss_li_fei : public CreatureScript
{
public:
    boss_li_fei() : CreatureScript("boss_li_fei") {}

    bool OnQuestAccept(Player* /*player*/, Creature* /*creature*/, Quest const* /*quest*/)
    {
        // TODO: there is some kick spell casted on retail at quest accept
        return true;
    }
};

class boss_li_fei_fight : public CreatureScript
{
    struct boss_li_fei_fightAI : public ScriptedAI
    {
        enum
        {
            EVENT_FEET_OF_FURY      = 1,
            EVENT_SHADOW_KICK       = 2,
            EVENT_SHADOW_KICK_STUN  = 3,

            QUEST_ONLY_THE_WORTHY_SHALL_PASS = 29421,
        };

        EventMap events;

        boss_li_fei_fightAI(Creature *creature)
            : ScriptedAI(creature)
        { }

        void Reset()
        {
            me->SetFullHealth();

            events.Reset();
            events.ScheduleEvent(EVENT_FEET_OF_FURY, 5000);
            events.ScheduleEvent(EVENT_SHADOW_KICK,  1000);
        }

        // Li Fei does not aggro himself
        void MoveInLineOfSight(Unit *) { }

        void DamageTaken(Unit* /*attacker*/, uint32 &damage)
        {
            if (!me->HealthBelowPctDamaged(10, damage))
                return;

            damage = 0;

            // copy, just to make sure that kill credit that phases players out
            // does not modify this threat list
            auto const threatList = me->getThreatManager().getThreatList();
            for (auto &hostileRef : threatList)
            {
                auto unit = hostileRef->getTarget();
                if (unit && unit->GetTypeId() == TYPEID_PLAYER)
                    unit->ToPlayer()->KilledMonsterCredit(54734, 0);
            }

            // TODO: it seems that spell 106275 is used to display a text message
            // after fight is over

            me->DeleteThreatList();
            me->CombatStop(true);

            Reset();
        }

        void KilledUnit(Unit *victim)
        {
            // Player managed to die with Blessings of Fire on
            if (auto const player = victim->ToPlayer())
                player->SetQuestStatus(QUEST_ONLY_THE_WORTHY_SHALL_PASS, QUEST_STATUS_FAILED);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (auto const eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FEET_OF_FURY:
                        if (auto const victim = me->GetVictim())
                            me->CastSpell(victim, 108958);
                        events.ScheduleEvent(EVENT_FEET_OF_FURY, 13000);
                        break;
                    case EVENT_SHADOW_KICK:
                        if (auto const victim = me->GetVictim())
                            me->CastSpell(victim, 108936);
                        events.ScheduleEvent(EVENT_SHADOW_KICK_STUN, 2500);
                        events.ScheduleEvent(EVENT_SHADOW_KICK, 30000);
                        break;
                    case EVENT_SHADOW_KICK_STUN:
                        if (auto const victim = me->GetVictim())
                            me->CastSpell(victim, 108944);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

public:
    boss_li_fei_fight() : CreatureScript("boss_li_fei_fight") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_li_fei_fightAI(creature);
    }
};

// Huo Benediction - 102630
class spell_huo_benediction: public SpellScriptLoader
{
    public:
        spell_huo_benediction() : SpellScriptLoader("spell_huo_benediction") { }

        class spell_huo_benediction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_huo_benediction_AuraScript);

            void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();

                if (!target)
                    return;

                std::list<Creature*> huoList;
                GetCreatureListWithEntryInGrid(huoList, target, 54958, 20.0f);

                for (auto huo: huoList)
                    if (huo->ToTempSummon())
                        if (huo->ToTempSummon()->GetOwnerGUID() == target->GetGUID())
                            return;

                // A partir d'ici on sait que le joueur n'a pas encore de Huo
                if (TempSummon* tempHuo = target->SummonCreature(54958, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0))
                {
                    tempHuo->setExplicitSeerGuid(target->GetGUID());
                    tempHuo->SetOwnerGUID(target->GetGUID());
                    tempHuo->GetMotionMaster()->MoveFollow(target, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                }
            }

            void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();

                if (!target)
                    return;

                std::list<Creature*> huoList;
                GetCreatureListWithEntryInGrid(huoList, target, 54958, 20.0f);

                for (auto huo: huoList)
                    if (huo->ToTempSummon())
                        if (huo->ToTempSummon()->GetOwnerGUID() == target->GetGUID())
                            huo->DespawnOrUnsummon();
            }

            void Register()
            {
                OnEffectApply  += AuraEffectApplyFn (spell_huo_benediction_AuraScript::OnApply,  EFFECT_0, SPELL_AURA_MOD_INCREASE_SPEED, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectRemove += AuraEffectRemoveFn(spell_huo_benediction_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_INCREASE_SPEED, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_huo_benediction_AuraScript();
        }
};

class AreaTrigger_at_temple_entrance : public AreaTriggerScript
{
    public:
        AreaTrigger_at_temple_entrance() : AreaTriggerScript("AreaTrigger_at_temple_entrance")
        {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/)
        {
            if (player->GetQuestStatus(29423) == QUEST_STATUS_INCOMPLETE)
            {
                player->KilledMonsterCredit(61128, 0);

                std::list<Creature*> huoList;
                GetCreatureListWithEntryInGrid(huoList, player, 54958, 20.0f);

                for (auto huo: huoList)
                {
                    if (huo->ToTempSummon())
                    {
                        if (huo->ToTempSummon()->GetOwnerGUID() == player->GetGUID())
                        {
                            huo->GetMotionMaster()->Clear();
                            huo->GetMotionMaster()->MovePoint(1, 950.0f, 3601.0f, 203.0f);
                            huo->DespawnOrUnsummon(5000);
                        }
                    }
                }
            }

            return true;
        }
};

class mob_trainee_nim : public CreatureScript
{
public:
    mob_trainee_nim() : CreatureScript("mob_trainee_nim") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_trainee_nimAI(creature);
    }

    struct mob_trainee_nimAI : public ScriptedAI
    {
        mob_trainee_nimAI(Creature* creature) : ScriptedAI(creature)
        {
        }
        std::set<uint64> guids;

        void MoveInLineOfSight(Unit * who)
        {
            Player * const player = who->ToPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(29409) != QUEST_STATUS_INCOMPLETE)
                return;

            if (who->GetDistance(me) < 20.f)
            {
                if (guids.find(player->GetGUID()) == guids.end())
                {
                    Talk(0, player->GetGUID());
                    guids.insert(player->GetGUID());
                }
            }
        }
    };
};

class mob_merchant_lorvo : public CreatureScript
{
public:
    mob_merchant_lorvo() : CreatureScript("mob_merchant_lorvo") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_merchant_lorvoAI(creature);
    }

    struct mob_merchant_lorvoAI : public ScriptedAI
    {
        mob_merchant_lorvoAI(Creature* creature) : ScriptedAI(creature)
        {
        }
        std::set<uint64> guids;

        void MoveInLineOfSight(Unit * who)
        {
            Player * const player = who->ToPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(29410) == QUEST_STATUS_COMPLETE)
            {
                if (who->GetDistance(me) < 20.f)
                {
                    if (guids.find(player->GetGUID()) == guids.end())
                    {
                        Talk(0, player->GetGUID());
                        guids.insert(player->GetGUID());
                    }
                }
            }
        }
    };
};

class mob_instructors : public CreatureScript
{
public:
    mob_instructors() : CreatureScript("mob_instructors") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_instructorsAI(creature);
    }

    struct mob_instructorsAI : public ScriptedAI
    {
        EventMap events;
        uint8 rand;

        mob_instructorsAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        uint32 punch1;
        uint32 punch2;
        uint32 punch3;

        void Reset()
        {
            punch1 = 300;
            punch2 = 2800;
            punch3 = 5300;
        }

        void UpdateAI(const uint32 diff)
        {
            if (punch1 <= diff)
            {
                me->HandleEmoteCommand(35);
                punch1 = 7500;
            }
            else
                punch1 -= diff;

            if (punch2 <= diff)
            {
                me->HandleEmoteCommand(36);
                punch2 = 7500;
            }
            else
                punch2 -= diff;

            if (punch3 <= diff)
            {
                me->HandleEmoteCommand(37);
                punch3 = 7500;
            }
            else
                punch3 -= diff;
        }
    };
};

class mob_aspiring_trainee : public CreatureScript
{
public:
    mob_aspiring_trainee() : CreatureScript("mob_aspiring_trainee") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aspiring_traineeAI(creature);
    }

    struct mob_aspiring_traineeAI : public ScriptedAI
    {
        mob_aspiring_traineeAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        uint32 punch1;
        uint32 punch2;
        uint32 punch3;

        void Reset()
        {
            punch1 = 1000;
            punch2 = 3500;
            punch3 = 6000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (punch1 <= diff)
            {
                me->HandleEmoteCommand(35);
                punch1 = 7500;
            }
            else
                punch1 -= diff;

            if (punch2 <= diff)
            {
                me->HandleEmoteCommand(36);
                punch2 = 7500;
            }
            else
                punch2 -= diff;

            if (punch3 <= diff)
            {
                me->HandleEmoteCommand(37);
                punch3 = 7500;
            }
            else
                punch3 -= diff;
        }
    };
};

class spell_lit_brazier_of_flame final : public SpellScriptLoader
{
    class script_impl final : public SpellScript
    {
        PrepareSpellScript(script_impl)

        enum
        {
            QUEST_THE_CHALLENGERS_FIRES = 29664
        };

        SpellCastResult CheckCast()
        {
            auto const caster = GetCaster()->ToPlayer();
            if (!caster)
                return SPELL_FAILED_DONT_REPORT;
            switch (caster->GetQuestStatus(QUEST_THE_CHALLENGERS_FIRES))
            {
                case QUEST_STATUS_NONE:
                    return SPELL_FAILED_DONT_REPORT;
                case QUEST_STATUS_COMPLETE:
                case QUEST_STATUS_REWARDED:
                    return SPELL_CAST_OK;
                default:
                    // For spell_area casts only
                    if (GetSpell()->IsTriggered())
                    {
                        auto const killCredit = GetSpellInfo()->Effects[EFFECT_2].MiscValue;
                        if (caster->GetReqKillOrCastCurrentCount(QUEST_THE_CHALLENGERS_FIRES, killCredit) != 0)
                            return SPELL_FAILED_DONT_REPORT;

                    }
                    return SPELL_CAST_OK;
            }
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(script_impl::CheckCast);
        }
    };

public:
    spell_lit_brazier_of_flame()
        : SpellScriptLoader("spell_lit_brazier_of_flame")
    { }

    SpellScript * GetSpellScript() const final
    {
        return new script_impl;
    }
};

class spell_feet_of_fury final : public SpellScriptLoader
{
    class script_impl final : public AuraScript
    {
        PrepareAuraScript(script_impl)

        uint64 targetGuid;

        bool Load() final
        {
            targetGuid = 0;
            return true;
        }

        void OnApply(AuraEffect const *, AuraEffectHandleModes)
        {
            auto const caster = GetCaster();
            if (!caster)
                return;

            if (auto const target = caster->GetVictim())
                targetGuid = target->GetGUID();
        }

        void PeriodicTick(AuraEffect const *)
        {
            PreventDefaultAction();

            if (targetGuid == 0)
                return;

            auto const caster = GetCaster();
            auto const target = ObjectAccessor::GetUnit(*caster, targetGuid);

            if (!target)
            {
                targetGuid = 0;
                return;
            }

            auto const spellId = GetSpellInfo()->Effects[EFFECT_0].TriggerSpell;
            caster->CastSpell(target, spellId, true);
        }

        void Register() final
        {
            OnEffectApply += AuraEffectApplyFn(script_impl::OnApply,  EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

public:
    spell_feet_of_fury()
        : SpellScriptLoader("spell_feet_of_fury")
    { }

    AuraScript * GetAuraScript() const final
    {
        return new script_impl;
    }
};

void AddSC_WanderingIsland_North()
{
    new mob_master_shang_xi();
    new go_wandering_weapon_rack();
    new mob_training_target();
    new mob_tushui_trainee();
    new mob_huojin_trainee();
    new boss_jaomin_ro();
    new mob_attacker_dimwind();
    new mob_min_dimwind();
    new mob_aysa_lake_escort();
    new mob_aysa();
    new boss_living_air();
    new boss_li_fei();
    new boss_li_fei_fight();
    new spell_huo_benediction();
    new AreaTrigger_at_temple_entrance();
    new mob_trainee_nim();
    new mob_instructors();
    new mob_aspiring_trainee();
    new mob_merchant_lorvo();
    new spell_lit_brazier_of_flame();
    new spell_feet_of_fury();
}
