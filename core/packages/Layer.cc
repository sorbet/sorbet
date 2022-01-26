#include "core/packages/Layer.h"
#include "core/GlobalState.h"

namespace sorbet::core::packages {
core::NameRef Layer::show(core::Context ctx) const {
    auto layers = ctx.state.packageDB().layerNames();
    ENFORCE(rank_ < layers.size());
    return layers[rank_];
}

bool Layer::exists() const {
    return rank_ < 255;
}

bool Layer::operator<(const Layer rhs) const {
    return rank_ < rhs.rank_;
}

bool Layer::operator==(const Layer rhs) const {
    return rank_ == rhs.rank_;
}
} // namespace sorbet::core::packages
