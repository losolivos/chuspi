#include "ScriptPCH.h"

class pvpquest : public PlayerScript
{
public:
	pvpquest() : PlayerScript("pvpquest") {}

	void OnPVPKill(Player* killer, Player* killed)
	{
		if (killer->GetQuestStatus(989898) == QUEST_STATUS_INCOMPLETE)
		    killer->KilledMonsterCredit(64645);
	}

};

void AddSC_pvpquest()
{
	new pvpquest();
}