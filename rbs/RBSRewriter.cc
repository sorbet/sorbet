#include "rbs/RBSRewriter.h"

#include "common/timers/Timer.h"
#include "rbs/AssertionsRewriter.h"
#include "rbs/CommentsAssociator.h"
#include "rbs/SigsRewriter.h"

using namespace std;

namespace sorbet::rbs {

void runRBSRewrite(core::GlobalState &gs, core::FileRef file, pm_node_t *node,
                   const vector<core::LocOffsets> &commentLocations, core::MutableContext &ctx,
                   parser::Prism::Parser &parser) {
    Timer timeit(gs.tracer(), "runRBSRewrite", {{"file", string(file.data(gs).path())}});

    auto associator = CommentsAssociator(ctx, parser, commentLocations);
    auto commentMap = associator.run(node);

    auto sigsRewriter = SigsRewriter(ctx, parser, commentMap.signaturesForNode);
    sigsRewriter.run(node);

    auto assertionsRewriter = AssertionsRewriter(ctx, parser, commentMap.assertionsForNode);
    assertionsRewriter.run(node);
}

} // namespace sorbet::rbs
