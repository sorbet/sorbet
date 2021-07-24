#ifndef SORBET_COMPILER_IREMITTER_CFG_HELPERS_H
#define SORBET_COMPILER_IREMITTER_CFG_HELPERS_H

#include "cfg/CFG.h"
#include <vector>

namespace sorbet::compiler {

class CFGHelpers {
public:
    static cfg::BasicBlock *findRubyBlockEntry(cfg::CFG &cfg, int rubyBlockId);
    static std::vector<cfg::BasicBlock *> findRubyBlockExits(cfg::CFG &cfg, int rubyBlockId);
};

} // namespace sorbet::compiler

#endif
