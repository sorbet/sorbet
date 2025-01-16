#ifndef RBS_COMMON_H
#define RBS_COMMON_H

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
};

/**
 * A parsed RBS method type.
 */
class MethodType {
public:
    core::LocOffsets loc;
    rbs_methodtype_t *node;

    MethodType(core::LocOffsets loc, rbs_methodtype_t *node) : loc(loc), node(node) {}

    ~MethodType() {
        free(node);
    }
};

/**
 * A parsed RBS type.
 */
class Type {
public:
    core::LocOffsets loc;
    rbs_node_t *node;

    Type(core::LocOffsets loc, rbs_node_t *node) : loc(loc), node(node) {}

    ~Type() {
        free(node);
    }
};

} // namespace sorbet::rbs

#endif // RBS_COMMON_H
