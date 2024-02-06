#ifndef SORBET_CORE_COMPILED_LEVEL_H
#define SORBET_CORE_COMPILED_LEVEL_H

#include "core/SigilTraits.h"
#include <stdint.h>

namespace sorbet::core {
enum class CompiledLevel : uint8_t {

    // This file has no compiled sigil present. The behavior here is the same as for `# compiled: false`, but it's
    // useful to distinguish files that have no sigil present for stats.
    None = 0,

    // This file is explicitly opted-out of compilation.
    False = 1,

    // This file should be compiled.
    True = 2,
};

template <> class SigilTraits<CompiledLevel> {
public:
    static constexpr CompiledLevel NONE = CompiledLevel::None;

    static constexpr std::string_view SIGIL_PREFIX = "compiled:";

    static CompiledLevel fromString(std::string_view s) {
        if (s == "false") {
            return CompiledLevel::False;
        } else if (s == "true") {
            return CompiledLevel::True;
        } else {
            return CompiledLevel::None;
        }
    }
};

} // namespace sorbet::core

#endif
