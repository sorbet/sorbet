#ifndef RUBY_TYPER_LSP_ERRORREPORTER_H
#define RUBY_TYPER_LSP_ERRORREPORTER_H

#include "ast/ast.h"
#include "core/core.h"
#include "main/lsp/LSPConfiguration.h"

namespace sorbet::realmain::lsp {

struct ErrorStatus {
    // The epoch at which we last sent diagnostics for this file.
    u4 sentEpoch = 0;
    // If true, the client believes this file has errors.
    bool hasErrors = false;
};
class ErrorReporter {
    const std::shared_ptr<const LSPConfiguration> config;
    // Maps from file ref ID to its error status.
    std::vector<ErrorStatus> fileErrorStatuses;
    UnorderedMap<core::FileRef, ErrorStatus> uncommittedFileErrorStatuses;
    void setMaxFileId(u4 id);
    ErrorStatus getFileErrorStatus(core::FileRef file);

public:
    ErrorReporter(std::shared_ptr<const LSPConfiguration> config);
    /**
     * Used for unit tests
     */
    const std::vector<ErrorStatus> &getFileErrorStatuses() const;
    /**
     * Used for unit tests
     */
    const UnorderedMap<core::FileRef, ErrorStatus> &getUncommittedFileErrorStatuses() const;

    /**
     * Sends diagnostics from a typecheck run of a single file to the client.
     * `epoch` specifies the epoch of the file updates that produced these diagnostics. Used to prevent emitting
     * outdated diagnostics from a slow path run if they had already been re-typechecked on the fast path.
     */
    void pushDiagnostics(u4 epoch, core::FileRef file, std::vector<std::unique_ptr<core::Error>> &errors,
                         const core::GlobalState &gs);
    /**
     * Moves `uncommittedFileErrorStatuses` to `fileErrorStatuses` and clears `uncommittedFileErrorStatuses`
     */
    void commit();
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_ERRORREPORTER_H