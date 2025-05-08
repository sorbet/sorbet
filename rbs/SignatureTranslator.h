#ifndef RBS_SIGNATURE_TRANSLATOR_H
#define RBS_SIGNATURE_TRANSLATOR_H

#include "parser/parser.h"
#include "rbs/rbs_common.h"
#include <string_view>
#include <vector>

namespace sorbet::rbs {

class SignatureTranslator final {
public:
    SignatureTranslator(core::MutableContext ctx) : ctx(ctx){};

    std::unique_ptr<parser::Node>
    translateAssertionType(std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams,
                           const RBSDeclaration &declaration);

    std::unique_ptr<parser::Node> translateAttrSignature(const parser::Send *send, const RBSDeclaration &declaration,
                                                         const std::vector<Comment> &annotations);
    std::unique_ptr<parser::Node> translateMethodSignature(const parser::Node *methodDef,
                                                           const RBSDeclaration &declaration,
                                                           const std::vector<Comment> &annotations);

    std::unique_ptr<parser::Node> translateType(const RBSDeclaration &declaration);

    parser::NodeVec translateTypeParams(const RBSDeclaration &declaration);

private:
    core::MutableContext ctx;
};

} // namespace sorbet::rbs

#endif // RBS_SIGNATURE_TRANSLATOR_H
