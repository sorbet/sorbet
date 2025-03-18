#ifndef RBS_SIGNATURE_TRANSLATOR_H
#define RBS_SIGNATURE_TRANSLATOR_H

#include "ast/ast.h"
#include "rbs/rbs_common.h"
#include <string_view>
#include <vector>

namespace sorbet::rbs {

class SignatureTranslator final {
public:
    SignatureTranslator(core::MutableContext ctx) : ctx(ctx){};

    ast::ExpressionPtr translateAssertionType(std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams,
                                              const rbs::Comment &assertion);

    ast::ExpressionPtr translateType(const ast::Send *send, const rbs::Comment &signature,
                                     const std::vector<Comment> &annotations);
    ast::ExpressionPtr translateSignature(const ast::MethodDef *methodDef, const rbs::Comment &signature,
                                          const std::vector<Comment> &annotations);

private:
    core::MutableContext ctx;

    rbs_string_t makeRBSString(const std::string_view &str);
};

} // namespace sorbet::rbs

#endif // RBS_SIGNATURE_TRANSLATOR_H
