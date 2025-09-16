#ifndef RBS_SIGNATURE_TRANSLATOR_PRISM_H
#define RBS_SIGNATURE_TRANSLATOR_PRISM_H

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

class SignatureTranslatorPrism final {
public:
    SignatureTranslatorPrism(core::MutableContext ctx) : ctx(ctx), parser(nullptr){};
    SignatureTranslatorPrism(core::MutableContext ctx, const parser::Prism::Parser& parser) : ctx(ctx), parser(&parser){};

    std::unique_ptr<parser::Node>
    translateAssertionType(std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams,
                           const RBSDeclaration &declaration);

    std::unique_ptr<parser::Node> translateAttrSignature(const pm_call_node_t *call, 
                                                         const RBSDeclaration &declaration,
                                                         const std::vector<Comment> &annotations);
    
    std::unique_ptr<parser::Node> translateMethodSignature(const pm_node_t *methodDef,
                                                           const RBSDeclaration &declaration,
                                                           const std::vector<Comment> &annotations);

    std::unique_ptr<parser::Node> translateType(const RBSDeclaration &declaration);

    parser::NodeVec translateTypeParams(const RBSDeclaration &declaration);

    // New Prism node creation methods
    pm_node_t* createPrismMethodSignature(const pm_node_t *methodDef,
                                          const RBSDeclaration &declaration,
                                          const std::vector<Comment> &annotations);

private:
    core::MutableContext ctx;
    const parser::Prism::Parser* parser; // For Prism node creation
};

} // namespace sorbet::rbs

#endif // RBS_SIGNATURE_TRANSLATOR_PRISM_H