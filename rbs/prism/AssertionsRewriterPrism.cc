#include "rbs/prism/AssertionsRewriterPrism.h"

#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "parser/parser.h"
#include "rbs/SignatureTranslator.h"
#include <regex>

using namespace std;

namespace sorbet::rbs {

namespace {

const regex not_nil_pattern_prism("^\\s*!nil\\s*(#.*)?$");
const regex untyped_pattern_prism("^\\s*untyped\\s*(#.*)?$");
const regex absurd_pattern_prism("^\\s*absurd\\s*(#.*)?$");

/*
 * Parse the comment and return the type as a `parser::Node` and the kind of assertion we need to apply (let, cast,
 * must, unsafe).
 *
 * The `typeParams` param is used to resolve the type parameters from the method signature.
 *
 * Returns `nullopt` if the comment is not a valid RBS expression (an error is produced).
 */
optional<pair<unique_ptr<parser::Node>, InlineCommentPrism::Kind>>
parseCommentPrism(core::MutableContext ctx, InlineCommentPrism comment,
                  vector<pair<core::LocOffsets, core::NameRef>> typeParams) {
    if (comment.kind == InlineCommentPrism::Kind::MUST || comment.kind == InlineCommentPrism::Kind::UNSAFE ||
        comment.kind == InlineCommentPrism::Kind::ABSURD) {
        return pair<unique_ptr<parser::Node>, InlineCommentPrism::Kind>{
            // The type should never be used but we need to hold the location...
            make_unique<parser::Nil>(comment.comment.typeLoc),
            comment.kind,
        };
    }

    auto signatureTranslator = rbs::SignatureTranslator(ctx);
    vector<Comment> comments;
    comments.push_back(comment.comment);
    auto declaration = RBSDeclaration(comments);
    auto type = signatureTranslator.translateAssertionType(typeParams, declaration);

    if (type == nullptr) {
        // We couldn't parse the type, we produced an error, we don't return anything
        return nullopt;
    }

    return pair{move(type), comment.kind};
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
vector<pair<core::LocOffsets, core::NameRef>> extractTypeParamsPrism(parser::Node *sig) {
    auto typeParams = vector<pair<core::LocOffsets, core::NameRef>>();

    // Do we have a previous signature?
    if (!sig) {
        return typeParams;
    }

    auto block = parser::cast_node<parser::Block>(sig);
    ENFORCE(block != nullptr);

    // Does the sig contain a `type_parameters()` invocation?
    auto send = parser::cast_node<parser::Send>(block->body.get());
    while (send && send->method != core::Names::typeParameters()) {
        send = parser::cast_node<parser::Send>(send->receiver.get());
    }

    if (send == nullptr) {
        return typeParams;
    }

    // Collect the type parameters
    for (auto &arg : send->args) {
        auto sym = parser::cast_node<parser::Symbol>(arg.get());
        if (sym == nullptr) {
            continue;
        }

        typeParams.emplace_back(arg->loc, sym->val);
    }

    return typeParams;
}

bool sameConstantPrism(core::MutableContext ctx, unique_ptr<parser::Node> &a, unique_ptr<parser::Node> &b) {
    auto aConst = parser::cast_node<parser::Const>(a.get());
    auto bConst = parser::cast_node<parser::Const>(b.get());

    if (aConst == nullptr || bConst == nullptr) {
        return false;
    }

    if (aConst->name != bConst->name) {
        return false;
    }

    if ((aConst->scope == nullptr && bConst->scope != nullptr) ||
        (aConst->scope != nullptr && bConst->scope == nullptr)) {
        return false;
    }

    return (aConst->scope == nullptr && bConst->scope == nullptr) ||
           sameConstantPrism(ctx, aConst->scope, bConst->scope);
}

void maybeSupplyGenericTypeArgumentsPrism(core::MutableContext ctx, unique_ptr<parser::Node> *node,
                                          unique_ptr<parser::Node> *type) {
    // We only rewrite `.new` calls
    auto newSend = parser::cast_node<parser::Send>(node->get());
    if (newSend == nullptr || newSend->method != core::Names::new_()) {
        return;
    }

    // We only rewrite when casted to a generic type
    auto bracketSend = parser::cast_node<parser::Send>(type->get());
    if (bracketSend == nullptr || bracketSend->method != core::Names::syntheticSquareBrackets()) {
        return;
    }

    // We only rewrite when the generic type is the same as the instantiated one
    if (!sameConstantPrism(ctx, newSend->receiver, bracketSend->receiver)) {
        return;
    }

    newSend->receiver = type->get()->deepCopy();
}

} // namespace

/**
 * Mark the given comment location as "consumed" so it won't be picked up by subsequent calls to `commentForPos`.
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

/*
 * Get the RBS comment for the given node.
 *
 * Returns `nullopt` if no comment is found or if the comment was already consumed.
 */
optional<rbs::InlineCommentPrism> AssertionsRewriterPrism::commentForNode(const unique_ptr<parser::Node> &node) {
    // TODO: Update to work with prism comment maps
    if (legacyCommentsByNode == nullptr) {
        return nullopt;
    }
    auto it = legacyCommentsByNode->find(node.get());
    if (it == legacyCommentsByNode->end()) {
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

            if (regex_match(content.begin(), content.end(), not_nil_pattern_prism)) {
                kind = InlineCommentPrism::Kind::MUST;
            } else if (regex_match(content.begin(), content.end(), untyped_pattern_prism)) {
                kind = InlineCommentPrism::Kind::UNSAFE;
            }
        } else if (regex_match(content.begin(), content.end(), absurd_pattern_prism)) {
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
 * Save the signature from the given block so it can be used to resolve the type parameters from the method signature.
 *
 * Returns `true` if the block is a `sig` send, `false` otherwise.
 */
bool AssertionsRewriterPrism::saveTypeParams(parser::Block *block) {
    if (block->body == nullptr) {
        return false;
    }

    auto send = parser::cast_node<parser::Send>(block->send.get());
    if (send == nullptr) {
        return false;
    }

    if (send->method != core::Names::sig()) {
        return false;
    }

    typeParams = extractTypeParamsPrism(block);

    return true;
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
unique_ptr<parser::Node>
AssertionsRewriterPrism::insertCast(unique_ptr<parser::Node> node,
                                    optional<pair<unique_ptr<parser::Node>, InlineCommentPrism::Kind>> pair) {
    if (!pair) {
        return node;
    }

    auto type = move(pair->first);
    auto kind = pair->second;

    maybeSupplyGenericTypeArgumentsPrism(ctx, &node, &type);

    if (kind == InlineCommentPrism::Kind::LET) {
        return parser::MK::TLet(type->loc, move(node), move(type));
    } else if (kind == InlineCommentPrism::Kind::CAST) {
        return parser::MK::TCast(type->loc, move(node), move(type));
    } else if (kind == InlineCommentPrism::Kind::MUST) {
        return parser::MK::TMust(type->loc, move(node));
    } else if (kind == InlineCommentPrism::Kind::UNSAFE) {
        return parser::MK::TUnsafe(type->loc, move(node));
    } else if (kind == InlineCommentPrism::Kind::ABSURD) {
        return parser::MK::TAbsurd(type->loc, move(node));
    } else if (kind == InlineCommentPrism::Kind::BIND) {
        if (auto e = ctx.beginIndexerError(type->loc, core::errors::Rewriter::RBSUnsupported)) {
            e.setHeader("`{}` binding can't be used as a trailing comment", "self");
        }
        return node;
    } else {
        Exception::raise("Unknown assertion kind");
    }
}

/**
 * Insert a cast into the given node if there is an not yet consumed RBS assertion comment after the node.
 * Optionally accepts another node to associate its comments instead.
 */
unique_ptr<parser::Node> AssertionsRewriterPrism::maybeInsertCast(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    if (auto inlineComment = commentForNode(node)) {
        if (auto type = parseCommentPrism(ctx, inlineComment.value(), typeParams)) {
            return insertCast(move(node), move(type));
        }
    }

    return node;
}

/**
 * Replace the synthetic node with a `T.bind` call.
 */
unique_ptr<parser::Node> AssertionsRewriterPrism::replaceSyntheticBind(unique_ptr<parser::Node> node) {
    auto inlineComment = commentForNode(node);

    if (!inlineComment) {
        // This should never happen
        Exception::raise("No inline comment found for synthetic bind");
    }

    auto pair = parseCommentPrism(ctx, inlineComment.value(), typeParams);

    if (!pair) {
        // We already raised an error while parsing the comment, so we just bind to `T.untyped`
        return parser::MK::TBindSelf(node->loc, parser::MK::TUntyped(node->loc));
    }

    auto kind = pair->second;

    if (kind != InlineCommentPrism::Kind::BIND) {
        // This should never happen
        Exception::raise("Invalid inline comment for synthetic bind");
    }

    auto type = move(pair->first);

    return parser::MK::TBindSelf(type->loc, move(type));
}

/**
 * Rewrite a collection of nodes, wrap them in an array and cast the array.
 */
parser::NodeVec AssertionsRewriterPrism::rewriteNodesAsArray(const unique_ptr<parser::Node> &node,
                                                             parser::NodeVec nodes) {
    if (auto inlineComment = commentForNode(node)) {
        if (nodes.size() > 1) {
            auto loc = nodes.front()->loc.join(nodes.back()->loc);
            auto arr = parser::MK::Array(loc, move(nodes));
            arr = rewriteNode(move(arr));

            auto vector = parser::NodeVec();
            vector.emplace_back(move(arr));
            nodes = move(vector);
        }
        if (auto type = parseCommentPrism(ctx, inlineComment.value(), typeParams)) {
            nodes.front() = insertCast(move(nodes.front()), move(type));
        }
    }

    rewriteNodes(&nodes);
    return nodes;
}

/**
 * Rewrite a collection of nodes in place.
 */
void AssertionsRewriterPrism::rewriteNodes(parser::NodeVec *nodes) {
    for (auto &node : *nodes) {
        node = rewriteNode(move(node));
    }
}

/**
 * Rewrite the body of a node.
 */
unique_ptr<parser::Node> AssertionsRewriterPrism::rewriteBody(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    if (auto begin = parser::cast_node<parser::Begin>(node.get())) {
        rewriteNodes(&begin->stmts);
        return node;
    }

    return rewriteNode(move(node));
}

/**
 * Rewrite a node.
 */
unique_ptr<parser::Node> AssertionsRewriterPrism::rewriteNode(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    unique_ptr<parser::Node> result;

    typecase(
        node.get(),

        // Scopes

        [&](parser::Module *module) {
            module->body = rewriteBody(move(module->body));
            result = move(node);

            typeParams.clear();
        },
        [&](parser::Class *klass) {
            klass->body = rewriteBody(move(klass->body));
            result = move(node);

            typeParams.clear();
        },
        [&](parser::SClass *sclass) {
            sclass->body = rewriteBody(move(sclass->body));
            result = move(node);

            typeParams.clear();
        },
        [&](parser::DefMethod *method) {
            method->body = rewriteBody(move(method->body));
            result = move(node);

            typeParams.clear();
        },
        [&](parser::DefS *method) {
            method->body = rewriteBody(move(method->body));
            result = move(node);

            typeParams.clear();
        },

        // Stmts

        [&](parser::Begin *begin) {
            node = maybeInsertCast(move(node));
            rewriteNodes(&begin->stmts);
            result = move(node);
        },
        [&](parser::Kwbegin *kwbegin) {
            node = maybeInsertCast(move(node));
            rewriteNodes(&kwbegin->stmts);
            result = move(node);
        },

        // Assigns

        [&](parser::Assign *asgn) {
            asgn->lhs = rewriteNode(move(asgn->lhs));
            asgn->rhs = rewriteNode(move(asgn->rhs));
            result = move(node);
        },
        [&](parser::AndAsgn *andAsgn) {
            andAsgn->left = rewriteNode(move(andAsgn->left));
            andAsgn->right = maybeInsertCast(move(andAsgn->right));
            andAsgn->right = rewriteNode(move(andAsgn->right));
            result = move(node);
        },
        [&](parser::OpAsgn *opAsgn) {
            opAsgn->left = rewriteNode(move(opAsgn->left));
            opAsgn->right = maybeInsertCast(move(opAsgn->right));
            opAsgn->right = rewriteNode(move(opAsgn->right));
            result = move(node);
        },
        [&](parser::OrAsgn *orAsgn) {
            orAsgn->left = rewriteNode(move(orAsgn->left));
            orAsgn->right = maybeInsertCast(move(orAsgn->right));
            orAsgn->right = rewriteNode(move(orAsgn->right));
            result = move(node);
        },
        [&](parser::Masgn *masgn) {
            masgn->lhs = rewriteNode(move(masgn->lhs));
            masgn->rhs = maybeInsertCast(move(masgn->rhs));
            masgn->rhs = rewriteNode(move(masgn->rhs));
            result = move(node);
        },

        // Sends

        [&](parser::Send *send) {
            send->receiver = rewriteNode(move(send->receiver));
            node = maybeInsertCast(move(node));
            rewriteNodes(&send->args);
            result = move(node);
        },
        [&](parser::CSend *csend) {
            csend->receiver = maybeInsertCast(move(csend->receiver));
            csend->receiver = rewriteNode(move(csend->receiver));

            node = maybeInsertCast(move(node));
            rewriteNodes(&csend->args);
            result = move(node);
        },
        [&](parser::Block *block) {
            if (saveTypeParams(block)) {
                // If this is a `sig` block, we need to save the type parameters so we can use them to resolve the type
                // parameters from the method signature.
                result = move(node);
                return;
            }

            node = maybeInsertCast(move(node));
            block->send = rewriteNode(move(block->send));
            block->body = rewriteBody(move(block->body));
            result = move(node);
        },

        // Loops

        [&](parser::While *wl) {
            node = maybeInsertCast(move(node));
            wl->cond = rewriteNode(move(wl->cond));
            wl->body = rewriteBody(move(wl->body));
            result = move(node);
        },
        [&](parser::WhilePost *wl) {
            wl->cond = rewriteNode(move(wl->cond));
            wl->body = rewriteBody(move(wl->body));
            result = move(node);
        },
        [&](parser::Until *until) {
            node = maybeInsertCast(move(node));
            until->cond = rewriteNode(move(until->cond));
            until->body = rewriteBody(move(until->body));
            result = move(node);
        },
        [&](parser::UntilPost *until) {
            until->cond = rewriteNode(move(until->cond));
            until->body = rewriteBody(move(until->body));
            result = move(node);
        },
        [&](parser::For *for_) {
            node = maybeInsertCast(move(node));
            for_->vars = rewriteNode(move(for_->vars));
            for_->expr = rewriteNode(move(for_->expr));
            for_->body = rewriteBody(move(for_->body));
            result = move(node);
        },

        [&](parser::Break *break_) {
            if (break_->exprs.empty()) {
                result = move(node);
                return;
            }

            rewriteNodes(&break_->exprs);
            break_->exprs = rewriteNodesAsArray(node, move(break_->exprs));
            result = move(node);
        },
        [&](parser::Next *next) {
            if (next->exprs.empty()) {
                result = move(node);
                return;
            }

            rewriteNodes(&next->exprs);
            next->exprs = rewriteNodesAsArray(node, move(next->exprs));
            result = move(node);
        },
        [&](parser::Return *ret) {
            if (ret->exprs.empty()) {
                result = move(node);
                return;
            }

            rewriteNodes(&ret->exprs);
            ret->exprs = rewriteNodesAsArray(node, move(ret->exprs));
            result = move(node);
        },

        // Conditions

        [&](parser::If *if_) {
            node = maybeInsertCast(move(node));
            if_->condition = rewriteNode(move(if_->condition));
            if_->then_ = rewriteBody(move(if_->then_));
            if_->else_ = rewriteBody(move(if_->else_));
            result = move(node);
        },
        [&](parser::Case *case_) {
            node = maybeInsertCast(move(node));
            case_->condition = rewriteNode(move(case_->condition));
            rewriteNodes(&case_->whens);
            case_->else_ = rewriteBody(move(case_->else_));
            result = move(node);
        },
        [&](parser::When *when) {
            when->body = rewriteBody(move(when->body));
            result = move(node);
        },
        [&](parser::CaseMatch *caseMatch) {
            node = maybeInsertCast(move(node));
            caseMatch->expr = rewriteNode(move(caseMatch->expr));
            rewriteNodes(&caseMatch->inBodies);
            caseMatch->elseBody = rewriteBody(move(caseMatch->elseBody));
            result = move(node);
        },
        [&](parser::InPattern *inPattern) {
            inPattern->pattern = rewriteNode(move(inPattern->pattern));
            inPattern->guard = rewriteNode(move(inPattern->guard));
            inPattern->body = rewriteBody(move(inPattern->body));
            result = move(node);
        },

        // Rescues

        [&](parser::Rescue *rescue) {
            rescue->body = rewriteBody(move(rescue->body));
            rewriteNodes(&rescue->rescue);
            rescue->else_ = rewriteBody(move(rescue->else_));
            result = move(node);
        },
        [&](parser::Resbody *resbody) {
            resbody->body = rewriteBody(move(resbody->body));
            result = move(node);
        },
        [&](parser::Ensure *ensure) {
            ensure->ensure = rewriteBody(move(ensure->ensure));
            ensure->body = rewriteBody(move(ensure->body));
            result = move(node);
        },

        // Others

        [&](parser::And *and_) {
            node = maybeInsertCast(move(node));
            and_->left = rewriteNode(move(and_->left));
            and_->right = rewriteNode(move(and_->right));
            result = move(node);
        },
        [&](parser::Or *or_) {
            node = maybeInsertCast(move(node));
            or_->left = rewriteNode(move(or_->left));
            or_->right = rewriteNode(move(or_->right));
            result = move(node);
        },

        [&](parser::Hash *hash) {
            node = maybeInsertCast(move(node));
            rewriteNodes(&hash->pairs);
            result = move(node);
        },
        [&](parser::Pair *pair) {
            pair->key = rewriteNode(move(pair->key));
            pair->value = rewriteNode(move(pair->value));
            result = move(node);
        },

        [&](parser::Array *arr) {
            node = maybeInsertCast(move(node));
            rewriteNodes(&arr->elts);
            result = move(node);
        },

        [&](parser::Splat *splat) {
            splat->var = rewriteNode(move(splat->var));
            result = move(node);
        },
        [&](parser::Super *super_) {
            rewriteNodes(&super_->args);
            result = maybeInsertCast(move(node));
        },
        [&](parser::Kwsplat *splat) {
            splat->expr = rewriteNode(move(splat->expr));
            result = move(node);
        },

        [&](parser::RBSPlaceholder *placeholder) {
            if (placeholder->kind == core::Names::Constants::RBSBind()) {
                result = replaceSyntheticBind(move(node));
            } else {
                Exception::raise("Unknown RBS placeholder kind");
            }
        },

        [&](parser::Node *other) { result = maybeInsertCast(move(node)); });

    return result;
}

pm_node_t *AssertionsRewriterPrism::run(pm_node_t *node) {
    // TODO: Implement actual Prism node rewriting
    // For now, just return the node unchanged
    return node;
}

} // namespace sorbet::rbs