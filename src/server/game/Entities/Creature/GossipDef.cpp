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

#include "QuestDef.h"
#include "GossipDef.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Formulas.h"

GossipMenu::GossipMenu()
{
    _menuId = 0;
}

GossipMenu::~GossipMenu()
{
    ClearMenu();
}

void GossipMenu::AddMenuItem(int32 menuItemId, uint8 icon, std::string const& message, uint32 sender, uint32 action, std::string const& boxMessage, uint32 boxMoney, bool coded /*= false*/)
{
    ASSERT(_menuItems.size() <= GOSSIP_MAX_MENU_ITEMS);

    // Find a free new id - script case
    if (menuItemId == -1)
    {
        menuItemId = 0;
        if (!_menuItems.empty())
        {
            for (GossipMenuItemContainer::const_iterator itr = _menuItems.begin(); itr != _menuItems.end(); ++itr)
            {
                if (int32(itr->first) > menuItemId)
                    break;

                menuItemId = itr->first + 1;
            }
        }
    }

    GossipMenuItem& menuItem = _menuItems[menuItemId];

    menuItem.MenuItemIcon    = icon;
    menuItem.Message         = message;
    menuItem.IsCoded         = coded;
    menuItem.Sender          = sender;
    menuItem.OptionType      = action;
    menuItem.BoxMessage      = boxMessage;
    menuItem.BoxMoney        = boxMoney;
}

/**
 * @name AddMenuItem
 * @brief Adds a localized gossip menu item from db by menu id and menu item id.
 * @param menuId Gossip menu id.
 * @param menuItemId Gossip menu item id.
 * @param sender Identifier of the current menu.
 * @param action Custom action given to OnGossipHello.
 */
void GossipMenu::AddMenuItem(uint32 menuId, uint32 menuItemId, uint32 sender, uint32 action)
{
    /// Find items for given menu id.
    GossipMenuItemsMapBounds bounds = sObjectMgr->GetGossipMenuItemsMapBounds(menuId);
    /// Return if there are none.
    if (bounds.first == bounds.second)
        return;

    /// Iterate over each of them.
    for (GossipMenuItemsContainer::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        /// Find the one with the given menu item id.
        if (itr->second.OptionIndex != menuItemId)
            continue;

        /// Store texts for localization.
        std::string strOptionText = itr->second.OptionText;
        std::string strBoxText = itr->second.BoxText;

        /// Check need of localization.
        if (GetLocale() > LOCALE_enUS)
            /// Find localizations from database.
            if (GossipMenuItemsLocale const* no = sObjectMgr->GetGossipMenuItemsLocale(MAKE_PAIR32(menuId, menuItemId)))
            {
                /// Translate texts if there are any.
                ObjectMgr::GetLocaleString(no->OptionText, GetLocale(), strOptionText);
                ObjectMgr::GetLocaleString(no->BoxText, GetLocale(), strBoxText);
            }

        /// Add menu item with existing method. Menu item id -1 is also used in ADD_GOSSIP_ITEM macro.
        AddMenuItem(-1, itr->second.OptionIcon, strOptionText, sender, action, strBoxText, itr->second.BoxMoney, itr->second.BoxCoded);
    }
}

void GossipMenu::AddGossipMenuItemData(uint32 menuItemId, uint32 gossipActionMenuId, uint32 gossipActionPoi)
{
    GossipMenuItemData& itemData = _menuItemData[menuItemId];

    itemData.GossipActionMenuId  = gossipActionMenuId;
    itemData.GossipActionPoi     = gossipActionPoi;
}

uint32 GossipMenu::GetMenuItemSender(uint32 menuItemId) const
{
    GossipMenuItemContainer::const_iterator itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return 0;

    return itr->second.Sender;
}

uint32 GossipMenu::GetMenuItemAction(uint32 menuItemId) const
{
    GossipMenuItemContainer::const_iterator itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return 0;

    return itr->second.OptionType;
}

bool GossipMenu::IsMenuItemCoded(uint32 menuItemId) const
{
    GossipMenuItemContainer::const_iterator itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return false;

    return itr->second.IsCoded;
}

void GossipMenu::ClearMenu()
{
    _menuItems.clear();
    _menuItemData.clear();
}

PlayerMenu::PlayerMenu(WorldSession* session) : _session(session)
{
    if (_session)
        _gossipMenu.SetLocale(_session->GetSessionDbLocaleIndex());
}

PlayerMenu::~PlayerMenu()
{
    ClearMenus();
}

void PlayerMenu::ClearMenus()
{
    _gossipMenu.ClearMenu();
    _questMenu.ClearMenu();
}

#define DEFAULT_GREETINGS_GOSSIP      68

void PlayerMenu::SendGossipMenu(uint32 titleTextId, uint64 objectGUID) const
{
    ObjectGuid guid = objectGUID;

    WorldPacket data(SMSG_GOSSIP_MESSAGE, 100);         // guess size

    if (titleTextId == DEFAULT_GOSSIP_MESSAGE && !_gossipMenu.GetMenuId())
        data << uint32(DEFAULT_GREETINGS_GOSSIP);           // default greeting ID
    else
        data << uint32(_gossipMenu.GetMenuId());            // new 2.4.0

    data << uint32(0);                                  // Friendship faction
    data << uint32(titleTextId);

    data.WriteBitSeq<0, 1>(guid);
    data.WriteBits(_gossipMenu.GetMenuItems().size(), 20);
    data.WriteBitSeq<6, 7>(guid);

    for (GossipMenuItemContainer::const_iterator itr = _gossipMenu.GetMenuItems().begin(); itr != _gossipMenu.GetMenuItems().end(); ++itr)
    {
        GossipMenuItem const& item = itr->second;
        data.WriteBits(item.Message.size(), 12);
        data.WriteBits(item.BoxMessage.size(), 12);
    }

    data.WriteBitSeq<4, 3, 2>(guid);
    data.WriteBits(_questMenu.GetMenuItemCount(), 19);

    for (uint32 iI = 0; iI < _questMenu.GetMenuItemCount(); ++iI)
    {
        QuestMenuItem const& item = _questMenu.GetItem(iI);
        uint32 questID = item.QuestId;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questID);
        std::string title = quest->GetTitle();

        int locale = _session->GetSessionDbLocaleIndex();
        if (locale >= 0)
            if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(questID))
                ObjectMgr::GetLocaleString(localeData->Title, locale, title);

        data.WriteBits(title.size(), 9);
        data.WriteBit(quest->IsRepeatable());                   // 3.3.3 changes icon: blue question or yellow exclamation, is repeatable
    }

    data.WriteBitSeq<5>(guid);

    for (uint32 iI = 0; iI < _questMenu.GetMenuItemCount(); ++iI)
    {
        QuestMenuItem const& item = _questMenu.GetItem(iI);
        uint32 questID = item.QuestId;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questID);
        std::string title = quest->GetTitle();

        int locale = _session->GetSessionDbLocaleIndex();
        if (locale >= 0)
            if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(questID))
                ObjectMgr::GetLocaleString(localeData->Title, locale, title);

        Player* plr = _session->GetPlayer();

        uint32 questStat = plr ? plr->GetQuestStatus(questID) : 0;

        if (questStat == QUEST_STATUS_COMPLETE || questStat == QUEST_STATUS_INCOMPLETE)
        {
            if (quest->IsRepeatable())
                questStat = 0;
            else
                questStat = 4;
        }
        else if (questStat == QUEST_STATE_NONE)
            questStat = 2;

        data << uint32(0);                              // quest flags 2
        data << uint32(questStat);                      // quest icon
        if (title.size() > 0)
            data.append(title.c_str(), title.size());       // quest title
        data << uint32(quest->GetFlags());              // quest flags
        data << int32(quest->GetQuestLevel());          // quest level
        data << uint32(questID);
    }

    for (GossipMenuItemContainer::const_iterator itr = _gossipMenu.GetMenuItems().begin(); itr != _gossipMenu.GetMenuItems().end(); ++itr)
    {
        GossipMenuItem const& item = itr->second;
        data << uint32(item.BoxMoney);                                  // money required to open menu, 2.0.3
        data << uint32(itr->first);
        if (item.BoxMessage.size() > 0)
            data.append(item.BoxMessage.c_str(), item.BoxMessage.size());
        data << uint8(item.IsCoded);                                    // makes pop up box password
        if (item.Message.size() > 0)
            data.append(item.Message.c_str(), item.Message.size());
        data << uint8(item.MenuItemIcon);
    }

    data.WriteByteSeq<3, 4, 7, 2, 1, 6, 0, 5>(guid);

    _session->SendPacket(&data);
}

void PlayerMenu::SendCloseGossip() const
{
    WorldPacket data(SMSG_GOSSIP_COMPLETE, 0);
    _session->SendPacket(&data);
}

void PlayerMenu::SendPointOfInterest(uint32 poiId) const
{
    PointOfInterest const* poi = sObjectMgr->GetPointOfInterest(poiId);
    if (!poi)
    {
        TC_LOG_ERROR("sql.sql", "Request to send non-existing POI (Id: %u), ignored.", poiId);
        return;
    }

    std::string iconText = poi->icon_name;
    int32 locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
        if (PointOfInterestLocale const* localeData = sObjectMgr->GetPointOfInterestLocale(poiId))
            ObjectMgr::GetLocaleString(localeData->IconName, locale, iconText);

    WorldPacket data(SMSG_GOSSIP_POI, 4 + 4 + 4 + 4 + 4 + 10);  // guess size
    data << uint32(poi->flags);
    data << float(poi->x);
    data << float(poi->y);
    data << uint32(poi->icon);
    data << uint32(poi->data);
    data << iconText;

    _session->SendPacket(&data);
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

QuestMenu::QuestMenu()
{
    _questMenuItems.reserve(16);                                   // can be set for max from most often sizes to speedup push_back and less memory use
}

QuestMenu::~QuestMenu()
{
    ClearMenu();
}

void QuestMenu::AddMenuItem(uint32 QuestId, uint8 Icon)
{
    if (!sObjectMgr->GetQuestTemplate(QuestId))
        return;

    ASSERT(_questMenuItems.size() <= GOSSIP_MAX_MENU_ITEMS);

    QuestMenuItem questMenuItem;

    questMenuItem.QuestId        = QuestId;
    questMenuItem.QuestIcon      = Icon;

    _questMenuItems.push_back(questMenuItem);
}

bool QuestMenu::HasItem(uint32 questId) const
{
    for (QuestMenuItemList::const_iterator i = _questMenuItems.begin(); i != _questMenuItems.end(); ++i)
        if (i->QuestId == questId)
            return true;

    return false;
}

void QuestMenu::ClearMenu()
{
    _questMenuItems.clear();
}

void PlayerMenu::SendQuestGiverQuestList(QEmote eEmote, const std::string& Title, uint64 npcGUID)
{
    ObjectGuid guid = npcGUID;
    ByteBuffer byteData;

    WorldPacket data(SMSG_QUESTGIVER_QUEST_LIST, 100);    // guess size
    data.WriteBits(Title.size(), 11);
    data.WriteBitSeq<2, 7>(guid);
    data.WriteBits(_questMenu.GetMenuItemCount(), 19);

    uint32 count = 0;
    for (; count < _questMenu.GetMenuItemCount(); ++count)
    {
        QuestMenuItem const& qmi = _questMenu.GetItem(count);

        uint32 questID = qmi.QuestId;

        if (Quest const* quest = sObjectMgr->GetQuestTemplate(questID))
        {
            Player* plr = _session->GetPlayer();
            std::string title = quest->GetTitle();

            int loc_idx = _session->GetSessionDbLocaleIndex();
            if (loc_idx >= 0)
                if (QuestLocale const* ql = sObjectMgr->GetQuestLocale(questID))
                    ObjectMgr::GetLocaleString(ql->Title, loc_idx, title);

            uint32 questStat = plr ? plr->GetQuestStatus(questID) : 0;

            if (questStat == QUEST_STATUS_COMPLETE || questStat == QUEST_STATUS_INCOMPLETE)
                questStat = quest->IsRepeatable() ? 0 : 4;
            else if (questStat == QUEST_STATE_NONE)
                questStat = 2;

            data.WriteBits(title.size(), 9);
            data.WriteBit(quest->IsRepeatable());

            byteData << uint32(questStat);                      // quest icon
            byteData << uint32(quest->GetFlags());              // quest flags
            byteData << uint32(questID);
            byteData << uint32(0);                              // quest flags 2
            byteData << int32(quest->GetQuestLevel());          // quest level
            byteData.WriteString(title);                        // quest title
        }
    }

    data.WriteBitSeq<5, 6, 3, 1, 0, 4>(guid);
    data.FlushBits();

    data.append(byteData);
    data << uint32(eEmote._Delay);                         // player emote
    data.WriteByteSeq<5, 7, 2, 6, 1>(guid);
    data.WriteString(Title);
    data.WriteByteSeq<3, 4>(guid);
    data << uint32(eEmote._Emote);                         // NPC emote
    data.WriteByteSeq<0>(guid);

    _session->SendPacket(&data);
    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_QUESTGIVER_QUEST_LIST NPC Guid=%u", GUID_LOPART(npcGUID));
}

void PlayerMenu::SendQuestGiverStatus(uint32 questStatus, uint64 npcGUID) const
{
    ObjectGuid guid = npcGUID;
    WorldPacket data(SMSG_QUESTGIVER_STATUS);
    data.WriteBitSeq<6, 5, 0, 1, 2, 4, 3, 7>(guid);
    data.WriteByteSeq<3>(guid);
    data << uint32(questStatus);
    data.WriteByteSeq<4, 5, 2, 1, 7, 0, 6>(guid);

    _session->SendPacket(&data);
    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_QUESTGIVER_STATUS NPC Guid=%u, status=%u", GUID_LOPART(npcGUID), questStatus);
}

void PlayerMenu::SendQuestGiverQuestDetails(Quest const* quest, uint64 npcGUID, bool /*activateAccept*/) const
{
    std::string questTitle           = quest->GetTitle();
    std::string questDetails         = quest->GetDetails();
    std::string questObjectives      = quest->GetObjectives();
    std::string questEndText         = quest->GetEndText();
    std::string questGiverTextWindow = quest->GetQuestGiverTextWindow();
    std::string questGiverTargetName = quest->GetQuestGiverTargetName();
    std::string questTurnTextWindow  = quest->GetQuestTurnTextWindow();
    std::string questTurnTargetName  = quest->GetQuestTurnTargetName();

    int32 locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
    {
        if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(quest->GetQuestId()))
        {
            ObjectMgr::GetLocaleString(localeData->Title, locale, questTitle);
            ObjectMgr::GetLocaleString(localeData->Details, locale, questDetails);
            ObjectMgr::GetLocaleString(localeData->Objectives, locale, questObjectives);
            ObjectMgr::GetLocaleString(localeData->EndText, locale, questEndText);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTextWindow, locale, questGiverTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTargetName, locale, questGiverTargetName);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTextWindow, locale, questTurnTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTargetName, locale, questTurnTargetName);
        }
    }

    uint32 rewItemDisplayId[QUEST_REWARDS_COUNT];
    uint32 rewChoiceItemDisplayId[QUEST_REWARD_CHOICES_COUNT];

    for (uint8 i = 0; i < QUEST_REWARDS_COUNT; i++)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardItemId[i]))
            rewItemDisplayId[i] = itemTemplate->DisplayInfoID;
        else
            rewItemDisplayId[i] = 0;
    }

    for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; i++)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[i]))
            rewChoiceItemDisplayId[i] = itemTemplate->DisplayInfoID;
        else
            rewChoiceItemDisplayId[i] = 0;
    }

    ObjectGuid guid = npcGUID;
    ObjectGuid guid2 = npcGUID;

    WorldPacket data(SMSG_QUESTGIVER_QUEST_DETAILS);
    data << uint32(quest->GetRewardReputationMask());
    data << uint32(quest->GetRewardSkillPoints());

    for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
    {
        data << uint32(quest->RewardFactionId[i]);
        data << uint32(quest->RewardFactionValueIdOverride[i]);
        data << uint32(quest->RewardFactionValueId[i]);
    }

    data << uint32(quest->RewardChoiceItemCount[3]);
    data << uint32(quest->GetSuggestedPlayers());
    data << uint32(quest->GetRewChoiceItemsCount());
    data << uint32(quest->RewardItemIdCount[3]);
    data << uint32(quest->RewardItemIdCount[2]);
    data << uint32(quest->XPValue(_session->GetPlayer()) * sWorld->getRate(RATE_XP_QUEST));
    data << uint32(rewChoiceItemDisplayId[3]);
    data << uint32(quest->RewardItemIdCount[1]);
    data << uint32(quest->RewardChoiceItemCount[4]);
    data << uint32(quest->GetQuestId());
    data << uint32(quest->RewardChoiceItemId[3]);
    data << uint32(quest->RewardChoiceItemId[2]);
    data << uint32(quest->RewardItemId[1]);
    data << uint32(rewChoiceItemDisplayId[1]);
    data << uint32(quest->GetQuestTurnInPortrait());
    data << uint32(quest->GetRewSpellCast());
    data << uint32(quest->RewardChoiceItemCount[1]);
    data << uint32(quest->GetQuestGiverPortrait());
    data << uint32(rewChoiceItemDisplayId[2]);
    data << uint32(quest->GetRewItemsCount());
    data << uint32(rewItemDisplayId[1]);
    data << uint32(quest->GetRewMoneyMaxLevel());
    data << uint32(quest->GetFlags());
    data << uint32(quest->GetRewSpell());
    data << uint32(quest->RewardChoiceItemId[0]);

    for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
    {
        data << uint32(quest->RewardCurrencyId[i]);
        data << uint32(quest->RewardCurrencyCount[i]);
    }

    data << uint32(quest->GetRewardSkillId());
    data << uint32(quest->RewardChoiceItemId[5]);
    data << uint32(quest->RewardChoiceItemId[1]);
    data << uint32(quest->RewardChoiceItemCount[0]);
    data << uint32(quest->RewardChoiceItemCount[4]);
    data << uint32(quest->RewardChoiceItemCount[2]);
    data << uint32(rewItemDisplayId[0]);
    data << uint32(quest->RewardItemId[2]);
    data << uint32(quest->RewardItemId[3]);
    data << uint32(quest->GetRewardPackage());
    data << uint32(quest->RewardChoiceItemCount[5]);
    data << uint32(quest->GetCharTitleId());
    data << uint32(rewChoiceItemDisplayId[0]);
    data << uint32(rewChoiceItemDisplayId[4]);
    data << uint32(quest->RewardItemIdCount[0]);
    data << uint32(quest->GetBonusTalents());
    data << uint32(rewItemDisplayId[2]);
    data << uint32(rewItemDisplayId[3]);
    data << uint32(0);                                      // QuestFlags2
    data << uint32(rewChoiceItemDisplayId[5]);
    data << uint32(quest->RewardItemId[0]);

    data.WriteBitSeq<4>(guid);
    data.WriteBitSeq<0, 3>(guid2);
    data.WriteBits(questGiverTextWindow.size(), 10);
    data.WriteBitSeq<2, 1>(guid);
    data.WriteBits(questTurnTextWindow.size(), 10);
    data.WriteBits(questDetails.size(), 12);
    data.WriteBitSeq<5>(guid);
    data.WriteBit(0);                                       // DisplayPopup
    data.WriteBitSeq<0>(guid);
    data.WriteBits(QUEST_EMOTE_COUNT, 21);
    data.WriteBits(0, 22);                                  // LearnSpells
    data.WriteBits(questObjectives.size(), 12);
    data.WriteBits(quest->GetQuestObjectiveCount(), 20);
    data.WriteBits(questTurnTargetName.size(), 8);
    data.WriteBitSeq<5, 4, 2, 7>(guid2);
    data.WriteBits(questGiverTargetName.size(), 8);
    data.WriteBits(questTitle.size(), 9);
    data.WriteBitSeq<6>(guid2);
    data.WriteBit(1);                                       // AutoLaunched
    data.WriteBitSeq<3>(guid);
    data.WriteBitSeq<1>(guid2);
    data.WriteBitSeq<7, 6>(guid);
    data.WriteBit(0);                                       // StartCheat
    data.FlushBits();

    for (uint8 i = 0; i < QUEST_EMOTE_COUNT; ++i)
    {
        data << uint32(quest->DetailsEmote[i]);
        data << uint32(quest->DetailsEmoteDelay[i]);
    }

    data.WriteByteSeq<4, 5>(guid2);
    data.WriteString(questTurnTargetName);
    data.WriteString(questGiverTextWindow);
    data.WriteByteSeq<1>(guid2);
    data.WriteByteSeq<0>(guid);

    for (auto const &questObjective : quest->m_questObjectives)
    {
        data << uint8(questObjective->Type);
        data << int32(questObjective->Amount);
        data << uint32(questObjective->Id);
        data << uint32(questObjective->ObjectId);
    }

    data.WriteByteSeq<0>(guid2);
    data.WriteByteSeq<4, 3>(guid);
    data.WriteString(questTurnTargetName);
    data.WriteString(questGiverTargetName);
    data.WriteByteSeq<2>(guid2);

    /*for (int i = 0; i < learnSpellsCount; i++)
        packet.ReadUInt32(spell, i);*/

    data.WriteByteSeq<6>(guid2);
    data.WriteString(questTitle);
    data.WriteByteSeq<7>(guid);
    data.WriteString(questObjectives);
    data.WriteByteSeq<2>(guid);
    data.WriteByteSeq<3>(guid2);
    data.WriteByteSeq<6>(guid);
    data.WriteString(questDetails);
    data.WriteByteSeq<5>(guid);
    data.WriteByteSeq<7>(guid2);
    data.WriteByteSeq<1>(guid);

    _session->SendPacket(&data);

    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS NPCGuid=%u, questid=%u", GUID_LOPART(npcGUID), quest->GetQuestId());
}

void PlayerMenu::SendQuestQueryResponse(Quest const* quest) const
{
    std::string questTitle = quest->GetTitle();
    std::string questDetails = quest->GetDetails();
    std::string questObjectives = quest->GetObjectives();
    std::string questEndText = quest->GetEndText();
    std::string questCompletedText = quest->GetCompletedText();
    std::string questGiverTextWindow = quest->GetQuestGiverTextWindow();
    std::string questGiverTargetName = quest->GetQuestGiverTargetName();
    std::string questTurnTextWindow = quest->GetQuestTurnTextWindow();
    std::string questTurnTargetName = quest->GetQuestTurnTargetName();

    /*std::string questObjectiveText[QUEST_OBJECTIVES_COUNT];
    for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        questObjectiveText[i] = quest->ObjectiveText[i];*/

    int32 locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
    {
        if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(quest->GetQuestId()))
        {
            ObjectMgr::GetLocaleString(localeData->Title, locale, questTitle);
            ObjectMgr::GetLocaleString(localeData->Details, locale, questDetails);
            ObjectMgr::GetLocaleString(localeData->Objectives, locale, questObjectives);
            ObjectMgr::GetLocaleString(localeData->EndText, locale, questEndText);
            ObjectMgr::GetLocaleString(localeData->CompletedText, locale, questCompletedText);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTextWindow, locale, questGiverTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTargetName, locale, questGiverTargetName);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTextWindow, locale, questTurnTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTargetName, locale, questTurnTargetName);

            // TODO SKYBRO: Fix objective locale text
            /*for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
                ObjectMgr::GetLocaleString(localeData->ObjectiveText[i], locale, questObjectiveText[i]);*/
        }
    }

    uint32 rewChoiceItemDisplayId[QUEST_REWARD_CHOICES_COUNT];
    for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; i++)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[i]))
            rewChoiceItemDisplayId[i] = itemTemplate->DisplayInfoID;
        else
            rewChoiceItemDisplayId[i] = 0;
    }

    WorldPacket data(SMSG_QUEST_QUERY_RESPONSE, 100);       // guess size
    data.WriteBit(1);                                       // has data
    data.WriteBits(questDetails.size(), 12);
    data.WriteBits(quest->GetQuestObjectiveCount(), 19);

    ByteBuffer objData;
    for (auto const &questObjective : quest->m_questObjectives)
    {
        data.WriteBits(questObjective->Description.size(), 8);
        data.WriteBits(questObjective->VisualEffects.size(), 22);

        objData << uint32(questObjective->Id);
        objData << uint32(questObjective->ObjectId);
        objData << uint32(questObjective->Flags);
        objData << int32(questObjective->Amount);
        objData.WriteString(questObjective->Description);
        objData << uint8(questObjective->Index);
        objData << uint8(questObjective->Type);

        for (auto const &visualEffect : questObjective->VisualEffects)
            objData << uint32(visualEffect);
    }

    data.WriteBits(questCompletedText.size(), 11);
    data.WriteBits(questGiverTargetName.size(), 8);
    data.WriteBits(questTurnTextWindow.size(), 10);
    data.WriteBits(questObjectives.size(), 12);
    data.WriteBits(questEndText.size(), 9);
    data.WriteBits(questTurnTargetName.size(), 8);
    data.WriteBits(questGiverTextWindow.size(), 10);
    data.WriteBits(questTitle.size(), 9);
    data.FlushBits();

    data << uint32(quest->RewardItemIdCount[2]);
    data << int32(quest->GetRewSpellCast());
    data << float(quest->GetRewHonorMultiplier());
    data.append(objData);
    data << uint32(quest->GetSrcItemId());
    data << uint32(0);                                              // 2965, itemSourceReq related
    data << float(quest->GetPointY());
    data.WriteString(questTurnTargetName);
    data << uint32(quest->GetNextQuestInChain());
    data.WriteString(questEndText);
    data << uint32(quest->RewardChoiceItemId[2]);

    for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
    {
        data << uint32(quest->RewardCurrencyCount[i]);
        data << uint32(quest->RewardCurrencyId[i]);
    }

    data << uint32(quest->GetMinLevel());
    data << float(quest->GetPointX());
    data << uint32(quest->RewardItemId[3]);

    for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
    {
        data << uint32(quest->RewardFactionValueId[i]);
        data << uint32(quest->RewardFactionValueIdOverride[i]);
        data << uint32(quest->RewardFactionId[i]);
    }

    data << uint32(quest->GetBonusTalents());
    data << uint32(rewChoiceItemDisplayId[2]);
    data << uint32(quest->GetQuestGiverPortrait());
    data << uint32(quest->GetQuestTurnInPortrait());
    data << uint32(quest->RewardChoiceItemCount[4]);
    data << uint32(0);                                              // 2962
    data.WriteString(questTitle);
    data << uint32(quest->RewardItemIdCount[0]);
    data << uint32(0);                                              // 2964
    data << uint32(rewChoiceItemDisplayId[0]);
    data << uint32(quest->GetRewardSkillId());
    data << uint32(quest->GetMinimapTargetMark());
    data << uint32(rewChoiceItemDisplayId[3]);
    data << uint32(rewChoiceItemDisplayId[4]);
    data << uint32(quest->RewardChoiceItemCount[1]);
    data << uint32(quest->GetQuestId());
    data << uint32(quest->RewardItemId[1]);
    data << uint32(quest->RewardItemIdCount[1]);
    data << uint32(quest->RewardItemId[2]);
    data << uint32(quest->GetRewardSkillPoints());
    data << uint32(0);                                              // 2963
    data << uint32(quest->RewardChoiceItemId[1]);
    data << uint32(quest->GetRewardPackage());
    data << uint32(quest->GetRewardReputationMask());
    data.WriteString(questTurnTextWindow);
    data << uint32(quest->GetSoundTurnIn());
    data << uint32(quest->GetXPId());
    data << uint32(quest->GetFlags() & QUEST_ALLOWED_FLAGS_MASK);
    data.WriteString(questCompletedText);
    data << uint32(quest->GetQuestLevel());
    data << uint32(0);                                              // 2966
    data << uint32(quest->GetSuggestedPlayers());
    data << uint32(quest->GetPointMapId());
    data << uint32(quest->GetRewMoneyMaxLevel());
    data << uint32(0);
    data << uint32(0);                                              // 2961
    data << uint32(rewChoiceItemDisplayId[5]);
    data << uint32(quest->GetZoneOrSort());
    data << uint32(quest->RewardItemId[0]);
    data << uint32(quest->GetXPId());
    data.WriteString(questDetails);
    data << uint32(quest->RewardChoiceItemCount[0]);
    data.WriteString(questGiverTargetName);
    data << uint32(rewChoiceItemDisplayId[1]);
    data << uint32(quest->RewardChoiceItemId[0]);
    data << uint32(quest->GetQuestMethod());
    data << uint32(quest->GetRewHonorAddition());
    data << uint32(quest->GetCharTitleId());
    data.WriteString(questGiverTextWindow);
    data << uint32(quest->GetRewardMoney());
    data << uint32(quest->GetType());
    data.WriteString(questObjectives);
    data << uint32(quest->GetRewSpell());
    data << uint32(quest->RewardChoiceItemId[3]);
    data << uint32(0);                                              // 2960
    data << uint32(quest->RewardChoiceItemCount[5]);
    data << uint32(quest->RewardChoiceItemCount[3]);
    data << uint32(quest->RewardItemIdCount[3]);
    data << uint32(quest->GetSoundAccept());
    data << uint32(quest->RewardChoiceItemId[5]);
    data << uint32(0);                                              // 2959
    data << uint32(quest->RewardChoiceItemCount[2]);
    data << uint32(quest->GetPointOpt());
    data << uint32(quest->RewardChoiceItemId[4]);
    data << uint32(quest->GetQuestId());

    _session->SendPacket(&data);
    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_QUEST_QUERY_RESPONSE questid=%u", quest->GetQuestId());
}

void PlayerMenu::SendQuestGiverOfferReward(Quest const* quest, uint64 npcGUID, bool enableNext) const
{
    std::string questTitle = quest->GetTitle();
    std::string questOfferRewardText = quest->GetOfferRewardText();
    std::string questGiverTextWindow = quest->GetQuestGiverTextWindow();
    std::string questGiverTargetName = quest->GetQuestGiverTargetName();
    std::string questTurnTextWindow = quest->GetQuestTurnTextWindow();
    std::string questTurnTargetName = quest->GetQuestTurnTargetName();

    int locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
    {
        if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(quest->GetQuestId()))
        {
            ObjectMgr::GetLocaleString(localeData->Title, locale, questTitle);
            ObjectMgr::GetLocaleString(localeData->OfferRewardText, locale, questOfferRewardText);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTextWindow, locale, questGiverTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTargetName, locale, questGiverTargetName);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTextWindow, locale, questTurnTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTargetName, locale, questTurnTargetName);
        }
    }

    uint32 rewItemDisplayId[QUEST_REWARDS_COUNT];
    for (uint8 i = 0; i < QUEST_REWARDS_COUNT; i++)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardItemId[i]);
        rewItemDisplayId[i] = itemTemplate ? itemTemplate->DisplayInfoID : 0;
    }

    uint32 rewChoiceItemDisplayId[QUEST_REWARD_CHOICES_COUNT];
    for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; i++)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[i]);
        rewChoiceItemDisplayId[i] = itemTemplate ? itemTemplate->DisplayInfoID : 0;
    }

    ObjectGuid guid = npcGUID;

    WorldPacket data(SMSG_QUESTGIVER_OFFER_REWARD, 50);     // guess size
    data << uint32(rewChoiceItemDisplayId[2]);
    data << uint32(rewChoiceItemDisplayId[0]);
    data << uint32(quest->RewardItemIdCount[0]);

    for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
    {
        data << uint32(quest->RewardCurrencyId[i]);
        data << uint32(quest->RewardCurrencyCount[i]);
    }

    data << uint32(quest->RewardChoiceItemCount[0]);
    data << uint32(quest->GetCharTitleId());
    data << uint32(quest->GetRewardPackage());
    data << uint32(0);                                      // unk
    data << uint32(0);                                      // 83
    data << uint32(quest->RewardChoiceItemCount[4]);
    data << uint32(0);                                      // 99
    data << uint32(quest->RewardChoiceItemCount[3]);

    for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
    {
        data << uint32(quest->RewardFactionId[i]);
        data << uint32(quest->RewardFactionValueIdOverride[i]);
        data << uint32(quest->RewardFactionValueId[i]);
    }

    data << uint32(rewItemDisplayId[3]);
    data << uint32(rewItemDisplayId[1]);
    data << uint32(quest->RewardItemId[3]);
    data << uint32(0);                                      // 75
    data << uint32(quest->XPValue(_session->GetPlayer()) * sWorld->getRate(RATE_XP_QUEST));
    data << uint32(quest->RewardItemIdCount[2]);
    data << uint32(quest->RewardChoiceItemCount[3]);
    data << uint32(rewItemDisplayId[0]);
    data << uint32(quest->RewardChoiceItemCount[5]);
    data << uint32(quest->GetBonusTalents());
    data << uint32(quest->RewardItemIdCount[3]);
    data << uint32(rewChoiceItemDisplayId[1]);
    data << uint32(quest->RewardChoiceItemId[3]);
    data << uint32(0);                                      // 84
    data << uint32(quest->GetFlags());
    data << uint32(quest->RewardChoiceItemId[5]);
    data << uint32(0);                                      // 594
    data << uint32(0);                                      // 70
    data << uint32(quest->RewardChoiceItemCount[1]);
    data << uint32(rewChoiceItemDisplayId[3]);
    data << uint32(0);                                      // 82
    data << uint32(quest->RewardItemId[1]);
    data << uint32(quest->RewardChoiceItemId[4]);
    data << uint32(0);                                      // 145
    data << uint32(quest->RewardItemId[2]);
    data << uint32(rewItemDisplayId[2]);
    data << uint32(0);                                      // 102
    data << uint32(quest->GetRewChoiceItemsCount());
    data << uint32(quest->GetSuggestedPlayers());
    data << uint32(quest->RewardChoiceItemId[2]);
    data << uint32(quest->RewardChoiceItemId[1]);
    data << uint32(quest->GetRewardMoney());
    data << uint32(quest->RewardChoiceItemId[0]);
    data << uint32(quest->RewardItemId[0]);
    data << uint32(0);                                      // 80
    data << uint32(quest->RewardItemIdCount[1]);
    data << uint32(quest->GetRewSpellCast());
    data << uint32(quest->GetQuestId());

    data.WriteBitSeq<4>(guid);
    data.WriteBits(questTitle.size(), 9);
    data.WriteBits(questGiverTextWindow.size(), 10);
    data.WriteBits(questTurnTextWindow.size(), 10);
    data.WriteBits(QUEST_EMOTE_COUNT, 21);
    data.WriteBit(enableNext);
    data.WriteBitSeq<6>(guid);
    data.WriteBits(questGiverTargetName.size(), 8);
    data.WriteBits(questTurnTargetName.size(), 8);
    data.WriteBitSeq<0, 2, 5, 1, 7>(guid);
    data.WriteBits(questOfferRewardText.size(), 12);
    data.WriteBitSeq<3>(guid);
    data.FlushBits();

    data.WriteByteSeq<5, 0>(guid);
    data.WriteString(questGiverTextWindow);
    data.WriteByteSeq<4, 1>(guid);

    for (uint8 i = 0; i < QUEST_EMOTE_COUNT; ++i)
    {
        data << uint32(quest->DetailsEmote[i]);
        data << uint32(quest->DetailsEmoteDelay[i]);        // DetailsEmoteDelay (in ms)
    }

    data.WriteString(questTitle);
    data.WriteByteSeq<6>(guid);
    data.WriteString(questTurnTargetName);
    data.WriteString(questTurnTextWindow);
    data.WriteString(questOfferRewardText);
    data.WriteByteSeq<2, 3, 7>(guid);
    data.WriteString(questGiverTargetName);

    _session->SendPacket(&data);
    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD NPCGuid=%u, questid=%u", GUID_LOPART(npcGUID), quest->GetQuestId());
}

void PlayerMenu::SendQuestGiverRequestItems(Quest const* quest, uint64 npcGUID, bool canComplete, bool closeOnCancel) const
{
    // We can always call to RequestItems, but this packet only goes out if there are actually
    // items.  Otherwise, we'll skip straight to the OfferReward

    std::string questTitle = quest->GetTitle();
    std::string requestItemsText = quest->GetRequestItemsText();

    uint32 itemCount = quest->GetQuestObjectiveCountType(QUEST_OBJECTIVE_TYPE_ITEM);
    uint32 currencyCount = quest->GetQuestObjectiveCountType(QUEST_OBJECTIVE_TYPE_CURRENCY);
    uint32 moneyCount = quest->GetQuestObjectiveCountType(QUEST_OBJECTIVE_TYPE_MONEY);
    if (!itemCount && !currencyCount && !moneyCount && canComplete)
    {
        SendQuestGiverOfferReward(quest, npcGUID, true);
        return;
    }

    int32 locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
    {
        if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(quest->GetQuestId()))
        {
            ObjectMgr::GetLocaleString(localeData->Title, locale, questTitle);
            ObjectMgr::GetLocaleString(localeData->RequestItemsText, locale, requestItemsText);
        }
    }

    uint32 requiredMoney = 0;
    ByteBuffer currencyData, itemData;
    for (const auto &questObjective : quest->m_questObjectives)
    {
        switch (questObjective->Type)
        {
            case QUEST_OBJECTIVE_TYPE_ITEM:
            {
                if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(questObjective->ObjectId))
                    itemData << uint32(itemTemplate->DisplayInfoID);
                else
                    itemData << uint32(0);

                itemData << uint32(questObjective->Amount);
                itemData << uint32(questObjective->ObjectId);

                break;
            }
            case QUEST_OBJECTIVE_TYPE_CURRENCY:
            {
                currencyData << uint32(questObjective->Amount);
                currencyData << uint32(questObjective->ObjectId);

                break;
            }
            case QUEST_OBJECTIVE_TYPE_MONEY:
            {
                requiredMoney = questObjective->Amount;
                break;
            }
            default:
                break;
        }
    }

    ObjectGuid guid = npcGUID;

    WorldPacket data(SMSG_QUESTGIVER_REQUEST_ITEMS, 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1 + 8 + 8 +
        questTitle.size() + requestItemsText.size() + itemCount * (4 + 4 + 4) + currencyCount * (4 + 4));

    data << uint32(0);
    data << uint32(canComplete ? quest->GetCompleteEmote() : quest->GetIncompleteEmote());
    data << uint32(quest->GetFlags());
    data << uint32(quest->GetQuestId());
    data << uint32(GUID_ENPART(npcGUID));
    data << uint32(requiredMoney);
    data << uint32(0);
    data << uint32(canComplete ? 0x5F : 0x5B);              // status flags
    data << uint32(0);

    data.WriteBit(closeOnCancel);
    data.WriteBitSeq<5, 1, 0>(guid);
    data.WriteBits(requestItemsText.size(), 12);
    data.WriteBitSeq<4>(guid);
    data.WriteBits(currencyCount, 21);
    data.WriteBits(itemCount, 20);
    data.WriteBitSeq<3, 2, 6, 7>(guid);
    data.WriteBits(questTitle.size(), 9);
    data.FlushBits();

    data.WriteByteSeq<6>(guid);
    data.append(itemData);
    data.WriteString(questTitle);
    data.append(currencyData);
    data.WriteByteSeq<4>(guid);
    data.WriteString(requestItemsText);
    data.WriteByteSeq<0, 7, 2, 5, 1, 3>(guid);

    _session->SendPacket(&data);
    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS NPCGuid=%u, questid=%u", GUID_LOPART(npcGUID), quest->GetQuestId());
}
