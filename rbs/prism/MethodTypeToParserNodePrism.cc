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
bool isSelfOrKernelPrism(pm_node_t *node) {
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

bool isRaisePrism(pm_node_t *node) {
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
core::AutocorrectSuggestion autocorrectAbstractBodyPrism(core::MutableContext ctx, pm_node_t *method,
                                                        core::LocOffsets method_declLoc, pm_node_t *method_body) {
    (void)ctx;
    (void)method;
    (void)method_declLoc;
    (void)method_body;

    // TODO: Implement autocorrect for Prism nodes
    return core::AutocorrectSuggestion{"TODO: Implement autocorrect for Prism", {}};
}
*/

void ensureAbstractMethodRaisesPrism(core::MutableContext ctx, const pm_node_t *node) {
    if (PM_NODE_TYPE_P(node, PM_DEF_NODE)) {
        auto *def = down_cast<pm_def_node_t>(const_cast<pm_node_t*>(node));
        if (def->body && isRaisePrism(def->body)) {
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

unique_ptr<parser::Node> handleAnnotationsPrism(core::MutableContext ctx, const pm_node_t *node,
                                                unique_ptr<parser::Node> sigBuilder,
                                                const vector<Comment> &annotations) {
    for (auto &annotation : annotations) {
        if (annotation.string == "final") {
            // no-op, `final` is handled in the `sig()` call later
        } else if (annotation.string == "abstract") {
            sigBuilder =
                parser::MK::Send0(annotation.typeLoc, move(sigBuilder), core::Names::abstract(), annotation.typeLoc);
            ensureAbstractMethodRaisesPrism(ctx, node);
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
parser::Args *getMethodArgsPrism(const pm_node_t *node) {
    if (PM_NODE_TYPE_P(node, PM_DEF_NODE)) {
        auto *def = down_cast<pm_def_node_t>(const_cast<pm_node_t*>(node));
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
        auto nameLoc = adjustNameLoc(declaration, list_node->node);

        ENFORCE(list_node->node->type == RBS_TYPES_FUNCTION_PARAM,
                "Unexpected node type `{}` in function parameter, expected `{}`", rbs_node_type_name(list_node->node),
                "FunctionParam");

        auto *param = (rbs_types_function_param_t *)list_node->node;
        auto arg = RBSArg{loc, nameLoc, param->name, param->type, kind};
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

    // Collect arguments (simplified - just positional for now)
    vector<RBSArg> args;
    collectArgs(declaration, functionType->required_positionals, args, RBSArg::Kind::Positional);

    // Build basic signature parameters
    auto sigParams = parser::NodeVec();
    auto typeToParserNode = TypeToParserNode(ctx, typeParams, parser);

    for (auto &arg : args) {
        auto type = typeToParserNode.toParserNode(arg.type, declaration);
        if (auto nameSymbol = arg.name) {
            auto nameStr = parser.resolveConstant(nameSymbol);
            auto name = ctx.state.enterNameUTF8(nameStr);
            sigParams.emplace_back(
                make_unique<parser::Pair>(arg.loc, parser::MK::Symbol(arg.nameLoc, name), move(type)));
        } else {
            // For now, use a default name since we don't have method args extraction
            auto name = ctx.state.enterNameUTF8("arg");
            sigParams.emplace_back(make_unique<parser::Pair>(arg.loc, parser::MK::Symbol(arg.loc, name), move(type)));
        }
    }

    // Build the sig
    auto sigBuilder = parser::MK::Self(fullTypeLoc);
    sigBuilder = handleAnnotationsPrism(ctx, methodDef, move(sigBuilder), annotations);

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

} // namespace sorbet::rbs
