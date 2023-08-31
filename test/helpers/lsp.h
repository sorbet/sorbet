#ifndef TEST_HELPERS_LSP_H
#define TEST_HELPERS_LSP_H

#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"
#include "main/lsp/wrapper.h"

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

std::string filePathToUri(const LSPConfiguration &config, std::string_view filePath);

std::string uriToFilePath(const LSPConfiguration &config, std::string_view uri);

/** Creates the parameters to the `initialize` message, which advertises the client's capabilities. */
std::unique_ptr<InitializeParams>
makeInitializeParams(std::optional<std::variant<std::string, JSONNullObject>> rootPath,
                     std::variant<std::string, JSONNullObject> rootUri, bool supportsMarkdown,
                     bool supportsCodeActionResolve,
                     std::optional<std::unique_ptr<SorbetInitializationOptions>> initOptions);

/** Create an LSPMessage containing a textDocument/definition request. */
std::unique_ptr<LSPMessage> makeDefinitionRequest(int id, std::string_view uri, int line, int character);

/** Create an LSPMessage containing a textDocument/hover request. */
std::unique_ptr<LSPMessage> makeHover(int id, std::string_view uri, int line, int character);

/** Create an LSPMessage containing a textDocument/codeAction request. */
std::unique_ptr<LSPMessage> makeCodeAction(int id, std::string_view uri, int line, int character);

/** Create an LSPMessage containing a textDocument/completion request. */
std::unique_ptr<LSPMessage> makeCompletion(int id, std::string_view uri, int line, int character);

/** Create an LSPMessage containing a WorkspaceSymbol request. */
std::unique_ptr<LSPMessage> makeWorkspaceSymbolRequest(int id, std::string_view query);

/** Create an LSPMessage containing a textDocument/didOpen notification. */
std::unique_ptr<LSPMessage> makeOpen(std::string_view uri, std::string_view contents, int version);

/** Create an LSPMessage containing a textDocument/didChange notification. */
std::unique_ptr<LSPMessage> makeChange(std::string_view uri, std::string_view contents, int version,
                                       bool cancellationExpected = false, int preemptionsExpected = 0);

/** Create an LSPMessage containing a textDocument/didClose notification. */
std::unique_ptr<LSPMessage> makeClose(std::string_view uri);

/** Create an LSPMessage containing a workspace/didChangeConfiguration notification */
std::unique_ptr<LSPMessage> makeConfigurationChange(std::unique_ptr<DidChangeConfigurationParams> params);

/** Checks that we are properly advertising Sorbet LSP's capabilities to clients. */
void checkServerCapabilities(const ServerCapabilities &capabilities);

/** Asserts that the LSPMessage is a ResponseMessage with the given id. */
void assertResponseMessage(int expectedId, const LSPMessage &response);

/** Asserts that the LSPMessage is a ResponseMessage with an error of the given code and that
 * contains the given message. */
void assertResponseError(int code, std::string_view msg, const LSPMessage &response);

/** Asserts that the LSPMessage is a NotificationMessage with the given method. */
void assertNotificationMessage(LSPMethod expectedMethod, const LSPMessage &response);

/** Retrieves the PublishDiagnosticsParam from a publishDiagnostics message, if applicable. Non-fatal fails and returns
 * an empty optional if it cannot be found. */
std::optional<const PublishDiagnosticsParams *> getPublishDiagnosticParams(const NotificationMessage &notifMsg);

/** Use the LSPWrapper to make a textDocument/completion request.
 */
std::unique_ptr<CompletionList> doTextDocumentCompletion(LSPWrapper &lspWrapper, const Range &range, int &nextId,
                                                         std::string_view filename);

/** Use the LSPWrapper to make a textDocument/prepareRename request.
 */
std::unique_ptr<PrepareRenameResult> doTextDocumentPrepareRename(LSPWrapper &lspWrapper, const Range &range,
                                                                 int &nextId, std::string_view filename);

/** Use the LSPWrapper to make a textDocument/rename request.
 */
std::unique_ptr<WorkspaceEdit> doTextDocumentRename(LSPWrapper &lspWrapper, const Range &range, int &nextId,
                                                    std::string_view filename, std::string newName,
                                                    std::string expectedErrorMessage);

/** Sends boilerplate initialization / initialized messages to start a new LSP session. */
std::vector<std::unique_ptr<LSPMessage>>
initializeLSP(std::string_view rootPath, std::string_view rootUri, LSPWrapper &lspWrapper, int &nextId,
              bool supportsMarkdown = true, bool supportsCodeActionResolve = true,
              std::optional<std::unique_ptr<SorbetInitializationOptions>> initOptions = std::nullopt);

/** Sends the given messages to LSPWrapper, and returns all responses to those messages. Works with single and
 * multithreaded LSP wrappers. */
std::vector<std::unique_ptr<LSPMessage>> getLSPResponsesFor(LSPWrapper &wrapper, std::unique_ptr<LSPMessage> message);
std::vector<std::unique_ptr<LSPMessage>> getLSPResponsesFor(LSPWrapper &wrapper,
                                                            std::vector<std::unique_ptr<LSPMessage>> messages);

std::string applyEdit(std::string_view source, const core::File &file, const Range &range, std::string_view newText,
                      bool reindent);

} // namespace sorbet::test
#endif // TEST_HELPERS_LSP_H
