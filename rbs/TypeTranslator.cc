#include "rbs/TypeTranslator.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "core/errors/internal.h"
#include "core/errors/rewriter.h"

using namespace std;

namespace sorbet::rbs {

namespace {

bool hasTypeParam(absl::Span<const std::pair<core::LocOffsets, core::NameRef>> typeParams, core::NameRef name) {
    return absl::c_any_of(typeParams, [&](const auto &param) { return param.second == name; });
}

} // namespace

ast::ExpressionPtr TypeTranslator::typeNameType(rbs_typename_t *typeName, bool isGeneric, core::LocOffsets loc) {
    rbs_node_list *typePath = typeName->rbs_namespace->path;

    ast::ExpressionPtr parent;
    vector<core::NameRef> pathNames;

    if (typeName->rbs_namespace->absolute) {
        parent = ast::MK::Constant(loc, core::Symbols::root());
    } else {
        parent = ast::MK::EmptyTree();
    }

    if (typePath != nullptr) {
        for (rbs_node_list_node *list_node = typePath->head; list_node != nullptr; list_node = list_node->next) {
            rbs_node_t *node = list_node->node;

            ENFORCE(node->type == RBS_AST_SYMBOL, "Unexpected node type `{}` in type name, expected `{}`",
                    rbs_node_type_name(node), "Symbol");

            rbs_ast_symbol_t *symbol = (rbs_ast_symbol_t *)node;
            auto nameStr = parser.resolveConstant(symbol);
            auto pathNameConst = ctx.state.enterNameConstant(nameStr);
            pathNames.emplace_back(pathNameConst);
            parent = ast::MK::UnresolvedConstant(loc, move(parent), pathNameConst);
        }
    }

    auto nameStr = parser.resolveConstant(typeName->name);
    auto nameUTF8 = ctx.state.enterNameUTF8(nameStr);
    auto nameConstant = ctx.state.enterNameConstant(nameUTF8);

    pathNames.emplace_back(nameConstant);

    if (pathNames.size() == 1) {
        if (isGeneric) {
            if (nameConstant == core::Names::Constants::Array()) {
                return ast::MK::T_Array(loc);
            } else if (nameConstant == core::Names::Constants::Class()) {
                return ast::MK::T_Class(loc);
            } else if (nameConstant == core::Names::Constants::Enumerable()) {
                return ast::MK::T_Enumerable(loc);
            } else if (nameConstant == core::Names::Constants::Enumerator()) {
                return ast::MK::T_Enumerator(loc);
            } else if (nameConstant == core::Names::Constants::Hash()) {
                return ast::MK::T_Hash(loc);
            } else if (nameConstant == core::Names::Constants::Set()) {
                return ast::MK::T_Set(loc);
            } else if (nameConstant == core::Names::Constants::Range()) {
                return ast::MK::T_Range(loc);
            }
        } else {
            // The type may refer to a type parameter, so we need to check if it exists as a NameKind::UTF8
            if (hasTypeParam(typeParams, nameUTF8)) {
                return ast::MK::Send1(loc, ast::MK::T(loc), core::Names::typeParameter(), loc,
                                      ast::MK::Symbol(loc, nameUTF8));
            }
        }
    } else if (pathNames.size() == 2 && isGeneric) {
        if (pathNames[0] == core::Names::Constants::Enumerator()) {
            if (pathNames[1] == core::Names::Constants::Lazy()) {
                return ast::MK::T_Enumerator_Lazy(loc);
            } else if (pathNames[1] == core::Names::Constants::Chain()) {
                return ast::MK::T_Enumerator_Chain(loc);
            }
        }
    }

    return ast::MK::UnresolvedConstant(loc, move(parent), nameConstant);
}

ast::ExpressionPtr TypeTranslator::classInstanceType(rbs_types_classinstance_t *node, core::LocOffsets loc) {
    auto offsets = locFromRange(loc, ((rbs_node_t *)node)->location->rg);
    auto argsValue = node->args;
    auto isGeneric = argsValue != nullptr && argsValue->length > 0;
    auto typeConstant = typeNameType(node->name, isGeneric, offsets);

    if (isGeneric) {
        auto argsStore = ast::Send::ARGS_store();
        for (rbs_node_list_node *list_node = argsValue->head; list_node != nullptr; list_node = list_node->next) {
            auto argType = toExpressionPtr(list_node->node, loc);
            argsStore.emplace_back(move(argType));
        }

        return ast::MK::Send(offsets, move(typeConstant), core::Names::squareBrackets(), offsets, argsStore.size(),
                             move(argsStore));
    }

    return typeConstant;
}

ast::ExpressionPtr TypeTranslator::classSingletonType(rbs_types_classsingleton_t *node, core::LocOffsets loc) {
    auto offsets = locFromRange(loc, ((rbs_node_t *)node)->location->rg);
    auto innerType = typeNameType(node->name, false, offsets);
    return ast::MK::Send1(loc, ast::MK::T(loc), core::Names::classOf(), loc, move(innerType));
}

ast::ExpressionPtr TypeTranslator::unionType(rbs_types_union_t *node, core::LocOffsets loc) {
    auto typesStore = ast::Send::ARGS_store();

    for (rbs_node_list_node *list_node = node->types->head; list_node != nullptr; list_node = list_node->next) {
        auto innerType = toExpressionPtr(list_node->node, loc);
        typesStore.emplace_back(move(innerType));
    }

    return ast::MK::Any(loc, move(typesStore));
}

ast::ExpressionPtr TypeTranslator::intersectionType(rbs_types_intersection_t *node, core::LocOffsets loc) {
    auto typesStore = ast::Send::ARGS_store();

    for (rbs_node_list_node *list_node = node->types->head; list_node != nullptr; list_node = list_node->next) {
        auto innerType = toExpressionPtr(list_node->node, loc);
        typesStore.emplace_back(move(innerType));
    }

    return ast::MK::All(loc, move(typesStore));
}

ast::ExpressionPtr TypeTranslator::optionalType(rbs_types_optional_t *node, core::LocOffsets loc) {
    auto innerType = toExpressionPtr(node->type, loc);

    if (ast::MK::isTUntyped(innerType)) {
        return innerType;
    }

    return ast::MK::Nilable(loc, move(innerType));
}

ast::ExpressionPtr TypeTranslator::voidType(rbs_types_bases_void_t *node, core::LocOffsets loc) {
    auto cSorbet = ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), core::Names::Constants::Sorbet());
    auto cPrivate = ast::MK::UnresolvedConstant(loc, move(cSorbet), core::Names::Constants::Private());
    auto cStatic = ast::MK::UnresolvedConstant(loc, move(cPrivate), core::Names::Constants::Static());

    return ast::MK::UnresolvedConstant(loc, move(cStatic), core::Names::Constants::Void());
}

ast::ExpressionPtr TypeTranslator::functionType(rbs_types_function_t *node, core::LocOffsets loc) {
    auto paramsStore = ast::Send::ARGS_store();
    int i = 0;
    for (rbs_node_list_node *list_node = node->required_positionals->head; list_node != nullptr;
         list_node = list_node->next) {
        auto argName = ctx.state.enterNameUTF8("arg" + to_string(i));
        paramsStore.emplace_back(ast::MK::Symbol(loc, argName));

        rbs_node_t *paramNode = list_node->node;
        ast::ExpressionPtr innerType;

        if (paramNode->type != RBS_TYPES_FUNCTION_PARAM) {
            if (auto e = ctx.beginError(loc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in function parameter type, expected `{}`",
                            rbs_node_type_name(paramNode), "FunctionParam");
            }
            innerType = ast::MK::Untyped(loc);
        } else {
            innerType = toExpressionPtr(((rbs_types_function_param_t *)paramNode)->type, loc);
        }

        paramsStore.emplace_back(move(innerType));

        i++;
    }

    rbs_node_t *returnValue = node->return_type;
    if (returnValue->type == RBS_TYPES_BASES_VOID) {
        return ast::MK::T_ProcVoid(loc, move(paramsStore));
    }

    auto returnType = toExpressionPtr(returnValue, loc);

    return ast::MK::T_Proc(loc, move(paramsStore), move(returnType));
}

ast::ExpressionPtr TypeTranslator::procType(rbs_types_proc_t *node, core::LocOffsets docLoc) {
    auto loc = locFromRange(docLoc, ((rbs_node_t *)node)->location->rg);
    auto function = ast::MK::Untyped(loc);

    rbs_node_t *functionTypeNode = node->type;
    switch (functionTypeNode->type) {
        case RBS_TYPES_FUNCTION: {
            function = functionType((rbs_types_function_t *)functionTypeNode, loc);
            break;
        }
        case RBS_TYPES_UNTYPEDFUNCTION: {
            return function;
        }
        default: {
            auto errLoc = locFromRange(docLoc, functionTypeNode->location->rg);
            if (auto e = ctx.beginError(errLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in proc type, expected `{}`",
                            rbs_node_type_name(functionTypeNode), "Function");
            }
        }
    }

    rbs_node_t *selfNode = node->self_type;
    if (selfNode != nullptr) {
        auto selfLoc = locFromRange(loc, selfNode->location->rg);
        auto selfType = toExpressionPtr(selfNode, selfLoc);
        function = ast::MK::Send1(loc, move(function), core::Names::bind(), loc, move(selfType));
    }

    return function;
}

ast::ExpressionPtr TypeTranslator::blockType(rbs_types_block_t *node, core::LocOffsets docLoc) {
    auto loc = locFromRange(docLoc, ((rbs_node_t *)node)->location->rg);
    auto function = ast::MK::Untyped(loc);

    rbs_node_t *functionTypeNode = node->type;
    switch (functionTypeNode->type) {
        case RBS_TYPES_FUNCTION: {
            function = functionType((rbs_types_function_t *)functionTypeNode, docLoc);
            break;
        }
        case RBS_TYPES_UNTYPEDFUNCTION: {
            return function;
        }
        default: {
            auto errLoc = locFromRange(docLoc, functionTypeNode->location->rg);
            if (auto e = ctx.beginError(errLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in block type, expected `{}`",
                            rbs_node_type_name(functionTypeNode), "Function");
            }

            return function;
        }
    }

    rbs_node_t *selfNode = node->self_type;
    if (selfNode != nullptr) {
        auto selfLoc = locFromRange(docLoc, selfNode->location->rg);
        auto selfType = toExpressionPtr(selfNode, selfLoc);
        function = ast::MK::Send1(selfLoc, move(function), core::Names::bind(), selfLoc, move(selfType));
    }

    if (!node->required) {
        return ast::MK::Nilable(loc, move(function));
    }

    return function;
}

ast::ExpressionPtr TypeTranslator::tupleType(rbs_types_tuple_t *node, core::LocOffsets loc) {
    auto typesStore = ast::Array::ENTRY_store();

    for (rbs_node_list_node *list_node = node->types->head; list_node != nullptr; list_node = list_node->next) {
        auto innerType = toExpressionPtr(list_node->node, loc);
        typesStore.emplace_back(move(innerType));
    }

    return ast::MK::Array(loc, move(typesStore));
}

ast::ExpressionPtr TypeTranslator::recordType(rbs_types_record_t *node, core::LocOffsets loc) {
    auto keysStore = ast::Hash::ENTRY_store();
    auto valuesStore = ast::Hash::ENTRY_store();

    for (rbs_hash_node_t *hash_node = node->all_fields->head; hash_node != nullptr; hash_node = hash_node->next) {
        switch (hash_node->key->type) {
            case RBS_AST_SYMBOL: {
                rbs_ast_symbol_t *keyNode = (rbs_ast_symbol_t *)hash_node->key;
                auto keyStr = parser.resolveConstant(keyNode);
                auto keyName = ctx.state.enterNameUTF8(keyStr);
                keysStore.emplace_back(ast::MK::Symbol(loc, keyName));
                break;
            }
            case RBS_AST_STRING: {
                rbs_ast_string_t *keyNode = (rbs_ast_string_t *)hash_node->key;
                string_view keyStr(reinterpret_cast<const char *>(keyNode->string.start));
                auto keyName = ctx.state.enterNameUTF8(keyStr);
                keysStore.emplace_back(ast::MK::String(loc, keyName));
                break;
            }
            default: {
                if (auto e = ctx.beginError(loc, core::errors::Internal::InternalError)) {
                    e.setHeader("Unexpected node type `{}` in record key type, expected `{}`",
                                rbs_node_type_name(hash_node->key), "Symbol");
                }
                continue;
            }
        }

        if (hash_node->value->type != RBS_TYPES_RECORD_FIELDTYPE) {
            if (auto e = ctx.beginError(loc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}` in record value type, expected `{}`",
                            rbs_node_type_name(hash_node->value), "RecordFieldtype");
            }

            continue;
        }

        rbs_types_record_fieldtype_t *valueNode = (rbs_types_record_fieldtype_t *)hash_node->value;
        auto innerType = toExpressionPtr(valueNode->type, loc);
        valuesStore.emplace_back(move(innerType));
    }

    return ast::MK::Hash(loc, move(keysStore), move(valuesStore));
}

ast::ExpressionPtr TypeTranslator::variableType(rbs_types_variable_t *node, core::LocOffsets loc) {
    rbs_ast_symbol_t *symbol = (rbs_ast_symbol_t *)node->name;
    auto nameStr = parser.resolveConstant(symbol);
    auto name = ctx.state.enterNameUTF8(nameStr);
    return ast::MK::Send1(loc, ast::MK::T(loc), core::Names::typeParameter(), loc, ast::MK::Symbol(loc, name));
}

ast::ExpressionPtr TypeTranslator::toExpressionPtr(rbs_node_t *node, core::LocOffsets docLoc) {
    switch (node->type) {
        case RBS_TYPES_ALIAS: {
            auto loc = locFromRange(docLoc, node->location->rg);
            if (auto e = ctx.beginError(loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS aliases are not supported");
            }
            return ast::MK::Untyped(docLoc);
        }
        case RBS_TYPES_BASES_ANY:
            return ast::MK::Untyped(docLoc);
        case RBS_TYPES_BASES_BOOL:
            return ast::MK::T_Boolean(docLoc);
        case RBS_TYPES_BASES_BOTTOM:
            return ast::MK::NoReturn(docLoc);
        case RBS_TYPES_BASES_CLASS: {
            auto loc = locFromRange(docLoc, node->location->rg);
            if (auto e = ctx.beginError(loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS type `{}` is not supported", "class");
            }
            return ast::MK::Untyped(docLoc);
        }
        case RBS_TYPES_BASES_INSTANCE:
            return ast::MK::AttachedClass(docLoc);
        case RBS_TYPES_BASES_NIL:
            return ast::MK::UnresolvedConstant(docLoc, ast::MK::EmptyTree(), core::Names::Constants::NilClass());
        case RBS_TYPES_BASES_SELF:
            return ast::MK::SelfType(docLoc);
        case RBS_TYPES_BASES_TOP:
            return ast::MK::Anything(docLoc);
        case RBS_TYPES_BASES_VOID:
            return voidType((rbs_types_bases_void_t *)node, docLoc);
        case RBS_TYPES_BLOCK:
            return blockType((rbs_types_block_t *)node, docLoc);
        case RBS_TYPES_CLASSINSTANCE:
            return classInstanceType((rbs_types_classinstance_t *)node, docLoc);
        case RBS_TYPES_CLASSSINGLETON:
            return classSingletonType((rbs_types_classsingleton_t *)node, docLoc);
        case RBS_TYPES_FUNCTION:
            return functionType((rbs_types_function_t *)node, docLoc);
        case RBS_TYPES_INTERFACE: {
            auto loc = locFromRange(docLoc, node->location->rg);
            if (auto e = ctx.beginError(loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS interfaces are not supported");
            }
            return ast::MK::Untyped(docLoc);
        }
        case RBS_TYPES_INTERSECTION:
            return intersectionType((rbs_types_intersection_t *)node, docLoc);
        case RBS_TYPES_LITERAL: {
            auto loc = locFromRange(docLoc, node->location->rg);
            if (auto e = ctx.beginError(loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS literal types are not supported");
            }
            return ast::MK::Untyped(docLoc);
        }
        case RBS_TYPES_OPTIONAL:
            return optionalType((rbs_types_optional_t *)node, docLoc);
        case RBS_TYPES_PROC:
            return procType((rbs_types_proc_t *)node, docLoc);
        case RBS_TYPES_RECORD:
            return recordType((rbs_types_record_t *)node, docLoc);
        case RBS_TYPES_TUPLE:
            return tupleType((rbs_types_tuple_t *)node, docLoc);
        case RBS_TYPES_UNION:
            return unionType((rbs_types_union_t *)node, docLoc);
        case RBS_TYPES_VARIABLE:
            return variableType((rbs_types_variable_t *)node, docLoc);
        default: {
            auto errLoc = locFromRange(docLoc, node->location->rg);
            if (auto e = ctx.beginError(errLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Unexpected node type `{}`", rbs_node_type_name(node));
            }

            return ast::MK::Untyped(docLoc);
        }
    }
}

} // namespace sorbet::rbs
