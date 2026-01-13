#include "rbs/prism/SigsRewriterPrism.h"

#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "parser/prism/Helpers.h"
#include "rbs/SignatureTranslator.h"
#include "rbs/TypeParamsToParserNodes.h"

extern "C" {
#include "prism.h"
}
#include "rbs/prism/SignatureTranslatorPrism.h"

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {

bool canHaveSignature(pm_node_t *node, const parser::Prism::Parser &parser) {
    if (node == nullptr) {
        return false;
    }

    switch (PM_NODE_TYPE(node)) {
        case PM_DEF_NODE:
            // Both instance methods and singleton methods use PM_DEF_NODE in Prism
            // (singleton methods have a receiver field set)
            return true;
        case PM_CALL_NODE: {
            return parser.isAttrAccessorCall(node) || parser.isVisibilityCall(node);
        }
        default:
            return false;
    }
}

/**
 * Extracts and parses the argument from the annotation string.
 *
 * Considering an annotation like `@requires_ancestor: SomeModule`,
 * this function will extract and parse `SomeModule` as a type then return the corresponding Prism node.
 *
 * We do not error if the node is not a constant, we just insert it as is and let the pipeline error down the line.
 */
pm_node_t *extractHelperArgument(core::MutableContext ctx, parser::Prism::Parser &parser, const Comment &annotation,
                                 int offset) {
    while (annotation.string[offset] == ' ') {
        offset++;
    }

    Comment comment = {
        core::LocOffsets{annotation.typeLoc.beginPos() + offset, annotation.typeLoc.endPos()},
        core::LocOffsets{annotation.typeLoc.beginPos() + offset, annotation.typeLoc.endPos()},
        annotation.string.substr(offset),
    };

    return rbs::SignatureTranslatorPrism(ctx, parser).translateType(RBSDeclaration{vector<Comment>{comment}});
}

/**
 * Extracts class-level helper method calls from annotations.
 *
 * For example, given the annotations:
 *     # @abstract
 *     # @interface
 *
 * This function will return Prism call nodes for:
 *     self.abstract!()
 *     self.interface!()
 *
 * It doesn't insert them into the body of the class/module/etc.
 */
vector<pm_node_t *> extractHelpers(core::MutableContext ctx, absl::Span<const Comment> annotations,
                                   parser::Prism::Parser &parser) {
    if (annotations.empty()) {
        return {};
    }

    Factory prism{parser};
    vector<pm_node_t *> helpers;
    helpers.reserve(annotations.size());

    for (const auto &annotation : annotations) {
        pm_node_t *helperNode = nullptr;

        if (annotation.string == "abstract") {
            helperNode = prism.Call0(annotation.typeLoc, prism.Self(annotation.typeLoc), "abstract!"sv);
        } else if (annotation.string == "interface") {
            helperNode = prism.Call0(annotation.typeLoc, prism.Self(annotation.typeLoc), "interface!"sv);
        } else if (annotation.string == "final") {
            helperNode = prism.Call0(annotation.typeLoc, prism.Self(annotation.typeLoc), "final!"sv);
        } else if (annotation.string == "sealed") {
            helperNode = prism.Call0(annotation.typeLoc, prism.Self(annotation.typeLoc), "sealed!"sv);
        } else if (absl::StartsWith(annotation.string, "requires_ancestor:"sv)) {
            if (auto type = extractHelperArgument(ctx, parser, annotation, "requires_ancestor:"sv.size())) {
                auto statementsList = array{type};
                auto *stmts = prism.StatementsNode(annotation.typeLoc, absl::MakeSpan(statementsList));

                pm_node_t *callNode =
                    prism.Call0(annotation.typeLoc, prism.Self(annotation.typeLoc), "requires_ancestor"sv);
                auto *call = down_cast<pm_call_node_t>(callNode);
                call->block = prism.Block(annotation.typeLoc, stmts);

                helperNode = callNode;
            }
        }

        if (helperNode != nullptr) {
            helpers.emplace_back(helperNode);
        }
    }

    return helpers;
}

/**
 * Wraps the body in a `PM_STATEMENTS_NODE` if it isn't already.
 *
 * This is useful for cases where we want to insert helpers into the body of a class/module/etc.
 */
pm_node_t *maybeWrapBody(pm_node_t *body, core::LocOffsets loc, parser::Prism::Parser &parser) {
    Factory prism{parser};

    if (body == nullptr) {
        return prism.StatementsNode(loc, absl::Span<pm_node_t *>{});
    }

    if (PM_NODE_TYPE_P(body, PM_STATEMENTS_NODE)) {
        return body; // Already wrapped
    }

    auto statementsList = array{body};
    return prism.StatementsNode(loc, absl::MakeSpan(statementsList));
}

/**
 * Returns true if the body contains an `extend T::Helpers` call already.
 */
bool containsExtendTHelper(pm_statements_node_t *body, const parser::Prism::Parser &prismParser,
                           core::MutableContext ctx) {
    ENFORCE(body != nullptr);

    auto statements = absl::MakeSpan(body->body.nodes, body->body.size);
    return absl::c_any_of(statements, [&](pm_node_t *stmt) {
        if (!PM_NODE_TYPE_P(stmt, PM_CALL_NODE)) {
            return false;
        }

        auto *call = down_cast<pm_call_node_t>(stmt);
        auto methodName = prismParser.resolveConstant(call->name);

        if (methodName != "extend") {
            return false;
        }

        if (call->receiver != nullptr && !PM_NODE_TYPE_P(call->receiver, PM_SELF_NODE)) {
            return false;
        }

        if (call->arguments == nullptr || call->arguments->arguments.size != 1) {
            return false;
        }

        pm_node_t *arg = call->arguments->arguments.nodes[0];
        if (!PM_NODE_TYPE_P(arg, PM_CONSTANT_PATH_NODE)) {
            return false;
        }

        auto *constantPath = down_cast<pm_constant_path_node_t>(arg);
        auto argName = prismParser.resolveConstant(constantPath->name);

        if (argName != "Helpers") {
            return false;
        }

        return prismParser.isT(constantPath->parent);
    });
}

/**
 * Inserts an `extend T::Helpers` call into the body if it doesn't already exist.
 */
void maybeInsertExtendTHelpers(pm_node_t *body, core::LocOffsets loc, const parser::Prism::Parser &prismParser,
                               core::MutableContext ctx, const parser::Prism::Factory &prism) {
    auto *statements = down_cast<pm_statements_node_t>(body);
    ENFORCE(statements != nullptr);

    if (containsExtendTHelper(statements, prismParser, ctx)) {
        return;
    }

    pm_node_t *tHelpers = prism.THelpers(loc);
    pm_node_t *selfNode = prism.Self(loc);
    pm_node_t *extendCall = prism.Call1(loc, selfNode, "extend"sv, tHelpers);

    pm_node_list_append(&statements->body, extendCall);
}

/**
 * Inserts the helpers into the body.
 */
void insertHelpers(pm_node_t *body, absl::Span<pm_node_t *const> helpers) {
    auto *statements = down_cast<pm_statements_node_t>(body);
    ENFORCE(statements != nullptr);

    for (auto *helper : helpers) {
        pm_node_list_append(&statements->body, helper);
    }
}

} // namespace

void SigsRewriterPrism::insertTypeParams(pm_node_t *node, pm_node_t *body) {
    ENFORCE(PM_NODE_TYPE_P(node, PM_CLASS_NODE) || PM_NODE_TYPE_P(node, PM_MODULE_NODE) ||
                PM_NODE_TYPE_P(node, PM_SINGLETON_CLASS_NODE),
            "Type parameters can only exist on classes, singleton classes, and modules");

    auto comments = commentsForNode(node);
    if (comments.signatures.empty()) {
        return;
    }

    if (comments.signatures.size() > 1) {
        if (auto e = ctx.beginIndexerError(comments.signatures[0].commentLoc(),
                                           core::errors::Rewriter::RBSMultipleGenericSignatures)) {
            e.setHeader("Generic classes and modules can only have one RBS generic signature");
            return;
        }
    }

    auto signature = comments.signatures[0];
    auto typeParamsTranslator = SignatureTranslatorPrism{ctx, parser};
    auto typeParams = typeParamsTranslator.translateTypeParams(signature);

    if (typeParams.empty()) {
        return;
    }

    ENFORCE(body != nullptr && PM_NODE_TYPE_P(body, PM_STATEMENTS_NODE), "Body must be a statements node");
    auto *statements = down_cast<pm_statements_node_t>(body);

    for (auto *typeParam : typeParams) {
        pm_node_list_append(&statements->body, typeParam);
    }
}

CommentsPrism SigsRewriterPrism::commentsForNode(pm_node_t *node) {
    auto comments = CommentsPrism{};

    if (node == nullptr) {
        return comments;
    }

    auto it = commentsByNode.find(node);
    if (it == commentsByNode.end()) {
        return comments;
    }

    enum class SignatureState { None, Started, Multiline };
    auto state = SignatureState::None;

    auto &nodes = it->second;
    auto declarationComments = vector<Comment>{};

    for (auto &commentNode : nodes) {
        if (absl::StartsWith(commentNode.string, "# @")) {
            auto comment = Comment{
                .commentLoc = commentNode.loc,
                .typeLoc = core::LocOffsets{commentNode.loc.beginPos() + 3, commentNode.loc.endPos()},
                .string = commentNode.string.substr(3),
            };
            comments.annotations.emplace_back(move(comment));
            continue;
        }

        if (absl::StartsWith(commentNode.string, "#:")) {
            if (state == SignatureState::Multiline) {
                if (auto e = ctx.beginIndexerError(commentNode.loc, core::errors::Rewriter::RBSMultilineMisformatted)) {
                    e.setHeader("Signature start (\"#:\") cannot appear after a multiline signature (\"#|\")");
                    return comments;
                }
            }
            state = SignatureState::Started;
            auto comment = Comment{
                .commentLoc = commentNode.loc,
                .typeLoc = core::LocOffsets{commentNode.loc.beginPos() + 2, commentNode.loc.endPos()},
                .string = commentNode.string.substr(2),
            };

            if (declarationComments.empty()) {
                declarationComments.emplace_back(move(comment));
            } else {
                comments.signatures.emplace_back(RBSDeclaration{move(declarationComments)});
                declarationComments.clear();
                declarationComments.emplace_back(move(comment));
            }
            continue;
        }

        if (absl::StartsWith(commentNode.string, "#|")) {
            if (state == SignatureState::None) {
                if (auto e = ctx.beginIndexerError(commentNode.loc, core::errors::Rewriter::RBSMultilineMisformatted)) {
                    e.setHeader("Multiline signature (\"#|\") must be preceded by a signature start (\"#:\")");
                    return comments;
                }
            }
            state = SignatureState::Multiline;
            auto comment = Comment{
                .commentLoc = commentNode.loc,
                .typeLoc = core::LocOffsets{commentNode.loc.beginPos() + 2, commentNode.loc.endPos()},
                .string = commentNode.string.substr(2),
            };
            declarationComments.emplace_back(move(comment));
            continue;
        }
    }

    if (!declarationComments.empty()) {
        auto rbsDeclaration = RBSDeclaration{move(declarationComments)};
        comments.signatures.emplace_back(move(rbsDeclaration));
    }

    return comments;
}

unique_ptr<vector<pm_node_t *>> SigsRewriterPrism::signaturesForNode(pm_node_t *node) {
    auto comments = commentsForNode(node);

    if (comments.signatures.empty()) {
        return nullptr;
    }

    auto signatures = make_unique<vector<pm_node_t *>>();
    auto signatureTranslator = rbs::SignatureTranslatorPrism{ctx, parser};

    for (auto &declaration : comments.signatures) {
        if (PM_NODE_TYPE_P(node, PM_DEF_NODE)) {
            auto sig = signatureTranslator.translateMethodSignature(node, declaration, comments.annotations);
            if (sig) {
                signatures->emplace_back(sig);
            }
        } else if (PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
            auto *call = down_cast<pm_call_node_t>(node);
            if (parser.isVisibilityCall(node)) {
                // For visibility modifiers, translate the signature for the inner method definition
                auto sig = signatureTranslator.translateMethodSignature(call->arguments->arguments.nodes[0],
                                                                        declaration, comments.annotations);
                if (sig) {
                    signatures->emplace_back(sig);
                }
            } else if (parser.isAttrAccessorCall(node)) {
                auto sig = signatureTranslator.translateAttrSignature(call, declaration, comments.annotations);
                if (sig) {
                    signatures->emplace_back(sig);
                }
            } else {
                Exception::raise("Unimplemented call node type for signatures");
            }
        } else {
            Exception::raise("Unimplemented node type for signatures: {}", (int)PM_NODE_TYPE(node));
        }
    }

    return signatures;
}

/**
 * Replace the synthetic type alias node with a `T.type_alias` call.
 */
pm_node_t *SigsRewriterPrism::replaceSyntheticTypeAlias(pm_node_t *node) {
    auto comments = commentsForNode(node);
    ENFORCE(!comments.signatures.empty(), "No inline comment found for synthetic type alias");
    ENFORCE(comments.signatures.size() <= 1, "Multiple signatures found for synthetic type alias");

    auto aliasDeclaration = comments.signatures[0];
    auto fullString = aliasDeclaration.string;
    auto typeBeginLoc = fullString.find("=");

    if (typeBeginLoc == string::npos) {
        // No '=' found, invalid type alias
        auto loc = parser.translateLocation(node->location);
        return prism.TTypeAlias(loc, prism.TUntyped(loc));
    }

    auto typeDeclaration = RBSDeclaration{vector<Comment>{Comment{
        .commentLoc = aliasDeclaration.commentLoc(),
        .typeLoc = core::LocOffsets{aliasDeclaration.fullTypeLoc().beginPos() + (uint32_t)typeBeginLoc + 1,
                                    aliasDeclaration.fullTypeLoc().endPos()},
        .string = fullString.substr(typeBeginLoc + 1),
    }}};

    auto signatureTranslator = rbs::SignatureTranslatorPrism{ctx, parser};
    absl::Span<pair<core::LocOffsets, core::NameRef>> typeParams; // Empty for type aliases
    auto type = signatureTranslator.translateAssertionType(typeParams, typeDeclaration);

    if (type == nullptr) {
        auto loc = parser.translateLocation(node->location);
        type = prism.TUntyped(loc);
    }

    auto loc = parser.translateLocation(type->location);
    return prism.TTypeAlias(loc, type);
}

void SigsRewriterPrism::rewriteNodes(pm_node_list_t &nodes) {
    for (size_t i = 0; i < nodes.size; i++) {
        nodes.nodes[i] = rewriteBody(nodes.nodes[i]);
    }
}

void SigsRewriterPrism::rewriteArgumentsNode(pm_arguments_node_t *args) {
    if (args) {
        rewriteNodes(args->arguments);
    }
}

pm_node_t *SigsRewriterPrism::rewriteBody(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    // Handle statements nodes (class/module bodies with multiple statements)
    if (PM_NODE_TYPE_P(node, PM_STATEMENTS_NODE)) {
        auto *statements = down_cast<pm_statements_node_t>(node);

        // Save old statements before modifying (pm_node_list_append can realloc, invalidating pointers)
        pm_node_list_t oldStmts = statements->body;
        statements->body = (pm_node_list_t){.size = 0, .capacity = 0, .nodes = nullptr};

        // For each statement, insert signatures before it if it's a signature target
        for (size_t i = 0; i < oldStmts.size; i++) {
            pm_node_t *stmt = oldStmts.nodes[i];

            if (canHaveSignature(stmt, parser)) {
                if (auto signatures = signaturesForNode(stmt)) {
                    // Add all signatures
                    for (auto sig : *signatures) {
                        pm_node_list_append(&statements->body, sig);
                    }
                }
            }

            // Add the rewritten statement
            pm_node_list_append(&statements->body, rewriteNode(stmt));
        }

        // Free the old list structure (not the nodes themselves, as they were moved)
        free(oldStmts.nodes);

        return node;
    }

    // Handle single node that is a signature target
    if (canHaveSignature(node, parser)) {
        if (auto signatures = signaturesForNode(node)) {
            // Wrap in a statements node with signatures + node
            return createStatementsWithSignatures(rewriteNode(node), move(signatures));
        }
    }

    return rewriteNode(node);
}

pm_statements_node_t *SigsRewriterPrism::rewriteBody(pm_statements_node_t *stmts) {
    return down_cast<pm_statements_node_t>(rewriteBody(up_cast(stmts)));
}

void SigsRewriterPrism::processClassBody(pm_node_t *node, pm_node_t *&body,
                                         absl::Span<pm_node_t *const> helpers) {
    auto loc = body ? parser.translateLocation(body->location)
                    : parser.translateLocation(node->location);

    body = maybeWrapBody(body, loc, parser);
    if (!helpers.empty()) {
        maybeInsertExtendTHelpers(body, loc, parser, ctx, prism);
        insertHelpers(body, helpers);
    }

    insertTypeParams(node, body);
}

pm_node_t *SigsRewriterPrism::rewriteClass(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    auto comments = commentsForNode(node);
    auto helpers = extractHelpers(ctx, comments.annotations, parser);

    if (comments.signatures.empty() && helpers.empty()) {
        return node;
    }

    switch (PM_NODE_TYPE(node)) {
        case PM_CLASS_NODE:
            processClassBody(node, down_cast<pm_class_node_t>(node)->body, helpers);
            break;
        case PM_MODULE_NODE:
            processClassBody(node, down_cast<pm_module_node_t>(node)->body, helpers);
            break;
        case PM_SINGLETON_CLASS_NODE:
            processClassBody(node, down_cast<pm_singleton_class_node_t>(node)->body, helpers);
            break;
        default:
            Exception::raise("Unimplemented node type for rewriteClass: {}", (int)PM_NODE_TYPE(node));
    }

    return node;
}

pm_node_t *SigsRewriterPrism::rewriteNode(pm_node_t *node) {
    if (node == nullptr) {
        return node;
    }

    switch (PM_NODE_TYPE(node)) {
        case PM_BEGIN_NODE: {
            auto *begin = down_cast<pm_begin_node_t>(node);
            begin->statements = down_cast<pm_statements_node_t>(rewriteNode(up_cast(begin->statements)));
            begin->rescue_clause = down_cast<pm_rescue_node_t>(rewriteNode(up_cast(begin->rescue_clause)));
            begin->else_clause = down_cast<pm_else_node_t>(rewriteNode(up_cast(begin->else_clause)));
            begin->ensure_clause = down_cast<pm_ensure_node_t>(rewriteNode(up_cast(begin->ensure_clause)));
            return node;
        }
        case PM_BLOCK_NODE: {
            auto *block = down_cast<pm_block_node_t>(node);
            block->body = rewriteBody(block->body);
            return node;
        }
        case PM_LOCAL_VARIABLE_WRITE_NODE: {
            auto *n = down_cast<pm_local_variable_write_node_t>(node);
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_AND_WRITE_NODE: {
            auto *n = down_cast<pm_local_variable_and_write_node_t>(node);
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE: {
            auto *n = down_cast<pm_local_variable_or_write_node_t>(node);
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE: {
            auto *n = down_cast<pm_local_variable_operator_write_node_t>(node);
            n->value = rewriteNode(n->value);
            return node;
        }
        case PM_MODULE_NODE: {
            auto *mod = down_cast<pm_module_node_t>(node);
            mod->body = rewriteBody(mod->body);
            return rewriteClass(node);
        }
        case PM_CLASS_NODE: {
            auto *cls = down_cast<pm_class_node_t>(node);
            cls->body = rewriteBody(cls->body);
            return rewriteClass(node);
        }
        case PM_DEF_NODE: {
            auto *def = down_cast<pm_def_node_t>(node);
            def->body = rewriteBody(def->body);
            return node;
        }
        case PM_SINGLETON_CLASS_NODE: {
            auto *sclass = down_cast<pm_singleton_class_node_t>(node);
            sclass->body = rewriteBody(sclass->body);
            return rewriteClass(node);
        }
        case PM_FOR_NODE: {
            auto *for_ = down_cast<pm_for_node_t>(node);
            if (auto *stmts = for_->statements) {
                for_->statements = rewriteBody(stmts);
            }
            return node;
        }
        case PM_ARRAY_NODE: {
            auto *array = down_cast<pm_array_node_t>(node);
            rewriteNodes(array->elements);
            return node;
        }
        case PM_HASH_NODE: {
            auto *hash = down_cast<pm_hash_node_t>(node);
            rewriteNodes(hash->elements);
            return node;
        }
        case PM_ASSOC_NODE: {
            auto *pair = down_cast<pm_assoc_node_t>(node);
            pair->key = rewriteNode(pair->key);
            pair->value = rewriteNode(pair->value);
            return node;
        }
        case PM_RESCUE_NODE: {
            auto *rescue = down_cast<pm_rescue_node_t>(node);
            if (auto *stmts = rescue->statements) {
                rescue->statements = rewriteBody(stmts);
            }
            if (auto *subsequent = rescue->subsequent) {
                rescue->subsequent = down_cast<pm_rescue_node_t>(rewriteNode(up_cast(subsequent)));
            }
            return node;
        }
        case PM_ELSE_NODE: {
            auto *else_ = down_cast<pm_else_node_t>(node);
            if (auto *stmts = else_->statements) {
                else_->statements = rewriteBody(stmts);
            }
            return node;
        }
        case PM_ENSURE_NODE: {
            auto *ensure = down_cast<pm_ensure_node_t>(node);
            if (auto *stmts = ensure->statements) {
                ensure->statements = rewriteBody(stmts);
            }
            return node;
        }
        case PM_IF_NODE: {
            auto *if_ = down_cast<pm_if_node_t>(node);
            if (auto *stmts = if_->statements) {
                if_->statements = rewriteBody(stmts);
            }
            if (auto *subsequent = if_->subsequent) {
                if_->subsequent = rewriteBody(subsequent);
            }
            return node;
        }
        case PM_UNLESS_NODE: {
            auto *unless_ = down_cast<pm_unless_node_t>(node);
            if (auto *stmts = unless_->statements) {
                unless_->statements = rewriteBody(stmts);
            }
            if (auto *elseClause = unless_->else_clause) {
                unless_->else_clause = down_cast<pm_else_node_t>(rewriteBody(up_cast(elseClause)));
            }
            return node;
        }
        case PM_MULTI_WRITE_NODE: {
            auto *masgn = down_cast<pm_multi_write_node_t>(node);
            masgn->value = rewriteNode(masgn->value);
            return node;
        }
        case PM_CASE_NODE: {
            auto *case_ = down_cast<pm_case_node_t>(node);
            rewriteNodes(case_->conditions);
            if (auto *elseClause = case_->else_clause) {
                case_->else_clause = down_cast<pm_else_node_t>(rewriteBody(up_cast(elseClause)));
            }
            return node;
        }
        case PM_CASE_MATCH_NODE: {
            auto *case_ = down_cast<pm_case_match_node_t>(node);
            rewriteNodes(case_->conditions);
            if (auto *elseClause = case_->else_clause) {
                case_->else_clause = down_cast<pm_else_node_t>(rewriteBody(up_cast(elseClause)));
            }
            return node;
        }
        case PM_WHEN_NODE: {
            auto *when = down_cast<pm_when_node_t>(node);
            if (auto *stmts = when->statements) {
                when->statements = rewriteBody(stmts);
            }
            return node;
        }
        case PM_CALL_NODE: {
            auto *call = down_cast<pm_call_node_t>(node);
            if (auto *block = call->block) {
                call->block = rewriteNode(block);
            }
            rewriteArgumentsNode(call->arguments);
            return node;
        }
        case PM_SUPER_NODE: {
            auto *sup = down_cast<pm_super_node_t>(node);
            rewriteArgumentsNode(sup->arguments);
            return node;
        }
        case PM_RETURN_NODE: {
            auto *ret = down_cast<pm_return_node_t>(node);
            rewriteArgumentsNode(ret->arguments);
            return node;
        }
        case PM_NEXT_NODE: {
            auto *n = down_cast<pm_next_node_t>(node);
            rewriteArgumentsNode(n->arguments);
            return node;
        }
        case PM_BREAK_NODE: {
            auto *b = down_cast<pm_break_node_t>(node);
            rewriteArgumentsNode(b->arguments);
            return node;
        }
        case PM_SPLAT_NODE: {
            auto *s = down_cast<pm_splat_node_t>(node);
            s->expression = rewriteNode(s->expression);
            return node;
        }
        case PM_CONSTANT_WRITE_NODE: {
            auto *write = down_cast<pm_constant_write_node_t>(node);
            // Check if this is a type alias assignment with synthetic marker
            if (auto *constantRead = PM_NODE_TYPE_P(write->value, PM_CONSTANT_READ_NODE)
                                         ? down_cast<pm_constant_read_node_t>(write->value)
                                         : nullptr) {
                if (constantRead->name == RBS_SYNTHETIC_TYPE_ALIAS_MARKER) {
                    // Replace the value with the T.type_alias call
                    write->value = replaceSyntheticTypeAlias(write->value);
                    return node;
                }
            }
            write->value = rewriteNode(write->value);
            return node;
        }
        case PM_PROGRAM_NODE: {
            auto *program = down_cast<pm_program_node_t>(node);
            rewriteBody(up_cast(program->statements));
            return node;
        }
        case PM_STATEMENTS_NODE: {
            auto *statements = down_cast<pm_statements_node_t>(node);
            rewriteNodes(statements->body);
            return node;
        }
        default:
            return node;
    }
}

pm_node_t *SigsRewriterPrism::run(pm_node_t *node) {
    // If there are no signature comments to process, we can skip the entire tree walk.
    if (commentsByNode.empty()) {
        return node;
    }

    return rewriteBody(node);
}

// Helper method to create statements nodes with signatures
pm_node_t *SigsRewriterPrism::createStatementsWithSignatures(pm_node_t *originalNode,
                                                             unique_ptr<vector<pm_node_t *>> signatures) {
    if (!signatures || signatures->empty()) {
        return originalNode;
    }

    // Build the body: signatures + original node
    vector<pm_node_t *> body{};
    body.reserve(signatures->size() + 1);

    for (auto *sigCall : *signatures) {
        body.push_back(sigCall);
    }
    body.push_back(originalNode);

    auto loc = parser.translateLocation(originalNode->location);
    return prism.StatementsNode(loc, absl::MakeSpan(body));
}

} // namespace sorbet::rbs
