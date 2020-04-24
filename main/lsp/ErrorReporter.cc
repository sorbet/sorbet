#include "main/lsp/ErrorReporter.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {
using namespace std;
ErrorReporter::ErrorReporter(shared_ptr<const LSPConfiguration> config) : config(config) {}

// Used for unit tests
const std::vector<ErrorStatus> &ErrorReporter::getFileErrorStatuses() const {
    return fileErrorStatuses;
};

void ErrorReporter::setMaxFileId(u4 id) {
    ENFORCE(id >= fileErrorStatuses.size());
    fileErrorStatuses.resize(id + 1, ErrorStatus{0, false});
};

void ErrorReporter::pushDiagnostics(u4 epoch, core::FileRef file, vector<unique_ptr<core::Error>> errors,
                                    const core::GlobalState &gs) {
    ENFORCE(file.exists());

    if (file.id() >= fileErrorStatuses.size()) {
        setMaxFileId(file.id());
    }

    ErrorStatus &fileErrorStatus = fileErrorStatuses[file.id()];

    if (file.data(gs).epoch > epoch || fileErrorStatus.sentEpoch > epoch) {
        return;
    }
    // If errors is empty and the file had no errors previously, break
    if (errors.empty() && fileErrorStatus.hasErrors == false) {
        return;
    }

    fileErrorStatus.hasErrors = !errors.empty();

    if (fileErrorStatus.hasErrors) {
        fileErrorStatus.sentEpoch = epoch;
    }

    const string uri = config->fileRef2Uri(gs, file);
    config->logger->debug("[ErrorReporter] Sending diagnostics for file {}, epoch {}", uri, epoch);
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

        vector<unique_ptr<DiagnosticRelatedInformation>> relatedInformation;
        for (auto &section : error->sections) {
            string sectionHeader = section.header;

            for (auto &errorLine : section.messages) {
                string message;
                if (errorLine.formattedMessage.length() > 0) {
                    message = errorLine.formattedMessage;
                } else {
                    message = sectionHeader;
                }
                auto location = config->loc2Location(gs, errorLine.loc);
                if (location == nullptr) {
                    continue;
                }
                relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(std::move(location), message));
            }
            // Add link to error documentation.
            relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(
                make_unique<Location>(absl::StrCat(config->opts.errorUrlBase, error->what.code),
                                      make_unique<Range>(make_unique<Position>(0, 0), make_unique<Position>(0, 0))),
                "Click for more information on this error."));
        }
        diagnostic->relatedInformation = move(relatedInformation);
        diagnostics.push_back(move(diagnostic));
    }

    auto params = make_unique<PublishDiagnosticsParams>(uri, move(diagnostics));
    config->output->write(make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentPublishDiagnostics, move(params))));
};
} // namespace sorbet::realmain::lsp