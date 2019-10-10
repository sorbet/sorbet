#ifndef SORBET_COMPILER_NAMES_H
#define SORBET_COMPILER_NAMES_H

#include "core/NameRef.h"

namespace sorbet::compiler {
class Names {
public:
    static core::NameRef sorbet_defineTopLevelModule;
    static core::NameRef sorbet_defineNestedModule;
    static core::NameRef sorbet_defineTopLevelClass;
    static core::NameRef sorbet_defineNestedClass;
    static core::NameRef sorbet_defineMethod;
    static core::NameRef sorbet_defineMethodSingleton;

    static void init(core::GlobalState &gs);
};
} // namespace sorbet::compiler
#endif
