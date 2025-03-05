#ifndef SORBET_RBS_COMMON_H
#define SORBET_RBS_COMMON_H

extern "C" {
#include "include/rbs.h"
}

#include "core/LocOffsets.h"

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

/**
 * A parsed RBS method type, this is the equivalent of a `sig` block  on a method with vanilla Sorbet.
 */
struct MethodType {
    core::LocOffsets loc;
    std::unique_ptr<rbs_methodtype_t> node;
};

/**
 * A parsed RBS type, this is the equivalent of a `sig` block on an attribute accessor with vanilla Sorbet.
 */
struct Type {
    core::LocOffsets loc;
    std::unique_ptr<rbs_node_t> node;
};

/**
 * An error that occurred while parsing an RBS type or method signature.
 */
struct ParseError {
    core::LocOffsets loc;
    std::string message;
};

core::LocOffsets locFromRange(core::LocOffsets loc, const range &range);

} // namespace sorbet::rbs

#endif // SORBET_RBS_COMMON_H
