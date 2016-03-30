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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include <list>
#include <vector>
#include <utility>

void WorldSession::SendVoidStorageTransferResult(VoidTransferError result)
{
    WorldPacket data(SMSG_VOID_TRANSFER_RESULT, 4);
    data << uint32(result);
    SendPacket(&data);
}

void WorldSession::HandleVoidStorageUnlock(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_VOID_STORAGE_UNLOCK");
    Player* player = GetPlayer();

    ObjectGuid npcGuid;

    recvData.ReadBitSeq<6, 5, 0, 3, 1, 7, 4, 2>(npcGuid);
    recvData.ReadByteSeq<7, 6, 5, 1, 4, 3, 2, 0>(npcGuid);

    Creature* unit = player->GetNPCIfCanInteractWith(npcGuid, UNIT_NPC_FLAG_VAULTKEEPER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageUnlock - Unit (GUID: %u) not found or player can't interact with it.", GUID_LOPART(npcGuid));
        return;
    }

    if (player->IsVoidStorageUnlocked())
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageUnlock - Player (GUID: %u, name: %s) tried to unlock void storage a 2nd time.",
                     player->GetGUIDLow(), player->GetName().c_str());
        return;
    }

    player->ModifyMoney(-int64(VOID_STORAGE_UNLOCK));
    player->UnlockVoidStorage();
}

void WorldSession::HandleVoidStorageQuery(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_VOID_STORAGE_QUERY");
    Player* player = GetPlayer();

    ObjectGuid npcGuid;

    recvData.ReadBitSeq<7, 0, 5, 6, 3, 1, 4, 2>(npcGuid);
    recvData.ReadByteSeq<3, 1, 7, 5, 2, 6, 0, 4>(npcGuid);

    Creature* unit = player->GetNPCIfCanInteractWith(npcGuid, UNIT_NPC_FLAG_VAULTKEEPER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageQuery - Unit (GUID: %u) not found or player can't interact with it.",
                     GUID_LOPART(npcGuid));
        return;
    }

    if (!player->IsVoidStorageUnlocked())
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageQuery - Player (GUID: %u, name: %s) queried void storage without unlocking it.",
                     player->GetGUIDLow(), player->GetName().c_str());
        return;
    }

    uint8 count = 0;
    for (uint8 i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
        if (player->GetVoidStorageItem(i))
            ++count;

    WorldPacket data(SMSG_VOID_STORAGE_CONTENTS);
    ByteBuffer itemData;

    data.WriteBits(count, 7);

    for (uint8 i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
    {
        VoidStorageItem* item = player->GetVoidStorageItem(i);
        if (!item)
            continue;

        ObjectGuid itemId = item->ItemId;
        ObjectGuid creatorGuid = item->CreatorGuid;

        data.WriteBitSeq<4, 0, 5>(itemId);
        data.WriteBitSeq<4, 3, 5, 7, 1>(creatorGuid);
        data.WriteBitSeq<7>(itemId);
        data.WriteBitSeq<6>(creatorGuid);
        data.WriteBitSeq<2>(itemId);
        data.WriteBitSeq<2, 0>(creatorGuid);
        data.WriteBitSeq<1, 6, 3>(itemId);

        itemData.WriteByteSeq<7>(itemId);
        itemData.WriteByteSeq<1, 6>(creatorGuid);
        itemData << uint32(i);
        itemData.WriteByteSeq<6>(itemId);
        itemData << uint32(item->ItemEntry);
        itemData.WriteByteSeq<2>(itemId);
        itemData.WriteByteSeq<7>(creatorGuid);
        itemData.WriteByteSeq<0>(itemId);
        itemData.WriteByteSeq<5>(creatorGuid);
        itemData.WriteByteSeq<3>(itemId);
        itemData << uint32(item->ItemSuffixFactor);
        itemData.WriteByteSeq<4>(itemId);
        itemData.WriteByteSeq<0>(creatorGuid);
        itemData.WriteByteSeq<5>(itemId);
        itemData.WriteByteSeq<3, 4>(creatorGuid);
        itemData << uint32(item->ItemRandomPropertyId);
        itemData.WriteByteSeq<2>(creatorGuid);
        itemData << uint32(0);                          // @TODO : Item Upgrade level !
        itemData.WriteByteSeq<1>(itemId);
    }

    data.FlushBits();

    if (itemData.size() > 0)
        data.append(itemData);

    SendPacket(&data);
}

void WorldSession::HandleVoidStorageTransfer(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_VOID_STORAGE_TRANSFER");
    Player* player = GetPlayer();

    // Read everything

    ObjectGuid npcGuid;

    uint32 countDeposit = recvData.ReadBits(24);

    std::vector<ObjectGuid> itemGuids(countDeposit);

    for (uint32 i = 0; i < countDeposit; ++i)
        recvData.ReadBitSeq<0, 6, 4, 5, 2, 1, 3, 7>(itemGuids[i]);

    recvData.ReadBitSeq<6, 1, 0>(npcGuid);
    uint32 countWithdraw = recvData.ReadBits(24);
    recvData.ReadBitSeq<7>(npcGuid);

    std::vector<ObjectGuid> itemIds(countWithdraw);

    for (uint32 i = 0; i < countWithdraw; ++i)
        recvData.ReadBitSeq<7, 2, 6, 3, 4, 0, 1, 5>(itemIds[i]);

    recvData.ReadBitSeq<5, 2, 3, 4>(npcGuid);

    if (countWithdraw > 9)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageTransfer - Player (GUID: %u, name: %s) wants to withdraw more than 9 items (%u).",
                     player->GetGUIDLow(), player->GetName().c_str(), countWithdraw);
        return;
    }

    if (countDeposit > 9)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageTransfer - Player (GUID: %u, name: %s) wants to deposit more than 9 items (%u).",
                     player->GetGUIDLow(), player->GetName().c_str(), countDeposit);
        return;
    }

    recvData.ReadByteSeq<2>(npcGuid);

    for (uint32 i = 0; i < countWithdraw; ++i)
        recvData.ReadByteSeq<4, 1, 3, 7, 5, 0, 2, 6>(itemIds[i]);

    for (uint32 i = 0; i < countDeposit; ++i)
        recvData.ReadByteSeq<1, 7, 5, 3, 2, 0, 6, 4>(itemGuids[i]);

    recvData.ReadByteSeq<1, 7, 4, 0, 3, 6, 5>(npcGuid);

    Creature* unit = player->GetNPCIfCanInteractWith(npcGuid, UNIT_NPC_FLAG_VAULTKEEPER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageTransfer - Unit (GUID: %u) not found or player can't interact with it.",
                     GUID_LOPART(npcGuid));
        return;
    }

    if (!player->IsVoidStorageUnlocked())
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageTransfer - Player (GUID: %u, name: %s) queried void storage without unlocking it.",
                     player->GetGUIDLow(), player->GetName().c_str());
        return;
    }

    if (itemGuids.size() > player->GetNumOfVoidStorageFreeSlots())
    {
        SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_FULL);
        return;
    }

    uint32 freeBagSlots = 0;
    if (itemIds.size() != 0)
    {
        // make this a Player function
        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            if (Bag* bag = player->GetBagByPos(i))
                freeBagSlots += bag->GetFreeSlots();
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
            if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                ++freeBagSlots;
    }

    if (itemIds.size() > freeBagSlots)
    {
        SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INVENTORY_FULL);
        return;
    }

    if (!player->HasEnoughMoney(uint64(itemGuids.size() * VOID_STORAGE_STORE_ITEM)))
    {
        SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_NOT_ENOUGH_MONEY);
        return;
    }

    std::pair<VoidStorageItem, uint8> depositItems[VOID_STORAGE_MAX_DEPOSIT];
    uint8 depositCount = 0;
    for (std::vector<ObjectGuid>::iterator itr = itemGuids.begin(); itr != itemGuids.end(); ++itr)
    {
        Item* item = player->GetItemByGuid(*itr);
        if (!item)
        {
            TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageTransfer - Player (GUID: %u, name: %s) wants to deposit an invalid item (item guid: " UI64FMTD ").",
                         player->GetGUIDLow(), player->GetName().c_str(), uint64(*itr));
            continue;
        }

        VoidStorageItem itemVS(sObjectMgr->GenerateVoidStorageItemId(), item->GetEntry(), item->GetUInt64Value(ITEM_FIELD_CREATOR), item->GetItemRandomPropertyId(), item->GetItemSuffixFactor());

        uint8 slot = player->AddVoidStorageItem(itemVS);

        depositItems[depositCount++] = std::make_pair(itemVS, slot);

        player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
    }

    int64 cost = depositCount * VOID_STORAGE_STORE_ITEM;

    player->ModifyMoney(-cost);

    VoidStorageItem withdrawItems[VOID_STORAGE_MAX_WITHDRAW];
    uint8 withdrawCount = 0;
    for (std::vector<ObjectGuid>::iterator itr = itemIds.begin(); itr != itemIds.end(); ++itr)
    {
        uint8 slot;
        VoidStorageItem* itemVS = player->GetVoidStorageItem(*itr, slot);
        if (!itemVS)
        {
            TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageTransfer - Player (GUID: %u, name: %s) tried to withdraw an invalid item (id: " UI64FMTD ")",
                         player->GetGUIDLow(), player->GetName().c_str(), uint64(*itr));
            continue;
        }

        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemVS->ItemEntry, 1);
        if (msg != EQUIP_ERR_OK)
        {
            SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INVENTORY_FULL);
            TC_LOG_DEBUG("network", "WORLD: HandleVoidStorageTransfer - Player (GUID: %u, name: %s) couldn't withdraw"
                         " item id " UI64FMTD " because inventory was full.", player->GetGUIDLow(), player->GetName().c_str(), uint64(*itr));
            return;
        }

        Item* item = player->StoreNewItem(dest, itemVS->ItemEntry, true, itemVS->ItemRandomPropertyId);
        item->SetUInt64Value(ITEM_FIELD_CREATOR, uint64(itemVS->CreatorGuid));
        item->SetBinding(true);
        player->SendNewItem(item, 1, false, false, false);

        withdrawItems[withdrawCount++] = *itemVS;

        player->DeleteVoidStorageItem(slot);
    }

    WorldPacket data(SMSG_VOID_STORAGE_TRANSFER_CHANGES);

    data.WriteBits(withdrawCount, 4);
    data.WriteBits(depositCount, 4);

    for (uint8 i = 0; i < depositCount; ++i)
    {
        ObjectGuid itemId = depositItems[i].first.ItemId;
        ObjectGuid creatorGuid = depositItems[i].first.CreatorGuid;

        data.WriteBitSeq<0>(itemId);
        data.WriteBitSeq<1>(creatorGuid);
        data.WriteBitSeq<3>(itemId);
        data.WriteBitSeq<3>(creatorGuid);
        data.WriteBitSeq<6>(itemId);
        data.WriteBitSeq<4, 5>(creatorGuid);
        data.WriteBitSeq<4, 5>(itemId);
        data.WriteBitSeq<0>(creatorGuid);
        data.WriteBitSeq<2>(itemId);
        data.WriteBitSeq<6, 2>(creatorGuid);
        data.WriteBitSeq<1, 7>(itemId);
        data.WriteBitSeq<7>(creatorGuid);
    }

    for (uint8 i = 0; i < withdrawCount; ++i)
    {
        ObjectGuid itemId = withdrawItems[i].ItemId;
        data.WriteBitSeq<7, 2, 6, 5, 1, 0, 4, 3>(itemId);
    }

    data.FlushBits();

    for (uint8 i = 0; i < withdrawCount; ++i)
    {
        ObjectGuid itemId = withdrawItems[i].ItemId;
        data.WriteByteSeq<6, 1, 5, 4, 3, 7, 2, 0>(itemId);
    }

    for (uint8 i = 0; i < depositCount; ++i)
    {
        ObjectGuid itemId = depositItems[i].first.ItemId;
        ObjectGuid creatorGuid = depositItems[i].first.CreatorGuid;

        data << uint32(depositItems[i].second); // slot
        data.WriteByteSeq<0>(creatorGuid);
        data.WriteByteSeq<7, 5>(itemId);
        data << uint32(depositItems[i].first.ItemRandomPropertyId);
        data.WriteByteSeq<2>(creatorGuid);
        data << uint32(depositItems[i].first.ItemEntry);
        data.WriteByteSeq<3>(itemId);
        data.WriteByteSeq<1, 5, 7, 4>(creatorGuid);
        data.WriteByteSeq<4>(itemId);
        data << uint32(0);                          // @TODO : Item Upgrade level !
        data.WriteByteSeq<2, 6, 0>(itemId);
        data << uint32(depositItems[i].first.ItemSuffixFactor);
        data.WriteByteSeq<6>(creatorGuid);
        data.WriteByteSeq<1>(itemId);
        data.WriteByteSeq<3>(creatorGuid);
    }

    SendPacket(&data);

    SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_NO_ERROR);
}

void WorldSession::HandleVoidSwapItem(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_VOID_SWAP_ITEM");

    Player* player = GetPlayer();
    uint32 newSlot;
    ObjectGuid npcGuid;
    ObjectGuid itemId;

    recvData >> newSlot;

    recvData.ReadBitSeq<2, 7>(itemId);
    recvData.ReadBitSeq<4, 6>(npcGuid);
    recvData.ReadBitSeq<1>(itemId);
    recvData.ReadBitSeq<0>(npcGuid);
    recvData.ReadBitSeq<6>(itemId);
    recvData.ReadBitSeq<1>(npcGuid);
    recvData.ReadBitSeq<0, 4, 3>(itemId);
    recvData.ReadBitSeq<2, 5, 7>(npcGuid);
    recvData.ReadBitSeq<5>(itemId);
    recvData.ReadBitSeq<3>(npcGuid);

    recvData.ReadByteSeq<4, 2, 7>(itemId);
    recvData.ReadByteSeq<2, 6>(npcGuid);
    recvData.ReadByteSeq<6>(itemId);
    recvData.ReadByteSeq<5, 1, 4, 3, 0>(npcGuid);
    recvData.ReadByteSeq<5, 0, 1>(itemId);
    recvData.ReadByteSeq<7>(npcGuid);
    recvData.ReadByteSeq<3>(itemId);

    Creature* unit = player->GetNPCIfCanInteractWith(npcGuid, UNIT_NPC_FLAG_VAULTKEEPER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidSwapItem - Unit (GUID: %u) not found or player can't interact with it.", GUID_LOPART(npcGuid));
        return;
    }

    if (!player->IsVoidStorageUnlocked())
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidSwapItem - Player (GUID: %u, name: %s) queried void storage without unlocking it.",
                     player->GetGUIDLow(), player->GetName().c_str());
        return;
    }

    uint8 oldSlot;
    if (!player->GetVoidStorageItem(itemId, oldSlot))
    {
        TC_LOG_DEBUG("network", "WORLD: HandleVoidSwapItem - Player (GUID: %u, name: %s) requested"
                     " swapping an invalid item (slot: %u, itemid: " UI64FMTD ").",
                     player->GetGUIDLow(), player->GetName().c_str(), newSlot, uint64(itemId));
        return;
    }

    bool usedSrcSlot = player->GetVoidStorageItem(oldSlot) != NULL; // should be always true
    bool usedDestSlot = player->GetVoidStorageItem(newSlot) != NULL;
    ObjectGuid itemIdDest;
    if (usedDestSlot)
        itemIdDest = player->GetVoidStorageItem(newSlot)->ItemId;

    if (!player->SwapVoidStorageItem(oldSlot, newSlot))
    {
        SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INTERNAL_ERROR_1);
        return;
    }

    WorldPacket data(SMSG_VOID_ITEM_SWAP_RESPONSE);

    data.WriteBit(!usedSrcSlot);

    data.WriteBitSeq<6, 1, 2, 4, 0, 5, 7, 3>(itemId);

    data.WriteBit(!usedDestSlot);

    data.WriteBitSeq<5, 1, 6, 4, 0, 2, 7, 3>(itemIdDest);

    data.WriteBit(!usedDestSlot);
    data.WriteBit(!usedSrcSlot);

    data.WriteByteSeq<3, 0, 2, 5, 4, 6, 7, 1>(itemIdDest);
    data.WriteByteSeq<2, 0, 7, 5, 3, 6, 4, 1>(itemId);

    if (usedDestSlot)
        data << uint32(oldSlot);
    if (usedSrcSlot)
        data << uint32(newSlot);

    SendPacket(&data);
}
