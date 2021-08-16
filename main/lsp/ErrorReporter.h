#ifndef RUBY_TYPER_LSP_ERRORREPORTER_H
#define RUBY_TYPER_LSP_ERRORREPORTER_H

#include "core/core.h"

namespace sorbet::realmain::lsp {
class LSPConfiguration;

struct EpochTimers {
    std::vector<Timer> firstDiagnosticLatencyTimers;
    std::vector<std::unique_ptr<Timer>> lastDiagnosticLatencyTimers;
    bool hasFirstDiagnosticEndTimes = false;
};

struct ErrorStatus {
    // The epoch at which we last sent diagnostics for this file.
    u4 lastReportedEpoch = 0;
    // The number of errors reported for this file during the last reported epoch.
    u4 errorCount = 0;
};

class ErrorReporter {
    const std::shared_ptr<const LSPConfiguration> config;
    // Maps from file ref ID to its error status.
    std::vector<ErrorStatus> fileErrorStatuses;
    ErrorStatus &getFileErrorStatus(core::FileRef file);
    UnorderedMap<u4, EpochTimers> epochTimers;
    // The number of errors currently displayed in the editor. Reset whenever Sorbet begins a non-incremental epoch,
    // which promises to retypecheck every file. Used to implement a global error limit.
    u4 clientErrorCount = 0;
    // Tracks the last epoch that was a full typecheck. Used to sanity check clientErrorCount.
    u4 lastFullTypecheckEpoch = 0;

public:
    ErrorReporter(std::shared_ptr<const LSPConfiguration> config);
    std::vector<core::FileRef> filesWithErrorsSince(u4 epoch);
    /**
     * Sends diagnostics from a typecheck run of a single file to the client.
     * `epoch` specifies the epoch of the file updates that produced these diagnostics. Used to prevent emitting
     * outdated diagnostics from a slow path run if they had already been re-typechecked on the fast path.
     */
    void pushDiagnostics(u4 epoch, core::FileRef file, const std::vector<std::unique_ptr<core::Error>> &errors,
                         const core::GlobalState &gs);

    void beginEpoch(u4 epoch, bool isIncremental, std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers);
    void endEpoch(u4 epoch, bool committed = true);
    u4 lastDiagnosticEpochForFile(core::FileRef file);

    // Sanity checks error count data.
    void sanityCheck() const;
};

class ErrorEpoch final {
    ErrorReporter &errorReporter;
    u4 epoch;

public:
    ErrorEpoch(ErrorReporter &errorReporter, u4 epoch, bool isIncremental,
               std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers);

    ~ErrorEpoch();

    bool committed = false;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_ERRORREPORTER_H
