#include "doctest/doctest.h"
// has to go first as it violates requirements

#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/common.h"
#include "common/sort/sort.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/requests/initialize.h"
#include "test/helpers/lsp.h"

namespace sorbet::test {
using namespace std;

string filePathToUri(const LSPConfiguration &config, string_view filePath) {
    return fmt::format("{}/{}", config.getClientConfig().rootUri, filePath);
}

string uriToFilePath(const LSPConfiguration &config, string_view uri) {
    string_view prefixUrl = config.getClientConfig().rootUri;
    if (!config.isUriInWorkspace(uri)) {
        FAIL_CHECK(fmt::format(
            "Unrecognized URI: `{}` is not contained in root URI `{}`, and thus does not correspond to a test file.",
            uri, prefixUrl));
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

unique_ptr<TextDocumentClientCapabilities> makeTextDocumentClientCapabilities(bool supportsMarkdown,
                                                                              bool supportsCodeActionResolve) {
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

    if (supportsCodeActionResolve) {
        auto codeActionCaps = make_unique<CodeActionCapabilities>();
        codeActionCaps->dataSupport = true;
        codeActionCaps->resolveSupport = make_unique<CodeActionResolveSupport>(vector<string>{"edit"});
        capabilities->codeAction = move(codeActionCaps);
    }

    return capabilities;
}

unique_ptr<InitializeParams>
makeInitializeParams(std::optional<variant<string, JSONNullObject>> rootPath, variant<string, JSONNullObject> rootUri,
                     bool supportsMarkdown, bool supportsCodeActionResolve,
                     std::optional<std::unique_ptr<SorbetInitializationOptions>> initOptions) {
    auto initializeParams = make_unique<InitializeParams>(rootUri, make_unique<ClientCapabilities>());
    if (rootPath) {
        initializeParams->rootPath = rootPath;
    }
    initializeParams->capabilities->workspace = makeWorkspaceClientCapabilities();
    initializeParams->capabilities->textDocument =
        makeTextDocumentClientCapabilities(supportsMarkdown, supportsCodeActionResolve);
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

unique_ptr<LSPMessage> makeCodeAction(int id, std::string_view uri, int line, int character) {
    auto textDocument = make_unique<TextDocumentIdentifier>(string(uri));
    auto range = make_unique<Range>(make_unique<Position>(line, character), make_unique<Position>(line, character));
    auto context = make_unique<CodeActionContext>(vector<unique_ptr<Diagnostic>>{});
    return make_unique<LSPMessage>(
        make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentCodeAction,
                                    make_unique<CodeActionParams>(move(textDocument), move(range), move(context))));
}

unique_ptr<LSPMessage> makeCompletion(int id, std::string_view uri, int line, int character) {
    return make_unique<LSPMessage>(
        make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentCompletion,
                                    make_unique<CompletionParams>(make_unique<TextDocumentIdentifier>(string(uri)),
                                                                  make_unique<Position>(line, character))));
}

unique_ptr<LSPMessage> makeWorkspaceSymbolRequest(int id, std::string_view query) {
    return make_unique<LSPMessage>(make_unique<RequestMessage>("2.0", id, LSPMethod::WorkspaceSymbol,
                                                               make_unique<WorkspaceSymbolParams>(string(query))));
}

/** Checks that we are properly advertising Sorbet LSP's capabilities to clients. */
void checkServerCapabilities(const ServerCapabilities &capabilities) {
    // Properties checked in the same order they are described in the LSP spec.
    CHECK(capabilities.textDocumentSync.has_value());
    auto &textDocumentSync = *(capabilities.textDocumentSync);
    auto textDocumentSyncValue = get<TextDocumentSyncKind>(textDocumentSync);
    CHECK_EQ(TextDocumentSyncKind::Full, textDocumentSyncValue);

    CHECK(capabilities.hoverProvider.value_or(false));

    CHECK(capabilities.completionProvider.has_value());
    if (capabilities.completionProvider.has_value()) {
        auto &completionProvider = *(capabilities.completionProvider);
        CHECK_EQ(realmain::lsp::InitializeTask::TRIGGER_CHARACTERS, completionProvider->triggerCharacters);
    }

    CHECK(capabilities.signatureHelpProvider.has_value());
    if (capabilities.signatureHelpProvider.has_value()) {
        auto &sigHelpProvider = *(capabilities.signatureHelpProvider);
        auto sigHelpTriggerChars = sigHelpProvider->triggerCharacters.value_or(vector<string>({}));
        CHECK_EQ(2, sigHelpTriggerChars.size());
        UnorderedSet<string> sigHelpTriggerSet(sigHelpTriggerChars.begin(), sigHelpTriggerChars.end());
        CHECK_NE(sigHelpTriggerSet.end(), sigHelpTriggerSet.find("("));
        CHECK_NE(sigHelpTriggerSet.end(), sigHelpTriggerSet.find(","));
    }

    // We don't support all possible features. Make sure we don't make any false claims.
    CHECK(capabilities.definitionProvider.value_or(false));
    CHECK(capabilities.typeDefinitionProvider.value_or(false));
    CHECK(capabilities.implementationProvider.value_or(false));
    CHECK(capabilities.referencesProvider.value_or(false));
    CHECK(capabilities.documentHighlightProvider.has_value());
    CHECK(capabilities.documentSymbolProvider.value_or(false));
    CHECK(capabilities.workspaceSymbolProvider.value_or(false));
    CHECK(capabilities.codeActionProvider.has_value());
    CHECK(capabilities.renameProvider.has_value());
    CHECK_FALSE(capabilities.codeLensProvider.has_value());
    CHECK(capabilities.documentFormattingProvider.value_or(false));
    CHECK_FALSE(capabilities.documentRangeFormattingProvider.has_value());
    CHECK_FALSE(capabilities.documentRangeFormattingProvider.has_value());
    CHECK_FALSE(capabilities.documentOnTypeFormattingProvider.has_value());
    CHECK_FALSE(capabilities.documentLinkProvider.has_value());
    CHECK_FALSE(capabilities.executeCommandProvider.has_value());
    CHECK_FALSE(capabilities.workspace.has_value());
}

void assertResponseMessage(int expectedId, const LSPMessage &response) {
    REQUIRE_MESSAGE(response.isResponse(),
                    fmt::format("Expected a response message, but received the following notification instead: {}",
                                response.toJSON()));

    auto &respMsg = response.asResponse();
    auto idIntPtr = get_if<int>(&respMsg.id);
    REQUIRE_MESSAGE(idIntPtr != nullptr, "Response message lacks an integer ID field.");
    INFO("Response message's ID does not match expected value.");
    REQUIRE_EQ(expectedId, *idIntPtr);
}

void assertResponseError(int code, string_view msg, const LSPMessage &response) {
    REQUIRE_MESSAGE(response.isResponse(),
                    fmt::format("Expected a response message with error `{}: {}`, but received:\n{}", code, msg,
                                response.toJSON()));
    auto &r = response.asResponse();
    auto &maybeError = r.error;
    REQUIRE_MESSAGE(maybeError.has_value(),
                    fmt::format("Expected a response message with an error, but received:\n{}", response.toJSON()));
    auto &error = *maybeError;
    {
        INFO(fmt::format("Response message contains error with unexpected code:\n{}", error->toJSON()));
        REQUIRE_EQ(error->code, code);
    }

    {
        INFO(fmt::format("Expected a response message with error `{}: {}`, but received:\n{}", code, msg,
                         response.toJSON()));
        REQUIRE_NE(error->message.find(msg), string::npos);
    }
}

void assertNotificationMessage(LSPMethod expectedMethod, const LSPMessage &response) {
    REQUIRE_MESSAGE(response.isNotification(),
                    fmt::format("Expected a notification, but received the following response message instead: {}",
                                response.toJSON()));
    {
        INFO(fmt::format("Unexpected method on notification message: expected {} but received {}.",
                         convertLSPMethodToString(expectedMethod), convertLSPMethodToString(response.method())));
        REQUIRE_EQ(expectedMethod, response.method());
    }
}

optional<const PublishDiagnosticsParams *> getPublishDiagnosticParams(const NotificationMessage &notifMsg) {
    auto publishDiagnosticParams = get_if<unique_ptr<PublishDiagnosticsParams>>(&notifMsg.params);
    if (!publishDiagnosticParams || !*publishDiagnosticParams) {
        FAIL_CHECK("textDocument/publishDiagnostics message is missing parameters.");
        return nullopt;
    }
    return (*publishDiagnosticParams).get();
}

unique_ptr<PrepareRenameResult> doTextDocumentPrepareRename(LSPWrapper &lspWrapper, const Range &range, int &nextId,
                                                            string_view filename) {
    auto uri = filePathToUri(lspWrapper.config(), filename);
    auto id = nextId++;
    auto params =
        make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(uri), range.start->copy());
    auto msg = make_unique<LSPMessage>(
        make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentPrepareRename, move(params)));
    auto responses = getLSPResponsesFor(lspWrapper, move(msg));
    if (responses.size() != 1) {
        FAIL_CHECK("Expected to get 1 response");
        return nullptr;
    }

    auto &responseMsg = responses.at(0);
    if (!responseMsg->isResponse()) {
        FAIL_CHECK("Expected response to actually be a response.");
    }

    auto &response = responseMsg->asResponse();
    if (!response.result.has_value()) {
        FAIL_CHECK("Expected result to have a value.");
    }

    auto &result = get<variant<JSONNullObject, unique_ptr<PrepareRenameResult>>>(*response.result);
    if (get_if<JSONNullObject>(&result) != nullptr) {
        return nullptr;
    }

    return move(get<unique_ptr<PrepareRenameResult>>(result));
}

unique_ptr<WorkspaceEdit> doTextDocumentRename(LSPWrapper &lspWrapper, const Range &range, int &nextId,
                                               string_view filename, string newName, string expectedErrorMessage) {
    auto uri = filePathToUri(lspWrapper.config(), filename);
    auto id = nextId++;

    auto params = make_unique<RenameParams>(make_unique<TextDocumentIdentifier>(uri), range.start->copy(), newName);
    auto msg =
        make_unique<LSPMessage>(make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentRename, move(params)));
    auto responses = getLSPResponsesFor(lspWrapper, move(msg));
    if (responses.size() != 1) {
        FAIL_CHECK("Expected to get 1 response");
        return nullptr;
    }
    auto &responseMsg = responses.at(0);
    if (!responseMsg->isResponse()) {
        FAIL_CHECK("Expected response to actually be a response.");
    }
    auto &response = responseMsg->asResponse();
    if (response.error.has_value()) {
        if (expectedErrorMessage.empty()) {
            return nullptr;
        }

        {
            auto &error = *response.error;
            INFO(fmt::format("Expected an error message containing `{}`, but received:\n{}", expectedErrorMessage,
                             error->message));
            REQUIRE_NE(error->message.find(expectedErrorMessage), string::npos);
        }
    }

    if (!response.result.has_value()) {
        FAIL_CHECK("Expected result to have a value.");
    }

    auto &result = get<variant<JSONNullObject, unique_ptr<WorkspaceEdit>>>(*response.result);
    if (get_if<JSONNullObject>(&result) != nullptr) {
        return nullptr;
    }

    return move(get<unique_ptr<WorkspaceEdit>>(result));
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
        FAIL_CHECK("Expected to get 1 response");
        return nullptr;
    }
    auto &responseMsg = responses.at(0);
    if (!responseMsg->isResponse()) {
        FAIL_CHECK("Expected response to actually be a response.");
    }
    auto &response = responseMsg->asResponse();
    if (!response.result.has_value()) {
        FAIL_CHECK("Expected result to have a value.");
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
                                             int &nextId, bool supportsMarkdown, bool supportsCodeActionResolve,
                                             optional<unique_ptr<SorbetInitializationOptions>> initOptions) {
    // Reset next id.
    nextId = 0;

    // Send 'initialize' message.
    {
        auto initializeParams = makeInitializeParams(string(rootPath), string(rootUri), supportsMarkdown,
                                                     supportsCodeActionResolve, move(initOptions));
        auto message = make_unique<LSPMessage>(
            make_unique<RequestMessage>("2.0", nextId++, LSPMethod::Initialize, move(initializeParams)));
        auto responses = getLSPResponsesFor(lspWrapper, move(message));

        // Should just have an 'initialize' response.
        CHECK_EQ(1, responses.size());
        if (responses.size() != 1) {
            return {};
        }

        assertResponseMessage(0, *responses.at(0));
        auto &respMsg = responses.at(0)->asResponse();
        CHECK_FALSE(respMsg.error.has_value());
        CHECK(respMsg.result.has_value());

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

std::unique_ptr<LSPMessage> makeConfigurationChange(std::unique_ptr<DidChangeConfigurationParams> params) {
    auto changeConfigNotification =
        make_unique<NotificationMessage>("2.0", LSPMethod::WorkspaceDidChangeConfiguration, move(params));
    return make_unique<LSPMessage>(move(changeConfigNotification));
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
            // take ~10 seconds. Wait 5 minutes if running in the debugger
            auto msg = mtWrapper->read(amIBeingDebugged() ? 300'000 : 20'000);
            if (!msg) {
                // We should be guaranteed to receive the fence response, so if this happens something is seriously
                // wrong.
                FAIL_CHECK("MultithreadedLSPWrapper::read() timed out; the language server might be hung.");
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
        FAIL_CHECK("LSPWrapper is neither a single nor a multi threaded LSP wrapper; should be impossible!");
        return {};
    }
}

vector<unique_ptr<LSPMessage>> getLSPResponsesFor(LSPWrapper &wrapper, unique_ptr<LSPMessage> message) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(move(message));
    return getLSPResponsesFor(wrapper, move(messages));
}

namespace {

// Like absl::StripTrailingAsciiWhitespace, but only blank characters (tabs and spaces)
[[nodiscard]] inline absl::string_view stripTrailingAsciiBlank(absl::string_view str) {
    // You can use this to jump to def.
    ENFORCE(true, "{}", absl::StripTrailingAsciiWhitespace(""));

    auto it = std::find_if_not(str.rbegin(), str.rend(), absl::ascii_isblank);
    return str.substr(0, str.rend() - it);
}

} // namespace

// reindent should probably be `true` for snippets (e.g., completion items) and false for things that are not snippets.
// At some point we might want to try to approximate what VS Code does more closely, but I've never
// been able to find a concise description of how they decide to reindent a snippet and when.
string applyEdit(string_view source, const core::File &file, const Range &range, string_view newText, bool reindent) {
    auto beginLine = static_cast<uint32_t>(range.start->line + 1);
    auto beginCol = static_cast<uint32_t>(range.start->character + 1);
    auto beginOffset = core::Loc::pos2Offset(file, {beginLine, beginCol}).value();

    auto endLine = static_cast<uint32_t>(range.end->line + 1);
    auto endCol = static_cast<uint32_t>(range.end->character + 1);
    auto endOffset = core::Loc::pos2Offset(file, {endLine, endCol}).value();

    auto lineStartOffset = core::Loc::pos2Offset(file, {beginLine, 1}).value();
    auto lineStartView = source.substr(lineStartOffset);
    auto firstNonWhitespace = lineStartView.find_first_not_of(" \t\n");
    auto indentAfterNewline = absl::StrCat("\n", source.substr(lineStartOffset, firstNonWhitespace));

    string actualEditedFileContents = string(source);
    // Only strip trailing whitespace if it will end up at the end of a line
    auto maybeStripped = (actualEditedFileContents.size() > endOffset && actualEditedFileContents[endOffset] == '\n')
                             ? stripTrailingAsciiBlank(newText)
                             : newText;
    auto indented = reindent ? absl::StrReplaceAll(maybeStripped, {{"\n", indentAfterNewline}}) : string(maybeStripped);
    actualEditedFileContents.replace(beginOffset, endOffset - beginOffset, indented);

    return actualEditedFileContents;
}

} // namespace sorbet::test
