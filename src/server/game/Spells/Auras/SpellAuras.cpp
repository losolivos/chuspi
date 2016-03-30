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

#include "SpellAuras.h"
#include "Common.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "DynamicObject.h"
#include "ObjectAccessor.h"
#include "Util.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Vehicle.h"
#include "ObjectVisitors.hpp"

AuraApplication::AuraApplication(Unit* target, Unit* caster, Aura *aura, uint32 effMask)
    : _target(target)
    , _base(aura)
    , _removeMode(AURA_REMOVE_NONE)
    , _slot(MAX_AURAS)
    , _flags(AFLAG_NONE)
    , _effectMask(0)
    , _effectsToApply(effMask)
    , _needClientUpdate(false)
{
    ASSERT(GetTarget() && GetBase());

    if (GetBase()->CanBeSentToClient())
    {
        // Try find slot for aura
        uint8 slot = MAX_AURAS;
        // Lookup for auras already applied from spell
        if (AuraApplication * foundAura = GetTarget()->GetAuraApplication(GetBase()->GetId(), GetBase()->GetCasterGUID(), GetBase()->GetCastItemGUID()))
        {
            // allow use single slot only by auras from same caster
            slot = foundAura->GetSlot();
        }
        else
        {
            Unit::VisibleAuraMap const* visibleAuras = GetTarget()->GetVisibleAuras();
            // lookup for free slots in units visibleAuras
            Unit::VisibleAuraMap::const_iterator itr = visibleAuras->find(0);
            for (uint32 freeSlot = 0; freeSlot < MAX_AURAS; ++itr, ++freeSlot)
            {
                if (itr == visibleAuras->end() || itr->first != freeSlot)
                {
                    slot = freeSlot;
                    break;
                }
            }
        }

        // Register Visible Aura
        if (slot < MAX_AURAS)
        {
            _slot = slot;
            GetTarget()->SetVisibleAura(slot, this);
            SetNeedClientUpdate();
        }
    }

    _InitFlags(caster, effMask);
}

void AuraApplication::_Remove()
{
    uint8 slot = GetSlot();

    if (slot >= MAX_AURAS)
        return;

    if (AuraApplication * foundAura = _target->GetAuraApplication(GetBase()->GetId(), GetBase()->GetCasterGUID(), GetBase()->GetCastItemGUID()))
    {
        // Reuse visible aura slot by aura which is still applied - prevent storing dead pointers
        if (slot == foundAura->GetSlot())
        {
            if (GetTarget()->GetVisibleAura(slot) == this)
            {
                GetTarget()->SetVisibleAura(slot, foundAura);
                foundAura->SetNeedClientUpdate();
            }
            // set not valid slot for aura - prevent removing other visible aura
            slot = MAX_AURAS;
        }
    }

    // update for out of range group members
    if (slot < MAX_AURAS)
    {
        GetTarget()->RemoveVisibleAura(slot);
        ClientUpdate(true);
    }
}

void AuraApplication::_InitFlags(Unit* caster, uint32 effMask)
{
    // mark as selfcasted if needed
    _flags |= (GetBase()->GetCasterGUID() == GetTarget()->GetGUID()) ? AFLAG_CASTER : AFLAG_NONE;

    // aura is casted by self or an enemy
    // one negative effect and we know aura is negative
    if (IsSelfcasted() || !caster || !caster->IsFriendlyTo(GetTarget()))
    {
        bool negativeFound = false;
        for (uint8 i = 0; i < GetBase()->GetSpellInfo()->Effects.size(); ++i)
        {
            if (((1<<i) & effMask) && !GetBase()->GetSpellInfo()->IsPositiveEffect(i))
            {
                negativeFound = true;
                break;
            }
        }
        _flags |= negativeFound ? AFLAG_NEGATIVE : AFLAG_POSITIVE;
    }
    // aura is casted by friend
    // one positive effect and we know aura is positive
    else
    {
        bool positiveFound = false;
        for (uint8 i = 0; i < GetBase()->GetSpellInfo()->Effects.size(); ++i)
        {
            if (((1<<i) & effMask) && GetBase()->GetSpellInfo()->IsPositiveEffect(i))
            {
                positiveFound = true;
                break;
            }
        }
        _flags |= positiveFound ? AFLAG_POSITIVE : AFLAG_NEGATIVE;
    }

    if (GetBase()->shouldSendEffectAmount())
        _flags |= AFLAG_ANY_EFFECT_AMOUNT_SENT;
}

void AuraApplication::_HandleEffect(uint8 effIndex, bool apply)
{
    AuraEffect *aurEff = GetBase()->GetEffect(effIndex);
    ASSERT(aurEff);
    ASSERT(HasEffect(effIndex) == (!apply));
    ASSERT((1<<effIndex) & _effectsToApply);
    TC_LOG_DEBUG("spells", "AuraApplication::_HandleEffect: %u, apply: %u: amount: %u", aurEff->GetAuraType(), apply, aurEff->GetAmount());

    if (apply)
    {
        ASSERT(!(_effectMask & (1<<effIndex)));
        _effectMask |= 1<<effIndex;
        aurEff->HandleEffect(this, AURA_EFFECT_HANDLE_REAL, true);
    }
    else
    {
        ASSERT(_effectMask & (1<<effIndex));
        _effectMask &= ~(1<<effIndex);
        aurEff->HandleEffect(this, AURA_EFFECT_HANDLE_REAL, false);

        // Remove all triggered by aura spells vs unlimited duration
        aurEff->CleanupTriggeredSpells(GetTarget());
    }
    SetNeedClientUpdate();
}

void AuraApplication::BuildBitsUpdatePacket(ByteBuffer& data, bool remove) const
{
    data.WriteBit(!remove);

    if (remove)
        return;

    Aura const *aura = GetBase();
    uint32 flags = _flags;
    if (aura->GetMaxDuration() > 0 && !(aura->GetSpellInfo()->AttributesEx5 & SPELL_ATTR5_HIDE_DURATION))
        flags |= AFLAG_DURATION;

    uint8 count = 0;
    if (flags & AFLAG_ANY_EFFECT_AMOUNT_SENT)
    {
        for (uint8 i = 0; i < aura->GetSpellInfo()->Effects.size(); ++i)
        {
            if (aura->GetEffect(i))
                ++count;
        }
    }

    data.WriteBit(flags & AFLAG_DURATION); // duration
    data.WriteBits(count, 22);
    data.WriteBit((flags & AFLAG_CASTER) == 0);

    if (!(flags & AFLAG_CASTER))
    {
        ObjectGuid casterGuid = aura->GetCasterGUID();
        data.WriteBitSeq<3, 0, 2, 6, 5, 7, 4, 1>(casterGuid);
    }

    data.WriteBit(flags & AFLAG_DURATION); // max duration
    data.WriteBits(0, 22); // second effect count
}

void AuraApplication::BuildBytesUpdatePacket(ByteBuffer& data, bool remove, uint32 overrideSpell) const
{
    if (remove)
    {
        data << uint8(_slot);
        return;
    }

    Aura const *aura = GetBase();
    uint32 flags = _flags;
    if (aura->GetMaxDuration() > 0 && !(aura->GetSpellInfo()->AttributesEx5 & SPELL_ATTR5_HIDE_DURATION))
        flags |= AFLAG_DURATION;

    if (flags & AFLAG_DURATION)
        data << uint32(aura->GetMaxDuration());

    if (!(flags & AFLAG_CASTER))
    {
        ObjectGuid casterGuid = aura->GetCasterGUID();
        data.WriteByteSeq<0, 7, 5, 6, 1, 3, 2, 4>(casterGuid);
    }

    data << uint8(flags);

    if (flags & AFLAG_ANY_EFFECT_AMOUNT_SENT)
    {
        for (uint8 i = 0; i < aura->GetSpellInfo()->Effects.size(); ++i)
        {
            if (AuraEffect const *eff = aura->GetEffect(i))
                data << float(eff->GetAmount());
        }
    }

    if (overrideSpell != 0)
        data << uint32(overrideSpell);
    else
        data << uint32(aura->GetId());

    if (flags & AFLAG_DURATION)
        data << uint32(aura->GetDuration());

    // effect value 2 for

    data << uint8(aura->GetSpellInfo()->StackAmount ? aura->GetStackAmount() : aura->GetCharges());
    data << uint32(GetEffectMask());
    data << uint16(aura->GetCasterLevel());
    data << uint8(_slot);
}

void AuraApplication::ClientUpdate(bool remove)
{
    _needClientUpdate = false;

    bool powerData = false;
    ObjectGuid targetGuid = GetTarget()->GetGUID();

    WorldPacket data(SMSG_AURA_UPDATE);

    data.WriteBit(false); // full update bit
    data.WriteBitSeq<6, 1, 0>(targetGuid);
    data.WriteBits(1, 24); // aura counter
    data.WriteBitSeq<2, 4>(targetGuid);
    data.WriteBit(powerData); // has power data, don't care about it ?
#if 0
    if (powerData)
    {
        packet.StartBitStream(guid2, 7, 0, 6);
        powerCounter = packet.ReadBits(21);
        packet.StartBitStream(guid2, 3, 1, 2, 4, 5);
    }
#endif
    data.WriteBitSeq<7, 3, 5>(targetGuid);

    BuildBitsUpdatePacket(data, remove);
    BuildBytesUpdatePacket(data, remove);
#if 0
    if (powerData)
    {
        packet.ReadXORBytes(guid2, 7, 4, 5, 1, 6);

        for (var i = 0; i < powerCounter; ++i)
        {
            packet.ReadInt32("Power Value", i);
            packet.ReadEnum<PowerType>("Power Type", TypeCode.UInt32, i);
        }

        packet.ReadInt32("Attack power");
        packet.ReadInt32("Spell power");
        packet.ReadXORBytes(guid2, 3);
        packet.ReadInt32("Current Health");
        packet.ReadXORBytes(guid2, 0, 2);
        packet.WriteGuid("PowerUnitGUID", guid2);
    }
#endif
    data.WriteByteSeq<0, 4, 3, 7, 5, 6, 2, 1>(targetGuid);

    _target->SendMessageToSet(&data, true);

    if (remove || _target->GetTypeId() != TYPEID_PLAYER)
        return;

    auto const aura = GetBase();
    if (aura->GetSpellInfo()->Attributes & SPELL_ATTR0_HIDDEN_CLIENTSIDE)
        return;

    uint32 mechanic = MECHANIC_NONE;
    uint8 effectIndex;

    for (uint8 i = 0; i < aura->GetSpellInfo()->Effects.size(); ++i)
    {
        auto const eff = aura->GetEffect(i);
        if (!eff)
            continue;

        switch (eff->GetAuraType())
        {
            case SPELL_AURA_MOD_CONFUSE:
                mechanic = MECHANIC_DISORIENTED;
                break;
            case SPELL_AURA_MOD_FEAR:
            case SPELL_AURA_MOD_FEAR_2:
                mechanic = MECHANIC_FEAR;
                break;
            case SPELL_AURA_MOD_STUN:
                mechanic = MECHANIC_STUN;
                break;
            case SPELL_AURA_MOD_ROOT:
                mechanic = MECHANIC_ROOT;
                break;
            case SPELL_AURA_TRANSFORM:
                mechanic = MECHANIC_POLYMORPH;
                break;
            case SPELL_AURA_MOD_SILENCE:
                mechanic = MECHANIC_SILENCE;
                break;
            case SPELL_AURA_MOD_DISARM:
            case SPELL_AURA_MOD_DISARM_OFFHAND:
            case SPELL_AURA_MOD_DISARM_RANGED:
                mechanic = MECHANIC_DISARM;
                break;
            default:
                break;
        }

        if (mechanic != MECHANIC_NONE)
        {
            effectIndex = i;
            break;
        }
    }

    if (mechanic == MECHANIC_NONE)
        return;

    data.Initialize(SMSG_LOSS_OF_CONTROL_AURA_UPDATE, 7);

    data.WriteBits(1, 22);
    data.WriteBits(mechanic, 8);
    data.WriteBits(mechanic, 8);
    data.FlushBits();
    data << uint8(GetSlot());
    data << uint8(effectIndex);

    _target->ToPlayer()->SendDirectMessage(&data);
}

void AuraApplication::SendFakeAuraUpdate(uint32 auraId, bool remove)
{
    bool powerData = false;
    ObjectGuid targetGuid = GetTarget()->GetGUID();
    WorldPacket data(SMSG_AURA_UPDATE);

    data.WriteBit(false); // full update bit
    data.WriteBitSeq<6, 1, 0>(targetGuid);
    data.WriteBits(1, 24); // aura counter
    data.WriteBitSeq<2, 4>(targetGuid);
    data.WriteBit(powerData); // has power data, don't care about it ?

    if (powerData)
    {
        //packet.StartBitStream(guid2, 7, 0, 6);
        //powerCounter = packet.ReadBits(21);
        //packet.StartBitStream(guid2, 3, 1, 2, 4, 5);
    }

    data.WriteBitSeq<7, 3, 5>(targetGuid);

    BuildBitsUpdatePacket(data, remove);
    BuildBytesUpdatePacket(data, remove, auraId);

    if (powerData)
    {
        //packet.ReadXORBytes(guid2, 7, 4, 5, 1, 6);

        //for (var i = 0; i < powerCounter; ++i)
        //{
            //packet.ReadInt32("Power Value", i);
            //packet.ReadEnum<PowerType>("Power Type", TypeCode.UInt32, i);
        //}

        //packet.ReadInt32("Attack power");
        //packet.ReadInt32("Spell power");
        //packet.ReadXORBytes(guid2, 3);
        //packet.ReadInt32("Current Health");

        //packet.ReadXORBytes(guid2, 0, 2);
        //packet.WriteGuid("PowerUnitGUID", guid2);
    }

    data.WriteByteSeq<0, 4, 3, 7, 5, 6, 2, 1>(targetGuid);

    _target->SendMessageToSet(&data, true);
 }

uint32 Aura::BuildEffectMaskForOwner(SpellInfo const* spellProto, uint32 avalibleEffectMask, WorldObject* owner)
{
    ASSERT(spellProto);
    ASSERT(owner);
    uint32 effMask = 0;
    switch (owner->GetTypeId())
    {
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            for (uint8 i = 0; i < spellProto->Effects.size(); ++i)
            {
                if (spellProto->Effects[i].IsUnitOwnedAuraEffect())
                    effMask |= 1 << i;
            }
            break;
        case TYPEID_DYNAMICOBJECT:
            for (uint8 i = 0; i < spellProto->Effects.size(); ++i)
            {
                if (spellProto->Effects[i].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA || spellProto->Effects[i].Effect == SPELL_EFFECT_CREATE_AREATRIGGER)
                    effMask |= 1 << i;
            }
            break;
        default:
            break;
    }
    return effMask & avalibleEffectMask;
}

Aura *Aura::TryRefreshStackOrCreate(SpellInfo const* spellproto, uint32 tryEffMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32* baseAmount /*= NULL*/, Item* castItem /*= NULL*/, uint64 casterGUID /*= 0*/, bool* refresh /*= NULL*/)
{
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || casterGUID);
    if (refresh)
        *refresh = false;
    uint32 effMask = Aura::BuildEffectMaskForOwner(spellproto, tryEffMask, owner);
    if (!effMask)
        return NULL;

    Aura *foundAura = owner->ToUnit()->_TryStackingOrRefreshingExistingAura(spellproto, effMask, caster, baseAmount, castItem, casterGUID);
    if (foundAura != NULL)
    {
        // we've here aura, which script triggered removal after modding stack amount
        // check the state here, so we won't create new Aura object
        if (foundAura->IsRemoved())
            return NULL;

        // Earthgrab Totem : Don't refresh root
        if (foundAura->GetId() == 64695)
            return NULL;

        if (refresh)
            *refresh = true;
        return foundAura;
    }
    else
        return Create(spellproto, effMask, owner, caster, spellPowerData, baseAmount, castItem, casterGUID);
}

Aura *Aura::TryCreate(SpellInfo const* spellproto, uint32 tryEffMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32* baseAmount /*= NULL*/, Item* castItem /*= NULL*/, uint64 casterGUID /*= 0*/)
{
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || casterGUID);
    uint32 effMask = Aura::BuildEffectMaskForOwner(spellproto, tryEffMask, owner);
    if (!effMask)
        return NULL;
    return Create(spellproto, effMask, owner, caster, spellPowerData, baseAmount, castItem, casterGUID);
}

Aura *Aura::Create(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32* baseAmount, Item* castItem, uint64 casterGUID)
{
    ASSERT(effMask);
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || casterGUID);
    // try to get caster of aura
    if (casterGUID)
    {
        if (owner->GetGUID() == casterGUID)
            caster = owner->ToUnit();
        else
            caster = ObjectAccessor::GetUnit(*owner, casterGUID);
    }
    else
        casterGUID = caster->GetGUID();

    // check if aura can be owned by owner
    if (owner->isType(TYPEMASK_UNIT))
        if (!owner->IsInWorld() || ((Unit*)owner)->IsDuringRemoveFromWorld())
            // owner not in world so don't allow to own not self casted single target auras
            if (casterGUID != owner->GetGUID() && spellproto->IsSingleTarget())
                return NULL;

    Aura *aura = NULL;
    switch (owner->GetTypeId())
    {
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            aura = new UnitAura(spellproto, effMask, owner, caster, spellPowerData, baseAmount, castItem, casterGUID);
            break;
        case TYPEID_DYNAMICOBJECT:
            aura = new DynObjAura(spellproto, effMask, owner, caster, spellPowerData, baseAmount, castItem, casterGUID);
            break;
        default:
            ASSERT(false);
            return NULL;
    }
    // aura can be removed in Unit::_AddAura call
    if (aura->IsRemoved())
        return NULL;

    return aura;
}

Aura::Aura(SpellInfo const* spellproto, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, Item* castItem, uint64 casterGUID) :
m_spellInfo(spellproto), m_casterGuid(casterGUID ? casterGUID : caster->GetGUID()),
m_castItemGuid(castItem ? castItem->GetGUID() : 0), m_applyTime(time(NULL)),
m_owner(owner), m_timeCla(0), m_updateTargetMapInterval(0),
m_casterLevel(caster ? caster->getLevel() : m_spellInfo->SpellLevel), m_procCharges(0), m_stackAmount(1),
m_isRemoved(false), m_isSingleTarget(false), m_isUsingCharges(false)
{
    LoadScripts();

    if (spellPowerData->CostPerSecond || spellPowerData->CostPerSecondPercentage)
        m_timeCla = 1 * IN_MILLISECONDS;

    m_maxDuration = CalcMaxDuration(caster);
    m_duration = m_maxDuration;
    m_procCharges = CalcMaxCharges(caster);
    m_isUsingCharges = m_procCharges != 0;
    // m_casterLevel = cast item level/caster level, caster level should be saved to db, confirmed with sniffs
}

AuraScript* Aura::GetScriptByName(std::string const& scriptName) const
{
    for (std::list<AuraScript*>::const_iterator itr = m_loadedScripts.begin(); itr != m_loadedScripts.end(); ++itr)
        if ((*itr)->_GetScriptName()->compare(scriptName) == 0)
            return *itr;
    return NULL;
}

void Aura::_InitEffects(uint32 effMask, Unit* caster, int32 *baseAmount)
{
    CallScriptInitEffectsHandlers(effMask);

    m_effects.resize(GetSpellInfo()->Effects.size());
    for (size_t i = 0; i < m_effects.size(); ++i)
        if (effMask & (uint32(1) << i))
            m_effects[i].reset(new AuraEffect(this, i, baseAmount ? baseAmount + i : NULL, caster));
}

Aura::~Aura()
{
    // unload scripts
    while (!m_loadedScripts.empty())
    {
        auto itr = m_loadedScripts.begin();
        (*itr)->_Unload();
        delete (*itr);
        m_loadedScripts.erase(itr);
    }

    // free effects memory
    m_effects.clear();

    ASSERT(m_applications.empty());
    _DeleteRemovedApplications();
}

Unit* Aura::GetCaster() const
{
    if (GetOwner()->GetGUID() == GetCasterGUID())
        return GetUnitOwner();
    if (AuraApplication const* aurApp = GetApplicationOfTarget(GetCasterGUID()))
        return aurApp->GetTarget();

    return ObjectAccessor::GetUnit(*GetOwner(), GetCasterGUID());
}

AuraObjectType Aura::GetType() const
{
    return (m_owner && m_owner->GetTypeId() == TYPEID_DYNAMICOBJECT) ? DYNOBJ_AURA_TYPE : UNIT_AURA_TYPE;
}

void Aura::_ApplyForTarget(Unit* target, Unit* caster, AuraApplication * auraApp)
{
    //ASSERT(target);
    //ASSERT(auraApp);
    // aura mustn't be already applied on target
    //ASSERT (!IsAppliedOnTarget(target->GetGUID()) && "Aura::_ApplyForTarget: aura musn't be already applied on target");

    m_applications[target->GetGUID()] = auraApp;

    // set infinity cooldown state for spells
    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_spellInfo->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE)
        {
            Item* castItem = m_castItemGuid ? caster->ToPlayer()->GetItemByGuid(m_castItemGuid) : NULL;
            caster->ToPlayer()->AddSpellAndCategoryCooldowns(m_spellInfo, castItem ? castItem->GetEntry() : 0, NULL, true);
        }
    }
}

void Aura::_UnapplyForTarget(Unit* target, Unit* caster, AuraApplication * auraApp)
{
    ASSERT(target);
    ASSERT(auraApp->GetRemoveMode());
    ASSERT(auraApp);

    ApplicationMap::iterator itr = m_applications.find(target->GetGUID());

    // TODO: Figure out why this happens
    if (itr == m_applications.end())
    {
        TC_LOG_ERROR("spells", "Aura::_UnapplyForTarget, target:%u, caster:%u, spell:%u was not found in owners application map!",
        target->GetGUIDLow(), caster ? caster->GetGUIDLow() : 0, auraApp->GetBase()->GetSpellInfo()->Id);
        ASSERT(false);
    }

    // aura has to be already applied
    ASSERT(itr->second == auraApp);
    m_applications.erase(itr);

    m_removedApplications.push_back(auraApp);

    // reset cooldown state for spells
    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetSpellInfo()->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE)
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existed cases)
        {
            caster->ToPlayer()->SendCooldownEvent(GetSpellInfo());
            // Glyph of Misdirection must remove cooldown after
            if (GetSpellInfo()->Id == 34477 && caster->HasAura(56829) && caster->GetMisdirectionTarget() && caster->GetPetGUID() == caster->GetMisdirectionTarget()->GetGUID())
                caster->ToPlayer()->RemoveSpellCooldown(34477, true);
        }
    }
}

// removes aura from all targets
// and marks aura as removed
void Aura::_Remove(AuraRemoveMode removeMode)
{
    ASSERT (!m_isRemoved);
    m_isRemoved = true;

    ApplicationMap::iterator appItr = m_applications.begin();
    for (appItr = m_applications.begin(); appItr != m_applications.end();)
    {
        AuraApplication * aurApp = appItr->second;
        Unit* target = aurApp->GetTarget();
        target->_UnapplyAura(aurApp, removeMode);
        appItr = m_applications.begin();
    }
}

void Aura::UpdateTargetMap(Unit* caster, bool apply)
{
    if (IsRemoved())
        return;

    m_updateTargetMapInterval = UPDATE_TARGET_MAP_INTERVAL;

    // fill up to date target list
    //       target, effMask
    std::map<Unit*, uint32> targets;

    FillTargetMap(targets, caster);

    UnitList targetsToRemove;

    // mark all auras as ready to remove
    for (ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end();++appIter)
    {
        std::map<Unit*, uint32>::iterator existing = targets.find(appIter->second->GetTarget());
        // not found in current area - remove the aura
        if (existing == targets.end())
            targetsToRemove.push_back(appIter->second->GetTarget());
        else
        {
            // needs readding - remove now, will be applied in next update cycle
            // (dbcs do not have auras which apply on same type of targets but have different radius, so this is not really needed)
            if (appIter->second->GetEffectMask() != existing->second || !CanBeAppliedOn(existing->first))
                targetsToRemove.push_back(appIter->second->GetTarget());
            // nothing todo - aura already applied
            // remove from auras to register list
            targets.erase(existing);
        }
    }

    // register auras for units
    for (std::map<Unit*, uint32>::iterator itr = targets.begin(); itr!= targets.end();)
    {
        // aura mustn't be already applied on target
        if (AuraApplication * aurApp = GetApplicationOfTarget(itr->first->GetGUID()))
        {
            // the core created 2 different units with same guid
            // this is a major failue, which i can't fix right now
            // let's remove one unit from aura list
            // this may cause area aura "bouncing" between 2 units after each update
            // but because we know the reason of a crash we can remove the assertion for now
            if (aurApp->GetTarget() != itr->first)
            {
                // remove from auras to register list
                targets.erase(itr++);
                continue;
            }
            else
            {
                // ok, we have one unit twice in target map (impossible, but...)
                ASSERT(false);
            }
        }

        bool addUnit = true;
        // check target immunities
        for (uint8 effIndex = 0; effIndex < GetSpellInfo()->Effects.size(); ++effIndex)
        {
            if (itr->first->IsImmunedToSpellEffect(GetSpellInfo(), effIndex))
                itr->second &= ~(1 << effIndex);
        }
        if (!itr->second
            || itr->first->IsImmunedToSpell(GetSpellInfo())
            || !CanBeAppliedOn(itr->first))
            addUnit = false;

        if (addUnit)
        {
            // persistent area aura does not hit flying targets
            if (GetType() == DYNOBJ_AURA_TYPE)
            {
                if (itr->first->isInFlight())
                    addUnit = false;
            }
            // unit auras can not stack with each other
            else // (GetType() == UNIT_AURA_TYPE)
            {
                // Allow to remove by stack when aura is going to be applied on owner
                if (itr->first != GetOwner())
                {
                    // check if not stacking aura already on target
                    // this one prevents unwanted usefull buff loss because of stacking and prevents overriding auras periodicaly by 2 near area aura owners
                    for (Unit::AuraApplicationMap::iterator iter = itr->first->GetAppliedAuras().begin(); iter != itr->first->GetAppliedAuras().end(); ++iter)
                    {
                        Aura const *aura = iter->second->GetBase();
                        if (!CanStackWith(aura))
                        {
                            addUnit = false;
                            break;
                        }
                    }
                }
            }
        }
        if (!addUnit)
            targets.erase(itr++);
        else
        {
            // owner has to be in world, or effect has to be applied to self
            if (!GetOwner()->IsSelfOrInSameMap(itr->first))
            {
                //TODO: There is a crash caused by shadowfiend load addon
                TC_LOG_FATAL("spells", "Aura %u: Owner %s (map %u) is not in the same map as target %s (map %u).", GetSpellInfo()->Id,
                    GetOwner()->GetName().c_str(), GetOwner()->IsInWorld() ? GetOwner()->GetMap()->GetId() : uint32(-1),
                    itr->first->GetName().c_str(), itr->first->IsInWorld() ? itr->first->GetMap()->GetId() : uint32(-1));
                ASSERT(false);
            }
            itr->first->_CreateAuraApplication(this, itr->second);
            ++itr;
        }
    }

    // remove auras from units no longer needing them
    for (UnitList::iterator itr = targetsToRemove.begin(); itr != targetsToRemove.end();++itr)
        if (AuraApplication * aurApp = GetApplicationOfTarget((*itr)->GetGUID()))
            (*itr)->_UnapplyAura(aurApp, AURA_REMOVE_BY_DEFAULT);

    if (!apply)
        return;

    // apply aura effects for units
    for (std::map<Unit*, uint32>::iterator itr = targets.begin(); itr!= targets.end();++itr)
    {
        if (AuraApplication * aurApp = GetApplicationOfTarget(itr->first->GetGUID()))
        {
            // owner has to be in world, or effect has to be applied to self
            ASSERT((!GetOwner()->IsInWorld() && GetOwner() == itr->first) || GetOwner()->IsInMap(itr->first));
            itr->first->_ApplyAura(aurApp, itr->second);
        }
    }
}

// targets have to be registered and not have effect applied yet to use this function
void Aura::_ApplyEffectForTargets(uint8 effIndex)
{
    // prepare list of aura targets
    UnitList targetList;
    for (ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
    {
        if ((appIter->second->GetEffectsToApply() & (1<<effIndex)) && !appIter->second->HasEffect(effIndex))
            targetList.push_back(appIter->second->GetTarget());
    }

    // apply effect to targets
    for (UnitList::iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
    {
        if (GetApplicationOfTarget((*itr)->GetGUID()))
        {
            // owner has to be in world, or effect has to be applied to self
            ASSERT((!GetOwner()->IsInWorld() && GetOwner() == *itr) || GetOwner()->IsInMap(*itr));
            (*itr)->_ApplyAuraEffect(this, effIndex);
        }
    }
}
void Aura::UpdateOwner(uint32 diff, WorldObject* owner)
{
    ASSERT(owner == m_owner);

    Unit* caster = GetCaster();
    // Apply spellmods for channeled auras
    // used for example when triggered spell of spell:10 is modded
    Spell* modSpell = NULL;
    Player* modOwner = NULL;
    if (caster)
    {
        modOwner = caster->GetSpellModOwner();
        if (modOwner)
        {
            modSpell = modOwner->FindCurrentSpellBySpellId(GetId());
            if (modSpell)
                modOwner->SetSpellModTakingSpell(modSpell, true);
        }
    }

    Update(diff, caster);

    if (m_updateTargetMapInterval <= int32(diff))
        UpdateTargetMap(caster);
    else
        m_updateTargetMapInterval -= diff;

    // update aura effects
    for (auto &effectPtr : m_effects)
        if (effectPtr)
            effectPtr->Update(diff, caster);

    // remove spellmods after effects update
    if (modSpell)
        modOwner->SetSpellModTakingSpell(modSpell, false);

    _DeleteRemovedApplications();
}

void Aura::Update(uint32 diff, Unit* caster)
{
    if (m_duration > 0)
    {
        m_duration -= diff;
        if (m_duration < 0)
            m_duration = 0;

        CallScriptAuraUpdateHandlers(diff);

        // handle manaPerSecond/manaPerSecondPerLevel
        if (m_timeCla)
        {
            if (m_timeCla > int32(diff))
                m_timeCla -= diff;
            else if (caster)
            {
                if (!m_spellPowerData)
                    return;

                if (int32 manaPerSecond = m_spellPowerData->CostPerSecond + int32(m_spellPowerData->CostPerSecondPercentage * caster->GetCreatePowers(Powers(m_spellPowerData->PowerType)) / 100))
                {
                    m_timeCla += 1000 - diff;

                    Powers powertype = Powers(m_spellPowerData->PowerType);
                    if (powertype == POWER_HEALTH)
                    {
                        if (caster->CountPctFromMaxHealth(manaPerSecond) < caster->GetHealth())
                            caster->ModifyHealth(-1 * caster->CountPctFromMaxHealth(manaPerSecond));
                        else
                        {
                            Remove();
                            return;
                        }
                    }
                    else
                    {
                        if (int32(caster->GetPower(powertype)) >= manaPerSecond)
                            caster->ModifyPower(powertype, -manaPerSecond);
                        //if (int32(caster->CountPctFromMaxPower(manaPerSecond, powertype)) <= caster->GetPower(powertype))
                            //caster->ModifyPower(powertype, -1 * int32(caster->CountPctFromMaxPower(manaPerSecond, powertype)));
                        else
                        {
                            Remove();
                            return;
                        }
                    }
                }
            }
        }
    }
}

int32 Aura::CalcMaxDuration(Unit* caster) const
{
    Player* modOwner = NULL;
    int32 maxDuration;

    if (caster)
    {
        modOwner = caster->GetSpellModOwner();
        maxDuration = caster->CalcSpellDuration(m_spellInfo);
    }
    else
        maxDuration = m_spellInfo->GetDuration();

    if (IsPassive() && !m_spellInfo->DurationEntry)
        maxDuration = -1;

    // Hack fix Duration for Tigereye Brew
    if (m_spellInfo->Id == 116740)
        maxDuration = 15000;

    // IsPermanent() checks max duration (which we are supposed to calculate here)
    if (maxDuration != -1 && modOwner)
        modOwner->ApplySpellMod(GetId(), SPELLMOD_DURATION, maxDuration);
    return maxDuration;
}

void Aura::SetDuration(int32 duration, bool withMods)
{
    if (withMods)
    {
        if (Unit* caster = GetCaster())
            if (Player* modOwner = caster->GetSpellModOwner())
                modOwner->ApplySpellMod(GetId(), SPELLMOD_DURATION, duration);
    }
    m_duration = duration;
    SetNeedClientUpdateForTargets();
}

void Aura::RefreshDuration(bool recalculate)
{
    SetDuration(GetMaxDuration());

    if (m_spellPowerData->CostPerSecond || m_spellPowerData->CostPerSecondPercentage)
        m_timeCla = 1 * IN_MILLISECONDS;

    if (GetSpellInfo()->IsChanneled())
    {
        if (Unit * caster = GetCaster())
            if (Spell * channeled = caster->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                if (channeled->GetSpellInfo()->Id == GetSpellInfo()->Id)
                    channeled->SendChannelStart(GetMaxDuration());
    }

    if (recalculate)
        RecalculateAmountOfEffects();
}

void Aura::RefreshTimers(bool recalculate, int32 oldPeriodicAmount)
{
    m_maxDuration = CalcMaxDuration();
    // In mop periodics continue to tick even on re-apply, extend duration by duration left to next tick
    //if (m_spellInfo->AttributesEx8 & SPELL_ATTR8_DONT_RESET_PERIODIC_TIMER)
    {
        int32 minAmplitude = m_maxDuration;
        int32 tickTimer = minAmplitude;
        for (uint8 i = 0; i < GetSpellInfo()->Effects.size(); ++i)
            if (AuraEffect const *eff = GetEffect(i))
            {
                // Handle Pandemic
                if (GetCaster() && GetCaster()->HasAura(131973))
                    if (eff->GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
                        m_maxDuration = std::min(m_duration + m_maxDuration, int32(m_maxDuration * 1.5f));

                // Save periodic info for tick-rolling handling
                if (int32 ampl = eff->GetAmplitude())
                {
                    if (ampl <= minAmplitude)
                    {
                        minAmplitude = ampl;
                        tickTimer = eff->GetPeriodicTimer();
                    }
                }
            }

        // Expand duration for the amount left to next tick
        if (minAmplitude && minAmplitude < m_maxDuration)
            m_maxDuration += tickTimer;
    }

    RefreshDuration(recalculate);

    Unit* caster = GetCaster();
    for (uint8 i = 0; i < GetSpellInfo()->Effects.size(); ++i)
        if (HasEffect(i))
        {
            auto eff = GetEffect(i);
            eff->CalculatePeriodic(caster, false, false);
            // Tick rolled sets information that of last tick moved to new aura, it must store its damage
            if (oldPeriodicAmount && eff->GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
                eff->SetRolledTickAmount(oldPeriodicAmount);
        }
}

void Aura::SetCharges(uint8 charges)
{
    if (m_procCharges == charges)
        return;

    m_procCharges = charges;
    m_isUsingCharges = m_procCharges != 0;

    SetNeedClientUpdateForTargets();
}

uint8 Aura::CalcMaxCharges(Unit* caster) const
{
    uint32 maxProcCharges = m_spellInfo->ProcCharges;
    if (SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId()))
        maxProcCharges = procEntry->charges;

    if (caster)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_CHARGES, maxProcCharges);
    return uint8(maxProcCharges);
}

uint8 Aura::CalcMaxStacks(Unit* caster) const
{
    uint32 maxStacks = m_spellInfo->StackAmount;

    if (caster)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_STACKS, maxStacks);

    return uint8(maxStacks);
}

bool Aura::ModCharges(int32 num, AuraRemoveMode removeMode)
{
    if (IsUsingCharges())
    {
        int32 charges = m_procCharges + num;
        int32 maxCharges = CalcMaxCharges();

        if (num > 0 && charges > maxCharges)
        {
            // limit charges (only on charges increase, charges may be changed manually)
            charges = maxCharges;
        }
        else if (charges <= 0)
        {
            // we're out of charges, remove
            Remove(removeMode);
            return true;
        }

        SetCharges(charges);
        // Incite needs stack-amount sync
        if (GetId() == 122016)
            SetStackAmount(charges);
    }
    return false;
}

void Aura::SetStackAmount(uint8 stackAmount)
{
    m_stackAmount = stackAmount;
    // Will add this because i couldn't figure out a proper rule yet
    switch (m_spellInfo->Id)
    {
        case 88819:
            SetCharges(stackAmount);
            ResyncSpellmodCharges();
            break;
    }
    Unit* caster = GetCaster();

    auto const applications = GetApplicationList();

    for (auto const &app : applications)
        if (!app->GetRemoveMode())
            HandleAuraSpecificMods(app, caster, false, true);

    for (auto &effect : m_effects)
        if (effect)
            effect->ChangeAmount(effect->CalculateAmount(caster), false, true);

    for (auto const &app : applications)
        if (!app->GetRemoveMode())
            HandleAuraSpecificMods(app, caster, true, true);

    SetNeedClientUpdateForTargets();
}

bool Aura::ModStackAmount(int32 num, AuraRemoveMode removeMode)
{
    int32 stackAmount = m_stackAmount + num;

    // limit the stack amount (only on stack increase, stack amount may be changed manually)
    if ((num > 0) && (stackAmount > int32(m_spellInfo->StackAmount)))
    {
        // not stackable aura - set stack amount to 1
        if (!m_spellInfo->StackAmount)
            stackAmount = 1;
        else
            stackAmount = m_spellInfo->StackAmount;
    }
    // we're out of stacks, remove
    else if (stackAmount <= 0)
    {
        Remove(removeMode);
        return true;
    }

    bool refresh = stackAmount >= GetStackAmount();
    bool resetCharges = true;

    // Agony doesn't refresh itself every tick
    if (m_spellInfo->Id == 980 || m_spellInfo->Id == 138349)
        refresh = false;

    // Save old amount used for tick-rolling
    int32 oldPeriodicAmount = 0;
    for (auto &effectPtr : m_effects)
        if (effectPtr && effectPtr->GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
        {
            oldPeriodicAmount = effectPtr->GetAmount();
            break;
        }

    SetStackAmount(stackAmount);

    // Will add this because i couldn't figure out a proper rule yet
    switch (m_spellInfo->Id)
    {
        case 88819:
            resetCharges = false;
            break;
    }

    if (refresh)
    {
        RefreshSpellMods();
        RefreshTimers(false, oldPeriodicAmount);

        auto charges = CalcMaxCharges();
        CallScriptRefreshChargesHandlers(charges);
        if (resetCharges)
        {
            SetCharges(charges);
            ResyncSpellmodCharges();
        }
    }
    SetNeedClientUpdateForTargets();
    return false;
}

void Aura::ResyncSpellmodCharges()
{
    for (auto &eff : m_effects)
        if (eff && (eff->GetAuraType() == SPELL_AURA_ADD_FLAT_MODIFIER || eff->GetAuraType() == SPELL_AURA_ADD_PCT_MODIFIER))
            if (auto &mod = eff->GetSpellModifier())
                mod->charges = GetCharges();
}

void Aura::RefreshSpellMods()
{
    for (Aura::ApplicationMap::const_iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
        if (Player* player = appIter->second->GetTarget()->ToPlayer())
            player->RestoreAllSpellMods(0, this);
}

bool Aura::IsArea() const
{
    for (uint8 i = 0; i < GetSpellInfo()->Effects.size(); ++i)
    {
        if (HasEffect(i) && GetSpellInfo()->Effects[i].IsAreaAuraEffect())
            return true;
    }
    return false;
}

bool Aura::IsPassive() const
{
    return GetSpellInfo()->IsPassive();
}

bool Aura::IsDeathPersistent() const
{
    return GetSpellInfo()->IsDeathPersistent();
}

bool Aura::CanBeSaved() const
{
    if (GetSpellInfo()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_LOGOUT)
        return false;

    if (IsPassive())
        return false;

    if (GetCasterGUID() != GetOwner()->GetGUID())
        if (GetSpellInfo()->IsSingleTarget())
            return false;

    // Can't be saved - aura handler relies on calculated amount and changes it
    if (HasEffectType(SPELL_AURA_CONVERT_RUNE))
        return false;

    // No point in saving this, since the stable dialog can't be open on aura load anyway.
    if (HasEffectType(SPELL_AURA_OPEN_STABLE))
        return false;

    // Can't save vehicle auras, it requires both caster & target to be in world
    if (HasEffectType(SPELL_AURA_CONTROL_VEHICLE))
        return false;

    // Also for some reason other auras put as MultiSlot crash core on keeping them after restart,
    // so put here only these for which you are sure they get removed
    switch (GetId())
    {
        // Silithyst
        case 29519:
        // Incanter's Absorbtion - considering the minimal duration and problems with aura stacking
        // we skip saving this aura
        case 44413:
        case 40075: // Fel Flak Fire
        case 55849: // Power Spark
        case 61669: // Aspect of the Beast
        // When a druid logins, he doesnt have either eclipse power, nor the marker auras, nor the eclipse buffs. Dont save them.
        case 48517:
        case 48518:
        case 67483:
        case 67484:
        // Pyromaniac
        case 83582:
        // Nature's Bounty
        case 96206:
        // Dark Flames
        case 99158:
        // Don't save special liquid auras
        case 101619: // Magma, Fall of Azeroth
        case 97151: // Magma, Firelands
        case 81114: // Magma, Blackwing Descent
        case 57634: // Magma, CoA Black / Chamber
        case 42201: // Water, Hyjal Past
        case 37025: // Water, Coilfang Raid
        case 36444: // Water, Lake Wintergrasp
        case 28801: // Slime, Naxxramas
        case 125667: // Second Wind dummy
        case 111757: // Glyph of Levitate Speed Bonus
        case 114232: // Sanctified Wrath bonus
        case 124458: // Healing Spheres tracker
        case 106284: // Gathering Muddy Water
        case 113901: // Demonic Gateway stacking aura
        case 108446: // Soul link
        case 110310: // Dampening
            return false;
        default:
            break;
    }

    // don't save auras removed by proc system
    if (IsUsingCharges() && !GetCharges())
        return false;

    return true;
}

bool Aura::CanBeSentToClient() const
{
    if (GetId() == 115098)
        return false;

    if (!IsPassive())
        return true;

    if ((GetSpellInfo()->AttributesEx8 & SPELL_ATTR8_AURA_SEND_AMOUNT) != 0)
        return true;

    for (auto const &spellEffect : m_spellInfo->Effects)
    {
        if (spellEffect.IsAreaAuraEffect()
                || spellEffect.ApplyAuraName == SPELL_AURA_ABILITY_IGNORE_AURASTATE
                || spellEffect.ApplyAuraName == SPELL_AURA_CAST_WHILE_WALKING
                || spellEffect.ApplyAuraName == SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS
                || spellEffect.ApplyAuraName == SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS_2
                || spellEffect.ApplyAuraName == SPELL_AURA_MOD_IGNORE_SHAPESHIFT
                || spellEffect.ApplyAuraName == SPELL_AURA_MOD_CHARGES
                || spellEffect.ApplyAuraName == SPELL_AURA_ADD_FLAT_MODIFIER
                || spellEffect.ApplyAuraName == SPELL_AURA_ADD_PCT_MODIFIER
                || spellEffect.ApplyAuraName == SPELL_AURA_SANCTITY_OF_BATTLE_COOLDOWN
                || spellEffect.ApplyAuraName == SPELL_AURA_SANCTITY_OF_BATTLE_GCD)
        {
            return true;
        }
    }

    return false;
}

void Aura::UnregisterSingleTarget()
{
    ASSERT(m_isSingleTarget);
    Unit* caster = GetCaster();
    // TODO: find a better way to do this.
    if (!caster)
        caster = ObjectAccessor::GetObjectInOrOutOfWorld(GetCasterGUID(), (Unit*)NULL);
    ASSERT(caster);
    caster->GetSingleCastAuras().remove(this);
    SetIsSingleTarget(false);
}

int32 Aura::CalcDispelChance(Unit* auraTarget, bool offensive) const
{
    // we assume that aura dispel chance is 100% on start
    // need formula for level difference based chance
    int32 resistChance = 0;

    // Apply dispel mod from aura caster
    if (Unit* caster = GetCaster())
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_RESIST_DISPEL_CHANCE, resistChance);

    // Dispel resistance from target SPELL_AURA_MOD_DISPEL_RESIST
    // Only affects offensive dispels
    if (offensive && auraTarget)
        resistChance += auraTarget->GetTotalAuraModifier(SPELL_AURA_MOD_DISPEL_RESIST);

    resistChance = resistChance < 0 ? 0 : resistChance;
    resistChance = resistChance > 100 ? 100 : resistChance;
    return 100 - resistChance;
}

void Aura::SetLoadedState(int32 maxduration, int32 duration, int32 charges, uint8 stackamount, uint32 recalculateMask, int32 *amount)
{
    m_maxDuration = maxduration;
    m_duration = duration;
    m_procCharges = charges;
    m_isUsingCharges = m_procCharges != 0;
    m_stackAmount = stackamount;
    Unit* caster = GetCaster();

    for (size_t i = 0; i < m_effects.size(); ++i)
    {
        if (m_effects[i])
        {
            m_effects[i]->SetAmount(amount[i]);
            m_effects[i]->SetCanBeRecalculated(recalculateMask & (1 << i));
            m_effects[i]->CalculatePeriodic(caster, false, true);
            m_effects[i]->CalculateSpellMod();
            m_effects[i]->RecalculateAmount(caster);
        }
    }
}

bool Aura::HasEffectType(AuraType type) const
{
    for (auto &effectPtr : m_effects)
        if (effectPtr && effectPtr->GetAuraType() == type)
            return true;
    return false;
}

void Aura::RecalculateAmountOfEffects()
{
    ASSERT (!IsRemoved());
    Unit* caster = GetCaster();
    for (auto &effectPtr : m_effects)
        if (effectPtr)
            effectPtr->RecalculateAmount(caster);
}

void Aura::HandleAllEffects(AuraApplication * aurApp, uint8 mode, bool apply)
{
    ASSERT (!IsRemoved());
    for (auto &effectPtr : m_effects)
    {
        if (effectPtr)
        {
            effectPtr->HandleEffect(aurApp, mode, apply);
            if (IsRemoved())
                break;
        }
    }
}

Unit::AuraApplicationList Aura::GetApplicationList() const
{
    Unit::AuraApplicationList applicationList;

    for (auto &kvPair : m_applications)
        if (kvPair.second->GetEffectMask())
            applicationList.push_back(kvPair.second);

    return applicationList;
}

void Aura::SetNeedClientUpdateForTargets() const
{
    for (ApplicationMap::const_iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
        appIter->second->SetNeedClientUpdate();
}

// trigger effects on real aura apply/remove
void Aura::HandleAuraSpecificMods(AuraApplication const* aurApp, Unit* caster, bool apply, bool onReapply)
{
    Unit* target = aurApp->GetTarget();
    AuraRemoveMode removeMode = aurApp->GetRemoveMode();
    // handle spell_area table
    SpellAreaForAreaMapBounds saBounds = sSpellMgr->GetSpellAreaForAuraMapBounds(GetId());
    if (saBounds.first != saBounds.second)
    {
        uint32 zone, area;
        target->GetZoneAndAreaId(zone, area);

        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            // some auras remove at aura remove
            if (!itr->second->IsFitToRequirements((Player*)target, zone, area))
                target->RemoveAurasDueToSpell(itr->second->spellId);
            // some auras applied at aura apply
            else if (itr->second->autocast)
            {
                if (!target->HasAura(itr->second->spellId))
                    target->CastSpell(target, itr->second->spellId, true);
            }
        }
    }

    // handle spell_linked_spell table
    if (!onReapply)
    {
        // apply linked auras
        if (apply)
        {
            if (std::vector<int32> const* spellTriggered = sSpellMgr->GetSpellLinked(GetId() + SPELL_LINK_AURA))
            {
                for (std::vector<int32>::const_iterator itr = spellTriggered->begin(); itr != spellTriggered->end(); ++itr)
                {
                    if (*itr < 0)
                        target->ApplySpellImmune(GetId(), IMMUNITY_ID, -(*itr), true);
                    else if (caster)
                        caster->AddAura(*itr, target);
                }
            }
        }
        else
        {
            // remove linked auras
            if (std::vector<int32> const* spellTriggered = sSpellMgr->GetSpellLinked(-(int32)GetId()))
            {
                for (std::vector<int32>::const_iterator itr = spellTriggered->begin(); itr != spellTriggered->end(); ++itr)
                {
                    if (*itr < 0)
                        target->RemoveAurasDueToSpell(-(*itr));
                    else if (removeMode != AURA_REMOVE_BY_DEATH)
                        target->CastSpell(target, *itr, true, NULL, NULL, GetCasterGUID());
                }
            }
            if (std::vector<int32> const* spellTriggered = sSpellMgr->GetSpellLinked(GetId() + SPELL_LINK_AURA))
            {
                for (std::vector<int32>::const_iterator itr = spellTriggered->begin(); itr != spellTriggered->end(); ++itr)
                {
                    if (*itr < 0)
                        target->ApplySpellImmune(GetId(), IMMUNITY_ID, -(*itr), false);
                    else
                        target->RemoveAura(*itr, GetCasterGUID(), 0, removeMode);
                }
            }
        }
    }
    else if (apply)
    {
        // modify stack amount of linked auras
        if (std::vector<int32> const* spellTriggered = sSpellMgr->GetSpellLinked(GetId() + SPELL_LINK_AURA))
        {
            for (std::vector<int32>::const_iterator itr = spellTriggered->begin(); itr != spellTriggered->end(); ++itr)
                if (*itr > 0)
                {
                    Aura *triggeredAura = target->GetAura(*itr, GetCasterGUID());
                    if (triggeredAura != NULL)
                        triggeredAura->ModStackAmount(GetStackAmount() - triggeredAura->GetStackAmount());
                }
        }
    }

    // mods at aura apply
    if (apply)
    {
        switch (GetSpellInfo()->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
                switch (GetId())
                {
                    case 145955://力量输出
                        if (Player* player = target->ToPlayer())
                        {
                            uint32 spec = player->GetSpecializationId(player->GetActiveSpec());
                            int32 amount = -GetEffect(0)->GetAmount();
                            switch (spec)
                            {
                            case SPEC_PALADIN_RETRIBUTION:
                                player->CastCustomSpell(145978, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_WARRIOR_ARMS:
                            case SPEC_WARRIOR_FURY:
                                player->CastCustomSpell(145991, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_DK_FROST:
                                player->CastCustomSpell(145959, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_DK_UNHOLY:
                                player->CastCustomSpell(145960, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            }
                        }
                        break;
                    case 146019://敏捷输出型
                        if (Player* player = target->ToPlayer())
                        {
                            uint32 spec = player->GetSpecializationId(player->GetActiveSpec());
                            int32 amount = -GetEffect(0)->GetAmount();
                            switch (spec)
                            {
                            case SPEC_HUNTER_BEASTMASTER:
                                player->CastCustomSpell(145964, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_HUNTER_MARKSMAN:
                                player->CastCustomSpell(145965, SPELLVALUE_BASE_POINT0, amount, player, true);
                            case SPEC_HUNTER_SURVIVAL:
                                player->CastCustomSpell(145966, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_SHAMAN_ENHANCEMENT:
                                player->CastCustomSpell(197009, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_ROGUE_ASSASSINATION:
                                player->CastCustomSpell(145983, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_ROGUE_COMBAT:
                                player->CastCustomSpell(145984, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_ROGUE_SUBTLETY:
                                player->CastCustomSpell(145985, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_MONK_WINDWALKER:
                                player->CastCustomSpell(145964, SPELLVALUE_BASE_POINT0, amount, player, true);
                                player->CastSpell(player, 145969, true);
                                break;
                            case SPEC_DRUID_FERAL:
                                player->CastCustomSpell(145961, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            }
                        }
                        break;
                    case 146025://坦克
                        if (Player* player = target->ToPlayer())
                        {
                            uint32 spec = player->GetSpecializationId(player->GetActiveSpec());
                            int32 amount = -GetEffect(0)->GetAmount();
                            switch (spec)
                            {
                            case SPEC_PALADIN_PROTECTION:
                                player->CastCustomSpell(145976, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_WARRIOR_PROTECTION:
                                player->CastCustomSpell(145992, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_DK_BLOOD:
                                player->CastCustomSpell(145958, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_DRUID_GUARDIAN:
                                player->CastCustomSpell(145962, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            case SPEC_MONK_BREWMASTER:
                                player->CastCustomSpell(145967, SPELLVALUE_BASE_POINT0, amount, player, true);
                                break;
                            }
                        }
                        break;
                    // Spark of Zandalar
                    case 138958:
                        if (target && target->GetTypeId() == TYPEID_PLAYER && target->HasAura(138958))
                        {
                            if (target->GetAura(138958)->GetStackAmount() == 10)
                            {
                                target->RemoveAurasDueToSpell(138958);
                                target->CastSpell(target, 138960, true, target->ToPlayer()->GetItemByGuid(GetCastItemGUID()));
                            }
                        }
                        break;
                    case 42292: // PvP Trinket
                    case 59752: // Every Man for Himself
                        if (target && target->GetTypeId() == TYPEID_PLAYER)
                        {
                            target->CastSpell(target, (target->ToPlayer()->GetTeam() == ALLIANCE ? 97403 : 97404), true);
                            if (Pet* pet = target->ToPlayer()->GetPet())
                                pet->RemoveAurasWithMechanic(IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK, AURA_REMOVE_BY_DEFAULT, GetId());
                        }
                        break;
                    // Magma, Echo of Baine
                    case 101619:
                        if (target && target->GetTypeId() == TYPEID_PLAYER && !target->HasAura(101866))
                            target->CastSpell(target, 101866, true);
                        break;
                    // Burning Rage, Item - Warrior T12 DPS 2P Bonus
                    case 99233:
                    {
                        if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                            break;

                        uint32 max_duration = 12000;

                        if (auto const aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARRIOR, 47, EFFECT_1))
                            max_duration -= 6000 * (0.01f * aurEff->GetAmount());

                        SetDuration(max_duration);
                        break;
                    }
                    // Blazing Power, Alysrazor
                    case 99461:
                        if (auto const aur = target->GetAura(98619))
                            aur->RefreshDuration();

                        if (!target->HasAura(100029) && aurApp->GetBase()->GetStackAmount() >= 25)
                            target->CastSpell(target, 100029, true);

                        break;
                    // Arion - Swirling Winds
                    case 83500:
                        target->RemoveAurasDueToSpell(83581);
                        break;
                    // Terrastra - Grounded
                    case 83581:
                        target->RemoveAurasDueToSpell(83500);
                        break;
                    case 32474: // Buffeting Winds of Susurrus
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            target->ToPlayer()->ActivateTaxiPathTo(506, GetId());
                        break;
                    case 33572: // Gronn Lord's Grasp, becomes stoned
                        if (GetStackAmount() >= 5 && !target->HasAura(33652))
                            target->CastSpell(target, 33652, true);
                        break;
                    case 50836: //Petrifying Grip, becomes stoned
                        if (GetStackAmount() >= 5 && !target->HasAura(50812))
                            target->CastSpell(target, 50812, true);
                        break;
                    case 60970: // Heroic Fury (remove Intercept cooldown)
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            target->ToPlayer()->RemoveSpellCooldown(20252, true);
                        break;
                    case 105785:    // Stolen Time mage T13 set bonus
                    {
                        if (target->HasAura(105790))
                            target->CastSpell(target, 105791, true); // cooldown bonus
                        break;
                    }
                    // Smoke Bomb, Rogue
                    case 88611:
                        if (caster)
                            if (!target->IsFriendlyTo(caster))
                                target->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                        break;
                }
                break;
            case SPELLFAMILY_DRUID:
                if (!caster)
                    break;
                // Rejuvenation
                if (GetSpellInfo()->SpellFamilyFlags[0] & 0x10 && GetEffect(EFFECT_0))
                {
                    // Druid T8 Restoration 4P Bonus
                    if (caster->HasAura(64760))
                    {
                        int32 heal = GetEffect(EFFECT_0)->GetAmount();
                        caster->CastCustomSpell(target, 64801, &heal, NULL, NULL, true, NULL, GetEffect(EFFECT_0));
                    }
                }
                break;
            case SPELLFAMILY_MAGE:
            {
                if (!caster)
                    break;

                switch (GetId())
                {
                    case 118:   // Polymorph
                    case 61305: // Polymorph (Black Cat)
                    {
                        // Polymorph Sound - Sheep && Penguin
                        // Glyph of the Penguin
                        if (caster->HasAura(52648))
                            caster->CastSpell(target, 61635, true);
                        // Glyph of the Monkey
                        else if (caster->HasAura(57927))
                            caster->CastSpell(target, 89729, true);
                        // Glyph of the Porcupine
                        else if (caster->HasAura(57924))
                            caster->CastSpell(target, 126834, true);
                        else
                            caster->CastSpell(target, 61634, true);

                        // Glyph of Polymorph
                        if (caster && caster->HasAura(56375))
                        {
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                        }

                        break;
                    }
                    case 44457: // Living Bomb
                    {
                        UnitList targets;
                        Trinity::AnyUnitHavingBuffInObjectRangeCheck u_check(caster, caster, 300.0f, 44457, false);
                        Trinity::UnitListSearcher<Trinity::AnyUnitHavingBuffInObjectRangeCheck> searcher(caster, targets, u_check);
                        Trinity::VisitNearbyObject(caster, 300.0f, searcher);
                        if (targets.size() >= 4)
                        {
                            std::list<Aura *> auras;
                            for (UnitList::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                                if (Aura *aur = (*itr)->GetAura(44457, caster->GetGUID()))
                                    auras.push_back(aur);

                            if (auras.size() >= 4)
                            {
                                auras.sort(Trinity::DurationOrderPred(false));
                                auras.pop_front();
                                auras.pop_front();
                                auras.pop_front();
                                for (auto itr = auras.begin(); itr != auras.end();)
                                {
                                    (*itr)->Remove();
                                    itr = auras.erase(itr);
                                }
                            }
                        }

                        break;
                    }
                    // Ring of Frost - 2.5 sec immune
                    case 82691:
                        target->AddAura(91264, target);
                        break;
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_PRIEST:
                if (!caster)
                    break;
                // Devouring Plague
                if (GetSpellInfo()->SpellFamilyFlags[0] & 0x02000000 && GetEffect(0))
                {
                    // Improved Devouring Plague
                    if (AuraEffect const *aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 3790, 0))
                    {
                        uint32 damage = caster->SpellDamageBonusDone(target, GetSpellInfo(), EFFECT_0, GetEffect(0)->GetAmount(), DOT);
                        damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), EFFECT_0, damage, DOT);
                        int32 basepoints0 = aurEff->GetAmount() * GetEffect(0)->GetTotalTicks() * int32(damage) / 100;
                        int32 heal = int32(CalculatePct(basepoints0, 15));

                        caster->CastCustomSpell(target, 63675, &basepoints0, NULL, NULL, true, NULL, GetEffect(0));
                        caster->CastCustomSpell(caster, 75999, &heal, NULL, NULL, true, NULL, GetEffect(0));
                    }
                }
                // Power Word: Shield
                else if ((m_spellInfo->Id == 17 || m_spellInfo->Id == 123258) && GetEffect(0))
                {
                    // Glyph of Power Word: Shield
                    if (AuraEffect *glyph = caster->GetAuraEffect(55672, 0))
                    {
                        int32 amount = GetEffect(EFFECT_0)->GetAmount();
                        // instantly heal m_amount% of the absorb-value
                        int32 heal = CalculatePct(amount, glyph->GetAmount());
                        caster->CastCustomSpell(GetUnitOwner(), 56160, &heal, NULL, NULL, true, 0, GetEffect(0));
                        // and reduce absorb amount
                        amount = AddPct(amount, -glyph->GetAmount());
                        GetEffect(EFFECT_0)->SetAmount(amount);
                    }
                }
                break;
            case SPELLFAMILY_PALADIN:
            {
                if (!caster)
                    break;

                switch (m_spellInfo->Id)
                {
                    // Grand Crusader
                    case 85416:
                        caster->ToPlayer()->RemoveSpellCooldown(31935, true);
                        break;
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                switch (GetId())
                {
                    // Blind
                    case 2094:
                    {
                        // Glyph of Blind
                        if (caster && caster->HasAura(91299))
                        {
                            // Manual iteration to prevent removing poisons with Dirty Tricks talent
                            auto periodicAuras = target->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                            for (auto iter = periodicAuras.begin(); iter != periodicAuras.end();)
                            {
                                Aura *aura = (*iter)->GetBase();
                                ++iter;
                                if (aura->GetSpellInfo()->IsPoisonOrBleedSpell() && caster->HasAura(108216) && aura->GetCasterGUID() == caster->GetGUID() || aura->GetId() == 32409)
                                    continue;
                                else
                                    target->RemoveAura(aura);
                            }
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                        }
                        break;
                    }
                    // Sap
                    case 6770:
                    {
                        // Remove stealth from target
                        target->RemoveAurasByType(SPELL_AURA_MOD_STEALTH, 0, 0, 11327);
                        break;
                    }
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
                // Maelstorm Weapon
                if (GetId() == 53817)
                {
                    // Item - Shaman T13 Enhancement 2P Bonus
                    if (target->HasAura(105866))
                        target->CastSpell(target, 105869, true);
                }
                // Spiritwalker's Grace
                else if (GetId() == 79206)
                {
                    // Item - Shaman T13 Restoration 4P Bonus (Spiritwalker's Grace)
                    if (target->HasAura(131557))
                        target->CastSpell(target, 131558, true);
                }

                break;
            case SPELLFAMILY_WARRIOR:
                // Heroic Fury
                if (m_spellInfo->Id == 60970)
                {
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        target->ToPlayer()->RemoveSpellCooldown(20252, true);
                }
                // Battle Shout && Commander Shout
                else if (m_spellInfo->Id == 469 || m_spellInfo->Id == 6673)
                {
                    if (caster && caster == target)
                    {
                        // Item - Warrior T12 DPS 2P Bonus
                        if (caster->HasAura(99234))
                            caster->CastSpell(caster, 99233, true);

                        // Glyph of Mystic Shout
                        if (caster->HasAura(58095))
                            caster->CastSpell(caster, 121186, true);
                    }
                }
                // Demoralizing Shout
                else if (m_spellInfo->Id == 125565)
                {
                    // Glyph of Incite
                    if (target->HasAura(122013))
                    {
                        target->CastSpell(target, 122016, true);
                        if (auto incite = target->GetAura(122016))
                            incite->ModStackAmount(3);
                    }
                }
                break;
            case SPELLFAMILY_HUNTER:
                if (!caster)
                    break;

                switch(GetId())
                {
                    case 3355: // Freezing Trap Effect
                    {
                        target->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                        // Glyph of Solace
                        if (caster->HasAura(119407))
                        {
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, 0, target->GetAura(32409)); // SW:D shall not be removed.
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                        }
                        break;
                    }
                    case 53257: // Cobra Strikes
                    {
                        if (!onReapply)
                            ModStackAmount(1);
                        break;
                    }
                    // Load and Load proc removes current Explosive Shot cooldown
                    case 56453:
                    {
                        if(target->GetTypeId() == TYPEID_PLAYER)
                            target->ToPlayer()->RemoveSpellCooldown(53301, true);
                         break;
                    }
                    default:
                        break;
                }
                break;
            case SPELLFAMILY_DEATHKNIGHT:
            {
                switch (GetId())
                {
                    // Vampiric Blood
                    case 55233:
                        // Item - Death Knight T13 Blood 4P Bonus
                        if (caster->HasAura(105587))
                            caster->CastSpell((Unit*)NULL, 105588, true);
                        break;
                    case 81162: // Will of the Necropolis
                    {
                        if (caster)
                            caster->CastSpell(caster, 96171, true);

                        break;
                    }
                    default:
                        break;
                }

                // Switching presence resets power to 0 or 70% with Glyph of Shifting Presences
                if (GetSpellInfo()->GetSpellSpecific() == SPELL_SPECIFIC_PRESENCE)
                {
                    if (AuraEffect const * const glyph = target->GetAuraEffect(58647, EFFECT_0))
                        target->SetPower(POWER_RUNIC_POWER, CalculatePct(target->GetPower(POWER_RUNIC_POWER), glyph->GetAmount()));
                    else
                        target->SetPower(POWER_RUNIC_POWER, 0);
                }

                break;
            }
            case SPELLFAMILY_MONK:
            {
                if (!caster)
                    break;

                switch (GetId())
                {
                    // Paralysis
                    case 115078:
                        // Glyph of Paralysis
                        if (caster->HasAura(125755))
                        {
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, 0, target->GetAura(32409)); // SW:D shall not be removed.
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                        }
                        break;
                    default:
                        break;
                }
            }
            default:
                break;
        }
    }
    // mods at aura remove
    else
    {
        switch (GetSpellInfo()->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
                switch (GetId())
                {
                    case 69674: // Mutated Infection (Rotface)
                    case 71224: // Mutated Infection (Rotface)
                    case 73022: // Mutated Infection (Rotface)
                    case 73023: // Mutated Infection (Rotface)
                    {
                        AuraRemoveMode removeMode = aurApp->GetRemoveMode();
                        if (removeMode == AURA_REMOVE_BY_EXPIRE || removeMode == AURA_REMOVE_BY_ENEMY_SPELL)
                            target->CastSpell(target, 69706, true);
                        break;
                    }
                    case 69483: // Dark Reckoning (ICC)
                        if (caster)
                            caster->CastSpell(target, 69482, false);
                        break;
                    case 91296: // Egg Shell, Corrupted Egg Shell
                        if (caster)
                            caster->CastSpell(caster, 91305, true);
                        break;
                    case 91308: // Egg Shell, Corrupted Egg Shell (H)
                        if (caster)
                            caster->CastSpell(caster, 91310, true);
                        break;
                    case 49440: // Racer Slam, Slamming
                        if (Creature* racerBunny = target->FindNearestCreature(27674, 25.0f))
                            target->CastSpell(racerBunny, 49302, false);
                        break;
                    case 52418: // Carrying Seaforium
                        if (removeMode == AURA_REMOVE_BY_CANCEL)
                            target->CastSpell(target, 52417, true);
                        break;
                    case 72368: // Shared Suffering
                    case 72369:
                        if (caster)
                        {
                            if (AuraEffect *aurEff = GetEffect(0))
                            {
                                int32 remainingDamage = aurEff->GetAmount() * (aurEff->GetTotalTicks() - aurEff->GetTickNumber());
                                if (remainingDamage > 0)
                                    caster->CastCustomSpell(caster, 72373, NULL, &remainingDamage, NULL, true);
                            }
                        }
                        break;
                    case 115192: // Subterfuge - Remove Stealth aura
                        target->RemoveAurasDueToSpell(115191);
                        break;
                    case 43681: // Inactive
                    {
                        if (target->GetTypeId() != TYPEID_PLAYER || aurApp->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                            return;

                        if (target->GetMap()->IsBattleground())
                            target->ToPlayer()->LeaveBattleground();
                        break;
                    }
                }
                break;
            case SPELLFAMILY_MAGE:
            {
                switch (GetId())
                {
                    case 66: // Invisibility
                    {
                        // Glyph of Invisibility
                        if (caster)
                            caster->RemoveAurasDueToSpell(87833);

                        if (removeMode != AURA_REMOVE_BY_EXPIRE)
                            break;

                        // Mage water elemental gains invisibility as mage
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            if (Pet* pet = target->ToPlayer()->GetPet())
                                pet->CastSpell(pet, 32612, true, NULL, GetEffect(1));

                        target->CastSpell(target, 32612, true, NULL, GetEffect(1));
                        target->CombatStop();

                        // Glyph of Invisibility
                        if (target->HasAura(56366))
                            target->CastSpell(target, 87833, true);

                        break;
                    }
                    case 32612: // Invisibility (triggered)
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            if (Pet* pet = target->ToPlayer()->GetPet())
                                if (pet->HasAura(32612))
                                    pet->RemoveAurasDueToSpell(32612);

                        break;
                    }
                    case 79683: // Arcane Missiles aura
                    {
                        // Arcane Missiles double-aura state
                        if (caster && !onReapply)
                            caster->RemoveAurasDueToSpell(79808);
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_WARLOCK:
                if (!caster)
                    break;
                // Improved Fear
                if (GetSpellInfo()->SpellFamilyFlags[1] & 0x00000400)
                {
                    if (AuraEffect *aurEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 98, 0))
                    {
                        uint32 spellId = 0;
                        switch (aurEff->GetId())
                        {
                            case 53759: spellId = 60947; break;
                            case 53754: spellId = 60946; break;
                            default:
                                TC_LOG_ERROR("spells", "Aura::HandleAuraSpecificMods: Unknown rank of Improved Fear (%d) found", aurEff->GetId());
                        }
                        if (spellId)
                            caster->CastSpell(target, spellId, true);
                    }
                }
                break;
            case SPELLFAMILY_PRIEST:
                if (!caster)
                    break;
                break;
            case SPELLFAMILY_ROGUE:
                switch (GetId())
                {
                    case 11327: // Cast Stealth on vanish removal
                        if (removeMode == AURA_REMOVE_BY_EXPIRE)
                        {
                            target->CastSpell(target, 131369, true);
                            target->CastSpell(target, 1784, true);
                        }
                        break;
                    // Nerve Strike - Apply debuff on Kidney Shot and Cheap Shot
                    case 408:
                    case 1833:
                        if (caster && caster->HasAura(108210))
                            caster->CastSpell(target, 112947, true);
                        break;
                }
                break;
            case SPELLFAMILY_PALADIN:
                if (!caster)
                    break;

                // Avenging Wrath
                if (m_spellInfo->Id == 31884)
                    target->RemoveAura(57318);
                // Communion
                else if (m_spellInfo->SpellFamilyFlags[2] & 0x20 && target == caster)
                    caster->RemoveAurasDueToSpell(63531);
                // Divine Protection
                else if (m_spellInfo->Id == 498)
                    // Item - Paladin T12 Protection 4P Bonus
                    if (caster->HasAura(99091) && (removeMode == AURA_REMOVE_BY_EXPIRE))
                        caster->CastSpell(caster, 99090, true);
                break;
            case SPELLFAMILY_DEATHKNIGHT:
            {
                switch (GetSpellInfo()->Id)
                {
                    case 56835: // Reaping
                    {
                        if (!GetEffect(0) || GetEffect(0)->GetAuraType() != SPELL_AURA_PERIODIC_DUMMY)
                            break;
                        if (target->GetTypeId() != TYPEID_PLAYER)
                            break;
                        if (target->ToPlayer()->getClass() != CLASS_DEATH_KNIGHT)
                            break;

                        // aura removed - remove death runes
                        target->ToPlayer()->RemoveRunesBySpell(GetId());
                        break;
                    }
                    case 81256: // Dancing Rune Weapon
                    {
                        // Item - Death Knight T12 Tank 4P Bonus
                        if (target->HasAura(98966) && (removeMode == AURA_REMOVE_BY_EXPIRE))
                            target->CastSpell(target, 101162, true); // +15% parry
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_HUNTER:
                // Glyph of Freezing Trap
                if (GetSpellInfo()->SpellFamilyFlags[0] & 0x00000008)
                    if (caster && caster->HasAura(56845))
                        target->CastSpell(target, 61394, true);
                break;
            case SPELLFAMILY_SHAMAN:
            {
                switch (GetSpellInfo()->Id)
                {
                    case 53817: // Maelstrom Weapon
                    {
                        // Maelstrom Weapon visual buff
                        if (caster && !onReapply)
                            caster->RemoveAurasDueToSpell(60349);

                        break;
                    }
                    // Grownding Totem effect
                    case 89523:
                    case 8178:
                        if (caster != target && removeMode != AURA_REMOVE_NONE)
                            caster->setDeathState(JUST_DIED);
                        break;
                    default:
                        break;
                }
                break;
            }
            case SPELLFAMILY_DRUID:
            {
                switch (GetSpellInfo()->Id)
                {
                    case 22812: // Barkskin
                    {
                        // Item - Druid T12 Feral 4P Bonus
                        if (caster && caster->HasAura(99009) && (removeMode == AURA_REMOVE_BY_EXPIRE))
                            caster->CastSpell(caster, 99011, true);

                        break;
                    }
                    case 81022: // Stampede and Incarnation (Feral)
                    {
                        if (caster)
                        {
                            if (Aura* aura = caster->GetAura(102543))
                            {
                                int32 dur = aura->GetDuration();
                                aura->TryRefreshStackOrCreate(aura->GetSpellInfo(), MAX_EFFECT_MASK, caster, caster, 0);
                                aura->SetDuration(dur);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_WARRIOR:
                // Shield Block
                if (GetId() == 2565)
                    // Item - Item - Warrior T12 Protection 4P Bonus
                    if (caster && caster->HasAura(99242) && (removeMode == AURA_REMOVE_BY_EXPIRE))
                        caster->CastSpell(caster, 99243, true);
                break;
            default:
                break;
        }
    }

    // mods at aura apply or remove
    switch (GetSpellInfo()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch (GetId())
            {
                case 50720: // Vigilance
                    if (apply)
                        target->CastSpell(caster, 59665, true, 0, NULL, caster->GetGUID());
                    else
                        target->SetReducedThreatPercent(0, 0);
                    break;
                case 71289: // Mind Control (Lady Deathwisper)
                    target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X, 100.0f, apply);
                    break;
            }
            break;
        case SPELLFAMILY_DRUID:
        {
            switch (GetSpellInfo()->Id)
            {
                // Tree of Life
                case 33891:
                {
                    if (apply)
                    {
                        caster->CastSpell(caster, 5420, true);
                        caster->CastSpell(caster, 81097, true);
                        caster->CastSpell(caster, 81098, true);
                        if (!caster->HasAura(117679))
                            caster->CastSpell(caster, 117679, true);
                    }
                    else
                    {
                        caster->RemoveAurasDueToSpell(5420);
                        caster->RemoveAurasDueToSpell(81097);
                        caster->RemoveAurasDueToSpell(81098);
                    }
                    break;
                }
                case 117679:
                    if (caster && !apply)
                        caster->RemoveAurasDueToSpell(33891);
                    break;
                case 5229:  // Enrage
                    if (target->HasAura(70726)) // Item - Druid T10 Feral 4P Bonus
                        if (apply)
                            target->CastSpell(target, 70725, true);
                    break;
                case 22812: // Glyph of Barkskin
                {
                    if (apply)
                    {
                        if (caster->HasAura(63057))
                            caster->AddAura(63058, caster);
                    }
                    else
                        caster->RemoveAura(63058);
                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELLFAMILY_ROGUE:
            // Stealth / Vanish
            if (GetSpellInfo()->SpellFamilyFlags[0] & 0x00400800)
            {
                // Master of subtlety
                if (auto const aurEff = target->GetAuraEffectOfRankedSpell(31223, 0))
                {
                    if (apply)
                    {
                        int32 basepoints0 = aurEff->GetAmount();
                        target->CastCustomSpell(target, 31665, &basepoints0, NULL, NULL , true);
                    }
                    else if (!target->GetAuraEffect(SPELL_AURA_MOD_SHAPESHIFT, SPELLFAMILY_ROGUE, Trinity::Flag128(0x400800)))
                    {
                        if (auto const aur = target->GetAura(31665))
                        {
                            aur->SetDuration(6 * IN_MILLISECONDS);
                            aur->SetMaxDuration(6 * IN_MILLISECONDS);
                        }
                    }
                }
                // Overkill
                if (target->HasAura(58426))
                {
                    if (apply)
                    {
                        target->CastSpell(target,58427,true);
                    }
                    else if (!target->GetAuraEffect(SPELL_AURA_MOD_SHAPESHIFT, SPELLFAMILY_ROGUE, Trinity::Flag128(0x400800)))
                    {
                        if (auto const aur = target->GetAura(58427))
                        {
                            aur->SetDuration(20 * IN_MILLISECONDS);
                            aur->SetMaxDuration(20 * IN_MILLISECONDS);
                        }
                    }
                }
                break;
            }
            // Sprint
            else if (GetSpellInfo()->Id == 2983)
            {
                // in official maybe there is only one icon?
                if (target->HasAura(58039)) // Glyph of Blurred Speed
                {
                    if (apply)
                        target->CastSpell(target, 61922, true); // Sprint (waterwalk)
                    else
                        target->RemoveAurasDueToSpell(61922);
                }
                break;
            }
            break;
        case SPELLFAMILY_HUNTER:
            switch (GetId())
            {
                case 19574: // Bestial Wrath
                    // The Beast Within cast on owner if talent present
                    if (Unit* owner = target->GetOwner())
                    {
                        // Search talent
                        if (owner->HasAura(34692))
                        {
                            if (apply)
                            {
                                owner->CastSpell(owner, 34471, true, 0, GetEffect(0));
                                // Apply immunity, it was a bug on MoP but never fixed by Blizzard until WoD...
                                owner->CastSpell(owner, 70029, true);
                                target->CastSpell(target, 70029, true);
                            }
                            else
                                owner->RemoveAurasDueToSpell(34471);
                        }
                    }
                    break;
            }
            break;
        case SPELLFAMILY_PALADIN:
        {
            switch (GetId())
            {
                case 31821:
                    // Aura Mastery Triggered Spell Handler
                    // If apply Concentration Aura -> trigger -> apply Aura Mastery Immunity
                    // If remove Concentration Aura -> trigger -> remove Aura Mastery Immunity
                    // If remove Aura Mastery -> trigger -> remove Aura Mastery Immunity
                    // Do effects only on aura owner
                    if (GetCasterGUID() != target->GetGUID())
                        break;

                    if (apply)
                    {
                        if ((GetSpellInfo()->Id == 31821 && target->HasAura(19746, GetCasterGUID())) || (GetSpellInfo()->Id == 19746 && target->HasAura(31821)))
                            target->CastSpell(target, 64364, true);
                    }
                    else
                    {
                        target->RemoveAurasDueToSpell(64364, GetCasterGUID());
                        target->RemoveOwnedAura(64364, GetCasterGUID());
                    }
                    break;
                case 31842: // Divine Favor
                    // Item - Paladin T10 Holy 2P Bonus
                    if (target->HasAura(70755))
                    {
                        if (apply)
                            target->CastSpell(target, 71166, true);
                        else
                            target->RemoveAurasDueToSpell(71166);
                    }
                    break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            switch (GetId())
            {
                case 66:    // Invisibility
                    if (apply && caster && caster->HasAura(56366))
                        caster->CastSpell(caster, 87833, true);
                    break;
                case 32612: // Invisibility (triggered)
                    if (!apply && caster)
                        caster->RemoveAurasDueToSpell(87833);
                    break;
                case 44544: // Fingers of Frost - Apply second-stack visual
                {
                    if (apply)
                    {
                        if (GetStackAmount() > 1 && !target->HasAura(126084))
                            target->CastSpell(target, 126084, true);
                    }
                    else
                        target->RemoveAurasDueToSpell(126084);
                    break;
                }
                case 44457: // Living Bomb
                case 114923: // Nether Tempest
                case 112948: // Frost Bomb
                {
                    // Brain Freeze requies most recently applied aura to proc
                    if (!caster || !caster->ToPlayer())
                        break;

                    if (apply)
                        caster->ToPlayer()->auraAppliedOnTarget(GetSpellInfo()->Id, target->GetGUID());
                    else
                        caster->ToPlayer()->auraRemovedFromTarget(GetSpellInfo()->Id, target->GetGUID());
                }
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            if (!caster)
                break;

            switch(GetSpellInfo()->Id)
            {
                // Devouring Plague
                case 2944:
                {
                    // Item - Priest T12 Shadow 4P Bonus
                    if (caster->HasAura(99157))
                    {
                        if (target->HasAura(589) && target->HasAura(2944) && target->HasAura(34914))
                            caster->CastSpell(caster, 99158, true);
                        else
                            caster->RemoveAurasDueToSpell(99158);
                    }
                    break;
                }
                // Levitate effect
                case 111758:
                {
                    if (apply)
                    {
                        // Glyph of Levitate
                        if (caster->HasAura(108939))
                            caster->CastSpell(target, 111757, true);
                        // Glyph of the Heavens
                        if (caster == target && caster->HasAura(120581))
                            caster->CastSpell(caster, 124433, true);
                    }
                    else if (!apply)
                    {
                        // Remove Glyph of Levitate
                        if (Aura * const speedBonus = target->GetAura(111757))
                            speedBonus->SetDuration(10 * IN_MILLISECONDS);
                        // Remove Glyph of the Heavens
                        target->RemoveAurasDueToSpell(124433);
                    }
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            if (!caster)
                break;

            switch(GetSpellInfo()->Id)
            {
                // Spell Reflection - Apply Visuals
                case 23920:
                {
                    auto player = caster->ToPlayer();
                    if (!player)
                        break;

                    // Remove all possible visuals
                    if (!apply)
                    {
                        player->RemoveAurasDueToSpell(147923);
                        player->RemoveAurasDueToSpell(146122);
                        player->RemoveAurasDueToSpell(146120);
                    }
                    else
                    {
                        uint32 spellVisual = player->GetTeam() == ALLIANCE ? 147923 : 146122;
                        // If shield equipped - switch visual
                        auto shieldItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                        if (shieldItem && shieldItem->GetTemplate()->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                            spellVisual = 146120;

                        player->CastSpell(player, spellVisual, true);
                    }
                    break;
                }
                // Shield Wall - Apply Visuals
                case 871:
                {
                    auto player = caster->ToPlayer();
                    if (!player)
                        break;

                    // Remove all possible visuals
                    if (!apply)
                    {
                        player->RemoveAurasDueToSpell(147925);
                        player->RemoveAurasDueToSpell(146127);
                        player->RemoveAurasDueToSpell(146128);
                    }
                    else
                    {
                        uint32 spellVisual = player->GetTeam() == ALLIANCE ? 147925 : 146127;
                        // If shield equipped - switch visual
                        auto shieldItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                        if (shieldItem && shieldItem->GetTemplate()->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                            spellVisual = 146128;

                        player->CastSpell(player, spellVisual, true);
                    }
                    break;
                }
            }
        }
        case SPELLFAMILY_MONK:
        {
            switch (GetSpellInfo()->Id)
            {
                // Vital Mists visual on 5 stacks
                case 118674:
                {
                    if (!apply && !onReapply)
                        target->RemoveAurasDueToSpell(122107);
                    else if (apply && GetStackAmount() == 5)
                        target->CastSpell(target, 122107, true);
                    break;
                }
                // Power Guard
                case 118636:
                {
                    if (apply)
                        target->CastSpell(target, 124899, true);
                    else
                        target->RemoveAurasDueToSpell(124899);
                    break;
                }
            }
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Unending Breath
            if (GetSpellInfo()->Id == 5697)
            {
                if (apply && caster)
                {
                    // Apply Soulburn effect
                    if (caster->HasAura(74434))
                        caster->CastSpell(target, 104242, true);
                }
                else
                    target->RemoveAurasDueToSpell(104242);
            }
            // Methamorphosis
            else if (GetSpellInfo()->Id == 103958)
            {
                // Apply Nether Plating bonus
                if (apply && target->HasSpell(114129))
                    target->CastSpell(target, 54817, true);
                else
                    target->RemoveAurasDueToSpell(54817);
            }
            break;
        }
        case SPELLFAMILY_DEATHKNIGHT:
        {
            if (!caster)
                break;

            switch (GetSpellInfo()->Id)
            {
                case 90259:
                {
                    uint32 DA[2] = { 96268, 124285 };
                    for (uint8 i = 0; i < 2; i++)
                        if (Aura* aura = caster->GetAura(DA[i]))
                            aura->RecalculateAmountOfEffects();
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
}

bool Aura::CanBeAppliedOn(Unit* target)
{
    // unit not in world or during remove from world
    if (!target->IsInWorld() || target->IsDuringRemoveFromWorld())
    {
        // area auras mustn't be applied
        if (GetOwner() != target)
            return false;
        // not selfcasted single target auras mustn't be applied
        if (GetCasterGUID() != GetOwner()->GetGUID() && GetSpellInfo()->IsSingleTarget())
            return false;
        return true;
    }
    else
        return CheckAreaTarget(target);
}

bool Aura::CheckAreaTarget(Unit* target)
{
    return CallScriptCheckAreaTargetHandlers(target);
}

bool Aura::CanStackWith(Aura const *existingAura) const
{
    // Can stack with self
    if (this == existingAura)
        return true;

    // Dynobj auras always stack
    if (existingAura->GetType() == DYNOBJ_AURA_TYPE)
        return true;

    SpellInfo const* existingSpellInfo = existingAura->GetSpellInfo();
    bool sameCaster = GetCasterGUID() == existingAura->GetCasterGUID();

    // passive auras don't stack with another rank of the spell cast by same caster
    if (IsPassive() && sameCaster && m_spellInfo->IsDifferentRankOf(existingSpellInfo))
        return false;

    // prevent remove triggering aura by triggered aura
    for (auto const &spellEffect : existingSpellInfo->Effects)
        if (spellEffect.TriggerSpell == m_spellInfo->Id)
            return true;

    // prevent remove triggered aura by triggering aura refresh
    for (auto const &spellEffect : m_spellInfo->Effects)
        if (spellEffect.TriggerSpell == existingSpellInfo->Id)
            return true;

    // check spell specific stack rules
    if (m_spellInfo->IsAuraExclusiveBySpecificWith(existingSpellInfo)
            || (sameCaster && m_spellInfo->IsAuraExclusiveBySpecificPerCasterWith(existingSpellInfo)))
        return false;

    // check spell group stack rules
    SpellGroupStackRule stackRule = sSpellMgr->CheckSpellGroupStackRules(m_spellInfo, existingSpellInfo);
    if (stackRule)
    {
        if (stackRule == SPELL_GROUP_STACK_RULE_EXCLUSIVE)
            return false;
        if (sameCaster && stackRule == SPELL_GROUP_STACK_RULE_EXCLUSIVE_FROM_SAME_CASTER)
            return false;
    }

    if (m_spellInfo->SpellFamilyName != existingSpellInfo->SpellFamilyName)
        return true;

    if (!sameCaster)
    {
        // Channeled auras can stack if not forbidden by db or aura type
        if (existingSpellInfo->IsChanneled())
            return true;

        if (m_spellInfo->AttributesEx3 & SPELL_ATTR3_STACK_FOR_DIFF_CASTERS || m_spellInfo->HasAura(SPELL_AURA_MOD_DAMAGE_FROM_CASTER))
            return true;

        // check same periodic auras

        auto const isPeriodicNotTargetingArea = [](SpellEffectInfo const &spellEffect) {
            switch (spellEffect.ApplyAuraName)
            {
                // DoT or HoT from different casters will stack
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_DUMMY:
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_POWER_BURN:
                case SPELL_AURA_OBS_MOD_POWER:
                case SPELL_AURA_OBS_MOD_HEALTH:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
                    // periodic auras which target areas are not allowed to stack this way (replenishment for example)
                    return !spellEffect.IsTargetingArea();
                default:
                    return false;
            }
        };

        for (auto const &spellEffect : m_spellInfo->Effects)
            if (isPeriodicNotTargetingArea(spellEffect))
                return true;

        for (auto const &spellEffect : existingSpellInfo->Effects)
            if (isPeriodicNotTargetingArea(spellEffect))
                return true;
    }

    if (HasEffectType(SPELL_AURA_CONTROL_VEHICLE) && existingAura->HasEffectType(SPELL_AURA_CONTROL_VEHICLE))
    {
        Vehicle* veh = nullptr;
        if (auto const owner = GetOwner()->ToUnit())
            veh = owner->GetVehicleKit();

        // We should probably just let it stack. Vehicle system will prevent undefined behaviour later
        if (!veh)
            return true;

        // No empty seat available
        if (!veh->GetAvailableSeatCount())
            return false;

        // Empty seat available (skip rest)
        return true;
    }

    // spell of same spell rank chain
    if (m_spellInfo->IsRankOf(existingSpellInfo))
    {
        // don't allow passive area auras to stack
        // This is ugly, maybe there is a better way but we need to avoid stacking of PvP power auras of two-handed weapons
        // It is possible that checking for castitemguid differences here may be fine as well, but not 100% sure about possible negative implications
        if (m_spellInfo->IsMultiSlotAura() && !IsArea() && !(existingSpellInfo->Id == 132586 && GetSpellInfo()->Id == 132586)) 
            return true;

        if (GetCastItemGUID() && existingAura->GetCastItemGUID())
            if (GetCastItemGUID() != existingAura->GetCastItemGUID() && (m_spellInfo->AttributesCu & SPELL_ATTR0_CU_ENCHANT_STACK))
                return true;

        // same spell with same caster should not stack
        return false;
    }

    return true;
}

bool Aura::IsProcOnCooldown() const
{
    /*if (m_procCooldown)
    {
        if (m_procCooldown > time(NULL))
            return true;
    }*/
    return false;
}

void Aura::AddProcCooldown(uint32 /*msec*/)
{
    //m_procCooldown = time(NULL) + msec;
}

void Aura::PrepareProcToTrigger(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    bool prepare = CallScriptPrepareProcHandlers(aurApp, eventInfo);
    if (!prepare)
        return;

    // take one charge, aura expiration will be handled in Aura::TriggerProcOnEvent (if needed)
    if (IsUsingCharges())
    {
        --m_procCharges;
        SetNeedClientUpdateForTargets();
    }

    SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId());

    ASSERT(procEntry);

    // cooldowns should be added to the whole aura (see 51698 area aura)
    AddProcCooldown(procEntry->cooldown);
}

bool Aura::IsProcTriggeredOnEvent(AuraApplication* aurApp, ProcEventInfo& eventInfo) const
{
    SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId());
    // only auras with spell proc entry can trigger proc
    if (!procEntry)
        return false;

    // check if we have charges to proc with
    if (IsUsingCharges() && !GetCharges())
        return false;

    // check proc cooldown
    if (IsProcOnCooldown())
        return false;

    // TODO:
    // something about triggered spells triggering, and add extra attack effect

    // do checks against db data
    if (!sSpellMgr->CanSpellTriggerProcOnEvent(*procEntry, eventInfo))
        return false;

    // do checks using conditions table
    ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_SPELL_PROC, GetId());
    ConditionSourceInfo condInfo = ConditionSourceInfo(eventInfo.GetActor(), eventInfo.GetActionTarget());
    if (!sConditionMgr->IsObjectMeetToConditions(condInfo, conditions))
        return false;

    // AuraScript Hook
    bool check = const_cast<Aura*>(this)->CallScriptCheckProcHandlers(aurApp, eventInfo);
    if (!check)
        return false;

    // TODO:
    // do allow additional requirements for procs
    // this is needed because this is the last moment in which you can prevent aura charge drop on proc
    // and possibly a way to prevent default checks (if there're going to be any)

    // Check if current equipment meets aura requirements
    // do that only for passive spells
    // TODO: this needs to be unified for all kinds of auras
    Unit* target = aurApp->GetTarget();
    if (IsPassive() && target->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetSpellInfo()->EquippedItemClass == ITEM_CLASS_WEAPON)
        {
            if (target->ToPlayer()->IsInFeralForm())
                return false;

            if (eventInfo.GetDamageInfo())
            {
                WeaponAttackType attType = eventInfo.GetDamageInfo()->GetAttackType();
                Item* item = NULL;
                if (attType == BASE_ATTACK)
                    item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                else if (attType == OFF_ATTACK)
                    item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                if (!item || item->IsBroken() || item->GetTemplate()->Class != ITEM_CLASS_WEAPON || !((1<<item->GetTemplate()->SubClass) & GetSpellInfo()->EquippedItemSubClassMask))
                    return false;
            }
        }
        else if (GetSpellInfo()->EquippedItemClass == ITEM_CLASS_ARMOR)
        {
            // Check if player is wearing shield
            Item* item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (!item || item->IsBroken() || item->GetTemplate()->Class != ITEM_CLASS_ARMOR || !((1<<item->GetTemplate()->SubClass) & GetSpellInfo()->EquippedItemSubClassMask))
                return false;
        }
    }

    return roll_chance_f(CalcProcChance(*procEntry, eventInfo));
}

float Aura::CalcProcChance(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo) const
{
    float chance = procEntry.chance;
    // calculate chances depending on unit with caster's data
    // so talents modifying chances and judgements will have properly calculated proc chance
    if (Unit* caster = GetCaster())
    {
        // calculate ppm chance if present and we're using weapon
        if (eventInfo.GetDamageInfo() && procEntry.ratePerMinute != 0)
        {
            uint32 WeaponSpeed = caster->GetAttackTime(eventInfo.GetDamageInfo()->GetAttackType());
            chance = caster->GetPPMProcChance(WeaponSpeed, procEntry.ratePerMinute, GetSpellInfo());
        }
        // apply chance modifer aura, applies also to ppm chance (see improved judgement of light spell)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_CHANCE_OF_SUCCESS, chance);
    }
    return chance;
}

void Aura::TriggerProcOnEvent(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    CallScriptProcHandlers(aurApp, eventInfo);

    for (uint8 i = 0; i < GetSpellInfo()->Effects.size(); ++i)
        if (aurApp->HasEffect(i))
            // OnEffectProc / AfterEffectProc hooks handled in AuraEffect::HandleProc()
            GetEffect(i)->HandleProc(aurApp, eventInfo);

    CallScriptAfterProcHandlers(aurApp, eventInfo);

    // Remove aura if we've used last charge to proc
    if (IsUsingCharges() && !GetCharges())
        Remove();
}

void Aura::_DeleteRemovedApplications()
{
    while (!m_removedApplications.empty())
    {
        delete m_removedApplications.front();
        m_removedApplications.pop_front();
    }
}

void Aura::LoadScripts()
{
    sScriptMgr->CreateAuraScripts(m_spellInfo->Id, m_loadedScripts);
    for (std::list<AuraScript*>::iterator itr = m_loadedScripts.begin(); itr != m_loadedScripts.end();)
    {
        if (!(*itr)->_Load(this))
        {
            std::list<AuraScript*>::iterator bitr = itr;
            ++itr;
            delete (*bitr);
            m_loadedScripts.erase(bitr);
            continue;
        }
        TC_LOG_DEBUG("spells", "Aura::LoadScripts: Script `%s` for aura `%u` is loaded now", (*itr)->_GetScriptName()->c_str(), m_spellInfo->Id);
        (*itr)->Register();
        ++itr;
    }
}

void Aura::CallScriptRefreshChargesHandlers(uint8 &charges)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_REFRESH_CHARGES);
        for (auto &hook : script->OnRefreshCharges)
            hook.Call(script, charges);
    }
}

void Aura::CallScriptInitEffectsHandlers(uint32 &effectMask)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_INIT_EFFECTS);
        for (auto &hook : script->OnInitEffects)
            hook.Call(script, effectMask);
    }
}

bool Aura::CallScriptCheckAreaTargetHandlers(Unit* target)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_CHECK_AREA_TARGET);
        for (auto &hook : script->DoCheckAreaTarget)
            if (!hook.Call(script, target))
                return false;
    }
    return true;
}

void Aura::CallScriptDispel(DispelInfo* dispelInfo)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_DISPEL);
        for (auto &hook : script->OnDispel)
            hook.Call(script, dispelInfo);
    }
}

void Aura::CallScriptAfterDispel(DispelInfo* dispelInfo)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_AFTER_DISPEL);
        for (auto &hook : script->AfterDispel)
            hook.Call(script, dispelInfo);
    }
}

bool Aura::CallScriptEffectApplyHandlers(AuraEffect const *aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    bool preventDefault = false;

    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_APPLY, aurApp);

        for (auto &hook : script->OnEffectApply)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, mode);

        if (!preventDefault)
            preventDefault = script->_IsDefaultActionPrevented();
    }

    return preventDefault;
}

bool Aura::CallScriptEffectRemoveHandlers(AuraEffect const *aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    bool preventDefault = false;

    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_REMOVE, aurApp);

        for (auto &hook : script->OnEffectRemove)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, mode);

        if (!preventDefault)
            preventDefault = script->_IsDefaultActionPrevented();
    }

    return preventDefault;
}

void Aura::CallScriptAfterEffectApplyHandlers(AuraEffect const *aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_AFTER_APPLY, aurApp);
        for (auto &hook : script->AfterEffectApply)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, mode);
    }
}

void Aura::CallScriptAfterEffectRemoveHandlers(AuraEffect const *aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_AFTER_REMOVE, aurApp);
        for (auto &hook : script->AfterEffectRemove)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, mode);
    }
}

bool Aura::CallScriptEffectPeriodicHandlers(AuraEffect const *aurEff, AuraApplication const* aurApp)
{
    bool preventDefault = false;

    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_PERIODIC, aurApp);

        for (auto &hook : script->OnEffectPeriodic)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff);

        if (!preventDefault)
            preventDefault = script->_IsDefaultActionPrevented();
    }

    return preventDefault;
}

void Aura::CallScriptAuraUpdateHandlers(uint32 diff)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_ON_UPDATE);
        for (auto &hook : script->OnAuraUpdate)
            hook.Call(script, diff);
    }
}

void Aura::CallScriptEffectUpdateHandlers(uint32 diff, AuraEffect *aurEff)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_UPDATE);
        for (auto &hook : script->OnEffectUpdate)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, diff, aurEff);
    }
}

void Aura::CallScriptEffectUpdatePeriodicHandlers(AuraEffect *aurEff)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_UPDATE_PERIODIC);
        for (auto &hook : script->OnEffectUpdatePeriodic)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff);
    }
}

void Aura::CallScriptEffectCalcAmountHandlers(AuraEffect const *aurEff, int32 &amount, bool &canBeRecalculated)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_CALC_AMOUNT);
        for (auto &hook : script->DoEffectCalcAmount)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, amount, canBeRecalculated);
    }
}

void Aura::CallScriptEffectCalcPeriodicHandlers(AuraEffect const *aurEff, bool & isPeriodic, int32 & amplitude)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_CALC_PERIODIC);
        for (auto &hook : script->DoEffectCalcPeriodic)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, isPeriodic, amplitude);
    }
}

void Aura::CallScriptEffectCalcSpellModHandlers(AuraEffect const *aurEff)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_CALC_SPELLMOD);
        for (auto &hook : script->DoEffectCalcSpellMod)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff);
    }
}

void Aura::CallScriptEffectDropModChargeHandlers(AuraEffect *aurEff, AuraApplication const *aurApp)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_DROP_MOD_CHARGE, aurApp);
        for (auto &hook : script->OnEffectDropModCharge)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff);
    }
}

void Aura::CallScriptEffectAbsorbHandlers(AuraEffect *aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount, bool& defaultPrevented)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_ABSORB, aurApp);

        for (auto &hook : script->OnEffectAbsorb)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, dmgInfo, absorbAmount);

        defaultPrevented = script->_IsDefaultActionPrevented();
    }
}

void Aura::CallScriptEffectAfterAbsorbHandlers(AuraEffect *aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_AFTER_ABSORB, aurApp);
        for (auto &hook : script->AfterEffectAbsorb)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, dmgInfo, absorbAmount);
    }
}

void Aura::CallScriptEffectManaShieldHandlers(AuraEffect *aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount, bool & /*defaultPrevented*/)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_MANASHIELD, aurApp);
        for (auto &hook : script->OnEffectManaShield)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, dmgInfo, absorbAmount);
    }
}

void Aura::CallScriptEffectAfterManaShieldHandlers(AuraEffect *aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_AFTER_MANASHIELD, aurApp);
        for (auto &hook : script->AfterEffectManaShield)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, dmgInfo, absorbAmount);
    }
}

void Aura::CallScriptEffectSplitHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & splitAmount)
{
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_SPLIT, aurApp);
        std::list<AuraScript::EffectSplitHandler>::iterator effEndItr = (*scritr)->OnEffectSplit.end(), effItr = (*scritr)->OnEffectSplit.begin();
        for (; effItr != effEndItr; ++effItr)
            if (effItr->IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                effItr->Call(*scritr, aurEff, dmgInfo, splitAmount);

        (*scritr)->_FinishScriptCall();
    }
}

void Aura::SetScriptData(uint32 type, uint32 data)
{
    for (auto &script : m_loadedScripts)
        script->SetData(type, data);
}

void Aura::SetScriptGuid(uint32 type, uint64 data)
{
    for (auto &script : m_loadedScripts)
        script->SetGuid(type, data);
}

bool Aura::CallScriptCheckProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_CHECK_PROC, aurApp);
        for (auto &hook : script->DoCheckProc)
            if (!hook.Call(script, eventInfo))
                return false;
    }
    return true;
}

bool Aura::CallScriptPrepareProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    bool prepare = true;

    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_PREPARE_PROC, aurApp);

        for (auto &hook : script->DoPrepareProc)
            hook.Call(script, eventInfo);

        if (prepare && script->_IsDefaultActionPrevented())
            prepare = false;
    }

    return prepare;
}

void Aura::CallScriptProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_PROC, aurApp);
        for (auto &hook : script->OnProc)
            hook.Call(script, eventInfo);
    }
}

void Aura::CallScriptAfterProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_AFTER_PROC, aurApp);
        for (auto &hook : script->AfterProc)
            hook.Call(script, eventInfo);
    }
}

bool Aura::CallScriptEffectProcHandlers(AuraEffect const *aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    bool preventDefault = false;

    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_PROC, aurApp);

        for (auto &hook : script->OnEffectProc)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, eventInfo);

        if (!preventDefault)
            preventDefault = script->_IsDefaultActionPrevented();
    }

    return preventDefault;
}

void Aura::CallScriptAfterEffectProcHandlers(AuraEffect const *aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    for (auto &script : m_loadedScripts)
    {
        auto const g = Trinity::makeScriptCallGuard(script, AURA_SCRIPT_HOOK_EFFECT_AFTER_PROC, aurApp);
        for (auto &hook : script->AfterEffectProc)
            if (hook.IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                hook.Call(script, aurEff, eventInfo);
    }
}

UnitAura::UnitAura(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32 *baseAmount, Item* castItem, uint64 casterGUID)
    : Aura(spellproto, owner, caster, spellPowerData, castItem, casterGUID)
{
    m_spellPowerData = spellPowerData;
    m_AuraDRGroup = DIMINISHING_NONE;

    _InitEffects(effMask, caster, baseAmount);
    GetUnitOwner()->_AddAura(this, caster);
}

void UnitAura::_ApplyForTarget(Unit* target, Unit* caster, AuraApplication * aurApp)
{
    Aura::_ApplyForTarget(target, caster, aurApp);

    // register aura diminishing on apply
    if (DiminishingGroup group = GetDiminishGroup())
        target->ApplyDiminishingAura(group, true);
}

void UnitAura::_UnapplyForTarget(Unit* target, Unit* caster, AuraApplication * aurApp)
{
    Aura::_UnapplyForTarget(target, caster, aurApp);

    // unregister aura diminishing (and store last time)
    if (DiminishingGroup group = GetDiminishGroup())
        target->ApplyDiminishingAura(group, false);
}

void UnitAura::Remove(AuraRemoveMode removeMode)
{
    if (IsRemoved())
        return;
    GetUnitOwner()->RemoveOwnedAura(this, removeMode);
}

void UnitAura::FillTargetMap(std::map<Unit*, uint32> & targets, Unit* caster)
{
    for (uint8 effIndex = 0; effIndex < GetSpellInfo()->Effects.size(); ++effIndex)
    {
        if (!HasEffect(effIndex))
            continue;
        UnitList targetList;
        // non-area aura
        if (GetSpellInfo()->Effects[effIndex].Effect == SPELL_EFFECT_APPLY_AURA)
        {
            targetList.push_back(GetUnitOwner());
        }
        else
        {
            float radius = GetSpellInfo()->Effects[effIndex].CalcRadius(caster);

            bool isolated = GetUnitOwner()->HasUnitState(UNIT_STATE_ISOLATED);
            switch (GetSpellInfo()->Effects[effIndex].Effect)
            {
                case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
                case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
                {
                    if (!GetUnitOwner()->IsAlive())
                        break;

                    targetList.push_back(GetUnitOwner());
                    if (!isolated)
                    {
                        Trinity::AnyGroupedUnitInObjectRangeCheck u_check(GetUnitOwner(), GetUnitOwner(), radius, GetSpellInfo()->Effects[effIndex].Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID);
                        Trinity::UnitListSearcher<Trinity::AnyGroupedUnitInObjectRangeCheck> searcher(GetUnitOwner(), targetList, u_check);
                        Trinity::VisitNearbyObject(GetOwner(), radius, searcher);
                    }
                    break;
                }
                case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
                {
                    targetList.push_back(GetUnitOwner());
                    if (!isolated)
                    {
                        Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(GetUnitOwner(), GetUnitOwner(), radius);
                        Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(GetUnitOwner(), targetList, u_check);
                        Trinity::VisitNearbyObject(GetOwner(), radius, searcher);
                    }
                    break;
                }
                case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
                {
                    Trinity::AnyAoETargetUnitInObjectRangeCheck u_check(GetUnitOwner(), GetUnitOwner(), radius); // No GetCharmer in searcher
                    Trinity::UnitListSearcher<Trinity::AnyAoETargetUnitInObjectRangeCheck> searcher(GetUnitOwner(), targetList, u_check);
                    Trinity::VisitNearbyObject(GetOwner(), radius, searcher);
                    break;
                }
                case SPELL_EFFECT_APPLY_AREA_AURA_PET:
                case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
                {
                    targetList.push_back(GetUnitOwner());
                    if (Unit* owner = GetUnitOwner()->GetCharmerOrOwner())
                        if (!isolated && GetUnitOwner()->IsWithinDistInMap(owner, radius))
                            targetList.push_back(owner);
                    break;
                }
            }
        }

        for (UnitList::iterator itr = targetList.begin(); itr!= targetList.end();++itr)
        {
            std::map<Unit*, uint32>::iterator existing = targets.find(*itr);
            if (existing != targets.end())
                existing->second |= 1<<effIndex;
            else
                targets[*itr] = 1<<effIndex;
        }
    }
}

DynObjAura::DynObjAura(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32 *baseAmount, Item* castItem, uint64 casterGUID)
    : Aura(spellproto, owner, caster, spellPowerData, castItem, casterGUID)
{
    ASSERT(GetDynobjOwner());
    ASSERT(GetDynobjOwner()->IsInWorld());
    ASSERT(GetCaster()->IsInWorld());
    ASSERT(GetDynobjOwner()->GetMap() == GetCaster()->GetMap());

    m_spellPowerData = spellPowerData;

    _InitEffects(effMask, caster, baseAmount);
    GetDynobjOwner()->SetAura(this);
}

void DynObjAura::Remove(AuraRemoveMode removeMode)
{
    if (IsRemoved())
        return;
    _Remove(removeMode);
}

void DynObjAura::FillTargetMap(std::map<Unit*, uint32> & targets, Unit* /*caster*/)
{
    Unit* dynObjOwnerCaster = GetDynobjOwner()->GetCaster();
    float radius = GetDynobjOwner()->GetRadius();

    for (uint8 effIndex = 0; effIndex < GetSpellInfo()->Effects.size(); ++effIndex)
    {
        if (!HasEffect(effIndex))
            continue;
        UnitList targetList;
        if (GetSpellInfo()->Effects[effIndex].TargetB.GetTarget() == TARGET_DEST_DYNOBJ_ALLY
            || GetSpellInfo()->Effects[effIndex].TargetB.GetTarget() == TARGET_UNIT_DEST_AREA_ALLY)
        {
            Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(GetDynobjOwner(), dynObjOwnerCaster, radius);
            Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(GetDynobjOwner(), targetList, u_check);
            Trinity::VisitNearbyObject(GetDynobjOwner(), radius, searcher);
        }
        else if (GetSpellInfo()->Effects[effIndex].Effect != SPELL_EFFECT_CREATE_AREATRIGGER)
        {
            Trinity::AnyAoETargetUnitInObjectRangeCheck u_check(GetDynobjOwner(), dynObjOwnerCaster, radius);
            Trinity::UnitListSearcher<Trinity::AnyAoETargetUnitInObjectRangeCheck> searcher(GetDynobjOwner(), targetList, u_check);
            Trinity::VisitNearbyObject(GetDynobjOwner(), radius, searcher);
        }

        for (UnitList::iterator itr = targetList.begin(); itr!= targetList.end();++itr)
        {
            if (dynObjOwnerCaster->MagicSpellHitResult((*itr), m_spellInfo))
                continue;

            std::map<Unit*, uint32>::iterator existing = targets.find(*itr);
            if (existing != targets.end())
                existing->second |= 1<<effIndex;
            else
                targets[*itr] = 1<<effIndex;
        }
    }
}
