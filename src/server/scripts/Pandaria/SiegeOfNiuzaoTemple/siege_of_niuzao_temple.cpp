/*
 * Copyright (C) 2008-2014 MoltenCore <http://www.molten-wow.com/>
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

#include "siege_of_niuzao_temple.h"

/*
***********************
***** TRASH MOBS ******
***********************
*/

// Sik'thik Guardian - 61928
class npc_sikthik_guardian : public CreatureScript
{
    enum Spells
    {
        SPELL_MALLEABLE_RESIN           = 121422
    };

    struct npc_sikthik_guardianAI : public ScriptedAI
    {
        npc_sikthik_guardianAI(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            chargeTimer = 0;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (chargeTimer <= diff)
            {
                DoCast(SELECT_TARGET_RANDOM, SPELL_MALLEABLE_RESIN, true, 1);
                chargeTimer = 10000;
            }else chargeTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        uint32 chargeTimer;
    };

public:
    npc_sikthik_guardian() : CreatureScript("npc_sikthik_guardian") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sikthik_guardianAI(creature);
    }
};


class npc_sikthik_amber_weaver : public CreatureScript
{
    enum Spells
    {
        SPELL_RESIN_WEAVE_COSMETIC      = 120596,
        SPELL_RESIN_WEAVING             = 121114,
        SPELL_RESIN_SHELL               = 120946,
    };

    struct npc_sikthik_amber_weaverAI : public ArcherAI
    {
        npc_sikthik_amber_weaverAI(Creature * creature) : ArcherAI(creature)
        {
            visualTimer = 3000;


            me->m_CombatDistance = 15.0f;
            me->m_SightDistance = 15.0f;
            m_minRange = 10.0f;
        }

        void Reset()
        {
            resinShellTimer = urand(10000, 12000);
            resinWeavingtimer = urand(6000, 8000);
        }

        void JustReachedHome()
        {
            visualTimer = 1000;
        }

        void EnterCombat(Unit* )
        {
            me->InterruptNonMeleeSpells(true);
        }

        void UpdateAI(uint32 const diff)
        {
            if (visualTimer)
            {
                if (visualTimer <= diff)
                {
                    DoCast(me, SPELL_RESIN_WEAVE_COSMETIC, false);
                    visualTimer = 0;
                } else visualTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (resinWeavingtimer <= diff)
            {
                DoCast(SELECT_TARGET_RANDOM, SPELL_RESIN_WEAVING, false);
                resinWeavingtimer = 10000;
                return;
            } else resinWeavingtimer -= diff;

            if (resinShellTimer <= diff)
            {
                DoCast(me, SPELL_RESIN_SHELL, false);
                resinShellTimer = 20000;
            } else resinShellTimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        uint32 visualTimer;
        uint32 resinShellTimer;
        uint32 resinWeavingtimer;
    };

public:
    npc_sikthik_amber_weaver() : CreatureScript("npc_sikthik_amber_weaver") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sikthik_amber_weaverAI(creature);
    }
};

// Resin weaving - 121114
class spell_resin_weaving : public SpellScriptLoader
{
    enum 
    {
        SPELL_ENCASED_IN_RESIN          = 121116
    };
public:
    spell_resin_weaving() : SpellScriptLoader("spell_resin_weaving") {}

    class aura_impl : public AuraScript
    {
        PrepareAuraScript(aura_impl);
        void OnRemove(AuraEffect const * aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit * owner = GetUnitOwner())
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                    owner->CastSpell(owner, SPELL_ENCASED_IN_RESIN, true);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(aura_impl::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new aura_impl();
    }
};


class npc_resin_flake : public CreatureScript
{
    enum Spells
    {
        // SPELL_RESIDUE_AURA           = 120940,
        SPELL_RESIDUE           = 120938,
    };

    struct npc_resin_flakeAI : public ScriptedAI
    {
        npc_resin_flakeAI(Creature * creature) : ScriptedAI(creature) {}

        /*
        void DamageTaken(Unit* , uint32& damage)
        {
            
            if (damage >= me->GetHealth())
                DoCast(me, SPELL_RESIDUE, true);
            
        }
        */
    };

public:
    npc_resin_flake() : CreatureScript("npc_resin_flake") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_resin_flakeAI(creature);
    }
};

static const Position attackPos = { 2080.853f, 5056.36f, 143.6199f, 0.0f };

enum
{
    DATA_CATAPULT_ASSIGNED      = 10,

    NPC_MANTID_SOLDIER          = 62348,

    SPELL_CATAPULT_CHANNEL      = 124067,
    SPELL_CATAPULT_PRECAST      = 124083,
    SPELL_FIRE_CATAPULT         = 124017
};

// Mantid Catapult - 63565
class npc_mantid_catapult : public CreatureScript
{
    enum
    {
        EVENT_INIT              = 1,
        EVENT_SOLDIER_CHANNEL,
        EVENT_CAST,
        EVENT_FIRE,
    };

    struct npc_mantid_catapultAI : public ScriptedAI
    {
        npc_mantid_catapultAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            me->setActive(true);
            events.Reset();
            if (soldierList.empty())
                events.ScheduleEvent(EVENT_INIT, urand(1000, 6000));
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INIT:
                    {
                        std::list<Creature*> cList;
                        me->GetCreatureListWithEntryInGrid(cList, NPC_MANTID_SOLDIER, 50.0f);
                        if (cList.empty())
                            break;

                        cList.sort(Trinity::ObjectDistanceOrderPred(me));

                        for (auto itr : cList)
                        {
                            if (itr->IsAlive()
                                &&!itr->AI()->GetData(DATA_CATAPULT_ASSIGNED))
                            {
                                soldierList.push_back(itr->GetGUID());
                                itr->AI()->SetGUID(me->GetGUID(), soldierList.size());
                            }

                            if (soldierList.size() >= 3)
                                break;
                        }

                        if (soldierList.empty())
                            return;

                        events.ScheduleEvent(EVENT_SOLDIER_CHANNEL, urand(5000, 18000)); // To make sure catapults dont fire simultaneously
                        break;
                    }
                    case EVENT_SOLDIER_CHANNEL:
                    {
                        bool gotSoldier = false;
                        bool combat = true;

                        // If soldiers are dead -> stop all events
                        // If soldiers are in combat -> reschedule this check

                        for (auto itr : soldierList)
                            if (Creature * soldier = Creature::GetCreature(*me, itr))
                            {
                                if (soldier->IsAlive())
                                {
                                    gotSoldier = true;
                                    if (!soldier->IsInCombat())
                                    {
                                        combat = false;
                                        soldier->CastSpell(soldier, SPELL_CATAPULT_CHANNEL, true);
                                    }
                                }
                            }

                        if (combat)
                            events.ScheduleEvent(EVENT_SOLDIER_CHANNEL, 5000);
                        else if (gotSoldier)
                            events.ScheduleEvent(EVENT_CAST, 2000);
                        break;
                    }
                    case EVENT_CAST:
                        DoCast(me, SPELL_CATAPULT_PRECAST, false);
                        events.ScheduleEvent(EVENT_FIRE, 3000); // To make sure catapults dont fire simultaneously
                        break;
                    case EVENT_FIRE:
                    {
                        Position pos = attackPos;
                        me->MovePosition(pos, frand(20.0f, 50.0f), (float)rand_norm() * static_cast<float>(2 * M_PI));
                        if (!me->IsWithinDist2d(&pos, 200.0f))
                            me->GetNearPosition(pos, 199.0f, me->GetAngle(&pos));

                        me->CastSpell(pos, SPELL_FIRE_CATAPULT, false);

                        events.ScheduleEvent(EVENT_SOLDIER_CHANNEL, 10000); // To make sure catapults dont fire simultaneously
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        std::list<uint64> soldierList;
    };

public:
    npc_mantid_catapult() : CreatureScript("npc_mantid_catapult") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_mantid_catapultAI(creature);
    }
};

// Sik'Thik soldier - 62348 + 61448
class npc_mantid_soldier_catapult : public CreatureScript
{
    enum Spells
    {
        SPELL_SERRATED_BLADE        = 119840
    };

    struct npc_mantid_soldier_catapultAI : public ScriptedAI
    {
        npc_mantid_soldier_catapultAI(Creature * creature) : ScriptedAI(creature)
        {
            catapultGUID = 0;
            counter = 0;
        }

        void Reset()
        {
            me->setActive(true);
            bladetimer = urand(5000, 6000);
        }

        uint32 GetData(uint32 data)
        {
            if (data == DATA_CATAPULT_ASSIGNED)
            {
                return catapultGUID;
                catapultGUID = 1;
            }

            return 0;
        }

        void SetGUID(uint64 guid, int32 data)
        {
            // 62384 only
            catapultGUID = guid;
            counter = data;

            // update home position based on the counter
            // soldiers line up behind the catapult
            if (Creature * catapult = Creature::GetCreature(*me, guid))
            {
                //Arrange around the position of the "spoon"
                Position anchorPos, myPos;
                catapult->GetNearPosition(anchorPos, -9.0f, 0.0f);
                myPos = anchorPos;
                me->MovePosition(myPos, INTERACTION_DISTANCE, float(M_PI / 2.0) * (float)counter);
                myPos.m_orientation = myPos.GetAngle(&anchorPos);

                me->SetHomePosition(myPos);
                EnterEvadeMode();
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (bladetimer <= diff)
            {
                DoCastVictim(SPELL_SERRATED_BLADE, false);
                bladetimer = urand(4000, 7000);
            } else bladetimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        uint64 catapultGUID;
        int32 counter;
        uint32 bladetimer;
    };

public:
    npc_mantid_soldier_catapult() : CreatureScript("npc_mantid_soldier_catapult") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_mantid_soldier_catapultAI(creature);
    }
};

 // Quest: Somewhere Inside
enum eSomewhereInside
{
    GO_MANTID_CAGE          = 213935,
    NPC_PRISONER            = 64520,
    SPELL_INTERACT          = 125993,
    SPELL_STEALTH           = 86603
};

// Shado Pan Prisoner - 64520
class npc_niuzao_shado_pan_prisoner : public CreatureScript
{
    enum Spells
    {
    };

    struct npc_niuzao_shado_pan_prisonerAI : public ScriptedAI
    {
        npc_niuzao_shado_pan_prisonerAI(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            stealthTimer = 0;
            exitTimer = 0;
        }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_INTERACT)
            {
                me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                Player * player = caster->ToPlayer();
                GameObject * cage = me->FindNearestGameObject(GO_MANTID_CAGE, 5.0f);
                if (player && cage)
                {
                    player->KilledMonsterCredit(NPC_PRISONER, me->GetGUID());
                    cage->UseDoorOrButton(DAY);
                    exitTimer = 2000;
                }
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
                stealthTimer = 1000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (exitTimer)
            {
                if (exitTimer <= diff)
                {
                    exitTimer = 0;
                    Position pos;
                    me->GetNearPosition(pos, 3.0f, 0.0f);
                    me->GetMotionMaster()->MovePoint(1, pos);
                } else exitTimer -= diff;
            }

            if (stealthTimer)
            {
                if (stealthTimer <= diff)
                {
                    stealthTimer = 0;
                    DoCast(me, SPELL_STEALTH, true);
                    Position pos;
                    me->GetNearPosition(pos, 20.0f, 0.0f);
                    me->GetMotionMaster()->MovePoint(2, pos);
                    me->DespawnOrUnsummon(3000);
                } else stealthTimer -= diff;
            }
        }
    private:
        uint32 exitTimer;
        uint32 stealthTimer;
    };

public:
    npc_niuzao_shado_pan_prisoner() : CreatureScript("npc_niuzao_shado_pan_prisoner") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_niuzao_shado_pan_prisonerAI(creature);
    }
};

/*

class npc_sikthik_guardian : public CreatureScript
{
    enum Spells
    {

    };

    struct npc_sikthik_guardianAI : public ScriptedAI
    {
        npc_sikthik_guardianAI(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {

        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();
        }
    };

public:
    npc_sikthik_guardian() : CreatureScript("npc_sikthik_guardian") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new npc_sikthik_guardianAI(creature);
    }
};
*/

// Sik'Thik Battle-Mender - 67093
class npc_sikthik_battle_mender : public CreatureScript
{
    enum Spells
    {
        SPELL_MENDING           = 131968,
        SPELL_WINDS_GRACE       = 131972
    };

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            mendingTimer = urand(5000, 6000);
            graceTimer = urand(1000, 2000);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (mendingTimer <= diff)
            {
                DoCast(SPELL_MENDING);
                mendingTimer = urand(1000, 2000);
            } else mendingTimer -= diff;

            if (graceTimer <= diff)
            {
                DoCast(me, SPELL_WINDS_GRACE, false);
                graceTimer = 60000;
            } else graceTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        uint32 mendingTimer;
        uint32 graceTimer;
    };

public:
    npc_sikthik_battle_mender() : CreatureScript("npc_sikthik_battle_mender") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new ai_impl(creature);
    }
};

// Sik'Thik Bladedancer - 61436
class npc_sikthik_bladedancer : public CreatureScript
{
    enum Spells
    {
        SPELL_BLADE_DANCE           = 124253,
        SPELL_SIKTHIK_STRIKE        = 119354
    };

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            danceTimer = urand(5000, 6000);
            strikeTimer = urand(1000, 2000);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (danceTimer <= diff)
            {
                DoCastVictim(SPELL_BLADE_DANCE, false);
                danceTimer = urand(7000, 9000);
            } else danceTimer -= diff;

            if (strikeTimer <= diff)
            {
                DoCastVictim(SPELL_SIKTHIK_STRIKE, false);
                strikeTimer = urand(5000, 5500);
            } else strikeTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        uint32 danceTimer;
        uint32 strikeTimer;
    };

public:
    npc_sikthik_bladedancer() : CreatureScript("npc_sikthik_bladedancer") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new ai_impl(creature);
    }
};

// Sik'Thik Builder - 62633
class npc_sikthik_builder : public CreatureScript
{
    enum Spells
    {
        SPELL_HURL_BRICK        = 121762
    };

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            brixTimer = urand(1000, 2000);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (brixTimer <= diff)
            {
                DoCastVictim(SPELL_HURL_BRICK, false);
                brixTimer = 500;
            } else brixTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        uint32 brixTimer;
    };

public:
    npc_sikthik_builder() : CreatureScript("npc_sikthik_builder") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new ai_impl(creature);
    }
};

// Sik'Thik engineer - 62632
class npc_sikthik_engineer : public CreatureScript
{
    enum Spells
    {
        SPELL_BURNING_PITCH             = 122259,
        SPELL_CRYSTALLIZE               = 122244,
        SPELL_CRYSTALLIZED_PITCH        = 122246
    };

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            pitchTimer = urand(1000, 2000);
            crystallizeTimer = 4000;
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_CRYSTALLIZE && target->HasAura(SPELL_BURNING_PITCH))
                DoCast(target, SPELL_CRYSTALLIZED_PITCH, true);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (pitchTimer <= diff)
            {
                DoCast(me, SPELL_BURNING_PITCH, false);
                pitchTimer = urand(4000, 5000);
                crystallizeTimer = 1000;
            } else pitchTimer -= diff;

            if (crystallizeTimer <= diff)
            {
                DoCast(me, SPELL_CRYSTALLIZE, false);
                crystallizeTimer = 10000;
            } else crystallizeTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        uint32 pitchTimer;
        uint32 crystallizeTimer;
    };

public:
    npc_sikthik_engineer() : CreatureScript("npc_sikthik_engineer") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new ai_impl(creature);
    }
};

// Sik'Thik Vanguard - 61434
class npc_sikthik_vanguard : public CreatureScript
{
    enum Spells
    {
        SPELL_BESIEGE                       = 119347,
        SPELL_WILL_OF_THE_EMPRESS           = 124172
    };

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            besiegeTimer = urand(5000, 6000);
            willTimer = 100;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (besiegeTimer <= diff)
            {
                DoCast(SELECT_TARGET_FARTHEST, SPELL_BESIEGE, false, 1, 0.0f, true, -SPELL_BESIEGE);
                besiegeTimer = urand(10000, 15000);
            } else besiegeTimer -= diff;

            if (willTimer <= diff)
            {
                DoCast(me, SPELL_WILL_OF_THE_EMPRESS, true);
                willTimer = urand(18000, 20000);
            } else willTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        uint32 besiegeTimer;
        uint32 willTimer;
    };

public:
    npc_sikthik_vanguard() : CreatureScript("npc_sikthik_vanguard") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new ai_impl(creature);
    }
};


 // Shado-Master Chum Kiu - 64517
class npc_chum_kiu : public CreatureScript
{
    enum Spells
    {
        SPELL_BESIEGE                       = 119347,
        SPELL_WILL_OF_THE_EMPRESS           = 124172
    };

    struct ai_impl : public ScriptedAI
    {
        ai_impl(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
            me->GetMotionMaster()->MoveSplinePath(1, false, true, 3.0f, false, false, false);
            unstealthTimer = me->GetSplineDuration() + 1000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (unstealthTimer)
            {
                if (unstealthTimer <= diff)
                {
                    me->SetFacingTo(0.4886922f);
                    me->RemoveAllAuras();
                    unstealthTimer = 0;
                    arrivalTimer = 1000;
                } else unstealthTimer -= diff;
            }

            if (arrivalTimer)
            {
                if (arrivalTimer <= diff)
                {
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    if (Player * target = me->SelectNearestPlayer(50.0f))
                        Talk(0, target->GetGUID());
                    arrivalTimer = 0;
                } else arrivalTimer -= diff;
            }
        }

    private:
        uint32 unstealthTimer;
        uint32 arrivalTimer;
    };

public:
    npc_chum_kiu() : CreatureScript("npc_chum_kiu") {}

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new ai_impl(creature);
    }
};
void AddSC_siege_of_niuzao_temple()
{
    // Hollowed Out Tree
    new npc_sikthik_guardian();
    new npc_sikthik_amber_weaver();
    new npc_resin_flake();
    new npc_mantid_soldier_catapult();
    new npc_mantid_catapult();
    new npc_niuzao_shado_pan_prisoner();
    new npc_sikthik_battle_mender();
    new npc_sikthik_bladedancer();
    new npc_sikthik_builder();
    new npc_sikthik_engineer();
    new npc_sikthik_vanguard();
    new spell_resin_weaving();
    new npc_chum_kiu();
};