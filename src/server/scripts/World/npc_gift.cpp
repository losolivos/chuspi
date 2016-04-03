#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Guild.h"
#include "SpellMgr.h"
#include "Pet.h"
#include "Spell.h"

class npc_gift : public CreatureScript
{
public:
    npc_gift()
        : CreatureScript("npc_gift") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Reclamar mi Regalo!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Salir", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->SEND_GOSSIP_MENU(907, creature->GetGUID());

        return true;
    }
    std::list<uint64> OnlyFirstGift;
    bool _access;

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        _access = true;

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            for (std::list<uint64>::iterator itr = OnlyFirstGift.begin(); itr != OnlyFirstGift.end(); ++itr)
            {
                if (player->GetGUID() == *itr) { player->SendChatMessage("ya recibiste tu regalo!"); _access = false; break; }
            }
            if (_access) { player->AddItem(79769, 1); OnlyFirstGift.push_back(player->GetGUID()); }
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        return true;
    }
};

class npc_giftt : public CreatureScript
{
public:
    npc_giftt()
        : CreatureScript("npc_giftt") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Reclamar mi Regalo!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Reclamar mi Segundo Regalo!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->SEND_GOSSIP_MENU(907, creature->GetGUID());

        return true;
    }
    std::list<uint64> OnlyFirstGift;
    bool _access;

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        _access = true;

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            for (std::list<uint64>::iterator itr = OnlyFirstGift.begin(); itr != OnlyFirstGift.end(); ++itr)
            {
                if (player->GetGUID() == *itr) { player->SendChatMessage("ya recibiste tu regalo!"); _access = false; break; }
            }
            if (_access) { player->AddItem(32458, 1); OnlyFirstGift.push_back(player->GetGUID()); }
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            for (std::list<uint64>::iterator itr = OnlyFirstGift.begin(); itr != OnlyFirstGift.end(); ++itr)
            {
                if (player->GetGUID() == *itr) { player->SendChatMessage("ya recibiste tu regalo!"); _access = false; break; }
            }
            if (_access) { player->AddItem(68823, 1); OnlyFirstGift.push_back(player->GetGUID()); }
            break;
        }
        return true;
    }
};

class npc_jade_serpent_riding_skill : public CreatureScript
{
public:
    npc_jade_serpent_riding_skill()
        : CreatureScript("npc_jade_serpent_riding_skill") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Get serpent riding skill!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "What is is?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Nothing there", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->SEND_GOSSIP_MENU(907, creature->GetGUID());

        return true;
    }
    bool _access;

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        _access = true;

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            if (player->GetReputationRank(1271) != REP_EXALTED) { player->SendChatMessage("You do not have reputation"); _access = false; }
            if (_access)
                player->learnSpell(130487, false);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->SendChatMessage("Here you can learn the skills of flight on cloud snakes. To do this, you need to have a reputation: Exalted");
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        return true;
    }
};

class npc_new_players_giftt : public CreatureScript
{
public:
    npc_new_players_giftt()
        : CreatureScript("npc_new_players_giftt") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Reclamar mi regalo!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Salir de aqui", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->SEND_GOSSIP_MENU(907, creature->GetGUID());

        return true;
    }
    std::list<uint64> OnlyFirstGift;
    bool _access;

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        _access = true;

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            for (std::list<uint64>::iterator itr = OnlyFirstGift.begin(); itr != OnlyFirstGift.end(); ++itr)
            {
                if (player->GetGUID() == *itr) { player->SendChatMessage("ya recibiste tu regalo!"); _access = false; break; }
            }
            if (_access) 
            { 
                player->AddItem(32458, 1);
                player->AddItem(68823, 1);
                OnlyFirstGift.push_back(player->GetGUID()); 
            }
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        return true;
    }
};

class npc_new_players_gift : public CreatureScript
{
public:
    npc_new_players_gift()
        : CreatureScript("npc_new_players_gift") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Get gift!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Nothing here", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->SEND_GOSSIP_MENU(907, creature->GetGUID());

        return true;
    }
    std::list<uint64> OnlyFirstGift;
    bool _access;

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        _access = true;

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            for (std::list<uint64>::iterator itr = OnlyFirstGift.begin(); itr != OnlyFirstGift.end(); ++itr)
            {
                if (player->GetGUID() == *itr) { player->SendChatMessage("you already got gift!"); _access = false; break; }
            }
            if (_access) 
            { 
                switch (player->getClass())
                {
                case CLASS_WARRIOR:
                    player->AddItem(94577, 1);
                    player->AddItem(86778, 1);
                    player->AddItem(86799, 1);
                    break;
                case CLASS_PALADIN:
                    player->AddItem(94577, 1);
                    player->AddItem(86778, 1);
                    player->AddItem(87544, 1);
                    player->AddItem(86764, 1);
                    player->AddItem(86799, 1);
                    break;
                case CLASS_DEATH_KNIGHT:
                    player->AddItem(86799, 1);   
                    break;
                case CLASS_PRIEST:
                case CLASS_WARLOCK:
                case CLASS_MAGE:
                    player->AddItem(82859, 1);
                    break;
                case CLASS_MONK:
                case CLASS_DRUID:
                        player->AddItem(86777, 1);
                        player->AddItem(86762, 2);
                        player->AddItem(82859,1);
                    break;
                case CLASS_ROGUE:
                    player->AddItem(87547, 2);
                    break;
                case CLASS_HUNTER:
                    player->AddItem(87546, 1);
                    break;
                case CLASS_SHAMAN:
                        player->AddItem(87544, 1);
                        player->AddItem(86764, 1);
                        player->AddItem(86762, 2);
                    break;
                default:
                    break;
                }

                player->AddItem(72582, 1);
                OnlyFirstGift.push_back(player->GetGUID()); 
            }
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        return true;
    }
};

class npc_morpher : public CreatureScript
{
public:
    npc_morpher()
        : CreatureScript("npc_morpher") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Humano!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Humana!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Orco!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Orca!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Draenei!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Draenei(m)!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Troll!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Troll(m)!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Gnomo!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Gnoma!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a No-Muerto!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a No-Muerta!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Enano!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Enana!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Elfo de Sangre!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Elfa de Sangre!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 16);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Elfo de la Noche!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 17);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Elfa de la Noche!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 18);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Tauren!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 19);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Tauren(m)!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 20);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Worgen!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Worgen(m)!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Goblin!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Convertir a Goblin(m)!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 24);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Cerrar Ventana", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 25);
        player->SEND_GOSSIP_MENU(907, creature->GetGUID());

        return true;
    }
    std::list<uint64> OnlyFirstGift;
    bool _access;

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            if (player)
                player->SetDisplayId(19723); // Human
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            if (player)
                player->SetDisplayId(37926); // Humana
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            if (player)
                player->SetDisplayId(37920); // Orco
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
            if (player)
                player->SetDisplayId(20316); // Orca
            break;
        case GOSSIP_ACTION_INFO_DEF + 5:
            if (player)
                player->SetDisplayId(37916); // Draenei
            break;
        case GOSSIP_ACTION_INFO_DEF + 6:
            if (player)
                player->SetDisplayId(20323); // Draeneimujer
            break;
        case GOSSIP_ACTION_INFO_DEF + 7:
            if (player)
                player->SetDisplayId(20321); //Troll
            break;
        case GOSSIP_ACTION_INFO_DEF + 8:
            if (player)
                player->SetDisplayId(37922); //Trolla
            break;
        case GOSSIP_ACTION_INFO_DEF + 9:
            if (player)
                player->SetDisplayId(37913); // Gnomo
            break;
        case GOSSIP_ACTION_INFO_DEF + 10:
            if (player)
                player->SetDisplayId(20581); // Gnoma
            break;
        case GOSSIP_ACTION_INFO_DEF + 11:
            if (player)
                player->SetDisplayId(37923); // No-Muerto
            break;
        case GOSSIP_ACTION_INFO_DEF + 12:
            if (player)
                player->SetDisplayId(37924); //No-Muerta
            break;
        case GOSSIP_ACTION_INFO_DEF + 13:
            if (player)
                player->SetDisplayId(20317); //Enano
            break;
        case GOSSIP_ACTION_INFO_DEF + 14:
            if (player)
                player->SetDisplayId(37918); // Enana
            break;
        case GOSSIP_ACTION_INFO_DEF + 15:
            if (player)
                player->SetDisplayId(20578); // Elfo de Sangre
            break;
        case GOSSIP_ACTION_INFO_DEF + 16:
            if (player)
                player->SetDisplayId(20579); // Elfa de Sangre
            break;
        case GOSSIP_ACTION_INFO_DEF + 17:
            if (player)
                player->SetDisplayId(20318); //Elfo de la Noche
            break;
        case GOSSIP_ACTION_INFO_DEF + 18:
            if (player)
                player->SetDisplayId(37919); //Elfa de la Noche
            break;
        case GOSSIP_ACTION_INFO_DEF + 19:
            if (player)
                player->SetDisplayId(20585); // Tauren
            break;
        case GOSSIP_ACTION_INFO_DEF + 20:
            if (player)
                player->SetDisplayId(20584); // Taurena
            break;
        case GOSSIP_ACTION_INFO_DEF + 21:
            if (player)
                player->SetDisplayId(37915); // Worgen
            break;
        case GOSSIP_ACTION_INFO_DEF + 22:
            if (player)
                player->SetDisplayId(37914); //Worgena
            break;
        case GOSSIP_ACTION_INFO_DEF + 23:
            if (player)
                player->SetDisplayId(20582); //Goblin
            break;
        case GOSSIP_ACTION_INFO_DEF + 24:
            if (player)
                player->SetDisplayId(20583); //Goblina
            break;
        case GOSSIP_ACTION_INFO_DEF + 25:
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        return true;
    }
};

class test_npc_for : public CreatureScript
{
public:
    test_npc_for() : CreatureScript("test_npc_for") { }

    struct test_npc_forAI : public ScriptedAI
    {
        test_npc_forAI(Creature* creature) : ScriptedAI(creature) {}

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            me->CastSpell(me, 135052, true);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage) 
        {
            damage = 0;
        }

    };
};

void AddSC_npc_gift()
{
    new npc_gift();
    new npc_giftt();
    new npc_jade_serpent_riding_skill();
    new npc_new_players_gift();
    new npc_new_players_giftt();
    new npc_morpher();
    new test_npc_for();
}