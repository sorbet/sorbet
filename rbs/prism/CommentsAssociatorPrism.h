#ifndef SORBET_RBS_COMMENTS_ASSOCIATOR_PRISM_H
#define SORBET_RBS_COMMENTS_ASSOCIATOR_PRISM_H

#include "common/common.h"
#include "parser/parser.h"
#include "parser/prism/Parser.h"

extern "C" {
#include "prism.h"
}

namespace sorbet::rbs {

struct CommentNodePrism {
    core::LocOffsets loc;
    std::string string;
};

struct CommentMapPrism {
    std::map<pm_node_t *, std::vector<CommentNodePrism>> signaturesForNode;
    std::map<pm_node_t *, std::vector<CommentNodePrism>> assertionsForNode;
};

class CommentsAssociatorPrism {
public:
    CommentsAssociatorPrism(core::MutableContext ctx, const parser::Prism::Parser &parser,
                            const std::vector<core::LocOffsets> &commentLocations);

    CommentMapPrism run(pm_node_t *node);

private:
    core::MutableContext ctx;
    [[maybe_unused]] const parser::Prism::Parser &parser;
    [[maybe_unused]] const std::vector<core::LocOffsets> &commentLocations;
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_COMMENTS_ASSOCIATOR_PRISM_H
