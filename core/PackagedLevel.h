#ifndef CORE_PACKAGED_LEVEL_H
#define CORE_PACKAGED_LEVEL_H

#include <cstdint>

namespace sorbet::core {

enum class PackagedLevel : uint8_t {
    False = 0,
    True = 1,
    None = 2,
};

}

#endif
