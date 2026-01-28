#include "rbs/prism/AssertionsRewriterPrism.h"

#include "absl/strings/match.h"
#include "core/errors/rewriter.h"
#include "rbs/prism/SignatureTranslatorPrism.h"
#include <cctype>
#include <regex>

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {

const regex notNilPattern("^\\s*!nil\\s*(#.*)?$");
const regex untypedPattern("^\\s*untyped\\s*(#.*)?$");
const regex absurdPattern("^\\s*absurd\\s*(#.*)?$");

/*
 * Parse the comment and return the type as a `pm_node_t*` and the kind of assertion we need to apply (let, cast,
 * must, unsafe).
 *
 * The `typeParams` param is used to resolve the type parameters from the method signature.
 *
 * Returns `nullopt` if the comment is not a valid RBS expression (an error is produced).
 */
optional<pair<pm_node_t *, InlineCommentPrism::Kind>>
parseComment(core::MutableContext ctx, parser::Prism::Parser &parser, InlineCommentPrism comment,
             absl::Span<pair<core::LocOffsets, core::NameRef>> typeParams) {
    Factory prism{parser};

    if (comment.kind == InlineCommentPrism::Kind::MUST || comment.kind == InlineCommentPrism::Kind::UNSAFE ||
        comment.kind == InlineCommentPrism::Kind::ABSURD) {
        // The type should never be used but we need to hold the location...
        return pair{prism.Nil(comment.comment.typeLoc), comment.kind};
    }

    auto type = rbs::SignatureTranslatorPrism(ctx, parser)
                    .translateAssertionType(absl::MakeSpan(typeParams), RBSDeclaration({comment.comment}));

    if (type == nullptr) {
        // We couldn't parse the type, we produced an error, we don't return anything
        return nullopt;
    }

    return pair{type, comment.kind};
}

/**
 * Parse the type parameters from the previous statement
 *
 * Given a case like this one:
 *
 *     #: [X] (X) -> void
 *     def foo(x)
 *       y = nil #: X?
 *     end
 *
 * We need to be aware of the type parameter `X` so we can use it to resolve the type of `y`.
 */
vector<pair<core::LocOffsets, core::NameRef>>
extractTypeParamsPrism(core::MutableContext ctx, const parser::Prism::Parser &parser, pm_node_t *block) {
    vector<pair<core::LocOffsets, core::NameRef>> typeParams;

    if (!block || !PM_NODE_TYPE_P(block, PM_BLOCK_NODE)) {
        return typeParams;
    }

    auto *blockNode = down_cast<pm_block_node_t>(block);
    if (!blockNode->body) {
        return typeParams;
    }

    // The body can be either a StatementsNode or directly a CallNode (from SigsRewriter)
    pm_node_t *node = blockNode->body;

    // If it's a statements node, get the first statement
    if (PM_NODE_TYPE_P(node, PM_STATEMENTS_NODE)) {
        auto *statements = down_cast<pm_statements_node_t>(node);
        ENFORCE(statements->body.size > 0);
        node = statements->body.nodes[0];
    }

    pm_call_node_t *call = nullptr;

    // Walk through the call chain to find type_parameters()
    while (node && PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
        auto *callNode = down_cast<pm_call_node_t>(node);
        auto methodName = parser.resolveConstant(callNode->name);

        if (methodName == "type_parameters") {
            call = callNode;
            break;
        }

        node = callNode->receiver;
    }

    if (call == nullptr) {
        return typeParams;
    }

    // Collect the type parameters from the arguments
    if (auto *args = call->arguments) {
        for (size_t i = 0; i < args->arguments.size; i++) {
            pm_node_t *arg = args->arguments.nodes[i];
            if (!PM_NODE_TYPE_P(arg, PM_SYMBOL_NODE)) {
                continue;
            }

            auto *sym = down_cast<pm_symbol_node_t>(arg);
            auto symbolName = parser.extractString(&sym->unescaped);
            auto nameRef = ctx.state.enterNameUTF8(symbolName);
            auto loc = parser.translateLocation(arg->location);
            typeParams.emplace_back(loc, nameRef);
        }
    }

    return typeParams;
}

/**
 * Check if two Prism nodes refer to the same constant.
 *
 * Recursively compares constant names and their scopes.
 * Handles both PM_CONSTANT_READ_NODE (e.g., `G2`) and PM_CONSTANT_PATH_NODE (e.g., `T::Array`).
 */
bool sameConstant(parser::Prism::Parser &parser, pm_node_t *a, pm_node_t *b) {
    if (a == nullptr || b == nullptr) {
        return false;
    }

    pm_node_type aType = PM_NODE_TYPE(a);
    pm_node_type bType = PM_NODE_TYPE(b);

    // Case 1: Both are PM_CONSTANT_READ_NODE (simple constants like `G2`)
    if (aType == PM_CONSTANT_READ_NODE && bType == PM_CONSTANT_READ_NODE) {
        auto *aConst = down_cast<pm_constant_read_node_t>(a);
        auto *bConst = down_cast<pm_constant_read_node_t>(b);
        return aConst->name == bConst->name;
    }

    // Case 2: Both are PM_CONSTANT_PATH_NODE (scoped constants like `T::Array`)
    if (aType == PM_CONSTANT_PATH_NODE && bType == PM_CONSTANT_PATH_NODE) {
        auto *aPath = down_cast<pm_constant_path_node_t>(a);
        auto *bPath = down_cast<pm_constant_path_node_t>(b);

        if (aPath->name != bPath->name) {
            return false;
        }

        // If both parents are null, we've matched the entire path
        if (aPath->parent == nullptr && bPath->parent == nullptr) {
            return true;
        }

        return sameConstant(parser, aPath->parent, bPath->parent);
    }

    return false;
}

/**
 * Deep copy a Prism node that appears in generic type instantiations.
 *
 * This handles only the node types that appear in generic type parameters like `G1[Integer]`.
 * Prism nodes don't have built-in copy functions, but we need to duplicate nodes when
 * they're used in multiple places in the AST (e.g., as both the receiver of .new() and
 * the type argument to T.let()).
 *
 * Only supports node types that actually appear in generic type expressions.
 * Will ENFORCE if an unsupported node type is encountered.
 */
pm_node_t *deepCopyGenericTypeNode(parser::Prism::Parser &parser, pm_node_t *node) {
    if (node == nullptr) {
        return nullptr;
    }

    Factory prism{parser};

    switch (PM_NODE_TYPE(node)) {
        // Examples:
        //   - G1[Integer] -> <syntheticSquareBrackets> call
        //   - G2[T.any(Integer, String)] -> T.any() call
        //   - G3[T.type_parameter(:X)] -> T.type_parameter() call
        case PM_CALL_NODE: {
            auto *original = down_cast<pm_call_node_t>(node);
            auto *copy = prism.allocateNode<pm_call_node_t>();

            *copy = (pm_call_node_t){.base = original->base,
                                     .receiver = deepCopyGenericTypeNode(parser, original->receiver),
                                     .call_operator_loc = original->call_operator_loc,
                                     .name = original->name,
                                     .message_loc = original->message_loc,
                                     .opening_loc = original->opening_loc,
                                     .arguments = down_cast<pm_arguments_node_t>(
                                         deepCopyGenericTypeNode(parser, up_cast(original->arguments))),
                                     .closing_loc = original->closing_loc,
                                     .block = deepCopyGenericTypeNode(parser, original->block)};

            return up_cast(copy);
        }

        case PM_ARGUMENTS_NODE: {
            auto *original = down_cast<pm_arguments_node_t>(node);
            vector<pm_node_t *> copiedArgs;
            copiedArgs.reserve(original->arguments.size);
            for (size_t i = 0; i < original->arguments.size; i++) {
                copiedArgs.push_back(deepCopyGenericTypeNode(parser, original->arguments.nodes[i]));
            }
            return up_cast(prism.createArgumentsNode(absl::MakeSpan(copiedArgs), original->base.location));
        }

        // Example: G1[Integer] -> Integer
        case PM_CONSTANT_READ_NODE: {
            auto *original = down_cast<pm_constant_read_node_t>(node);
            return prism.ConstantReadNode(original->name, original->base.location);
        }

        // Examples:
        //   - Array[String] -> T::Array
        //   - G1[Foo::Bar] -> Foo::Bar
        case PM_CONSTANT_PATH_NODE: {
            auto *original = down_cast<pm_constant_path_node_t>(node);
            return prism.ConstantPathNode(original->base.location, deepCopyGenericTypeNode(parser, original->parent),
                                          original->name);
        }

        // Examples:
        //   - G1[X] where X is a type parameter -> T.type_parameter(:X)
        //   - The :X symbol needs to be copied with proper string allocation
        case PM_SYMBOL_NODE: {
            auto *original = down_cast<pm_symbol_node_t>(node);
            auto symbolStr = parser.extractString(&original->unescaped);
            auto loc = parser.translateLocation(original->base.location);
            return prism.Symbol(loc, symbolStr);
        }

        default:
            unreachable("deepCopyGenericTypeNode: unhandled node type {} - this node type appears in generic type "
                        "instantiations but is not yet supported",
                        PM_NODE_TYPE(node));
    }
}

/**
 * If `node` is a `.new` call and `type` is a generic type instantiation of the same class,
 * replace the receiver of `.new` with the generic type.
 *
 * For example:
 *
 *     G1.new #: G1[Integer]
 *
 * becomes:
 *
 *     T.let(G1[Integer].new, G1[Integer])
 *
 * This approach avoids calling `G1[Integer]`, which would have triggered
 * other type-checking errors about `G1::[]` not being defined
 * (since `G1` could be typed purely in RBS, and not `extend T::Generic`).
 * However, it requires that we clone the `G1` path, so we can use it twice.
 */
void maybeSupplyGenericTypeArguments(core::MutableContext ctx, parser::Prism::Parser &parser, pm_node_t **node,
                                     pm_node_t **type) {
    // We only rewrite `.new` calls
    if (!PM_NODE_TYPE_P(*node, PM_CALL_NODE)) {
        return;
    }

    auto *newCall = down_cast<pm_call_node_t>(*node);
    auto methodName = parser.resolveConstant(newCall->name);

    if (methodName != "new"sv) {
        return;
    }

    // We only rewrite when casted to a generic type (syntheticSquareBrackets)
    if (!PM_NODE_TYPE_P(*type, PM_CALL_NODE)) {
        return;
    }

    auto *bracketCall = down_cast<pm_call_node_t>(*type);
    auto bracketMethodName = parser.resolveConstant(bracketCall->name);

    if (bracketMethodName != "<syntheticSquareBrackets>"sv) {
        return;
    }

    // We only rewrite when the generic type is the same as the instantiated one
    if (!sameConstant(parser, newCall->receiver, bracketCall->receiver)) {
        return;
    }

    // We need to create a deep copy of the type node because it will be used in two places:
    // 1. As the receiver of .new()
    // 2. As the type argument to T.let()
    newCall->receiver = deepCopyGenericTypeNode(parser, *type);
}

} // namespace

/**
 * Save the type parameters from a `sig` block so they can be used to resolve type variables in method body assertions.
 *
 * The extracted type parameters remain active until we exit the method scope (when `typeParams` is cleared in the
 * DEF_NODE case). This allows inline RBS comments like `#: X?` to reference method type parameters like `X`.
 *
 * Returns `true` if the node is a `sig` call (so caller can skip further processing), `false` otherwise.
 */
bool AssertionsRewriterPrism::saveMethodTypeParams(pm_node_t *call) {
    if (!call || !PM_NODE_TYPE_P(call, PM_CALL_NODE)) {
        return false;
    }

    auto *callNode = down_cast<pm_call_node_t>(call);

    // Check if this is a sig() call
    auto methodName = parser.resolveConstant(callNode->name);
    if (methodName != "sig") {
        return false;
    }

    // Check if it has a block
    if (!callNode->block) {
        return false;
    }

    this->typeParams = extractTypeParamsPrism(ctx, parser, callNode->block);

    return true;
}

/**
 * Mark the given comment location as "consumed" so it won't be picked up by subsequent calls to `commentForNode`.
 */
void AssertionsRewriterPrism::consumeComment(core::LocOffsets loc) {
    consumedComments.emplace(make_pair(loc.beginPos(), loc.endPos()));
}

/**
 * Check if the given comment location has been consumed.
 */
bool AssertionsRewriterPrism::hasConsumedComment(core::LocOffsets loc) {
    return consumedComments.count(make_pair(loc.beginPos(), loc.endPos()));
}

/**
 * Helper to convert Prism location to core::LocOffsets.
 */
core::LocOffsets AssertionsRewriterPrism::translateLocation(pm_location_t location) {
    const uint8_t *sourceStart = (const uint8_t *)ctx.file.data(ctx).source().data();
    uint32_t start = static_cast<uint32_t>(location.start - sourceStart);
    uint32_t end = static_cast<uint32_t>(location.end - sourceStart);
    return core::LocOffsets{start, end};
}

/**
 * Get the RBS comment for the given Prism node.
 *
 * Returns `nullopt` if no comment is found or if the comment was already consumed.
 */
optional<rbs::InlineCommentPrism> AssertionsRewriterPrism::commentForNode(pm_node_t *node) {
    if (node == nullptr) {
        return nullopt;
    }

    auto it = commentsByNode.find(node);
    if (it == commentsByNode.end()) {
        return nullopt;
    }

    for (const auto &commentNode : it->second) {
        if (!absl::StartsWith(commentNode.string, CommentsAssociatorPrism::RBS_PREFIX)) {
            continue;
        }

        auto contentStart = commentNode.loc.beginPos() + 2; // +2 for the #: prefix
        auto content = commentNode.string.substr(2);        // skip the #: prefix

        // Skip whitespace after the #:
        while (contentStart < commentNode.loc.endPos() && isspace(content[0])) {
            contentStart++;
            content = content.substr(1);
        }

        auto kind = InlineCommentPrism::Kind::LET;
        if (absl::StartsWith(content, "as ")) {
            kind = InlineCommentPrism::Kind::CAST;
            contentStart += 3;
            content = content.substr(3);

            if (regex_match(content.begin(), content.end(), notNilPattern)) {
                kind = InlineCommentPrism::Kind::MUST;
            } else if (regex_match(content.begin(), content.end(), untypedPattern)) {
                kind = InlineCommentPrism::Kind::UNSAFE;
            }
        } else if (regex_match(content.begin(), content.end(), absurdPattern)) {
            kind = InlineCommentPrism::Kind::ABSURD;
        } else if (absl::StartsWith(content, "self as ")) {
            kind = InlineCommentPrism::Kind::BIND;
            contentStart += 8;
            content = content.substr(8);
        }

        if (hasConsumedComment(commentNode.loc)) {
            continue;
        }
        consumeComment(commentNode.loc);

        return InlineCommentPrism{
            rbs::Comment{
                commentNode.loc,
                core::LocOffsets{contentStart, commentNode.loc.endPos()},
                content,
            },
            kind,
        };
    }

    return nullopt;
}

/**
 * Replace the given node with a cast node.
 *
 * The kind of cast depends on the annotation kind:
 *
 * - `x #: X`: `T.let(x, X)`
 * - `x #: as X`: `T.cast(x, X)`
 * - `x #: as !nil`: `T.must(x)`
 * - `x #: as untyped`: `T.unsafe(x)`
 */
pm_node_t *AssertionsRewriterPrism::insertCast(pm_node_t *node,
                                               optional<pair<pm_node_t *, InlineCommentPrism::Kind>> pair) {
    if (!pair) {
        return node;
    }

    auto [type, kind] = *pair;

    maybeSupplyGenericTypeArguments(ctx, parser, &node, &type);

    // Use the type node's location for all assertion constructs.
    // The type node's location points to where the type appears in the comment (e.g., "String"
    // in "#: as String"). This means the resulting T.cast/T.let/etc call will have the location
    // of the type, not the expression being wrapped.
    auto typeLoc = translateLocation(type->location);

    switch (kind) {
        case InlineCommentPrism::Kind::LET:
            return prism.TLet(typeLoc, node, type);
        case InlineCommentPrism::Kind::CAST:
            return prism.TCast(typeLoc, node, type);
        case InlineCommentPrism::Kind::MUST:
            return prism.TMust(typeLoc, node);
        case InlineCommentPrism::Kind::UNSAFE:
            return prism.TUnsafe(typeLoc, node);
        case InlineCommentPrism::Kind::ABSURD:
            return prism.TAbsurd(typeLoc, node);
        case InlineCommentPrism::Kind::BIND:
            if (auto e = ctx.beginIndexerError(typeLoc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("`{}` binding can't be used as a trailing comment", "self");
            }
            return node;
    }
    unreachable("Unknown assertion kind");
}

/**
 * Replace the synthetic RBS placeholder node with a `T.bind(self, Type)` call.
 */
pm_node_t *AssertionsRewriterPrism::replaceSyntheticBind(pm_node_t *node) {
    auto inlineComment = commentForNode(node);
    ENFORCE(inlineComment, "No inline comment found for synthetic bind");

    auto pair = parseComment(ctx, parser, inlineComment.value(), absl::MakeSpan(typeParams));

    if (!pair) {
        // We already raised an error while parsing the comment, so we just bind to `T.untyped`
        auto loc = translateLocation(node->location);
        return prism.TBindSelf(loc, prism.TUntyped(loc));
    }

    auto [type, kind] = *pair;
    ENFORCE(kind == InlineCommentPrism::Kind::BIND, "Invalid inline comment for synthetic bind");

    auto typeLoc = translateLocation(type->location);

    return prism.TBindSelf(typeLoc, type);
}

/**
 * Insert a cast into the given Prism node if there is an not yet consumed RBS assertion comment.
 */
pm_node_t *AssertionsRewriterPrism::maybeInsertCast(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    if (auto inlineComment = commentForNode(node)) {
        if (auto type = parseComment(ctx, parser, inlineComment.value(), absl::MakeSpan(typeParams))) {
            return insertCast(node, type);
        }
    }

    return node;
}

/**
 * Rewrite a collection of Prism nodes in place.
 */
void AssertionsRewriterPrism::rewriteNodes(pm_node_list_t &nodes) {
    for (size_t i = 0; i < nodes.size; i++) {
        nodes.nodes[i] = rewriteNode(nodes.nodes[i]);
    }
}

void AssertionsRewriterPrism::rewriteArgumentsNode(pm_arguments_node_t *args) {
    if (args) {
        rewriteNodes(args->arguments);
    }
}

/**
 * Rewrite a collection of nodes, wrap them in an array and cast the array.
 */
void AssertionsRewriterPrism::rewriteNodesAsArray(pm_node_t *node, pm_node_list_t &nodes) {
    if (auto inlineComment = commentForNode(node)) {
        if (nodes.size > 1) {
            auto nodeSpan = absl::MakeSpan(nodes.nodes, nodes.size);
            // Get location spanning from first to last node
            auto loc = translateLocation(nodeSpan.front()->location).join(translateLocation(nodeSpan.back()->location));

            auto arr = prism.Array(loc, nodeSpan);
            arr = rewriteNode(arr);

            // Replace nodes list with single array element
            prism.free(nodes.nodes);

            pm_node_t **buffer = prism.calloc<pm_node_t *>(1);
            buffer[0] = arr;
            nodes = (pm_node_list){.size = 1, .capacity = 1, .nodes = buffer};
        }

        if (auto type = parseComment(ctx, parser, inlineComment.value(), absl::MakeSpan(typeParams))) {
            nodes.nodes[0] = insertCast(nodes.nodes[0], type);
        }
    }

    rewriteNodes(nodes);
}

/**
 * Rewrite the body of a node (class, module, method, etc).
 *
 * This function handles ANY node type that can be a body:
 * 1. StatementsNode - iterates and rewrites each statement in its body array
 * 2. BeginNode, or any other node type - delegates to rewriteNode which handles all node types
 * 3. nullptr - returns nullptr
 *
 * The key insight: StatementsNode is the only node type with a flat array of statements
 * that we can iterate directly. All other compound nodes (BeginNode, IfNode, etc.) have
 * their own specific structure handled by rewriteNode.
 *
 * The returned node should be assigned back to the body field of the parent node.
 * For example: def->body = rewriteBody(def->body)
 */
pm_node_t *AssertionsRewriterPrism::rewriteBody(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    // StatementsNode is special: it's a flat array of statements that we iterate directly
    // This is the most common case for method/class/module bodies
    if (PM_NODE_TYPE_P(node, PM_STATEMENTS_NODE)) {
        auto *statements = down_cast<pm_statements_node_t>(node);

        // Iterate over each statement and rewrite it in place
        for (size_t i = 0; i < statements->body.size; i++) {
            statements->body.nodes[i] = rewriteNode(statements->body.nodes[i]);
        }

        return node;
    }

    // For any other node type (BeginNode, single expressions, etc), delegate to rewriteNode
    // rewriteNode has a comprehensive switch statement that handles all node types properly
    return rewriteNode(node);
}

pm_statements_node_t *AssertionsRewriterPrism::rewriteBody(pm_statements_node_t *stmts) {
    return down_cast<pm_statements_node_t>(rewriteBody(up_cast(stmts)));
}

/**
 * Rewrite a node.
 */
pm_node_t *AssertionsRewriterPrism::rewriteNode(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    // If all comments have been consumed, we can skip the rest of the tree.
    if (consumedComments.size() >= totalComments) {
        return node;
    }

    switch (PM_NODE_TYPE(node)) {
        // Scopes
        case PM_MODULE_NODE: {
            auto *module = down_cast<pm_module_node_t>(node);
            module->body = rewriteBody(module->body);
            typeParams.clear();
            return node;
        }
        case PM_CLASS_NODE: {
            auto *klass = down_cast<pm_class_node_t>(node);
            klass->body = rewriteBody(klass->body);
            typeParams.clear();
            return node;
        }
        case PM_SINGLETON_CLASS_NODE: {
            auto *sclass = down_cast<pm_singleton_class_node_t>(node);
            sclass->body = rewriteBody(sclass->body);
            typeParams.clear();
            return node;
        }
        case PM_DEF_NODE: {
            auto *def = down_cast<pm_def_node_t>(node);
            def->body = rewriteBody(def->body);
            typeParams.clear();
            return node;
        }

        // Begin statements
        case PM_BEGIN_NODE: {
            auto *begin = down_cast<pm_begin_node_t>(node);
            node = maybeInsertCast(node);
            if (begin->statements) {
                begin->statements = rewriteBody(begin->statements);
            }
            if (begin->rescue_clause) {
                begin->rescue_clause = down_cast<pm_rescue_node_t>(rewriteNode(up_cast(begin->rescue_clause)));
            }
            if (begin->else_clause) {
                begin->else_clause = down_cast<pm_else_node_t>(rewriteNode(up_cast(begin->else_clause)));
            }
            if (begin->ensure_clause) {
                begin->ensure_clause = down_cast<pm_ensure_node_t>(rewriteNode(up_cast(begin->ensure_clause)));
            }
            return node;
        }

        // Simple write assignments
        case PM_LOCAL_VARIABLE_WRITE_NODE: {
            auto *write = down_cast<pm_local_variable_write_node_t>(node);
            write->value = rewriteNode(write->value);
            return node;
        }
        case PM_INSTANCE_VARIABLE_WRITE_NODE: {
            auto *write = down_cast<pm_instance_variable_write_node_t>(node);
            write->value = rewriteNode(write->value);
            return node;
        }
        case PM_CLASS_VARIABLE_WRITE_NODE: {
            auto *write = down_cast<pm_class_variable_write_node_t>(node);
            write->value = rewriteNode(write->value);
            return node;
        }
        case PM_GLOBAL_VARIABLE_WRITE_NODE: {
            auto *write = down_cast<pm_global_variable_write_node_t>(node);
            write->value = rewriteNode(write->value);
            return node;
        }
        case PM_CONSTANT_WRITE_NODE: {
            auto *write = down_cast<pm_constant_write_node_t>(node);
            write->value = rewriteNode(write->value);
            return node;
        }
        case PM_CONSTANT_PATH_WRITE_NODE: {
            auto *write = down_cast<pm_constant_path_write_node_t>(node);
            write->target = down_cast<pm_constant_path_node_t>(rewriteNode(up_cast(write->target)));
            write->value = rewriteNode(write->value);
            return node;
        }

        // And assignments
        case PM_LOCAL_VARIABLE_AND_WRITE_NODE: {
            auto *andWrite = down_cast<pm_local_variable_and_write_node_t>(node);
            andWrite->value = rewriteNode(maybeInsertCast(andWrite->value));
            return node;
        }
        case PM_INSTANCE_VARIABLE_AND_WRITE_NODE: {
            auto *andWrite = down_cast<pm_instance_variable_and_write_node_t>(node);
            andWrite->value = rewriteNode(maybeInsertCast(andWrite->value));
            return node;
        }
        case PM_CLASS_VARIABLE_AND_WRITE_NODE: {
            auto *andWrite = down_cast<pm_class_variable_and_write_node_t>(node);
            andWrite->value = rewriteNode(maybeInsertCast(andWrite->value));
            return node;
        }
        case PM_GLOBAL_VARIABLE_AND_WRITE_NODE: {
            auto *andWrite = down_cast<pm_global_variable_and_write_node_t>(node);
            andWrite->value = rewriteNode(maybeInsertCast(andWrite->value));
            return node;
        }
        case PM_CONSTANT_AND_WRITE_NODE: {
            auto *andWrite = down_cast<pm_constant_and_write_node_t>(node);
            andWrite->value = rewriteNode(maybeInsertCast(andWrite->value));
            return node;
        }
        case PM_CONSTANT_PATH_AND_WRITE_NODE: {
            auto *andWrite = down_cast<pm_constant_path_and_write_node_t>(node);
            andWrite->target = down_cast<pm_constant_path_node_t>(rewriteNode(up_cast(andWrite->target)));
            andWrite->value = rewriteNode(maybeInsertCast(andWrite->value));
            return node;
        }
        case PM_CALL_AND_WRITE_NODE: {
            auto *andWrite = down_cast<pm_call_and_write_node_t>(node);
            andWrite->receiver = rewriteNode(andWrite->receiver);
            andWrite->value = rewriteNode(maybeInsertCast(andWrite->value));
            return node;
        }
        case PM_INDEX_AND_WRITE_NODE: {
            auto *andWrite = down_cast<pm_index_and_write_node_t>(node);
            andWrite->receiver = rewriteNode(andWrite->receiver);
            rewriteArgumentsNode(andWrite->arguments);
            if (andWrite->block != nullptr) {
                andWrite->block = down_cast<pm_block_argument_node_t>(rewriteNode(up_cast(andWrite->block)));
            }
            andWrite->value = maybeInsertCast(andWrite->value);
            andWrite->value = rewriteNode(andWrite->value);
            return node;
        }

        // Operator assignments
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE: {
            auto *opWrite = down_cast<pm_local_variable_operator_write_node_t>(node);
            opWrite->value = rewriteNode(maybeInsertCast(opWrite->value));
            return node;
        }
        case PM_INSTANCE_VARIABLE_OPERATOR_WRITE_NODE: {
            auto *opWrite = down_cast<pm_instance_variable_operator_write_node_t>(node);
            opWrite->value = rewriteNode(maybeInsertCast(opWrite->value));
            return node;
        }
        case PM_CLASS_VARIABLE_OPERATOR_WRITE_NODE: {
            auto *opWrite = down_cast<pm_class_variable_operator_write_node_t>(node);
            opWrite->value = rewriteNode(maybeInsertCast(opWrite->value));
            return node;
        }
        case PM_GLOBAL_VARIABLE_OPERATOR_WRITE_NODE: {
            auto *opWrite = down_cast<pm_global_variable_operator_write_node_t>(node);
            opWrite->value = rewriteNode(maybeInsertCast(opWrite->value));
            return node;
        }
        case PM_CONSTANT_OPERATOR_WRITE_NODE: {
            auto *opWrite = down_cast<pm_constant_operator_write_node_t>(node);
            opWrite->value = rewriteNode(maybeInsertCast(opWrite->value));
            return node;
        }
        case PM_CONSTANT_PATH_OPERATOR_WRITE_NODE: {
            auto *opWrite = down_cast<pm_constant_path_operator_write_node_t>(node);
            opWrite->target = down_cast<pm_constant_path_node_t>(rewriteNode(up_cast(opWrite->target)));
            opWrite->value = rewriteNode(maybeInsertCast(opWrite->value));
            return node;
        }
        case PM_CALL_OPERATOR_WRITE_NODE: {
            auto *opWrite = down_cast<pm_call_operator_write_node_t>(node);
            opWrite->receiver = rewriteNode(opWrite->receiver);
            opWrite->value = rewriteNode(maybeInsertCast(opWrite->value));
            return node;
        }
        case PM_INDEX_OPERATOR_WRITE_NODE: {
            auto *opWrite = down_cast<pm_index_operator_write_node_t>(node);
            opWrite->receiver = rewriteNode(opWrite->receiver);
            rewriteArgumentsNode(opWrite->arguments);
            if (opWrite->block != nullptr) {
                opWrite->block = down_cast<pm_block_argument_node_t>(rewriteNode(up_cast(opWrite->block)));
            }
            opWrite->value = maybeInsertCast(opWrite->value);
            opWrite->value = rewriteNode(opWrite->value);
            return node;
        }

        // Or assignments
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE: {
            auto *orWrite = down_cast<pm_local_variable_or_write_node_t>(node);
            orWrite->value = rewriteNode(maybeInsertCast(orWrite->value));
            return node;
        }
        case PM_INSTANCE_VARIABLE_OR_WRITE_NODE: {
            auto *orWrite = down_cast<pm_instance_variable_or_write_node_t>(node);
            orWrite->value = rewriteNode(maybeInsertCast(orWrite->value));
            return node;
        }
        case PM_CLASS_VARIABLE_OR_WRITE_NODE: {
            auto *orWrite = down_cast<pm_class_variable_or_write_node_t>(node);
            orWrite->value = rewriteNode(maybeInsertCast(orWrite->value));
            return node;
        }
        case PM_GLOBAL_VARIABLE_OR_WRITE_NODE: {
            auto *orWrite = down_cast<pm_global_variable_or_write_node_t>(node);
            orWrite->value = rewriteNode(maybeInsertCast(orWrite->value));
            return node;
        }
        case PM_CONSTANT_OR_WRITE_NODE: {
            auto *orWrite = down_cast<pm_constant_or_write_node_t>(node);
            orWrite->value = rewriteNode(maybeInsertCast(orWrite->value));
            return node;
        }
        case PM_CONSTANT_PATH_OR_WRITE_NODE: {
            auto *orWrite = down_cast<pm_constant_path_or_write_node_t>(node);
            orWrite->target = down_cast<pm_constant_path_node_t>(rewriteNode(up_cast(orWrite->target)));
            orWrite->value = rewriteNode(maybeInsertCast(orWrite->value));
            return node;
        }
        case PM_CALL_OR_WRITE_NODE: {
            auto *orWrite = down_cast<pm_call_or_write_node_t>(node);
            orWrite->receiver = rewriteNode(orWrite->receiver);
            orWrite->value = rewriteNode(maybeInsertCast(orWrite->value));
            return node;
        }
        case PM_INDEX_OR_WRITE_NODE: {
            auto *orWrite = down_cast<pm_index_or_write_node_t>(node);
            orWrite->receiver = rewriteNode(orWrite->receiver);
            rewriteArgumentsNode(orWrite->arguments);
            if (orWrite->block != nullptr) {
                orWrite->block = down_cast<pm_block_argument_node_t>(rewriteNode(up_cast(orWrite->block)));
            }
            orWrite->value = maybeInsertCast(orWrite->value);
            orWrite->value = rewriteNode(orWrite->value);
            return node;
        }

        // Multi-assignment
        case PM_MULTI_WRITE_NODE: {
            auto *masgn = down_cast<pm_multi_write_node_t>(node);
            masgn->value = rewriteNode(maybeInsertCast(masgn->value));
            return node;
        }

        // Calls
        case PM_CALL_NODE: {
            auto *call = down_cast<pm_call_node_t>(node);
            if (saveMethodTypeParams(node)) {
                // If this is a `sig` call, we need to save the type parameters so we can use them to resolve the type
                // parameters from the method signature.
                return node;
            }
            call->receiver = rewriteNode(call->receiver);
            node = maybeInsertCast(node);
            rewriteArgumentsNode(call->arguments);
            call->block = rewriteNode(call->block);
            return node;
        }

        // Blocks
        case PM_BLOCK_NODE: {
            auto *block = down_cast<pm_block_node_t>(node);
            node = maybeInsertCast(node);
            block->body = rewriteBody(block->body);
            return node;
        }
        case PM_LAMBDA_NODE: {
            auto *lambda = down_cast<pm_lambda_node_t>(node);
            node = maybeInsertCast(node);
            lambda->body = rewriteBody(lambda->body);
            return node;
        }

        // Loops
        case PM_WHILE_NODE: {
            auto *wl = down_cast<pm_while_node_t>(node);
            // Check if this is a post-while (BEGIN_MODIFIER flag)
            bool isPost = PM_NODE_FLAG_P(wl, PM_LOOP_FLAGS_BEGIN_MODIFIER);
            if (!isPost) {
                node = maybeInsertCast(node);
            }
            wl->predicate = rewriteNode(wl->predicate);
            if (wl->statements) {
                wl->statements = rewriteBody(wl->statements);
            }
            return node;
        }
        case PM_UNTIL_NODE: {
            auto *until = down_cast<pm_until_node_t>(node);
            // Check if this is a post-until (BEGIN_MODIFIER flag)
            bool isPost = PM_NODE_FLAG_P(until, PM_LOOP_FLAGS_BEGIN_MODIFIER);
            if (!isPost) {
                node = maybeInsertCast(node);
            }
            until->predicate = rewriteNode(until->predicate);
            if (until->statements) {
                until->statements = rewriteBody(until->statements);
            }
            return node;
        }
        case PM_FOR_NODE: {
            auto *for_ = down_cast<pm_for_node_t>(node);
            node = maybeInsertCast(node);
            for_->index = rewriteNode(for_->index);
            for_->collection = rewriteNode(for_->collection);
            if (for_->statements) {
                for_->statements = rewriteBody(for_->statements);
            }
            return node;
        }

        // Control flow
        case PM_BREAK_NODE: {
            auto *break_ = down_cast<pm_break_node_t>(node);
            if (auto *args = break_->arguments; !args || args->arguments.size == 0) {
                return node;
            }
            rewriteNodesAsArray(node, break_->arguments->arguments);
            return node;
        }
        case PM_NEXT_NODE: {
            auto *next = down_cast<pm_next_node_t>(node);
            if (auto *args = next->arguments; !args || args->arguments.size == 0) {
                return node;
            }
            rewriteNodesAsArray(node, next->arguments->arguments);
            return node;
        }
        case PM_RETURN_NODE: {
            auto *ret = down_cast<pm_return_node_t>(node);
            if (auto *args = ret->arguments; !args || args->arguments.size == 0) {
                return node;
            }
            rewriteNodesAsArray(node, ret->arguments->arguments);
            return node;
        }

        // Conditionals
        case PM_IF_NODE: {
            auto *if_ = down_cast<pm_if_node_t>(node);
            node = maybeInsertCast(node);
            if_->predicate = rewriteNode(if_->predicate);
            if (if_->statements) {
                if_->statements = rewriteBody(if_->statements);
            }
            if (if_->subsequent) {
                if_->subsequent = rewriteBody(if_->subsequent);
            }
            return node;
        }
        case PM_UNLESS_NODE: {
            auto *unless_ = down_cast<pm_unless_node_t>(node);
            node = maybeInsertCast(node);
            unless_->predicate = rewriteNode(unless_->predicate);
            if (unless_->statements) {
                unless_->statements = rewriteBody(unless_->statements);
            }
            if (unless_->else_clause) {
                unless_->else_clause = down_cast<pm_else_node_t>(rewriteBody(up_cast(unless_->else_clause)));
            }
            return node;
        }
        case PM_CASE_NODE: {
            auto *case_ = down_cast<pm_case_node_t>(node);
            node = maybeInsertCast(node);
            case_->predicate = rewriteNode(case_->predicate);
            rewriteNodes(case_->conditions);
            if (case_->else_clause) {
                case_->else_clause = down_cast<pm_else_node_t>(rewriteBody(up_cast(case_->else_clause)));
            }
            return node;
        }
        case PM_WHEN_NODE: {
            auto *when = down_cast<pm_when_node_t>(node);
            if (when->statements) {
                when->statements = rewriteBody(when->statements);
            }
            return node;
        }
        case PM_CASE_MATCH_NODE: {
            auto *caseMatch = down_cast<pm_case_match_node_t>(node);
            node = maybeInsertCast(node);
            caseMatch->predicate = rewriteNode(caseMatch->predicate);
            rewriteNodes(caseMatch->conditions);
            if (caseMatch->else_clause) {
                caseMatch->else_clause = down_cast<pm_else_node_t>(rewriteBody(up_cast(caseMatch->else_clause)));
            }
            return node;
        }
        case PM_IN_NODE: {
            auto *inPattern = down_cast<pm_in_node_t>(node);
            inPattern->pattern = rewriteNode(inPattern->pattern);
            if (inPattern->statements) {
                inPattern->statements = down_cast<pm_statements_node_t>(rewriteBody(up_cast(inPattern->statements)));
            }
            return node;
        }

        // Rescue/Ensure
        case PM_RESCUE_MODIFIER_NODE: {
            auto *rescueMod = down_cast<pm_rescue_modifier_node_t>(node);
            node = maybeInsertCast(node);
            rescueMod->rescue_expression = rewriteNode(rescueMod->rescue_expression);
            rescueMod->expression = rewriteNode(rescueMod->expression);
            return node;
        }
        case PM_RESCUE_NODE: {
            auto *rescue = down_cast<pm_rescue_node_t>(node);
            if (rescue->statements) {
                rescue->statements = rewriteBody(rescue->statements);
            }
            if (rescue->subsequent) {
                rescue->subsequent = down_cast<pm_rescue_node_t>(rewriteNode(up_cast(rescue->subsequent)));
            }
            return node;
        }
        case PM_ELSE_NODE: {
            auto *else_ = down_cast<pm_else_node_t>(node);
            if (else_->statements) {
                else_->statements = rewriteBody(else_->statements);
            }
            return node;
        }
        case PM_ENSURE_NODE: {
            auto *ensure = down_cast<pm_ensure_node_t>(node);
            if (ensure->statements) {
                ensure->statements = rewriteBody(ensure->statements);
            }
            return node;
        }

        // Boolean operators
        case PM_AND_NODE: {
            auto *and_ = down_cast<pm_and_node_t>(node);
            node = maybeInsertCast(node);
            and_->left = rewriteNode(and_->left);
            and_->right = rewriteNode(and_->right);
            return node;
        }
        case PM_OR_NODE: {
            auto *or_ = down_cast<pm_or_node_t>(node);
            node = maybeInsertCast(node);
            or_->left = rewriteNode(or_->left);
            or_->right = rewriteNode(or_->right);
            return node;
        }

        // Collections
        case PM_HASH_NODE: {
            auto *hash = down_cast<pm_hash_node_t>(node);
            node = maybeInsertCast(node);
            rewriteNodes(hash->elements);
            return node;
        }
        case PM_KEYWORD_HASH_NODE: {
            auto *kwh = down_cast<pm_keyword_hash_node_t>(node);
            rewriteNodes(kwh->elements);
            return node;
        }
        case PM_ASSOC_NODE: {
            auto *pair = down_cast<pm_assoc_node_t>(node);
            pair->key = rewriteNode(pair->key);
            pair->value = rewriteNode(pair->value);
            return node;
        }
        case PM_ARRAY_NODE: {
            auto *arr = down_cast<pm_array_node_t>(node);
            node = maybeInsertCast(node);
            rewriteNodes(arr->elements);
            return node;
        }

        // Splat
        case PM_SPLAT_NODE: {
            auto *splat = down_cast<pm_splat_node_t>(node);
            splat->expression = rewriteNode(splat->expression);
            return node;
        }
        case PM_ASSOC_SPLAT_NODE: {
            auto *splat = down_cast<pm_assoc_splat_node_t>(node);
            splat->value = rewriteNode(maybeInsertCast(splat->value));
            return node;
        }

        // Super
        case PM_SUPER_NODE: {
            auto *super_ = down_cast<pm_super_node_t>(node);
            rewriteArgumentsNode(super_->arguments);
            node = maybeInsertCast(node);
            return node;
        }

        // Program (top-level AST root)
        case PM_PROGRAM_NODE: {
            auto *program = down_cast<pm_program_node_t>(node);
            // Rewrite the top-level statements
            rewriteBody(up_cast(program->statements));
            return node;
        }

        case PM_PARENTHESES_NODE: {
            auto *paren = down_cast<pm_parentheses_node_t>(node);
            node = maybeInsertCast(node);
            paren->body = rewriteNode(paren->body);
            return node;
        }

        case PM_STATEMENTS_NODE: {
            auto *statements = down_cast<pm_statements_node_t>(node);
            rewriteNodes(statements->body);
            return node;
        }

        // Synthetic bind node for bind comments
        // We use PM_CONSTANT_READ_NODE with PM_CONSTANT_ID_UNSET to distinguish it from regular constant reads
        case PM_CONSTANT_READ_NODE: {
            auto *constantRead = down_cast<pm_constant_read_node_t>(node);

            // Check if this is a synthetic bind node
            if (constantRead->name == RBS_SYNTHETIC_BIND_MARKER) {
                return replaceSyntheticBind(node);
            }

            return maybeInsertCast(node);
        }

        default:
            return maybeInsertCast(node);
    }
}

pm_node_t *AssertionsRewriterPrism::run(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    // If there are no assertion comments to process we can skip entire tree walk.
    if (commentsByNode.empty()) {
        return node;
    }

    // Calculate total number of comments for early termination.
    totalComments = 0;
    for (const auto &[_, comments] : commentsByNode) {
        totalComments += comments.size();
    }

    return rewriteBody(node);
}

} // namespace sorbet::rbs
