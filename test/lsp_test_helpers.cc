#include "gtest/gtest.h"
#include <cxxopts.hpp>
// has to go first as it violates requirements

#include "spdlog/spdlog.h"
// has to come before the next one. This comment stops formatter from reordering them
#include "spdlog/sinks/stdout_color_sinks.h"
#include "test/lsp_test_helpers.h"

namespace sorbet::test {
using namespace std;

unique_ptr<SymbolConfiguration> makeSymbolConfiguration() {
    auto config = make_unique<SymbolConfiguration>();
    config->dynamicRegistration = true;
    auto symbolOptions = make_unique<SymbolKindOptions>();
    symbolOptions->valueSet = getAllEnumKinds<SymbolKind, SymbolKind::File, SymbolKind::TypeParameter>();
    config->symbolKind = std::move(symbolOptions);
    return config;
}

unique_ptr<WorkspaceClientCapabilities> makeWorkspaceClientCapabilities() {
    auto capabilities = make_unique<WorkspaceClientCapabilities>();
    // The following client config options were cargo-culted from existing tests.
    // TODO(jvilk): Prune these down to only the ones we care about.
    capabilities->applyEdit = true;
    auto workspaceEdit = make_unique<WorkspaceEditCapabilities>();
    workspaceEdit->documentChanges = true;
    capabilities->workspaceEdit = unique_ptr<WorkspaceEditCapabilities>(std::move(workspaceEdit));
    capabilities->didChangeConfiguration = makeDynamicRegistrationOption(true);
    capabilities->didChangeWatchedFiles = makeDynamicRegistrationOption(true);
    capabilities->symbol = makeSymbolConfiguration();
    capabilities->executeCommand = makeDynamicRegistrationOption(true);
    capabilities->configuration = true;
    capabilities->workspaceFolders = true;
    return capabilities;
}

unique_ptr<TextDocumentClientCapabilities> makeTextDocumentClientCapabilities() {
    auto capabilities = make_unique<TextDocumentClientCapabilities>();

    auto publishDiagnostics = make_unique<PublishDiagnosticsCapabilities>();
    publishDiagnostics->relatedInformation = true;
    capabilities->publishDiagnostics = std::move(publishDiagnostics);

    auto synchronization = makeDynamicRegistrationOption<SynchronizationCapabilities>(true);
    synchronization->willSave = true;
    synchronization->willSaveWaitUntil = true;
    synchronization->didSave = true;
    capabilities->synchronization = std::move(synchronization);

    auto completion = makeDynamicRegistrationOption<CompletionCapabilities>(true);
    completion->contextSupport = true;
    auto completionItem = make_unique<CompletionItemCapabilities>();
    completionItem->snippetSupport = true;
    completionItem->commitCharactersSupport = true;
    completionItem->documentationFormat = {MarkupKind::Markdown, MarkupKind::Plaintext};
    completion->completionItem = std::move(completionItem);
    auto completionItemKind = make_unique<CompletionItemKindCapabilities>();
    completionItemKind->valueSet =
        getAllEnumKinds<CompletionItemKind, CompletionItemKind::Text, CompletionItemKind::TypeParameter>();
    completion->completionItemKind = std::move(completionItemKind);
    capabilities->completion = std::move(completion);

    auto hover = makeDynamicRegistrationOption<HoverCapabilities>(true);
    hover->contentFormat = {MarkupKind::Markdown, MarkupKind::Plaintext};
    capabilities->hover = std::move(hover);

    auto signatureHelp = makeDynamicRegistrationOption<SignatureHelpCapabilities>(true);
    auto signatureInformation = make_unique<SignatureInformationCapabilities>();
    signatureInformation->documentationFormat = {MarkupKind::Markdown, MarkupKind::Plaintext};
    signatureHelp->signatureInformation = std::move(signatureInformation);
    capabilities->signatureHelp = std::move(signatureHelp);

    auto documentSymbol = makeDynamicRegistrationOption<DocumentSymbolCapabilities>(true);
    auto symbolKind = make_unique<SymbolKindOptions>();
    symbolKind->valueSet = getAllEnumKinds<SymbolKind, SymbolKind::File, SymbolKind::TypeParameter>();
    documentSymbol->symbolKind = move(symbolKind);
    capabilities->documentSymbol = move(documentSymbol);

    capabilities->definition = makeDynamicRegistrationOption(true);
    capabilities->references = makeDynamicRegistrationOption(true);
    capabilities->documentHighlight = makeDynamicRegistrationOption(true);
    capabilities->codeAction = makeDynamicRegistrationOption<CodeActionCapabilities>(true);
    capabilities->codeLens = makeDynamicRegistrationOption(true);
    capabilities->formatting = makeDynamicRegistrationOption(true);
    capabilities->rangeFormatting = makeDynamicRegistrationOption(true);
    capabilities->onTypeFormatting = makeDynamicRegistrationOption(true);
    capabilities->rename = makeDynamicRegistrationOption<RenameCapabilities>(true);
    capabilities->documentLink = makeDynamicRegistrationOption(true);
    capabilities->typeDefinition = makeDynamicRegistrationOption(true);
    capabilities->implementation = makeDynamicRegistrationOption(true);
    capabilities->colorProvider = makeDynamicRegistrationOption(true);

    return capabilities;
}

unique_ptr<JSONBaseType> makeInitializeParams(string rootPath, string rootUri) {
    auto initializeParams = make_unique<InitializeParams>();
    initializeParams->processId = 12345;
    initializeParams->rootPath = rootPath;
    initializeParams->rootUri = rootUri;
    initializeParams->capabilities = make_unique<ClientCapabilities>();
    initializeParams->capabilities->workspace = makeWorkspaceClientCapabilities();
    initializeParams->capabilities->textDocument = makeTextDocumentClientCapabilities();
    initializeParams->trace = TraceKind::Off;

    auto workspaceFolder = make_unique<WorkspaceFolder>();
    workspaceFolder->uri = rootUri;
    workspaceFolder->name = "pay-server";

    vector<unique_ptr<WorkspaceFolder>> workspaceFolders;
    workspaceFolders.push_back(move(workspaceFolder));
    initializeParams->workspaceFolders =
        make_optional<variant<JSONNullObject, vector<unique_ptr<WorkspaceFolder>>>>(move(workspaceFolders));
    return initializeParams;
}

unique_ptr<RequestMessage> makeRequestMessage(unique_ptr<JSONDocument<int>> &doc, string method, int id,
                                              unique_ptr<JSONBaseType> &params) {
    auto initialize = make_unique<RequestMessage>();
    initialize->jsonrpc = "2.0";
    initialize->method = method;
    initialize->id = id;
    initialize->params = params->toJSONValue(doc);
    return initialize;
}

/** Checks that we are properly advertising Sorbet LSP's capabilities to clients. */
void checkServerCapabilities(const unique_ptr<ServerCapabilities> &capabilities) {
    // Properties checked in the same order they are described in the LSP spec.
    EXPECT_TRUE(capabilities->textDocumentSync.has_value());
    auto &textDocumentSync = *(capabilities->textDocumentSync);
    auto textDocumentSyncValue = get_if<TextDocumentSyncKind>(&textDocumentSync);
    EXPECT_NE(nullptr, textDocumentSyncValue);
    if (textDocumentSyncValue != nullptr) {
        EXPECT_EQ(TextDocumentSyncKind::Incremental, *(textDocumentSyncValue));
    }

    EXPECT_TRUE(capabilities->hoverProvider.value_or(false));

    EXPECT_TRUE(capabilities->completionProvider.has_value());
    if (capabilities->completionProvider.has_value()) {
        auto &completionProvider = *(capabilities->completionProvider);
        auto triggerCharacters = completionProvider->triggerCharacters.value_or(vector<string>({}));
        EXPECT_EQ(1, triggerCharacters.size());
        if (triggerCharacters.size() == 1) {
            EXPECT_EQ(".", triggerCharacters.at(0));
        }
    }

    EXPECT_TRUE(capabilities->signatureHelpProvider.has_value());
    if (capabilities->signatureHelpProvider.has_value()) {
        auto &sigHelpProvider = *(capabilities->signatureHelpProvider);
        auto sigHelpTriggerChars = sigHelpProvider->triggerCharacters.value_or(vector<string>({}));
        EXPECT_EQ(2, sigHelpTriggerChars.size());
        UnorderedSet<string> sigHelpTriggerSet(sigHelpTriggerChars.begin(), sigHelpTriggerChars.end());
        EXPECT_NE(sigHelpTriggerSet.end(), sigHelpTriggerSet.find("("));
        EXPECT_NE(sigHelpTriggerSet.end(), sigHelpTriggerSet.find(","));
    }

    // We don't support all possible features. Make sure we don't make any false claims.
    EXPECT_TRUE(capabilities->definitionProvider.value_or(false));
    EXPECT_FALSE(capabilities->typeDefinitionProvider.has_value());
    EXPECT_FALSE(capabilities->implementationProvider.has_value());
    EXPECT_TRUE(capabilities->referencesProvider.value_or(false));
    EXPECT_FALSE(capabilities->documentHighlightProvider.has_value());
    EXPECT_TRUE(capabilities->documentSymbolProvider.value_or(false));
    EXPECT_TRUE(capabilities->workspaceSymbolProvider.value_or(false));
    EXPECT_FALSE(capabilities->codeActionProvider.has_value());
    EXPECT_FALSE(capabilities->codeLensProvider.has_value());
    EXPECT_FALSE(capabilities->documentFormattingProvider.has_value());
    EXPECT_FALSE(capabilities->documentRangeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities->documentRangeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities->documentOnTypeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities->renameProvider.has_value());
    EXPECT_FALSE(capabilities->documentLinkProvider.has_value());
    EXPECT_FALSE(capabilities->colorProvider.has_value());
    EXPECT_FALSE(capabilities->foldingRangeProvider.has_value());
    EXPECT_FALSE(capabilities->executeCommandProvider.has_value());
    EXPECT_FALSE(capabilities->workspace.has_value());
}

void failWithUnexpectedLSPResponse(const unique_ptr<JSONDocument<JSONBaseType>> &item) {
    if (auto rootPtr = dynamic_cast<ResponseMessage *>(item->root.get())) {
        FAIL() << fmt::format("Expected a notification, but received the following response message instead: {}",
                              rootPtr->toJSON());
    } else if (auto rootPtr = dynamic_cast<NotificationMessage *>(item->root.get())) {
        FAIL() << fmt::format("Expected a response message, but received the following notification instead: {}",
                              rootPtr->toJSON());
    } else {
        // Received something else! This can *only* happen if our test logic is buggy. Any invalid LSP responses
        // are rejected before they reach this point.
        FAIL() << "Received unexpected response message type; this should never happen.";
    }
}

optional<unique_ptr<JSONDocument<ResponseMessage>>>
assertResponseMessage(int expectedId, unique_ptr<JSONDocument<JSONBaseType>> &response) {
    auto maybeRespMsgDoc = response->dynamicCast<ResponseMessage>();
    if (!maybeRespMsgDoc) {
        failWithUnexpectedLSPResponse(response);
        return nullopt;
    }

    auto &respMsgDoc = *maybeRespMsgDoc;
    auto idIntPtr = get_if<int>(&respMsgDoc->root->id);
    EXPECT_NE(nullptr, idIntPtr) << "Response message lacks an integer ID field.";
    if (idIntPtr != nullptr) {
        EXPECT_EQ(expectedId, *idIntPtr) << "Response message's ID does not match expected value.";
    }
    return move(respMsgDoc);
}

optional<unique_ptr<JSONDocument<NotificationMessage>>>
assertNotificationMessage(string expectedMethod, unique_ptr<JSONDocument<JSONBaseType>> &response) {
    auto maybeNotifMsgDoc = response->dynamicCast<NotificationMessage>();
    if (!maybeNotifMsgDoc) {
        failWithUnexpectedLSPResponse(response);
        return nullopt;
    }

    auto &notifMsgDoc = *maybeNotifMsgDoc;
    EXPECT_EQ(expectedMethod, notifMsgDoc->root->method) << "Unexpected method on notification message.";
    return move(notifMsgDoc);
}

optional<unique_ptr<PublishDiagnosticsParams>>
getPublishDiagnosticParams(const unique_ptr<JSONDocument<NotificationMessage>> &doc) {
    auto &notifMsg = doc->root;
    if (!notifMsg->params.has_value()) {
        ADD_FAILURE() << "textDocument/publishDiagnostics message is missing parameters.";
        return nullopt;
    }
    auto &params = *notifMsg->params;
    auto paramsValuePtr = get_if<unique_ptr<rapidjson::Value>>(&params);
    if (!paramsValuePtr) {
        // It's an array rather than a single object.
        ADD_FAILURE() << "textDocument/publishDiagnostics message unexpectedly had array in `params` field";
        return nullopt;
    }
    auto &paramsValue = *paramsValuePtr;
    // TODO(jvilk): Need a better way to unwrap these.
    return PublishDiagnosticsParams::fromJSONValue(doc->memoryOwner->GetAllocator(), *paramsValue.get(),
                                                   "NotificationMessage.params");
}
} // namespace sorbet::test
