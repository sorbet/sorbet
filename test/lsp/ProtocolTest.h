#ifndef TEST_LSP_PROTOCOLTEST_H
#define TEST_LSP_PROTOCOLTEST_H

#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "main/lsp/wrapper.h"

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
    UnorderedMap<std::string, std::shared_ptr<core::File>> sourceFileContents;
    std::vector<std::unique_ptr<LSPMessage>> pendingRequests;

    /** The next ID to use when sending an LSP message. */
    int nextId = 0;

    bool paused = false;

    virtual ~ProtocolTest() = default;

    void SetUp() override;

    /** Get an absolute file URI for the given relative file path. */
    std::string getUri(std::string_view filePath);

    void initializeLSP();

    std::unique_ptr<LSPMessage> openFile(std::string_view path, std::string_view contents);

    std::unique_ptr<LSPMessage> changeFile(std::string_view path, std::string_view newContents, int version);

    std::unique_ptr<LSPMessage> documentSymbol(std::string_view path);

    std::unique_ptr<LSPMessage> getDefinition(std::string_view path, int line, int character);

    std::unique_ptr<LSPMessage> cancelRequest(int id);

    std::vector<std::unique_ptr<LSPMessage>> send(const LSPMessage &message);

    std::vector<std::unique_ptr<LSPMessage>> send(std::vector<std::unique_ptr<LSPMessage>> messages);

    void assertDiagnostics(std::vector<std::unique_ptr<LSPMessage>> messages,
                           std::vector<ExpectedDiagnostic> diagnostics);
};

} // namespace sorbet::test::lsp

#endif // TEST_LSP_PROTOCOLTEST_H
