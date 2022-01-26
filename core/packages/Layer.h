#ifndef SORBET_CORE_PACKAGES_LAYER_H
#define SORBET_CORE_PACKAGES_LAYER_H

#include "core/Context.h"
#include <string_view>

namespace sorbet::core::packages {
class Layer final {
public:
    Layer() = default;
    Layer(int rank) : rank_(rank){};
    core::NameRef show(core::Context ctx) const;
    bool exists() const;
    bool operator<(const Layer rhs) const;
    bool operator==(const Layer rhs) const;

private:
    uint8_t rank_ = 255;
};
} // namespace sorbet::core::packages
#endif // SORBET_CORE_PACKAGES_LAYER_H
