#include "MethodTypeTranslator.h"
#include "TypeTranslator.h"
#include "absl/strings/escaping.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "core/errors/rewriter.h"
#include "rewriter/util/Util.h"

using namespace sorbet::ast;

namespace sorbet::rbs {

namespace {

core::NameRef expressionName(core::MutableContext ctx, const ast::ExpressionPtr *expr) {
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
            [&](const ast::ExpressionPtr &p) {
                if (auto e = ctx.beginError(expr->loc(), core::errors::Rewriter::RBSError)) {
                    e.setHeader("Unexpected expression type: {}", p.showRaw(ctx));
                }

                name = ctx.state.enterNameUTF8("<error>");
                cursor = nullptr;
            });
    }

    return name;
}

struct RBSArg {
    core::LocOffsets loc;
    rbs_ast_symbol_t *name;
    rbs_node_t *type;
    bool optional;
};

void collectArgs(core::MutableContext ctx, core::LocOffsets docLoc, rbs_node_list_t *field, std::vector<RBSArg> &args,
                 bool optional) {
    for (rbs_node_list_node_t *list_node = field->head; list_node != nullptr; list_node = list_node->next) {
        if (list_node->node->type != RBS_TYPES_FUNCTION_PARAM) {
            if (auto e = ctx.beginError(docLoc, core::errors::Rewriter::RBSError)) {
                e.setHeader("Unexpected node type `{}` in function parameter list, expected `{}`",
                            rbs_node_type_name(list_node->node), "FunctionParam");
            }
            continue;
        }

        auto loc = TypeTranslator::nodeLoc(docLoc, list_node->node);
        auto node = (rbs_types_function_param_t *)list_node->node;
        auto arg = RBSArg{loc, node->name, node->type, optional};
        args.emplace_back(arg);
    }
}

void collectKeywords(core::MutableContext ctx, core::LocOffsets docLoc, rbs_hash_t *field, std::vector<RBSArg> &args,
                     bool optional) {
    for (rbs_hash_node_t *hash_node = field->head; hash_node != nullptr; hash_node = hash_node->next) {
        if (hash_node->key->type != RBS_AST_SYMBOL) {
            if (auto e = ctx.beginError(docLoc, core::errors::Rewriter::RBSError)) {
                e.setHeader("Unexpected node type `{}` in keyword argument name, expected `{}`",
                            rbs_node_type_name(hash_node->key), "Symbol");
            }
            continue;
        }

        if (hash_node->value->type != RBS_TYPES_FUNCTION_PARAM) {
            if (auto e = ctx.beginError(docLoc, core::errors::Rewriter::RBSError)) {
                e.setHeader("Unexpected node type `{}` in keyword argument value, expected `{}`",
                            rbs_node_type_name(hash_node->value), "FunctionParam");
            }
            continue;
        }

        rbs_ast_symbol_t *keyNode = (rbs_ast_symbol_t *)hash_node->key;
        rbs_types_function_param_t *valueNode = (rbs_types_function_param_t *)hash_node->value;
        auto arg = RBSArg{docLoc, keyNode, valueNode->type, optional};
        args.emplace_back(arg);
    }
}

} // namespace

sorbet::ast::ExpressionPtr MethodTypeTranslator::methodSignature(core::MutableContext ctx,
                                                                 sorbet::ast::MethodDef *methodDef,
                                                                 MethodType methodType,
                                                                 std::vector<Comment> annotations) {
    rbs_methodtype_t *node = methodType.node.get();

    if (node->type->type != RBS_TYPES_FUNCTION) {
        auto errLoc = TypeTranslator::nodeLoc(methodType.loc, node->type);
        if (auto e = ctx.beginError(errLoc, core::errors::Rewriter::RBSError)) {
            e.setHeader("Unexpected node type `{}` in method signature, expected `{}`", rbs_node_type_name(node->type),
                        "Function");
        }

        return ast::MK::Untyped(methodType.loc);
    }

    // Collect type parameters

    std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams;
    for (rbs_node_list_node_t *list_node = node->type_params->head; list_node != nullptr; list_node = list_node->next) {
        auto loc = TypeTranslator::nodeLoc(methodType.loc, list_node->node);

        if (list_node->node->type != RBS_AST_TYPEPARAM) {
            if (auto e = ctx.beginError(loc, core::errors::Rewriter::RBSError)) {
                e.setHeader("Unexpected node type `{}` in type parameter list, expected `{}`",
                            rbs_node_type_name(list_node->node), "TypeParam");
            }

            continue;
        }

        auto node = (rbs_ast_typeparam_t *)list_node->node;
        rbs_constant_t *constant = rbs_constant_pool_id_to_constant(fake_constant_pool, node->name->constant_id);
        std::string_view string(constant->start, constant->length);
        typeParams.emplace_back(loc, ctx.state.enterNameUTF8(string));
    }

    // Collect positionals

    rbs_types_function_t *functionType = (rbs_types_function_t *)node->type;
    std::vector<RBSArg> args;

    collectArgs(ctx, methodType.loc, functionType->required_positionals, args, false);

    rbs_node_list_t *optionalPositionals = functionType->optional_positionals;
    if (optionalPositionals && optionalPositionals->length > 0) {
        collectArgs(ctx, methodType.loc, optionalPositionals, args, false);
    }

    rbs_node_t *restPositionals = functionType->rest_positionals;
    if (restPositionals) {
        if (restPositionals->type != RBS_TYPES_FUNCTION_PARAM) {
            if (auto e = ctx.beginError(methodType.loc, core::errors::Rewriter::RBSError)) {
                e.setHeader("Unexpected node type `{}` in rest positional argument, expected `{}`",
                            rbs_node_type_name(restPositionals), "FunctionParam");
            }
        } else {
            auto loc = TypeTranslator::nodeLoc(methodType.loc, restPositionals);
            auto node = (rbs_types_function_param_t *)restPositionals;
            auto arg = RBSArg{loc, node->name, node->type, false};
            args.emplace_back(arg);
        }
    }

    rbs_node_list_t *trailingPositionals = functionType->trailing_positionals;
    if (trailingPositionals && trailingPositionals->length > 0) {
        collectArgs(ctx, methodType.loc, trailingPositionals, args, false);
    }

    // Collect keywords

    collectKeywords(ctx, methodType.loc, functionType->required_keywords, args, false);
    collectKeywords(ctx, methodType.loc, functionType->optional_keywords, args, false);

    rbs_node_t *restKeywords = functionType->rest_keywords;
    if (restKeywords) {
        if (restKeywords->type != RBS_TYPES_FUNCTION_PARAM) {
            if (auto e = ctx.beginError(methodType.loc, core::errors::Rewriter::RBSError)) {
                e.setHeader("Unexpected node type `{}` in rest keyword argument, expected `{}`",
                            rbs_node_type_name(restKeywords), "FunctionParam");
            }
        } else {
            auto loc = TypeTranslator::nodeLoc(methodType.loc, restKeywords);
            auto node = (rbs_types_function_param_t *)restKeywords;
            auto arg = RBSArg{loc, node->name, node->type, false};
            args.emplace_back(arg);
        }
    }

    // Collect block

    rbs_types_block_t *block = node->block;
    if (block) {
        // TODO: RBS doesn't have location on blocks yet
        auto arg = RBSArg{methodType.loc, nullptr, (rbs_node_t *)block, false};
        args.emplace_back(arg);
    }

    Send::ARGS_store sigParams;
    for (int i = 0; i < args.size(); i++) {
        auto &arg = args[i];
        core::NameRef name;
        auto nameSymbol = arg.name;

        if (nameSymbol) {
            // The RBS arg is named in the signature, so we use the explicit name used
            rbs_constant_t *nameConstant =
                rbs_constant_pool_id_to_constant(fake_constant_pool, nameSymbol->constant_id);
            std::string_view nameStr(nameConstant->start, nameConstant->length);
            name = ctx.state.enterNameUTF8(nameStr);
        } else {
            // The RBS arg is not named in the signature, so we get it from the method definition
            name = expressionName(ctx, &methodDef->args[i]);
        }

        auto type = TypeTranslator::toRBI(ctx, typeParams, arg.type, methodType.loc);
        if (arg.optional) {
            type = ast::MK::Nilable(arg.loc, std::move(type));
        }

        sigParams.emplace_back(ast::MK::Symbol(arg.loc, name));
        sigParams.emplace_back(std::move(type));
    }

    ENFORCE(sigParams.size() % 2 == 0, "Sig params must be arg name/type pairs");

    // Build the sig

    auto sigBuilder = ast::MK::Self(methodType.loc);
    bool isFinal = false;

    for (auto &annotation : annotations) {
        if (annotation.string == "final") {
            isFinal = true;
        } else if (annotation.string == "abstract") {
            sigBuilder = ast::MK::Send0(annotation.loc, std::move(sigBuilder), core::Names::abstract(), annotation.loc);
        } else if (annotation.string == "override") {
            sigBuilder =
                ast::MK::Send0(annotation.loc, std::move(sigBuilder), core::Names::override_(), annotation.loc);
        } else if (annotation.string == "override(allow_incompatible: true)") {
            Send::ARGS_store overrideArgs;
            overrideArgs.emplace_back(ast::MK::Symbol(annotation.loc, core::Names::allowIncompatible()));
            overrideArgs.emplace_back(ast::MK::True(annotation.loc));
            sigBuilder = ast::MK::Send(annotation.loc, std::move(sigBuilder), core::Names::override_(), annotation.loc,
                                       0, std::move(overrideArgs));
        }
    }

    if (typeParams.size() > 0) {
        Send::ARGS_store typeParamsStore;
        for (auto &param : typeParams) {
            typeParamsStore.emplace_back(ast::MK::Symbol(param.first, param.second));
        }
        sigBuilder = ast::MK::Send(methodType.loc, std::move(sigBuilder), core::Names::typeParameters(), methodType.loc,
                                   typeParamsStore.size(), std::move(typeParamsStore));
    }

    if (sigParams.size() > 0) {
        sigBuilder = ast::MK::Send(methodType.loc, std::move(sigBuilder), core::Names::params(), methodType.loc, 0,
                                   std::move(sigParams));
    }

    rbs_node_t *returnValue = functionType->return_type;
    if (returnValue->type == RBS_TYPES_BASES_VOID) {
        auto loc = TypeTranslator::nodeLoc(methodType.loc, returnValue);
        sigBuilder = ast::MK::Send0(loc, std::move(sigBuilder), core::Names::void_(), loc);
    } else {
        auto returnType = TypeTranslator::toRBI(ctx, typeParams, returnValue, methodType.loc);
        sigBuilder = ast::MK::Send1(methodType.loc, std::move(sigBuilder), core::Names::returns(), methodType.loc,
                                    std::move(returnType));
    }

    auto sigArgs = Send::ARGS_store();
    sigArgs.emplace_back(ast::MK::Constant(methodType.loc, core::Symbols::T_Sig_WithoutRuntime()));
    if (isFinal) {
        sigArgs.emplace_back(ast::MK::Symbol(methodType.loc, core::Names::final_()));
    }

    auto sig = ast::MK::Send(methodType.loc, ast::MK::Constant(methodType.loc, core::Symbols::Sorbet_Private_Static()),
                             core::Names::sig(), methodType.loc, sigArgs.size(), std::move(sigArgs));

    auto sigSend = ast::cast_tree<ast::Send>(sig);
    sigSend->setBlock(ast::MK::Block0(methodType.loc, std::move(sigBuilder)));
    sigSend->flags.isRewriterSynthesized = true;

    return sig;
}

sorbet::ast::ExpressionPtr MethodTypeTranslator::attrSignature(core::MutableContext ctx, sorbet::ast::Send *send,
                                                               Type attrType) {
    auto typeParams = std::vector<std::pair<core::LocOffsets, core::NameRef>>();
    auto sigBuilder = ast::MK::Self(attrType.loc);

    if (send->fun == core::Names::attrWriter()) {
        ENFORCE(send->numPosArgs() >= 1);

        // For attr writer, we need to add the param to the sig
        auto name = rewriter::ASTUtil::getAttrName(ctx, send->fun, send->getPosArg(0));
        Send::ARGS_store sigArgs;
        sigArgs.emplace_back(ast::MK::Symbol(name.second, name.first));
        sigArgs.emplace_back(TypeTranslator::toRBI(ctx, typeParams, attrType.node.get(), attrType.loc));
        sigBuilder = ast::MK::Send(attrType.loc, std::move(sigBuilder), core::Names::params(), attrType.loc, 0,
                                   std::move(sigArgs));
    }

    auto returnType = TypeTranslator::toRBI(ctx, typeParams, attrType.node.get(), attrType.loc);
    sigBuilder = ast::MK::Send1(attrType.loc, std::move(sigBuilder), core::Names::returns(), attrType.loc,
                                std::move(returnType));

    auto sig = ast::MK::Send1(attrType.loc, ast::MK::Constant(attrType.loc, core::Symbols::Sorbet_Private_Static()),
                              core::Names::sig(), attrType.loc,
                              ast::MK::Constant(attrType.loc, core::Symbols::T_Sig_WithoutRuntime()));
    auto sigSend = ast::cast_tree<ast::Send>(sig);
    sigSend->setBlock(ast::MK::Block0(attrType.loc, std::move(sigBuilder)));
    sigSend->flags.isRewriterSynthesized = true;

    return sig;
}

} // namespace sorbet::rbs
