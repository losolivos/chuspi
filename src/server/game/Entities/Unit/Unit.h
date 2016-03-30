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

#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "Object.h"
#include "Opcodes.h"
#include "SpellAuraDefines.h"
#include "UpdateFields.h"
#include "SharedDefines.h"
#include "ThreatManager.h"
#include "HostileRefManager.h"
#include "FollowerReference.h"
#include "FollowerRefManager.h"
#include "EventProcessor.h"
#include "MotionMaster.h"
#include "DBCStructure.h"
#include "SpellInfo.h"
#include "Path.h"
#include "Timer.h"
#include "UnitDefines.hpp"
#include "../AreaTrigger/AreaTrigger.h"
#include "Util.h"

#include <array>
#include <list>
#include <type_traits>
#include <unordered_map>

class SpellCastTargets;
class WorldPacket;

typedef std::pair<SpellValueMod, int32>     CustomSpellValueMod;
class CustomSpellValues : public std::vector<CustomSpellValueMod>
{
    public:
        void AddSpellMod(SpellValueMod mod, int32 value)
        {
            push_back(std::make_pair(mod, value));
        }
};

struct FactionTemplateEntry;
struct SpellValue;

class AuraApplication;
class Aura;
class UnitAura;
class AuraEffect;
class Creature;
class Spell;
class SpellInfo;
class DynamicObject;
class AreaTrigger;
class GameObject;
class Item;
class Pet;
class PetAura;
class Minion;
class Guardian;
class UnitAI;
class Totem;
class Transport;
class Vehicle;
class TransportBase;

typedef std::list<Unit*> UnitList;
typedef std::list< std::pair<Aura*, uint8> > DispelChargesList;

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

typedef std::list<SpellImmune> SpellImmuneList;

extern float baseMoveSpeed[MAX_MOVE_TYPE];
extern float playerBaseMoveSpeed[MAX_MOVE_TYPE];

namespace Movement {
    class MoveSpline;
} // namespace

struct DiminishingReturn
{
    DiminishingReturn(DiminishingGroup group, uint32 t, uint32 count)
        : DRGroup(group), stack(0), hitTime(t), hitCount(count)
    {}

    DiminishingGroup        DRGroup:16;
    uint16                  stack:16;
    uint32                  hitTime;
    uint32                  hitCount;
};

class DispelInfo
{
public:
    explicit DispelInfo(Unit* dispeller, uint32 dispellerSpellId, uint8 chargesRemoved) :
    _dispellerUnit(dispeller), _dispellerSpell(dispellerSpellId), _chargesRemoved(chargesRemoved) {}

    Unit* GetDispeller() const { return _dispellerUnit; }
    uint32 GetDispellerSpellId() const { return _dispellerSpell; }
    uint8 GetRemovedCharges() const { return _chargesRemoved; }
    void SetRemovedCharges(uint8 amount)
    {
        _chargesRemoved = amount;
    }
private:
    Unit* _dispellerUnit;
    uint32 _dispellerSpell;
    uint8 _chargesRemoved;
};

struct CleanDamage
{
    CleanDamage(uint32 mitigated, uint32 absorbed, WeaponAttackType _attackType, MeleeHitOutcome _hitOutCome) :
    absorbed_damage(absorbed), mitigated_damage(mitigated), attackType(_attackType), hitOutCome(_hitOutCome) {}

    uint32 absorbed_damage;
    uint32 mitigated_damage;

    WeaponAttackType attackType;
    MeleeHitOutcome hitOutCome;
};

struct CalcDamageInfo;

class DamageInfo
{
private:
    Unit* const m_attacker;
    Unit* const m_victim;
    uint32 m_damage;
    SpellInfo const* const m_spellInfo;
    SpellSchoolMask const m_schoolMask;
    DamageEffectType const m_damageType;
    WeaponAttackType m_attackType;
    uint32 m_absorb;
    uint32 m_resist;
    uint32 m_block;
public:
    explicit DamageInfo(Unit* _attacker, Unit* _victim, uint32 _damage, SpellInfo const* _spellInfo, SpellSchoolMask _schoolMask, DamageEffectType _damageType);
    explicit DamageInfo(CalcDamageInfo& dmgInfo);

    void ModifyDamage(int32 amount);
    void AbsorbDamage(uint32 amount);
    void ResistDamage(uint32 amount);
    void BlockDamage(uint32 amount);

    Unit* GetAttacker() const { return m_attacker; };
    Unit* GetVictim() const { return m_victim; };
    SpellInfo const* GetSpellInfo() const { return m_spellInfo; };
    SpellSchoolMask GetSchoolMask() const { return m_schoolMask; };
    DamageEffectType GetDamageType() const { return m_damageType; };
    WeaponAttackType GetAttackType() const { return m_attackType; };
    uint32 GetDamage() const { return m_damage; };
    uint32 GetAbsorb() const { return m_absorb; };
    uint32 GetResist() const { return m_resist; };
    uint32 GetBlock() const { return m_block; };
};

class HealInfo
{
private:
    uint32 m_heal;
    uint32 m_absorb;
public:
    explicit HealInfo(uint32 heal)
        : m_heal(heal)
    {
        m_absorb = 0;
    }

    void AbsorbHeal(uint32 amount)
    {
        amount = std::min(amount, GetHeal());
        m_absorb += amount;
        m_heal -= amount;
    }

    uint32 GetHeal() const { return m_heal; };
};

class ProcEventInfo
{
public:
    ProcEventInfo(Unit* actor, Unit* actionTarget, Unit* procTarget, uint32 typeMask,
                  uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask,
                  Spell* spell, DamageInfo* damageInfo, HealInfo* healInfo);

    Unit* GetActor() { return _actor; };
    Unit* GetActionTarget() const { return _actionTarget; }
    Unit* GetProcTarget() const { return _procTarget; }

    uint32 GetTypeMask() const { return _typeMask; }
    uint32 GetSpellTypeMask() const { return _spellTypeMask; }
    uint32 GetSpellPhaseMask() const { return _spellPhaseMask; }
    uint32 GetHitMask() const { return _hitMask; }

    SpellInfo const* GetSpellInfo() const { return _damageInfo->GetSpellInfo(); }
    SpellSchoolMask GetSchoolMask() const { return _damageInfo->GetSchoolMask(); }

    DamageInfo* GetDamageInfo() const { return _damageInfo; }
    HealInfo* GetHealInfo() const { return _healInfo; }

private:
    Unit* const _actor;
    Unit* const _actionTarget;
    Unit* const _procTarget;
    uint32 _typeMask;
    uint32 _spellTypeMask;
    uint32 _spellPhaseMask;
    uint32 _hitMask;
    Spell* _spell;
    DamageInfo* _damageInfo;
    HealInfo* _healInfo;
};

// Struct for use in Unit::CalculateMeleeDamage
// Need create structure like in SMSG_ATTACKER_STATE_UPDATE opcode
struct CalcDamageInfo
{
    Unit  *attacker;             // Attacker
    Unit  *target;               // Target for damage
    uint32 damageSchoolMask;
    uint32 damage;
    uint32 absorb;
    uint32 resist;
    uint32 blocked_amount;
    uint32 HitInfo;
    uint32 TargetState;
// Helper
    WeaponAttackType attackType; //
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx;
    uint32 cleanDamage;          // Used only for rage calculation
    MeleeHitOutcome hitOutCome;  // TODO: remove this field (need use TargetState)
};

// Spell damage info structure based on structure sending in SMSG_SPELL_NON_MELEE_DAMAGE_LOG opcode
struct SpellNonMeleeDamage{
    SpellNonMeleeDamage(Unit* _attacker, Unit* _target, uint32 _SpellID, uint32 _schoolMask)
        : target(_target), attacker(_attacker), SpellID(_SpellID), damage(0), overkill(0), schoolMask(_schoolMask),
        absorb(0), resist(0), physicalLog(false), unused(false), blocked(0), HitInfo(0), cleanDamage(0)
    {}

    Unit   *target;
    Unit   *attacker;
    uint32 SpellID;
    uint32 damage;
    uint32 overkill;
    uint32 schoolMask;
    uint32 absorb;
    uint32 resist;
    bool   physicalLog;
    bool   unused;
    uint32 blocked;
    uint32 HitInfo;
    // Used for help
    uint32 cleanDamage;
};

struct SpellPeriodicAuraLogInfo
{
    SpellPeriodicAuraLogInfo(AuraEffect const *_auraEff, uint32 _damage, uint32 _overDamage, uint32 _absorb, uint32 _resist, float _multiplier, bool _critical)
        : auraEff(_auraEff), damage(_damage), overDamage(_overDamage), absorb(_absorb), resist(_resist), multiplier(_multiplier), critical(_critical){}

    AuraEffect const *auraEff;
    uint32 damage;
    uint32 overDamage;                                      // overkill/overheal
    uint32 absorb;
    uint32 resist;
    float  multiplier;
    bool   critical;
};

uint32 createProcExtendMask(SpellNonMeleeDamage* damageInfo, SpellMissInfo missCondition);

#define MAX_DECLINED_NAME_CASES 5

struct DeclinedName
{
    std::string name[MAX_DECLINED_NAME_CASES];
};

struct GlobalCooldown
{
    explicit GlobalCooldown(uint32 _dur = 0, uint32 _time = 0) : duration(_dur), cast_time(_time) {}

    uint32 duration;
    uint32 cast_time;
};

typedef std::unordered_map<uint32 /*category*/, GlobalCooldown> GlobalCooldownList;

class GlobalCooldownMgr                                     // Shared by Player and CharmInfo
{
public:
    bool HasGlobalCooldown(Unit* caster, SpellInfo const* spellInfo) const;
    void AddGlobalCooldown(SpellInfo const* spellInfo, uint32 gcd);
    void CancelGlobalCooldown(SpellInfo const* spellInfo);
    int32 GetGlobalCooldown(Unit* caster, SpellInfo const* spellInfo);

private:
    GlobalCooldownList m_GlobalCooldowns;
};

struct UnitActionBarEntry
{
    UnitActionBarEntry() : packedData(uint32(ACT_DISABLED) << 24) {}

    uint32 packedData;

    // helper
    ActiveStates GetType() const { return ActiveStates(UNIT_ACTION_BUTTON_TYPE(packedData)); }
    uint32 GetAction() const { return UNIT_ACTION_BUTTON_ACTION(packedData); }
    bool IsActionBarForSpell() const
    {
        ActiveStates Type = GetType();
        return Type == ACT_DISABLED || Type == ACT_ENABLED || Type == ACT_PASSIVE;
    }

    void SetActionAndType(uint32 action, ActiveStates type)
    {
        packedData = MAKE_UNIT_ACTION_BUTTON(action, type);
    }

    void SetType(ActiveStates type)
    {
        packedData = MAKE_UNIT_ACTION_BUTTON(UNIT_ACTION_BUTTON_ACTION(packedData), type);
    }

    void SetAction(uint32 action)
    {
        packedData = (packedData & 0xFF000000) | UNIT_ACTION_BUTTON_ACTION(action);
    }
};

typedef std::list<Player*> SharedVisionList;

typedef UnitActionBarEntry CharmSpellInfo;

struct CharmInfo
{
    public:
        explicit CharmInfo(Unit* unit);
        ~CharmInfo();
        void RestoreState();
        uint32 GetPetNumber() const { return m_petnumber; }
        void SetPetNumber(uint32 petnumber, bool statwindow);

        void SetCommandState(CommandStates st) { m_CommandState = st; }
        CommandStates GetCommandState() const { return m_CommandState; }
        bool HasCommandState(CommandStates state) const { return (m_CommandState == state); }

        void InitPossessCreateSpells();
        void InitCharmCreateSpells();
        void InitPetActionBar();
        void InitEmptyActionBar(bool withAttack = true);

                                                            //return true if successful
        bool AddSpellToActionBar(SpellInfo const* spellInfo, ActiveStates newstate = ACT_DECIDE);
        bool RemoveSpellFromActionBar(uint32 spell_id);
        void LoadPetActionBar();
        void BuildActionBar(WorldPacket* data);
        void SetSpellAutocast(SpellInfo const* spellInfo, bool state);
        void SetActionBar(uint8 index, uint32 spellOrAction, ActiveStates type)
        {
            PetActionBar[index].SetActionAndType(spellOrAction, type);
        }
        UnitActionBarEntry const* GetActionBarEntry(uint8 index) const { return &(PetActionBar[index]); }

        void ToggleCreatureAutocast(SpellInfo const* spellInfo, bool apply);

        CharmSpellInfo* GetCharmSpell(uint8 index) { return &(m_charmspells[index]); }

        GlobalCooldownMgr& GetGlobalCooldownMgr() { return m_GlobalCooldownMgr; }

        void SetIsCommandAttack(bool val);
        bool IsCommandAttack();
        void SetIsCommandFollow(bool val);
        bool IsCommandFollow();
        void SetIsAtStay(bool val);
        bool IsAtStay();
        void SetIsFollowing(bool val);
        bool IsFollowing();
        void SetIsReturning(bool val);
        bool IsReturning();
        void SaveStayPosition();
        void GetStayPosition(float &x, float &y, float &z);

    private:

        Unit* m_unit;
        UnitActionBarEntry PetActionBar[MAX_UNIT_ACTION_BAR_INDEX];
        CharmSpellInfo m_charmspells[4];
        CommandStates   m_CommandState;
        uint32          m_petnumber;

        //for restoration after charmed
        ReactStates     m_oldReactState;

        bool m_isCommandAttack;
        bool _isCommandFollow;
        bool m_isAtStay;
        bool m_isFollowing;
        bool m_isReturning;
        float m_stayX;
        float m_stayY;
        float m_stayZ;

        GlobalCooldownMgr m_GlobalCooldownMgr;
};

struct SpellProcEventEntry;                                 // used only privately

class Unit : public WorldObject
{
    enum DamageTrackingInfo
    {
        DAMAGE_TRACKING_PERIOD = 120,
        DAMAGE_TRACKING_UPDATE_INTERVAL = 1 * IN_MILLISECONDS
    };

    public:
        class RemainingPeriodicAmount final
        {
        public:
            RemainingPeriodicAmount(int32 total, int32 ticks)
                : total_(total)
                , ticks_(ticks)
            { }

            int32 total() const { return total_; }

            int32 perTick() const { return total_ ? total_ / ticks_ : 0; }

        private:
            int32 total_;
            int32 ticks_;
        };

        typedef std::set<Unit*> AttackerSet;
        typedef std::set<Unit*> ControlList;
        typedef std::pair<uint32, uint8> spellEffectPair;
        typedef std::multimap<uint32, Aura *> AuraMap;
        typedef std::multimap<uint32, AuraApplication *> AuraApplicationMap;
        typedef std::multimap<uint32, AuraApplication *> AuraStateAurasMap;
        typedef std::list<AuraEffect *> AuraEffectList;
        typedef std::list<Aura *> AuraList;
        typedef std::list<AuraApplication *> AuraApplicationList;
        typedef std::list<DiminishingReturn> Diminishing;
        typedef std::set<uint32> ComboPointHolderSet;
        typedef std::multimap<uint32, uint32> AuraIdList;

        typedef std::map<uint8, AuraApplication*> VisibleAuraMap;

    public:
        virtual ~Unit();

        UnitAI* GetAI() { return i_AI; }
        void SetAI(UnitAI* newAI) { i_AI = newAI; }

        void AddToWorld();
        void RemoveFromWorld();

        void CleanupBeforeRemoveFromMap(bool finalCleanup);
        void CleanupsBeforeDelete(bool finalCleanup = true);                        // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

        DiminishingLevels GetDiminishing(DiminishingGroup  group);
        void IncrDiminishing(DiminishingGroup group);
        float ApplyDiminishingToDuration(DiminishingGroup  group, int32 &duration, Unit* caster, DiminishingLevels Level, int32 limitduration);
        void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
        void ClearDiminishings() { m_Diminishing.clear(); }

        // target dependent range checks
        float GetSpellMaxRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;
        float GetSpellMinRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;

        virtual void Update(uint32 time);

        void setAttackTimer(WeaponAttackType type, uint32 time) { m_attackTimer[type] = time; }
        void resetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        uint32 getAttackTimer(WeaponAttackType type) const { return m_attackTimer[type]; }
        bool isAttackReady(WeaponAttackType type = BASE_ATTACK) const { return m_attackTimer[type] == 0; }
        bool haveOffhandWeapon() const;
        bool CanDualWield() const { return m_canDualWield; }
        void SetCanDualWield(bool value) { m_canDualWield = value; }
        float GetCombatReach() const { return m_floatValues[UNIT_FIELD_COMBAT_REACH]; }
        float GetMeleeReach() const { float reach = m_floatValues[UNIT_FIELD_COMBAT_REACH]; return reach > MIN_MELEE_REACH ? reach : MIN_MELEE_REACH; }
        bool IsWithinCombatRange(const Unit* obj, float dist2compare) const;
        bool IsWithinMeleeRange(const Unit* obj, float dist = MELEE_RANGE) const;
        uint32 m_extraAttacks;
        bool m_canDualWield;
        int32 insightCount;

        void _addAttacker(Unit* pAttacker)                  // must be called only from Unit::Attack(Unit*)
        {
            m_attackers.insert(pAttacker);
        }
        void _removeAttacker(Unit* pAttacker)               // must be called only from Unit::AttackStop()
        {
            m_attackers.erase(pAttacker);
        }
        Unit* getAttackerForHelper() const                 // If someone wants to help, who to give them
        {
            if (GetVictim() != NULL)
                return GetVictim();

            if (!m_attackers.empty())
                return *(m_attackers.begin());

            return NULL;
        }
        bool Attack(Unit* victim, bool meleeAttack);
        void CastStop(uint32 except_spellid = 0);
        bool AttackStop();
        void RemoveAllAttackers();
        AttackerSet const& getAttackers() const { return m_attackers; }
        bool isAttackingPlayer() const;
        Unit* GetVictim() const { return m_attacking; }

        void CombatStop(bool includingCast = false);
        void CombatStopWithPets(bool includingCast = false);
        void StopAttackFaction(uint32 faction_id);
        void GetAttackableUnitListInRange(std::list<Unit*> &list, float fMaxSearchRange) const;
        void GetFriendlyUnitListInRange(std::list<Unit*> &list, float fMaxSearchRange) const;
        Unit* SelectNearbyTarget(Unit* exclude = NULL, float dist = NOMINAL_MELEE_RANGE) const;
        Unit* SelectNearbyAlly(Unit* exclude = NULL, float dist = NOMINAL_MELEE_RANGE) const;
        Unit* SelectNearestTarget(float dist = 0) const;
        void SendMeleeAttackStop(Unit* victim = NULL);
        void SendMeleeAttackStart(Unit* victim);
        bool IsVisionObscured(Unit* victim, SpellInfo const* spellInfo);

        // Part of Evade mechanics
        time_t GetLastDamagedTime() const { return _lastDamagedTime; }
        void SetLastDamagedTime(time_t val) { _lastDamagedTime = val; }

        void AddUnitState(uint32 f) { m_state |= f; }
        bool HasUnitState(const uint32 f) const { return (m_state & f); }
        void ClearUnitState(uint32 f) { m_state &= ~f; }
        bool CanFreeMove() const
        {
            return !HasUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT |
                UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED) && GetOwnerGUID() == 0;
        }

        uint32 HasUnitTypeMask(uint32 mask) const { return mask & m_unitTypeMask; }
        void AddUnitTypeMask(uint32 mask) { m_unitTypeMask |= mask; }
        bool IsSummon() const   { return m_unitTypeMask & UNIT_MASK_SUMMON; }
        bool isGuardian() const { return m_unitTypeMask & UNIT_MASK_GUARDIAN; }
        bool isPet() const      { return m_unitTypeMask & UNIT_MASK_PET; }
        bool isHunterPet() const{ return m_unitTypeMask & UNIT_MASK_HUNTER_PET; }
        bool isTotem() const    { return m_unitTypeMask & UNIT_MASK_TOTEM; }
        bool IsVehicle() const  { return m_unitTypeMask & UNIT_MASK_VEHICLE; }

        bool IsPetGuardianStuff() const { return m_unitTypeMask & ( UNIT_MASK_SUMMON | UNIT_MASK_GUARDIAN | UNIT_MASK_PET | UNIT_MASK_HUNTER_PET | UNIT_MASK_TOTEM ); }

        uint8 getLevel() const { return uint8(GetUInt32Value(UNIT_FIELD_LEVEL)); }
        uint8 getLevelForTarget(WorldObject const* /*target*/) const { return getLevel(); }
        void SetLevel(uint8 lvl);
        uint8 getRace(bool forceoriginal = false) const;
        uint32 getRaceMask() const { return 1 << (getRace()-1); }
        uint32 getORaceMask() const { return 1 << (getRace(true) - 1); }
        uint8 getClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, 1); }
        uint32 getClassMask() const { return 1 << (getClass()-1); }
        uint8 getGender() const { return GetByteValue(UNIT_FIELD_BYTES_0, 3); }

        float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STAT0+stat)); }
        void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STAT0+stat, val); }
        uint32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL); }
        void SetArmor(int32 val) { SetResistance(SPELL_SCHOOL_NORMAL, val); }

        uint32 GetResistance(SpellSchools school) const { return GetUInt32Value(UNIT_FIELD_RESISTANCES+school); }
        uint32 GetResistance(SpellSchoolMask mask) const;
        void SetResistance(SpellSchools school, int32 val) { SetStatInt32Value(UNIT_FIELD_RESISTANCES+school, val); }

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }

        bool IsFullHealth() const { return GetHealth() == GetMaxHealth(); }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
        HealthBelowPct(T pct) const
        {
            return GetHealth() < CountPctFromMaxHealth(pct);
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
        HealthBelowPctDamaged(T pct, uint32 damage) const
        {
            return int64(GetHealth()) - int64(damage) < int64(CountPctFromMaxHealth(pct));
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
        HealthAbovePct(T pct) const
        {
            return GetHealth() > CountPctFromMaxHealth(pct);
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
        HealthAbovePctHealed(T pct, uint32 heal) const
        {
            return uint64(GetHealth()) + uint64(heal) > CountPctFromMaxHealth(pct);
        }

        float GetHealthPct() const
        {
            return GetMaxHealth() ? 100.f * GetHealth() / GetMaxHealth() : 0.0f;
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, uint32>::type
        CountPctFromMaxHealth(T pct) const
        {
            return CalculatePct(GetMaxHealth(), pct);
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, uint32>::type
        CountPctFromCurHealth(T pct) const
        {
            return CalculatePct(GetHealth(), pct);
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, int32>::type
        CountPctFromMaxMana(T pct) const
        {
            return CalculatePct(GetMaxPower(POWER_MANA), pct);
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, int32>::type
        CountPctFromCurMana(T pct) const
        {
            return CalculatePct(GetPower(POWER_MANA), pct);
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, int32>::type
        CountPctFromMaxPower(T pct, Powers power) const
        {
            return CalculatePct(GetMaxPower(power), pct);
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, int32>::type
        CountPctFromCurPower(T pct, Powers power) const
        {
            return CalculatePct(GetPower(power), pct);
        }

        void SetHealth(uint32 val);
        void SetMaxHealth(uint32 val);
        inline void SetFullHealth() { SetHealth(GetMaxHealth()); }
        int32 ModifyHealth(int32 val);
        int32 GetHealthGain(int32 dVal);

        Powers getPowerType() const { return Powers(GetUInt32Value(UNIT_FIELD_DISPLAY_POWER)); }
        void setPowerType(Powers power);
        int32 GetPower(Powers power) const;
        int32 GetMinPower(Powers power) const { return power == POWER_ECLIPSE ? -100 : 0; }
        int32 GetMaxPower(Powers power) const;
        void SetPower(Powers power, int32 val, bool sendLog = true);
        void SetMaxPower(Powers power, int32 val);
        SpellPowerEntry const* GetSpellPowerEntryBySpell(SpellInfo const* spell) const;

        // returns the change in power
        int32 ModifyPower(Powers power, int32 val);
        int32 ModifyPowerPct(Powers power, float pct, bool apply = true);

        uint32 GetAttackTime(WeaponAttackType att) const
        {
           float f_BaseAttackTime = GetFloatValue(UNIT_FIELD_BASEATTACKTIME+att) / m_modAttackSpeedPct[att];
           return (uint32)f_BaseAttackTime;
        }

        void SetAttackTime(WeaponAttackType att, uint32 val) { SetFloatValue(UNIT_FIELD_BASEATTACKTIME+att, val*m_modAttackSpeedPct[att]); }
        void ApplyAttackTimePercentMod(WeaponAttackType att, float val, bool apply);
        void ApplyCastTimePercentMod(float val, bool apply);

        SheathState GetSheath() const { return SheathState(GetByteValue(UNIT_FIELD_BYTES_2, 0)); }
        virtual void SetSheath(SheathState sheathed) { SetByteValue(UNIT_FIELD_BYTES_2, 0, sheathed); }

        // faction template id
        uint32 getFaction() const { return GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }
        void setFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction); }
        FactionTemplateEntry const* getFactionTemplateEntry() const;

        ReputationRank GetReactionTo(Unit const* target) const;
        ReputationRank static GetFactionReactionTo(FactionTemplateEntry const* factionTemplateEntry, Unit const* target);

        bool IsHostileTo(Unit const* unit) const;
        bool IsHostileToPlayers() const;
        bool IsFriendlyTo(Unit const* unit) const;
        bool IsNeutralToAll() const;
        bool IsInPartyWith(Unit const* unit) const;
        bool IsInRaidWith(Unit const* unit) const;
        void GetPartyMembers(std::list<Unit*> &units);
        bool IsContestedGuard() const
        {
            if (FactionTemplateEntry const* entry = getFactionTemplateEntry())
                return entry->IsContestedGuardFaction();

            return false;
        }
        bool IsPvP() const { return HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_PVP); }
        void SetPvP(bool state)
        {
            if (state)
                SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_PVP);
            else
                RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_PVP);
        }
        uint32 GetCreatureType() const;
        uint32 GetCreatureTypeMask() const
        {
            uint32 creatureType = GetCreatureType();
            return (creatureType >= 1) ? (1 << (creatureType - 1)) : 0;
        }

        uint8 getStandState() const { return GetByteValue(UNIT_FIELD_BYTES_1, 0); }
        bool IsSitState() const;
        bool IsStandState() const;
        void SetStandState(uint8 state);

        void  SetStandFlags(uint8 flags) { SetByteFlag(UNIT_FIELD_BYTES_1, 2, flags); }
        void  RemoveStandFlags(uint8 flags) { RemoveByteFlag(UNIT_FIELD_BYTES_1, 2, flags); }

        bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        void Mount(uint32 mount, uint32 vehicleId = 0, uint32 creatureEntry = 0);
        void Dismount();
        MountCapabilityEntry const* GetMountCapability(uint32 mountType) const;
        bool SetVehicleId(uint32 id);

        void SendDurabilityLoss(Player* receiver, uint32 percent);
        void PlayOneShotAnimKit(uint32 id);

        uint16 GetMaxSkillValueForLevel(Unit const* target = NULL) const { return (target ? getLevelForTarget(target) : getLevel()) * 5; }
        void DealDamageMods(Unit* victim, uint32 &damage, uint32* absorb);
        uint32 DealDamage(Unit* victim, uint32 damage, CleanDamage const* cleanDamage = NULL, DamageEffectType damagetype = DIRECT_DAMAGE, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const* spellProto = NULL, bool durabilityLoss = true);
        uint32 CalcStaggerDamage(Player* victim, uint32 damage);
        void Kill(Unit* victim, bool durabilityLoss = true, SpellInfo const* spellProto = NULL);
        int32 DealHeal(Unit* victim, uint32 addhealth, SpellInfo const* spellProto = NULL);

        void ProcDamageAndSpell(Unit* victim, uint32 procAttacker, uint32 procVictim, uint32 procEx, uint32 amount, uint32 absorb = 0, WeaponAttackType attType = BASE_ATTACK, SpellInfo const* procSpell = NULL, SpellInfo const* procAura = NULL, bool procSpellIsHeal = false);
        void ProcDamageAndSpellFor(bool isVictim, Unit* target, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellInfo const* procSpell, uint32 damage, uint32 absorb = 0, SpellInfo const* procAura = NULL, bool procSpellIsHeal = false);

        bool IsNoBreakingCC(bool isVictim, Unit* target, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellInfo const* procSpell, uint32 damage, uint32 absorb, SpellInfo const* procAura, SpellInfo const* spellProto) const;

        void GetProcAurasTriggeredOnEvent(std::list<AuraApplication*>& aurasTriggeringProc, std::list<AuraApplication*>* procAuras, ProcEventInfo eventInfo);
        void TriggerAurasProcOnEvent(CalcDamageInfo& damageInfo);
        void TriggerAurasProcOnEvent(std::list<AuraApplication*>* myProcAuras, std::list<AuraApplication*>* targetProcAuras, Unit* actionTarget, uint32 typeMaskActor, uint32 typeMaskActionTarget, uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask, Spell* spell, DamageInfo* damageInfo, HealInfo* healInfo);
        void TriggerAurasProcOnEvent(ProcEventInfo& eventInfo, std::list<AuraApplication*>& procAuras);

        void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate (Unit* victim, WeaponAttackType attType = BASE_ATTACK, bool extra = false);

        void CalculateMeleeDamage(Unit* victim, uint32 damage, CalcDamageInfo* damageInfo, WeaponAttackType attackType = BASE_ATTACK);
        void DealMeleeDamage(CalcDamageInfo* damageInfo, bool durabilityLoss);
        void HandleProcExtraAttackFor(Unit* victim);

        void CalculateSpellDamageTaken(SpellNonMeleeDamage* damageInfo, int32 damage, SpellInfo const* spellInfo, WeaponAttackType attackType = BASE_ATTACK, bool crit = false);
        void DealSpellDamage(SpellNonMeleeDamage* damageInfo, bool durabilityLoss);

        // player or player's pet resilience (-1%)
        uint32 GetCritDamageReduction(uint32 damage) const { return GetCombatRatingDamageReduction(CR_RESILIENCE_CRIT_TAKEN, 33.0f, damage); }
        uint32 GetDamageReduction(uint32 damage) const { return GetCombatRatingDamageReduction(CR_RESILIENCE_PLAYER_DAMAGE_TAKEN, 100.0f, damage); }

        void ApplyResilience(const Unit* victim, int32 * damage) const;

        float MeleeSpellMissChance(const Unit* victim, WeaponAttackType attType, uint32 spellId) const;
        SpellMissInfo MeleeSpellHitResult(Unit* victim, SpellInfo const* spell);
        SpellMissInfo MagicSpellHitResult(Unit* victim, SpellInfo const* spell);
        SpellMissInfo SpellHitResult(Unit* victim, SpellInfo const* spell, bool canReflect = false);

        float GetUnitDodgeChance()    const;
        float GetUnitParryChance()    const;
        float GetUnitBlockChance()    const;
        float GetUnitMissChance(WeaponAttackType attType)     const;
        float GetUnitCriticalChance(WeaponAttackType attackType, const Unit* victim) const;
        int32 GetMechanicResistChance(const SpellInfo* spell);
        bool CanUseAttackType(uint8 attacktype) const
        {
            switch (attacktype)
            {
                case BASE_ATTACK: return !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
                case OFF_ATTACK: return !HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISARM_OFFHAND);
                case RANGED_ATTACK: return !HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISARM_RANGED);
            }
            return true;
        }

        virtual uint32 GetBlockPercent() { return 30; }

        uint32 GetUnitMeleeSkill(Unit const* target = NULL) const { return (target ? getLevelForTarget(target) : getLevel()) * 5; }
        float GetWeaponProcChance() const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM,  const SpellInfo* spellProto) const;

        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit* victim, WeaponAttackType attType) const;
        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit* victim, WeaponAttackType attType, int32 crit_chance, int32 miss_chance, int32 dodge_chance, int32 parry_chance, int32 block_chance) const;

        bool IsVendor()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR); }
        bool IsTrainer()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER); }
        bool IsQuestGiver()   const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER); }
        bool IsGossip()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP); }
        bool IsTaxi()         const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_FLIGHTMASTER); }
        bool IsGuildMaster()  const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER); }
        bool IsBattleMaster() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEMASTER); }
        bool IsBanker()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER); }
        bool IsInnkeeper()    const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER); }
        bool IsSpiritHealer() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER); }
        bool IsSpiritGuide()  const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITGUIDE); }
        bool IsTabardDesigner()const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDDESIGNER); }
        bool IsAuctioner()    const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER); }
        bool IsArmorer()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_REPAIR); }
        bool IsServiceProvider() const
        {
            return HasFlag(UNIT_NPC_FLAGS,
                UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_FLIGHTMASTER |
                UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_BATTLEMASTER | UNIT_NPC_FLAG_BANKER |
                UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_SPIRITHEALER |
                UNIT_NPC_FLAG_SPIRITGUIDE | UNIT_NPC_FLAG_TABARDDESIGNER | UNIT_NPC_FLAG_AUCTIONEER);
        }
        bool isSpiritService() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE); }

        bool isInFlight()  const { return HasUnitState(UNIT_STATE_IN_FLIGHT); }

        bool IsInCombat()  const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        void CombatStart(Unit* target, bool initialAggro = true);
        void SetInCombatState(bool PvP, Unit* enemy = NULL, bool isControlled = false);
        void SetInCombatWith(Unit* enemy);
        void ClearInCombat();
        void SetBoundingRadius(float radius);
        void SetCombatReach(float radius);
        uint32 GetCombatTimer() const { return m_CombatTimer; }

        bool HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName, uint32 familyFlags) const;
        bool virtual HasSpell(uint32 /*spellID*/) const { return false; }
        bool HasCrowdControlAuraType(AuraType type, uint32 excludeAura = 0, uint32 dispelType = 0) const;
        bool HasCrowdControlAura(Unit* excludeCasterChannel = NULL, uint32 dispelType = 0) const;
        bool HasBreakableByDamageAuraType(AuraType type, uint32 excludeAura = 0) const;
        bool HasBreakableByDamageCrowdControlAura(Unit* excludeCasterChannel = NULL) const;

        bool HasStealthAura()      const { return HasAuraType(SPELL_AURA_MOD_STEALTH); }
        bool HasInvisibilityAura() const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY); }
        bool isFeared()  const { return HasAuraType(SPELL_AURA_MOD_FEAR) || HasAuraType(SPELL_AURA_MOD_FEAR_2); }
        bool isInRoots() const { return HasAuraType(SPELL_AURA_MOD_ROOT); }
        bool IsPolymorphed() const;

        bool isFrozen() const;

        bool isSef() const;

        bool isTargetableForAttack(bool checkFakeDeath = true) const;

        bool IsValidAttackTarget(Unit const* target) const;
        bool _IsValidAttackTarget(Unit const* target, SpellInfo const* bySpell, WorldObject const* obj = NULL) const;

        bool IsValidAssistTarget(Unit const* target) const;
        bool _IsValidAssistTarget(Unit const* target, SpellInfo const* bySpell) const;

        virtual bool IsInWater() const;
        virtual bool IsUnderWater() const;
        virtual void UpdateUnderwaterState(Map* m, float x, float y, float z);
        bool isInAccessiblePlaceFor(Creature const* c) const;

        void SendHealSpellLog(Unit* victim, uint32 SpellID, uint32 Damage, uint32 OverHeal, uint32 Absorb, bool critical = false);
        int32 HealBySpell(Unit* victim, SpellInfo const* spellInfo, uint32 addHealth, bool critical = false);
        void SendEnergizeSpellLog(Unit* victim, uint32 SpellID, uint32 Damage, Powers powertype);
        void EnergizeBySpell(Unit* victim, uint32 SpellID, int32 Damage, Powers powertype);
        uint32 SpellNonMeleeDamageLog(Unit* victim, uint32 spellID, uint32 damage);

        void CastSpell(SpellCastTargets const& targets, SpellInfo const* spellInfo, CustomSpellValues const* value, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0, float periodicDamageModifier = 0.0f);
        void CastSpell(Unit* victim, uint32 spellId, bool triggered, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0, float periodicDamageModifier = 0.0f);
        void CastSpell(Unit* victim, uint32 spellId, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0, float periodicDamageModifier = 0.0f);
        void CastSpell(Unit* victim, SpellInfo const* spellInfo, bool triggered, Item* castItem= NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastSpell(Unit* victim, SpellInfo const* spellInfo, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem= NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0, float periodicDamageModifier = 0.0f);
        void CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0);

        void CastSpell(Position const &pos, uint32 spellId, bool triggered, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, uint64 originalCaster = 0)
        {
            CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), spellId, triggered, castItem, triggeredByAura, originalCaster);
        }

        void CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item* castItem= NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, int32 const* bp3, int32 const* bp4, int32 const* bp5, bool triggered, Item* castItem= NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastCustomSpell(uint32 spellId, SpellValueMod mod, int32 value, Unit* Victim = NULL, bool triggered = true, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastCustomSpell(uint32 spellId, CustomSpellValues const &value, Unit* Victim = NULL, bool triggered = true, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastCustomSpell(float x, float y, float z, uint32 spellId, CustomSpellValues const &value, bool triggered = true, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastCustomSpell(float x, float y, float z, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item* castItem= NULL, AuraEffect const *triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastSpell(GameObject* go, uint32 spellId, bool triggered, Item* castItem = NULL, AuraEffect *triggeredByAura = NULL, uint64 originalCaster = 0);
        Aura *ToggleAura(uint32 spellId, Unit* target);
        Aura *AddAura(uint32 spellId, Unit* target);
        Aura *AddAura(SpellInfo const* spellInfo, uint32 effMask, Unit* target);
        Aura* AddAuraForTarget(Aura* aura, Unit* target);
        void SetAuraStack(uint32 spellId, Unit* target, uint32 stack);
        void SendPlaySpellVisualKit(uint32 id, uint32 unkParam);
        void SendPlaySpellVisual(ObjectGuid source, ObjectGuid target, uint32 spellVisual, float speed);

        void DeMorph();

        void SendAttackStateUpdate(CalcDamageInfo* damageInfo);
        void SendAttackStateUpdate(uint32 HitInfo, Unit* target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount);
        void SendSpellNonMeleeDamageLog(SpellNonMeleeDamage* log);
        void SendSpellNonMeleeDamageLog(Unit* target, uint32 SpellID, uint32 Damage, SpellSchoolMask damageSchoolMask, uint32 AbsorbedDamage, uint32 Resist, bool PhysicalDamage, uint32 Blocked, bool CriticalHit = false);
        void SendPeriodicAuraLog(SpellPeriodicAuraLogInfo* pInfo);
        void SendSpellMiss(Unit* target, uint32 spellID, SpellMissInfo missInfo);
        void SendSpellDamageResist(Unit* target, uint32 spellId);
        void SendSpellDamageImmune(Unit* target, uint32 spellId);

        void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false);
        void SendTeleportPacket(Position &oldPos);
        virtual bool UpdatePosition(float x, float y, float z, float ang, bool teleport = false);
        // returns true if unit's position really changed
        bool UpdatePosition(const Position &pos, bool teleport = false) { return UpdatePosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), teleport); }
        void UpdateOrientation(float orientation);
        void UpdateHeight(float newZ);

        void SendMoveKnockBack(Player* player, float speedXY, float speedZ, float vcos, float vsin);
        void KnockbackFrom(float x, float y, float speedXY, float speedZ);
        void JumpTo(float speedXY, float speedZ, bool forward = true);
        void JumpTo(WorldObject* obj, float speedZ);

        void MonsterMoveWithSpeed(float x, float y, float z, float speed);
        //void SetFacing(float ori, WorldObject* obj = NULL);
        //void SendMonsterMove(float NewPosX, float NewPosY, float NewPosZ, uint8 type, uint32 MovementFlags, uint32 Time, Player* player = NULL);
        void SendMovementFlagUpdate(bool self = false);

        /*! These methods send the same packet to the client in apply and unapply case.
            The client-side interpretation of this packet depends on the presence of relevant movementflags
            which are sent with movementinfo. Furthermore, these packets are broadcast to nearby players as well
            as the current unit.
        */
        void SendMovementHover(bool apply);
        void SendMovementFeatherFall();
        void SendMovementWaterWalking();
        void SendMovementGravityChange();
        void SendMovementCanFlyChange();

        void SendSetPlayHoverAnim(bool enable);

        bool IsLevitating() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);}
        bool IsWalking() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING);}
        virtual bool SetWalk(bool enable);
        virtual bool SetDisableGravity(bool disable, bool packetOnly = false);
        bool SetHover(bool enable);

        void SetInFront(Unit const* target);
        void SetFacingTo(float ori);
        void SetFacingToObject(WorldObject* object);

        void SendChangeCurrentVictimOpcode(HostileReference* pHostileReference);
        void SendClearThreatListOpcode();
        void SendRemoveFromThreatListOpcode(HostileReference* pHostileReference);
        void SendThreatListUpdate();

        void SendClearTarget();

        void BuildHeartBeatMsg(WorldPacket* data) const;

        bool IsAlive() const { return (m_deathState == ALIVE); };
        bool isDying() const { return (m_deathState == JUST_DIED); };
        bool isDead() const { return (m_deathState == DEAD || m_deathState == CORPSE); };
        DeathState getDeathState() { return m_deathState; };
        virtual void setDeathState(DeathState s);           // overwrited in Creature/Player/Pet

        uint64 GetOwnerGUID() const { return GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
        void SetOwnerGUID(uint64 owner);
        uint64 GetCreatorGUID() const { return GetUInt64Value(UNIT_FIELD_CREATEDBY); }
        void SetCreatorGUID(uint64 creator) { SetUInt64Value(UNIT_FIELD_CREATEDBY, creator); }
        uint64 GetMinionGUID() const { return GetUInt64Value(UNIT_FIELD_SUMMON); }
        void SetMinionGUID(uint64 guid) { SetUInt64Value(UNIT_FIELD_SUMMON, guid); }
        uint64 GetCharmerGUID() const { return GetUInt64Value(UNIT_FIELD_CHARMEDBY); }
        void SetCharmerGUID(uint64 owner) { SetUInt64Value(UNIT_FIELD_CHARMEDBY, owner); }
        uint64 GetCharmGUID() const { return  GetUInt64Value(UNIT_FIELD_CHARM); }
        void SetPetGUID(uint64 guid) { m_SummonSlot[SUMMON_SLOT_PET] = guid; }
        uint64 GetPetGUID() const { return m_SummonSlot[SUMMON_SLOT_PET]; }
        void SetCritterGUID(uint64 guid) { SetUInt64Value(UNIT_FIELD_CRITTER, guid); }
        uint64 GetCritterGUID() const { return GetUInt64Value(UNIT_FIELD_CRITTER); }

        bool IsControlledByPlayer() const { return m_ControlledByPlayer; }
        uint64 GetCharmerOrOwnerGUID() const { return GetCharmerGUID() ? GetCharmerGUID() : GetOwnerGUID(); }
        uint64 GetCharmerOrOwnerOrOwnGUID() const
        {
            if (uint64 guid = GetCharmerOrOwnerGUID())
                return guid;
            return GetGUID();
        }
        bool isCharmedOwnedByPlayerOrPlayer() const { return IS_PLAYER_GUID(GetCharmerOrOwnerOrOwnGUID()); }

        Player* GetSpellModOwner() const;

        Unit* GetOwner() const;
        Guardian *GetGuardianPet() const;
        Minion *GetFirstMinion() const;
        Unit* GetCharmer() const;
        Unit* GetCharm() const;
        Unit* GetCharmerOrOwner() const { return GetCharmerGUID() ? GetCharmer() : GetOwner(); }
        Unit* GetCharmerOrOwnerOrSelf() const
        {
            if (Unit* u = GetCharmerOrOwner())
                return u;

            return (Unit*)this;
        }
        Player* GetCharmerOrOwnerPlayerOrPlayerItself() const;
        Player* GetAffectingPlayer() const;

        void SetMinion(Minion *minion, bool apply, bool stampeded = false);
        void GetAllMinionsByEntry(std::list<Creature*>& Minions, uint32 entry);
        Unit* GetFirstMinionByEntry(uint32 entry);
        void RemoveAllMinionsByEntry(uint32 entry);
        void SetCharm(Unit* target, bool apply);
        Unit* GetNextRandomRaidMemberOrPet(float radius);
        bool SetCharmedBy(Unit* charmer, CharmType type, AuraApplication const* aurApp = NULL);
        void RemoveCharmedBy(Unit* charmer);
        void RestoreFaction();

        ControlList m_Controlled;
        Unit* GetFirstControlled() const;
        void RemoveAllControlled();

        bool isCharmed() const { return GetCharmerGUID() != 0; }
        bool isPossessed() const { return HasUnitState(UNIT_STATE_POSSESSED); }
        bool isPossessedByPlayer() const { return HasUnitState(UNIT_STATE_POSSESSED) && IS_PLAYER_GUID(GetCharmerGUID()); }
        bool isPossessing() const
        {
            if (Unit* u = GetCharm())
                return u->isPossessed();
            else
                return false;
        }
        bool isPossessing(Unit* u) const { return u->isPossessed() && GetCharmGUID() == u->GetGUID(); }

        CharmInfo* GetCharmInfo() { return m_charmInfo; }
        CharmInfo* InitCharmInfo();
        void DeleteCharmInfo();
        void UpdateCharmAI();
        //Player* GetMoverSource() const;
        Player* m_movedPlayer;
        SharedVisionList const& GetSharedVisionList() { return m_sharedVision; }
        void AddPlayerToVision(Player* player);
        void RemovePlayerFromVision(Player* player);
        bool HasSharedVision() const { return !m_sharedVision.empty(); }
        void RemoveBindSightAuras();
        void RemoveCharmAuras();

        // aura apply/remove helpers - you should better not use these
        Aura *_TryStackingOrRefreshingExistingAura(SpellInfo const* newAura, uint32 effMask, Unit* caster, int32* baseAmount = NULL, Item* castItem = NULL, uint64 casterGUID = 0);
        void _AddAura(UnitAura *aura, Unit* caster);
        AuraApplication * _CreateAuraApplication(Aura *aura, uint32 effMask);
        void _ApplyAuraEffect(Aura *aura, uint32 effIndex);
        void _ApplyAura(AuraApplication * aurApp, uint32 effMask);
        void _UnapplyAura(AuraApplicationMap::iterator &i, AuraRemoveMode removeMode);
        void _UnapplyAura(AuraApplication * aurApp, AuraRemoveMode removeMode);
        void _RemoveNoStackAuraApplicationsDueToAura(Aura *aura);
        void _RemoveNoStackAurasDueToAura(Aura *aura);
        bool _IsNoStackAuraDueToAura(Aura *appliedAura, Aura *existingAura) const;
        void _RegisterAuraEffect(AuraEffect *aurEff, bool apply);

        // m_ownedAuras container management
        AuraMap      & GetOwnedAuras()       { return m_ownedAuras; }
        AuraMap const& GetOwnedAuras() const { return m_ownedAuras; }

        void RemoveOwnedAura(AuraMap::iterator &i, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveOwnedAura(uint32 spellId, uint64 casterGUID = 0, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveOwnedAura(Aura *aura, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);

        Aura *GetOwnedAura(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0, Aura *except = NULL) const;

        // m_appliedAuras container management
        AuraApplicationMap      & GetAppliedAuras()       { return m_appliedAuras; }
        AuraApplicationMap const& GetAppliedAuras() const { return m_appliedAuras; }

        void RemoveAura(AuraApplicationMap::iterator &i, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(uint32 spellId, uint64 casterGUID = 0, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(AuraApplication * aurApp, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(Aura *aur, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAllSymbiosisAuras();

        void RemoveAurasDueToSpell(uint32 spellId, uint64 casterGUID = 0, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAuraFromStack(uint32 spellId, uint64 casterGUID = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT, int32 num = -1);
        void RemoveAurasDueToSpellByDispel(uint32 spellId, uint32 dispellerSpellId, uint64 casterGUID, Unit* dispeller, uint8 chargesRemoved = 1);
        void RemoveAurasDueToSpellBySteal(uint32 spellId, uint64 casterGUID, Unit* stealer);
        void RemoveAurasDueToItemSpell(Item* castItem, uint32 spellId);
        void RemoveAurasByType(AuraType auraType, uint64 casterGUID = 0, Aura *aura = NULL, uint32 exceptAuraId = 0, bool negative = true, bool positive = true);
        void RemoveNotOwnSingleTargetAuras(uint32 newPhase = 0x0);
        void RemoveAurasWithInterruptFlags(uint32 flag, uint32 except = 0);
        void RemoveAurasWithAttribute(uint32 flags);
        Aura* GetCCBreakAura();
        void RemoveAurasWithMechanic(uint32 mechanic_mask, AuraRemoveMode removemode = AURA_REMOVE_BY_DEFAULT, uint32 except = 0, uint8 count = 0);
        void RemoveMovementImpairingAuras();

        void RemoveAreaAurasDueToLeaveWorld();
        void RemoveAllAuras();
        void RemoveNonPassivesAuras();
        void RemoveArenaAuras();
        void RemoveAllAurasOnDeath();
        void RemoveNegativeAuras();
        void RemoveAllAurasRequiringDeadTarget();
        void RemoveAllAurasExceptType(AuraType type);
        void RemoveAllAurasByType(AuraType type);
        void DelayOwnedAuras(uint32 spellId, uint64 caster, int32 delaytime);

        void _RemoveAllAuraStatMods();
        void _ApplyAllAuraStatMods();

        AuraEffectList const& GetAuraEffectsByType(AuraType type) const { return m_modAuras[type]; }

        AuraList      & GetSingleCastAuras()       { return m_scAuras; }
        AuraList const& GetSingleCastAuras() const { return m_scAuras; }

        AuraEffect *GetAuraEffect(uint32 spellId, uint8 effIndex, uint64 casterGUID = 0) const;
        AuraEffect *GetAuraEffectOfRankedSpell(uint32 spellId, uint8 effIndex, uint64 casterGUID = 0) const;
        AuraEffect *GetAuraEffect(AuraType type, SpellFamilyNames name, uint32 iconId, uint8 effIndex) const; // spell mustn't have familyflags
        AuraEffect *GetAuraEffect(AuraType type, SpellFamilyNames family, Trinity::Flag128 const &familyFlags, uint64 casterGUID = 0);
        inline AuraEffect *GetDummyAuraEffect(SpellFamilyNames name, uint32 iconId, uint32 effIndex) const { return GetAuraEffect(SPELL_AURA_DUMMY, name, iconId, effIndex);}

        AuraApplication * GetAuraApplication(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0, AuraApplication * except = NULL) const;
        Aura *GetAura(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0) const;

        AuraApplication * GetAuraApplicationOfRankedSpell(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0, AuraApplication * except = NULL) const;
        Aura *GetAuraOfRankedSpell(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0) const;

        void GetDispellableAuraList(Unit* caster, uint32 dispelMask, DispelChargesList& dispelList);

        bool HasAuraEffect(uint32 spellId, uint8 effIndex, uint64 caster = 0) const;
        uint32 GetAuraCount(uint32 spellId) const;
        bool HasAura(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0) const;
        bool HasAuraType(AuraType auraType) const;
        bool HasAuraTypeWithCaster(AuraType auratype, uint64 caster) const;
        bool HasAuraTypeWithMiscvalue(AuraType auratype, int32 miscvalue) const;
        bool HasAuraTypeWithAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        bool HasAuraTypeWithValue(AuraType auratype, int32 value) const;
        bool HasNegativeAuraWithInterruptFlag(uint32 flag, uint64 guid = 0);
        bool HasNegativeAuraWithAttribute(uint32 flag, uint64 guid = 0);
        bool HasAuraWithMechanic(uint32 mechanicMask);

        void RemoveSoulSwapDOT(Unit* target);
        void ApplySoulSwapDOT(Unit* target);

        AuraEffect *IsScriptOverriden(SpellInfo const* spell, int32 script) const;
        uint32 GetDiseasesByCaster(uint64 casterGUID, bool remove = false);
        uint32 GetDoTsByCaster(uint64 casterGUID) const;

        int32 GetTotalAuraModifier(AuraType auratype) const;
        float GetTotalFloatAuraModifier(AuraType auratype) const;
        float GetTotalAuraMultiplier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype);
        int32 GetMaxNegativeAuraModifier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifierWithPassives(AuraType auratype);

        int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask, AuraEffect const *except = NULL) const;
        int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

        int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

        int32 GetTotalAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        float GetTotalAuraMultiplierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        int32 GetMaxPositiveAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        int32 GetMaxNegativeAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;

        float GetResistanceBuffMods(SpellSchools school, bool positive) const { return GetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school); }
        void SetResistanceBuffMods(SpellSchools school, bool positive, float val) { SetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val); }
        void ApplyResistanceBuffModsMod(SpellSchools school, bool positive, float val, bool apply) { ApplyModSignedFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val, apply); }
        void ApplyResistanceBuffModsPercentMod(SpellSchools school, bool positive, float val, bool apply) { ApplyPercentModFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val, apply); }
        void InitStatBuffMods()
        {
            for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i) SetFloatValue(UNIT_FIELD_POSSTAT0+i, 0);
            for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i) SetFloatValue(UNIT_FIELD_NEGSTAT0+i, 0);
        }
        void ApplyStatBuffMod(Stats stat, float val, bool apply) { ApplyModSignedFloatValue((val > 0 ? UNIT_FIELD_POSSTAT0+stat : UNIT_FIELD_NEGSTAT0+stat), val, apply); }
        void ApplyStatPercentBuffMod(Stats stat, float val, bool apply)
        {
            ApplyPercentModFloatValue(UNIT_FIELD_POSSTAT0+stat, val, apply);
            ApplyPercentModFloatValue(UNIT_FIELD_NEGSTAT0+stat, val, apply);
        }
        void SetCreateStat(Stats stat, float val) { m_createStats[stat] = val; }
        void SetCreateHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val); }
        uint32 GetCreateHealth() const { return GetUInt32Value(UNIT_FIELD_BASE_HEALTH); }
        void SetCreateMana(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_MANA, val); }
        uint32 GetCreateMana() const { return GetUInt32Value(UNIT_FIELD_BASE_MANA); }
        uint32 GetPowerIndex(uint32 powerType) const;
        int32 GetCreatePowers(Powers power) const;
        float GetPosStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_POSSTAT0+stat); }
        float GetNegStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_NEGSTAT0+stat); }
        float GetCreateStat(Stats stat) const { return m_createStats[stat]; }

        void SetCurrentCastedSpell(Spell* pSpell);
        virtual void ProhibitSpellSchool(SpellSchoolMask /*idSchoolMask*/, uint32 /*unTimeMs*/) { }
        void InterruptSpell(CurrentSpellTypes spellType, bool withDelayed = true, bool withInstant = true);
        void FinishSpell(CurrentSpellTypes spellType, bool ok = true);

        // set withDelayed to true to account delayed spells as casted
        // delayed+channeled spells are always accounted as casted
        // we can skip channeled or delayed checks using flags
        bool IsNonMeleeSpellCasted(bool withDelayed, bool skipChanneled = false, bool skipAutorepeat = false, bool isAutoshoot = false, bool skipInstant = true) const;

        // set withDelayed to true to interrupt delayed spells too
        // delayed+channeled spells are always interrupted
        void InterruptNonMeleeSpells(bool withDelayed, uint32 spellid = 0, bool withInstant = true);
        void InterruptNonMeleeSpellsExcept(bool withDelayed, uint32 except, bool withInstant = true);

        Spell* GetCurrentSpell(CurrentSpellTypes spellType) const { return m_currentSpells[spellType]; }
        Spell* GetCurrentSpell(uint32 spellType) const { return m_currentSpells[spellType]; }
        Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;
        int32 GetCurrentSpellCastTime(uint32 spell_id) const;

        uint32 m_addDmgOnce;
        uint64 m_SummonSlot[MAX_SUMMON_SLOT];
        uint64 m_ObjectSlot[MAX_GAMEOBJECT_SLOT];

        ShapeshiftForm GetShapeshiftForm() const { return ShapeshiftForm(GetByteValue(UNIT_FIELD_BYTES_2, 3)); }
        void SetShapeshiftForm(ShapeshiftForm form)
        {
            SetByteValue(UNIT_FIELD_BYTES_2, 3, form);
        }

        inline bool IsInFeralForm() const
        {
            ShapeshiftForm form = GetShapeshiftForm();
            return form == FORM_CAT || form == FORM_BEAR;
        }

        inline bool IsInDisallowedMountForm() const
        {
            ShapeshiftForm form = GetShapeshiftForm();
            return form != FORM_NONE && form != FORM_BATTLESTANCE && form != FORM_BERSERKERSTANCE && form != FORM_DEFENSIVESTANCE &&
                form != FORM_SHADOW && form != FORM_STEALTH && form != FORM_UNDEAD && form != FORM_WISE_SERPENT && form != FORM_STURDY_OX && form != FORM_FIERCE_TIGER && form != FORM_MOONKIN;
        }

        float m_modMeleeHitChance;
        float m_modRangedHitChance;
        float m_modSpellHitChance;
        int32 m_baseSpellCritChance;

        float m_threatModifier[MAX_SPELL_SCHOOL];
        float m_modAttackSpeedPct[3];

        // Event handler
        EventProcessor m_Events;

        // stat system
        bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
        void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value) { m_auraModifiersGroup[unitMod][modifierType] = value; }
        float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
        float GetTotalStatValue(Stats stat) const;
        float GetTotalAuraModValue(UnitMods unitMod) const;
        SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
        Stats GetStatByAuraGroup(UnitMods unitMod) const;
        Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
        bool CanModifyStats() const { return m_canModifyStats; }
        void SetCanModifyStats(bool modifyStats) { m_canModifyStats = modifyStats; }
        virtual bool UpdateStats(Stats stat) = 0;
        virtual bool UpdateAllStats() = 0;
        virtual void UpdateResistances(uint32 school) = 0;
        virtual void UpdateArmor() = 0;
        virtual void UpdateMaxHealth() = 0;
        virtual void UpdateMaxPower(Powers power) = 0;
        virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
        virtual void UpdateDamagePhysical(WeaponAttackType attType) = 0;
        float GetTotalAttackPowerValue(WeaponAttackType attType) const;
        float GetWeaponDamageRange(WeaponAttackType attType, WeaponDamageRange type) const;
        void SetBaseWeaponDamage(WeaponAttackType attType, WeaponDamageRange damageRange, float value) { m_weaponDamage[attType][damageRange] = value; }

        bool isInFrontInMap(Unit const* target, float distance, float arc = M_PI) const;
        bool isInBackInMap(Unit const* target, float distance, float arc = M_PI) const;

        // Visibility system
        bool IsVisible() const { return (m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GM) > SEC_PLAYER) ? false : true; }
        void SetVisible(bool x);

        // common function for visibility checks for player/creatures with detection code
        void SetPhaseMask(uint32 newPhaseMask, bool update);// overwrite WorldObject::SetPhaseMask
        void UpdateObjectVisibility(bool forced = true);

        void OnRelocated();

        SpellImmuneList m_spellImmune[MAX_SPELL_IMMUNITY];
        uint32 m_lastSanctuaryTime;

        // Threat related methods
        bool CanHaveThreatList() const;
        void AddThreat(Unit* victim, float fThreat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const* threatSpell = NULL);
        float ApplyTotalThreatModifier(float fThreat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
        void DeleteThreatList();
        void TauntApply(Unit* victim);
        void TauntFadeOut(Unit* taunter);
        ThreatManager& getThreatManager() { return m_ThreatManager; }
        void addHatedBy(HostileReference* pHostileReference) { m_HostileRefManager.insertFirst(pHostileReference); };
        void removeHatedBy(HostileReference* /*pHostileReference*/) { /* nothing to do yet */ }
        HostileRefManager& getHostileRefManager() { return m_HostileRefManager; }

        VisibleAuraMap const* GetVisibleAuras() const { return &m_visibleAuras; }

        AuraApplication * GetVisibleAura(uint8 slot)
        {
            auto const itr = m_visibleAuras.find(slot);
            return (itr != m_visibleAuras.end()) ? itr->second : 0;
        }

        AuraApplication const * GetVisibleAura(uint8 slot) const
        {
            auto const itr = m_visibleAuras.find(slot);
            return (itr != m_visibleAuras.end()) ? itr->second : 0;
        }

        void SetVisibleAura(uint8 slot, AuraApplication * aur) { m_visibleAuras[slot]=aur; UpdateAuraForGroup(slot); }
        void RemoveVisibleAura(uint8 slot) { m_visibleAuras.erase(slot); UpdateAuraForGroup(slot); }

        uint32 GetInterruptMask() const { return m_interruptMask; }
        void AddInterruptMask(uint32 mask) { m_interruptMask |= mask; }
        void UpdateInterruptMask();

        uint32 GetDisplayId() const { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }
        void SetDisplayId(uint32 modelId);
        uint32 GetNativeDisplayId() const { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }
        void RestoreDisplayId();
        void SetNativeDisplayId(uint32 modelId) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, modelId); }
        void setTransForm(uint32 spellid) { m_transform = spellid;}
        uint32 getTransForm() const { return m_transform;}

        // DynamicObject management
        void _RegisterDynObject(DynamicObject* dynObj);
        void _UnregisterDynObject(DynamicObject* dynObj);
        DynamicObject* GetDynObject(uint32 spellId);
        int32 CountDynObject(uint32 spellId);
        void GetDynObjectList(std::list<DynamicObject*> &list, uint32 spellId);
        void RemoveDynObject(uint32 spellId);
        void RemoveAllDynObjects();

        // AreaTrigger management
        void _RegisterAreaTrigger(AreaTrigger* areaTrigger);
        void _UnregisterAreaTrigger(AreaTrigger* areaTrigger);
        AreaTrigger* GetAreaTrigger(uint32 spellId);
        int32 CountAreaTrigger(uint32 spellId);
        void GetAreaTriggerList(std::list<AreaTrigger*> &list, uint32 spellId);
        void RemoveAreaTrigger(uint32 spellId);
        void RemoveAllAreasTrigger();

        void AddAuraAreaTrigger(IAreaTrigger* interface);
        IAreaTrigger* RemoveAuraAreaTrigger(AuraEffect const* auraEffect, AuraApplication const* auraApplication);

        GameObject* GetGameObject(uint32 spellId) const;
        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        void RemoveAllGameObjects();

        uint32 CalculateDamage(WeaponAttackType attType, bool normalized, bool addTotalPct);
        float GetAPMultiplier(WeaponAttackType attType, bool normalized);
        void ModifyAuraState(AuraStateType flag, bool apply);
        uint32 BuildAuraStateUpdateForTarget(Unit* target) const;
        bool HasAuraState(AuraStateType flag, SpellInfo const* spellProto = NULL, Unit const* Caster = NULL) const;
        void UnsummonAllTotems();
        Unit* GetMagicHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo);
        Unit* GetMeleeHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo = NULL);

        int32 SpellBaseDamageBonusDone(SpellSchoolMask schoolMask);
        int32 SpellBaseDamageBonusTaken(SpellSchoolMask schoolMask);
        uint32 SpellDamageBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 effIndex, uint32 pdamage, DamageEffectType damagetype, uint32 stack = 1);
        uint32 SpellDamageBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 effIndex, uint32 pdamage, DamageEffectType damagetype, uint32 stack = 1);
        int32 SpellBaseHealingBonusDone(SpellSchoolMask schoolMask);
        int32 SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask);
        uint32 SpellHealingBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 effIndex, uint32 healamount, DamageEffectType damagetype, uint32 stack = 1);
        uint32 SpellHealingBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 effIndex, uint32 healamount, DamageEffectType damagetype, uint32 stack = 1);

        uint32 MeleeDamageBonusDone(Unit *pVictim, uint32 damage, WeaponAttackType attType, SpellInfo const *spellProto = NULL);
        uint32 MeleeDamageBonusTaken(Unit* attacker, uint32 pdamage,WeaponAttackType attType, SpellInfo const *spellProto = NULL);


        bool   isSpellBlocked(Unit* victim, SpellInfo const* spellProto, WeaponAttackType attackType = BASE_ATTACK);
        bool   isBlockCritical();
        bool   isSpellCrit(Unit* victim, SpellInfo const* spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK) const;
        float GetSpellCrit(Unit* victim, SpellInfo const* spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK) const;
        uint32 SpellCriticalDamageBonus(SpellInfo const* spellProto, uint32 damage, Unit* victim);
        uint32 SpellCriticalHealingBonus(SpellInfo const* spellProto, uint32 damage, Unit* victim);

        void SetContestedPvP(Player* attackedPlayer = NULL);

        RemainingPeriodicAmount GetRemainingPeriodicAmount(uint64 caster, uint32 spellId, AuraType auraType, uint8 effectIndex = 0) const;
        int32 GetSplineDuration() const;

        void ApplyAbsoluteImmune(uint32 spellid, bool apply);
        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
        void ApplySpellDispelImmunity(const SpellInfo* spellProto, DispelType type, bool apply);
        virtual bool IsImmunedToSpell(SpellInfo const* spellInfo);
                                                            // redefined in Creature
        bool IsImmunedToDamage(SpellSchoolMask meleeSchoolMask);
        bool IsImmunedToDamage(SpellInfo const* spellInfo);
        virtual bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const;
                                                            // redefined in Creature
        static bool IsDamageReducedByArmor(SpellSchoolMask damageSchoolMask, SpellInfo const* spellInfo = NULL, uint8 effIndex = MAX_SPELL_EFFECTS);
        uint32 CalcArmorReducedDamage(Unit* victim, const uint32 damage, SpellInfo const* spellInfo, WeaponAttackType attackType=MAX_ATTACK);
        void CalcAbsorbResist(Unit* victim, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32 *absorb, uint32 *resist, SpellInfo const* spellInfo = NULL);
        void CalcHealAbsorb(Unit* victim, const SpellInfo* spellProto, uint32 &healAmount, uint32 &absorb);
        bool IsSpellResisted(Unit* victim, SpellSchoolMask schoolMask, SpellInfo const* spellInfo);

        void  UpdateSpeed(UnitMoveType mtype, bool forced);
        float GetSpeed(UnitMoveType mtype) const
        {
            return m_speed_rate[mtype] * (IsControlledByPlayer() ? playerBaseMoveSpeed[mtype] : baseMoveSpeed[mtype]);
        }
        float GetSpeedRate(UnitMoveType mtype) const { return m_speed_rate[mtype]; }
        void SetSpeed(UnitMoveType mtype, float rate, bool forced = false);
        float m_TempSpeed;

        bool isHover() const { return HasAuraType(SPELL_AURA_HOVER); }
        bool isCamouflaged() const { return HasAuraType(SPELL_AURA_MOD_CAMOUFLAGE); }

        float ApplyEffectModifiers(SpellInfo const* spellProto, uint8 effect_index, float value) const;
        int32 CalculateSpellDamage(Unit const* target, SpellInfo const* spellProto, uint8 effect_index, int32 const* basePoints = NULL) const;
        int32 CalcSpellDuration(SpellInfo const* spellProto);
        int32 ModSpellDuration(SpellInfo const* spellProto, Unit const* target, int32 duration, bool positive, uint32 effectMask);
        void  ModSpellCastTime(SpellInfo const* spellProto, int32 & castTime, Spell* spell=NULL);

        void addFollower(FollowerReference* pRef) { m_FollowingRefManager.insertFirst(pRef); }
        void removeFollower(FollowerReference* /*pRef*/) { /* nothing to do yet */ }
        static Unit* GetUnit(WorldObject& object, uint64 guid);
        static Player* GetPlayer(WorldObject& object, uint64 guid);
        static Creature* GetCreature(WorldObject& object, uint64 guid);

        MotionMaster* GetMotionMaster() { return &i_motionMaster; }
        const MotionMaster* GetMotionMaster() const { return &i_motionMaster; }

        bool IsStopped() const { return !(HasUnitState(UNIT_STATE_MOVING)); }
        void StopMoving();

        void AddUnitMovementFlag(uint32 f) { m_movementInfo.flags |= f; }
        void RemoveUnitMovementFlag(uint32 f) { m_movementInfo.flags &= ~f; }
        bool HasUnitMovementFlag(uint32 f) const { return (m_movementInfo.flags & f) == f; }
        uint32 GetUnitMovementFlags() const { return m_movementInfo.flags; }
        void SetUnitMovementFlags(uint32 f)
        {
            m_movementInfo.flags = f;
        }

        void ClearMovementData()
        {
            m_movementInfo.alive32 = 0;
            m_movementInfo.bits[MovementInfo::Bit::FallData] = false;
            m_movementInfo.bits[MovementInfo::Bit::FallDirection] = false;
        }

        void AddExtraUnitMovementFlag(uint16 f) { m_movementInfo.flags2 |= f; }
        void RemoveExtraUnitMovementFlag(uint16 f) { m_movementInfo.flags2 &= ~f; }
        uint16 HasExtraUnitMovementFlag(uint16 f) const { return m_movementInfo.flags2 & f; }
        uint16 GetExtraUnitMovementFlags() const { return m_movementInfo.flags2; }
        void SetExtraUnitMovementFlags(uint16 f) { m_movementInfo.flags2 = f; }
        bool IsSplineEnabled() const;

        void WriteMovementUpdate(WorldPacket &data) const;

        float GetPositionZMinusOffset() const
        {
            float offset = 0.0f;
            if (HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
                offset = GetFloatValue(UNIT_FIELD_HOVERHEIGHT);

            return GetPositionZ() - offset;
        }

        void SetControlled(bool apply, UnitState state);

        void AddComboPointHolder(uint32 lowguid) { m_ComboPointHolders.insert(lowguid); }
        void RemoveComboPointHolder(uint32 lowguid) { m_ComboPointHolders.erase(lowguid); }
        void ClearComboPointHolders();

        ///----------Pet responses methods-----------------
        void SendPetCastFail(uint32 spellid, SpellCastResult msg);
        void SendPetActionFeedback (uint8 msg);
        void SendPetTalk (uint32 pettalk);
        void SendPetAIReaction(uint64 guid);
        ///----------End of Pet responses methods----------

        void propagateSpeedChange() { GetMotionMaster()->propagateSpeedChange(); }

        // reactive attacks
        void ClearAllReactives();
        void StartReactiveTimer(ReactiveType reactive) { m_reactiveTimer[reactive] = REACTIVE_TIMER_START;}
        void UpdateReactives(uint32 p_time);

        // group updates
        void UpdateAuraForGroup(uint8 slot);

        // proc trigger system
        bool CanProc(){return !m_procDeep;}
        void SetCantProc(bool apply)
        {
            if (apply)
                ++m_procDeep;
            else
            {
                ASSERT(m_procDeep);
                --m_procDeep;
            }
        }

        uint32 GetModelForForm(ShapeshiftForm form);
        uint32 GetModelForTotem(PlayerTotemType totemType);

        void SetReducedThreatPercent(uint32 pct, uint64 guid)
        {
            m_reducedThreatPercent = pct;
            m_misdirectionTargetGUID = guid;
        }
        uint32 GetReducedThreatPercent() { return m_reducedThreatPercent; }
        Unit* GetMisdirectionTarget() { return m_misdirectionTargetGUID ? GetUnit(*this, m_misdirectionTargetGUID) : NULL; }

        bool IsAIEnabled, NeedChangeAI;
        uint64 LastCharmerGUID;

        bool CreateVehicleKit(uint32 id, uint32 creatureEntry);
        void RemoveVehicleKit(bool dismount = false);
        Vehicle* GetVehicleKit() const { return m_vehicleKit; }
        Vehicle* GetVehicle() const { return m_vehicle; }
        bool IsOnVehicle() const { return m_vehicle != NULL; }
        bool IsOnVehicle(Unit const *vehicle) const;
        Unit* GetVehicleBase() const;
        Creature* GetVehicleCreatureBase() const;
        float GetTransOffsetX() const { return m_movementInfo.t_pos.GetPositionX(); }
        float GetTransOffsetY() const { return m_movementInfo.t_pos.GetPositionY(); }
        float GetTransOffsetZ() const { return m_movementInfo.t_pos.GetPositionZ(); }
        float GetTransOffsetO() const { return m_movementInfo.t_pos.GetOrientation(); }
        uint32 GetTransTime() const { return m_movementInfo.t_time; }
        int8 GetTransSeat() const { return m_movementInfo.t_seat; }
        uint64 GetTransGUID() const;
        // Returns the transport this unit is on directly (if on vehicle and transport, return vehicle)
        TransportBase* GetDirectTransport() const;

        bool m_ControlledByPlayer;

        bool HandleSpellClick(Unit* clicker, int8 seatId = -1);
        void EnterVehicle(Unit* base, int8 seatId = -1, bool fullTriggered = false);
        void ExitVehicle(Position const* exitPosition = NULL);
        void ChangeSeat(int8 seatId, bool next = true);

        // Should only be called by AuraEffect::HandleAuraControlVehicle(AuraApplication const* auraApp, uint8 mode, bool apply) const;
        void _ExitVehicle(Position const* exitPosition = NULL);
        void _EnterVehicle(Vehicle* vehicle, int8 seatId, AuraApplication const* aurApp = NULL);
        void setVehicleEjectPos(float x, float y, float z, float o) { m_vehicleEjectPos.Relocate(x, y, z, o); }
        void setVehicleEjectPos(Position const* exitPosition) { m_vehicleEjectPos.Relocate(exitPosition); }
        Position getVehicleEjectPos() const { return m_vehicleEjectPos; }

        void BuildMovementPacket(ByteBuffer *data) const;

        bool isMoving() const   { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING); }
        bool isTurning() const  { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_TURNING); }
        virtual bool CanFly() const = 0;
        bool IsFlying() const   { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY); }
        bool canWaterWalk() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WATERWALKING); }
        void SetCanFly(bool apply);
        bool SetSwim(bool enable);

        void RewardRage(float baseRage, bool attacker);

        virtual float GetFollowAngle() const { return static_cast<float>(M_PI/2); }

        void OutDebugInfo() const;
        virtual bool isBeingLoaded() const { return false;}
        bool IsDuringRemoveFromWorld() const {return m_duringRemoveFromWorld;}

        Pet* ToPet() { if (isPet()) return reinterpret_cast<Pet*>(this); else return NULL; }
        Pet const* ToPet() const { if (isPet()) return reinterpret_cast<Pet const*>(this); else return NULL; }

        Totem* ToTotem() { if (isTotem()) return reinterpret_cast<Totem*>(this); else return NULL; }
        Totem const* ToTotem() const { if (isTotem()) return reinterpret_cast<Totem const*>(this); else return NULL; }

        TempSummon* ToTempSummon() { if (IsSummon()) return reinterpret_cast<TempSummon*>(this); else return NULL; }
        TempSummon const* ToTempSummon() const { if (IsSummon()) return reinterpret_cast<TempSummon const*>(this); else return NULL; }

        bool isAINotifyScheduled() const { return m_AINotifyScheduled; }
        void setAINotifyScheduled(bool val) { m_AINotifyScheduled = val; }

        void SetTarget(uint64 guid)
        {
            if (!_focusSpell)
                SetUInt64Value(UNIT_FIELD_TARGET, guid);
        }

        // Handling caster facing during spell cast
        void FocusTarget(Spell const* focusSpell, uint64 target);
        void ReleaseFocus(Spell const* focusSpell);

        void SetEclipsePower(int32 power, bool send = true);

        uint32 GetNpcDamageTakenInPastSecs(uint32 secs) const;
        uint32 GetPlayerDamageTakenInPastSecs(uint32 secs) const;
        uint32 GetDamageTakenInPastSecs(uint32 secs) const;
        uint32 GetTotalDamageTakenFromPlayer(uint64 guid) { return m_playerTotalDamage[guid]; }

        // Movement info
        Movement::MoveSpline *movespline;

        // helper for dark simulacrum spell
        Unit* GetSimulacrumTarget();
        void setSimulacrumTarget(uint64 guid) { simulacrumTargetGUID = guid; }
        void removeSimulacrumTarget() { simulacrumTargetGUID = 0; }

        void SetInKillingProcess(bool killingPhase) { m_IsInKillingProcess = killingPhase; }
        bool IsInKillingProcess() const { return m_IsInKillingProcess; }

        void DisableHealthRegen() { m_disableHealthRegen = true; }
        void ReenableHealthRegen() { m_disableHealthRegen = false; }
        bool HealthRegenIsDisable() const { return m_disableHealthRegen; }
    
        void DisableEvadeMode() { m_disableEnterEvadeMode = true; }
        void ReenableEvadeMode() { m_disableEnterEvadeMode = false; }
        bool EvadeModeIsDisable() const { return m_disableEnterEvadeMode; }

    protected:
        explicit Unit(bool isWorldObject);

        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const;

        UnitAI* i_AI, *i_disabledAI;

        void _UpdateSpells(uint32 time);
        void _DeleteRemovedAuras();

        void _UpdateAutoRepeatSpell();

        bool m_AutoRepeatFirstCast;

        uint32 m_attackTimer[MAX_ATTACK];

        float m_createStats[MAX_STATS];

        AttackerSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        int32 m_procDeep;

        typedef std::list<DynamicObject*> DynObjectList;
        DynObjectList m_dynObj;

        typedef std::list<AreaTrigger*> AreaTriggerList;
        AreaTriggerList m_AreaTrigger;

        typedef std::list<IAreaTrigger*> AuraAreaTriggerList;
        AuraAreaTriggerList m_auraAreaTriggers;

        typedef std::list<GameObject*> GameObjectList;
        GameObjectList m_gameObj;
        bool m_isSorted;
        uint32 m_transform;

        Spell* m_currentSpells[CURRENT_MAX_SPELL];

        AuraMap m_ownedAuras;
        AuraApplicationMap m_appliedAuras;
        AuraList m_removedAuras;
        AuraMap::iterator m_auraUpdateIterator;
        uint32 m_removedAurasCount;

        AuraEffectList m_modAuras[TOTAL_AURAS];
        AuraList m_scAuras;                        // casted singlecast auras
        AuraApplicationList m_interruptableAuras;             // auras which have interrupt mask applied on unit
        AuraStateAurasMap m_auraStateAuras;        // Used for improve performance of aura state checks on aura apply/remove
        uint32 m_interruptMask;
        AuraIdList _SoulSwapDOTList;

        float m_auraModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_END];
        float m_weaponDamage[MAX_ATTACK][2];
        bool m_canModifyStats;
        VisibleAuraMap m_visibleAuras;

        float m_speed_rate[MAX_MOVE_TYPE];

        CharmInfo* m_charmInfo;
        SharedVisionList m_sharedVision;

        virtual SpellSchoolMask GetMeleeDamageSchoolMask() const;

        MotionMaster i_motionMaster;

        uint32 m_reactiveTimer[MAX_REACTIVE];

        ThreatManager m_ThreatManager;

        Vehicle* m_vehicle;
        Vehicle* m_vehicleKit;

        uint32 m_unitTypeMask;
        LiquidTypeEntry const* _lastLiquid;

        bool m_IsInKillingProcess;

        Position m_vehicleEjectPos;

        bool m_disableHealthRegen;
        bool m_disableEnterEvadeMode;

        bool IsAlwaysVisibleFor(WorldObject const* seer) const;
        bool IsAlwaysDetectableFor(WorldObject const* seer) const;

        void DisableSpline();

    private:
        bool IsTriggeredAtSpellProcEvent(Unit* victim, Aura *aura, SpellInfo const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, bool active, SpellProcEventEntry const* & spellProcEvent);
        bool HandleDummyAuraProc(Unit* victim, uint32 damage, uint32 absorb, AuraEffect *triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleAuraProcOnPowerAmount(Unit* victim, uint32 damage, AuraEffect *triggeredByAura, SpellInfo const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleAuraProc(Unit* victim, uint32 damage, Aura *triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown, bool * handled);
        bool HandleProcTriggerSpell(Unit* victim, uint32 damage, AuraEffect *triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown, bool procSpellIsHeal = false);
        bool HandleOverrideClassScriptAuraProc(Unit* victim, uint32 damage, AuraEffect *triggeredByAura, SpellInfo const* procSpell, uint32 cooldown);
        bool HandleAuraRaidProcFromChargeWithValue(AuraEffect *triggeredByAura);
        bool HandleAuraRaidProcFromCharge(AuraEffect *triggeredByAura);

        void UpdateSplineMovement(uint32 t_diff);
        void UpdateSplinePosition();

        // player or player's pet
        float GetCombatRatingReduction(CombatRating cr) const;
        uint32 GetCombatRatingDamageReduction(CombatRating cr, float cap, uint32 damage) const;

    protected:
        void SendMoveRoot(uint32 value);
        void SendMoveUnroot(uint32 value);
        void SetFeared(bool apply);
        void SetConfused(bool apply);
        void SetStunned(bool apply);
        void SetRooted(bool apply);

    private:
        Position m_lastVisibilityUpdPos;
        bool m_AINotifyScheduled;

        uint32 m_rootTimes;

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_CombatTimer;
        TimeTrackerSmall m_movesplineTimer;

        uint64 simulacrumTargetGUID;

        Diminishing m_Diminishing;
        // Manage all Units that are threatened by us
        HostileRefManager m_HostileRefManager;

        FollowerRefManager m_FollowingRefManager;

        ComboPointHolderSet m_ComboPointHolders;

        uint32 m_reducedThreatPercent;
        uint64 m_misdirectionTargetGUID;

        bool m_cleanupDone; // lock made to not add stuff after cleanup before delete
        bool m_duringRemoveFromWorld; // lock made to not add stuff after beginning removing from world

        Spell const* _focusSpell;   ///> Locks the target during spell cast for proper facing
        bool _isWalkingBeforeCharm; // Are we walking before we were charmed?

        time_t _lastDamagedTime;

        int32 damageTrackingTimer_;

        std::array<uint32, DAMAGE_TRACKING_PERIOD> playerDamageTaken_;
        std::array<uint32, DAMAGE_TRACKING_PERIOD> npcDamageTaken_;

        // used to track total damage each player has made to the unit
        std::map<uint64, uint32> m_playerTotalDamage;
};

namespace Trinity
{
    class DelayedCastEvent final : public BasicEvent
    {
    public:
        DelayedCastEvent(Unit * caster, const Unit * target, uint32 spellId, bool triggered) :
            _caster(caster), _spellId(spellId), _triggered(triggered)
        {
            _unitTargetGUID = target->GetGUID();
        }

        DelayedCastEvent(Unit * caster, const Position * destPos, uint32 spellId, bool triggered) :
            _caster(caster), _spellId(spellId), _triggered(triggered)
        {
            _unitTargetGUID = 0;
            pos.Relocate(destPos);
        }

        bool Execute(uint64, uint32) final
        {
            bool destTarget = !_unitTargetGUID;

            if (!_unitTargetGUID)
                _caster->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), _spellId, _triggered);
            else if (Unit * target = Unit::GetUnit(*_caster, _unitTargetGUID))
                _caster->CastSpell(target, _spellId, _triggered);
            return true;
        }

    private:
        Unit * _caster;
        bool _triggered;
        uint32 _spellId;
        uint64 _unitTargetGUID;
        Position pos;

    };

    // Binary predicate for sorting Units based on percent value of a power
    class PowerPctOrderPred
    {
        public:
            PowerPctOrderPred(Powers power, bool ascending = true) : m_power(power), m_ascending(ascending) {}
            bool operator() (const Unit* a, const Unit* b) const
            {
                float rA = a->GetMaxPower(m_power) ? float(a->GetPower(m_power)) / float(a->GetMaxPower(m_power)) : 0.0f;
                float rB = b->GetMaxPower(m_power) ? float(b->GetPower(m_power)) / float(b->GetMaxPower(m_power)) : 0.0f;
                return m_ascending ? rA < rB : rA > rB;
            }
        private:
            const Powers m_power;
            const bool m_ascending;
    };

    // Binary predicate for sorting Units based on percent value of health
    class HealthPctOrderPred
    {
        public:
            HealthPctOrderPred(bool ascending = true) : m_ascending(ascending) {}
            bool operator() (const Unit* a, const Unit* b) const
            {
                float rA = a->GetMaxHealth() ? float(a->GetHealth()) / float(a->GetMaxHealth()) : 0.0f;
                float rB = b->GetMaxHealth() ? float(b->GetHealth()) / float(b->GetMaxHealth()) : 0.0f;
                return m_ascending ? rA < rB : rA > rB;
            }
        private:
            const bool m_ascending;
    };

    // Binary predicate for sorting Units based on value of distance of an GameObject
    class DistanceCompareOrderPred
    {
        public:
            DistanceCompareOrderPred(const WorldObject* object, bool ascending = true) : m_object(object), m_ascending(ascending) {}
            bool operator() (const Unit* a, const Unit* b) const
            {
                return m_ascending ? a->GetDistance(m_object) < b->GetDistance(m_object) :
                    a->GetDistance(m_object) > b->GetDistance(m_object);
            }
        private:
            const WorldObject* m_object;
            const bool m_ascending;
    };
}

#endif
