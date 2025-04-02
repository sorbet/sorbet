#ifndef SORBET_RBS_COMMENTS_ASSOCIATOR_H
#define SORBET_RBS_COMMENTS_ASSOCIATOR_H

#include "parser/parser.h"
#include <memory>

namespace sorbet::rbs {

struct CommentNode {
    core::LocOffsets loc;
    std::string_view string;
};

class CommentsAssociator {
public:
    CommentsAssociator(core::MutableContext ctx, std::vector<std::pair<size_t, size_t>> commentLocations)
        : ctx(ctx), commentLocations(commentLocations), commentByLine(){};

    void run(std::unique_ptr<parser::Node> &tree);

    std::map<parser::Node *, std::vector<CommentNode>> &getCommentsByNode() {
        return commentsByNode;
    }

private:
    core::MutableContext ctx;
    std::vector<std::pair<size_t, size_t>> commentLocations;

    // @kaan: use a ordered map on the line number
    std::map<int, CommentNode> commentByLine;

    std::map<parser::Node *, std::vector<CommentNode>> commentsByNode;

    void associateCommentsToLines();
    void walkNodes(parser::Node *node);
    void associateCommentsToNode(parser::Node *node);
    void consumeDanglingComments(parser::Node *node);
    void associateInlineCommentToNode(parser::Node *node);
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_COMMENTS_ASSOCIATOR_H
