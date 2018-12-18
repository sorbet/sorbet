#ifndef TEST_LSP_TEST_HELPERS_H
#define TEST_LSP_TEST_HELPERS_H

#include "common/common.h"
#include "main/lsp/json_types.h"
#include "test/LSPTest.h"

using namespace sorbet::realmain::lsp;

/** Constructs a vector with all enum values from MIN to MAX. Assumes a contiguous enum and properly chosen min/max
 * values. Our serialization/deserialization code will throw if we pick an improper value. */
template <typename T, T MAX, T MIN> std::vector<T> getAllEnumKinds() {
    std::vector<T> symbols;
    for (int i = (int)MIN; i <= (int)MAX; i++) {
        symbols.push_back((T)i);
    }
    return symbols;
};

template <typename T = DynamicRegistrationOption>
std::unique_ptr<T> makeDynamicRegistrationOption(bool dynamicRegistration) {
    auto option = std::make_unique<T>();
    option->dynamicRegistration = dynamicRegistration;
    return option;
};

/** Creates the parameters to the `initialize` message, which advertises the client's capabilities. */
std::unique_ptr<JSONBaseType> makeInitializeParams(std::string rootPath, std::string rootUri);

/** Creates a RequestMessage object from the given items. */
std::unique_ptr<RequestMessage> makeRequestMessage(std::unique_ptr<JSONDocument<int>> &doc, std::string method, int id,
                                                   std::unique_ptr<JSONBaseType> &params);

/** Checks that we are properly advertising Sorbet LSP's capabilities to clients. */
void checkServerCapabilities(const std::unique_ptr<ServerCapabilities> &capabilities);

/** Asserts that the JSONBaseType is a ResponseMessage with the given id. Returns the response message on success, fails
 * the test otherwise. */
optional<std::unique_ptr<JSONDocument<ResponseMessage>>>
assertResponseMessage(int expectedId, unique_ptr<JSONDocument<JSONBaseType>> &response);

/** Asserts that the JSONBaseType is a NotificationMessage with the given method. Returns the notification message on
 * success, fails the test otherwise. */
optional<std::unique_ptr<JSONDocument<NotificationMessage>>>
assertNotificationMessage(std::string expectedMethod, unique_ptr<JSONDocument<JSONBaseType>> &response);

/** Retrieves the PublishDiagnosticsParam from a publishDiagnostics message, if applicable. Non-fatal fails and returns
 * an empty optional if it cannot be found. */
optional<std::unique_ptr<PublishDiagnosticsParams>>
getPublishDiagnosticParams(const std::unique_ptr<JSONDocument<NotificationMessage>> &doc);

/** Adds a failure that reports that an error indicated in a test file is missing from Sorbet's output. */
void reportMissingError(const std::string &filename, const std::shared_ptr<ErrorAssertion> &assertion,
                        const std::string &sourceLine);

/** Adds a failure that Sorbet LSP reported an error that was not covered by an ErrorAssertion. */
void reportUnexpectedLSPError(const std::string &filename, const std::unique_ptr<Diagnostic> &diagnostic,
                              const std::string &sourceLine);

/** Adds a failure that Sorbet reported an error that was not covered by an ErrorAssertion. */
void reportUnexpectedError(const std::string &filename, int lineNum, const std::string &errorMsg,
                           const std::string &sourceLine);

#endif // TEST_LSP_TEST_HELPERS_H