#ifndef RBS_PRISM_TYPE_TO_PARSER_NODE_PRISM_H
#define RBS_PRISM_TYPE_TO_PARSER_NODE_PRISM_H

#include "parser/prism/Parser.h"
#include "prism.h"
#include "rbs/rbs_common.h"

namespace sorbet::rbs {

class TypeToParserNodePrism {
    core::MutableContext ctx;
    absl::Span<const std::pair<core::LocOffsets, core::NameRef>> typeParams;
    Parser parser;
    const parser::Prism::Parser *prismParser;

public:
    TypeToParserNodePrism(core::MutableContext ctx, absl::Span<const std::pair<core::LocOffsets, core::NameRef>> typeParams,
                          Parser parser, const parser::Prism::Parser *prismParser)
        : ctx(ctx), typeParams(typeParams), parser(parser), prismParser(prismParser) {}

    /**
     * Convert an RBS type to a Prism `pm_node_t`.
     *
     * For example:
     * - `Integer?` -> `T.nilable(Integer)`
     * - `(A | B)` -> `T.any(A, B)`
     */
    pm_node_t *toPrismNode(const rbs_node_t *node, const RBSDeclaration &declaration);

private:
    pm_node_t *namespaceConst(const rbs_namespace_t *rbsNamespace, const RBSDeclaration &declaration);
    pm_node_t *typeNameType(const rbs_type_name_t *typeName, bool isGeneric, const RBSDeclaration &declaration);
    pm_node_t *aliasType(const rbs_types_alias_t *node, core::LocOffsets loc, const RBSDeclaration &declaration);
    pm_node_t *classInstanceType(const rbs_types_class_instance_t *node, core::LocOffsets loc,
                                 const RBSDeclaration &declaration);
    pm_node_t *classSingletonType(const rbs_types_class_singleton_t *node, core::LocOffsets loc,
                                  const RBSDeclaration &declaration);
    pm_node_t *intersectionType(const rbs_types_intersection_t *node, core::LocOffsets loc,
                                const RBSDeclaration &declaration);
    pm_node_t *unionType(const rbs_types_union_t *node, core::LocOffsets loc, const RBSDeclaration &declaration);
    pm_node_t *optionalType(const rbs_types_optional_t *node, core::LocOffsets loc,
                            const RBSDeclaration &declaration);
    pm_node_t *voidType(const rbs_types_bases_void_t *node, core::LocOffsets loc);
    pm_node_t *functionType(const rbs_types_function_t *node, core::LocOffsets loc,
                            const RBSDeclaration &declaration);
    pm_node_t *procType(const rbs_types_proc_t *node, core::LocOffsets loc, const RBSDeclaration &declaration);
    pm_node_t *blockType(const rbs_types_block_t *node, core::LocOffsets loc, const RBSDeclaration &declaration);
    pm_node_t *tupleType(const rbs_types_tuple_t *node, core::LocOffsets loc, const RBSDeclaration &declaration);
    pm_node_t *recordType(const rbs_types_record_t *node, core::LocOffsets loc, const RBSDeclaration &declaration);
    pm_node_t *variableType(const rbs_types_variable_t *node, core::LocOffsets loc);

    // Helper methods for Prism node creation
    pm_node_t *createConstantReadNode(const std::string &name);
    pm_constant_id_t addConstantToPool(const char *name);
    template<typename NodeType> NodeType *allocateNode();
    pm_node_t initializeBaseNode(pm_node_type_t type);
};

} // namespace sorbet::rbs

#endif // RBS_PRISM_TYPE_TO_PARSER_NODE_PRISM_H