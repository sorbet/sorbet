#ifndef PACKAGER_GEN_PACKAGES_H
#define PACKAGER_GEN_PACKAGES_H

#include "core/GlobalState.h"

namespace sorbet::packager {

class GenPackages final {
    ~GenPackages() = default;

public:
    static void run(core::GlobalState &gs);
    static void runStrict(core::GlobalState &gs);
};

} // namespace sorbet::packager

#endif
