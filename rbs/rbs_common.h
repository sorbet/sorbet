#ifndef SORBET_RBS_COMMON_H
#define SORBET_RBS_COMMON_H

extern "C" {
#include "include/rbs.h"
}

#include "core/LocOffsets.h"
#include "rbs/Parser.h"

namespace sorbet::rbs {

/**
 * A single RBS type comment found on a method definition or accessor.
 *
 * RBS type comments are formatted as `#: () -> void` for methods or `#: Integer` for attributes.
 */
struct Comment {
    core::LocOffsets loc;    // The location of the comment in the file
    std::string_view string; // The type string (excluding the `#: ` prefix)
                             // this is only a view on the string owned by the File.source() data.
};

core::LocOffsets locFromRange(core::LocOffsets loc, const range &range);

} // namespace sorbet::rbs

#endif // SORBET_RBS_COMMON_H
