#ifndef SORBET_RBS_RBS_REWRITER_PRISM_H
#define SORBET_RBS_RBS_REWRITER_PRISM_H

#include "core/Context.h"
#include "core/LocOffsets.h"
#include "main/options/options.h"
#include "parser/prism/Parser.h"
#include <vector>

extern "C" {
#include "prism.h"
}

namespace sorbet::rbs {

pm_node_t *runPrismRBSRewrite(core::GlobalState &gs, core::FileRef file, pm_node_t *node,
                              const std::vector<core::LocOffsets> &commentLocations,
                              const realmain::options::Printers &print, core::MutableContext &ctx,
                              parser::Prism::Parser &parser);

} // namespace sorbet::rbs

#endif // SORBET_RBS_RBS_REWRITER_PRISM_H
