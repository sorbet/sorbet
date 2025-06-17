#ifndef SORBET_RBS_COMMENTS_ASSOCIATOR_H
#define SORBET_RBS_COMMENTS_ASSOCIATOR_H

#include "common/common.h"
#include "parser/parser.h"
#include <memory>
#include <regex>
#include <string_view>

namespace sorbet::rbs {

struct CommentNode {
    core::LocOffsets loc;
    std::string_view string;
};

struct CommentMap {
    std::map<parser::Node *, std::vector<CommentNode>> signaturesForNode;
    std::map<parser::Node *, std::vector<CommentNode>> assertionsForNode;
};

class CommentsAssociator {
public:
    static const std::string_view RBS_PREFIX;

    CommentsAssociator(core::MutableContext ctx, std::vector<core::LocOffsets> commentLocations);
    CommentMap run(std::unique_ptr<parser::Node> &tree);

private:
    static const std::string_view ANNOTATION_PREFIX;
    static const std::string_view MULTILINE_RBS_PREFIX;
    static const std::string_view BIND_PREFIX;

    core::MutableContext ctx;
    std::vector<core::LocOffsets> commentLocations;
    std::map<int, CommentNode> commentByLine;
    std::map<parser::Node *, std::vector<CommentNode>> signaturesForNode;
    std::map<parser::Node *, std::vector<CommentNode>> assertionsForNode;
    std::vector<std::pair<bool, core::LocOffsets>> contextAllowingTypeAlias;
    int lastLine;

    void walkNode(parser::Node *node);
    void walkNodes(parser::NodeVec &nodes);
    void walkStatements(parser::NodeVec &nodes);
    std::unique_ptr<parser::Node> walkBody(parser::Node *node, std::unique_ptr<parser::Node> body);
    void associateAssertionCommentsToNode(parser::Node *node, bool adjustLocForHeredoc);
    void associateSignatureCommentsToNode(parser::Node *node);
    void consumeCommentsInsideNode(parser::Node *node, std::string kind);
    void consumeCommentsBetweenLines(int startLine, int endLine, std::string kind);
    void consumeCommentsUntilLine(int line);
    std::optional<uint32_t> locateTargetLine(parser::Node *node);

    int maybeInsertStandalonePlaceholders(parser::NodeVec &nodes, int index, int lastLine, int currentLine);
    bool nestingAllowsTypeAlias();
};

} // namespace sorbet::rbs
#endif // SORBET_RBS_COMMENTS_ASSOCIATOR_H
