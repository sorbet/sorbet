#ifndef SORBET_PARSER_PRISM_HELPERS_H
#define SORBET_PARSER_PRISM_HELPERS_H

#include <type_traits>
extern "C" {
#include "prism.h"
}

namespace sorbet::parser::Prism {

using std::is_same_v;

// Returns true if the given `T` is a Prism node types. All Prism node types start with a `pm_node_t base` member.
template <typename T> constexpr bool isPrismNode = is_same_v<decltype(T::base), pm_node_t>;

// Take a pointer to a Prism node "subclass" (a thing with an embedded `pm_node_t base` as its first member),
// and up-casts it back to a general `pm_node_t` pointer.
template <typename PrismNode> pm_node_t *up_cast(PrismNode *node) {
    static_assert(!is_same_v<PrismNode, pm_node_t>,
                  "There's no need to call `up_cast` here, because this is already a `pm_node_t`.");
    static_assert(isPrismNode<PrismNode>,
                  "The `up_cast` function should only be called on Prism node pointers.");
    return reinterpret_cast<pm_node_t *>(node);
}

} // namespace sorbet::parser::Prism

#endif // SORBET_PARSER_PRISM_HELPERS_H
