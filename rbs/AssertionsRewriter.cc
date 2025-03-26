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

/**
 * Check if the given range contains a heredoc marker `<<-` or `<<~`
 */
bool hasHeredocMarker(core::Context ctx, const uint32_t fromPos, const uint32_t toPos) {
    string source(ctx.file.data(ctx).source().substr(fromPos, toPos - fromPos));
    regex heredoc_pattern("(\\s+=\\s<<-|~)");
    smatch matches;
    return regex_search(source, matches, heredoc_pattern);
}

/**
 * Check if the given expression is a heredoc
 */
bool isHeredoc(core::Context ctx, core::LocOffsets assignLoc, const unique_ptr<parser::Node> &node) {
    if (node == nullptr) {
        return false;
    }

    auto result = false;
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
                if (hasHeredocMarker(ctx, assignLoc.endPos(), lit->loc.beginPos())) {
                    result = true;
                }
            } else {
                // Multi-line heredoc, we look for the heredoc marker inside the string itself.
                if (hasHeredocMarker(ctx, lit->loc.beginPos(), lit->loc.endPos())) {
                    result = true;
                }
            }
        },
        [&](parser::DString *lit) {
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
                if (hasHeredocMarker(ctx, assignLoc.endPos(), lit->loc.beginPos())) {
                    result = true;
                }
            } else {
                // Multi-line heredoc, we look for the heredoc marker inside the string itself.
                if (hasHeredocMarker(ctx, lit->loc.beginPos(), lit->loc.endPos())) {
                    result = true;
                }
            }
        },
        [&](parser::Send *send) {
            result = isHeredoc(ctx, assignLoc, send->receiver) ||
                     absl::c_any_of(send->args, [&](const unique_ptr<parser::Node> &arg) {
                         return isHeredoc(ctx, assignLoc, arg);
                     });
        },
        [&](parser::Node *expr) { result = false; });

    return result;
}

optional<rbs::InlineComment> commentForPos(core::Context ctx, uint32_t fromPos) {
    auto source = ctx.file.data(ctx).source();

    // Get the position of the end of the line from the startingLoc
    auto endPos = source.find('\n', fromPos);
    if (endPos == string::npos) {
        // If we don't find a newline, we just use the rest of the file
        endPos = source.size();
    }

    if (fromPos == endPos) {
        // If the start and end of the comment are the same, we don't have a comment
        return nullopt;
    }

    // Find the position of the `#:` between the fromPos and the end of the line
    auto commentStart = source.substr(0, endPos).find("#:", fromPos);
    if (commentStart == string::npos) {
        return nullopt;
    }

    // Adjust the location to be the correct position depending on the number of spaces after the `#:`
    auto contentStart = commentStart + 2;
    auto content = source.substr(contentStart, endPos - contentStart);
    auto kind = InlineComment::Kind::LET;

    return InlineComment{
        rbs::Comment{core::LocOffsets{(uint32_t)commentStart, static_cast<uint32_t>(endPos)},
                     // core::LocOffsets{(uint32_t)contentStart, static_cast<uint32_t>(endPos)},
                     content},
        kind,
    };
}

optional<rbs::InlineComment> commentForNode(core::Context ctx, unique_ptr<parser::Node> &node,
                                            core::LocOffsets fromLoc) {
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
    if (fromLoc.exists() && isHeredoc(ctx, fromLoc, node)) {
        fromPos = fromLoc.beginPos();
    }

    return commentForPos(ctx, fromPos);
}

optional<pair<unique_ptr<parser::Node>, InlineComment::Kind>>
parseComment(core::MutableContext ctx, InlineComment comment,
             vector<pair<core::LocOffsets, core::NameRef>> typeParams) {
    if (comment.kind == InlineComment::Kind::MUST) {
        return pair<unique_ptr<parser::Node>, InlineComment::Kind>{
            make_unique<parser::Nil>(comment.comment.loc),
            comment.kind,
        };
    }

    auto signatureTranslator = rbs::SignatureTranslator(ctx);
    auto type = signatureTranslator.translateAssertionType(typeParams, comment.comment);

    if (type == nullptr) {
        return nullopt;
    }

    return pair{move(type), comment.kind};
}

unique_ptr<parser::Node> insertCast(unique_ptr<parser::Node> node,
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
    } else {
        Exception::raise("Unknown assertion kind");
    }
}

} // namespace

/**
 * Get the RBS type from the given assign
 */
optional<pair<unique_ptr<parser::Node>, InlineComment::Kind>>
AssertionsRewriter::assertionForNode(unique_ptr<parser::Node> &node, core::LocOffsets fromLoc) {
    if (auto inlineComment = commentForNode(ctx, node, fromLoc)) {
        auto typeParams = lastTypeParams();
        return parseComment(ctx, inlineComment.value(), typeParams);
    } else {
        return nullopt;
    }

    return nullopt;
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
vector<pair<core::LocOffsets, core::NameRef>> AssertionsRewriter::lastTypeParams() {
    auto typeParams = vector<pair<core::LocOffsets, core::NameRef>>();

    // Do we have a previous signature?
    if (!lastSignature) {
        return typeParams;
    }

    auto block = parser::cast_node<parser::Block>(lastSignature);
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

void AssertionsRewriter::maybeSaveSignature(parser::Block *block) {
    if (block->body == nullptr) {
        return;
    }

    auto send = parser::cast_node<parser::Send>(block->send.get());
    if (send == nullptr) {
        return;
    }

    if (send->method != core::Names::sig()) {
        return;
    }

    lastSignature = block;
}

unique_ptr<parser::Node> AssertionsRewriter::maybeInsertCast(unique_ptr<parser::Node> node) {
    if (auto type = assertionForNode(node, core::LocOffsets::none())) {
        return insertCast(move(node), move(type));
    }

    return node;
}

parser::NodeVec AssertionsRewriter::rewriteNodes(parser::NodeVec nodes) {
    auto oldStmts = move(nodes);
    auto newStmts = parser::NodeVec();

    for (auto &node : oldStmts) {
        node = rewriteNode(move(node));
        newStmts.emplace_back(std::move(node));
    }

    return newStmts;
}

unique_ptr<parser::Node> AssertionsRewriter::rewriteBegin(unique_ptr<parser::Node> node) {
    auto begin = parser::cast_node<parser::Begin>(node.get());
    ENFORCE(begin != nullptr);

    auto oldStmts = move(begin->stmts);
    begin->stmts = parser::NodeVec();

    for (auto &stmt : oldStmts) {
        // stmt = maybeInsertCast(move(stmt));
        begin->stmts.emplace_back(rewriteNode(move(stmt)));
    }

    return node;
}

unique_ptr<parser::Node> AssertionsRewriter::rewriteBody(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    if (parser::isa_node<parser::Begin>(node.get())) {
        return rewriteBegin(move(node));
    }

    // node = maybeInsertCast(move(node));
    return rewriteNode(move(node));
}

unique_ptr<parser::Node> AssertionsRewriter::rewriteNode(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    // std::cout << "rewriteNode: " << node->nodeName() << std::endl;

    unique_ptr<parser::Node> result;

    typecase(
        node.get(),
        [&](parser::Module *module) {
            module->body = rewriteBody(move(module->body));
            result = move(node);
        },
        [&](parser::Class *klass) {
            klass->body = rewriteBody(move(klass->body));
            result = move(node);
        },
        [&](parser::SClass *sclass) {
            sclass->body = rewriteBody(move(sclass->body));
            result = move(node);
        },
        [&](parser::DefMethod *method) {
            method->body = rewriteBody(move(method->body));
            result = move(node);
        },
        [&](parser::DefS *method) {
            method->body = rewriteBody(move(method->body));
            result = move(node);
        },
        [&](parser::Begin *begin) {
            //
            result = rewriteBegin(move(node));
        },
        [&](parser::Block *block) {
            maybeSaveSignature(block);
            block->body = rewriteBody(move(block->body));
            result = move(node);
        },

        // Assigns

        [&](parser::Assign *asgn) {
            if (auto comment = commentForNode(ctx, asgn->rhs, asgn->lhs->loc)) {
                if (auto type = parseComment(ctx, comment.value(), lastTypeParams())) {
                    asgn->rhs = insertCast(move(asgn->rhs), move(type));
                }
            } else {
                asgn->rhs = rewriteNode(move(asgn->rhs));
            }
            result = move(node);
        },
        [&](parser::AndAsgn *andAsgn) {
            if (auto comment = commentForNode(ctx, andAsgn->right, andAsgn->left->loc)) {
                if (auto type = parseComment(ctx, comment.value(), lastTypeParams())) {
                    andAsgn->right = insertCast(move(andAsgn->right), move(type));
                }
            } else {
                andAsgn->right = rewriteNode(move(andAsgn->right));
            }
            result = move(node);
        },
        [&](parser::OpAsgn *opAsgn) {
            if (auto comment = commentForNode(ctx, opAsgn->right, opAsgn->left->loc)) {
                if (auto type = parseComment(ctx, comment.value(), lastTypeParams())) {
                    opAsgn->right = insertCast(move(opAsgn->right), move(type));
                }
            } else {
                opAsgn->right = rewriteNode(move(opAsgn->right));
            }
            result = move(node);
        },
        [&](parser::OrAsgn *orAsgn) {
            if (auto comment = commentForNode(ctx, orAsgn->right, orAsgn->left->loc)) {
                if (auto type = parseComment(ctx, comment.value(), lastTypeParams())) {
                    orAsgn->right = insertCast(move(orAsgn->right), move(type));
                }
            } else {
                orAsgn->right = rewriteNode(move(orAsgn->right));
            }
            result = move(node);
        },
        [&](parser::Masgn *masgn) {
            if (auto comment = commentForNode(ctx, masgn->rhs, masgn->lhs->loc)) {
                if (auto type = parseComment(ctx, comment.value(), lastTypeParams())) {
                    masgn->rhs = insertCast(move(masgn->rhs), move(type));
                }
            } else {
                masgn->rhs = rewriteNode(move(masgn->rhs));
            }
            result = move(node);
        },

        // Others

        [&](parser::Send *send) {
            send->receiver = rewriteNode(move(send->receiver));
            send->args = rewriteNodes(move(send->args));
            result = move(node);
        },
        [&](parser::Hash *hash) {
            hash->pairs = rewriteNodes(move(hash->pairs));
            result = move(node);
        },

        [&](parser::Return *ret) {
            ret->exprs = rewriteNodes(move(ret->exprs));
            result = move(node);
        },
        [&](parser::Break *break_) {
            break_->exprs = rewriteNodes(move(break_->exprs));
            result = move(node);
        },
        [&](parser::Next *next) {
            next->exprs = rewriteNodes(move(next->exprs));
            result = move(node);
        },

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
        [&](parser::AndAsgn *andAsgn) {
            andAsgn->right = rewriteNode(move(andAsgn->right));
            result = move(node);
        },
        [&](parser::OrAsgn *orAsgn) {
            orAsgn->right = rewriteNode(move(orAsgn->right));
            result = move(node);
        },
        [&](parser::OpAsgn *opAsgn) {
            opAsgn->right = rewriteNode(move(opAsgn->right));
            result = move(node);
        },
        [&](parser::CSend *csend) {
            csend->receiver = rewriteNode(move(csend->receiver));
            csend->args = rewriteNodes(move(csend->args));
            result = move(node);
        },
        [&](parser::Kwbegin *kwbegin) {
            kwbegin->stmts = rewriteNodes(move(kwbegin->stmts));
            result = move(node);
        },
        [&](parser::When *when) {
            when->body = rewriteBody(move(when->body));
            result = move(node);
        },
        [&](parser::While *wl) {
            wl->body = rewriteBody(move(wl->body));
            result = move(node);
        },
        [&](parser::WhilePost *wl) {
            wl->body = rewriteBody(move(wl->body));
            result = move(node);
        },
        [&](parser::Until *until) {
            until->body = rewriteBody(move(until->body));
            result = move(node);
        },
        [&](parser::UntilPost *until) {
            until->body = rewriteBody(move(until->body));
            result = move(node);
        },
        [&](parser::For *for_) {
            for_->body = rewriteBody(move(for_->body));
            result = move(node);
        },
        [&](parser::Rescue *rescue) {
            rescue->body = rewriteBody(move(rescue->body));
            rescue->rescue = rewriteNodes(move(rescue->rescue));
            rescue->else_ = rewriteBody(move(rescue->else_));
            result = move(node);
        },
        [&](parser::Resbody *resbody) {
            resbody->body = rewriteBody(move(resbody->body));
            result = move(node);
        },
        [&](parser::Ensure *ensure) {
            ensure->body = rewriteBody(move(ensure->body));
            result = move(node);
        },
        [&](parser::If *if_) {
            if_->then_ = rewriteBody(move(if_->then_));
            if_->else_ = rewriteBody(move(if_->else_));
            result = move(node);
        },
        [&](parser::Masgn *masgn) {
            masgn->rhs = rewriteNode(move(masgn->rhs));
            result = move(node);
        },
        [&](parser::Case *case_) {
            case_->whens = rewriteNodes(move(case_->whens));
            case_->else_ = rewriteBody(move(case_->else_));
            result = move(node);
        },
        [&](parser::Node *other) { result = move(node); });

    return result;
}

unique_ptr<parser::Node> AssertionsRewriter::run(unique_ptr<parser::Node> node) {
    if (node == nullptr) {
        return node;
    }

    return rewriteBody(move(node));
}

} // namespace sorbet::rbs
