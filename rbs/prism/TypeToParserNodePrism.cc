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

bool isEnumerator(pm_node_t *node, parser::Prism::Parser &prismParser, core::GlobalState &gs) {
    auto *constNode = PM_NODE_TYPE_P(node, PM_CONSTANT_READ_NODE) ? down_cast<pm_constant_read_node_t>(node) : nullptr;
    auto *pathNode = PM_NODE_TYPE_P(node, PM_CONSTANT_PATH_NODE) ? down_cast<pm_constant_path_node_t>(node) : nullptr;

    if (!constNode && !pathNode) {
        return false;
    }

    auto nameId = constNode ? constNode->name : pathNode->name;
    auto name = gs.enterNameConstant(prismParser.resolveConstant(nameId));
    return name == core::Names::Constants::Enumerator() && (constNode || pathNode->parent == nullptr);
}

} // namespace

// Converts an RBS namespace to a Prism constant path node.
//   Foo::Bar -> ConstantPathNode(ConstantReadNode(Foo), Bar)
//   ::Foo    -> ConstantPathNode(nullptr, Foo)
pm_node_t *TypeToParserNodePrism::namespaceConst(const rbs_namespace_t *rbsNamespace, const RBSDeclaration &declaration,
                                                 core::LocOffsets loc) {
    rbs_node_list *typePath = rbsNamespace->path;

    pm_node_t *parent = nullptr;
    bool isAbsolute = rbsNamespace->absolute;

    if (typePath == nullptr) {
        return nullptr;
    }

    for (rbs_node_list_node *list_node = typePath->head; list_node != nullptr; list_node = list_node->next) {
        rbs_node_t *node = list_node->node;

        ENFORCE(node->type == RBS_AST_SYMBOL, "Unexpected node type `{}` in type name, expected `{}`",
                rbs_node_type_name(node), "Symbol");

        auto *symbol = (rbs_ast_symbol_t *)node;
        auto nameStr = parser.resolveConstant(symbol);

        if (parent != nullptr || isAbsolute) {
            parent = prism.ConstantPathNode(loc, parent, nameStr);
        } else {
            parent = prism.ConstantReadNode(nameStr, loc);
        }
    }

    return parent;
}

pm_node_t *TypeToParserNodePrism::typeNameType(const rbs_type_name_t *typeName, bool isGeneric,
                                               const RBSDeclaration &declaration) {
    auto loc = declaration.typeLocFromRange(typeName->base.location->rg);

    pm_node_t *parent = namespaceConst(typeName->rbs_namespace, declaration, loc);
    bool isAbsolute = typeName->rbs_namespace && typeName->rbs_namespace->absolute;

    auto nameStr = parser.resolveConstant(typeName->name);
    auto nameUTF8 = ctx.state.enterNameUTF8(nameStr);
    auto nameConstant = ctx.state.enterNameConstant(nameUTF8);

    if (isGeneric) {
        if (!parent) { // Root level constants
            if (nameConstant == core::Names::Constants::Array()) {
                return prism.T_Array(loc);
            } else if (nameConstant == core::Names::Constants::Class()) {
                return prism.T_Class(loc);
            } else if (nameConstant == core::Names::Constants::Module()) {
                return prism.T_Module(loc);
            } else if (nameConstant == core::Names::Constants::Enumerable()) {
                return prism.T_Enumerable(loc);
            } else if (nameConstant == core::Names::Constants::Enumerator()) {
                return prism.T_Enumerator(loc);
            } else if (nameConstant == core::Names::Constants::Hash()) {
                return prism.T_Hash(loc);
            } else if (nameConstant == core::Names::Constants::Set()) {
                return prism.T_Set(loc);
            } else if (nameConstant == core::Names::Constants::Range()) {
                return prism.T_Range(loc);
            }
        } else if (nameConstant == core::Names::Constants::Lazy() && isEnumerator(parent, prismParser, ctx.state)) {
            return prism.T_Enumerator_Lazy(loc);
        } else if (nameConstant == core::Names::Constants::Chain() && isEnumerator(parent, prismParser, ctx.state)) {
            return prism.T_Enumerator_Chain(loc);
        }
    } else {
        // The type may refer to a type parameter, so we need to check if it exists as a NameKind::UTF8
        if (hasTypeParam(typeParams, nameUTF8)) {
            return prism.TTypeParameter(loc, prism.Symbol(loc, nameStr));
        }
    }

    if (parent || isAbsolute) {
        return prism.ConstantPathNode(loc, parent, nameStr);
    } else {
        return prism.ConstantReadNode(nameStr, loc);
    }
}

pm_node_t *TypeToParserNodePrism::aliasType(const rbs_types_alias_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration) {
    pm_node_t *parent = namespaceConst(node->name->rbs_namespace, declaration, loc);
    auto nameView = parser.resolveConstant(node->name->name);
    auto nameStr = "type " + string(nameView);

    if (parent != nullptr || (node->name->rbs_namespace && node->name->rbs_namespace->absolute)) {
        return prism.ConstantPathNode(loc, parent, nameStr);
    } else {
        return prism.ConstantReadNode(nameStr, loc);
    }
}

pm_node_t *TypeToParserNodePrism::classInstanceType(const rbs_types_class_instance_t *node, core::LocOffsets loc,
                                                    const RBSDeclaration &declaration) {
    auto argsValue = node->args;
    auto isGeneric = argsValue != nullptr && argsValue->length > 0;
    auto typeConstant = typeNameType(node->name, isGeneric, declaration);

    if (isGeneric) {
        auto args = translateNodeList(argsValue, declaration);
        auto methodName = core::Names::syntheticSquareBrackets().shortName(ctx.state);
        return prism.Call(loc, typeConstant, methodName, absl::MakeSpan(args));
    }

    return typeConstant;
}

pm_node_t *TypeToParserNodePrism::classSingletonType(const rbs_types_class_singleton_t *node, core::LocOffsets loc,
                                                     const RBSDeclaration &declaration) {
    auto innerType = typeNameType(node->name, false, declaration);
    return prism.Call1(loc, prism.T(loc), "class_of"sv, innerType);
}

pm_node_t *TypeToParserNodePrism::unionType(const rbs_types_union_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration) {
    auto args = translateNodeList(node->types, declaration);
    return prism.TAny(loc, absl::MakeSpan(args));
}

pm_node_t *TypeToParserNodePrism::intersectionType(const rbs_types_intersection_t *node, core::LocOffsets loc,
                                                   const RBSDeclaration &declaration) {
    auto args = translateNodeList(node->types, declaration);
    return prism.TAll(loc, absl::MakeSpan(args));
}

pm_node_t *TypeToParserNodePrism::optionalType(const rbs_types_optional_t *node, core::LocOffsets loc,
                                               const RBSDeclaration &declaration) {
    auto innerType = toPrismNode(node->type, declaration);
    if (prismParser.isTUntyped(innerType)) {
        return innerType;
    }
    return prism.TNilable(loc, innerType);
}

pm_node_t *TypeToParserNodePrism::functionType(const rbs_types_function_t *node, core::LocOffsets loc,
                                               const RBSDeclaration &declaration) {
    vector<pm_node_t *> pairs;
    pairs.reserve(node->required_positionals->length);

    int argIndex = 0;
    for (rbs_node_list_node *list_node = node->required_positionals->head; list_node != nullptr;
         list_node = list_node->next) {
        auto argName = "arg" + to_string(argIndex);
        auto key = prism.Symbol(loc, argName);

        rbs_node_t *paramNode = list_node->node;
        pm_node_t *innerType;

        if (paramNode->type != RBS_TYPES_FUNCTION_PARAM) {
            if (auto e = ctx.beginIndexerError(loc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in function parameter type, expected `{}`",
                            rbs_node_type_name(paramNode), "FunctionParam");
            }
            innerType = prism.TUntyped(loc);
        } else {
            innerType = toPrismNode(((rbs_types_function_param_t *)paramNode)->type, declaration);
        }

        pairs.push_back(prism.AssocNode(loc, key, innerType));
        argIndex++;
    }

    rbs_node_t *returnValue = node->return_type;
    pm_node_t *argsHash = pairs.empty() ? nullptr : prism.KeywordHash(loc, absl::MakeSpan(pairs));

    if (returnValue->type == RBS_TYPES_BASES_VOID) {
        return prism.TProcVoid(loc, argsHash);
    }

    auto returnType = toPrismNode(returnValue, declaration);
    return prism.TProc(loc, argsHash, returnType);
}

pm_node_t *TypeToParserNodePrism::procType(const rbs_types_proc_t *node, core::LocOffsets loc,
                                           const RBSDeclaration &declaration) {
    rbs_node_t *functionTypeNode = node->type;
    pm_node_t *function;

    switch (functionTypeNode->type) {
        case RBS_TYPES_FUNCTION: {
            function = functionType(rbs_down_cast<rbs_types_function_t>(functionTypeNode), loc, declaration);
            break;
        }
        case RBS_TYPES_UNTYPED_FUNCTION: {
            return prism.TUntyped(loc);
        }
        default: {
            auto errLoc = declaration.typeLocFromRange(functionTypeNode->location->rg);
            if (auto e = ctx.beginIndexerError(errLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in proc type, expected `{}` or `{}`",
                            rbs_node_type_name(functionTypeNode), "Function", "UntypedFunction");
            }
            return prism.TUntyped(loc);
        }
    }

    if (auto *selfNode = node->self_type) {
        auto selfType = toPrismNode(selfNode, declaration);
        return prism.Call1(loc, function, "bind"sv, selfType);
    }

    return function;
}

pm_node_t *TypeToParserNodePrism::blockType(const rbs_types_block_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration) {
    rbs_node_t *functionTypeNode = node->type;
    pm_node_t *function;

    switch (functionTypeNode->type) {
        case RBS_TYPES_FUNCTION: {
            function = functionType(rbs_down_cast<rbs_types_function_t>(functionTypeNode), loc, declaration);
            break;
        }
        case RBS_TYPES_UNTYPED_FUNCTION: {
            return prism.TUntyped(loc);
        }
        default: {
            auto errLoc = declaration.typeLocFromRange(functionTypeNode->location->rg);
            if (auto e = ctx.beginIndexerError(errLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in block type, expected `{}`",
                            rbs_node_type_name(functionTypeNode), "Function");
            }
            return prism.TUntyped(loc);
        }
    }

    if (auto *selfNode = node->self_type) {
        auto selfLoc = declaration.typeLocFromRange(selfNode->location->rg);
        auto selfType = toPrismNode(selfNode, declaration);
        function = prism.Call1(selfLoc, function, "bind"sv, selfType);
    }

    if (!node->required) {
        return prism.TNilable(loc, function);
    }

    return function;
}

pm_node_t *TypeToParserNodePrism::tupleType(const rbs_types_tuple_t *node, core::LocOffsets loc,
                                            const RBSDeclaration &declaration) {
    auto types = translateNodeList(node->types, declaration);
    return prism.Array(loc, absl::MakeSpan(types));
}

pm_node_t *TypeToParserNodePrism::recordType(const rbs_types_record_t *node, core::LocOffsets loc,
                                             const RBSDeclaration &declaration) {
    vector<pm_node_t *> pairs;
    pairs.reserve(node->all_fields->length);

    for (rbs_hash_node_t *hash_node = node->all_fields->head; hash_node != nullptr; hash_node = hash_node->next) {
        pm_node_t *key = nullptr;

        switch (hash_node->key->type) {
            case RBS_AST_SYMBOL: {
                auto *keyNode = (rbs_ast_symbol_t *)hash_node->key;
                key = prism.Symbol(loc, parser.resolveConstant(keyNode));
                break;
            }
            case RBS_AST_STRING: {
                auto *keyNode = (rbs_ast_string_t *)hash_node->key;
                string_view keyStr(reinterpret_cast<const char *>(keyNode->string.start));
                key = prism.String(loc, keyStr);
                break;
            }
            default: {
                if (auto e = ctx.beginIndexerError(loc, core::errors::Internal::InternalError)) {
                    e.setHeader("Unexpected node type `{}` in record key type, expected `{}`",
                                rbs_node_type_name(hash_node->key), "Symbol");
                }
                continue;
            }
        }

        if (hash_node->value->type != RBS_TYPES_RECORD_FIELD_TYPE) {
            if (auto e = ctx.beginIndexerError(loc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in record value type, expected `{}`",
                            rbs_node_type_name(hash_node->value), "RecordFieldtype");
            }
            continue;
        }

        auto *valueNode = (rbs_types_record_field_type_t *)hash_node->value;
        pairs.push_back(prism.AssocNode(loc, key, toPrismNode(valueNode->type, declaration)));
    }

    return prism.Hash(loc, absl::MakeSpan(pairs));
}

pm_node_t *TypeToParserNodePrism::variableType(const rbs_types_variable_t *node, core::LocOffsets loc) {
    return prism.TTypeParameter(loc, prism.Symbol(loc, parser.resolveConstant(node->name)));
}

vector<pm_node_t *> TypeToParserNodePrism::translateNodeList(rbs_node_list *list, const RBSDeclaration &declaration) {
    vector<pm_node_t *> result;
    result.reserve(list->length);
    for (rbs_node_list_node *node = list->head; node != nullptr; node = node->next) {
        result.push_back(toPrismNode(node->node, declaration));
    }
    return result;
}

pm_node_t *TypeToParserNodePrism::toPrismNode(const rbs_node_t *node, const RBSDeclaration &declaration) {
    auto nodeLoc = declaration.typeLocFromRange(node->location->rg);

    switch (node->type) {
        case RBS_TYPES_ALIAS:
            return aliasType((rbs_types_alias_t *)node, nodeLoc, declaration);
        case RBS_TYPES_BASES_ANY:
            return prism.TUntyped(nodeLoc);
        case RBS_TYPES_BASES_BOOL:
            return prism.T_Boolean(nodeLoc);
        case RBS_TYPES_BASES_BOTTOM:
            return prism.Call0(nodeLoc, prism.T(nodeLoc), "noreturn"sv);
        case RBS_TYPES_BASES_CLASS: {
            if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS type `{}` is not supported", "class");
            }
            return prism.TUntyped(nodeLoc);
        }
        case RBS_TYPES_BASES_INSTANCE:
            return prism.TAttachedClass(nodeLoc);
        case RBS_TYPES_BASES_NIL:
            return prism.ConstantReadNode("NilClass"sv, nodeLoc);
        case RBS_TYPES_BASES_SELF:
            return prism.TSelfType(nodeLoc);
        case RBS_TYPES_BASES_TOP:
            return prism.TAnything(nodeLoc);
        case RBS_TYPES_BASES_VOID:
            return prism.SorbetPrivateStaticVoid(nodeLoc);
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
            return prism.TUntyped(nodeLoc);
        }
        case RBS_TYPES_INTERSECTION:
            return intersectionType((rbs_types_intersection_t *)node, nodeLoc, declaration);
        case RBS_TYPES_LITERAL: {
            if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS literal types are not supported");
            }
            return prism.TUntyped(nodeLoc);
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
            return prism.TUntyped(nodeLoc);
        }
    }
}

} // namespace sorbet::rbs
