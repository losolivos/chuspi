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

#ifndef TRINITY_PHASEMGR_H
#define TRINITY_PHASEMGR_H

#include "Define.h"
#include "ConditionMgr.h"

#include <unordered_map>

class AuraEffect;
struct Condition;
class Player;

// Phasing (visibility)
enum PhasingFlags
{
    PHASE_FLAG_OVERWRITE_EXISTING           = 0x01,       // don't stack with existing phases, overwrites existing phases
    PHASE_FLAG_NO_MORE_PHASES               = 0x02,       // stop calculating phases after this phase was applied (no more phases will be applied)
    PHASE_FLAG_NEGATE_PHASE                 = 0x04        // negate instead to add the phasemask
};

enum PhaseUpdateFlag
{
    PHASE_UPDATE_FLAG_ZONE_UPDATE           = 0x01,
    PHASE_UPDATE_FLAG_AREA_UPDATE           = 0x02,

    // Internal flags
    PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED    = 0x08,
    PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED    = 0x10,
};

struct PhaseDefinition
{
    PhaseDefinition() :
        zoneId(), entry(), phasemask(), phaseId(), terrainswapmap(),
        worldMapAreaId(), flags()
    { }

    uint32 zoneId;
    uint16 entry;
    uint32 phasemask;
    uint16 phaseId;
    uint16 terrainswapmap;
    uint16 worldMapAreaId;
    uint8 flags;

    bool IsOverwritingExistingPhases() const { return flags & PHASE_FLAG_OVERWRITE_EXISTING; }
    bool IsLastDefinition() const { return flags & PHASE_FLAG_NO_MORE_PHASES; }
    bool IsNegatingPhasemask() const { return flags & PHASE_FLAG_NEGATE_PHASE; }
};

typedef std::set<uint16> PhaseShiftSet;

typedef std::vector<PhaseDefinition> PhaseDefinitionVector;
typedef std::unordered_map<uint32 /*zoneId*/, PhaseDefinitionVector> PhaseDefinitionMap;

struct SpellPhaseInfo
{
    uint32 spellId;
    uint32 phasemask;
    uint16 terrainswapmap;
};

typedef std::unordered_map<uint32 /*spellId*/, SpellPhaseInfo> SpellPhaseStore;

struct PhaseInfo final
{
    PhaseInfo()
        : phasemask(), phaseId(), terrainswapmap(), worldMapAreaId()
    { }

    uint32 phasemask;
    uint16 phaseId;
    uint16 terrainswapmap;
    uint16 worldMapAreaId;

    bool NeedsServerSideUpdate() const { return phasemask; }
    bool NeedsClientSideUpdate() const { return terrainswapmap || phaseId || worldMapAreaId; }
};

struct PhaseData
{
    typedef std::unordered_map<uint32 /*spellId*/, PhaseInfo> PhaseInfoContainer;
    typedef std::vector<PhaseDefinition const*> PhaseDefinitionVector;

    PhaseData(Player* _player)
        : _PhasemaskThroughDefinitions(0)
        , _PhasemaskThroughAuras(0)
        , _CustomPhasemask(0)
        , player(_player)
    { }

    uint32 _PhasemaskThroughDefinitions;
    uint32 _PhasemaskThroughAuras;
    uint32 _CustomPhasemask;

    uint32 GetCurrentPhasemask() const;
    inline uint32 GetPhaseMaskForSpawn() const;

    void ResetDefinitions() { _PhasemaskThroughDefinitions = 0; activePhaseDefinitions.clear(); }
    void AddPhaseDefinition(PhaseDefinition const* phaseDefinition);
    bool HasActiveDefinitions() const { return !activePhaseDefinitions.empty(); }

    void AddAuraInfo(uint32 const spellId, PhaseInfo phaseInfo);
    uint32 RemoveAuraInfo(uint32 const spellId);

    void SendPhaseMaskToPlayer();
    void SendPhaseshiftToPlayer();

    void GetActivePhases(std::set<uint32>& phases) const;

private:
    Player* player;
    PhaseDefinitionVector activePhaseDefinitions;
    PhaseInfoContainer spellPhaseInfo;
};

struct PhaseUpdateData
{
    PhaseUpdateData()
        : _conditionTypeFlags()
        , _questId()
    { }

    void AddConditionType(ConditionTypes const conditionType) { _conditionTypeFlags |= (1 << conditionType); }
    void AddQuestUpdate(uint32 const questId);

    bool IsConditionRelated(Condition const* condition) const;

private:
    uint32 _conditionTypeFlags;
    uint32 _questId;
};

class PhaseMgr
{
public:
    PhaseMgr(Player* _player);
    ~PhaseMgr() {}

    uint32 GetCurrentPhasemask() { return phaseData.GetCurrentPhasemask(); };
    inline uint32 GetPhaseMaskForSpawn() { return phaseData.GetCurrentPhasemask(); }

    // Phase definitions update handling
    void NotifyConditionChanged(PhaseUpdateData const& updateData);
    void NotifyStoresReloaded() { Recalculate(); Update(); }

    void Update();

    // Aura phase effects
    void RegisterPhasingAuraEffect(AuraEffect const* auraEffect);
    void UnRegisterPhasingAuraEffect(AuraEffect const* auraEffect);

    // Update flags (delayed phasing)
    void AddUpdateFlag(PhaseUpdateFlag const updateFlag) { _UpdateFlags |= updateFlag; }
    void RemoveUpdateFlag(PhaseUpdateFlag const updateFlag);

    // Needed for modify phase command
    void SetCustomPhase(uint32 const phaseMask);

    // Debug
    void SendDebugReportToPlayer(Player* const debugger);

    static bool IsConditionTypeSupported(ConditionTypes const conditionType);

    void GetActivePhases(std::set<uint32>& phases) const;

private:
    void Recalculate();

    bool CheckDefinition(PhaseDefinition const* phaseDefinition);

    bool NeedsPhaseUpdateWithData(PhaseUpdateData const updateData) const;

    bool IsUpdateInProgress() const { return (_UpdateFlags & PHASE_UPDATE_FLAG_ZONE_UPDATE) || (_UpdateFlags & PHASE_UPDATE_FLAG_AREA_UPDATE); }

    PhaseDefinitionMap const* _PhaseDefinitionStore;
    SpellPhaseStore const* _SpellPhaseStore;

    Player* player;
    PhaseData phaseData;
    uint8 _UpdateFlags;
};

#endif
