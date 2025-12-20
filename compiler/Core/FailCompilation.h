#ifndef SORBET_COMPILER_CORE_FAIL_COMPILATION_H
#define SORBET_COMPILER_CORE_FAIL_COMPILATION_H

#include "compiler/Core/AbortCompilation.h"
#include "compiler/Errors/Errors.h"
#include "core/GlobalState.h"

namespace sorbet::compiler {

// Add an error to GlobalState, and then throw to abort compilation.
// Use only when compilation CANNOT continue.
// (Emitting any old GlobalState error will still cause Sorbet to exit non-zero.)
template <typename... Args>
[[noreturn]] void failCompilation(const core::GlobalState &gs, const core::Loc &loc, fmt::format_string<Args...> msg,
                                  Args &&...args) {
    if (auto e = gs.beginError(loc, core::errors::Compiler::Unanalyzable)) {
        e.setHeader(msg, std::forward<Args>(args)...);
    }

    throw AbortCompilation(fmt::format(msg, std::forward<Args>(args)...));
}

} // namespace sorbet::compiler
#endif
