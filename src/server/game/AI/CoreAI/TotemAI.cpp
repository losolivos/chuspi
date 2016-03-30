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

#include "TotemAI.h"
#include "Totem.h"
#include "Creature.h"
#include "DBCStores.h"
#include "ObjectAccessor.h"
#include "SpellMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ObjectVisitors.hpp"

int TotemAI::Permissible(Creature const* creature)
{
    if (creature->isTotem())
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

TotemAI::TotemAI(Creature* c) : CreatureAI(c), i_victimGuid(0)
{
    ASSERT(c->isTotem());
}

void TotemAI::MoveInLineOfSight(Unit* /*who*/)
{
}

void TotemAI::EnterEvadeMode()
{
    me->CombatStop(true);
}

void TotemAI::UpdateAI(uint32 const /*diff*/)
{
    if (me->ToTotem()->GetTotemType() != TOTEM_ACTIVE)
        return;

    if (!me->IsAlive())
        return;

    // pointer to appropriate target if found any
    Unit* victim = i_victimGuid ? ObjectAccessor::GetUnit(*me, i_victimGuid) : NULL;

    if (me->IsNonMeleeSpellCasted(false))
    {
        if (victim && victim->HasCrowdControlAura())
            victim = NULL;
        else
            return;
    }


    // Search spell
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(me->ToTotem()->GetSpell());
    if (!spellInfo)
        return;

    // Get spell range
    float max_range = spellInfo->GetMaxRange(false);

    // Apply SPELLMOD_RANGE from owner (required by Elemental Reach talent at least)
    if (Player * const modOwner = me->GetSpellModOwner())
        modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_RANGE, max_range);


    // Searing Totem prioritize Flame Shock or Stormstrike targets
    if (spellInfo->Id == 3606)
    {
        if (Unit * caster = me->GetOwner())
        {
            std::list<Unit*> targets;
            Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, max_range);
            Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
            Trinity::VisitNearbyObject(caster, max_range, searcher);
            // Find Stormstrike and Flame Shock -> Stormstrike -> Flame Shock
            const uint64 guid = caster->GetGUID();
            auto itr = std::find_if(targets.begin(), targets.end(), [guid](Unit *u) { return u->HasAura(17364, guid) && u->HasAura(8050, guid); });
            if (itr == targets.end())
                itr = std::find_if(targets.begin(), targets.end(), [guid](Unit *u) { return u->HasAura(17364, guid); });
            if (itr == targets.end())
                itr = std::find_if(targets.begin(), targets.end(), [guid](Unit *u) { return u->HasAura(8050, guid); });
            if (itr != targets.end())
                victim = *itr;
        }

        if (!victim)
        {
            victim = me->GetOwner()->GetVictim();
            if (!victim)
                return;
        }
    }

    // Search victim if no, not attackable, or out of range, or friendly (possible in case duel end)
    if (!victim ||
        !victim->isTargetableForAttack() || !me->IsWithinDistInMap(victim, max_range) ||
        me->IsFriendlyTo(victim) || !me->canSeeOrDetect(victim) || victim->HasCrowdControlAura())
    {
        victim = NULL;
        Trinity::NearestAttackableNoCCUnitInObjectRangeCheck u_check(me, me, max_range);
        Trinity::UnitLastSearcher<Trinity::NearestAttackableNoCCUnitInObjectRangeCheck> checker(me, victim, u_check);
        Trinity::VisitNearbyObject(me, max_range, checker);
    }

    // If have target
    if (victim)
    {
        // remember
        i_victimGuid = victim->GetGUID();

        // attack
        me->SetInFront(victim);                         // client change orientation by self
        me->CastSpell(victim, me->ToTotem()->GetSpell(), false);
    }
    else
        i_victimGuid = 0;
}

void TotemAI::AttackStart(Unit* /*victim*/)
{
    // Sentry totem sends ping on attack
    if (me->GetEntry() == SENTRY_TOTEM_ENTRY && me->GetOwner()->GetTypeId() == TYPEID_PLAYER)
    {
        // everything's fine, do it
        ObjectGuid totemGuid = me->GetGUID();

        WorldPacket data(SMSG_MINIMAP_PING, (8+4+4));
        data.WriteBitSeq<6, 5, 1, 2, 4, 0, 3, 7>(totemGuid);
        data.WriteByteSeq<0, 5, 2>(totemGuid);
        data << float(me->GetPositionX());
        data.WriteByteSeq<4, 1, 7, 3>(totemGuid);
        data << float(me->GetPositionY());
        data.WriteByteSeq<6>(totemGuid);

        me->GetOwner()->ToPlayer()->SendDirectMessage(&data);
    }
}
