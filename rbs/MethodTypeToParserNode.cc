#include "rbs/MethodTypeToParserNode.h"

#include "absl/algorithm/container.h"
#include "absl/strings/escaping.h"
#include "common/typecase.h"
#include "core/GlobalState.h"
#include "core/errors/internal.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "rbs/TypeToParserNode.h"
#include "rewriter/util/Util.h"

using namespace std;

namespace sorbet::rbs {

namespace {

struct RBSArg {
    core::LocOffsets loc;
    core::LocOffsets nameLoc;
    rbs_ast_symbol_t *name;
    rbs_node_t *type;

    enum class Kind {
        Positional,
        OptionalPositional,
        RestPositional,
        Keyword,
        OptionalKeyword,
        RestKeyword,
        Block,
    };

    Kind kind;
};

core::LocOffsets adjustNameLoc(const RBSDeclaration &declaration, rbs_node_t *node) {
    auto range = node->location->rg;

    auto nameRange = node->location->children->entries[0].rg;
    if (nameRange.start != -1 && nameRange.end != -1) {
        range.start.char_pos = nameRange.start;
        range.end.char_pos = nameRange.end;
    }

    return declaration.typeLocFromRange(range);
}

bool isSelfOrKernel(parser::Node *node) {
    if (parser::isa_node<parser::Self>(node)) {
        return true;
    }

    if (auto constant = parser::cast_node<parser::Const>(node)) {
        return constant->name == core::Names::Constants::Kernel() &&
               (constant->scope == nullptr || parser::isa_node<parser::Cbase>(constant->scope.get()));
    }

    return false;
}

bool isRaise(parser::Node *node) {
    auto raise = parser::cast_node<parser::Send>(node);

    if (!raise) {
        return false;
    }

    if (raise->method != core::Names::raise()) {
        return false;
    }

    return raise->receiver == nullptr || isSelfOrKernel(raise->receiver.get());
}

core::AutocorrectSuggestion autocorrectAbstractBody(core::MutableContext ctx, parser::Node *method,
                                                    core::LocOffsets method_declLoc, parser::Node *method_body) {
    core::LocOffsets editLoc;
    string corrected;

    auto lineStart = core::Loc::pos2Detail(ctx.file.data(ctx), method_declLoc.endPos()).line;
    auto lineEnd = core::Loc::pos2Detail(ctx.file.data(ctx), method->loc.endPos()).line;

    if (method_body) {
        editLoc = method_body->loc;
        corrected = "raise \"Abstract method called\"";
    } else if (lineStart == lineEnd) {
        editLoc = method_declLoc.copyEndWithZeroLength().join(method->loc.copyEndWithZeroLength());
        corrected = " = raise(\"Abstract method called\")";
    } else {
        editLoc = method_declLoc.copyEndWithZeroLength();
        auto [_endLoc, indentLength] = ctx.locAt(method->loc).findStartOfIndentation(ctx);
        string indent(indentLength + 2, ' ');
        corrected = "\n" + indent + "raise \"Abstract method called\"";
    }

    return core::AutocorrectSuggestion{fmt::format("Add `raise` to the method body"),
                                       {core::AutocorrectSuggestion::Edit{ctx.locAt(editLoc), corrected}}};
}

void ensureAbstractMethodRaises(core::MutableContext ctx, const parser::Node *node) {
    if (auto method = parser::cast_node<parser::DefMethod>((parser::Node *)node)) {
        if (isRaise(method->body.get())) {
            // If the method raises properly, we remove the body the body to not error later (see error 5019)
            method->body = nullptr;
            return;
        }

        if (auto e = ctx.beginIndexerError(node->loc, core::errors::Rewriter::RBSAbstractMethodNoRaises)) {
            e.setHeader("Methods declared @abstract with an RBS comment must always raise");
            auto autocorrect = autocorrectAbstractBody(ctx, method, method->declLoc, method->body.get());
            e.addAutocorrect(move(autocorrect));
        }
    } else if (auto method = parser::cast_node<parser::DefS>((parser::Node *)node)) {
        if (isRaise(method->body.get())) {
            // If the method raises properly, we remove the body the body to not error later (see error 5019)
            method->body = nullptr;
            return;
        }

        if (auto e = ctx.beginIndexerError(node->loc, core::errors::Rewriter::RBSAbstractMethodNoRaises)) {
            e.setHeader("Methods declared @abstract with an RBS comment must always raise");
            auto autocorrect = autocorrectAbstractBody(ctx, method, method->declLoc, method->body.get());
            e.addAutocorrect(move(autocorrect));
        }
    }
}

unique_ptr<parser::Node> handleAnnotations(core::MutableContext ctx, const parser::Node *node,
                                           unique_ptr<parser::Node> sigBuilder, const vector<Comment> &annotations) {
    for (auto &annotation : annotations) {
        if (annotation.string == "final") {
            // no-op, `final` is handled in the `sig()` call later
        } else if (annotation.string == "abstract") {
            sigBuilder =
                parser::MK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::abstract(), annotation.typeLoc);

            ensureAbstractMethodRaises(ctx, node);
        } else if (annotation.string == "overridable") {
            sigBuilder =
                parser::MK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::overridable(), annotation.typeLoc);
        } else if (annotation.string == "override") {
            sigBuilder =
                parser::MK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::override_(), annotation.typeLoc);
        } else if (annotation.string == "override(allow_incompatible: true)") {
            auto pairs = parser::NodeVec();
            auto key = parser::MK::Symbol(annotation.typeLoc, core::Names::allowIncompatible());
            auto value = parser::MK::True(annotation.typeLoc);
            pairs.emplace_back(make_unique<parser::Pair>(annotation.typeLoc, move(key), move(value)));
            auto hash = parser::MK::Hash(annotation.typeLoc, true, move(pairs));

            auto args = parser::NodeVec();
            args.emplace_back(move(hash));

            sigBuilder = parser::MK::Send(annotation.typeLoc, move(sigBuilder), core::Names::override_(),
                                          annotation.typeLoc, move(args));
        }
    }

    return sigBuilder;
}

core::NameRef nodeName(const parser::Node *node) {
    core::NameRef name;

    typecase(
        node, [&](const parser::Arg *a) { name = a->name; }, [&](const parser::Restarg *a) { name = a->name; },
        [&](const parser::Kwarg *a) { name = a->name; }, [&](const parser::Blockarg *a) { name = a->name; },
        [&](const parser::Kwoptarg *a) { name = a->name; }, [&](const parser::Optarg *a) { name = a->name; },
        [&](const parser::Kwrestarg *a) { name = a->name; }, [&](const parser::Shadowarg *a) { name = a->name; },
        [&](const parser::Symbol *s) { name = s->val; },
        [&](const parser::Node *other) {
            Exception::raise("Unexpected expression type: {}", ((parser::Node *)node)->nodeName());
        });

    return name;
}

string argKindToString(RBSArg::Kind kind) {
    switch (kind) {
        case RBSArg::Kind::Positional:
            return "positional";
        case RBSArg::Kind::OptionalPositional:
            return "optional positional";
        case RBSArg::Kind::RestPositional:
            return "rest positional";
        case RBSArg::Kind::Keyword:
            return "keyword";
        case RBSArg::Kind::OptionalKeyword:
            return "optional keyword";
        case RBSArg::Kind::RestKeyword:
            return "rest keyword";
        case RBSArg::Kind::Block:
            return "block";
    }
}

string nodeKindToString(const parser::Node *node) {
    string kind;

    typecase(
        node, [&](const parser::Arg *parserArg) { kind = "positional"; },
        [&](const parser::Optarg *parserArg) { kind = "optional positional"; },
        [&](const parser::Restarg *parserArg) { kind = "rest positional"; },
        [&](const parser::Kwarg *parserArg) { kind = "keyword"; },
        [&](const parser::Kwoptarg *parserArg) { kind = "optional keyword"; },
        [&](const parser::Kwrestarg *parserArg) { kind = "rest keyword"; },
        [&](const parser::Blockarg *parserArg) { kind = "block"; },
        [&](const parser::Node *other) {
            Exception::raise("Unexpected expression type: {}", ((parser::Node *)node)->nodeName());
        });

    return kind;
}

optional<core::AutocorrectSuggestion> autocorrectArg(core::MutableContext ctx, const parser::Node *methodArg,
                                                     RBSArg arg, unique_ptr<parser::Node> type) {
    if (arg.kind == RBSArg::Kind::Block || parser::isa_node<parser::Blockarg>((parser::Node *)methodArg)) {
        // Block arguments are not autocorrected
        return nullopt;
    }

    string corrected;
    auto source = ctx.file.data(ctx.state).source();
    auto typeString = source.substr(type->loc.beginPos(), type->loc.endPos() - type->loc.beginPos());

    typecase(
        methodArg,
        // Should be: `Type name`
        [&](const parser::Arg *a) {
            if (arg.name) {
                auto nameString = nodeName(a).toString(ctx.state);
                corrected = fmt::format("{} {}", typeString, nameString);
            } else {
                corrected = fmt::format("{}", typeString);
            }
        },
        // Should be: `?Type name`
        [&](const parser::Optarg *a) {
            if (arg.name) {
                auto nameString = nodeName(a).toString(ctx.state);
                corrected = fmt::format("?{} {}", typeString, nameString);
            } else {
                corrected = fmt::format("?{}", typeString);
            }
        },
        // Should be: `*Type name`
        [&](const parser::Restarg *a) {
            if (arg.name) {
                auto nameString = nodeName(a).toString(ctx.state);
                corrected = fmt::format("*{} {}", typeString, nameString);
            } else {
                corrected = fmt::format("*{}", typeString);
            }
        },
        // Should be: `name: Type`
        [&](const parser::Kwarg *a) {
            auto nameString = nodeName(a).toString(ctx.state);
            corrected = fmt::format("{}: {}", nameString, typeString);
        },
        // Should be: `?name: Type`
        [&](const parser::Kwoptarg *a) {
            auto nameString = nodeName(a).toString(ctx.state);
            corrected = fmt::format("?{}: {}", nameString, typeString);
        },
        // Should be: `**Type name`
        [&](const parser::Kwrestarg *a) {
            if (arg.name) {
                auto nameString = nodeName(a).toString(ctx.state);
                corrected = fmt::format("**{} {}", typeString, nameString);
            } else {
                corrected = fmt::format("**{}", typeString);
            }
        },
        [&](const parser::Node *other) { Exception::raise("Unexpected expression type: {}", other->nodeName()); });

    core::LocOffsets loc = arg.loc;

    // Adjust the location to account for the autocorrect
    // TODO: remove this once we fixed the location generation by the parser
    if (arg.kind == RBSArg::Kind::OptionalPositional || arg.kind == RBSArg::Kind::RestPositional ||
        arg.kind == RBSArg::Kind::OptionalKeyword) {
        loc.beginLoc -= 1;
    } else if (arg.kind == RBSArg::Kind::RestKeyword) {
        loc.beginLoc -= 2;
    }

    return core::AutocorrectSuggestion{fmt::format("Replace with `{}`", argKindToString(arg.kind)),
                                       {core::AutocorrectSuggestion::Edit{ctx.locAt(loc), corrected}}};
}

bool checkParameterKindMatch(const RBSArg &arg, const parser::Node *methodArg) {
    auto kindMatch = false;

    typecase(
        methodArg, [&](const parser::Arg *parserArg) { kindMatch = arg.kind == RBSArg::Kind::Positional; },
        [&](const parser::Optarg *parserArg) { kindMatch = arg.kind == RBSArg::Kind::OptionalPositional; },
        [&](const parser::Restarg *parserArg) { kindMatch = arg.kind == RBSArg::Kind::RestPositional; },
        [&](const parser::Kwarg *parserArg) { kindMatch = arg.kind == RBSArg::Kind::Keyword; },
        [&](const parser::Kwoptarg *parserArg) { kindMatch = arg.kind == RBSArg::Kind::OptionalKeyword; },
        [&](const parser::Kwrestarg *parserArg) { kindMatch = arg.kind == RBSArg::Kind::RestKeyword; },
        [&](const parser::Blockarg *parserArg) { kindMatch = arg.kind == RBSArg::Kind::Block; },
        [&](const parser::Node *other) { Exception::raise("Unexpected expression type: {}", methodArg->nodeName()); });

    return kindMatch;
}

parser::Params *getMethodParams(const parser::Node *node) {
    parser::Node *args;

    typecase(
        node, [&](const parser::DefMethod *defMethod) { args = defMethod->params.get(); },
        [&](const parser::DefS *defS) { args = defS->params.get(); },
        [&](const parser::Node *other) {
            Exception::raise("Unexpected expression type: {}", ((parser::Node *)node)->nodeName());
        });

    return parser::cast_node<parser::Params>(args);
}

void collectArgs(const RBSDeclaration &declaration, rbs_node_list_t *field, vector<RBSArg> &args, RBSArg::Kind kind) {
    if (field == nullptr || field->length == 0) {
        return;
    }

    for (rbs_node_list_node_t *list_node = field->head; list_node != nullptr; list_node = list_node->next) {
        ENFORCE(list_node->node->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in function parameter list, expected `{}`",
                rbs_node_type_name(list_node->node), "FunctionParam");

        auto loc = declaration.typeLocFromRange(list_node->node->location->rg);
        auto nameLoc = adjustNameLoc(declaration, list_node->node);
        auto node = (rbs_types_function_param_t *)list_node->node;
        auto arg = RBSArg{loc, nameLoc, node->name, node->type, kind};
        args.emplace_back(arg);
    }
}

void collectKeywords(const RBSDeclaration &declaration, rbs_hash_t *field, vector<RBSArg> &args, RBSArg::Kind kind) {
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

        auto nameLoc = declaration.typeLocFromRange(hash_node->key->location->rg);
        auto loc = nameLoc.join(declaration.typeLocFromRange(hash_node->value->location->rg));
        rbs_ast_symbol_t *keyNode = (rbs_ast_symbol_t *)hash_node->key;
        rbs_types_function_param_t *valueNode = (rbs_types_function_param_t *)hash_node->value;
        auto arg = RBSArg{loc, nameLoc, keyNode, valueNode->type, kind};
        args.emplace_back(arg);
    }
}

} // namespace

unique_ptr<parser::Node> MethodTypeToParserNode::methodSignature(const parser::Node *methodDef,
                                                                 const rbs_method_type_t *methodType,
                                                                 const RBSDeclaration &declaration,
                                                                 const vector<Comment> &annotations) {
    const auto &node = *methodType;
    // Method signatures can have multiple lines, so we need
    // - full type location
    // - first line type location
    // - token specific location
    // for different parts of the signature
    //
    // For example,
    // - The whole signature block needs to be mapped to the full type location
    // - The `sig` call is always mapped to just the first line of the signature
    // - The return value needs to has a calculated location based on the token range
    auto fullTypeLoc = declaration.fullTypeLoc();
    auto firstLineTypeLoc = declaration.firstLineTypeLoc();
    auto commentLoc = declaration.commentLoc();

    if (node.type->type != RBS_TYPES_FUNCTION) {
        auto errLoc = declaration.typeLocFromRange(node.type->location->rg);
        if (auto e = ctx.beginIndexerError(errLoc, core::errors::Rewriter::RBSUnsupported)) {
            e.setHeader("Unexpected node type `{}` in method signature, expected `{}`", rbs_node_type_name(node.type),
                        "Function");
        }

        return nullptr;
    }
    auto *functionType = (rbs_types_function_t *)node.type;

    // Collect type parameters

    vector<pair<core::LocOffsets, core::NameRef>> typeParams;
    for (rbs_node_list_node_t *list_node = node.type_params->head; list_node != nullptr; list_node = list_node->next) {
        auto loc = declaration.typeLocFromRange(list_node->node->location->rg);

        ENFORCE(list_node->node->type == RBS_AST_TYPE_PARAM,
                "Unexpected node type `{}` in type parameter list, expected `{}`", rbs_node_type_name(list_node->node),
                "TypeParam");

        auto node = (rbs_ast_type_param_t *)list_node->node;
        auto str = parser.resolveConstant(node->name);
        typeParams.emplace_back(loc, ctx.state.enterNameUTF8(str));
    }

    // Collect positionals

    vector<RBSArg> args;

    collectArgs(declaration, functionType->required_positionals, args, RBSArg::Kind::Positional);
    collectArgs(declaration, functionType->optional_positionals, args, RBSArg::Kind::OptionalPositional);

    rbs_node_t *restPositionals = functionType->rest_positionals;
    if (restPositionals) {
        ENFORCE(restPositionals->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in rest positional argument, expected `{}`",
                rbs_node_type_name(restPositionals), "FunctionParam");

        auto loc = declaration.typeLocFromRange(restPositionals->location->rg);
        auto nameLoc = adjustNameLoc(declaration, restPositionals);
        auto node = (rbs_types_function_param_t *)restPositionals;
        auto arg = RBSArg{loc, nameLoc, node->name, node->type, RBSArg::Kind::RestPositional};
        args.emplace_back(arg);
    }

    collectArgs(declaration, functionType->trailing_positionals, args, RBSArg::Kind::Positional);

    // Collect keywords

    collectKeywords(declaration, functionType->required_keywords, args, RBSArg::Kind::Keyword);
    collectKeywords(declaration, functionType->optional_keywords, args, RBSArg::Kind::OptionalKeyword);

    rbs_node_t *restKeywords = functionType->rest_keywords;
    if (restKeywords) {
        ENFORCE(restKeywords->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in rest keyword argument, expected `{}`", rbs_node_type_name(restKeywords),
                "FunctionParam");

        auto loc = declaration.typeLocFromRange(restKeywords->location->rg);
        auto nameLoc = adjustNameLoc(declaration, restKeywords);
        auto node = (rbs_types_function_param_t *)restKeywords;
        auto arg = RBSArg{loc, nameLoc, node->name, node->type, RBSArg::Kind::RestKeyword};
        args.emplace_back(arg);
    }

    // Collect block

    auto *block = node.block;
    if (block) {
        auto loc = declaration.typeLocFromRange(block->base.location->rg);
        auto arg = RBSArg{loc, loc, nullptr, (rbs_node_t *)block, RBSArg::Kind::Block};
        args.emplace_back(arg);
    }

    auto sigParams = parser::NodeVec();
    sigParams.reserve(args.size());

    auto typeToParserNode = TypeToParserNode(ctx, typeParams, parser);

    auto methodParams = getMethodParams(methodDef);
    for (int i = 0; i < args.size(); i++) {
        auto &arg = args[i];
        auto type = typeToParserNode.toParserNode(arg.type, declaration);

        if (!methodParams || i >= methodParams->params.size()) {
            if (auto e = ctx.beginIndexerError(fullTypeLoc, core::errors::Rewriter::RBSParameterMismatch)) {
                e.setHeader("RBS signature has more parameters than in the method definition");
            }

            return nullptr;
        }

        auto methodParam = methodParams->params[i].get();

        if (!checkParameterKindMatch(arg, methodParam)) {
            if (auto e = ctx.beginIndexerError(arg.loc, core::errors::Rewriter::RBSIncorrectParameterKind)) {
                e.setHeader("Argument kind mismatch for `{}`, method declares `{}`, but RBS signature declares `{}`",
                            nodeName(methodParam).show(ctx.state), nodeKindToString(methodParam),
                            argKindToString(arg.kind));

                e.maybeAddAutocorrect(autocorrectArg(ctx, methodParam, arg, type->deepCopy()));
            }
        }

        if (auto nameSymbol = arg.name) {
            // The RBS arg is named in the signature, so we use the explicit name used
            auto nameStr = parser.resolveConstant(nameSymbol);
            auto name = ctx.state.enterNameUTF8(nameStr);
            sigParams.emplace_back(
                make_unique<parser::Pair>(arg.loc, parser::MK::Symbol(arg.nameLoc, name), move(type)));
        } else {
            // The RBS arg is not named in the signature, so we get it from the method definition
            auto name = nodeName(methodParam);
            sigParams.emplace_back(
                make_unique<parser::Pair>(arg.loc, parser::MK::Symbol(methodParam->loc, name), move(type)));
        }
    }

    // Build the sig

    auto sigBuilder = parser::MK::Self(fullTypeLoc);
    sigBuilder = handleAnnotations(ctx, methodDef, move(sigBuilder), annotations);

    if (typeParams.size() > 0) {
        auto typeParamsVector = parser::NodeVec();
        typeParamsVector.reserve(typeParams.size());

        for (auto &param : typeParams) {
            typeParamsVector.emplace_back(parser::MK::Symbol(param.first, param.second));
        }
        sigBuilder = parser::MK::Send(fullTypeLoc, move(sigBuilder), core::Names::typeParameters(), fullTypeLoc,
                                      move(typeParamsVector));
    }

    if (sigParams.size() > 0) {
        auto hash = parser::MK::Hash(fullTypeLoc, true, move(sigParams));
        auto args = parser::NodeVec();
        args.emplace_back(move(hash));
        sigBuilder = parser::MK::Send(fullTypeLoc, move(sigBuilder), core::Names::params(), fullTypeLoc, move(args));
    }

    rbs_node_t *returnValue = functionType->return_type;
    if (returnValue->type == RBS_TYPES_BASES_VOID) {
        auto loc = declaration.typeLocFromRange(returnValue->location->rg);
        sigBuilder = parser::MK::Send0(fullTypeLoc, move(sigBuilder), core::Names::void_(), loc);
    } else {
        auto nameLoc = declaration.typeLocFromRange(returnValue->location->rg);
        auto returnType = typeToParserNode.toParserNode(returnValue, declaration);
        sigBuilder =
            parser::MK::Send1(fullTypeLoc, move(sigBuilder), core::Names::returns(), nameLoc, move(returnType));
    }

    auto sigArgs = parser::NodeVec();

    auto final = absl::c_find_if(annotations, [](const Comment &annotation) { return annotation.string == "final"; });
    if (final != annotations.end()) {
        sigArgs.emplace_back(parser::MK::Symbol(final->typeLoc, core::Names::final_()));
    }

    auto sig = parser::MK::Send(fullTypeLoc, parser::MK::T_Sig_WithoutRuntime(firstLineTypeLoc), core::Names::sig(),
                                firstLineTypeLoc, move(sigArgs));

    return make_unique<parser::Block>(commentLoc, move(sig), nullptr, move(sigBuilder));
}

unique_ptr<parser::Node> MethodTypeToParserNode::attrSignature(const parser::Send *send, const rbs_node_t *type,
                                                               const RBSDeclaration &declaration,
                                                               const vector<Comment> &annotations) {
    auto typeParams = vector<pair<core::LocOffsets, core::NameRef>>();

    auto fullTypeLoc = declaration.fullTypeLoc();
    auto firstLineTypeLoc = declaration.firstLineTypeLoc();
    auto commentLoc = declaration.commentLoc();

    auto sigBuilder = parser::MK::Self(fullTypeLoc.copyWithZeroLength());
    sigBuilder = handleAnnotations(ctx, send, move(sigBuilder), annotations);

    if (send->args.size() == 0) {
        if (auto e = ctx.beginIndexerError(send->loc, core::errors::Rewriter::RBSUnsupported)) {
            e.setHeader("RBS signatures do not support accessor without arguments");
        }

        return nullptr;
    }

    auto typeTranslator = TypeToParserNode(ctx, typeParams, parser);
    auto returnType = typeTranslator.toParserNode(type, declaration);

    if (send->method == core::Names::attrWriter()) {
        if (send->args.size() > 1) {
            if (auto e = ctx.beginIndexerError(send->loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("RBS signatures for attr_writer do not support multiple arguments");
            }

            return nullptr;
        }

        // For attr writer, we need to add the param to the sig
        auto argName = nodeName(send->args[0].get());

        // The origin location points to the `:name` symbol, so we need to adjust it to point to the actual name
        auto argLoc = core::LocOffsets{
            send->args[0]->loc.beginPos() + 1,
            send->args[0]->loc.endPos(),
        };

        auto pairs = parser::NodeVec();
        pairs.emplace_back(
            make_unique<parser::Pair>(argLoc, parser::MK::Symbol(argLoc, argName), returnType->deepCopy()));
        auto hash = parser::MK::Hash(send->loc, true, move(pairs));
        auto sigArgs = parser::NodeVec();
        sigArgs.emplace_back(move(hash));
        sigBuilder = parser::MK::Send(send->loc, move(sigBuilder), core::Names::params(), send->loc, move(sigArgs));
    }

    sigBuilder =
        parser::MK::Send1(fullTypeLoc, move(sigBuilder), core::Names::returns(), returnType->loc, move(returnType));

    auto sigArgs = parser::NodeVec();

    auto final = absl::c_find_if(annotations, [](const Comment &annotation) { return annotation.string == "final"; });
    if (final != annotations.end()) {
        sigArgs.emplace_back(parser::MK::Symbol(final->typeLoc, core::Names::final_()));
    }

    auto sig = parser::MK::Send(fullTypeLoc, parser::MK::T_Sig_WithoutRuntime(firstLineTypeLoc), core::Names::sig(),
                                firstLineTypeLoc, move(sigArgs));

    return make_unique<parser::Block>(commentLoc, move(sig), nullptr, move(sigBuilder));
}

} // namespace sorbet::rbs
