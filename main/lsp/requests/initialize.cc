#include "main/lsp/requests/initialize.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

const std::vector<std::string> InitializeTask::TRIGGER_CHARACTERS = {".", ":", "@", "#"};

InitializeTask::InitializeTask(LSPConfiguration &config, MessageId id, std::unique_ptr<InitializeParams> params)
    : LSPRequestTask(config, id, LSPMethod::Initialize), mutableConfig(config), params(move(params)) {}

bool InitializeTask::canPreempt(const LSPIndexer &indexer) const {
    return false;
}

void InitializeTask::preprocess(LSPPreprocessor &preprocessor) {
    mutableConfig.setClientConfig(make_shared<LSPClientConfiguration>(*params));
}

unique_ptr<ResponseMessage> InitializeTask::runRequest(LSPTypecheckerDelegate &ts) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::Initialize);
    const auto &opts = config.opts;
    auto serverCap = make_unique<ServerCapabilities>();
    serverCap->textDocumentSync = TextDocumentSyncKind::Full;
    serverCap->definitionProvider = true;
    serverCap->typeDefinitionProvider = true;
    serverCap->documentSymbolProvider = opts.lspDocumentSymbolEnabled;
    serverCap->workspaceSymbolProvider = true;
    serverCap->documentHighlightProvider = opts.lspDocumentHighlightEnabled;
    serverCap->hoverProvider = true;
    serverCap->referencesProvider = true;
    serverCap->implementationProvider = true;
    serverCap->documentFormattingProvider = opts.lspDocumentFormatRubyfmtEnabled;
    serverCap->sorbetShowSymbolProvider = true;

    auto codeActionProvider = make_unique<CodeActionOptions>();
    codeActionProvider->codeActionKinds = {CodeActionKind::Quickfix, CodeActionKind::SourceFixAllSorbet,
                                           CodeActionKind::RefactorExtract, CodeActionKind::RefactorRewrite};
    codeActionProvider->resolveProvider = true;
    serverCap->codeActionProvider = move(codeActionProvider);

    if (opts.lspSignatureHelpEnabled) {
        auto sigHelpProvider = make_unique<SignatureHelpOptions>();
        sigHelpProvider->triggerCharacters = {"(", ","};
        serverCap->signatureHelpProvider = move(sigHelpProvider);
    }

    serverCap->renameProvider = make_unique<RenameOptions>(true);

    auto completionProvider = make_unique<CompletionOptions>();
    completionProvider->triggerCharacters = TRIGGER_CHARACTERS;
    serverCap->completionProvider = move(completionProvider);

    response->result = make_unique<InitializeResult>(move(serverCap));
    return response;
}
} // namespace sorbet::realmain::lsp
