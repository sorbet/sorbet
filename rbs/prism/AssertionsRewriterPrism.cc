#include "rbs/prism/AssertionsRewriterPrism.h"

using namespace std;

namespace sorbet::rbs {

AssertionsRewriterPrism::AssertionsRewriterPrism(
    core::MutableContext ctx, parser::Prism::Parser &parser,
    std::unordered_map<pm_node_t *, std::vector<CommentNodePrism>> &commentsByNode)
    : ctx(ctx) {}

pm_node_t *AssertionsRewriterPrism::run(pm_node_t *node) {
    return node;
}

} // namespace sorbet::rbs
