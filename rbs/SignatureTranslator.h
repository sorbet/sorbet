#ifndef RBS_SIGNATURE_TRANSLATOR_H
#define RBS_SIGNATURE_TRANSLATOR_H

#include "ast/ast.h"
#include "parser/parser.h"
#include "rbs/rbs_common.h"
#include <string_view>
#include <vector>

namespace sorbet::rbs {

class SignatureTranslator final {
public:
    SignatureTranslator(core::MutableContext ctx) : ctx(ctx){};

    ast::ExpressionPtr translateAssertionType(std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams,
                                              const rbs::Comment &assertion);

    std::unique_ptr<parser::Node> translateType(const parser::Send *send, const rbs::Comment &signature,
                                                const std::vector<Comment> &annotations);
    std::unique_ptr<parser::Node> translateSignature(const parser::Node *methodDef, const rbs::Comment &signature,
                                                     const std::vector<Comment> &annotations);

private:
    core::MutableContext ctx;
};

} // namespace sorbet::rbs

#endif // RBS_SIGNATURE_TRANSLATOR_H
