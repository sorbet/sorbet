#include "compiler/IREmitter/CFGHelpers.h"
#include "cfg/CFG.h"

#include <vector>

using namespace std;

namespace sorbet::compiler {

namespace {

bool validExit(cfg::CFG &cfg, int targetRegionId, int rubyBlockId, vector<cfg::BasicBlock *> &exits,
               cfg::BasicBlock *candidate) {
    if (candidate->rubyBlockId == rubyBlockId) {
        return false;
    }

    if (candidate == cfg.deadBlock()) {
        return false;
    }

    if (absl::c_find(exits, candidate) != exits.end()) {
        return false;
    }

    return candidate->rubyBlockId == targetRegionId;
}

} // namespace

vector<cfg::BasicBlock *> CFGHelpers::findRegionExits(cfg::CFG &cfg, int targetRegionId, int sourceRegionId) {
    vector<cfg::BasicBlock *> exits;

    for (auto &node : cfg.basicBlocks) {
        if (node->rubyBlockId != sourceRegionId) {
            continue;
        }

        auto *thenb = node->bexit.thenb;
        auto *elseb = node->bexit.elseb;

        if (validExit(cfg, targetRegionId, sourceRegionId, exits, thenb)) {
            exits.emplace_back(thenb);
        }

        if (validExit(cfg, targetRegionId, sourceRegionId, exits, elseb)) {
            exits.emplace_back(elseb);
        }
    }

    return exits;
}

cfg::BasicBlock *CFGHelpers::findRegionEntry(cfg::CFG &cfg, int rubyBlockId) {
    for (auto &bb : cfg.basicBlocks) {
        if (bb->rubyBlockId == rubyBlockId) {
            return bb.get();
        }
    }

    // This can happen when the ruby block is completely optimized away.
    return nullptr;
}

} // namespace sorbet::compiler
