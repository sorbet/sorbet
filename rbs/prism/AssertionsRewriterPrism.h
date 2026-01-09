#ifndef SORBET_RBS_ASSERTIONS_REWRITER_PRISM_H
#define SORBET_RBS_ASSERTIONS_REWRITER_PRISM_H

#include "common/common.h"
#include "parser/parser.h"
#include "rbs/prism/CommentsAssociatorPrism.h"

extern "C" {
#include "prism.h"
}

namespace sorbet::rbs {

class AssertionsRewriterPrism {
public:
    AssertionsRewriterPrism(core::MutableContext ctx, parser::Prism::Parser &parser,
                            UnorderedMap<pm_node_t *, std::vector<CommentNodePrism>> &commentsByNode);
    pm_node_t *run(pm_node_t *node);

private:
    core::MutableContext ctx;
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_ASSERTIONS_REWRITER_PRISM_H
