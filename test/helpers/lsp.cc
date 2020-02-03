#include "gtest/gtest.h"
#include <cxxopts.hpp>
// has to go first as it violates requirements

#include "absl/strings/match.h"
#include "common/common.h"
#include "common/sort.h"
#include "test/helpers/lsp.h"

namespace sorbet::test {
using namespace std;

string filePathToUri(const LSPConfiguration &config, string_view filePath) {
    return fmt::format("{}/{}", config.getClientConfig().rootUri, filePath);
}

string uriToFilePath(const LSPConfiguration &config, string_view uri) {
    string_view prefixUrl = config.getClientConfig().rootUri;
    if (!config.isUriInWorkspace(uri)) {
        ADD_FAILURE() << fmt::format(
            "Unrecognized URI: `{}` is not contained in root URI `{}`, and thus does not correspond to a test file.",
            uri, prefixUrl);
        return "";
    }
    return string(uri.substr(prefixUrl.length() + 1));
}

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

unique_ptr<InitializeParams>
makeInitializeParams(variant<string, JSONNullObject> rootPath, variant<string, JSONNullObject> rootUri,
                     bool supportsMarkdown, std::optional<std::unique_ptr<SorbetInitializationOptions>> initOptions) {
    auto initializeParams = make_unique<InitializeParams>(rootPath, rootUri, make_unique<ClientCapabilities>());
    initializeParams->capabilities->workspace = makeWorkspaceClientCapabilities();
    initializeParams->capabilities->textDocument = makeTextDocumentClientCapabilities(supportsMarkdown);
    initializeParams->trace = TraceKind::Off;

    string stringRootUri;
    if (auto str = get_if<string>(&rootUri)) {
        stringRootUri = *str;
    }
    auto workspaceFolder = make_unique<WorkspaceFolder>(stringRootUri, "pay-server");
    vector<unique_ptr<WorkspaceFolder>> workspaceFolders;
    workspaceFolders.push_back(move(workspaceFolder));
    initializeParams->workspaceFolders =
        make_optional<variant<JSONNullObject, vector<unique_ptr<WorkspaceFolder>>>>(move(workspaceFolders));

    initializeParams->initializationOptions = move(initOptions);
    return initializeParams;
}

unique_ptr<LSPMessage> makeDefinitionRequest(int id, std::string_view uri, int line, int character) {
    return make_unique<LSPMessage>(make_unique<RequestMessage>(
        "2.0", id, LSPMethod::TextDocumentDefinition,
        make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(string(uri)),
                                                make_unique<Position>(line, character))));
}

unique_ptr<LSPMessage> makeHover(int id, std::string_view uri, int line, int character) {
    return make_unique<LSPMessage>(make_unique<RequestMessage>(
        "2.0", id, LSPMethod::TextDocumentHover,
        make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(string(uri)),
                                                make_unique<Position>(line, character))));
}

unique_ptr<LSPMessage> makeWorkspaceSymbolRequest(int id, std::string_view query) {
    return make_unique<LSPMessage>(make_unique<RequestMessage>("2.0", id, LSPMethod::WorkspaceSymbol,
                                                               make_unique<WorkspaceSymbolParams>(string(query))));
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
    EXPECT_TRUE(capabilities.typeDefinitionProvider.value_or(false));
    EXPECT_FALSE(capabilities.implementationProvider.has_value());
    EXPECT_TRUE(capabilities.referencesProvider.value_or(false));
    EXPECT_TRUE(capabilities.documentHighlightProvider.has_value());
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

void assertResponseMessage(int expectedId, const LSPMessage &response) {
    ASSERT_TRUE(response.isResponse()) << fmt::format(
        "Expected a response message, but received the following notification instead: {}", response.toJSON());

    auto &respMsg = response.asResponse();
    auto idIntPtr = get_if<int>(&respMsg.id);
    ASSERT_NE(nullptr, idIntPtr) << "Response message lacks an integer ID field.";
    ASSERT_EQ(expectedId, *idIntPtr) << "Response message's ID does not match expected value.";
}

void assertResponseError(int code, string_view msg, const LSPMessage &response) {
    ASSERT_TRUE(response.isResponse()) << fmt::format(
        "Expected a response message with error `{}: {}`, but received:\n{}", code, msg, response.toJSON());
    auto &r = response.asResponse();
    auto &maybeError = r.error;
    ASSERT_TRUE(maybeError.has_value()) << fmt::format("Expected a response message with an error, but received:\n{}",
                                                       response.toJSON());
    auto &error = *maybeError;
    ASSERT_EQ(error->code, code) << fmt::format("Response message contains error with unexpected code:\n{}",
                                                error->toJSON());
    ASSERT_NE(error->message.find(msg), string::npos) << fmt::format(
        "Expected a response message with error `{}: {}`, but received:\n{}", code, msg, response.toJSON());
}

void assertNotificationMessage(LSPMethod expectedMethod, const LSPMessage &response) {
    ASSERT_TRUE(response.isNotification()) << fmt::format(
        "Expected a notification, but received the following response message instead: {}", response.toJSON());
    ASSERT_EQ(expectedMethod, response.method())
        << fmt::format("Unexpected method on notification message: expected {} but received {}.",
                       convertLSPMethodToString(expectedMethod), convertLSPMethodToString(response.method()));
}

optional<const PublishDiagnosticsParams *> getPublishDiagnosticParams(const NotificationMessage &notifMsg) {
    auto publishDiagnosticParams = get_if<unique_ptr<PublishDiagnosticsParams>>(&notifMsg.params);
    if (!publishDiagnosticParams || !*publishDiagnosticParams) {
        ADD_FAILURE() << "textDocument/publishDiagnostics message is missing parameters.";
        return nullopt;
    }
    return (*publishDiagnosticParams).get();
}

unique_ptr<CompletionList> doTextDocumentCompletion(LSPWrapper &lspWrapper, const Range &range, int &nextId,
                                                    string_view filename) {
    auto uri = filePathToUri(lspWrapper.config(), filename);
    auto pos = make_unique<CompletionParams>(make_unique<TextDocumentIdentifier>(uri), range.start->copy());
    auto id = nextId++;
    auto msg =
        make_unique<LSPMessage>(make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentCompletion, move(pos)));
    auto responses = getLSPResponsesFor(lspWrapper, move(msg));
    if (responses.size() != 1) {
        ADD_FAILURE() << "Expected to get 1 response";
        return nullptr;
    }
    auto &responseMsg = responses.at(0);
    if (!responseMsg->isResponse()) {
        ADD_FAILURE() << "Expected response to actually be a response.";
    }
    auto &response = responseMsg->asResponse();
    if (!response.result.has_value()) {
        ADD_FAILURE() << "Expected result to have a value.";
    }

    auto completionList = move(get<unique_ptr<CompletionList>>(*response.result));
    fast_sort(completionList->items, [&](const auto &left, const auto &right) -> bool {
        string leftText = left->sortText.has_value() ? left->sortText.value() : left->label;
        string rightText = right->sortText.has_value() ? right->sortText.value() : right->label;

        return leftText < rightText;
    });

    return completionList;
}

vector<unique_ptr<LSPMessage>> initializeLSP(string_view rootPath, string_view rootUri, LSPWrapper &lspWrapper,
                                             int &nextId, bool supportsMarkdown,
                                             optional<unique_ptr<SorbetInitializationOptions>> initOptions) {
    // Reset next id.
    nextId = 0;

    // Send 'initialize' message.
    {
        auto initializeParams =
            makeInitializeParams(string(rootPath), string(rootUri), supportsMarkdown, move(initOptions));
        auto message = make_unique<LSPMessage>(
            make_unique<RequestMessage>("2.0", nextId++, LSPMethod::Initialize, move(initializeParams)));
        auto responses = getLSPResponsesFor(lspWrapper, move(message));

        // Should just have an 'initialize' response.
        EXPECT_EQ(1, responses.size());
        if (responses.size() != 1) {
            return {};
        }

        assertResponseMessage(0, *responses.at(0));
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

    // Complete initialization handshake with an 'initialized' message.
    {
        auto initialized =
            make_unique<NotificationMessage>("2.0", LSPMethod::Initialized, make_unique<InitializedParams>());
        return getLSPResponsesFor(lspWrapper, make_unique<LSPMessage>(move(initialized)));
    }
}

unique_ptr<LSPMessage> makeOpen(string_view uri, string_view contents, int version) {
    auto params = make_unique<DidOpenTextDocumentParams>(
        make_unique<TextDocumentItem>(string(uri), "ruby", static_cast<double>(version), string(contents)));
    return make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(params)));
}

unique_ptr<LSPMessage> makeChange(string_view uri, string_view contents, int version, bool cancellationExpected,
                                  int preemptionsExpected) {
    auto textDoc = make_unique<VersionedTextDocumentIdentifier>(string(uri), static_cast<double>(version));
    auto textDocChange = make_unique<TextDocumentContentChangeEvent>(string(contents));
    vector<unique_ptr<TextDocumentContentChangeEvent>> textChanges;
    textChanges.push_back(move(textDocChange));

    auto didChangeParams = make_unique<DidChangeTextDocumentParams>(move(textDoc), move(textChanges));
    didChangeParams->sorbetCancellationExpected = cancellationExpected;
    didChangeParams->sorbetPreemptionsExpected = preemptionsExpected;
    auto didChangeNotif =
        make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidChange, move(didChangeParams));
    return make_unique<LSPMessage>(move(didChangeNotif));
}

unique_ptr<LSPMessage> makeClose(string_view uri) {
    auto didCloseParams = make_unique<DidCloseTextDocumentParams>(make_unique<TextDocumentIdentifier>(string(uri)));
    auto didCloseNotif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidClose, move(didCloseParams));
    return make_unique<LSPMessage>(move(didCloseNotif));
}

vector<unique_ptr<LSPMessage>> getLSPResponsesFor(LSPWrapper &wrapper, vector<unique_ptr<LSPMessage>> messages) {
    if (auto stWrapper = dynamic_cast<SingleThreadedLSPWrapper *>(&wrapper)) {
        return stWrapper->getLSPResponsesFor(move(messages));
    } else if (auto mtWrapper = dynamic_cast<MultiThreadedLSPWrapper *>(&wrapper)) {
        // Fences are only used in tests. Use an ID that is likely to be unique to this method.
        const int fenceId = 909090;
        // Chase messages with a fence, and wait for a fence response.
        messages.push_back(
            make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, fenceId)));
        // ASSUMPTION: There are no other messages still being processed. This should be true if the tests are
        // disciplined. Also, even if they aren't, what should we do with stray messages? Passing them on seems most
        // correct.
        mtWrapper->send(messages);

        vector<unique_ptr<LSPMessage>> responses;
        while (true) {
            // In tests, wait a maximum of 20 seconds for a response. It seems like sanitized builds running locally
            // take ~10 seconds.
            auto msg = mtWrapper->read(20000);
            if (!msg) {
                // We should be guaranteed to receive the fence response, so if this happens something is seriously
                // wrong.
                ADD_FAILURE() << "MultithreadedLSPWrapper::read() timed out; the language server might be hung.";
                break;
            }

            if (msg->isNotification() && msg->method() == LSPMethod::SorbetFence &&
                get<int>(msg->asNotification().params) == fenceId) {
                break;
            }
            responses.push_back(move(msg));
        }
        return responses;
    } else {
        ADD_FAILURE() << "LSPWrapper is neither a single nor a multi threaded LSP wrapper; should be impossible!";
        return {};
    }
}

vector<unique_ptr<LSPMessage>> getLSPResponsesFor(LSPWrapper &wrapper, unique_ptr<LSPMessage> message) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(move(message));
    return getLSPResponsesFor(wrapper, move(messages));
}

} // namespace sorbet::test
