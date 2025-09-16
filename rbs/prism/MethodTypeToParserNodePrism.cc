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
core::LocOffsets translateLocation(pm_location_t location);

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

bool isRaise(pm_node_t *node) {
    if (!PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
        return false;
    }

    auto *call = down_cast<pm_call_node_t>(node);
    // TODO: Check if method name is 'raise' and receiver is nil or self/Kernel
    // For now, simplified implementation
    (void)call; // Suppress unused warning
    return false;
}

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

core::LocOffsets translateLocation(pm_location_t location) {
    // TODO: This should be shared with CommentsAssociatorPrism
    // Use proper pointer arithmetic for location translation
    const uint8_t *sourceStart = location.start;
    const uint8_t *sourceEnd = location.end;
    uint32_t start = static_cast<uint32_t>(sourceStart - sourceStart); // This will be 0 for now
    uint32_t end = static_cast<uint32_t>(sourceEnd - sourceStart);
    return core::LocOffsets{start, end};
}

/* TODO: Implement when needed
bool checkParameterKindMatch(const RBSArg &arg, const pm_node_t *methodArg) {
    // TODO: Implement proper parameter kind matching for Prism nodes
    // This should handle all Prism parameter node types
    (void)arg;
    (void)methodArg;
    return true; // Placeholder - always return true for now
}
*/

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

unique_ptr<parser::Node> MethodTypeToParserNodePrism::methodSignature(const pm_node_t *methodDef,
                                                                      const rbs_method_type_t *methodType,
                                                                      const RBSDeclaration &declaration,
                                                                      const vector<Comment> &annotations) {
    const auto &node = *methodType;
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

    // Collect arguments (following the same pattern as original)
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

    auto methodArgs = getMethodArgs(methodDef);
    for (int i = 0; i < args.size(); i++) {
        auto &arg = args[i];
        auto type = typeToParserNode.toParserNode(arg.type, declaration);

        if (!methodArgs || i >= methodArgs->args.size()) {
            if (auto e = ctx.beginIndexerError(fullTypeLoc, core::errors::Rewriter::RBSParameterMismatch)) {
                e.setHeader("RBS signature has more parameters than in the method definition");
            }
            return nullptr;
        }

        auto methodArg = methodArgs->args[i].get();
        (void)methodArg; // Suppress unused warning

        // TODO: Implement parameter kind validation
        // For now, skip validation to get basic functionality working
        // if (!checkParameterKindMatch(arg, methodArg)) {
        //     validation code...
        // }

        if (auto nameSymbol = arg.name) {
            // The RBS arg is named in the signature, so we use the explicit name used
            auto nameStr = parser.resolveConstant(nameSymbol);
            auto name = ctx.state.enterNameUTF8(nameStr);
            sigParams.emplace_back(
                make_unique<parser::Pair>(arg.loc, parser::MK::Symbol(arg.nameLoc, name), move(type)));
        } else {
            // The RBS arg is not named in the signature, so we get it from the method definition
            // TODO: Implement nodeName extraction from methodArg
            // For now, use a generic name
            auto name = ctx.state.enterNameUTF8("arg");
            sigParams.emplace_back(make_unique<parser::Pair>(arg.loc, parser::MK::Symbol(arg.loc, name), move(type)));
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

        auto typeParamsArg = parser::MK::Array(fullTypeLoc, move(typeParamsVector));
        sigBuilder = parser::MK::Send1(fullTypeLoc, move(sigBuilder), core::Names::typeParameters(), fullTypeLoc,
                                       move(typeParamsArg));
    }

    // Build the signature following the same pattern as the original
    if (sigParams.size() > 0) {
        auto hash = parser::MK::Hash(fullTypeLoc, true, move(sigParams));
        auto args = parser::NodeVec();
        args.emplace_back(move(hash));
        sigBuilder = parser::MK::Send(fullTypeLoc, move(sigBuilder), core::Names::params(), fullTypeLoc, move(args));
    }

    // Handle return type
    auto returnType = typeToParserNode.toParserNode(functionType->return_type, declaration);
    if (functionType->return_type->type == RBS_TYPES_BASES_VOID) {
        auto loc = declaration.typeLocFromRange(functionType->return_type->location->rg);
        sigBuilder = parser::MK::Send0(fullTypeLoc, move(sigBuilder), core::Names::void_(), loc);
    } else {
        auto nameLoc = declaration.typeLocFromRange(functionType->return_type->location->rg);
        sigBuilder =
            parser::MK::Send1(fullTypeLoc, move(sigBuilder), core::Names::returns(), nameLoc, move(returnType));
    }

    // Build the final signature call
    auto sigArgs = parser::NodeVec();
    sigArgs.emplace_back(parser::MK::T_Sig_WithoutRuntime(firstLineTypeLoc));

    auto sig = parser::MK::Send(fullTypeLoc, parser::MK::SorbetPrivateStatic(fullTypeLoc), core::Names::sig(),
                                firstLineTypeLoc, move(sigArgs));

    return make_unique<parser::Block>(commentLoc, move(sig), nullptr, move(sigBuilder));
}

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
pm_node_t *MethodTypeToParserNodePrism::createPrismMethodSignature(const pm_node_t *methodDef,
                                                                   const rbs_method_type_t *methodType,
                                                                   const RBSDeclaration &declaration,
                                                                   const std::vector<Comment> &annotations) {
    if (!prismParser) {
        return nullptr; // Need Prism parser for node creation
    }

    // For now, create a placeholder sig call
    // TODO: Parse the RBS method type and create appropriate signature nodes
    // This should eventually use methodType to build the proper signature structure
    (void)methodDef; // Suppress unused warnings for now
    (void)methodType;
    (void)declaration;
    (void)annotations;

    return createSigCallPlaceholder();
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

pm_node_t *MethodTypeToParserNodePrism::createSigCallPlaceholder() {
    if (!prismParser)
        return nullptr;

    pm_location_t loc = getZeroWidthLocation();

    // Add method names to constant pool
    pm_constant_id_t sig_method_id = addConstantToPool("sig");
    pm_constant_id_t returns_id = addConstantToPool("returns");

    if (sig_method_id == PM_CONSTANT_ID_UNSET || returns_id == PM_CONSTANT_ID_UNSET) {
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

    // Create block body: returns(String)
    pm_node_t *string_const = createStringConstant();
    if (!string_const)
        return nullptr;

    // Create arguments for returns call
    pm_node_t *returns_arguments = createSingleArgumentNode(string_const);
    if (!returns_arguments)
        return nullptr;

    // Create returns(String) call
    pm_call_node_t *returns_call = allocateNode<pm_call_node_t>();
    if (!returns_call)
        return nullptr;

    *returns_call = (pm_call_node_t){.base = initializeBaseNode(PM_CALL_NODE),
                                     .receiver = nullptr, // No explicit receiver (implicit self)
                                     .call_operator_loc = {.start = nullptr, .end = nullptr},
                                     .name = returns_id,
                                     .message_loc = loc,
                                     .opening_loc = loc,
                                     .arguments = down_cast<pm_arguments_node_t>(returns_arguments),
                                     .closing_loc = loc,
                                     .block = nullptr};

    // Create block node
    pm_block_node_t *block = allocateNode<pm_block_node_t>();
    if (!block)
        return nullptr;

    *block = (pm_block_node_t){.base = initializeBaseNode(PM_BLOCK_NODE),
                               .locals = {.size = 0, .capacity = 0, .ids = nullptr},
                               .parameters = nullptr,
                               .body = up_cast(returns_call),
                               .opening_loc = loc,
                               .closing_loc = loc};

    // Create the main sig call: Sorbet::Private::Static.sig(T::Sig::WithoutRuntime) { returns(String) }
    pm_call_node_t *call = allocateNode<pm_call_node_t>();
    if (!call)
        return nullptr;

    *call = (pm_call_node_t){
        .base = initializeBaseNode(PM_CALL_NODE),
        .receiver = receiver, // Sorbet::Private::Static constant path
        .call_operator_loc = loc,
        .name = sig_method_id, // "sig" method
        .message_loc = loc,
        .opening_loc = loc,
        .arguments = down_cast<pm_arguments_node_t>(arguments), // T::Sig::WithoutRuntime argument
        .closing_loc = loc,
        .block = up_cast(block) // { returns(String) } block
    };

    return up_cast(call);
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
