#include "rbs/MethodTypeTranslator.h"

#include "absl/algorithm/container.h"
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

struct RBSArg {
    core::LocOffsets loc;
    rbs_ast_symbol_t *name;
    rbs_node_t *type;
};

ast::ExpressionPtr handleAnnotations(ast::ExpressionPtr sigBuilder, const vector<Comment> &annotations) {
    for (auto &annotation : annotations) {
        if (annotation.string == "final") {
            // no-op, `final` is handled in the `sig()` call later
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

    return sigBuilder;
}

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

void collectArgs(core::LocOffsets docLoc, rbs_node_list_t *field, vector<RBSArg> &args) {
    if (field == nullptr || field->length == 0) {
        return;
    }

    for (rbs_node_list_node_t *list_node = field->head; list_node != nullptr; list_node = list_node->next) {
        ENFORCE(list_node->node->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in function parameter list, expected `{}`",
                rbs_node_type_name(list_node->node), "FunctionParam");

        auto loc = locFromRange(docLoc, list_node->node->location->rg);
        auto node = (rbs_types_function_param_t *)list_node->node;
        auto arg = RBSArg{loc, node->name, node->type};
        args.emplace_back(arg);
    }
}

void collectKeywords(core::LocOffsets docLoc, rbs_hash_t *field, vector<RBSArg> &args) {
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

        auto loc = locFromRange(docLoc, hash_node->key->location->rg);
        rbs_ast_symbol_t *keyNode = (rbs_ast_symbol_t *)hash_node->key;
        rbs_types_function_param_t *valueNode = (rbs_types_function_param_t *)hash_node->value;
        auto arg = RBSArg{loc, keyNode, valueNode->type};
        args.emplace_back(arg);
    }
}

} // namespace

ast::ExpressionPtr MethodTypeTranslator::methodSignature(const ast::MethodDef *methodDef,
                                                         const rbs_methodtype_t *methodType,
                                                         const core::LocOffsets &methodTypeLoc,
                                                         const vector<Comment> &annotations) {
    const auto &node = *methodType;

    if (node.type->type != RBS_TYPES_FUNCTION) {
        auto errLoc = locFromRange(methodTypeLoc, node.type->location->rg);
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
        auto loc = locFromRange(methodTypeLoc, list_node->node->location->rg);

        ENFORCE(list_node->node->type == RBS_AST_TYPEPARAM,
                "Unexpected node type `{}` in type parameter list, expected `{}`", rbs_node_type_name(list_node->node),
                "TypeParam");

        auto node = (rbs_ast_typeparam_t *)list_node->node;
        auto str = parser.resolveConstant(node->name);
        typeParams.emplace_back(loc, ctx.state.enterNameUTF8(str));
    }

    // Collect positionals

    vector<RBSArg> args;

    collectArgs(methodTypeLoc, functionType->required_positionals, args);
    collectArgs(methodTypeLoc, functionType->optional_positionals, args);

    rbs_node_t *restPositionals = functionType->rest_positionals;
    if (restPositionals) {
        ENFORCE(restPositionals->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in rest positional argument, expected `{}`",
                rbs_node_type_name(restPositionals), "FunctionParam");

        auto loc = locFromRange(methodTypeLoc, restPositionals->location->rg);
        auto node = (rbs_types_function_param_t *)restPositionals;
        auto arg = RBSArg{loc, node->name, node->type};
        args.emplace_back(arg);
    }

    collectArgs(methodTypeLoc, functionType->trailing_positionals, args);

    // Collect keywords

    collectKeywords(methodTypeLoc, functionType->required_keywords, args);
    collectKeywords(methodTypeLoc, functionType->optional_keywords, args);

    rbs_node_t *restKeywords = functionType->rest_keywords;
    if (restKeywords) {
        ENFORCE(restKeywords->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in rest keyword argument, expected `{}`", rbs_node_type_name(restKeywords),
                "FunctionParam");

        auto loc = locFromRange(methodTypeLoc, restKeywords->location->rg);
        auto node = (rbs_types_function_param_t *)restKeywords;
        auto arg = RBSArg{loc, node->name, node->type};
        args.emplace_back(arg);
    }

    // Collect block

    auto *block = node.block;
    if (block) {
        // TODO: RBS doesn't have location on blocks yet
        auto arg = RBSArg{methodTypeLoc, nullptr, (rbs_node_t *)block};
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
            auto nameStr = parser.resolveConstant(nameSymbol);
            name = ctx.state.enterNameUTF8(nameStr);
        } else {
            if (i >= methodDef->args.size()) {
                if (auto e = ctx.beginError(methodTypeLoc, core::errors::Rewriter::RBSParameterMismatch)) {
                    e.setHeader("RBS signature has more parameters than in the method definition");
                }

                return ast::MK::EmptyTree();
            }

            // The RBS arg is not named in the signature, so we get it from the method definition
            name = expressionName(&methodDef->args[i]);
        }

        auto typeTranslator = TypeTranslator(ctx, typeParams, parser);
        auto type = typeTranslator.toExpressionPtr(arg.type, methodTypeLoc);
        sigParams.emplace_back(ast::MK::Symbol(arg.loc, name));
        sigParams.emplace_back(move(type));
    }

    ENFORCE(sigParams.size() % 2 == 0, "Sig params must be arg name/type pairs");

    // Build the sig

    auto sigBuilder = ast::MK::Self(methodTypeLoc);
    sigBuilder = handleAnnotations(std::move(sigBuilder), annotations);

    if (typeParams.size() > 0) {
        ast::Send::ARGS_store typeParamsStore;
        typeParamsStore.reserve(typeParams.size());

        for (auto &param : typeParams) {
            typeParamsStore.emplace_back(ast::MK::Symbol(param.first, param.second));
        }
        sigBuilder = ast::MK::Send(methodTypeLoc, move(sigBuilder), core::Names::typeParameters(), methodTypeLoc,
                                   typeParamsStore.size(), move(typeParamsStore));
    }

    if (sigParams.size() > 0) {
        sigBuilder =
            ast::MK::Send(methodTypeLoc, move(sigBuilder), core::Names::params(), methodTypeLoc, 0, move(sigParams));
    }

    rbs_node_t *returnValue = functionType->return_type;
    if (returnValue->type == RBS_TYPES_BASES_VOID) {
        auto loc = locFromRange(methodTypeLoc, returnValue->location->rg);
        sigBuilder = ast::MK::Send0(loc, move(sigBuilder), core::Names::void_(), loc);
    } else {
        auto typeTranslator = TypeTranslator(ctx, typeParams, parser);
        auto returnType = typeTranslator.toExpressionPtr(returnValue, methodTypeLoc);
        sigBuilder =
            ast::MK::Send1(methodTypeLoc, move(sigBuilder), core::Names::returns(), methodTypeLoc, move(returnType));
    }

    auto sigArgs = ast::Send::ARGS_store();
    sigArgs.emplace_back(ast::MK::Constant(methodTypeLoc, core::Symbols::T_Sig_WithoutRuntime()));

    bool isFinal = absl::c_any_of(annotations, [](const Comment &annotation) { return annotation.string == "final"; });

    if (isFinal) {
        sigArgs.emplace_back(ast::MK::Symbol(methodTypeLoc, core::Names::final_()));
    }

    auto sig = ast::MK::Send(methodTypeLoc, ast::MK::Constant(methodTypeLoc, core::Symbols::Sorbet_Private_Static()),
                             core::Names::sig(), methodTypeLoc, sigArgs.size(), move(sigArgs));

    auto sigSend = ast::cast_tree<ast::Send>(sig);
    sigSend->setBlock(ast::MK::Block0(methodTypeLoc, move(sigBuilder)));
    sigSend->flags.isRewriterSynthesized = true;

    return sig;
}

ast::ExpressionPtr MethodTypeTranslator::attrSignature(const ast::Send *send, rbs_node_t *type,
                                                       const core::LocOffsets &typeLoc,
                                                       const vector<Comment> &annotations) {
    auto typeParams = vector<pair<core::LocOffsets, core::NameRef>>();
    auto sigBuilder = ast::MK::Self(typeLoc.copyWithZeroLength());
    sigBuilder = handleAnnotations(std::move(sigBuilder), annotations);

    if (send->numPosArgs() == 0) {
        if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::RBSUnsupported)) {
            e.setHeader("RBS signatures do not support accessor without arguments");
        }

        return ast::MK::EmptyTree();
    }

    auto typeTranslator = TypeTranslator(ctx, typeParams, parser);
    auto returnType = typeTranslator.toExpressionPtr(type, typeLoc);

    if (send->fun == core::Names::attrWriter()) {
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
        sigArgs.emplace_back(returnType.deepCopy());
        sigBuilder = ast::MK::Send(typeLoc, move(sigBuilder), core::Names::params(), typeLoc, 0, move(sigArgs));
    }

    sigBuilder = ast::MK::Send1(typeLoc, move(sigBuilder), core::Names::returns(), typeLoc, move(returnType));

    auto sig =
        ast::MK::Send1(typeLoc, ast::MK::Constant(typeLoc, core::Symbols::Sorbet_Private_Static()), core::Names::sig(),
                       typeLoc, ast::MK::Constant(typeLoc, core::Symbols::T_Sig_WithoutRuntime()));
    auto sigSend = ast::cast_tree<ast::Send>(sig);
    sigSend->setBlock(ast::MK::Block0(typeLoc, move(sigBuilder)));
    sigSend->flags.isRewriterSynthesized = true;

    return sig;
}

} // namespace sorbet::rbs
