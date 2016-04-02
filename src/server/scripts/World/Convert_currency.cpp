/*
Developer: Blackmetal
Script: npc_convert_currency
ScriptName: convert_currency
*/

#include "ScriptPCH.h"
#include "WorldPacket.h"

#define hp_token 888101 //Honor Token ID
#define quantity 1 //Quantity of Honor Token
#define converter_text_buy "Exchange Success!" 
#define converter_text_err "You don't have enough Tokens!" 
#define converter_text_cb  "You are in combat!"
class npc_convert_currency : public CreatureScript
{
public:
	npc_convert_currency() : CreatureScript("convert_currency")
	{
	}
	bool OnGossipHello(Player* pPlayer, Creature* pCreature)
	{
		if (pPlayer->IsInCombat())
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pCreature->MonsterWhisper(converter_text_cb, pPlayer->GetGUID(), true);
			return true;
		}

		pPlayer->PlayerTalkClass->ClearMenus();
		pPlayer->ADD_GOSSIP_ITEM(9, "Convert to Honor Point ( Require: 1 Honor Token )", GOSSIP_SENDER_MAIN, 999100);
		pPlayer->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetGUID());
		return true;
	}
	void SendDefaultMenu(Player* pPlayer, Creature* pCreature, uint32 uiAction)
	{
		switch (uiAction)
		{

		case 999100://Convert from Justice Point to Honor point required Honor Token 3000:1000
			if (pPlayer->HasItemCount(hp_token, quantity))
			{
				pPlayer->DestroyItemCount(hp_token, quantity, true, false);
				pCreature->MonsterWhisper(converter_text_buy, pPlayer->GetGUID(), true);
				pPlayer->ModifyCurrency(CURRENCY_TYPE_HONOR_POINTS, 100000, MODIFY_CURRENCY_NO_PRINT_LOG); // +1000 honor
				pPlayer->CLOSE_GOSSIP_MENU();
			}
			else
			{
				pCreature->MonsterWhisper(converter_text_err, pPlayer->GetGUID(), true);
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

void AddSC_npc_convert_currency()
{
	new npc_convert_currency;
}