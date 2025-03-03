#ifndef RBS_TYPE_TRANSLATOR_H
#define RBS_TYPE_TRANSLATOR_H

#include "ast/ast.h"
#include "rbs/rbs_common.h"
#include <memory>

namespace sorbet::rbs {

class TypeTranslator {
    core::MutableContext ctx;
    const std::vector<std::pair<core::LocOffsets, core::NameRef>> &typeParams;
    const std::shared_ptr<parserstate> &parser;

public:
    TypeTranslator(core::MutableContext ctx, const std::vector<std::pair<core::LocOffsets, core::NameRef>> &typeParams,
                   const std::shared_ptr<parserstate> &parser)
        : ctx(ctx), typeParams(typeParams), parser(parser) {}

    /**
     * Convert an RBS type to a Sorbet compatible expression ptr.
     *
     * For example:
     * - `Integer?` -> `T.nilable(Integer)`
     * - `(A | B)` -> `T.any(A, B)`
     */
    ast::ExpressionPtr toExpressionPtr(rbs_node_t *node, core::LocOffsets loc);

private:
    ast::ExpressionPtr typeNameType(rbs_typename_t *typeName, bool isGeneric, core::LocOffsets loc);
    ast::ExpressionPtr classInstanceType(rbs_types_classinstance_t *node, core::LocOffsets loc);
    ast::ExpressionPtr classSingletonType(rbs_types_classsingleton_t *node, core::LocOffsets loc);
    ast::ExpressionPtr intersectionType(rbs_types_intersection_t *node, core::LocOffsets loc);
    ast::ExpressionPtr unionType(rbs_types_union_t *node, core::LocOffsets loc);
    ast::ExpressionPtr optionalType(rbs_types_optional_t *node, core::LocOffsets loc);
    ast::ExpressionPtr voidType(rbs_types_bases_void_t *node, core::LocOffsets loc);
    ast::ExpressionPtr functionType(rbs_types_function_t *node, core::LocOffsets loc);
    ast::ExpressionPtr procType(rbs_types_proc_t *node, core::LocOffsets docLoc);
    ast::ExpressionPtr blockType(rbs_types_block_t *node, core::LocOffsets docLoc);
    ast::ExpressionPtr tupleType(rbs_types_tuple_t *node, core::LocOffsets loc);
    ast::ExpressionPtr recordType(rbs_types_record_t *node, core::LocOffsets loc);
    ast::ExpressionPtr variableType(rbs_types_variable_t *node, core::LocOffsets loc);
};

} // namespace sorbet::rbs

#endif // RBS_TYPE_TRANSLATOR_H
