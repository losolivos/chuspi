/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

/* ScriptData
SDName: Orgrimmar
SD%Complete: 0
SDComment: Quest support:
SDCategory: Orgrimmar
EndScriptData */

/* ContentData
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"

class npc_garrosh_quest : public CreatureScript
{
public:
    npc_garrosh_quest() : CreatureScript("npc_garrosh_quest") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == 31013) // The Horde Way
        {
            if (Creature* master = player->SummonCreature(62087, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 300000))
            {
                master->setExplicitSeerGuid(player->GetGUID());
                master->AI()->SetGUID(player->GetGUID());
            }
        }

        return true;
    }
};

class npc_garrosh_the_horde_way : public CreatureScript
{
public:
    npc_garrosh_the_horde_way() : CreatureScript("npc_garrosh_the_horde_way") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_garrosh_the_horde_wayAI (creature);
    }

    struct npc_garrosh_the_horde_wayAI : public ScriptedAI
    {
        npc_garrosh_the_horde_wayAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 phaseId;
        uint32 phaseTimer;
        uint64 playerGUID;

        void Reset()
        {
            playerGUID = 0;
            phaseId = 0;
            phaseTimer = 2000;
        }

        void SetGUID(uint64 guid , int32 /* = 0 */)
        {
            playerGUID = guid;
        }

        void UpdateAI(const uint32 diff)
        {
            if (playerGUID == 0)
                return;

            if (phaseTimer < diff)
            {
                if (phaseId > 17)
                {
                    me->GetMotionMaster()->MovePoint(1, 1634.78f, -4361.15f, 26.76f);
                    if (Player * player = Unit::GetPlayer(*me, playerGUID))
                        player->KilledMonsterCredit(62089);
                    playerGUID = 0;
                    me->ForcedDespawn(5000);
                }

                Talk(phaseId, playerGUID);

                phaseTimer = 6500;
                if (phaseId == 0)
                    phaseTimer = 6000;
                else if (phaseId == 5 || phaseId == 7)
                    phaseTimer = 5000;
                else if (phaseId == 6)
                    phaseTimer = 8500;
                else if (phaseId == 8 || phaseId == 15)
                    phaseTimer = 2000;
                else if (phaseId == 11 || phaseId == 14)
                    phaseTimer = 4000;
                else if (phaseId == 12)
                    phaseTimer = 13000;
                else if (phaseId == 13)
                    phaseTimer = 15000;
                ++phaseId;
            }
            else
                phaseTimer -= diff;
        }
    };
};

void AddSC_orgrimmar()
{
    new npc_garrosh_quest();
    new npc_garrosh_the_horde_way();
}
