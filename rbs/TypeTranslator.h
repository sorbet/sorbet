#ifndef RBS_TYPE_TRANSLATOR_H
#define RBS_TYPE_TRANSLATOR_H

#include "ast/ast.h"
#include "rbs/rbs_common.h"
#include <memory>

namespace sorbet::rbs {

class TypeTranslator {
public:
    /**
     * Convert an RBS type to a Sorbet compatible expression ptr.
     *
     * For example:
     * - `Integer?` -> `T.nilable(Integer)`
     * - `(A | B)` -> `T.any(A, B)`
     */
    static ast::ExpressionPtr toExpressionPtr(core::MutableContext ctx,
                                              const std::vector<std::pair<core::LocOffsets, core::NameRef>> &typeParams,
                                              rbs_node_t *node, core::LocOffsets loc);
};

} // namespace sorbet::rbs

#endif // RBS_TYPE_TRANSLATOR_H
