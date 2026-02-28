#include "rbs/TypeToParserNode.h"
#include "core/GlobalState.h"
#include "core/errors/internal.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"

using namespace std;

namespace sorbet::rbs {

namespace {

bool hasTypeParam(absl::Span<const pair<core::LocOffsets, core::NameRef>> typeParams, core::NameRef name) {
    return absl::c_any_of(typeParams, [&](const auto &param) { return param.second == name; });
}

bool isEnumerator(const unique_ptr<parser::Node> &node) {
    auto constNode = parser::cast_node<parser::Const>(node.get());

    if (!constNode) {
        return false;
    }

    return constNode->name == core::Names::Constants::Enumerator() &&
           (constNode->scope == nullptr || parser::isa_node<parser::Cbase>(constNode->scope.get()));
}

} // namespace

unique_ptr<parser::Node> TypeToParserNode::namespaceConst(const rbs_namespace_t *rbsNamespace,
                                                          const RBSDeclaration &declaration) {
    auto loc = declaration.typeLocFromRange(((rbs_node_t *)rbsNamespace)->location);
    rbs_node_list *typePath = rbsNamespace->path;

    unique_ptr<parser::Node> parent;

    if (rbsNamespace->absolute) {
        parent = parser::MK::Cbase(loc);
    } else {
        parent = nullptr;
    }

    if (typePath != nullptr) {
        for (rbs_node_list_node *list_node = typePath->head; list_node != nullptr; list_node = list_node->next) {
            rbs_node_t *node = list_node->node;

            ENFORCE(node->type == RBS_AST_SYMBOL, "Unexpected node type `{}` in type name, expected `{}`",
                    rbs_node_type_name(node), "Symbol");

            rbs_ast_symbol_t *symbol = (rbs_ast_symbol_t *)node;
            auto nameStr = parser.resolveConstant(symbol);
            auto pathNameConst = ctx.state.enterNameConstant(nameStr);
            parent = parser::MK::Const(loc, move(parent), pathNameConst);
        }
    }

    return parent;
}

unique_ptr<parser::Node> TypeToParserNode::typeNameType(const rbs_type_name_t *typeName, bool isGeneric,
                                                        const RBSDeclaration &declaration) {
    auto loc = declaration.typeLocFromRange(((rbs_node_t *)typeName)->location);

    auto parent = namespaceConst(typeName->rbs_namespace, declaration);

    auto nameStr = parser.resolveConstant(typeName->name);
    auto nameUTF8 = ctx.state.enterNameUTF8(nameStr);
    auto nameConstant = ctx.state.enterNameConstant(nameUTF8);

    if (isGeneric) {
        if (parent == nullptr || parser::isa_node<parser::Cbase>(parent.get())) {
            if (nameConstant == core::Names::Constants::Array()) {
                return parser::MK::T_Array(loc);
            } else if (nameConstant == core::Names::Constants::Class()) {
                return parser::MK::T_Class(loc);
            } else if (nameConstant == core::Names::Constants::Module()) {
                return parser::MK::T_Module(loc);
            } else if (nameConstant == core::Names::Constants::Enumerable()) {
                return parser::MK::T_Enumerable(loc);
            } else if (nameConstant == core::Names::Constants::Enumerator()) {
                return parser::MK::T_Enumerator(loc);
            } else if (nameConstant == core::Names::Constants::Hash()) {
                return parser::MK::T_Hash(loc);
            } else if (nameConstant == core::Names::Constants::Set()) {
                return parser::MK::T_Set(loc);
            } else if (nameConstant == core::Names::Constants::Range()) {
                return parser::MK::T_Range(loc);
            }
        } else if (nameConstant == core::Names::Constants::Lazy() && isEnumerator(parent)) {
            return parser::MK::T_Enumerator_Lazy(loc);
        } else if (nameConstant == core::Names::Constants::Chain() && isEnumerator(parent)) {
            return parser::MK::T_Enumerator_Chain(loc);
        }
    } else {
        // The type may refer to a type parameter, so we need to check if it exists as a NameKind::UTF8
        if (hasTypeParam(typeParams, nameUTF8)) {
            return parser::MK::TTypeParameter(loc, parser::MK::Symbol(loc, nameUTF8));
        }
    }

    return parser::MK::Const(loc, move(parent), nameConstant);
}

unique_ptr<parser::Node> TypeToParserNode::aliasType(const rbs_types_alias_t *node, core::LocOffsets loc,
                                                     const RBSDeclaration &declaration) {
    vector<core::NameRef> pathNames;
    auto parent = namespaceConst(node->name->rbs_namespace, declaration);
    auto nameView = parser.resolveConstant(node->name->name);
    auto nameStr = "type " + string(nameView);
    auto nameUTF8 = ctx.state.enterNameUTF8(nameStr);
    auto nameConstant = ctx.state.enterNameConstant(nameUTF8);

    return parser::MK::Const(loc, move(parent), nameConstant);
}

unique_ptr<parser::Node> TypeToParserNode::classInstanceType(const rbs_types_class_instance_t *node,
                                                             core::LocOffsets loc, const RBSDeclaration &declaration) {
    auto argsValue = node->args;
    auto isGeneric = argsValue != nullptr && argsValue->length > 0;
    auto typeConstant = typeNameType(node->name, isGeneric, declaration);

    if (isGeneric) {
        auto args = parser::NodeVec();
        args.reserve(argsValue->length);
        for (rbs_node_list_node *list_node = argsValue->head; list_node != nullptr; list_node = list_node->next) {
            auto argType = toParserNode(list_node->node, declaration);
            args.emplace_back(move(argType));
        }

        return parser::MK::Send(loc, move(typeConstant), core::Names::syntheticSquareBrackets(), loc, move(args));
    }

    return typeConstant;
}

unique_ptr<parser::Node> TypeToParserNode::classSingletonType(const rbs_types_class_singleton_t *node,
                                                              core::LocOffsets loc, const RBSDeclaration &declaration) {
    auto innerType = typeNameType(node->name, false, declaration);
    return parser::MK::TClassOf(loc, move(innerType));
}

unique_ptr<parser::Node> TypeToParserNode::unionType(const rbs_types_union_t *node, core::LocOffsets loc,
                                                     const RBSDeclaration &declaration) {
    auto args = parser::NodeVec();
    args.reserve(node->types->length);

    for (rbs_node_list_node *list_node = node->types->head; list_node != nullptr; list_node = list_node->next) {
        auto innerType = toParserNode(list_node->node, declaration);
        args.emplace_back(move(innerType));
    }

    return parser::MK::TAny(loc, move(args));
}

unique_ptr<parser::Node> TypeToParserNode::intersectionType(const rbs_types_intersection_t *node, core::LocOffsets loc,
                                                            const RBSDeclaration &declaration) {
    auto args = parser::NodeVec();
    args.reserve(node->types->length);

    for (rbs_node_list_node *list_node = node->types->head; list_node != nullptr; list_node = list_node->next) {
        auto innerType = toParserNode(list_node->node, declaration);
        args.emplace_back(move(innerType));
    }

    return parser::MK::TAll(loc, move(args));
}

unique_ptr<parser::Node> TypeToParserNode::optionalType(const rbs_types_optional_t *node, core::LocOffsets loc,
                                                        const RBSDeclaration &declaration) {
    auto innerType = toParserNode(node->type, declaration);

    if (parser::MK::isTUntyped(innerType)) {
        return innerType;
    }

    return parser::MK::TNilable(loc, move(innerType));
}

unique_ptr<parser::Node> TypeToParserNode::voidType(const rbs_types_bases_void_t *node, core::LocOffsets loc) {
    return make_unique<parser::ResolvedConst>(loc, core::Symbols::void_());
}

unique_ptr<parser::Node> TypeToParserNode::functionType(const rbs_types_function_t *node, core::LocOffsets loc,
                                                        const RBSDeclaration &declaration) {
    auto pairs = parser::NodeVec();
    pairs.reserve(node->required_positionals->length);

    int i = 0;
    for (rbs_node_list_node *list_node = node->required_positionals->head; list_node != nullptr;
         list_node = list_node->next) {
        auto argName = ctx.state.enterNameUTF8("arg" + to_string(i));
        auto key = parser::MK::Symbol(loc, argName);

        rbs_node_t *paramNode = list_node->node;
        unique_ptr<parser::Node> innerType;

        if (paramNode->type != RBS_TYPES_FUNCTION_PARAM) {
            if (auto e = ctx.beginIndexerError(loc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in function parameter type, expected `{}`",
                            rbs_node_type_name(paramNode), "FunctionParam");
            }
            innerType = parser::MK::TUntyped(loc);
        } else {
            innerType = toParserNode(((rbs_types_function_param_t *)paramNode)->type, declaration);
        }

        pairs.emplace_back(make_unique<parser::Pair>(loc, move(key), move(innerType)));

        i++;
    }

    rbs_node_t *returnValue = node->return_type;
    if (returnValue->type == RBS_TYPES_BASES_VOID) {
        return parser::MK::TProcVoid(loc, make_unique<parser::Hash>(loc, true, move(pairs)));
    }

    auto returnType = toParserNode(returnValue, declaration);

    return parser::MK::TProc(loc, make_unique<parser::Hash>(loc, true, move(pairs)), move(returnType));
}

unique_ptr<parser::Node> TypeToParserNode::procType(const rbs_types_proc_t *node, core::LocOffsets loc,
                                                    const RBSDeclaration &declaration) {
    auto function = parser::MK::TUntyped(loc);

    rbs_node_t *functionTypeNode = node->type;
    switch (functionTypeNode->type) {
        case RBS_TYPES_FUNCTION: {
            function = functionType((rbs_types_function_t *)functionTypeNode, loc, declaration);
            break;
        }
        case RBS_TYPES_UNTYPED_FUNCTION: {
            return function;
        }
        default: {
            auto errLoc = declaration.typeLocFromRange(functionTypeNode->location);
            if (auto e = ctx.beginIndexerError(errLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in proc type, expected `{}`",
                            rbs_node_type_name(functionTypeNode), "Function");
            }
        }
    }

    rbs_node_t *selfNode = node->self_type;
    if (selfNode != nullptr) {
        auto selfType = toParserNode(selfNode, declaration);
        function = parser::MK::Send1(loc, move(function), core::Names::bind(), loc, move(selfType));
    }

    return function;
}

unique_ptr<parser::Node> TypeToParserNode::blockType(const rbs_types_block_t *node, core::LocOffsets loc,
                                                     const RBSDeclaration &declaration) {
    auto function = parser::MK::TUntyped(loc);

    rbs_node_t *functionTypeNode = node->type;
    switch (functionTypeNode->type) {
        case RBS_TYPES_FUNCTION: {
            function = functionType((rbs_types_function_t *)functionTypeNode, loc, declaration);
            break;
        }
        case RBS_TYPES_UNTYPED_FUNCTION: {
            return function;
        }
        default: {
            auto errLoc = declaration.typeLocFromRange(functionTypeNode->location);
            if (auto e = ctx.beginIndexerError(errLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in block type, expected `{}`",
                            rbs_node_type_name(functionTypeNode), "Function");
            }

            return function;
        }
    }

    rbs_node_t *selfNode = node->self_type;
    if (selfNode != nullptr) {
        auto selfLoc = declaration.typeLocFromRange(selfNode->location);
        auto selfType = toParserNode(selfNode, declaration);
        function = parser::MK::Send1(selfLoc, move(function), core::Names::bind(), selfLoc, move(selfType));
    }

    if (!node->required) {
        return parser::MK::TNilable(loc, move(function));
    }

    return function;
}

unique_ptr<parser::Node> TypeToParserNode::tupleType(const rbs_types_tuple_t *node, core::LocOffsets loc,
                                                     const RBSDeclaration &declaration) {
    auto typesStore = parser::NodeVec();
    typesStore.reserve(node->types->length);

    for (rbs_node_list_node *list_node = node->types->head; list_node != nullptr; list_node = list_node->next) {
        auto innerType = toParserNode(list_node->node, declaration);
        typesStore.emplace_back(move(innerType));
    }

    return parser::MK::Array(loc, move(typesStore));
}

unique_ptr<parser::Node> TypeToParserNode::recordType(const rbs_types_record_t *node, core::LocOffsets loc,
                                                      const RBSDeclaration &declaration) {
    auto pairs = parser::NodeVec();
    pairs.reserve(node->all_fields->length);

    for (rbs_hash_node_t *hash_node = node->all_fields->head; hash_node != nullptr; hash_node = hash_node->next) {
        unique_ptr<parser::Node> key;

        switch (hash_node->key->type) {
            case RBS_AST_SYMBOL: {
                rbs_ast_symbol_t *keyNode = (rbs_ast_symbol_t *)hash_node->key;
                auto keyStr = parser.resolveConstant(keyNode);
                auto keyName = ctx.state.enterNameUTF8(keyStr);
                key = parser::MK::Symbol(loc, keyName);
                break;
            }
            case RBS_AST_STRING: {
                rbs_ast_string_t *keyNode = (rbs_ast_string_t *)hash_node->key;
                string_view keyStr(reinterpret_cast<const char *>(keyNode->string.start));
                auto keyName = ctx.state.enterNameUTF8(keyStr);
                key = parser::MK::String(loc, keyName);
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

        rbs_types_record_field_type_t *valueNode = (rbs_types_record_field_type_t *)hash_node->value;
        auto innerType = toParserNode(valueNode->type, declaration);
        pairs.emplace_back(make_unique<parser::Pair>(loc, move(key), move(innerType)));
    }

    return parser::MK::Hash(loc, false, move(pairs));
}

unique_ptr<parser::Node> TypeToParserNode::variableType(const rbs_types_variable_t *node, core::LocOffsets loc) {
    rbs_ast_symbol_t *symbol = (rbs_ast_symbol_t *)node->name;
    auto nameStr = parser.resolveConstant(symbol);
    auto name = ctx.state.enterNameUTF8(nameStr);
    return parser::MK::TTypeParameter(loc, parser::MK::Symbol(loc, name));
}

unique_ptr<parser::Node> TypeToParserNode::toParserNode(const rbs_node_t *node, const RBSDeclaration &declaration) {
    auto nodeLoc = declaration.typeLocFromRange(((rbs_node_t *)node)->location);
    switch (node->type) {
        case RBS_TYPES_ALIAS: {
            return aliasType((rbs_types_alias_t *)node, nodeLoc, declaration);
        }
        case RBS_TYPES_BASES_ANY:
            return parser::MK::TUntyped(nodeLoc);
        case RBS_TYPES_BASES_BOOL:
            return parser::MK::T_Boolean(nodeLoc);
        case RBS_TYPES_BASES_BOTTOM:
            return parser::MK::TNoReturn(nodeLoc);
        case RBS_TYPES_BASES_CLASS: {
            if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS type `{}` is not supported", "class");
            }
            return parser::MK::TUntyped(nodeLoc);
        }
        case RBS_TYPES_BASES_INSTANCE:
            return parser::MK::TAttachedClass(nodeLoc);
        case RBS_TYPES_BASES_NIL:
            return make_unique<parser::ResolvedConst>(nodeLoc, core::Symbols::NilClass());
        case RBS_TYPES_BASES_SELF:
            return parser::MK::TSelfType(nodeLoc);
        case RBS_TYPES_BASES_TOP:
            return parser::MK::TAnything(nodeLoc);
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
            return parser::MK::TUntyped(nodeLoc);
        }
        case RBS_TYPES_INTERSECTION:
            return intersectionType((rbs_types_intersection_t *)node, nodeLoc, declaration);
        case RBS_TYPES_LITERAL: {
            if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS literal types are not supported");
            }
            return parser::MK::TUntyped(nodeLoc);
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

            return parser::MK::TUntyped(nodeLoc);
        }
    }
}

} // namespace sorbet::rbs
