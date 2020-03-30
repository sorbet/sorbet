#ifndef COMPILER_NAMES_H
#define COMPILER_NAMES_H

#include "compiler/Core/ForwardDeclarations.h"

namespace sorbet::compiler {

class Names {
public:
    static core::NameRef defineMethod(const core::GlobalState &gs);
    static core::NameRef defineMethodSingleton(const core::GlobalState &gs);
    static core::NameRef returnValue(const core::GlobalState &gs);

    static void init(core::GlobalState &gs);
};

} // namespace sorbet::compiler
#endif
