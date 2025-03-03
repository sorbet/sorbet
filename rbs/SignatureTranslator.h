#ifndef RBS_SIGNATURE_TRANSLATOR_H
#define RBS_SIGNATURE_TRANSLATOR_H

#include "ast/ast.h"
#include "rbs/rbs_common.h"
#include <string_view>

namespace sorbet::rbs {

class SignatureTranslator final {
public:
    sorbet::rbs::Comment signature;

    SignatureTranslator(sorbet::rbs::Comment signature) : signature(signature) {}

    ast::ExpressionPtr translateType(core::MutableContext ctx, const ast::Send *send,
                                     const std::vector<Comment> &annotations);
    ast::ExpressionPtr translateSignature(core::MutableContext ctx, const ast::MethodDef *methodDef,
                                          const std::vector<rbs::Comment> &annotations);

private:
    rbs_string_t makeRBSString(const std::string_view &str);
};

} // namespace sorbet::rbs

#endif // RBS_SIGNATURE_TRANSLATOR_H
