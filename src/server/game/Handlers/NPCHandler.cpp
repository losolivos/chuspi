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
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "UpdateMask.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "Pet.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "ScriptMgr.h"
#include "CreatureAI.h"
#include "SpellInfo.h"
#include "Guild.h"

enum StableResultCode
{
    STABLE_ERR_MONEY        = 0x01,                         // "you don't have enough money"
    STABLE_ERR_STABLE       = 0x06,                         // currently used in most fail cases
    STABLE_SUCCESS_STABLE   = 0x08,                         // stable success
    STABLE_SUCCESS_UNSTABLE = 0x09,                         // unstable/swap success
    STABLE_SUCCESS_BUY_SLOT = 0x0A,                         // buy slot success
    STABLE_ERR_EXOTIC       = 0x0C,                         // "you are unable to control exotic creatures"
};

void WorldSession::HandleTabardVendorActivateOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleTabardVendorActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendTabardVendorActivate(guid);
}

void WorldSession::SendTabardVendorActivate(uint64 guid)
{
    ObjectGuid playerGuid = guid;
    WorldPacket data(SMSG_PLAYER_TABARD_VENDOR_SHOW);

    data.WriteBitSeq<7, 0, 3, 6, 4, 1, 5, 2>(playerGuid);
    data.WriteByteSeq<6, 2, 5, 7, 1, 0, 4, 3>(playerGuid);

    SendPacket(&data);
}

void WorldSession::HandleBankerActivateOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BANKER_ACTIVATE");

    recvData.ReadBitSeq<0, 2, 1, 6, 7, 3, 4, 5>(guid);
    recvData.ReadByteSeq<7, 4, 0, 3, 2, 1, 5, 6>(guid);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BANKER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleBankerActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendShowBank(guid);
}

void WorldSession::SendShowBank(uint64 guid)
{
    WorldPacket data(SMSG_SHOW_BANK);
    ObjectGuid npcGuid = guid;

    data.WriteBitSeq<1, 2, 0, 4, 6, 3, 5, 7>(npcGuid);
    data.WriteByteSeq<4, 6, 0, 5, 7, 3, 2, 1>(npcGuid);

    SendPacket(&data);
}

void WorldSession::HandleTrainerListOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    recvData.ReadBitSeq<5, 7, 6, 3, 1, 4, 0, 2>(guid);
    recvData.ReadByteSeq<4, 7, 5, 2, 1, 3, 0, 6>(guid);

    SendTrainerList(guid);
}

void WorldSession::SendTrainerList(uint64 guid)
{
    std::string str = GetTrinityString(LANG_NPC_TAINER_HELLO);
    SendTrainerList(guid, str);
}

void WorldSession::SendTrainerList(uint64 guid, const std::string& strTitle)
{
    TC_LOG_DEBUG("network", "WORLD: SendTrainerList");

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: SendTrainerList - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // trainer list loaded at check;
    if (!unit->isCanTrainingOf(_player, true))
        return;

    CreatureTemplate const* ci = unit->GetCreatureTemplate();

    if (!ci)
    {
        TC_LOG_DEBUG("network", "WORLD: SendTrainerList - (GUID: %u) NO CREATUREINFO!", GUID_LOPART(guid));
        return;
    }

    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        TC_LOG_DEBUG("network", "WORLD: SendTrainerList - Training spells not found for creature (GUID: %u Entry: %u)",
            GUID_LOPART(guid), unit->GetEntry());
        return;
    }

    ByteBuffer dataBuffer;
    WorldPacket data(SMSG_TRAINER_LIST);
    ObjectGuid npcGuid = guid;

    data.WriteBitSeq<0>(npcGuid);
    data.WriteBits(strTitle.size(), 11);
    data.WriteBitSeq<5, 6, 1, 2, 7, 4, 3>(npcGuid);

    // reputation discount
    float fDiscountMod = _player->GetReputationPriceDiscount(unit);

    uint32 count = 0;
    for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;

        bool valid = true;
        for (auto const &spellId : tSpell->learnedSpell)
        {
            if (spellId == 0)
                continue;

            auto const spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            if (!_player->IsSpellFitByClassAndRace(spellInfo))
            {
                valid = false;
                break;
            }
        }

        if (!valid)
            continue;

        ConditionList conditions = sConditionMgr->GetConditionsForNpcTrainerEvent(unit->GetEntry(), tSpell->spell);
        if (!sConditionMgr->IsObjectMeetToConditions(_player, unit, conditions))
        {
            TC_LOG_DEBUG("condition", "SendTrainerList: conditions not met for creature entry %u spell %u", unit->GetEntry(), tSpell->spell);
            continue;
        }

        TrainerSpellState state = _player->GetTrainerSpellState(tSpell);

        dataBuffer << uint32(tSpell->reqSkillValue);
        dataBuffer << uint32(floor(tSpell->spellCost * fDiscountMod));
        dataBuffer << uint8(tSpell->reqLevel);

        //prev + req or req + 0
        uint8 maxReq = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (!tSpell->learnedSpell[i])
                continue;
            if (uint32 prevSpellId = sSpellMgr->GetPrevSpellInChain(tSpell->learnedSpell[i]))
            {
                dataBuffer << uint32(prevSpellId);
                ++maxReq;
            }
            if (maxReq == 1)
                break;
            SpellsRequiringSpellMapBounds spellsRequired = sSpellMgr->GetSpellsRequiredForSpellBounds(tSpell->learnedSpell[i]);
            for (SpellsRequiringSpellMap::const_iterator itr2 = spellsRequired.first; itr2 != spellsRequired.second && maxReq < 2; ++itr2)
            {
                dataBuffer << uint32(itr2->second);
                ++maxReq;
            }
            if (maxReq == 1)
                break;
        }
        while (maxReq < 1)
        {
            dataBuffer << uint32(0);
            ++maxReq;
        }

        dataBuffer << uint32(0); // Profession Dialog or Profession Button
        dataBuffer << uint32(0); // Profession Dialog or Profession Button
        dataBuffer << uint32(tSpell->spell);
        dataBuffer << uint8(state == TRAINER_SPELL_GREEN_DISABLED ? TRAINER_SPELL_GREEN : state);
        dataBuffer << uint32(tSpell->reqSkill);

        ++count;
    }

    data.WriteBits(count, 19);
    data.FlushBits();

    if (dataBuffer.size() > 0)
        data.append(dataBuffer);

    data.WriteByteSeq<5, 7, 6>(npcGuid);

    if (strTitle.size() > 0)
        data.append(strTitle.c_str(), strTitle.size());

    data << uint32(1); // different value for each trainer, also found in CMSG_TRAINER_BUY_SPELL
    data.WriteByteSeq<2, 3, 1, 0, 4>(npcGuid);
    data << uint32(unit->GetCreatureTemplate()->trainer_type);

    SendPacket(&data);
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 spellId;
    int32 trainerId;

    recvData >> trainerId >> spellId;

    recvData.ReadBitSeq<0, 5, 4, 6, 1, 2, 7, 3>(guid);
    recvData.ReadByteSeq<3, 7, 2, 0, 5, 6, 1, 4>(guid);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u", uint32(GUID_LOPART(guid)), spellId);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleTrainerBuySpellOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (!unit->isCanTrainingOf(_player, true))
    {
        SendTrainerService(guid, spellId, 0);
        return;
    }

    // check present spell in trainer spell list
    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        SendTrainerService(guid, spellId, 0);
        return;
    }

    // not found, cheat?
    TrainerSpell const* trainer_spell = trainer_spells->Find(spellId);
    if (!trainer_spell)
    {
        SendTrainerService(guid, spellId, 0);
        return;
    }

    // can't be learn, cheat? Or double learn with lags...
    if (_player->GetTrainerSpellState(trainer_spell) != TRAINER_SPELL_GREEN)
    {
        SendTrainerService(guid, spellId, 0);
        return;
    }

    // apply reputation discount
    uint32 nSpellCost = uint32(floor(trainer_spell->spellCost * _player->GetReputationPriceDiscount(unit)));

    // check money requirement
    if (!_player->HasEnoughMoney(uint64(nSpellCost)))
    {
        SendTrainerService(guid, spellId, 1);
        return;
    }

    _player->ModifyMoney(-int64(nSpellCost));

    unit->SendPlaySpellVisualKit(179, 0);       // 53 SpellCastDirected
    _player->SendPlaySpellVisualKit(362, 1);    // 113 EmoteSalute

    // learn explicitly or cast explicitly
    if (trainer_spell->IsCastable())
        _player->CastSpell(_player, trainer_spell->spell, true);
    else
        _player->learnSpell(spellId, false);

    SendTrainerService(guid, spellId, 2);

}

void WorldSession::SendTrainerService(uint64 guid, uint32 spellId, uint32 result)
{
    WorldPacket data(SMSG_TRAINER_SERVICE, 16);
    ObjectGuid npcGuid = guid;

    data << uint32(result);         // 2 == Success. 1 == "Not enough money for trainer service." 0 == "Trainer service %d unavailable."
    data << uint32(spellId);        // should be same as in packet from client

    data.WriteBitSeq<0, 3, 6, 1, 2, 5, 7, 4>(npcGuid);
    data.WriteByteSeq<6, 2, 3, 5, 7, 4, 1, 0>(npcGuid);

    SendPacket(&data);
}

void WorldSession::HandleGossipHelloOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GOSSIP_HELLO");

    ObjectGuid guid;

    recvData.ReadBitSeq<2, 3, 0, 7, 5, 4, 6, 1>(guid);
    recvData.ReadByteSeq<2, 6, 0, 3, 1, 5, 7, 4>(guid);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGossipHelloOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // set faction visible if needed
    if (FactionTemplateEntry const* factionTemplateEntry = sFactionTemplateStore.LookupEntry(unit->getFaction()))
        _player->GetReputationMgr().SetVisible(factionTemplateEntry);

    GetPlayer()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
    // remove fake death
    //if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
    //    GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (unit->IsArmorer() || unit->isCivilian() || unit->IsQuestGiver() || unit->IsServiceProvider() || unit->isGuard())
        unit->StopMoving();

    // If spiritguide, no need for gossip menu, just put player into resurrect queue
    if (unit->IsSpiritGuide())
    {
        Battleground* bg = _player->GetBattleground();
        if (bg)
        {
            bg->AddPlayerToResurrectQueue(unit->GetGUID(), _player->GetGUID());
            sBattlegroundMgr->SendAreaSpiritHealerQueryOpcode(_player, bg, unit->GetGUID());
            return;
        }
    }

    if (!sScriptMgr->OnGossipHello(_player, unit))
    {
//        _player->TalkedToCreature(unit->GetEntry(), unit->GetGUID());
        _player->PrepareGossipMenu(unit, unit->GetCreatureTemplate()->GossipMenuId, true);
        _player->SendPreparedGossip(unit);
    }
    unit->AI()->sGossipHello(_player);
}

void WorldSession::HandleSpiritHealerActivateOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_SPIRIT_HEALER_ACTIVATE");

    ObjectGuid guid;

    recvData.ReadBitSeq<7, 2, 1, 5, 6, 0, 3, 4>(guid);
    recvData.ReadByteSeq<1, 7, 6, 0, 5, 2, 4, 3>(guid);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_SPIRITHEALER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleSpiritHealerActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    _player->ResurrectPlayer(0.5f, true);

    _player->DurabilityLossAll(0.25f, true);

    // get corpse nearest graveyard
    WorldSafeLocsEntry const* corpseGrave = NULL;
    Corpse* corpse = _player->GetCorpse();
    if (corpse)
        corpseGrave = sObjectMgr->GetClosestGraveYard(
            corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetMapId(), _player->GetTeam());

    // now can spawn bones
    _player->SpawnCorpseBones();

    // teleport to nearest from corpse graveyard, if different from nearest to player ghost
    if (corpseGrave)
    {
        WorldSafeLocsEntry const* ghostGrave = sObjectMgr->GetClosestGraveYard(
            _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId(), _player->GetTeam());

        if (corpseGrave != ghostGrave)
            _player->TeleportTo(corpseGrave->map_id, corpseGrave->x, corpseGrave->y, corpseGrave->z, _player->GetOrientation());
        // or update at original position
        else
            _player->UpdateObjectVisibility();
    }
    // or update at original position
    else
        _player->UpdateObjectVisibility();
}

void WorldSession::HandleBinderActivateOpcode(WorldPacket& recvData)
{
    ObjectGuid npcGUID;

    recvData.ReadBitSeq<6, 4, 2, 0, 3, 7, 5, 1>(npcGUID);
    recvData.ReadByteSeq<2, 6, 0, 5, 3, 1, 7, 4>(npcGUID);

    if (!GetPlayer()->IsInWorld() || !GetPlayer()->IsAlive())
        return;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_INNKEEPER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleBinderActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature* npc)
{
    // prevent set homebind to instances in any case
    if (GetPlayer()->GetMap()->Instanceable())
        return;

    uint32 bindspell = 3286;

    // update sql homebind
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PLAYER_HOMEBIND);
    stmt->setUInt16(0, _player->GetMapId());
    stmt->setUInt16(1, _player->GetAreaId());
    stmt->setFloat (2, _player->GetPositionX());
    stmt->setFloat (3, _player->GetPositionY());
    stmt->setFloat (4, _player->GetPositionZ());
    stmt->setUInt32(5, _player->GetGUIDLow());
    CharacterDatabase.Execute(stmt);

    _player->m_homebindMapId = _player->GetMapId();
    _player->m_homebindAreaId = _player->GetAreaId();
    _player->m_homebindX = _player->GetPositionX();
    _player->m_homebindY = _player->GetPositionY();
    _player->m_homebindZ = _player->GetPositionZ();

    // send spell for homebinding (3286)
    _player->CastSpell(_player, bindspell, true);

    SendTrainerService(npc->GetGUID(), bindspell, 2);
    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleListPetsOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Recv MSG_LIST_PETS");
    ObjectGuid npcGUID;

    recvData.ReadBitSeq<0, 7, 2, 4, 5, 3, 1, 6>(npcGUID);
    recvData.ReadByteSeq<0, 2, 3, 1, 7, 5, 6, 4>(npcGUID);

    if (!CheckStableMaster(npcGUID))
        return;

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveAurasByType(SPELL_AURA_MOUNTED);

    SendPetList(npcGUID, PET_SLOT_ACTIVE_FIRST, PET_SLOT_STABLE_LAST);
}

void WorldSession::SendPetList(uint64 guid, uint8 first, uint8 last)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_INFO_LIST);

    stmt->setUInt32(0, _player->GetGUIDLow());
    stmt->setUInt8(1, first);
    stmt->setUInt8(2, last);

    _sendPetListCallback.SetParam(guid);
    _sendPetListCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::SendPetListCallback(PreparedQueryResult result, uint64 guid)
{
    if (!GetPlayer())
        return;

    TC_LOG_DEBUG("network", "WORLD: Recv SMSG_PET_STABLE_LIST Send.");

    WorldPacket data(SMSG_PET_STABLE_LIST, 200);           // guessed size
    ObjectGuid npcGuid = guid;
    ByteBuffer dataBuffer;

    data.WriteBitSeq<6, 1, 5, 4, 2>(npcGuid);
    data.WriteBits(result ? result->GetRowCount() : 0, 19);
    data.WriteBitSeq<7>(npcGuid);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            std::string name = fields[4].GetString();

            data.WriteBits(name.size(), 8);

            uint8 petStableState = fields[0].GetUInt8() <= PET_SLOT_ACTIVE_LAST ? 1 : 2;

            uint32 modelId = 0;
            if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(fields[1].GetUInt32()))
                modelId = cInfo->Modelid1 ? cInfo->Modelid1 : cInfo->Modelid2;

            dataBuffer << uint32(fields[2].GetUInt32());          // creature entry
            dataBuffer << uint32(fields[1].GetUInt32());          // petnumber
            dataBuffer << uint32(fields[3].GetUInt8());           // level
            dataBuffer << uint8(petStableState);                  // 1 = current, 2/3 = in stable (any from 4, 5, ... create problems with proper show)
            dataBuffer << uint32(fields[0].GetUInt8());           // slot
            dataBuffer << uint32(modelId);                        // creature modelid

            if (name.size())
                dataBuffer.append(name.c_str(), name.size());
        }
        while (result->NextRow());
    }

    data.WriteBitSeq<3, 0>(npcGuid);
    data.FlushBits();
    data.append(dataBuffer);
    data.WriteByteSeq<2, 0, 6, 1, 7, 5, 3, 4>(npcGuid);

    SendPacket(&data);
}

void WorldSession::SendStableResult(uint8 res)
{
    WorldPacket data(SMSG_STABLE_RESULT, 1);
    data << uint8(res);
    SendPacket(&data);
}

void WorldSession::HandleSetPetSlot(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Recv CMSG_SET_PET_SLOT.");
    ObjectGuid npcGuid;
    uint32 petId;
    uint8 newSlot;

    recvData >> petId >> newSlot;

    recvData.ReadBitSeq<3, 2, 4, 6, 0, 1, 7, 5>(npcGuid);
    recvData.ReadByteSeq<5, 3, 2, 0, 1, 4, 7, 6>(npcGuid);

    if (!CheckStableMaster(npcGuid))
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    if (newSlot > PET_SLOT_STABLE_LAST)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    Pet* pet = _player->GetPet();

    // if we move the pet already summoned...
    if (pet && pet->GetCharmInfo() && pet->GetCharmInfo()->GetPetNumber() == petId)
        GetPlayer()->RemovePet(PET_REMOVE_DISMISS, PET_REMOVE_FLAG_RETURN_REAGENT | PET_REMOVE_FLAG_RESET_CURRENT);

    // if we move to the pet already summoned
    if (newSlot <= PET_SLOT_ACTIVE_LAST)
    {
        if (pet && pet->GetCharmInfo()->GetPetNumber() == GetPlayer()->GetPetIdBySlot(newSlot))
            GetPlayer()->RemovePet(PET_REMOVE_DISMISS, PET_REMOVE_FLAG_RETURN_REAGENT | PET_REMOVE_FLAG_RESET_CURRENT);
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_INFO_FOR_SLOT_CHANGE);
    stmt->setUInt32(0, petId);

    _setPetSlotCallback.SetParam(newSlot);
    _setPetSlotCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::SetPetSlotCallback(PreparedQueryResult result, uint8 newSlot)
{
    if (!GetPlayer())
        return;

    if (!result)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    Field* fields = result->Fetch();

    uint8 oldSlot = fields[0].GetUInt8();
    uint32 newPetEntry = fields[1].GetUInt32();
    uint32 newPetId = fields[2].GetUInt32();

    if (newPetEntry == 0)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(newPetEntry);
    if (!creatureInfo)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    if (!creatureInfo->isTameable(GetPlayer()->CanTameExoticPets()))
    {
        SendStableResult(creatureInfo->isExotic() ? STABLE_ERR_EXOTIC : STABLE_ERR_STABLE);
        return;
    }

    PreparedStatement* stmt;
    uint32 oldPetId = GetPlayer()->GetPetIdBySlot(newSlot);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SLOT_BY_PET_ID);
    stmt->setUInt32(0, newPetId);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SLOT);
    stmt->setUInt32(0, newPetId);
    stmt->setUInt8(1, newSlot);
    trans->Append(stmt);

    if (oldPetId != 0)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SLOT_BY_PET_ID);
        stmt->setUInt32(0, oldPetId);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SLOT);
        stmt->setUInt32(0, oldPetId);
        stmt->setUInt8(1, oldSlot);
        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);

    GetPlayer()->SwapPetSlots(oldSlot, newSlot);

    SendStableResult(STABLE_SUCCESS_STABLE);
}

void WorldSession::HandleRepairItemOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_REPAIR_ITEM");

    ObjectGuid npcGUID, itemGUID;
    bool guildBank;                                        // new in 2.3.2, bool that means from guild bank money

    recvData.ReadBitSeq<1, 7>(npcGUID);
    recvData.ReadBitSeq<7, 6>(itemGUID);
    recvData.ReadBitSeq<2>(npcGUID);
    recvData.ReadBitSeq<5>(itemGUID);
    recvData.ReadBitSeq<6>(npcGUID);
    recvData.ReadBitSeq<4, 0>(itemGUID);

    guildBank = recvData.ReadBit();

    recvData.ReadBitSeq<3, 0>(npcGUID);
    recvData.ReadBitSeq<1, 2>(itemGUID);
    recvData.ReadBitSeq<5>(npcGUID);
    recvData.ReadBitSeq<3>(itemGUID);
    recvData.ReadBitSeq<4>(npcGUID);

    recvData.ReadByteSeq<1>(npcGUID);
    recvData.ReadByteSeq<1, 3>(itemGUID);
    recvData.ReadByteSeq<7>(npcGUID);
    recvData.ReadByteSeq<4>(itemGUID);
    recvData.ReadByteSeq<4, 6>(npcGUID);
    recvData.ReadByteSeq<6>(itemGUID);
    recvData.ReadByteSeq<2>(npcGUID);
    recvData.ReadByteSeq<7, 0>(itemGUID);
    recvData.ReadByteSeq<0>(npcGUID);
    recvData.ReadByteSeq<2>(itemGUID);
    recvData.ReadByteSeq<5, 3>(npcGUID);
    recvData.ReadByteSeq<5>(itemGUID);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_REPAIR);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleRepairItemOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // reputation discount
    float discountMod = _player->GetReputationPriceDiscount(unit);

    if (itemGUID)
    {
        TC_LOG_DEBUG("network", "ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        Item* item = _player->GetItemByGuid(itemGUID);
        if (item)
            _player->DurabilityRepair(item->GetPos(), true, discountMod, guildBank);
    }
    else
    {
        TC_LOG_DEBUG("network", "ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));
        _player->DurabilityRepairAll(true, discountMod, guildBank);
    }
}
