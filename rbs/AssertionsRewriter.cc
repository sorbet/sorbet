#include "rbs/AssertionsRewriter.h"

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

const regex not_nil_pattern("^\\s*!nil\\s*(#.*)?$");
const regex untyped_pattern("^\\s*untyped\\s*(#.*)?$");

/**
 * Check if the given range is the start of a heredoc assignment `= <<~FOO` and return the position of the end of the
 * heredoc marker.
 *
 * Returns -1 if no heredoc marker is found.
 */
uint32_t hasHeredocMarker(core::Context ctx, const uint32_t fromPos, const uint32_t toPos) {
    string source(ctx.file.data(ctx).source().substr(fromPos, toPos - fromPos));
    regex heredoc_pattern("\\s*=?\\s*<<(-|~)[^,\\s\\n#]+(,\\s*<<(-|~)[^,\\s\\n#]+)*");
    smatch match;
    if (regex_search(source, match, heredoc_pattern)) {
        return fromPos + match.length();
    }
    return UINT32_MAX;
}

/**
 * Check if the given expression is a heredoc
 */
uint32_t heredocPos(core::Context ctx, core::LocOffsets assignLoc, const unique_ptr<parser::Node> &node) {
    if (node == nullptr) {
        return UINT32_MAX;
    }

    uint32_t result = UINT32_MAX;
    typecase(
        node.get(),
        [&](parser::String *lit) {
            // For some reason, heredoc strings are parser differently if they contain a single line or more.
            //
            // Single line heredocs do not contain the `<<-` or `<<~` markers inside their location.
            //
            // For example, this heredoc:
            //
            //     <<~MSG
            //       foo
            //     MSG

            // has the `<<-` or `<<~` markers **outside** its location.
            //
            // While this heredoc:
            //
            //     <<~MSG
            //       foo
            //     MSG
            //
            // has the `<<-` or `<<~` markers **inside** its location.

            auto lineStart = core::Loc::pos2Detail(ctx.file.data(ctx), lit->loc.beginLoc).line;
            auto lineEnd = core::Loc::pos2Detail(ctx.file.data(ctx), lit->loc.endLoc).line;

            if (lineEnd - lineStart <= 1) {
                // Single line heredoc, we look for the heredoc marker outside, ie. between the assign `=` sign
                // and the begining of the string.
                result = hasHeredocMarker(ctx, assignLoc.endPos(), lit->loc.beginPos());
            } else {
                // Multi-line heredoc, we look for the heredoc marker inside the string itself.
                result = hasHeredocMarker(ctx, lit->loc.beginPos(), lit->loc.endPos());
            }
        },
        [&](parser::DString *lit) { result = hasHeredocMarker(ctx, lit->loc.beginPos(), lit->loc.endPos()); },
        [&](parser::Array *arr) {
            for (auto &elem : arr->elts) {
                result = heredocPos(ctx, assignLoc, elem);
                if (result != UINT32_MAX) {
                    break;
                }
            }
        },
        [&](parser::Send *send) { result = heredocPos(ctx, assignLoc, send->receiver); },
        [&](parser::Node *expr) { result = false; });

    return result;
}

/*
 * Parse the comment and return the type as a `parser::Node` and the kind of assertion we need to apply (let, cast,
 * must, unsafe).
 *
 * The `typeParams` param is used to resolve the type parameters from the method signature.
 *
 * Returns `nullopt` if the comment is not a valid RBS expression (an error is produced).
 */
optional<pair<unique_ptr<parser::Node>, InlineComment::Kind>>
parseComment(core::MutableContext ctx, InlineComment comment,
             vector<pair<core::LocOffsets, core::NameRef>> typeParams) {
    if (comment.kind == InlineComment::Kind::MUST) {
        return pair<unique_ptr<parser::Node>, InlineComment::Kind>{
            // The type should never be used but we need to hold the location...
            make_unique<parser::Nil>(comment.comment.typeLoc),
            comment.kind,
        };
    } else if (comment.kind == InlineComment::Kind::UNSAFE) {
        return pair<unique_ptr<parser::Node>, InlineComment::Kind>{
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
vector<pair<core::LocOffsets, core::NameRef>> extractTypeParams(parser::Node *sig) {
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

} // namespace

/**
 * Mark the given comment location as "consumed" so it won't be picked up by subsequent calls to `commentForPos`.
 */
void AssertionsRewriter::consumeComment(core::LocOffsets loc) {
    consumedComments.emplace(make_pair(loc.beginPos(), loc.endPos()));
}

/**
 * Check if the given comment location has been consumed.
 */
bool AssertionsRewriter::hasConsumedComment(core::LocOffsets loc) {
    return consumedComments.count(make_pair(loc.beginPos(), loc.endPos()));
}

/*
 * Get the RBS comment for the given position.
 *
 * This function looks up for a comment starting at the given position until the end of the line.
 *
 * Returns `nullopt` if no comment is found.
 *
 * This function will mark the comment location as consumed so it won't be picked up by subsequent calls to this
 * function. This is to avoid applying the same comment multiple times:
 *
 *     foo x #: as !nil
 *
 * We only want to apply the `as !nil` comment on the `foo` call and not on the `x` argument.
 */
optional<rbs::InlineComment> AssertionsRewriter::commentForPos(uint32_t fromPos, vector<char> allowedTokens = {}) {
    auto source = ctx.file.data(ctx).source();

    // Get the position of the end of the line from the startingLoc
    auto endPos = source.find('\n', fromPos);
    if (endPos == string::npos) {
        // If we don't find a newline, we just use the rest of the file
        endPos = source.size();
    }

    if (fromPos == endPos) {
        // We reached the end of the line, we don't have a comment
        return nullopt;
    }

    auto commentStart = fromPos;
    while (commentStart < endPos) {
        char c = source[commentStart];
        if (c == ' ') {
            // Skip whitespace until we find a `#:`
            commentStart++;
            continue;
        } else if (find(allowedTokens.begin(), allowedTokens.end(), c) != allowedTokens.end()) {
            // Skip allowed tokens like `,` or `.` depending on the callsite
            commentStart++;
            continue;
        } else if (c == '#' && commentStart + 1 < endPos && source[commentStart + 1] == ':') {
            // We found a `#:`
            break;
        } else {
            // We found a character that is not a space or a `#`, so we don't have a comment
            return nullopt;
        }
    }

    if (commentStart == endPos) {
        // We didn't find a `#:`
        return nullopt;
    }

    // Consume the spaces after the `#:`
    auto contentStart = commentStart + 2;
    char c = source[contentStart];
    while (c == ' ' && contentStart < endPos) {
        contentStart++;
        c = source[contentStart];
    }

    auto content = source.substr(contentStart, endPos - contentStart);
    auto kind = InlineComment::Kind::LET;

    if (absl::StartsWith(content, "as ")) {
        // We found a `as` keyword, this is a `T.cast` comment
        kind = InlineComment::Kind::CAST;
        contentStart += 3;
        content = content.substr(3);

        if (regex_match(content.begin(), content.end(), not_nil_pattern)) {
            // We found a `as !nil`, so a `T.must` comment
            kind = InlineComment::Kind::MUST;
        } else if (regex_match(content.begin(), content.end(), untyped_pattern)) {
            // We found a `as untyped`, so a `T.unsafe` comment
            kind = InlineComment::Kind::UNSAFE;
        }
    }

    auto commentLoc = core::LocOffsets{(uint32_t)commentStart, static_cast<uint32_t>(endPos)};

    // If we already consumed this comment, we don't return it since it's been "consumed" already
    if (hasConsumedComment(commentLoc)) {
        return nullopt;
    }

    // We consume the comment so it won't be picked up by subsequent calls to this function
    consumeComment(commentLoc);

    return InlineComment{
        rbs::Comment{
            commentLoc,
            core::LocOffsets{(uint32_t)contentStart, static_cast<uint32_t>(endPos)},
            content,
        },
        kind,
    };
}

/*
 * Get the RBS comment for the given node.
 *
 * Returns `nullopt` if no comment is found or if the comment was already consumed.
 *
 * The `fromLoc` param is used to detect heredocs if any. Consider this example:
 *
 *     foo = <<~MSG
 *       bar
 *     MSG
 *
 * Since heredocs are parsed differently, we need to special case them. Using `fromLoc` as the end of the `foo` lhs
 * location, we can detect the heredoc between the `=` sign and the end of the line.
 *
 * This hack won't be necessary once we migrate the parsing to Prism.
 */
optional<rbs::InlineComment> AssertionsRewriter::commentForNode(unique_ptr<parser::Node> &node,
                                                                core::LocOffsets fromLoc,
                                                                vector<char> allowedTokens = {}) {
    // We want to find the comment right after the end of the assign
    auto fromPos = node->loc.endPos();

    // On heredocs, adding the comment at the end of the assign won't work because this is invalid Ruby syntax:
    // ```
    // <<~MSG
    //   foo
    // MSG #: String
    // ```
    // We add a special case for heredocs to allow adding the comment at the end of the assign:
    // ```
    // <<~MSG #: String
    //   foo
    // MSG
    // ```
    if (fromLoc.exists()) {
        if (auto pos = heredocPos(ctx, fromLoc, node)) {
            if (pos != -1) {
                fromPos = pos;
            }
        }
    }

    return commentForPos(fromPos, allowedTokens);
}

void AssertionsRewriter::checkDanglingCommentWithDecl(uint32_t nodeEnd, uint32_t declEnd, string kind) {
    if (auto assertion = commentForPos(nodeEnd)) {
        if (auto e = ctx.beginError(assertion.value().comment.commentLoc, core::errors::Rewriter::RBSAssertionError)) {
            e.setHeader("Unexpected RBS assertion comment found after `{}` end", kind);
        }
    }

    auto decLine = core::Loc::pos2Detail(ctx.file.data(ctx), declEnd).line;
    auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), nodeEnd).line;

    if ((endLine > decLine)) {
        if (auto assertion = commentForPos(declEnd)) {
            if (auto e =
                    ctx.beginError(assertion.value().comment.commentLoc, core::errors::Rewriter::RBSAssertionError)) {
                e.setHeader("Unexpected RBS assertion comment found after `{}` declaration", kind);
            }
        }
    }
}

void AssertionsRewriter::checkDanglingComment(uint32_t nodeEnd, string kind) {
    if (auto assertion = commentForPos(nodeEnd)) {
        if (auto e = ctx.beginError(assertion.value().comment.commentLoc, core::errors::Rewriter::RBSAssertionError)) {
            e.setHeader("Unexpected RBS assertion comment found after `{}`", kind);
        }
    }
}

/**
 * Save the signature from the given block so it can be used to resolve the type parameters from the method signature.
 *
 * Returns `true` if the block is a `sig` send, `false` otherwise.
 */
bool AssertionsRewriter::saveTypeParams(parser::Block *block) {
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

    typeParams = extractTypeParams(block);

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
AssertionsRewriter::insertCast(unique_ptr<parser::Node> node,
                               optional<pair<unique_ptr<parser::Node>, InlineComment::Kind>> pair) {
    if (!pair) {
        return node;
    }

    auto type = move(pair->first);
    auto kind = pair->second;

    if (kind == InlineComment::Kind::LET) {
        return parser::MK::TLet(type->loc, move(node), move(type));
    } else if (kind == InlineComment::Kind::CAST) {
        return parser::MK::TCast(type->loc, move(node), move(type));
    } else if (kind == InlineComment::Kind::MUST) {
        return parser::MK::TMust(type->loc, move(node));
    } else if (kind == InlineComment::Kind::UNSAFE) {
        return parser::MK::TUnsafe(type->loc, move(node));
    } else {
        Exception::raise("Unknown assertion kind");
    }
}

/**
 * Insert a cast into the given node if there is an not yet consumed RBS assertion comment after the node.
 */
unique_ptr<parser::Node> AssertionsRewriter::maybeInsertCast(unique_ptr<parser::Node> node,
                                                             core::LocOffsets assignLoc = core::LocOffsets::none(),
                                                             vector<char> allowedTokens = {}) {
    if (node == nullptr) {
        return node;
    }

    if (auto inlineComment = commentForNode(node, assignLoc, allowedTokens)) {
        if (auto type = parseComment(ctx, inlineComment.value(), typeParams)) {
            return insertCast(move(node), move(type));
        }
    }

    return node;
}

/**
 * Rewrite a collection of nodes, wrap them in an array and cast the array.
 */
parser::NodeVec AssertionsRewriter::rewriteNodesAsArray(parser::NodeVec nodes) {
    if (nodes.size() == 1) {
        rewriteNodes(&nodes);
        return nodes;
    }

    auto loc = core::LocOffsets{
        nodes.front()->loc.beginPos(),
        nodes.back()->loc.endPos(),
    };
    auto arr = parser::MK::Array(loc, move(nodes));
    arr = rewriteNode(move(arr));

    auto vector = parser::NodeVec();
    vector.emplace_back(move(arr));
    return vector;
}

/**
 * Rewrite a collection of nodes in place.
 */
void AssertionsRewriter::rewriteNodes(parser::NodeVec *nodes) {
    for (auto &node : *nodes) {
        node = rewriteNode(move(node));
    }
}

/**
 * Rewrite the body of a node.
 */
unique_ptr<parser::Node> AssertionsRewriter::rewriteBody(unique_ptr<parser::Node> node) {
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
unique_ptr<parser::Node> AssertionsRewriter::rewriteNode(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    unique_ptr<parser::Node> result;

    typecase(
        node.get(),

        // Scopes

        [&](parser::Module *module) {
            module->body = rewriteBody(move(module->body));
            checkDanglingCommentWithDecl(module->loc.endPos(), module->declLoc.endPos(), "module");
            result = move(node);

            typeParams.clear();
        },
        [&](parser::Class *klass) {
            klass->body = rewriteBody(move(klass->body));
            checkDanglingCommentWithDecl(klass->loc.endPos(), klass->declLoc.endPos(), "class");
            result = move(node);

            typeParams.clear();
        },
        [&](parser::SClass *sclass) {
            sclass->body = rewriteBody(move(sclass->body));
            checkDanglingCommentWithDecl(sclass->loc.endPos(), sclass->declLoc.endPos() + 8, "sclass");
            result = move(node);

            typeParams.clear();
        },
        [&](parser::DefMethod *method) {
            method->body = rewriteBody(move(method->body));
            checkDanglingCommentWithDecl(method->loc.endPos(), method->declLoc.endPos(), "method");
            result = move(node);

            typeParams.clear();
        },
        [&](parser::DefS *method) {
            method->body = rewriteBody(move(method->body));
            checkDanglingCommentWithDecl(method->loc.endPos(), method->declLoc.endPos(), "method");
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
            asgn->rhs = maybeInsertCast(move(asgn->rhs), asgn->lhs->loc);
            asgn->rhs = rewriteNode(move(asgn->rhs));
            result = move(node);
        },
        [&](parser::AndAsgn *andAsgn) {
            andAsgn->left = rewriteNode(move(andAsgn->left));
            andAsgn->right = maybeInsertCast(move(andAsgn->right), andAsgn->left->loc);
            andAsgn->right = rewriteNode(move(andAsgn->right));
            result = move(node);
        },
        [&](parser::OpAsgn *opAsgn) {
            opAsgn->left = rewriteNode(move(opAsgn->left));
            opAsgn->right = maybeInsertCast(move(opAsgn->right), opAsgn->left->loc);
            opAsgn->right = rewriteNode(move(opAsgn->right));
            result = move(node);
        },
        [&](parser::OrAsgn *orAsgn) {
            orAsgn->left = rewriteNode(move(orAsgn->left));
            orAsgn->right = maybeInsertCast(move(orAsgn->right), orAsgn->left->loc);
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
            send->receiver = maybeInsertCast(move(send->receiver), core::LocOffsets::none(), {'.'});
            send->receiver = rewriteNode(move(send->receiver));

            if (send->method.isSetter(ctx.state) && send->args.size() == 1) {
                // This is a `foo.x=(y)` method, we treat it as a `x = y` assignment
                send->args[0] = maybeInsertCast(move(send->args[0]));
                result = move(node);
                return;
            }

            node = maybeInsertCast(move(node));

            for (auto &arg : send->args) {
                arg = rewriteNode(move(arg));

                if (auto splat = parser::cast_node<parser::Splat>(arg.get())) {
                    splat->var = maybeInsertCast(move(splat->var), core::LocOffsets::none(), {','});
                } else if (auto kwsplat = parser::cast_node<parser::Kwsplat>(arg.get())) {
                    kwsplat->expr = maybeInsertCast(move(kwsplat->expr), core::LocOffsets::none(), {','});
                } else {
                    arg = maybeInsertCast(move(arg), core::LocOffsets::none(), {','});
                }
            }

            result = move(node);
        },
        [&](parser::CSend *csend) {
            csend->receiver = maybeInsertCast(move(csend->receiver), core::LocOffsets::none(), {'&', '.'});
            csend->receiver = rewriteNode(move(csend->receiver));

            if (csend->method.isSetter(ctx.state) && csend->args.size() == 1) {
                // This is a `foo&.x=(y)` method, we treat it as a `x = y` assignment
                csend->args[0] = maybeInsertCast(move(csend->args[0]));
                result = move(node);
                return;
            }

            node = maybeInsertCast(move(node));

            for (auto &arg : csend->args) {
                arg = rewriteNode(move(arg));

                if (auto splat = parser::cast_node<parser::Splat>(arg.get())) {
                    splat->var = maybeInsertCast(move(splat->var), core::LocOffsets::none(), {','});
                } else if (auto kwsplat = parser::cast_node<parser::Kwsplat>(arg.get())) {
                    kwsplat->expr = maybeInsertCast(move(kwsplat->expr), core::LocOffsets::none(), {','});
                } else {
                    arg = maybeInsertCast(move(arg), core::LocOffsets::none(), {','});
                }
            }

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
                checkDanglingComment(break_->loc.endPos(), "break");
                result = move(node);
                return;
            }

            break_->exprs = rewriteNodesAsArray(move(break_->exprs));
            result = move(node);
        },
        [&](parser::Next *next) {
            if (next->exprs.empty()) {
                checkDanglingComment(next->loc.endPos(), "next");
                result = move(node);
                return;
            }

            next->exprs = rewriteNodesAsArray(move(next->exprs));
            result = move(node);
        },
        [&](parser::Return *ret) {
            if (ret->exprs.empty()) {
                checkDanglingComment(ret->loc.endPos(), "return");
                result = move(node);
                return;
            }

            ret->exprs = rewriteNodesAsArray(move(ret->exprs));
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

        // Rescues

        [&](parser::Rescue *rescue) {
            rescue->body = rewriteBody(move(rescue->body));
            rewriteNodes(&rescue->rescue);
            rescue->else_ = rewriteBody(move(rescue->else_));
            result = move(node);
        },
        [&](parser::Resbody *resbody) {
            if (resbody->var) {
                checkDanglingComment(resbody->var->loc.endPos(), "rescue");
            } else if (resbody->exception) {
                checkDanglingComment(resbody->exception->loc.endPos(), "rescue");
            } else {
                checkDanglingComment(resbody->loc.beginPos() + 6, "rescue");
            }
            resbody->body = rewriteBody(move(resbody->body));
            result = move(node);
        },
        [&](parser::Ensure *ensure) {
            ensure->body = rewriteBody(move(ensure->body));
            result = move(node);
        },

        // Others

        [&](parser::And *and_) {
            and_->left = rewriteNode(move(and_->left));
            and_->right = rewriteNode(move(and_->right));
            result = move(node);
        },
        [&](parser::Or *or_) {
            or_->left = rewriteNode(move(or_->left));
            or_->right = rewriteNode(move(or_->right));
            result = move(node);
        },

        [&](parser::Hash *hash) {
            node = maybeInsertCast(move(node));

            for (auto &elem : hash->pairs) {
                elem = rewriteNode(move(elem));

                if (auto pair = parser::cast_node<parser::Pair>(elem.get())) {
                    pair->value = maybeInsertCast(move(pair->value), core::LocOffsets::none(), {','});
                } else if (auto splat = parser::cast_node<parser::Splat>(elem.get())) {
                    splat->var = maybeInsertCast(move(splat->var), core::LocOffsets::none(), {','});
                } else if (auto kwsplat = parser::cast_node<parser::Kwsplat>(elem.get())) {
                    kwsplat->expr = maybeInsertCast(move(kwsplat->expr), core::LocOffsets::none(), {','});
                } else {
                    continue;
                }
            }

            result = move(node);
        },
        [&](parser::Pair *pair) {
            pair->key = rewriteNode(move(pair->key));
            pair->value = rewriteNode(move(pair->value));
            result = move(node);
        },

        [&](parser::Array *arr) {
            node = maybeInsertCast(move(node));

            for (auto &elem : arr->elts) {
                elem = rewriteNode(move(elem));

                if (auto splat = parser::cast_node<parser::Splat>(elem.get())) {
                    splat->var = maybeInsertCast(move(splat->var), core::LocOffsets::none(), {','});
                } else if (auto kwsplat = parser::cast_node<parser::Kwsplat>(elem.get())) {
                    kwsplat->expr = maybeInsertCast(move(kwsplat->expr), core::LocOffsets::none(), {','});
                } else {
                    elem = maybeInsertCast(move(elem), core::LocOffsets::none(), {','});
                }
            }

            result = move(node);
        },

        [&](parser::Splat *splat) {
            splat->var = rewriteNode(move(splat->var));
            result = move(node);
        },
        [&](parser::Kwsplat *splat) {
            splat->expr = rewriteNode(move(splat->expr));
            result = move(node);
        },

        [&](parser::Node *other) { result = maybeInsertCast(move(node)); });

    return result;
}

unique_ptr<parser::Node> AssertionsRewriter::run(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    return rewriteBody(move(node));
}

} // namespace sorbet::rbs
