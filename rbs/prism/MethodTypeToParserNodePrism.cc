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
                parser::MK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::abstract(), annotation.typeLoc);
            ensureAbstractMethodRaises(ctx, node);
        } else if (annotation.string == "overridable") {
            sigBuilder =
                parser::MK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::overridable(), annotation.typeLoc);
        } else if (annotation.string == "override") {
            sigBuilder =
                parser::MK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::override_(), annotation.typeLoc);
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
    auto sigBuilder = parser::MK::Send0(fullTypeLoc, parser::MK::Self(fullTypeLoc), core::Names::void_(), fullTypeLoc);

    auto sigArgs = parser::NodeVec();
    sigArgs.emplace_back(parser::MK::T_Sig_WithoutRuntime(firstLineTypeLoc));

    auto sig = parser::MK::Send(fullTypeLoc, parser::MK::SorbetPrivateStatic(fullTypeLoc), core::Names::sig(),
                                firstLineTypeLoc, move(sigArgs));

    return make_unique<parser::Block>(commentLoc, move(sig), nullptr, move(sigBuilder));
}
*/

// Prism node creation methods
pm_node_t *MethodTypeToParserNodePrism::methodSignature(const pm_node_t *methodDef, const rbs_method_type_t *methodType,
                                                        const RBSDeclaration &declaration,
                                                        const std::vector<Comment> &annotations) {
    // fmt::print("DEBUG: MethodTypeToParserNodePrism::methodSignature called\n");

    if (!prismParser) {
        return nullptr; // Need Prism parser for node creation
    }

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
    pm_location_t full_loc = convertLocOffsets(fullTypeLoc);
    pm_location_t tiny_loc = convertLocOffsets(firstLineTypeLoc.copyWithZeroLength());
    pm_location_t end_zero_loc = convertLocOffsets(fullTypeLoc.copyEndWithZeroLength());

    // Add method names to constant pool
    pm_constant_id_t sig_method_id = addConstantToPool("sig");
    if (sig_method_id == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    // Create receiver: Sorbet::Private::Static
    pm_node_t *receiver = createSorbetPrivateStaticConstant();
    if (!receiver)
        return nullptr;

    // Create argument: T::Sig::WithoutRuntime
    pm_node_t *t_sig_arg = createTSigWithoutRuntimeConstant();
    if (!t_sig_arg)
        return nullptr;

    // Create arguments list
    pm_node_t *arguments = createSingleArgumentNode(t_sig_arg);
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
        fmt::print("DEBUG: Processing arg, hasName={}, kind={}\n", (arg.name != nullptr), static_cast<int>(arg.kind));

        // Create symbol node for parameter name
        pm_node_t *symbolNode = nullptr;
        if (arg.name) {
            symbolNode = createSymbolNode(arg.name, arg.nameLoc);
        } else {
            // Fallback to method parameter name when RBS omitted it
            if (!methodParams.empty() && paramIndex < methodParams.size() &&
                methodParams[paramIndex].nameId != PM_CONSTANT_ID_UNSET) {
                core::LocOffsets tinyLocOffsets = firstLineTypeLoc.copyWithZeroLength();
                symbolNode = createSymbolNodeFromConstant(methodParams[paramIndex].nameId, tinyLocOffsets);
            }
            if (!symbolNode) {
                // As a last resort, synthesize a tiny constant name 'arg'
                symbolNode = createConstantReadNode("arg");
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
        typeNode->location = convertLocOffsets(arg.loc);

        // Create association node (key-value pair) with consistent zero-width location
        core::LocOffsets tinyLocOffsets = firstLineTypeLoc.copyWithZeroLength();
        fmt::print("DEBUG: Creating assoc with tinyLoc: {}..{} (was arg.loc: {}..{})\n",
                   tinyLocOffsets.beginPos(), tinyLocOffsets.endPos(),
                   arg.loc.beginPos(), arg.loc.endPos());
        pm_node_t *pairNode = createAssocNode(symbolNode, typeNode, tinyLocOffsets);
        if (pairNode) {
            sigParams.push_back(pairNode);
            debugPrintLocation("param.symbol.base", symbolNode->location);
            debugPrintLocation("param.type.base", typeNode->location);
            debugPrintLocation("param.pair.base", pairNode->location);
        }
        paramIndex++;
    }

    // Build sig chain: self -> .params(hash) -> .void()/.returns(Type)
    pm_node_t *sigReceiver = createSelfNode();
    if (!sigReceiver)
        return nullptr;

    // Add .params() call if we have parameters
    fmt::print("DEBUG: sigParams.size() = {}\n", sigParams.size());
    if (sigParams.size() > 0) {
        pm_constant_id_t params_id = addConstantToPool("params");
        if (params_id == PM_CONSTANT_ID_UNSET) {
            return nullptr;
        }

        core::LocOffsets hashLoc = fullTypeLoc;
        // Use KeywordHash for params (bare keyword args)
        pm_node_t *paramsHash = createKeywordHashNode(sigParams, hashLoc);
        if (!paramsHash) {
            return nullptr;
        }

        // Create arguments node containing the hash
        pm_node_t *paramsArgs = createSingleArgumentNode(paramsHash);
        if (!paramsArgs) {
            return nullptr;
        }

        // Create .params() method call
        pm_call_node_t *paramsCall = createMethodCall(sigReceiver, params_id, paramsArgs, tiny_loc, full_loc, tiny_loc);
        if (!paramsCall) {
            return nullptr;
        }

        sigReceiver = up_cast(paramsCall);
        debugPrintLocation("params.call.base", paramsCall->base.location);
        debugPrintLocation("params.call.msg", paramsCall->message_loc);
    }

    // Add return type call (.void() or .returns(Type))
    pm_node_t *blockBody = nullptr;

    // Pre-calculate return type locations to avoid redundant calculations
    pm_location_t return_type_full_loc =
        convertLocOffsets(declaration.typeLocFromRange(functionType->return_type->location->rg));
    pm_location_t return_type_zero_loc = convertLocOffsets(
        declaration.typeLocFromRange(functionType->return_type->location->rg).copyWithZeroLength());

    if (functionType->return_type->type == RBS_TYPES_BASES_VOID) {
        // Create: sigReceiver.void()
        pm_constant_id_t void_id = addConstantToPool("void");
        if (void_id == PM_CONSTANT_ID_UNSET)
            return nullptr;

        pm_call_node_t *voidCall = createMethodCall(sigReceiver, void_id, nullptr, return_type_zero_loc, full_loc, tiny_loc);
        if (!voidCall)
            return nullptr;
        blockBody = up_cast(voidCall);
        debugPrintLocation("void.call.base", voidCall->base.location);
        debugPrintLocation("void.call.msg", voidCall->message_loc);
    } else {
        // Create: sigReceiver.returns(Type)
        pm_constant_id_t returns_id = addConstantToPool("returns");
        if (returns_id == PM_CONSTANT_ID_UNSET)
            return nullptr;

        // Convert actual return type from RBS AST
        pm_node_t *returnTypeNode = typeToPrismNode.toPrismNode(functionType->return_type, declaration);
        if (!returnTypeNode)
            return nullptr;

        // Set return type node base.location to actual return type span
        returnTypeNode->location = return_type_full_loc;

        pm_node_t *returnsArguments = createSingleArgumentNode(returnTypeNode);
        if (!returnsArguments)
            return nullptr;

        pm_call_node_t *returnsCall =
            createMethodCall(sigReceiver, returns_id, returnsArguments, return_type_zero_loc, full_loc, tiny_loc);
        if (!returnsCall)
            return nullptr;
        blockBody = up_cast(returnsCall);
        debugPrintLocation("returns.call.base", returnsCall->base.location);
        debugPrintLocation("returns.call.msg", returnsCall->message_loc);
    }

    if (!blockBody)
        return nullptr;

    // Create block node
    pm_block_node_t *block = allocateNode<pm_block_node_t>();
    if (!block)
        return nullptr;

    *block = (pm_block_node_t){.base = initializeBaseNode(PM_BLOCK_NODE),
                               .locals = {.size = 0, .capacity = 0, .ids = nullptr},
                               .parameters = nullptr,
                               .body = blockBody,
                               .opening_loc = tiny_loc,
                               .closing_loc = end_zero_loc};
    // Ensure block base.location covers the full RBS type span
    block->base.location = full_loc;

    // Create the main sig call
    pm_call_node_t *call =
        createMethodCall(receiver, sig_method_id, arguments, tiny_loc, full_loc, tiny_loc, up_cast(block));
    if (!call)
        return nullptr;

    // Debug print important locations to diagnose substr crashes
    debugPrintLocation("sig.call.base", call->base.location);
    debugPrintLocation("sig.call.msg", call->message_loc);
    debugPrintLocation("block.open", block->opening_loc);
    debugPrintLocation("block.close", block->closing_loc);

    (void)commentLoc; // Suppress unused warning
    return up_cast(call);
}

// Node creation helper implementations (moved from SignatureTranslatorPrism)

template <typename T> T *MethodTypeToParserNodePrism::allocateNode() {
    T *node = (T *)calloc(1, sizeof(T));
    return node; // Returns nullptr on allocation failure
}

pm_node_t MethodTypeToParserNodePrism::initializeBaseNode(pm_node_type_t type) {
    if (!prismParser) {
        // Return a default-initialized node if parser is not available
        return (pm_node_t){.type = type, .flags = 0, .node_id = 0, .location = {.start = nullptr, .end = nullptr}};
    }

    pm_parser_t *p = prismParser->getInternalParser();
    pm_location_t loc = getZeroWidthLocation();

    return (pm_node_t){.type = type, .flags = 0, .node_id = ++p->node_id, .location = loc};
}

pm_node_t *MethodTypeToParserNodePrism::createConstantReadNode(const char *name) {
    pm_constant_id_t constant_id = addConstantToPool(name);
    if (constant_id == PM_CONSTANT_ID_UNSET)
        return nullptr;

    pm_constant_read_node_t *node = allocateNode<pm_constant_read_node_t>();
    if (!node)
        return nullptr;

    *node = (pm_constant_read_node_t){.base = initializeBaseNode(PM_CONSTANT_READ_NODE), .name = constant_id};

    return up_cast(node);
}

pm_node_t *MethodTypeToParserNodePrism::createConstantPathNode(pm_node_t *parent, const char *name) {
    pm_constant_id_t name_id = addConstantToPool(name);
    if (name_id == PM_CONSTANT_ID_UNSET)
        return nullptr;

    pm_constant_path_node_t *node = allocateNode<pm_constant_path_node_t>();
    if (!node)
        return nullptr;

    pm_location_t loc = getZeroWidthLocation();

    *node = (pm_constant_path_node_t){.base = initializeBaseNode(PM_CONSTANT_PATH_NODE),
                                      .parent = parent,
                                      .name = name_id,
                                      .delimiter_loc = loc,
                                      .name_loc = loc};

    return up_cast(node);
}

pm_node_t *MethodTypeToParserNodePrism::createSingleArgumentNode(pm_node_t *arg) {
    pm_arguments_node_t *arguments = allocateNode<pm_arguments_node_t>();
    if (!arguments)
        return nullptr;

    pm_node_t **arg_nodes = (pm_node_t **)calloc(1, sizeof(pm_node_t *));
    if (!arg_nodes) {
        free(arguments);
        return nullptr;
    }
    arg_nodes[0] = arg;

    *arguments = (pm_arguments_node_t){.base = initializeBaseNode(PM_ARGUMENTS_NODE),
                                       .arguments = {.size = 1, .capacity = 1, .nodes = arg_nodes}};
    // Align base.location with contained argument's location (tiny point)
    arguments->base.location = arg->location;

    return up_cast(arguments);
}

pm_node_t *MethodTypeToParserNodePrism::createSorbetPrivateStaticConstant() {
    // Build a root-anchored constant path ::Sorbet::Private::Static
    pm_node_t *sorbet = createConstantPathNode(nullptr, "Sorbet");
    if (!sorbet)
        return nullptr;

    pm_node_t *sorbet_private = createConstantPathNode(sorbet, "Private");
    if (!sorbet_private)
        return nullptr;

    return createConstantPathNode(sorbet_private, "Static");
}

pm_node_t *MethodTypeToParserNodePrism::createTSigWithoutRuntimeConstant() {
    // Build a root-anchored constant path ::T::Sig::WithoutRuntime
    pm_node_t *t_const = createConstantPathNode(nullptr, "T");
    if (!t_const)
        return nullptr;

    pm_node_t *t_sig = createConstantPathNode(t_const, "Sig");
    if (!t_sig)
        return nullptr;

    return createConstantPathNode(t_sig, "WithoutRuntime");
}

pm_node_t *MethodTypeToParserNodePrism::createSelfNode() {
    pm_self_node_t *selfNode = allocateNode<pm_self_node_t>();
    if (!selfNode)
        return nullptr;

    *selfNode = (pm_self_node_t){.base = initializeBaseNode(PM_SELF_NODE)};

    return up_cast(selfNode);
}

pm_constant_id_t MethodTypeToParserNodePrism::addConstantToPool(const char *name) {
    if (!prismParser)
        return PM_CONSTANT_ID_UNSET;

    pm_parser_t *p = prismParser->getInternalParser();
    size_t name_len = strlen(name);
    uint8_t *stable = (uint8_t *)calloc(name_len, sizeof(uint8_t));
    if (!stable) {
        return PM_CONSTANT_ID_UNSET;
    }
    memcpy(stable, name, name_len);
    pm_constant_id_t id = pm_constant_pool_insert_constant(&p->constant_pool, stable, name_len);
    return id; // Returns PM_CONSTANT_ID_UNSET on failure
}

pm_location_t MethodTypeToParserNodePrism::getZeroWidthLocation() {
    if (!prismParser) {
        return {.start = nullptr, .end = nullptr};
    }

    pm_parser_t *p = prismParser->getInternalParser();
    const uint8_t *source_start = p->start;
    return {.start = source_start, .end = source_start};
}

pm_location_t MethodTypeToParserNodePrism::convertLocOffsets(core::LocOffsets loc) {
    if (!prismParser) {
        return {.start = nullptr, .end = nullptr};
    }

    pm_parser_t *p = prismParser->getInternalParser();
    const uint8_t *source_start = p->start;

    // Convert byte offsets to pointers
    const uint8_t *start_ptr = source_start + loc.beginPos();
    const uint8_t *end_ptr = source_start + loc.endPos();

    return {.start = start_ptr, .end = end_ptr};
}

pm_node_t *MethodTypeToParserNodePrism::createSymbolNode(rbs_ast_symbol_t *name, core::LocOffsets nameLoc) {
    if (!name || !prismParser) {
        return nullptr;
    }

    pm_symbol_node_t *symbolNode = allocateNode<pm_symbol_node_t>();
    if (!symbolNode) {
        return nullptr;
    }

    // Access parser only if needed; currently unused
    // Get the symbol name string
    auto nameStr = parser.resolveConstant(name);
    std::string nameString(nameStr); // Convert string_view to string

    // Copy symbol text into a stable buffer to avoid dangling pointer
    uint8_t *stable = (uint8_t *)calloc(nameString.size(), sizeof(uint8_t));
    if (!stable) {
        return nullptr;
    }
    memcpy(stable, nameString.data(), nameString.size());

    // Create a string literal for the unescaped field
    pm_string_t unescaped_string;
    unescaped_string.source = stable;
    unescaped_string.length = nameString.length();

    pm_location_t location = convertLocOffsets(nameLoc.copyWithZeroLength());

    *symbolNode = (pm_symbol_node_t){.base = initializeBaseNode(PM_SYMBOL_NODE),
                                     .opening_loc = location,
                                     .value_loc = location,
                                     .closing_loc = location,
                                     .unescaped = unescaped_string};
    symbolNode->base.location = location;

    return up_cast(symbolNode);
}

pm_node_t *MethodTypeToParserNodePrism::createSymbolNodeFromConstant(pm_constant_id_t nameId,
                                                                     core::LocOffsets nameLoc) {
    if (!prismParser || nameId == PM_CONSTANT_ID_UNSET) {
        return nullptr;
    }

    auto nameView = prismParser->resolveConstant(nameId);
    std::string nameString(nameView);

    uint8_t *stable = (uint8_t *)calloc(nameString.size(), sizeof(uint8_t));
    if (!stable) {
        return nullptr;
    }
    memcpy(stable, nameString.data(), nameString.size());

    pm_symbol_node_t *symbolNode = allocateNode<pm_symbol_node_t>();
    if (!symbolNode) {
        return nullptr;
    }

    pm_location_t location = convertLocOffsets(nameLoc.copyWithZeroLength());

    pm_string_t unescaped_string;
    unescaped_string.source = stable;
    unescaped_string.length = nameString.length();

    *symbolNode = (pm_symbol_node_t){.base = initializeBaseNode(PM_SYMBOL_NODE),
                                     .opening_loc = location,
                                     .value_loc = location,
                                     .closing_loc = location,
                                     .unescaped = unescaped_string};
    symbolNode->base.location = location;

    return up_cast(symbolNode);
}

pm_node_t *MethodTypeToParserNodePrism::createAssocNode(pm_node_t *key, pm_node_t *value, core::LocOffsets loc) {
    if (!key || !value) {
        return nullptr;
    }

    pm_assoc_node_t *assocNode = allocateNode<pm_assoc_node_t>();
    if (!assocNode) {
        return nullptr;
    }

    pm_location_t location = convertLocOffsets(loc.copyWithZeroLength());

    *assocNode = (pm_assoc_node_t){
        .base = initializeBaseNode(PM_ASSOC_NODE), .key = key, .value = value, .operator_loc = location};
    assocNode->base.location = location;

    return up_cast(assocNode);
}

pm_node_t *MethodTypeToParserNodePrism::createHashNode(const std::vector<pm_node_t *> &pairs, core::LocOffsets loc) {
    if (pairs.empty()) {
        return nullptr;
    }

    pm_hash_node_t *hashNode = allocateNode<pm_hash_node_t>();
    if (!hashNode) {
        return nullptr;
    }

    // Allocate array of pm_node_t* for elements
    pm_node_t **elements = nullptr;
    if (!pairs.empty()) {
        // Use calloc to zero-initialize and avoid uninitialized memory
        elements = (pm_node_t **)calloc(pairs.size(), sizeof(pm_node_t *));
        if (!elements) {
            return nullptr;
        }
        for (size_t i = 0; i < pairs.size(); i++) {
            elements[i] = pairs[i];
        }
    }

    // Base span covers the whole hash. For keyword-arg style (no braces), opening/closing are null.
    pm_location_t base_loc = convertLocOffsets(loc);
    pm_location_t opening_loc = {.start = nullptr, .end = nullptr};
    pm_location_t closing_loc = {.start = nullptr, .end = nullptr};

    *hashNode = (pm_hash_node_t){.base = initializeBaseNode(PM_HASH_NODE),
                                 .opening_loc = opening_loc,
                                 .elements = {.size = pairs.size(), .capacity = pairs.size(), .nodes = elements},
                                 .closing_loc = closing_loc};
    hashNode->base.location = base_loc;

    return up_cast(hashNode);
}

pm_node_t *MethodTypeToParserNodePrism::createKeywordHashNode(const std::vector<pm_node_t *> &pairs,
                                                              core::LocOffsets loc) {
    if (pairs.empty()) {
        return nullptr;
    }

    pm_keyword_hash_node_t *hashNode = allocateNode<pm_keyword_hash_node_t>();
    if (!hashNode) {
        return nullptr;
    }

    pm_node_t **elements = nullptr;
    elements = (pm_node_t **)calloc(pairs.size(), sizeof(pm_node_t *));
    if (!elements) {
        return nullptr;
    }
    for (size_t i = 0; i < pairs.size(); i++) {
        elements[i] = pairs[i];
    }

    pm_location_t base_loc = convertLocOffsets(loc);

    *hashNode =
        (pm_keyword_hash_node_t){.base = initializeBaseNode(PM_KEYWORD_HASH_NODE),
                                 .elements = {.size = pairs.size(), .capacity = pairs.size(), .nodes = elements}};
    hashNode->base.location = base_loc;

    return up_cast(hashNode);
}

void MethodTypeToParserNodePrism::debugPrintLocation(const char *label, pm_location_t loc) {
    if (!prismParser) {
        fmt::print("DEBUG {}: parser not available\n", label);
        return;
    }

    pm_parser_t *p = prismParser->getInternalParser();
    size_t b = loc.start ? (size_t)(loc.start - p->start) : (size_t)0;
    size_t e = loc.end ? (size_t)(loc.end - p->start) : (size_t)0;
    fmt::print("DEBUG {}: {}..{}\n", label, b, e);
}

pm_call_node_t *MethodTypeToParserNodePrism::createMethodCall(pm_node_t *receiver, pm_constant_id_t method_id,
                                                              pm_node_t *arguments, pm_location_t message_loc,
                                                              pm_location_t full_loc, pm_location_t tiny_loc,
                                                              pm_node_t *block) {
    pm_call_node_t *call = allocateNode<pm_call_node_t>();
    if (!call) {
        return nullptr;
    }

    *call = (pm_call_node_t){.base = initializeBaseNode(PM_CALL_NODE),
                             .receiver = receiver,
                             .call_operator_loc = tiny_loc,
                             .name = method_id,
                             .message_loc = message_loc,
                             .opening_loc = tiny_loc,
                             .arguments = down_cast<pm_arguments_node_t>(arguments),
                             .closing_loc = tiny_loc,
                             .block = block};
    call->base.location = full_loc;

    return call;
}

} // namespace sorbet::rbs
