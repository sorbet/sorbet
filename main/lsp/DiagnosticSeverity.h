#ifndef SORBET_MAIN_LSP_DIAGNOSTIC_SEVERITY_H
#define SORBET_MAIN_LSP_DIAGNOSTIC_SEVERITY_H

#include "core/lsp/DiagnosticSeverity.h"
#include "main/lsp/json_enums.h"

namespace sorbet::realmain::lsp {

DiagnosticSeverity convertDiagnosticSeverity(core::lsp::DiagnosticSeverity severity);

core::lsp::DiagnosticSeverity convertDiagnosticSeverity(DiagnosticSeverity severity);

} // namespace sorbet::realmain::lsp

#endif
