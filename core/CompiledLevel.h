#ifndef SORBET_CORE_COMPILED_LEVEL_H
#define SORBET_CORE_COMPILED_LEVEL_H

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
}

#endif
