#ifndef RBS_TYPE_TO_PARSER_NODE_H
#define RBS_TYPE_TO_PARSER_NODE_H

#include "parser/parser.h"
#include "rbs/rbs_common.h"

namespace sorbet::rbs {

class TypeToParserNode {
    core::MutableContext ctx;
    absl::Span<const std::pair<core::LocOffsets, core::NameRef>> typeParams;
    Parser parser;

public:
    TypeToParserNode(core::MutableContext ctx, absl::Span<const std::pair<core::LocOffsets, core::NameRef>> typeParams,
                     Parser parser)
        : ctx(ctx), typeParams(typeParams), parser(parser) {}

    /**
     * Convert an RBS type to a Sorbet `parser::Node`.
     *
     * For example:
     * - `Integer?` -> `T.nilable(Integer)`
     * - `(A | B)` -> `T.any(A, B)`
     */
    std::unique_ptr<parser::Node> toParserNode(const rbs_node_t *node, const RBSDeclaration &declaration);

private:
    std::unique_ptr<parser::Node> namespaceConst(const rbs_namespace_t *rbsNamespace,
                                                 const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> typeNameType(const rbs_type_name_t *typeName, bool isGeneric,
                                               const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> aliasType(const rbs_types_alias_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> classInstanceType(const rbs_types_class_instance_t *node, core::LocOffsets loc,
                                                    const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> classSingletonType(const rbs_types_class_singleton_t *node, core::LocOffsets loc,
                                                     const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> intersectionType(const rbs_types_intersection_t *node, core::LocOffsets loc,
                                                   const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> unionType(const rbs_types_union_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> optionalType(const rbs_types_optional_t *node, core::LocOffsets loc,
                                               const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> voidType(const rbs_types_bases_void_t *node, core::LocOffsets loc);
    std::unique_ptr<parser::Node> functionType(const rbs_types_function_t *node, core::LocOffsets loc,
                                               const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> procType(const rbs_types_proc_t *node, core::LocOffsets loc,
                                           const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> blockType(const rbs_types_block_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> tupleType(const rbs_types_tuple_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> recordType(const rbs_types_record_t *node, core::LocOffsets loc,
                                             const RBSDeclaration &declaration);
    std::unique_ptr<parser::Node> variableType(const rbs_types_variable_t *node, core::LocOffsets loc);
};

} // namespace sorbet::rbs

#endif // RBS_TYPE_TO_PARSER_NODE_H
