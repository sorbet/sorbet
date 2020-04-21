#ifndef RUBY_TYPER_LSP_ERRORREPORTER_H
#define RUBY_TYPER_LSP_ERRORREPORTER_H

#include "ast/ast.h"
#include "core/core.h"
#include "main/lsp/LSPConfiguration.h"

namespace sorbet::realmain::lsp {

class ErrorReporter {
    const std::shared_ptr<const LSPConfiguration> config;
    /** Stores the epoch in which we last sent diagnostics to the client for each file. */
    std::vector<u4> diagnosticEpochs;
    /** List of files that have had errors in last run*/
    UnorderedSet<core::FileRef> filesThatHaveErrors;

public:
    ErrorReporter(std::shared_ptr<const LSPConfiguration> config);
    const std::vector<u4> &getDiagnosticEpochs() const;
    const UnorderedSet<core::FileRef> &getFilesThatHaveErrors() const;
    /**
     * Sends diagnostics from a typecheck run of a single file to the client.
     * `epoch` specifies the epoch of the file updates that produced these diagnostics. Used to prevent emitting
     * outdated diagnostics from a slow path run if they had already been re-typechecked on the fast path.
     */
    void pushDiagnostics(u4 epoch, core::FileRef file, std::vector<std::unique_ptr<core::Error>> errors,
                         std::unique_ptr<core::GlobalState> &gs);
    void setMaxFileId(u4 id);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_ERRORREPORTER_H