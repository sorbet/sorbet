#include "wrapper.h"
#include "core/errors/namer.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include <regex>

using namespace std;

namespace sorbet::realmain::lsp {

const std::string LSPWrapper::EMPTY_STRING = "";

class LSPWrapper::LSPOutputToVector final : public LSPOutput {
private:
    vector<unique_ptr<LSPMessage>> output;

public:
    LSPOutputToVector() = default;

    void rawWrite(unique_ptr<LSPMessage> msg) override {
        output.push_back(move(msg));
    }

    std::vector<unique_ptr<LSPMessage>> getOutput() {
        return move(output);
    }
};

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(unique_ptr<LSPMessage> message) {
    auto result = lspLoop->processRequest(move(gs), move(message));
    gs = move(result.gs);

    // Should always run typechecking at least once for each request post-initialization.
    ENFORCE(!config->initialized || gs->lspTypecheckCount > 0, "Fatal error: LSPLoop did not typecheck GlobalState.");

    // Retrieve any notifications that would normally be sent asynchronously in a multithreaded scenario.
    auto notifs = output->getOutput();
    result.responses.insert(result.responses.end(), make_move_iterator(notifs.begin()),
                            make_move_iterator(notifs.end()));

    return move(result.responses);
}

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(vector<unique_ptr<LSPMessage>> &messages) {
    auto result = lspLoop->processRequests(move(gs), move(messages));
    gs = move(result.gs);

    // Should always run typechecking at least once for each request post-initialization.
    ENFORCE(!config->initialized || gs->lspTypecheckCount > 0, "Fatal error: LSPLoop did not typecheck GlobalState.");

    return move(result.responses);
}

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(const string &json) {
    return getLSPResponsesFor(LSPMessage::fromClient(json));
}

void LSPWrapper::instantiate(std::unique_ptr<core::GlobalState> gs, const shared_ptr<spdlog::logger> &logger,
                             bool disableFastPath) {
    output = make_shared<LSPWrapper::LSPOutputToVector>();
    ENFORCE(gs->errorQueue->ignoreFlushes); // LSP needs this
    workers = WorkerPool::create(0, *logger);
    config = make_shared<LSPConfiguration>(opts, output, *workers, logger, true, disableFastPath);
    // Configure LSPLoop to disable configatron.
    lspLoop = make_unique<LSPLoop>(std::move(gs), config);
}

LSPWrapper::LSPWrapper(options::Options &&options, std::string_view rootPath, bool disableFastPath)
    : opts(std::move(options)) {
    opts.rawInputDirNames.emplace_back(rootPath);

    // All of this stuff is ignored by LSP, but we need it to construct ErrorQueue/GlobalState.
    stderrColorSink = make_shared<spd::sinks::ansicolor_stderr_sink_mt>();
    auto logger = make_shared<spd::logger>("console", stderrColorSink);
    typeErrorsConsole = make_shared<spd::logger>("typeDiagnostics", stderrColorSink);
    typeErrorsConsole->set_pattern("%v");
    auto gs = make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger)));
    unique_ptr<KeyValueStore> kvstore;
    payload::createInitialGlobalState(gs, opts, kvstore);

    // If we don't tell the errorQueue to ignore flushes, then we won't get diagnostic messages.
    gs->errorQueue->ignoreFlushes = true;
    if (!opts.stripeMode) {
        // Definitions in multiple locations interact poorly with autoloader this error is enforced
        // in Stripe code.
        gs->suppressErrorClass(sorbet::core::errors::Namer::MultipleBehaviorDefs.code);
    }
    instantiate(std::move(gs), logger, disableFastPath);
}

LSPWrapper::LSPWrapper(string_view rootPath, bool disableFastPath)
    : LSPWrapper(options::Options(), rootPath, disableFastPath) {}

LSPWrapper::LSPWrapper(unique_ptr<core::GlobalState> gs, options::Options &&options,
                       const shared_ptr<spdlog::logger> &logger, bool disableFastPath)
    : opts(std::move(options)) {
    instantiate(std::move(gs), logger, disableFastPath);
}

// Define so we can properly destruct unique_ptr<LSPOutputToVector> (which the default destructor can't delete since we
// forward decl it in the header)
LSPWrapper::~LSPWrapper() {}

int LSPWrapper::getTypecheckCount() const {
    if (gs) {
        return gs->lspTypecheckCount;
    }
    return 0;
}

void LSPWrapper::enableAllExperimentalFeatures() {
    enableExperimentalFeature(LSPExperimentalFeature::Autocomplete);
    enableExperimentalFeature(LSPExperimentalFeature::WorkspaceSymbols);
    enableExperimentalFeature(LSPExperimentalFeature::DocumentSymbol);
    enableExperimentalFeature(LSPExperimentalFeature::SignatureHelp);
    enableExperimentalFeature(LSPExperimentalFeature::QuickFix);
}

void LSPWrapper::enableExperimentalFeature(LSPExperimentalFeature feature) {
    switch (feature) {
        case LSPExperimentalFeature::Autocomplete:
            opts.lspAutocompleteEnabled = true;
            break;
        case LSPExperimentalFeature::QuickFix:
            opts.lspQuickFixEnabled = true;
            break;
        case LSPExperimentalFeature::WorkspaceSymbols:
            opts.lspWorkspaceSymbolsEnabled = true;
            break;
        case LSPExperimentalFeature::DocumentSymbol:
            opts.lspDocumentSymbolEnabled = true;
            break;
        case LSPExperimentalFeature::SignatureHelp:
            opts.lspSignatureHelpEnabled = true;
            break;
    }
}

} // namespace sorbet::realmain::lsp
