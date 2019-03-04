#include "test/lsp/ProtocolTest.h"
#include "test/helpers/lsp.h"
#include "test/helpers/position_assertions.h"

namespace sorbet::test::lsp {
using namespace std;

void ProtocolTest::SetUp() {
    // Always use fast path
    // TODO: Toggleable?
    lspWrapper = make_unique<LSPWrapper>(true);
    rootPath = "/Users/jvilk/stripe/pay-server";
    rootUri = fmt::format("file://{}", rootPath);
}

void ProtocolTest::initializeLSP() {
    sorbet::test::initializeLSP(rootPath, rootUri, *lspWrapper, nextId);
}

string ProtocolTest::getUri(string_view filePath) {
    return filePathToUri(rootUri, filePath);
}

unique_ptr<LSPMessage> ProtocolTest::openFile(string_view path, string_view contents) {
    sourceFileContents[string(path)] =
        make_shared<core::File>(string(path), string(contents), core::File::Type::Normal);
    auto uri = getUri(path);
    auto didOpenParams =
        make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(uri, "ruby", 1, string(contents)));
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", "textDocument/didOpen");
    didOpenNotif->params = didOpenParams->toJSONValue(lspWrapper->alloc);
    return make_unique<LSPMessage>(move(didOpenNotif));
}

unique_ptr<LSPMessage> ProtocolTest::changeFile(string_view path, string_view newContents, int version) {
    sourceFileContents[string(path)] =
        make_shared<core::File>(string(path), string(newContents), core::File::Type::Normal);
    auto uri = getUri(path);
    auto textDocIdent = make_unique<VersionedTextDocumentIdentifier>(uri, version);
    vector<unique_ptr<TextDocumentContentChangeEvent>> changeEvents;
    changeEvents.push_back(make_unique<TextDocumentContentChangeEvent>(string(newContents)));
    auto didChangeParams = make_unique<DidChangeTextDocumentParams>(move(textDocIdent), move(changeEvents));
    auto didChangeNotif = make_unique<NotificationMessage>("2.0", "textDocument/didChange");
    didChangeNotif->params = didChangeParams->toJSONValue(lspWrapper->alloc);
    return make_unique<LSPMessage>(move(didChangeNotif));
}

unique_ptr<LSPMessage> ProtocolTest::documentSymbol(string_view path) {
    auto docSymParams = make_unique<DocumentSymbolParams>(make_unique<TextDocumentIdentifier>(getUri(path)));
    auto req = make_unique<RequestMessage>("2.0", nextId++, "textDocument/documentSymbol");
    req->params = docSymParams->toJSONValue(lspWrapper->alloc);
    return make_unique<LSPMessage>(move(req));
}

unique_ptr<LSPMessage> ProtocolTest::getDefinition(std::string_view path, int line, int character) {
    return makeDefinitionRequest(lspWrapper->alloc, nextId++, getUri(path), line, character);
}

unique_ptr<LSPMessage> ProtocolTest::cancelRequest(int id) {
    return makeRequestMessage(lspWrapper->alloc, "$/cancelRequest", nextId++, CancelParams(id));
}

vector<unique_ptr<LSPMessage>> ProtocolTest::send(const LSPMessage &message) {
    return lspWrapper->getLSPResponsesFor(message);
}

vector<unique_ptr<LSPMessage>> ProtocolTest::send(std::vector<std::unique_ptr<LSPMessage>> messages) {
    return lspWrapper->getLSPResponsesFor(messages);
}

void ProtocolTest::assertDiagnostics(vector<unique_ptr<LSPMessage>> messages, vector<ExpectedDiagnostic> expected) {
    // uri => diagnostics for that URI.
    map<std::string, vector<std::unique_ptr<Diagnostic>>> diagnostics;
    for (auto &msg : messages) {
        if (!assertNotificationMessage("textDocument/publishDiagnostics", *msg)) {
            // Assertion failed: Received a non-diagnostic. No need to continue.
            return;
        }
        if (auto diagnosticParams = getPublishDiagnosticParams(lspWrapper->alloc, msg->asNotification())) {
            // Will explicitly overwrite older diagnostics that are irrelevant.
            diagnostics[uriToFilePath(rootUri, (*diagnosticParams)->uri)] = move((*diagnosticParams)->diagnostics);
        }
    }

    // Convert ExpectedDiagnostic into ErrorAssertion objects.
    std::vector<std::shared_ptr<ErrorAssertion>> errorAssertions;
    for (auto e : expected) {
        auto range = RangeAssertion::makeRange(e.line);
        errorAssertions.push_back(ErrorAssertion::make(e.path, range, e.line, e.message));
    }

    // Use same logic as main test runner.
    ErrorAssertion::checkAll(sourceFileContents, errorAssertions, diagnostics);
}

} // namespace sorbet::test::lsp
