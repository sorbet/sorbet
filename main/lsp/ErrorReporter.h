#ifndef RUBY_TYPER_LSP_ERRORREPORTER_H
#define RUBY_TYPER_LSP_ERRORREPORTER_H

#include "ast/ast.h"
#include "core/core.h"
#include "main/lsp/LSPConfiguration.h"

namespace sorbet::realmain::lsp {

struct ErrorStatus {
    // The epoch at which we last sent diagnostics for this file.
    u4 sentEpoch;
    // If true, the client believes this file has errors.
    bool hasErrors;
};

class ErrorReporter {
    const std::shared_ptr<const LSPConfiguration> config;
    // Maps from file ref ID to its error status.
    std::vector<ErrorStatus> fileErrorStatuses;

    void setMaxFileId(u4 id);

public:
    ErrorReporter(std::shared_ptr<const LSPConfiguration> config);
    const std::vector<ErrorStatus> &getFileErrorStatuses() const;
    /**
     * Sends diagnostics from a typecheck run of a single file to the client.
     * `epoch` specifies the epoch of the file updates that produced these diagnostics. Used to prevent emitting
     * outdated diagnostics from a slow path run if they had already been re-typechecked on the fast path.
     */
    void pushDiagnostics(u4 epoch, core::FileRef file, std::vector<std::unique_ptr<core::Error>> errors,
                         const core::GlobalState &gs);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_ERRORREPORTER_H