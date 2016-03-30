
#include "throne_of_thunder.h"
#include <array>

enum eCreatures
{
    MOB_LIVING_SAND                         = 69153, // Summoned when Sandstorm hits a Quicksand
    MOB_BLESSED_LOA_SPIRIT                  = 69480, // Summoned by Mar'li, heals a councillor
    MOB_SHADOWED_LOA_SPIRIT                 = 69548, // Summoned by Mar'li, kills player
    MOB_TWISTED_FATE_FIRST                  = 69740, // First Twisted Fate npc to be summoned, will move toward the second and vice-versa
    MOB_TWISTED_FATE_SECOND                 = 69746, // Second Twisted Fate npc to be summoned
};

enum eSpells
{
    //===============================================
    // Gara'Jal's Soul
    SPELL_LINGERING_PRESENCE                = 136467, // When the spirit of Gara'jal leaves a councillor
    SPELL_POSSESSED                         = 136442, // When the spirit of Gara'jal merges with a councillor (periodic dummy ? Dark Energy maybe)
    SPELL_DARK_POWER                        = 136507, // Damages increase with each cast

    // Soul Fragment
    SPELL_SOUL_FRAGMENT_SELECTOR            = 137645, // Trigger 137641 (AREA_ENTRY ?)
    SPELL_SOUL_FRAGMENT_PERIODIC            = 137641, // Override spells with 137643
    SPELL_SOUL_FRAGMENT_SWITCHER            = 137643, // Trigger 137641
    SPELL_SOUL_FRAGMENT_DUMMY               = 137670, // Maybe visual
    SPELL_SHADOWED_SOUL                     = 137650, // Each time 137641 ticks

    //===============================================
    // Frost King Malakk

    // Frigid Assault
    SPELL_FRIGID_ASSAULT                    = 136904, // Apply trigger auras
    SPELL_FRIGID_ASSAULT_STACK              = 136903, // Stun owner when stacks reach 15
    SPELL_FRIGID_ASSAULT_DAMAGES            = 136911, // Damages
    SPELL_FRIGID_ASSAULT_STUN               = 136910, // When 136903 reaches 15 stacks

    // Biting Cold <=> Frostbite
    SPELL_BITING_COLD                       = 136917, // Main damages (select random target, need AuraScript or SpellScript)
    SPELL_BITING_COLD_PERIODIC_DAMAGES      = 136992, // Periodically trigger 136991
    SPELL_BITING_COLD_ALLY_DAMAGES          = 136991, // Periodic damages to allies
    SPELL_BITING_COLD_PERIODIC_DUMMY        = 137579, // Periodic trigger 137580
    SPELL_BITING_COLD_DUMMY                 = 137580, // Visual maybe ?
    // When Malakk has the possessed aura, Biting Cold turns into Frostbite
    SPELL_FROSTBITE                         = 136990, // Main damages (works like Biting Cold)
    SPELL_FROSTBITE_PERIODIC_DAMAGES        = 136922, // Periodic trigger damages to ally (amount must be multiplied to fit stack amount)
    SPELL_FROSTBITE_ALLY_DAMAGES            = 136937, // Damages to allies
    SPELL_FROSTBITE_SCREEN_EFFECT           = 136926, // Screen effect
    SPELL_FROSTBITE_DUMMY_AURA              = 137575, // Unknown
    // Is there something that can stack ?
    // Bodyheat triggers when Frostbite hits an ally in Heroic
    SPELL_BODY_HEAT                         = 137084, // Triggers 137085 on expire (AuraScript)
    SPELL_CHILLED_TO_THE_BONE               = 137085, // Prevents player from reducing Frostbite amount of nearby players (or only caster ?)

    //===============================================
    // Kazra'jin

    // Reckless Charge
    // SPELL_RECKLESS_CHARGE                   = 137117, // Visual on the boss while travelling (launch everything) ("They see me rollin ! FUCK YEAH !")
    SPELL_RECKLESS_CHARGE_GROUND_AT         = 138026, // Visual on the ground while boss is travelling (damage dealer ?)
    SPELL_RECKLESS_CHARGE_SHATTER_GROUND    = 137122, // Final damages + Knock back and visual of rocks appearing around
    SPELL_RECKLESS_CHARGE_UNIQUE_DAMAGES    = 137133, // Damage on unique target (AT ? Kazra'jin ?)
    SPELL_RECKLESS_CHARGE_PRE_PATH          = 000000, // TBF: Visual of dark pools on ground before charge

    SPELL_RECKLESS_CHARGE                   = 137107,
    SPELL_RECKLESS_CHARGE_VISUAL            = 137117,
    SPELL_RECKLESS_CHARGE_FACE              = 137121, // Forces facing towards target
    SPELL_RECKLESS_CHARGE_SOUND             = 137131,

    // Need black visual on ground before cast

    // Overload
    // When Kazra'jin is empowered, he overloads after performing a Reckless Charge
    SPELL_OVERLOAD                          = 137149, // Dummy aura to handle the strike back (HandleDummyAuraProc ?) (must add UNIT_STATE_STUN manually)
    SPELL_OVERLOAD_DAMAGES                  = 137151, // Damage spell
    SPELL_OVERLOAD_VISUAL                   = 137163, // Visual on caster of spell that triggered mirror effect
    // In Heroic, Overload turns into Discharge
    SPELL_DISCHARGE                         = 137166, // Periodic dummy ? Why ? (must add UNIT_STATE_STUN manually)
    SPELL_DISCHARGE_DAMAGES                 = 136935, // Damage spell
    SPELL_DISCHARGE_VISUAL                  = 137172, // Visual on all players when spell ticks

    //===============================================
    // Sul the Sandcrawler

    SPELL_SAND_BOLT                         = 136189, // Trigger Missile 136190
    SPELL_SAND_BOLT_DAMAGES                 = 136190, // Damages

    // Quicksand (fuckin AT)
    // Quicksand is an AT, but handling the spell with an AT is too complex... we'll use another mechanism,
    SPELL_QUICKSAND_PERIODIC_DAMAGES        = 136860, // Periodic damages to any target within 7 yards : we must handle apply / remove manually
    SPELL_QUICKSAND_AT_VISUAL               = 137572, // Visual
    SPELL_QUICKSAND_AT_VISUAL_INIT          = 136851,
    SPELL_ENSNARED                          = 136878, // Slow player and stacks; when it reaches 5 stacks, player is Entrapped
    SPELL_ENTRAPPED                         = 136857, // Need to prevent second effect... so annoying

    // Sandstorm
    SPELL_SAND_STORM                        = 136894, // Periodic trigger 136899,
    SPELL_SAND_STORM_DAMAGES                = 136899, // Damages
    SPELL_SAND_STORM_DUMMY_AURA             = 136895, // Maybe visual since it has the same duration as 136894

    //===============================================
    // High Priestess Mar'li

    // Wrath of the Loa
    SPELL_WRATH_OF_THE_LOA                  = 137344,
    SPELL_WRATH_OF_THE_LOA_DARK             = 137347, // When Possessed

    // Loa Spirit <=> Twisted Fate
    SPELL_BLESSED_LOA_SPIRIT                = 137203, // Handle dummy cause Blizzard messed that up
    SPELL_SUMMON_BLESSED_LOA_SPIRIT         = 137200, // Summons 69480
    SPELL_SHADOWED_LOA_SPIRIT               = 137350, // Handle dummy cause Blizzard messed that up (Possessed version of Loa Spirit)
    SPELL_SUMMON_SHADOWED_LOA_SPIRIT        = 137351, // Summons 69548
    // In Heroic, Shadowed Loa Spirit is replaced with Twisted Fate
    SPELL_TWISTED_FATE_PERIODIC             = 137986, // Periodic trigger 137972,
    SPELL_TWISTED_FATE_DAMAGES              = 137972, // Periodic damages while linked AND while not linked (must update value in handler of 137986)
    SPELL_TWISTED_FATE_DUMMY_AURA           = 137964, // Visual maybe ?
    // First npc of Twisted Fate
    SPELL_TWISTED_FATE                      = 137891, // Launch everything (triggers 137893)
    SPELL_TWISTED_FATE_SUMMON_FIRST         = 137893, // Summon the first Twisted Fate
    SPELL_TWISTED_FATE_FORCE_FIRST          = 137943, // Force cast 137950 on self
    SPELL_TWISTED_FATE_CLONE_FIRST          = 137950, // Clone, Dummy, Size... WHAT ELSE ?
    // Second npc of Twisted Fate
    SPELL_TWISTED_FATE_FORCE_SUMMON_SECOND  = 137962, // Trigger 137963 on the most distant player (fuck target)
    SPELL_TWISTED_FATE_SUMMON_SECOND        = 137963, // Summon the second Twisted Fate
    SPELL_TWISTED_FATE_FORCE_SECOND         = 137964, // Force cast 137965 on self
    SPELL_TWISTED_FATE_CLONE_SECOND         = 137965, // Clone, Dummy, Size... WHAT ELSE ?

    //===============================================
    // Summons
    SPELL_FORTIFIED                         = 136864, // Living Sand : On hit by Sandstorm
    SPELL_TREACHEROUS_GROUND                = 137614, // Living Sand : On emerge from reunited Quicksand
    SPELL_BLESSED_GIFT                      = 137303, // Blessed Loa Spirit, heal 5% of Max Health on weakest councillor
    SPELL_BLESSED_TIME_OUT                  = 137204, // Dummy visual for blizz
    SPELL_MARKED_SOUL                       = 137359, // Shadowed Loa Spirit / Player ? Force Loa to follow player
    SPELL_SHADOWED_GIFT                     = 137390, // Instantly kills player (Shadowed Loa Spirit)
    SPELL_SHADOWED_TIME_OUT                 = 137398, // Dummy visual for blizz
    
    //===============================================
    // Visuals
    SPELL_GARA_JALS_SOUL                    = 136423, // NPC Visual
    SPELL_BLESSED_TRANSFORMATION            = 137181, // NPC Visual
    SPELL_SHADOWED_TRANSFORMATION           = 137271, // NPC Visual
    SPELL_GARAJAL_GHOST                     = 000000, // TBF
    
    //===============================================
    // Shared
    SPELL_ZERO_POWER                        = 72242,
};


enum eEvents
{
    //===============================================
    // Frost King Malakk
    EVENT_FRIGID_ASSAULT                    = 1,
    EVENT_BITING_COLD                       = 2,
    EVENT_FROSTBITE                         = 3,

    //===============================================
    // Kazra'jin
    EVENT_RECKLESS_CHARGE_PRE_PATH          = 4,
    EVENT_RECKLESS_CHARGE                   = 5,

    //===============================================
    // Sul the Sandcrawler
    EVENT_SAND_BOLT                         = 6,
    EVENT_QUICKSAND                         = 7,
    EVENT_SANDSTORM                         = 8,

    //===============================================
    // High Priestess Mar'li
    EVENT_WRATH_OF_THE_LOA                  = 9,
    EVENT_WRATH_OF_THE_LOA_DARK             = 10,
    EVENT_BLESSED_LOA_SPIRIT                = 11,
    EVENT_SHADOWED_LOA_SPIRIT               = 12,
    EVENT_TWISTED_FATE                      = 13,

    //===============================================
    // Blessed Loa Spirit
    EVENT_BLESSED_GIFT                      = 14,
    EVENT_MOVE_COUNCILLOR                   = 21,

    //===============================================
    // Shadowed Loa Spiri
    EVENT_SHADOWED_GIFT                     = 15,

    //===============================================
    // Councillots
    EVENT_INCREASE_POWER                    = 16,
    EVENT_DARK_POWER                        = 17,

    //===============================================
    // Quicksand Stalker
    EVENT_QUICKSAND_PERIODIC                = 18, // This only handles apply of the Quicksand damages aura, which handle the rooting by itself
    EVENT_TRY_MERGE                         = 19, // Try merge event is used to merge Quicksand when they are summoend by others Quicksand (only scheduled once)
    EVENT_ACTIVATE_SAND                     = 22,
    
    //===============================================
    // Garajal
    EVENT_SUMMON_SOUL                       = 20,
};


enum eActions
{
    //===============================================
    // Gara'jal
    ACTION_ENTER_COMBAT         = 0, // Garajal + Garajal's soul
    ACTION_EXIT_COUNCILLOR      = 1, // Garajal's soul
    ACTION_FIGHT_RESET          = 2,
    ACTION_FIGHT_BEGIN          = 3,
    ACTION_COUNCILLOR_DIED      = 4,

    //===============================================
    // Councillors
    ACTION_COUNCILLORS_ENTER_COMBAT         = 4, // When one enters combat, everybody else must enter combat
    ACTION_SET_POSSESSED                    = 5, // Initialize the events to possessed phase (called from SpellScript)
    ACTION_SET_UNPOSSESSED                  = 6, // Reset the events to normal (same as for SET_POSSESED)
    ACTION_DARK_POWER                       = 7, // Initialize the Dark Power phase (called from the handler of the periodic dummy aura I assume)

    //===============================================
    // Kazra'jin
    ACTION_RESET_DAMAGES                    = 8, // Must be called each time the periodic aura ticks

    //===============================================
    // Living Sand
    ACTION_FORTIFY                          = 9,

    //===============================================
    // Twisted Fate Helper
    ACTION_TWISTED_FATE_END_FIGHT           = 10, // Used to deallocate memory

    //===============================================
    // Twisted Fate (common)
    ACTION_OTHER_TWISTED_FATE_DIED          = 11,

    //===============================================
    // Quicksand
    ACTION_CREATE_LIVING_SAND               = 12,
};


enum eMotions
{
    //===============================================
    // Gara'jal
    POINT_COUNCILLOR                        = 4343,

    //===============================================
    // Kazra'jin
    POINT_RECKLESS_CHARGE_LAND              = 5000, // Position where Kazrajin lands after performing Reckless Charge
    POINT_RECKLESS_CHARGE_PLAYER            = 6714, // Position where Kazrajin rolls to after landing from Reckless Charge

    //===============================================
    // Blessed Loa Spirit
    POINT_BLESSED_LOA_SPIRIT_COUNCILLOR     = 9413, // Point to identify the councillor the Blessed Loa Spirit is going toward
    // No need to do a point for the Shadowed Loa Spirit since it will not
    // reach the target but only go 6 yards away from him (so we can use MoveFollow)

    //===============================================
    // Twisted Fate (common)
    POINT_MIDDLE                            = 6653, // Point representing the middle point on the line formed by the two Twisted Fate
};


enum eHelperStatus
{
    STATUS_RESET                            = 0,
    STATUS_PROGRESS                         = 1,
    STATUS_DONE                             = 2,
};


enum eDatas
{
    DATA_DARK_POWER_COUNT                   = 0,
    DATA_QUICKSAND_MERGE_COUNT              = 1,
    DATA_SHADOWED_LOA_SPIRIT_TARGET_GUID    = 2, // Accessor to get the GUID of the player the Shadowed Loa Spirit will follow
    DATA_BLESSED_LOA_SPIRIT_TARGET_GUID     = 3, // Accessor to get the GUID of the boss the Blessed Loa Spirit will follow
    DATA_TWISTED_FATE_GUID                  = 4, // Accessor to get the GUID of the twisted fate the caller is linked to
    DATA_TWISTED_FATE_OTHER_DIED            = 5, // Accessor to know if one twisted fate has been unlinked from the other
    DATA_RECKLESS_CHARGE_TARGET_GUID        = 6, // Accessor to get the GUID of the player Kazrajin is targetting for the purpose of Reckless Charge
    DATA_DAMAGES_PAST_SEC                   = 7, // Accessor to get the amount of damages Kazrajin received in the last second
};


enum eTalks : uint32
{
    TALK_AGGRO                  = 0,
    TALK_POSSESS                = 1,
    TALK_SPECIAL                = 2,
    EMOTE_POSSESS               = 6,

    // sul
    TALK_SUL_QUICKSAND          = 3,
    TALK_SUL_SLAY               = 4,
    TALK_SUL_DEATH              = 5,
    TALK_SUL_SANDSTORM          = 7,

    // malakk
    TALK_MALAKK_SLAY            = 3,
    TALK_MALAKK_DEATH           = 4,
    TALK_MALAKK_FROSTBITE       = 5,

    // marli
    TALK_MARLI_SLAY             = 3,
    TALK_MARLI_DEATH            = 4,
    TALK_MARLI_BLESSED          = 5,
    TALK_MARLI_SHADOWED         = 7,

    // kazra'jin
    TALK_KAZRAJIN_CHARGE        = 3,
    TALK_KAZRAJIN_SLAY          = 4,
    TALK_KAZRAJIN_DEATH         = 5,
    TALK_KAZRAJIN_OVERLOAD      = 7
};

//=========================================================
// Helpers

static Creature *GetGarajal(WorldObject *pSource)
{
    return ObjectAccessor::GetCreature(*pSource, pSource->GetInstanceScript()->GetData64(MOB_GARA_JAL));
}

static Creature *GetGarajalsSoul(WorldObject *pSource)
{
    return ObjectAccessor::GetCreature(*pSource, pSource->GetInstanceScript()->GetData64(MOB_GARA_JALS_SOUL));
}

static Creature *GetFrostKingMalakk(WorldObject *pSource)
{
    return ObjectAccessor::GetCreature(*pSource, pSource->GetInstanceScript()->GetData64(BOSS_COUNCIL_FROST_KING_MALAKK));
}

static Creature *GetKazrajin(WorldObject *pSource)
{
    return ObjectAccessor::GetCreature(*pSource, pSource->GetInstanceScript()->GetData64(BOSS_COUNCIL_KAZRAJIN));
}

static Creature *GetSulTheSandcrawler(WorldObject *pSource)
{
    return ObjectAccessor::GetCreature(*pSource, pSource->GetInstanceScript()->GetData64(BOSS_COUNCIL_SUL_THE_SANDCRAWLER));
}

static Creature *GetHighPriestessMarli(WorldObject *pSource)
{
    return ObjectAccessor::GetCreature(*pSource, pSource->GetInstanceScript()->GetData64(BOSS_COUNCIL_HIGH_PRIESTESS_MARLI));
}

static Creature *GetBossByEntry(uint32 uiEntry, WorldObject *pSource)
{
    switch(uiEntry)
    {
    case BOSS_COUNCIL_FROST_KING_MALAKK:
        return GetFrostKingMalakk(pSource);

    case BOSS_COUNCIL_KAZRAJIN:
        return GetKazrajin(pSource);

    case BOSS_COUNCIL_SUL_THE_SANDCRAWLER:
        return GetSulTheSandcrawler(pSource);

    case BOSS_COUNCIL_HIGH_PRIESTESS_MARLI:
        return GetHighPriestessMarli(pSource);

    default:
        return NULL;
    }
}

// Convenient typedef for the accessors
typedef Creature* (*Accessor)(WorldObject *pSource);

static const std::array<uint32, 4> uiBossEntries = {BOSS_COUNCIL_FROST_KING_MALAKK, BOSS_COUNCIL_KAZRAJIN, BOSS_COUNCIL_SUL_THE_SANDCRAWLER, BOSS_COUNCIL_HIGH_PRIESTESS_MARLI};


//=========================================================
// Creature Scripts

// Base class for the councillor's AI (only override common functions)
class boss_council_of_elders_base_AI : public BossAI
{
public:
    boss_council_of_elders_base_AI(Creature *pCreature) :
        BossAI(pCreature, DATA_COUNCIL_OF_ELDERS), pInstance(pCreature->GetInstanceScript())
    {
        events.Reset();
    }

    EventMap darkPowerEvents;

    // Override Reset to reset the EventMap in one place and force
    // reset of the fight by sending DoAction to the helper.
    // Note: if one boss reset, every other boss need to reset. Otherwise,
    // it would be a very major fail, meaning that a creature can reset while
    // other close creatures are still in combat.
    void Reset()
    {
        pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        if (Creature *pGarajal = GetGarajal(me))
        {
            if (pGarajal->AI())
                pGarajal->AI()->DoAction(ACTION_FIGHT_RESET);
        }

        events.Reset();
        darkPowerEvents.Reset();

        me->setPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 0);

        DoCast(me, SPELL_ZERO_POWER);
        uiDarkPowerCount        = 0;
        uiDamageTakenPossessed  = 0;
    }

    // Override EnterCombat to send the DoAction to the helper
    void EnterCombat(Unit *pAttacker) 
    {
        pInstance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        if (Creature* pGarajal = GetGarajal(me))
        {
            if (pGarajal->AI())
                pGarajal->AI()->DoAction(ACTION_FIGHT_BEGIN);
        }

        Talk(TALK_AGGRO);

        InitStandartEvents();
    }

    void EnterEvadeMode()
    {
        me->SetFullHealth();

        BossAI::EnterEvadeMode();
    }

    // Override DoAction for the generic actions
    void DoAction(const int32 iAction) 
    {
        switch(iAction)
        {
        case ACTION_COUNCILLORS_ENTER_COMBAT:
            // The call to AttackStart will be kinda useless for some
            // boss since they can't move, which will result in useless
            // call to MoveChase. Anyway, I do not want to recode this
            // function now.
            InitStandartEvents();
            break;

        case ACTION_SET_POSSESSED:
            Talk(TALK_POSSESS);
            Talk(EMOTE_POSSESS);
            InitPossessedEvents();
            events.ScheduleEvent(EVENT_INCREASE_POWER, GetPowerTimer());
            uiDarkPowerCount = 0;
            break;

        case ACTION_SET_UNPOSSESSED:
            me->SetPower(POWER_ENERGY, 0);
            InitStandartEvents();
            uiDarkPowerCount = 0;
            break;

        case ACTION_DARK_POWER:
            InitDarkPower();
            uiDarkPowerCount = 0;
            break;
        }
    }

    void DamageTaken(Unit *pAttacker, uint32 &ruiAmount)
    {
        // Heroic only shit..
        // if (Aura* pAura = me->GetAura(SPELL_DISCHARGE))

        if (!me->HasAura(SPELL_POSSESSED))
            return;

        uiDamageTakenPossessed += ruiAmount;

        if (uiDamageTakenPossessed >= (float)(me->GetMaxHealth() * 0.25f))
        {
            // No remove when no other councillor alive
            if (IsACouncillorAlive())
            {
                if (Creature *pGarajal = GetGarajalsSoul(me))
                    pGarajal->AI()->DoAction(ACTION_EXIT_COUNCILLOR);
            }
            uiDamageTakenPossessed = 0; // Reset in both case to prevent chain call to IsACouncillorAlive
        }
    }

    void RewardCurrencyAndUpdateState()
    {
        Map::PlayerList const &lPlayers = me->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
        {
            if (Player * const player = itr->GetSource())
            {
                player->ModifyCurrency(CURRENCY_TYPE_VALOR_POINTS, 4000);
            }
        }

        pInstance->UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, MOB_GARA_JAL, me);
    }
    
    void JustDied(Unit *pKiller)
    {
        pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        switch(me->GetEntry())
        {
        case BOSS_COUNCIL_FROST_KING_MALAKK:
            Talk(TALK_MALAKK_DEATH);
            break;
            
        case BOSS_COUNCIL_KAZRAJIN:
            Talk(TALK_KAZRAJIN_DEATH);
            break;
            
        case BOSS_COUNCIL_SUL_THE_SANDCRAWLER:
            Talk(TALK_SUL_DEATH);
            break;
            
        case BOSS_COUNCIL_HIGH_PRIESTESS_MARLI:
            Talk(TALK_MARLI_DEATH);
            break;
            
        default:
            break;
        }

        if (Creature* pGarajal = GetGarajal(me))
        {
            if (CreatureAI* pAI = pGarajal->AI())
            {
                pAI->DoAction(ACTION_COUNCILLOR_DIED);

                if (pAI->GetData(0) < 4)
                    me->SetLootRecipient(nullptr);
                else
                    RewardCurrencyAndUpdateState();
            }
        }
    }
    
    void KilledUnit(Unit *pKilled)
    {
        switch(me->GetEntry())
        {
        case BOSS_COUNCIL_FROST_KING_MALAKK:
            Talk(TALK_MALAKK_SLAY);
            break;
            
        case BOSS_COUNCIL_KAZRAJIN:
            Talk(TALK_KAZRAJIN_SLAY);
            break;
            
        case BOSS_COUNCIL_SUL_THE_SANDCRAWLER:
            Talk(TALK_SUL_SLAY);
            break;
            
        case BOSS_COUNCIL_HIGH_PRIESTESS_MARLI:
            Talk(TALK_MARLI_SLAY);
            break;
            
        default:
            break;
        }
    }

    // Helper function to prevent removing the possessed aura when no
    // other councillor is alive.
    bool IsACouncillorAlive() const
    {
        Creature    *pMalakk    = GetFrostKingMalakk(me),
                    *pKazrajin  = GetKazrajin(me),
                    *pSul       = GetSulTheSandcrawler(me),
                    *pMarli     = GetHighPriestessMarli(me);

        // Pointers should not be null
        if (!pMalakk || !pKazrajin || !pSul || !pMarli)
            return false;

        switch(me->GetEntry())
        {
        case BOSS_COUNCIL_FROST_KING_MALAKK:
            return (pKazrajin->IsAlive() || pSul->IsAlive() || pMarli->IsAlive());

        case BOSS_COUNCIL_KAZRAJIN:
            return (pSul->IsAlive() || pMarli->IsAlive() || pMalakk->IsAlive());

        case BOSS_COUNCIL_SUL_THE_SANDCRAWLER:
            return (pMarli->IsAlive() || pMalakk->IsAlive() || pKazrajin->IsAlive());

        case BOSS_COUNCIL_HIGH_PRIESTESS_MARLI:
            return (pMalakk->IsAlive()|| pKazrajin->IsAlive() || pSul->IsAlive());

        default:
            return false;
        }
    }
    
    uint32 GetData(uint32 uiIndex)
    {
        if (uiIndex == DATA_DARK_POWER_COUNT)
            return uiDarkPowerCount;

        return 0;
    }

// Use the protected scope, so we do not have to declare again the same attributes
protected:
    EventMap        events;
    InstanceScript  *pInstance;
    uint32          uiDarkPowerCount; // Internal counter to know the real amount of damages of Dark Power
    // Internal counter to know the amount of damages we have
    // taken since the possession began. When this amount is
    // equal or higher than 25% of our max health point, we
    // get rid of Garajal's Soul.
    uint32          uiDamageTakenPossessed;

    // Virtual methods used to init the events when fight begin
    // or possession begins / ends. Use pure virtual function to
    // be sure everybody overrides it.
    virtual void InitStandartEvents() = 0;
    virtual void InitPossessedEvents() = 0;

    // Method to init the DarkPower event, so there is no need
    // to repeat the same code each time.
    void InitDarkPower()
    {
        events.Reset();
        darkPowerEvents.Reset();
        darkPowerEvents.ScheduleEvent(EVENT_DARK_POWER, 1000);
    }
    
    uint32 GetPowerTimer() const
    {
        if (Aura *pLingeringPresence = me->GetAura(SPELL_LINGERING_PRESENCE))
        {
            float fReduce   = 1 - (pLingeringPresence->GetStackAmount() / 10);
            uint32 uiTimer  = (2 * IN_MILLISECONDS) * fReduce;
            return uiTimer;
        }
        else
            return 2 * IN_MILLISECONDS;
    }
};
typedef boss_council_of_elders_base_AI CouncilBaseAI;


// Frost King Malakk AI
class boss_frost_king_malakk : public CreatureScript
{
public:
    boss_frost_king_malakk() : CreatureScript("boss_frost_king_malakk") { }

    class boss_frost_king_malakk_AI : public CouncilBaseAI
    {
    public:
        boss_frost_king_malakk_AI(Creature *pCreature) :
            CouncilBaseAI(pCreature)
        {

        }

        // No need to override Reset since there is nothing to reset here
        // No need to override EnterCombat since there is nothing to do here

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            darkPowerEvents.Update(uiDiff);
            events.Update(uiDiff);

            switch (darkPowerEvents.ExecuteEvent())
            {
            case EVENT_DARK_POWER:
                DoCast(me, SPELL_DARK_POWER, true);
                ++uiDarkPowerCount;
                darkPowerEvents.ScheduleEvent(EVENT_DARK_POWER, 1 * IN_MILLISECONDS);
                break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 uiEventId = events.ExecuteEvent())
            {
                switch (uiEventId)
                {
                case EVENT_FRIGID_ASSAULT:
                    DoCast(me, SPELL_FRIGID_ASSAULT);
                    events.ScheduleEvent(EVENT_FRIGID_ASSAULT, urand(15, 35) * IN_MILLISECONDS);
                    break;

                case EVENT_BITING_COLD:
                    DoCastAOE(SPELL_BITING_COLD); // Spell is wierd... handle target selection in SpellScript
                    events.ScheduleEvent(EVENT_BITING_COLD, urand(8, 16) * IN_MILLISECONDS);
                    break;

                case EVENT_FROSTBITE:
                    Talk(TALK_SPECIAL);
                    DoCastAOE(SPELL_FROSTBITE); // Handle target selection in SpellScript
                    events.ScheduleEvent(EVENT_FROSTBITE, urand(8, 16) * IN_MILLISECONDS);
                    break;

                case EVENT_INCREASE_POWER:
                    me->ModifyPower(POWER_ENERGY, 3);
                    if (me->GetPower(POWER_ENERGY) < 100)
                        events.ScheduleEvent(EVENT_INCREASE_POWER, GetPowerTimer());
                    else
                        DoAction(ACTION_DARK_POWER);
                    break;

                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        void InitStandartEvents()
        {
            darkPowerEvents.Reset();
            events.Reset();

            events.ScheduleEvent(EVENT_FRIGID_ASSAULT, 10 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_BITING_COLD, 15 * IN_MILLISECONDS);
        }

        void InitPossessedEvents()
        {
            events.Reset();

            events.ScheduleEvent(EVENT_FRIGID_ASSAULT, 10 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_FROSTBITE, 15 * IN_MILLISECONDS);
        }
    };

    CreatureAI* GetAI(Creature *pCreature) const
    {
        return new boss_frost_king_malakk_AI(pCreature);
    }
};


// Kazra'jin AI
class boss_kazrajin : public CreatureScript
{
public:
    boss_kazrajin() : CreatureScript("boss_kazrajin") { }

    class boss_kazrajin_AI : public CouncilBaseAI
    {
    public:
        boss_kazrajin_AI(Creature *pCreature):
            CouncilBaseAI(pCreature), uiDamagesDoneInPastSecs(0)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            SetCombatMovement(false);
        }

        // Override reset to reset the amount of damages received, and the
        // movement flags.
        void Reset()
        {
            uiDamagesDoneInPastSecs = 0;

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            CouncilBaseAI::Reset(); // Finalize Reset
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            darkPowerEvents.Update(uiDiff);
            events.Update(uiDiff);

            switch (darkPowerEvents.ExecuteEvent())
            {
            case EVENT_DARK_POWER:
                DoCast(me, SPELL_DARK_POWER, true);
                ++uiDarkPowerCount;
                darkPowerEvents.ScheduleEvent(EVENT_DARK_POWER, 1 * IN_MILLISECONDS);
                break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 uiEventId = events.ExecuteEvent())
            {
                switch (uiEventId)
                {
                case EVENT_RECKLESS_CHARGE_PRE_PATH:
                {
                    //DoCast(me, SPELL_RECKLESS_CHARGE_PRE_PATH);
                    DoCast(me, SPELL_RECKLESS_CHARGE);

                    events.ScheduleEvent(EVENT_RECKLESS_CHARGE, 3 * IN_MILLISECONDS);
                    break;
                }

                case EVENT_RECKLESS_CHARGE:
                    events.ScheduleEvent(EVENT_RECKLESS_CHARGE_PRE_PATH, 7 * IN_MILLISECONDS);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    if (rand()%10>4)
                        Talk(TALK_KAZRAJIN_CHARGE);
                    DoCast(me, SPELL_RECKLESS_CHARGE_VISUAL); // Launch everything
                    // Summon npcs for the visual of Reckless Charge while travelling ?
                    // Handle next part in MovementInform.
                    break;

                case EVENT_INCREASE_POWER:
                    me->ModifyPower(POWER_ENERGY, 3);
                    if (me->GetPower(POWER_ENERGY) < 100)
                        events.ScheduleEvent(EVENT_INCREASE_POWER, GetPowerTimer());
                    else
                        DoAction(ACTION_DARK_POWER);
                    break;

                default:
                    break;
                }
            }
        }

        void MovementInform(uint32 uiMotionType, uint32 uiMotionPointId)
        {
            if (uiMotionPointId == POINT_RECKLESS_CHARGE_LAND)
            {
                me->RemoveAura(SPELL_RECKLESS_CHARGE_VISUAL);
                DoCastAOE(SPELL_RECKLESS_CHARGE_SHATTER_GROUND);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

                if (me->HasAura(SPELL_POSSESSED))
                {
                    Talk(TALK_SPECIAL);
                    if (IsHeroic())
                        DoCast(me, SPELL_DISCHARGE);
                    else
                    {
                        Talk(TALK_KAZRAJIN_OVERLOAD);
                        DoCast(me, SPELL_OVERLOAD);
                    }

                    events.RescheduleEvent(EVENT_RECKLESS_CHARGE_PRE_PATH, 1000+rand()%2000);
                }
                else
                    events.RescheduleEvent(EVENT_RECKLESS_CHARGE_PRE_PATH, 1000+rand()%2000);
            }
        }
    

        // Override Damage Taken again to handle the Discharge aura.
        /*
        void DamageTaken(Unit *pAttacker, uint32 &ruiAmount)
        {
            if (!me->HasAura(SPELL_POSSESSED))
                return;

            if (me->HasAura(SPELL_POSSESSED))
            {
                if (me->HasAura(SPELL_DISCHARGE))
                    uiDamagesDoneInPastSecs += ruiAmount;
            }
            
            uiDamageTakenPossessed += ruiAmount;
            if ((float)uiDamageTakenPossessed >= (float)((float)me->GetMaxHealth() * 0.25f))
            {
                // No remove when no other councillor alive
                if (IsACouncillorAlive())
                    DoCast(me, SPELL_LINGERING_PRESENCE);
                uiDamageTakenPossessed = 0; // Reset in both case to prevent chain call to IsACouncillorAlive
            }
        }*/

        // Override DoAction again to handle the Discharge Aura.
        /*
        void DoAction(int32 iAction)
        {
            switch(iAction)
            {
            case ACTION_RESET_DAMAGES:
                uiDamagesDoneInPastSecs = 0;
                break;

            default:
                CouncilBaseAI::DoAction(iAction);
                return;
            }
        }
        
        uint32 GetData(uint32 uiIndex) 
        {
            return uiDarkPowerCount;
        }*/

        uint64 GetGUID(int32 iIndex) 
        {
            if (iIndex == DATA_RECKLESS_CHARGE_TARGET_GUID)
                return uiRecklessChargeTargetGUID;

            return 0;
        }

    private:
        // Amount of damages received during past seconds in Heroic,
        // cause spell ticks each second, instead of being a permanent
        // dummy. Fuck blizzard's logic.
        uint32 uiDamagesDoneInPastSecs;
        uint64 uiRecklessChargeTargetGUID;
        void InitStandartEvents()
        {
            darkPowerEvents.Reset();
            events.Reset();

            events.ScheduleEvent(EVENT_RECKLESS_CHARGE_PRE_PATH, urand(3, 4) * IN_MILLISECONDS);
        }

        void InitPossessedEvents()
        {
            events.Reset();

            events.ScheduleEvent(EVENT_RECKLESS_CHARGE_PRE_PATH, urand(3, 4) * IN_MILLISECONDS);
        }
    };
    

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new boss_kazrajin_AI(pCreature);
    }
    
};


// Sul the Sandcrawler AI
class boss_sul_the_sandcrawler : public CreatureScript
{
public:
    boss_sul_the_sandcrawler() : CreatureScript("boss_sul_the_sandcrawler") { }

    class boss_sul_the_sandcrawler_AI : public CouncilBaseAI
    {
    public:
        boss_sul_the_sandcrawler_AI(Creature *pCreature) :
            CouncilBaseAI(pCreature)
        {
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            darkPowerEvents.Update(uiDiff);
            events.Update(uiDiff);

            switch (darkPowerEvents.ExecuteEvent())
            {
            case EVENT_DARK_POWER:
                DoCast(me, SPELL_DARK_POWER, true);
                ++uiDarkPowerCount;
                darkPowerEvents.ScheduleEvent(EVENT_DARK_POWER, 1 * IN_MILLISECONDS);
                break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 uiEventId = events.ExecuteEvent())
            {
                switch(uiEventId)
                {
                case EVENT_SAND_BOLT:
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(pTarget, SPELL_SAND_BOLT); 
                    events.ScheduleEvent(EVENT_SAND_BOLT, 4000 + (rand()%30*100));
                    break;

                case EVENT_QUICKSAND:
                {
                    Talk(TALK_SUL_QUICKSAND);
                    std::list<Player*> playerList;
                    std::list<Player*> tempList;
                    me->GetPlayerListInGrid(playerList, 150.f);

                    std::copy(std::begin(playerList), std::end(playerList), std::inserter(tempList, tempList.begin()));

                    playerList.remove_if([this](Player const* pPlayer) -> bool
                    {
                        return this->me->GetExactDist2d(pPlayer) < 15.0f; // Remove players that are closer than 15 yards
                    });

                    // Pick one of the players in the list if not empty
                    if (!playerList.empty())
                    {
                        if (Player *pPlayer = Trinity::Containers::SelectRandomContainerElement<std::list<Player*>>(playerList))
                            me->SummonCreature(MOB_LIVING_SAND, *pPlayer);
                    }
                    else if (!tempList.empty())
                    {
                        if (Player *pPlayer = Trinity::Containers::SelectRandomContainerElement<std::list<Player*>>(tempList))
                            me->SummonCreature(MOB_LIVING_SAND, *pPlayer);
                    }

                    events.ScheduleEvent(EVENT_QUICKSAND, urand(20, 25) * IN_MILLISECONDS);
                    break;
                }

                case EVENT_SANDSTORM:
                    Talk(TALK_SPECIAL);
                    Talk(TALK_SUL_SANDSTORM);
                    DoCastAOE(SPELL_SAND_STORM);
                    events.ScheduleEvent(EVENT_SANDSTORM, 40 * IN_MILLISECONDS);
                    break;
                    
                case EVENT_INCREASE_POWER:
                    me->ModifyPower(POWER_ENERGY, 3);
                    if (me->GetPower(POWER_ENERGY) < 100)
                        events.ScheduleEvent(EVENT_INCREASE_POWER, GetPowerTimer());
                    else
                        DoAction(ACTION_DARK_POWER);
                    break;

                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        void InitStandartEvents()
        {
            darkPowerEvents.Reset();
            events.Reset();

            events.ScheduleEvent(EVENT_SAND_BOLT, 5 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_QUICKSAND, 10 * IN_MILLISECONDS);
        }

        void InitPossessedEvents()
        {
            events.Reset();

            events.ScheduleEvent(EVENT_SAND_BOLT, 5 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_QUICKSAND, 7 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SANDSTORM, 10 * IN_MILLISECONDS);
        }
    };

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new boss_sul_the_sandcrawler_AI(pCreature);
    }
};


// High Priestess Mar'li
class boss_high_priestess_marli : public CreatureScript
{
public:
    boss_high_priestess_marli() : CreatureScript("boss_high_priestess_marli") { }

    class boss_high_priestess_marli_AI : public CouncilBaseAI
    {
    public:
        boss_high_priestess_marli_AI(Creature *pCreature) :
            CouncilBaseAI(pCreature)
        {

        }

        // Override Reset() to clean the lists
        void Reset()
        {
            uiShadowedSpiritPlayerGUIDs.clear();
            uiBlessedLoaSpiritBossGUIDs.clear();

            CouncilBaseAI::Reset();
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            darkPowerEvents.Update(uiDiff);
            events.Update(uiDiff);

            switch (darkPowerEvents.ExecuteEvent())
            {
            case EVENT_DARK_POWER:
                DoCast(me, SPELL_DARK_POWER, true);
                ++uiDarkPowerCount;
                darkPowerEvents.ScheduleEvent(EVENT_DARK_POWER, 1 * IN_MILLISECONDS);
                break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 uiEventId = events.ExecuteEvent())
            {
                switch (uiEventId)
                {
                case EVENT_WRATH_OF_THE_LOA:
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM)) // Target is TARGET_UNIT_ENEMY
                        DoCast(pTarget, SPELL_WRATH_OF_THE_LOA);
                    events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA, urand(4, 8) * IN_MILLISECONDS);
                    break;

                case EVENT_BLESSED_LOA_SPIRIT:
                    // Check that we are not the only left councillor (otherwise it would be cheaty)
                    for (uint8 i = 0; i < 3; ++i)
                    {
                        if (Creature *pCouncillor = GetBossByEntry(uiBossEntries[i], me))
                        {
                            if (pCouncillor->IsAlive())
                            {
                                // Select a target now
                                std::list<Creature*> councillors = { GetFrostKingMalakk(me), GetKazrajin(me), GetSulTheSandcrawler(me) };
                                councillors.remove_if([](Creature const* pCouncil) -> bool { return pCouncil->isDead(); });
                                councillors.sort([](Creature const* first, Creature const* second) -> bool { return first->GetHealthPct() < second->GetHealthPct(); });

                                if (councillors.front())
                                    uiBlessedLoaSpiritBossGUIDs.push_back(councillors.front()->GetGUID());

                                // Cast after having init the list to be sure the guid has been set
                                DoCast(me, SPELL_BLESSED_LOA_SPIRIT);
                                events.ScheduleEvent(EVENT_BLESSED_LOA_SPIRIT, urand(20, 28) * IN_MILLISECONDS);


                                // Get out of the loop and break again; this way, we do not schedule
                                // the event if there is no other boss than Mar'li alive.
                                break;
                            }
                        }
                    }
                    break;

                case EVENT_WRATH_OF_THE_LOA_DARK:
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                        DoCast(pTarget, SPELL_WRATH_OF_THE_LOA_DARK); // Target is TARGET_UNIT_ENEMY
                    events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_DARK, urand(3, 6) * IN_MILLISECONDS);
                    break;

                case EVENT_SHADOWED_LOA_SPIRIT:
                {
                    Talk(TALK_SPECIAL);
                    std::list<Player*> playerList;
                    me->GetPlayerListInGrid(playerList, 500.0f);

                    // Remove too close targets (otherwise player would be instantly killed)
                    playerList.remove_if([this](Player const* pPlayer) -> bool
                    {
                        // Instakill triggers when spirit is 6 yards away from player or less
                        // so remove players in a 10 yards range to let the other some time
                        // to try to get to safety.
                        return this->me->GetExactDist2d(pPlayer) <= 10.0f;
                    });

                    // List is empty, reschedule event, and break
                    if (playerList.empty())
                    {
                        events.ScheduleEvent(EVENT_SHADOWED_LOA_SPIRIT, urand(15, 25) * IN_MILLISECONDS);
                        break;
                    }
                    else
                    {
                        // Instead of computing again the player list, we'll send the
                        // guid of a randomly chosen target in the playerList to the
                        // summoned creature.
                        if (Player *pPlayer = Trinity::Containers::SelectRandomContainerElement(playerList))
                            uiShadowedSpiritPlayerGUIDs.push_back(pPlayer->GetGUID());

                        DoCast(me, SPELL_SHADOWED_LOA_SPIRIT);
                        events.ScheduleEvent(EVENT_SHADOWED_LOA_SPIRIT, urand(15, 25) * IN_MILLISECONDS);
                    }
                    break;
                }

                case EVENT_TWISTED_FATE:
                    DoCastAOE(SPELL_TWISTED_FATE); // Automatically handle target selection in the SpellScript
                    events.ScheduleEvent(EVENT_TWISTED_FATE, urand(10, 20) * IN_MILLISECONDS);
                    break;
                    
                case EVENT_INCREASE_POWER:
                    me->ModifyPower(POWER_ENERGY, 3);
                    if (me->GetPower(POWER_ENERGY) < 100)
                        events.ScheduleEvent(EVENT_INCREASE_POWER, GetPowerTimer());
                    else
                        DoAction(ACTION_DARK_POWER);
                    break;

                default:
                    break;
                }
            }

            if (!DoSpellAttackIfReady(me->HasAura(SPELL_POSSESSED) ? SPELL_WRATH_OF_THE_LOA_DARK : SPELL_WRATH_OF_THE_LOA))
                DoMeleeAttackIfReady();
        }

        // Override function to return the GUIDs of the targets for the Loa Spirits.
        uint64 GetGUID(int32 iIndex) const
        {
            switch(iIndex)
            {
            case DATA_BLESSED_LOA_SPIRIT_TARGET_GUID:
                if (uiBlessedLoaSpiritBossGUIDs.empty())
                    return 0;
                else
                    return uiBlessedLoaSpiritBossGUIDs.back();
                break;

            case DATA_SHADOWED_LOA_SPIRIT_TARGET_GUID:
                if (uiShadowedSpiritPlayerGUIDs.empty())
                    return 0;
                else
                    return uiShadowedSpiritPlayerGUIDs.back();
                break;

            default:
                return 0;
            }
        }

        // Override JustDied() handler to clean the lists (it would use memory for nothing)
        void JustDied(Unit *pKiller)
        {
            uiShadowedSpiritPlayerGUIDs.clear();
            uiBlessedLoaSpiritBossGUIDs.clear();

            CouncilBaseAI::JustDied(pKiller);
        }

    private:
        // GUIDs of the players that will be chased by a Shadowed Loa Spirit
        // Always select the last in the list when a Shadowed Loa Spirit
        // is summoned.
        std::list<uint64> uiShadowedSpiritPlayerGUIDs;

        // GUIDS of the boss that will be chased by a Blessed Loa Spirit.
        // Like above, select the last in the list when a Blessed Loa Spirit
        // is summoned.
        std::list<uint64> uiBlessedLoaSpiritBossGUIDs;

        void InitStandartEvents()
        {
            darkPowerEvents.Reset();
            events.Reset();

            events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA, 5 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_BLESSED_LOA_SPIRIT, 25 * IN_MILLISECONDS);
        }

        void InitPossessedEvents()
        {
            events.Reset();

            events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_DARK, 5 * IN_MILLISECONDS);
            if (IsHeroic())
                events.ScheduleEvent(EVENT_TWISTED_FATE, 25 * IN_MILLISECONDS);
            else
                events.ScheduleEvent(EVENT_SHADOWED_LOA_SPIRIT, 20 * IN_MILLISECONDS);
        }
    };

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new boss_high_priestess_marli_AI(pCreature);
    }
};


// Garajal
class mob_garajal : public CreatureScript
{
public:
    mob_garajal() : CreatureScript("mob_garajal") { }
    
    struct mob_garajal_AI : public BossAI
    {
        mob_garajal_AI(Creature *pCreature) : BossAI(pCreature, DATA_COUNCIL_OF_ELDERS)
        {
            me->SetReactState(REACT_PASSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
            events.Reset();
        }

        std::list<uint64> m_lBossGuids;
        uint32 m_uiDeadCouncillors;

        void Reset()
        {
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            m_uiDeadCouncillors += uiData;
        }

        uint32 GetData(uint32 uiType)
        {
            return m_uiDeadCouncillors;
        }

        void ResetFight()
        {
            if (instance->GetBossState(DATA_COUNCIL_OF_ELDERS) == NOT_STARTED)
                return;

            //mutex.acquire();

            FillBossGuids(m_lBossGuids);

            instance->SetBossState(DATA_COUNCIL_OF_ELDERS, NOT_STARTED);

            summons.DespawnAll();

            me->SetVisible(true);
            events.Reset();

            for (const uint64 uiGuid : m_lBossGuids)
            {
                if (Creature* pBoss = ObjectAccessor::GetCreature(*me, uiGuid))
                {
                    if (pBoss->IsAlive())
                    {
                        if (pBoss->IsInCombat() && pBoss->AI())
                            pBoss->AI()->EnterEvadeMode();
                    }
                    else
                    {
                        float x, y, z, o;
                        pBoss->GetHomePosition(x, y, z, o);
                        pBoss->NearTeleportTo(x, y, z, o);
                        pBoss->Respawn();
                        pBoss->SetFacingTo(o);
                    }
                }
            }

            DespawnCreatures();

            //mutex.release();
        }

        void FillBossGuids(std::list<uint64>&list)
        {
            m_uiDeadCouncillors = 0;

            list.clear();

            list.push_back(instance->GetData64(BOSS_COUNCIL_FROST_KING_MALAKK));
            list.push_back(instance->GetData64(BOSS_COUNCIL_HIGH_PRIESTESS_MARLI));
            list.push_back(instance->GetData64(BOSS_COUNCIL_SUL_THE_SANDCRAWLER));
            list.push_back(instance->GetData64(BOSS_COUNCIL_KAZRAJIN));
        }

        void FinishFight()
        {
            if (instance->GetBossState(DATA_COUNCIL_OF_ELDERS) != DONE)
                instance->SetBossState(DATA_COUNCIL_OF_ELDERS, DONE);

            /*
            if (IsHeroic())
            {
                if (Creature *pTwistedFateHelper = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_TWISTED_FATE_HELPER)))
                    pTwistedFateHelper->AI()->DoAction(ACTION_TWISTED_FATE_END_FIGHT);
            }
            */

            /*
            std::list<Player*> playerList;
            me->GetPlayerListInGrid(playerList, 500.0f);

            for (Player *pIter : playerList)
                pIter->RemoveAurasDueToSpell(SPELL_SHADOWED_SOUL); */

            // Something for Gara'jal here I suppose

            summons.DespawnAll();
            DespawnCreatures();
        }

        void BeginFight()
        {
            if (instance->GetBossState(DATA_COUNCIL_OF_ELDERS) == IN_PROGRESS)
                return;

            m_uiDeadCouncillors = 0;

            instance->SetBossState(DATA_COUNCIL_OF_ELDERS, IN_PROGRESS);

            FillBossGuids(m_lBossGuids);

            for (const uint64 uiGuid : m_lBossGuids)
            {
                if (Creature* pBoss = ObjectAccessor::GetCreature(*me, uiGuid))
                {
                    /*
                    if (!pBoss->IsAlive())
                        pBoss->Respawn();
                    */
                    if (!pBoss->IsInCombat() && pBoss->AI())
                    {
                        DoZoneInCombat(pBoss, 150.f);
                        pBoss->AI()->DoAction(ACTION_COUNCILLORS_ENTER_COMBAT);
                    }
                }
            }
            events.ScheduleEvent(EVENT_SUMMON_SOUL, 3 * IN_MILLISECONDS);
        }

        void DoAction(const int32 iAction)
        {
            switch (iAction)
            {
            case ACTION_FIGHT_BEGIN:
                BeginFight();
                break;
            case ACTION_FIGHT_RESET:
                ResetFight();
                break;
            case ACTION_COUNCILLOR_DIED:
                ++m_uiDeadCouncillors;
                if (m_uiDeadCouncillors > 3)
                    FinishFight();
                else
                if (Creature* pSoul = ObjectAccessor::GetCreature(*me, instance->GetData64(MOB_GARA_JALS_SOUL)))
                    pSoul->AI()->DoAction(ACTION_ENTER_COMBAT);
                break;
            default:
                break;
            }
        }

        void JustSummoned(Creature* pSummoned)
        {
            summons.Summon(pSummoned);
        }

        void SummonedCreatureDespawn(Creature* pSummoned)
        {
            summons.Despawn(pSummoned);
        }

        inline void DespawnCreatures() const
        {
            DespawnCreaturesByEntry(MOB_LIVING_SAND);
            DespawnCreaturesByEntry(MOB_BLESSED_LOA_SPIRIT);
            DespawnCreaturesByEntry(MOB_SHADOWED_LOA_SPIRIT);
            DespawnCreaturesByEntry(MOB_TWISTED_FATE_FIRST);
            DespawnCreaturesByEntry(MOB_TWISTED_FATE_SECOND);
        }

        void DespawnCreaturesByEntry(uint32 uiEntry) const
        {
            std::list<Creature*> minionsList;
            GetCreatureListWithEntryInGrid(minionsList, me, uiEntry, 500.0f);

            for (Creature *pMinion : minionsList)
                pMinion->DespawnOrUnsummon();
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (events.Empty())
                return;

            events.Update(uiDiff);

            while (uint32 uiEventId = events.ExecuteEvent())
            {
                switch (uiEventId)
                {
                case EVENT_SUMMON_SOUL:
                    Talk(0);
                    me->SetVisible(false);
                    if (Creature *pSoul = me->SummonCreature(MOB_GARA_JALS_SOUL, *me))
                    {
                        pSoul->AI()->DoAction(ACTION_ENTER_COMBAT);
                        instance->SetData64(MOB_GARA_JALS_SOUL, pSoul->GetGUID());
                    }
                    break;

                default:
                    break;
                }
            }
        }
    private:
        EventMap        events;
        InstanceScript  *pInstance;
/*    protected:
        ACE_Recursive_Thread_Mutex mutex;*/

    };
    
    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new mob_garajal_AI(pCreature);
    }
};

class guidVectorPredicate
{
public:
    guidVectorPredicate(uint64 guid) : _guid(guid) {}

    bool operator()(uint64 uiGuid) const
    {
        return uiGuid == _guid;
    }
private:
    uint64 _guid;
};
// Garajal's Soul
class mob_garajals_soul : public CreatureScript
{
    enum eEvents : uint32
    {
        EVENT_NONE,
        EVENT_POSSESS
    };
public:
    mob_garajals_soul() : CreatureScript("mob_garajals_soul") { }

    class mob_garajals_soul_AI : public ScriptedAI
    {
    public:
        mob_garajals_soul_AI(Creature *pCreature) :
            ScriptedAI(pCreature), pInstance(pCreature->GetInstanceScript()), uiCouncillorEntry(0)
        {
            InitList(m_lBossGuids);
        }

        void InitList(std::list<uint64> &list)
        {
            list.push_back(pInstance->GetData64(BOSS_COUNCIL_FROST_KING_MALAKK));
            list.push_back(pInstance->GetData64(BOSS_COUNCIL_HIGH_PRIESTESS_MARLI));
            list.push_back(pInstance->GetData64(BOSS_COUNCIL_SUL_THE_SANDCRAWLER));
            list.push_back(pInstance->GetData64(BOSS_COUNCIL_KAZRAJIN));
        }
        
        void Reset()
        {
            uiCouncillorEntry = 0;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->AddAura(SPELL_GARA_JALS_SOUL, me);
            me->SetWalk(false);
        }
        
        void EnterEvadeMode() 
        {
            uiCouncillorEntry = 0; // Reset councillor entry to prevent bug
            me->GetMotionMaster()->MovementExpired(); // Reset movement so we will not continue to follow a councillor
            ScriptedAI::EnterEvadeMode();
        }

        bool Possess(Unit* pCreature)
        {
            DoCast(pCreature, SPELL_POSSESSED);
            me->EnterVehicle(pCreature, 0, true);

            return me->IsOnVehicle();
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            if (uiPointId == 1)
            {
                if (Creature* pNextCouncillor = ObjectAccessor::GetCreature(*me, pInstance->GetData64(uiCouncillorEntry)))
                {
                    if (!Possess(pNextCouncillor))
                        events.ScheduleEvent(EVENT_POSSESS, 200);
                }
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (me->IsOnVehicle())
                return;

            events.Update(uiDiff);

            switch (events.ExecuteEvent())
            {
            case EVENT_POSSESS:
                if (Creature *pNextCouncillor = GetNextCouncillor(uiCouncillorEntry))
                {
                    uiCouncillorEntry = pNextCouncillor->GetEntry();

                    float fX, fY;
                    GetPositionWithDistInOrientation(me, me->GetExactDist2d(pNextCouncillor) - 5.f, me->GetAngle(pNextCouncillor), fX, fY);

                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MovePoint(1, fX, fY, me->GetMap()->GetHeight(fX, fY, pNextCouncillor->GetPositionZ()) + 0.8f);
                }
                else
                    events.ScheduleEvent(EVENT_POSSESS, 300);
            }
        }

        void DoAction(const int32 iAction)
        {
            switch(iAction)
            {
            case ACTION_ENTER_COMBAT:
                // Always possess Malakk first
                events.ScheduleEvent(EVENT_POSSESS, 3000);
                break;

            case ACTION_EXIT_COUNCILLOR:
                // Set Garajal visible again
                DoCast(SPELL_LINGERING_PRESENCE);
                me->ExitVehicle();
                // DoAction may be called after EnterEvadeMode() because possessed boss can reset 
                // after the call. Return to prevent following a new councillor.
                if (!uiCouncillorEntry || CheckBossState())
                    return;

                // In Heroic, each time Garajal is forced out of a councillor, he leaves
                // a Soul Fragment behind. (In fact there is no npc summoned, just a spell
                // cast).
                if (IsHeroic())
                    DoCastAOE(SPELL_SOUL_FRAGMENT_SELECTOR);

                // Select a new councillor
                events.ScheduleEvent(EVENT_POSSESS, 3000);
                break;

            default:
                break;
            }
        }
        
        bool CheckBossState()
        {
            for (uint64 councGuid : m_lBossGuids)
            {
                Creature* pCouncillor = ObjectAccessor::GetCreature(*me, councGuid);

                if (pCouncillor && pCouncillor->IsAlive() && (!pCouncillor->IsInCombat() || pCouncillor->IsInEvadeMode()))
                    return true;
            }

            return false;
        }

    private:
        uint32          uiCouncillorEntry;
        InstanceScript  *pInstance;
        std::list<uint64> m_lBossGuids;

        // Helper function to find the next boss to possess
        Creature *GetNextCouncillor(uint32 uiOriginalEntry = 0)
        {
            uint32      uiNextEntry = 0;
            float       fHealthNumber = 0.f;

            if (Creature* pGarajal = GetGarajal(me))
            {
                // We're the only councillor alive, no need to perform this check
                if (pGarajal->AI()->GetData(0) > 3)
                {
                    return NULL;
                }
            }

            // This is the first call, init original entry with the current councillor entry
            if (!uiOriginalEntry)
            {
                if (Creature* pMalakk = GetFrostKingMalakk(me))
                {
                    uiCouncillorEntry = BOSS_COUNCIL_FROST_KING_MALAKK;
                    return pMalakk;
                }
            }

            std::list<uint64> tempList;
            InitList(tempList);

            tempList.remove_if(guidVectorPredicate(pInstance->GetData64(uiCouncillorEntry)));
            for (auto const pGuid : tempList)
            {
                if (Creature* pCreature = ObjectAccessor::GetCreature(*me, pGuid))
                {
                    if (pCreature->IsAlive())
                    {
                        if (fHealthNumber < pCreature->GetHealthPct())
                        {
                            fHealthNumber = pCreature->GetHealthPct();
                            uiNextEntry = pCreature->GetEntry();
                        }
                    }
                }
            }

            if (Creature* pCreature = ObjectAccessor::GetCreature(*me, pInstance->GetData64(uiNextEntry)))
                return pCreature;

            return NULL;
        }
    };
    
    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new mob_garajals_soul_AI(pCreature);
    }
};

// Living Sand AI
class mob_living_sand : public CreatureScript
{
public:
    mob_living_sand() : CreatureScript("mob_living_sand") { }

    class mob_living_sand_AI : public ScriptedAI
    {
    public:
        mob_living_sand_AI(Creature *pCreature) :
            ScriptedAI(pCreature), pInstance(pCreature->GetInstanceScript())
        {
        }

        void IsSummonedBy(Unit *pSummoner)
        {
            me->AddAura(SPELL_QUICKSAND_AT_VISUAL_INIT, me);
            Initialize();
        }

        void Reset()
        {
        }

        void EnterEvadeMode()
        {
        }
        
        void Initialize()
        {
            me->RemoveAurasDueToSpell(SPELL_FORTIFIED);
            me->SetHealth(me->GetMaxHealth());
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
            me->GetMotionMaster()->MovementExpired();
            me->GetMotionMaster()->MoveIdle();
            events.ScheduleEvent(EVENT_QUICKSAND_PERIODIC, 500);
        }

        void DoAction(const int32 iAction)
        {
            switch(iAction)
            {
            case ACTION_CREATE_LIVING_SAND:
                if (!me->HasAura(SPELL_QUICKSAND_AT_VISUAL) && !me->HasAura(SPELL_QUICKSAND_AT_VISUAL_INIT))
                {
                    DoCast(me, SPELL_FORTIFIED, true);
                }
                else
                {
                    me->RemoveAura(SPELL_QUICKSAND_AT_VISUAL_INIT, 0, 0, AURA_REMOVE_BY_EXPIRE);
                    me->RemoveAura(SPELL_QUICKSAND_AT_VISUAL, 0, 0, AURA_REMOVE_BY_EXPIRE);
                    me->RemoveAllAuras();
                    events.ScheduleEvent(EVENT_ACTIVATE_SAND, 4000);
                    events.CancelEvent(EVENT_QUICKSAND_PERIODIC);
                }
                break;
            default:
                break;
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 uiEventId = events.ExecuteEvent())
            {
                switch (uiEventId)
                {
                case EVENT_QUICKSAND_PERIODIC:
                {
                    std::list<Player*> playerList;
                    me->GetPlayerListInGrid(playerList, 100.f);

                    for (Player *pPlayer : playerList)
                    {
                        if (pPlayer->GetExactDist2d(me) < (7.4f + me->GetFloatValue(UNIT_FIELD_BOUNDING_RADIUS)))
                        {
                            if (Aura* pAura = pPlayer->GetAura(SPELL_QUICKSAND_PERIODIC_DAMAGES, me->GetGUID()))
                            {
                                pAura->RefreshDuration(false);
                            }
                            else
                                me->AddAura(SPELL_QUICKSAND_PERIODIC_DAMAGES, pPlayer);
                        }
                        else
                            pPlayer->RemoveAura(SPELL_QUICKSAND_PERIODIC_DAMAGES, me->GetGUID(), 0, AURA_REMOVE_BY_EXPIRE);
                            // Handle the root and cie in the AuraScript
                    }

                    events.ScheduleEvent(EVENT_QUICKSAND_PERIODIC, 500);
                }
                    break;
                case EVENT_ACTIVATE_SAND:
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                    if (!me->IsInCombat())
                        DoZoneInCombat(me, 100.f);
                    else if (me->GetVictim())
                        me->GetMotionMaster()->MoveChase(me->GetVictim());
                    else if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        AttackStart(pTarget);
                    break;
                }
            }

            if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE) || !UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
        
        void DamageTaken(Unit* pDealer, uint32& uiDamage)
        {
            if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
            {
                uiDamage = 0;
                return;
            }

            if (uiDamage >= me->GetHealth())
            {
                uiDamage = 0;
                DoCast(me, SPELL_QUICKSAND_AT_VISUAL, true);
                Initialize();
            }
        }

    private:
        InstanceScript  *pInstance;
    };

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new mob_living_sand_AI(pCreature);
    }
};


// Blessed Loa Spirit
class mob_blessed_loa_spirit : public CreatureScript
{
public:
    mob_blessed_loa_spirit() : CreatureScript("mob_blessed_loa_spirit") { }

    class mob_blessed_loa_spirit_AI : public ScriptedAI
    {
    public:
        mob_blessed_loa_spirit_AI(Creature *pCreature) :
            ScriptedAI(pCreature), pInstance(pCreature->GetInstanceScript()), uiTargetGuid(0)
        {
            me->SetReactState(REACT_PASSIVE);
            events.Reset();
        }

        void InitList(std::list<uint64> &list)
        {
            list.push_back(pInstance->GetData64(BOSS_COUNCIL_FROST_KING_MALAKK));
            list.push_back(pInstance->GetData64(BOSS_COUNCIL_HIGH_PRIESTESS_MARLI));
            list.push_back(pInstance->GetData64(BOSS_COUNCIL_SUL_THE_SANDCRAWLER));
            list.push_back(pInstance->GetData64(BOSS_COUNCIL_KAZRAJIN));
        }

        void Reset()
        {
            DoCast(me, SPELL_BLESSED_TRANSFORMATION);
        }

        // Override function to be sure there won't be any call to MoveChase (at least in AttackStart)
        void AttackStart(Unit *pTarget) { }

        void IsSummonedBy(Unit *pSummoner)
        {
            HandleTargetSelection();
        }

        void JustDied(Unit* /*pkiller*/)
        {
            me->DespawnOrUnsummon(5000);
        }

        void Move()
        {
            if (Creature *pCouncillor = ObjectAccessor::GetCreature(*me, uiTargetGuid))
                me->GetMotionMaster()->MovePoint(POINT_BLESSED_LOA_SPIRIT_COUNCILLOR, *pCouncillor);
            else
                HandleTargetSelection();
        }

        void HandleTargetSelection()
        {
            float fHealthNumber = 100.f;
            std::list<uint64> tempList;

            InitList(tempList);

            for (auto const pGuid : tempList)
            {
                if (Creature* pCreature = ObjectAccessor::GetCreature(*me, pGuid))
                {
                    if (!pCreature->IsAlive())
                        continue;

                    if (fHealthNumber >= pCreature->GetHealthPct())
                    {
                        fHealthNumber = pCreature->GetHealthPct();
                        uiTargetGuid = pGuid;
                    }
                }
            }

            events.RescheduleEvent(EVENT_BLESSED_GIFT, 20 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_MOVE_COUNCILLOR, 500);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 uiEventId = events.ExecuteEvent())
            {
                switch (uiEventId)
                {
                case EVENT_MOVE_COUNCILLOR:
                    Move();
                    break;
                case EVENT_BLESSED_GIFT:
                    if (Creature *pCouncillor = ObjectAccessor::GetCreature(*me, uiTargetGuid))
                    {
                        DoCast(me, SPELL_BLESSED_TIME_OUT);
                        me->GetMotionMaster()->MovementExpired();
                        me->GetMotionMaster()->MoveJump(*pCouncillor, 42.0f, 42.0f, EVENT_JUMP);
                    }
                    break;

                default:
                    break;
                }
            }
        }

        void MovementInform(uint32 uiMotionType, uint32 uiMotionPointId)
        {
            switch (uiMotionType)
            {
            case POINT_MOTION_TYPE:
                if (uiMotionPointId == POINT_BLESSED_LOA_SPIRIT_COUNCILLOR)
                {
                    if (Creature *pCouncillor = ObjectAccessor::GetCreature(*me, uiTargetGuid))
                    {
                        if (me->GetExactDist2d(pCouncillor) <= 5.f)
                        {
                            DoCast(pCouncillor, SPELL_BLESSED_GIFT);
                            me->DisappearAndDie();
                        }
                        else
                            events.RescheduleEvent(EVENT_MOVE_COUNCILLOR, 200);
                    }
                }
                break;

            case EFFECT_MOTION_TYPE:
                if (uiMotionPointId == EVENT_JUMP)
                {
                    if (Creature *pCouncillor = ObjectAccessor::GetCreature(*me, uiTargetGuid))
                    {
                        DoCast(pCouncillor, SPELL_BLESSED_GIFT);
                        me->DisappearAndDie();
                    }
                }
                break;

            default:
                break;
            }
        }

    private:
        EventMap        events;
        InstanceScript  *pInstance;
        uint64          uiTargetGuid; // GUID of the councillor we are moving toward
    };

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new mob_blessed_loa_spirit_AI(pCreature);
    }
};


// Shadowed Loa Spirit AI
class mob_shadowed_loa_spirit : public CreatureScript
{
public:
    mob_shadowed_loa_spirit() : CreatureScript("mob_shadowed_loa_spirit") { }

    class mob_shadowed_loa_spirit_AI : public ScriptedAI
    {
    public:
        mob_shadowed_loa_spirit_AI(Creature *pCreature) :
            ScriptedAI(pCreature), pInstance(pCreature->GetInstanceScript()), uiTargetGuid(0)
        {
            me->SetReactState(REACT_PASSIVE);
            events.Reset();
        }

        void Reset()
        {
            DoCast(me, SPELL_SHADOWED_TRANSFORMATION);
        }

        // Override function to be sure there won't be any call to MoveChase (at least in AttackStart)
        void AttackStart(Unit *pTarget) { } 

        void IsSummonedBy(Unit *pSummoner)
        {
            HandleTargetSelection();
        }

        void Move()
        {
            if (Player *pTarget = ObjectAccessor::GetPlayer(*me, uiTargetGuid))
            {
                if (pTarget->IsAlive())
                {
                    me->GetMotionMaster()->MovePoint(POINT_BLESSED_LOA_SPIRIT_COUNCILLOR, pTarget->GetPosition());
                    events.ScheduleEvent(EVENT_MOVE_COUNCILLOR, 500);
                }
                else
                {
                    if (Aura* pAura = pTarget->GetAura(SPELL_MARKED_SOUL, me->GetGUID()))
                        pAura->Remove(AURA_REMOVE_BY_DEATH);

                    HandleTargetSelection();
                }
            }
            else
                HandleTargetSelection();
        }

        void HandleTargetSelection()
        {
            DoCast(SPELL_MARKED_SOUL);

            std::list<Player*> players;
            GetPlayerListInGrid(players, me, 300.f);

            if (!players.empty())
            {
                for (auto const pPlayer : players)
                {
                    if (pPlayer->HasAura(SPELL_MARKED_SOUL, me->GetGUID()))
                    {
                        SetTargetGuid(pPlayer->GetGUID());
                        break;
                    }
                }
            }
            
            events.Reset();
            events.ScheduleEvent(EVENT_MOVE_COUNCILLOR, 500);
            events.ScheduleEvent(EVENT_SHADOWED_GIFT, 20 * IN_MILLISECONDS);
        }

        void SetTargetGuid(uint64 guid)
        {
            uiTargetGuid = guid;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            events.Update(uiDiff);

            while (uint32 uiEventId = events.ExecuteEvent())
            {
                switch (uiEventId)
                {
                case EVENT_MOVE_COUNCILLOR:
                    Move();
                    break;
                case EVENT_SHADOWED_GIFT:
                    if (Player* pTarget = ObjectAccessor::GetPlayer(*me, uiTargetGuid))
                    {
                        if (pTarget->IsAlive())
                        {
                            DoCast(me, SPELL_SHADOWED_TIME_OUT);
                            me->GetMotionMaster()->MovementExpired();
                            me->GetMotionMaster()->MoveJump(*pTarget, 42.0f, 42.0f, 5050);
                            return;
                        }
                    }
                    HandleTargetSelection();
                    break;

                default:
                    break;
                }
            }
        }

        void MovementInform(uint32 uiMotionType, uint32 uiMotionPointId)
        {
            if (uiMotionPointId == POINT_BLESSED_LOA_SPIRIT_COUNCILLOR)
            {
                if (Player* pTarget = ObjectAccessor::GetPlayer(*me, uiTargetGuid))
                {
                    if (me->GetExactDist2d(pTarget) <= 6.f && pTarget->IsAlive())
                    {
                        DoCast(pTarget, SPELL_SHADOWED_GIFT, true);
                        pTarget->RemoveAurasDueToSpell(SPELL_MARKED_SOUL, me->GetGUID());
                        me->DisappearAndDie();
                    }
                    else
                        events.RescheduleEvent(EVENT_MOVE_COUNCILLOR, 200);
                }
            }

            if (uiMotionPointId == 5050)
            {
                if (Player* pPlayer = ObjectAccessor::GetPlayer(*me, uiTargetGuid))
                {
                    DoCast(pPlayer, SPELL_SHADOWED_GIFT);
                    pPlayer->RemoveAurasDueToSpell(SPELL_MARKED_SOUL, me->GetGUID());
                    me->DisappearAndDie();
                }
            }

        }

        void JustDied(Unit *pKiller)
        {
            if (Player *pPlayer = ObjectAccessor::GetPlayer(*me, uiTargetGuid))
                pPlayer->RemoveAurasDueToSpell(SPELL_MARKED_SOUL);

            me->DespawnOrUnsummon(5000);
        }

    private:
        EventMap        events;
        InstanceScript  *pInstance;
        uint64          uiTargetGuid; // GUID of the councillor we are moving toward
    };
        

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new mob_shadowed_loa_spirit_AI(pCreature);
    }
};


// Helper to handle Twisted Fate correctly
class mob_twisted_fate_helper : public CreatureScript
{
public:
    mob_twisted_fate_helper() : CreatureScript("mob_twisted_fate_helper") { }

    struct TwistedFate
    {
        uint64 uiFirstPlayerGuid; // Guid of the first player of the Twisted Fate
        uint64 uiSecondPlayerGuid; // Guid of the second player of the Twisted Fate

        uint64 uiFirstTwistedFateGuid; // Guid of the first Twisted Fate
        uint64 uiSecondTwistedFateGuid; // Guid of the second Twisted Fate

        TwistedFate(uint64 uiFirstPlayer, uint64 uiFirstNpc) :
            uiFirstPlayerGuid(0), uiFirstTwistedFateGuid(uiFirstNpc)
        {
            uiSecondPlayerGuid = 0;
            uiSecondTwistedFateGuid = 0;
        }
    };

    class mob_twisted_fate_helper_AI : public ScriptedAI
    {
    public:
        mob_twisted_fate_helper_AI(Creature *pCreature) :
            ScriptedAI(pCreature), pInstance(pCreature->GetInstanceScript())
        {
            twistedFatesList.clear();
        }
    
        void Reset()
        {
            twistedFatesList.clear();
        }

        void AddTwistedFate(TwistedFate *pFate)
        {
            twistedFatesList.push_back(pFate);
        }

        // Link a player and a twisted fate (only used for the second mob)
        void Link(uint64 uiPlayerGuid, uint64 uiFateGuid)
        {
            if (twistedFatesList.empty())
                return;

            // Link the two twisted fates together == send the guid of the other to each one,
            // init them correctly, start their movement toward each other.
            if (TwistedFate* pFate = twistedFatesList.back())
            {
                pFate->uiSecondPlayerGuid = uiPlayerGuid;
                pFate->uiSecondTwistedFateGuid = uiFateGuid;

                Creature    *pFirst     = ObjectAccessor::GetCreature(*me, pFate->uiFirstTwistedFateGuid),
                            *pSecond    = ObjectAccessor::GetCreature(*me, pFate->uiSecondTwistedFateGuid);

                if (!pFirst || !pSecond)
                    return;

                // If there are not enough players, remove the creatures
                if (pFate->uiSecondPlayerGuid == pFate->uiFirstPlayerGuid)
                {
                    pFirst->DisappearAndDie();
                    pSecond->DisappearAndDie();
                    return;
                }
                
                pFirst->SetFacingToObject(pSecond);
                // I don't know how they are supposed to behave, so I will just say they meet at the exact middle
                // of the two positions they represent.
                Position const middlePosition = { (pFirst->GetPositionX() + pSecond->GetPositionX()) / 2.0f,
                                                  (pFirst->GetPositionY() + pSecond->GetPositionY()) / 2.0f,
                                                  pFirst->GetPositionZ(), pFirst->GetOrientation() };

                // Send the guid of each to the other
                pFirst->AI()->SetGUID(pSecond->GetGUID(), DATA_TWISTED_FATE_GUID);
                pSecond->AI()->SetGUID(pFirst->GetGUID(), DATA_TWISTED_FATE_GUID);

                // Make them move
                pFirst->GetMotionMaster()->MovePoint(POINT_MIDDLE, middlePosition);
                pSecond->GetMotionMaster()->MovePoint(POINT_MIDDLE, middlePosition);

                // And make them cast their periodic
                pFirst->CastSpell(pFirst, SPELL_TWISTED_FATE_PERIODIC);
                pSecond->CastSpell(pSecond, SPELL_TWISTED_FATE_PERIODIC);
            }
        }

        // Delete a TwistedFate when first and second are dead
        void Unlink(uint64 uiGuid)
        {
            if (twistedFatesList.empty())
                return;

            for(TwistedFate *pFate : twistedFatesList)
            {
                if (pFate->uiFirstTwistedFateGuid == uiGuid || pFate->uiSecondTwistedFateGuid == uiGuid)
                {
                    delete pFate;
                    return;
                }
            }
        }

        void DoAction(const int32 iAction)
        {
            switch(iAction)
            {
            // Fight ended, delete dynamically allocated memory
            case ACTION_TWISTED_FATE_END_FIGHT:
                for(auto i: twistedFatesList)
                    delete i;
                break;

            default:
                break;
            }
        }

    private:
        InstanceScript  *pInstance;

        std::list<TwistedFate*> twistedFatesList; // Store dynamically allocated twisted fate helpers
    };

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new mob_twisted_fate_helper_AI(pCreature);
    }
};
typedef mob_twisted_fate_helper::mob_twisted_fate_helper_AI TwistedFateHelperAI;
typedef mob_twisted_fate_helper::TwistedFate TwistedFate_t;


// Twisted Fate AI
class mob_twisted_fate : public CreatureScript
{
public:
    mob_twisted_fate() : CreatureScript("mob_twisted_fate") { }

    class mob_twisted_fate_AI : public ScriptedAI
    {
    public:
        mob_twisted_fate_AI(Creature *pCreature) :
            ScriptedAI(pCreature), pInstance(pCreature->GetInstanceScript())
        {
            me->SetReactState(REACT_PASSIVE);
        }

        // Add a new TwistedFate_t to the helper
        void IsSummonedBy(Unit *pSummoner)
        {
            Creature *pHelper = ObjectAccessor::GetCreature(*me, pInstance->GetData64(NPC_TWISTED_FATE_HELPER));
            if (!pHelper)
            {
                me->DisappearAndDie();
                return;
            }

            TwistedFateHelperAI *pHelperAI = dynamic_cast<TwistedFateHelperAI*>(pHelper->AI());
            if (!pHelperAI)
            {
                me->DisappearAndDie();
                return;
            }

            switch (me->GetEntry())
            {
                // Create a new TwistedFate_t
            case MOB_TWISTED_FATE_FIRST:
                pHelperAI->AddTwistedFate(new TwistedFate_t(pSummoner ? pSummoner->GetGUID() : uint64(0), me->GetGUID()));
                DoCastAOE(SPELL_TWISTED_FATE_FORCE_SUMMON_SECOND); // Force the most distant player to summon the second twisted fate
                break;

                // Finalize it with the second npc and launch everything
            case MOB_TWISTED_FATE_SECOND:
                pHelperAI->Link(pSummoner ? pSummoner->GetGUID() : uint64(0), me->GetGUID());
                break;
            }
        }

        void SetGUID(uint64 uiGuid, int32 uiIndex)
        {
            if (uiIndex == DATA_TWISTED_FATE_GUID)
                uiOtherTwistedFateGuid = uiGuid;
        }

        void JustDied(Unit *pKiller)
        {
            // Do not do something wierd when the other is dead => free memory
            // of the helper.
            if (bOtherTwistedFateDied)
            {
                if (Creature *pHelper = ObjectAccessor::GetCreature(*me, pInstance->GetData64(NPC_TWISTED_FATE_HELPER)))
                {
                    if (TwistedFateHelperAI *pHelperAI = dynamic_cast<TwistedFateHelperAI*>(pHelper->AI()))
                        pHelperAI->Unlink(me->GetGUID());
                }
            }
            else
            {
                // When a Twisted Fate dies, the other stops and then begins to cast an AOE every 3 seconds
                if (Creature *pOther = ObjectAccessor::GetCreature(*me, uiOtherTwistedFateGuid))
                    pOther->AI()->DoAction(ACTION_OTHER_TWISTED_FATE_DIED);
            }

        }

        uint32 GetData(uint32 uiIndex)
        {
            if (uiIndex == DATA_TWISTED_FATE_OTHER_DIED)
                return (uint32)bOtherTwistedFateDied;

            return 0;
        }

        void DoAction(const int32 iAction)
        {
            switch (iAction)
            {
                // Stop moving and set bOtherTwistedFateDied to true so the aura script will be able
                // to compute the amount of damages correctly.
            case ACTION_OTHER_TWISTED_FATE_DIED:
                me->StopMoving();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                bOtherTwistedFateDied = true;
                break;

            default:
                break;
            }
        }

    protected:
        InstanceScript  *pInstance;
        uint64          uiOtherTwistedFateGuid; // Guid of the other twisted fate

        // Boolean to control whenever we are in the linked phase or not,
        // and used to compute correctly the amount of damages of the
        // SPELL_TWISTED_FATE_DAMAGES.
        bool            bOtherTwistedFateDied;
    };

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new mob_twisted_fate_AI(pCreature);
    }
};


//=========================================================
// Spell Scripts


// Possessed
class spell_garajal_possessed : public SpellScriptLoader
{
public:
    spell_garajal_possessed() : SpellScriptLoader("spell_garajal_possessed") { }

    class spell_garajal_possessed_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_garajal_possessed_AuraScript)

        void HandleApply(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
        {
            if (GetOwner() && GetOwner()->ToCreature())
            {
                Creature *pOwner = GetOwner()->ToCreature();
                pOwner->AI()->DoAction(ACTION_SET_POSSESSED);
            }
        }

        void HandleRemove(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
        {
            if (GetOwner())
            {
                if (GetOwner()->ToCreature())
                    GetOwner()->ToCreature()->AI()->DoAction(ACTION_SET_UNPOSSESSED);
            }
        }

        /*void HandlePeriodic(AuraEffect const* pAuraEffect)
        {
            if (!GetOwner())
                return;
                
            if (Creature *pOwner = GetOwner()->ToCreature())
            {
                if (pOwner->GetPower(POWER_ENERGY) < 100)
                    pOwner->ModifyPower(POWER_ENERGY, 1);
                    
                if (pOwner->GetPower(POWER_ENERGY) == 100)
                    pOwner->AI()->DoAction(ACTION_DARK_POWER);
            }
        }*/
        
        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_garajal_possessed_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_garajal_possessed_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            // OnEffectPeriodic    += AuraEffectPeriodicFn(spell_garajal_possessed_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_garajal_possessed_AuraScript();
    }
};


// Frigid Assault
class spell_malakk_frigid_assault : public SpellScriptLoader
{
public:
    spell_malakk_frigid_assault() : SpellScriptLoader("spell_malakk_frigid_assault") { }

    class spell_malakk_frigid_assault_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_malakk_frigid_assault_AuraScript)

        void HandleEffectApply(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
        {
            Player *pOwner;
            if (GetOwner() && GetOwner()->ToPlayer())
            {
                pOwner = GetOwner()->ToPlayer();
                // Stun when stacks reach 15
                if (GetStackAmount() == 15)
                {
                    pOwner->CastSpell(pOwner, SPELL_FRIGID_ASSAULT_STUN, true);
                    Remove(AURA_REMOVE_BY_DEFAULT);
                }
            }
        }

        void Register()
        {
            // BTW: Periodic dummy choice was quite surprising...
            // Note: better to use AfterEffectApply, cause stack amount is not yet set to 15 when OnEffectApply is called
            AfterEffectApply += AuraEffectApplyFn(spell_malakk_frigid_assault_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK); // Indeed, it should be AfterEffectReapply
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_malakk_frigid_assault_AuraScript();
    }
};


// Biting Cold
class spell_malakk_biting_cold : public SpellScriptLoader
{
public:
    spell_malakk_biting_cold() : SpellScriptLoader("spell_malakk_biting_cold") { }

    class spell_malakk_biting_cold_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malakk_biting_cold_SpellScript)

        void HandleEffectHitTarget(SpellEffIndex effIndex)
        {
            // Since spell selects one random target, there is no way we can handle this another way
            if (Unit *pHit = GetHitUnit())
            {
                if (Unit *pCaster = GetCaster())
                {
                    pCaster->CastSpell(pHit, SPELL_BITING_COLD_PERIODIC_DAMAGES, true);
                    pCaster->CastSpell(pHit, SPELL_BITING_COLD_PERIODIC_DUMMY, true);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_malakk_biting_cold_SpellScript::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_malakk_biting_cold_SpellScript();
    }
};


// Frostbite (Periodic)
class spell_malakk_frostbite_periodic : public SpellScriptLoader
{
public:
    spell_malakk_frostbite_periodic() : SpellScriptLoader("spell_malakk_frostbite_periodic") { }

    class spell_malakk_frostbite_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_malakk_frostbite_periodic_AuraScript)

        void HandlePeriodic(AuraEffect const* pAuraEffect)
        {
            if (!GetOwner())
                return;

            // Stack amount can be reduced when players are standing close to the owner
            if (Player *pOwner = GetOwner()->ToPlayer())
            {
                std::list<Player*> playerList;
                pOwner->GetPlayerListInGrid(playerList, 4.0f);

                playerList.remove(pOwner); // Remove self
                playerList.remove_if([](Player const* pPlayer) -> bool { return pPlayer->HasAura(SPELL_CHILLED_TO_THE_BONE) ; }); // Remove players with Chilled to the Bone

                uint32 uiReduceAmount = 0;

                switch (pOwner->GetMap()->GetDifficulty())
                {
                    // In 25-man raid, the amount is 1 * number of players within 4 yards
                case MAN25_HEROIC_DIFFICULTY:
                case MAN25_DIFFICULTY:
                    uiReduceAmount = playerList.size() > 4 ? 4 : playerList.size();
                    break;

                    // Otherwise it is 2 * number of players within 4 yards
                default:
                    uiReduceAmount = playerList.size() > 2 ? 4 : playerList.size() * 2;
                    break;
                }

                // And the amount can't be reduced below 1
                if (GetStackAmount() <= uiReduceAmount) // Indeed, if the update was done in real time, this should never happen
                    SetStackAmount(1);
                else
                    SetStackAmount(5 - uiReduceAmount);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_malakk_frostbite_periodic_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_malakk_frostbite_periodic_AuraScript();
    }
};


// Frostbite (main damages)
class spell_malakk_frostbite : public SpellScriptLoader
{
public:
    spell_malakk_frostbite() : SpellScriptLoader("spell_malakk_frostbite") { }

    class spell_malakk_frostbite_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malakk_frostbite_SpellScript)

        // Handler to select target (cause TARGET_UNIT_SRC_AREA_ENTRY doesn't work fine)
        void SelectTarget(std::list<WorldObject*>& targets)
        {
            targets.clear();
            if (Unit *pCaster = GetCaster())
            {
                std::list<Player*> playerList;
                pCaster->GetPlayerListInGrid(playerList, 500.0f);

                if (!playerList.empty())
                    targets.push_back(Trinity::Containers::SelectRandomContainerElement(playerList));
            }
        }

        // Handler to apply the visual
        void HandleOnHit(SpellEffIndex eEffIndex)
        {
            if (Unit *pHit = GetHitUnit())
            {
                if (Unit *pCaster = GetCaster())
                {
                    pHit->CastSpell(pHit, SPELL_FROSTBITE_SCREEN_EFFECT, true);
                    pCaster->CastSpell(pHit, SPELL_FROSTBITE_PERIODIC_DAMAGES, true);

                    if (pCaster->ToCreature()->AI())
                        pCaster->ToCreature()->AI()->Talk(TALK_MALAKK_FROSTBITE, pHit->GetGUID());
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_malakk_frostbite_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_malakk_frostbite_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_malakk_frostbite_SpellScript();
    }
};

// Frostbite (allies damages)
class spell_malakk_frostbite_allies : public SpellScriptLoader
{
public:
    spell_malakk_frostbite_allies() : SpellScriptLoader("spell_malakk_frostbite_allies") { }

    class spell_malakk_frostbite_allies_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malakk_frostbite_allies_SpellScript)
        
        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(notPlayerPredicate());
        }

        int32 value;

        bool Load()
        {
            if (GetCaster())
            {
                value = GetCaster()->GetMap()->IsHeroic() ? 60000 : 22000;
                return true;
            }
            return false;
        }

        void HandleEffectHitTarget(SpellEffIndex eff_idx)
        {
            Unit* pTarget = GetHitUnit();
            Unit* pCaster = GetCaster();

            if (!pTarget || !pCaster)
                return;

            if (Aura* pAura = pCaster->GetAura(SPELL_FROSTBITE_PERIODIC_DAMAGES))
            {
                SetHitDamage(pAura->GetStackAmount() * value);
            }
        }

        void HandleOnCast()
        {
            // Cast Body Heat on heroic
            if (Unit *pCaster = GetCaster())
            {
                if (pCaster->GetMap()->IsHeroic())
                    pCaster->CastSpell(pCaster, SPELL_BODY_HEAT, true);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_malakk_frostbite_allies_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_malakk_frostbite_allies_SpellScript::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            OnCast += SpellCastFn(spell_malakk_frostbite_allies_SpellScript::HandleOnCast);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_malakk_frostbite_allies_SpellScript();
    }
};


// Body Heat
class spell_malakk_body_heat : public SpellScriptLoader
{
public:
    spell_malakk_body_heat() : SpellScriptLoader("spell_malakk_body_heat") { }

    class spell_malakk_body_heat_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malakk_body_heat_SpellScript)

        void SelectTargets(std::list<WorldObject*>& targets)
        {
            std::list<Player*> playerList;
            if (GetCaster())
                GetCaster()->GetPlayerListInGrid(playerList, 4.0f);

            if (!playerList.empty() && playerList.size() > 1)
            {
                targets.clear();
                playerList.remove(GetCaster()->ToPlayer());

                for (Player* iter : playerList)
                    targets.push_back(iter);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_malakk_body_heat_SpellScript::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_malakk_body_heat_SpellScript();
    }

    class spell_malakk_body_heat_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_malakk_body_heat_AuraScript)

        void HandleEffectRemove(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
        {
            if (!GetOwner())
                return;

            if (Player *pOwner = GetOwner()->ToPlayer())
                pOwner->CastSpell(pOwner, SPELL_CHILLED_TO_THE_BONE, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_malakk_body_heat_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_malakk_body_heat_AuraScript();
    }
};


// Reckless Charge
class spell_kazrajin_reckless_charge : public SpellScriptLoader
{
public:
    spell_kazrajin_reckless_charge() : SpellScriptLoader("spell_kazrajin_reckless_charge") { }

    class spell_kazrajin_reckless_charge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kazrajin_reckless_charge_SpellScript)

        void HandleCast(SpellEffIndex eff_idx)
        {
            if (Unit *pCaster = GetCaster())
            {
                if (Creature *pKazrajin = pCaster->ToCreature())
                {
                    std::list<Player*> players;
                    GetPlayerListInGrid(players, pKazrajin, 100.f);

                    for (auto const pPlayer : players)
                    {
                        if (pPlayer->HasAura(SPELL_RECKLESS_CHARGE_FACE, pKazrajin->GetGUID()))
                        {
                            pKazrajin->CastSpell(pKazrajin, SPELL_RECKLESS_CHARGE_SOUND, true);
                            pPlayer->RemoveAurasDueToSpell(SPELL_RECKLESS_CHARGE_FACE);
                            // Compute position of landing
                            float fDist = pKazrajin->GetExactDist2d(pPlayer) - 3.f; // Remove 5 yards to continue rolling
                            float fAngle = pKazrajin->GetAngle(pPlayer);
                            float fX, fY;
                            GetPositionWithDistInOrientation(pKazrajin, fDist, fAngle, fX, fY);

                            uint8 m_pointCount = ((uint8)fDist / 3) + 1;

                            for (uint8 i = 0; i < m_pointCount; ++i)
                            {
                                float x, y;
                                GetPositionWithDistInOrientation(pKazrajin, fDist - (i * 3), fAngle, x, y);
                                pKazrajin->CastSpell(x, y, pKazrajin->GetMap()->GetHeight(x, y, pKazrajin->GetPositionZ() + 0.8f), SPELL_RECKLESS_CHARGE_GROUND_AT, true);
                            }

                            if (pKazrajin->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
                                pKazrajin->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                            pKazrajin->GetMotionMaster()->MoveCharge(fX, fY, pPlayer->GetPositionZ(), 42.f, POINT_RECKLESS_CHARGE_LAND);
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_kazrajin_reckless_charge_SpellScript::HandleCast, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kazrajin_reckless_charge_SpellScript();
    }
};

class withinRangePredicate
{
public:
    withinRangePredicate(Unit* _caster) : caster(_caster) {}

    bool operator()(WorldObject* target) const
    {
        if (target && target->GetExactDist2d(caster) < 9.f)
            return true;
        return false;
    }

private:
    Unit* caster;
};

class spell_kazrajin_reckless_charge_targeting : public SpellScriptLoader
{
public:
    spell_kazrajin_reckless_charge_targeting() : SpellScriptLoader("spell_kazrajin_reckless_charge_targeting") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            std::list<WorldObject*>tempTargets;
            std::copy(std::begin(targets), std::end(targets), std::inserter(tempTargets, tempTargets.begin()));

            if (Unit* caster = GetCaster())
            {
                targets.remove_if(withinRangePredicate(caster));
            }

            if (targets.size() > 1)
                Trinity::Containers::RandomResizeList(targets, 1);
            else if (targets.empty())
                std::copy(std::begin(tempTargets), std::end(tempTargets), std::inserter(targets, targets.begin()));
        }

        void HandleEffectHitTarget(SpellEffIndex eff_idx)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();

            if (!caster || !target)
                return;

            caster->AddAura(SPELL_RECKLESS_CHARGE_FACE, target);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_impl::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

// Overload
class spell_kazrajin_overload : public SpellScriptLoader
{
public:
    spell_kazrajin_overload() : SpellScriptLoader("spell_kazrajin_overload") { }

    class spell_kazrajin_overload_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kazrajin_overload_AuraScript)

        void HandleEffectApply(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
        {
            if (!GetOwner())
                return;

            // Stunned on aura apply
            if (Creature *pOwner = GetOwner()->ToCreature())
            {
                pOwner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                pOwner->SetControlled(true, UNIT_STATE_STUNNED);
            }
        }

        void HandleOnProc(ProcEventInfo& rProcInfo)
        {
            if (Unit *pCaster = rProcInfo.GetActor())
            {
                if (!rProcInfo.GetDamageInfo())
                    return;

                int32 uiDamages = rProcInfo.GetDamageInfo()->GetDamage() * 0.4f; // 40% of damages returned to the player

                if (Unit *pVictim = rProcInfo.GetActionTarget())
                {
                    pVictim->CastCustomSpell(pCaster, SPELL_OVERLOAD_DAMAGES, &uiDamages, NULL, NULL, NULL, NULL, NULL, true);
                    pVictim->CastSpell(pCaster, SPELL_OVERLOAD_VISUAL, true);
                }
            }
        }

        void HandleEffectRemove(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
        {
            if (!GetOwner())
                return;

            // Unstunned on aura remove
            if (Creature *pOwner = GetOwner()->ToCreature())
                pOwner->SetControlled(false, UNIT_STATE_STUNNED);
        }

        void Register()
        {
            // Note: there is no stunning spell, I'm quite sure of that... even if
            // it is really wierd.
            OnEffectApply += AuraEffectApplyFn(spell_kazrajin_overload_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnProc += AuraProcFn(spell_kazrajin_overload_AuraScript::HandleOnProc);
            OnEffectRemove += AuraEffectRemoveFn(spell_kazrajin_overload_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_kazrajin_overload_AuraScript();
    }
};


// Discharge
class spell_kazrajin_discharge : public SpellScriptLoader
{
public:
    spell_kazrajin_discharge() : SpellScriptLoader("spell_kazrajin_discharge") { }

    class spell_kazrajin_discharge_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kazrajin_discharge_AuraScript)

        void HandleEffectApply(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
        {
            if (!GetOwner())
                return;

            // Stunned on aura apply
            if (Creature *pOwner = GetOwner()->ToCreature())
                pOwner->SetControlled(true, UNIT_STATE_STUNNED);
        }

        void HandlePeriodic(AuraEffect const* pAuraEffect)
        {
            if (!GetOwner())
                return;

            if (Creature *pOwner = GetOwner()->ToCreature())
            {
                int32 uiDamagesInPastSecs = (int32)pOwner->AI()->GetData(DATA_DAMAGES_PAST_SEC) * 0.05f; // 5% of damages taken in past sec
                pOwner->AI()->DoAction(ACTION_RESET_DAMAGES);

                pOwner->CastCustomSpell(pOwner, SPELL_DISCHARGE_DAMAGES, &uiDamagesInPastSecs, NULL, NULL, NULL, NULL, NULL, true);
                pOwner->AI()->DoCastAOE(SPELL_DISCHARGE_VISUAL);
            }
        }

        void HandleEffectRemove(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
        {
            if (!GetOwner())
                return;

            // Unstunned on aura remove
            if (Creature *pOwner = GetOwner()->ToCreature())
                pOwner->SetControlled(false, UNIT_STATE_STUNNED);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_kazrajin_discharge_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_kazrajin_discharge_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            OnEffectRemove += AuraEffectRemoveFn(spell_kazrajin_discharge_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_kazrajin_discharge_AuraScript();
    }
};


// Ensnared / Entrapped (the most fucked up spell ever made in WoW) (handled through Quicksand)
class spell_quicksand_periodic : public SpellScriptLoader
{
public:
    spell_quicksand_periodic() : SpellScriptLoader("spell_quicksand_periodic") { }

    class spell_quicksand_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_quicksand_periodic_AuraScript)

        void HandlePeriodic(AuraEffect const* pAuraEffect)
        {
            Unit* caster = GetCaster();
            Unit* pTarget = GetOwner()->ToUnit();

            if (!caster || !pTarget)
                return;

            if (Aura* pAura = pTarget->GetAura(SPELL_ENSNARED))
            {
                if (pAura->GetCasterGUID() == caster->GetGUID())
                {
                    pAura->ModStackAmount(1);
                    if (pAura->GetStackAmount() >= 5)
                    {
                        caster->AddAura(SPELL_ENTRAPPED, pTarget);
                        pAura->Remove();
                    }
                }
            }
            else if (!pTarget->HasAura(SPELL_ENTRAPPED))
                caster->AddAura(SPELL_ENSNARED, pTarget);
        }

        void HandleOnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            Unit* caster = GetCaster();
            Unit* pTarget = GetOwner()->ToUnit();

            if (!caster || !pTarget)
                return;

            pTarget->RemoveAura(SPELL_ENSNARED, caster->GetGUID());

        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_quicksand_periodic_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            OnEffectRemove += AuraEffectRemoveFn(spell_quicksand_periodic_AuraScript::HandleOnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_quicksand_periodic_AuraScript();
    }
}; 


// Entrapped
class spell_quicksand_entrapped : public SpellScriptLoader
{
public:
    spell_quicksand_entrapped() : SpellScriptLoader("spell_quicksand_entrapped") { }

    class spell_quicksand_entrapped_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_quicksand_entrapped_SpellScript)

        void HandleAdditionalSpell(SpellEffIndex eEffIndex)
        {
            PreventHitDefaultEffect(eEffIndex);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_quicksand_entrapped_SpellScript::HandleAdditionalSpell, EFFECT_1, SPELL_EFFECT_203);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_quicksand_entrapped_SpellScript();
    }
};


// Sandstorm
class spell_sul_sandstorm : public SpellScriptLoader
{
    enum : uint32
    {
        SANDSTORM_VISUAL    = 136895
    };
public:
    spell_sul_sandstorm() : SpellScriptLoader("spell_sul_sandstorm") { }

    class spell_sul_sandstorm_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sul_sandstorm_SpellScript)

        void HandleAfterCast()
        {
            if (Unit *pCaster = GetCaster())
            {
                pCaster->AddAura(SANDSTORM_VISUAL, pCaster);

                //std::list<Creature*> quicksandsList;
                std::list<Creature*> livingSandsList;

                //pCaster->GetCreatureListWithEntryInGrid(quicksandsList, NPC_QUICKSAND_STALKER, 500.0f);
                pCaster->GetCreatureListWithEntryInGrid(livingSandsList, MOB_LIVING_SAND, 500.0f);

                /*
                for (Creature *pQuicksand : quicksandsList)
                pQuicksand->AI()->DoAction(ACTION_CREATE_LIVING_SAND);*/

                for (Creature *pLivingSand : livingSandsList)
                    pLivingSand->AI()->DoAction(ACTION_CREATE_LIVING_SAND);
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_sul_sandstorm_SpellScript::HandleAfterCast);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_sul_sandstorm_SpellScript();
    }
};


// Summon Blessed Loa Spirit
class spell_marli_summon_blessed_loa_spirit : public SpellScriptLoader
{
public:
    spell_marli_summon_blessed_loa_spirit() : SpellScriptLoader("spell_marli_summon_blessed_loa_spirit") { }

    class spell_marli_summon_blessed_loa_spirit_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_marli_summon_blessed_loa_spirit_SpellScript)

        void HandleDummy(SpellEffIndex eEffIndex)
        {
            if (Unit *pCaster = GetCaster())
            {
                pCaster->CastSpell(pCaster, SPELL_SUMMON_BLESSED_LOA_SPIRIT, false);

                if (pCaster->ToCreature()->AI())
                    pCaster->ToCreature()->AI()->Talk(TALK_MARLI_BLESSED);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_marli_summon_blessed_loa_spirit_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_marli_summon_blessed_loa_spirit_SpellScript();
    }
};


// Summon Shadowed Loa Spirit
class spell_marli_summon_shadowed_loa_spirit : public SpellScriptLoader
{
public:
    spell_marli_summon_shadowed_loa_spirit() : SpellScriptLoader("spell_marli_summon_shadowed_loa_spirit") { }

    class spell_marli_summon_shadowed_loa_spirit_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_marli_summon_shadowed_loa_spirit_SpellScript)

        void HandleDummy(SpellEffIndex eEffIndex)
        {
            if (Unit *pCaster = GetCaster())
            {
                pCaster->CastSpell(pCaster, SPELL_SUMMON_SHADOWED_LOA_SPIRIT, false);

                if (pCaster->ToCreature()->AI())
                    pCaster->ToCreature()->AI()->Talk(TALK_MARLI_SHADOWED);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_marli_summon_shadowed_loa_spirit_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_marli_summon_shadowed_loa_spirit_SpellScript();
    }
};


// Twisted Fate (first)
class spell_marli_twisted_fate_first : public SpellScriptLoader
{
public:
    spell_marli_twisted_fate_first() : SpellScriptLoader("spell_marli_twisted_fate_first") { }

    class spell_marli_twisted_fate_first_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_marli_twisted_fate_first_SpellScript)

        void SelectTarget(std::list<WorldObject*>& targets)
        {
            if (!GetCaster())
                return;

            // Select one random player...
            std::list<Player*> playerList;
            GetCaster()->GetPlayerListInGrid(playerList, 500.0f);
            if (!playerList.empty())
            {
                targets.clear();
                targets.push_back(Trinity::Containers::SelectRandomContainerElement(playerList));
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_marli_twisted_fate_first_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_marli_twisted_fate_first_SpellScript();
    }
};


// Twisted Fate (second)
class spell_marli_twisted_fate_second : public SpellScriptLoader
{
public:
    spell_marli_twisted_fate_second() : SpellScriptLoader("spell_marli_twisted_fate_second") { }

    class spell_marli_twisted_fate_second_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_marli_twisted_fate_second_SpellScript)

        void SelectTarget(std::list<WorldObject*>& targets)
        {
            if (!GetCaster())
                return;

            // Find the fathest player
            std::list<Player*> playerList;
            GetCaster()->GetPlayerListInGrid(playerList, 500.0f);
            playerList.sort(Trinity::DistanceCompareOrderPred(GetCaster(), false));

            if (!playerList.empty() && playerList.front())
            {
                targets.clear();
                targets.push_back(playerList.front());
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_marli_twisted_fate_second_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_marli_twisted_fate_second_SpellScript();
    }
};


// Twisted Fate (Periodic Damages)
class spell_marli_twisted_fate_damages : public SpellScriptLoader
{
public:
    spell_marli_twisted_fate_damages() : SpellScriptLoader("spell_marli_twisted_fate_damages") { }

    class spell_marli_twisted_fate_damages_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_marli_twisted_fate_damages_AuraScript)

        void CalculateAmount(AuraEffect const* pAuraEffect, int32 &ruiAmount, bool &rbCanBeRecalculated)
        {
            if (!GetOwner())
                return;

            if (Creature *pOwner = GetOwner()->ToCreature())
            {
                if (Creature *pTwistedFateTarget = ObjectAccessor::GetCreature(*pOwner, pOwner->AI()->GetGUID(DATA_TWISTED_FATE_GUID)))
                {
                    if (pTwistedFateTarget->IsAlive())
                    {
                        // The max amount is 250000, but it reduces the father the twisted fates are from
                        // each other. Let's say the maximum dist between them is 55 yards. If we want this
                        // to be pseudo linear, we'll say that with each yard between the twisted fates, we'll
                        // reduce this amount by 100 / 55. 
                        float fDist = pOwner->GetExactDist2d(pTwistedFateTarget);
                        ruiAmount = 250000 - CalculatePct(250000, fDist * 100 / 55);
                    }
                }
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_marli_twisted_fate_damages_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_marli_twisted_fate_damages_AuraScript();
    }
};


// Dark Power
class spell_dark_power : public SpellScriptLoader
{
public:
    spell_dark_power() : SpellScriptLoader("spell_dark_power") { }
    
    class spell_dark_power_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dark_power_SpellScript)
        
        void HandleHitTarget(SpellEffIndex eEffectIndex)
        {
            if (Unit *pUnit = GetCaster())
            {
                if (Creature *pCaster = pUnit->ToCreature())
                {
                    if (CreatureAI *pAI = pCaster->AI())
                    {
                        uint32 const uiDarkPowerCount = pAI->GetData(DATA_DARK_POWER_COUNT);
                        int32 iCustomValue = GetSpellInfo()->Effects[(uint32)eEffectIndex].BasePoints;

                        // Add 10% for each stack of Dark Power
                        for (uint32 i = 0; i < uiDarkPowerCount; ++i)
                            iCustomValue *= 1.1f;

                        SetHitDamage(iCustomValue);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dark_power_SpellScript::HandleHitTarget, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };
    
    SpellScript *GetSpellScript() const
    {
        return new spell_dark_power_SpellScript();
    }
};


// Soul Fragment Target Selector
class spell_soul_fragment_target_selector : public SpellScriptLoader
{
public:
    spell_soul_fragment_target_selector() : SpellScriptLoader("spell_soul_fragment_target_selector") { }

    class spell_soul_fragment_target_selector_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_soul_fragment_target_selector_SpellScript)

        void SelectTarget(std::list<WorldObject*>& targets)
        {
            if (!GetCaster())
                return;

            std::list<Player*> playerList;
            GetCaster()->GetPlayerListInGrid(playerList, 500.0f);

            if (!playerList.empty())
            {
                targets.clear();
                targets.push_back(Trinity::Containers::SelectRandomContainerElement(playerList));
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_soul_fragment_target_selector_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_soul_fragment_target_selector_SpellScript();
    }
};


// Soul Fragment Switcher
class spell_soul_fragment_switcher : public SpellScriptLoader
{
public:
    spell_soul_fragment_switcher() : SpellScriptLoader("spell_soul_fragment_switcher") { }

    class spell_soul_fragment_switcher_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_soul_fragment_switcher_SpellScript)

        void HandleHitTarget(SpellEffIndex eEffIndex)
        {
            if (Unit *pCaster = GetCaster())
            {
                if (Aura *pSoulFragment = pCaster->GetAura(SPELL_SOUL_FRAGMENT_PERIODIC))
                    pSoulFragment->Remove();
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_soul_fragment_switcher_SpellScript::HandleHitTarget, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_soul_fragment_switcher_SpellScript();
    }
};

class distancePredicate
{
private:
    Unit* caster;
public:
    distancePredicate(Unit* _caster) : caster(_caster) {}

    bool operator()(WorldObject* target) const
    {
        return target->GetExactDist2d(caster) < 15.f;
    }
};

class spell_marked_soul : public SpellScriptLoader
{
public:
    spell_marked_soul() : SpellScriptLoader("spell_marked_soul") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            std::list<WorldObject*> tempTargets;
            targets.remove_if(notPlayerPredicate());
            std::copy(std::begin(targets), std::end(targets), std::inserter(tempTargets, tempTargets.begin()));
            targets.remove_if(distancePredicate(GetCaster()));

            if (!targets.empty())
            {
                if (targets.size() > 1)
                    Trinity::Containers::RandomResizeList(targets, 1);
            }
            else
            {
                std::copy(std::begin(tempTargets), std::end(tempTargets), std::inserter(targets, targets.begin()));

                if (targets.size() > 1)
                    Trinity::Containers::RandomResizeList(targets, 1);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

class correctGuidPredicate
{
private:
    uint64 casterGuid;
public:
    correctGuidPredicate(uint64 guid) : casterGuid(guid) {}

    bool operator()(WorldObject* target) const
    {
        if (target)
        {
            return !target->ToUnit()->HasAura(SPELL_MARKED_SOUL, casterGuid);
        }
        return false;
    }
};

class spell_shadowed_gift : public SpellScriptLoader
{
public:
    spell_shadowed_gift() : SpellScriptLoader("spell_shadowed_gift") {}

    class spell_impl : public SpellScript
    {
        PrepareSpellScript(spell_impl);

        void SelectTargets(std::list<WorldObject*>&targets)
        {
            targets.remove_if(notPlayerPredicate());

            if (Unit* caster = GetCaster())
                targets.remove_if(correctGuidPredicate(caster->GetGUID()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_impl::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_impl();
    }
};

void AddSC_boss_council_of_elders()
{
    new boss_frost_king_malakk();
    new boss_kazrajin();
    new boss_sul_the_sandcrawler();
    new boss_high_priestess_marli();
    new mob_garajal();
    new mob_garajals_soul();
    new mob_living_sand();
    new mob_blessed_loa_spirit();
    new mob_shadowed_loa_spirit();
    new mob_twisted_fate_helper();
    new mob_twisted_fate();
    new spell_garajal_possessed();
    new spell_malakk_frigid_assault();
    new spell_malakk_biting_cold();
    new spell_malakk_frostbite();
    new spell_malakk_frostbite_periodic();
    new spell_malakk_frostbite_allies();
    new spell_malakk_body_heat();
    new spell_kazrajin_reckless_charge();
    new spell_kazrajin_reckless_charge_targeting();
    new spell_kazrajin_overload();
    new spell_kazrajin_discharge();
    new spell_quicksand_periodic();
    new spell_quicksand_entrapped();
    new spell_sul_sandstorm();
    new spell_marli_summon_blessed_loa_spirit();
    new spell_marli_summon_shadowed_loa_spirit();
    new spell_marli_twisted_fate_first();
    new spell_marli_twisted_fate_second();
    new spell_marli_twisted_fate_damages();
    new spell_dark_power();
    new spell_soul_fragment_target_selector();
    new spell_soul_fragment_switcher();
    new spell_marked_soul();
    new spell_shadowed_gift();
}
