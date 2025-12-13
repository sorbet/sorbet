#ifndef SORBET_COMPILER_IREMITTER_CFG_HELPERS_H
#define SORBET_COMPILER_IREMITTER_CFG_HELPERS_H

#include "cfg/CFG.h"
#include <vector>

namespace sorbet::compiler {

class CFGHelpers {
public:
    // A heuristic for finding the first block in a region: find the basic-block with the lowest id for the given ruby
    // block id.
    static cfg::BasicBlock *findRegionEntry(cfg::CFG &cfg, int regionId);

    // Find the blocks in the target region that are jumped to from the source region. This explicitly ignores the dead
    // block as a valid jump target.
    static std::vector<cfg::BasicBlock *> findRegionExits(cfg::CFG &cfg, int targetRegionId, int sourceRegionId);
};

} // namespace sorbet::compiler

#endif
