#include "rbs/prism/RBSRewriterPrism.h"

#include "common/timers/Timer.h"
#include "rbs/prism/AssertionsRewriterPrism.h"
#include "rbs/prism/CommentsAssociatorPrism.h"
#include "rbs/prism/SigsRewriterPrism.h"

using namespace std;

namespace sorbet::rbs {

pm_node_t *runPrismRBSRewrite(core::GlobalState &gs, core::FileRef file, pm_node_t *node,
                              const vector<core::LocOffsets> &commentLocations,
                              const realmain::options::Printers &print, core::MutableContext &ctx,
                              parser::Prism::Parser &parser) {
    Timer timeit(gs.tracer(), "runPrismRBSRewrite", {{"file", string(file.data(gs).path())}});

    auto associator = CommentsAssociatorPrism(ctx, parser, commentLocations);
    auto commentMap = associator.run(node);

    auto sigsRewriter = SigsRewriterPrism(ctx, parser, commentMap.signaturesForNode);
    node = sigsRewriter.run(node);

    auto assertionsRewriter = AssertionsRewriterPrism(ctx, parser, commentMap.assertionsForNode);
    node = assertionsRewriter.run(node);

    if (print.RBSRewriteTree.enabled) {
        print.RBSRewriteTree.fmt("{}\n", parser.prettyPrint(node));
    }

    return node;
}

} // namespace sorbet::rbs
