#include "rbs/prism/MethodTypeToParserNodePrism.h"

#include "absl/algorithm/container.h"
#include "core/errors/rewriter.h"
#include "parser/prism/Helpers.h"
#include "rbs/prism/TypeToParserNodePrism.h"
#include "rbs/rbs_method_common.h"

extern "C" {
#include "prism/util/pm_constant_pool.h"
}

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {

bool isSelfOrKernel(pm_node_t *node, const parser::Prism::Parser *prismParser) {
    if (PM_NODE_TYPE_P(node, PM_SELF_NODE)) {
        return true;
    }

    if (PM_NODE_TYPE_P(node, PM_CONSTANT_READ_NODE)) {
        auto *constant = down_cast<pm_constant_read_node_t>(node);
        auto name = prismParser->resolveConstant(constant->name);
        // Check if it's Kernel constant with no scope (::Kernel or bare Kernel)
        return name == "Kernel";
    }

    if (PM_NODE_TYPE_P(node, PM_CONSTANT_PATH_NODE)) {
        auto *constantPath = down_cast<pm_constant_path_node_t>(node);
        // Check if it's ::Kernel (parent is nullptr, representing root ::)
        // We reject Foo::Kernel or any other scoped constant
        if (constantPath->parent == nullptr) {
            auto name = prismParser->resolveConstant(constantPath->name);
            return name == "Kernel";
        }
    }

    return false;
}

core::AutocorrectSuggestion autocorrectAbstractBody(core::MutableContext ctx, pm_node_t *method,
                                                    const parser::Prism::Parser *prismParser, pm_node_t *method_body) {
    core::LocOffsets editLoc;
    string corrected;

    auto *def = down_cast<pm_def_node_t>(method);
    auto methodLoc = prismParser->translateLocation(method->location);
    auto nameLoc = prismParser->translateLocation(def->name_loc);

    auto lineStart = core::Loc::pos2Detail(ctx.file.data(ctx), nameLoc.endPos()).line;
    auto lineEnd = core::Loc::pos2Detail(ctx.file.data(ctx), methodLoc.endPos()).line;

    if (method_body) {
        editLoc = prismParser->translateLocation(method_body->location);
        corrected = "raise \"Abstract method called\"";
    } else if (lineStart == lineEnd) {
        editLoc = nameLoc.copyEndWithZeroLength().join(methodLoc.copyEndWithZeroLength());
        corrected = " = raise(\"Abstract method called\")";
    } else {
        editLoc = nameLoc.copyEndWithZeroLength();
        auto [_endLoc, indentLength] = ctx.locAt(methodLoc).findStartOfIndentation(ctx);
        string indent(indentLength + 2, ' ');
        corrected = "\n" + indent + "raise \"Abstract method called\"";
    }

    return core::AutocorrectSuggestion{fmt::format("Add `{}` to the method body", "raise"),
                                       {core::AutocorrectSuggestion::Edit{ctx.locAt(editLoc), corrected}}};
}

// Returns true if the node is a valid abstract method (def node with a body that only raises)
// e.g. def abstract_method = raise
bool isValidAbstractMethod(pm_node_t *node, const parser::Prism::Parser *prismParser) {
    if (!PM_NODE_TYPE_P(node, PM_DEF_NODE)) {
        return false;
    }

    auto *def = down_cast<pm_def_node_t>(node);

    if (def->body == nullptr) {
        return false;
    }

    pm_node_t *bodyNode = def->body;

    // Unwrap statements node if it contains exactly one statement
    if (PM_NODE_TYPE_P(bodyNode, PM_STATEMENTS_NODE)) {
        auto *stmts = down_cast<pm_statements_node_t>(bodyNode);
        if (stmts->body.size != 1) {
            return false; // Multiple statements, not just a raise
        }
        bodyNode = stmts->body.nodes[0];
    }

    if (!PM_NODE_TYPE_P(bodyNode, PM_CALL_NODE)) {
        return false;
    }

    auto *call = down_cast<pm_call_node_t>(bodyNode);
    auto methodName = prismParser->resolveConstant(call->name);

    // Check if it's a raise call with no receiver or self/Kernel receiver
    return methodName == "raise" && (call->receiver == nullptr || isSelfOrKernel(call->receiver, prismParser));
}

void ensureAbstractMethodRaises(core::MutableContext ctx, pm_node_t *node, parser::Prism::Parser *prismParser) {
    if (isValidAbstractMethod(node, prismParser)) {
        // Method properly raises, remove body to avoid error 5019 later in the pipeline
        auto *def = down_cast<pm_def_node_t>(node);
        prismParser->destroyNode(def->body);
        def->body = nullptr;
        return;
    }

    auto *def = down_cast<pm_def_node_t>(node);
    auto nodeLoc = prismParser->translateLocation(node->location);

    if (auto e = ctx.beginIndexerError(nodeLoc, core::errors::Rewriter::RBSAbstractMethodNoRaises)) {
        e.setHeader("Methods declared @abstract with an RBS comment must always raise");
        auto autocorrect = autocorrectAbstractBody(ctx, node, prismParser, def->body);
        e.addAutocorrect(move(autocorrect));
    }
}

pm_node_t *handleAnnotations(core::MutableContext ctx, pm_node_t *node, pm_node_t *sigBuilder,
                             absl::Span<const Comment> annotations, parser::Prism::Parser *prismParser,
                             const parser::Prism::Factory &prism) {
    static constexpr string_view OVERRIDE_ALLOW_INCOMPATIBLE_PREFIX = "override(allow_incompatible: ";

    for (auto &annotation : annotations) {
        if (annotation.string == "final") {
            // no-op, `final` is handled in the `sig()` call later
        } else if (annotation.string == "abstract") {
            sigBuilder = prism.Call0(annotation.typeLoc, sigBuilder, core::Names::abstract().show(ctx.state));
            ensureAbstractMethodRaises(ctx, node, prismParser);
        } else if (annotation.string == "overridable") {
            sigBuilder = prism.Call0(annotation.typeLoc, sigBuilder, core::Names::overridable().show(ctx.state));
        } else if (annotation.string == "override") {
            sigBuilder = prism.Call0(annotation.typeLoc, sigBuilder, core::Names::override_().show(ctx.state));
        } else if (annotation.string.find(OVERRIDE_ALLOW_INCOMPATIBLE_PREFIX) == 0 && annotation.string.back() == ')') {
            auto valueStr =
                annotation.string.substr(OVERRIDE_ALLOW_INCOMPATIBLE_PREFIX.size(),
                                         annotation.string.size() - OVERRIDE_ALLOW_INCOMPATIBLE_PREFIX.size() - 1);

            pm_node_t *value = nullptr;
            if (valueStr == "true") {
                value = prism.True(annotation.typeLoc);
            } else if (valueStr == "false") {
                value = prism.False(annotation.typeLoc);
            } else if (valueStr == ":visibility") {
                value = prism.Symbol(annotation.typeLoc, core::Names::visibility().show(ctx.state));
            }

            if (value) {
                auto key = prism.Symbol(annotation.typeLoc, core::Names::allowIncompatible().show(ctx.state));
                auto pair = prism.AssocNode(annotation.typeLoc, key, value);
                auto hashElements = array{pair};
                auto hash = prism.KeywordHash(annotation.typeLoc, absl::MakeSpan(hashElements));
                sigBuilder =
                    prism.Call1(annotation.typeLoc, sigBuilder, core::Names::override_().show(ctx.state), hash);
            }
        }
    }
    return sigBuilder;
}

// Convert the PM_NODE_TYPE to a string to be used in error messages
string_view nodeKindToString(const pm_node_t *node) {
    switch (PM_NODE_TYPE(node)) {
        case PM_REQUIRED_PARAMETER_NODE:
            return "positional"sv;
        case PM_OPTIONAL_PARAMETER_NODE:
            return "optional positional"sv;
        case PM_REST_PARAMETER_NODE:
            return "rest positional"sv;
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE:
            return "keyword"sv;
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE:
            return "optional keyword"sv;
        case PM_KEYWORD_REST_PARAMETER_NODE:
            return "rest keyword"sv;
        case PM_BLOCK_PARAMETER_NODE:
            return "block"sv;
        default:
            return "unknown"sv;
    }
}

// Get the NameRef for an anonymous parameter node
core::NameRef anonymousParamName(core::MutableContext ctx, const pm_node_t *node) {
    switch (PM_NODE_TYPE(node)) {
        case PM_REST_PARAMETER_NODE:
            return core::Names::star();
        case PM_KEYWORD_REST_PARAMETER_NODE:
            return core::Names::starStar();
        case PM_BLOCK_PARAMETER_NODE:
            return core::Names::ampersand();
        default:
            unreachable("Unexpected anonymous parameter node type");
    }
}

optional<core::AutocorrectSuggestion> autocorrectArg(core::MutableContext ctx, pm_node_t *methodArg, RBSArg arg,
                                                     const parser::Prism::Parser &prismParser,
                                                     const RBSDeclaration &declaration) {
    if (arg.kind == RBSArg::Kind::Block || PM_NODE_TYPE_P(methodArg, PM_BLOCK_PARAMETER_NODE)) {
        // Block arguments are not autocorrected
        return nullopt;
    }

    string corrected;
    auto source = ctx.file.data(ctx.state).source();

    // Note: Whitequark's autocorrectArg takes unique_ptr<parser::Node> type and extracts from type->loc.
    // In Prism, we cannot pass the converted typeNode because toPrismNode() creates synthesized nodes
    // with new locations for the signature, not nodes that preserve the original RBS source locations.
    // Instead, we extract directly from the RBS source using declaration.typeLocFromRange(arg.type->location->rg).
    auto typeLoc = declaration.typeLocFromRange(arg.type->location);
    string typeString = string(source.substr(typeLoc.beginPos(), typeLoc.endPos() - typeLoc.beginPos()));

    switch (PM_NODE_TYPE(methodArg)) {
        // Should be: `Type name`
        case PM_REQUIRED_PARAMETER_NODE: {
            auto *param = down_cast<pm_required_parameter_node_t>(methodArg);
            if (arg.name) {
                auto nameStr = prismParser.resolveConstant(param->name);
                corrected = fmt::format("{} {}", typeString, nameStr);
            } else {
                corrected = fmt::format("{}", typeString);
            }
            break;
        }
        // Should be: `?Type name`
        case PM_OPTIONAL_PARAMETER_NODE: {
            auto *param = down_cast<pm_optional_parameter_node_t>(methodArg);
            if (arg.name) {
                auto nameStr = prismParser.resolveConstant(param->name);
                corrected = fmt::format("?{} {}", typeString, nameStr);
            } else {
                corrected = fmt::format("?{}", typeString);
            }
            break;
        }
        // Should be: `*Type name`
        case PM_REST_PARAMETER_NODE: {
            auto *param = down_cast<pm_rest_parameter_node_t>(methodArg);
            if (arg.name) {
                auto nameStr = prismParser.resolveConstant(param->name);
                corrected = fmt::format("*{} {}", typeString, nameStr);
            } else {
                corrected = fmt::format("*{}", typeString);
            }
            break;
        }
        // Should be: `name: Type`
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: {
            auto *param = down_cast<pm_required_keyword_parameter_node_t>(methodArg);
            auto nameStr = prismParser.resolveConstant(param->name);
            corrected = fmt::format("{}: {}", nameStr, typeString);
            break;
        }
        // Should be: `?name: Type`
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: {
            auto *param = down_cast<pm_optional_keyword_parameter_node_t>(methodArg);
            auto nameStr = prismParser.resolveConstant(param->name);
            corrected = fmt::format("?{}: {}", nameStr, typeString);
            break;
        }
        // Should be: `**Type name`
        case PM_KEYWORD_REST_PARAMETER_NODE: {
            auto *param = down_cast<pm_keyword_rest_parameter_node_t>(methodArg);
            if (arg.name) {
                auto nameStr = prismParser.resolveConstant(param->name);
                corrected = fmt::format("**{} {}", typeString, nameStr);
            } else {
                corrected = fmt::format("**{}", typeString);
            }
            break;
        }
        default:
            return nullopt;
    }

    core::LocOffsets loc = arg.loc;

    // Adjust the location to include RBS prefix symbols in the replacement range.
    // arg.loc points to the type itself, but corrected string includes prefixes:
    // -1 for single-char prefixes: `?` (optional), `*` (rest)
    // -2 for double-char prefix: `**` (keyword rest)
    if (arg.kind == RBSArg::Kind::OptionalPositional || arg.kind == RBSArg::Kind::RestPositional ||
        arg.kind == RBSArg::Kind::OptionalKeyword) {
        loc.beginLoc -= 1;
    } else if (arg.kind == RBSArg::Kind::RestKeyword) {
        loc.beginLoc -= 2;
    }

    return core::AutocorrectSuggestion{fmt::format("Replace with `{}`", argKindToString(arg.kind)),
                                       {core::AutocorrectSuggestion::Edit{ctx.locAt(loc), corrected}}};
}

bool checkParameterKindMatch(const RBSArg &arg, const pm_node_t *methodArg) {
    switch (PM_NODE_TYPE(methodArg)) {
        case PM_REQUIRED_PARAMETER_NODE:
            return arg.kind == RBSArg::Kind::Positional;
        case PM_OPTIONAL_PARAMETER_NODE:
            return arg.kind == RBSArg::Kind::OptionalPositional;
        case PM_REST_PARAMETER_NODE:
            return arg.kind == RBSArg::Kind::RestPositional;
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE:
            return arg.kind == RBSArg::Kind::Keyword;
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE:
            return arg.kind == RBSArg::Kind::OptionalKeyword;
        case PM_KEYWORD_REST_PARAMETER_NODE:
            return arg.kind == RBSArg::Kind::RestKeyword;
        case PM_BLOCK_PARAMETER_NODE:
            return arg.kind == RBSArg::Kind::Block;
        default:
            return false;
    }
}

void collectPositionalParams(const RBSDeclaration &declaration, rbs_node_list_t *field, vector<RBSArg> &args,
                             RBSArg::Kind kind) {
    ENFORCE(kind == RBSArg::Kind::Positional || kind == RBSArg::Kind::OptionalPositional ||
            kind == RBSArg::Kind::RestPositional);

    if (field == nullptr) {
        return;
    }

    for (auto *list_node = field->head; list_node != nullptr; list_node = list_node->next) {
        ENFORCE(list_node->node->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in function parameter, expected `{}`", rbs_node_type_name(list_node->node),
                "FunctionParam");

        auto *param = rbs_down_cast<rbs_types_function_param_t>(list_node->node);

        auto loc = declaration.typeLocFromRange(list_node->node->location);
        auto nameLoc = RBS_LOCATION_NULL_RANGE_P(param->name_range) ? loc : declaration.typeLocFromRange(param->name_range);
        auto arg = RBSArg{.loc = loc, .nameLoc = nameLoc, .name = param->name, .type = param->type, .kind = kind};
        args.emplace_back(arg);
    }
}

void collectKeywordParams(const RBSDeclaration &declaration, rbs_hash_t *field, vector<RBSArg> &params,
                          RBSArg::Kind kind) {
    ENFORCE(kind == RBSArg::Kind::Keyword || kind == RBSArg::Kind::OptionalKeyword);

    if (field == nullptr) {
        return;
    }

    for (auto *hash_node = field->head; hash_node != nullptr; hash_node = hash_node->next) {
        ENFORCE(hash_node->key->type == RBS_AST_SYMBOL,
                "Unexpected node type `{}` in keyword argument name, expected `{}`", rbs_node_type_name(hash_node->key),
                "Symbol");

        ENFORCE(hash_node->value->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in keyword argument value, expected `{}`",
                rbs_node_type_name(hash_node->value), "FunctionParam");

        auto nameLoc = declaration.typeLocFromRange(hash_node->key->location);
        auto loc = nameLoc.join(declaration.typeLocFromRange(hash_node->value->location));
        auto *keyNode = rbs_down_cast<rbs_ast_symbol_t>(hash_node->key);
        auto *valueNode = rbs_down_cast<rbs_types_function_param_t>(hash_node->value);
        params.emplace_back(
            RBSArg{.loc = loc, .nameLoc = nameLoc, .name = keyNode, .type = valueNode->type, .kind = kind});
    }
}

void collectRestParam(const RBSDeclaration &declaration, rbs_node_t *restParam, vector<RBSArg> &args,
                      RBSArg::Kind kind) {
    ENFORCE(kind == RBSArg::Kind::RestPositional || kind == RBSArg::Kind::RestKeyword);
    ENFORCE(restParam->type == RBS_TYPES_FUNCTION_PARAM, "Unexpected node type `{}` in rest argument, expected `{}`",
            rbs_node_type_name(restParam), "FunctionParam");

    auto *param = rbs_down_cast<rbs_types_function_param_t>(restParam);
    
    auto loc = declaration.typeLocFromRange(restParam->location);
    auto nameLoc = RBS_LOCATION_NULL_RANGE_P(param->name_range) ? loc : declaration.typeLocFromRange(param->name_range);
    args.emplace_back(RBSArg{.loc = loc, .nameLoc = nameLoc, .name = param->name, .type = param->type, .kind = kind});
}

// Extract parameter name from a Prism parameter node
pm_constant_id_t getParamName(pm_node_t *paramNode) {
    if (paramNode == nullptr) {
        return PM_CONSTANT_ID_UNSET;
    }

    switch (PM_NODE_TYPE(paramNode)) {
        case PM_REQUIRED_PARAMETER_NODE:
            return down_cast<pm_required_parameter_node_t>(paramNode)->name;
        case PM_OPTIONAL_PARAMETER_NODE:
            return down_cast<pm_optional_parameter_node_t>(paramNode)->name;
        case PM_REST_PARAMETER_NODE:
            return down_cast<pm_rest_parameter_node_t>(paramNode)->name;
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE:
            return down_cast<pm_required_keyword_parameter_node_t>(paramNode)->name;
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE:
            return down_cast<pm_optional_keyword_parameter_node_t>(paramNode)->name;
        case PM_KEYWORD_REST_PARAMETER_NODE:
            return down_cast<pm_keyword_rest_parameter_node_t>(paramNode)->name;
        case PM_BLOCK_PARAMETER_NODE:
            return down_cast<pm_block_parameter_node_t>(paramNode)->name;
        default:
            return PM_CONSTANT_ID_UNSET;
    }
}

// Collects method parameter nodes (in positional/keyword/block order) from a Prism def node
vector<pm_node_t *> getMethodParams(pm_def_node_t *def) {
    vector<pm_node_t *> result;
    if (!def || def->parameters == nullptr) {
        return result;
    }

    auto *params = def->parameters;

    auto restSize = params->rest == nullptr ? 0 : 1;
    auto kwrestSize = params->keyword_rest == nullptr ? 0 : 1;
    auto blockSize = params->block == nullptr ? 0 : 1;

    result.reserve(params->requireds.size + params->optionals.size + restSize + params->posts.size +
                   params->keywords.size + kwrestSize + blockSize);

    for (size_t i = 0; i < params->requireds.size; i++) {
        result.push_back(params->requireds.nodes[i]);
    }

    for (size_t i = 0; i < params->optionals.size; i++) {
        result.push_back(params->optionals.nodes[i]);
    }

    if (params->rest) {
        result.push_back(params->rest);
    }

    for (size_t i = 0; i < params->posts.size; i++) {
        result.push_back(params->posts.nodes[i]);
    }
    for (size_t i = 0; i < params->keywords.size; i++) {
        result.push_back(params->keywords.nodes[i]);
    }

    if (params->keyword_rest) {
        result.push_back(params->keyword_rest);
    }

    if (params->block) {
        result.push_back(up_cast(params->block));
    }

    return result;
}

} // namespace

pm_node_t *MethodTypeToParserNodePrism::attrSignature(pm_call_node_t *call, const rbs_node_t *type,
                                                      const RBSDeclaration &declaration,
                                                      absl::Span<const Comment> annotations) {
    auto typeParams = vector<pair<core::LocOffsets, core::NameRef>>{};
    auto fullTypeLoc = declaration.fullTypeLoc();
    auto firstLineTypeLoc = declaration.firstLineTypeLoc();
    auto commentLoc = declaration.commentLoc();
    auto callLoc = prismParser.translateLocation(call->base.location);

    pm_node_t *sigBuilder = prism.Self(fullTypeLoc.copyWithZeroLength());
    sigBuilder = handleAnnotations(ctx, &call->base, sigBuilder, annotations, &prismParser, prism);

    if (call->arguments == nullptr || call->arguments->arguments.size == 0) {
        if (auto e = ctx.beginIndexerError(callLoc, core::errors::Rewriter::RBSUnsupported)) {
            e.setHeader("RBS signatures do not support accessor without arguments");
        }
        return nullptr;
    }

    auto typeTranslator = TypeToParserNodePrism(ctx, typeParams, parser, prismParser);

    auto methodName = prismParser.resolveConstant(call->name);

    if (methodName == "attr_writer") {
        if (call->arguments->arguments.size > 1) {
            if (auto e = ctx.beginIndexerError(callLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("attr_writer can only have a single parameter");
            }
            return nullptr;
        }

        // For attr_writer, we need to add the param to the sig
        pm_node_t *arg = call->arguments->arguments.nodes[0];
        if (!PM_NODE_TYPE_P(arg, PM_SYMBOL_NODE)) {
            return nullptr;
        }
        auto *symbolNode = down_cast<pm_symbol_node_t>(arg);
        auto argName = prismParser.extractString(&symbolNode->unescaped);

        // The location points to the `:name` symbol, adjust to point to actual name
        auto argLoc = prismParser.translateLocation(arg->location);
        argLoc = core::LocOffsets{argLoc.beginPos() + 1, argLoc.endPos()};

        pm_node_t *keyNode = prism.Symbol(argLoc, argName);
        pm_node_t *paramType = typeTranslator.toPrismNode(type, declaration);
        pm_node_t *assoc = prism.AssocNode(callLoc, keyNode, paramType);

        auto hashElements = array{assoc};
        pm_node_t *hash = prism.KeywordHash(callLoc, absl::MakeSpan(hashElements));
        sigBuilder = prism.Call1(callLoc, sigBuilder, "params"sv, hash);
    }

    pm_node_t *returnType = typeTranslator.toPrismNode(type, declaration);
    sigBuilder = prism.Call1(fullTypeLoc, sigBuilder, "returns"sv, returnType);

    vector<pm_node_t *> sigArgs;
    auto finalAnnotation =
        absl::c_find_if(annotations, [](const Comment &annotation) { return annotation.string == "final"; });
    if (finalAnnotation != annotations.end()) {
        sigArgs.push_back(prism.Symbol(finalAnnotation->typeLoc, "final"sv));
    }

    pm_node_t *sigReceiver = prism.TSigWithoutRuntime(firstLineTypeLoc);
    pm_node_t *block = prism.Block(commentLoc, sigBuilder);
    return prism.Call(fullTypeLoc, sigReceiver, "sig"sv, absl::MakeSpan(sigArgs), block);
}

pm_node_t *MethodTypeToParserNodePrism::methodSignature(pm_node_t *methodDef, const rbs_method_type_t *methodType,
                                                        const RBSDeclaration &declaration,
                                                        absl::Span<const Comment> annotations) {
    const auto &node = *methodType;

    // Collect type parameters
    vector<pair<core::LocOffsets, core::NameRef>> typeParams;
    for (auto *list_node = node.type_params->head; list_node != nullptr; list_node = list_node->next) {
        auto loc = declaration.typeLocFromRange(list_node->node->location);

        ENFORCE(list_node->node->type == RBS_AST_TYPE_PARAM,
                "Unexpected node type `{}` in type parameter list, expected `{}`", rbs_node_type_name(list_node->node),
                "TypeParam");

        auto typeParamNode = rbs_down_cast<rbs_ast_type_param_t>(list_node->node);
        auto str = parser.resolveConstant(typeParamNode->name);
        typeParams.emplace_back(loc, ctx.state.enterNameUTF8(str));
    }

    // Validate that we have a function type
    if (node.type->type != RBS_TYPES_FUNCTION) {
        auto errLoc = declaration.typeLocFromRange(node.type->location);
        if (auto e = ctx.beginIndexerError(errLoc, core::errors::Rewriter::RBSUnsupported)) {
            e.setHeader("Unexpected node type `{}` in method signature, expected `{}`", rbs_node_type_name(node.type),
                        "Function");
        }
        return nullptr;
    }

    auto *functionType = rbs_down_cast<rbs_types_function_t>(node.type);

    // Collect RBS parameters for sig params
    vector<RBSArg> args;

    auto restPositionalsSize = functionType->rest_positionals ? 1 : 0;
    auto restKeywordsSize = functionType->rest_keywords ? 1 : 0;
    auto blockSize = node.block ? 1 : 0;

    size_t requiredPositionalsSize =
        functionType->required_positionals ? functionType->required_positionals->length : 0;
    size_t optionalPositionalsSize =
        functionType->optional_positionals ? functionType->optional_positionals->length : 0;
    size_t trailingPositionalsSize =
        functionType->trailing_positionals ? functionType->trailing_positionals->length : 0;
    size_t requiredKeywordsSize = functionType->required_keywords ? functionType->required_keywords->length : 0;
    size_t optionalKeywordsSize = functionType->optional_keywords ? functionType->optional_keywords->length : 0;

    args.reserve(requiredPositionalsSize + optionalPositionalsSize + restPositionalsSize + trailingPositionalsSize +
                 requiredKeywordsSize + optionalKeywordsSize + restKeywordsSize + blockSize);

    collectPositionalParams(declaration, functionType->required_positionals, args, RBSArg::Kind::Positional);

    collectPositionalParams(declaration, functionType->optional_positionals, args, RBSArg::Kind::OptionalPositional);
    if (functionType->rest_positionals) {
        collectRestParam(declaration, functionType->rest_positionals, args, RBSArg::Kind::RestPositional);
    }
    collectPositionalParams(declaration, functionType->trailing_positionals, args, RBSArg::Kind::Positional);
    collectKeywordParams(declaration, functionType->required_keywords, args, RBSArg::Kind::Keyword);
    collectKeywordParams(declaration, functionType->optional_keywords, args, RBSArg::Kind::OptionalKeyword);
    if (functionType->rest_keywords) {
        collectRestParam(declaration, functionType->rest_keywords, args, RBSArg::Kind::RestKeyword);
    }
    if (auto *rbsBlock = node.block) {
        auto loc = declaration.typeLocFromRange(rbsBlock->base.location);
        auto arg = RBSArg{
            .loc = loc, .nameLoc = loc, .name = nullptr, .type = (rbs_node_t *)rbsBlock, .kind = RBSArg::Kind::Block};
        args.emplace_back(arg);
    }

    auto fullTypeLoc = declaration.fullTypeLoc();
    auto firstLineTypeLoc = declaration.firstLineTypeLoc();
    auto commentLoc = declaration.commentLoc();

    pm_node_t *receiver = prism.TSigWithoutRuntime(firstLineTypeLoc);

    vector<pm_node_t *> methodParams;
    if (PM_NODE_TYPE_P(methodDef, PM_DEF_NODE)) {
        auto def = down_cast<pm_def_node_t>(methodDef);
        methodParams = getMethodParams(def);
    }

    auto typeToPrismNode = TypeToParserNodePrism(ctx, typeParams, parser, prismParser);

    // Only error if RBS has more parameters than the method.
    // If RBS has fewer, generate a partial sig and let the resolver error on missing types.
    if (args.size() > methodParams.size()) {
        if (auto e = ctx.beginIndexerError(fullTypeLoc, core::errors::Rewriter::RBSParameterMismatch)) {
            e.setHeader("RBS signature has more parameters than in the method definition");
        }
        return nullptr;
    }

    vector<pm_node_t *> sigParams;
    sigParams.reserve(args.size());

    for (size_t i = 0; i < args.size(); i++) {
        const auto &arg = args[i];
        pm_node_t *typeNode = typeToPrismNode.toPrismNode(arg.type, declaration);

        pm_node_t *methodParam = methodParams[i];
        if (!checkParameterKindMatch(arg, methodParam)) {
            if (auto e = ctx.beginIndexerError(arg.loc, core::errors::Rewriter::RBSIncorrectParameterKind)) {
                auto paramNameId = getParamName(methodParams[i]);
                string_view methodParamNameStr;
                if (paramNameId != PM_CONSTANT_ID_UNSET) {
                    methodParamNameStr = prismParser.resolveConstant(paramNameId);
                } else {
                    methodParamNameStr = anonymousParamName(ctx, methodParam).shortName(ctx.state);
                }
                e.setHeader("Argument kind mismatch for `{}`, method declares `{}`, but RBS signature declares `{}`",
                            methodParamNameStr, nodeKindToString(methodParam), argKindToString(arg.kind));

                e.maybeAddAutocorrect(autocorrectArg(ctx, methodParam, arg, prismParser, declaration));
            }
        }

        pm_node_t *symbolNode = nullptr;
        if (arg.name) {
            symbolNode = createSymbolNode(arg.name, arg.nameLoc);
        } else {
            // RBS sig didn't give a name, fallback to the name from method def
            core::LocOffsets tinyLocOffsets = firstLineTypeLoc.copyWithZeroLength();
            auto *methodParam = methodParams[i];
            auto methodParamName = getParamName(methodParam);

            if (methodParamName != PM_CONSTANT_ID_UNSET) {
                symbolNode = prism.SymbolFromConstant(tinyLocOffsets, methodParamName);
            } else {
                // Anonymous parameter - use the canonical name (*, **, &)
                auto name = anonymousParamName(ctx, methodParam);
                symbolNode = prism.Symbol(tinyLocOffsets, name.shortName(ctx.state));
            }
        }

        sigParams.push_back(prism.AssocNode(firstLineTypeLoc.copyWithZeroLength(), symbolNode, typeNode));
    }

    pm_node_t *sigReceiver = prism.Self(fullTypeLoc);
    sigReceiver = handleAnnotations(ctx, methodDef, sigReceiver, annotations, &prismParser, prism);

    if (!typeParams.empty()) {
        vector<pm_node_t *> typeParamSymbols;
        typeParamSymbols.reserve(typeParams.size());

        for (auto &param : typeParams) {
            string nameStr = param.second.show(ctx.state);
            pm_node_t *symbolNode = prism.Symbol(param.first, nameStr);
            typeParamSymbols.push_back(symbolNode);
        }

        pm_node_t *typeParamsCall =
            prism.Call(fullTypeLoc, sigReceiver, "type_parameters"sv, absl::MakeSpan(typeParamSymbols));
        ENFORCE(typeParamsCall, "Failed to create type parameters call");

        sigReceiver = typeParamsCall;
    }

    if (!sigParams.empty()) {
        sigReceiver = prism.Call1(fullTypeLoc, sigReceiver, "params"sv,
                                  prism.KeywordHash(fullTypeLoc, absl::MakeSpan(sigParams)));
    }

    pm_node_t *blockBody = nullptr;

    if (functionType->return_type->type == RBS_TYPES_BASES_VOID) {
        blockBody =
            prism.Call0(declaration.typeLocFromRange(functionType->return_type->location), sigReceiver, "void"sv);
    } else {
        auto returnTypeLoc = declaration.typeLocFromRange(functionType->return_type->location);
        pm_node_t *returnTypeNode = typeToPrismNode.toPrismNode(functionType->return_type, declaration);
        ENFORCE(returnTypeNode, "Failed to create return type node");
        returnTypeNode->location = prismParser.convertLocOffsets(returnTypeLoc);
        blockBody = prism.Call1(returnTypeLoc, sigReceiver, "returns"sv, returnTypeNode);
    }

    pm_node_t *block = prism.Block(commentLoc, blockBody);

    vector<pm_node_t *> sigArgs;
    auto finalAnnotation =
        absl::c_find_if(annotations, [](const Comment &annotation) { return annotation.string == "final"; });
    if (finalAnnotation != annotations.end()) {
        sigArgs.push_back(prism.Symbol(finalAnnotation->typeLoc, core::Names::final_().show(ctx.state)));
    }

    return prism.Call(fullTypeLoc, receiver, "sig"sv, absl::MakeSpan(sigArgs), block);
}

pm_node_t *MethodTypeToParserNodePrism::createSymbolNode(rbs_ast_symbol_t *name, core::LocOffsets nameLoc) {
    if (!name) {
        return nullptr;
    }
    return prism.Symbol(nameLoc, parser.resolveConstant(name));
}

} // namespace sorbet::rbs
