#include "rbs/CommentsAssociator.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "rbs/SignatureTranslator.h"

using namespace std;

namespace sorbet::rbs {

const string_view CommentsAssociator::RBS_PREFIX = "#:";
const string_view CommentsAssociator::ANNOTATION_PREFIX = "# @";
const string_view CommentsAssociator::MULTILINE_RBS_PREFIX = "#|";
const string_view CommentsAssociator::BIND_PREFIX = "#: self as ";

const regex TYPE_ALIAS_PATTERN("^#: type\\s*([a-z][A-Za-z0-9_]*)\\s*=\\s*([^\\n]*)$");

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

        assertionsForNode[node] = move(comments);
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

    signaturesForNode[node] = move(comments);
}

int CommentsAssociator::maybeInsertStandalonePlaceholders(parser::NodeVec &nodes, int index, int lastLine,
                                                          int currentLine) {
    if (lastLine == currentLine) {
        return 0;
    }

    auto inserted = 0;
    parser::Node *continuationFor = nullptr;

    // We look for all comments between lastLine and currentLine
    for (auto it = commentByLine.begin(); it != commentByLine.end();) {
        if (it->first <= lastLine) {
            it++;
            continue;
        }

        if (it->first >= currentLine) {
            break;
        }

        if (continuationFor && absl::StartsWith(it->second.string, MULTILINE_RBS_PREFIX)) {
            signaturesForNode[continuationFor].emplace_back(move(it->second));
            continuationFor->loc = continuationFor->loc.join(it->second.loc);
            it = commentByLine.erase(it);
            continue;
        }

        if (absl::StartsWith(it->second.string, BIND_PREFIX)) {
            continuationFor = nullptr;

            auto placeholder = make_unique<parser::RBSPlaceholder>(it->second.loc, core::Names::Constants::RBSBind());

            vector<CommentNode> comments;
            comments.emplace_back(it->second);
            it = commentByLine.erase(it);
            assertionsForNode[placeholder.get()] = move(comments);

            nodes.insert(nodes.begin() + index, move(placeholder));

            inserted++;

            continue;
        }

        std::smatch matches;
        auto str = string(it->second.string);
        if (std::regex_match(str, matches, TYPE_ALIAS_PATTERN)) {
            if (!contextAllowingTypeAlias.empty()) {
                if (auto [allow, loc] = contextAllowingTypeAlias.back(); !allow) {
                    if (auto e = ctx.beginError(it->second.loc, core::errors::Rewriter::RBSUnusedComment)) {
                        e.setHeader("Unexpected RBS type alias comment");
                        e.addErrorLine(ctx.locAt(loc),
                                       "RBS type aliases are only allowed in class and module bodies. Found in:");
                    }

                    it = commentByLine.erase(it);
                    continue;
                }
            }

            auto nameStr = "type " + matches[1].str();
            auto nameConstant = ctx.state.enterNameConstant(nameStr);
            auto placeholder =
                make_unique<parser::RBSPlaceholder>(it->second.loc, core::Names::Constants::RBSTypeAlias());

            vector<CommentNode> comments;
            comments.emplace_back(it->second);
            signaturesForNode[placeholder.get()] = move(comments);

            continuationFor = placeholder.get();

            auto constantNode = make_unique<parser::Const>(it->second.loc, nullptr, nameConstant);
            auto assignNode = make_unique<parser::Assign>(it->second.loc, move(constantNode), move(placeholder));
            nodes.insert(nodes.begin() + index, move(assignNode));

            it = commentByLine.erase(it);

            inserted++;
            continue;
        }

        continuationFor = nullptr;

        it++;
    }

    return inserted;
}

void CommentsAssociator::processTrailingComments(parser::Node *node, parser::NodeVec &nodes) {
    if (node == nullptr || contextAllowingTypeAlias.empty()) {
        return;
    }

    int endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
    maybeInsertStandalonePlaceholders(nodes, nodes.size(), lastLine, endLine);
    lastLine = endLine;
}

unique_ptr<parser::Node> CommentsAssociator::walkBody(parser::Node *node, unique_ptr<parser::Node> body) {
    if (body == nullptr) {
        auto nodes = parser::NodeVec();
        processTrailingComments(node, nodes);

        if (!nodes.empty()) {
            return make_unique<parser::Begin>(node->loc, move(nodes));
        }
        return nullptr;
    }

    if (auto *begin = parser::cast_node<parser::Begin>(body.get())) {
        // The body is already a Begin node, so we don't need any wrapping
        walkNode(body.get());
        processTrailingComments(node, begin->stmts);
        return body;
    }

    // The body is a single node, we'll need to wrap it if we find standalone RBS comments
    auto beforeNodes = parser::NodeVec();

    // Visit standalone RBS comments after before the body node
    auto currentLine = core::Loc::pos2Detail(ctx.file.data(ctx), body->loc.beginPos()).line;
    maybeInsertStandalonePlaceholders(beforeNodes, 0, lastLine, currentLine);
    lastLine = currentLine;

    walkNode(body.get());

    // Visit standalone RBS comments after the body node
    auto afterNodes = parser::NodeVec();
    processTrailingComments(node, afterNodes);

    if (!beforeNodes.empty() || !afterNodes.empty()) {
        auto nodes = parser::NodeVec();
        nodes.reserve(beforeNodes.size() + 1 + afterNodes.size());

        for (auto &before : beforeNodes) {
            nodes.emplace_back(move(before));
        }

        auto loc = body->loc; // Grab the loc before moving the node out.
        nodes.emplace_back(move(body));

        for (auto &after : afterNodes) {
            nodes.emplace_back(move(after));
        }
        return make_unique<parser::Begin>(loc, move(nodes));
    }

    return body;
}

void CommentsAssociator::walkNodes(parser::NodeVec &nodes) {
    for (auto &node : nodes) {
        walkNode(node.get());
    }
}

void CommentsAssociator::walkStatements(parser::NodeVec &nodes) {
    for (int i = 0; i < nodes.size(); i++) {
        auto *stmt = nodes[i].get();

        if (parser::isa_node<parser::Ensure>(stmt)) {
            // Ensure need to be visited handled differently because of how we desugar their structure.
            // The bind needs to be added _inside_ them and not before if we want the type to be applied properly.
            walkNode(stmt);
            continue;
        }

        auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), stmt->loc.beginPos()).line;

        auto inserted = maybeInsertStandalonePlaceholders(nodes, i, lastLine, beginLine);
        i += inserted;

        walkNode(stmt);

        lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), stmt->loc.endPos()).line;
    }
}

void CommentsAssociator::walkNode(parser::Node *node) {
    if (node == nullptr) {
        return;
    }

    // If all RBS comments have been processed and associated with nodes, we can skip walking the rest of the tree.
    if (commentByLine.empty()) {
        return;
    }

    typecase(
        node,

        [&](parser::And *and_) {
            associateAssertionCommentsToNode(node);
            walkNode(and_->right.get());
            walkNode(and_->left.get());
            consumeCommentsInsideNode(node, "and");
        },
        [&](parser::AndAsgn *andAsgn) {
            associateAssertionCommentsToNode(andAsgn->right.get(), true);
            walkNode(andAsgn->right.get());
            consumeCommentsInsideNode(node, "and_asgn");
        },
        [&](parser::Array *array) {
            associateAssertionCommentsToNode(node);
            walkNodes(array->elts);
            consumeCommentsInsideNode(node, "array");
        },
        [&](parser::Assign *assign) {
            associateAssertionCommentsToNode(assign->rhs.get(), true);
            walkNode(assign->rhs.get());
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

            walkStatements(begin->stmts);
            lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsInsideNode(node, "begin");
        },
        [&](parser::Block *block) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), block->send->loc.beginPos()).line;
            associateAssertionCommentsToNode(node);
            walkNode(block->send.get());
            consumeCommentsUntilLine(beginLine);
            block->body = walkBody(block, move(block->body));
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

            walkNodes(break_->exprs);
            consumeCommentsInsideNode(node, "break");
        },
        [&](parser::Case *case_) {
            associateAssertionCommentsToNode(node);
            walkNode(case_->condition.get());
            walkNodes(case_->whens);
            case_->else_ = walkBody(case_, move(case_->else_));
            consumeCommentsInsideNode(node, "case");
        },
        [&](parser::CaseMatch *case_) {
            associateAssertionCommentsToNode(node);
            walkNode(case_->expr.get());
            walkNodes(case_->inBodies);
            case_->elseBody = walkBody(case_, move(case_->elseBody));
            consumeCommentsInsideNode(node, "case");
        },
        [&](parser::Class *cls) {
            contextAllowingTypeAlias.push_back(make_pair(true, cls->declLoc));
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            consumeCommentsUntilLine(beginLine);
            cls->body = walkBody(cls, move(cls->body));
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsBetweenLines(beginLine, endLine, "class");
            contextAllowingTypeAlias.pop_back();
        },
        [&](parser::CSend *csend) {
            if (csend->method.isSetter(ctx.state) && csend->args.size() == 1) {
                // This is a `foo&.x=(y)` method, we treat it as a `x = y` assignment
                associateAssertionCommentsToNode(csend->args[0].get());
                return;
            }
            associateAssertionCommentsToNode(node);
            walkNode(csend->receiver.get());
            walkNodes(csend->args);
            consumeCommentsInsideNode(node, "csend");
        },
        [&](parser::DefMethod *def) {
            contextAllowingTypeAlias.push_back(make_pair(false, def->declLoc));
            associateSignatureCommentsToNode(node);
            def->body = walkBody(def, move(def->body));
            consumeCommentsInsideNode(node, "method");
            contextAllowingTypeAlias.pop_back();
        },
        [&](parser::DefS *def) {
            contextAllowingTypeAlias.push_back(make_pair(false, def->declLoc));
            associateSignatureCommentsToNode(node);
            def->body = walkBody(def, move(def->body));
            consumeCommentsInsideNode(node, "method");
            contextAllowingTypeAlias.pop_back();
        },
        [&](parser::Ensure *ensure) {
            walkNode(ensure->body.get());
            ensure->ensure = walkBody(ensure, move(ensure->ensure));
            consumeCommentsInsideNode(node, "ensure");
        },
        [&](parser::For *for_) {
            associateAssertionCommentsToNode(node);
            walkNode(for_->expr.get());
            walkNode(for_->vars.get());
            for_->body = walkBody(for_, move(for_->body));
            consumeCommentsInsideNode(node, "for");
        },
        [&](parser::Hash *hash) {
            if (!hash->kwargs) {
                associateAssertionCommentsToNode(node);
            }
            walkNodes(hash->pairs);
            consumeCommentsInsideNode(node, "hash");
        },
        [&](parser::If *if_) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;

            if (beginLine == endLine) {
                associateAssertionCommentsToNode(node);
            }

            walkNode(if_->condition.get());

            lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            if_->then_ = walkBody(if_->then_.get(), move(if_->then_));

            if (if_->then_) {
                lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), if_->then_->loc.endPos()).line;
            }
            if_->else_ = walkBody(if_->else_.get(), move(if_->else_));

            if (beginLine != endLine) {
                associateAssertionCommentsToNode(node);
            }

            consumeCommentsInsideNode(node, "if");
        },
        [&](parser::InPattern *inPattern) {
            walkNode(inPattern->pattern.get());
            walkNode(inPattern->guard.get());
            inPattern->body = walkBody(inPattern, move(inPattern->body));
            consumeCommentsInsideNode(node, "in_pattern");
        },
        [&](parser::Kwsplat *kwsplat) {
            walkNode(kwsplat->expr.get());
            consumeCommentsInsideNode(node, "kwsplat");
        },
        [&](parser::Kwbegin *kwbegin) {
            associateAssertionCommentsToNode(node);
            walkStatements(kwbegin->stmts);
            lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsInsideNode(node, "begin");
        },
        [&](parser::Masgn *masgn) {
            associateAssertionCommentsToNode(masgn->rhs.get(), true);
            walkNode(masgn->rhs.get());
            consumeCommentsInsideNode(node, "masgn");
        },
        [&](parser::Module *mod) {
            contextAllowingTypeAlias.push_back(make_pair(true, mod->declLoc));
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            consumeCommentsUntilLine(beginLine);
            mod->body = walkBody(mod, move(mod->body));
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsBetweenLines(beginLine, endLine, "module");
            contextAllowingTypeAlias.pop_back();
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

            walkNodes(next->exprs);
            consumeCommentsInsideNode(node, "next");
        },
        [&](parser::OpAsgn *opAsgn) {
            associateAssertionCommentsToNode(opAsgn->right.get(), true);
            walkNode(opAsgn->right.get());
            consumeCommentsInsideNode(node, "op_asgn");
        },
        [&](parser::Or *or_) {
            associateAssertionCommentsToNode(node);
            walkNode(or_->right.get());
            walkNode(or_->left.get());
            consumeCommentsInsideNode(node, "or");
        },
        [&](parser::OrAsgn *orAsgn) {
            associateAssertionCommentsToNode(orAsgn->right.get(), true);
            walkNode(orAsgn->right.get());
            consumeCommentsInsideNode(node, "or_asgn");
        },
        [&](parser::Pair *pair) {
            walkNode(pair->value.get());
            walkNode(pair->key.get());
            consumeCommentsInsideNode(node, "pair");
        },
        [&](parser::Resbody *resbody) {
            resbody->body = walkBody(resbody, move(resbody->body));
            consumeCommentsInsideNode(node, "rescue");
        },
        [&](parser::Rescue *rescue) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;

            if (beginLine == endLine) {
                // Single line rescue that may have an assertion comment so we need to start from the else node
                rescue->else_ = walkBody(rescue, move(rescue->else_));
                walkNodes(rescue->rescue);
                rescue->body = walkBody(rescue, move(rescue->body));
            } else {
                rescue->body = walkBody(rescue->body.get(), move(rescue->body));
                walkNodes(rescue->rescue);
                rescue->else_ = walkBody(rescue, move(rescue->else_));
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

            walkNodes(ret->exprs);
            consumeCommentsInsideNode(node, "return");
        },
        [&](parser::SClass *sclass) {
            contextAllowingTypeAlias.push_back(make_pair(true, sclass->declLoc));
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;
            consumeCommentsUntilLine(beginLine);
            sclass->body = walkBody(sclass, move(sclass->body));
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;
            consumeCommentsBetweenLines(beginLine, endLine, "sclass");
            contextAllowingTypeAlias.pop_back();
        },
        [&](parser::Send *send) {
            if (parser::MK::isVisibilitySend(send) || parser::MK::isAttrAccessorSend(send)) {
                associateSignatureCommentsToNode(send);
                associateAssertionCommentsToNode(send);
                walkNode(send->receiver.get());
                walkNodes(send->args);
                consumeCommentsInsideNode(send, "send");
            } else if (send->method == core::Names::squareBracketsEq() || send->method.isSetter(ctx.state)) {
                // This is an assign through a send, either: `foo[key]=(y)` or `foo.x=(y)`
                //
                // Note: the parser groups the args on the right hand side of the assignment into an array node:
                //  * for `foo.x = 1, 2` the args are `[1, 2]`
                //  * for `foo[k1, k2] = 1, 2` the args are `[k1, k2, [1, 2]]`
                //
                // We always apply the cast starting from the last arg.
                for (auto it = send->args.rbegin(); it != send->args.rend(); ++it) {
                    walkNode((*it).get());
                }
                walkNode(send->receiver.get());
                consumeCommentsInsideNode(node, "send");
            } else {
                associateAssertionCommentsToNode(send);

                walkNode(send->receiver.get());
                walkNodes(send->args);
                consumeCommentsInsideNode(node, "send");
            }
        },
        [&](parser::Splat *splat) {
            walkNode(splat->var.get());
            consumeCommentsInsideNode(node, "splat");
        },
        [&](parser::Super *super_) {
            associateAssertionCommentsToNode(node);
            walkNodes(super_->args);
            consumeCommentsInsideNode(node, "super");
        },
        [&](parser::Until *until) {
            associateAssertionCommentsToNode(node);
            walkNode(until->cond.get());
            until->body = walkBody(until, move(until->body));
            consumeCommentsInsideNode(node, "until");
        },
        [&](parser::UntilPost *untilPost) {
            walkNode(untilPost->cond.get());
            untilPost->body = walkBody(untilPost, move(untilPost->body));
            consumeCommentsInsideNode(node, "until");
        },
        [&](parser::When *when) {
            when->body = walkBody(when, move(when->body));

            if (auto body = when->body.get()) {
                consumeCommentsInsideNode(body, "when");
            }
        },
        [&](parser::While *while_) {
            associateAssertionCommentsToNode(node);
            walkNode(while_->cond.get());
            while_->body = walkBody(while_, move(while_->body));
            consumeCommentsInsideNode(node, "while");
        },
        [&](parser::WhilePost *whilePost) {
            walkNode(whilePost->cond.get());
            whilePost->body = walkBody(whilePost, move(whilePost->body));
            consumeCommentsInsideNode(node, "while");
        },
        [&](parser::Node *other) {
            associateAssertionCommentsToNode(node);
            consumeCommentsInsideNode(node, "other");
        });
}

CommentMap CommentsAssociator::run(unique_ptr<parser::Node> &node) {
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

    lastLine = 0;
    walkNode(node.get());

    // Check for any remaining comments
    for (const auto &[line, comment] : commentByLine) {
        if (absl::StartsWith(comment.string, RBS_PREFIX) || absl::StartsWith(comment.string, MULTILINE_RBS_PREFIX)) {
            if (auto e = ctx.beginError(comment.loc, core::errors::Rewriter::RBSUnusedComment)) {
                e.setHeader("Unused RBS comment. Couldn't associate it with a method definition or a type assertion");
            }
        }
    }

    return CommentMap{signaturesForNode, assertionsForNode};
}

CommentsAssociator::CommentsAssociator(core::MutableContext ctx, vector<core::LocOffsets> commentLocations)
    : ctx(ctx), commentLocations(commentLocations), commentByLine() {
    for (auto &loc : commentLocations) {
        auto comment_string = ctx.file.data(ctx).source().substr(loc.beginPos(), loc.length());
        auto start32 = static_cast<uint32_t>(loc.beginPos());
        auto end32 = static_cast<uint32_t>(loc.endPos());
        auto comment = CommentNode{core::LocOffsets{start32, end32}, comment_string};

        auto line = core::Loc::pos2Detail(ctx.file.data(ctx), start32).line;
        commentByLine[line] = comment;
    }
}

} // namespace sorbet::rbs
