#ifndef SORBET_CORE_LSP_DIAGNOSTIC_SEVERITY_H
#define SORBET_CORE_LSP_DIAGNOSTIC_SEVERITY_H

#include <stdint.h>

namespace sorbet::core::lsp {

// Identical to realmain::lsp::DiagnosticSeverity.
// Needs to be here so that we can store this in GlobalState without depending on all of realmain::lsp
enum class DiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4,
};

} // namespace sorbet::core::lsp

#endif
