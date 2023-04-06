#include "core/errors/infer.h"
#include "core/GlobalState.h"

namespace sorbet::core::errors::Infer {

ErrorClass errorClassForUntyped(const GlobalState &gs, FileRef file) {
    if (gs.highlightUntyped && file.data(gs).strictLevel < core::StrictLevel::Strong &&
        file.data(gs).isOpenInClient()) {
        return core::errors::Infer::UntypedValueInformation;
    } else {
        return core::errors::Infer::UntypedValue;
    }
}

} // namespace sorbet::core::errors::Infer
