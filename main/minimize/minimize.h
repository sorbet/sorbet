#ifndef SORBET_MINIMIZE_H
#define SORBET_MINIMIZE_H

#include "common/concurrency/WorkerPool.h"
#include "core/GlobalState.h"
#include "main/options/options.h"

namespace sorbet::realmain {

class Minimize final {
public:
    static void indexAndResolveForMinimize(std::unique_ptr<core::GlobalState> &sourceGS,
                                           std::unique_ptr<core::GlobalState> &rbiGS, options::Options &opts,
                                           WorkerPool &workers, std::string minimizeRBI);
    static void writeDiff(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                          options::PrinterConfig &outfile);
};

} // namespace sorbet::realmain

#endif
