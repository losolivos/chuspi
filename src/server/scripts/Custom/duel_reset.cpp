#include "ScriptPCH.h"

class duel_reset : public PlayerScript
{
public:
duel_reset() : PlayerScript("duel_reset"){}

void OnDuelEnd(Player* pWinner, Player* pLoser, DuelCompleteType type)

{
    if (pWinner->GetAreaId() == 5974)
    {

		if (type != DUEL_WON)
		return;
        pWinner->RemoveArenaSpellCooldowns();
        pLoser->RemoveArenaSpellCooldowns();
        pWinner->SetHealth(pWinner->GetMaxHealth());
        if ( pWinner->getPowerType() == POWER_MANA )
            pWinner->SetPower(POWER_MANA, pWinner->GetMaxPower(POWER_MANA));
        pLoser->SetHealth(pLoser->GetMaxHealth());
        if ( pLoser->getPowerType() == POWER_MANA )
            pLoser->SetPower(POWER_MANA, pLoser->GetMaxPower(POWER_MANA));
	

    }
}
};

void AddSC_duel_reset()
{
new duel_reset();
}