#include "core/core.h"

namespace sorbet::sig_finder {

struct SigLoc {
    core::LocOffsets sig;
    core::LocOffsets body;
};

std::optional<SigLoc> findSignature(const core::GlobalState &gs, const core::SymbolRef &methodDef);
} // namespace sorbet::sig_finder
