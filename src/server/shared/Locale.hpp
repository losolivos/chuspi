#ifndef TRINITY_SHARED_LOCALE_HPP
#define TRINITY_SHARED_LOCALE_HPP

enum LocaleConstant
{
    LOCALE_enUS =  0,
    LOCALE_koKR =  1,
    LOCALE_frFR =  2,
    LOCALE_deDE =  3,
    LOCALE_zhCN =  4,
    LOCALE_zhTW =  5,
    LOCALE_esES =  6,
    LOCALE_esMX =  7,
    LOCALE_ruRU =  8,
    LOCALE_ptPT =  9,
    LOCALE_itIT = 10,
};

const int TOTAL_LOCALES = 11;
const LocaleConstant DEFAULT_LOCALE = LOCALE_enUS;

#define MAX_LOCALES 10

extern char const * const localeNames[];

#endif // TRINITY_SHARED_LOCALE_HPP
