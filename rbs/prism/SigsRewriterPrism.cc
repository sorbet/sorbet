#include "rbs/prism/SigsRewriterPrism.h"

using namespace std;

namespace sorbet::rbs {

SigsRewriterPrism::SigsRewriterPrism(core::MutableContext ctx, const parser::Prism::Parser &parser,
                                     std::map<pm_node_t *, std::vector<CommentNodePrism>> &commentsByNode)
    : ctx(ctx), parser(parser), commentsByNode(&commentsByNode) {}

pm_node_t *SigsRewriterPrism::run(pm_node_t *node) {
    [[maybe_unused]] auto &_parser = this->parser;
    [[maybe_unused]] auto *_commentsByNode = this->commentsByNode;
    return node;
}

} // namespace sorbet::rbs
