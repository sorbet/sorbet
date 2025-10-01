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

/* TODO: Implement when needed
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
*/

/* TODO: Implement when needed
string nodeKindToString(const pm_node_t *node) {
    // TODO: Add proper node kind string mapping for Prism nodes
    // This should handle all the Prism parameter node types
    (void)node; // Suppress unused warning for now
    return "unknown";
}
*/

// core::LocOffsets translateLocation(pm_location_t location) {
//     // TODO: This should be shared with CommentsAssociatorPrism
//     // Use proper pointer arithmetic for location translation
//     const uint8_t *sourceStart = location.start;
//     const uint8_t *sourceEnd = location.end;
//     uint32_t start = static_cast<uint32_t>(sourceStart - sourceStart); // This will be 0 for now
//     uint32_t end = static_cast<uint32_t>(sourceEnd - sourceStart);
//     return core::LocOffsets{start, end};
// }

/* TODO: Implement when needed
bool checkParameterKindMatch(const RBSArg &arg, const pm_node_t *methodArg) {
    // TODO: Implement proper parameter kind matching for Prism nodes
    // This should handle all Prism parameter node types
    (void)arg;
    (void)methodArg;
    return true; // Placeholder - always return true for now
}
*/

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
        // Use same location for nameLoc for now - could implement adjustNameLoc later
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
    pm_location_t loc;
};

void appendParamName(vector<MethodParamInfo> &out, pm_node_t *paramNode) {
    if (paramNode == nullptr) {
        return;
    }

    switch (PM_NODE_TYPE(paramNode)) {
        case PM_REQUIRED_PARAMETER_NODE: {
            auto *n = down_cast<pm_required_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, n->base.location});
            break;
        }
        case PM_OPTIONAL_PARAMETER_NODE: {
            auto *n = down_cast<pm_optional_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, n->base.location});
            break;
        }
        case PM_REST_PARAMETER_NODE: {
            auto *n = down_cast<pm_rest_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, n->base.location});
            break;
        }
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: {
            auto *n = down_cast<pm_required_keyword_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, n->base.location});
            break;
        }
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: {
            auto *n = down_cast<pm_optional_keyword_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, n->base.location});
            break;
        }
        case PM_KEYWORD_REST_PARAMETER_NODE: {
            auto *n = down_cast<pm_keyword_rest_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, n->base.location});
            break;
        }
        case PM_BLOCK_PARAMETER_NODE: {
            auto *n = down_cast<pm_block_parameter_node_t>(paramNode);
            out.push_back(MethodParamInfo{n->name, n->base.location});
            break;
        }
        default:
            break;
    }
}

vector<MethodParamInfo> collectMethodParamsFromDef(pm_def_node_t *def) {
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
                                                        const std::vector<Comment> &annotations) {
    // fmt::print("DEBUG: MethodTypeToParserNodePrism::methodSignature called\n");

    if (!prismParser) {
        return nullptr; // Need Prism parser for node creation
    }

    // Set up shared helpers
    PMK::setParser(prismParser);

    // Parse the RBS method type and create appropriate signature nodes
    const auto &node = *methodType;

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
    std::vector<RBSArg> args;
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

    // Add method names to constant pool
    pm_constant_id_t sig_method_id = PMK::addConstantToPool("sig");
    if (sig_method_id == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    // Create receiver: Sorbet::Private::Static
    pm_node_t *receiver = PMK::createSorbetPrivateStaticConstant();
    if (!receiver)
        return nullptr;

    // Create argument: T::Sig::WithoutRuntime
    pm_node_t *t_sig_arg = PMK::createTSigWithoutRuntimeConstant();
    if (!t_sig_arg)
        return nullptr;

    // Create arguments list
    pm_node_t *arguments = PMK::createSingleArgumentNode(t_sig_arg);
    if (!arguments)
        return nullptr;

    // Create sig parameter pairs for .params() call
    std::vector<pm_node_t *> sigParams;
    sigParams.reserve(args.size());

    // Create type converter for RBS types to Prism nodes
    // TODO: Collect type parameters like the original
    std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams; // Empty for now
    auto typeToPrismNode = TypeToParserNodePrism(ctx, typeParams, parser, prismParser);

    // Collect Ruby method parameter names once (mirror WQ)
    std::vector<MethodParamInfo> methodParams;
    if (PM_NODE_TYPE_P((pm_node_t *)methodDef, PM_DEF_NODE)) {
        auto def = down_cast<pm_def_node_t>((pm_node_t *)methodDef);
        methodParams = collectMethodParamsFromDef(def);
    }

    size_t paramIndex = 0;
    for (const auto &arg : args) {
        // fmt::print("DEBUG: Processing arg, hasName={}, kind={}\n", (arg.name != nullptr),
        // static_cast<int>(arg.kind));

        // Create symbol node for parameter name
        pm_node_t *symbolNode = nullptr;
        if (arg.name) {
            symbolNode = createSymbolNode(arg.name, arg.nameLoc);
        } else {
            // Fallback to method parameter name when RBS omitted it
            if (!methodParams.empty() && paramIndex < methodParams.size() &&
                methodParams[paramIndex].nameId != PM_CONSTANT_ID_UNSET) {
                core::LocOffsets tinyLocOffsets = firstLineTypeLoc.copyWithZeroLength();
                symbolNode = PMK::createSymbolNodeFromConstant(methodParams[paramIndex].nameId, tinyLocOffsets);
            }
            if (!symbolNode) {
                // As a last resort, synthesize a tiny constant name 'arg'
                symbolNode = PMK::createConstantReadNode("arg");
                if (symbolNode) {
                    symbolNode->location = tiny_loc;
                }
            }
        }
        if (!symbolNode) {
            continue;
        }

        // Create type node from RBS type
        pm_node_t *typeNode = typeToPrismNode.toPrismNode(arg.type, declaration);
        if (!typeNode) {
            continue;
        }
        // Ensure type node has a valid location within the file (use the arg span)
        typeNode->location = PMK::convertLocOffsets(arg.loc);

        // Create association node (key-value pair) with consistent zero-width location
        core::LocOffsets tinyLocOffsets = firstLineTypeLoc.copyWithZeroLength();
        // fmt::print("DEBUG: Creating assoc with tinyLoc: {}..{} (was arg.loc: {}..{})\n", tinyLocOffsets.beginPos(),
        //            tinyLocOffsets.endPos(), arg.loc.beginPos(), arg.loc.endPos());
        pm_node_t *pairNode = PMK::createAssocNode(symbolNode, typeNode, tinyLocOffsets);
        if (pairNode) {
            sigParams.push_back(pairNode);
            // debugPrintLocation("param.symbol.base", symbolNode->location);
            // debugPrintLocation("param.type.base", typeNode->location);
            // debugPrintLocation("param.pair.base", pairNode->location);
        }
        paramIndex++;
    }

    // Build sig chain: self -> .params(hash) -> .void()/.returns(Type)
    pm_node_t *sigReceiver = PMK::createSelfNode();
    if (!sigReceiver)
        return nullptr;

    // Add .params() call if we have parameters
    // fmt::print("DEBUG: sigParams.size() = {}\n", sigParams.size());
    if (sigParams.size() > 0) {
        pm_constant_id_t params_id = PMK::addConstantToPool("params");
        if (params_id == PM_CONSTANT_ID_UNSET) {
            return nullptr;
        }

        core::LocOffsets hashLoc = fullTypeLoc;
        // Use KeywordHash for params (bare keyword args)
        pm_node_t *paramsHash = PMK::createKeywordHashNode(sigParams, hashLoc);
        if (!paramsHash) {
            return nullptr;
        }

        // Create arguments node containing the hash
        pm_node_t *paramsArgs = PMK::createSingleArgumentNode(paramsHash);
        if (!paramsArgs) {
            return nullptr;
        }

        // Create .params() method call
        pm_call_node_t *paramsCall =
            PMK::createMethodCall(sigReceiver, params_id, paramsArgs, tiny_loc, full_loc, tiny_loc);
        if (!paramsCall) {
            return nullptr;
        }

        sigReceiver = up_cast(paramsCall);
        // debugPrintLocation("params.call.base", paramsCall->base.location);
        // debugPrintLocation("params.call.msg", paramsCall->message_loc);
    }

    // Add return type call (.void() or .returns(Type))
    pm_node_t *blockBody = nullptr;

    // Pre-calculate return type locations to avoid redundant calculations
    pm_location_t return_type_full_loc =
        PMK::convertLocOffsets(declaration.typeLocFromRange(functionType->return_type->location->rg));
    pm_location_t return_type_zero_loc = PMK::convertLocOffsets(
        declaration.typeLocFromRange(functionType->return_type->location->rg).copyWithZeroLength());

    if (functionType->return_type->type == RBS_TYPES_BASES_VOID) {
        // Create: sigReceiver.void()
        pm_constant_id_t void_id = PMK::addConstantToPool("void");
        if (void_id == PM_CONSTANT_ID_UNSET)
            return nullptr;

        pm_call_node_t *voidCall =
            PMK::createMethodCall(sigReceiver, void_id, nullptr, return_type_zero_loc, full_loc, tiny_loc);
        if (!voidCall)
            return nullptr;
        blockBody = up_cast(voidCall);
        // debugPrintLocation("void.call.base", voidCall->base.location);
        // debugPrintLocation("void.call.msg", voidCall->message_loc);
    } else {
        // Create: sigReceiver.returns(Type)
        pm_constant_id_t returns_id = PMK::addConstantToPool("returns");
        if (returns_id == PM_CONSTANT_ID_UNSET)
            return nullptr;

        // Convert actual return type from RBS AST
        pm_node_t *returnTypeNode = typeToPrismNode.toPrismNode(functionType->return_type, declaration);
        if (!returnTypeNode)
            return nullptr;

        // Set return type node base.location to actual return type span
        returnTypeNode->location = return_type_full_loc;

        pm_node_t *returnsArguments = PMK::createSingleArgumentNode(returnTypeNode);
        if (!returnsArguments)
            return nullptr;

        pm_call_node_t *returnsCall =
            PMK::createMethodCall(sigReceiver, returns_id, returnsArguments, return_type_zero_loc, full_loc, tiny_loc);
        if (!returnsCall)
            return nullptr;
        blockBody = up_cast(returnsCall);
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

    // Create the main sig call
    pm_call_node_t *call =
        PMK::createMethodCall(receiver, sig_method_id, arguments, tiny_loc, full_loc, tiny_loc, up_cast(block));
    if (!call)
        return nullptr;

    // Debug print important locations to diagnose substr crashes
    // debugPrintLocation("sig.call.base", call->base.location);
    // debugPrintLocation("sig.call.msg", call->message_loc);
    // debugPrintLocation("block.open", block->opening_loc);
    // debugPrintLocation("block.close", block->closing_loc);

    (void)commentLoc; // Suppress unused warning
    return up_cast(call);
}

pm_node_t *MethodTypeToParserNodePrism::createSymbolNode(rbs_ast_symbol_t *name, core::LocOffsets nameLoc) {
    if (!name) {
        return nullptr;
    }

    // Convert RBS symbol to string and use shared helper
    auto nameStr = parser.resolveConstant(name);
    std::string nameString(nameStr);

    return PMK::createSymbolNode(nameString.c_str(), nameLoc);
}

} // namespace sorbet::rbs
