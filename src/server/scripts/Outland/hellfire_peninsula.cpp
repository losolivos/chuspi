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
SDName: Hellfire_Peninsula
SD%Complete: 100
SDComment: Quest support: 9375, 9410, 9418, 10129, 10146, 10162, 10163, 10340, 10346, 10347, 10382 (Special flight paths)
SDCategory: Hellfire Peninsula
EndScriptData */

/* ContentData
npc_aeranas
npc_ancestral_wolf
go_haaleshi_altar
npc_naladu
npc_tracy_proudwell
npc_trollbane
npc_wounded_blood_elf
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"

/*######
## npc_aeranas
######*/

enum eAeranas
{
    SAY_SUMMON              = -1000138,
    SAY_FREE                = -1000139,

    FACTION_HOSTILE         = 16,
    FACTION_FRIENDLY        = 35,

    SPELL_ENVELOPING_WINDS  = 15535,
    SPELL_SHOCK             = 12553,

    C_AERANAS               = 17085
};

class npc_aeranas : public CreatureScript
{
public:
    npc_aeranas() : CreatureScript("npc_aeranas") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_aeranasAI (creature);
    }

    struct npc_aeranasAI : public ScriptedAI
    {
        npc_aeranasAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 Faction_Timer;
        uint32 EnvelopingWinds_Timer;
        uint32 Shock_Timer;

        void Reset()
        {
            Faction_Timer = 8000;
            EnvelopingWinds_Timer = 9000;
            Shock_Timer = 5000;

            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->setFaction(FACTION_FRIENDLY);

            DoScriptText(SAY_SUMMON, me);
        }

        void UpdateAI(const uint32 diff)
        {
            if (Faction_Timer)
            {
                if (Faction_Timer <= diff)
                {
                    me->setFaction(FACTION_HOSTILE);
                    Faction_Timer = 0;
                } else Faction_Timer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (HealthBelowPct(30))
            {
                me->setFaction(FACTION_FRIENDLY);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->RemoveAllAuras();
                me->DeleteThreatList();
                me->CombatStop(true);
                DoScriptText(SAY_FREE, me);
                return;
            }

            if (Shock_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_SHOCK);
                Shock_Timer = 10000;
            } else Shock_Timer -= diff;

            if (EnvelopingWinds_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_ENVELOPING_WINDS);
                EnvelopingWinds_Timer = 25000;
            } else EnvelopingWinds_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_ancestral_wolf
######*/

enum eAncestralWolf
{
    EMOTE_WOLF_LIFT_HEAD            = -1000496,
    EMOTE_WOLF_HOWL                 = -1000497,
    SAY_WOLF_WELCOME                = -1000498,

    SPELL_ANCESTRAL_WOLF_BUFF       = 29981,

    NPC_RYGA                        = 17123
};

class npc_ancestral_wolf : public CreatureScript
{
public:
    npc_ancestral_wolf() : CreatureScript("npc_ancestral_wolf") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ancestral_wolfAI(creature);
    }

    struct npc_ancestral_wolfAI : public npc_escortAI
    {
        npc_ancestral_wolfAI(Creature* creature) : npc_escortAI(creature)
        {
            if (creature->GetOwner() && creature->GetOwner()->GetTypeId() == TYPEID_PLAYER)
                Start(false, false, creature->GetOwner()->GetGUID());
            else
                TC_LOG_ERROR("scripts", "TRINITY: npc_ancestral_wolf can not obtain owner or owner is not a player.");

            creature->SetSpeed(MOVE_WALK, 1.5f);
            Reset();
        }

        Unit* pRyga;

        void Reset()
        {
            pRyga = NULL;
            DoCast(me, SPELL_ANCESTRAL_WOLF_BUFF, true);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!pRyga && who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == NPC_RYGA && me->IsWithinDistInMap(who, 15.0f))
                pRyga = who;

            npc_escortAI::MoveInLineOfSight(who);
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 0:
                    DoScriptText(EMOTE_WOLF_LIFT_HEAD, me);
                    break;
                case 2:
                    DoScriptText(EMOTE_WOLF_HOWL, me);
                    break;
                case 50:
                    if (pRyga && pRyga->IsAlive() && !pRyga->IsInCombat())
                        DoScriptText(SAY_WOLF_WELCOME, pRyga);
                    break;
            }
        }
    };
};

/*######
## go_haaleshi_altar
######*/

class go_haaleshi_altar : public GameObjectScript
{
public:
    go_haaleshi_altar() : GameObjectScript("go_haaleshi_altar") { }

    bool OnGossipHello(Player* /*player*/, GameObject* go)
    {
        go->SummonCreature(C_AERANAS, -1321.79f, 4043.80f, 116.24f, 1.25f, TEMPSUMMON_TIMED_DESPAWN, 180000);
        return false;
    }
};

/*######
## npc_naladu
######*/

#define GOSSIP_NALADU_ITEM1 "Why don't you escape?"

enum eNaladu
{
    GOSSIP_TEXTID_NALADU1   = 9788
};

class npc_naladu : public CreatureScript
{
public:
    npc_naladu() : CreatureScript("npc_naladu") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
            player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_NALADU1, creature->GetGUID());

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_NALADU_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }
};

/*######
## npc_tracy_proudwell
######*/

#define GOSSIP_TEXT_REDEEM_MARKS        "I have marks to redeem!"
#define GOSSIP_TRACY_PROUDWELL_ITEM1    "I heard that your dog Fei Fei took Klatu's prayer beads..."
#define GOSSIP_TRACY_PROUDWELL_ITEM2    "<back>"

enum eTracy
{
    GOSSIP_TEXTID_TRACY_PROUDWELL1       = 10689,
    QUEST_DIGGING_FOR_PRAYER_BEADS       = 10916
};

class npc_tracy_proudwell : public CreatureScript
{
public:
    npc_tracy_proudwell() : CreatureScript("npc_tracy_proudwell") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TRACY_PROUDWELL_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_TRACY_PROUDWELL1, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
                break;
            case GOSSIP_ACTION_TRADE:
                player->GetSession()->SendListInventory(creature->GetGUID());
                break;
        }

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (creature->IsVendor())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_REDEEM_MARKS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        if (player->GetQuestStatus(QUEST_DIGGING_FOR_PRAYER_BEADS) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TRACY_PROUDWELL_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }
};

/*######
## npc_trollbane
######*/

#define GOSSIP_TROLLBANE_ITEM1      "Tell me of the Sons of Lothar."
#define GOSSIP_TROLLBANE_ITEM2      "<more>"
#define GOSSIP_TROLLBANE_ITEM3      "Tell me of your homeland."

enum eTrollbane
{
    GOSSIP_TEXTID_TROLLBANE1        = 9932,
    GOSSIP_TEXTID_TROLLBANE2        = 9933,
    GOSSIP_TEXTID_TROLLBANE3        = 8772
};

class npc_trollbane : public CreatureScript
{
public:
    npc_trollbane() : CreatureScript("npc_trollbane") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TROLLBANE_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_TROLLBANE1, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_TROLLBANE2, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_TROLLBANE3, creature->GetGUID());
                break;
        }

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TROLLBANE_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TROLLBANE_ITEM3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }
};

/*######
## npc_wounded_blood_elf
######*/

enum eWoundedBloodElf
{
    SAY_ELF_START               = -1000117,
    SAY_ELF_SUMMON1             = -1000118,
    SAY_ELF_RESTING             = -1000119,
    SAY_ELF_SUMMON2             = -1000120,
    SAY_ELF_COMPLETE            = -1000121,
    SAY_ELF_AGGRO               = -1000122,

    QUEST_ROAD_TO_FALCON_WATCH  = 9375
};

class npc_wounded_blood_elf : public CreatureScript
{
public:
    npc_wounded_blood_elf() : CreatureScript("npc_wounded_blood_elf") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_ROAD_TO_FALCON_WATCH)
        {
            if (npc_escortAI* pEscortAI = CAST_AI(npc_wounded_blood_elf::npc_wounded_blood_elfAI, creature->AI()))
                pEscortAI->Start(true, false, player->GetGUID());

            // Change faction so mobs attack
            creature->setFaction(775);
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wounded_blood_elfAI(creature);
    }

    struct npc_wounded_blood_elfAI : public npc_escortAI
    {
        npc_wounded_blood_elfAI(Creature* creature) : npc_escortAI(creature) {}

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 0:
                    DoScriptText(SAY_ELF_START, me, player);
                    break;
                case 9:
                    DoScriptText(SAY_ELF_SUMMON1, me, player);
                    // Spawn two Haal'eshi Talonguard
                    DoSpawnCreature(16967, -15, -15, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    DoSpawnCreature(16967, -17, -17, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                case 13:
                    DoScriptText(SAY_ELF_RESTING, me, player);
                    break;
                case 14:
                    DoScriptText(SAY_ELF_SUMMON2, me, player);
                    // Spawn two Haal'eshi Windwalker
                    DoSpawnCreature(16966, -15, -15, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    DoSpawnCreature(16966, -17, -17, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                case 27:
                    DoScriptText(SAY_ELF_COMPLETE, me, player);
                    // Award quest credit
                    player->GroupEventHappens(QUEST_ROAD_TO_FALCON_WATCH, me);
                    break;
            }
        }

        void Reset() { }

        void EnterCombat(Unit* /*who*/)
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
                DoScriptText(SAY_ELF_AGGRO, me);
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->AI()->AttackStart(me);
        }
    };
};

/*######
## npc_fel_guard_hound
######*/

enum eFelGuard
{
    SPELL_SUMMON_POO                              = 37688,

    NPC_DERANGED_HELBOAR                          = 16863
};

class npc_fel_guard_hound : public CreatureScript
{
public:
    npc_fel_guard_hound() : CreatureScript("npc_fel_guard_hound") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fel_guard_houndAI(creature);
    }

    struct npc_fel_guard_houndAI : public ScriptedAI
    {
        npc_fel_guard_houndAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 uiCheckTimer;
        uint64 uiHelboarGUID;

        void Reset()
        {
            uiCheckTimer = 5000; //check for creature every 5 sec
            uiHelboarGUID = 0;
        }

        void MovementInform(uint32 uiType, uint32 uiId)
        {
            if (uiType != POINT_MOTION_TYPE || uiId != 1)
                return;

            if (Creature* pHelboar = me->GetCreature(*me, uiHelboarGUID))
            {
                pHelboar->RemoveCorpse();
                DoCast(SPELL_SUMMON_POO);

                if (Player* owner = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                    me->GetMotionMaster()->MoveFollow(owner, 0.0f, 0.0f);
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (uiCheckTimer <= uiDiff)
            {
                if (Creature* pHelboar = me->FindNearestCreature(NPC_DERANGED_HELBOAR, 10.0f, false))
                {
                    if (pHelboar->GetGUID() != uiHelboarGUID && me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE && !me->FindCurrentSpellBySpellId(SPELL_SUMMON_POO))
                    {
                        uiHelboarGUID = pHelboar->GetGUID();
                        me->GetMotionMaster()->MovePoint(1, pHelboar->GetPositionX(), pHelboar->GetPositionY(), pHelboar->GetPositionZ());
                    }
                }
                uiCheckTimer = 5000;
            }
            else
                uiCheckTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_demoniac_scryer
######*/

#define GOSSIP_ITEM_ATTUNE          "Yes, Scryer. You may possess me."

enum
{
    GOSSIP_TEXTID_PROTECT           = 10659,
    GOSSIP_TEXTID_ATTUNED           = 10643,

    QUEST_DEMONIAC                  = 10838,
    NPC_HELLFIRE_WARDLING           = 22259,
    NPC_BUTTRESS                    = 22267,                //the 4x nodes
    NPC_SPAWNER                     = 22260,                //just a dummy, not used

    MAX_BUTTRESS                    = 4,
    TIME_TOTAL                      = MINUTE*10*IN_MILLISECONDS,

    SPELL_SUMMONED_DEMON            = 7741,                 //visual spawn-in for demon
    SPELL_DEMONIAC_VISITATION       = 38708,                //create item

    SPELL_BUTTRESS_APPERANCE        = 38719,                //visual on 4x bunnies + the flying ones
    SPELL_SUCKER_CHANNEL            = 38721,                //channel to the 4x nodes
    SPELL_SUCKER_DESPAWN_MOB        = 38691
};

class npc_demoniac_scryer : public CreatureScript
{
public:
    npc_demoniac_scryer() : CreatureScript("npc_demoniac_scryer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_demoniac_scryerAI(creature);
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->CLOSE_GOSSIP_MENU();
            creature->CastSpell(player, SPELL_DEMONIAC_VISITATION, false);
        }

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (CAST_AI(npc_demoniac_scryer::npc_demoniac_scryerAI, creature->AI())->m_bIsComplete)
        {
            if (player->GetQuestStatus(QUEST_DEMONIAC) == QUEST_STATUS_INCOMPLETE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ATTUNE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ATTUNED, creature->GetGUID());
            return true;
        }
        return false;
    }

    struct npc_demoniac_scryerAI : public ScriptedAI
    {
        npc_demoniac_scryerAI(Creature* creature) : ScriptedAI(creature)
        {
            m_bIsComplete = false;
            m_uiSpawnDemonTimer = 15000;
            m_uiSpawnButtressTimer = 45000;
            m_uiButtressCount = 0;
            Reset();
        }

        bool m_bIsComplete;

        uint32 m_uiSpawnDemonTimer;
        uint32 m_uiSpawnButtressTimer;
        uint32 m_uiButtressCount;

        void Reset() { }

        //we don't want anything to happen when attacked
        void AttackedBy(Unit* /*pEnemy*/) { }
        void AttackStart(Unit* /*pEnemy*/) { }

        void DoSpawnButtress()
        {
            ++m_uiButtressCount;

            float fAngle = 0.0f;

            switch(m_uiButtressCount)
            {
            case 1: fAngle = 0.0f; break;
            case 2: fAngle = M_PI+M_PI/2; break;
            case 3: fAngle = M_PI/2; break;
            case 4: fAngle = M_PI; break;
            }

            float fX, fY, fZ;
            me->GetNearPoint(me, fX, fY, fZ, 0.0f, 5.0f, fAngle);

            uint32 uiTime = TIME_TOTAL - (m_uiSpawnButtressTimer * m_uiButtressCount);

            me->SummonCreature(NPC_BUTTRESS, fX, fY, fZ, me->GetAngle(fX, fY), TEMPSUMMON_TIMED_DESPAWN, uiTime);
        }

        void DoSpawnDemon()
        {
            float fX, fY, fZ;
            Position pos;
            me->GetPosition(&pos);
            me->GetRandomPoint(pos, 20.0f, fX, fY, fZ);

            me->SummonCreature(NPC_HELLFIRE_WARDLING, fX, fY, fZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
        }

        void JustSummoned(Creature* pSummoned)
        {
            if (pSummoned->GetEntry() == NPC_HELLFIRE_WARDLING)
            {
                pSummoned->CastSpell(pSummoned, SPELL_SUMMONED_DEMON, true);
                pSummoned->AI()->AttackStart(me);
                pSummoned->AI()->Talk(0);
            }
            else
            {
                if (pSummoned->GetEntry() == NPC_BUTTRESS)
                {
                    pSummoned->CastSpell(pSummoned, SPELL_BUTTRESS_APPERANCE, false);
                    pSummoned->CastSpell(me, SPELL_SUCKER_CHANNEL, true);
                }
            }
        }

        void SpellHitTarget(Unit * target, const SpellInfo * spell)
        {
            if (target->GetEntry() == NPC_HELLFIRE_WARDLING && spell->Id == SPELL_SUCKER_DESPAWN_MOB)
                ((Creature*)target)->ForcedDespawn();
        }

        void UpdateAI(uint32 const uiDiff)
        {
            if (m_bIsComplete || !me->IsAlive())
                return;

            if (m_uiSpawnButtressTimer <= uiDiff)
            {
                if (m_uiButtressCount >= MAX_BUTTRESS)
                {
                    me->CastSpell(me, SPELL_SUCKER_DESPAWN_MOB, false);

                    if (me->IsInCombat())
                    {
                        me->RemoveAllAuras();
                        me->DeleteThreatList();
                        me->CombatStop(true);
                    }

                    m_bIsComplete = true;
                    return;
                }

                m_uiSpawnButtressTimer = 45000;
                DoSpawnButtress();
            }
            else
                m_uiSpawnButtressTimer -= uiDiff;

            if (m_uiSpawnDemonTimer <= uiDiff)
            {
                DoSpawnDemon();
                m_uiSpawnDemonTimer = 15000;
            }
            else
                m_uiSpawnDemonTimer -= uiDiff;
        }
    };
};

/*########################################
## Quest: The Exorcism of Colonel Jules
##########################################*/

enum ExorcismSpells
 {
		SPELL_JULES_GOES_PRONE = 39283, //Jules bleibt in de rluft liegen (fuer schwebephase)
		SPELL_JULES_THREATENS_AURA = 39284, //Visueller Effekt, bleibt das ganze Event lang aktiv
		SPELL_JULES_GOES_UPRIGHT = 39294, //Jules haengt kopfueber (fuer flugphase)
		SPELL_JULES_VOMITS_AURA = 39295, //Visueller Effekt fuer flugphase (uebelkeit und uebergeben)
		
		SPELL_BARADAS_COMMAND = 39277, //Castanimation
		SPELL_BARADA_FALTERS = 39278, //Cast startet die schwebephase (etwa 20sek nach eventbeginn)
		};

enum ExorcismTexts
 {
		SAY_BARADA_1 = 0,
		SAY_BARADA_2 = 1,
		SAY_BARADA_3 = 2,
		SAY_BARADA_4 = 3,
		SAY_BARADA_5 = 4,
		SAY_BARADA_6 = 5,
		SAY_BARADA_7 = 6,
		SAY_BARADA_8 = 7,
		
		SAY_JULES_1 = 0,
		SAY_JULES_2 = 1,
		SAY_JULES_3 = 2,
		SAY_JULES_4 = 3,
		SAY_JULES_5 = 4,
		};

Position const exorcismPos[11] =
{
	    { -707.123f, 2751.686f, 101.592f, 4.577416f }, //Barada Waypoint-1      0
		{ -710.731f, 2749.075f, 101.592f, 1.513286f }, //Barada Castposition    1
		{ -710.332f, 2754.394f, 102.948f, 3.207566f }, //Jules Schwebeposition  2
		{ -714.261f, 2747.754f, 103.391f, 0.0f },      //Jules Waypoint-1       3
		{ -713.113f, 2750.194f, 103.391f, 0.0f },      //Jules Waypoint-2       4
		{ -710.385f, 2750.896f, 103.391f, 0.0f },      //Jules Waypoint-3       5
		{ -708.309f, 2750.062f, 103.391f, 0.0f },      //Jules Waypoint-4       6
		{ -707.401f, 2747.696f, 103.391f, 0.0f },      //Jules Waypoint-5       7  uebergeben
		{ -708.591f, 2745.266f, 103.391f, 0.0f },      //Jules Waypoint-6       8
		{ -710.597f, 2744.035f, 103.391f, 0.0f },      //Jules Waypoint-7       9
		{ -713.089f, 2745.302f, 103.391f, 0.0f },      //Jules Waypoint-8      10
		};

enum ExorcismMisc
 {
		NPC_DARKNESS_RELEASED = 22507,
		NPC_FOUL_PURGE = 22506,
		NPC_COLONEL_JULES = 22432,
		
		BARADAS_GOSSIP_MESSAGE = 10683,
		
		QUEST_THE_EXORCISM_OF_COLONEL_JULES = 10935,
		
		ACTION_START_EVENT = 1,
		ACTION_JULES_HOVER = 2,
		ACTION_JULES_FLIGH = 3,
		ACTION_JULES_MOVE_HOME = 4,
		};

enum ExorcismEvents
 {
	EVENT_BARADAS_TALK = 1,
		
		    //Colonel Jules
		EVENT_SUMMON_SKULL = 1,
		};

class npc_barada : public CreatureScript
 {
	public:
		npc_barada() : CreatureScript("npc_barada") { }
		
			bool OnGossipHello(Player* player, Creature* creature)
			 {
			if (player->GetQuestStatus(QUEST_THE_EXORCISM_OF_COLONEL_JULES) == QUEST_STATUS_INCOMPLETE)
				 player->ADD_GOSSIP_ITEM(0, "I am ready, Anchorite. Let us begin the exorcism.", GOSSIP_SENDER_MAIN, 1);
			
				player->SEND_GOSSIP_MENU(BARADAS_GOSSIP_MESSAGE, creature->GetGUID());
			return true;
			}
		
			bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
			 {
			player->PlayerTalkClass->ClearMenus();
			if (action == 1)
				 {
				creature->AI()->Talk(SAY_BARADA_1);
				creature->GetAI()->DoAction(ACTION_START_EVENT);
				CAST_AI(npc_barada::npc_baradaAI, creature->GetAI())->playerGUID = player->GetGUID();
				player->CLOSE_GOSSIP_MENU();
				}
			return true;
			}
		
			struct npc_baradaAI : public ScriptedAI
			 {
			npc_baradaAI(Creature* creature) : ScriptedAI(creature) { }
			
			EventMap events;
			uint8 step;
			Creature* jules;
			uint64 playerGUID;
			
				void Reset()
				 {
				events.Reset();
				step = 0;
				
					playerGUID = 0;
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
				}
			
				void DoAction(const int32 action)
				 {
				if (action == ACTION_START_EVENT)
					 {
					jules = me->FindNearestCreature(NPC_COLONEL_JULES, 20.0f, true);
					
						if (jules)
						jules->AI()->Talk(SAY_JULES_1);
						
						me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
						me->SetSpeed(MOVE_WALK, 0.5f);
						me->GetMotionMaster()->MovePoint(0, exorcismPos[1]);
					Talk(SAY_BARADA_2);
					
						me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
					}
				}
			
				void MovementInform(uint32 type, uint32 id)
				 {
				if (type != POINT_MOTION_TYPE)
					 return;
				
					if (id == 0)
					 me->GetMotionMaster()->MovePoint(1, exorcismPos[1]);
				
					if (id == 1)
					 events.ScheduleEvent(EVENT_BARADAS_TALK, 2000);
				}
			
				void JustDied(Unit* /*killer*/)
				 {
				if (jules)
					{
					jules->GetAI()->DoAction(ACTION_JULES_MOVE_HOME);
					jules->RemoveAllAuras();
					}
				}
			
				void UpdateAI(uint32 const diff)
				 {
				events.Update(diff);
				
					while (uint32 eventId = events.ExecuteEvent())
					 {
					switch (eventId)
						{
						case EVENT_BARADAS_TALK:
							switch (step)
								{
								case 0:
									me->SetFacingTo(1.513286f);
									
										me->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
									events.ScheduleEvent(EVENT_BARADAS_TALK, 3000);
									step++;
									break;
									case 1:
										DoCast(SPELL_BARADAS_COMMAND);
										events.ScheduleEvent(EVENT_BARADAS_TALK, 5000);
										step++;
										break;
										case 2:
											Talk(SAY_BARADA_3);
											events.ScheduleEvent(EVENT_BARADAS_TALK, 7000);
											step++;
											break;
											case 3:
												if (jules)
													jules->AI()->Talk(SAY_JULES_2);
												
													events.ScheduleEvent(EVENT_BARADAS_TALK, 18000);
												step++;
												break;
												case 4:
													DoCast(SPELL_BARADA_FALTERS);
													me->HandleEmoteCommand(EMOTE_STAND_STATE_NONE);
													
														if (jules)
														jules->GetAI()->DoAction(ACTION_JULES_HOVER);
													
														events.ScheduleEvent(EVENT_BARADAS_TALK, 11000);
													step++;
													break;
													case 5:
														if (jules)
															jules->AI()->Talk(SAY_JULES_3);
														
															events.ScheduleEvent(EVENT_BARADAS_TALK, 13000);
														step++;
														break;
														case 6:
															Talk(SAY_BARADA_4);
															events.ScheduleEvent(EVENT_BARADAS_TALK, 5000);
															step++;
															break;
															case 7:
																if (jules)
																	jules->AI()->Talk(SAY_JULES_3);
																
																	events.ScheduleEvent(EVENT_BARADAS_TALK, 13000);
																step++;
																break;
																case 8:
																	Talk(SAY_BARADA_4);
																	events.ScheduleEvent(EVENT_BARADAS_TALK, 12000);
																	step++;
																	break;
																	case 9:
																		if (jules)
																			jules->AI()->Talk(SAY_JULES_4);
																		
																			events.ScheduleEvent(EVENT_BARADAS_TALK, 12000);
																		step++;
																		break;
																		case 10:
																			Talk(SAY_BARADA_4);
																			events.ScheduleEvent(EVENT_BARADAS_TALK, 5000);
																			step++;
																			break;
																			case 11:
																				if (jules)
																					jules->GetAI()->DoAction(ACTION_JULES_FLIGH);
																				
																					events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																				step++;
																				break;
																				case 12:
																					if (jules)
																						jules->AI()->Talk(SAY_JULES_4);
																					
																						events.ScheduleEvent(EVENT_BARADAS_TALK, 8000);
																					step++;
																					break;
																					case 13:
																						Talk(SAY_BARADA_5);
																						events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																						step++;
																						break;
																						case 14:
																							if (jules)
																								jules->AI()->Talk(SAY_JULES_4);
																							
																								events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																							step++;
																							break;
																							case 15:
																								Talk(SAY_BARADA_6);
																								events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																								step++;
																								break;
																								case 16:
																									if (jules)
																										jules->AI()->Talk(SAY_JULES_5);
																									
																										events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																									step++;
																									break;
																									case 17:
																										Talk(SAY_BARADA_7);
																										events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																										step++;
																										break;
																										case 18:
																											if (jules)
																												jules->AI()->Talk(SAY_JULES_3);
																											
																												events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																											step++;
																											break;
																											case 19:
																												Talk(SAY_BARADA_7);
																												events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																												step++;
																												break;
																												case 20:
																																														 
																													if (jules)
																														{
																														jules->GetAI()->DoAction(ACTION_JULES_MOVE_HOME);
																														jules->RemoveAura(SPELL_JULES_VOMITS_AURA);
																														}
																													
																														events.ScheduleEvent(EVENT_BARADAS_TALK, 10000);
																													step++;
																													break;
																													case 21:
																														                                //Ende
																															if (playerGUID)
																															{
																															if (Player* player = ObjectAccessor::FindPlayer(playerGUID))
																																player->KilledMonsterCredit(NPC_COLONEL_JULES, 0);
																																me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
																															}
																														
																															if (jules)
																															jules->RemoveAllAuras();
																														
																															me->RemoveAura(SPELL_BARADAS_COMMAND);
																															me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
																														
																															Talk(SAY_BARADA_8);
																														me->GetMotionMaster()->MoveTargetedHome();
																														EnterEvadeMode();
																														break;
																														}
							break;
							}
					}
				}
			};
		
			CreatureAI* GetAI(Creature* creature) const override
			 {
			return new npc_baradaAI(creature);
			}
		};

class npc_colonel_jules : public CreatureScript
 {
	public:
		npc_colonel_jules() : CreatureScript("npc_colonel_jules") { }
		
			struct npc_colonel_julesAI : public ScriptedAI
			 {
			npc_colonel_julesAI(Creature* creature) : ScriptedAI(creature), summons(me) { }
			
				EventMap events;
				SummonList summons;
			
				uint8 circleRounds;
				uint8 point;
			
				bool wpreached;
			
				void Reset()
				 {
				events.Reset();
				
					summons.DespawnAll();
				circleRounds = 0;
				point = 3;
				wpreached = false;
				}
			
				void DoAction(const int32 action)
				 {
				switch (action)
					{
					case ACTION_JULES_HOVER:
						me->AddAura(SPELL_JULES_GOES_PRONE, me);
						me->AddAura(SPELL_JULES_THREATENS_AURA, me);
						
							me->SetCanFly(true);
							me->SetSpeed(MOVE_RUN, 0.2f);
						
							me->SetFacingTo(3.207566f);
						                    //MovePoint moechte er nicht, warum auch immer...
							                    //me->GetMotionMaster()->MovePoint(11, exorcismPos[2]);
							me->GetMotionMaster()->MoveJump(exorcismPos[2], 2.0f, 2.0f);
						
							events.ScheduleEvent(EVENT_SUMMON_SKULL, 10000);
						break;
						case ACTION_JULES_FLIGH:
							circleRounds++;
							
								me->RemoveAura(SPELL_JULES_GOES_PRONE);
							
								me->AddAura(SPELL_JULES_GOES_UPRIGHT, me);
								me->AddAura(SPELL_JULES_VOMITS_AURA, me);
							
								wpreached = true;
							me->GetMotionMaster()->MovePoint(point, exorcismPos[point]);
							break;
							case ACTION_JULES_MOVE_HOME:
								wpreached = false;
								me->SetSpeed(MOVE_RUN, 1.0f);
								me->GetMotionMaster()->MovePoint(11, exorcismPos[2]);
								
									events.CancelEvent(EVENT_SUMMON_SKULL);
								break;
								}
				}
			
				void JustSummoned(Creature* summon)
				 {
				summons.Summon(summon);
				
					if (summon->GetAI())
						summon->GetMotionMaster()->MoveRandom(10.0f);
				}
			
				void MovementInform(uint32 type, uint32 id)
				 {
				if (type != POINT_MOTION_TYPE)
					 return;
				
					if (id < 10)
					 wpreached = true;
				
					if (id == 8)
					 {
					for (uint8 i = 0; i < (circleRounds /2); i++)
						 DoSummon(NPC_FOUL_PURGE, exorcismPos[8]);
					}
				
					if (id == 10)
					 {
					wpreached = true;
					point = 3;
					circleRounds++;
					}
				}
			
				void UpdateAI(uint32 const diff)
				 {
				if (wpreached)
					{
					me->GetMotionMaster()->MovePoint(point, exorcismPos[point]);
					point++;
					wpreached = false;
					}
				
					events.Update(diff);
				
					while (uint32 eventId = events.ExecuteEvent())
					 {
					switch (eventId)
						{
						case EVENT_SUMMON_SKULL:
							uint8 summonCount = urand(1, 2);
							
								for (uint8 i = 0; i < summonCount; i++)
								 me->SummonCreature(NPC_DARKNESS_RELEASED, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 1.5f, 0, TEMPSUMMON_MANUAL_DESPAWN);
							
								events.ScheduleEvent(EVENT_SUMMON_SKULL, urand(10000, 15000));
							break;
							}
					}
				}
			};
		
			CreatureAI* GetAI(Creature* creature) const override
			 {
			return new npc_colonel_julesAI(creature);
			}
		};
void AddSC_hellfire_peninsula()
{
    new npc_aeranas();
    new npc_ancestral_wolf();
    new go_haaleshi_altar();
    new npc_naladu();
    new npc_tracy_proudwell();
    new npc_trollbane();
    new npc_wounded_blood_elf();
    new npc_fel_guard_hound();
	new npc_barada();
	new npc_colonel_jules();
    new npc_demoniac_scryer();
}
