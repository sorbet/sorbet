#include "rbs/prism/CommentsAssociatorPrism.h"

using namespace std;

namespace sorbet::rbs {

CommentsAssociatorPrism::CommentsAssociatorPrism(core::MutableContext ctx, parser::Prism::Parser &parser,
                                                 std::vector<core::LocOffsets> commentLocations)
    : ctx(ctx), parser(parser), commentLocations(commentLocations) {}

CommentMapPrism CommentsAssociatorPrism::run(pm_node_t *node) {
    [[maybe_unused]] auto &_parser = this->parser;
    [[maybe_unused]] auto &_commentLocations = this->commentLocations;
    return CommentMapPrism{};
}

} // namespace sorbet::rbs
