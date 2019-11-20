#ifndef TEST_LSP_PROTOCOLTEST_H
#define TEST_LSP_PROTOCOLTEST_H

#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "main/lsp/wrapper.h"
#include "test/helpers/MockFileSystem.h"

namespace sorbet::test::lsp {
using namespace sorbet::realmain::lsp;

struct ExpectedDiagnostic {
    std::string path;
    int line;
    std::string message;
};

class ProtocolTest : public testing::Test {
protected:
    std::unique_ptr<LSPWrapper> lspWrapper;
    std::string rootPath;
    std::string rootUri;
    // Contains the current source file contents. Used to print pretty error messages.
    // TODO(jvilk): Remove and instead get state from Sorbet directly. Will be hard to maintain
    // once we test incremental diffs.
    UnorderedMap<std::string, std::shared_ptr<core::File>> sourceFileContents;
    std::vector<std::unique_ptr<LSPMessage>> pendingRequests;
    // Currently active diagnostics, specifically using map to enforce sort order on filename.
    std::map<std::string, std::vector<std::unique_ptr<Diagnostic>>> diagnostics;
    // Emulated file system.
    std::shared_ptr<sorbet::test::MockFileSystem> fs;

    /** The next ID to use when sending an LSP message. */
    int nextId = 0;

    virtual ~ProtocolTest() = default;

    void SetUp() override;

    /** Get an absolute file URI for the given relative file path. */
    std::string getUri(std::string_view filePath);

    std::vector<std::unique_ptr<LSPMessage>> initializeLSP();

    std::unique_ptr<LSPMessage> openFile(std::string_view path, std::string_view contents);

    std::unique_ptr<LSPMessage> closeFile(std::string_view path);

    std::unique_ptr<LSPMessage> changeFile(std::string_view path, std::string_view newContents, int version);

    std::unique_ptr<LSPMessage> documentSymbol(std::string_view path);

    std::unique_ptr<LSPMessage> workspaceSymbol(std::string_view query);

    std::unique_ptr<LSPMessage> getDefinition(std::string_view path, int line, int character);

    std::unique_ptr<LSPMessage> cancelRequest(int id);

    void writeFilesToFS(std::vector<std::pair<std::string, std::string>> files);

    void deleteFileFromFS(std::string_view filename);

    std::unique_ptr<LSPMessage> watchmanFileUpdate(std::vector<std::string> updatedFilePaths);

    std::vector<std::unique_ptr<LSPMessage>> sendRaw(const std::string &json);

    std::vector<std::unique_ptr<LSPMessage>> send(const LSPMessage &message);

    std::vector<std::unique_ptr<LSPMessage>> send(std::vector<std::unique_ptr<LSPMessage>> messages);

    void assertDiagnostics(std::vector<std::unique_ptr<LSPMessage>> messages, std::vector<ExpectedDiagnostic> expected);

    std::string readFile(std::string_view uri);

    std::vector<std::unique_ptr<Location>> getDefinitions(std::string_view uri, int line, int character);

    /**
     * ProtocolTest maintains the latest diagnostics for files received over a session, as LSP is not required to
     * re-send diagnostics that have not changed. send() automatically updates diagnostics, but if a test manually
     * invokes getLSPResponsesFor on lspWrapper, it should update diagnostics with this function.
     */
    void updateDiagnostics(const std::vector<std::unique_ptr<LSPMessage>> &messages);
};

} // namespace sorbet::test::lsp

#endif // TEST_LSP_PROTOCOLTEST_H
