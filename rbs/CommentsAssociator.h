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
    static const std::string_view RBS_PREFIX;

    CommentsAssociator(core::MutableContext ctx, std::vector<core::LocOffsets> commentLocations);
    std::map<parser::Node *, std::vector<CommentNode>> run(std::unique_ptr<parser::Node> &tree);

private:
    static const std::string_view ANNOTATION_PREFIX;
    static const std::string_view MULTILINE_RBS_PREFIX;

    core::MutableContext ctx;
    std::vector<core::LocOffsets> commentLocations;
    std::map<int, CommentNode> commentByLine;
    std::map<parser::Node *, std::vector<CommentNode>> commentsByNode;

    void walkNodes(parser::Node *node);
    void associateAssertionCommentsToNode(parser::Node *node, bool adjustLocForHeredoc);
    void associateSignatureCommentsToNode(parser::Node *node);
    void consumeCommentsBetweenLines(int startLine, int endLine, std::string kind);
    void consumeCommentsUntilLine(int line);
    uint32_t locateTargetLine(parser::Node *node);
};

} // namespace sorbet::rbs
#endif // SORBET_RBS_COMMENTS_ASSOCIATOR_H
