#ifndef TRINITY_GAME_SPELLS_SPELL_MODIFIER_HPP
#define TRINITY_GAME_SPELLS_SPELL_MODIFIER_HPP

#include "Flag128.hpp"

#include <cstdint>
#include <memory>

class AuraEffect;
class SpellInfo;

enum SpellModOp
{
    SPELLMOD_DAMAGE                     = 0,
    SPELLMOD_DURATION                   = 1,
    SPELLMOD_THREAT                     = 2,
    SPELLMOD_EFFECT1                    = 3,
    SPELLMOD_CHARGES                    = 4,
    SPELLMOD_RANGE                      = 5,
    SPELLMOD_RADIUS                     = 6,
    SPELLMOD_CRITICAL_CHANCE            = 7,
    SPELLMOD_ALL_EFFECTS                = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME      = 9,
    SPELLMOD_CASTING_TIME               = 10,
    SPELLMOD_COOLDOWN                   = 11,
    SPELLMOD_EFFECT2                    = 12,
    SPELLMOD_IGNORE_ARMOR               = 13,
    SPELLMOD_COST                       = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS          = 15,
    SPELLMOD_RESIST_MISS_CHANCE         = 16,
    SPELLMOD_JUMP_TARGETS               = 17,
    SPELLMOD_CHANCE_OF_SUCCESS          = 18,
    SPELLMOD_ACTIVATION_TIME            = 19,
    SPELLMOD_DAMAGE_MULTIPLIER          = 20,
    SPELLMOD_GLOBAL_COOLDOWN            = 21,
    SPELLMOD_DOT                        = 22,
    SPELLMOD_EFFECT3                    = 23,
    SPELLMOD_BONUS_MULTIPLIER           = 24,
    // spellmod 25
    SPELLMOD_PROC_PER_MINUTE            = 26,
    SPELLMOD_VALUE_MULTIPLIER           = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE       = 28,
    // spellmod 29
    SPELLMOD_SPELL_COST_REFUND_ON_FAIL  = 30,
    SPELLMOD_STACKS                     = 31, // 55673 and 146962, increase charges
    SPELLMOD_EFFECT4                    = 32,
    SPELLMOD_EFFECT5                    = 33,
    SPELLMOD_UNK34                      = 34, // something cost-related
    SPELLMOD_JUMP_DISTANCE              = 35,
};

#define MAX_SPELLMOD 36

// Note: SPELLMOD_* values is aura types in fact
enum SpellModType
{
    SPELLMOD_FLAT         = 107,                            // SPELL_AURA_ADD_FLAT_MODIFIER
    SPELLMOD_PCT          = 108                             // SPELL_AURA_ADD_PCT_MODIFIER
};

// Spell modifier (used for modify other spells)
struct SpellModifier final
{
    typedef std::shared_ptr<SpellModifier> Ptr;

    explicit SpellModifier(AuraEffect *effect)
        : ownerEffect(effect)
        , op(SPELLMOD_DAMAGE)
        , type(SPELLMOD_FLAT)
        , mask()
        , value(0)
        , charges(0)
    { }

    static SpellModifier::Ptr create(AuraEffect *effect)
    {
        return std::make_shared<SpellModifier>(effect);
    }

    bool isAffectingSpell(SpellInfo const *spell) const;

    AuraEffect *ownerEffect;
    SpellModOp op;
    SpellModType type;
    Trinity::Flag128 mask;
    std::float_t value;
    std::int16_t charges;
};

#endif // TRINITY_GAME_SPELLS_SPELL_MODIFIER_HPP
