#include "compiler/IREmitter/CFGHelpers.h"
#include "cfg/CFG.h"

#include <vector>

using namespace std;

namespace sorbet::compiler {

namespace {

bool validExit(cfg::CFG &cfg, int rubyBlockId, vector<cfg::BasicBlock *> &exits, cfg::BasicBlock *candidate) {
    if (candidate->rubyBlockId == rubyBlockId) {
        return false;
    }

    if (candidate == cfg.deadBlock()) {
        return false;
    }

    // Block calls will be jumped over explicitly, using the basicBlockJumpOverrides map constructed in
    // IREmitterHelpers::getRubyBlocks2FunctionsMapping, so they shouldn't be considered to be valid exits.
    if (candidate->bexit.cond.variable == cfg::LocalRef::blockCall()) {
        return false;
    }

    if (absl::c_find(exits, candidate) != exits.end()) {
        return false;
    }

    return true;
}

} // namespace

// Find all of the nodes that are jumped to from the given ruby block.
vector<cfg::BasicBlock *> CFGHelpers::findRubyBlockExits(cfg::CFG &cfg, int rubyBlockId) {
    vector<cfg::BasicBlock *> exits;

    for (auto &node : cfg.basicBlocks) {
        if (node->rubyBlockId != rubyBlockId) {
            continue;
        }

        auto *thenb = node->bexit.thenb;
        auto *elseb = node->bexit.elseb;

        if (validExit(cfg, rubyBlockId, exits, thenb)) {
            exits.emplace_back(thenb);
        }

        if (validExit(cfg, rubyBlockId, exits, elseb)) {
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
