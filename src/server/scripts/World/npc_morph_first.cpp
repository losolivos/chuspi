/*
Developer: Dway
Script: Npc Morpher
ScriptName: npc_morph
Modify: BlackMetalz
*/

#include "ScriptPCH.h"
#include "WorldPacket.h"

#define reward 392 //reward id
#define pocet 100 //pocet rewardu
#define deffender_morpher_text_buy "Transform Success" 
#define deffender_morpher_text_err "You dont have enough Tokens" 
#define deffender_morpher_text_cb  "You are in combat"
class npc_morph_first : public CreatureScript
{
public:
	npc_morph_first() : CreatureScript("npc_morph_first")
	{
	}
	bool OnGossipHello(Player* pPlayer, Creature* pCreature)
	{
		if (pPlayer->IsInCombat())
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pCreature->MonsterWhisper(deffender_morpher_text_cb, pPlayer->GetGUID(), true);
			return true;
		}

		pPlayer->PlayerTalkClass->ClearMenus();
		pPlayer->ADD_GOSSIP_ITEM(12, "Recuerda que usar estas transformaciones tienen un costo de 100p de Honor", GOSSIP_SENDER_MAIN, 8888);
		pPlayer->ADD_GOSSIP_ITEM(9, "Demorph", GOSSIP_SENDER_MAIN, 20001);
		pPlayer->ADD_GOSSIP_ITEM(6, "Lady Sylvanas Windrunner", GOSSIP_SENDER_MAIN, 20002);
		pPlayer->ADD_GOSSIP_ITEM(6, "Gul'dan", GOSSIP_SENDER_MAIN, 20003);
		pPlayer->ADD_GOSSIP_ITEM(6, "Akama", GOSSIP_SENDER_MAIN, 20004);
		pPlayer->ADD_GOSSIP_ITEM(6, "Maiev Shadowsong", GOSSIP_SENDER_MAIN, 20005);
		pPlayer->ADD_GOSSIP_ITEM(6, "Medivh", GOSSIP_SENDER_MAIN, 20006);
		pPlayer->ADD_GOSSIP_ITEM(6, "Tucnak", GOSSIP_SENDER_MAIN, 20007);
		pPlayer->ADD_GOSSIP_ITEM(6, "Geist", GOSSIP_SENDER_MAIN, 20008);
		pPlayer->ADD_GOSSIP_ITEM(6, "Undead Pirat", GOSSIP_SENDER_MAIN, 20009);
		pPlayer->ADD_GOSSIP_ITEM(6, "Velen the Prophet", GOSSIP_SENDER_MAIN, 20010);
		pPlayer->ADD_GOSSIP_ITEM(6, "Christmas Blood Elf 18+", GOSSIP_SENDER_MAIN, 20011);
		pPlayer->ADD_GOSSIP_ITEM(6, "Arthas", GOSSIP_SENDER_MAIN, 20012);
		pPlayer->ADD_GOSSIP_ITEM(6, "Lich king", GOSSIP_SENDER_MAIN, 20013);
		pPlayer->ADD_GOSSIP_ITEM(6, "Humr", GOSSIP_SENDER_MAIN, 20014);
		pPlayer->ADD_GOSSIP_ITEM(6, "Terrorfiend", GOSSIP_SENDER_MAIN, 20015);
		pPlayer->ADD_GOSSIP_ITEM(6, "Troll na netopyrovi", GOSSIP_SENDER_MAIN, 20016);
		pPlayer->ADD_GOSSIP_ITEM(6, "Teron Gorefiend", GOSSIP_SENDER_MAIN, 20017);
		pPlayer->ADD_GOSSIP_ITEM(6, "King Varian Wrynn", GOSSIP_SENDER_MAIN, 20018);
		pPlayer->ADD_GOSSIP_ITEM(6, "Naga", GOSSIP_SENDER_MAIN, 20019);
		pPlayer->ADD_GOSSIP_ITEM(6, "Santa Goblin", GOSSIP_SENDER_MAIN, 20020);
		pPlayer->ADD_GOSSIP_ITEM(6, "Christmas Gnome Female", GOSSIP_SENDER_MAIN, 20021);
		pPlayer->ADD_GOSSIP_ITEM(6, "Aldaron the Reckless", GOSSIP_SENDER_MAIN, 20022);
		pPlayer->ADD_GOSSIP_ITEM(6, "Ogre", GOSSIP_SENDER_MAIN, 20023);
		pPlayer->ADD_GOSSIP_ITEM(6, "Christmas Undead", GOSSIP_SENDER_MAIN, 20024);
		pPlayer->ADD_GOSSIP_ITEM(6, "Christmas Night Elf. 18+", GOSSIP_SENDER_MAIN, 20025);
		pPlayer->ADD_GOSSIP_ITEM(6, "Gnome Diver", GOSSIP_SENDER_MAIN, 20026);
		pPlayer->ADD_GOSSIP_ITEM(6, "Santa Claus Gnom", GOSSIP_SENDER_MAIN, 20027);
		pPlayer->ADD_GOSSIP_ITEM(6, "Human Diver", GOSSIP_SENDER_MAIN, 20028);
		pPlayer->ADD_GOSSIP_ITEM(6, "Troll Diver", GOSSIP_SENDER_MAIN, 20029);
		pPlayer->ADD_GOSSIP_ITEM(6, "Li Li Stormstout", GOSSIP_SENDER_MAIN, 20030);
		pPlayer->ADD_GOSSIP_ITEM(6, "Chen Stormstout", GOSSIP_SENDER_MAIN, 20031);
		pPlayer->ADD_GOSSIP_ITEM(6, "Valeera Sanguinar", GOSSIP_SENDER_MAIN, 20032);
		pPlayer->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetGUID());
		return true;
	}
	void SendDefaultMenu(Player* pPlayer, Creature* pCreature, uint32 uiAction)
	{
		switch (uiAction)
		{
		case 8888:
			pPlayer->PlayerTalkClass->ClearMenus();
			OnGossipHello(pPlayer, pCreature);
			break;

		case 20001://Demorph
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->DeMorph();
			break;

		case 20002://Lady Sylvanas Windrunner
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(28213);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;
		case 20003://Gul'dan
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(16642);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20004://Akama
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(20681);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20005://Maiev Shadowsong
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(20628);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20006://Medivh
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(18718);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20007://Tucnak
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(24698);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20008://Geist
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(24579);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20009://Undead Pirat
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(25042);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20010://Velen the Prophet
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(17822);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20011://Christmas Blood Elf 18+
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(18785);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20012://Arthas
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(21976);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20013://Lich king
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(22235);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20014://Humr
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(19726);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20015://Terrorfiend
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(18373);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20016://Troll na netopyrovi
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(15303);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20017://Teron Gorefiend
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(21576);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20018://King Varian Wrynn
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(28127);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20019://Naga
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(18390);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20020://Santa Goblin
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(15698);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20021://Christmas Gnome Female
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(15799);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20022://Aldaron the Reckless
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(15925);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20023://Ogre
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(1122);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20024://Christmas Undead
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(18811);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20025://Christmas Night Elf. 18+
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(15748);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20026://Gnome Diver
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(27657);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20027://Santa Claus Gnom
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(15806);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20028://Human Diver
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(23426);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20029://Troll Diver
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(17272);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;
			
			// New 
			
		case 20030://Li Li Stormstout
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(44514);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20031://Chen Stormstout
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(39698);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;

		case 20032://Valeera Sanguinar
			if (pPlayer->HasCurrency(reward, pocet))
			{
				pPlayer->ModifyCurrency(reward, pocet, true, false);
				pCreature->MonsterWhisper(deffender_morpher_text_buy, pPlayer->GetGUID(), true);
				pPlayer->SetDisplayId(26365);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(deffender_morpher_text_err, pPlayer->GetGUID(), true);
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			break;


		}
	}


	bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
	{
		if (uiSender == GOSSIP_SENDER_MAIN)
			SendDefaultMenu(pPlayer, pCreature, uiAction);
		return true;
	}

};

void AddSC_npc_morph_first()
{
	new npc_morph_first;
}