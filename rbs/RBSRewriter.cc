#include "rbs/RBSRewriter.h"

#include "common/timers/Timer.h"
#include "rbs/AssertionsRewriter.h"
#include "rbs/CommentsAssociator.h"
#include "rbs/SigsRewriter.h"

using namespace std;

namespace sorbet::rbs {

void runPrismRBSRewrite(core::GlobalState &gs, core::FileRef file, pm_node_t *node,
                        const vector<core::LocOffsets> &commentLocations, core::MutableContext &ctx,
                        parser::Prism::Parser &parser) {
    Timer timeit(gs.tracer(), "runPrismRBSRewrite", {{"file", string(file.data(gs).path())}});

    auto associator = CommentsAssociatorPrism(ctx, parser, commentLocations);
    auto commentMap = associator.run(node);

    auto sigsRewriter = SigsRewriterPrism(ctx, parser, commentMap.signaturesForNode);
    sigsRewriter.run(node);

    auto assertionsRewriter = AssertionsRewriterPrism(ctx, parser, commentMap.assertionsForNode);
    assertionsRewriter.run(node);
}

} // namespace sorbet::rbs
