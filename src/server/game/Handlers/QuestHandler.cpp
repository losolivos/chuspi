/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "QuestDef.h"
#include "ObjectAccessor.h"
#include "Group.h"
#include "Battleground.h"
#include "BattlegroundAV.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"

void WorldSession::HandleQuestgiverStatusQueryOpcode(WorldPacket & recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<3, 1, 2, 6, 4, 7, 0, 5>(guid);
    recvData.ReadByteSeq<3, 0, 5, 2, 7, 6, 1, 4>(guid);

    uint32 questStatus = DIALOG_STATUS_NONE;
    uint32 defstatus = DIALOG_STATUS_NONE;

    Object* questgiver = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
    if (!questgiver)
    {
        TC_LOG_INFO("network", "Error in CMSG_QUESTGIVER_STATUS_QUERY, called for not found questgiver (Typeid: %u GUID: %u)", GuidHigh2TypeId(GUID_HIPART(guid)), GUID_LOPART(guid));
        return;
    }

    switch (questgiver->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY for npc, guid = %u", uint32(GUID_LOPART(guid)));
            Creature* cr_questgiver=questgiver->ToCreature();
            if (!cr_questgiver->IsHostileTo(_player))       // do not show quest status to enemies
            {
                questStatus = sScriptMgr->GetDialogStatus(_player, cr_questgiver);
                if (questStatus > 6)
                    questStatus = getDialogStatus(_player, cr_questgiver, defstatus);
            }
            break;
        }
        case TYPEID_GAMEOBJECT:
        {
            TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY for GameObject guid = %u", uint32(GUID_LOPART(guid)));
            GameObject* go_questgiver=(GameObject*)questgiver;
            questStatus = sScriptMgr->GetDialogStatus(_player, go_questgiver);
            if (questStatus > 6)
                questStatus = getDialogStatus(_player, go_questgiver, defstatus);
            break;
        }
        default:
            TC_LOG_ERROR("network", "QuestGiver called for unexpected type %u", questgiver->GetTypeId());
            break;
    }

    //inform client about status of quest
    _player->PlayerTalkClass->SendQuestGiverStatus(questStatus, guid);
}

void WorldSession::HandleQuestgiverHelloOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<6, 3, 4, 2, 5, 7, 0, 1>(guid);
    recvData.ReadByteSeq<0, 2, 6, 1, 4, 7, 5, 3>(guid);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_HELLO npc = %u", GUID_LOPART(guid));

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleQuestgiverHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.",
            GUID_LOPART(guid));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);
    // Stop the npc if moving
    creature->StopMoving();

    if (sScriptMgr->OnGossipHello(_player, creature))
        return;

    _player->PrepareGossipMenu(creature, creature->GetCreatureTemplate()->GossipMenuId, true);
    _player->SendPreparedGossip(creature);

    creature->AI()->sGossipHello(_player);
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 questId;
    uint8 unk1;

    recvData >> questId;

    recvData.ReadBitSeq<2, 3, 7, 4, 0, 1, 5>(guid);
    unk1 = recvData.ReadBit();
    recvData.ReadBitSeq<6>(guid);
    recvData.ReadByteSeq<6, 0, 3, 4, 7, 5, 2, 1>(guid);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST npc = %u, quest = %u, unk1 = %u", uint32(GUID_LOPART(guid)), questId, unk1);

    Object* object = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT|TYPEMASK_ITEM|TYPEMASK_PLAYER);

    // no or incorrect quest giver
    if (!object || (object->GetTypeId() != TYPEID_PLAYER && !object->hasQuest(questId)) ||
        (object->GetTypeId() == TYPEID_PLAYER && object != _player && !object->ToPlayer()->CanShareQuest(questId)))
    {
        _player->PlayerTalkClass->SendCloseGossip();
        _player->SaveToDB();
        _player->SetDivider(0);
        return;
    }

    if (object && object->GetTypeId() == TYPEID_PLAYER && object->ToPlayer()->GetQuestStatus(questId) == QUEST_STATUS_NONE)
        return;

    // some kind of WPE protection
    if (!_player->CanInteractWithQuestGiver(object))
        return;

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        // prevent cheating
        if (!GetPlayer()->CanTakeQuest(quest, true))
        {
            _player->PlayerTalkClass->SendCloseGossip();
            _player->SetDivider(0);
            return;
        }

        if (object && object->GetTypeId() == TYPEID_PLAYER && !quest->HasFlag(QUEST_FLAGS_SHARABLE))
            return;

        if (_player->GetDivider() != 0)
        {
            Player* player = ObjectAccessor::FindPlayer(_player->GetDivider());
            if (player)
            {
                player->SendPushToPartyResponse(_player, QUEST_PARTY_MSG_ACCEPT_QUEST);
                _player->SetDivider(0);
            }
        }

        if (_player->CanAddQuest(quest, true))
        {
            _player->AddQuestAndCheckCompletion(quest, object);

            if (quest->HasFlag(QUEST_FLAGS_PARTY_ACCEPT))
            {
                if (Group* group = _player->GetGroup())
                {
                    for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
                    {
                        Player* player = itr->GetSource();

                        if (!player || player == _player)     // not self
                            continue;

                        if (player->CanTakeQuest(quest, true))
                        {
                            player->SetDivider(_player->GetGUID());

                            //need confirmation that any gossip window will close
                            player->PlayerTalkClass->SendCloseGossip();

                            _player->SendQuestConfirmAccept(quest, player);
                        }
                    }
                }
            }
        }
    }

    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleQuestgiverQueryQuestOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 questId;
    bool unk1;

    recvData >> questId;

    recvData.ReadBitSeq<3, 4, 1, 0, 6, 2, 7, 5>(guid);
    unk1 = recvData.ReadBit();
    recvData.ReadByteSeq<6, 7, 4, 5, 3, 1, 0, 2>(guid);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_QUERY_QUEST npc = %u, quest = %u, unk1 = %u", uint32(GUID_LOPART(guid)), questId, unk1);

    // Verify that the guid is valid and is a questgiver or involved in the requested quest
    Object* object = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM);
    if (!object || (!object->hasQuest(questId) && !object->hasInvolvedQuest(questId)))
    {
        _player->PlayerTalkClass->SendCloseGossip();
        return;
    }

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        // not sure here what should happen to quests with QUEST_FLAGS_AUTOCOMPLETE
        // if this breaks them, add && object->GetTypeId() == TYPEID_ITEM to this check
        // item-started quests never have that flag
        if (!_player->CanTakeQuest(quest, true))
            return;

        if (quest->IsAutoAccept() && _player->CanAddQuest(quest, true))
            _player->AddQuestAndCheckCompletion(quest, object);

        _player->PlayerTalkClass->SendQuestGiverQuestDetails(quest, object->GetGUID(), true);
    }
}

void WorldSession::HandleQuestQueryOpcode(WorldPacket & recvData)
{
    if (!_player)
        return;

    uint32 questId;
    recvData >> questId;
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUEST_QUERY quest = %u", questId);

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
        _player->PlayerTalkClass->SendQuestQueryResponse(quest);
}

void WorldSession::HandleQuestgiverChooseRewardOpcode(WorldPacket& recvData)
{
    uint32 questId;
    uint32 itemId;
    ObjectGuid guid;

    recvData >> questId >> itemId;
    recvData.ReadBitSeq<6, 7, 4, 5, 0, 3, 2, 1>(guid);
    recvData.ReadByteSeq<4, 1, 2, 5, 6, 0, 7, 3>(guid);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_CHOOSE_REWARD npc = %u, quest = %u, itemId = %u", uint32(GUID_LOPART(guid)), questId, itemId);

    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return;

    Object* object = (guid == GetPlayer()->GetGUID() && quest->HasFlag(QUEST_FLAGS_AUTOCOMPLETE | QUEST_FLAGS_AUTO_SUBMIT))
            ? GetPlayer()
            : ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);

    if (object != GetPlayer())
    {
        if (!object || !object->hasInvolvedQuest(questId))
            return;

        // some kind of WPE protection
        if (!_player->CanInteractWithQuestGiver(object))
            return;
    }

    if ((!_player->CanSeeStartQuest(quest) &&  _player->GetQuestStatus(quest) == QUEST_STATUS_NONE) ||
        (_player->GetQuestStatus(quest) != QUEST_STATUS_COMPLETE && !quest->IsAutoComplete()))
    {
        TC_LOG_ERROR("network", "HACK ALERT: Player %s (guid: %u) is trying to complete quest (id: %u) but he has no right to do it!",
            _player->GetName().c_str(), _player->GetGUIDLow(), questId);
        return;
    }

    if (_player->CanRewardQuest(quest, itemId, true))
    {
        _player->RewardQuest(quest, itemId, object);

        switch (object->GetTypeId())
        {
        case TYPEID_UNIT:
            if (!sScriptMgr->OnQuestReward(_player, object->ToCreature(), quest, itemId))
            {
                // Send next quest
                if (Quest const* nextQuest = _player->GetNextQuest(guid, quest))
                {
                    if (_player->CanAddQuest(nextQuest, true) && _player->CanTakeQuest(nextQuest, true))
                    {
                        if (nextQuest->IsAutoAccept())
                            _player->AddQuestAndCheckCompletion(nextQuest, object);

                        _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextQuest, guid, true);
                    }
                }

                object->ToCreature()->AI()->sQuestReward(_player, quest, itemId);
            }
            break;
        case TYPEID_GAMEOBJECT:
            if (!sScriptMgr->OnQuestReward(_player, ((GameObject*)object), quest, itemId))
            {
                // Send next quest
                if (Quest const* nextQuest = _player->GetNextQuest(guid, quest))
                {
                    if (_player->CanAddQuest(nextQuest, true) && _player->CanTakeQuest(nextQuest, true))
                    {
                        if (nextQuest->IsAutoAccept())
                            _player->AddQuestAndCheckCompletion(nextQuest, object);

                        _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextQuest, guid, true);
                    }
                }

                object->ToGameObject()->AI()->QuestReward(_player, quest, itemId);
            }
            break;
        case TYPEID_PLAYER:
            if (quest->HasFlag(QUEST_FLAGS_AUTOCOMPLETE))
            {
                // Send next quest
                if (Quest const* nextQuest = _player->GetNextQuest(guid, quest))
                {
                    if (_player->CanAddQuest(nextQuest, true) && _player->CanTakeQuest(nextQuest, true))
                    {
                        if (nextQuest->IsAutoAccept())
                            _player->AddQuestAndCheckCompletion(nextQuest, object);

                        _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextQuest, guid, true);
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    else
        _player->PlayerTalkClass->SendQuestGiverOfferReward(quest, guid, true);
}

void WorldSession::HandleQuestgiverRequestRewardOpcode(WorldPacket & recvData)
{
    uint32 questId;
    ObjectGuid guid;

    recvData >> questId;
    recvData.ReadBitSeq<5, 0, 1, 2, 4, 3, 7, 6>(guid);
    recvData.ReadByteSeq<5, 7, 1, 3, 4, 6, 2, 0>(guid);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_REQUEST_REWARD npc = %u, quest = %u", uint32(GUID_LOPART(guid)), questId);

    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return;

    Object* object = (guid == GetPlayer()->GetGUID() && quest->HasFlag(QUEST_FLAGS_AUTOCOMPLETE))
            ? GetPlayer()
            : ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);

    if (object != GetPlayer())
    {
        if (!object || !object->hasInvolvedQuest(questId))
            return;
        // some kind of WPE protection
        if (!_player->CanInteractWithQuestGiver(object))
            return;
    }

    if (_player->CanCompleteQuest(questId))
        _player->CompleteQuest(questId);

    if (_player->GetQuestStatus(quest) != QUEST_STATUS_COMPLETE)
        return;

    _player->PlayerTalkClass->SendQuestGiverOfferReward(quest, guid, true);
}

void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recvData)
{
    uint8 slot;
    recvData >> slot;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTLOG_REMOVE_QUEST slot = %u", slot);

    if (slot < MAX_QUEST_LOG_SIZE)
    {
        if (uint32 questId = _player->GetQuestSlotQuestId(slot))
        {
            if (!_player->TakeQuestSourceItem(questId, true))
                return;                                     // can't un-equip some items, reject quest cancel

            if (const Quest *quest = sObjectMgr->GetQuestTemplate(questId))
            {
                for (uint8 i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
                {
                    if (quest->RequiredSourceItemId[i])
                    {
                        uint32 count = quest->RequiredSourceItemCount[i];
                        _player->DestroyItemCount(quest->RequiredSourceItemId[i], count ? count : 9999, true);
                    }
                }

                if (quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED))
                    _player->RemoveTimedQuest(questId);
            }

            _player->TakeQuestSourceItem(questId, true); // remove quest src item from player
            _player->SetQuestStatus(questId, QUEST_STATUS_NONE);
            _player->GetAchievementMgr().RemoveTimedAchievement(ACHIEVEMENT_TIMED_TYPE_QUEST, questId);

            for (auto &objectiveSaveStatus : _player->m_questObjectiveStatusSave)
                if (!objectiveSaveStatus.second)
                    _player->m_questObjectiveStatus.erase(objectiveSaveStatus.first);

            TC_LOG_INFO("network", "Player %u abandoned quest %u", _player->GetGUIDLow(), questId);
        }

        _player->SetQuestSlot(slot, 0);

        _player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED, 1);
    }
}

void WorldSession::HandleQuestConfirmAccept(WorldPacket& recvData)
{
    uint32 questId;
    recvData >> questId;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT questId = %u", questId);

    if (const Quest* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        if (!quest->HasFlag(QUEST_FLAGS_PARTY_ACCEPT))
            return;

        Player* pOriginalPlayer = ObjectAccessor::FindPlayer(_player->GetDivider());
        if (!pOriginalPlayer)
            return;

        if (quest->IsRaidQuest())
        {
            if (!_player->IsInSameRaidWith(pOriginalPlayer))
                return;
        }
        else
        {
            if (!_player->IsInSameGroupWith(pOriginalPlayer))
                return;
        }

        if (_player->CanAddQuest(quest, true))
            _player->AddQuestAndCheckCompletion(quest, NULL); // NULL, this prevent DB script from duplicate running

        _player->SetDivider(0);
    }
}

void WorldSession::HandleQuestgiverCompleteQuest(WorldPacket& recvData)
{
    uint32 questId;
    ObjectGuid playerGuid;
    bool autoCompleteMode;      // 0 - standard complete quest mode with npc, 1 - auto-complete mode
    recvData >> questId;

    recvData.ReadBitSeq<0, 4, 3>(playerGuid);
    autoCompleteMode = recvData.ReadBit();
    recvData.ReadBitSeq<1, 5, 6, 7, 2>(playerGuid);
    recvData.ReadByteSeq<7, 0, 4, 1, 6, 3, 5, 2>(playerGuid);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_COMPLETE_QUEST npc = %u, questId = %u", uint32(GUID_LOPART(playerGuid)), questId);

    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return;

    Object* object = (playerGuid == GetPlayer()->GetGUID() && quest->HasFlag(QUEST_FLAGS_AUTOCOMPLETE))
            ? GetPlayer()
            : ObjectAccessor::GetObjectByTypeMask(*_player, playerGuid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);

    if (object != GetPlayer())
    {
        if (!object || !object->hasInvolvedQuest(questId))
            return;

        if (!autoCompleteMode)
        {
            // some kind of WPE protection
            if (!_player->CanInteractWithQuestGiver(object))
                return;
        }
        else if (!quest->HasFlag(QUEST_FLAGS_AUTO_SUBMIT))
        {
            TC_LOG_ERROR("network", "Possible hacking attempt: Player %s [playerGuid: %u] tried to complete questId [entry: %u] by auto-submit flag for quest which not suport it.",
                         _player->GetName().c_str(), _player->GetGUIDLow(), questId);
            return;
        }
    }

    if (!_player->CanSeeStartQuest(quest) && _player->GetQuestStatus(quest) == QUEST_STATUS_NONE)
    {
        TC_LOG_ERROR("network", "Possible hacking attempt: Player %s [playerGuid: %u] tried to complete questId [entry: %u] without being in possession of the questId!",
                      _player->GetName().c_str(), _player->GetGUIDLow(), questId);
        return;
    }

    // TODO: need a virtual function
    if (_player->InBattleground())
        if (Battleground* bg = _player->GetBattleground())
            if (bg->GetTypeID() == BATTLEGROUND_AV)
                ((BattlegroundAV*)bg)->HandleQuestComplete(questId, _player);

    if (_player->GetQuestStatus(quest) != QUEST_STATUS_COMPLETE)
    {
        if (quest->IsRepeatable())
            _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, playerGuid, _player->CanCompleteRepeatableQuest(quest), false);
        else
            _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, playerGuid, _player->CanRewardQuest(quest, false), false);
    }
    else
    {
        // quest requires items, money or currency to complete
        if (quest->GetQuestObjectiveCountType(QUEST_OBJECTIVE_TYPE_ITEM)
            || quest->GetQuestObjectiveCountType(QUEST_OBJECTIVE_TYPE_MONEY)
            || quest->GetQuestObjectiveCountType(QUEST_OBJECTIVE_TYPE_CURRENCY))
            _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, playerGuid, _player->CanRewardQuest(quest, false), false);
        else
            _player->PlayerTalkClass->SendQuestGiverOfferReward(quest, playerGuid, !autoCompleteMode);
    }
}

void WorldSession::HandlePushQuestToParty(WorldPacket& recvPacket)
{
    uint32 questId;
    recvPacket >> questId;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_PUSHQUESTTOPARTY questId = %u", questId);

    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return;

    {
        auto const questStatus = _player->GetQuestStatus(quest);
        if (questStatus == QUEST_STATUS_NONE || questStatus == QUEST_STATUS_REWARDED)
            return;
    }

    Player * const sender = GetPlayer();

    Group* group = sender->GetGroup();
    if (!group)
        return;

    for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* receiver = itr->GetSource();

        if (!receiver || receiver == sender)         // skip self
            continue;

        sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_SHARING_QUEST);

        if (!receiver->SatisfyQuestStatus(quest, false))
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_HAVE_QUEST);
            continue;
        }

        if (receiver->GetQuestStatus(quest) == QUEST_STATUS_COMPLETE)
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_FINISH_QUEST);
            continue;
        }

        if (!receiver->CanTakeQuest(quest, false))
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_CANT_TAKE_QUEST);
            continue;
        }

        if (!receiver->SatisfyQuestLog(false))
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_LOG_FULL);
            continue;
        }

        if (receiver->GetDivider() != 0)
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_BUSY);
            continue;
        }

        sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_SHARING_QUEST);

        if (quest->IsAutoAccept() && receiver->CanAddQuest(quest, true) && receiver->CanTakeQuest(quest, true))
            receiver->AddQuestAndCheckCompletion(quest, sender);

        if (quest->IsAutoComplete() && quest->IsRepeatable() && !quest->IsDailyOrWeekly())
            receiver->PlayerTalkClass->SendQuestGiverRequestItems(quest, sender->GetGUID(), receiver->CanCompleteRepeatableQuest(quest), true);
        else
        {
            receiver->SetDivider(sender->GetGUID());
            receiver->PlayerTalkClass->SendQuestGiverQuestDetails(quest, sender->GetGUID(), true);
        }
    }
}

void WorldSession::HandleQuestPushResult(WorldPacket& recvPacket)
{
    uint64 guid;
    uint8 msg;
    recvPacket >> guid >> msg;

    TC_LOG_DEBUG("network", "WORLD: Received SMSG_QUEST_PUSH_RESULT");

    if (_player->GetDivider() != 0)
    {
        Player* player = ObjectAccessor::FindPlayer(_player->GetDivider());
        if (player)
        {
            WorldPacket data(SMSG_QUEST_PUSH_RESULT, (8+1));
            ObjectGuid guidObj = player->GetGUID();
            data.WriteBitSeq<1, 0, 6, 5, 7, 4, 3, 2>(guidObj);
            data.WriteByteSeq<7, 5, 1, 6, 3, 2, 4, 0>(guidObj);
            data << uint8(msg);
            player->GetSession()->SendPacket(&data);
            _player->SetDivider(0);
        }
    }
}

uint32 WorldSession::getDialogStatus(Player* player, Object* questgiver, uint32 defstatus)
{
    uint32 result = defstatus;

    QuestRelationBounds qr;
    QuestRelationBounds qir;

    switch (questgiver->GetTypeId())
    {
        case TYPEID_GAMEOBJECT:
        {
            qr  = sObjectMgr->GetGOQuestRelationBounds(questgiver->GetEntry());
            qir = sObjectMgr->GetGOQuestInvolvedRelationBounds(questgiver->GetEntry());
            break;
        }
        case TYPEID_UNIT:
        {
            qr  = sObjectMgr->GetCreatureQuestRelationBounds(questgiver->GetEntry());
            qir = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(questgiver->GetEntry());
            break;
        }
        default:
            //its imposible, but check ^)
            TC_LOG_ERROR("network", "Warning: GetDialogStatus called for unexpected type %u", questgiver->GetTypeId());
            return DIALOG_STATUS_NONE;
    }

    for (QuestRelations::const_iterator i = qir.first; i != qir.second; ++i)
    {
        uint32 result2 = 0;
        uint32 quest_id = i->second;
        Quest const* quest = sObjectMgr->GetQuestTemplate(quest_id);
        if (!quest)
            continue;

        ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK, quest->GetQuestId());
        if (!sConditionMgr->IsObjectMeetToConditions(player, conditions))
            continue;

        QuestStatus status = player->GetQuestStatus(quest);
        if ((status == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(quest_id)) ||
            (quest->IsAutoComplete() && player->CanTakeQuest(quest, false)))
        {
            if (quest->IsAutoComplete() && quest->IsRepeatable())
                result2 = DIALOG_STATUS_REWARD_REP;
            else
                result2 = DIALOG_STATUS_REWARD;
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
            result2 = DIALOG_STATUS_INCOMPLETE;

        if (result2 > result)
            result = result2;
    }

    for (QuestRelations::const_iterator i = qr.first; i != qr.second; ++i)
    {
        uint32 result2 = 0;
        uint32 quest_id = i->second;
        Quest const* quest = sObjectMgr->GetQuestTemplate(quest_id);
        if (!quest)
            continue;

        ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK, quest->GetQuestId());
        if (!sConditionMgr->IsObjectMeetToConditions(player, conditions))
            continue;

        QuestStatus status = player->GetQuestStatus(quest);
        if (status == QUEST_STATUS_NONE)
        {
            if (player->CanSeeStartQuest(quest))
            {
                if (player->SatisfyQuestLevel(quest, false))
                {
                    if (quest->IsAutoComplete() || (quest->IsRepeatable() && player->IsQuestRewarded(quest_id)))
                        result2 = DIALOG_STATUS_REWARD_REP;
                    else if (player->getLevel() <= ((player->GetQuestLevel(quest) == -1) ? player->getLevel() : player->GetQuestLevel(quest) + sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF)))
                    {
                        if (quest->HasFlag(QUEST_FLAGS_DAILY) || quest->HasFlag(QUEST_FLAGS_WEEKLY))
                            result2 = DIALOG_STATUS_AVAILABLE_REP;
                        else
                            result2 = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                        result2 = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
                }
                else
                    result2 = DIALOG_STATUS_UNAVAILABLE;
            }
        }

        if (result2 > result)
            result = result2;
    }

    return result;
}

void WorldSession::HandleQuestgiverStatusMultipleQuery(WorldPacket& /*recvPacket*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY");

    uint32 count = 0;
    for (Player::ClientGUIDs::const_iterator itr = _player->m_clientGUIDs.begin(); itr != _player->m_clientGUIDs.end(); ++itr)
    {
        if (IS_CRE_OR_VEH_OR_PET_GUID(*itr))
        {
            // need also pet quests case support
            Creature* questgiver = ObjectAccessor::GetCreatureOrPetOrVehicle(*GetPlayer(), *itr);
            if (!questgiver || questgiver->IsHostileTo(_player))
                continue;
            if (!questgiver->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
                continue;

            count++;

        }
        else if (IS_GAMEOBJECT_GUID(*itr))
        {
            GameObject* questgiver = GetPlayer()->GetMap()->GetGameObject(*itr);
            if (!questgiver)
                continue;
            if (questgiver->GetGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
                continue;

            count++;
        }
    }

    ByteBuffer buff;
    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE);
    data.WriteBits(count, 21);

    for (Player::ClientGUIDs::const_iterator itr = _player->m_clientGUIDs.begin(); itr != _player->m_clientGUIDs.end(); ++itr)
    {
        uint32 questStatus = DIALOG_STATUS_NONE;
        uint32 defstatus = DIALOG_STATUS_NONE;

        if (IS_CRE_OR_VEH_OR_PET_GUID(*itr))
        {
            // need also pet quests case support
            Creature* questgiver = ObjectAccessor::GetCreatureOrPetOrVehicle(*GetPlayer(), *itr);
            if (!questgiver || questgiver->IsHostileTo(_player))
                continue;
            if (!questgiver->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
                continue;
            questStatus = sScriptMgr->GetDialogStatus(_player, questgiver);
            if (questStatus > 6)
                questStatus = getDialogStatus(_player, questgiver, defstatus);

            ObjectGuid guid = questgiver->GetGUID();

            data.WriteBitSeq<6, 3, 4, 2, 5, 1, 7, 0>(guid);
            buff.WriteByteSeq<7, 1, 2>(guid);
            buff << uint32(questStatus);
            buff.WriteByteSeq<3, 5, 4, 0, 6>(guid);
        }
        else if (IS_GAMEOBJECT_GUID(*itr))
        {
            GameObject* questgiver = GetPlayer()->GetMap()->GetGameObject(*itr);
            if (!questgiver)
                continue;
            if (questgiver->GetGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
                continue;
            questStatus = sScriptMgr->GetDialogStatus(_player, questgiver);
            if (questStatus > 6)
                questStatus = getDialogStatus(_player, questgiver, defstatus);

            ObjectGuid guid = questgiver->GetGUID();

            data.WriteBitSeq<6, 3, 4, 2, 5, 1, 7, 0>(guid);
            buff.WriteByteSeq<7, 1, 2>(guid);
            buff << uint32(questStatus);
            buff.WriteByteSeq<3, 5, 4, 0, 6>(guid);
        }
    }

    data.FlushBits();
    data.append(buff);

    SendPacket(&data);
}

void WorldSession::HandleQueryQuestsCompleted(WorldPacket& /*recvData*/)
{
    size_t rew_count = _player->GetRewardedQuestCount();

    WorldPacket data(SMSG_QUERY_QUESTS_COMPLETED_RESPONSE, 4 + 4 * rew_count);
    data << uint32(rew_count);

    const RewardedQuestSet &rewQuests = _player->getRewardedQuests();
    for (RewardedQuestSet::const_iterator itr = rewQuests.begin(); itr != rewQuests.end(); ++itr)
        data << uint32(*itr);

    SendPacket(&data);
}
