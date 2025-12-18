#ifndef SORBET_RBS_COMMON_H
#define SORBET_RBS_COMMON_H

#include <type_traits>

extern "C" {
#include "include/rbs.h"
}

#include "core/LocOffsets.h"
#include "rbs/Parser.h"

// Macro to check if an RBS location range is null (unset)
// From ext/rbs_extension/legacy_location.c in the RBS repository
#define NULL_LOC_RANGE_P(rg) ((rg).start == -1)

namespace sorbet::rbs {

/**
 * Creates an rbs_string_t from a std::string_view.
 *
 * The returned rbs_string_t contains pointers into str's buffer, so str_view must outlive
 * the rbs_string_t and any objects that use it.
 */
rbs_string_t makeRBSString(const std::string_view str);

// This list of helpers below includes RBS node types that
// we actually needed to be able to cast, and is not exhaustive.
template <typename Type> struct RBSNodeTypeHelper {
    static_assert(always_false_v<Type>,
                  "RBSNodeTypeHelper is not specialized for this type. "
                  "You'll need to add a new `DEF_RBS_TYPE_HELPER(rbs_new_node_type, RBS_NEW_NODE_ENUM)`"
                  "definition to the list in `rbs/rbs_common.h`");
};

#define DEF_RBS_TYPE_HELPER(rbs_struct_type, type_enum)         \
    template <> struct RBSNodeTypeHelper<rbs_struct_type> {     \
        static constexpr enum rbs_node_type TypeID = type_enum; \
    }

DEF_RBS_TYPE_HELPER(rbs_ast_symbol_t, RBS_AST_SYMBOL);
DEF_RBS_TYPE_HELPER(rbs_ast_type_param_t, RBS_AST_TYPE_PARAM);
DEF_RBS_TYPE_HELPER(rbs_types_function_t, RBS_TYPES_FUNCTION);
DEF_RBS_TYPE_HELPER(rbs_types_function_param_t, RBS_TYPES_FUNCTION_PARAM);

#undef DEF_RBS_TYPE_HELPER

// Safe down_cast for RBS nodes (from rbs_node_t to specific types)
// In debug builds, checks the node type before casting
template <typename RBSNode> RBSNode *rbs_down_cast(rbs_node_t *anyNode) {
    (void)RBSNodeTypeHelper<rbs_ast_symbol_t>::TypeID;
    static_assert(std::is_same_v<decltype(RBSNode::base), rbs_node_t>,
                  "The `rbs_down_cast` function should only be called on RBS node pointers.");
    ENFORCE(anyNode != nullptr, "Failed to cast an RBS node. Expected type {}, but got null",
            (int)RBSNodeTypeHelper<RBSNode>::TypeID);
    ENFORCE(anyNode->type == RBSNodeTypeHelper<RBSNode>::TypeID,
            "Failed to cast an RBS node. Expected type {}, but got type {}", (int)RBSNodeTypeHelper<RBSNode>::TypeID,
            (int)anyNode->type);
    return reinterpret_cast<RBSNode *>(anyNode);
}

/**
 * A single RBS type comment found on a method definition or accessor.
 *
 * RBS type comments are formatted as `#: () -> void` for methods or `#: Integer` for attributes.
 */
struct Comment {
    core::LocOffsets commentLoc; // The location of the comment in the file
    core::LocOffsets typeLoc;    // The location of the actual RBS content in the file
    std::string_view string;     // The type string (excluding the `#: ` prefix)
                                 // this is only a view on the string owned by the File.source() data.
};

/**
 * A collection of RBS type comments that collectively describe a method signature, attribute type, or type assertion.
 */
class RBSDeclaration {
public:
    /**
     * All the comments into a single string.
     */
    std::string string;

    RBSDeclaration(std::vector<Comment> comments) : comments(std::move(comments)) {
        std::string result;
        for (const auto &comment : this->comments) {
            result += comment.string;
        }
        this->string = std::move(result);
    }

    /**
     * Returns the location that all the comments cover.
     *
     * For multiline comments, this starts at the first comment's location and ends at the last comment's location.
     *
     * ```
     * v starts from here
     * #: (Integer) ->
     * #| void
     *       ^ ends here
     * ```
     */
    core::LocOffsets commentLoc() const;

    /**
     * Returns entire type location
     *
     * ```
     *   v starts from here
     * #: (Integer) ->
     * #| void
     *       ^ ends here
     * ```
     */
    core::LocOffsets fullTypeLoc() const;

    /**
     * Returns the type location of the first comment.
     *
     * ```
     *   v starts from here
     * #: (Integer) ->
     *               ^ ends here
     * #| void
     * ```
     *
     * Usually used for generating `sig` nodes only.
     */
    core::LocOffsets firstLineTypeLoc() const;

    /**
     * Returns the type location of the RBS declaration that covers the given range.
     * For example, if given the range of the `void` token:
     *
     * ```
     * #: (Integer) ->
     *    v starts from here
     * #| void
     *       ^ ends here
     * ```
     */
    core::LocOffsets typeLocFromRange(const rbs_range_t &range) const;

private:
    std::vector<Comment> comments;
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_COMMON_H
