#ifndef SORBET_COMPILER_NAMES_H
#define SORBET_COMPILER_NAMES_H

#include "core/NameRef.h"

namespace sorbet::compiler {
class Names {
public:
    static core::NameRef sorbet_defineTopClassOrModule(const core::GlobalState &gs);
    static core::NameRef sorbet_defineMethod(const core::GlobalState &gs);
    static core::NameRef sorbet_defineMethodSingleton(const core::GlobalState &gs);

    static void init(core::GlobalState &gs);
};
} // namespace sorbet::compiler
#endif
