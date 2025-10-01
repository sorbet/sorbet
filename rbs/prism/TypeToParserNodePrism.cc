#include "rbs/prism/TypeToParserNodePrism.h"

#include "common/typecase.h"
#include "core/GlobalState.h"
#include "core/errors/internal.h"
#include "core/errors/rewriter.h"
#include "parser/prism/Helpers.h"

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {

bool hasTypeParam(absl::Span<const pair<core::LocOffsets, core::NameRef>> typeParams, core::NameRef name) {
    return absl::c_any_of(typeParams, [&](const auto &param) { return param.second == name; });
}

} // namespace

// Helper methods for Prism node creation (copied from MethodTypeToParserNodePrism)
pm_node_t *TypeToParserNodePrism::createConstantReadNode(const std::string &name) {
    fmt::print("TypeToParserNodePrism::createConstantReadNode\n");
    pm_constant_id_t constant_id = addConstantToPool(name.c_str());
    if (constant_id == PM_CONSTANT_ID_UNSET)
        return nullptr;

    pm_constant_read_node_t *node = allocateNode<pm_constant_read_node_t>();
    if (!node)
        return nullptr;

    *node = (pm_constant_read_node_t){.base = initializeBaseNode(PM_CONSTANT_READ_NODE), .name = constant_id};

    return up_cast(node);
}

pm_constant_id_t TypeToParserNodePrism::addConstantToPool(const char *name) {
    if (!prismParser)
        return PM_CONSTANT_ID_UNSET;

    pm_parser_t *p = prismParser->getInternalParser();
    size_t name_len = strlen(name);
    fmt::print("DEBUG addConstant name='{}' strlen={}\n", name, name_len);
    // Copy bytes into a stable buffer to avoid dangling pointers in the pool
    uint8_t *stable = (uint8_t *)calloc(name_len, sizeof(uint8_t));
    if (!stable) {
        return PM_CONSTANT_ID_UNSET;
    }
    memcpy(stable, name, name_len);
    pm_constant_id_t id = pm_constant_pool_insert_constant(&p->constant_pool, stable, name_len);
    return id; // Returns PM_CONSTANT_ID_UNSET on failure
}

template <typename NodeType> NodeType *TypeToParserNodePrism::allocateNode() {
    NodeType *node = (NodeType *)calloc(1, sizeof(NodeType));
    return node; // Returns nullptr on allocation failure
}

pm_node_t TypeToParserNodePrism::initializeBaseNode(pm_node_type_t type) {
    if (!prismParser) {
        pm_location_t loc = {.start = nullptr, .end = nullptr};
        return (pm_node_t){.type = type, .flags = 0, .node_id = 0, .location = loc};
    }
    pm_parser_t *p = prismParser->getInternalParser();
    // Use a zero-width location anchored to the start of the file buffer
    pm_location_t loc = {.start = p->start, .end = p->start};
    return (pm_node_t){.type = type, .flags = 0, .node_id = ++p->node_id, .location = loc};
}

pm_node_t *TypeToParserNodePrism::namespaceConst(const rbs_namespace_t *rbsNamespace,
                                                 const RBSDeclaration &declaration) {
    // auto loc = declaration.typeLocFromRange(((rbs_node_t *)rbsNamespace)->location->rg);
    rbs_node_list *typePath = rbsNamespace->path;

    pm_node_t *parent = nullptr;

    if (rbsNamespace->absolute) {
        // Create root constant access (::)
        parent = this->createConstantReadNode("");
    }

    if (typePath != nullptr) {
        for (rbs_node_list_node *list_node = typePath->head; list_node != nullptr; list_node = list_node->next) {
            rbs_node_t *node = list_node->node;

            ENFORCE(node->type == RBS_AST_SYMBOL, "Unexpected node type `{}` in type name, expected `{}`",
                    rbs_node_type_name(node), "Symbol");

            rbs_ast_symbol_t *symbol = (rbs_ast_symbol_t *)node;
            auto nameStr = parser.resolveConstant(symbol);
            string nameString(nameStr);

            pm_node_t *constNode = this->createConstantReadNode(nameString);
            if (parent) {
                // Create constant path node for scoped access
                // This is simplified - full implementation would use pm_constant_path_node_create
                parent = constNode;
            } else {
                parent = constNode;
            }
        }
    }

    return parent;
}

pm_node_t *TypeToParserNodePrism::typeNameType(const rbs_type_name_t *typeName, bool isGeneric,
                                               const RBSDeclaration &declaration) {
    // auto loc = declaration.typeLocFromRange(((rbs_node_t *)typeName)->location->rg);

    pm_node_t *parent = namespaceConst(typeName->rbs_namespace, declaration);

    auto nameStr = parser.resolveConstant(typeName->name);
    fmt::print("DEBUG nameStr='{}' size={}\n", std::string(nameStr), nameStr.size());
    auto nameUTF8 = ctx.state.enterNameUTF8(nameStr);
    auto nameConstant = ctx.state.enterNameConstant(nameUTF8);

    // Ensure proper string conversion from string_view
    string nameString{nameStr.data(), nameStr.size()};

    if (isGeneric) {
        if (!parent) { // Root level constants
            if (nameConstant == core::Names::Constants::Array()) {
                return this->createConstantReadNode("T::Array");
            } else if (nameConstant == core::Names::Constants::Class()) {
                return this->createConstantReadNode("T::Class");
            } else if (nameConstant == core::Names::Constants::Enumerable()) {
                return this->createConstantReadNode("T::Enumerable");
            } else if (nameConstant == core::Names::Constants::Enumerator()) {
                return this->createConstantReadNode("T::Enumerator");
            } else if (nameConstant == core::Names::Constants::Hash()) {
                return this->createConstantReadNode("T::Hash");
            } else if (nameConstant == core::Names::Constants::Set()) {
                return this->createConstantReadNode("T::Set");
            } else if (nameConstant == core::Names::Constants::Range()) {
                return this->createConstantReadNode("T::Range");
            }
        }
    } else {
        // Check if it's a type parameter
        if (hasTypeParam(typeParams, nameUTF8)) {
            // Create type parameter reference - this would need special handling
            // For now, return the name as a constant
            fmt::print("DEBUG: Creating type parameter reference: {}\n", nameString);
            return this->createConstantReadNode(nameString);
        }
    }

    fmt::print("DEBUG: Creating constant read node: {}\n", nameString);
    return this->createConstantReadNode(nameString);
}

pm_node_t *TypeToParserNodePrism::aliasType(const rbs_types_alias_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration) {
    // auto parent = namespaceConst(node->name->rbs_namespace, declaration);
    auto nameView = parser.resolveConstant(node->name->name);
    auto nameStr = "type " + string(nameView);

    return this->createConstantReadNode(nameStr);
}

pm_node_t *TypeToParserNodePrism::classInstanceType(const rbs_types_class_instance_t *node, core::LocOffsets loc,
                                                    const RBSDeclaration &declaration) {
    fmt::print("TypeToParserNodePrism::classInstanceType\n");
    auto argsValue = node->args;
    auto isGeneric = argsValue != nullptr && argsValue->length > 0;
    return typeNameType(node->name, isGeneric, declaration);
}

pm_node_t *TypeToParserNodePrism::classSingletonType(const rbs_types_class_singleton_t *node, core::LocOffsets loc,
                                                     const RBSDeclaration &declaration) {
    // Simplified - should create T.class_of call
    return this->createConstantReadNode("T.class_of");
}

pm_node_t *TypeToParserNodePrism::unionType(const rbs_types_union_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration) {
    // Simplified - should create T.any call with all union members
    return this->createConstantReadNode("T.any");
}

pm_node_t *TypeToParserNodePrism::intersectionType(const rbs_types_intersection_t *node, core::LocOffsets loc,
                                                   const RBSDeclaration &declaration) {
    // Simplified - should create T.all call with all intersection members
    return this->createConstantReadNode("T.all");
}

pm_node_t *TypeToParserNodePrism::optionalType(const rbs_types_optional_t *node, core::LocOffsets loc,
                                               const RBSDeclaration &declaration) {
    // Simplified - should create T.nilable call with inner type
    fmt::print("TypeToParserNodePrism::optionalType\n");
    return this->createConstantReadNode("T.nilable");
}

pm_node_t *TypeToParserNodePrism::voidType(const rbs_types_bases_void_t *node, core::LocOffsets loc) {
    return this->createConstantReadNode("T.void");
}

pm_node_t *TypeToParserNodePrism::functionType(const rbs_types_function_t *node, core::LocOffsets loc,
                                               const RBSDeclaration &declaration) {
    return this->createConstantReadNode("T.proc");
}

pm_node_t *TypeToParserNodePrism::procType(const rbs_types_proc_t *node, core::LocOffsets loc,
                                           const RBSDeclaration &declaration) {
    return functionType((rbs_types_function_t *)node->type, loc, declaration);
}

pm_node_t *TypeToParserNodePrism::blockType(const rbs_types_block_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration) {
    auto function = functionType((rbs_types_function_t *)node->type, loc, declaration);
    if (!node->required) {
        // Should make it nilable, but simplified for now
    }
    return function;
}

pm_node_t *TypeToParserNodePrism::tupleType(const rbs_types_tuple_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration) {
    return this->createConstantReadNode("Array");
}

pm_node_t *TypeToParserNodePrism::recordType(const rbs_types_record_t *node, core::LocOffsets loc,
                                             const RBSDeclaration &declaration) {
    return this->createConstantReadNode("Hash");
}

pm_node_t *TypeToParserNodePrism::variableType(const rbs_types_variable_t *node, core::LocOffsets loc) {
    rbs_ast_symbol_t *symbol = (rbs_ast_symbol_t *)node->name;
    auto nameStr = parser.resolveConstant(symbol);
    string nameString(nameStr);
    return this->createConstantReadNode(nameString);
}

pm_node_t *TypeToParserNodePrism::toPrismNode(const rbs_node_t *node, const RBSDeclaration &declaration) {
    auto nodeLoc = declaration.typeLocFromRange(((rbs_node_t *)node)->location->rg);

    fmt::print("DEBUG type={}\n", rbs_node_type_name((rbs_node_t *)node));
    switch (node->type) {
        case RBS_TYPES_ALIAS:
            return aliasType((rbs_types_alias_t *)node, nodeLoc, declaration);
        case RBS_TYPES_BASES_ANY:
            return this->createConstantReadNode("T.untyped");
        case RBS_TYPES_BASES_BOOL:
            return this->createConstantReadNode("T::Boolean");
        case RBS_TYPES_BASES_BOTTOM:
            return this->createConstantReadNode("T.noreturn");
        case RBS_TYPES_BASES_CLASS: {
            if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS type `{}` is not supported", "class");
            }
            return this->createConstantReadNode("T.untyped");
        }
        case RBS_TYPES_BASES_INSTANCE:
            return this->createConstantReadNode("T.attached_class");
        case RBS_TYPES_BASES_NIL:
            return this->createConstantReadNode("NilClass");
        case RBS_TYPES_BASES_SELF:
            return this->createConstantReadNode("T.self_type");
        case RBS_TYPES_BASES_TOP:
            return this->createConstantReadNode("T.anything");
        case RBS_TYPES_BASES_VOID:
            return voidType((rbs_types_bases_void_t *)node, nodeLoc);
        case RBS_TYPES_BLOCK:
            return blockType((rbs_types_block_t *)node, nodeLoc, declaration);
        case RBS_TYPES_CLASS_INSTANCE:
            return classInstanceType((rbs_types_class_instance_t *)node, nodeLoc, declaration);
        case RBS_TYPES_CLASS_SINGLETON:
            return classSingletonType((rbs_types_class_singleton_t *)node, nodeLoc, declaration);
        case RBS_TYPES_FUNCTION:
            return functionType((rbs_types_function_t *)node, nodeLoc, declaration);
        case RBS_TYPES_INTERFACE: {
            if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS interfaces are not supported");
            }
            return this->createConstantReadNode("T.untyped");
        }
        case RBS_TYPES_INTERSECTION:
            return intersectionType((rbs_types_intersection_t *)node, nodeLoc, declaration);
        case RBS_TYPES_LITERAL: {
            if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS literal types are not supported");
            }
            return this->createConstantReadNode("T.untyped");
        }
        case RBS_TYPES_OPTIONAL:
            return optionalType((rbs_types_optional_t *)node, nodeLoc, declaration);
        case RBS_TYPES_PROC:
            return procType((rbs_types_proc_t *)node, nodeLoc, declaration);
        case RBS_TYPES_RECORD:
            return recordType((rbs_types_record_t *)node, nodeLoc, declaration);
        case RBS_TYPES_TUPLE:
            return tupleType((rbs_types_tuple_t *)node, nodeLoc, declaration);
        case RBS_TYPES_UNION:
            return unionType((rbs_types_union_t *)node, nodeLoc, declaration);
        case RBS_TYPES_VARIABLE:
            return variableType((rbs_types_variable_t *)node, nodeLoc);
        default: {
            if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}`", rbs_node_type_name((rbs_node_t *)node));
            }

            return this->createConstantReadNode("T.untyped");
        }
    }
}

} // namespace sorbet::rbs
