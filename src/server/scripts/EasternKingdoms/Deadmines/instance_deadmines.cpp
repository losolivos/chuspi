/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "Vehicle.h"
#include "ScriptedSmoothEscortAI.h"
#include "SpellScript.h"
#include "ScriptedGossip.h"
#include "CreatureTextMgr.h"
#include "deadmines.h"

enum eSpells
{
    SPELL_HELIX_RIDE                      = 88337,
    SPELL_RIDE_FACE_TARGETTING            = 88349,
};

enum eScriptTexts
{
    // Vanessa At Ship (VAS) - intro
    VAS_EMOTE_NOISE                       = 0,
    VAS_SAY_1                             = 1,
    VAS_SAY_2                             = 2,
    VAS_SAY_3                             = 3,
    VAS_SAY_4                             = 4,
    VAS_SAY_5                             = 5,
    VAS_EMOTE_ELIXIR                      = 6,
    // Vanesssa Nightmare Event (VNE)
    VNE_TRAP_EMOTE_ACTIVATE_VALVES        = 0,
    VNE_EMOTE_NIGHTMARE_TAKE_HOLD         = 0,
    VNE_EMOTE_NIGHTMARE_CHANGE            = 1,
    // Glibtok Nightmare
    VNE_SAY_GLUBTOK_1                     = 2,
    VNE_SAY_GLUBTOK_2                     = 3,
    VNE_EMOTE_GLUBTOK_NIGHTMARE           = 4,
    // Helix Nightmare
    VNE_SAY_HELIX_1                       = 5,
    VNE_SAY_HELIX_2                       = 6,
    VNE_EMOTE_HELIX_NIGHTMARE_1           = 0,
    VNE_EMOTE_HELIX_NIGHTMARE_2           = 1,
    // Foe Reaper 5000 Nightmare
    VNE_SAY_FOE_REAPER_1                  = 7,
    VNE_SAY_FOE_REAPER_2                  = 8,
    VNE_EMOTE_FOE_REAPER_NIGHTMARE        = 9,
    // Ripsnarl Nightmare
    VNE_SAY_RIPSNARL_1                    = 10,
    VNE_SAY_RIPSNARL_2                    = 11,
    VNE_EMOTE_RIPSNARL_NIGHTMARE          = 12,
    VNE_EMOTE_SAVE_EMME                   = 0,
    VNE_EMOTE_SAVE_ERIK                   = 1,
    VNE_EMOTE_SAVE_CALISSA                = 2,
    VNE_EMOTE_NIGHTMARE_OVER              = 3,
    VNE_RIPSNARL_SAY_SORRY                = 0,
    VNE_CALISSA_SAY_I_LOVE                = 0,
    VNE_CALISSA_DYING                     = 1,
};

enum eEvents
{
    // Vanessa Event, ship intro
    EVENT_START_VANESSA_INTRO             = 1,
    EVENT_VANESSA_AT_SHIP_HELLO_EMOTE     = 2,
    EVENT_VAS_SAY_1                       = 3,
    EVENT_VAS_SAY_2                       = 4,
    EVENT_VAS_SAY_3                       = 5,
    EVENT_VAS_SAY_4                       = 6,
    EVENT_VAS_SAY_5                       = 7,
    EVENT_VAS_JUMP                        = 9,
    EVENT_VAS_CAST_POISON                 = 10,
    EVENT_VAS_ELIXIR                      = 11,
    EVENT_TELEPORT_PLAYERS                = 12,
    // Vanessa Event, Magma Vehicle
    EVENT_RIDE_MAGMA_VEHICLE              = 13,
    EVENT_MAGMA_TRAP_THROW                = 14,
    EVENT_PLAYERS_JUMP                    = 15,
    EVENT_VEHICLE_EMOTE_ACTIVATE_VALVES   = 16,
    // Vanessa Event, Glubtok Nightmare
    EVENT_START_GLUBTOK_NIGHTMARE         = 17,
    EVENT_VNE_SAY_GLUBTOK_1               = 18,
    EVENT_VNE_SAY_GLUBTOK_2               = 19,
    EVENT_GLUBTOK_NIGHTMARE               = 20,
    EVENT_ICICLE_TARGETING                = 21,
    EVENT_ICICLE                          = 22,
    // Vanessa Event, Helix Nightmare
    EVENT_START_HELIX_NIGHTMARE           = 23,
    EVENT_VNE_SAY_HELIX_1                 = 24,
    EVENT_VNE_SAY_HELIX_2                 = 25,
    EVENT_HELIX_NIGHTMARE                 = 26,
    EVENT_VNE_HELIX_EMOTE_SPIDER          = 27,
    // Vanessa Event, Foe Reaper Nightmare
    EVENT_START_REAPER_NIGHTMARE          = 28,
    EVENT_VNE_SAY_REAPER_1                = 29,
    EVENT_VNE_SAY_REAPER_2                = 30,
    EVENT_REAPER_NIGHTMARE                = 31,
    // Vanessa Event, Ripsnarl Nightmare
    EVENT_START_RIPSNARL_NIGHTMARE        = 32,
    EVENT_VNE_SAY_RIPSNARL_1              = 33,
    EVENT_VNE_SAY_RIPSNARL_2              = 34,
    EVENT_RIPSNARL_NIGHTMARE              = 35,
    EVENT_EMOTE_SAVE_EMME                 = 36,
    EVENT_EMOTE_SAVE_ERIK                 = 37,
    EVENT_EMOTE_SAVE_CALISSA              = 38,
    EVENT_NIGHTMARE_OVER                  = 39,
    EVENT_NIGHTMARE_CHECK_VALID_PLAYER    = 41,
    // Boss Spell
    EVENT_SPIRIT_STRIKE                   = 1, // All boss
    EVENT_ACTIVATE_SPIDERS                = 2, // Helix
    EVENT_HELIX_RIDE                      = 3, // Helix
    EVENT_INSTALL_WALL_LEVEL_ONE          = 4, // Reaper 5000
    EVENT_CHANGE_ANGLE_L                  = 6, // Reaper 5000
    EVENT_CHANGE_ANGLE_R                  = 7, // Reaper 5000
    EVENT_TAUNT                           = 7, // Ripsnarl
    EVENT_CLARISSA_SAY                    = 8,
    EVENT_RIPSNARL_SAY                    = 9,
    // Defias cannon
    EVENT_SHOOT                           = 1,
    EVENT_INSTALL_PASSENGER               = 2,
    // Monkey
    EVENT_TROWN                           = 1,
};

enum eActions
{
    ACTION_FREE                           = 1
};

const Position IntroVanessaPos = {-75.58507f, -819.9636f, 47.06727f, 6.178465f};
const Position VASJumpPos = {-65.4138f, -819.954f, 41.0384f, 0.0f};
const Position VASMovePos = {-66.7813f, -837.863f, 41.063f, 4.60767f};

const uint32 MonkeyEntry[4] = {48278, 48440, 48441, 48442};

const Position ValveSpawnPos[4]=
{
    {-212.352f, -575.398f, 37.3316f, 5.70723f},
    {-209.682f, -585.892f, 37.3316f, 0.95993f},
    {-199.194f, -583.154f, 37.3316f, 2.60054f},
    {-201.809f, -572.622f, 37.3316f, 4.17134f},
};

// Vanessa and Glubtok
const Position GlubtokEvent[3]=
{
    {-230.717f, -563.014f, 51.3129f, 1.0472f},
    {-229.340f, -560.363f, 51.3129f, 5.7421f},
    {-174.821f, -579.755f, 19.396f, 3.19395f},
};

const Position IcecleCasterSpawPos[2]=
{
    {-211.918f, -581.821f, 21.0601f, 0.0f},
    {-96.0469f, -694.434f, 8.50863f, 0.0f},
};

// Vanessa and Helix
const Position HelixEvent[2]=
{
    {-172.809f, -576.911f, 19.3976f, 4.8171f},
    {-172.686f, -580.047f, 19.3975f, 3.1765f},
};

// Vanessa and Reaper
const Position ReaperEvent[3]=
{
    {-159.392f, -580.517f, 19.3982f, 6.19592f},
    {-155.471f, -580.695f, 19.3978f, 0.00000f},
    {-101.455f, -663.649f, 7.50581f, 1.85005f},
};

// Vanessa and Ripsnarl
const Position RipsnarlEvent[3]=
{
    {-98.5816f, -662.663f, 7.49801f, 3.49066f},
    {-101.214f, -663.453f, 7.50482f, 1.86750f},
    {-83.1632f, -774.964f, 26.9035f, 1.71042f},
};

const Position EmmeWorgensSpawnPos[3]=
{
    {-101.917f, -718.755f, 8.72638f, 5.51524f},
    {-94.4028f, -719.727f, 8.59865f, 3.56047f},
    {-97.7917f, -717.854f, 8.66809f, 4.52040f},
};

const Position ErikWorgensSpawnPos[3]=
{
    {3.13715f, -760.031f, 9.72600f, 5.39307f},
    {4.04340f, -767.278f, 9.82791f, 1.43117f},
    {9.17882f, -762.134f, 9.54959f, 3.38594f},
};

#define ACTION_START_RUN          1

const Position EngineerWP[20]=
{
    {-208.083f, -606.835f, 28.4378f, 0.0f},
    {-201.707f, -606.191f, 30.9627f, 0.0f},
    {-192.853f, -602.701f, 34.5089f, 0.0f},
    {-185.060f, -595.575f, 39.0992f, 0.0f},
    {-180.563f, -588.515f, 42.2823f, 0.0f},
    {-179.391f, -580.213f, 45.5360f, 0.0f},
    {-180.439f, -571.626f, 49.0135f, 0.0f},
    {-182.718f, -566.373f, 51.2297f, 0.0f},
    {-188.065f, -560.440f, 51.2297f, 0.0f},
    {-195.981f, -555.304f, 51.2297f, 0.0f},
    {-204.066f, -553.000f, 51.2297f, 0.0f},
    {-212.691f, -554.166f, 51.2297f, 0.0f},
    {-221.570f, -558.759f, 51.2297f, 0.0f},
    {-226.836f, -564.753f, 51.2297f, 0.0f},
    {-229.837f, -571.348f, 51.2364f, 0.0f},
    {-232.957f, -577.111f, 51.2233f, 0.0f},
    {-238.753f, -578.478f, 51.2046f, 0.0f},
    {-245.617f, -578.495f, 51.1501f, 0.0f},
    {-252.936f, -578.274f, 51.1501f, 0.0f},
    {-257.755f, -578.086f, 51.1501f, 0.0f},
};

DoorData const doorData[] =
{
    {13965,  DATA_GLUBTOK,           DOOR_TYPE_PASSAGE,   BOUNDARY_NONE},
    {17153,  DATA_HELIX_GEARBREAKER, DOOR_TYPE_ROOM,      BOUNDARY_NONE},
    {16400,  DATA_HELIX_GEARBREAKER, DOOR_TYPE_PASSAGE,   BOUNDARY_NONE},
    {17154,  DATA_FOE_REAPER_5000,   DOOR_TYPE_ROOM,      BOUNDARY_NONE},
    {16399,  DATA_FOE_REAPER_5000,   DOOR_TYPE_PASSAGE,   BOUNDARY_NONE},
    {0,      0,                      DOOR_TYPE_ROOM,      BOUNDARY_NONE},
};

class instance_deadmines : public InstanceMapScript
{
public:
    instance_deadmines() : InstanceMapScript("instance_deadmines", 36){ }

    struct instance_deadmines_InstanceMapScript : public InstanceScript
    {
        instance_deadmines_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_BOSS_ENCOUNTER);
            LoadDoorData(doorData);
        }

        std::list<uint64> CannonGUID;
        uint32 Encounter[MAX_ENCOUNTER];
        uint64 DoorGUID[3];
        uint64 uiGlubtokGUID;
        uint64 helixGUID;
        uint64 uiFoeReaper5000GUID;
        uint64 uiCookieGUID;
        uint64 uiNoteFromVanessaGUID;
        uint64 uiEventVanessaGUID;
        uint64 uiTrapGUID;
        uint64 uiAnchorGUID;
        uint64 uiIcicleCasterGUID;
        uint64 uiEmmeGUID;
        uint64 uiErikGUID;
        uint64 uiCalissaGUID;
        uint8 uiActiveValveCount;
        bool reaperEngineer;
        EventMap events;

        void Initialize()
        {
            uiGlubtokGUID = 0;
            helixGUID = 0;
            uiFoeReaper5000GUID = 0;
            uiCookieGUID = 0;
            uiEventVanessaGUID = 0;
            uiTrapGUID = 0;
            uiAnchorGUID = 0;
            uiIcicleCasterGUID = 0;
            uiActiveValveCount = 0;
            uiEmmeGUID = 0;
            uiErikGUID = 0;
            uiCalissaGUID = 0;
            reaperEngineer = false;
            events.Reset();
            memset(&Encounter, NOT_STARTED, sizeof(Encounter));
            memset(&DoorGUID, 0, sizeof(DoorGUID));
            CannonGUID.clear();
        }

        bool CheckAchievementCriteriaMeet(uint32 criteria_id, Player const* /*source*/, Unit const* /*target = NULL*/, uint32 /*miscvalue1 = 0*/)
        {
            return criteria_id == 16211 && Encounter[DATA_ADMIRA_RIPSNARL] == IN_PROGRESS;
        }

        void OnPlayerExit(Player* player)
        {
            player->SetPhaseMask(1, true);
        }

        void Update(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_VANESSA_INTRO:
                    {
                        if (Creature* vanessa = instance->SummonCreature(49429, IntroVanessaPos))
                        {
                            uiEventVanessaGUID = vanessa->GetGUID();
                            vanessa->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                            vanessa->CastSpell(vanessa, 89279, false);
                            vanessa->setActive(true);
                        }

                        events.ScheduleEvent(EVENT_VANESSA_AT_SHIP_HELLO_EMOTE, 3000);
                        break;
                    }
                    case EVENT_VANESSA_AT_SHIP_HELLO_EMOTE:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VAS_EMOTE_NOISE);

                        events.ScheduleEvent(EVENT_VAS_SAY_1, 3000);
                        break;
                    }
                    case EVENT_VAS_SAY_1:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VAS_SAY_1);

                        events.ScheduleEvent(EVENT_VAS_JUMP, 1000);
                        break;
                    }
                    case EVENT_VAS_JUMP:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                        {
                            vanessa->RemoveAura(89279);
                            vanessa->GetMotionMaster()->MoveJump(VASJumpPos.m_positionX, VASJumpPos.m_positionY, VASJumpPos.m_positionZ, 10.0f, 10.0f);
                        }

                        events.ScheduleEvent(EVENT_VAS_SAY_2, 5000);
                        break;
                    }
                    case EVENT_VAS_SAY_2:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                        {
                            vanessa->GetMotionMaster()->MovePoint(0, VASMovePos);
                            vanessa->SetHomePosition(VASMovePos.m_positionX, VASMovePos.m_positionY, VASMovePos.m_positionZ, 1.48f);

                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VAS_SAY_2);
                        }

                        events.ScheduleEvent(EVENT_VAS_SAY_3, 10000);
                        break;
                    }
                    case EVENT_VAS_SAY_3:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VAS_SAY_3);

                        events.ScheduleEvent(EVENT_VAS_SAY_4, 11000);
                        break;
                    }
                    case EVENT_VAS_SAY_4:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                        {
                            vanessa->SetFacingTo(1.48f);

                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VAS_SAY_4);
                        }

                        events.ScheduleEvent(EVENT_VAS_CAST_POISON, 3000);
                        break;
                    }
                    case EVENT_VAS_CAST_POISON:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            vanessa->CastSpell((Unit*)NULL, 92100, false);

                        events.ScheduleEvent(EVENT_VAS_SAY_5, 2000);
                        break;
                    }
                    case EVENT_VAS_SAY_5:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VAS_SAY_5);

                        events.ScheduleEvent(EVENT_VAS_ELIXIR, 5000);
                        break;
                    }
                    case EVENT_VAS_ELIXIR:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                        {
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VAS_EMOTE_ELIXIR);

                            vanessa->CastSpell((Unit*)NULL, 92113, true);
                            vanessa->CastSpell((Unit*)NULL, 92120, true);
                            vanessa->DespawnOrUnsummon(2000);
                            uiEventVanessaGUID = 0;
                        }

                        events.ScheduleEvent(EVENT_TELEPORT_PLAYERS, 1500);
                        break;
                    }
                    case EVENT_TELEPORT_PLAYERS:
                    {
                        Map::PlayerList const &PlayerList = instance->GetPlayers();

                        if (PlayerList.isEmpty())
                        {
                            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                            return;
                        }

                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                        {
                            if (Player* player = itr->GetSource())
                            {
                                if (player->IsInWorld())
                                {
                                    player->SetPhaseMask(2, true);
                                    player->RemovePet(PET_REMOVE_DISMISS, PET_REMOVE_FLAG_RETURN_REAGENT | PET_REMOVE_FLAG_RESET_CURRENT);
                                    player->UpdatePosition(-205.757f, -579.097f, 42.9862f, 0.0f, false);
                                }
                            }
                        }

                        events.ScheduleEvent(EVENT_RIDE_MAGMA_VEHICLE, 1500);
                        break;
                    }
                    case EVENT_RIDE_MAGMA_VEHICLE:
                    {
                        if (Creature* trap = instance->GetCreature(uiTrapGUID))
                        {
                            trap->CastSpell((Unit*)NULL, 92378, true);

                            if (Creature* anchor = instance->GetCreature(uiAnchorGUID))
                                anchor->CastSpell(trap, 43785, true);

                            for (int i = 0; i < 4; ++i)
                                if (Creature* valve = instance->SummonCreature(49457, ValveSpawnPos[i]))
                                    valve->SetPhaseMask(2, true);
                        }

                        events.ScheduleEvent(EVENT_VEHICLE_EMOTE_ACTIVATE_VALVES, 10000);
                        break;
                    }
                    case EVENT_VEHICLE_EMOTE_ACTIVATE_VALVES:
                    {
                        if (Creature* trap = instance->GetCreature(uiTrapGUID))
                        {
                            if (trap->IsAIEnabled)
                                trap->AI()->Talk(VNE_TRAP_EMOTE_ACTIVATE_VALVES);

                            std::list<Creature*> lValves;
                            trap->GetCreatureListWithEntryInGrid(lValves, 49457, 30.0f);

                            if (!lValves.empty())
                                for (std::list<Creature*>::const_iterator itr = lValves.begin(); itr != lValves.end(); ++itr)
                                    if (*itr)
                                        (*itr)->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                        }
                        break;
                    }
                    case EVENT_MAGMA_TRAP_THROW:
                    {
                        if (Creature* trap = instance->GetCreature(uiTrapGUID))
                            trap->CastSpell(trap, 68576, true);

                        events.ScheduleEvent(EVENT_PLAYERS_JUMP, 50);
                        break;
                    }
                    case EVENT_PLAYERS_JUMP:
                    {
                        Map::PlayerList const &PlayerList = instance->GetPlayers();

                        if (PlayerList.isEmpty())
                        {
                            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                            return;
                        }

                        if (Creature* trap = instance->GetCreature(uiTrapGUID))
                            for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                                if (Player* player = itr->GetSource())
                                    trap->CastSpell(player, 92489, false);

                        events.ScheduleEvent(EVENT_START_GLUBTOK_NIGHTMARE, 1000);
                        break;
                    }
                    case EVENT_START_GLUBTOK_NIGHTMARE:
                    {
                        DoStartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, 27527);
                        Map::PlayerList const &PlayerList = instance->GetPlayers();

                        if (PlayerList.isEmpty())
                        {
                            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                            return;
                        }

                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->GetSource())
                            {
                                player->SetPhaseMask(4, true);
                                player->AddAura(92559, player);
                                player->AddAura(92563, player);
                            }

                        if (Creature* vanessa = instance->SummonCreature(49671, GlubtokEvent[0], NULL, 14000))
                        {
                            vanessa->setActive(true);
                            vanessa->SetPhaseMask(4, true);
                            uiEventVanessaGUID = vanessa->GetGUID();
                            sCreatureTextMgr->SendChat(vanessa, VNE_EMOTE_NIGHTMARE_TAKE_HOLD, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                            if (Creature* glubtok_image = instance->SummonCreature(49670, GlubtokEvent[1], NULL, 14000))
                            {
                                glubtok_image->SetPhaseMask(4, true);
                                glubtok_image->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                glubtok_image->SetReactState(REACT_PASSIVE);
                            }

                            if (Creature* glubtok = instance->SummonCreature(49670, GlubtokEvent[2]))
                                glubtok->SetPhaseMask(4, true);
                        }

                        events.ScheduleEvent(EVENT_VNE_SAY_GLUBTOK_1, 1000);
                        break;
                    }
                    case EVENT_VNE_SAY_GLUBTOK_1:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VNE_SAY_GLUBTOK_1);

                        events.ScheduleEvent(EVENT_VNE_SAY_GLUBTOK_2, 7000);
                        break;
                    }
                    case EVENT_VNE_SAY_GLUBTOK_2:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VNE_SAY_GLUBTOK_2);

                        events.ScheduleEvent(EVENT_GLUBTOK_NIGHTMARE, 4000);
                        break;
                    }
                    case EVENT_GLUBTOK_NIGHTMARE:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                        {
                            vanessa->CastSpell((Unit*)NULL, 92583, true);
                            vanessa->CastSpell((Unit*)NULL, 92565, true);
                            sCreatureTextMgr->SendChat(vanessa, VNE_EMOTE_GLUBTOK_NIGHTMARE, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
                        }

                        if (Creature* icecle_caster = instance->SummonCreature(45979, IcecleCasterSpawPos[0]))
                        {
                            uiIcicleCasterGUID = icecle_caster->GetGUID();
                            icecle_caster->SetPhaseMask(4, true);
                            events.ScheduleEvent(EVENT_ICICLE_TARGETING, urand(10000, 20000));
                        }
                        break;
                    }
                    case EVENT_ICICLE_TARGETING:
                    {
                        if (Creature* icecle_caster = instance->GetCreature(uiIcicleCasterGUID))
                            icecle_caster->CastSpell((Unit*)NULL, 92210, false);

                        events.ScheduleEvent(EVENT_ICICLE_TARGETING, urand(3000, 10000));
                        break;
                    }
                    case EVENT_START_HELIX_NIGHTMARE:
                    {
                        Map::PlayerList const &PlayerList = instance->GetPlayers();

                        if (PlayerList.isEmpty())
                        {
                            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                            return;
                        }

                        if (Creature* icecle_caster = instance->GetCreature(uiIcicleCasterGUID))
                        {
                            icecle_caster->CastSpell((Unit*)NULL, 92584, false);
                            icecle_caster->SetPhaseMask(8, true);
                        }

                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->GetSource())
                            {
                                player->SetPhaseMask(8, true);
                                player->AddAura(92563, player);
                            }

                        if (Creature* vanessa = instance->SummonCreature(49671, HelixEvent[0], NULL, 15000))
                        {
                            vanessa->setActive(true);
                            vanessa->SetPhaseMask(8, true);
                            uiEventVanessaGUID = vanessa->GetGUID();
                            sCreatureTextMgr->SendChat(vanessa, VNE_EMOTE_NIGHTMARE_CHANGE, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                            if (Creature* helix = instance->SummonCreature(49674, HelixEvent[1]))
                            {
                                helix->SetPhaseMask(8, true);
                                helix->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                helix->SetReactState(REACT_PASSIVE);
                            }
                        }

                        events.ScheduleEvent(EVENT_VNE_SAY_HELIX_1, 5000);
                        break;
                    }
                    case EVENT_VNE_SAY_HELIX_1:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VNE_SAY_HELIX_1);

                        events.ScheduleEvent(EVENT_VNE_SAY_HELIX_2, 5000);
                        break;
                    }
                    case EVENT_VNE_SAY_HELIX_2:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VNE_SAY_HELIX_2);

                        events.ScheduleEvent(EVENT_HELIX_NIGHTMARE, 4000);
                        break;
                    }
                    case EVENT_HELIX_NIGHTMARE:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                        {
                            vanessa->CastSpell((Unit*)NULL, 92583, true);
                            vanessa->CastSpell((Unit*)NULL, 92566, true);
                        }
                        break;
                    }
                    case EVENT_START_REAPER_NIGHTMARE:
                    {
                        HandleGameObject(DoorGUID[1], true);
                        Map::PlayerList const &PlayerList = instance->GetPlayers();

                        if (PlayerList.isEmpty())
                        {
                            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                            return;
                        }

                        if (Creature* icecle_caster = instance->GetCreature(uiIcicleCasterGUID))
                            icecle_caster->CastSpell((Unit*)NULL, 92585, false);

                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->GetSource())
                            {
                                player->SetPhaseMask(16, true);
                                player->AddAura(92563, player);
                            }

                        if (Creature* vanessa = instance->SummonCreature(49671, ReaperEvent[0], NULL, 14000))
                        {
                            vanessa->setActive(true);
                            vanessa->SetPhaseMask(16, true);
                            uiEventVanessaGUID = vanessa->GetGUID();
                            sCreatureTextMgr->SendChat(vanessa, VNE_EMOTE_NIGHTMARE_CHANGE, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                            if (Creature* reaper = instance->SummonCreature(49681, ReaperEvent[1], NULL, 14000))
                            {
                                reaper->SetPhaseMask(16, true);
                                reaper->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                reaper->SetReactState(REACT_PASSIVE);
                                reaper->CastSpell(reaper, 88348, false);
                            }

                            if (Creature* reaper = instance->SummonCreature(49681, ReaperEvent[2]))
                            {
                                reaper->SetPhaseMask(16, true);
                                reaper->CastSpell(reaper, 24263, false);
                            }
                        }

                        events.ScheduleEvent(EVENT_VNE_SAY_REAPER_1, 4000);
                        break;
                    }
                    case EVENT_VNE_SAY_REAPER_1:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VNE_SAY_FOE_REAPER_1);

                        events.ScheduleEvent(EVENT_VNE_SAY_REAPER_2, 3000);
                        break;
                    }
                    case EVENT_VNE_SAY_REAPER_2:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VNE_SAY_FOE_REAPER_2);

                        events.ScheduleEvent(EVENT_REAPER_NIGHTMARE, 4000);
                        break;
                    }
                    case EVENT_REAPER_NIGHTMARE:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                        {
                            vanessa->CastSpell((Unit*)NULL, 92583, true);
                            vanessa->CastSpell((Unit*)NULL, 92567, true);
                            sCreatureTextMgr->SendChat(vanessa, VNE_EMOTE_FOE_REAPER_NIGHTMARE, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
                        }
                        break;
                    }
                    case EVENT_START_RIPSNARL_NIGHTMARE:
                    {
                        if (Creature* icecle_caster = instance->SummonCreature(45979, IcecleCasterSpawPos[1]))
                        {
                            icecle_caster->SetPhaseMask(32, true);
                            uiIcicleCasterGUID = icecle_caster->GetGUID();
                            icecle_caster->CastSpell((Unit*)NULL, 92586, false);
                        }

                        Map::PlayerList const &PlayerList = instance->GetPlayers();

                        if (PlayerList.isEmpty())
                        {
                            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                            return;
                        }

                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->GetSource())
                            {
                                player->SetPhaseMask(32, true);
                                player->AddAura(92563, player);
                            }

                        if (Creature* vanessa = instance->SummonCreature(49671, RipsnarlEvent[0], NULL, 18000))
                        {
                            vanessa->setActive(true);
                            vanessa->SetPhaseMask(32, true);
                            uiEventVanessaGUID = vanessa->GetGUID();
                            sCreatureTextMgr->SendChat(vanessa, VNE_EMOTE_NIGHTMARE_CHANGE, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                            if (Creature* ripsnarl = instance->SummonCreature(49682, RipsnarlEvent[1], NULL, 18000))
                                ripsnarl->SetPhaseMask(32, true);
                        }

                        events.ScheduleEvent(EVENT_VNE_SAY_RIPSNARL_1, 3000);
                        break;
                    }
                    case EVENT_VNE_SAY_RIPSNARL_1:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VNE_SAY_RIPSNARL_1);

                        events.ScheduleEvent(EVENT_VNE_SAY_RIPSNARL_2, 5000);
                        break;
                    }
                    case EVENT_VNE_SAY_RIPSNARL_2:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            if (vanessa->IsAIEnabled)
                                vanessa->AI()->Talk(VNE_SAY_RIPSNARL_2);

                        events.ScheduleEvent(EVENT_RIPSNARL_NIGHTMARE, 5000);
                        break;
                    }
                    case EVENT_RIPSNARL_NIGHTMARE:
                    {
                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                        {
                            vanessa->CastSpell((Unit*)NULL, 92583, false);
                            vanessa->CastSpell((Unit*)NULL, 92568, false);
                            sCreatureTextMgr->SendChat(vanessa, VNE_EMOTE_RIPSNARL_NIGHTMARE, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
                        }

                        events.ScheduleEvent(EVENT_EMOTE_SAVE_EMME, 3000);
                        break;
                    }
                    case EVENT_EMOTE_SAVE_EMME:
                    {
                        HandleGameObject(DoorGUID[2], true);

                        if (Creature* vanessa = instance->GetCreature(uiEventVanessaGUID))
                            vanessa->CastSpell((Unit*)NULL, 92604, false);

                        if (Creature* icecle_caster = instance->GetCreature(uiIcicleCasterGUID))
                            sCreatureTextMgr->SendChat(icecle_caster, VNE_EMOTE_SAVE_EMME, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                        if (Creature* emme = instance->GetCreature(uiEmmeGUID))
                        {
                            for (int i = 0; i < 3; ++i)
                                if (Creature* worgen = emme->SummonCreature(49532, EmmeWorgensSpawnPos[i]))
                                    if (worgen->IsAIEnabled)
                                        worgen->AI()->AttackStart(emme);

                            emme->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            emme->CastSpell((Unit*)NULL, 92308, false);
                        }
                        break;
                    }
                    case EVENT_EMOTE_SAVE_ERIK:
                    {
                        if (Creature* icecle_caster = instance->GetCreature(uiIcicleCasterGUID))
                            sCreatureTextMgr->SendChat(icecle_caster, VNE_EMOTE_SAVE_ERIK, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                        if (Creature* erik = instance->GetCreature(uiErikGUID))
                        {
                            for (int i = 0; i < 3; ++i)
                                if (Creature* worgen = erik->SummonCreature(49532, ErikWorgensSpawnPos[i]))
                                    if (worgen->IsAIEnabled)
                                        worgen->AI()->AttackStart(erik);

                            erik->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            erik->CastSpell((Unit*)NULL, 92308, false);
                        }
                        break;
                    }
                    case EVENT_EMOTE_SAVE_CALISSA:
                    {
                        if (Creature* icecle_caster = instance->GetCreature(uiIcicleCasterGUID))
                            sCreatureTextMgr->SendChat(icecle_caster, VNE_EMOTE_SAVE_CALISSA, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);

                        if (Creature* calissa = instance->GetCreature(uiCalissaGUID))
                            if (Creature* james = calissa->GetVehicleCreatureBase())
                            {
                                james->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                calissa->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                calissa->AddAura(92608, calissa);
                                calissa->AddThreat(james, 0.0f);
                                james->AddThreat(calissa, 0.0f);
                                calissa->SetInCombatWith(james);
                                james->SetInCombatWith(calissa);
                            }
                        break;
                    }
                    case EVENT_NIGHTMARE_OVER:
                    {
                        const Position pos = {-79.44965f, -819.8351f, 39.89838f, 0.01745329f};
                        instance->SummonCreature(49541, pos);

                        for (int i = 0; i < 3; ++i)
                            HandleGameObject(DoorGUID[i], true);

                        if (Creature* icecle_caster = instance->GetCreature(uiIcicleCasterGUID))
                        {
                            sCreatureTextMgr->SendChat(icecle_caster, VNE_EMOTE_NIGHTMARE_OVER, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
                            icecle_caster->CastSpell((Unit*)NULL, 92587, false);
                            icecle_caster->CastSpell((Unit*)NULL, 92609, false);
                            icecle_caster->DespawnOrUnsummon(2000);
                        }

                        Map::PlayerList const &PlayerList = instance->GetPlayers();

                        if (PlayerList.isEmpty())
                        {
                            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                            return;
                        }

                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->GetSource())
                            {
                                player->SetPhaseMask(1, true);
                                player->RemoveAura(92113);
                                player->RemoveAurasByType(SPELL_AURA_SCREEN_EFFECT);
                            }
                        break;
                    }
                    case EVENT_NIGHTMARE_CHECK_VALID_PLAYER:
                    {
                        Map::PlayerList const &PlayerList = instance->GetPlayers();

                        if (PlayerList.isEmpty())
                        {
                            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                            events.Reset();
                            return;
                        }

                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->GetSource())
                                if (player->IsAlive())
                                {
                                    events.ScheduleEvent(EVENT_NIGHTMARE_CHECK_VALID_PLAYER, 3000);
                                    return;
                                }

                        events.Reset();
                        SetBossState(DATA_VANESSA_VANCLEEF, DONE);
                        break;
                    }
                }
            }
        }

        void OnCreatureCreate(Creature *creature)
        {
            switch (creature->GetEntry())
            {
                case BOSS_GLUBTOK:
                    uiGlubtokGUID = creature->GetGUID();
                    break;
                case BOSS_HELIX:
                    helixGUID = creature->GetGUID();
                    break;
                case BOSS_FOE_REAPER_5000:
                    uiFoeReaper5000GUID = creature->GetGUID();
                    break;
                case BOSS_CAPTAIN_COOKIE:
                    uiCookieGUID = creature->GetGUID();
                    break;
                case 49454:
                    uiTrapGUID = creature->GetGUID();
                    creature->setActive(true);
                    break;
                case 51624:
                    uiAnchorGUID = creature->GetGUID();
                    creature->setActive(true);
                    break;
                case 49493:
                case 49494:
                case 49495:
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    creature->SetReactState(REACT_PASSIVE);
                    break;
                case 49534:
                    uiEmmeGUID = creature->GetGUID();
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    creature->SetReactState(REACT_PASSIVE);
                    break;
                case 49535:
                    uiErikGUID = creature->GetGUID();
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    creature->SetReactState(REACT_PASSIVE);
                    break;
                case 49536:
                    uiCalissaGUID = creature->GetGUID();
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    creature->SetReactState(REACT_PASSIVE);
                    break;
                case 49564:
                    creature->SetCanFly(true);
                    uiNoteFromVanessaGUID = creature->GetGUID();
                    break;
                case 48266:
                    CannonGUID.push_back(creature->GetGUID());
                    break;
                case 48505:
                    creature->setPowerType(POWER_ENERGY);
                    creature->SetPower(POWER_ENERGY, 100);
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case 208002:
                    go->setActive(true);
                    break;
                case 13965:
                case 17153:
                case 16400:
                    AddDoor(go, true);
                    break;
                case 17154:
                {
                    if (go->GetPhaseMask() == 1)
                        AddDoor(go, true);
                    else
                        DoorGUID[0] = go->GetGUID();
                    break;
                }
                case 16399:
                {
                    if (go->GetPhaseMask() == 1)
                        AddDoor(go, true);
                    else
                        DoorGUID[1] = go->GetGUID();
                    break;
                }
                case 16397:
                {
                    if (go->GetPhaseMask() == 1)
                        AddDoor(go, true);
                    else
                        DoorGUID[2] = go->GetGUID();
                    break;
                }
            }
        }

        void OnGameObjectRemove(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case 13965:
                case 17153:
                case 16400:
                    AddDoor(go, false);
                    break;
                case 17154:
                    {
                        if (go->GetPhaseMask() == 1)
                            AddDoor(go, false);
                    }
                    break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case DATA_ADMIRA_RIPSNARL:
                    if (Creature* cookie = instance->GetCreature(uiCookieGUID))
                        if (cookie->IsAIEnabled)
                            cookie->AI()->SetData(DATA_ADMIRA_RIPSNARL, state);
                    break;
                case DATA_CAPTAIN_COOKIE:
                    if (state == DONE
                        && Encounter[DATA_VANESSA_EVENT_GLUBTOK] == NOT_STARTED
                        && instance->GetDifficulty() == HEROIC_DIFFICULTY)
                    {
                        if (Creature* note = instance->GetCreature(uiNoteFromVanessaGUID))
                            if (note->IsAIEnabled)
                                note->AI()->Reset();
                    }
                    break;
            }

            if (type <= DATA_VANESSA_VANCLEEF && state == IN_PROGRESS)
                for (std::list<uint64>::const_iterator itr = CannonGUID.begin(); itr != CannonGUID.end(); ++itr)
                    if (Creature* cannon = instance->GetCreature(*itr))
                        if (Vehicle* vehicle = cannon->GetVehicleKit())
                            if (Unit* passenger = vehicle->GetPassenger(0))
                                if (passenger->GetTypeId() == TYPEID_PLAYER)
                                {
                                    passenger->ExitVehicle();
                                    cannon->Kill(cannon);
                                }

            return true;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_NOTE_USED:
                {
                    if (data == DONE && Encounter[DATA_VANESSA_EVENT_GLUBTOK] == NOT_STARTED)
                    {
                        events.ScheduleEvent(EVENT_START_VANESSA_INTRO, 3000);
                        events.ScheduleEvent(EVENT_NIGHTMARE_CHECK_VALID_PLAYER, 3000);
                        Encounter[DATA_VANESSA_EVENT_GLUBTOK] = IN_PROGRESS;

                        for (int i = 0; i < 3; ++i)
                            HandleGameObject(DoorGUID[i], false);
                    }
                    break;
                }
                case DATA_VALVE_ACTIVATED:
                {
                    ++uiActiveValveCount;

                    if (uiActiveValveCount == 4)
                    {
                        uiActiveValveCount = 0;
                        events.ScheduleEvent(EVENT_MAGMA_TRAP_THROW, 1000);

                        if (Creature* trap = instance->GetCreature(uiTrapGUID))
                        {
                            std::list<Creature*> lValves;
                            trap->GetCreatureListWithEntryInGrid(lValves, 49457, 30.0f);

                            if (!lValves.empty())
                                for (std::list<Creature*>::const_iterator itr = lValves.begin(); itr != lValves.end(); ++itr)
                                    if (*itr)
                                        (*itr)->CastSpell(*itr, 92401, false);
                        }
                    }
                    break;
                }
                case DATA_VANESSA_EVENT_GLUBTOK:
                {
                    if (data == DONE)
                    {
                        events.Reset();
                        events.ScheduleEvent(EVENT_START_HELIX_NIGHTMARE, 500);
                    }
                    break;
                }
                case DATA_VANESSA_EVENT_HELIX_GEARBREAKER:
                {
                    if (data == DONE)
                    {
                        events.Reset();
                        events.ScheduleEvent(EVENT_START_REAPER_NIGHTMARE, 500);
                    }
                    break;
                }
                case DATA_VANESSA_EVENT_FOE_REAPER_5000:
                {
                    if (data == DONE)
                    {
                        events.Reset();
                        events.ScheduleEvent(EVENT_START_RIPSNARL_NIGHTMARE, 500);
                    }
                    break;
                }
                case DATA_EMME_SAVED:
                {
                    if (data == DONE)
                        events.ScheduleEvent(EVENT_EMOTE_SAVE_ERIK, 3000);

                    if (data == FAIL)
                        EventFail();
                    break;
                }
                case DATA_ERIK_SAVED:
                {
                    if (data == DONE)
                        events.ScheduleEvent(EVENT_EMOTE_SAVE_CALISSA, 4000);

                    if (data == FAIL)
                        EventFail();
                    break;
                }
                case DATA_VANESSA_EVENT_ADMIRA_RIPSNARL:
                {
                    events.Reset();

                    if (data == DONE)
                    {
                        events.ScheduleEvent(EVENT_NIGHTMARE_OVER, 3000);
                        events.CancelEvent(EVENT_NIGHTMARE_CHECK_VALID_PLAYER);
                    }

                    if (data == FAIL)
                        EventFail();
                    break;
                }
                case DATA_ENGINEER_RUN:
                {
                    if (!reaperEngineer)
                    {
                        reaperEngineer = true;

                        if (Creature* engineer = instance->SummonCreature(48439, EngineerWP[0]))
                        {
                            engineer->RemoveAllAuras();

                            if (engineer->IsAIEnabled)
                                engineer->AI()->DoAction(ACTION_START_RUN);
                        }
                    }
                    break;
                }
            }

            if (type < MAX_ENCOUNTER)
                Encounter[type] = data;

            SaveToDB();
        }

        void EventFail()
        {
            SetBossState(DATA_VANESSA_VANCLEEF, DONE);
            Map::PlayerList const &PlayerList = instance->GetPlayers();

            if (PlayerList.isEmpty())
                return;

            for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                if (Player* player = itr->GetSource())
                {
                    player->SetPhaseMask(1, false);
                    player->RemoveAura(92113);
                    player->RemoveAurasByType(SPELL_AURA_SCREEN_EFFECT);
                }
        }

        uint32 GetData(uint32 type)
        {
            if (type < MAX_ENCOUNTER)
                return Encounter[type];

            return 0;
        }

        uint64 GetData64(uint32 data)
        {
            switch (data)
            {
                case DATA_GLUBTOK:           return uiGlubtokGUID;
                case DATA_HELIX_GEARBREAKER: return helixGUID;
                case DATA_FOE_REAPER_5000:   return uiFoeReaper5000GUID;
                case DATA_CAPTAIN_COOKIE:    return uiCookieGUID;
            }

            return 0;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "D M " << Encounter[0] << " " << Encounter[1]  << " " << Encounter[2]<< " " << Encounter[3] << " " << GetBossSaveData();
            OUT_SAVE_INST_DATA_COMPLETE;
            return saveStream.str();
        }

        void Load(const char* in)
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            char dataHead1, dataHead2;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2;

            if (dataHead1 == 'D' && dataHead2 == 'M')
            {
                loadStream >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3];

                for (uint8 i = 0; i < MAX_BOSS_ENCOUNTER; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;

                    SetBossState(i, EncounterState(tmpState));
                }

            } else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_deadmines_InstanceMapScript(map);
    }
};

class npc_vanessas_steam_valve : public CreatureScript
{
public:
    npc_vanessas_steam_valve() : CreatureScript("npc_vanessas_steam_valve") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_vanessas_steam_valveAI (creature);
    }

    struct npc_vanessas_steam_valveAI : public ScriptedAI
    {
        npc_vanessas_steam_valveAI(Creature* creature) : ScriptedAI(creature)
        {
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void OnSpellClick(Unit * /*player*/, bool& result)
        {
            if (!result)
                return;

            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            InstanceScript* instance = me->GetInstanceScript();

            if (instance)
                instance->SetData(DATA_VALVE_ACTIVATED, DONE);
        }
    };
};

class npc_glubtok_icicle : public CreatureScript
{
public:
    npc_glubtok_icicle() : CreatureScript("npc_glubtok_icicle") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_glubtok_icicleAI (creature);
    }

    struct npc_glubtok_icicleAI : public ScriptedAI
    {
        npc_glubtok_icicleAI(Creature* creature) : ScriptedAI(creature)
        {
            events.ScheduleEvent(EVENT_ICICLE, 4000);
            me->SetReactState(REACT_PASSIVE);
            me->SetInCombatWithZone();
        }

        EventMap events;

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_ICICLE)
            {
                me->CastSpell(me, 92201, false);
                me->CastSpell(me, 92202, false);
            }
        }
    };
};

class npc_nightmare_glubtok : public CreatureScript
{
public:
    npc_nightmare_glubtok() : CreatureScript("npc_nightmare_glubtok") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_nightmare_glubtokAI (creature);
    }

    struct npc_nightmare_glubtokAI : public ScriptedAI
    {
        npc_nightmare_glubtokAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void JustDied(Unit* /*killer*/)
        {
            InstanceScript* instance = me->GetInstanceScript();

            if (instance)
                instance->SetData(DATA_VANESSA_EVENT_GLUBTOK, DONE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_SPIRIT_STRIKE, urand(3000, 7000));
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SPIRIT_STRIKE)
            {
                me->CastSpell(me->GetVictim(), 59304, false);
                events.ScheduleEvent(EVENT_SPIRIT_STRIKE, urand(3000, 7000));
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_nightmare_helix : public CreatureScript
{
public:
    npc_nightmare_helix() : CreatureScript("npc_nightmare_helix") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_nightmare_helixAI (creature);
    }

    struct npc_nightmare_helixAI : public ScriptedAI
    {
        npc_nightmare_helixAI(Creature* creature) : ScriptedAI(creature)
        {
            uiIntroTimer = 15000;
            Intro = true;
        }

        EventMap events;
        uint32 uiIntroTimer;
        bool Intro;

        void JustDied(Unit* /*killer*/)
        {
            InstanceScript* instance = me->GetInstanceScript();

            if (instance)
                instance->SetData(DATA_VANESSA_EVENT_HELIX_GEARBREAKER, DONE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_SPIRIT_STRIKE, urand(3000, 7000));
            events.ScheduleEvent(EVENT_HELIX_RIDE, urand(5000, 10000));
        }

        void OnExitVehicle(Unit* /*vehicle*/, uint32 /*seatId*/)
        {
            events.ScheduleEvent(EVENT_HELIX_RIDE, urand(2000, 7000));
        }

        void UpdateAI(uint32 const diff)
        {
            if (Intro)
            {
                if (uiIntroTimer <= diff)
                {
                    Intro = false;
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetInCombatWithZone();
                    Talk(VNE_EMOTE_HELIX_NIGHTMARE_1);
                    events.ScheduleEvent(EVENT_VNE_HELIX_EMOTE_SPIDER, 2000);
                }
                else
                    uiIntroTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SPIRIT_STRIKE:
                        {
                            Unit* target = me->GetVehicleBase();

                            if (!target)
                                target = me->GetVictim();

                            me->CastSpell(target, 59304, false);
                            events.ScheduleEvent(EVENT_SPIRIT_STRIKE, urand(3000, 7000));
                        }
                        break;
                    case EVENT_VNE_HELIX_EMOTE_SPIDER:
                        Talk(VNE_EMOTE_HELIX_NIGHTMARE_2);
                        events.ScheduleEvent(EVENT_ACTIVATE_SPIDERS, 1000);
                        break;
                    case EVENT_ACTIVATE_SPIDERS:
                        {
                            std::list<Creature*> lSpiders;
                            me->GetCreatureListWithEntryInGrid(lSpiders, 49493, 50.0f);

                            if (!lSpiders.empty())
                            {
                                int count = 0;
                                std::list<Creature*>::const_iterator spider = lSpiders.begin();

                                while (count < 3 && spider != lSpiders.end())
                                {
                                    if (*spider && (*spider)->IsAlive())
                                        if (!(*spider)->IsInCombat())
                                        {
                                            (*spider)->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                                            (*spider)->SetReactState(REACT_AGGRESSIVE);
                                            (*spider)->SetInCombatWithZone();
                                            ++count;
                                        }

                                    ++spider;
                                }
                            }

                            if (Creature* spider = me->FindNearestCreature(49495, 50.0f))
                            {
                                spider->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                                spider->SetReactState(REACT_AGGRESSIVE);
                                spider->SetInCombatWithZone();
                            }

                            events.ScheduleEvent(EVENT_ACTIVATE_SPIDERS, urand(5000, 25000));
                        }
                        break;
                    case EVENT_HELIX_RIDE:
                        me->CastSpell(me, SPELL_HELIX_RIDE, false);
                        me->CastSpell(me, SPELL_RIDE_FACE_TARGETTING, false);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_vanessa_lightning_platter : public CreatureScript
{
public:
    npc_vanessa_lightning_platter() : CreatureScript("npc_vanessa_lightning_platter") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_vanessa_lightning_platterAI(creature);
    }

    struct npc_vanessa_lightning_platterAI : public ScriptedAI
    {
        npc_vanessa_lightning_platterAI(Creature* creature) : ScriptedAI(creature)
        {
            events.ScheduleEvent(EVENT_INSTALL_WALL_LEVEL_ONE, 100);
            me->CastSpell(me, 95519, false);
            float angle = frand(-M_PI, M_PI);
            float x, y, z;
            me->GetPosition(x, y, z);
            me->UpdatePosition(x, y, z, angle, true);
        }

        EventMap events;

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INSTALL_WALL_LEVEL_ONE:
                        {
                            Position pos;
                            me->GetPosition(&pos);

                            for (int i = 0; i < 8; ++i)
                                if (Creature* wall = me->SummonCreature(49521, pos))
                                    wall->EnterVehicle(me, i);

                            events.ScheduleEvent(urand(EVENT_CHANGE_ANGLE_L, EVENT_CHANGE_ANGLE_R), 1000);
                        }
                        break;
                    case EVENT_CHANGE_ANGLE_L:
                        {
                            float x, y, z, o;
                            me->GetPosition(x, y, z, o);

                            if (o < -M_PI)
                                o += M_PI * 2;

                            me->NearTeleportTo(x, y, z, o - 0.025f, true);
                            events.ScheduleEvent(EVENT_CHANGE_ANGLE_L, 50);
                        }
                        break;
                    case EVENT_CHANGE_ANGLE_R:
                        {
                            float x, y, z, o;
                            me->GetPosition(x, y, z, o);

                            if (o > M_PI)
                                o -= M_PI * 2;

                            me->NearTeleportTo(x, y, z, o + 0.025f, true);
                            events.ScheduleEvent(EVENT_CHANGE_ANGLE_R, 50);
                        }
                        break;
                }
            }
        }
    };
};

class npc_nightmare_reaper : public CreatureScript
{
public:
    npc_nightmare_reaper() : CreatureScript("npc_nightmare_reaper") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_nightmare_reaperAI (creature);
    }

    struct npc_nightmare_reaperAI : public ScriptedAI
    {
        npc_nightmare_reaperAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void JustDied(Unit* /*killer*/)
        {
            InstanceScript* instance = me->GetInstanceScript();

            if (instance)
                instance->SetData(DATA_VANESSA_EVENT_FOE_REAPER_5000, DONE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_SPIRIT_STRIKE, urand(3000, 7000));
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SPIRIT_STRIKE)
            {
                me->CastSpell(me->GetVictim(), 59304, false);
                events.ScheduleEvent(EVENT_SPIRIT_STRIKE, urand(3000, 7000));
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_emme_harrington : public CreatureScript
{
public:
    npc_emme_harrington() : CreatureScript("npc_emme_harrington") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_emme_harringtonAI (creature);
    }

    struct npc_emme_harringtonAI : public ScriptedAI
    {
        npc_emme_harringtonAI(Creature* creature) : ScriptedAI(creature)
        {
            Died = false;
            uiDeadWorgenCount = 0;
            me->SetReactState(REACT_PASSIVE);
            events.ScheduleEvent(EVENT_TAUNT, urand(7000, 14000));
        }

        bool Died;
        EventMap events;
        uint8 uiDeadWorgenCount;

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (!Died)
                {
                    InstanceScript* instance = me->GetInstanceScript();

                    if (instance)
                        instance->SetData(DATA_EMME_SAVED, FAIL);

                    Died = true;
                    events.Reset();
                    me->SetPhaseMask(33, true);
                    me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->CastSpell((Unit*)NULL, 92603, false);
                    me->DespawnOrUnsummon(2000);
                }
            }
        }

        void SummonedCreatureDies(Creature* summoned, Unit* /*killer*/)
        {
            if (summoned->GetEntry() == 49532)
            {
                ++uiDeadWorgenCount;

                if (uiDeadWorgenCount == 3)
                {
                    me->DespawnOrUnsummon(2000);
                    me->CastSpell((Unit*)NULL, 92604, false);
                    InstanceScript* instance = me->GetInstanceScript();

                    if (instance)
                        instance->SetData(DATA_EMME_SAVED, DONE);
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_TAUNT)
            {
                me->CastSpell((Unit*)NULL, 92308, false);
                events.ScheduleEvent(EVENT_TAUNT, urand(7000, 14000));
            }
        }
    };
};

class npc_erik_harrington : public CreatureScript
{
public:
    npc_erik_harrington() : CreatureScript("npc_erik_harrington") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_erik_harringtonAI (creature);
    }

    struct npc_erik_harringtonAI : public ScriptedAI
    {
        npc_erik_harringtonAI(Creature* creature) : ScriptedAI(creature)
        {
            Died = false;
            uiDeadWorgenCount = 0;
            me->SetReactState(REACT_PASSIVE);
            events.ScheduleEvent(EVENT_TAUNT, urand(7000, 14000));
        }

        bool Died;
        EventMap events;
        uint8 uiDeadWorgenCount;

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (!Died)
                {
                    InstanceScript* instance = me->GetInstanceScript();

                    if (instance)
                        instance->SetData(DATA_ERIK_SAVED, FAIL);

                    Died = true;
                    events.Reset();
                    me->SetPhaseMask(33, true);
                    me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->CastSpell((Unit*)NULL, 92603, false);
                    me->DespawnOrUnsummon(2000);
                }
            }
        }

        void SummonedCreatureDies(Creature* summoned, Unit* /*killer*/)
        {
            if (summoned->GetEntry() == 49532)
            {
                ++uiDeadWorgenCount;

                if (uiDeadWorgenCount == 3)
                {
                    me->DespawnOrUnsummon(2000);
                    me->CastSpell((Unit*)NULL, 92604, false);
                    InstanceScript* instance = me->GetInstanceScript();

                    if (instance)
                        instance->SetData(DATA_ERIK_SAVED, DONE);
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_TAUNT)
            {
                me->CastSpell((Unit*)NULL, 92308, false);
                events.ScheduleEvent(EVENT_TAUNT, urand(7000, 14000));
            }
        }
    };
};

class npc_calissa_harrington : public CreatureScript
{
public:
    npc_calissa_harrington() : CreatureScript("npc_calissa_harrington") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_calissa_harringtonAI (creature);
    }

    struct npc_calissa_harringtonAI : public ScriptedAI
    {
        npc_calissa_harringtonAI(Creature* creature) : ScriptedAI(creature)
        {
            if (Creature* james = me->SummonCreature(49539, RipsnarlEvent[2]))
            {
                james->SetReactState(REACT_PASSIVE);
                james->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                james->SetInCombatWith(me);
                james->AddThreat(me, 0.0f);
                james->SetUInt32Value(UNIT_NPC_EMOTESTATE, 468);
                me->EnterVehicle(james);
            }

            Died = false;
        }

        bool Died;
        EventMap events;

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_RIPSNARL_SAY, 10000);
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (!Died)
                {
                    InstanceScript* instance = me->GetInstanceScript();

                    if (instance)
                        instance->SetData(DATA_VANESSA_EVENT_ADMIRA_RIPSNARL, FAIL);

                    Died = true;
                    events.Reset();
                    me->ExitVehicle();
                    me->SetPhaseMask(33, true);
                    me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->CastSpell((Unit*)NULL, 92603, false);
                    me->DespawnOrUnsummon(2000);
                    Talk(VNE_CALISSA_DYING);
                }
            }
        }

        void SummonedCreatureDies(Creature* summoned, Unit* /*killer*/)
        {
            if (summoned->GetEntry() == 49539)
            {
                me->RemoveAura(92608);
                me->DespawnOrUnsummon(2000);
                InstanceScript* instance = me->GetInstanceScript();

                if (instance)
                    instance->SetData(DATA_VANESSA_EVENT_ADMIRA_RIPSNARL, DONE);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RIPSNARL_SAY:
                        {
                            if (Creature* james = me->GetVehicleCreatureBase())
                                if (james->IsAIEnabled)
                                    james->AI()->Talk(VNE_RIPSNARL_SAY_SORRY);

                            events.ScheduleEvent(EVENT_CLARISSA_SAY, 2000);
                        }
                        break;
                    case EVENT_CLARISSA_SAY:
                        Talk(VNE_CALISSA_SAY_I_LOVE);
                        break;
                }
            }
        }
    };
};

class spell_ride_magma_vehicle : public SpellScriptLoader
{
    public:
        spell_ride_magma_vehicle() : SpellScriptLoader("spell_ride_magma_vehicle") { }

        class spell_ride_magma_vehicle_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ride_magma_vehicle_SpellScript)

            void TriggerSpell(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!(caster && target))
                    return;

                target->CastSpell(caster, GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_ride_magma_vehicle_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_ride_magma_vehicle_SpellScript();
        }
};

class spell_icicle_targeting : public SpellScriptLoader
{
    public:
        spell_icicle_targeting() : SpellScriptLoader("spell_icicle_targeting") { }

        class spell_icicle_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_icicle_targeting_SpellScript)

            void TriggerSpell(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!(caster && target))
                    return;

                caster->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, false);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_icicle_targeting_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_icicle_targeting_SpellScript();
        }
};

struct CannonCoord
{
    const Position CannonPos;
    const Position TargetPos;
};

const CannonCoord Coords[8]=
{
    {{-122.9f, -388.81f, 59.076f, 0.0f},{-90.43f, -375.74f, 58.016f, 0.0f}},
    {{-82.31f, -775.50f, 26.893f, 0.0f},{-88.42f, -724.72f, 8.6750f, 0.0f}},
    {{-89.25f, -782.52f, 17.256f, 0.0f},{-100.8f, -703.77f, 9.2940f, 0.0f}},
    {{-72.10f, -786.89f, 39.553f, 0.0f},{-72.55f, -731.22f, 8.5869f, 0.0f}},
    {{-46.81f, -783.19f, 18.493f, 0.0f},{-30.63f, -727.73f, 8.5210f, 0.0f}},
    {{-58.64f, -787.13f, 39.350f, 0.0f},{-49.32f, -730.05f, 9.3204f, 0.0f}},
    {{-40.00f, -793.30f, 39.475f, 0.0f},{-12.05f, -740.25f, 9.1094f, 0.0f}},
    {{-30.26f, -793.06f, 19.237f, 0.0f},{0.5121f, -768.22f, 9.8013f, 0.0f}},
};

class npc_defias_cannon : public CreatureScript
{
public:
    npc_defias_cannon() : CreatureScript("npc_defias_cannon") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_defias_cannonAI (creature);
    }

    struct npc_defias_cannonAI : public ScriptedAI
    {
        npc_defias_cannonAI(Creature* creature) : ScriptedAI(creature)
        {
            events.ScheduleEvent(EVENT_INSTALL_PASSENGER, 100);
            uiTargetGUID = 0;
            me->AddUnitState(UNIT_STATE_ROOT);
            me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
        }

        EventMap events;
        uint64 uiTargetGUID;

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void ResetVehicle()
        {
            me->CastStop();
            events.Reset();

            if (Creature* target = Unit::GetCreature(*me, uiTargetGUID))
                target->DespawnOrUnsummon();

            uiTargetGUID = 0;
        }

        void SummonedCreatureDies(Creature* summoned, Unit* /*killer*/)
        {
            summoned->ExitVehicle();
            ResetVehicle();
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                me->SetControlled(true, UNIT_STATE_ROOT);

                if (who->ToCreature())
                    events.ScheduleEvent(EVENT_SHOOT, 1000);
                else
                {
                    InstanceScript* instance = me->GetInstanceScript();

                    if (instance)
                        if (instance->IsEncounterInProgress())
                        {
                            who->ExitVehicle();
                            me->Kill(me);
                        }

                    events.Reset();
                    me->CastStop();
                }
            }
            else
                if (who->ToCreature())
                    ResetVehicle();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (uint32 event_id = events.ExecuteEvent())
            {
                switch (event_id)
                {
                    case EVENT_INSTALL_PASSENGER:
                        {
                            int CannonId = -1;
                            float dist = 100500.0f;

                            for (int i = 0; i < 8; ++i)
                            {
                                float _dist = me->GetExactDist(&Coords[i].CannonPos);

                                if (dist > _dist && _dist < 10.0f)
                                {
                                    dist = _dist;
                                    CannonId = i;
                                }
                            }

                            if (CannonId >= 0)
                            {
                                uint32 DriverEntry = (CannonId == 0 ? 48230 : 48522);
                                float x, y;
                                me->GetNearPoint2D(x, y, 2.5f, me->GetOrientation() + M_PI);

                                if (Creature* driver = me->SummonCreature(DriverEntry, x, y, me->GetPositionZ(), me->GetOrientation()))
                                    driver->CastSpell(me, 46598, true);

                                if (Creature* target = me->SummonCreature(45979, Coords[CannonId].TargetPos))
                                {
                                    me->SetFacingTo(me->GetAngle(target));
                                    target->SetCanFly(true);
                                    uiTargetGUID = target->GetGUID();
                                }
                            }
                        }
                        break;
                    case EVENT_SHOOT:
                        {
                            if (Creature* target = Unit::GetCreature(*me, uiTargetGUID))
                                me->CastSpell(target, 89697, false, NULL, NULL, uiTargetGUID);

                            events.ScheduleEvent(EVENT_SHOOT, urand(2000, 5000));
                        }
                        break;
                }
            }
        }
    };
};

class spell_deadmines_cannonball_trigger : public SpellScriptLoader
{
    public:
        spell_deadmines_cannonball_trigger() : SpellScriptLoader("spell_deadmines_cannonball_trigger") { }

        class spell_deadmines_cannonball_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deadmines_cannonball_trigger_SpellScript)

            void Function(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                if (Unit* caster = GetOriginalCaster())
                {
                    float x, y, z;
                    GetExplTargetDest() ? GetExplTargetDest()->GetPosition(x, y, z) : caster->GetPosition(x, y, z);
                    caster->CastSpell(x, y, z, GetSpellInfo()->Effects[effIndex].TriggerSpell, true, NULL, NULL, GetCaster() ? GetCaster()->GetGUID() : 0);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_deadmines_cannonball_trigger_SpellScript::Function, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_deadmines_cannonball_trigger_SpellScript();
        }
};

class npc_mining_powder : public CreatureScript
{
public:
    npc_mining_powder() : CreatureScript("npc_mining_powder") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mining_powderAI (creature);
    }

    struct npc_mining_powderAI : public ScriptedAI
    {
        npc_mining_powderAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        bool Boom;

        void Reset()
        {
            Boom = false;
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (!Boom)
                {
                    Boom = true;
                    me->SetDisplayId(11686);
                    me->CastSpell((Unit*)NULL, 90096, false);
                    me->CastSpell((Unit*)NULL, 89769, false);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->DespawnOrUnsummon(1000);
                }
            }
        }
    };
};

enum DeadminesTeleporter
{
    MINE_ENTER                                   = 200,
    MAST_ROOM                                    = 201,
    GOBLIN_FOUNDRY                               = 202,
    SHIP                                         = 203,
};

class go_deadmines_teleporter : public GameObjectScript
{
    public:
        go_deadmines_teleporter() : GameObjectScript("go_deadmines_teleporter") { }

        bool OnGossipSelect(Player* player, GameObject* /*gameObject*/, uint32 /*sender*/, uint32 action)
        {
            player->CLOSE_GOSSIP_MENU();

            switch (action)
            {
                case MINE_ENTER:
                    player->TeleportTo(36, -64.15278f, -385.9896f, 57.1919f, 1.850049f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
                case MAST_ROOM:
                    player->TeleportTo(36, -305.3212f, -491.2917f, 53.2320f, 0.488691f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
                case GOBLIN_FOUNDRY:
                    player->TeleportTo(36, -201.0955f, -606.0504f, 23.3022f, 2.740162f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
                case SHIP:
                    player->TeleportTo(36, -129.9149f, -788.8976f, 21.3409f, 0.366517f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
            }

            return true;
        }

        bool OnGossipHello(Player* player, GameObject* gameObject)
        {
            if (player->IsInCombat())
                return true;

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Mine entrance", GOSSIP_SENDER_MAIN, MINE_ENTER);

            if (InstanceScript* instance = gameObject->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_GLUBTOK) == DONE && instance->GetBossState(DATA_HELIX_GEARBREAKER) == DONE)
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Planks and timber", GOSSIP_SENDER_MAIN, MAST_ROOM);

                    if (instance->GetBossState(DATA_FOE_REAPER_5000) == DONE)
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Parts and metal", GOSSIP_SENDER_MAIN, GOBLIN_FOUNDRY);

                        if (instance->GetBossState(DATA_ADMIRA_RIPSNARL) == DONE)
                            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Cannons and cannonballs", GOSSIP_SENDER_MAIN, SHIP);
                    }
                }
            }

            player->SEND_GOSSIP_MENU(17824, gameObject->GetGUID());
            return true;
        }
};

class go_defias_cannon : public GameObjectScript
{
public:
    go_defias_cannon() : GameObjectScript("go_defias_cannon") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (InstanceScript* instance = go->GetInstanceScript())
            for (int data = DATA_GLUBTOK; data <= DATA_FOE_REAPER_5000; ++data)
                if (instance->GetBossState(data) != DONE)
                    return true;

        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
        go->SetGoState(GO_STATE_ACTIVE);
        go->PlayDirectSound(1400);

        if (Creature* boom = go->SummonCreature(45979, -106.967f, -660.372f, 7.505134f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 2000))
        {
            if (boom->IsAIEnabled)
                boom->AI()->Talk(4);

            boom->CastSpell((Unit*)NULL, 92074, false);
        }

        if (GameObject* door = player->FindNearestGameObject(16397, 50.0f))
        {
            door->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
            door->PlayDirectSound(3079);
        }

        return true;
    }
};

class npc_note_from_vanessa : public CreatureScript
{
public:
    npc_note_from_vanessa() : CreatureScript("npc_note_from_vanessa") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_note_from_vanessaAI (creature);
    }

    bool OnGossipHello(Player* /*player*/, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
            if (instance->GetBossState(DATA_CAPTAIN_COOKIE) == DONE && instance->GetData(DATA_VANESSA_EVENT_GLUBTOK) == NOT_STARTED)
            {
                instance->SetData(DATA_NOTE_USED, DONE);
                creature->SetVisible(false);
                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            }

        return true;
    }

    struct npc_note_from_vanessaAI : public ScriptedAI
    {
        npc_note_from_vanessaAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            if (InstanceScript* instance = me->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_CAPTAIN_COOKIE) == DONE && instance->GetData(DATA_VANESSA_EVENT_GLUBTOK) == NOT_STARTED)
                {
                    me->SetVisible(true);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
                else
                {
                    me->SetVisible(false);
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
            }
        }
    };
};

class spell_eject_all_passengers : public SpellScriptLoader
{
    public:
        spell_eject_all_passengers() : SpellScriptLoader("spell_eject_all_passengers") { }

        class spell_eject_all_passengers_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eject_all_passengers_SpellScript)

            bool Load()
            {
                return GetCaster()->IsVehicle();
            }

            void HandleDummy(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetCaster()->GetVehicleKit()->RemoveAllPassengers();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_eject_all_passengers_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_eject_all_passengers_SpellScript();
        }
};

class spell_deadmines_spark : public SpellScriptLoader
{
    public:
        spell_deadmines_spark() : SpellScriptLoader("spell_deadmines_spark") { }

        class spell_deadmines_sparkSpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deadmines_sparkSpellScript)

            void CheckTargetDist(std::list<WorldObject*> &targetList)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                for (auto itr = targetList.begin() ; itr != targetList.end();)
                {
                    if (caster->GetExactDist2d((*itr)) > 2.0f)
                        itr = targetList.erase(itr);
                    else
                        ++itr;
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_deadmines_sparkSpellScript::CheckTargetDist, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deadmines_sparkSpellScript();
        }
};

class go_deadmines_ball_and_chain : public GameObjectScript
{
public:
    go_deadmines_ball_and_chain() : GameObjectScript("go_deadmines_ball_and_chain") { }

    bool OnGossipHello(Player* /*player*/, GameObject* go)
    {
        for (int i = 0; i < 4; ++i)
            if (Creature* monkey = go->FindNearestCreature(MonkeyEntry[i], 3.0f))
                if (monkey->IsAIEnabled)
                    monkey->AI()->DoAction(ACTION_FREE);

        return true;
    }
};

class npc_mining_monkey : public CreatureScript
{
public:
    npc_mining_monkey() : CreatureScript("npc_mining_monkey") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mining_monkeyAI (creature);
    }

    struct npc_mining_monkeyAI : public ScriptedAI
    {
        npc_mining_monkeyAI(Creature* creature) : ScriptedAI(creature)
        {
            Free = false;
            events.ScheduleEvent(EVENT_TROWN, 500);
        }

        bool Free;
        EventMap events;

        void Reset()
        {
            if (!Free)
            {
                me->AddUnitState(UNIT_STATE_ROOT);
                me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 233);
            }
            else
            {
                me->ClearUnitState(UNIT_STATE_ROOT);
                me->RemoveUnitMovementFlag(MOVEMENTFLAG_ROOT);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                me->GetMotionMaster()->MoveRandom(10.0f);
                me->setFaction(1665);
            }
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_FREE)
            {
                Free = true;
                me->DeleteThreatList();
                me->CombatStop();
                Reset();
                me->DespawnOrUnsummon(25000);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_TROWN)
            {
                me->CastSpell(me->GetVictim(), 91038, false);
                events.ScheduleEvent(EVENT_TROWN, 1500);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_deadmines_goblin_engineer : public CreatureScript
{
public:
    npc_deadmines_goblin_engineer() : CreatureScript("npc_deadmines_goblin_engineer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_deadmines_goblin_engineerAI(creature);
    }

    struct npc_deadmines_goblin_engineerAI : public SmoothEscortAI
    {
        npc_deadmines_goblin_engineerAI(Creature* creature) : SmoothEscortAI(creature), lSummons(me)
        {
            lSummons.DespawnAll();
        }

        SummonList lSummons;

        void WaypointReached(uint32 point)
        {
            if (lSummons.empty())
                return;

            switch (point)
            {
                case 3:
                    {
                        SummonList::const_iterator itr = lSummons.begin();

                        if (Creature* craftsman = Unit::GetCreature(*me, *itr))
                            if (craftsman->IsAIEnabled)
                                craftsman->AI()->Talk(0);
                    }
                    break;
                case 5:
                    Talk(0);
                    break;
                case 7:
                    {
                        SummonList::const_iterator itr = lSummons.begin();
                        ++itr;

                        if (Creature* craftsman = Unit::GetCreature(*me, *itr))
                            if (craftsman->IsAIEnabled)
                                craftsman->AI()->Talk(1);
                    }
                    break;
                case 11:
                    {
                        SummonList::const_iterator itr = lSummons.begin();

                        if (Creature* craftsman = Unit::GetCreature(*me, *itr))
                            if (craftsman->IsAIEnabled)
                                craftsman->AI()->Talk(2);
                    }
                    break;
                case 19:
                    lSummons.DespawnAll();
                    me->DespawnOrUnsummon();
                    break;
            }
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_START_RUN)
            {
                float x, y, z, angle = M_PI / 2;
                me->GetPosition(x, y, z);

                for (int i = 0; i < 2; ++i)
                    if (Creature* craftsman = me->SummonCreature(48280, x, y, z))
                    {
                        craftsman->RemoveAllAuras();
                        craftsman->GetMotionMaster()->MoveFollow(me, 0.5f, angle);
                        angle = angle * -1;
                        lSummons.Summon(craftsman);
                    }

                for (int i = 0; i < 20; ++i)
                    AddWaypoint(i, EngineerWP[i].GetPositionX(), EngineerWP[i].GetPositionY(), EngineerWP[i].GetPositionZ());

                Start(true);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            SmoothEscortAI::UpdateAI(diff);
        }
    };
};

class at_reaper_room_enter : public AreaTriggerScript
{
    public:
        at_reaper_room_enter() : AreaTriggerScript("at_reaper_room_enter") { }

    private:
        bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
                instance->SetData(DATA_ENGINEER_RUN, DONE);

            return false;
        }
};

void AddSC_instance_deadmines()
{
    new instance_deadmines();

    new npc_vanessas_steam_valve();
    new npc_glubtok_icicle();
    new npc_nightmare_glubtok();
    new npc_nightmare_helix();
    new npc_vanessa_lightning_platter();
    new npc_nightmare_reaper();
    new npc_emme_harrington();
    new npc_erik_harrington();
    new npc_calissa_harrington();
    new npc_defias_cannon();
    new npc_mining_powder();
    new npc_note_from_vanessa();
    new npc_mining_monkey();
    new npc_deadmines_goblin_engineer();

    new go_deadmines_teleporter();
    new go_defias_cannon();
    new go_deadmines_ball_and_chain();

    new at_reaper_room_enter();

    new spell_ride_magma_vehicle();
    new spell_icicle_targeting();
    new spell_deadmines_cannonball_trigger();
    new spell_eject_all_passengers();
    new spell_deadmines_spark();
}
