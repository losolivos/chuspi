/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: mount_hyjal
SD%Complete: 10%
SDComment: Support for quest: 25731#
SDCategory: Mount Hyjal
EndScriptData */

/* ContentData
npc_marion_wormwing

EndContentData */

#include "ScriptPCH.h"

/*######
# npc_marion_wormwing
######*/

#define GOSSIP_START_1 "<Soften her up.>"
#define GOSSIP_START_2 "Why are you stealing eggs?"

#define GOSSIP_REP_2_1 "Who? Who are you giving the eggs to?"
#define GOSSIP_REP_2_2 "Brood of the Earth-Warder... you answer to a black dragon? Give me a name!"

#define GOSSIP_LAST_1 "<Order Thisalee to kill the harpy.>"
#define GOSSIP_LAST_2 "<Ask Thisalee to release the harpy.>"

// Raufen TODO
// Get Gossip NPC Text
class npc_marion_wormwing : public CreatureScript
{
public:
    npc_marion_wormwing() : CreatureScript("npc_marion_wormwing") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_marion_wormwingAI (creature);
    }

    struct npc_marion_wormwingAI : public ScriptedAI
    {
        npc_marion_wormwingAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & damage)
        {
            if (me->GetHealthPct() < 50)
            {
                damage = 0;
                me->setFaction(35);
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            }
        }
    };

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
        }
        else if (action == GOSSIP_ACTION_INFO_DEF+2)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_REP_2_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        else if (action == GOSSIP_ACTION_INFO_DEF+3)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_REP_2_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        else if (action == GOSSIP_ACTION_INFO_DEF+4)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LAST_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LAST_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        else if (action == GOSSIP_ACTION_INFO_DEF+5)
        {
            player->CLOSE_GOSSIP_MENU();
            player->Kill(creature);
            player->KilledMonsterCredit(41170);
        }
        else if (action == GOSSIP_ACTION_INFO_DEF+6)
        {
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            player->CLOSE_GOSSIP_MENU();
            creature->ForcedDespawn(3000);
            player->KilledMonsterCredit(41170);
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (!player->GetQuestRewardStatus(25731))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_START_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_START_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        }

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }
};

class AreaTrigger_at_nemesis_crystal : public AreaTriggerScript
{
    public:

        AreaTrigger_at_nemesis_crystal()
            : AreaTriggerScript("at_nemesis_crystal") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/)
        {
            if (Creature * turtle = GetClosestCreatureWithEntry(player, 41581, 30.0f))
            {
                if (GameObject * crystal = GetClosestGameObjectWithEntry(player, 203375, 15.0f))
                {
                    float fX, fY, fZ;
                    crystal->GetContactPoint(turtle, fX, fY, fZ, 2.0f);
                    turtle->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
                }
            }
            return true;
        }
};

class npc_child_of_tortolla : public CreatureScript
{
    enum
    {
        DATA_NEMESIS        = 1,
        AREA_THE_CRUCIBLE   = 5099
    };

    struct npc_child_of_tortollaAI : public ScriptedAI
    {
        npc_child_of_tortollaAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset()
        {
            checkTimer = 5000;
        }

        void IsSummonedBy(Unit * who)
        {
            me->GetMotionMaster()->MoveFollow(who, 1.5f, PET_FOLLOW_ANGLE);
            me->setFaction(35);
        }

        void SetData(uint32 type, uint32 data)
        {
            if (type == DATA_NEMESIS && data == DATA_NEMESIS)
            {
                if (Player * owner = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                {
                    owner->KilledMonsterCredit(41602);
                    owner->KilledMonsterCredit(41614);
                }

                me->ForcedDespawn(5000);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (checkTimer <= diff)
            {
                if (me->GetAreaId() != AREA_THE_CRUCIBLE)
                    me->DespawnOrUnsummon();
                checkTimer = 5000;
            } else checkTimer -= diff;
        }

    private:
        uint32 checkTimer;
    };

public:
    npc_child_of_tortolla() : CreatureScript("npc_child_of_tortolla") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_child_of_tortollaAI (creature);
    }
};

// Spells Inferno and Weakening for Quest: The Return of Baron Geddon (25464)

class spell_inferno_baron_geddon : public SpellScriptLoader
{
    public:
        spell_inferno_baron_geddon() : SpellScriptLoader("spell_inferno_baron_geddon") { }

    private:
        class spell_inferno_baron_geddon_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_inferno_baron_geddon_AuraScript)

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                const_cast<AuraEffect*>(aurEff)->SetAmount(2500);
            }

            void OnPeriodic(AuraEffect const* aurEff)
            {
                PreventDefaultAction();

                if (Unit* caster = GetTarget())
                {
                    int32 damage = aurEff->GetAmount();
                    damage += aurEff->GetTickNumber() % 2 == 0 ? 2500 : 0;
                    const_cast<AuraEffect*>(aurEff)->SetAmount(damage);
                    caster->CastCustomSpell(caster, 74817, &damage, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_inferno_baron_geddon_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_inferno_baron_geddon_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_inferno_baron_geddon_AuraScript();
        }
};

class spell_weakening_baron_geddon : public SpellScriptLoader
{
    public:
        spell_weakening_baron_geddon() : SpellScriptLoader("spell_weakening_baron_geddon") { }

    private:
        class spell_weakening_baron_geddon_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_weakening_baron_geddon_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetTarget();

                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (!(target && caster))
                    return;

                caster->CastSpell(target, 82924, false);
            }

            void OnPeriodic(AuraEffect const* aurEff)
            {
                PreventDefaultAction();

                if (Unit * const caster = GetCaster())
                {
                    caster->CastSpell(caster, 75193, true);
                    // TODO: Check spell_dbc why custom spell doesn't work
                    if (auto player = caster->ToPlayer())
                        player->KilledMonsterCredit(40334);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_weakening_baron_geddon_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_weakening_baron_geddon_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_weakening_baron_geddon_AuraScript();
        }
};

// Spell Feed Spawn Of Smolderos for Quest: Walking The Dog (25294)

enum eWalkingTheDogEntries
{
    SPELL_FEED_SPAWN_OF_SMOLDEROS   =   74142,
    NPC_SMOLDEROS                   =   39659,
    QUEST_WALKING_THE_DOG           =   25294,
    ZONE_DARKWHISPER_GORGE          =   4991,
    QUEST_CREDIT                    =   39673

};


class spell_feed_spawn_of_smolderos : public SpellScriptLoader
{
public:
    spell_feed_spawn_of_smolderos() : SpellScriptLoader("spell_feed_spawn_of_smolderos") { }

    class spell_feed_spawn_of_smolderos_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_feed_spawn_of_smolderos_SpellScript)

        bool Validate(SpellInfo const * /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_FEED_SPAWN_OF_SMOLDEROS))
                return false;
            return true;
        }

        SpellCastResult CheckIfInZone()
        {
            Unit* caster = GetCaster();

            if (caster && caster->GetAreaId()==ZONE_DARKWHISPER_GORGE && SearchForSmolderos(caster))
                return SPELL_CAST_OK;
            else
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
                if (Player* player = caster->ToPlayer())
                {
                    player->KilledMonsterCredit(QUEST_CREDIT, 0);
                }
        }

        bool SearchForSmolderos(Unit* caster)
        {
            std::list<Creature*> lSmolderos;
            caster->GetCreatureListWithEntryInGrid(lSmolderos,NPC_SMOLDEROS,10.0f);

            if (lSmolderos.size()>0)
                return true;
            else
                return false;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_feed_spawn_of_smolderos_SpellScript::CheckIfInZone);
            OnEffectHit += SpellEffectFn(spell_feed_spawn_of_smolderos_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_feed_spawn_of_smolderos_SpellScript();
    }
};

enum ElementaryQuest
{
    SPELL_AURA_OF_FIRE          = 74287,
    SPELL_AURA_OF_EARTH         = 74288,
    SPELL_AURA_OF_AIR           = 74290,
    SPELL_TWILIGHT_EMPOWERMENT  = 74292,
    SPELL_ELEMENTAL_BLAST       = 74294,
    SPELL_RESET_AURAS           = 74303,

    NPC_CRUCIBLE_OF_FIRE        = 39730,
    NPC_CRUCIBLE_OF_EARTH       = 39737,
    NPC_CRUCIBLE_OF_AIR         = 39736,
    NPC_CRUCIBLE_OF_WATER       = 39738,
    NPC_THE_MANIPULATOR         = 39756,
    GO_PORTAL_VISUAL            = 202724,

    QUEST_ELEMENTARY            = 25303,

    CRUCIBLES_CNT               = 4
};

struct CrucibleData
{
    uint32 npc_entry;
    uint32 aura_id;
};

static const CrucibleData crucibleData[CRUCIBLES_CNT] =
{
    {NPC_CRUCIBLE_OF_FIRE, SPELL_AURA_OF_FIRE},
    {NPC_CRUCIBLE_OF_EARTH, SPELL_AURA_OF_EARTH},
    {NPC_CRUCIBLE_OF_AIR, SPELL_AURA_OF_AIR},
    {NPC_CRUCIBLE_OF_WATER, SPELL_TWILIGHT_EMPOWERMENT}
};

static const Position summonPos = {5002.502441f, -2033.615845f, 1273.269653f, 0.517997f};

class npc_elementary_crucible : public CreatureScript
{
    class DespawnPortalEvent : public BasicEvent
    {
    public:
        DespawnPortalEvent(Unit &owner, uint64 guid) : _owner(owner), _guid(guid) { }

        bool Execute(uint64 /*execTime*/, uint32 /*diff*/)
        {
            if (GameObject * go = ObjectAccessor::GetGameObject(_owner, _guid))
                go->Delete();
            return false;
        }

    private:
        Unit &_owner;
        uint64 _guid;
    };

public:

    void FailQuest(Player * player)
    {
        player->CastSpell(player, SPELL_ELEMENTAL_BLAST, true);
        player->FailQuest(QUEST_ELEMENTARY);
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_ELEMENTARY) != QUEST_STATUS_INCOMPLETE)
            return true;

        int8 idx = -1;
        uint32 entry = creature->GetEntry();

        for (uint8 i = 0; i < CRUCIBLES_CNT; ++i)
        {
            if  (crucibleData[i].npc_entry == entry)
            {
                idx = i;
                break;
            }
        }

        if (idx < 0)
            return true;

        for (uint8 i = 0; i < idx; ++i)
        {
            if (!player->HasAura(crucibleData[i].aura_id))
            {
                FailQuest(player);
                return true;
            }
        }

        if (player->HasAura(crucibleData[idx].aura_id))
        {
            FailQuest(player);
            return true;
        }
        else
            player->CastSpell(player, crucibleData[idx].aura_id, true);

        if (idx == 2) // Air
        {
            if (GameObject * go = creature->SummonGameObject(GO_PORTAL_VISUAL, summonPos.GetPositionX(), summonPos.GetPositionY(), summonPos.GetPositionZ(), summonPos.GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 10000))
                creature->m_Events.AddEvent(new DespawnPortalEvent(*creature, go->GetGUID()), creature->m_Events.CalculateTime(10000));

            if (Creature * creature = player->SummonCreature(NPC_THE_MANIPULATOR, summonPos, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000))
            {
                creature->AddThreat(player, 1.0f);
                creature->SetInCombatWith(player);
                player->SetInCombatWith(creature);
                creature->GetMotionMaster()->MoveChase(player, 10.0f);
            }
        }
        else if (idx == 3)
            player->CastSpell(player, SPELL_RESET_AURAS, true);

        return true;
    }

    npc_elementary_crucible() : CreatureScript("npc_elementary_crucible") { }
};

class spell_elementary_remove_auras : public SpellScriptLoader
{
    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl)

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit * victim  = GetHitUnit())
                if (uint32 spellId = (uint32) GetEffectValue())
                    victim->RemoveAurasDueToSpell(spellId);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleScript, EFFECT_1, SPELL_EFFECT_DUMMY);
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleScript, EFFECT_2, SPELL_EFFECT_DUMMY);
        }
    };

public:
    spell_elementary_remove_auras() : SpellScriptLoader("spell_elementary_remove_auras") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class npc_the_manipulator : public CreatureScript
{
    enum
    {
        SAY_AGGRO,
        SAY_DEATH,

        SPELL_FIREBALL          = 9053,
        SPELL_IMMOLATE          = 11962
    };

    struct npc_the_manipulatorAI : public CasterAI
    {
        npc_the_manipulatorAI(Creature * creature) : CasterAI(creature) { }

        void Reset()
        {
            fireballTimer = urand(5000, 6000);
            immolateTimer = urand(2000, 3000);
        }

        void EnterCombat(Unit * victim)
        {
            Talk(SAY_AGGRO, victim->GetGUID());
        }

        void JustDied(Unit *)
        {
            Talk(SAY_DEATH);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (!me->HasUnitState(UNIT_STATE_CASTING))
            {
                if (immolateTimer <= diff)
                {
                    DoCastVictim(SPELL_IMMOLATE);
                    immolateTimer = urand(10000, 15000);
                    return;
                }

                if (fireballTimer <= diff)
                {
                    DoCastVictim(SPELL_FIREBALL);
                    fireballTimer = 3000;
                    return;
                }
            }

            if (fireballTimer > diff)
                fireballTimer -= diff;

            if (immolateTimer > diff)
                immolateTimer -= diff;
        }
    private:
        uint32 fireballTimer;
        uint32 immolateTimer;
    };
public:
    npc_the_manipulator() : CreatureScript("npc_the_manipulator") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_the_manipulatorAI(creature);
    }
};

void AddSC_mount_hyjal()
{
    new npc_marion_wormwing();
    new AreaTrigger_at_nemesis_crystal();
    new npc_child_of_tortolla();
    new spell_inferno_baron_geddon();
    new spell_weakening_baron_geddon();
    new spell_feed_spawn_of_smolderos();
    new npc_elementary_crucible();
    new spell_elementary_remove_auras();
    new npc_the_manipulator();
}
