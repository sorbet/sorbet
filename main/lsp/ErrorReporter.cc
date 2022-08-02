#include "main/lsp/ErrorReporter.h"
#include "core/errors/infer.h"
#include "core/errors/internal.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
namespace sorbet::realmain::lsp {
using namespace std;
ErrorReporter::ErrorReporter(shared_ptr<const LSPConfiguration> config) : config(move(config)) {}

vector<core::FileRef> ErrorReporter::filesWithErrorsSince(uint32_t epoch) {
    vector<core::FileRef> filesUpdatedSince;
    for (size_t i = 1; i < fileErrorStatuses.size(); ++i) {
        ErrorStatus fileErrorStatus = fileErrorStatuses[i];
        if (fileErrorStatus.lastReportedEpoch >= epoch && fileErrorStatus.errorCount > 0) {
            filesUpdatedSince.push_back(core::FileRef(i));
        }
    }
    return filesUpdatedSince;
}

void ErrorReporter::beginEpoch(uint32_t epoch, bool isIncremental, vector<unique_ptr<Timer>> diagnosticLatencyTimers) {
    ENFORCE(epochTimers.find(epoch) == epochTimers.end());
    vector<Timer> firstDiagnosticLatencyTimers;
    if (config->getClientConfig().enableTypecheckInfo) {
        Timer::timedSleep(chrono::milliseconds(5), *config->logger, "delay so timer is reported");
    }
    for (auto &timer : diagnosticLatencyTimers) {
        firstDiagnosticLatencyTimers.emplace_back(timer->clone("first_diagnostic_latency"));
    }

    if (!isIncremental) {
        // Sorbet is going to retypecheck every file. Reset the error count so we do not suppress new errors.
        this->lastFullTypecheckEpoch = epoch;
        this->clientErrorCount = 0;
    }

    epochTimers[epoch] = EpochTimers{move(firstDiagnosticLatencyTimers), move(diagnosticLatencyTimers)};
}

void ErrorReporter::endEpoch(uint32_t epoch, bool committed) {
    auto it = epochTimers.find(epoch);
    ENFORCE(it != epochTimers.end());
    if (!committed) {
        for (auto &timer : it->second.lastDiagnosticLatencyTimers) {
            timer->cancel();
        }

        for (auto &timer : it->second.firstDiagnosticLatencyTimers) {
            timer.cancel();
        }
    }

    epochTimers.erase(it);
}

namespace {

void assertNoCritical(const vector<unique_ptr<core::Error>> &errors) {
    if constexpr (!debug_mode) {
        return;
    }

    for (const auto &error : errors) {
        if (error->isCritical()) {
            // Fail fast.
            // Exception::raise will have already printed a backtrace, we just need to crash the process.
            __builtin_trap();
        }
    }
}

} // namespace

void ErrorReporter::pushDiagnostics(uint32_t epoch, core::FileRef file, const vector<unique_ptr<core::Error>> &errors,
                                    const core::GlobalState &gs) {
    ENFORCE(file.exists());

    ErrorStatus &fileErrorStatus = getFileErrorStatus(file);
    if (fileErrorStatus.lastReportedEpoch > epoch) {
        // Internal errors should always crash Sorbet in tests. Most of the time though, ENFORCE
        // failures and Exception::raise translate into user-visible errors with `beginError`.
        // Regardless of epoch, we want internal errors to crash the process.
        assertNoCritical(errors);
        return;
    }

    // Update clientErrorCount
    if (fileErrorStatus.lastReportedEpoch >= this->lastFullTypecheckEpoch) {
        // clientErrorCount counts the errors from the last reported epoch. Subtract them since this action will
        // revoke them via publishing a new error list.
        this->clientErrorCount = this->clientErrorCount - fileErrorStatus.errorCount;
    }

    uint32_t errorsToReport = errors.size();
    {
        const auto maxErrors = config->opts.lspErrorCap;
        // N.B.: A value of 0 means no maximum is imposed.
        if (maxErrors > 0) {
            if (maxErrors > this->clientErrorCount) {
                errorsToReport = min(static_cast<uint32_t>(errors.size()), maxErrors - this->clientErrorCount);
            } else {
                errorsToReport = 0;
            }
        }
    }

    this->clientErrorCount += errorsToReport;

    fileErrorStatus.lastReportedEpoch = epoch;

    auto it = epochTimers.find(epoch);
    ENFORCE(it != epochTimers.end());
    if (!it->second.hasFirstDiagnosticEndTimes) {
        it->second.hasFirstDiagnosticEndTimes = true;
        for (auto &timer : it->second.firstDiagnosticLatencyTimers) {
            timer.setEndTime();
        }

        for (auto &timer : it->second.lastDiagnosticLatencyTimers) {
            timer->setEndTime();
        }
    }

    // If the error reporter is not going to report errors and the file had no errors previously, break
    // Avoids an issue where ErrorReporter sends an empty error list for files that have hidden errors.
    if (errorsToReport == 0 && fileErrorStatus.errorCount == 0) {
        return;
    }

    fileErrorStatus.errorCount = errorsToReport;

    const string uri = config->fileRef2Uri(gs, file);
    vector<unique_ptr<Diagnostic>> diagnostics;
    for (auto &error : errors) {
        if (errorsToReport == 0) {
            break;
        }
        errorsToReport--;

        ENFORCE(!error->isSilenced);

        auto range = Range::fromLoc(gs, error->loc);
        if (range == nullptr) {
            continue;
        }

        auto diagnostic = make_unique<Diagnostic>(std::move(range), error->header);
        diagnostic->code = error->what.code;
        if (error->what.code == sorbet::core::errors::Infer::DeadBranchInferencer.code) {
            vector<DiagnosticTag> tags;
            tags.push_back(DiagnosticTag::Unnecessary);
            diagnostic->tags = move(tags);
        }
        diagnostic->severity = DiagnosticSeverity::Error;

        if (!error->autocorrects.empty()) {
            diagnostic->message += " (fix available)";
        }

        vector<unique_ptr<DiagnosticRelatedInformation>> relatedInformation;
        for (auto &section : error->sections) {
            string sectionHeader = section.header;

            for (auto &errorLine : section.messages) {
                string message = errorLine.formattedMessage.length() > 0 ? errorLine.formattedMessage : sectionHeader;
                auto location = config->loc2Location(gs, errorLine.loc);
                if (location == nullptr) {
                    // This was probably from an addErrorNote call. Still want to report the note.
                    location = config->loc2Location(gs, error->loc);
                    message = "\n    " + message;
                }
                if (location == nullptr) {
                    continue;
                }
                relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(std::move(location), message));
            }
        }
        // Add link to error documentation.
        diagnostic->relatedInformation = move(relatedInformation);
        diagnostic->codeDescription =
            make_unique<CodeDescription>(absl::StrCat(config->opts.errorUrlBase, error->what.code));
        diagnostics.push_back(move(diagnostic));
    }

    config->logger->debug("[ErrorReporter] Sending diagnostics for file {}, epoch {}", uri, epoch);
    for (auto &timer : it->second.lastDiagnosticLatencyTimers) {
        timer->setEndTime();
    }
    auto params = make_unique<PublishDiagnosticsParams>(uri, move(diagnostics));
    config->output->write(make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentPublishDiagnostics, move(params))));
};

ErrorStatus &ErrorReporter::getFileErrorStatus(core::FileRef file) {
    if (file.id() >= fileErrorStatuses.size()) {
        fileErrorStatuses.resize(file.id() + 1, ErrorStatus{0, false});
    }
    return fileErrorStatuses[file.id()];
};

uint32_t ErrorReporter::lastDiagnosticEpochForFile(core::FileRef file) {
    return getFileErrorStatus(file).lastReportedEpoch;
}

void ErrorReporter::sanityCheck() const {
    if (!debug_mode) {
        return;
    }

    uint32_t errorCount = 0;
    for (auto &status : this->fileErrorStatuses) {
        if (status.lastReportedEpoch >= this->lastFullTypecheckEpoch) {
            errorCount += status.errorCount;
        }
    }
    ENFORCE(errorCount == this->clientErrorCount);
}

ErrorEpoch::ErrorEpoch(ErrorReporter &errorReporter, uint32_t epoch, bool isIncremental,
                       std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers)
    : errorReporter(errorReporter), epoch(epoch) {
    errorReporter.beginEpoch(epoch, isIncremental, move(diagnosticLatencyTimers));
};

ErrorEpoch::~ErrorEpoch() {
    errorReporter.endEpoch(epoch, committed);
};
} // namespace sorbet::realmain::lsp
