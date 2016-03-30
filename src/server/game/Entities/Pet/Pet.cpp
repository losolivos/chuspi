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
#include "DatabaseEnv.h"
#include "Log.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Pet.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "CreatureAI.h"
#include "Unit.h"
#include "Util.h"
#include "Group.h"

#define PET_XP_FACTOR 0.05f

Pet::Pet(Player* owner, PetType type)
    : Guardian(NULL, owner, true)
    , m_removed(false)
    , m_petType(type)
    , m_duration(0)
    , m_auraRaidUpdateMask(0)
    , m_loading(false)
    , m_specialization(0)
    , m_declinedname(NULL)
    , m_isWarlockPet(false)
    , m_Stampeded(false)
{
    m_unitTypeMask |= UNIT_MASK_PET;
    if (type == HUNTER_PET)
        m_unitTypeMask |= UNIT_MASK_HUNTER_PET;

    if (!(m_unitTypeMask & UNIT_MASK_CONTROLABLE_GUARDIAN))
    {
        m_unitTypeMask |= UNIT_MASK_CONTROLABLE_GUARDIAN;
        InitCharmInfo();
    }

    m_name = "Pet";
    m_regenTimer = PET_FOCUS_REGEN_INTERVAL;
}

Pet::~Pet()
{
    delete m_declinedname;
}

void Pet::AddToWorld()
{
    ///- Register the pet for guid lookup
    if (!IsInWorld())
    {
        ///- Register the pet for guid lookup
        sObjectAccessor->AddObject(this);
        Unit::AddToWorld();
        AIM_Initialize();
    }

    // Prevent stuck pets when zoning. Pets default to "follow" when added to world
    // so we'll reset flags and let the AI handle things
    if (GetCharmInfo() && GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        GetCharmInfo()->SetIsCommandAttack(false);
        GetCharmInfo()->SetIsCommandFollow(false);
        GetCharmInfo()->SetIsAtStay(false);
        GetCharmInfo()->SetIsFollowing(false);
        GetCharmInfo()->SetIsReturning(false);
    }

}

void Pet::RemoveFromWorld()
{
    ///- Remove the pet from the accessor
    if (IsInWorld())
    {
        ///- Don't call the function for Creature, normal mobs + totems go in a different storage
        Unit::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

bool Pet::LoadPetFromDB(PetLoadMode loadMode, uint32 value)
{
    class LoadingStateTracker
    {
    public:
        LoadingStateTracker(bool &loadingState)
            : m_loadingState(&loadingState)
        {
            *m_loadingState = true;
        }

        ~LoadingStateTracker()
        {
            *m_loadingState = false;
        }

    private:
        bool *m_loadingState;
    };

    LoadingStateTracker tracker(m_loading);

    Player* owner = GetOwner();
    uint32 ownerid = owner->GetGUIDLow();

    PreparedStatement* stmt = NULL;
    uint32 slot = 0;
    switch (loadMode)
    {
        case PET_LOAD_BY_ID:
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_INFO_BY_ID);
            stmt->setUInt32(0, value);
            break;
        case PET_LOAD_BY_ENTRY:
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_INFO_BY_ENTRY);
            stmt->setUInt32(0, ownerid);
            stmt->setUInt32(1, value);
            break;
        case PET_LOAD_BY_SLOT:
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_INFO_BY_SLOT);
            stmt->setUInt32(0, ownerid);
            stmt->setUInt8(1, value);
            break;
    }

    // 0   1      2         3      4           5            6     7        8           9         10         11               12    13
    // id, entry, model_id, level, experience, react_state, name, renamed, cur_health, cur_mana, save_time, create_spell_id, type, specialization

    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
        return false;

    Field* fields = result->Fetch();

    uint32 petId = fields[0].GetUInt32();
    uint32 petEntry = fields[1].GetUInt32();

    bool current = petId == owner->GetCurrentPetId();

    uint32 summonSpellId = fields[11].GetUInt32();

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(summonSpellId);

    bool isTemporarySummon = spellInfo && spellInfo->GetDuration() > 0;
    if (current && isTemporarySummon)
        return false;

    PetType petType = PetType(fields[12].GetUInt8());
    if (petType == HUNTER_PET)
    {
        auto const creatureInfo = sObjectMgr->GetCreatureTemplate(petEntry);
        if (!creatureInfo || !creatureInfo->isTameable(owner->CanTameExoticPets()))
            return false;
    }

    if (current && owner->IsPetNeedBeTemporaryUnsummoned())
    {
        owner->SetTemporaryUnsummonedPetNumber(petId);
        return false;
    }

    Map* map = owner->GetMap();
    uint32 guid = sObjectMgr->GenerateLowGuid(HIGHGUID_PET);
    if (!Create(guid, map, owner->GetPhaseMask(), petEntry, petId))
        return false;

    float px, py, pz;
    owner->GetClosePoint(px, py, pz, GetObjectSize(), PET_FOLLOW_DIST, GetFollowAngle());
    Relocate(px, py, pz, owner->GetOrientation());

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.pet", "Pet (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",
                       GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    setPetType(petType);
    if (owner->getClass() == CLASS_WARLOCK)
        m_isWarlockPet = true;
    setFaction(owner->getFaction());
    SetUInt32Value(UNIT_CREATED_BY_SPELL, summonSpellId);

    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (cinfo->type == CREATURE_TYPE_CRITTER)
    {
        map->AddToMap(this->ToCreature());
        return true;
    }

    m_charmInfo->SetPetNumber(petId, IsPermanentPetFor(owner));

    SetDisplayId(fields[2].GetUInt32());
    SetNativeDisplayId(fields[2].GetUInt32());
    uint8 petlevel = fields[3].GetUInt8();
    SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    SetUInt32Value(UNIT_NPC_FLAGS + 1, UNIT_NPC_FLAG2_NONE);
    SetName(fields[6].GetString());

    switch (getPetType())
    {
        case SUMMON_PET:
            petlevel = owner->getLevel();

            // either rogue, or mage
            SetUInt32Value(UNIT_FIELD_BYTES_0, (IsPetGhoul() || IsWarlockPet()) ? 0x400 : 0x800);
            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
                                                            // this enables popup window (pet dismiss, cancel)
            break;
        case HUNTER_PET:
            SetUInt32Value(UNIT_FIELD_BYTES_0, PET_BYTES_0);
            SetSheath(SHEATH_STATE_MELEE);
            SetByteFlag(UNIT_FIELD_BYTES_2, 2, fields[7].GetBool() ? UNIT_CAN_BE_ABANDONED : UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);

            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
                                                            // this enables popup window (pet abandon, cancel)
            setPowerType(POWER_FOCUS);
            SetSpecializationId(fields[13].GetUInt8());
            break;
        default:
            TC_LOG_ERROR("entities.pet", "Pet have incorrect type (%u) for pet loading.", getPetType());
            break;
    }

    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(NULL))); // cast can't be helped here
    SetCreatorGUID(owner->GetGUID());

    InitStatsForLevel(petlevel);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, fields[4].GetUInt32());

    SynchronizeLevelWithOwner();

    SetReactState(ReactStates(fields[5].GetUInt8()));
    SetCanModifyStats(true);

    if (getPetType() == SUMMON_PET && !current)              //all (?) summon pets come with full health when called, but not when they are current
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    else
    {
        uint32 savedhealth = fields[8].GetUInt32();
        uint32 savedmana = fields[9].GetUInt32();
        if (savedhealth == 0 && getPetType() == HUNTER_PET)
            setDeathState(JUST_DIED);
        else
        {
            SetHealth(savedhealth > GetMaxHealth() ? GetMaxHealth() : savedhealth);
            SetPower(POWER_MANA, savedmana > uint32(GetMaxPower(POWER_MANA)) ? GetMaxPower(POWER_MANA) : savedmana);
        }
    }

    // Send fake summon spell cast - this is needed for correct cooldown application for spells
    // Example: 46584 - without this cooldown (which should be set always when pet is loaded) isn't set clientside
    // TODO: pets should be summoned from real cast instead of just faking it?
    /*if (summon_spell_id)
    {
        WorldPacket data(SMSG_SPELL_GO, (8+8+4+4+2));
        data.append(owner->GetPackGUID());
        data.append(owner->GetPackGUID());
        data << uint8(0);
        data << uint32(summon_spell_id);
        data << uint32(256); // CAST_FLAG_UNKNOWN3
        data << uint32(0);
        owner->SendMessageToSet(&data, true);
    }*/

    owner->SetMinion(this, true);
    map->AddToMap(this->ToCreature());

    uint32 timediff = uint32(time(NULL) - fields[10].GetUInt32());
    _LoadAuras(timediff);

    if (owner->GetTypeId() == TYPEID_PLAYER && owner->ToPlayer()->InArena())
        RemoveArenaAuras();

    // load action bar, if data broken will fill later by default spells.
    if (!isTemporarySummon)
    {
        m_charmInfo->LoadPetActionBar();

        _LoadSpells();
        _LoadSpellCooldowns();
        LearnPetPassives();
        InitLevelupSpellsForLevel();
        CastPetAuras(current);
    }

    CleanupActionBar();                                     // remove unknown spells from action bar after load

    TC_LOG_DEBUG("entities.pet", "New Pet has guid %u", GetGUIDLow());

    owner->PetSpellInitialize();

    if (owner->GetGroup())
        owner->SetGroupUpdateFlag(GROUP_UPDATE_PET);

    owner->SendTalentsInfoData(true);

    if (getPetType() == HUNTER_PET)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_DECLINED_NAME);
        stmt->setUInt32(0, GetCharmInfo()->GetPetNumber());

        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
        {
            delete m_declinedname;
            m_declinedname = new DeclinedName;
            Field* fields2 = result->Fetch();
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            {
                m_declinedname->name[i] = fields2[i].GetString();
            }
        }
    }

    //set last used pet number (for use in BG's)
    if (owner->GetTypeId() == TYPEID_PLAYER && isControlled() && !isTemporarySummoned() && (getPetType() == SUMMON_PET || getPetType() == HUNTER_PET))
        owner->ToPlayer()->SetLastPetNumber(petId);

    if (owner->getClass() == CLASS_WARLOCK)
    {
        if (owner->HasAura(108503))
            owner->RemoveAura(108503);

        // Supplant Command Demon
        if (owner->getLevel() >= 56)
        {
            int32 bp = 0;

            owner->RemoveAura(119904);

            switch (GetEntry())
            {
                case ENTRY_IMP:
                case ENTRY_FEL_IMP:
                    bp = 119905;// Cauterize Master
                    break;
                case ENTRY_VOIDWALKER:
                case ENTRY_VOIDLORD:
                    bp = 119907;// Disarm
                    break;
                case ENTRY_SUCCUBUS:
                    bp = 119909;// Whilplash
                    break;
                case ENTRY_SHIVARRA:
                    bp = 119913;// Fellash
                    break;
                case ENTRY_FELHUNTER:
                    bp = 119910;// Spell Lock
                    break;
                case ENTRY_OBSERVER:
                    bp = 119911;// Optical Blast
                    break;
                case ENTRY_FELGUARD:
                    bp = 119914;// Felstorm
                    break;
                case ENTRY_WRATHGUARD:
                    bp = 119915;// Wrathstorm
                    break;
                default:
                    break;
            }

            if (bp)
                owner->CastCustomSpell(owner, 119904, &bp, NULL, NULL, true);
        }
    }

    owner->SetCurrentPetId(GetCharmInfo()->GetPetNumber());
    owner->SetCurrentPetSlot(loadMode == PET_LOAD_BY_SLOT ? value : fields[14].GetUInt8());
    return true;
}

void Pet::SavePetToDB(bool newPet, SQLTransaction trans)
{
    if (!GetEntry())
        return;

    // save only fully controlled creature
    if (!isControlled())
        return;

    // not save not player pets
    if (!IS_PLAYER_GUID(GetOwnerGUID()))
        return;

    Player* owner = GetOwner();
    if (!owner)
        return;

    bool internalTransaction = false;
    if (!trans)
    {
        trans = CharacterDatabase.BeginTransaction();
        internalTransaction = true;
    }

    PreparedStatement* stmt;
    uint8 index = 0;

    if (!newPet)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PET_INFO);
    }
    else
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_INFO);
        stmt->setUInt32(index++, GetCharmInfo()->GetPetNumber());
    }

    stmt->setUInt32(index++, owner->GetGUIDLow());
    stmt->setUInt32(index++, GetEntry());
    stmt->setUInt32(index++, GetNativeDisplayId());
    stmt->setUInt32(index++, GetUInt32Value(UNIT_CREATED_BY_SPELL));
    stmt->setUInt8(index++, getPetType());
    stmt->setUInt8(index++, getLevel());
    stmt->setUInt32(index++, GetUInt32Value(UNIT_FIELD_PETEXPERIENCE));
    stmt->setUInt8(index++, GetReactState());
    stmt->setString(index++, GetName());
    stmt->setBool(index++, !HasByteFlag(UNIT_FIELD_BYTES_2, 2, UNIT_CAN_BE_RENAMED));
    stmt->setUInt32(index++, GetHealth());
    stmt->setUInt32(index++, static_cast<uint32>(GetPower(POWER_MANA)));
    stmt->setUInt32(index++, static_cast<uint32>(time(NULL)));
    stmt->setUInt32(index++, GetSpecializationId());

    if (!newPet)
        stmt->setUInt32(index++, GetCharmInfo()->GetPetNumber());

    trans->Append(stmt);

    _SaveActionBar(trans, newPet);
    _SaveAuras(trans);
    _SaveSpells(trans);
    _SaveSpellCooldowns(trans);

    if (internalTransaction)
        CharacterDatabase.CommitTransaction(trans);
}

void Pet::DeletePetFromDB()
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_INFO_BY_ID);
    stmt->setUInt32(0, GetCharmInfo()->GetPetNumber());
    trans->Append(stmt);

    CharacterDatabase.DirectCommitTransaction(trans);
}

void Pet::setDeathState(DeathState s)                       // overwrite virtual Creature::setDeathState and Unit::setDeathState
{
    Creature::setDeathState(s);
    if (getDeathState() == CORPSE)
    {
        if (getPetType() == HUNTER_PET)
        {
            // pet corpse non lootable and non skinnable
            SetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            //SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        }
    }
    else if (getDeathState() == ALIVE)
    {
        if (getPetType() == HUNTER_PET)
        {
            CastPetAuras(true);

            if (Unit* owner = GetOwner())
                if (Player* player = owner->ToPlayer())
                    player->StopCastingCharm();
        }
        else
        {
            //RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
            CastPetAuras(true);
        }
    }
}

void Pet::Update(uint32 diff)
{
    if (m_removed)                                           // pet already removed, just wait in remove queue, no updates
        return;

    if (m_loading)
        return;

    switch (m_deathState)
    {
        case CORPSE:
        {
            if ((getPetType() != HUNTER_PET || m_corpseRemoveTime <= time(NULL)) && !isStampeded())
            {
                //hunters' pets never get removed because of death, NEVER!
                Remove(PET_REMOVE_DISMISS, PET_REMOVE_FLAG_RESET_CURRENT);
                return;
            }
            break;
        }
        case ALIVE:
        {
            // unsummon pet that lost owner
            Player* owner = GetOwner();
            if (!owner || (!IsWithinDistInMap(owner, GetMap()->GetVisibilityRange()) && !isPossessed()) || (isControlled() && !owner->GetPetGUID()))
            {
                Remove(PET_REMOVE_DISMISS, PET_REMOVE_FLAG_RETURN_REAGENT | PET_REMOVE_FLAG_RESET_CURRENT);
                return;
            }

            if (isControlled())
            {
                if (owner->GetPetGUID() != GetGUID() && !HasAura(130201)) // Stampede
                {
                    TC_LOG_ERROR("entities.pet", "Pet %u is not pet of owner %s, removed", GetEntry(), GetOwner()->GetName().c_str());
                    Remove(PET_REMOVE_DISMISS, PET_REMOVE_FLAG_RESET_CURRENT);
                    return;
                }
            }

            if (m_duration > 0)
            {
                if (uint32(m_duration) > diff)
                    m_duration -= diff;
                else
                {
                    if (isStampeded())
                    {
                        UnSummon(0, true);
                        owner->m_Controlled.erase(this);
                    }
                    else
                        Remove(PET_REMOVE_ABANDON);

                    return;
                }
            }

            //regenerate focus for hunter pets or energy for deathknight's ghoul
            if (m_regenTimer)
            {
                if (m_regenTimer > diff)
                    m_regenTimer -= diff;
                else
                {
                    switch (getPowerType())
                    {
                        case POWER_FOCUS:
                            Regenerate(POWER_FOCUS);
                            m_regenTimer += PET_FOCUS_REGEN_INTERVAL - diff;
                            if (!m_regenTimer)
                                ++m_regenTimer;
                            // Reset if large diff (lag) causes focus to get 'stuck'
                            if (m_regenTimer > PET_FOCUS_REGEN_INTERVAL)
                                m_regenTimer = PET_FOCUS_REGEN_INTERVAL;
                            break;
                        default:
                            m_regenTimer = 0;
                            break;
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    Creature::Update(diff);
}

void Pet::Regenerate(Powers power)
{
    uint32 curValue = GetPower(power);
    uint32 maxValue = GetMaxPower(power);

    if (curValue >= maxValue)
        return;

    // Skip regeneration for power type we cannot have
    auto const powerIndex = GetPowerIndex(power);
    if (powerIndex == MAX_POWERS)
        return;

    float addvalue = 0.0f;
    float rangedHaste = (isHunterPet() && GetOwner()) ? GetOwner()->ToPlayer()->GetFloatValue(UNIT_FIELD_MOD_RANGED_HASTE) : 0.0f;

    switch (power)
    {
        case POWER_FOCUS:
        {
            // For hunter pets - Pets regen focus 125% more faster than owners
            addvalue += (24.0f + CalculatePct(24.0f, rangedHaste)) * sWorld->getRate(RATE_POWER_FOCUS);
            addvalue *= 1.25f;
            break;
        }
        case POWER_ENERGY:
        {
            // For deathknight's ghoul and Warlock's pets
            addvalue = 20;
            break;
        }
        default:
            return;
    }

    // Apply modifiers (if any).
    addvalue *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, power);
    addvalue += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, power) * (isHunterPet()? PET_FOCUS_REGEN_INTERVAL : CREATURE_REGEN_INTERVAL) / (5 * IN_MILLISECONDS);

    int32 intAddValue = int32(addvalue);

    if (IsAIEnabled)
        AI()->RegeneratePower(power, intAddValue);

    ModifyPower(power, intAddValue);
}

void Pet::Remove(PetRemoveMode mode, uint8 removeFlags)
{
    if (isStampeded())
        return;

    GetOwner()->RemovePet(mode, removeFlags);
}

void Pet::GivePetXP(uint32 xp)
{
    if (getPetType() != HUNTER_PET)
        return;

    if (xp < 1)
        return;

    if (!IsAlive())
        return;

    uint8 maxlevel = std::min((uint8)sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL), GetOwner()->getLevel());
    uint8 petlevel = getLevel();

    // If pet is detected to be at, or above(?) the players level, don't hand out XP
    if (petlevel >= maxlevel)
       return;

    uint32 nextLvlXP = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
    uint32 curXP = GetUInt32Value(UNIT_FIELD_PETEXPERIENCE);
    uint32 newXP = curXP + xp;

    // Check how much XP the pet should receive, and hand off have any left from previous levelups
    while (newXP >= nextLvlXP && petlevel < maxlevel)
    {
        // Subtract newXP from amount needed for nextlevel, and give pet the level
        newXP -= nextLvlXP;
        ++petlevel;

        GivePetLevel(petlevel);

        nextLvlXP = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
    }
    // Not affected by special conditions - give it new XP
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, petlevel < maxlevel ? newXP : 0);
}

void Pet::GivePetLevel(uint8 level)
{
    if (!level || level == getLevel())
        return;

    if (getPetType()==HUNTER_PET)
    {
        SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
        SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(level)*PET_XP_FACTOR));
    }

    InitStatsForLevel(level);
    InitLevelupSpellsForLevel();
}

bool Pet::CreateBaseAtCreature(Creature* creature)
{
    ASSERT(creature);

    if (!CreateBaseAtTamed(creature->GetCreatureTemplate(), creature->GetMap(), creature->GetPhaseMask()))
        return false;

    Relocate(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.pet", "Pet (guidlow %d, entry %d) not created base at creature. Suggested coordinates isn't valid (X: %f Y: %f)",
                       GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (!cinfo)
    {
        TC_LOG_ERROR("entities.pet", "CreateBaseAtCreature() failed, creatureInfo is missing!");
        return false;
    }

    SetDisplayId(creature->GetDisplayId());

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family))
        SetName(cFamily->Name);
    else
        SetName(creature->GetNameForLocaleIdx(sObjectMgr->GetDBCLocaleIndex()));

    return true;
}

bool Pet::CreateBaseAtCreatureInfo(CreatureTemplate const* cinfo, Unit* owner)
{
    if (!CreateBaseAtTamed(cinfo, owner->GetMap(), owner->GetPhaseMask()))
        return false;

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family))
        SetName(cFamily->Name);

    Relocate(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation());

    return true;
}

bool Pet::CreateBaseAtTamed(CreatureTemplate const* cinfo, Map* map, uint32 phaseMask)
{
    TC_LOG_DEBUG("entities.pet", "Pet::CreateBaseForTamed");
    uint32 guid = sObjectMgr->GenerateLowGuid(HIGHGUID_PET);
    uint32 petId = sObjectMgr->GeneratePetNumber();
    if (!Create(guid, map, phaseMask, cinfo->Entry, petId))
        return false;

    GetCharmInfo()->SetPetNumber(petId, true);

    setPowerType(POWER_FOCUS);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(getLevel()+1)*PET_XP_FACTOR));
    SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    SetUInt32Value(UNIT_NPC_FLAGS + 1, UNIT_NPC_FLAG2_NONE);

    if (cinfo->type == CREATURE_TYPE_BEAST)
    {
        SetUInt32Value(UNIT_FIELD_BYTES_0, PET_BYTES_0);
        SetSheath(SHEATH_STATE_MELEE);
        SetByteFlag(UNIT_FIELD_BYTES_2, 2, UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);
    }

    return true;
}

// TODO: Move stat mods code to pet passive auras
bool Guardian::InitStatsForLevel(uint8 petlevel)
{
    CreatureTemplate const* cinfo = GetCreatureTemplate();
    ASSERT(cinfo);

    SetLevel(petlevel);

    //Determine pet type
    PetType petType = MAX_PET_TYPE;
    if (isPet() && GetOwner()->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetOwner()->getClass() == CLASS_WARLOCK
                || GetOwner()->getClass() == CLASS_SHAMAN        // Fire Elemental
                || GetOwner()->getClass() == CLASS_PRIEST        // Shadowfiend and Mindbender
                || GetOwner()->getClass() == CLASS_DEATH_KNIGHT  // Risen Ghoul
                || GetOwner()->getClass() == CLASS_MAGE          // Water Elemental
                || GetOwner()->getClass() == CLASS_MONK)         // Transcendence
        {
            petType = SUMMON_PET;
        }
        else if (GetOwner()->getClass() == CLASS_HUNTER)
        {
            petType = HUNTER_PET;
            m_unitTypeMask |= UNIT_MASK_HUNTER_PET;
        }
        else
            TC_LOG_ERROR("entities.pet", "Unknown type pet %u is summoned by player class %u", GetEntry(), GetOwner()->getClass());
    }

    uint32 creature_ID = (petType == HUNTER_PET) ? 1 : cinfo->Entry;

    SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));

    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(petlevel*50));

    SetAttackTime(BASE_ATTACK, BASE_ATTACK_TIME);
    SetAttackTime(OFF_ATTACK, BASE_ATTACK_TIME);
    SetAttackTime(RANGED_ATTACK, BASE_ATTACK_TIME);

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
    SetFloatValue(UNIT_MOD_CAST_HASTE, 1.0f);

    //scale
    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family);
    if (cFamily && cFamily->minScale > 0.0f && petType == HUNTER_PET)
    {
        float scale;
        if (getLevel() >= cFamily->maxScaleLevel)
        {
            if (cinfo->type_flags & CREATURE_TYPEFLAGS_EXOTIC)
                scale = 1.0f;
            else
                scale = cFamily->maxScale;
        }
        else if (getLevel() <= cFamily->minScaleLevel)
            scale = cFamily->minScale;
        else
            scale = cFamily->minScale + float(getLevel() - cFamily->minScaleLevel) / cFamily->maxScaleLevel * (cFamily->maxScale - cFamily->minScale);

        if (sObjectMgr->GetCreatureModelInfo(GetDisplayId()))
        {
            SetBoundingRadius(GetOwner()->GetFloatValue(UNIT_FIELD_BOUNDING_RADIUS));
            SetCombatReach(GetOwner()->GetFloatValue(UNIT_FIELD_COMBAT_REACH));
        }

        SetObjectScale(scale);
    }

    // Resistance
    for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        SetModifierValue(UnitMods(UNIT_MOD_RESISTANCE_START + i), BASE_VALUE, float(cinfo->resistance[i]));

    //health, mana, armor and resistance
    PetLevelInfo const* pInfo = sObjectMgr->GetPetLevelInfo(creature_ID, petlevel);
    if (pInfo)                                      // exist in DB
    {
        if (petType == HUNTER_PET)
        {
            SetCreateHealth(CalculatePct(m_owner->GetCreateHealth(), 70));
            SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, 0);
        }
        else
        {
            if (creature_ID != 510)
                SetCreateHealth(pInfo->health);
            if (petType != HUNTER_PET && GetOwner() && GetOwner()->getClass() != CLASS_WARLOCK && creature_ID != 510) // hunter's pets use focus and Warlock's pets use energy
                SetCreateMana(pInfo->mana);

            if (pInfo->armor > 0)
                SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(pInfo->armor));

            for (uint8 stat = 0; stat < MAX_STATS; ++stat)
                SetCreateStat(Stats(stat), float(pInfo->stats[stat]));
        }
    }
    else                                            // not exist in DB, use some default fake data
    {
        // remove elite bonuses included in DB values
        CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(petlevel, cinfo->unit_class);
        SetCreateHealth(stats->BaseHealth[cinfo->expansion]);
        SetCreateMana(stats->BaseMana);

        SetCreateStat(STAT_STRENGTH, 22);
        SetCreateStat(STAT_AGILITY, 22);
        SetCreateStat(STAT_STAMINA, 25);
        SetCreateStat(STAT_INTELLECT, 28);
        SetCreateStat(STAT_SPIRIT, 27);
    }

    // Greater Fire and Earth Elementals
    if (GetEntry() == 61029 || GetEntry() == 61056)
        petType = MAX_PET_TYPE;

    SetBonusDamage(0);
    switch (petType)
    {
        case SUMMON_PET:
        {
            //the damage bonus used for pets is either fire or shadow damage, whatever is higher
            uint32 fire  = GetOwner()->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);
            uint32 shadow = GetOwner()->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            uint32 val  = (fire > shadow) ? fire : shadow;
            SetBonusDamage(int32 (val));

            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));

            switch (GetEntry())
            {
                case ENTRY_GHOUL:
                    SetCreateHealth(GetCreateHealth() / 7);
                    CastSpell(this, 47466, true);
                    // Glyph of the Geist
                    if (GetOwner()->HasAura(58640))
                        SetDisplayId(25664);
                    break;
                case ENTRY_FEL_IMP:
                    CastSpell(this, 115578, true); // Grimoire of Supremacy - +20% damage done
                    break;
                case ENTRY_VOIDLORD:
                    CastSpell(this, 115578, true); // Grimoire of Supremacy - +20% damage done
                    break;
                case ENTRY_SHIVARRA:
                    CastSpell(this, 114355, true); // Dual-Wield
                    CastSpell(this, 115578, true); // Grimoire of Supremacy - +20% damage done
                    break;
                case ENTRY_OBSERVER:
                    CastSpell(this, 115578, true); // Grimoire of Supremacy - +20% damage done
                    break;
                case ENTRY_WRATHGUARD:
                    CastSpell(this, 114355, true); // Dual-Wield
                    CastSpell(this, 115578, true); // Grimoire of Supremacy - +20% damage done
                    break;
                case ENTRY_WATER_ELEMENTAL:
                {
                    SetCreateHealth(GetOwner()->CountPctFromMaxHealth(50));
                    SetCreateMana(GetOwner()->GetMaxPower(POWER_MANA));
                    SetBonusDamage(int32(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FROST)));
                    // Glyph of Unbound Elemental
                    if (GetOwner()->HasAura(146976))
                        CastSpell(this, 147358, true);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case HUNTER_PET:
        {
            SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(petlevel)*PET_XP_FACTOR));
            //these formula may not be correct; however, it is designed to be close to what it should be
            //this makes dps 0.5 of pets level
            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
            //damage range is then petlevel / 2
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));

            if (GetOwner()->ToPlayer())
                ApplyAttackTimePercentMod(BASE_ATTACK, GetOwner()->ToPlayer()->GetRatingBonusValue(CR_HASTE_RANGED), true);
            //damage is increased afterwards as strength and pet scaling modify attack power
            break;
        }
        default:
        {
            switch (GetEntry())
            {
                case ENTRY_GHOUL:
                {
                    //the damage bonus used for pets is either fire or shadow damage, whatever is higher
                    uint32 fire  = GetOwner()->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);
                    uint32 shadow = GetOwner()->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
                    uint32 val  = (fire > shadow) ? fire : shadow;
                    SetBonusDamage(int32 (val));

                    // Glyph of the Geist
                    if (GetOwner()->HasAura(58640))
                        SetDisplayId(25664);

                    // Hardcode : Ghoul Base HP
                    if (IsPetGhoul() && getLevel() > 86)
                    {
                        SetCreateHealth(GetCreateHealth() / 7);
                        CastSpell(this, 47466, true);
                    }

                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));
                    break;
                }
                case ENTRY_IMP:
                case ENTRY_VOIDWALKER:
                case ENTRY_SUCCUBUS:
                case ENTRY_FELHUNTER:
                case ENTRY_FELGUARD:
                {
                    if (getPowerType() != POWER_ENERGY)
                        setPowerType(POWER_ENERGY);

                    SetMaxPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));
                    SetPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));

                    // Soul link
                    if (GetOwner()->HasSpell(108415))
                        GetOwner()->CastSpell(GetOwner(), 108415, true);
                    break;
                }
                case ENTRY_TREANT_GUARDIAN:
                case ENTRY_TREANT_FERAL:
                case ENTRY_TREANT_BALANCE:
                case ENTRY_TREANT_RESTO:
                {
                    SetCreateHealth(GetOwner()->CountPctFromMaxHealth(10));
                    float bonusDmg = GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NATURE) * 0.15f;
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 2.5f - (petlevel / 2) + bonusDmg));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 2.5f + (petlevel / 2) + bonusDmg));
                    break;
                }
                case 15352: // Earth Elemental Totem - 2062
                {
                    SetCreateHealth(GetOwner()->GetMaxHealth());
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 40));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 40));
                    break;
                }
                case 15438: // Fire Elemental Totem - 2894
                {
                    SetCreateHealth(uint32(GetOwner()->GetMaxHealth() * 0.75f));
                    SetCreateMana(28 + 100 * petlevel);
                    SetBonusDamage(int32(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 0.4f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 30));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 30));
                    break;
                }
                case 61056: // Earth Elemental Totem - 2062
                {
                    SetCreateHealth(GetOwner()->GetMaxHealth());
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 40 * 1.5f));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 40 * 1.5f));
                    break;
                }
                case 61029: // Fire Elemental Totem - 2894
                {
                    SetCreateHealth(uint32(GetOwner()->GetMaxHealth() * 0.75f));
                    SetCreateMana(28 + 100 * petlevel);
                    SetBonusDamage(int32(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 0.4f * 1.5f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 30 * 1.5f));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 30 * 1.5f));
                    break;
                }
                case 19668: // Shadowfiend
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petlevel);
                        SetCreateHealth(28 + 30*petlevel);
                    }

                    int32 bonus_dmg = (int32(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SHADOW)));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petlevel * 4 - petlevel) + bonus_dmg));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petlevel * 4 + petlevel) + bonus_dmg));

                    // Glyph of the Sha
                    if (m_owner->HasAura(147776))
                    {
                        SetObjectScale(0.2f);
                        SetDisplayId(41966);
                    }

                    break;
                }
                case 55659: // Wild Imp
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10 * petlevel);
                        SetCreateHealth(28 + 30 * petlevel);
                    }

                    SetBonusDamage(int32(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FIRE)));

                    break;
                }
                case 63508: // Xuen, the White Tiger
                {
                    if (GetOwner()->GetTypeId() != TYPEID_PLAYER)
                        break;

                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petlevel);
                        SetCreateHealth(28 + 30*petlevel);
                    }

                    SetBonusDamage(int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK)));
                    int32 bonus_dmg = (int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.25f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 4 - petlevel + bonus_dmg));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 4 + petlevel + bonus_dmg));
                    SetAttackTime(BASE_ATTACK, 1 * IN_MILLISECONDS);

                    float crit_chance = 5.0f;
                    crit_chance += GetOwner()->GetFloatValue(PLAYER_CRIT_PERCENTAGE);
                    crit_chance += GetOwner()->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, SPELL_SCHOOL_MASK_NORMAL);
                    m_baseSpellCritChance = crit_chance;

                    m_modMeleeHitChance = float(GetOwner()->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE));
                    m_modMeleeHitChance += GetOwner()->ToPlayer()->GetRatingBonusValue(CR_HIT_MELEE);
                    m_modSpellHitChance = m_modMeleeHitChance;

                    break;
                }
                case 61994: // Murder of Crows
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petlevel);
                        SetCreateHealth(28 + 30*petlevel);
                    }

                    int32 bonus_dmg = (int32(GetOwner()->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.1f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 4 - petlevel + bonus_dmg));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 4 + petlevel + bonus_dmg));

                    break;
                }
                case 62005: // Dire Beast - Dungeons
                case 62210: // Dire Beast - Valley of the Four Winds
                case 62855: // Dire Beast - Kalimdor
                case 62856: // Dire Beast - Eastern Kingdoms
                case 62857: // Dire Beast - Outland
                case 62858: // Dire Beast - Northrend
                case 62860: // Dire Beast - Krasarang Wilds
                case 62865: // Dire Beast - Jade Forest
                case 64617: // Dire Beast - Vale of Eternal Blossoms
                case 64618: // Dire Beast - Kun-Lai Summit
                case 64619: // Dire Beast - Townlong Steppes
                case 64620: // Dire Beast - Dread Wastes
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petlevel);
                        SetCreateHealth(28 + 30*petlevel);
                    }

                    int32 bonus_dmg = (int32(GetOwner()->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.2f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 4 - petlevel + bonus_dmg));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 4 + petlevel + bonus_dmg));

                    break;
                }
                case 59190: // Psyfiend
                {
                    SetCreateHealth(m_owner->CountPctFromMaxHealth(10));
                    break;
                }
                case 65282: // Void Tendrils
                {
                    SetCreateHealth(m_owner->CountPctFromMaxHealth(20));
                    break;
                }
                case 62982: // Mindbender
                case 67236: // Mindbender (Sha)
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petlevel);
                        SetCreateHealth(28 + 30*petlevel);
                    }

                    // Mindbender deals 80% more damage than Shadowfiend
                    int32 bonus_dmg = (int32(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SHADOW)));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(((petlevel * 4 - petlevel) + bonus_dmg) * 1.8f));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(((petlevel * 4 + petlevel) + bonus_dmg) * 1.8f));

                    // Glyph of the Sha
                    if (m_owner->HasAura(147776))
                    {
                        SetObjectScale(0.2f);
                        SetDisplayId(41966);
                    }
                    
                    break;
                }
                case 19833: // Snake Trap - Venomous Snake
                {
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petlevel / 2) - 25));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petlevel / 2) - 18));
                    break;
                }
                case 19921: // Snake Trap - Viper
                {
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel / 2 - 10));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel / 2));
                    break;
                }
                case 58488: // Feral Spirit (Symbiosis)
                case 29264: // Feral Spirit
                {
                    SetCreateHealth(uint32(GetOwner()->GetMaxHealth() / 3.7f));

                    SetBonusDamage(int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f));

                    // 14AP == 1dps, wolf's strike speed == 2s so dmg = basedmg + AP / 14 * 2
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petlevel * 4 - petlevel) + (GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f * 2 / 14)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petlevel * 4 + petlevel) + (GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f * 2 / 14)));

                    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(GetOwner()->GetArmor()) * 0.35f);  //  Bonus Armor (35% of player armor)
                    SetModifierValue(UNIT_MOD_STAT_STAMINA, BASE_VALUE, float(GetOwner()->GetStat(STAT_STAMINA)) * 0.3f);  //  Bonus Stamina (30% of player stamina)
                    break;
                }
                case 31216: // Mirror Image
                {
                    // Stolen Mirror Images should have mage display id instead of dkay
                    if (GetOwner()->GetSimulacrumTarget())
                        SetDisplayId(GetOwner()->GetSimulacrumTarget()->GetDisplayId());
                    else
                        SetDisplayId(GetOwner()->GetDisplayId());
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 30*petlevel);
                        SetCreateHealth(28 + 10*petlevel);
                    }
                    // Sequence is important!
                    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
                    // here mirror image casts on summoner spell (not present in client dbc) 49866
                    // here should be auras (not present in client dbc): 35657, 35658, 35659, 35660 selfcasted by mirror images (stats related?)
                    // Clone Me!
                    GetOwner()->CastSpell(this, 45204, true);
                    GetOwner()->CastSpell(this, 41055, true);

                    SetBonusDamage(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FROST));
                    break;
                }
                case ENTRY_GARGOYLE:
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petlevel);
                        SetCreateHealth(28 + 30*petlevel);
                    }

                    // Convert Owner's haste into the Gargoyle spell haste
                    float ownerHaste = 1.0f + GetOwner()->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HASTE_MELEE)
                            * GetOwner()->ToPlayer()->GetRatingMultiplier(CR_HASTE_MELEE) / 100.0f;
                    ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED, ownerHaste, false);

                    // also make gargoyle benefit from haste auras, like unholy presence
                    int meleeHaste = GetOwner()->GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_HASTE);
                    ApplyCastTimePercentMod(meleeHaste, true);

                    SetBonusDamage(int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));
                    break;
                }
                case ENTRY_BLOODWORM:
                {
                    // In Mists of Pandaria, each Bloodworm receives exactly 15% of it's master's current health on spawn.
                    SetCreateHealth(GetOwner()->CountPctFromMaxHealth(15));
                    SetBonusDamage(int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.006f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - 30 - (petlevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel - 30 + (petlevel / 4)));

                    break;
                }
                case 59764: // Healing Tide Totem
                {
                    SetCreateHealth(GetOwner()->CountPctFromMaxHealth(10));
                    break;
                }
                // Guardian of Ancient Kings
                case 46506:
                {
                    if (Player* pOwner = GetOwner()->ToPlayer())
                    {
                        m_modMeleeHitChance = pOwner->GetFloatValue(PLAYER_FIELD_UI_HIT_MODIFIER) + pOwner->GetRatingBonusValue(CR_HIT_MELEE);
                        m_baseSpellCritChance = pOwner->GetFloatValue(PLAYER_CRIT_PERCENTAGE) + pOwner->GetRatingBonusValue(CR_HIT_SPELL);
                    }

                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 0.75f * GetOwner()->GetFloatValue(UNIT_FIELD_MINDAMAGE));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 0.75f * GetOwner()->GetFloatValue(UNIT_FIELD_MAXDAMAGE));
                    break;
                }
                default:
                    break;
            }
            break;
        }
    }

    UpdateAllStats();

    SetFullHealth();
    if (GetOwner() && GetOwner()->getClass() == CLASS_WARLOCK)
        SetPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));
    else
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    return true;
}

bool Pet::HaveInDiet(ItemTemplate const* item) const
{
    if (item->SubClass != ITEM_SUBCLASS_FOOD_DRINK)
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return false;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if (!cFamily)
        return false;

    uint32 diet = cFamily->petFoodMask;
    uint32 FoodMask = 1 << (item->FoodType-1);
    return diet & FoodMask;
}

uint32 Pet::GetCurrentFoodBenefitLevel(uint32 itemlevel)
{
    // -5 or greater food level
    if (getLevel() <= itemlevel + 5)                         //possible to feed level 60 pet with level 55 level food for full effect
        return 35000;
    // -10..-6
    else if (getLevel() <= itemlevel + 10)                   //pure guess, but sounds good
        return 17000;
    // -14..-11
    else if (getLevel() <= itemlevel + 14)                   //level 55 food gets green on 70, makes sense to me
        return 8000;
    // -15 or less
    else
        return 0;                                           //food too low level
}

void Pet::_LoadSpellCooldowns()
{
    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_SPELL_COOLDOWN);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    ByteBuffer dataBuffer;
    time_t curTime = time(NULL);

    do
    {
        Field* fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();
        time_t db_time  = time_t(fields[1].GetUInt32());

        if (!sSpellMgr->GetSpellInfo(spell_id))
        {
            TC_LOG_ERROR("entities.pet", "Pet %u have unknown spell %u in `pet_spell_cooldown`, skipping.", m_charmInfo->GetPetNumber(), spell_id);
            continue;
        }

        // skip outdated cooldown
        if (db_time <= curTime)
            continue;

        dataBuffer << uint32(spell_id);
        dataBuffer << uint32((db_time - curTime) * IN_MILLISECONDS);

        _AddCreatureSpellCooldown(spell_id, db_time);

        TC_LOG_DEBUG("entities.pet", "Pet (Number: %u) spell %u cooldown loaded (%u secs).", m_charmInfo->GetPetNumber(), spell_id, uint32(db_time-curTime));
    }
    while (result->NextRow());

    if (!dataBuffer.empty())
    {
        ObjectGuid petGuid = GetGUID();

        WorldPacket data(SMSG_SPELL_COOLDOWN, 4 + dataBuffer.size() + 1 + 8);
        data.WriteBits(dataBuffer.size() / sizeof(uint32) / 2, 21);
        data.WriteBit(0);
        data.WriteBitSeq<4, 2, 5, 6, 0, 3, 7, 1>(petGuid);
        data.FlushBits();
        data.append(dataBuffer);
        data.WriteByteSeq<4>(petGuid);
        data << uint8(1);
        data.WriteByteSeq<1, 5, 7, 6, 0, 2, 3>(petGuid);

        GetOwner()->SendDirectMessage(&data);
    }
}

void Pet::_SaveSpellCooldowns(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELL_COOLDOWNS);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    trans->Append(stmt);

    time_t curTime = time(NULL);

    // remove oudated and save active
    for (CreatureSpellCooldowns::iterator itr = m_CreatureSpellCooldowns.begin(); itr != m_CreatureSpellCooldowns.end();)
    {
        if (itr->second <= curTime)
            m_CreatureSpellCooldowns.erase(itr++);
        else
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SPELL_COOLDOWN);
            stmt->setUInt32(0, m_charmInfo->GetPetNumber());
            stmt->setUInt32(1, itr->first);
            stmt->setUInt32(2, uint32(itr->second));
            trans->Append(stmt);

            ++itr;
        }
    }
}

void Pet::_LoadSpells()
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_SPELL);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            addSpell(fields[0].GetUInt32(), ActiveStates(fields[1].GetUInt8()), PETSPELL_UNCHANGED);
        }
        while (result->NextRow());
    }
}

void Pet::_SaveSpells(SQLTransaction& trans)
{
    for (PetSpellMap::iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
    {
        ++next;

        // prevent saving family passives to DB
        if (itr->second.type == PETSPELL_FAMILY)
            continue;

        PreparedStatement* stmt;

        switch (itr->second.state)
        {
            case PETSPELL_REMOVED:
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELL_BY_SPELL);
                stmt->setUInt32(0, m_charmInfo->GetPetNumber());
                stmt->setUInt32(1, itr->first);
                trans->Append(stmt);

                m_spells.erase(itr);
                continue;
            case PETSPELL_CHANGED:
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELL_BY_SPELL);
                stmt->setUInt32(0, m_charmInfo->GetPetNumber());
                stmt->setUInt32(1, itr->first);
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SPELL);
                stmt->setUInt32(0, m_charmInfo->GetPetNumber());
                stmt->setUInt32(1, itr->first);
                stmt->setUInt8(2, itr->second.active);
                trans->Append(stmt);

                break;
            case PETSPELL_NEW:
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SPELL);
                stmt->setUInt32(0, m_charmInfo->GetPetNumber());
                stmt->setUInt32(1, itr->first);
                stmt->setUInt8(2, itr->second.active);
                trans->Append(stmt);
                break;
            case PETSPELL_UNCHANGED:
                continue;
        }
        itr->second.state = PETSPELL_UNCHANGED;
    }
}

void Pet::_SaveActionBar(SQLTransaction& trans, bool newPet)
{
    uint32 petId = GetCharmInfo()->GetPetNumber();

    if (!newPet)
    {
        for (uint8 i = ACTION_BAR_INDEX_START; i != ACTION_BAR_INDEX_END; ++i)
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PET_ACTION_BAR);

            stmt->setUInt8(0, m_charmInfo->GetActionBarEntry(i)->GetType());
            stmt->setUInt32(1, m_charmInfo->GetActionBarEntry(i)->GetAction());
            stmt->setUInt32(2, petId);
            stmt->setUInt8(3, i);

            trans->Append(stmt);
        }
    }
    else
    {
        for (uint8 i = ACTION_BAR_INDEX_START; i != ACTION_BAR_INDEX_END; ++i)
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_ACTION_BAR);

            stmt->setUInt32(0, petId);
            stmt->setUInt8(1, i);
            stmt->setUInt8(2, m_charmInfo->GetActionBarEntry(i)->GetType());
            stmt->setUInt32(3, m_charmInfo->GetActionBarEntry(i)->GetAction());

            trans->Append(stmt);
        }
    }
}

void Pet::_LoadAuras(uint32 timediff)
{
    TC_LOG_DEBUG("entities.pet", "Loading auras for pet %u", GetGUIDLow());

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_AURA);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    auto auraResult = CharacterDatabase.Query(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_AURA_EFFECT);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    auto auraEffectResult = CharacterDatabase.Query(stmt);

    std::vector<auraEffectData> auraEffectList;
    if (auraEffectResult)
    {
        auraEffectList.reserve(auraEffectResult->GetRowCount());
        do
        {
            auto const fields = auraEffectResult->Fetch();

            auto const slot = fields[0].GetUInt8();
            auto const effect = fields[1].GetUInt8();
            auto const amount = fields[2].GetInt32();
            auto const baseamount = fields[3].GetInt32();

            auraEffectList.emplace_back(slot, effect, amount, baseamount);
        }
        while (auraEffectResult->NextRow());
    }

    if (auraResult)
    {
        do
        {
            int32 damage[MAX_SPELL_EFFECTS];
            int32 baseDamage[MAX_SPELL_EFFECTS];

            auto fields = auraResult->Fetch();

            uint8 slot = fields[0].GetUInt8();
            uint64 caster_guid = fields[1].GetUInt64();
            // NULL guid stored - pet is the caster of the spell - see Pet::_SaveAuras
            if (!caster_guid)
                caster_guid = GetGUID();
            uint32 spellid = fields[2].GetUInt32();
            uint32 effmask = fields[3].GetUInt32();
            uint32 recalculatemask = fields[4].GetUInt32();
            uint8 stackcount = fields[5].GetUInt8();
            int32 maxduration = fields[6].GetInt32();
            int32 remaintime = fields[7].GetInt32();
            uint8 remaincharges = fields[8].GetUInt8();

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
            if (!spellInfo)
            {
                TC_LOG_ERROR("entities.pet", "Unknown aura (spellid %u), ignore.", spellid);
                continue;
            }

            // negative effects should continue counting down after logout
            if (remaintime != -1 && !spellInfo->IsPositive())
            {
                if (remaintime / IN_MILLISECONDS <= int32(timediff))
                    continue;

                remaintime -= timediff * IN_MILLISECONDS;
            }

            // prevent wrong values of remaincharges
            if (spellInfo->ProcCharges)
            {
                if (remaincharges <= 0 || remaincharges > spellInfo->ProcCharges)
                    remaincharges = spellInfo->ProcCharges;
            }
            else
                remaincharges = 0;

            for (auto const &effectData : auraEffectList)
            {
                if (effectData._slot == slot)
                {
                    damage[effectData._effect] = effectData._amount;
                    baseDamage[effectData._effect] = effectData._baseamount;
                }
            }

            Aura *aura = Aura::TryCreate(spellInfo, effmask, this, NULL, &spellInfo->spellPower, &baseDamage[0], NULL, caster_guid);
            if (aura != NULL)
            {
                if (!aura->CanBeSaved())
                {
                    aura->Remove();
                    continue;
                }
                aura->SetLoadedState(maxduration, remaintime, remaincharges, stackcount, recalculatemask, &damage[0]);
                aura->ApplyForTargets();
                TC_LOG_INFO("entities.pet", "Added aura spellid %u, effectmask %u", spellInfo->Id, effmask);
            }
        }
        while (auraResult->NextRow());
    }
}

void Pet::_SaveAuras(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_AURAS);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_AURA_EFFECTS);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    trans->Append(stmt);

    for (AuraMap::const_iterator itr = m_ownedAuras.begin(); itr != m_ownedAuras.end(); ++itr)
    {
        // check if the aura has to be saved
        if (!itr->second->CanBeSaved() || IsPetAura(itr->second))
            continue;

        Aura *aura = itr->second;
        AuraApplication *foundAura = aura->GetApplicationOfTarget(GetGUID());
        if (!foundAura)
            continue;

        uint32 effMask = 0;
        uint32 recalculateMask = 0;
        uint8 index = 0;
        for (uint8 i = 0; i < aura->GetSpellInfo()->Effects.size(); ++i)
        {
            if (auto const auraEffect = aura->GetEffect(i))
            {
                index = 0;
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_AURA_EFFECT);
                stmt->setUInt32(index++, m_charmInfo->GetPetNumber());
                stmt->setUInt8(index++, foundAura->GetSlot());
                stmt->setUInt8(index++, i);
                stmt->setInt32(index++, auraEffect->GetBaseAmount());
                stmt->setInt32(index++, auraEffect->GetAmount());

                trans->Append(stmt);

                effMask |= (1 << i);
                if (auraEffect->CanBeRecalculated())
                    recalculateMask |= (1 << i);
            }
        }

        // don't save guid of caster in case we are caster of the spell - guid for pet is generated every pet load, so it won't match saved guid anyways
        uint64 casterGUID = (aura->GetCasterGUID() == GetGUID()) ? 0 : aura->GetCasterGUID();

        index = 0;
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_AURA);
        stmt->setUInt32(index++, m_charmInfo->GetPetNumber());
        stmt->setUInt8(index++, foundAura->GetSlot());
        stmt->setUInt64(index++, casterGUID);
        stmt->setUInt32(index++, aura->GetId());
        stmt->setUInt32(index++, effMask);
        stmt->setUInt32(index++, recalculateMask);
        stmt->setUInt8(index++, aura->GetStackAmount());
        stmt->setInt32(index++, aura->GetMaxDuration());
        stmt->setInt32(index++, aura->GetDuration());
        stmt->setUInt8(index++, aura->GetCharges());

        trans->Append(stmt);
    }
}

bool Pet::addSpell(uint32 spellId, ActiveStates active /*= ACT_DECIDE*/, PetSpellState state /*= PETSPELL_NEW*/, PetSpellType type /*= PETSPELL_NORMAL*/)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
    {
        // do pet spell book cleanup
        if (state == PETSPELL_UNCHANGED)                    // spell load case
        {
            TC_LOG_ERROR("entities.pet", "Pet::addSpell: Non-existed in SpellStore spell #%u request, deleting for all pets in `pet_spell`.", spellId);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_PET_SPELL);

            stmt->setUInt32(0, spellId);

            CharacterDatabase.Execute(stmt);
        }
        else
            TC_LOG_ERROR("entities.pet", "Pet::addSpell: Non-existed in SpellStore spell #%u request.", spellId);

        return false;
    }

    PetSpellMap::iterator itr = m_spells.find(spellId);
    if (itr != m_spells.end())
    {
        if (itr->second.state == PETSPELL_REMOVED)
        {
            m_spells.erase(itr);
            state = PETSPELL_CHANGED;
        }
        else if (state == PETSPELL_UNCHANGED && itr->second.state != PETSPELL_UNCHANGED)
        {
            // can be in case spell loading but learned at some previous spell loading
            itr->second.state = PETSPELL_UNCHANGED;

            if (active == ACT_ENABLED)
                ToggleAutocast(spellInfo, true);
            else if (active == ACT_DISABLED)
                ToggleAutocast(spellInfo, false);

            return false;
        }
        else
            return false;
    }

    PetSpell newspell;
    newspell.state = state;
    newspell.type = type;

    if (active == ACT_DECIDE)                               // active was not used before, so we save it's autocast/passive state here
    {
        if (spellInfo->IsAutocastable())
            newspell.active = ACT_DISABLED;
        else
            newspell.active = ACT_PASSIVE;
    }
    else
        newspell.active = active;

    if (spellInfo->IsRanked())
    {
        for (PetSpellMap::const_iterator itr2 = m_spells.begin(); itr2 != m_spells.end(); ++itr2)
        {
            if (itr2->second.state == PETSPELL_REMOVED)
                continue;

            SpellInfo const* oldRankSpellInfo = sSpellMgr->GetSpellInfo(itr2->first);

            if (!oldRankSpellInfo)
                continue;

            if (spellInfo->IsDifferentRankOf(oldRankSpellInfo))
            {
                // replace by new high rank
                if (spellInfo->IsHighRankOf(oldRankSpellInfo))
                {
                    newspell.active = itr2->second.active;

                    if (newspell.active == ACT_ENABLED)
                        ToggleAutocast(oldRankSpellInfo, false);

                    unlearnSpell(itr2->first, false, false);
                    break;
                }
                // ignore new lesser rank
                else
                    return false;
            }
        }
    }

    m_spells[spellId] = newspell;

    if (spellInfo->IsPassive() && (!spellInfo->CasterAuraState || HasAuraState(AuraStateType(spellInfo->CasterAuraState))))
        CastSpell(this, spellId, true);
    else
        m_charmInfo->AddSpellToActionBar(spellInfo);

    if (newspell.active == ACT_ENABLED)
        ToggleAutocast(spellInfo, true);

    return true;
}

bool Pet::learnSpell(uint32 spell_id)
{
    if (IsPetGhoul())
        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spell_id))
            if (!info->IsPassive() && (spell_id != 47481 && spell_id != 47468 && spell_id != 47482 && spell_id != 47484))
                return false;

    // prevent duplicated entires in spell book
    if (!addSpell(spell_id))
        return false;

    if (!m_loading)
    {
        WorldPacket data(SMSG_PET_LEARNED_SPELL, 4);
        data.WriteBits(1, 22);
        data << uint32(spell_id);
        GetOwner()->GetSession()->SendPacket(&data);
        GetOwner()->PetSpellInitialize();
    }
    return true;
}

void Pet::InitLevelupSpellsForLevel()
{
    uint8 level = getLevel();

    if (PetLevelupSpellSet const* levelupSpells = GetCreatureTemplate()->family ? sSpellMgr->GetPetLevelupSpellList(GetCreatureTemplate()->family) : NULL)
    {
        // PetLevelupSpellSet ordered by levels, process in reversed order
        for (PetLevelupSpellSet::const_reverse_iterator itr = levelupSpells->rbegin(); itr != levelupSpells->rend(); ++itr)
        {
            // will called first if level down
            if (itr->first > level)
                unlearnSpell(itr->second, true);                 // will learn prev rank if any
            // will called if level up
            else
                learnSpell(itr->second);                        // will unlearn prev rank if any
        }
    }

    int32 petSpellsId = GetCreatureTemplate()->PetSpellDataId ? -(int32)GetCreatureTemplate()->PetSpellDataId : GetEntry();

    // default spells (can be not learned if pet level (as owner level decrease result for example) less first possible in normal game)
    if (PetDefaultSpellsEntry const* defSpells = sSpellMgr->GetPetDefaultSpellsEntry(petSpellsId))
    {
        for (uint8 i = 0; i < MAX_CREATURE_SPELL_DATA_SLOT; ++i)
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(defSpells->spellid[i]);
            if (!spellEntry)
                continue;

            // will called first if level down
            if (spellEntry->SpellLevel > level)
                unlearnSpell(spellEntry->Id, true);
            // will called if level up
            else
                learnSpell(spellEntry->Id);
        }
    }
}

bool Pet::unlearnSpell(uint32 spell_id, bool learn_prev, bool clear_ab)
{
    if (removeSpell(spell_id, learn_prev, clear_ab))
    {
        if (!m_loading)
        {
            WorldPacket data(SMSG_PET_REMOVED_SPELL, 4);
            data.WriteBits(1, 22);
            data << uint32(spell_id);
            GetOwner()->GetSession()->SendPacket(&data);
        }
        return true;
    }
    return false;
}

bool Pet::removeSpell(uint32 spell_id, bool learn_prev, bool clear_ab)
{
    PetSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return false;

    if (itr->second.state == PETSPELL_REMOVED)
        return false;

    if (itr->second.state == PETSPELL_NEW)
        m_spells.erase(itr);
    else
        itr->second.state = PETSPELL_REMOVED;

    RemoveAurasDueToSpell(spell_id);

    if (learn_prev)
    {
        if (uint32 prev_id = sSpellMgr->GetPrevSpellInChain (spell_id))
            learnSpell(prev_id);
        else
            learn_prev = false;
    }

    // if remove last rank or non-ranked then update action bar at server and client if need
    if (clear_ab && !learn_prev && m_charmInfo->RemoveSpellFromActionBar(spell_id))
    {
        if (!m_loading)
        {
            // need update action bar for last removed rank
            if (Unit* owner = GetOwner())
                if (owner->GetTypeId() == TYPEID_PLAYER)
                    owner->ToPlayer()->PetSpellInitialize();
        }
    }

    return true;
}

void Pet::CleanupActionBar()
{
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        if (UnitActionBarEntry const* ab = m_charmInfo->GetActionBarEntry(i))
            if (ab->GetAction() && ab->IsActionBarForSpell())
            {
                if (!HasSpell(ab->GetAction()))
                    m_charmInfo->SetActionBar(i, 0, ACT_PASSIVE);
                else if (ab->GetType() == ACT_ENABLED)
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ab->GetAction()))
                        ToggleAutocast(spellInfo, true);
                }
            }
}

void Pet::InitPetCreateSpells()
{
    m_charmInfo->InitPetActionBar();
    m_spells.clear();

    LearnPetPassives();
    InitLevelupSpellsForLevel();

    CastPetAuras(false);
}

void Pet::ToggleAutocast(SpellInfo const* spellInfo, bool apply)
{
    if (!spellInfo->IsAutocastable())
        return;

    uint32 spellid = spellInfo->Id;

    PetSpellMap::iterator itr = m_spells.find(spellid);
    if (itr == m_spells.end())
        return;

    uint32 i;

    if (apply)
    {
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i)
            ;                                               // just search

        if (i == m_autospells.size())
        {
            m_autospells.push_back(spellid);

            if (itr->second.active != ACT_ENABLED)
            {
                itr->second.active = ACT_ENABLED;
                if (itr->second.state != PETSPELL_NEW)
                    itr->second.state = PETSPELL_CHANGED;
            }
        }
    }
    else
    {
        AutoSpellList::iterator itr2 = m_autospells.begin();
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i, ++itr2)
            ;                                               // just search

        if (i < m_autospells.size())
        {
            m_autospells.erase(itr2);
            if (itr->second.active != ACT_DISABLED)
            {
                itr->second.active = ACT_DISABLED;
                if (itr->second.state != PETSPELL_NEW)
                    itr->second.state = PETSPELL_CHANGED;
            }
        }
    }
}

bool Pet::IsPermanentPetFor(Player* owner)
{
    switch (getPetType())
    {
        case SUMMON_PET:
            switch (owner->getClass())
            {
                case CLASS_WARLOCK:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_DEMON;
                case CLASS_DEATH_KNIGHT:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_UNDEAD;
                case CLASS_MAGE:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_ELEMENTAL;
                default:
                    return false;
            }
        case HUNTER_PET:
            return true;
        default:
            return false;
    }
}

bool Pet::Create(uint32 guidlow, Map* map, uint32 phaseMask, uint32 Entry, uint32 pet_number)
{
    ASSERT(map);
    SetMap(map);

    SetPhaseMask(phaseMask, false);
    Object::_Create(guidlow, pet_number, HIGHGUID_PET);

    m_DBTableGuid = guidlow;
    m_originalEntry = Entry;

    if (!InitEntry(Entry))
        return false;

    SetSheath(SHEATH_STATE_MELEE);

    return true;
}

bool Pet::HasSpell(uint32 spell) const
{
    PetSpellMap::const_iterator itr = m_spells.find(spell);
    return itr != m_spells.end() && itr->second.state != PETSPELL_REMOVED;
}

// Get all passive spells in our skill line
void Pet::LearnPetPassives()
{
    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if (!cFamily)
        return;

    PetFamilySpellsStore::const_iterator petStore = sPetFamilySpellsStore.find(cFamily->ID);
    if (petStore != sPetFamilySpellsStore.end())
    {
        // For general hunter pets skill 270
        // Passive 01~10, Passive 00 (20782, not used), Ferocious Inspiration (34457)
        // Scale 01~03 (34902~34904, bonus from owner, not used)
        for (PetFamilySpellsSet::const_iterator petSet = petStore->second.begin(); petSet != petStore->second.end(); ++petSet)
            addSpell(*petSet, ACT_DECIDE, PETSPELL_NEW, PETSPELL_FAMILY);
    }
}

void Pet::CastPetAuras(bool current)
{
    Player * const owner = GetOwner();
    if (!owner)
        return;

    if (!IsPermanentPetFor(owner))
        return;

    PetAuraSet const &petAuras = owner->GetPetAuras();
    for (PetAuraSet::const_iterator itr = petAuras.begin(); itr != petAuras.end();)
    {
        PetAura const* pa = *itr;
        ++itr;

        if (!current && pa->IsRemovedOnChangePet())
            owner->RemovePetAura(pa);
        else
            CastPetAura(pa);
    }
}

void Pet::CastPetAura(PetAura const* aura)
{
    uint32 auraId = aura->GetAura(GetEntry());
    if (!auraId)
        return;

    CastSpell(this, auraId, true);
}

bool Pet::IsPetAura(Aura const *aura) const
{
    auto const owner = GetOwner();
    if (!owner)
        return false;

    // if the owner has that pet aura, return true
    PetAuraSet const &petAuras = owner->GetPetAuras();
    for (PetAuraSet::const_iterator itr = petAuras.begin(); itr != petAuras.end(); ++itr)
    {
        if ((*itr)->GetAura(GetEntry()) == aura->GetId())
            return true;
    }
    return false;
}

void Pet::learnSpellHighRank(uint32 spellid)
{
    learnSpell(spellid);

    if (uint32 next = sSpellMgr->GetNextSpellInChain(spellid))
        learnSpellHighRank(next);
}

void Pet::SynchronizeLevelWithOwner()
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    switch (getPetType())
    {
        // always same level
        case SUMMON_PET:
            GivePetLevel(owner->getLevel());
            break;
        // always same level since 4.1.0
        case HUNTER_PET:
            GivePetLevel(owner->getLevel());
            break;
        default:
            break;
    }
}

void Pet::LearnSpecializationSpell()
{
    for (uint32 i = 0; i < sSpecializationSpellStore.GetNumRows(); i++)
    {
        SpecializationSpellEntry const* specializationEntry = sSpecializationSpellStore.LookupEntry(i);
        if (!specializationEntry)
            continue;

        if (specializationEntry->SpecializationEntry != GetSpecializationId())
            continue;

        learnSpell(specializationEntry->LearnSpell);
    }
}

void Pet::UnlearnSpecializationSpell()
{
    for (uint32 i = 0; i < sSpecializationSpellStore.GetNumRows(); i++)
    {
        SpecializationSpellEntry const* specializationEntry = sSpecializationSpellStore.LookupEntry(i);
        if (!specializationEntry)
            continue;

        if (specializationEntry->SpecializationEntry != GetSpecializationId())
            continue;

        unlearnSpell(specializationEntry->LearnSpell, false);
    }
}

void Pet::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{
    ByteBuffer dataBuffer;
    time_t curTime = time(NULL);

    for (PetSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 unSpellId = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(unSpellId);
        if (!spellInfo)
        {
            ASSERT(spellInfo);
            continue;
        }

        // Not send cooldown for this spells
        if (spellInfo->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE)
            continue;

        if (spellInfo->PreventionType != SPELL_PREVENTION_TYPE_SILENCE)
            continue;

        if ((idSchoolMask & spellInfo->GetSchoolMask()) && GetCreatureSpellCooldownDelay(unSpellId) < unTimeMs)
        {
            dataBuffer << uint32(unSpellId);
            dataBuffer << uint32(unTimeMs);                       // in m.secs
            _AddCreatureSpellCooldown(unSpellId, curTime + unTimeMs/IN_MILLISECONDS);
        }
    }

    if (!dataBuffer.empty())
    {
        ObjectGuid petGuid = GetGUID();

        WorldPacket data(SMSG_SPELL_COOLDOWN, 4 + dataBuffer.size() + 1 + 8);
        data.WriteBits(dataBuffer.size() / sizeof(uint32) / 2, 21);
        data.WriteBit(0);
        data.WriteBitSeq<4, 2, 5, 6, 0, 3, 7, 1>(petGuid);
        data.FlushBits();
        data.append(dataBuffer);
        data.WriteByteSeq<4>(petGuid);
        data << uint8(0);
        data.WriteByteSeq<1, 5, 7, 6, 0, 2, 3>(petGuid);

        GetOwner()->SendDirectMessage(&data);
    }
}

void Pet::AssignToSlot(uint8 slot, SQLTransaction trans)
{
    // We must ensure that:
    // - this pet is not assigned to any other slot
    // - this slot is not occupied by any other pet

    bool internalTransaction = false;
    if (!trans)
    {
        trans = CharacterDatabase.BeginTransaction();
        internalTransaction = true;
    }

    PreparedStatement* stmt;
    uint32 petId = GetCharmInfo()->GetPetNumber();
    uint32 ownerGuid = GetOwner()->GetGUIDLow();

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SLOT_BY_PET_ID);
    stmt->setUInt32(0, petId);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SLOT_BY_OWNER);
    stmt->setUInt32(0, ownerGuid);
    stmt->setUInt8(1, slot);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SLOT);
    stmt->setUInt32(0, petId);
    stmt->setUInt8(1, slot);
    trans->Append(stmt);

    if (internalTransaction)
        CharacterDatabase.CommitTransaction(trans);

    GetOwner()->SetPetSlotUsed(GetCharmInfo()->GetPetNumber(), slot);
}
