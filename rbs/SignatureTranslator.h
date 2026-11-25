#ifndef SORBET_RBS_SIGNATURE_TRANSLATOR_H
#define SORBET_RBS_SIGNATURE_TRANSLATOR_H

#include "parser/parser.h"
#include "parser/prism/Parser.h"
#include "rbs/rbs_common.h"
#include <memory>
#include <string_view>
#include <vector>

extern "C" {
#include "prism.h"
}

namespace sorbet::rbs {

class SignatureTranslator final {
public:
    SignatureTranslator(core::MutableContext ctx) : ctx(ctx), prismParser{nullptr} {}
    SignatureTranslator(core::MutableContext ctx, parser::Prism::Parser &prismParser)
        : ctx(ctx), prismParser{&prismParser} {}

    pm_node_t *translateMethodSignature(pm_node_t *methodDef, const RBSDeclaration &declaration,
                                        absl::Span<const Comment> annotations);

    pm_node_t *translateAssertionType(absl::Span<std::pair<core::LocOffsets, core::NameRef>> typeParams,
                                      const RBSDeclaration &declaration);

    pm_node_t *translateType(const RBSDeclaration &declaration);

    pm_node_t *translateAttrSignature(pm_call_node_t *call, const RBSDeclaration &declaration,
                                      absl::Span<const Comment> annotations);

    std::vector<pm_node_t *> translateTypeParams(const RBSDeclaration &declaration);

private:
    core::MutableContext ctx;
    parser::Prism::Parser *prismParser; // For Prism node creation
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_SIGNATURE_TRANSLATOR_H
