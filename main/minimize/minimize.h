#ifndef SORBET_MINIMIZE_H
#define SORBET_MINIMIZE_H

#include "core/GlobalState.h"
#include "main/options/options.h"

namespace sorbet::realmain {

// TODO(jez) These names were chosen to match as closely as possible with names used in missing_methods.rbi.
// It's possible we want to change them.

class Minimize final {
public:
    static void writeDiff(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                          const options::PrinterConfig &outfile);
};

} // namespace sorbet::realmain

#endif
