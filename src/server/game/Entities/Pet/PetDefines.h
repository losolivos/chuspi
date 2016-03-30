#ifndef TRINITY_GAME_PET_DEFINES_H
#define TRINITY_GAME_PET_DEFINES_H

enum PetType
{
    SUMMON_PET              = 0,
    HUNTER_PET              = 1,
    MAX_PET_TYPE            = 4,
};

enum PetRemoveMode
{
    PET_REMOVE_DISMISS,
    PET_REMOVE_ABANDON
};

enum PetRemoveFlag
{
    PET_REMOVE_FLAG_NONE           = 0x00,
    PET_REMOVE_FLAG_RETURN_REAGENT = 0x01,
    PET_REMOVE_FLAG_RESET_CURRENT  = 0x02
};

enum PetByteMask
{
    PET_BYTES_0 = 0x02020300 // class = hunter, gender = none, power = focus
};

enum PetTameError
{
    PET_TAME_ERROR_UNKNOWN_ERROR            = 0,    // checked
    PET_TAME_ERROR_INVALID_CREATURE         = 1,    // checked
    PET_TAME_ERROR_TOO_MANY_PETS            = 2,    // checked
    PET_TAME_ERROR_CREATURE_ALREADY_OWNED   = 3,    // checked
    PET_TAME_ERROR_NOT_TAMEABLE             = 4,    // checked
    PET_TAME_ERROR_ANOTHER_SUMMON_ACTIVE    = 5,    // checked
    PET_TAME_ERROR_YOU_CANT_TAME            = 6,    // checked
    PET_TAME_ERROR_NO_PET_AVAILABLE         = 7,    // checked
    PET_TAME_ERROR_INTERNAL_ERROR           = 8,    // checked
    PET_TAME_ERROR_TOO_HIGH_LEVEL           = 9,    // checked
    PET_TAME_ERROR_DEAD                     = 10,   // checked
    PET_TAME_ERROR_NOT_DEAD                 = 11,   // checked
    PET_TAME_ERROR_CANT_CONTROL_EXOTIC      = 12,   // checked
    PET_TAME_ERROR_INVALID_SLOT             = 13    // checked
};

// stored in character_pet.slot
enum PetSlot
{
    PET_SLOT_ACTIVE_FIRST = 0,
    PET_SLOT_ACTIVE_LAST  = 4,
    PET_SLOT_STABLE_FIRST = 5,
    PET_SLOT_STABLE_LAST  = 54,

    MAX_PET_STABLES       = (PET_SLOT_STABLE_LAST - PET_SLOT_STABLE_FIRST + 1)
};

enum PetLoadMode
{
    PET_LOAD_BY_ID,
    PET_LOAD_BY_ENTRY,
    PET_LOAD_BY_SLOT
};

enum PetSpellState
{
    PETSPELL_UNCHANGED = 0,
    PETSPELL_CHANGED   = 1,
    PETSPELL_NEW       = 2,
    PETSPELL_REMOVED   = 3
};

enum PetSpellType
{
    PETSPELL_NORMAL = 0,
    PETSPELL_FAMILY = 1,
    PETSPELL_TALENT = 2,
};

enum ActionFeedback
{
    FEEDBACK_NONE            = 0,
    FEEDBACK_PET_DEAD        = 1,
    FEEDBACK_NOTHING_TO_ATT  = 2,
    FEEDBACK_CANT_ATT_TARGET = 3
};

enum PetTalk
{
    PET_TALK_SPECIAL_SPELL  = 0,
    PET_TALK_ATTACK         = 1
};

enum PetNameInvalidReason
{
    // custom, not send
    PET_NAME_SUCCESS                                        = 0,

    PET_NAME_INVALID                                        = 1,
    PET_NAME_NO_NAME                                        = 2,
    PET_NAME_TOO_SHORT                                      = 3,
    PET_NAME_TOO_LONG                                       = 4,
    PET_NAME_MIXED_LANGUAGES                                = 6,
    PET_NAME_PROFANE                                        = 7,
    PET_NAME_RESERVED                                       = 8,
    PET_NAME_THREE_CONSECUTIVE                              = 11,
    PET_NAME_INVALID_SPACE                                  = 12,
    PET_NAME_CONSECUTIVE_SPACES                             = 13,
    PET_NAME_RUSSIAN_CONSECUTIVE_SILENT_CHARACTERS          = 14,
    PET_NAME_RUSSIAN_SILENT_CHARACTER_AT_BEGINNING_OR_END   = 15,
    PET_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME              = 16
};

#define PET_FOLLOW_DIST  1.0f
#define PET_FOLLOW_ANGLE (M_PI/2)

#endif // TRINITY_GAME_PET_DEFINES_H
