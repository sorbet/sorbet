#include "core/GlobalState.h"
#include "core/packages/Layer.h"

namespace sorbet::core::packages {
std::string_view Layer::show(core::Context ctx) const {
    return ctx.state.packageDB().getLayerName(rank_);
}

bool Layer::operator<(const Layer rhs) const {
    return rank_ < rhs.rank_;
}

bool Layer::operator==(const Layer rhs) const {
    return rank_ == rhs.rank_;
}
} // namespace sorbet::core::packages
