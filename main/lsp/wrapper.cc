#include "wrapper.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include <iostream>
#include <regex>

using namespace std;
namespace sorbet::realmain::lsp {

const std::string LSPWrapper::EMPTY_STRING = "";

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(const LSPMessage &message) {
    auto result = lspLoop->processRequest(move(gs), message);
    gs = move(result.gs);

    // Should always run typechecking at least once for each request post-initialization.
    ENFORCE(!initialized || gs->lspTypecheckCount > 0, "Fatal error: LSPLoop did not typecheck GlobalState.");
    if (message.isNotification() && message.method() == LSPMethod::Initialize) {
        initialized = true;
    }

    return move(result.responses);
}

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(vector<unique_ptr<LSPMessage>> &messages) {
    // Determine boolean before moving messages.
    bool foundPostInitializationRequest = messages.size() > 0;
    if (!initialized) {
        foundPostInitializationRequest = false;
        for (auto &message : messages) {
            if (initialized) {
                foundPostInitializationRequest = true;
                break;
            } else if (message->isNotification() && message->method() == LSPMethod::Initialized) {
                initialized = true;
            }
        }
    }

    auto result = lspLoop->processRequests(move(gs), move(messages));
    gs = move(result.gs);

    // Should always run typechecking at least once for each request post-initialization.
    ENFORCE(!initialized || !foundPostInitializationRequest || gs->lspTypecheckCount > 0,
            "Fatal error: LSPLoop did not typecheck GlobalState.");

    return move(result.responses);
}

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(const string &message) {
    return getLSPResponsesFor(*LSPMessage::fromClient(message));
}

void LSPWrapper::instantiate(std::unique_ptr<core::GlobalState> gs, const shared_ptr<spdlog::logger> &logger,
                             bool disableFastPath) {
    ENFORCE(gs->errorQueue->ignoreFlushes); // LSP needs this
    workers = make_unique<WorkerPool>(0, logger);
    // N.B.: stdin will not actually be used the way we are driving LSP.
    // Configure LSPLoop to disable configatron.
    lspLoop = make_unique<LSPLoop>(std::move(gs), opts, logger, *workers.get(), STDIN_FILENO, lspOstream, true,
                                   disableFastPath);
}

LSPWrapper::LSPWrapper(string_view rootPath, bool disableFastPath) {
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
    instantiate(std::move(gs), logger, disableFastPath);
}

LSPWrapper::LSPWrapper(unique_ptr<core::GlobalState> gs, options::Options &&options,
                       const shared_ptr<spdlog::logger> &logger, bool disableFastPath)
    : opts(std::move(options)) {
    instantiate(std::move(gs), logger, disableFastPath);
}

int LSPWrapper::getTypecheckCount() const {
    if (gs) {
        return gs->lspTypecheckCount;
    }
    return 0;
}

void LSPWrapper::enableAllExperimentalFeatures() {
    enableExperimentalFeature(LSPExperimentalFeature::Hover);
    enableExperimentalFeature(LSPExperimentalFeature::GoToDefinition);
    enableExperimentalFeature(LSPExperimentalFeature::FindReferences);
    enableExperimentalFeature(LSPExperimentalFeature::Autocomplete);
    enableExperimentalFeature(LSPExperimentalFeature::WorkspaceSymbols);
    enableExperimentalFeature(LSPExperimentalFeature::DocumentSymbol);
    enableExperimentalFeature(LSPExperimentalFeature::SignatureHelp);
}

void LSPWrapper::enableExperimentalFeature(LSPExperimentalFeature feature) {
    switch (feature) {
        case LSPExperimentalFeature::Hover:
            opts.lspHoverEnabled = true;
            break;
        case LSPExperimentalFeature::GoToDefinition:
            opts.lspGoToDefinitionEnabled = true;
            break;
        case LSPExperimentalFeature::FindReferences:
            opts.lspFindReferencesEnabled = true;
            break;
        case LSPExperimentalFeature::Autocomplete:
            opts.lspAutocompleteEnabled = true;
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
