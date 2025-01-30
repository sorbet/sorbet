#ifndef RBS_METHOD_TYPE_TRANSLATOR_H
#define RBS_METHOD_TYPE_TRANSLATOR_H

#include "ast/ast.h"
#include "rbs/rbs_common.h"

namespace sorbet::rbs {

class MethodTypeTranslator {
public:
    /**
     * Convert an RBS method signature comment to a Sorbet signature.
     *
     * For example the signature comment `#: () -> void` will be translated as `sig { void }`.
     */
    static sorbet::ast::ExpressionPtr methodSignature(core::MutableContext ctx, const sorbet::ast::MethodDef *methodDef,
                                                      const MethodType type, const std::vector<Comment> &annotations);

    /**
     * Convert an RBS attribute type comment to a Sorbet signature.
     *
     * For example the attribute type comment `#: Integer` will be translated as `sig { returns(Integer) }`.
     */
    static sorbet::ast::ExpressionPtr attrSignature(core::MutableContext ctx, const sorbet::ast::Send *send,
                                                    const Type type);
};

} // namespace sorbet::rbs

#endif // RBS_METHOD_TYPE_TRANSLATOR_H
