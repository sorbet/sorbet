#ifndef TEST_HELPERS_LSP_H
#define TEST_HELPERS_LSP_H

#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"
#include "main/lsp/wrapper.h"

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

/** Creates the parameters to the `initialize` message, which advertises the client's capabilities. */
std::unique_ptr<InitializeParams> makeInitializeParams(std::string rootPath, std::string rootUri);

/** Create an LSPMessage containing a textDocument/definition request. */
std::unique_ptr<LSPMessage> makeDefinitionRequest(rapidjson::MemoryPoolAllocator<> &alloc, int id, std::string_view uri,
                                                  int line, int character);

/** Checks that we are properly advertising Sorbet LSP's capabilities to clients. */
void checkServerCapabilities(const ServerCapabilities &capabilities);

/** Asserts that the LSPMessage is a ResponseMessage with the given id. Returns true on success, fails
 * the test otherwise. */
bool assertResponseMessage(int expectedId, const LSPMessage &response);

/** Asserts that the LSPMessage is a ResponseMessage with an error of the given code and that
 * contains the given message. Returns true on success, fails otherwise. */
bool assertResponseError(int code, std::string_view msg, const LSPMessage &response);

/** Asserts that the LSPMessage is a NotificationMessage with the given method. Returns true on
 * success, fails the test otherwise. */
bool assertNotificationMessage(const LSPMethod expectedMethod, const LSPMessage &response);

/** Retrieves the PublishDiagnosticsParam from a publishDiagnostics message, if applicable. Non-fatal fails and returns
 * an empty optional if it cannot be found. */
std::optional<PublishDiagnosticsParams *> getPublishDiagnosticParams(rapidjson::MemoryPoolAllocator<> &alloc,
                                                                     NotificationMessage &notifMsg);

/** Sends boilerplate initialization / initialized messages to start a new LSP session. */
std::vector<std::unique_ptr<LSPMessage>> initializeLSP(std::string_view rootPath, std::string_view rootUri,
                                                       LSPWrapper &lspWrapper, int &nextId);

} // namespace sorbet::test
#endif // TEST_HELPERS_LSP_H
