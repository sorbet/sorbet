#include "rbs/prism/MethodTypeToParserNodePrism.h"

#include "absl/algorithm/container.h"
#include "absl/strings/escaping.h"
#include "common/typecase.h"
#include "core/GlobalState.h"
#include "core/errors/internal.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "parser/prism/Helpers.h"
#include "rbs/TypeToParserNode.h"
#include "rbs/prism/TypeToParserNodePrism.h"
#include "rbs/rbs_method_common.h"
#include "rewriter/util/Util.h"
#include <cstring>

extern "C" {
#include "prism/util/pm_constant_pool.h"
}

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {

// Forward declarations
// core::LocOffsets translateLocation(pm_location_t location);

/* TODO: Implement when needed
core::LocOffsets adjustNameLoc(const RBSDeclaration &declaration, rbs_node_t *node) {
    auto range = node->location->rg;

    auto nameRange = node->location->children->entries[0].rg;
    if (nameRange.start != -1 && nameRange.end != -1) {
        range.start.char_pos = nameRange.start;
        range.end.char_pos = nameRange.end;
    }

    return declaration.typeLocFromRange(range);
}
*/

/* TODO: Implement when needed
bool isSelfOrKernel(pm_node_t *node) {
    if (PM_NODE_TYPE_P(node, PM_SELF_NODE)) {
        return true;
    }

    if (PM_NODE_TYPE_P(node, PM_CONSTANT_READ_NODE)) {
        auto *constant = down_cast<pm_constant_read_node_t>(node);
        // TODO: Check if the constant name is "Kernel" and has no scope
        // For now, simplified check
        (void)constant; // Suppress unused warning
        return false;
    }

    return false;
}
*/

// bool isRaise(pm_node_t *node) {
//     if (!PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
//         return false;
//     }

//     auto *call = down_cast<pm_call_node_t>(node);
//     // TODO: Check if method name is 'raise' and receiver is nil or self/Kernel
//     // For now, simplified implementation
//     (void)call; // Suppress unused warning
//     return false;
// }

/* TODO: Migrate autocorrect functions to work with Prism nodes
core::AutocorrectSuggestion autocorrectAbstractBody(core::MutableContext ctx, pm_node_t *method,
                                                        core::LocOffsets method_declLoc, pm_node_t *method_body) {
    (void)ctx;
    (void)method;
    (void)method_declLoc;
    (void)method_body;

    // TODO: Implement autocorrect for Prism nodes
    return core::AutocorrectSuggestion{"TODO: Implement autocorrect for Prism", {}};
}
*/

/* TODO: Implement when needed
void ensureAbstractMethodRaises(core::MutableContext ctx, const pm_node_t *node) {
    if (PM_NODE_TYPE_P(node, PM_DEF_NODE)) {
        auto *def = down_cast<pm_def_node_t>(const_cast<pm_node_t *>(node));
        if (def->body && isRaise(def->body)) {
            // Method raises properly, remove body to not error later
            // TODO: Implement body nulling for Prism nodes
            return;
        }

        if (auto e = ctx.beginIndexerError(translateLocation(node->location),
                                           core::errors::Rewriter::RBSAbstractMethodNoRaises)) {
            e.setHeader("Methods declared @abstract with an RBS comment must always raise");
            // TODO: Add autocorrect for Prism nodes
        }
    }
}
*/

/* TODO: Implement when needed
unique_ptr<parser::Node> handleAnnotations(core::MutableContext ctx, const pm_node_t *node,
                                           unique_ptr<parser::Node> sigBuilder, const vector<Comment> &annotations) {
    for (auto &annotation : annotations) {
        if (annotation.string == "final") {
            // no-op, `final` is handled in the `sig()` call later
        } else if (annotation.string == "abstract") {
            sigBuilder =
                parser::PMK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::abstract(), annotation.typeLoc);
            ensureAbstractMethodRaises(ctx, node);
        } else if (annotation.string == "overridable") {
            sigBuilder =
                parser::PMK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::overridable(),
annotation.typeLoc); } else if (annotation.string == "override") { sigBuilder = parser::PMK::Send0(annotation.typeLoc,
move(sigBuilder), core::Names::override_(), annotation.typeLoc);
        }
        // TODO: Add support for other annotations like override(allow_incompatible: true)
    }
    return sigBuilder;
}
*/

/* TODO: Implement when needed
core::NameRef nodeName(const pm_node_t *node) {
    core::NameRef name;
    // TODO: Implement proper node name extraction for Prism nodes
    // This should handle PM_PARAMETER_NODE, PM_REQUIRED_PARAMETER_NODE, etc.
    (void)node; // Suppress unused warning for now
    return name;
}
*/

string nodeKindToString(const pm_node_t *node) {
    switch (PM_NODE_TYPE(node)) {
        case PM_REQUIRED_PARAMETER_NODE:
            return "positional";
        case PM_OPTIONAL_PARAMETER_NODE:
            return "optional positional";
        case PM_REST_PARAMETER_NODE:
            return "rest positional";
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE:
            return "keyword";
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE:
            return "optional keyword";
        case PM_KEYWORD_REST_PARAMETER_NODE:
            return "rest keyword";
        case PM_BLOCK_PARAMETER_NODE:
            return "block";
        default:
            return "unknown";
    }
}

// core::LocOffsets translateLocation(pm_location_t location) {
//     // TODO: This should be shared with CommentsAssociatorPrism
//     // Use proper pointer arithmetic for location translation
//     const uint8_t *sourceStart = location.start;
//     const uint8_t *sourceEnd = location.end;
//     uint32_t start = static_cast<uint32_t>(sourceStart - sourceStart); // This will be 0 for now
//     uint32_t end = static_cast<uint32_t>(sourceEnd - sourceStart);
//     return core::LocOffsets{start, end};
// }

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
    auto typeLoc = declaration.typeLocFromRange(arg.type->location->rg);
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

    // Adjust the location to account for the autocorrect
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

/* TODO: Implement when needed
parser::Args *getMethodArgs(const pm_node_t *node) {
    if (PM_NODE_TYPE_P(node, PM_DEF_NODE)) {
        auto *def = down_cast<pm_def_node_t>(const_cast<pm_node_t *>(node));
        // TODO: Convert Prism parameters to parser::Args
        // For now, return nullptr to indicate no args
        (void)def; // Suppress unused warning
        return nullptr;
    }
    return nullptr;
}
*/

void collectArgs(const RBSDeclaration &declaration, rbs_node_list_t *field, vector<RBSArg> &args, RBSArg::Kind kind) {
    if (field == nullptr || field->length == 0) {
        return;
    }

    for (rbs_node_list_node_t *list_node = field->head; list_node != nullptr; list_node = list_node->next) {
        auto loc = declaration.typeLocFromRange(list_node->node->location->rg);
        auto nameLoc = loc;

        ENFORCE(list_node->node->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in function parameter, expected `{}`", rbs_node_type_name(list_node->node),
                "FunctionParam");

        auto *param = (rbs_types_function_param_t *)list_node->node;
        auto arg = RBSArg{loc, nameLoc, param->name, param->type, kind};
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

// Collects method parameter names (in positional/keyword/block order) from a Prism def node
namespace {
struct MethodParamInfo {
    pm_constant_id_t nameId;
    pm_node_t *node;
};

void appendParamName(vector<MethodParamInfo> &out, pm_node_t *paramNode) {
    if (paramNode == nullptr) {
        return;
    }

    switch (PM_NODE_TYPE(paramNode)) {
        case PM_REQUIRED_PARAMETER_NODE: {
            auto *n = down_cast<pm_required_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, paramNode});
            break;
        }
        case PM_OPTIONAL_PARAMETER_NODE: {
            auto *n = down_cast<pm_optional_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, paramNode});
            break;
        }
        case PM_REST_PARAMETER_NODE: {
            auto *n = down_cast<pm_rest_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, paramNode});
            break;
        }
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: {
            auto *n = down_cast<pm_required_keyword_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, paramNode});
            break;
        }
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: {
            auto *n = down_cast<pm_optional_keyword_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, paramNode});
            break;
        }
        case PM_KEYWORD_REST_PARAMETER_NODE: {
            auto *n = down_cast<pm_keyword_rest_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, paramNode});
            break;
        }
        case PM_BLOCK_PARAMETER_NODE: {
            auto *n = down_cast<pm_block_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, paramNode});
            break;
        }
        default:
            break;
    }
}

vector<MethodParamInfo> getMethodArgs(pm_def_node_t *def) {
    vector<MethodParamInfo> result;
    if (!def || def->parameters == nullptr) {
        return result;
    }

    auto *params = def->parameters;

    // requireds
    if (params->requireds.size > 0) {
        for (size_t i = 0; i < params->requireds.size; i++) {
            appendParamName(result, params->requireds.nodes[i]);
        }
    }
    // optionals
    if (params->optionals.size > 0) {
        for (size_t i = 0; i < params->optionals.size; i++) {
            appendParamName(result, params->optionals.nodes[i]);
        }
    }
    // rest
    appendParamName(result, params->rest);
    // posts (trailing positionals)
    if (params->posts.size > 0) {
        for (size_t i = 0; i < params->posts.size; i++) {
            appendParamName(result, params->posts.nodes[i]);
        }
    }
    // keywords
    if (params->keywords.size > 0) {
        for (size_t i = 0; i < params->keywords.size; i++) {
            appendParamName(result, params->keywords.nodes[i]);
        }
    }
    // keyword rest
    appendParamName(result, params->keyword_rest);
    // block
    appendParamName(result, up_cast(params->block));

    return result;
}
} // namespace

// (Removed minimal type builders that accessed private members)

/* TODO: Implement when needed
unique_ptr<parser::Node> MethodTypeToParserNodePrism::attrSignature(const pm_call_node_t *call, const rbs_node_t *type,
                                                                    const RBSDeclaration &declaration,
                                                                    const vector<Comment> &annotations) {
    // TODO: This function needs migration from parser::Send to pm_call_node_t
    // The original function:
    // 1. Determines the attribute type (reader/writer/accessor)
    // 2. Converts RBS type to parser node
    // 3. Builds appropriate signature for the attribute

    (void)call; // Suppress unused warnings
    (void)type;
    (void)declaration;
    (void)annotations;

    // For now, return a basic stub signature that will allow compilation
    // TODO: Implement full attribute signature translation for Prism
    auto commentLoc = declaration.commentLoc();
    auto fullTypeLoc = declaration.fullTypeLoc();
    auto firstLineTypeLoc = declaration.firstLineTypeLoc();

    // Build a minimal sig { void } signature
    auto sigBuilder = parser::PMK::Send0(fullTypeLoc, parser::PMK::Self(fullTypeLoc), core::Names::void_(),
fullTypeLoc);

    auto sigArgs = parser::NodeVec();
    sigArgs.emplace_back(parser::PMK::T_Sig_WithoutRuntime(firstLineTypeLoc));

    auto sig = parser::PMK::Send(fullTypeLoc, parser::PMK::SorbetPrivateStatic(fullTypeLoc), core::Names::sig(),
                                firstLineTypeLoc, move(sigArgs));

    return make_unique<parser::Block>(commentLoc, move(sig), nullptr, move(sigBuilder));
}
*/

pm_node_t *MethodTypeToParserNodePrism::methodSignature(const pm_node_t *methodDef, const rbs_method_type_t *methodType,
                                                        const RBSDeclaration &declaration,
                                                        const vector<Comment> &annotations) {
    // fmt::print("DEBUG: MethodTypeToParserNodePrism::methodSignature called\n");

    if (!prismParser) {
        return nullptr; // Need Prism parser for node creation
    }

    // Set up shared helpers
    PMK::setParser(prismParser);

    // Parse the RBS method type and create appropriate signature nodes
    const auto &node = *methodType;

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

    // Validate that we have a function type
    if (node.type->type != RBS_TYPES_FUNCTION) {
        auto errLoc = declaration.typeLocFromRange(node.type->location->rg);
        if (auto e = ctx.beginIndexerError(errLoc, core::errors::Rewriter::RBSUnsupported)) {
            e.setHeader("Unexpected node type `{}` in method signature, expected `{}`", rbs_node_type_name(node.type),
                        "Function");
        }
        return nullptr;
    }

    auto *functionType = (rbs_types_function_t *)node.type;

    (void)methodDef;   // Suppress unused warning for now
    (void)annotations; // Suppress unused warning for now

    // Collect RBS parameters for sig params
    vector<RBSArg> args;
    collectArgs(declaration, functionType->required_positionals, args, RBSArg::Kind::Positional);

    collectArgs(declaration, functionType->optional_positionals, args, RBSArg::Kind::OptionalPositional);
    // Include trailing positionals to match non-Prism behavior
    collectArgs(declaration, functionType->trailing_positionals, args, RBSArg::Kind::Positional);
    if (functionType->rest_positionals) {
        auto loc = declaration.typeLocFromRange(functionType->rest_positionals->location->rg);
        auto *param = (rbs_types_function_param_t *)functionType->rest_positionals;
        auto arg = RBSArg{loc, loc, param->name, param->type, RBSArg::Kind::RestPositional};
        args.emplace_back(arg);
    }
    collectKeywords(declaration, functionType->required_keywords, args, RBSArg::Kind::Keyword);
    collectKeywords(declaration, functionType->optional_keywords, args, RBSArg::Kind::OptionalKeyword);
    if (functionType->rest_keywords) {
        auto loc = declaration.typeLocFromRange(functionType->rest_keywords->location->rg);
        auto *param = (rbs_types_function_param_t *)functionType->rest_keywords;
        auto arg = RBSArg{loc, loc, param->name, param->type, RBSArg::Kind::RestKeyword};
        args.emplace_back(arg);
    }
    auto *rbsBlock = node.block;
    if (rbsBlock) {
        auto loc = declaration.typeLocFromRange(rbsBlock->base.location->rg);
        auto arg = RBSArg{loc, loc, nullptr, (rbs_node_t *)rbsBlock, RBSArg::Kind::Block};
        args.emplace_back(arg);
    }

    auto fullTypeLoc = declaration.fullTypeLoc();
    auto firstLineTypeLoc = declaration.firstLineTypeLoc();
    auto commentLoc = declaration.commentLoc();

    // Pre-calculate frequently used locations to avoid redundant calculations
    pm_location_t full_loc = PMK::convertLocOffsets(fullTypeLoc);
    pm_location_t tiny_loc = PMK::convertLocOffsets(firstLineTypeLoc.copyWithZeroLength());
    pm_location_t end_zero_loc = PMK::convertLocOffsets(fullTypeLoc.copyEndWithZeroLength());

    // Create receiver: Sorbet::Private::Static
    pm_node_t *receiver = PMK::SorbetPrivateStatic();
    if (!receiver)
        return nullptr;

    // Create argument: T::Sig::WithoutRuntime
    pm_node_t *t_sig_arg = PMK::TSigWithoutRuntime();
    if (!t_sig_arg)
        return nullptr;

    // Create sig parameter pairs for .params() call
    vector<pm_node_t *> sigParams;
    sigParams.reserve(args.size());

    // Create type converter for RBS types to Prism nodes
    auto typeToPrismNode = TypeToParserNodePrism(ctx, typeParams, parser, prismParser);

    // Collect Ruby method parameter names once (mirror WQ)
    vector<MethodParamInfo> methodArgs;
    if (PM_NODE_TYPE_P((pm_node_t *)methodDef, PM_DEF_NODE)) {
        auto def = down_cast<pm_def_node_t>((pm_node_t *)methodDef);
        methodArgs = getMethodArgs(def);
    }

    size_t paramIndex = 0;
    for (const auto &arg : args) {
        // fmt::print("DEBUG: Processing arg, hasName={}, kind={}\n", (arg.name != nullptr),
        // static_cast<int>(arg.kind));

        // Create type node from RBS type
        pm_node_t *typeNode = typeToPrismNode.toPrismNode(arg.type, declaration);
        if (!typeNode) {
            continue;
        }

        if (methodArgs.empty() || paramIndex >= methodArgs.size()) {
            if (auto e = ctx.beginIndexerError(fullTypeLoc, core::errors::Rewriter::RBSParameterMismatch)) {
                e.setHeader("RBS signature has more parameters than in the method definition");
            }

            return nullptr;
        }

        // Check parameter kind match and generate autocorrect if needed
        pm_node_t *methodArg = methodArgs[paramIndex].node;
        if (!checkParameterKindMatch(arg, methodArg)) {
            if (auto e = ctx.beginIndexerError(arg.loc, core::errors::Rewriter::RBSIncorrectParameterKind)) {
                auto methodArgNameStr = prismParser->resolveConstant(methodArgs[paramIndex].nameId);
                e.setHeader("Argument kind mismatch for `{}`, method declares `{}`, but RBS signature declares `{}`",
                            methodArgNameStr, nodeKindToString(methodArg), argKindToString(arg.kind));

                e.maybeAddAutocorrect(autocorrectArg(ctx, methodArg, arg, *prismParser, declaration));
            }
        }

        // Create symbol node for parameter name
        pm_node_t *symbolNode = nullptr;
        if (arg.name) {
            symbolNode = createSymbolNode(arg.name, arg.nameLoc);
        } else {
            // Fallback to method parameter name when RBS omitted it
            if (!methodArgs.empty() && paramIndex < methodArgs.size() &&
                methodArgs[paramIndex].nameId != PM_CONSTANT_ID_UNSET) {
                core::LocOffsets tinyLocOffsets = firstLineTypeLoc.copyWithZeroLength();
                symbolNode = PMK::SymbolFromConstant(tinyLocOffsets, methodArgs[paramIndex].nameId);
            }
            if (!symbolNode) {
                // As a last resort, synthesize a tiny constant name 'arg'
                symbolNode = PMK::ConstantReadNode("arg");
                if (symbolNode) {
                    symbolNode->location = tiny_loc;
                }
            }
        }
        if (!symbolNode) {
            continue;
        }

        // Create association node (key-value pair) with consistent zero-width location
        core::LocOffsets tinyLocOffsets = firstLineTypeLoc.copyWithZeroLength();
        // fmt::print("DEBUG: Creating assoc with tinyLoc: {}..{} (was arg.loc: {}..{})\n", tinyLocOffsets.beginPos(),
        //            tinyLocOffsets.endPos(), arg.loc.beginPos(), arg.loc.endPos());
        pm_node_t *pairNode = PMK::AssocNode(tinyLocOffsets, symbolNode, typeNode);
        if (pairNode) {
            sigParams.push_back(pairNode);
            // debugPrintLocation("param.symbol.base", symbolNode->location);
            // debugPrintLocation("param.type.base", typeNode->location);
            // debugPrintLocation("param.pair.base", pairNode->location);
        }
        paramIndex++;
    }

    // Build sig chain
    pm_node_t *sigReceiver = PMK::Self();
    if (!sigReceiver)
        return nullptr;

    // Add .type_parameters() call if we have type parameters
    if (typeParams.size() > 0) {
        vector<pm_node_t *> typeParamSymbols;
        typeParamSymbols.reserve(typeParams.size());

        for (auto &param : typeParams) {
            string nameStr = param.second.show(ctx.state);
            pm_node_t *symbolNode = PMK::Symbol(param.first, nameStr.c_str());
            typeParamSymbols.push_back(symbolNode);
        }

        pm_node_t *typeParamsCall = PMK::Send(fullTypeLoc, sigReceiver, "type_parameters", typeParamSymbols);
        ENFORCE(typeParamsCall, "Failed to create type parameters call");

        sigReceiver = typeParamsCall;
    }

    // Add .params() call if we have parameters
    // fmt::print("DEBUG: sigParams.size() = {}\n", sigParams.size());
    if (sigParams.size() > 0) {
        pm_constant_id_t params_id = PMK::addConstantToPool("params");
        if (params_id == PM_CONSTANT_ID_UNSET) {
            return nullptr;
        }

        core::LocOffsets hashLoc = fullTypeLoc;
        // Use KeywordHash for params (bare keyword args)
        pm_node_t *paramsHash = PMK::KeywordHash(hashLoc, sigParams);
        if (!paramsHash) {
            return nullptr;
        }

        // Create .params() method call
        pm_node_t *paramsCall = PMK::Send1(fullTypeLoc, sigReceiver, "params", paramsHash);
        if (!paramsCall) {
            return nullptr;
        }

        sigReceiver = paramsCall;
        // debugPrintLocation("params.call.base", paramsCall->base.location);
        // debugPrintLocation("params.call.msg", paramsCall->message_loc);
    }

    // Add return type call (.void() or .returns(Type))
    pm_node_t *blockBody = nullptr;

    // Pre-calculate return type location for setting on the return type node
    pm_location_t return_type_full_loc =
        PMK::convertLocOffsets(declaration.typeLocFromRange(functionType->return_type->location->rg));

    if (functionType->return_type->type == RBS_TYPES_BASES_VOID) {
        // Create: sigReceiver.void()
        blockBody =
            PMK::Send0(declaration.typeLocFromRange(functionType->return_type->location->rg), sigReceiver, "void");
        if (!blockBody)
            return nullptr;
        // debugPrintLocation("void.call.base", voidCall->base.location);
        // debugPrintLocation("void.call.msg", voidCall->message_loc);
    } else {
        // Create: sigReceiver.returns(Type)
        // Convert actual return type from RBS AST
        pm_node_t *returnTypeNode = typeToPrismNode.toPrismNode(functionType->return_type, declaration);
        ENFORCE(returnTypeNode, "Failed to create return type node");

        // Set return type node base.location to actual return type span
        returnTypeNode->location = return_type_full_loc;

        blockBody = PMK::Send1(declaration.typeLocFromRange(functionType->return_type->location->rg), sigReceiver,
                               "returns", returnTypeNode);
        if (!blockBody)
            return nullptr;
        // debugPrintLocation("returns.call.base", returnsCall->base.location);
        // debugPrintLocation("returns.call.msg", returnsCall->message_loc);
    }

    if (!blockBody)
        return nullptr;

    // Create block node
    pm_block_node_t *block = PMK::allocateNode<pm_block_node_t>();
    if (!block)
        return nullptr;

    *block = (pm_block_node_t){.base = PMK::initializeBaseNode(PM_BLOCK_NODE),
                               .locals = {.size = 0, .capacity = 0, .ids = nullptr},
                               .parameters = nullptr,
                               .body = blockBody,
                               .opening_loc = tiny_loc,
                               .closing_loc = end_zero_loc};
    // Ensure block base.location covers the full RBS type span
    block->base.location = full_loc;

    // Create the main sig call with block
    vector<pm_node_t *> sig_args = {t_sig_arg};
    pm_node_t *call = PMK::Send(fullTypeLoc, receiver, "sig", sig_args, up_cast(block));
    if (!call)
        return nullptr;

    // Debug print important locations to diagnose substr crashes
    // debugPrintLocation("sig.call.base", call->base.location);
    // debugPrintLocation("sig.call.msg", call->message_loc);
    // debugPrintLocation("block.open", block->opening_loc);
    // debugPrintLocation("block.close", block->closing_loc);

    (void)commentLoc; // Suppress unused warning
    return call;
}

pm_node_t *MethodTypeToParserNodePrism::createSymbolNode(rbs_ast_symbol_t *name, core::LocOffsets nameLoc) {
    if (!name) {
        return nullptr;
    }

    // Convert RBS symbol to string and use shared helper
    auto nameStr = parser.resolveConstant(name);
    string nameString(nameStr);

    return PMK::Symbol(nameLoc, nameString.c_str());
}

} // namespace sorbet::rbs
