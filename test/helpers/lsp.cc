#include "gtest/gtest.h"
#include <cxxopts.hpp>
// has to go first as it violates requirements

#include "common/common.h"
#include "test/helpers/lsp.h"

namespace sorbet::test {
using namespace std;

template <typename T = DynamicRegistrationOption>
std::unique_ptr<T> makeDynamicRegistrationOption(bool dynamicRegistration) {
    auto option = std::make_unique<T>();
    option->dynamicRegistration = dynamicRegistration;
    return option;
};

/** Constructs a vector with all enum values from MIN to MAX. Assumes a contiguous enum and properly chosen min/max
 * values. Our serialization/deserialization code will throw if we pick an improper value. */
template <typename T, T MAX, T MIN> std::vector<T> getAllEnumKinds() {
    std::vector<T> symbols;
    for (int i = (int)MIN; i <= (int)MAX; i++) {
        symbols.push_back((T)i);
    }
    return symbols;
};

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

unique_ptr<TextDocumentClientCapabilities> makeTextDocumentClientCapabilities(bool supportsMarkdown) {
    auto capabilities = make_unique<TextDocumentClientCapabilities>();
    vector<MarkupKind> supportedTextFormats({MarkupKind::Plaintext});
    if (supportsMarkdown) {
        supportedTextFormats.push_back(MarkupKind::Markdown);
    }

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
    completionItem->documentationFormat = supportedTextFormats;
    completion->completionItem = std::move(completionItem);
    auto completionItemKind = make_unique<CompletionItemKindCapabilities>();
    completionItemKind->valueSet =
        getAllEnumKinds<CompletionItemKind, CompletionItemKind::Text, CompletionItemKind::TypeParameter>();
    completion->completionItemKind = std::move(completionItemKind);
    capabilities->completion = std::move(completion);

    auto hover = makeDynamicRegistrationOption<HoverCapabilities>(true);
    hover->contentFormat = supportedTextFormats;
    capabilities->hover = std::move(hover);

    auto signatureHelp = makeDynamicRegistrationOption<SignatureHelpCapabilities>(true);
    auto signatureInformation = make_unique<SignatureInformationCapabilities>();
    signatureInformation->documentationFormat = supportedTextFormats;
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

unique_ptr<InitializeParams> makeInitializeParams(variant<string, JSONNullObject> rootPath,
                                                  variant<string, JSONNullObject> rootUri, bool enableTypecheckInfo,
                                                  bool supportsMarkdown) {
    auto initializeParams = make_unique<InitializeParams>(rootPath, rootUri, make_unique<ClientCapabilities>());
    initializeParams->capabilities->workspace = makeWorkspaceClientCapabilities();
    initializeParams->capabilities->textDocument = makeTextDocumentClientCapabilities(supportsMarkdown);
    initializeParams->trace = TraceKind::Off;

    string stringRootUri = "";
    if (auto str = get_if<string>(&rootUri)) {
        stringRootUri = *str;
    }
    auto workspaceFolder = make_unique<WorkspaceFolder>(stringRootUri, "pay-server");
    vector<unique_ptr<WorkspaceFolder>> workspaceFolders;
    workspaceFolders.push_back(move(workspaceFolder));
    initializeParams->workspaceFolders =
        make_optional<variant<JSONNullObject, vector<unique_ptr<WorkspaceFolder>>>>(move(workspaceFolders));

    auto sorbetInitParams = make_unique<SorbetInitializationOptions>();
    sorbetInitParams->enableTypecheckInfo = enableTypecheckInfo;
    initializeParams->initializationOptions = move(sorbetInitParams);
    return initializeParams;
}

unique_ptr<LSPMessage> makeDefinitionRequest(int id, std::string_view uri, int line, int character) {
    return make_unique<LSPMessage>(make_unique<RequestMessage>(
        "2.0", id, LSPMethod::TextDocumentDefinition,
        make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(string(uri)),
                                                make_unique<Position>(line, character))));
}

/** Checks that we are properly advertising Sorbet LSP's capabilities to clients. */
void checkServerCapabilities(const ServerCapabilities &capabilities) {
    // Properties checked in the same order they are described in the LSP spec.
    EXPECT_TRUE(capabilities.textDocumentSync.has_value());
    auto &textDocumentSync = *(capabilities.textDocumentSync);
    auto textDocumentSyncValue = get<TextDocumentSyncKind>(textDocumentSync);
    EXPECT_EQ(TextDocumentSyncKind::Full, textDocumentSyncValue);

    EXPECT_TRUE(capabilities.hoverProvider.value_or(false));

    EXPECT_TRUE(capabilities.completionProvider.has_value());
    if (capabilities.completionProvider.has_value()) {
        auto &completionProvider = *(capabilities.completionProvider);
        auto triggerCharacters = completionProvider->triggerCharacters.value_or(vector<string>({}));
        EXPECT_EQ(1, triggerCharacters.size());
        if (triggerCharacters.size() == 1) {
            EXPECT_EQ(".", triggerCharacters.at(0));
        }
    }

    EXPECT_TRUE(capabilities.signatureHelpProvider.has_value());
    if (capabilities.signatureHelpProvider.has_value()) {
        auto &sigHelpProvider = *(capabilities.signatureHelpProvider);
        auto sigHelpTriggerChars = sigHelpProvider->triggerCharacters.value_or(vector<string>({}));
        EXPECT_EQ(2, sigHelpTriggerChars.size());
        UnorderedSet<string> sigHelpTriggerSet(sigHelpTriggerChars.begin(), sigHelpTriggerChars.end());
        EXPECT_NE(sigHelpTriggerSet.end(), sigHelpTriggerSet.find("("));
        EXPECT_NE(sigHelpTriggerSet.end(), sigHelpTriggerSet.find(","));
    }

    // We don't support all possible features. Make sure we don't make any false claims.
    EXPECT_TRUE(capabilities.definitionProvider.value_or(false));
    EXPECT_FALSE(capabilities.typeDefinitionProvider.has_value());
    EXPECT_FALSE(capabilities.implementationProvider.has_value());
    EXPECT_TRUE(capabilities.referencesProvider.value_or(false));
    EXPECT_FALSE(capabilities.documentHighlightProvider.has_value());
    EXPECT_TRUE(capabilities.documentSymbolProvider.value_or(false));
    EXPECT_TRUE(capabilities.workspaceSymbolProvider.value_or(false));
    EXPECT_TRUE(capabilities.codeActionProvider.has_value());
    EXPECT_FALSE(capabilities.codeLensProvider.has_value());
    EXPECT_FALSE(capabilities.documentFormattingProvider.has_value());
    EXPECT_FALSE(capabilities.documentRangeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities.documentRangeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities.documentOnTypeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities.renameProvider.has_value());
    EXPECT_FALSE(capabilities.documentLinkProvider.has_value());
    EXPECT_FALSE(capabilities.executeCommandProvider.has_value());
    EXPECT_FALSE(capabilities.workspace.has_value());
}

void failWithUnexpectedLSPResponse(const LSPMessage &item) {
    if (item.isResponse()) {
        FAIL() << fmt::format("Expected a notification, but received the following response message instead: {}",
                              item.asResponse().toJSON());
    } else if (item.isNotification()) {
        FAIL() << fmt::format("Expected a response message, but received the following notification instead: {}",
                              item.asNotification().toJSON());
    } else {
        // Received something else! This can *only* happen if our test logic is buggy. Any invalid LSP responses
        // are rejected before they reach this point.
        FAIL() << "Received unexpected response message type; this should never happen.";
    }
}

bool assertResponseMessage(int expectedId, const LSPMessage &response) {
    if (!response.isResponse()) {
        failWithUnexpectedLSPResponse(response);
        return false;
    }

    auto &respMsg = response.asResponse();
    auto idIntPtr = get_if<int>(&respMsg.id);
    EXPECT_NE(nullptr, idIntPtr) << "Response message lacks an integer ID field.";
    if (idIntPtr != nullptr) {
        EXPECT_EQ(expectedId, *idIntPtr) << "Response message's ID does not match expected value.";
        return expectedId == *idIntPtr;
    }
    return false;
}

bool assertResponseError(int code, string_view msg, const LSPMessage &response) {
    EXPECT_TRUE(response.isResponse()) << fmt::format(
        "Expected a response message with error `{}: {}`, but received:\n{}", code, msg, response.toJSON());
    if (response.isResponse()) {
        auto &r = response.asResponse();
        auto &maybeError = r.error;
        if (maybeError.has_value()) {
            auto &error = *maybeError;
            EXPECT_EQ(error->code, code) << fmt::format("Response message contains error with unexpected code:\n{}",
                                                        error->toJSON());

            bool msgMatches = error->message.find(msg) != string::npos;
            EXPECT_TRUE(msgMatches) << fmt::format("Expected a response message with error `{}: {}`, but received:\n{}",
                                                   code, msg, response.toJSON());
            return error->code == code && msgMatches;
        } else {
            ADD_FAILURE() << fmt::format("Expected a response message with an error, but received:\n{}",
                                         response.toJSON());
        }
    }
    return false;
}

bool assertNotificationMessage(const LSPMethod expectedMethod, const LSPMessage &response) {
    if (!response.isNotification()) {
        failWithUnexpectedLSPResponse(response);
        return false;
    }
    EXPECT_EQ(expectedMethod, response.method()) << "Unexpected method on notification message.";
    return expectedMethod == response.method();
}

optional<PublishDiagnosticsParams *> getPublishDiagnosticParams(NotificationMessage &notifMsg) {
    auto publishDiagnosticParams = get_if<unique_ptr<PublishDiagnosticsParams>>(&notifMsg.params);
    if (!publishDiagnosticParams || !*publishDiagnosticParams) {
        ADD_FAILURE() << "textDocument/publishDiagnostics message is missing parameters.";
        return nullopt;
    }
    return (*publishDiagnosticParams).get();
}

vector<unique_ptr<LSPMessage>> initializeLSP(string_view rootPath, string_view rootUri, LSPWrapper &lspWrapper,
                                             int &nextId, bool enableTypecheckInfo, bool supportsMarkdown) {
    // Reset next id.
    nextId = 0;

    // Send 'initialize' message.
    {
        auto initializeParams =
            makeInitializeParams(string(rootPath), string(rootUri), enableTypecheckInfo, supportsMarkdown);
        LSPMessage message(make_unique<RequestMessage>("2.0", nextId++, LSPMethod::Initialize, move(initializeParams)));
        auto responses = lspWrapper.getLSPResponsesFor(message);

        // Should just have an 'initialize' response.
        EXPECT_EQ(1, responses.size());
        if (responses.size() != 1) {
            return {};
        }

        if (assertResponseMessage(0, *responses.at(0))) {
            auto &respMsg = responses.at(0)->asResponse();
            EXPECT_FALSE(respMsg.error.has_value());
            EXPECT_TRUE(respMsg.result.has_value());

            if (respMsg.result.has_value()) {
                auto &result = *respMsg.result;
                auto &initializeResult = get<unique_ptr<InitializeResult>>(result);
                checkServerCapabilities(*initializeResult->capabilities);
            } else {
                return {};
            }
        }
    }

    // Complete initialization handshake with an 'initialized' message.
    {
        rapidjson::Value emptyObject(rapidjson::kObjectType);
        auto initialized =
            make_unique<NotificationMessage>("2.0", LSPMethod::Initialized, make_unique<InitializedParams>());
        return lspWrapper.getLSPResponsesFor(LSPMessage(move(initialized)));
    }
}

unique_ptr<LSPMessage> makeDidChange(std::string_view uri, std::string_view contents, int version) {
    auto textDoc = make_unique<VersionedTextDocumentIdentifier>(string(uri), version);
    auto textDocChange = make_unique<TextDocumentContentChangeEvent>(string(contents));
    vector<unique_ptr<TextDocumentContentChangeEvent>> textChanges;
    textChanges.push_back(move(textDocChange));

    auto didChangeParams = make_unique<DidChangeTextDocumentParams>(move(textDoc), move(textChanges));
    auto didChangeNotif =
        make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidChange, move(didChangeParams));
    return make_unique<LSPMessage>(move(didChangeNotif));
}

} // namespace sorbet::test
