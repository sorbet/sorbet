#include "rbs/CommentsAssociator.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/typecase.h"
#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "rbs/SignatureTranslator.h"

using namespace std;

namespace sorbet::rbs {

namespace {}; // namespace

void CommentsAssociator::consumeDanglingComments(parser::Node *node) {
    auto nodeEndLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;

    vector<CommentNode> comments;

    for (auto &[line, comment] : commentByLine) {
        if (line < nodeEndLine) {
            comments.push_back(comment);
        }

        // TODO: produce an error
    }

    // FIXME: make it one iterator only
    for (auto &comment : comments) {
        auto commentLine = core::Loc::pos2Detail(ctx.file.data(ctx), comment.loc.beginPos()).line;
        commentByLine.erase(commentLine);
    }
}
void CommentsAssociator::associateCommentsToLines() {
    for (auto &[start, end] : commentLocations) {
        auto comment_string = ctx.file.data(ctx).source().substr(start, end - start);
        auto start32 = static_cast<uint32_t>(start);
        auto end32 = static_cast<uint32_t>(end);
        auto comment = CommentNode{core::LocOffsets{start32, end32}, comment_string};

        auto line = core::Loc::pos2Detail(ctx.file.data(ctx), start32).line;
        commentByLine[line] = comment;
    }
}

void CommentsAssociator::associateCommentsToNode(parser::Node *node) {
    auto nodeStartLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.beginPos()).line;

    vector<CommentNode> comments;

    for (auto &[line, comment] : commentByLine) {
        if (line < nodeStartLine) {
            comments.push_back(comment);
            continue;
        }

        // FIXME: Once we have an ordered map, we can break as soon as the line number > node line
        // break;
    }

    // TODO: This is a hack to remove the comments from the commentByLine map.
    // merge it with a custom iterator earlier
    for (auto &comment : comments) {
        auto commentLine = core::Loc::pos2Detail(ctx.file.data(ctx), comment.loc.beginPos()).line;
        commentByLine.erase(commentLine);
    }

    commentsByNode[node] = comments;
}

void CommentsAssociator::associateInlineCommentToNode(parser::Node *node) {
    auto nodeEndLine = core::Loc::pos2Detail(ctx.file.data(ctx), node->loc.endPos()).line;

    auto comment = commentByLine.find(nodeEndLine);
    if (comment != commentByLine.end()) {
        vector<CommentNode> comments;
        comments.push_back(comment->second);
        commentsByNode[node] = comments;
        commentByLine.erase(nodeEndLine);
    }
}

void CommentsAssociator::walkNodes(parser::Node *node) {
    if (node == nullptr) {
        return;
    }

    // std::cerr << "Visit: " << node->nodeName() << "\n";

    // TODO: Implement

    typecase(
        node,

        // Blocks

        [&](parser::Begin *begin) {
            associateCommentsToNode(node);

            for (auto &stmt : begin->stmts) {
                walkNodes(stmt.get());
            }
        },

        // Classes

        [&](parser::Class *cls) {
            associateCommentsToNode(node);

            if (auto body = cls->body.get()) {
                walkNodes(body);
            }

            consumeDanglingComments(node);
        },

        // Methods

        [&](parser::DefMethod *def) { associateCommentsToNode(node); },
        [&](parser::DefS *def) { associateCommentsToNode(node); },

        // Assigns

        [&](parser::Assign *assign) { associateInlineCommentToNode(assign->rhs.get()); },

        // Other

        [&](parser::Node *other) {
            // Do nothing
        });
}

void CommentsAssociator::run(unique_ptr<parser::Node> &node) {
    associateCommentsToLines();
    walkNodes(node.get());

    // for (auto &[node, comments] : commentsByNode) {
    //     fmt::print("Node {}: {}\n", node->toString(ctx), comments.size());
    //     for (auto &comment : comments) {
    //         fmt::print("  {}\n", comment.string);
    //     }
    //     fmt::print("\n\n");
    // }
};

} // namespace sorbet::rbs
