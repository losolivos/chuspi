#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "CreatureAI.h"
#include "SpellScript.h"
#include "Vehicle.h"

/*
Uldum Zone:
Easy Money (27003) - npc_lady_hump_tanaris, npc_adarrah_easy_money
Traitors! (27922) - go_neferset_frond
Escape From the Lost City (28112) - npc_prince_nadun_lost_city
Take it to 'Em! (27993) - npc_harrison_jones_uldum
Premature Explosionation (27141) - go_powder_keg
On to Something (27196) - npc_harrison_jones_obelisk, npc_harrison_jones_on_to_something
The Thrill of Discovery (27511) - AreaTrigger_at_chambers_of_the_star
Lessons From the Past (27541) - npc_schnottz_scout
*/


/* Lady Hump - Starts event for Easy Money quest
* Missing:
* Caravan Escort and Cameras system
*/
class npc_lady_hump_tanaris : public CreatureScript
{
public:
    npc_lady_hump_tanaris() : CreatureScript("npc_lady_hump_tanaris") { }

    bool OnGossipHello(Player * player, Creature * creature)
    {
        if (player->GetQuestStatus(27003) == QUEST_STATUS_INCOMPLETE)
        {
            //player->SendCinematicStart(161);
            player->CastSpell(player, 89404, true);
            player->TeleportTo(player->GetMapId(), -10995.93f, -1254.66f, 13.25f, 4.67f);
            player->KilledMonsterCredit(44833);
        }
        return true;
    }
};

// Adarrah - Finish event for Easy Money quest
class npc_adarrah_easy_money : public CreatureScript
{
public:
    npc_adarrah_easy_money() : CreatureScript("npc_adarrah_easy_money") { }

    bool OnQuestComplete(Player* player, Creature* creature, const Quest* quest)
    {
        if (GameObject * go = GetClosestGameObjectWithEntry(player, 206951, 15.f))
            go->UseDoorOrButton();

        // TODO: Jailer should spawn after short text event
        if (Creature * const jailer = player->SummonCreature(48029, -11025.f, -1280.f, 13.79f, 0.7f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 180000))
        {
            // TODO: Move to creature_text
            creature->MonsterSay("Here comes the guard! Take him out!", 0, 0);
            jailer->AI()->AttackStart(player);
        }

        return true;
    }
};

/* Nefreset Frond object for Traitors! quest
* Missing:
* Some cosmetic changes done to nearby NPCs
*/
class go_neferset_frond : public GameObjectScript
{
public:
    go_neferset_frond() : GameObjectScript("go_neferset_frond") { }

    bool OnGossipHello(Player* player, GameObject* /*go*/)
    {
        if (Creature * const camera = GetClosestCreatureWithEntry(player, 47473, 30.f))
        {
            if (!camera->GetVehicleKit()->IsVehicleInUse())
            {
                player->CastSpell(player, 88525, true);
                player->EnterVehicle(camera);
            }
        }
        return false;
    }
};

class npc_uldum_camera_traitors_q : public CreatureScript
{
public:
    npc_uldum_camera_traitors_q() : CreatureScript("npc_uldum_camera_traitors_q") { }

    struct npc_uldum_camera_traitors_qAI : public ScriptedAI
    {
        npc_uldum_camera_traitors_qAI(Creature* creature) : ScriptedAI(creature) { }

        bool eventStarted;
        uint8 phase;
        uint32 phaseTimer;

        void Reset()
        {
            phase = 0;
            phaseTimer = 5000;
            eventStarted = false;
        }

        void UpdateAI(uint32 const diff)
        {
            if (eventStarted)
            {
                if (phaseTimer <= diff)
                {
                    if (phase < 2)
                    {
                        if (Creature const * const siamat = GetClosestCreatureWithEntry(me, 47451, 50.f))
                        {
                            siamat->AI()->Talk(phase);
                            phaseTimer = 10000;
                        }
                    }
                    else
                    {
                        if (Unit * const pas = me->GetVehicleKit()->GetPassenger(0))
                            if (Player * const player = pas->ToPlayer())
                                player->KilledMonsterCredit(47466);

                        me->GetVehicleKit()->RemoveAllPassengers();
                    }
                    ++phase;
                }
                else phaseTimer -= diff;
            }
        }

        void PassengerBoarded(Unit * who, int8 /*seat*/, bool enter)
        {
            if (enter)
                eventStarted = true;
            else
                Reset();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_uldum_camera_traitors_qAI(creature);
    }
};

/* Prince Naduun start event for Escape From the Lost City
* Missing:
* Escape Cinematic
*/
class npc_prince_nadun_lost_city : public CreatureScript
{
public:
    npc_prince_nadun_lost_city() : CreatureScript("npc_prince_nadun_lost_city") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == 28112)
        {
            player->TeleportTo(player->GetMapId(), -9443.31f, -958.36f, 111.02f, 0.015f);
        }
        return true;
    }
};

// Harrison Jones for Take it to 'Em!
class npc_harrison_jones_uldum : public CreatureScript
{
public:
    npc_harrison_jones_uldum() : CreatureScript("npc_harrison_jones_uldum") { }

    struct npc_harrison_jones_uldumAI : public ScriptedAI
    {
        npc_harrison_jones_uldumAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() { }

        void MoveInLineOfSight(Unit* who)
        {
            if (Player * const player = who->ToPlayer())
                if (player->GetQuestStatus(27993) == QUEST_STATUS_INCOMPLETE && me->GetDistance(who) < 20.f)
                    player->KilledMonsterCredit(47318);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_harrison_jones_uldumAI(creature);
    }
};

/* Powder Keg for Premature Explosionation
* Missing:
* Cinematic event with explosion
*/
class go_powder_keg : public GameObjectScript
{
public:
    go_powder_keg() : GameObjectScript("go_powder_keg") { }

    bool OnGossipHello(Player* player, GameObject* /*go*/)
    {
        if (player->GetQuestStatus(27141) == QUEST_STATUS_INCOMPLETE)
        {
            player->CastSpell(player, 89404, true);
            player->TeleportTo(player->GetMapId(), -9207.99f, -1560.32f, 65.46f, 0.82f);
            player->KilledMonsterCredit(45143);
        }
        return false;
    }
};

// Harrison Jones gossip to start On to Something event
class npc_harrison_jones_obelisk : public CreatureScript
{
public:
    npc_harrison_jones_obelisk() : CreatureScript("npc_harrison_jones_obelisk") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(27196) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I'm ready, Doctor Jones!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->CLOSE_GOSSIP_MENU();
            Position pos;
            creature->GetPosition(&pos);
            if (Creature const * const harrison = player->SummonCreature(45238, pos, TEMPSUMMON_TIMED_DESPAWN, 16000))
                harrison->AI()->SetGUID(player->GetGUID(), 0);
        }

        return true;
    }
};

// Harrison Jones On to Something event
class npc_harrison_jones_on_to_something : public CreatureScript
{
public:
    npc_harrison_jones_on_to_something() : CreatureScript("npc_harrison_jones_on_to_something") { }

    struct script_impl : public ScriptedAI
    {
        script_impl(Creature* creature) : ScriptedAI(creature) { }

        uint32 moveTimer;
        bool wait;
        void Reset()
        {
            wait = false;
            moveTimer = 0;
        }

        void SetGUID(uint64 guid, int32 /* = 0 */)
        {
            me->SetOwnerGUID(guid);
            Talk(0, guid);
            me->GetMotionMaster()->MovePoint(1, -9152.527f, -1532.1818f, 71.2173f);
        }

        void UpdateAI(uint32 const diff)
        {
            if (wait)
            {
                if (moveTimer < diff)
                {
                    wait = false;
                    me->GetMotionMaster()->MoveJump(-9148.899f, -1535.172f, 73.777f, 10.f, 10.f, 2);
                }
                else
                    moveTimer -= diff;
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            switch (pointId)
            {
            case 1:
                Talk(1);
                if (Player * const player = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                    player->KilledMonsterCredit(me->GetEntry());
                wait = true;
                moveTimer = 3000;
                break;
            case 2:
                me->GetMotionMaster()->MoveJump(-9144.42f, -1539.485f, 74.9625f, 10.f, 10.f, 3);
                break;
            case 3:
                Talk(2);
                me->GetMotionMaster()->MoveJump(-9115.3f, -1555.26f, 11.f, 10.f, 10.f, 4);
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new script_impl(creature);
    }
};

// Areatriggers for The Thrill of Discovery
class AreaTrigger_at_chambers_of_the_star : public AreaTriggerScript
{
public:
    AreaTrigger_at_chambers_of_the_star() : AreaTriggerScript("at_chambers_of_the_star") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* trigger)
    {

        uint32 creditEntry = 0;
        if (trigger->id == 6284)
            creditEntry = 45757;
        else if (trigger->id == 6288)
            creditEntry = 45760;
        else if (trigger->id == 6289)
            creditEntry = 45759;

        if (creditEntry)
            player->KilledMonsterCredit(creditEntry);
        return true;
    }
};

// Schnottz Scout for Lessons From the Past
class npc_schnottz_scout : public CreatureScript
{
public:
    npc_schnottz_scout() : CreatureScript("npc_schnottz_scout") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->hasQuest(27541) && !player->HasItemCount(61930))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Take the scout's Journal.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if (player->GetQuestStatus(27541) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(61929))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Take the scout's spectacles.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);


        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->AddItem(61930, 1);
            player->CLOSE_GOSSIP_MENU();
        }
        else if (action == GOSSIP_ACTION_INFO_DEF + 2)
        {
            player->AddItem(61929, 1);
            player->CLOSE_GOSSIP_MENU();
        }

        return true;
    }
};

void AddSC_uldum()
{
    new npc_lady_hump_tanaris();
    new npc_adarrah_easy_money();

    new go_neferset_frond();
    new npc_uldum_camera_traitors_q();

    new npc_prince_nadun_lost_city();

    new npc_harrison_jones_uldum();
    new go_powder_keg();

    new npc_harrison_jones_obelisk();
    new npc_harrison_jones_on_to_something();

    new AreaTrigger_at_chambers_of_the_star();

    new npc_schnottz_scout();
}
