#ifndef RBS_SIGNATURE_TRANSLATOR_H
#define RBS_SIGNATURE_TRANSLATOR_H

#include "ast/ast.h"
#include "rbs/rbs_common.h"
#include <string_view>
#include <vector>

namespace sorbet::rbs {

class SignatureTranslator final {
public:
    SignatureTranslator(std::vector<Comment> annotations) : annotations(annotations){};

    ast::ExpressionPtr translateAssertionType(core::MutableContext ctx,
                                              std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams,
                                              const rbs::Comment &assertion);

    ast::ExpressionPtr translateType(core::MutableContext ctx, const ast::Send *send, const rbs::Comment &signature);
    ast::ExpressionPtr translateSignature(core::MutableContext ctx, const ast::MethodDef *methodDef,
                                          const rbs::Comment &signature);

private:
    std::vector<Comment> annotations;

    rbs_string_t makeRBSString(const std::string_view &str);
};

} // namespace sorbet::rbs

#endif // RBS_SIGNATURE_TRANSLATOR_H
