#include "main/minimize/minimize.h"

namespace sorbet::realmain {

void Minimize::writeDiff(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                         const options::PrinterConfig &outfile) {
    outfile.fmt("{}\n", "Hello, world!");
}

} // namespace sorbet::realmain
