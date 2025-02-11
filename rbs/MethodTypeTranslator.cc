#include "rbs/MethodTypeTranslator.h"

#include "absl/strings/escaping.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "core/errors/internal.h"
#include "core/errors/rewriter.h"
#include "rbs/TypeTranslator.h"
#include "rewriter/util/Util.h"

using namespace std;

namespace sorbet::rbs {

namespace {

core::NameRef expressionName(const ast::ExpressionPtr *expr) {
    core::NameRef name;
    auto cursor = expr;

    while (cursor != nullptr) {
        typecase(
            *cursor,
            [&](const ast::UnresolvedIdent &p) {
                name = p.name;
                cursor = nullptr;
            },
            [&](const ast::OptionalArg &p) { cursor = &p.expr; }, [&](const ast::RestArg &p) { cursor = &p.expr; },
            [&](const ast::KeywordArg &p) { cursor = &p.expr; }, [&](const ast::BlockArg &p) { cursor = &p.expr; },
            [&](const ast::ExpressionPtr &p) { Exception::raise("Unexpected expression type: {}", p.nodeName()); });
    }

    return name;
}

struct RBSArg {
    core::LocOffsets loc;
    rbs_ast_symbol_t *name;
    rbs_node_t *type;
};

void collectArgs(core::MutableContext ctx, core::LocOffsets docLoc, rbs_node_list_t *field, vector<RBSArg> &args) {
    if (field == nullptr || field->length == 0) {
        return;
    }

    for (rbs_node_list_node_t *list_node = field->head; list_node != nullptr; list_node = list_node->next) {
        ENFORCE(list_node->node->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in function parameter list, expected `{}`",
                rbs_node_type_name(list_node->node), "FunctionParam");

        auto loc = TypeTranslator::nodeLoc(docLoc, list_node->node);
        auto node = (rbs_types_function_param_t *)list_node->node;
        auto arg = RBSArg{loc, node->name, node->type};
        args.emplace_back(arg);
    }
}

void collectKeywords(core::MutableContext ctx, core::LocOffsets docLoc, rbs_hash_t *field, vector<RBSArg> &args) {
    if (field == nullptr) {
        return;
    }

    for (rbs_hash_node_t *hash_node = field->head; hash_node != nullptr; hash_node = hash_node->next) {
        ENFORCE(hash_node->key->type == RBS_AST_SYMBOL,
                "Unexpected node type `{}` in keyword argument name, expected `{}`", rbs_node_type_name(hash_node->key),
                "Symbol");

        ENFORCE(hash_node->value->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in keyword argument value, expected `{}`",
                rbs_node_type_name(hash_node->value), "FunctionParam");

        rbs_ast_symbol_t *keyNode = (rbs_ast_symbol_t *)hash_node->key;
        rbs_types_function_param_t *valueNode = (rbs_types_function_param_t *)hash_node->value;
        auto arg = RBSArg{docLoc, keyNode, valueNode->type};
        args.emplace_back(arg);
    }
}

} // namespace

ast::ExpressionPtr MethodTypeTranslator::methodSignature(core::MutableContext ctx, const ast::MethodDef *methodDef,
                                                         const MethodType methodType,
                                                         const vector<Comment> &annotations) {
    const auto &node = *methodType.node;

    if (node.type->type != RBS_TYPES_FUNCTION) {
        auto errLoc = TypeTranslator::nodeLoc(methodType.loc, node.type);
        if (auto e = ctx.beginError(errLoc, core::errors::Rewriter::RBSUnsupported)) {
            e.setHeader("Unexpected node type `{}` in method signature, expected `{}`", rbs_node_type_name(node.type),
                        "Function");
        }

        return ast::MK::EmptyTree();
    }
    auto *functionType = (rbs_types_function_t *)node.type;

    // Collect type parameters

    vector<pair<core::LocOffsets, core::NameRef>> typeParams;
    for (rbs_node_list_node_t *list_node = node.type_params->head; list_node != nullptr; list_node = list_node->next) {
        auto loc = TypeTranslator::nodeLoc(methodType.loc, list_node->node);

        ENFORCE(list_node->node->type == RBS_AST_TYPEPARAM,
                "Unexpected node type `{}` in type parameter list, expected `{}`", rbs_node_type_name(list_node->node),
                "TypeParam");

        auto node = (rbs_ast_typeparam_t *)list_node->node;
        rbs_constant_t *constant = rbs_constant_pool_id_to_constant(fake_constant_pool, node->name->constant_id);
        string_view string(constant->start, constant->length);
        typeParams.emplace_back(loc, ctx.state.enterNameUTF8(string));
    }

    // Collect positionals

    vector<RBSArg> args;

    collectArgs(ctx, methodType.loc, functionType->required_positionals, args);
    collectArgs(ctx, methodType.loc, functionType->optional_positionals, args);

    rbs_node_t *restPositionals = functionType->rest_positionals;
    if (restPositionals) {
        ENFORCE(restPositionals->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in rest positional argument, expected `{}`",
                rbs_node_type_name(restPositionals), "FunctionParam");

        auto loc = TypeTranslator::nodeLoc(methodType.loc, restPositionals);
        auto node = (rbs_types_function_param_t *)restPositionals;
        auto arg = RBSArg{loc, node->name, node->type};
        args.emplace_back(arg);
    }

    collectArgs(ctx, methodType.loc, functionType->trailing_positionals, args);

    // Collect keywords

    collectKeywords(ctx, methodType.loc, functionType->required_keywords, args);
    collectKeywords(ctx, methodType.loc, functionType->optional_keywords, args);

    rbs_node_t *restKeywords = functionType->rest_keywords;
    if (restKeywords) {
        ENFORCE(restKeywords->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in rest keyword argument, expected `{}`", rbs_node_type_name(restKeywords),
                "FunctionParam");

        auto loc = TypeTranslator::nodeLoc(methodType.loc, restKeywords);
        auto node = (rbs_types_function_param_t *)restKeywords;
        auto arg = RBSArg{loc, node->name, node->type};
        args.emplace_back(arg);
    }

    // Collect block

    auto *block = node.block;
    if (block) {
        // TODO: RBS doesn't have location on blocks yet
        auto arg = RBSArg{methodType.loc, nullptr, (rbs_node_t *)block};
        args.emplace_back(arg);
    }

    ast::Send::ARGS_store sigParams;
    sigParams.reserve(args.size() * 2);

    for (int i = 0; i < args.size(); i++) {
        auto &arg = args[i];
        core::NameRef name;
        auto nameSymbol = arg.name;

        if (nameSymbol) {
            // The RBS arg is named in the signature, so we use the explicit name used
            rbs_constant_t *nameConstant =
                rbs_constant_pool_id_to_constant(fake_constant_pool, nameSymbol->constant_id);
            string_view nameStr(nameConstant->start, nameConstant->length);
            name = ctx.state.enterNameUTF8(nameStr);
        } else {
            // The RBS arg is not named in the signature, so we get it from the method definition
            name = expressionName(&methodDef->args[i]);
        }

        auto type = TypeTranslator::toRBI(ctx, typeParams, arg.type, methodType.loc);
        sigParams.emplace_back(ast::MK::Symbol(arg.loc, name));
        sigParams.emplace_back(move(type));
    }

    ENFORCE(sigParams.size() % 2 == 0, "Sig params must be arg name/type pairs");

    // Build the sig

    auto sigBuilder = ast::MK::Self(methodType.loc);
    bool isFinal = false;

    for (auto &annotation : annotations) {
        if (annotation.string == "final") {
            isFinal = true;
        } else if (annotation.string == "abstract") {
            sigBuilder = ast::MK::Send0(annotation.loc, move(sigBuilder), core::Names::abstract(), annotation.loc);
        } else if (annotation.string == "overridable") {
            sigBuilder = ast::MK::Send0(annotation.loc, move(sigBuilder), core::Names::overridable(), annotation.loc);
        } else if (annotation.string == "override") {
            sigBuilder = ast::MK::Send0(annotation.loc, move(sigBuilder), core::Names::override_(), annotation.loc);
        } else if (annotation.string == "override(allow_incompatible: true)") {
            ast::Send::ARGS_store overrideArgs;
            overrideArgs.emplace_back(ast::MK::Symbol(annotation.loc, core::Names::allowIncompatible()));
            overrideArgs.emplace_back(ast::MK::True(annotation.loc));
            sigBuilder = ast::MK::Send(annotation.loc, move(sigBuilder), core::Names::override_(), annotation.loc, 0,
                                       move(overrideArgs));
        }
    }

    if (typeParams.size() > 0) {
        ast::Send::ARGS_store typeParamsStore;
        typeParamsStore.reserve(typeParams.size());

        for (auto &param : typeParams) {
            typeParamsStore.emplace_back(ast::MK::Symbol(param.first, param.second));
        }
        sigBuilder = ast::MK::Send(methodType.loc, move(sigBuilder), core::Names::typeParameters(), methodType.loc,
                                   typeParamsStore.size(), move(typeParamsStore));
    }

    if (sigParams.size() > 0) {
        sigBuilder =
            ast::MK::Send(methodType.loc, move(sigBuilder), core::Names::params(), methodType.loc, 0, move(sigParams));
    }

    rbs_node_t *returnValue = functionType->return_type;
    if (returnValue->type == RBS_TYPES_BASES_VOID) {
        auto loc = TypeTranslator::nodeLoc(methodType.loc, returnValue);
        sigBuilder = ast::MK::Send0(loc, move(sigBuilder), core::Names::void_(), loc);
    } else {
        auto returnType = TypeTranslator::toRBI(ctx, typeParams, returnValue, methodType.loc);
        sigBuilder =
            ast::MK::Send1(methodType.loc, move(sigBuilder), core::Names::returns(), methodType.loc, move(returnType));
    }

    auto sigArgs = ast::Send::ARGS_store();
    sigArgs.emplace_back(ast::MK::Constant(methodType.loc, core::Symbols::T_Sig_WithoutRuntime()));
    if (isFinal) {
        sigArgs.emplace_back(ast::MK::Symbol(methodType.loc, core::Names::final_()));
    }

    auto sig = ast::MK::Send(methodType.loc, ast::MK::Constant(methodType.loc, core::Symbols::Sorbet_Private_Static()),
                             core::Names::sig(), methodType.loc, sigArgs.size(), move(sigArgs));

    auto sigSend = ast::cast_tree<ast::Send>(sig);
    sigSend->setBlock(ast::MK::Block0(methodType.loc, move(sigBuilder)));
    sigSend->flags.isRewriterSynthesized = true;

    return sig;
}

ast::ExpressionPtr MethodTypeTranslator::attrSignature(core::MutableContext ctx, const ast::Send *send,
                                                       const Type attrType) {
    auto typeParams = vector<pair<core::LocOffsets, core::NameRef>>();
    auto sigBuilder = ast::MK::Self(attrType.loc.copyWithZeroLength());

    if (send->fun == core::Names::attrWriter()) {
        if (send->numPosArgs() == 0) {
            if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS signatures do not support attr_writer without arguments");
            }

            return ast::MK::EmptyTree();
        }

        if (send->numPosArgs() > 1) {
            if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS signatures for attr_writer do not support multiple arguments");
            }

            return ast::MK::EmptyTree();
        }

        // For attr writer, we need to add the param to the sig
        auto name = rewriter::ASTUtil::getAttrName(ctx, send->fun, send->getPosArg(0));
        ast::Send::ARGS_store sigArgs;
        sigArgs.emplace_back(ast::MK::Symbol(name.second, name.first));
        sigArgs.emplace_back(TypeTranslator::toRBI(ctx, typeParams, attrType.node.get(), attrType.loc));
        sigBuilder =
            ast::MK::Send(attrType.loc, move(sigBuilder), core::Names::params(), attrType.loc, 0, move(sigArgs));
    }

    auto returnType = TypeTranslator::toRBI(ctx, typeParams, attrType.node.get(), attrType.loc);
    sigBuilder = ast::MK::Send1(attrType.loc, move(sigBuilder), core::Names::returns(), attrType.loc, move(returnType));

    auto sig = ast::MK::Send1(attrType.loc, ast::MK::Constant(attrType.loc, core::Symbols::Sorbet_Private_Static()),
                              core::Names::sig(), attrType.loc,
                              ast::MK::Constant(attrType.loc, core::Symbols::T_Sig_WithoutRuntime()));
    auto sigSend = ast::cast_tree<ast::Send>(sig);
    sigSend->setBlock(ast::MK::Block0(attrType.loc, move(sigBuilder)));
    sigSend->flags.isRewriterSynthesized = true;

    return sig;
}

} // namespace sorbet::rbs
