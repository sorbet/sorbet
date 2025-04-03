#ifndef SORBET_RBS_ASSERTIONS_REWRITER_H
#define SORBET_RBS_ASSERTIONS_REWRITER_H

#include "parser/parser.h"
#include "rbs/rbs_common.h"
#include <memory>

namespace sorbet::rbs {

struct InlineComment {
    enum class Kind {
        LET,
        CAST,
        MUST,
        UNSAFE,
    };

    Comment comment;
    Kind kind;
};

class AssertionsRewriter {
public:
    AssertionsRewriter(core::MutableContext ctx) : ctx(ctx){};
    std::unique_ptr<parser::Node> run(std::unique_ptr<parser::Node> tree);

private:
    core::MutableContext ctx;
    std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams = {};
    std::set<std::pair<uint32_t, uint32_t>> consumedComments = {};

    void consumeComment(core::LocOffsets loc);
    bool hasConsumedComment(core::LocOffsets loc);
    std::optional<InlineComment> commentForPos(uint32_t fromPos, std::vector<char> allowedTokens);
    std::optional<InlineComment> commentForNode(std::unique_ptr<parser::Node> &node, core::LocOffsets fromLoc,
                                                std::vector<char> allowedTokens);

    std::unique_ptr<parser::Node> rewriteBody(std::unique_ptr<parser::Node> tree);
    std::unique_ptr<parser::Node> rewriteNode(std::unique_ptr<parser::Node> tree);
    parser::NodeVec rewriteNodesAsArray(parser::NodeVec nodes);
    void rewriteNodes(parser::NodeVec *nodes);

    bool saveTypeParams(parser::Block *block);
    std::unique_ptr<parser::Node> maybeInsertCast(std::unique_ptr<parser::Node> node, core::LocOffsets assignLoc,
                                                  std::vector<char> allowedTokens);
    std::unique_ptr<parser::Node>
    insertCast(std::unique_ptr<parser::Node> node,
               std::optional<std::pair<std::unique_ptr<parser::Node>, InlineComment::Kind>> pair);

    void checkDanglingCommentWithDecl(uint32_t nodeEnd, uint32_t declEnd, std::string kind);
    void checkDanglingComment(uint32_t nodeEnd, std::string kind);
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_ASSERTIONS_REWRITER_H
