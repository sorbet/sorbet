#include "core/errors/infer.h"
#include "core/GlobalState.h"

namespace sorbet::core::errors::Infer {

ErrorClass errorClassForUntyped(const GlobalState &gs, FileRef file) {
    if (!gs.trackUntyped) {
        return UntypedValue;
    }

    auto isOpenInClient = file.data(gs).isOpenInClient();
    if (!isOpenInClient) {
        // Note: this metric, despite being a prod metric, will not get reported in the normal way
        // to the metrics file, the web trace file, nor statsd. We call getAndClearHistogram BEFORE
        // calling getAndClearThreadCounters, which means that the metric will have been deleted
        // before reporting to SignalFX.
        //
        // (We explicitly don't even compute this in LSP, as evidenced by the isOpenInClient guard,
        // and so don't have to worry about the non-CLI metrics pipeline clearing this histogram
        // before publishing.)
        //
        // Why all this? To avoid spamming high-cardinality metrics to statsd services.
        prodHistogramInc("untyped.usages", file.id());
    }

    if (isOpenInClient && file.data(gs).strictLevel < core::StrictLevel::Strong) {
        return UntypedValueInformation;
    } else {
        return UntypedValue;
    }
}

} // namespace sorbet::core::errors::Infer
