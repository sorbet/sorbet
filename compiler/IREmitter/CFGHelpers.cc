#include "compiler/IREmitter/CFGHelpers.h"
#include "cfg/CFG.h"

#include <vector>

using namespace std;

namespace sorbet::compiler {

// Find all of the nodes that are jumped to from the given ruby block.
vector<cfg::BasicBlock *> CFGHelpers::findRubyBlockExits(cfg::CFG &cfg, int rubyBlockId) {
    vector<cfg::BasicBlock *> exits;

    for (auto &node : cfg.basicBlocks) {
        if (node->rubyBlockId != rubyBlockId) {
            continue;
        }

        auto *thenb = node->bexit.thenb;
        auto *elseb = node->bexit.elseb;

        if (thenb->rubyBlockId != rubyBlockId && thenb != cfg.deadBlock() &&
            absl::c_all_of(exits, [&](auto *other) { return other != thenb; })) {
            exits.emplace_back(thenb);
        }

        if (elseb->rubyBlockId != rubyBlockId && elseb != cfg.deadBlock() &&
            absl::c_all_of(exits, [&](auto *other) { return other != elseb; })) {
            exits.emplace_back(elseb);
        }
    }

    return exits;
}

// Find the basic-block with the lowest id for the given ruby block id.
cfg::BasicBlock *CFGHelpers::findRubyBlockEntry(cfg::CFG &cfg, int rubyBlockId) {
    for (auto &bb : cfg.basicBlocks) {
        if (bb->rubyBlockId == rubyBlockId) {
            return bb.get();
        }
    }

    // This can happen when the ruby block is completely optimized away.
    return nullptr;
}

} // namespace sorbet::compiler
