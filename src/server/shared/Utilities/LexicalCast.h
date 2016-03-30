#ifndef TRINITY_SHARED_UTILITIES_LEXICAL_CAST_H
#define TRINITY_SHARED_UTILITIES_LEXICAL_CAST_H

#include "Define.h"

#include <type_traits>
#include <cstdlib>

namespace Trinity {

template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type
lexicalCast(char const *value)
{
#ifndef _MSC_VER
    return static_cast<T>(std::strtoull(value, nullptr, 0));
#else
    return static_cast<T>(_strtoui64(value, nullptr, 0));
#endif
}

template <typename T>
typename std::enable_if<std::is_signed<T>::value, T>::type
lexicalCast(char const *value)
{
#ifndef _MSC_VER
    return static_cast<T>(std::strtoll(value, nullptr, 0));
#else
    return static_cast<T>(_strtoi64(value, nullptr, 0));
#endif
}

} // namespace Trinity

#endif // TRINITY_SHARED_UTILITIES_LEXICAL_CAST_H
