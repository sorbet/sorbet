#ifndef SORBET_MINIMIZE_H
#define SORBET_MINIMIZE_H

#include "core/GlobalState.h"
#include "main/options/options.h"

namespace sorbet::realmain {

class Minimize final {
public:
    static void writeDiff(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                          options::PrinterConfig &outfile);
};

} // namespace sorbet::realmain

#endif
