#include "main/lsp/ErrorReporter.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {
using namespace std;
ErrorReporter::ErrorReporter(shared_ptr<const LSPConfiguration> config) : config(config) {}

const vector<u4> &ErrorReporter::getDiagnosticEpochs() const {
    return diagnosticEpochs;
};

const UnorderedSet<core::FileRef> &ErrorReporter::getFilesThatHaveErrors() const {
    return filesThatHaveErrors;
};

void ErrorReporter::setMaxFileId(u4 id) {
    ENFORCE(id >= diagnosticEpochs.size());
    diagnosticEpochs.resize(id + 1, 0);
};

void ErrorReporter::pushDiagnostics(u4 epoch, core::FileRef file, vector<unique_ptr<core::Error>> errors,
                                    std::unique_ptr<core::GlobalState> &gs) {
    const string uri = config->fileRef2Uri(*gs, file);
    config->logger->debug("[ErrorReporter] Sending diagnostics for file {}, epoch {}", uri, epoch);

    if (file.id() >= diagnosticEpochs.size()) {
        setMaxFileId(file.id());
    }

    // Update epoch of the file that was typechecked, since we've recalculated the set of diagnostics in these
    if (diagnosticEpochs[file.id()] < epoch) {
        diagnosticEpochs[file.id()] = epoch;
    }

    ENFORCE(diagnosticEpochs[file.id()] <= epoch);
    ENFORCE(file.data(*gs).epoch <= epoch);

    if (errors.empty()) {
        // If errors is empty, but the file previously had errors, remove the file from filesThatHaveErrors
        // and send the empty errors to VSCode to clear the notifications. If the file had no errors previously,
        // break
        if (filesThatHaveErrors.find(file) == filesThatHaveErrors.end()) {
            return;
        }
        filesThatHaveErrors.erase(file);
    } else {
        filesThatHaveErrors.insert(file);
    }

    vector<unique_ptr<Diagnostic>> diagnostics;
    for (auto &error : errors) {
        auto range = Range::fromLoc(*gs, error->loc);
        if (range == nullptr) {
            continue;
        }
        auto diagnostic = make_unique<Diagnostic>(std::move(range), error->header);
        diagnostic->code = error->what.code;
        diagnostic->severity = DiagnosticSeverity::Error;

        vector<unique_ptr<DiagnosticRelatedInformation>> relatedInformation;
        // adding diagnostics to a vector to send to VS Code
        for (auto &section : error->sections) {
            string sectionHeader = section.header;

            for (auto &errorLine : section.messages) {
                string message;
                if (errorLine.formattedMessage.length() > 0) {
                    message = errorLine.formattedMessage;
                } else {
                    message = sectionHeader;
                }
                auto location = config->loc2Location(*gs, errorLine.loc);
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
            diagnostic->relatedInformation = move(relatedInformation);
            diagnostics.push_back(move(diagnostic));
        }
    }

    auto params = make_unique<PublishDiagnosticsParams>(uri, move(diagnostics));
    config->output->write(make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentPublishDiagnostics, move(params))));
};
} // namespace sorbet::realmain::lsp