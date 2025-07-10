#include "main/lsp/DiagnosticSeverity.h"

namespace sorbet::realmain::lsp {

DiagnosticSeverity convertDiagnosticSeverity(core::lsp::DiagnosticSeverity severity) {
    switch (severity) {
        case core::lsp::DiagnosticSeverity::Error:
            return DiagnosticSeverity::Error;
        case core::lsp::DiagnosticSeverity::Warning:
            return DiagnosticSeverity::Warning;
        case core::lsp::DiagnosticSeverity::Information:
            return DiagnosticSeverity::Information;
        case core::lsp::DiagnosticSeverity::Hint:
            return DiagnosticSeverity::Hint;
    }
}

core::lsp::DiagnosticSeverity convertDiagnosticSeverity(DiagnosticSeverity severity) {
    switch (severity) {
        case DiagnosticSeverity::Error:
            return core::lsp::DiagnosticSeverity::Error;
        case DiagnosticSeverity::Warning:
            return core::lsp::DiagnosticSeverity::Warning;
        case DiagnosticSeverity::Information:
            return core::lsp::DiagnosticSeverity::Information;
        case DiagnosticSeverity::Hint:
            return core::lsp::DiagnosticSeverity::Hint;
    }
}

} // namespace sorbet::realmain::lsp
