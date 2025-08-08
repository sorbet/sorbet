#ifndef SORBET_RBS_ASSERTIONS_REWRITER_PRISM_H
#define SORBET_RBS_ASSERTIONS_REWRITER_PRISM_H

#include "parser/parser.h"
#include "parser/prism/Parser.h"
#include "rbs/prism/CommentsAssociatorPrism.h"
#include "rbs/rbs_common.h"
#include <memory>

extern "C" {
#include "prism.h"
}

namespace sorbet::rbs {

struct InlineCommentPrism {
    enum class Kind {
        ABSURD,
        BIND,
        CAST,
        LET,
        MUST,
        UNSAFE,
    };

    Comment comment;
    Kind kind;
};

class AssertionsRewriterPrism {
public:
    AssertionsRewriterPrism(core::MutableContext ctx, std::map<parser::Node *, std::vector<CommentNodePrism>> &commentsByNode)
        : ctx(ctx), legacyCommentsByNode(&commentsByNode), prismCommentsByNode(nullptr){};
    AssertionsRewriterPrism(core::MutableContext ctx, std::map<pm_node_t *, std::vector<CommentNodePrism>> &commentsByNode)
        : ctx(ctx), legacyCommentsByNode(nullptr), prismCommentsByNode(&commentsByNode){};
    pm_node_t * run(pm_node_t *node);

private:
    core::MutableContext ctx;
    std::map<parser::Node *, std::vector<CommentNodePrism>> *legacyCommentsByNode;
    [[maybe_unused]] std::map<pm_node_t *, std::vector<CommentNodePrism>> *prismCommentsByNode;
    std::vector<std::pair<core::LocOffsets, core::NameRef>> typeParams = {};
    std::set<std::pair<uint32_t, uint32_t>> consumedComments = {};

    void consumeComment(core::LocOffsets loc);
    bool hasConsumedComment(core::LocOffsets loc);
    std::optional<InlineCommentPrism> commentForPos(uint32_t fromPos, std::vector<char> allowedTokens);
    std::optional<InlineCommentPrism> commentForNode(const std::unique_ptr<parser::Node> &node);

    std::unique_ptr<parser::Node> rewriteBody(std::unique_ptr<parser::Node> tree);
    std::unique_ptr<parser::Node> rewriteNode(std::unique_ptr<parser::Node> tree);
    parser::NodeVec rewriteNodesAsArray(const std::unique_ptr<parser::Node> &node, parser::NodeVec nodes);
    void rewriteNodes(parser::NodeVec *nodes);

    bool saveTypeParams(parser::Block *block);
    std::unique_ptr<parser::Node> maybeInsertCast(std::unique_ptr<parser::Node> node);
    std::unique_ptr<parser::Node> replaceSyntheticBind(std::unique_ptr<parser::Node> node);
    std::unique_ptr<parser::Node>
    insertCast(std::unique_ptr<parser::Node> node,
               std::optional<std::pair<std::unique_ptr<parser::Node>, InlineCommentPrism::Kind>> pair);

    void checkDanglingCommentWithDecl(uint32_t nodeEnd, uint32_t declEnd, std::string kind);
    void checkDanglingComment(uint32_t nodeEnd, std::string kind);
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_ASSERTIONS_REWRITER_PRISM_H