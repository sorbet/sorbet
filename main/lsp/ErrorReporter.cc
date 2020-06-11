#include "main/lsp/ErrorReporter.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
namespace sorbet::realmain::lsp {
using namespace std;
ErrorReporter::ErrorReporter(shared_ptr<const LSPConfiguration> config) : config(move(config)) {}

vector<core::FileRef> ErrorReporter::filesWithErrorsSince(u4 epoch) {
    vector<core::FileRef> filesUpdatedSince;
    for (size_t i = 1; i < fileErrorStatuses.size(); ++i) {
        ErrorStatus fileErrorStatus = fileErrorStatuses[i];
        if (fileErrorStatus.lastReportedEpoch >= epoch && fileErrorStatus.hasErrors) {
            filesUpdatedSince.push_back(core::FileRef(i));
        }
    }
    return filesUpdatedSince;
}

void ErrorReporter::beginEpoch(u4 epoch, vector<unique_ptr<Timer>> diagnosticLatencyTimers) {
    ENFORCE(epochTimers.find(epoch) == epochTimers.end());
    vector<Timer> firstDiagnosticLatencyTimers;
    if (config->getClientConfig().enableTypecheckInfo) {
        Timer::timedSleep(chrono::milliseconds(50), *config->logger, "delay so timer is reported");
    }
    for (auto &timer : diagnosticLatencyTimers) {
        firstDiagnosticLatencyTimers.emplace_back(timer->clone("first_diagnostic_latency"));
    }

    epochTimers[epoch] = EpochTimers{move(firstDiagnosticLatencyTimers), move(diagnosticLatencyTimers)};
}

void ErrorReporter::endEpoch(u4 epoch, bool committed) {
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

void ErrorReporter::pushDiagnostics(u4 epoch, core::FileRef file, const vector<unique_ptr<core::Error>> &errors,
                                    const core::GlobalState &gs) {
    ENFORCE(file.exists());

    ErrorStatus &fileErrorStatus = getFileErrorStatus(file);
    if (fileErrorStatus.lastReportedEpoch > epoch) {
        return;
    }

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

    // If errors is empty and the file had no errors previously, break
    if (errors.empty() && fileErrorStatus.hasErrors == false) {
        return;
    }

    fileErrorStatus.hasErrors = !errors.empty();

    const string uri = config->fileRef2Uri(gs, file);
    vector<unique_ptr<Diagnostic>> diagnostics;
    for (auto &error : errors) {
        ENFORCE(!error->isSilenced);

        auto range = Range::fromLoc(gs, error->loc);
        if (range == nullptr) {
            continue;
        }

        auto diagnostic = make_unique<Diagnostic>(std::move(range), error->header);
        diagnostic->code = error->what.code;
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
                    continue;
                }
                relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(std::move(location), message));
            }
        }
        // Add link to error documentation.
        relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(
            make_unique<Location>(absl::StrCat(config->opts.errorUrlBase, error->what.code),
                                  make_unique<Range>(make_unique<Position>(0, 0), make_unique<Position>(0, 0))),
            "Click for more information on this error."));
        diagnostic->relatedInformation = move(relatedInformation);
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

ErrorEpoch::ErrorEpoch(ErrorReporter &errorReporter, u4 epoch,
                       std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers)
    : errorReporter(errorReporter), epoch(epoch) {
    errorReporter.beginEpoch(epoch, move(diagnosticLatencyTimers));
};

ErrorEpoch::~ErrorEpoch() {
    errorReporter.endEpoch(epoch, committed);
};
} // namespace sorbet::realmain::lsp
