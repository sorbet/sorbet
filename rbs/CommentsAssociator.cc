#include "rbs/CommentsAssociator.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "rbs/SignatureTranslator.h"
#include <regex>

using namespace std;

namespace sorbet::rbs {

const string_view CommentsAssociator::RBS_PREFIX = "#:";
const string_view CommentsAssociator::ANNOTATION_PREFIX = "# @";
const string_view CommentsAssociator::MULTILINE_RBS_PREFIX = "#|";

// Static regex pattern to avoid recompilation
static const regex HEREDOC_PATTERN("\\s*=?\\s*<<(-|~)[^,\\s\\n#]+(,\\s*<<(-|~)[^,\\s\\n#]+)*");

/**
 * Check if the given range is the start of a heredoc assignment `= <<~FOO` and return the position of the end of the
 * heredoc marker.
 *
 * Returns -1 if no heredoc marker is found.
 */
optional<uint32_t> hasHeredocMarker(core::Context ctx, const uint32_t fromPos, const uint32_t toPos) {
    string_view source(ctx.file.data(ctx).source().substr(fromPos, toPos - fromPos));

    string source_str(source);
    smatch match;
    if (regex_search(source_str, HEREDOC_PATTERN)) {
        return fromPos + source_str.length();
    }

    return nullopt;
}

optional<uint32_t> CommentsAssociator::locateTargetLine(parser::Node *node) {
    optional<uint32_t> result = nullopt;

    if (node == nullptr) {
        return result;
    }

    typecase(
        node,
        [&](parser::String *lit) {
            if (hasHeredocMarker(ctx, lit->loc.beginPos(), lit->loc.endPos())) {
                result = core::Loc::pos2Detail(ctx.file.data(ctx), lit->loc.beginPos()).line;
            }
        },
        [&](parser::DString *lit) {
            if (hasHeredocMarker(ctx, lit->loc.beginPos(), lit->loc.endPos())) {
                result = core::Loc::pos2Detail(ctx.file.data(ctx), lit->loc.beginPos()).line;
            }
        },
        [&](parser::Array *arr) {
            for (auto &elem : arr->elts) {
                if (auto line = locateTargetLine(elem.get())) {
                    result = *line;
                    break;
                }
            }
        },
        [&](parser::Send *send) { result = locateTargetLine(send->receiver.get()); }, [&](parser::Node *expr) {});

    return result;
}

void CommentsAssociator::consumeCommentsInsideNode(parser::Node *node, string kind) {
    auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
    auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
    consumeCommentsBetweenLines(beginLine, endLine, kind);
}

void CommentsAssociator::consumeCommentsBetweenLines(int startLine, int endLine, string kind) {
    auto it = commentByLine.begin();
    while (it != commentByLine.end() && it->first < startLine) {
        ++it;
    }
    if (it == commentByLine.end()) {
        return;
    }
    auto startIt = it;

    while (it != commentByLine.end()) {
        if (it->first < endLine) {
            if (absl::StartsWith(it->second.string, RBS_PREFIX) ||
                absl::StartsWith(it->second.string, MULTILINE_RBS_PREFIX)) {
                if (it->first == startLine) {
                    if (auto e = ctx.beginError(it->second.loc, core::errors::Rewriter::RBSUnusedComment)) {
                        e.setHeader("Unexpected RBS assertion comment found after `{}` declaration", kind);
                    }
                } else {
                    if (auto e = ctx.beginError(it->second.loc, core::errors::Rewriter::RBSUnusedComment)) {
                        e.setHeader("Unexpected RBS assertion comment found in `{}`", kind);
                    }
                }
            }
        } else if (it->first == endLine) {
            if (absl::StartsWith(it->second.string, RBS_PREFIX) ||
                absl::StartsWith(it->second.string, MULTILINE_RBS_PREFIX)) {
                if (auto e = ctx.beginError(it->second.loc, core::errors::Rewriter::RBSAssertionError)) {
                    e.setHeader("Unexpected RBS assertion comment found after `{}` end", kind);
                }
            }
        } else {
            break;
        }
        ++it;
    }
    commentByLine.erase(startIt, it);
}

void CommentsAssociator::consumeCommentsUntilLine(int line) {
    auto it = commentByLine.begin();
    while (it != commentByLine.end()) {
        if (it->first < line) {
            if (absl::StartsWith(it->second.string, RBS_PREFIX) ||
                absl::StartsWith(it->second.string, MULTILINE_RBS_PREFIX)) {
                if (auto e = ctx.beginIndexerError(it->second.loc, core::errors::Rewriter::RBSUnusedComment)) {
                    e.setHeader("Unused RBS signature comment. No method definition found after it");
                }
            }
            ++it;
        } else {
            break;
        }
    }
    commentByLine.erase(commentByLine.begin(), it);
}

void CommentsAssociator::associateAssertionCommentsToNode(parser::Node *node, bool adjustLocForHeredoc = false) {
    uint32_t targetLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
    if (adjustLocForHeredoc) {
        if (auto line = locateTargetLine(node)) {
            targetLine = *line;
        }
    }

    vector<CommentNode> comments;

    auto it = commentByLine.find(targetLine);
    if (it != commentByLine.end() && absl::StartsWith(it->second.string, RBS_PREFIX)) {
        comments.emplace_back(it->second);
        commentByLine.erase(it);

        commentsByNode[node] = move(comments);
    }
}

void CommentsAssociator::associateSignatureCommentsToNode(parser::Node *node) {
    auto nodeStartLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;

    vector<CommentNode> comments;

    for (auto it = commentByLine.begin(); it != commentByLine.end();) {
        auto commentLine = it->first;

        if (commentLine >= nodeStartLine) {
            break;
        }

        if (absl::StartsWith(it->second.string, RBS_PREFIX) ||
            absl::StartsWith(it->second.string, MULTILINE_RBS_PREFIX) ||
            absl::StartsWith(it->second.string, ANNOTATION_PREFIX)) {
            comments.emplace_back(it->second);
            it = commentByLine.erase(it);
            continue;
        }

        it++;
    }

    commentsByNode[node] = move(comments);
}

void CommentsAssociator::walkNodes(parser::Node *node) {
    if (node == nullptr) {
        return;
    }

    typecase(
        node,

        [&](parser::And *and_) {
            associateAssertionCommentsToNode(node);
            walkNodes(and_->right.get());
            walkNodes(and_->left.get());
            consumeCommentsInsideNode(node, "and");
        },
        [&](parser::AndAsgn *andAsgn) {
            associateAssertionCommentsToNode(andAsgn->right.get(), true);
            walkNodes(andAsgn->right.get());
            consumeCommentsInsideNode(node, "and_asgn");
        },
        [&](parser::Array *array) {
            associateAssertionCommentsToNode(node);
            for (auto &elem : array->elts) {
                walkNodes(elem.get());
            }
            consumeCommentsInsideNode(node, "array");
        },
        [&](parser::Assign *assign) {
            associateAssertionCommentsToNode(assign->rhs.get(), true);
            walkNodes(assign->rhs.get());
            consumeCommentsInsideNode(node, "assign");
        },
        [&](parser::Begin *begin) {
            // This is a workaround that will be removed once we migrate to prism. We need to differentiate between
            // implicit and explicit begin nodes.
            //
            // (let4 &&= "foo") #: Integer
            // vs
            // take_block { |x|
            //   puts x
            //   x #: as String
            // }
            if (begin->stmts.size() > 0 && begin->stmts[0]->loc.endPos() + 1 == node->loc.endPos()) {
                associateAssertionCommentsToNode(node);
            }
            for (auto &stmt : begin->stmts) {
                walkNodes(stmt.get());
            }
            consumeCommentsInsideNode(node, "begin");
        },
        [&](parser::Block *block) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            consumeCommentsUntilLine(beginLine);

            associateAssertionCommentsToNode(node);
            walkNodes(block->send.get());
            walkNodes(block->body.get());
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsBetweenLines(beginLine, endLine, "block");
        },
        [&](parser::Break *break_) {
            // Only associate comments if the last expression is on the same line as the break
            if (!break_->exprs.empty()) {
                auto breakLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
                auto lastExprLine =
                    core::Loc::pos2Detail(ctx.file.data(ctx), break_->exprs.back()->loc.beginPos()).line;
                if (lastExprLine == breakLine) {
                    associateAssertionCommentsToNode(node);
                }
            }

            for (auto it = break_->exprs.rbegin(); it != break_->exprs.rend(); ++it) {
                walkNodes(it->get());
            }
            consumeCommentsInsideNode(node, "break");
        },
        [&](parser::Case *case_) {
            associateAssertionCommentsToNode(node);
            walkNodes(case_->condition.get());
            for (auto &when : case_->whens) {
                walkNodes(when.get());
            }
            walkNodes(case_->else_.get());
            consumeCommentsInsideNode(node, "case");
        },
        [&](parser::CaseMatch *case_) {
            associateAssertionCommentsToNode(node);
            walkNodes(case_->expr.get());
            for (auto &inBody : case_->inBodies) {
                walkNodes(inBody.get());
            }
            walkNodes(case_->elseBody.get());
            consumeCommentsInsideNode(node, "case");
        },
        [&](parser::Class *cls) {
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            consumeCommentsUntilLine(beginLine);

            if (auto body = cls->body.get()) {
                walkNodes(body);
            }
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsBetweenLines(beginLine, endLine, "class");
        },
        [&](parser::CSend *csend) {
            if (csend->method.isSetter(ctx.state) && csend->args.size() == 1) {
                // This is a `foo&.x=(y)` method, we treat it as a `x = y` assignment
                associateAssertionCommentsToNode(csend->args[0].get());
                return;
            }
            associateAssertionCommentsToNode(node);
            walkNodes(csend->receiver.get());
            for (auto &arg : csend->args) {
                walkNodes(arg.get());
            }
            consumeCommentsInsideNode(node, "csend");
        },
        [&](parser::DefMethod *def) {
            associateSignatureCommentsToNode(node);
            walkNodes(def->body.get());
            consumeCommentsInsideNode(node, "method");
        },
        [&](parser::DefS *def) {
            associateSignatureCommentsToNode(node);
            walkNodes(def->body.get());
            consumeCommentsInsideNode(node, "method");
        },
        [&](parser::Ensure *ensure) {
            walkNodes(ensure->body.get());
            walkNodes(ensure->ensure.get());
            consumeCommentsInsideNode(node, "ensure");
        },
        [&](parser::For *for_) {
            associateAssertionCommentsToNode(node);
            walkNodes(for_->expr.get());
            walkNodes(for_->vars.get());
            walkNodes(for_->body.get());
            consumeCommentsInsideNode(node, "for");
        },
        [&](parser::Hash *hash) {
            if (!hash->kwargs) {
                associateAssertionCommentsToNode(node);
            }
            for (auto &elem : hash->pairs) {
                walkNodes(elem.get());
            }
            consumeCommentsInsideNode(node, "hash");
        },
        [&](parser::If *if_) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;

            if (beginLine == endLine) {
                associateAssertionCommentsToNode(node);
            }

            walkNodes(if_->condition.get());
            walkNodes(if_->then_.get());
            walkNodes(if_->else_.get());

            if (beginLine != endLine) {
                associateAssertionCommentsToNode(node);
            }

            consumeCommentsInsideNode(node, "if");
        },
        [&](parser::InPattern *inPattern) {
            walkNodes(inPattern->pattern.get());
            walkNodes(inPattern->guard.get());
            walkNodes(inPattern->body.get());
            consumeCommentsInsideNode(node, "in_pattern");
        },
        [&](parser::Kwsplat *kwsplat) {
            walkNodes(kwsplat->expr.get());
            consumeCommentsInsideNode(node, "kwsplat");
        },
        [&](parser::Kwbegin *kwbegin) {
            associateAssertionCommentsToNode(node);
            for (auto &stmt : kwbegin->stmts) {
                walkNodes(stmt.get());
            }
            consumeCommentsInsideNode(node, "begin");
        },
        [&](parser::Masgn *masgn) {
            associateAssertionCommentsToNode(masgn->rhs.get(), true);
            walkNodes(masgn->rhs.get());
            consumeCommentsInsideNode(node, "masgn");
        },
        [&](parser::Module *mod) {
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            consumeCommentsUntilLine(beginLine);

            if (auto body = mod->body.get()) {
                walkNodes(body);
            }
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsBetweenLines(beginLine, endLine, "module");
        },
        [&](parser::Next *next) {
            // Only associate comments if the last expression is on the same line as the next
            if (!next->exprs.empty()) {
                auto nextLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
                auto lastExprLine = core::Loc::pos2Detail(ctx.file.data(ctx), next->exprs.back()->loc.beginPos()).line;
                if (lastExprLine == nextLine) {
                    associateAssertionCommentsToNode(node);
                }
            }

            for (auto &expr : next->exprs) {
                walkNodes(expr.get());
            }
            consumeCommentsInsideNode(node, "next");
        },
        [&](parser::OpAsgn *opAsgn) {
            associateAssertionCommentsToNode(opAsgn->right.get(), true);
            walkNodes(opAsgn->right.get());
            consumeCommentsInsideNode(node, "op_asgn");
        },
        [&](parser::Or *or_) {
            associateAssertionCommentsToNode(node);
            walkNodes(or_->right.get());
            walkNodes(or_->left.get());
            consumeCommentsInsideNode(node, "or");
        },
        [&](parser::OrAsgn *orAsgn) {
            associateAssertionCommentsToNode(orAsgn->right.get(), true);
            walkNodes(orAsgn->right.get());
            consumeCommentsInsideNode(node, "or_asgn");
        },
        [&](parser::Pair *pair) {
            walkNodes(pair->value.get());
            walkNodes(pair->key.get());
            consumeCommentsInsideNode(node, "pair");
        },
        [&](parser::Resbody *resbody) {
            walkNodes(resbody->body.get());
            consumeCommentsInsideNode(node, "rescue");
        },
        [&](parser::Rescue *rescue) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;

            if (beginLine == endLine) {
                // Single line rescue that may have an assertion comment so we need to start from the else node
                walkNodes(rescue->else_.get());
                for (auto &rescued : rescue->rescue) {
                    walkNodes(rescued.get());
                }
                walkNodes(rescue->body.get());
            } else {
                walkNodes(rescue->body.get());
                for (auto &rescued : rescue->rescue) {
                    walkNodes(rescued.get());
                }
                walkNodes(rescue->else_.get());
            }
            consumeCommentsBetweenLines(beginLine, endLine, "rescue");
        },
        [&](parser::Return *ret) {
            // Only associate comments if the last expression is on the same line as the return
            if (!ret->exprs.empty()) {
                auto returnLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
                auto lastExprLine = core::Loc::pos2Detail(ctx.file.data(ctx), ret->exprs.back()->loc.beginPos()).line;
                if (lastExprLine == returnLine) {
                    associateAssertionCommentsToNode(node);
                }
            }

            for (auto &expr : ret->exprs) {
                walkNodes(expr.get());
            }
            consumeCommentsInsideNode(node, "return");
        },
        [&](parser::SClass *sclass) {
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            consumeCommentsUntilLine(beginLine);

            if (auto body = sclass->body.get()) {
                walkNodes(body);
            }
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsBetweenLines(beginLine, endLine, "sclass");
        },
        [&](parser::Send *send) {
            if (parser::MK::isVisibilitySend(send)) {
                associateSignatureCommentsToNode(send->args[0].get());
                auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
                auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
                consumeCommentsBetweenLines(beginLine, endLine, "send");
            } else if (parser::MK::isAttrAccessorSend(send)) {
                associateSignatureCommentsToNode(send);
                auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
                auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
                consumeCommentsBetweenLines(beginLine, endLine, "send");
            } else {
                if (send->method == core::Names::squareBracketsEq()) {
                    // This is a `foo[key]=(y)` method, walk y for chained method calls
                    walkNodes(send->args.back().get());
                    walkNodes(send->receiver.get());
                    return;
                } else if (send->method.isSetter(ctx.state) && send->args.size() == 1) {
                    // This is a `foo.x=(y)` method, we treat it as a `x = y` assignment
                    associateAssertionCommentsToNode(send->args[0].get());
                    walkNodes(send->receiver.get());
                    return;
                }
                associateAssertionCommentsToNode(send);

                walkNodes(send->receiver.get());

                for (auto &arg : send->args) {
                    walkNodes(arg.get());
                }
                consumeCommentsInsideNode(node, "send");
            }
        },
        [&](parser::Splat *splat) {
            walkNodes(splat->var.get());
            consumeCommentsInsideNode(node, "splat");
        },
        [&](parser::Until *until) {
            associateAssertionCommentsToNode(node);
            walkNodes(until->cond.get());
            walkNodes(until->body.get());
            consumeCommentsInsideNode(node, "until");
        },
        [&](parser::UntilPost *untilPost) {
            walkNodes(untilPost->cond.get());
            walkNodes(untilPost->body.get());
            consumeCommentsInsideNode(node, "until");
        },
        [&](parser::When *when) {
            walkNodes(when->body.get());
            consumeCommentsInsideNode(node, "when");
        },
        [&](parser::While *while_) {
            associateAssertionCommentsToNode(node);
            walkNodes(while_->cond.get());
            walkNodes(while_->body.get());
            consumeCommentsInsideNode(node, "while");
        },
        [&](parser::WhilePost *whilePost) {
            walkNodes(whilePost->cond.get());
            walkNodes(whilePost->body.get());
            consumeCommentsInsideNode(node, "while");
        },
        [&](parser::Node *other) {
            associateAssertionCommentsToNode(node);
            consumeCommentsInsideNode(node, "other");
        });
}

map<parser::Node *, vector<CommentNode>> CommentsAssociator::run(unique_ptr<parser::Node> &node) {
    // Remove any comments that don't start with RBS prefixes
    for (auto it = commentByLine.begin(); it != commentByLine.end();) {
        if (!absl::StartsWith(it->second.string, RBS_PREFIX) &&
            !absl::StartsWith(it->second.string, ANNOTATION_PREFIX) &&
            !absl::StartsWith(it->second.string, MULTILINE_RBS_PREFIX)) {
            it = commentByLine.erase(it);
        } else {
            ++it;
        }
    }

    walkNodes(node.get());

    // Check for any remaining comments
    for (const auto &[line, comment] : commentByLine) {
        if (absl::StartsWith(comment.string, RBS_PREFIX) || absl::StartsWith(comment.string, MULTILINE_RBS_PREFIX)) {
            if (auto e = ctx.beginError(comment.loc, core::errors::Rewriter::RBSUnusedComment)) {
                e.setHeader("Unused RBS comment. Couldn't associate it with a method definition or a type assertion");
            }
        }
    }

    return move(commentsByNode);
}

CommentsAssociator::CommentsAssociator(core::MutableContext ctx, vector<core::LocOffsets> commentLocations)
    : ctx(ctx), commentLocations(commentLocations), commentByLine() {
    for (auto &loc : commentLocations) {
        auto comment_string = ctx.file.data(ctx).source().substr(loc.beginPos(), loc.endPos() - loc.beginPos());
        auto start32 = static_cast<uint32_t>(loc.beginPos());
        auto end32 = static_cast<uint32_t>(loc.endPos());
        auto comment = CommentNode{core::LocOffsets{start32, end32}, comment_string};

        auto line = core::Loc::pos2Detail(ctx.file.data(ctx), start32).line;
        commentByLine[line] = comment;
    }
}

} // namespace sorbet::rbs
