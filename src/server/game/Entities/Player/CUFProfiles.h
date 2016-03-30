#ifndef TRINITY_GAME_CUFPROFILES_H
#define TRINITY_GAME_CUFPROFILES_H

#include <cstdint>
#include <bitset>
#include <string>

struct CufProfile final
{
    enum
    {
        MaxProfiles = 5,
        MaxNameLength = 128
    };

    /// Bit index used in the many bool options of CompactUnitFrames
    enum
    {
        DisplayMainTankAndAssistant,
        KeepGroupsTogether,
        AutoActivate5Players,
        AutoActivate15Players,
        DisplayPowerBar,
        DisplayBorder,
        AutoActivatePvp,
        AutoActivate40Players,
        AutoActivateSpec1,
        AutoActivate3Players,
        AutoActivate2Players,
        UseClassColors,
        Unk13,
        AutoActivateSpec2,
        HorizontalGroups,
        Unk16,
        DisplayOnlyDispellableDebuffs,
        DisplayNonBossDebuffs,
        DisplayPets,
        AutoActivate25Players,
        DisplayHealPrediction,
        DisplayAggroHighlight,
        AutoActivate10Players,
        Unk24,
        AutoActivatePve,

        LastBitIndex = AutoActivatePve
    };

    std::string name;

    std::uint16_t unk0;
    std::uint16_t unk1;
    std::uint16_t frameWidth;
    std::uint16_t frameHeight;
    std::uint16_t unk4;

    std::uint8_t unk5;
    std::uint8_t unk6;
    std::uint8_t unk7;
    std::uint8_t sortType;
    std::uint8_t healthText;

    std::bitset<LastBitIndex + 1> bits;
};

#endif // TRINITY_GAME_CUFPROFILES_H
