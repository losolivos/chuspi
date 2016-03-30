/*
 * Copyright (C) 2012-2013 Trinity <http://www.pandashan.com/>
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

#ifndef TRINITYCORE_AREATRIGGER_H
#define TRINITYCORE_AREATRIGGER_H

#include "Object.h"

class Unit;
class SpellInfo;
class Spell;
class AuraEffect;
class AuraApplication;

enum
{
    AREATRIGGER_FLAG_SCALED = 1 << 0,
};

enum
{
    AREATRIGGER_COLLISION_TYPE_NONE         = 0,
    AREATRIGGER_COLLISION_TYPE_SPHERE       = 1,
};

enum AreatriggerInterpolation
{
    AREATRIGGER_INTERPOLATION_NONE,
    AREATRIGGER_INTERPOLATION_LINEAR,
};

struct AreaTriggerTemplate
{
    uint32 Entry;
    uint32 Flags;
    uint32 CollisionType;
    float Radius;
    float ScaleX;
    float ScaleY;
    uint32 ScriptId;
};

typedef std::unordered_map<uint32, AreaTriggerTemplate> AreaTriggerTemplateContainer;

class IAreaTrigger
{
public:
    void Initialize(Unit* caster, WorldObject* target, SpellInfo const* spellInfo)
    {
        m_caster = caster;
        m_target = target;
        m_spellInfo = spellInfo;
        m_location = m_target;
    }

    void Initialize(AuraEffect const* auraEffect, AuraApplication const* auraApplication);

    virtual ~IAreaTrigger() {}

    virtual void OnUpdate(uint32 diff) {}
    virtual void OnExpire() {}
    virtual void OnDestroy() {}

    void Destroy();

    Unit* GetCaster() { return m_caster; }
    WorldLocation* GetLocation() { return m_location; }
    float GetRange() { return m_range; }
    WorldObject* GetTarget() { return m_target; }
    SpellInfo const* GetSpellInfo() { return m_spellInfo; }
    AuraApplication const* GetAuraApplication() { return m_auraApplication; }
    AuraEffect const* GetAuraEffect();

public:
    float m_range;

protected:
    WorldLocation* m_location;
    Unit* m_caster;
    WorldObject* m_target;
    SpellInfo const* m_spellInfo;
    AuraApplication const* m_auraApplication;
    uint32 m_auraEffectIndex;
};

class IAreaTriggerOnce : public IAreaTrigger
{
public:
    IAreaTriggerOnce()
        : m_mapTypeMask(GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER)
    {
    }

    virtual void OnUpdate(uint32 diff) override;

    virtual bool CheckTriggering(WorldObject* triggering) = 0;
    virtual void OnTrigger(WorldObject* triggering) = 0;

protected:
    uint32 m_mapTypeMask;
};

class IAreaTriggerAuraUpdater;
class IAreaTriggerAura : public IAreaTrigger
{
    friend class IAreaTriggerAuraUpdater;
public:
    IAreaTriggerAura()
        : m_mapTypeMask(GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER)
    {
        m_updateNumber = 0;
    }

    ~IAreaTriggerAura();

    virtual void OnUpdate(uint32 diff) override;
    virtual void OnDestroy() override;

    virtual bool CheckTriggering(WorldObject* triggering) = 0;
    virtual void OnTriggeringApply(WorldObject* triggering) {}
    virtual void OnTriggeringUpdate(WorldObject* triggering) {}
    virtual void OnTriggeringRemove(WorldObject* triggering) {}

protected:
    uint32 m_mapTypeMask;

private:
    struct Triggering
    {
        uint32 UpdateNumber;
        uint64 Guid;
    };
    typedef std::list<Triggering> TriggeringList;

    TriggeringList m_triggerings;
    uint32 m_updateNumber;
};


class AreaTrigger : public WorldObject, public GridObject<AreaTrigger>
{
    public:
        AreaTrigger();
        ~AreaTrigger();

        void AddToWorld();
        void RemoveFromWorld();

        bool CreateAreaTrigger(uint32 guidlow, uint32 triggerEntry, Unit* caster, SpellInfo const* spell, Position const& pos);
        void Update(uint32 p_time);
        void Remove();
        void Expire();
        uint32 GetSpellId() const { return GetUInt32Value(AREATRIGGER_SPELLID); }
        int32 GetDuration() const { return _duration; }
        uint32 GetTimer() const { return m_timer; }
        void SetTimer(uint32 newTimer) { m_timer = newTimer; }
        void SetDuration(int32 newDuration) { _duration = newDuration; }
        void Delay(int32 delaytime) { SetDuration(GetDuration() - delaytime); }
        Unit* GetCaster() const { return m_caster; }
        uint64 GetCasterGUID() const { return GetUInt64Value(AREATRIGGER_CASTER); }
        void BindToCaster();
        void UnbindFromCaster();

        float GetVisualRadius() const { return m_visualRadius; }
        void SetVisualRadius(float radius) { m_visualRadius = radius; }

    protected:
        uint32 m_timer;
        int32 _duration;
        Unit* m_caster;
        float m_visualRadius;
        IAreaTrigger* m_interface;
};
#endif