#ifndef TEST_LSP_PROTOCOLTEST_H
#define TEST_LSP_PROTOCOLTEST_H

#include "doctest/doctest.h"
// ^ Violates linting rules, so include first.
#include "common/counters/Counters.h"
#include "common/counters/Counters_impl.h"
#include "main/lsp/json_types.h"
#include "main/lsp/wrapper.h"
#include "test/helpers/CounterStateDatabase.h"
#include "test/helpers/MockFileSystem.h"

namespace sorbet::test::lsp {
using namespace sorbet::realmain::lsp;

struct ExpectedDiagnostic {
    std::string path;
    int line;
    std::string message;
};

/**
 * If parameter is 'true', LSP is configured in multithreaded mode.
 */
class ProtocolTest {
protected:
    const bool useMultithreading;
    const bool useCache;
    std::unique_ptr<LSPWrapper> lspWrapper;
    std::string rootPath;
    std::string rootUri;
    std::string cacheDir;
    // Contains the current source file contents. Used to print pretty error messages.
    // TODO(jvilk): Remove and instead get state from Sorbet directly. Will be hard to maintain
    // once we test incremental diffs.
    UnorderedMap<std::string, std::shared_ptr<core::File>> sourceFileContents;
    // Currently active diagnostics, specifically using map to enforce sort order on filename.
    std::map<std::string, std::vector<std::unique_ptr<Diagnostic>>> diagnostics;
    // Emulated file system.
    std::shared_ptr<sorbet::test::MockFileSystem> fs;

    /** The next ID to use when sending an LSP message. */
    int nextId = 0;

    ProtocolTest(bool useMultithreading = false, bool useCache = false);

    ~ProtocolTest();

    /** Reset lspWrapper and other internal state. */
    void resetState(std::shared_ptr<sorbet::realmain::options::Options> opts = nullptr);

    /** Get an absolute file URI for the given relative file path. */
    std::string getUri(std::string_view filePath);

    std::vector<std::unique_ptr<LSPMessage>>
    initializeLSP(bool supportsMarkdown = true, bool supportsCodeActionResolve = true,
                  std::optional<std::unique_ptr<SorbetInitializationOptions>> opts = std::nullopt);

    std::unique_ptr<LSPMessage> openFile(std::string_view path, std::string_view contents);

    std::unique_ptr<LSPMessage> closeFile(std::string_view path);

    std::unique_ptr<LSPMessage> changeFile(std::string_view path, std::string_view newContents, int version,
                                           bool cancellationExpected = false, int preemptionsExpected = 0);

    std::unique_ptr<LSPMessage> documentSymbol(std::string_view path);

    std::unique_ptr<LSPMessage> workspaceSymbol(std::string_view query);

    std::unique_ptr<LSPMessage> getDefinition(std::string_view path, int line, int character);

    std::unique_ptr<LSPMessage> hover(std::string_view path, int line, int character);

    std::unique_ptr<LSPMessage> codeAction(std::string_view path, int line, int character);

    std::unique_ptr<LSPMessage> completion(std::string_view path, int line, int character);

    std::unique_ptr<LSPMessage> cancelRequest(int id);

    void writeFilesToFS(std::vector<std::pair<std::string, std::string>> files);

    void deleteFileFromFS(std::string_view filename);

    std::unique_ptr<LSPMessage> watchmanFileUpdate(std::vector<std::string> updatedFilePaths);

    std::vector<std::unique_ptr<LSPMessage>> sendRaw(const std::string &json);

    std::vector<std::unique_ptr<LSPMessage>> send(const LSPMessage &message);

    std::vector<std::unique_ptr<LSPMessage>> send(std::vector<std::unique_ptr<LSPMessage>> messages);

    void sendAsyncRaw(const std::string &json);

    void sendAsync(const LSPMessage &message);

    std::unique_ptr<LSPMessage> readAsync();

    void assertErrorDiagnostics(std::vector<std::unique_ptr<LSPMessage>> messages,
                                std::vector<ExpectedDiagnostic> expected);

    void assertUntypedDiagnostics(std::vector<std::unique_ptr<LSPMessage>> messages,
                                  std::vector<ExpectedDiagnostic> expected);

    std::string readFile(std::string_view uri);

    std::vector<std::unique_ptr<Location>> getDefinitions(std::string_view uri, int line, int character);

    /**
     * ProtocolTest maintains the latest diagnostics for files received over a session, as LSP is not required to
     * re-send diagnostics that have not changed. send() automatically updates diagnostics, but if a test manually
     * invokes getLSPResponsesFor on lspWrapper, it should update diagnostics with this function.
     */
    void updateDiagnostics(const std::vector<std::unique_ptr<LSPMessage>> &messages);
    void updateDiagnostics(const LSPMessage &message);

    /**
     * Request all counter metrics from the server. Used to assert that metrics are reporting correctly.
     */
    const CounterStateDatabase getCounters();

    /**
     * Set a flag that forces the slow path to block indefinitely after saving undo state. Setting this flag to `false`
     * will immediately unblock any currently blocked slow paths.
     */
    void setSlowPathBlocked(bool blocked) {
        lspWrapper->setSlowPathBlocked(blocked);
    }
};

} // namespace sorbet::test::lsp

#endif // TEST_LSP_PROTOCOLTEST_H
