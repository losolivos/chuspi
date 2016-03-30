#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "SpellScript.h"

enum eKahTirSpells
{
    SPELL_DEVASTATING_ARC       = 124946,
    SPELL_SUMMON_QUILEN         = 124980,
    SPELL_TITANIC_STRENGTH      = 124976,
};

enum eKahTirEvents
{
    EVENT_DEVASTATING_ARC       = 1,
    EVENT_SUMMON_QUILEN         = 2,
    EVENT_TITANIC_STRENGTH      = 3,
};

class mob_kah_tir : public CreatureScript
{
    public:
        mob_kah_tir() : CreatureScript("mob_kah_tir")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_kah_tirAI(creature);
        }

        struct mob_kah_tirAI : public ScriptedAI
        {
            mob_kah_tirAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_DEVASTATING_ARC, 40000);
                events.ScheduleEvent(EVENT_SUMMON_QUILEN, 12000);
                events.ScheduleEvent(EVENT_TITANIC_STRENGTH, 20000);
            }

            void JustDied(Unit* /*killer*/)
            {
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DEVASTATING_ARC:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_DEVASTATING_ARC, false);
                            events.ScheduleEvent(EVENT_DEVASTATING_ARC,      60000);
                            break;
                        case EVENT_SUMMON_QUILEN:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_SUMMON_QUILEN, false);
                            events.ScheduleEvent(EVENT_SUMMON_QUILEN, 50000);
                            break;
                        case EVENT_TITANIC_STRENGTH:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_TITANIC_STRENGTH, false);
                            events.ScheduleEvent(EVENT_TITANIC_STRENGTH,      30000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum eLithIkSpells
{
    SPELL_BLADE_FURY       = 125370,
    SPELL_TORNADO          = 125398,
    SPELL_TORNADO_DMG      = 131693,
    SPELL_WINDSONG         = 125373,
};

enum eLithIkEvents
{
    EVENT_BLADE_FURY       = 1,
    EVENT_TORNADO          = 2,
    EVENT_WINDSONG         = 3,
};

class mob_lith_ik : public CreatureScript
{
    public:
        mob_lith_ik() : CreatureScript("mob_lith_ik")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_lith_ikAI(creature);
        }

        struct mob_lith_ikAI : public ScriptedAI
        {
            mob_lith_ikAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_TORNADO,       5000);
                events.ScheduleEvent(EVENT_BLADE_FURY,   25000);
                events.ScheduleEvent(EVENT_WINDSONG,     30000);
            }

            void JustSummoned(Creature* summon)
            {
                if (summon->GetEntry() == 64267)
                {
                    summon->DespawnOrUnsummon(15000);
                    summon->AddAura(SPELL_TORNADO_DMG, summon);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->GetMotionMaster()->MoveRandom(20.0f);
                }

            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TORNADO:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_TORNADO, false);
                            events.ScheduleEvent(EVENT_TORNADO,      70000);
                            break;
                        case EVENT_BLADE_FURY:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_BLADE_FURY, false);
                            events.ScheduleEvent(EVENT_BLADE_FURY,      30000);
                            break;
                        case EVENT_WINDSONG:
                            me->CastSpell(me, SPELL_WINDSONG, false);
                            events.ScheduleEvent(EVENT_WINDSONG,      25000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum eDarkwoodsFaerieSpells
{
    SPELL_DISGUISE         = 121308,
    SPELL_FAE_SPIRIT       = 122567,
    SPELL_NIGHT_SKY        = 123318,
    SPELL_STARSURGE        = 123330,
};

enum eDarkwoodsFaerieEvents
{
    EVENT_DISGUISE          = 1,
    EVENT_FAE_SPIRIT        = 2,
    EVENT_NIGHT_SKY         = 3,
    EVENT_STARSURGE         = 4,
};

class mob_darkwoods_faerie : public CreatureScript
{
    public:
        mob_darkwoods_faerie() : CreatureScript("mob_darkwoods_faerie")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_darkwoods_faerieAI(creature);
        }

        struct mob_darkwoods_faerieAI : public ScriptedAI
        {
            mob_darkwoods_faerieAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_DISGUISE,       5000);
                events.ScheduleEvent(EVENT_FAE_SPIRIT,    15000);
                events.ScheduleEvent(EVENT_NIGHT_SKY,     22000);
                events.ScheduleEvent(EVENT_STARSURGE,     30000);
            }

            void JustSummoned(Creature* summon)
            {
                if (summon->GetEntry() == 64267)
                {
                    summon->DespawnOrUnsummon(15000);
                    summon->AddAura(SPELL_TORNADO_DMG, summon);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->GetMotionMaster()->MoveRandom(20.0f);
                }

            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DISGUISE:
                            if (SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(me, SPELL_DISGUISE, false);
                            events.ScheduleEvent(EVENT_DISGUISE,      70000);
                            break;
                        case EVENT_FAE_SPIRIT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_FAE_SPIRIT, false);
                            events.ScheduleEvent(EVENT_FAE_SPIRIT,      15000);
                            break;
                        case EVENT_NIGHT_SKY:
                            me->CastSpell(me, SPELL_NIGHT_SKY, false);
                            events.ScheduleEvent(EVENT_NIGHT_SKY,      22000);
                            break;
                        case EVENT_STARSURGE:
                            me->CastSpell(me, SPELL_STARSURGE, false);
                            events.ScheduleEvent(EVENT_STARSURGE,      30000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum eHeiFengSpells
{
    SPELL_DEEP_BREATH          = 125030,
    SPELL_SERPENT_SWEEP        = 125063,
    SPELL_SHADOW_DETONATION    = 124956,
};

enum eHeiFengEvents
{
    EVENT_DEEP_BREATH          = 1,
    EVENT_SERPENT_SWEEP        = 2,
    EVENT_SHADOW_DETONATION    = 3,
};

class mob_hei_feng : public CreatureScript
{
    public:
        mob_hei_feng() : CreatureScript("mob_hei_feng")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_hei_fengAI(creature);
        }

        struct mob_hei_fengAI : public ScriptedAI
        {
            mob_hei_fengAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_DEEP_BREATH,       5000);
                events.ScheduleEvent(EVENT_SERPENT_SWEEP,    15000);
                events.ScheduleEvent(EVENT_SHADOW_DETONATION,     22000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DEEP_BREATH:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_DEEP_BREATH, false);
                            events.ScheduleEvent(EVENT_DEEP_BREATH,      30000);
                            break;
                        case EVENT_SERPENT_SWEEP:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_SERPENT_SWEEP, false);
                            events.ScheduleEvent(EVENT_SERPENT_SWEEP,      15000);
                            break;
                        case EVENT_SHADOW_DETONATION:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_SHADOW_DETONATION, false);
                            events.ScheduleEvent(EVENT_SHADOW_DETONATION,      22000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum eEshelonSpells
{
    SPELL_RAIN_DANCE    = 124860,
    SPELL_TORRENT       = 124935,
    SPELL_WATER_BOLT    = 124854
};

enum eEshelonEvents
{
    EVENT_RAIN_DANCE        = 1,
    EVENT_TORRENT           = 2,
    EVENT_WATER_BOLT        = 3
};

class mob_eshelon : public CreatureScript
{
    public:
        mob_eshelon() : CreatureScript("mob_eshelon")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_eshelonAI(creature);
        }

        struct mob_eshelonAI : public ScriptedAI
        {
            mob_eshelonAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_RAIN_DANCE,   5000);
                events.ScheduleEvent(EVENT_TORRENT,     15000);
                events.ScheduleEvent(EVENT_WATER_BOLT,  25000);
            }

            void JustDied(Unit* /*killer*/)
            {
            }

            void JustSummoned(Creature* summon)
            {
                summon->DespawnOrUnsummon(12000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);


                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RAIN_DANCE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_RAIN_DANCE, false);
                            events.ScheduleEvent(EVENT_RAIN_DANCE,       5000);
                            break;
                        case EVENT_TORRENT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_TORRENT, false);
                            events.ScheduleEvent(EVENT_TORRENT, 15000);
                            break;
                        case EVENT_WATER_BOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_WATER_BOLT, false);
                            events.ScheduleEvent(EVENT_WATER_BOLT, 25000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class go_sikthik_cage : public GameObjectScript
{
public:
    go_sikthik_cage() : GameObjectScript("go_sikthik_cage") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        // If counter is 7 (script is called before counting) max is 8
        if (player->GetQuestSlotCounter(player->FindQuestSlot(31688), 0) == 7 && player->GetQuestStatus(31688) == QUEST_STATUS_INCOMPLETE)
        {
            player->SummonCreature(65586, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
            player->KilledMonsterCredit(65586);
        }

        return false;
    }
};

// Back on Their Feet quest
class spell_item_cintron_infused_bandage : public SpellScriptLoader
{
public:
    spell_item_cintron_infused_bandage() : SpellScriptLoader("spell_item_cintron_infused_bandage") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (!GetCaster())
                return;

            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                if (auto target = GetTarget()->ToCreature())
                    if (target->GetEntry() == 61692)
                        if (auto player = GetCaster()->ToPlayer())
                        {
                            target->HandleEmoteCommand(EMOTE_STATE_STAND);
                            target->ForcedDespawn(4000);
                            player->KilledMonsterCredit(61692);
                        }
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(script_impl::OnRemove, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Dust to Dust quest
class spell_item_shado_pan_torch : public SpellScriptLoader
{
public:
    spell_item_shado_pan_torch() : SpellScriptLoader("spell_item_shado_pan_torch") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                if (auto target = GetTarget()->ToCreature())
                    if (target->GetEntry() == 60925)
                    {
                        target->RemoveAurasDueToSpell(106246);
                        target->ForcedDespawn();
                    }
        }

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            auto caster = GetCaster();
            auto target = GetTarget();
            if (!caster || !target)
                return;

            if (target->GetTypeId() == TYPEID_UNIT && target->GetEntry() == 60925 && !target->HasAura(106246))
            {
                if (auto player = caster->ToPlayer())
                    player->KilledMonsterCredit(target->GetEntry());
                target->CastSpell(target, 106246, true);
            }
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(script_impl::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            AfterEffectApply += AuraEffectApplyFn(script_impl::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// --------------------------------------------------------------
// Quest: Ranger Rescue (30774)
// --------------------------------------------------------------
enum RangerRescue
{
    QUEST_RANGER_RESCUE                     = 30774,
    QUEST_OBJECTIVE_LONGYIN_RANGER_RESCUED  = 263418,
    QUEST_OBJECTIVE_FREE_LIN_SILENTSTRIKE   = 263419,

    GO_DRYWOOD_CAGE                         = 211511,
    NPC_LONGYING_RANGER                     = 60730,
    NPC_LONGYING_RANGER_HELPER              = 60763,
    NPC_SUNA_SILENTSTRIKE                   = 60901,

    SPELL_SUMMON_LONGYING_RANGER            = 117670,

    EVENT_SUNA_KNEEL_TALK                   = 1,
    EVENT_SUNA_TALK                         = 2,
    EVENT_SUNA_TALK_2                       = 3,
    EVENT_SUNA_STAND                        = 4
};

class go_drywood_cage : public GameObjectScript
{
public:
    go_drywood_cage() : GameObjectScript("go_drywood_cage") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->GetQuestStatus(QUEST_RANGER_RESCUE) == QUEST_STATUS_INCOMPLETE
            && player->GetQuestObjectiveCounter(QUEST_OBJECTIVE_LONGYIN_RANGER_RESCUED) < 4)
        {
            if (auto ranger = GetClosestCreatureWithEntry(player, NPC_LONGYING_RANGER, 10.f))
            {
                go->SetGoState(GO_STATE_ACTIVE);

                player->QuestObjectiveSatisfy(QUEST_OBJECTIVE_LONGYIN_RANGER_RESCUED, 1);
                player->CastSpell(player, SPELL_SUMMON_LONGYING_RANGER, true);

                if (auto rangerHelper = GetClosestCreatureWithEntry(player, NPC_LONGYING_RANGER_HELPER, 10.f))
                    rangerHelper->AI()->Talk(0, player->GetGUID());

                ranger->DisappearAndDie();
            }
        }

        return true;
    }
};

class npc_longying_ranger : public CreatureScript
{
public:
    npc_longying_ranger() : CreatureScript("npc_longying_ranger") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_longying_ranger_AI(creature);
    }

    struct npc_longying_ranger_AI : public ScriptedAI
    {
        npc_longying_ranger_AI(Creature* creature) : ScriptedAI(creature) { }

        void JustRespawned()
        {
            if (GameObject* cage = GetClosestGameObjectWithEntry(me, GO_DRYWOOD_CAGE, 10.f))
                cage->SetGoState(GO_STATE_READY);
        }
    };
};

class npc_lin_silentstrike : public CreatureScript
{
public:
    npc_lin_silentstrike() : CreatureScript("npc_lin_silentstrike") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lin_silentstrikeAI(creature);
    }

    struct npc_lin_silentstrikeAI : public ScriptedAI
    {
        npc_lin_silentstrikeAI(Creature* creature) : ScriptedAI(creature) { }

        uint64 sunaGUID;

        void Reset()
        {
            sunaGUID = 0;

            events.Reset();
            events.ScheduleEvent(EVENT_SUNA_KNEEL_TALK, 5000);
            events.ScheduleEvent(EVENT_SUNA_TALK, 13000);
            events.ScheduleEvent(EVENT_SUNA_TALK_2, 21000);
            events.ScheduleEvent(EVENT_SUNA_STAND, 29000);
        }

        void SetGUID(uint64 guid, int32)
        {
            if (auto suna = me->SummonCreature(NPC_SUNA_SILENTSTRIKE, 2659.59f, 3268.618f, 425.33f, 5.56f, TEMPSUMMON_TIMED_DESPAWN, 34000))
            {
                sunaGUID = suna->GetGUID();

                suna->AI()->Talk(0);
                suna->GetMotionMaster()->MovePoint(1, 2674.21f, 3257.52f, 426.31f);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                Creature* suna = me->GetCreature(*me, sunaGUID);
                if (!suna)
                {
                    Reset();
                    return;
                }

                switch (eventId)
                {
                    case EVENT_SUNA_KNEEL_TALK:
                    {
                        suna->SetStandState(UNIT_STAND_STATE_KNEEL);
                        suna->AI()->Talk(1);
                        break;
                    }
                    case EVENT_SUNA_TALK:
                    {
                        suna->AI()->Talk(2);
                        break;
                    }
                    case EVENT_SUNA_TALK_2:
                    {
                        suna->AI()->Talk(3);
                        break;
                    }
                    case EVENT_SUNA_STAND:
                    {
                        suna->SetStandState(UNIT_STAND_STATE_STAND);
                        Reset();
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_RANGER_RESCUE) == QUEST_STATUS_INCOMPLETE
            && !player->GetQuestObjectiveCounter(QUEST_OBJECTIVE_FREE_LIN_SILENTSTRIKE))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Examine the body.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->QuestObjectiveSatisfy(QUEST_OBJECTIVE_FREE_LIN_SILENTSTRIKE, 1);
            creature->AI()->SetGUID(player->GetGUID(), 0);
            player->CLOSE_GOSSIP_MENU();
        }

        return false;
    }
};

void AddSC_townlong_steppes()
{
    //Rare mobs
    new mob_kah_tir();
    new mob_lith_ik();
    new mob_eshelon();
    //Elite mobs
    new mob_darkwoods_faerie();
    new mob_hei_feng();
    //Quests
    new go_sikthik_cage();
    new spell_item_cintron_infused_bandage();
    new spell_item_shado_pan_torch();
    new go_drywood_cage();
    new npc_longying_ranger();
    new npc_lin_silentstrike();
}
