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

#include "ScriptedEscortAI.h"
#include "Vehicle.h"

class npc_sanitron_500 : public CreatureScript
{
    enum
    {
        NPC_BUNNY               = 46165,
        NPC_CLEAN_CANNON        = 46208,
        SPELL_DECONTAMINATE_1   = 86075,
        SPELL_DECONTAMINATE_2   = 86084,
        SPELL_DECONTAMINATE_3   = 86086,
        SPELL_CANNON_BURST      = 86080,
        SPELL_IRRADIATED        = 80653,
    };

    struct npc_sanitron_500AI : public ScriptedAI
    {
        npc_sanitron_500AI(Creature * c) : ScriptedAI(c)
        {
            me->SetCorpseDelay(5);
        }

        void Reset()
        {
            done = false;
            stepTimer = 2000;
            nextStep = false;
        }

        bool isClose(float y1, float y2, float size)
        {
            float dy = fabs(y1 - y2);
            return (dy - size) <= 10.0f;
        }

        void CastSpellFromBunny(uint32 spellId)
        {
            if (cList.empty())
                return;

            if (Vehicle * veh = me->GetVehicleKit())
            {
                if (Unit * passenger = veh->GetPassenger(0))
                {
                    for(NpcList::const_iterator itr = cList.begin(); itr != cList.end(); ++itr)
                        if (isClose(me->GetPositionY(), (*itr)->GetPositionY(), me->GetObjectSize() + (*itr)->GetObjectSize()))
                            (*itr)->CastSpell(me, spellId, true);
                    passenger->CastSpell(passenger, spellId, true);
                }
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == WAYPOINT_MOTION_TYPE)
            {
                switch(id)
                {
                case 0:
                     CastSpellFromBunny(SPELL_DECONTAMINATE_1);
                     break;
                case 1:
                    {
                        Talk(1);
                        if (Vehicle * veh = me->GetVehicleKit())
                            if (Unit * passenger = veh->GetPassenger(0))
                                passenger->RemoveAurasDueToSpell(SPELL_IRRADIATED); // remove Irradiated buff

                        NpcList cannonList;
                        me->GetCreatureListWithEntryInGrid(cannonList, NPC_CLEAN_CANNON, 30.0f);
                        if (!cannonList.empty())
                            for(NpcList::const_iterator itr = cannonList.begin(); itr != cannonList.end(); ++itr)
                                (*itr)->CastSpell(me, SPELL_CANNON_BURST, true);
                        break;
                    }
                case 2:
                    {
                        Talk(2);
                        CastSpellFromBunny(SPELL_DECONTAMINATE_3);
                        done = true;
                        stepTimer = 5000;
                        break;
                    }
                }
            }
        }

         void PassengerBoarded(Unit* who, int8 seatId, bool apply)
         {
             if (apply)
             {
                 if (seatId)
                 {
                     who->ChangeSeat(0);
                 }else
                 {
                     me->GetCreatureListWithEntryInGrid(cList, NPC_BUNNY, 50.0f);
                     Talk(0);
                     nextStep = true;
                 }
             }
         }

        void UpdateAI(uint32 const diff)
        {
            if (nextStep)
            {
                if (stepTimer <= diff)
                {
                    me->GetMotionMaster()->MovePath(4618501, false);
                    nextStep = false;
                }else stepTimer -= diff;
            }
            else if (done)
            {
                if (stepTimer <= diff)
                {
                    me->DealDamage(me, me->GetMaxHealth());
                    me->DespawnOrUnsummon(5000);
                    done = false;
                }else stepTimer -= diff;
            }
        }

    private:
        bool nextStep;
        bool done;
        uint32 stepTimer;
        typedef std::list<Creature*> NpcList;
        NpcList cList;
    };

public:
    npc_sanitron_500() : CreatureScript("npc_sanitron_500") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_sanitron_500AI(creature);
    }
};

/*######
## npc_canon_propre
######*/

class npc_canon_propre : public CreatureScript
{
public:
    npc_canon_propre() : CreatureScript("npc_canon_propre") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_canon_propreAI (creature);
    }

    struct npc_canon_propreAI : public ScriptedAI
    {
        npc_canon_propreAI(Creature* creature) : ScriptedAI(creature) {}

        bool hasPassenger;

        void Reset()
        {
            hasPassenger = false;
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (hasPassenger)
                return;

            if (Creature* imunAgent = me->FindNearestCreature(45847, 10.0f))
            {
                if (me->GetVehicleKit())
                {
                    hasPassenger = true;
                    me->GetVehicleKit()->AddPassenger(imunAgent);
                }
            }
        }
    };
};

/*######
## npc_gnomeregan_recrue
######*/

class npc_gnomeregan_recrue : public CreatureScript
{
public:
    npc_gnomeregan_recrue() : CreatureScript("npc_gnomeregan_recrue") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gnomeregan_recrueAI (creature);
    }

    struct npc_gnomeregan_recrueAI : public ScriptedAI
    {
        npc_gnomeregan_recrueAI(Creature* creature) : ScriptedAI(creature) {}

        bool hasTarget;

        void Reset()
        {
            hasTarget = false;
        }

        void DamageTaken(Unit* doneBy, uint32& damage)
        {
            if (doneBy->ToCreature())
                if (me->GetHealth() <= damage || me->GetHealthPct() <= 80.0f)
                    damage = 0;
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType /*damageType*/, const SpellInfo * /*spellInfo*/)
        {
            if (target->ToCreature())
                if (target->GetHealth() <= damage || target->GetHealthPct() <= 70.0f)
                    damage = 0;
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (hasTarget)
            {
                DoMeleeAttackIfReady();
                return;
            }

            float x = 0, y = 0;
            GetPositionWithDistInOrientation(me, 2.5f, me->GetOrientation(), x, y);

            if (Creature* LivingInfection = me->SummonCreature(42185, x, y, me->GetPositionZ()))
            {
                LivingInfection->setFaction(16);
                LivingInfection->SetFacingToObject(me);
                LivingInfection->Attack(me, true);
                hasTarget = true;
            }
        }
    };
};

void AddSC_dun_morogh()
{
    new npc_sanitron_500();
    new npc_canon_propre();
    new npc_gnomeregan_recrue();
}
