#ifndef SORBET_CORE_TRACK_UNTYPED_H
#define SORBET_CORE_TRACK_UNTYPED_H

#include <stdint.h>

namespace sorbet::core {
enum class TrackUntyped : uint8_t {
    Nowhere,
    EverywhereButTests,
    Everywhere,
};
} // namespace sorbet::core

#endif
