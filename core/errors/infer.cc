#include "core/errors/infer.h"
#include "core/GlobalState.h"

namespace sorbet::core::errors::Infer {

ErrorClass errorClassForUntyped(const GlobalState &gs, FileRef file, SymbolRef blame) {
    if (!gs.trackUntyped) {
        return UntypedValue;
    }

    auto isOpenInClient = file.data(gs).isOpenInClient();
    if (gs.printingFileTable) {
        // Note: this metric, despite being a prod metric, will not get reported in the normal way
        // to the metrics file, the web trace file, nor statsd. We call getAndClearHistogram BEFORE
        // calling getAndClearThreadCounters on the main thread, which means that the metric will
        // have been deleted before reporting to SignalFX. We don't even compute this if we are
        // not running that code path (i.e. printing in realmain), because:
        //
        // - Tracking this metric causes a noticeable slowdown (it involves growing and merging
        //   large UnorderedMap's), and
        // - If we did accidentally forget to clear the metric (e.g., in all LSP code paths), it
        //   would spam statsd services
        prodHistogramInc("untyped.usages", file.id());
    }

    if constexpr (sorbet::track_untyped_blame_mode) {
        histogramInc("untyped.blames", blame.rawId());
    }

    if (isOpenInClient && file.data(gs).strictLevel < core::StrictLevel::Strong) {
        return UntypedValueInformation;
    } else {
        return UntypedValue;
    }
}

} // namespace sorbet::core::errors::Infer
