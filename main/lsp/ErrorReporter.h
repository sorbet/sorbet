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
    uint32_t lastReportedEpoch = 0;
    // The number of errors reported for this file during the last reported epoch.
    uint32_t errorCount = 0;
};

class ErrorReporter {
    const std::shared_ptr<const LSPConfiguration> config;
    // Maps from file ref ID to its error status.
    std::vector<ErrorStatus> fileErrorStatuses;
    ErrorStatus &getFileErrorStatus(core::FileRef file);
    UnorderedMap<uint32_t, EpochTimers> epochTimers;
    // The number of errors currently displayed in the editor. Reset whenever Sorbet begins a non-incremental epoch,
    // which promises to retypecheck every file. Used to implement a global error limit.
    uint32_t clientErrorCount = 0;
    // Tracks the last epoch that was a full typecheck. Used to sanity check clientErrorCount.
    uint32_t lastFullTypecheckEpoch = 0;

public:
    ErrorReporter(std::shared_ptr<const LSPConfiguration> config);
    std::vector<core::FileRef> filesWithErrorsSince(uint32_t epoch);
    /**
     * Sends diagnostics from a typecheck run of a single file to the client.
     * `epoch` specifies the epoch of the file updates that produced these diagnostics. Used to prevent emitting
     * outdated diagnostics from a slow path run if they had already been re-typechecked on the fast path.
     */
    void pushDiagnostics(uint32_t epoch, core::FileRef file, const std::vector<std::unique_ptr<core::Error>> &errors,
                         const core::GlobalState &gs);

    /**
     * Checks whether, given the ErrorStatus for the file, diagnostics would even be reported for
     * this file.
     *
     * Sometimes the lastReportedEpoch can be greater than the file's epoch. The file's epoch is a
     * function of which edit last changed the source contents of the file, while the
     * lastReportedEpoch is a function of which edit last triggered a fast or slow path which caused
     * the file to be typechecked, whether as a part of the edit or included by way of looking for
     * downstream files.
     *
     * When this happens, it means we can short circuit, because the file has already been
     * typechecked by a followup edit.
     */
    bool wouldReportForFile(uint32_t epoch, core::FileRef file) const;

    void beginEpoch(uint32_t epoch, bool isIncremental, std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers);
    void endEpoch(uint32_t epoch, bool committed = true);
    uint32_t lastDiagnosticEpochForFile(core::FileRef file);

    // Sanity checks error count data.
    void sanityCheck() const;
};

class ErrorEpoch final {
    ErrorReporter &errorReporter;
    uint32_t epoch;

public:
    ErrorEpoch(ErrorReporter &errorReporter, uint32_t epoch, bool isIncremental,
               std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers);

    ~ErrorEpoch();

    bool committed = false;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_ERRORREPORTER_H
