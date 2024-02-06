#ifndef SORBET_CORE_SIGIL_TRAITS_H
#define SORBET_CORE_SIGIL_TRAITS_H

#include <string_view>

namespace sorbet::core {

template <class Sigil> class SigilTraits {
public:
    // The special sigil level that corresponds to the sigil being absent from the file.
    static const Sigil NONE;

    // A string like `typed:` or `compiled:`
    // If there's meant to be a trailing `:` in the sigil, it should be in this string.
    static const std::string_view SIGIL_PREFIX;

    // Produce a Sigil from its string representation `s`
    static Sigil fromString(std::string_view s);
};

} // namespace sorbet::core

#endif
