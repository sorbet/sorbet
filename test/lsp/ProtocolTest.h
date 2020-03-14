#ifndef TEST_LSP_PROTOCOLTEST_H
#define TEST_LSP_PROTOCOLTEST_H

#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "common/Counters.h"
#include "common/Counters_impl.h"
#include "main/lsp/json_types.h"
#include "main/lsp/wrapper.h"
#include "test/helpers/MockFileSystem.h"

namespace sorbet::test::lsp {
using namespace sorbet::realmain::lsp;

struct ExpectedDiagnostic {
    std::string path;
    int line;
    std::string message;
};

class CounterStateDatabase final {
    const CounterState counters;

public:
    CounterStateDatabase(CounterState counters);

    // Get counter value or 0.
    CounterImpl::CounterType getCounter(ConstExprStr counter) const;

    CounterImpl::CounterType getCategoryCounter(ConstExprStr counter, ConstExprStr category) const;

    CounterImpl::CounterType getCategoryCounterSum(ConstExprStr counter) const;

    CounterImpl::CounterType getHistogramCount(ConstExprStr histogram) const;

    std::vector<CounterImpl::Timing> getTimings(ConstExprStr counter,
                                                std::vector<std::pair<ConstExprStr, ConstExprStr>> tags = {}) const;
};

struct ProtocolTestConfig {
    bool useMultithreading = false;
    // Should the test use kvstore?
    bool useCache = false;
};

/**
 * If parameter is 'true', LSP is configured in multithreaded mode.
 */
class ProtocolTest : public testing::TestWithParam<ProtocolTestConfig> {
protected:
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

    ~ProtocolTest() override = default;

    void SetUp() override;

    void TearDown() override;

    /** Reset lspWrapper and other internal state. */
    void resetState();

    /** Get an absolute file URI for the given relative file path. */
    std::string getUri(std::string_view filePath);

    std::vector<std::unique_ptr<LSPMessage>>
    initializeLSP(bool supportsMarkdown = true,
                  std::optional<std::unique_ptr<SorbetInitializationOptions>> opts = std::nullopt);

    std::unique_ptr<LSPMessage> openFile(std::string_view path, std::string_view contents);

    std::unique_ptr<LSPMessage> closeFile(std::string_view path);

    std::unique_ptr<LSPMessage> changeFile(std::string_view path, std::string_view newContents, int version,
                                           bool cancellationExpected = false, int preemptionsExpected = 0);

    std::unique_ptr<LSPMessage> documentSymbol(std::string_view path);

    std::unique_ptr<LSPMessage> workspaceSymbol(std::string_view query);

    std::unique_ptr<LSPMessage> getDefinition(std::string_view path, int line, int character);

    std::unique_ptr<LSPMessage> hover(std::string_view path, int line, int character);

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

    void assertDiagnostics(std::vector<std::unique_ptr<LSPMessage>> messages, std::vector<ExpectedDiagnostic> expected);

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
};

} // namespace sorbet::test::lsp

#endif // TEST_LSP_PROTOCOLTEST_H
