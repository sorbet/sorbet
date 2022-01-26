#ifndef SORBET_CORE_PACKAGES_LAYER_H
#define SORBET_CORE_PACKAGES_LAYER_H

#include "core/Context.h"
#include <string_view>


namespace sorbet::core::packages {
class Layer final {
public:
    Layer() = default;
    Layer(int rank) : rank_(rank) {};
    std::string_view show(core::Context ctx) const;
    bool operator<(const Layer rhs) const;
    bool operator==(const Layer rhs) const;

private:
    uint8_t rank_;
};
} // namespace sorbet::core::packages
#endif // SORBET_CORE_PACKAGES_LAYER_H