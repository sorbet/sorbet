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
    CommentsAssociator(core::MutableContext ctx, std::vector<core::LocOffsets> commentLocations);
    std::map<parser::Node *, std::vector<CommentNode>> run(std::unique_ptr<parser::Node> &tree);

private:
    core::MutableContext ctx;
    std::vector<core::LocOffsets> commentLocations;

    std::map<int, CommentNode> commentByLine;
    std::map<parser::Node *, std::vector<CommentNode>> commentsByNode;

    static const std::string_view RBS_PREFIX;
    static const std::string_view ANNOTATION_PREFIX;
    static const std::string_view MULTILINE_RBS_PREFIX;

    void walkNodes(parser::Node *node);
    void associateCommentsToNode(parser::Node *node, const InlinedVector<std::string_view, 3> &prefixes = {
                                                         RBS_PREFIX, ANNOTATION_PREFIX, MULTILINE_RBS_PREFIX});
    void associateInlineCommentToNode(parser::Node *node);
    void consumeCommentsUntilLine(int line);
};

} // namespace sorbet::rbs
#endif // SORBET_RBS_COMMENTS_ASSOCIATOR_H
