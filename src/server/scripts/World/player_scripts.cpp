#include "ScriptMgr.h"
#include "Player.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"

class HunterScript : public PlayerScript
{
    enum
    {
        PET_CREATE_SPELL_ID   = 13481,
        PET_START_LEVEL       = 1,
        PET_START_EXPERIENCE  = 0,
        PET_START_REACT_STATE = 0,
        PET_START_MANA        = 0
    };

    struct PetInfo
    {
        typedef std::pair<uint8, uint32> ActionBarItem;

        uint32 entry;
        uint32 model;
        std::string name;
        uint32 health;
        ActionBarItem actionBar[ACTION_BAR_INDEX_END];

        PetInfo() { }

        PetInfo(uint32 newEntry, uint32 newModel, char const *newName,
                uint32 newHealth, uint32 spellId)
            : entry(newEntry), model(newModel), name(newName), health(newHealth)
        {
            actionBar[0] = ActionBarItem(ACT_COMMAND, 2);
            actionBar[1] = ActionBarItem(ACT_COMMAND, 1);
            actionBar[2] = ActionBarItem(ACT_COMMAND, 0);
            actionBar[3] = ActionBarItem(ACT_DISABLED, 2649);
            actionBar[4] = ActionBarItem(ACT_DISABLED, spellId);
            actionBar[5] = ActionBarItem(ACT_PASSIVE, 0);
            actionBar[6] = ActionBarItem(ACT_PASSIVE, 0);
            actionBar[7] = ActionBarItem(ACT_REACTION, 2);
            actionBar[8] = ActionBarItem(ACT_REACTION, 1);
            actionBar[9] = ActionBarItem(ACT_REACTION, 0);
        }
    };

public:
    HunterScript()
        : PlayerScript("HunterScript")
    { }

    void OnCreate(Player* player)
    {
        if (player->getClass() != CLASS_HUNTER)
            return;

        PetInfo petInfo;

        switch (player->getRace()) {
            case RACE_HUMAN:
                petInfo = PetInfo(42717, 903, "Wolf", 192, 17253);
                break;
            case RACE_DWARF:
                petInfo = PetInfo(42713, 822, "Bear", 212, 16827);
                break;
            case RACE_ORC:
                petInfo = PetInfo(42719, 744, "Boar", 212, 17253);
                break;
            case RACE_NIGHTELF:
                petInfo = PetInfo(42718, 17090, "Cat", 192, 16827);
                break;
            case RACE_UNDEAD_PLAYER:
                petInfo = PetInfo(51107, 368, "Spider", 202, 17253);
                break;
            case RACE_TAUREN:
                petInfo = PetInfo(42720, 29057, "Tallstrider", 192, 16827);
                break;
            case RACE_TROLL:
                petInfo = PetInfo(42721, 23518, "Raptor", 192, 16827);
                break;
            case RACE_GOBLIN:
                petInfo = PetInfo(42715, 27692, "Crab", 212, 16827);
                break;
            case RACE_BLOODELF:
                petInfo = PetInfo(42710, 23515, "Dragonhawk", 202, 17253);
                break;
            case RACE_DRAENEI:
                petInfo = PetInfo(42712, 29056, "Moth", 192, 49966);
                break;
            case RACE_WORGEN:
                petInfo = PetInfo(42722, 30221, "Dog", 192, 17253);
                break;
            case RACE_PANDAREN_NEUTRAL:
                petInfo = PetInfo(57239, 42656, "Turtle", 192, 17253);
                break;
            default:
                return;
        }

        uint32 petId = sObjectMgr->GeneratePetNumber();

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_INFO);
        stmt->setUInt32(0, petId);
        stmt->setUInt32(1, player->GetGUIDLow());
        stmt->setUInt32(2, petInfo.entry);
        stmt->setUInt32(3, petInfo.model);
        stmt->setUInt32(4, PET_CREATE_SPELL_ID);
        stmt->setUInt8(5, HUNTER_PET);
        stmt->setUInt8(6, PET_START_LEVEL);
        stmt->setUInt32(7, PET_START_EXPERIENCE);
        stmt->setUInt8(8, PET_START_REACT_STATE);
        stmt->setString(9, petInfo.name);
        stmt->setBool(10, false);
        stmt->setUInt32(11, petInfo.health);
        stmt->setUInt32(12, PET_START_MANA);
        stmt->setUInt32(13, static_cast<uint32>(time(NULL)));
        stmt->setUInt32(14, 0);

        trans->Append(stmt);

        for (uint8 i = ACTION_BAR_INDEX_START; i != ACTION_BAR_INDEX_END; ++i)
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_ACTION_BAR);
            stmt->setUInt32(0, petId);
            stmt->setUInt8(1, i);
            stmt->setUInt8(2, petInfo.actionBar[i].first);
            stmt->setUInt32(3, petInfo.actionBar[i].second);

            trans->Append(stmt);
        }

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CURRENT_PET_ID);
        stmt->setUInt32(0, player->GetGUIDLow());
        stmt->setUInt32(1, petId);

        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SLOT);
        stmt->setUInt32(0, petId);
        stmt->setUInt8(1, PET_SLOT_ACTIVE_FIRST);

        trans->Append(stmt);

        CharacterDatabase.CommitTransaction(trans);
    }
};

class PlayerLoginScript : public PlayerScript
{
public:
    PlayerLoginScript() : PlayerScript("PlayerLoginScript") { }

    void OnEndWatching(Player* player)
    {
        if (player->HasAura(91847))
        {
            //player->CastSpell(player, 74100, false);
            WorldLocation loc;
            loc.m_mapId = 1;
            loc.m_positionX = 1440.89f;
            loc.m_positionY = -5021.77f;
            loc.m_positionZ = 12.0647f;
            player->RemoveAllAuras();
            player->SetHomebind(loc, 374);
            player->TeleportTo(1, 1440.89f, -5021.77f, 12.0647f, 1.6081f);
        }
    }

    void OnLogin(Player* player)
    {
        if (player->GetMapId() == 654 && player->GetPhaseMask() == 1)
        {
            PhaseShiftSet terrainswap;
            terrainswap.insert(638);

            PhaseShiftSet phaseId;
            phaseId.insert(170);

            PhaseShiftSet worldMapAreaIds;

            player->GetSession()->SendSetPhaseShift(phaseId, terrainswap, worldMapAreaIds);
        }

        ////if (player->HasFlagExtraWorgenForm())
        ////player->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_WORGEN_TRANSFORM2);

        //if (player->HasAura(SPELL_KEYS_TO_THE_HOT_ROD))
        //{
        //    player->RemoveAura(SPELL_KEYS_TO_THE_HOT_ROD);
        //    player->CastSpell(player, SPELL_KEYS_TO_THE_HOT_ROD, false);
        //}

        //uint32 uiSpellId = 0;
        //Unit::AuraEffectList const& phases = player->GetAuraEffectsByType(SPELL_AURA_PHASE);
        //if (!phases.empty())
        //    for (Unit::AuraEffectList::const_iterator itr = phases.begin(); itr != phases.end(); ++itr)
        //        uiSpellId = (*itr)->GetSpellInfo()->Id;

        //if (uiSpellId)
        //{
        //    player->RemoveAura(uiSpellId);
        //    player->CastSpell(player, uiSpellId, false);
        //}

        //uiSpellId = 0;
        //Unit::AuraEffectList const& overrided = player->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_SPELLS);
        //if (!overrided.empty())
        //    for (Unit::AuraEffectList::const_iterator itr = overrided.begin(); itr != overrided.end(); ++itr)
        //        uiSpellId = (*itr)->GetSpellInfo()->Id;

        //if (uiSpellId)
        //{
        //    player->RemoveAura(uiSpellId);
        //    player->CastSpell(player, uiSpellId, false);
        //}
    }
};

void AddSC_player_scripts()
{
    new HunterScript();
    new PlayerLoginScript();
}
