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
#include "WorldPacket.h"
#include "Log.h"
#include "Corpse.h"
#include "GameObject.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "WorldSession.h"
#include "LootMgr.h"
#include "Object.h"
#include "Group.h"
#include "World.h"
#include "Util.h"
#include "GuildMgr.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

void WorldSession::HandleAutostoreLootItemOpcode(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_AUTOSTORE_LOOT_ITEM");

    Player* player = GetPlayer();
    uint64 lguid = player->GetLootGUID();
    Loot* loot = NULL;
    uint8 lootSlot = 0;

    std::vector<ObjectGuid> guids(recvData.ReadBits(23));

    for (auto &objGuid : guids)
        recvData.ReadBitSeq<3, 7, 2, 4, 0, 5, 6, 1>(objGuid);

    for (auto &objGuid : guids)
    {
        recvData.ReadByteSeq<4, 7, 6, 5>(objGuid);
        recvData >> lootSlot;
        recvData.ReadByteSeq<3, 1, 0, 2>(objGuid);

        if (GUID_LOPART(objGuid) != GUID_LOPART(lguid))
            return;

        if (IS_GAMEOBJECT_GUID(lguid))
        {
            GameObject* go = player->GetMap()->GetGameObject(lguid);

            // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
            if (!go || ((go->GetOwnerGUID() != _player->GetGUID() && go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE) && !go->IsWithinDistInMap(_player, INTERACTION_DISTANCE)))
            {
                player->SendLootRelease(lguid);
                return;
            }

            loot = &go->loot;

            // use personal loot in LFR
            if (player->GetMap()->GetDifficulty() == RAID_TOOL_DIFFICULTY)
                loot = &go->m_lfrLoot[player->GetGUID()];
        }
        else if (IS_ITEM_GUID(lguid))
        {
            Item* pItem = player->GetItemByGuid(lguid);

            if (!pItem)
            {
                player->SendLootRelease(lguid);
                return;
            }

            loot = &pItem->loot;
        }
        else if (IS_CORPSE_GUID(lguid))
        {
            Corpse* bones = ObjectAccessor::GetCorpse(*player, lguid);
            if (!bones)
            {
                player->SendLootRelease(lguid);
                return;
            }

            loot = &bones->loot;
        }
        else
        {
            Creature* creature = GetPlayer()->GetMap()->GetCreature(lguid);

            bool lootAllowed = creature && creature->IsAlive() == (player->getClass() == CLASS_ROGUE && creature->lootForPickPocketed);

            if (!lootAllowed || (!creature->IsWithinDistInMap(_player, INTERACTION_DISTANCE) && !_player->HasSpell(125048)))
            {
                player->SendLootRelease(lguid);
                return;
            }

            loot = &creature->loot;

            // use personal loot in LFR
            if (player->GetMap()->GetDifficulty() == RAID_TOOL_DIFFICULTY)
                loot = &creature->m_lfrLoot[player->GetGUID()];
        }

        player->StoreLootItem(lootSlot, loot);

        // If player is removing the last LootItem, delete the empty container.
        if (loot->isLooted() && IS_ITEM_GUID(lguid))
            player->GetSession()->DoLootRelease(lguid);
    }
}

void WorldSession::HandleLootMoneyOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_LOOT_MONEY");

    Player* player = GetPlayer();
    uint64 guid = player->GetLootGUID();
    if (!guid)
        return;

    Loot* loot = NULL;
    MoneyLootFlags lootFlags = 0;

    switch (GUID_HIPART(guid))
    {
        case HIGHGUID_GAMEOBJECT:
        {
            GameObject* go = GetPlayer()->GetMap()->GetGameObject(guid);

            // do not check distance for GO if player is the owner of it (ex. fishing bobber)
            if (go && ((go->GetOwnerGUID() == player->GetGUID() || go->IsWithinDistInMap(player, INTERACTION_DISTANCE))))
            {
                loot = &go->loot;

                // use personal loot in LFR
                if (player->GetMap()->GetDifficulty() == RAID_TOOL_DIFFICULTY)
                    loot = &go->m_lfrLoot[player->GetGUID()];
            }

            break;
        }
        case HIGHGUID_CORPSE:                               // remove insignia ONLY in BG
        {
            Corpse* bones = ObjectAccessor::GetCorpse(*player, guid);

            if (bones && bones->IsWithinDistInMap(player, INTERACTION_DISTANCE))
            {
                loot = &bones->loot;
                lootFlags |= MONEY_LOOT_FLAG_FROM_ENEMY;
            }

            break;
        }
        case HIGHGUID_ITEM:
        {
            if (Item* item = player->GetItemByGuid(guid))
            {
                loot = &item->loot;
            }
            break;
        }
        case HIGHGUID_UNIT:
        case HIGHGUID_VEHICLE:
        {
            Creature* creature = player->GetMap()->GetCreature(guid);
            bool lootAllowed = creature && creature->IsAlive() == (player->getClass() == CLASS_ROGUE && creature->lootForPickPocketed);
            if (lootAllowed && creature->IsWithinDistInMap(player, INTERACTION_DISTANCE))
            {
                loot = &creature->loot;

                // use personal loot in LFR
                if (player->GetMap()->GetDifficulty() == RAID_TOOL_DIFFICULTY)
                    loot = &creature->m_lfrLoot[player->GetGUID()];

                lootFlags |= MONEY_LOOT_FLAG_FROM_ENEMY;
            }
            break;
        }
        default:
            return;                                         // unlootable type
    }

    if (loot)
    {
        if (!IS_ITEM_GUID(guid) && player->GetGroup() && player->GetMap()->GetDifficulty() != RAID_TOOL_DIFFICULTY)      //item can be looted only single player
            player->LootMoneyInGroup(loot->gold, lootFlags);
        else
            player->LootMoney(loot->gold, lootFlags | MONEY_LOOT_FLAG_SOLO);

        loot->gold = 0;
        loot->NotifyMoneyRemoved();

        // Delete the money loot record from the DB
        if (loot->containerID > 0)
            loot->DeleteLootMoneyFromContainerItemDB();

        // Delete container if empty
        if (loot->isLooted() && IS_ITEM_GUID(guid))
            player->GetSession()->DoLootRelease(guid);
    }
}

void WorldSession::HandleLootOpcode(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_LOOT");

    ObjectGuid guid;

    recvData.ReadBitSeq<0, 6, 7, 3, 5, 1, 2, 4>(guid);
    recvData.ReadByteSeq<5, 1, 2, 6, 0, 7, 3, 4>(guid);

    // Check possible cheat
    if (!_player->IsAlive())
        return;

    GetPlayer()->SendLoot(guid, LOOT_CORPSE);

    // interrupt cast
    if (GetPlayer()->IsNonMeleeSpellCasted(false))
        GetPlayer()->InterruptNonMeleeSpells(false);
}

void WorldSession::HandleLootReleaseOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_LOOT_RELEASE");

    // cheaters can modify lguid to prevent correct apply loot release code and re-loot
    // use internal stored guid
    ObjectGuid guid;

    recvData.ReadBitSeq<3, 6, 1, 0, 7, 2, 5, 4>(guid);
    recvData.ReadByteSeq<0, 6, 4, 7, 1, 2, 5, 3>(guid);

    if (uint64 lguid = GetPlayer()->GetLootGUID())
        if (lguid == guid)
            DoLootRelease(lguid);
}

void WorldSession::DoLootRelease(uint64 lguid)
{
    Player *player = GetPlayer();
    Loot *loot;

    player->SetLootGUID(0);
    player->SendLootRelease(lguid);

    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);

    if (!player->IsInWorld())
        return;

    if (IS_GAMEOBJECT_GUID(lguid))
    {
        GameObject* go = GetPlayer()->GetMap()->GetGameObject(lguid);

        // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
        if (!go || ((go->GetOwnerGUID() != _player->GetGUID() && go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE) && !go->IsWithinDistInMap(_player, INTERACTION_DISTANCE)))
            return;

        loot = &go->loot;

        // use personal loot for LFR
        if (player->GetMap()->GetDifficulty() == RAID_TOOL_DIFFICULTY)
            loot = &go->m_lfrLoot[player->GetGUID()];

        if (go->GetGoType() == GAMEOBJECT_TYPE_DOOR)
        {
            // locked doors are opened with spelleffect openlock, prevent remove its as looted
            go->UseDoorOrButton();
        }
        else if (loot->isLooted() || go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
        {
            if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGHOLE)
            {                                               // The fishing hole used once more
                go->AddUse();                               // if the max usage is reached, will be despawned in next tick
                if (go->GetUseCount() >= go->GetGOValue().FishingHole.MaxOpens)
                    go->SetLootState(GO_JUST_DEACTIVATED);
                else
                    go->SetLootState(GO_READY);
            }
            else
            {
                // check if all players have looted their personal loot before despawning
                if (player->GetMap()->GetDifficulty() == RAID_TOOL_DIFFICULTY)
                {
                    if (Group* group = player->GetGroup())
                    {
                        uint8 lootedCounter = 0;
                        for (auto &memberSlot : group->GetMemberSlots())
                        {
                            Loot* memberLoot = &go->m_lfrLoot[memberSlot.guid];
                            if (memberLoot->isLooted())
                                lootedCounter++;
                        }

                        // if everyone has remove their loot, despawn
                        if (lootedCounter == group->GetMembersCount())
                            go->SetLootState(GO_JUST_DEACTIVATED);
                    }
                }
                else
                    go->SetLootState(GO_JUST_DEACTIVATED);
            }

            loot->clear();
        }
        else
        {
            // not fully looted object
            go->SetLootState(GO_ACTIVATED, player);

            // if the round robin player release, reset it.
            if (player->GetGUID() == loot->roundRobinPlayer)
            {
                if (Group* group = player->GetGroup())
                {
                    if (group->GetLootMethod() != MASTER_LOOT)
                        loot->roundRobinPlayer = 0;
                }
                else
                    loot->roundRobinPlayer = 0;
            }
        }
    }
    else if (IS_CORPSE_GUID(lguid))        // ONLY remove insignia at BG
    {
        Corpse* corpse = ObjectAccessor::GetCorpse(*player, lguid);
        if (!corpse || !corpse->IsWithinDistInMap(_player, INTERACTION_DISTANCE))
            return;

        loot = &corpse->loot;

        if (loot->isLooted())
        {
            loot->clear();
            corpse->RemoveFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);
        }
    }
    else if (IS_ITEM_GUID(lguid))
    {
        Item* pItem = player->GetItemByGuid(lguid);
        if (!pItem)
            return;

        ItemTemplate const* proto = pItem->GetTemplate();

        // destroy only 5 items from stack in case prospecting and milling
        if (proto->Flags & (ITEM_PROTO_FLAG_PROSPECTABLE | ITEM_PROTO_FLAG_MILLABLE))
        {
            pItem->m_lootGenerated = false;
            pItem->loot.clear();

            uint32 count = pItem->GetCount();

            // >=5 checked in spell code, but will work for cheating cases also with removing from another stacks.
            if (count > 5)
                count = 5;

            player->DestroyItemCount(pItem, count, true);
        }
        else
        {
            if (pItem->loot.isLooted()) // Only delete item if no loot or money (unlooted loot is saved to db)
                player->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
        }
        return;                                             // item can be looted only single player
    }
    else
    {
        Creature* creature = GetPlayer()->GetMap()->GetCreature(lguid);

        bool lootAllowed = creature && creature->IsAlive() == (player->getClass() == CLASS_ROGUE && creature->lootForPickPocketed);
        if (!lootAllowed || !creature->IsWithinDistInMap(_player, INTERACTION_DISTANCE))
            return;

        loot = &creature->loot;

        // use personal loot for LFR
        if (player->GetMap()->GetDifficulty() == RAID_TOOL_DIFFICULTY)
            loot = &creature->m_lfrLoot[player->GetGUID()];

        if (loot->isLooted())
        {
            // skip pickpocketing loot for speed, skinning timer reduction is no-op in fact
            if (!creature->IsAlive())
                creature->AllLootRemovedFromCorpse();

            // check if all players have looted their personal loot before removing loot flag
            if (player->GetMap()->GetDifficulty() == RAID_TOOL_DIFFICULTY)
            {
                if (Group* group = player->GetGroup())
                {
                    uint8 lootedCounter = 0;
                    for (auto &memberSlot : group->GetMemberSlots())
                    {
                        Loot* memberLoot = &creature->m_lfrLoot[memberSlot.guid];
                        if (memberLoot->isLooted())
                            lootedCounter++;
                    }

                    // if everyone has remove their loot, remove flag
                    if (lootedCounter == group->GetMembersCount())
                        creature->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                }
            }
            else
                creature->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            loot->clear();
        }
        else
        {
            // if the round robin player release, reset it.
            if (player->GetGUID() == loot->roundRobinPlayer)
            {
                if (Group* group = player->GetGroup())
                {
                    if (group->GetLootMethod() != MASTER_LOOT)
                    {
                        loot->roundRobinPlayer = 0;
                        group->SendLooter(creature, NULL);

                        // force update of dynamic flags, otherwise other group's players still not able to loot.
                        creature->ForceValuesUpdateAtIndex(OBJECT_FIELD_DYNAMIC_FLAGS);
                    }
                }
                else
                    loot->roundRobinPlayer = 0;
            }
        }
    }

    //Player is not looking at loot list, he doesn't need to see updates on the loot list
    loot->RemoveLooter(player->GetGUID());
}

void WorldSession::HandleLootMasterAskForRoll(WorldPacket& recvData)
{
    ObjectGuid guid = 0;
    uint8 slot = 0;

    recvData >> slot;

    recvData.ReadBitSeq<6, 0, 4, 3, 2, 7, 1, 5>(guid);
    recvData.ReadByteSeq<2, 0, 7, 5, 3, 6, 1, 4>(guid);

    if (!_player->GetGroup() || _player->GetGroup()->GetLooterGuid() != _player->GetGUID())
    {
        _player->SendLootRelease(GetPlayer()->GetLootGUID());
        return;
    }

    Loot* loot = NULL;

    if (IS_CRE_OR_VEH_GUID(guid))
    {
        Creature* creature = GetPlayer()->GetMap()->GetCreature(guid);
        if (!creature)
            return;

        loot = &creature->loot;
    }
    else if (IS_GAMEOBJECT_GUID(guid))
    {
        GameObject* pGO = GetPlayer()->GetMap()->GetGameObject(guid);
        if (!pGO)
            return;

        loot = &pGO->loot;
    }

    if (!loot || loot->alreadyAskedForRoll)
        return;

    if (slot >= loot->items.size() + loot->quest_items.size())
    {
        TC_LOG_DEBUG("loot", "MasterLootItem: Player %s might be using a hack! (slot %d, size %lu)",
                     GetPlayer()->GetName().c_str(), slot, (unsigned long)loot->items.size());
        return;
    }

    LootItem& item = slot >= loot->items.size() ? loot->quest_items[slot - loot->items.size()] : loot->items[slot];

    loot->alreadyAskedForRoll = true;

    _player->GetGroup()->DoRollForAllMembers(guid, slot, _player->GetMapId(), loot, item, _player);
}

void WorldSession::HandleLootMasterGiveOpcode(WorldPacket& recvData)
{
    ObjectGuid targetPlayerGuid = 0;

    recvData.ReadBitSeq<6, 2, 5, 7, 3, 4, 0>(targetPlayerGuid);

    uint32 count = recvData.ReadBits(23);
    // As long as AoE loot is not implemented, count must be equal to 1.
    if (count > 1)
        return;

    std::vector<ObjectGuid> guids(count);
    std::vector<uint8> types(count);

    for (uint32 i = 0; i < count; ++i)
        recvData.ReadBitSeq<1, 4, 3, 6, 0, 2, 7, 5>(guids[i]);

    recvData.ReadBitSeq<1>(targetPlayerGuid);

    recvData.ReadByteSeq<5, 6>(targetPlayerGuid);

    for (uint32 i = 0; i < count; ++i)
    {
        recvData >> types[i];
        recvData.ReadByteSeq<3, 5, 0, 6, 2, 1, 4, 7>(guids[i]);
    }

    recvData.ReadByteSeq<3, 2, 0, 4, 1, 7>(targetPlayerGuid);

    if (!_player->GetGroup() || _player->GetGroup()->GetLooterGuid() != _player->GetGUID())
    {
        _player->SendLootRelease(GetPlayer()->GetLootGUID());
        return;
    }

    Player* target = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(targetPlayerGuid, 0, HIGHGUID_PLAYER));
    if (!target)
        return;

    TC_LOG_DEBUG("network", "WorldSession::HandleLootMasterGiveOpcode (CMSG_LOOT_MASTER_GIVE, 0x02A3) Target = [%s].", target->GetName().c_str());

    uint64 const lguid = GetPlayer()->GetLootGUID();

    for (uint32 i = 0; i < count; ++i)
    {
        // ObjectGuid lootguid = guids[i];
        uint8 slotid = types[i];
        Loot* loot = NULL;

        if (GUID_LOPART(guids[i]) != GUID_LOPART(lguid))
            continue;

        if (IS_CRE_OR_VEH_GUID(lguid))
        {
            Creature* creature = GetPlayer()->GetMap()->GetCreature(lguid);
            if (!creature)
                return;

            loot = &creature->loot;
        }
        else if (IS_GAMEOBJECT_GUID(lguid))
        {
            GameObject* pGO = GetPlayer()->GetMap()->GetGameObject(lguid);
            if (!pGO)
                return;

            loot = &pGO->loot;
        }

        if (!loot)
            return;

        if (slotid >= loot->items.size() + loot->quest_items.size())
        {
            TC_LOG_DEBUG("loot", "MasterLootItem: Player %s might be using a hack! (slot %d, size %lu)",
                         GetPlayer()->GetName().c_str(), slotid, (unsigned long)loot->items.size());
            return;
        }

        LootItem& item = slotid >= loot->items.size() ? loot->quest_items[slotid - loot->items.size()] : loot->items[slotid];

        ItemPosCountVec dest;
        InventoryResult msg = target->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item.itemid, item.count);
        if (item.follow_loot_rules && !item.AllowedForPlayer(target))
            msg = EQUIP_ERR_CANT_EQUIP_EVER;
        if (msg != EQUIP_ERR_OK)
        {
            target->SendEquipError(msg, NULL, NULL, item.itemid);
            // send duplicate of error massage to master looter
            _player->SendEquipError(msg, NULL, NULL, item.itemid);
            return;
        }

        // list of players allowed to receive this item in trade
        AllowedLooterSet looters = item.GetAllowedLooters();

        // not move item from loot to target inventory
        Item* newitem = target->StoreNewItem(dest, item.itemid, true, item.randomPropertyId, looters);
        target->SendNewItem(newitem, uint32(item.count), false, false, true);
        target->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, item.itemid, item.count);
        target->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE, loot->loot_type, item.count);
        target->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM, item.itemid, item.count);

        // mark as looted
        item.count=0;
        item.is_looted=true;

        loot->NotifyItemRemoved(slotid);
        --loot->unlootedCount;
    }
}
