/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "MoveSplineInit.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Vehicle.h"
#include "CombatAI.h"
#include "Player.h"
#include "WorldSession.h"

/*######
## npc_thorim
######*/

#define GOSSIP_HN "Thorim?"
#define GOSSIP_SN1 "Can you tell me what became of Sif?"
#define GOSSIP_SN2 "He did more than that, Thorim. He controls Ulduar now."
#define GOSSIP_SN3 "It needn't end this way."

enum eThorim
{
    QUEST_SIBLING_RIVALRY = 13064,
    NPC_THORIM = 29445,
    GOSSIP_TEXTID_THORIM1 = 13799,
    GOSSIP_TEXTID_THORIM2 = 13801,
    GOSSIP_TEXTID_THORIM3 = 13802,
    GOSSIP_TEXTID_THORIM4 = 13803
};

class npc_thorim : public CreatureScript
{
public:
    npc_thorim() : CreatureScript("npc_thorim") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_SIBLING_RIVALRY) == QUEST_STATUS_INCOMPLETE) {
            player->ADD_GOSSIP_ITEM(0, GOSSIP_HN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_THORIM1, creature->GetGUID());
            return true;
        }
        return false;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(0, GOSSIP_SN1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_THORIM2, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->ADD_GOSSIP_ITEM(0, GOSSIP_SN2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_THORIM3, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                player->ADD_GOSSIP_ITEM(0, GOSSIP_SN3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_THORIM4, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                player->CLOSE_GOSSIP_MENU();
                player->CompleteQuest(QUEST_SIBLING_RIVALRY);
                break;
        }
        return true;
    }
};

/*######
## npc_victorious_challenger
######*/

#define GOSSIP_CHALLENGER            "Let's do this, sister."

enum eVictoriousChallenger
{
    QUEST_TAKING_ALL_CHALLENGERS    = 12971,
    QUEST_DEFENDING_YOUR_TITLE      = 13423,

    SPELL_SUNDER_ARMOR              = 11971,
    SPELL_REND_VC                   = 11977
};

class npc_victorious_challenger : public CreatureScript
{
public:
    npc_victorious_challenger() : CreatureScript("npc_victorious_challenger") { }

    struct npc_victorious_challengerAI : public ScriptedAI
    {
        npc_victorious_challengerAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 SunderArmorTimer;
        uint32 RendTimer;

        void Reset()
        {
            me->RestoreFaction();

            SunderArmorTimer = 10000;
            RendTimer        = 15000;
        }

        void UpdateAI(const uint32 diff)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            if (RendTimer < diff)
            {
                DoCast(me->GetVictim(), SPELL_REND_VC, true);
                RendTimer = 15000;
            }
            else
                RendTimer -= diff;

            if (SunderArmorTimer < diff)
            {
                DoCast(me->GetVictim(), SPELL_SUNDER_ARMOR, true);
                SunderArmorTimer = 10000;
            }
            else
                SunderArmorTimer -= diff;

            DoMeleeAttackIfReady();
        }

        void KilledUnit(Unit* /*victim*/)
        {
            me->RestoreFaction();
        }

    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_TAKING_ALL_CHALLENGERS) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_DEFENDING_YOUR_TITLE) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_CHALLENGER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }

        return false;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            player->CLOSE_GOSSIP_MENU();
            creature->setFaction(14);
            creature->AI()->AttackStart(player);
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_victorious_challengerAI(creature);
    }
};

/*######
## npc_injured_goblin
######*/

enum eInjuredGoblin
{
    QUEST_BITTER_DEPARTURE     = 12832,
    SAY_QUEST_ACCEPT           =  -1800042,
    SAY_END_WP_REACHED         =  -1800043
};

#define GOSSIP_ITEM_1       "I am ready, lets get you out of here"

class npc_injured_goblin : public CreatureScript
{
public:
    npc_injured_goblin() : CreatureScript("npc_injured_goblin") { }

    struct npc_injured_goblinAI : public npc_escortAI
    {
        npc_injured_goblinAI(Creature* creature) : npc_escortAI(creature) { }

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 26:
                    DoScriptText(SAY_END_WP_REACHED, me, player);
                    break;
                case 27:
                    player->GroupEventHappens(QUEST_BITTER_DEPARTURE, me);
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/) {}

        void Reset() {}

        void JustDied(Unit* /*killer*/)
        {
            Player* player = GetPlayerForEscort();
            if (HasEscortState(STATE_ESCORT_ESCORTING) && player)
                player->FailQuest(QUEST_BITTER_DEPARTURE);
        }

       void UpdateAI(const uint32 uiDiff)
        {
            npc_escortAI::UpdateAI(uiDiff);
            if (!UpdateVictim())
                return;
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_injured_goblinAI(creature);
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_BITTER_DEPARTURE) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->PlayerTalkClass->SendGossipMenu(9999999, creature->GetGUID());
        }
        else
            player->SEND_GOSSIP_MENU(999999, creature->GetGUID());
        return true;
    }

    bool OnQuestAccept(Player* /*player*/, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_BITTER_DEPARTURE)
            DoScriptText(SAY_QUEST_ACCEPT, creature);

        return false;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        npc_escortAI* pEscortAI = CAST_AI(npc_injured_goblin::npc_injured_goblinAI, creature->AI());

        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            pEscortAI->Start(true, true, player->GetGUID());
            creature->setFaction(113);
        }
        return true;
    }
};

/*######
## npc_roxi_ramrocket
######*/

#define SPELL_MECHANO_HOG           60866
#define SPELL_MEKGINEERS_CHOPPER    60867

class npc_roxi_ramrocket : public CreatureScript
{
public:
    npc_roxi_ramrocket() : CreatureScript("npc_roxi_ramrocket") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        //Quest Menu
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        //Trainer Menu
        if ( creature->IsTrainer() )
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_TEXT_TRAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

        //Vendor Menu
        if ( creature->IsVendor() )
            if (player->HasSpell(SPELL_MECHANO_HOG) || player->HasSpell(SPELL_MEKGINEERS_CHOPPER))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_TRAIN:
            player->GetSession()->SendTrainerList(creature->GetGUID());
            break;
        case GOSSIP_ACTION_TRADE:
            player->GetSession()->SendListInventory(creature->GetGUID());
            break;
        }
        return true;
    }
};

/*######
## npc_brunnhildar_prisoner
######*/

enum brunhildar {
    NPC_QUEST_GIVER            = 29592,

    SPELL_ICE_PRISON           = 54894,
    SPELL_KILL_CREDIT_PRISONER = 55144,
    SPELL_KILL_CREDIT_DRAKE    = 55143,
    SPELL_SUMMON_LIBERATED     = 55073,
    SPELL_ICE_LANCE            = 55046
};

class npc_brunnhildar_prisoner : public CreatureScript
{
public:
    npc_brunnhildar_prisoner() : CreatureScript("npc_brunnhildar_prisoner") { }

    struct npc_brunnhildar_prisonerAI : public ScriptedAI
    {
        npc_brunnhildar_prisonerAI(Creature* creature) : ScriptedAI(creature) {}

        uint64 drakeGUID;
        uint16 enter_timer;
        bool hasEmptySeats;

        void Reset()
        {
            me->CastSpell(me, SPELL_ICE_PRISON, true);
            enter_timer = 0;
            drakeGUID = 0;
            hasEmptySeats = false;
        }

        void UpdateAI(const uint32 diff)
        {
            //TODO: not good script
            if (!drakeGUID)
                return;

            Creature* drake = Unit::GetCreature(*me, drakeGUID);
            if (!drake)
            {
                drakeGUID = 0;
                return;
            }

            // drake unsummoned, passengers dropped
            if (!me->IsOnVehicle(drake) && !hasEmptySeats)
                me->DespawnOrUnsummon(3000);

            if (enter_timer <= 0)
                return;

            if (enter_timer < diff)
            {
                enter_timer = 0;
                if (hasEmptySeats)
                    me->JumpTo(drake, 25.0f);
                else
                    Reset();
            }
            else
                enter_timer -= diff;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who || !drakeGUID)
                return;

            Creature* drake = Unit::GetCreature(*me, drakeGUID);
            if (!drake)
            {
                drakeGUID = 0;
                return;
            }

            if (!me->IsOnVehicle(drake) && !me->HasAura(SPELL_ICE_PRISON))
            {
                if (who->IsVehicle() && me->IsWithinDist(who, 25.0f, true) && who->ToCreature() && who->ToCreature()->GetEntry() == 29709)
                {
                    uint8 seat = who->GetVehicleKit()->GetNextEmptySeat(0, true);
                    if (seat <= 0)
                        return;

                    me->EnterVehicle(who, seat);
                    me->SendMovementFlagUpdate();
                    hasEmptySeats = false;
                }
            }

            if (who->ToCreature() && me->IsOnVehicle(drake))
            {
                if (who->ToCreature()->GetEntry() == NPC_QUEST_GIVER && me->IsWithinDist(who, 15.0f, false))
                {
                    Unit* rider = drake->GetVehicleKit()->GetPassenger(0);
                    if (!rider)
                        return;

                    rider->CastSpell(rider, SPELL_KILL_CREDIT_PRISONER, true);

                    me->ExitVehicle();
                    me->CastSpell(me, SPELL_SUMMON_LIBERATED, true);
                    me->DespawnOrUnsummon(500);

                    // drake is empty now, deliver credit for drake and despawn him
                    if (drake->GetVehicleKit()->HasEmptySeat(1) &&
                        drake->GetVehicleKit()->HasEmptySeat(2) &&
                        drake->GetVehicleKit()->HasEmptySeat(3))
                    {
                        // not working rider->CastSpell(rider, SPELL_KILL_CREDIT_DRAKE, true);
                        if (rider->ToPlayer())
                            rider->ToPlayer()->KilledMonsterCredit(29709, 0);

                        drake->DespawnOrUnsummon(0);
                    }
                }
            }
        }

        void SpellHit(Unit* hitter, const SpellInfo* spell)
        {
            if (!hitter || !spell)
                return;

            if (spell->Id != SPELL_ICE_LANCE)
                return;

            me->RemoveAura(SPELL_ICE_PRISON);
            enter_timer = 500;

            if (hitter->IsVehicle())
                drakeGUID = hitter->GetGUID();
            else
                return;

            if (hitter->GetVehicleKit()->GetNextEmptySeat(0, true))
                hasEmptySeats = true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_brunnhildar_prisonerAI(creature);
    }
};

class npc_icefang : public CreatureScript
{
public:
    npc_icefang() : CreatureScript("npc_icefang") { }

    struct npc_icefangAI : public npc_escortAI
    {
        npc_icefangAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                if (apply)
                    Start(false, true, who->GetGUID());
            }
        }

        void WaypointReached(uint32 /*waypointId*/)
        {
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_icefangAI (creature);
    }
};

enum CloseRift
{
    SPELL_DESPAWN_RIFT          = 61665
};

class spell_close_rift : public SpellScriptLoader
{
    public:
        spell_close_rift() : SpellScriptLoader("spell_close_rift") { }

        class spell_close_rift_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_close_rift_AuraScript);

            bool Load()
            {
                _counter = 0;
                return true;
            }

            bool Validate(SpellInfo const* /*spell*/)
            {
                return sSpellMgr->GetSpellInfo(SPELL_DESPAWN_RIFT);
            }

            void HandlePeriodic(AuraEffect const * /*aurEff*/)
            {
                if (++_counter == 5)
                    GetTarget()->CastSpell((Unit*)NULL, SPELL_DESPAWN_RIFT, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_close_rift_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }

        private:
            uint8 _counter;

        };

        AuraScript* GetAuraScript() const
        {
            return new spell_close_rift_AuraScript();
        }
};

/*######
## go_rusty_cage
######*/

enum eRustyCage
{
    NPC_GOBLIN_PRISIONER    = 29466
};

class go_rusty_cage : public GameObjectScript
{
public:
    go_rusty_cage() : GameObjectScript("go_rusty_cage") { }

    bool OnGossipHello(Player* player, GameObject* pGO)
    {
        if (Creature* goblinPrisoner = pGO->FindNearestCreature(NPC_GOBLIN_PRISIONER, 5.0f, true))
        {
            pGO->SetGoState(GO_STATE_ACTIVE);
            player->KilledMonsterCredit(NPC_GOBLIN_PRISIONER, goblinPrisoner->GetGUID());
            goblinPrisoner->ForcedDespawn(2000);
            goblinPrisoner->MonsterSay(RAND<const char*>("Time to hightail it! Thanks, friend!",
                                                        "I can't believe it! I'm free to go!",
                                                        "I'm free? I'm free!"), LANG_UNIVERSAL, 0);
        }

        return true;
    }
};

/*######
## npc_goblin_prisoner
######*/

enum eGoblinPrisoner
{
    GO_RUSTY_CAGE = 191544
};

class npc_goblin_prisoner : public CreatureScript
{
public:
    npc_goblin_prisoner() : CreatureScript("npc_goblin_prisoner") { }

    struct npc_goblin_prisonerAI : public ScriptedAI
    {
        npc_goblin_prisonerAI(Creature* creature) : ScriptedAI (creature){ }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);

            if (GameObject* pGO = me->FindNearestGameObject(GO_RUSTY_CAGE, 5.0f))
                if (pGO->GetGoState() == GO_STATE_ACTIVE)
                    pGO->SetGoState(GO_STATE_READY);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_goblin_prisonerAI(creature);
    }
};

class spell_q12906_discipline : public SpellScriptLoader
{
public:
    spell_q12906_discipline() : SpellScriptLoader("spell_q12906_discipline") { }

    class spell_q12906_discipline_SpellScript : public SpellScript
    {
    public:
        PrepareSpellScript(spell_q12906_discipline_SpellScript)

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* pCaster = GetCaster();
            if (Player* player = pCaster->ToPlayer())
            {
                if (Creature* target = GetHitCreature())
                {
                    if (target->getStandState() == UNIT_STAND_STATE_STAND)
                        return;

                    switch(urand(1,2))
                    {
                        case 1:
                        {
                            target->MonsterSay(RAND<const char*>("Curse you! You will not treat me like a beast!",
                                                    "Enough! I will teach you some manners, wench!",
                                                    "I'd rather die fighting than live like a slave!"), LANG_UNIVERSAL, 0);
                            target->SetStandState(UNIT_STAND_STATE_STAND);
                            target->AI()->AttackStart(player);
                            break;
                        }
                        case 2:
                        {
                            target->MonsterSay(RAND<const char*>("We will have revenge... some day.",
                                                                 "You treat us worse than animals!",
                                                                 "Back... to work..."), LANG_UNIVERSAL, 0);
                            target->SetStandState(UNIT_STAND_STATE_STAND);
                            target->HandleEmoteCommand(EMOTE_STATE_WORK);
                            break;
                        }
                    }
                    player->KilledMonsterCredit(29886);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_q12906_discipline_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_q12906_discipline_SpellScript();
    }
};

/*######
## npc_loklira_crone
######*/

#define GOSSIP_LOKLIRACRONE     "Tell me about this proposal"
#define GOSSIP_LOKLIRACRONE1    "What happened then?"
#define GOSSIP_LOKLIRACRONE2    "You want me to take part in the Hyldsmeet to end the war?"
#define GOSSIP_LOKLIRACRONE3    "Very well. I'll take part in this competition."

enum eLokliraCrone
{
    QUEST_HYLDSMEET     = 12970,

    GOSSIP_TEXTID_LOK1  = 13778,
    GOSSIP_TEXTID_LOK2  = 13779,
    GOSSIP_TEXTID_LOK3  = 13780
};

class npc_loklira_crone : public CreatureScript
{
public:
    npc_loklira_crone() : CreatureScript("npc_loklira_crone") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_HYLDSMEET) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOKLIRACRONE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }
        return false;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOKLIRACRONE1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LOK1, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOKLIRACRONE2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LOK2, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOKLIRACRONE3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LOK3, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                player->CLOSE_GOSSIP_MENU();
                player->CompleteQuest(QUEST_HYLDSMEET);
                break;
        }
        return true;
    }
};

/*######
## Quest: The Drakkensryd (12886)
######*/

enum eHyldsmeetProtodrake
{
    //QUEST_DRAKKENSRYD              = 12886,
    ENTRY_DRAKE_RIDER              = 29800
};

static const Position centerPos = {7432.099609f, -533.930176f, 1896.850098f, 0.0f};

class npc_hyldsmeet_protodrake_static : public CreatureScript
{
    struct npc_hyldsmeet_protodrake_staticAI : public ScriptedAI
    {
        npc_hyldsmeet_protodrake_staticAI(Creature * creature) : ScriptedAI(creature)
        {
            creature->setActive(true);
            init = false;
            initTimer = urand(1000, 30000);
            respawnTimer = 0;
        }

        void Reset()
        {
            if (vPos.empty())
                MakePath();
        }

        void JustReachedHome()
        {
            if (init)
                StartMovement();
        }

        void StartMovement()
        {
            me->SetWalk(false);
            Movement::MoveSplineInit init(me);
            init.Path() = vPos;
            init.SetSmooth();
            init.SetCyclic();
            init.SetFly();
            init.SetUncompressed();
            init.Launch();
        }

        void MakePath()
        {
            float angle = centerPos.GetAngle(me);
            float dist = me->GetExactDist2d(&centerPos);
            float x, y, z;
            float Z = me->GetPositionZ();
            for (int i=0; i < 23; ++i )
            {
                centerPos.GetPosition(x, y, z);
                angle += 15.0f * M_PI / 180.0f;
                angle = centerPos.NormalizeOrientation(angle);

                x += dist * cos(angle);
                y += dist * sin(angle);
                z = Z + frand(0.0f, 40.0f);
                vPos.push_back(G3D::Vector3(x, y, z));
            }
        }

        void SummonedCreatureDies(Creature* /*summon*/, Unit* /*killer*/)
        {
            respawnTimer = 45000;
        }

        void PassengerBoarded(Unit* /*who*/, int8 /*seatId*/, bool /*apply*/)
        {
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void JustSummoned(Creature* summon)
        {
            summon->setActive(true);
            summon->SetReactState(REACT_AGGRESSIVE);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!init)
            {
                if (initTimer <= diff)
                {
                    init = true;
                    StartMovement();
                }else initTimer -= diff;
            }

            if (respawnTimer)
            {
                if (respawnTimer <= diff)
                {
                    me->GetVehicleKit()->InstallAllAccessories(false);
                    respawnTimer = 0;
                }else respawnTimer -= diff;
            }
        }

    private:
        std::vector<G3D::Vector3> vPos;
        bool init;
        uint32 initTimer;
        uint32 respawnTimer;
    };
public:
    npc_hyldsmeet_protodrake_static() : CreatureScript("npc_hyldsmeet_protodrake_static") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_hyldsmeet_protodrake_staticAI(creature);
    }
};

class npc_hyldsmeet_protodrake : public CreatureScript
{
    enum NPCs
    {
        NPC_HYLDSMEET_DRAKERIDER = 29694
    };

    struct npc_hyldsmeet_protodrakeAI : public ScriptedAI
    {
        npc_hyldsmeet_protodrakeAI(Creature* pCreature) : ScriptedAI(pCreature) { }

        void Reset()
        {
            cycleTimer = 0;
            moveTimer = 0;
        }

        void PassengerBoarded(Unit* pWho, int8 /*seatId*/, bool apply)
        {
            if (pWho && apply)
            {
                moveTimer = 2000;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (cycleTimer)
            {
                if (cycleTimer <= diff)
                {
                    // me->GetMotionMaster()->MoveSplinePath(2, true, true, 32.0f, true);
                    cycleTimer = 0;
                }else cycleTimer -= diff;
            }

            if (moveTimer)
            {
                if (moveTimer <= diff)
                {
                    moveTimer = 0;
                    me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                    // me->GetMotionMaster()->MoveSplinePath(1, true, false, 32.0f);
                    cycleTimer = me->GetSplineDuration();
                }else moveTimer -= diff;
            }
        }
    private:
        uint32 cycleTimer;
        uint32 moveTimer;
    };

public:
    npc_hyldsmeet_protodrake() : CreatureScript("npc_hyldsmeet_protodrake") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_hyldsmeet_protodrakeAI(pCreature);
    }
};

class npc_column_ornament : public CreatureScript
{
    enum
    {
        SPELL_HYLDNIR_HARPOON           = 54933
    };

    class VehicleExitEvent : public BasicEvent
    {
        public:
            VehicleExitEvent(Unit * owner) : _owner(owner) { }

            bool Execute(uint64 /*execTime*/, uint32 /*diff*/)
            {
                _owner->ExitVehicle();
                return false;
            }

        private:
            Unit * _owner;
    };

    struct npc_column_ornamentAI : public ScriptedAI
    {
        npc_column_ornamentAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->setActive(true);
        }

        void SpellHit(Unit* hitter, const SpellInfo* spell)
        {
            if (!hitter || !spell)
                return;

            if (spell->Id == SPELL_HYLDNIR_HARPOON)
            {
                me->m_Events.AddEvent(new VehicleExitEvent(hitter), me->m_Events.CalculateTime(hitter->GetSplineDuration() + 500));
            }
        }

    };

public:
    npc_column_ornament() : CreatureScript("npc_column_ornament") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_column_ornamentAI(creature);
    }
};

void AddSC_storm_peaks()
{
    new npc_thorim();
    new npc_victorious_challenger();
    new npc_injured_goblin();
    new npc_roxi_ramrocket();
    new npc_brunnhildar_prisoner();
    new npc_icefang();
    new spell_close_rift();
    new npc_goblin_prisoner();
    new go_rusty_cage();
    new spell_q12906_discipline();
    new npc_loklira_crone();
    new npc_hyldsmeet_protodrake();
    new npc_column_ornament();
    new npc_hyldsmeet_protodrake_static();
}
