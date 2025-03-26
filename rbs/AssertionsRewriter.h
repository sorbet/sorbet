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
    };

    Comment comment;
    Kind kind;
};

class AssertionsRewriter {
public:
    AssertionsRewriter(core::MutableContext ctx) : ctx(ctx), lastSignature(nullptr){};
    std::unique_ptr<parser::Node> run(std::unique_ptr<parser::Node> tree);

private:
    core::MutableContext ctx;
    parser::Node *lastSignature;

    std::vector<std::pair<core::LocOffsets, core::NameRef>> lastTypeParams();

    std::optional<std::pair<std::unique_ptr<parser::Node>, InlineComment::Kind>>
    assertionForNode(std::unique_ptr<parser::Node> &node, core::LocOffsets fromLoc);

    std::unique_ptr<parser::Node> rewriteBegin(std::unique_ptr<parser::Node> tree);
    std::unique_ptr<parser::Node> rewriteBody(std::unique_ptr<parser::Node> tree);
    std::unique_ptr<parser::Node> rewriteNode(std::unique_ptr<parser::Node> tree);
    parser::NodeVec rewriteNodes(parser::NodeVec nodes);

    void maybeSaveSignature(parser::Block *block);
    std::unique_ptr<parser::Node> maybeInsertCast(std::unique_ptr<parser::Node> node);
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_ASSERTIONS_REWRITER_H
