#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"

enum eBonobosSpells
{
    SPELL_GOING_BANANAS     = 125363,
    SPELL_BANANARANG        = 125311,
    SPELL_TOSS_FILTH        = 125365,
};

enum eBonobosEvents
{
    EVENT_GOING_BANANAS         = 1,
    EVENT_BANANARANG            = 2,
    EVENT_TOSS_FILTH            = 3,
};

class mob_bonobos : public CreatureScript
{
    public:
        mob_bonobos() : CreatureScript("mob_bonobos")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_bonobosAI(creature);
        }

        struct mob_bonobosAI : public ScriptedAI
        {
            mob_bonobosAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_GOING_BANANAS,       12000);
                events.ScheduleEvent(EVENT_BANANARANG,           8000);
                events.ScheduleEvent(EVENT_TOSS_FILTH,          15000);
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
                        case EVENT_GOING_BANANAS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_GOING_BANANAS, false);
                            events.ScheduleEvent(EVENT_GOING_BANANAS,      10000);
                            break;
                        case EVENT_BANANARANG:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_BANANARANG, false);
                            events.ScheduleEvent(EVENT_BANANARANG, 20000);
                            break;
                        case EVENT_TOSS_FILTH:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_TOSS_FILTH, false);
                            events.ScheduleEvent(EVENT_TOSS_FILTH, 15000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum eSeleNaSpells
{
    SPELL_RAIN_DANCE    = 124860,
    SPELL_TORRENT       = 124935,
    SPELL_WATER_BOLT    = 124854,
};

enum eSeleNaEvents
{
    EVENT_RAIN_DANCE        = 1,
    EVENT_TORRENT           = 2,
    EVENT_WATER_BOLT        = 3,
};

class mob_sele_na : public CreatureScript
{
    public:
        mob_sele_na() : CreatureScript("mob_sele_na")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_sele_naAI(creature);
        }

        struct mob_sele_naAI : public ScriptedAI
        {
            mob_sele_naAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_RAIN_DANCE,       5000);
                events.ScheduleEvent(EVENT_TORRENT,         15000);
                events.ScheduleEvent(EVENT_WATER_BOLT,      25000);
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

enum eBlackhoofSpells
{
    SPELL_BELLOWING_RAGE    = 124297,
    SPELL_RUSHING_RAGE      = 124302,
    SPELL_YAUNGOL_STOMP     = 124289,
};

enum eBlackhoofEvents
{
    EVENT_BELLOWING_RAGE        = 1,
    EVENT_RUSHING_RAGE          = 2,
    EVENT_YAUNGOL_STOMP         = 3,
};

class mob_blackhoof : public CreatureScript
{
    public:
        mob_blackhoof() : CreatureScript("mob_blackhoof")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_blackhoofAI(creature);
        }

        struct mob_blackhoofAI : public ScriptedAI
        {
            mob_blackhoofAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_RUSHING_RAGE,         5000);
                events.ScheduleEvent(EVENT_YAUNGOL_STOMP,       15000);
                events.ScheduleEvent(EVENT_BELLOWING_RAGE,      25000);
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
                        case EVENT_BELLOWING_RAGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_BELLOWING_RAGE, false);
                            events.ScheduleEvent(EVENT_BELLOWING_RAGE,       25000);
                            break;
                        case EVENT_RUSHING_RAGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_RUSHING_RAGE, false);
                            events.ScheduleEvent(EVENT_RUSHING_RAGE, 5000);
                            break;
                        case EVENT_YAUNGOL_STOMP:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_YAUNGOL_STOMP, false);
                            events.ScheduleEvent(EVENT_YAUNGOL_STOMP, 15000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum eIkThikWarriorSpells
{
    SPELL_PIERCE_ARMOR      = 6016,
    SPELL_SHOCK_AND_AWE        = 118538,
};

enum eIkThikWarriorEvents
{
    EVENT_PIERCE_ARMOR          = 1,
    EVENT_SHOCK_AND_AWE            = 2,
};

class mob_ik_thik_warrior : public CreatureScript
{
    public:
        mob_ik_thik_warrior() : CreatureScript("mob_ik_thik_warrior") { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_ik_thik_warriorAI(creature);
        }

        struct mob_ik_thik_warriorAI : public ScriptedAI
        {
            mob_ik_thik_warriorAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_PIERCE_ARMOR,         5000);
                events.ScheduleEvent(EVENT_SHOCK_AND_AWE,        15000);
            }

            void JustDied(Unit* /*killer*/) { }

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
                        case EVENT_PIERCE_ARMOR:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_PIERCE_ARMOR, false);
                            events.ScheduleEvent(EVENT_PIERCE_ARMOR,       25000);
                            break;
                        case EVENT_SHOCK_AND_AWE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_SHOCK_AND_AWE, false);
                            events.ScheduleEvent(EVENT_SHOCK_AND_AWE, 40000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_hop_hunting_q : public CreatureScript
{
public:
    npc_hop_hunting_q() : CreatureScript("npc_hop_hunting_q") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (creature->IsVendor())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        if (player->GetQuestStatus(30053) == QUEST_STATUS_INCOMPLETE)
        {
            std::string gossip = "";
            switch (creature->GetEntry())
            {
            case 62377:
                gossip = "I'm helping a friend brew some beer, and we need hops. Do you have any to spare?";
                break;
            case 62385:
                gossip = "Do you have any hops you can spare?";
                break;
            case 57385:
                gossip = "Can I buy some hops from you?";
                break;
            }
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        }

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            creature->AI()->Talk(0, player->GetGUID());
            player->KilledMonsterCredit(creature->GetEntry());
            player->CLOSE_GOSSIP_MENU();
        }
        else if (action == GOSSIP_ACTION_TRADE)
            player->GetSession()->SendListInventory(creature->GetGUID());

        return false;
    }
};

// Muddy Water quest
class spell_gen_gather_muddy_water : public SpellScriptLoader
{
public:
    spell_gen_gather_muddy_water() : SpellScriptLoader("spell_gen_gather_muddy_water") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            auto target = GetHitUnit();
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return;

            auto player = GetCaster()->ToPlayer();
            if (player->GetQuestStatus(29951) == QUEST_STATUS_INCOMPLETE)
            {
                if (!player->HasAura(106284))
                {
                    player->MonsterTextEmote("Mudmug's vial will slowly spill water while you are moving. Plan your path carefully!", player->GetGUID() , true);
                    player->CastSpell(player, 106284, true);
                }
                else
                    player->CastSpell(player, 106294, true);

                if (player->GetPower(POWER_ALTERNATE_POWER) == 100)
                {
                    player->AddItem(76356, 1);
                    player->RemoveAurasDueToSpell(106284);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class spell_gen_gather_muddy_water_aura : public SpellScriptLoader
{
public:
    spell_gen_gather_muddy_water_aura() : SpellScriptLoader("spell_gen_gather_muddy_water_aura") { }

    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void HandlePeriodic(AuraEffect const * /*aurEff*/)
        {
            if (!GetTarget()->isMoving())
                PreventDefaultAction();
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Crouching Carrot, Hidden Turnip quest
class npc_orange_painted_turnip : public CreatureScript
{
public:
    npc_orange_painted_turnip() : CreatureScript("npc_orange_painted_turnip") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_orange_painted_turnipAI(creature);
    }

    struct npc_orange_painted_turnipAI : public ScriptedAI
    {
        npc_orange_painted_turnipAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->SetReactState(REACT_PASSIVE);
            creature->setFaction(35);
        }

        uint32 timer;

        void Reset()
        {
            timer = 8000;
            std::list<Creature*> clist;
            GetCreatureListWithEntryInGrid(clist, me, 56538, 10.f);
            for (auto c : clist)
            {
                if (!me->IsWithinLOSInMap(c))
                    continue;

                float x, y, z;
                me->GetClosePoint(x, y, z, 0.f, 0.f, c->GetAngle(me));
                c->AI()->Talk(0);
                c->GetMotionMaster()->MoveIdle();
                c->GetMotionMaster()->MovePoint(1, x, y, z);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (timer <= diff)
            {
                std::list<Creature*> clist;
                GetCreatureListWithEntryInGrid(clist, me, 56538, 10.f);
                for (auto c : clist)
                {
                    if (!me->IsWithinLOSInMap(c))
                        continue;

                    if (auto player = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                        player->KilledMonsterCredit(56544);

                    c->AI()->Talk(1);
                    c->GetMotionMaster()->MoveFleeing(me, 4000);
                    c->ForcedDespawn(4000);
                }
                me->ForcedDespawn(5000);
                timer = 10000;
            }
            else
                timer -= diff;
        }
    };
};

void AddSC_valley_of_the_four_winds()
{
    // Rare Mobs
    new mob_bonobos();
    new mob_sele_na();
    new mob_blackhoof();

    // Standard Mobs
    new mob_ik_thik_warrior();

    // Quests
    new npc_hop_hunting_q();
    new spell_gen_gather_muddy_water();
    new spell_gen_gather_muddy_water_aura();
    new npc_orange_painted_turnip();
}
