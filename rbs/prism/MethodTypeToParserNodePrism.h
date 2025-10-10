#ifndef RBS_METHOD_TYPE_TO_PARSER_NODE_PRISM_H
#define RBS_METHOD_TYPE_TO_PARSER_NODE_PRISM_H

#include "parser/parser.h"
#include "parser/prism/Helpers.h"
#include "parser/prism/Parser.h"
#include "rbs/rbs_common.h"
#include <memory>

extern "C" {
#include "prism.h"
}

namespace sorbet::rbs {

class MethodTypeToParserNodePrism {
    core::MutableContext ctx;
    Parser parser;
    const parser::Prism::Parser *prismParser; // For Prism node creation

public:
    MethodTypeToParserNodePrism(core::MutableContext ctx, Parser parser)
        : ctx(ctx), parser(parser), prismParser(nullptr) {}
    MethodTypeToParserNodePrism(core::MutableContext ctx, Parser parser, const parser::Prism::Parser &prismParser)
        : ctx(ctx), parser(parser), prismParser(&prismParser) {}

    /**
     * Create a Prism signature node from RBS method signature.
     *
     * For example the signature comment `#: () -> void` will be translated as
     * `sig { void }`.
     */
    pm_node_t *methodSignature(const pm_node_t *methodDef, const rbs_method_type_t *methodType,
                               const RBSDeclaration &declaration, const std::vector<Comment> &annotations);

    /**
     * Convert an RBS attribute type comment to a Sorbet signature.
     *
     * For example the attribute type comment `#: Integer` will be translated as
     * `sig { returns(Integer) }`.
     */
    std::unique_ptr<parser::Node> attrSignature(const pm_call_node_t *call, const rbs_node_t *type,
                                                const RBSDeclaration &declaration,
                                                const std::vector<Comment> &annotations);

private:
    // RBS-specific node creators (thin wrappers around PMK)
    pm_node_t *createSymbolNode(rbs_ast_symbol_t *name, core::LocOffsets nameLoc);
};

} // namespace sorbet::rbs

#endif // RBS_METHOD_TYPE_TO_PARSER_NODE_PRISM_H
