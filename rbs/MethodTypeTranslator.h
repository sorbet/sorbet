#ifndef RBS_METHOD_TYPE_TRANSLATOR_H
#define RBS_METHOD_TYPE_TRANSLATOR_H

#include "ast/ast.h"
#include "rbs/rbs_common.h"

namespace sorbet::rbs {

class MethodTypeTranslator {
    core::MutableContext ctx;
    Parser parser;

public:
    MethodTypeTranslator(core::MutableContext ctx, Parser parser) : ctx(ctx), parser(parser) {}

    /**
     * Convert an RBS method signature comment to a Sorbet signature.
     *
     * For example the signature comment `#: () -> void` will be translated as `sig { void }`.
     */
    ast::ExpressionPtr methodSignature(const ast::MethodDef *methodDef, const rbs_methodtype_t *methodType,
                                       const core::LocOffsets methodTypeLoc, const std::vector<Comment> &annotations);

    /**
     * Convert an RBS attribute type comment to a Sorbet signature.
     *
     * For example the attribute type comment `#: Integer` will be translated as `sig { returns(Integer) }`.
     */
    ast::ExpressionPtr attrSignature(const ast::Send *send, rbs_node_t *type, const core::LocOffsets typeLoc,
                                     const std::vector<Comment> &annotations);
};

} // namespace sorbet::rbs

#endif // RBS_METHOD_TYPE_TRANSLATOR_H
