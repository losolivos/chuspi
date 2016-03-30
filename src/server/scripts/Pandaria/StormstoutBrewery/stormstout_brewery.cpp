/*
    Dungeon : Stormstout Brewery 85-87
    Instance General Script
*/

#include "stormstout_brewery.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"

enum eHabaneroBeer
{
    NPC_BARREL              = 56731,

    SPELL_PROC_EXPLOSION    = 106787
};

static const Position aPartyWps[]=
{
    { -766.2f, 1390.8f, 146.7f, 1.8f },
    { -775.55f, 1422.47f, 139.5f, 3.4f },
    { -801.81f, 1415.04f, 139.7f, 3.38f },
    { -814.9f, 1411.7f, 134.6f, 3.38f },
    { -811.6f, 1399.9f, 132.3f, 4.98f },
    { -804.4f, 1383.6f, 126.7f, 5.9f },
    { -783.2f, 1383.4f, 126.7f, 4.9f }
};

class spell_stormstout_brewery_habanero_beer : public SpellScriptLoader
{
    public:
        spell_stormstout_brewery_habanero_beer() : SpellScriptLoader("spell_stormstout_brewery_habanero_beer") { }

        class spell_stormstout_brewery_habanero_beer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_stormstout_brewery_habanero_beer_SpellScript);

            void HandleInstaKill(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster())
                    return;

                std::list<Creature*> creatureList;

                GetCreatureListWithEntryInGrid(creatureList, GetCaster(), NPC_BARREL, 10.0f);

                GetCaster()->RemoveAurasDueToSpell(SPELL_PROC_EXPLOSION);

                for (auto barrel : creatureList)
                {
                    if (barrel->HasAura(SPELL_PROC_EXPLOSION))
                    {
                        barrel->RemoveAurasDueToSpell(SPELL_PROC_EXPLOSION);
                        barrel->CastSpell(barrel, GetSpellInfo()->Id, true);
                    }
                }
            }

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->ToCreature())
                        caster->ToCreature()->ForcedDespawn(1000);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_stormstout_brewery_habanero_beer_SpellScript::HandleInstaKill, EFFECT_1, SPELL_EFFECT_INSTAKILL);
                AfterCast += SpellCastFn(spell_stormstout_brewery_habanero_beer_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_stormstout_brewery_habanero_beer_SpellScript();
        }
};

class PlayerPredicate
{
public:
    bool operator()(WorldObject* target) const
    {
        return target && target->GetTypeId() == TYPEID_PLAYER;
    }
};

class spell_spicy_explosion : public SpellScriptLoader
{
public:
    spell_spicy_explosion() : SpellScriptLoader("spell_spicy_explosion") {}

    class spell_spicy_explosion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_spicy_explosion_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(PlayerPredicate());
        }

        void OnAfterCast()
        {
            if (GetCaster())
                GetCaster()->AddObjectToRemoveList();
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_spicy_explosion_SpellScript::OnAfterCast);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_spicy_explosion_SpellScript::SelectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_spicy_explosion_SpellScript();
    }
};

class npc_chen_stormstout : public CreatureScript
{
public:
    npc_chen_stormstout() : CreatureScript("npc_chen_stormstout") { }

    enum eTalks : uint32
    {
        TALK_NOT_ZAN,
        TALK_HAPPENED,
        TALK_WHERE,
        TALK_ABANDONED,
        TALK_GAO,
        TALK_COOKIES,
        TALK_GHOST
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_TALK_1,
        EVENT_TALK_2,
        EVENT_TALK_3,
        EVENT_TALK_4,
        EVENT_TALK_5,
        EVENT_TALK_6,
        EVENT_TALK_7,
    };

    struct npc_chen_stormstoutAI : public ScriptedAI
    {
        npc_chen_stormstoutAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        void GetAuntieAndDoAction(int8 action)
        {
            Creature* pAuntie = GetClosestCreatureWithEntry(me, NPC_AUNTIE_STORMSTOUT, 12.f);

            if (pAuntie)
            {
                if (pAuntie->AI())
                    pAuntie->AI()->DoAction(action);
            }
        }

        void DoAction(const int32 iAction)
        {
            events.Reset();

            switch (iAction)
            {
            case 0:
                events.ScheduleEvent(EVENT_TALK_1, 8000);
                break;
            case 1:
                events.ScheduleEvent(EVENT_TALK_2, 6000);
                break;
            case 2:
                events.ScheduleEvent(EVENT_TALK_3, 7400);
                break;
            case 3:
                events.ScheduleEvent(EVENT_TALK_4, 7000);
                break;
            case 4:
                events.ScheduleEvent(EVENT_TALK_5, 13000);
                break;
            case 5:
                events.ScheduleEvent(EVENT_TALK_6, 4000);
                break;
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_TALK_1:
                    Talk(TALK_NOT_ZAN);
                    GetAuntieAndDoAction(1);
                    break;
                case EVENT_TALK_2:
                    Talk(TALK_HAPPENED);
                    GetAuntieAndDoAction(2);
                    break;
                case EVENT_TALK_3:
                    Talk(TALK_WHERE);
                    GetAuntieAndDoAction(3);
                    break;
                case EVENT_TALK_4:
                    Talk(TALK_ABANDONED);
                    GetAuntieAndDoAction(4);
                    break;
                case EVENT_TALK_5:
                    Talk(TALK_GAO);
                    GetAuntieAndDoAction(5);
                    break;
                case EVENT_TALK_6:
                    Talk(TALK_COOKIES);
                    events.ScheduleEvent(EVENT_TALK_7, 7000);
                    break;
                case EVENT_TALK_7:
                    Talk(TALK_GHOST);
                    break;
                }
            }

        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_chen_stormstoutAI(pCreature);
    }
};


class npc_auntie_stormstout : public CreatureScript
{
public:
    npc_auntie_stormstout() : CreatureScript("npc_auntie_stormstout") { }

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_TALK_1,
        EVENT_TALK_2,
        EVENT_TALK_3,
        EVENT_TALK_4,
        EVENT_TALK_5,
        EVENT_TALK_6
    };

    enum eTalks : uint32
    {
        TALK_HELLO,
        TALK_ZAN,
        TALK_DAY,
        TALK_SIZE,
        TALK_ABANDONED,
        TALK_COOKIES
    };

    struct npc_auntie_stormstoutAI : public ScriptedAI
    {
        npc_auntie_stormstoutAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_bEventStarted = false;
        }

        bool m_bEventStarted;

        void GetChenAndDoAction(int8 action)
        {
            Creature* pChen = GetClosestCreatureWithEntry(me, NPC_CHEN_STORMSTOUT, 12.f);

            if (pChen)
            {
                if (pChen->AI())
                    pChen->AI()->DoAction(action);
            }
        }

        void MoveInLineOfSight(Unit* pWho)
        {
            if (!m_bEventStarted && pWho && pWho->ToPlayer())
            {
                HandleEventStart();
                m_bEventStarted = true;
            }
        }

        void HandleEventStart()
        {
            if (m_bEventStarted)
                return;

            events.ScheduleEvent(EVENT_TALK_1, 5000);
        }

        void DoAction(const int32 iAction)
        {
            events.Reset();

            switch (iAction)
            {
            case 1:
                events.ScheduleEvent(EVENT_TALK_2, 6000);
                break;
            case 2:
                events.ScheduleEvent(EVENT_TALK_3, 4500);
                break;        
            case 3:
                events.ScheduleEvent(EVENT_TALK_4, 7000);
                break;
            case 4:
                events.ScheduleEvent(EVENT_TALK_5, 6500);
                break;
            case 5:
                events.ScheduleEvent(EVENT_TALK_6, 5000);
                break;
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_TALK_1:
                    Talk(TALK_HELLO);
                    GetChenAndDoAction(0);
                    break;
                case EVENT_TALK_2:
                    Talk(TALK_ZAN);
                    GetChenAndDoAction(1);
                    break;
                case EVENT_TALK_3:
                    Talk(TALK_DAY);
                    GetChenAndDoAction(2);
                    break;
                case EVENT_TALK_4:
                    Talk(TALK_SIZE);
                    GetChenAndDoAction(3);
                    break;
                case EVENT_TALK_5:
                    Talk(TALK_ABANDONED);
                    GetChenAndDoAction(4);
                    break;
                case EVENT_TALK_6:
                    Talk(TALK_COOKIES);
                    GetChenAndDoAction(5);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_auntie_stormstoutAI(pCreature);
    }
};


class npc_aqua_dancer : public CreatureScript
{
public:
    npc_aqua_dancer() : CreatureScript("npc_aqua_dancer") { }

    enum eSpells : uint32
    {
        SPELL_SPLASH                = 107030,
        SPELL_AQUATIC_ILLUSION      = 107044
    };

    enum eTalks : uint32
    {
        TALK_DEATH
    };

    struct npc_aqua_dancerAI : public ScriptedAI
    {
        npc_aqua_dancerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            Initialize();
        }

        uint64 m_uiHozenGuid;

        void Reset()
        {}

        void Initialize()
        {
            me->SetUnitMovementFlags(MOVEMENTFLAG_DISABLE_GRAVITY);

            Creature* pHozen = GetClosestCreatureWithEntry(me, NPC_SODDEN_HOZEN_BRAWLER, 30.f);

            if (pHozen)
            {
                DoCast(pHozen, SPELL_AQUATIC_ILLUSION, true);
                m_uiHozenGuid = pHozen->GetGUID();
            }
        }

        void DamageTaken(Unit* pDealer, uint32 &uiDamage)
        {
            uiDamage = me->GetHealth();
        }

        void JustDied(Unit* pKiller)
        {
            DoCast(me, SPELL_SPLASH, true);

            if (Creature* pHozen = ObjectAccessor::GetCreature(*me, m_uiHozenGuid))
            {
                Talk(TALK_DEATH);

                me->DealDamage(pHozen, (uint32)pHozen->GetMaxHealth() / 2, 0, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FROST);

                if (pHozen->AI())
                    pHozen->AI()->DoZoneInCombat();
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_aqua_dancerAI(pCreature);
    }
};

class npc_fiery_trickster : public CreatureScript
{
public:
    npc_fiery_trickster() : CreatureScript("npc_fiery_trickster") {}

    enum eSpells : uint32
    {
        SPELL_FIERY_ILLUSION        = 107175,
        SPELL_BLAZING_SPARK         = 107071
    };

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_INIT,
        EVENT_FIRE_SPARK
    };

    enum eTalks : uint32
    {
        TALK_DEATH
    };

    struct npc_fiery_tricksterAI : public ScriptedAI
    {
        npc_fiery_tricksterAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetUnitMovementFlags(MOVEMENTFLAG_DISABLE_GRAVITY);
            events.ScheduleEvent(EVENT_INIT, urand(400, 800));
            events.ScheduleEvent(EVENT_FIRE_SPARK, urand(4000, 9000));
        }

        uint64 m_uiHozenGuid;

        void Initialize()
        {
            Creature* pHozen = GetClosestCreatureWithEntry(me, NPC_INFLAMED_HOZEN_BRAWLER, 39.f);

            if (pHozen && !pHozen->HasAura(SPELL_FIERY_ILLUSION))
            {
                DoCast(pHozen, SPELL_FIERY_ILLUSION, true);
                m_uiHozenGuid = pHozen->GetGUID();
            }
        }

        void DamageTaken(Unit* pDealer, uint32 &uiDamage)
        {
            uiDamage = me->GetHealth();
        }

        void JustDied(Unit* pKiller)
        {
            if (Creature* pHozen = ObjectAccessor::GetCreature(*me, m_uiHozenGuid))
            {
                Talk(TALK_DEATH);
                me->DealDamage(pHozen, (uint32)pHozen->GetMaxHealth() / 2, 0, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE);

                if (pHozen->AI())
                    pHozen->AI()->DoZoneInCombat();
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_INIT:
                    Initialize();
                    break;
                case EVENT_FIRE_SPARK:
                    DoCast(me, SPELL_BLAZING_SPARK, true);
                    events.ScheduleEvent(EVENT_FIRE_SPARK, urand(6000, 11000));
                    break;
                }
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_fiery_tricksterAI(pCreature);
    }
};

static const uint32 auiEmotes[] =
{
    EMOTE_ONESHOT_TALK,
    EMOTE_ONESHOT_CHEER,
    EMOTE_ONESHOT_EAT,
    EMOTE_ONESHOT_LAUGH,
};

class npc_hozen_party_animal : public CreatureScript
{
public:
    npc_hozen_party_animal() : CreatureScript("npc_hozen_party_animal") {}

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_MOVE
    };

    struct npc_hozen_party_animalAI : public ScriptedAI
    {
        npc_hozen_party_animalAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_bCanAggroMore = true;
            me->HandleEmoteCommand(auiEmotes[(urand(0, 3))]);
        }

        uint32 m_uiWayPoint;
        bool m_bCanAggroMore;

        void Initialize()
        {
            me->CombatStop(true);
            me->AttackStop();
            me->SetReactState(REACT_PASSIVE);

            me->GetMotionMaster()->Clear(false);

            m_uiWayPoint = (me->GetDistance(aPartyWps[0]) > me->GetDistance(aPartyWps[1])) ? 1 : 0;

            events.ScheduleEvent(EVENT_MOVE, 100);
        }

        void DoAction(const int32 iAction)
        {
            if (iAction == 0)
                Initialize();
            else if (iAction == 1)
                m_bCanAggroMore = false;
        }

        void Move()
        {
            me->GetMotionMaster()->MovePoint(m_uiWayPoint, aPartyWps[m_uiWayPoint]);
        }

        void EnterCombat(Unit* pWho)
        {
            std::list<Creature*> temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 15.f);

            if (m_bCanAggroMore)
            {
                for (auto pCreature : temp)
                {
                    if (pCreature->AI() && pCreature->IsAlive() && !pCreature->IsInCombat())
                    {
                        pCreature->AI()->DoAction(1);
                        pCreature->AI()->DoZoneInCombat();
                    }
                }
            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            switch (uiPointId)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                events.ScheduleEvent(EVENT_MOVE, 100);
                break;
            case 6:
                me->SetVisible(false);
                break;
            }

            ++m_uiWayPoint;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MOVE:
                    Move();
                    break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_hozen_party_animalAI(pCreature);
    }
};

class npc_hozen_bouncer : public CreatureScript
{
public:
    npc_hozen_bouncer() : CreatureScript("npc_hozen_bouncer")
    {}

    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_TALK
    };

    enum eTalks : uint32
    {
        TALK_DOWN,
        TALK_MEAN,
        TALK_NEW,
        TALK_PARTY
    };

    enum eSpells : uint32
    {
        SPELL_BOUNCE    = 107019
    };

    struct npc_hozen_bouncerAI : public ScriptedAI
    {
        npc_hozen_bouncerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            if (me->GetInstanceScript())
            {
                m_bBouncing = me->GetInstanceScript()->GetData(DATA_OOK_OOK) == DONE;

                if (m_bBouncing)
                    me->AddAura(SPELL_BOUNCE, me);
            }

            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }

        uint32 m_uiTalk;
        bool m_bBouncing;

        void DoAction(const int32 iAction)
        {
            m_uiTalk = iAction;
            m_bBouncing = false;
            me->RemoveAllAuras();
            
            float x, y;

            GetPositionWithDistInOrientation(me, 18.f, me->GetOrientation(), x, y);
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(0, x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ()));
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            me->SetFacingTo(1.83f);
            events.ScheduleEvent(EVENT_TALK, 2000*(m_uiTalk+1)*2);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_TALK:
                    Talk(m_uiTalk);
                    if (m_uiTalk < 2)
                        events.ScheduleEvent(EVENT_TALK, 5000);
                    m_uiTalk += 2;
                    break;
                }
                
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_hozen_bouncerAI(pCreature);
    }
};

class npc_controlled_hozen : public CreatureScript
{
public:
    npc_controlled_hozen() : CreatureScript("npc_controlled_hozen") {}

    struct npc_controlled_hozenAI : public ScriptedAI
    {
        npc_controlled_hozenAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        void EnterEvadeMode()
        {
            me->CombatStop(true);
            me->AttackStop();
            me->RemoveAllAurasExceptType(SPELL_AURA_PROC_TRIGGER_SPELL);
            me->GetMotionMaster()->MoveTargetedHome();
        }

        void EnterCombat(Unit* pWho)
        {
            std::list<Creature*> temp;
            GetCreatureListWithEntryInGrid(temp, me, me->GetEntry(), 60.f);

            for (auto pCreature : temp)
            {
                if (pCreature->AI() && pCreature->IsAlive() && !pCreature->IsInCombat())
                    pCreature->AI()->DoZoneInCombat();
            }
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_controlled_hozenAI(pCreature);
    }
};

void AddSC_stormstout_brewery()
{
    new spell_stormstout_brewery_habanero_beer();
    new spell_spicy_explosion();
    new npc_chen_stormstout();
    new npc_auntie_stormstout();
    new npc_aqua_dancer();
    new npc_fiery_trickster();
    new npc_hozen_party_animal();
    new npc_hozen_bouncer();
    new npc_controlled_hozen();
}