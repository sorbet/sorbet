#include "rbs/prism/CommentsAssociatorPrism.h"

using namespace std;

namespace sorbet::rbs {

CommentsAssociatorPrism::CommentsAssociatorPrism(core::MutableContext ctx, const parser::Prism::Parser &parser,
                                                 const std::vector<core::LocOffsets> &commentLocations)
    : ctx(ctx), parser(parser), commentLocations(commentLocations) {}

CommentMapPrism CommentsAssociatorPrism::run(pm_node_t *node) {
    return CommentMapPrism{};
}

} // namespace sorbet::rbs
