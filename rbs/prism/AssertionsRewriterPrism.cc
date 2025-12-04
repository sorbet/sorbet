#include "rbs/prism/AssertionsRewriterPrism.h"

using namespace std;

namespace sorbet::rbs {

AssertionsRewriterPrism::AssertionsRewriterPrism(core::MutableContext ctx,
                                                 std::map<pm_node_t *, std::vector<CommentNodePrism>> &commentsByNode)
    : ctx(ctx), commentsByNode(&commentsByNode) {}

pm_node_t *AssertionsRewriterPrism::run(pm_node_t *node) {
    [[maybe_unused]] auto *_commentsByNode = this->commentsByNode;
    return node;
}

} // namespace sorbet::rbs
