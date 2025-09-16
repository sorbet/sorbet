#ifndef RBS_METHOD_TYPE_TO_PARSER_NODE_PRISM_H
#define RBS_METHOD_TYPE_TO_PARSER_NODE_PRISM_H

#include "parser/parser.h"
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
    const parser::Prism::Parser* prismParser; // For Prism node creation

public:
    MethodTypeToParserNodePrism(core::MutableContext ctx, Parser parser) : ctx(ctx), parser(parser), prismParser(nullptr) {}
    MethodTypeToParserNodePrism(core::MutableContext ctx, Parser parser, const parser::Prism::Parser& prismParser)
        : ctx(ctx), parser(parser), prismParser(&prismParser) {}

    /**
     * Convert an RBS method signature comment to a Sorbet signature.
     *
     * For example the signature comment `#: () -> void` will be translated as `sig { void }`.
     */
    std::unique_ptr<parser::Node> methodSignature(const pm_node_t *methodDef, const rbs_method_type_t *methodType,
                                                  const RBSDeclaration &declaration,
                                                  const std::vector<Comment> &annotations);

    /**
     * Convert an RBS attribute type comment to a Sorbet signature.
     *
     * For example the attribute type comment `#: Integer` will be translated as `sig { returns(Integer) }`.
     */
    std::unique_ptr<parser::Node> attrSignature(const pm_call_node_t *call, const rbs_node_t *type,
                                                const RBSDeclaration &declaration,
                                                const std::vector<Comment> &annotations);

    /**
     * Create a Prism signature node from RBS method signature.
     *
     * This creates actual pm_node_t* instead of parser::Node*.
     */
    pm_node_t* createPrismMethodSignature(const pm_node_t *methodDef, const rbs_method_type_t *methodType,
                                          const RBSDeclaration &declaration,
                                          const std::vector<Comment> &annotations);

    /**
     * Create a placeholder sig call for when no RBS signature is available.
     */
    pm_node_t* createSigCallPlaceholder();

private:
    // Prism node creation helpers
    template<typename T> T* allocateNode();
    pm_node_t initializeBaseNode(pm_node_type_t type);
    pm_node_t* createConstantReadNode(const char* name);
    pm_node_t* createConstantPathNode(pm_node_t* parent, const char* name);
    pm_node_t* createSingleArgumentNode(pm_node_t* arg);

    // High-level node creators
    pm_node_t* createSorbetPrivateStaticConstant();
    pm_node_t* createTSigWithoutRuntimeConstant();
    pm_node_t* createStringConstant();

    // Utility helpers
    pm_constant_id_t addConstantToPool(const char* name);
    pm_location_t getZeroWidthLocation();
};

} // namespace sorbet::rbs

#endif // RBS_METHOD_TYPE_TO_PARSER_NODE_PRISM_H