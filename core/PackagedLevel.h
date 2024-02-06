#ifndef CORE_PACKAGED_LEVEL_H
#define CORE_PACKAGED_LEVEL_H

#include "core/SigilTraits.h"
#include <cstdint>

namespace sorbet::core {

enum class PackagedLevel : uint8_t {
    False = 0,
    True = 1,
    None = 2,
};

template <> class SigilTraits<PackagedLevel> {
public:
    static constexpr PackagedLevel NONE = PackagedLevel::None;

    static constexpr std::string_view SIGIL_PREFIX = "packaged:";

    static PackagedLevel fromString(std::string_view s) {
        if (s == "false") {
            return PackagedLevel::False;
        } else if (s == "true") {
            return PackagedLevel::True;
        } else {
            return PackagedLevel::None;
        }
    }
};

} // namespace sorbet::core

#endif
