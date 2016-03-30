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

#ifndef PET_H
#define PET_H

#include "PetDefines.h"
#include "TemporarySummon.h"

#include <unordered_map>

struct PetSpell
{
    ActiveStates active;
    PetSpellState state;
    PetSpellType type;
};

typedef std::unordered_map<uint32, PetSpell> PetSpellMap;
typedef std::vector<uint32> AutoSpellList;

#define PET_FOCUS_REGEN_INTERVAL 4 * IN_MILLISECONDS

class Player;

class Pet : public Guardian
{
    public:
        explicit Pet(Player* owner, PetType type = MAX_PET_TYPE);
        virtual ~Pet();

        void AddToWorld();
        void RemoveFromWorld();

        void AssignToSlot(uint8 slot, SQLTransaction trans = SQLTransaction());

        PetType getPetType() const { return m_petType; }
        bool isWarlockPet() const { return m_isWarlockPet; }
        void setPetType(PetType type) { m_petType = type; }
        bool isControlled() const { return (getPetType() == SUMMON_PET || getPetType() == HUNTER_PET) && !isStampeded(); }
        bool isTemporarySummoned() const { return m_duration > 0; }
        bool isStampeded() const { return m_Stampeded; }

        bool IsPermanentPetFor(Player* owner);              // pet have tab in character windows and set UNIT_FIELD_PETNUMBER

        bool Create(uint32 guidlow, Map* map, uint32 phaseMask, uint32 Entry, uint32 pet_number);
        bool CreateBaseAtCreature(Creature* creature);
        bool CreateBaseAtCreatureInfo(CreatureTemplate const* cinfo, Unit* owner);
        bool CreateBaseAtTamed(CreatureTemplate const* cinfo, Map* map, uint32 phaseMask);
        bool LoadPetFromDB(PetLoadMode loadMode, uint32 value);
        bool isBeingLoaded() const { return m_loading;}
        void SavePetToDB(bool newPet = false, SQLTransaction trans = SQLTransaction());
        void DeletePetFromDB();

        void Remove(PetRemoveMode mode, uint8 removeFlags = PET_REMOVE_FLAG_NONE);

        void setDeathState(DeathState s);                   // overwrite virtual Creature::setDeathState and Unit::setDeathState
        void Update(uint32 diff);                           // overwrite virtual Creature::Update and Unit::Update

        uint8 GetPetAutoSpellSize() const { return m_autospells.size(); }
        uint32 GetPetAutoSpellOnPos(uint8 pos) const
        {
            if (pos >= m_autospells.size())
                return 0;
            else
                return m_autospells[pos];
        }

        void GivePetXP(uint32 xp);
        void GivePetLevel(uint8 level);
        void SynchronizeLevelWithOwner();
        bool HaveInDiet(ItemTemplate const* item) const;
        uint32 GetCurrentFoodBenefitLevel(uint32 itemlevel);
        void SetDuration(int32 dur) { m_duration = dur; }
        int32 GetDuration() { return m_duration; }

        /*
        bool UpdateStats(Stats stat);
        bool UpdateAllStats();
        void UpdateResistances(uint32 school);
        void UpdateArmor();
        void UpdateMaxHealth();
        void UpdateMaxPower(Powers power);
        void UpdateAttackPowerAndDamage(bool ranged = false);
        void UpdateDamagePhysical(WeaponAttackType attType);
        */

        void ToggleAutocast(SpellInfo const* spellInfo, bool apply);

        bool HasSpell(uint32 spell) const;

        void LearnPetPassives();
        void CastPetAuras(bool current);
        void CastPetAura(PetAura const* aura);
        bool IsPetAura(Aura const *aura) const;

        void _LoadSpellCooldowns();
        void _SaveSpellCooldowns(SQLTransaction& trans);
        void _LoadAuras(uint32 timediff);
        void _SaveAuras(SQLTransaction& trans);
        void _LoadSpells();
        void _SaveSpells(SQLTransaction& trans);
        void _SaveActionBar(SQLTransaction& trans, bool newPet);

        bool addSpell(uint32 spellId, ActiveStates active = ACT_DECIDE, PetSpellState state = PETSPELL_NEW, PetSpellType type = PETSPELL_NORMAL);
        bool learnSpell(uint32 spell_id);
        void learnSpellHighRank(uint32 spellid);
        void InitLevelupSpellsForLevel();
        bool unlearnSpell(uint32 spell_id, bool learn_prev, bool clear_ab = true);
        bool removeSpell(uint32 spell_id, bool learn_prev, bool clear_ab = true);
        void CleanupActionBar();
        virtual void ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs);

        PetSpellMap     m_spells;
        AutoSpellList   m_autospells;
        bool m_isWarlockPet;
        bool m_Stampeded;

        void InitPetCreateSpells();

        uint64 GetAuraUpdateMaskForRaid() const { return m_auraRaidUpdateMask; }
        void SetAuraUpdateMaskForRaid(uint8 slot) { m_auraRaidUpdateMask |= (uint64(1) << slot); }
        void ResetAuraUpdateMaskForRaid() { m_auraRaidUpdateMask = 0; }

        DeclinedName const* GetDeclinedNames() const { return m_declinedname; }

        bool    m_removed;                                  // prevent overwrite pet state in DB at next Pet::Update if pet already removed(saved)

        Player * GetOwner() { return Minion::GetOwner()->ToPlayer(); }

        Player const * GetOwner() const { return Minion::GetOwner()->ToPlayer(); }

        uint32 GetSpecializationId() const { return m_specialization; }
        void SetSpecializationId(uint32 id) { m_specialization = id; }
        void LearnSpecializationSpell();
        void UnlearnSpecializationSpell();

    private:
        void Regenerate(Powers power);

        PetType m_petType;
        int32   m_duration;                                 // time until unsummon (used mostly for summoned guardians and not used for controlled pets)
        uint64  m_auraRaidUpdateMask;
        bool    m_loading;
        uint32  m_regenTimer;
        uint32  m_specialization;

        DeclinedName *m_declinedname;

    private:
        void SaveToDB(uint32, uint32, uint32)               // override of Creature::SaveToDB     - must not be called
        {
            ASSERT(false);
        }
        void DeleteFromDB()                                 // override of Creature::DeleteFromDB - must not be called
        {
            ASSERT(false);
        }
};
#endif
