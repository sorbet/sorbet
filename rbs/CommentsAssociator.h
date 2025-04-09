#ifndef SORBET_RBS_COMMENTS_ASSOCIATOR_H
#define SORBET_RBS_COMMENTS_ASSOCIATOR_H

#include "common/common.h"
#include "parser/parser.h"
#include <memory>
#include <string_view>

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

    std::map<int, CommentNode> commentByLine;
    std::map<parser::Node *, std::vector<CommentNode>> commentsByNode;

    static const std::string_view RBS_PREFIX;
    static const std::string_view ANNOTATION_PREFIX;

    void associateCommentsToLines();
    void walkNodes(parser::Node *node);
    void associateCommentsToNode(parser::Node *node,
                                 const InlinedVector<std::string_view, 2> &prefixes = {RBS_PREFIX, ANNOTATION_PREFIX});
    void consumeDanglingComments(parser::Node *node);
    void associateInlineCommentToNode(parser::Node *node);
    void consumePrecedingComments(parser::Node *node, const std::string_view &prefix);
};

} // namespace sorbet::rbs
#endif // SORBET_RBS_COMMENTS_ASSOCIATOR_H
