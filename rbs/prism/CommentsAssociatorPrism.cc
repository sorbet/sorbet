#include "rbs/prism/CommentsAssociatorPrism.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "rbs/SignatureTranslator.h"

using namespace std;

namespace sorbet::rbs {

const string_view CommentsAssociatorPrism::RBS_PREFIX = "#:";
const string_view CommentsAssociatorPrism::ANNOTATION_PREFIX = "# @";
const string_view CommentsAssociatorPrism::MULTILINE_RBS_PREFIX = "#|";
const string_view CommentsAssociatorPrism::BIND_PREFIX = "#: self as ";

const regex TYPE_ALIAS_PATTERN_PRISM("^#: type\\s*([a-z][A-Za-z0-9_]*)\\s*=\\s*([^\\n]*)$");

// Static regex pattern to avoid recompilation
static const regex HEREDOC_PATTERN_PRISM("\\s*=?\\s*<<(-|~)[^,\\s\\n#]+(,\\s*<<(-|~)[^,\\s\\n#]+)*");

/**
 * Check if the given range is the start of a heredoc assignment `= <<~FOO` and return the position of the end of the
 * heredoc marker.
 *
 * Returns -1 if no heredoc marker is found.
 */
optional<uint32_t> hasHeredocMarkerPrism(core::Context ctx, const uint32_t fromPos, const uint32_t toPos) {
    string_view source(ctx.file.data(ctx).source().substr(fromPos, toPos - fromPos));

    string source_str(source);
    smatch match;
    if (regex_search(source_str, HEREDOC_PATTERN_PRISM)) {
        return fromPos + source_str.length();
    }

    return nullopt;
}

optional<uint32_t> CommentsAssociatorPrism::locateTargetLine(pm_node_t *node) {
    optional<uint32_t> result = nullopt;

    if (node == nullptr) {
        return result;
    }

    typecase(
        node,
        [&](pm_string_node_t *lit) {
            auto loc = translateLocation(node->location);
            if (hasHeredocMarkerPrism(ctx, loc.beginPos(), loc.endPos())) {
                result = core::Loc::pos2Detail(ctx.file.data(ctx), loc.beginPos()).line;
            }
        },
        [&](pm_interpolated_string_node_t *lit) {
            auto loc = translateLocation(node->location);
            if (hasHeredocMarkerPrism(ctx, loc.beginPos(), loc.endPos())) {
                result = core::Loc::pos2Detail(ctx.file.data(ctx), loc.beginPos()).line;
            }
        },
        [&](pm_array_node_t *arr) {
            for (size_t i = 0; i < arr->elements.size; i++) {
                if (auto line = locateTargetLine(arr->elements.nodes[i])) {
                    result = *line;
                    break;
                }
            }
        },
        [&](pm_call_node_t *send) { result = locateTargetLine(send->receiver); },
        [&](pm_node_t *expr) {
            // No special handling for other node types
        });

    return result;
}

core::LocOffsets CommentsAssociatorPrism::translateLocation(pm_location_t location) {
    const uint8_t *sourceStart = (const uint8_t *)ctx.file.data(ctx).source().data();
    uint32_t start = static_cast<uint32_t>(location.start - sourceStart);
    uint32_t end = static_cast<uint32_t>(location.end - sourceStart);
    return core::LocOffsets{start, end};
}

void CommentsAssociatorPrism::consumeCommentsInsideNode(pm_node_t *node, string kind) {
    auto loc = translateLocation(node->location);
    auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), loc.beginPos()).line;
    auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), loc.endPos()).line;
    consumeCommentsBetweenLines(beginLine, endLine, kind);
}

void CommentsAssociatorPrism::consumeCommentsBetweenLines(int startLine, int endLine, string kind) {
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

void CommentsAssociatorPrism::consumeCommentsUntilLine(int line) {
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

void CommentsAssociatorPrism::associateAssertionCommentsToNode(pm_node_t *node, bool adjustLocForHeredoc = false) {
    auto loc = translateLocation(node->location);
    uint32_t targetLine = core::Loc::pos2Detail(ctx.file.data(ctx), loc.endPos()).line;
    if (adjustLocForHeredoc) {
        if (auto line = locateTargetLine(node)) {
            targetLine = *line;
        }
    }

    vector<CommentNodePrism> comments;

    auto it = commentByLine.find(targetLine);
    if (it != commentByLine.end() && absl::StartsWith(it->second.string, RBS_PREFIX)) {
        comments.emplace_back(it->second);
        commentByLine.erase(it);

        assertionsForNode[node] = move(comments);
    }
}

void CommentsAssociatorPrism::associateSignatureCommentsToNode(pm_node_t *node) {
    auto loc = translateLocation(node->location);
    auto nodeStartLine = core::Loc::pos2Detail(ctx.file.data(ctx), loc.beginPos()).line;

    vector<CommentNodePrism> comments;

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

int CommentsAssociatorPrism::maybeInsertStandalonePlaceholders(pm_node_list_t &nodes, int index, int lastLine,
                                                               int currentLine) {
    if (lastLine == currentLine) {
        return 0;
    }

    auto inserted = 0;
    pm_node_t *continuationFor = nullptr;

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
            // TODO: Fix location joining for Prism nodes
            // continuationFor->loc = continuationFor->loc.join(it->second.loc);
            it = commentByLine.erase(it);
            continue;
        }

        if (absl::StartsWith(it->second.string, BIND_PREFIX)) {
            continuationFor = nullptr;

            // TODO: Create RBSPlaceholder equivalent for Prism
            // Placeholder creation and insertion disabled until Prism equivalent is implemented
            it = commentByLine.erase(it);

            inserted++;

            continue;
        }

        std::smatch matches;
        auto str = string(it->second.string);
        if (std::regex_match(str, matches, TYPE_ALIAS_PATTERN_PRISM)) {
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
            auto placeholder = nullptr; // TODO: Create RBSPlaceholder equivalent for Prism

            // TODO: Handle type alias comments with Prism nodes
            // Placeholder creation disabled until Prism equivalent is implemented
            continuationFor = nullptr;

            // TODO: Create Const and Assign equivalents for Prism
            // Node insertion disabled until Prism equivalent is implemented

            it = commentByLine.erase(it);

            inserted++;
            continue;
        }

        continuationFor = nullptr;

        it++;
    }

    return inserted;
}

pm_node_t *CommentsAssociatorPrism::walkBody(pm_node_t *node, pm_node_t *body) {
    if (body == nullptr) {
        return nullptr;
    }

    if (body && body->type == PM_BEGIN_NODE) {
        // The body is already a Begin node, so we don't need any wrapping
        auto *begin = (pm_begin_node_t *)body;
        walkNode(body);

        // Visit standalone RBS comments after the last node in the body
        auto loc = translateLocation(node->location);
        int endLine = core::Loc::pos2Detail(ctx.file.data(ctx), loc.endPos()).line;
        // TODO: Fix maybeInsertStandalonePlaceholders for Prism nodes
        // maybeInsertStandalonePlaceholders(begin->statements, 0, lastLine, endLine);
        lastLine = endLine;

        return body;
    }

    // The body is a single node, we'll need to wrap it if we find standalone RBS comments
    // TODO: Handle beforeNodes with Prism node lists

    // Visit standalone RBS comments after before the body node
    auto currentLine = core::Loc::pos2Detail(ctx.file.data(ctx), body->loc.beginPos()).line;
    maybeInsertStandalonePlaceholders(beforeNodes, 0, lastLine, currentLine);
    lastLine = currentLine;

    walkNode(body.get());

    // Visit standalone RBS comments after the body node
    // TODO: Handle afterNodes with Prism node lists
    int endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;
    maybeInsertStandalonePlaceholders(afterNodes, 0, lastLine, endLine);
    lastLine = endLine;

    if (!beforeNodes.empty() || !afterNodes.empty()) {
        // TODO: Handle nodes creation with Prism node lists
        for (auto &before : beforeNodes) {
            nodes.emplace_back(move(before));
        }
        auto loc = body->loc;
        nodes.emplace_back(move(body));
        for (auto &after : afterNodes) {
            nodes.emplace_back(move(after));
        }
        // TODO: Create Begin node with Prism
        return nullptr;
    }

    return body;
}

void CommentsAssociatorPrism::walkNodes(pm_node_list_t &nodes) {
    for (size_t i = 0; i < nodes.size; i++) {
        auto node = nodes.nodes[i];
        walkNode(node);
    }
}

void CommentsAssociatorPrism::walkStatements(pm_node_list_t &nodes) {
    for (size_t i = 0; i < nodes.size; i++) {
        auto *stmt = nodes.nodes[i];

        if (stmt->type == PM_ENSURE_NODE) {
            // Ensure need to be visited handled differently because of how we desugar their structure.
            // The bind needs to be added _inside_ them and not before if we want the type to be applied properly.
            walkNode(stmt);
            continue;
        }

        auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), stmt->location.start).line;

        auto inserted = maybeInsertStandalonePlaceholders(nodes, i, lastLine, beginLine);
        i += inserted;

        walkNode(stmt);

        lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), stmt->location.end).line;
    }
}

void CommentsAssociatorPrism::walkNode(pm_node_t *node) {
    if (node == nullptr) {
        return;
    }

    typecase(
        node,

        [&](pm_and_node_t *and_) {
            associateAssertionCommentsToNode(node);
            walkNode(and_->right);
            walkNode(and_->left);
            consumeCommentsInsideNode(node, "and");
        },
        [&](pm_local_variable_and_write_node_t *andAsgn) {
            associateAssertionCommentsToNode(andAsgn->value, true);
            walkNode(andAsgn->value);
            consumeCommentsInsideNode(node, "and_asgn");
        },
        [&](pm_array_node_t *array) {
            associateAssertionCommentsToNode(node);
            walkNodes(array->elts);
            consumeCommentsInsideNode(node, "array");
        },
        [&](pm_local_variable_write_node_t *assign) {
            associateAssertionCommentsToNode(assign->value, true);
            walkNode(assign->value);
            consumeCommentsInsideNode(node, "assign");
        },
        [&](pm_begin_node_t *begin) {
            // This is a workaround that will be removed once we migrate to prism. We need to differentiate between
            // implicit and explicit begin nodes.
            //
            // (let4 &&= "foo") #: Integer
            // vs
            // take_block { |x|
            //   puts x
            //   x #: as String
            // }
            if (begin->stmts.size() > 0 && begin->stmts[0]->loc.endPos() + 1 == node->location.end) {
                associateAssertionCommentsToNode(node);
            }

            walkStatements(begin->stmts);
            lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;
            consumeCommentsInsideNode(node, "begin");
        },
        [&](pm_block_node_t *block) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
            consumeCommentsUntilLine(beginLine);

            associateAssertionCommentsToNode(node);
            walkNode((pm_node_t *)block->call);
            block->body = walkBody(block, move(block->body));
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;
            consumeCommentsBetweenLines(beginLine, endLine, "block");
        },
        [&](pm_break_node_t *break_) {
            // Only associate comments if the last expression is on the same line as the break
            if (!break_->exprs.empty()) {
                auto breakLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
                auto lastExprLine =
                    core::Loc::pos2Detail(ctx.file.data(ctx), break_->exprs.back()->loc.beginPos()).line;
                if (lastExprLine == breakLine) {
                    associateAssertionCommentsToNode(node);
                }
            }

            walkNodes(break_->exprs);
            consumeCommentsInsideNode(node, "break");
        },
        [&](pm_case_node_t *case_) {
            associateAssertionCommentsToNode(node);
            walkNode(case_->predicate);
            walkNodes(case_->whens);
            case_->else_ = walkBody(case_, move(case_->else_));
            consumeCommentsInsideNode(node, "case");
        },
        [&](pm_case_match_node_t *case_) {
            associateAssertionCommentsToNode(node);
            walkNode(case_->predicate);
            walkNodes(case_->inBodies);
            case_->elseBody = walkBody(case_, move(case_->elseBody));
            consumeCommentsInsideNode(node, "case");
        },
        [&](pm_class_node_t *cls) {
            contextAllowingTypeAlias.push_back(make_pair(true, cls->declLoc));
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
            consumeCommentsUntilLine(beginLine);
            cls->body = walkBody(cls, move(cls->body));
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;
            consumeCommentsBetweenLines(beginLine, endLine, "class");
            contextAllowingTypeAlias.pop_back();
        },
        [&](pm_call_node_t *csend) {
            if (csend->method.isSetter(ctx.state) && csend->args.size() == 1) {
                // This is a `foo&.x=(y)` method, we treat it as a `x = y` assignment
                associateAssertionCommentsToNode((pm_node_t *)csend->arguments->arguments.nodes[0]);
                return;
            }
            associateAssertionCommentsToNode(node);
            walkNode(csend->receiver);
            walkNodes(csend->args);
            consumeCommentsInsideNode(node, "csend");
        },
        [&](pm_def_node_t *def) {
            contextAllowingTypeAlias.push_back(make_pair(false, def->declLoc));
            associateSignatureCommentsToNode(node);
            def->body = walkBody(def, move(def->body));
            consumeCommentsInsideNode(node, "method");
            contextAllowingTypeAlias.pop_back();
        },
        [&](pm_def_node_t *def) {
            contextAllowingTypeAlias.push_back(make_pair(false, def->declLoc));
            associateSignatureCommentsToNode(node);
            def->body = walkBody(def, move(def->body));
            consumeCommentsInsideNode(node, "method");
            contextAllowingTypeAlias.pop_back();
        },
        [&](pm_ensure_node_t *ensure) {
            walkNode((pm_node_t *)ensure->statements);
            ensure->ensure = walkBody(ensure, move(ensure->ensure));
            consumeCommentsInsideNode(node, "ensure");
        },
        [&](pm_for_node_t *for_) {
            associateAssertionCommentsToNode(node);
            walkNode(for_->collection);
            walkNode(for_->index);
            for_->body = walkBody(for_, move(for_->body));
            consumeCommentsInsideNode(node, "for");
        },
        [&](pm_hash_node_t *hash) {
            if (!hash->kwargs) {
                associateAssertionCommentsToNode(node);
            }
            walkNodes(hash->pairs);
            consumeCommentsInsideNode(node, "hash");
        },
        [&](pm_if_node_t *if_) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;

            if (beginLine == endLine) {
                associateAssertionCommentsToNode(node);
            }

            walkNode(if_->predicate);

            lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
            // TODO: Fix walkBody calls for Prism nodes

            if (if_->then_) {
                lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), if_->then_->loc.endPos()).line;
            }
            // TODO: Fix walkBody calls for Prism nodes

            if (beginLine != endLine) {
                associateAssertionCommentsToNode(node);
            }

            consumeCommentsInsideNode(node, "if");
        },
        [&](pm_in_node_t *inPattern) {
            walkNode(inPattern->pattern);
            walkNode(inPattern->guard);
            inPattern->body = walkBody(inPattern, move(inPattern->body));
            consumeCommentsInsideNode(node, "in_pattern");
        },
        [&](pm_keyword_hash_node_t *kwsplat) {
            // TODO: Fix kwsplat field access for Prism nodes
            consumeCommentsInsideNode(node, "kwsplat");
        },
        [&](pm_begin_node_t *kwbegin) {
            associateAssertionCommentsToNode(node);
            walkStatements(kwbegin->stmts);
            lastLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;
            consumeCommentsInsideNode(node, "begin");
        },
        [&](pm_multi_write_node_t *masgn) {
            associateAssertionCommentsToNode(masgn->value, true);
            walkNode(masgn->value);
            consumeCommentsInsideNode(node, "masgn");
        },
        [&](pm_module_node_t *mod) {
            contextAllowingTypeAlias.push_back(make_pair(true, mod->declLoc));
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
            consumeCommentsUntilLine(beginLine);
            mod->body = walkBody(mod, move(mod->body));
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;
            consumeCommentsBetweenLines(beginLine, endLine, "module");
            contextAllowingTypeAlias.pop_back();
        },
        [&](pm_next_node_t *next) {
            // Only associate comments if the last expression is on the same line as the next
            if (!next->exprs.empty()) {
                auto nextLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
                auto lastExprLine = core::Loc::pos2Detail(ctx.file.data(ctx), next->exprs.back()->loc.beginPos()).line;
                if (lastExprLine == nextLine) {
                    associateAssertionCommentsToNode(node);
                }
            }

            walkNodes(next->exprs);
            consumeCommentsInsideNode(node, "next");
        },
        [&](pm_local_variable_operator_write_node_t *opAsgn) {
            associateAssertionCommentsToNode(opAsgn->value, true);
            walkNode(opAsgn->value);
            consumeCommentsInsideNode(node, "op_asgn");
        },
        [&](pm_or_node_t *or_) {
            associateAssertionCommentsToNode(node);
            walkNode(or_->right);
            walkNode(or_->left);
            consumeCommentsInsideNode(node, "or");
        },
        [&](pm_local_variable_or_write_node_t *orAsgn) {
            associateAssertionCommentsToNode(orAsgn->value, true);
            walkNode(orAsgn->value);
            consumeCommentsInsideNode(node, "or_asgn");
        },
        [&](pm_assoc_node_t *pair) {
            walkNode(pair->value);
            walkNode(pair->key);
            consumeCommentsInsideNode(node, "pair");
        },
        [&](pm_rescue_node_t *resbody) {
            resbody->body = walkBody(resbody, move(resbody->body));
            consumeCommentsInsideNode(node, "rescue");
        },
        [&](pm_rescue_node_t *rescue) {
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;

            if (beginLine == endLine) {
                // Single line rescue that may have an assertion comment so we need to start from the else node
                rescue->else_ = walkBody(rescue, move(rescue->else_));
                walkNodes(rescue->rescue);
                rescue->body = walkBody(rescue, move(rescue->body));
            } else {
                // TODO: Fix walkBody calls for Prism nodes
                walkNodes(rescue->rescue);
                rescue->else_ = walkBody(rescue, move(rescue->else_));
            }

            consumeCommentsBetweenLines(beginLine, endLine, "rescue");
        },
        [&](pm_return_node_t *ret) {
            // Only associate comments if the last expression is on the same line as the return
            if (!ret->exprs.empty()) {
                auto returnLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
                auto lastExprLine = core::Loc::pos2Detail(ctx.file.data(ctx), ret->exprs.back()->loc.beginPos()).line;
                if (lastExprLine == returnLine) {
                    associateAssertionCommentsToNode(node);
                }
            }

            walkNodes(ret->exprs);
            consumeCommentsInsideNode(node, "return");
        },
        [&](pm_singleton_class_node_t *sclass) {
            contextAllowingTypeAlias.push_back(make_pair(true, sclass->declLoc));
            associateSignatureCommentsToNode(node);
            auto beginLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.start).line;
            consumeCommentsUntilLine(beginLine);
            sclass->body = walkBody(sclass, move(sclass->body));
            auto endLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->location.end).line;
            consumeCommentsBetweenLines(beginLine, endLine, "sclass");
            contextAllowingTypeAlias.pop_back();
        },
        [&](pm_call_node_t *send) {
            // TODO: Implement isVisibilitySend for Prism nodes
            if (false) {
                associateSignatureCommentsToNode(send);
                consumeCommentsInsideNode(node, "send");
            } else if (false) { // TODO: Implement isAttrAccessorSend for Prism nodes
                associateSignatureCommentsToNode(send);
                associateAssertionCommentsToNode(send);
                walkNode(send->receiver);
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
                    // TODO: Fix iterator access for Prism arguments
                }
                walkNode(send->receiver);
                consumeCommentsInsideNode(node, "send");
            } else {
                associateAssertionCommentsToNode(send);

                walkNode(send->receiver);
                walkNodes(send->args);
                consumeCommentsInsideNode(node, "send");
            }
        },
        [&](pm_splat_node_t *splat) {
            walkNode(splat->expression);
            consumeCommentsInsideNode(node, "splat");
        },
        [&](pm_super_node_t *super_) {
            associateAssertionCommentsToNode(node);
            walkNodes(super_->args);
            consumeCommentsInsideNode(node, "super");
        },
        [&](pm_until_node_t *until) {
            associateAssertionCommentsToNode(node);
            walkNode(until->predicate);
            until->body = walkBody(until, move(until->body));
            consumeCommentsInsideNode(node, "until");
        },
        [&](pm_until_node_t *untilPost) {
            walkNode(untilPost->predicate);
            untilPost->body = walkBody(untilPost, move(untilPost->body));
            consumeCommentsInsideNode(node, "until");
        },
        [&](pm_when_node_t *when) {
            when->body = walkBody(when, move(when->body));

            if (auto body = (pm_node_t *)when->statements) {
                consumeCommentsInsideNode(body, "when");
            }
        },
        [&](pm_while_node_t *while_) {
            associateAssertionCommentsToNode(node);
            walkNode(while_->predicate);
            while_->body = walkBody(while_, move(while_->body));
            consumeCommentsInsideNode(node, "while");
        },
        [&](pm_while_node_t *whilePost) {
            walkNode(whilePost->predicate);
            whilePost->body = walkBody(whilePost, move(whilePost->body));
            consumeCommentsInsideNode(node, "while");
        },
        [&](pm_node_t *other) {
            associateAssertionCommentsToNode(node);
            consumeCommentsInsideNode(node, "other");
        });
}

CommentMapPrismNode CommentsAssociatorPrism::run(pm_node_t *node) {
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
    walkNode(node);

    // Check for any remaining comments
    for (const auto &[line, comment] : commentByLine) {
        if (absl::StartsWith(comment.string, RBS_PREFIX) || absl::StartsWith(comment.string, MULTILINE_RBS_PREFIX)) {
            if (auto e = ctx.beginError(comment.loc, core::errors::Rewriter::RBSUnusedComment)) {
                e.setHeader("Unused RBS comment. Couldn't associate it with a method definition or a type assertion");
            }
        }
    }

    return CommentMapPrismNode{signaturesForNode, assertionsForNode};
}

CommentsAssociatorPrism::CommentsAssociatorPrism(core::MutableContext ctx, vector<core::LocOffsets> commentLocations)
    : ctx(ctx), commentLocations(commentLocations), commentByLine() {
    for (auto &loc : commentLocations) {
        auto comment_string = ctx.file.data(ctx).source().substr(loc.beginPos(), loc.endPos() - loc.beginPos());
        auto start32 = static_cast<uint32_t>(loc.beginPos());
        auto end32 = static_cast<uint32_t>(loc.endPos());
        auto comment = CommentNodePrism{core::LocOffsets{start32, end32}, comment_string};

        auto line = core::Loc::pos2Detail(ctx.file.data(ctx), start32).line;
        commentByLine[line] = comment;
    }
}

} // namespace sorbet::rbs
