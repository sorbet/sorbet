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

/* TODO: Implement when needed
core::LocOffsets translateLocation(pm_location_t location) {
    // TODO: This should be shared with CommentsAssociatorPrism
    // Use proper pointer arithmetic for location translation
    const uint8_t *sourceStart = location.start;
    const uint8_t *sourceEnd = location.end;
    uint32_t start = static_cast<uint32_t>(sourceStart - sourceStart); // This will be 0 for now
    uint32_t end = static_cast<uint32_t>(sourceEnd - sourceStart);
    return core::LocOffsets{start, end};
}
*/

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

/* TODO: Implement when needed
void collectArgs(const RBSDeclaration &declaration, rbs_node_list_t *field, vector<RBSArg> &args, RBSArg::Kind kind) {
    if (field == nullptr || field->length == 0) {
        return;
    }

    for (rbs_node_list_node_t *list_node = field->head; list_node != nullptr; list_node = list_node->next) {
        auto loc = declaration.typeLocFromRange(list_node->node->location->rg);
        auto nameLoc = adjustNameLoc(declaration, list_node->node);

        ENFORCE(list_node->node->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in function parameter, expected `{}`", rbs_node_type_name(list_node->node),
                "FunctionParam");

        auto *param = (rbs_types_function_param_t *)list_node->node;
        auto arg = RBSArg{loc, nameLoc, param->name, param->type, kind};
        args.emplace_back(arg);
    }
}
*/

/* TODO: Implement when needed
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
*/

} // namespace

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

    // For simple function translations, focus on return type only for now
    // TODO: Add parameter handling later
    (void)methodDef;   // Suppress unused warning for now
    (void)annotations; // Suppress unused warning for now

    pm_location_t loc = getZeroWidthLocation();

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

    // Create block body based on return type
    pm_node_t *blockBody = nullptr;

    if (functionType->return_type->type == RBS_TYPES_BASES_VOID) {
        // Create void call: void
        pm_constant_id_t void_id = addConstantToPool("void");
        if (void_id == PM_CONSTANT_ID_UNSET)
            return nullptr;

        pm_call_node_t *voidCall = allocateNode<pm_call_node_t>();
        if (!voidCall)
            return nullptr;

        *voidCall = (pm_call_node_t){.base = initializeBaseNode(PM_CALL_NODE),
                                     .receiver = nullptr, // No explicit receiver (implicit self)
                                     .call_operator_loc = {.start = nullptr, .end = nullptr},
                                     .name = void_id,
                                     .message_loc = loc,
                                     .opening_loc = {.start = nullptr, .end = nullptr},
                                     .arguments = nullptr,
                                     .closing_loc = {.start = nullptr, .end = nullptr},
                                     .block = nullptr};
        blockBody = up_cast(voidCall);
    } else {
        // For non-void types, create returns(Type) call
        pm_constant_id_t returns_id = addConstantToPool("returns");
        if (returns_id == PM_CONSTANT_ID_UNSET)
            return nullptr;

        // For simple cases, default to String type
        // TODO: Parse actual return type from RBS AST
        pm_node_t *returnTypeNode = createStringConstant();
        if (!returnTypeNode)
            return nullptr;

        pm_node_t *returnsArguments = createSingleArgumentNode(returnTypeNode);
        if (!returnsArguments)
            return nullptr;

        pm_call_node_t *returnsCall = allocateNode<pm_call_node_t>();
        if (!returnsCall)
            return nullptr;

        *returnsCall = (pm_call_node_t){.base = initializeBaseNode(PM_CALL_NODE),
                                        .receiver = nullptr, // No explicit receiver (implicit self)
                                        .call_operator_loc = {.start = nullptr, .end = nullptr},
                                        .name = returns_id,
                                        .message_loc = loc,
                                        .opening_loc = loc,
                                        .arguments = down_cast<pm_arguments_node_t>(returnsArguments),
                                        .closing_loc = loc,
                                        .block = nullptr};
        blockBody = up_cast(returnsCall);
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
                               .opening_loc = loc,
                               .closing_loc = loc};

    // Create the main sig call
    pm_call_node_t *call = allocateNode<pm_call_node_t>();
    if (!call)
        return nullptr;

    *call = (pm_call_node_t){.base = initializeBaseNode(PM_CALL_NODE),
                             .receiver = receiver,
                             .call_operator_loc = loc,
                             .name = sig_method_id,
                             .message_loc = loc,
                             .opening_loc = loc,
                             .arguments = down_cast<pm_arguments_node_t>(arguments),
                             .closing_loc = loc,
                             .block = up_cast(block)};

    (void)declaration; // Suppress unused warning for now

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

    return up_cast(arguments);
}

pm_node_t *MethodTypeToParserNodePrism::createSorbetPrivateStaticConstant() {
    // Create Sorbet constant read
    pm_node_t *sorbet = createConstantReadNode("Sorbet");
    if (!sorbet)
        return nullptr;

    // Create Sorbet::Private constant path
    pm_node_t *sorbet_private = createConstantPathNode(sorbet, "Private");
    if (!sorbet_private)
        return nullptr;

    // Create Sorbet::Private::Static constant path
    return createConstantPathNode(sorbet_private, "Static");
}

pm_node_t *MethodTypeToParserNodePrism::createTSigWithoutRuntimeConstant() {
    // Create T constant read
    pm_node_t *t_const = createConstantReadNode("T");
    if (!t_const)
        return nullptr;

    // Create T::Sig constant path
    pm_node_t *t_sig = createConstantPathNode(t_const, "Sig");
    if (!t_sig)
        return nullptr;

    // Create T::Sig::WithoutRuntime constant path
    return createConstantPathNode(t_sig, "WithoutRuntime");
}

pm_node_t *MethodTypeToParserNodePrism::createStringConstant() {
    return createConstantReadNode("String");
}

pm_constant_id_t MethodTypeToParserNodePrism::addConstantToPool(const char *name) {
    if (!prismParser)
        return PM_CONSTANT_ID_UNSET;

    pm_parser_t *p = prismParser->getInternalParser();
    pm_constant_id_t id = pm_constant_pool_insert_constant(&p->constant_pool, (const uint8_t *)name, strlen(name));
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


} // namespace sorbet::rbs
