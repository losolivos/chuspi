#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "SpellScript.h"
#include "Vehicle.h"

#define GOSSIP_WIND     "I would like to go back on the top of the temple"

class mob_master_shang_xi_temple : public CreatureScript
{
    public:
        mob_master_shang_xi_temple() : CreatureScript("mob_master_shang_xi_temple") { }

        bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest)
        {
            if (quest->GetQuestId() == 29776) // Morning Breeze Village
            {
                if (Creature* vehicle = player->SummonCreature(55685, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()))
                {
                    player->AddAura(99385, vehicle);
                    player->EnterVehicle(vehicle);
                }
            }

            return true;
        }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (creature->IsQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            if (player->GetQuestStatus(29776) != QUEST_STATUS_NONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_WIND, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            player->PlayerTalkClass->SendGossipMenu(1, creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
        {
            if (action == GOSSIP_ACTION_INFO_DEF + 1)
            {
                /* The vehicle bug for now on TaranZhu, too much lags
                 *if (Creature* vehicle = player->SummonCreature(55685, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()))
                {
                    player->AddAura(99385, vehicle);
                    player->EnterVehicle(vehicle);
                }*/

                player->NearTeleportTo(926.58f, 3605.33f, 251.63f, 3.114f);
            }

            player->PlayerTalkClass->SendCloseGossip();
            return true;
        }
};

class npc_wind_vehicle : public CreatureScript
{
public:
    npc_wind_vehicle() : CreatureScript("npc_wind_vehicle") { }

    struct npc_wind_vehicleAI : public npc_escortAI
    {
        npc_wind_vehicleAI(Creature* creature) : npc_escortAI(creature)
        {}

        uint32 IntroTimer;

        void Reset()
        {
            IntroTimer = 100;
        }

        void WaypointReached(uint32 waypointId)
        {
            if (waypointId == 6)
            {
                if (me->GetVehicleKit())
                    me->GetVehicleKit()->RemoveAllPassengers();

                me->DespawnOrUnsummon();
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (IntroTimer)
            {
                if (IntroTimer <= diff)
                {
                    Start(false, true);
                    IntroTimer = 0;
                }
                else
                    IntroTimer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wind_vehicleAI(creature);
    }
};

struct AreaTrigger_at_wind_temple_entrance final : public AreaTriggerScript
{
    enum
    {
        SPELL_SUMMON_AYSA_MARKER = 104571
    };

    AreaTrigger_at_wind_temple_entrance()
        : AreaTriggerScript("AreaTrigger_at_wind_temple_entrance")
    { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/) final
    {
        if (player->GetQuestStatus(29785) == QUEST_STATUS_INCOMPLETE && !player->HasAura(SPELL_SUMMON_AYSA_MARKER))
            player->CastSpell(player, SPELL_SUMMON_AYSA_MARKER, true);
        return true;
    }
};

// @todo - script texts
class mob_aysa_wind_temple_escort final : public CreatureScript
{
    struct mob_aysa_wind_temple_escortAI final : public npc_escortAI
    {
        enum
        {
            SPELL_SUMMON_AYSA_MARKER = 104571
        };

        mob_aysa_wind_temple_escortAI(Creature* creature)
            : npc_escortAI(creature)
        {
            if (auto const summon = me->ToTempSummon())
                summon->setExplicitSeerGuid(summon->GetSummonerGUID());
        }

        uint32 introTimer_;

        void Reset() final
        {
            introTimer_ = 100;
            me->SetReactState(REACT_PASSIVE);
        }

        void DoAction(const int32 /*param*/) final
        {
            SetEscortPaused(false);
        }

        void WaypointReached(uint32 waypointId) final
        {
            switch (waypointId)
            {
                case 2:
                    SetEscortPaused(true);
                    me->SetFacingTo(2.38f);
                    break;
                case 7:
                    SetEscortPaused(true);
                    break;
                case 10:
                {
                    auto const summoner = me->ToTempSummon()->GetSummoner();
                    if (summoner && summoner->GetTypeId() == TYPEID_PLAYER)
                    {
                        auto const player = summoner->ToPlayer();

                        player->KilledMonsterCredit(55666);
                        player->AreaExploredOrEventHappens(29785);
                        player->RemoveAurasDueToSpell(SPELL_SUMMON_AYSA_MARKER);
                    }

                    SetEscortPaused(true);
                    me->DespawnOrUnsummon(10000);
                    break;
                }
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff) final
        {
            if (introTimer_)
            {
                if (introTimer_ <= diff)
                {
                    Start(false, true);
                    introTimer_ = 0;
                }
                else
                    introTimer_ -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };

public:
    mob_aysa_wind_temple_escort()
        : CreatureScript("mob_aysa_wind_temple_escort")
    { }

    CreatureAI * GetAI(Creature* creature) const final
    {
        return new mob_aysa_wind_temple_escortAI(creature);
    }
};

class mob_frightened_wind : public CreatureScript
{
public:
    mob_frightened_wind() : CreatureScript("mob_frightened_wind") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_frightened_windAI(creature);
    }

    struct mob_frightened_windAI : public ScriptedAI
    {
        mob_frightened_windAI(Creature* creature) : ScriptedAI(creature)
        {}

        uint32 tornadeTimer;

        enum Spells
        {
            SPELL_TORNADE    = 107278,
        };

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            tornadeTimer = 8 * IN_MILLISECONDS;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tornadeTimer <= diff)
            {
                me->ToggleAura(SPELL_TORNADE, me);

                if (!me->HasAura(SPELL_TORNADE))
                {
                    std::list<Creature*> aysaList;
                    GetCreatureListWithEntryInGrid(aysaList, me, 55744, 50.0f);

                    for (auto aysa: aysaList)
                        aysa->AI()->DoAction(1);
                }
                tornadeTimer = 8 * IN_MILLISECONDS;
            }
            else
                tornadeTimer -= diff;
        }
    };
};

class npc_aysa_in_wind_temple : public CreatureScript
{
    public:
        npc_aysa_in_wind_temple() : CreatureScript("npc_aysa_in_wind_temple") { }

        bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest)
        {
            if (quest->GetQuestId() == 29786) // Bataille Pyrotechnique
            {
                if (Creature* aysa = player->SummonCreature(64543, 543.94f, 4317.31f, 212.24f, 1.675520f, TEMPSUMMON_TIMED_DESPAWN, 10000))
                {
                    aysa->setExplicitSeerGuid(player->GetGUID());
                    aysa->GetMotionMaster()->MovePoint(1, 643.45f, 4228.66f, 202.90f);
                }

                if (Creature* dafeng = player->SummonCreature(64532, 543.56f, 4320.97f, 212.24f, 5.445430f, TEMPSUMMON_TIMED_DESPAWN, 10000))
                {
                    dafeng->setExplicitSeerGuid(player->GetGUID());
                    dafeng->GetMotionMaster()->MovePoint(1, 643.45f, 4228.66f, 202.90f);
                }
            }

            return true;
        }
};

enum Enums
{
    NPC_ROCKET_LAUNCHER = 64507,
    SPELL_ROCKET_LAUNCH = 104855,

    EVENT_NEXT_MOVEMENT = 1,
    EVENT_STUNNED       = 2,
    EVENT_LIGHTNING     = 3,

    POINT_STUN          = 100,

    SPELL_SERPENT_SWEEP = 125990,
    SPELL_STUNNED       = 125992,
    SPELL_LIGHTNING     = 126006,
};

static Position const ZhaoPos[] =
{
    {719.36f, 4164.60f, 216.06f, 0.0f}, // Center
    {745.91f, 4154.35f, 223.48f, 0.0f},
    {717.04f, 4141.16f, 219.83f, 0.0f},
    {689.62f, 4153.16f, 217.63f, 0.0f},
    {684.53f, 4173.24f, 216.98f, 0.0f},
    {704.77f, 4190.16f, 218.24f, 0.0f},
    {736.90f, 4183.85f, 221.41f, 0.0f}
};

static const Position zhaoStunPos = {723.163025f, 4163.799805f, 202.082993f, 0.0f };

class boss_zhao_ren : public CreatureScript
{
public:
    boss_zhao_ren() : CreatureScript("boss_zhao_ren") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_zhao_renAI(creature);
    }

    struct boss_zhao_renAI : public ScriptedAI
    {
        boss_zhao_renAI(Creature* creature) : ScriptedAI(creature)
        {}

        EventMap _events;
        bool eventStarted;
        uint8 hitCount;
        uint8 currentPos;

        void Reset()
        {
            _events.Reset();
            me->SetReactState(REACT_PASSIVE);
            me->SetDisableGravity(true);

            eventStarted = false;
            hitCount = 0;
            currentPos = 0;

            me->SetFullHealth();
            me->RemoveAurasDueToSpell(SPELL_STUNNED);

            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(0, ZhaoPos[0].GetPositionX(), ZhaoPos[0].GetPositionY(), ZhaoPos[0].GetPositionZ());
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_ROCKET_LAUNCH)
            {
                if (++hitCount >= 5)
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(POINT_STUN, zhaoStunPos);
                    hitCount = 0;
                }
            }
        }

        bool checkPlayers()
        {
            std::list<Player*> playerList;
            GetPlayerListInGrid(playerList, me, 80.0f);

            for (auto player : playerList)
                if (player->GetQuestStatus(29786) == QUEST_STATUS_INCOMPLETE)
                    if (player->IsAlive())
                        return true;

            return false;
        }

        void GoToNextPos()
        {
            if (++currentPos > 6)
                currentPos = 1;

            me->GetMotionMaster()->MovePoint(currentPos, ZhaoPos[currentPos].GetPositionX(), ZhaoPos[currentPos].GetPositionY(), ZhaoPos[currentPos].GetPositionZ());
        }

        Player* GetRandomPlayer()
        {
            std::list<Player*> playerList;
            GetPlayerListInGrid(playerList, me, 50.0f);

            if (playerList.empty())
                return NULL;

            Trinity::Containers::RandomResizeList(playerList, 1);

            return *playerList.begin();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (!id)
                return;

            if (id == POINT_STUN)
            {
                me->CastSpell(me, SPELL_STUNNED, true);
                _events.RescheduleEvent(EVENT_LIGHTNING, 17000);
                _events.ScheduleEvent(EVENT_STUNNED, 12000);
            }
            else
                _events.ScheduleEvent(EVENT_NEXT_MOVEMENT, 200);
        }

        void JustDied(Unit* /*attacker*/)
        {
            std::list<Player*> playerList;
            GetPlayerListInGrid(playerList, me, 50.0f);

            for (auto player : playerList)
                if (player->GetQuestStatus(29786) == QUEST_STATUS_INCOMPLETE)
                    if (player->IsAlive())
                        player->KilledMonsterCredit(me->GetEntry());
        }

        void UpdateAI(const uint32 diff)
        {
            if (checkPlayers())
            {
                if (!eventStarted)  // Event not started, player found
                {
                    _events.ScheduleEvent(EVENT_NEXT_MOVEMENT, 1000);
                    _events.ScheduleEvent(EVENT_LIGHTNING, 5000);
                    eventStarted = true;
                }
            }
            else
            {
                if (eventStarted)  // Event started, no player found
                    Reset();

                return;
            }

            _events.Update(diff);

            switch (_events.ExecuteEvent())
            {
                case EVENT_NEXT_MOVEMENT:
                {
                    if (me->HasAura(SPELL_STUNNED))
                        _events.ScheduleEvent(EVENT_NEXT_MOVEMENT, 2000);

                    GoToNextPos();
                    break;
                }
                case EVENT_STUNNED:
                {
                    me->RemoveAurasDueToSpell(SPELL_STUNNED);
                    me->CastSpell(me, SPELL_SERPENT_SWEEP, false);
                    me->SetDisableGravity(true);
                    _events.ScheduleEvent(EVENT_NEXT_MOVEMENT, 4000);
                    break;
                }
                case EVENT_LIGHTNING:
                {
                    if (Player* player = GetRandomPlayer())
                        me->CastSpell(player, SPELL_LIGHTNING, false);

                    _events.ScheduleEvent(EVENT_LIGHTNING, 5000);
                    break;
                }
            }
        }
    };
};

class npc_rocket_launcher : public CreatureScript
{
public:
    npc_rocket_launcher() : CreatureScript("npc_rocket_launcher") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rocket_launcherAI (creature);
    }

    struct npc_rocket_launcherAI : public ScriptedAI
    {
        npc_rocket_launcherAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 cooldown;

        void Reset()
        {
            cooldown = 0;
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void OnSpellClick(Unit* Clicker, bool &/*result*/)
        {
            if (cooldown)
                return;

            Creature* const zhao = GetClosestCreatureWithEntry(me, 55786, 50.0f);

            if (zhao && zhao->IsWithinDist2d(Clicker, 10.0f))
                me->CastSpell(zhao, SPELL_ROCKET_LAUNCH, false);
            else
            {
                Clicker->ToPlayer()->GetSession()->SendNotification("Wait until the Onyx Serpent is directly overhead.");
                return;
            }

            cooldown = 5000;
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void EnterCombat(Unit* /*who*/)
        {
            return;
        }

        void UpdateAI(const uint32 diff)
        {
            if (cooldown)
            {
                if (cooldown <= diff)
                {
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    cooldown = 0;
                }
                else
                    cooldown -= diff;
            }
        }
    };
};

class mob_master_shang_xi_after_zhao : public CreatureScript
{
    public:
        mob_master_shang_xi_after_zhao() : CreatureScript("mob_master_shang_xi_after_zhao") { }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
        {
            if (quest->GetQuestId() == 29787) // Worthy of Passing
            {
                if (Creature* master = player->SummonCreature(56159, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0))
                {
                    master->setExplicitSeerGuid(player->GetGUID());
                    master->AI()->SetGUID(player->GetGUID());
                }
            }

            return true;
        }
};

// 56159 - Worthy of Passing(29787)
class mob_master_shang_xi_after_zhao_escort : public CreatureScript
{
    public:
        mob_master_shang_xi_after_zhao_escort() : CreatureScript("mob_master_shang_xi_after_zhao_escort") { }

    struct mob_master_shang_xi_after_zhao_escortAI : public npc_escortAI
    {
        mob_master_shang_xi_after_zhao_escortAI(Creature* creature) : npc_escortAI(creature)
        {
            summonGUID = 0;
        }

        uint32 IntroTimer;

        uint64 playerGuid;

        void Reset()
        {
            IntroTimer = 250;
            me->SetReactState(REACT_PASSIVE);
        }

        void SetGUID(uint64 guid, int32)
        {
            playerGuid = guid;
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 1:
                    Talk(0);
                    break;
                case 4:
                    Talk(1);
                    break;
                case 6:
                    Talk(2);
                    break;
                case 7:
                    me->SetWalk(true);
                    break;
                case 8:
                    Talk(3);
                    break;
                case 10:
                    if (Creature * creature = me->SummonCreature(56274, 845.89f, 4372.62f, 223.98f, 4.78f, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    {
                        creature->setExplicitSeerGuid(playerGuid);
                        summonGUID = creature->GetGUID();
                        creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        creature->SetReactState(REACT_DEFENSIVE);
                    }
                    break;
                case 15:
                    me->SetFacingTo(5.91f);
                    if (Creature * creature = Creature::GetCreature(*me, summonGUID))
                        creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    SetEscortPaused(true);
                    me->SetWalk(false);
                    break;
                case 16:
                    me->SetFacingTo(4.537860f);

                    if (Player* owner = ObjectAccessor::GetPlayer(*me, playerGuid))
                        owner->AddAura(59074, owner);
                    break;
                case 17:
                    Talk(5);
                    break;
                case 21:
                    me->DespawnOrUnsummon(1000);
                    break;
                default:
                    break;
            }
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            if (summon->GetEntry() == 56274)
            {
                SetEscortPaused(false);
                Talk(4);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (IntroTimer)
            {
                if (IntroTimer <= diff)
                {
                    Start(false, true);
                    IntroTimer = 0;
                }
                else
                    IntroTimer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    private:
        uint64 summonGUID;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_master_shang_xi_after_zhao_escortAI(creature);
    }
};

class mob_master_shang_xi_thousand_staff : public CreatureScript
{
    public:
        mob_master_shang_xi_thousand_staff() : CreatureScript("mob_master_shang_xi_thousand_staff") { }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
        {
            if (quest->GetQuestId() == 29790) // Passing Wisdom
            {
                if (Creature* master = player->SummonCreature(56686, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0))
                {
                    master->setExplicitSeerGuid(player->GetGUID());
                    master->AI()->SetGUID(player->GetGUID());
                }
            }

            return true;
        }
};

// Passing Wisdom (29790)
class mob_master_shang_xi_thousand_staff_escort : public CreatureScript
{
    public:
        mob_master_shang_xi_thousand_staff_escort() : CreatureScript("mob_master_shang_xi_thousand_staff_escort") { }

    struct mob_master_shang_xi_thousand_staff_escortAI : public ScriptedAI
    {
        mob_master_shang_xi_thousand_staff_escortAI(Creature* creature) : ScriptedAI(creature)
        {}

        uint32 IntroTimer;
        uint8 phase;

        uint64 playerGuid;

        void Reset()
        {
            phase = 0;
            IntroTimer = 250;
            me->SetReactState(REACT_PASSIVE);
        }

        void SetGUID(uint64 guid, int32)
        {
            playerGuid = guid;
        }

        void UpdateAI(const uint32 diff)
        {
            if (IntroTimer < diff)
            {
                if (phase < 6)
                    Talk(phase, playerGuid);

                IntroTimer = 15000;
                ++phase;

                if (phase == 7)
                {
                    auto const creature = me->SummonCreature(57874, 873.09f, 4462.25f, 241.27f, 3.80f, TEMPSUMMON_TIMED_DESPAWN, 60000);
                    creature->setExplicitSeerGuid(playerGuid);

                    me->ForcedDespawn();

                    if (Player* owner = ObjectAccessor::GetPlayer(*me, playerGuid))
                        owner->KilledMonsterCredit(56688);
                }
            }
            else IntroTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_master_shang_xi_thousand_staff_escortAI(creature);
    }
};

// Grab Air Balloon - 95247
class spell_grab_air_balloon: public SpellScriptLoader
{
    public:
        spell_grab_air_balloon() : SpellScriptLoader("spell_grab_air_balloon") { }

        class spell_grab_air_balloon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_grab_air_balloon_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                PreventHitAura();

                if (Unit* caster = GetCaster())
                    if (Creature* balloon = caster->SummonCreature(55649, 915.55f, 4563.66f, 230.68f, 2.298090f, TEMPSUMMON_MANUAL_DESPAWN, 0))
                    {
                        balloon->setExplicitSeerGuid(caster->GetGUID());
                        caster->EnterVehicle(balloon, 0);
                    }
            }

            void Register()
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_grab_air_balloon_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_grab_air_balloon_SpellScript();
        }
};

class mob_shang_xi_air_balloon final : public VehicleScript
{
    struct mob_shang_xi_air_balloonAI final : public npc_escortAI
    {
        mob_shang_xi_air_balloonAI(Creature* creature)
            : npc_escortAI(creature)
        { }

        uint64 playerGUID;
        uint32 eventTimer;
        uint32 phase;

        void Reset() final
        {
            playerGUID = 0;
            eventTimer = 250;
            phase = 0;

            me->setActive(true);
            me->SetReactState(REACT_PASSIVE);
        }

        void SetGUID(uint64 guid, int32) final
        {
            playerGUID = guid;
        }

        void removeNpcPassengers()
        {
            for (auto i = 1; i != 3; ++i)
            {
                auto const passenger = me->GetVehicleKit()->GetPassenger(i);
                if (!passenger)
                    continue;

                passenger->_ExitVehicle();
                passenger->ToCreature()->DespawnOrUnsummon(1000);
            }
        }

        void WaypointReached(uint32 waypointId) final
        {
            switch (waypointId)
            {
                case 11:
                    removeNpcPassengers();
                    break;
                case 12:
                {
                    auto const passenger = me->GetVehicleKit()->GetPassenger(0);
                    if (passenger && passenger->GetTypeId() == TYPEID_PLAYER)
                    {
                        passenger->ToPlayer()->KilledMonsterCredit(55939);
                        passenger->AddAura(50550, passenger);
                    }

                    me->GetVehicleKit()->RemoveAllPassengers();
                    break;
                }
            }
        }

        /*
        seat 0 = player
        seat 1 = Ji Firepaw
        seat 2 = Aysa Cloudsinger
        */

        void UpdateAI(const uint32 diff) final
        {
            if (playerGUID == 0)
            {
                removeNpcPassengers();
                me->DespawnOrUnsummon();
                return;
            }

            if (phase <= 24)
            {
                if (eventTimer <= diff)
                {
                    if (phase == 0)
                    {
                        Start(false, true);
                        eventTimer = 2000;
                    }
                    else if (phase == 1)
                    {
                        passengerTalk(0, 1);
                        eventTimer = 5000;
                    }
                    else if (phase == 2)
                    {
                        passengerTalk(0, 2);
                        eventTimer = 8000;
                    }
                    else if (phase == 3)
                    {
                        passengerTalk(1, 1);
                        eventTimer = 6000;
                    }
                    else if (phase == 4)
                    {
                        passengerTalk(1, 2);
                        eventTimer = 6000;
                    }
                    else if (phase == 5)
                    {
                        passengerTalk(2, 1);
                        eventTimer = 6000;
                    }
                    else if (phase == 6)
                    {
                        passengerTalk(2, 2);
                        eventTimer = 7500;
                    }
                    else if (phase == 7)
                    {
                        passengerTalk(3, 2);
                        eventTimer = 6000;
                    }
                    else if (phase == 8)
                    {
                        passengerTalk(4, 2);
                        eventTimer = 6000;
                    }
                    // Need custom chat builder as creature is too far.
                    else if (phase == 9)
                    {
                        shenzinTalk("I am in pain, but it warms my heart that Liu Lang's grandchildren have not forgotten me.", 27822);
                        eventTimer = 7000;
                    }
                    else if (phase == 10)
                    {
                        shenzinTalk("There is a thorn in my side. I cannot remove it.", 27823);
                        eventTimer = 5000;
                    }
                    else if (phase == 11)
                    {
                        shenzinTalk("The pain is unbearable, and I can no longer swim straight.", 27824);
                        eventTimer = 5000;
                    }
                    else if (phase == 12)
                    {
                        shenzinTalk("Please grandchildren, can you remove this thorn? I cannot do so on my own.", 27825);
                        eventTimer = 7000;
                    }
                    else if (phase == 13)
                    {
                        passengerTalk(5, 2);
                        eventTimer = 7000;
                    }
                    else if (phase == 14)
                    {
                        shenzinTalk("It is in the forest where your feet do not walk. Continue along the mountains and you will find it.", 27826);
                        eventTimer = 8000;
                    }
                    else if (phase == 15)
                    {
                        passengerTalk(6, 2);
                        eventTimer = 6000;
                    }
                    else if (phase == 16)
                    {
                        me->SetSpeed(MOVE_FLIGHT, 3.0f, true);
                        shenzinTalk("Thank you, grandchildren.", 27827);
                        eventTimer = 8000;
                    }
                    else if (phase == 17)
                    {
                        passengerTalk(3, 1);
                        eventTimer = 6000;
                    }
                    else if (phase == 18)
                    {
                        passengerTalk(4, 1);
                        eventTimer = 8000;
                        me->SetSpeed(MOVE_FLIGHT, 4.0f, true);
                    }
                    else if (phase == 19)
                    {
                        passengerTalk(7, 2);
                        eventTimer = 8000;
                    }
                    else if (phase == 20)
                    {
                        passengerTalk(5, 1);
                        eventTimer = 8000;
                    }
                    else if (phase == 21)
                    {
                        passengerTalk(8, 2);
                        eventTimer = 6000;
                    }
                    else if (phase == 22)
                    {
                        passengerTalk(6, 1);
                        eventTimer = 6000;
                    }
                    else if (phase == 23)
                    {
                        passengerTalk(9, 2);
                        eventTimer = 6000;
                    }
                    else if (phase == 24)
                    {
                        passengerTalk(10, 2);
                        eventTimer = 6000;
                    }
                    ++phase;
                }
                else
                {
                    eventTimer -= diff;
                }
            }

            npc_escortAI::UpdateAI(diff);
        }

        void shenzinTalk(std::string text, uint32 soundId)
        {
            if (Player * player = me->GetVehicleKit()->GetPassenger(0)->ToPlayer())
            {
                WorldPacket packet(SMSG_MESSAGE_CHAT, 200);
                me->BuildMonsterChat(&packet, CHAT_MSG_MONSTER_SAY, text, 0, "Shen-zin Su", playerGUID);
                player->GetSession()->SendPacket(&packet);

                player->SendPlaySound(soundId, true);
            }
        }

        void passengerTalk(uint32 talkId, uint32 seatId)
        {
            if (Unit * unit = me->GetVehicleKit()->GetPassenger(seatId))
                if (Creature * creature = unit->ToCreature())
                    creature->AI()->Talk(talkId, playerGUID);
        }
    };

public:
    mob_shang_xi_air_balloon()
        : VehicleScript("mob_shang_xi_air_balloon")
    { }

    void OnAddPassenger(Vehicle* veh, Unit* passenger, int8 seatId) final
    {
        if (seatId != 0 || passenger->GetTypeId() != TYPEID_PLAYER)
            return;

        auto const player = passenger->ToPlayer();

        if (auto const firepaw = player->SummonCreature(56660, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 0.f, TEMPSUMMON_TIMED_DESPAWN, 300000))
            firepaw->_EnterVehicle(veh, 1);

        if (auto const aysa = player->SummonCreature(56662, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 0.f, TEMPSUMMON_TIMED_DESPAWN, 300000))
            aysa->_EnterVehicle(veh, 2);

        if (auto const cre = veh->GetBase()->ToCreature())
            cre->AI()->SetGUID(player->GetGUID());

        player->KilledMonsterCredit(56378);
    }

    void OnRemovePassenger(Vehicle *veh, Unit *passenger) final
    {
        if (passenger->GetTypeId() == TYPEID_PLAYER)
            if (auto const cre = veh->GetBase()->ToCreature())
                cre->AI()->SetGUID(0);
    }

    CreatureAI * GetAI(Creature* creature) const final
    {
        return new mob_shang_xi_air_balloonAI(creature);
    }
};

class spell_monkey_wisdom : public SpellScriptLoader
{
public:
    spell_monkey_wisdom() : SpellScriptLoader("spell_monkey_wisdom") {}

    class spell_monkey_wisdom_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_monkey_wisdom_SpellScript);

        void HandleScript(SpellEffIndex /*eff*/)
        {
            std::string text_str[9] =
            {
                "Peel banana first, eat second.",
                "Wet fur not fun to sleep on.",
                "Don't roll in own poo unless you want to smell like poo all day.",
                "Steal a banana from a hozen, expect an angry hozen.",
                "Poo not good to eat, but very good to throw.",
                "Mouth only hole that banana go in.",
                "Don't throw banana peel where going to walk.",
                "Firecracker for throwing, banana for eating.",
                "Don't pull own tail when there are other tails to pull."
            };

            GetHitUnit()->MonsterTextEmote(text_str[urand(0, 8)], GetHitUnit()->GetGUID(), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_monkey_wisdom_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_KILL_CREDIT2);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_monkey_wisdom_SpellScript();
    }
};

void AddSC_WanderingIsland_West()
{
    new mob_master_shang_xi_temple();
    new npc_wind_vehicle();
    new AreaTrigger_at_wind_temple_entrance();
    new mob_aysa_wind_temple_escort();
    new mob_frightened_wind();
    new npc_aysa_in_wind_temple();
    new boss_zhao_ren();
    new npc_rocket_launcher();
    new mob_master_shang_xi_after_zhao();
    new mob_master_shang_xi_after_zhao_escort();
    new mob_master_shang_xi_thousand_staff();
    new mob_master_shang_xi_thousand_staff_escort();
    new spell_grab_air_balloon();
    new mob_shang_xi_air_balloon();
    new spell_monkey_wisdom();
}
