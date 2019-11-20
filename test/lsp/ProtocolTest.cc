#include "test/lsp/ProtocolTest.h"
#include "test/helpers/MockFileSystem.h"
#include "test/helpers/lsp.h"
#include "test/helpers/position_assertions.h"

namespace sorbet::test::lsp {
using namespace std;

void ProtocolTest::SetUp() {
    rootPath = "/Users/jvilk/stripe/pay-server";
    rootUri = fmt::format("file://{}", rootPath);
    fs = make_shared<MockFileSystem>(rootPath);
    lspWrapper = LSPWrapper::createSingleThreaded(rootPath);
    lspWrapper->opts->fs = fs;
    lspWrapper->enableAllExperimentalFeatures();
}

vector<unique_ptr<LSPMessage>> ProtocolTest::initializeLSP() {
    auto responses = sorbet::test::initializeLSP(rootPath, rootUri, *lspWrapper, nextId);
    updateDiagnostics(responses);
    return responses;
}

string ProtocolTest::getUri(string_view filePath) {
    return filePathToUri(lspWrapper->config(), filePath);
}

unique_ptr<LSPMessage> ProtocolTest::openFile(string_view path, string_view contents) {
    sourceFileContents[string(path)] =
        make_shared<core::File>(string(path), string(contents), core::File::Type::Normal);
    return makeOpen(getUri(path), contents, 1);
}

unique_ptr<LSPMessage> ProtocolTest::closeFile(string_view path) {
    // File is closed, so update contents from mock FS.
    try {
        sourceFileContents[string(path)] =
            make_shared<core::File>(string(path), string(fs->readFile(path)), core::File::Type::Normal);
    } catch (FileNotFoundException e) {
        auto it = sourceFileContents.find(path);
        if (it != sourceFileContents.end()) {
            sourceFileContents.erase(it);
        }
    }
    return makeClose(getUri(path));
}

unique_ptr<LSPMessage> ProtocolTest::changeFile(string_view path, string_view newContents, int version) {
    sourceFileContents[string(path)] =
        make_shared<core::File>(string(path), string(newContents), core::File::Type::Normal);
    return makeChange(getUri(path), newContents, version);
}

unique_ptr<LSPMessage> ProtocolTest::documentSymbol(string_view path) {
    auto docSymParams = make_unique<DocumentSymbolParams>(make_unique<TextDocumentIdentifier>(getUri(path)));
    auto req = make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentDocumentSymbol, move(docSymParams));
    return make_unique<LSPMessage>(move(req));
}

unique_ptr<LSPMessage> ProtocolTest::workspaceSymbol(string_view query) {
    return makeWorkspaceSymbolRequest(nextId++, query);
}

unique_ptr<LSPMessage> ProtocolTest::getDefinition(string_view path, int line, int character) {
    return makeDefinitionRequest(nextId++, getUri(path), line, character);
}

unique_ptr<LSPMessage> ProtocolTest::watchmanFileUpdate(vector<string> updatedFilePaths) {
    auto req = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange,
                                                make_unique<WatchmanQueryResponse>("", "", false, updatedFilePaths));
    return make_unique<LSPMessage>(move(req));
}

void ProtocolTest::writeFilesToFS(vector<pair<string, string>> files) {
    for (auto &file : files) {
        sourceFileContents[file.first] =
            make_shared<core::File>(string(file.first), string(file.second), core::File::Type::Normal);
    }
    fs->writeFiles(files);
}

void ProtocolTest::deleteFileFromFS(string_view filename) {
    auto it = sourceFileContents.find(filename);
    if (it != sourceFileContents.end()) {
        sourceFileContents.erase(it);
    }
    fs->deleteFile(filename);
}

unique_ptr<LSPMessage> ProtocolTest::cancelRequest(int id) {
    return make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::$CancelRequest, make_unique<CancelParams>(id)));
}

// Verify that messages are sound (contains proper JSON shape for method type) by serializing and re-parsing them.
vector<unique_ptr<LSPMessage>> verify(const vector<unique_ptr<LSPMessage>> &msgs) {
    vector<unique_ptr<LSPMessage>> reparsedMessages;
    for (auto &msg : msgs) {
        reparsedMessages.push_back(LSPMessage::fromClient(msg->toJSON()));
    }
    return reparsedMessages;
}

std::vector<std::unique_ptr<LSPMessage>> ProtocolTest::sendRaw(const std::string &json) {
    auto responses = verify(lspWrapper->getLSPResponsesFor(json));
    updateDiagnostics(responses);
    return responses;
}

vector<unique_ptr<LSPMessage>> ProtocolTest::send(const LSPMessage &message) {
    // Verify that message is sound (contains proper JSON shape for method type) by serializing and re-parsing it.
    return sendRaw(message.toJSON());
}

vector<unique_ptr<LSPMessage>> ProtocolTest::send(vector<unique_ptr<LSPMessage>> messages) {
    vector<unique_ptr<LSPMessage>> reparsedMessages = verify(messages);
    auto responses = verify(lspWrapper->getLSPResponsesFor(move(reparsedMessages)));
    updateDiagnostics(responses);
    return responses;
}

void ProtocolTest::updateDiagnostics(const vector<unique_ptr<LSPMessage>> &messages) {
    for (auto &msg : messages) {
        if (msg->isNotification() && msg->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            if (auto diagnosticParams = getPublishDiagnosticParams(msg->asNotification())) {
                // Will explicitly overwrite older diagnostics that are irrelevant.
                // TODO: Have a better way of copying.
                rapidjson::MemoryPoolAllocator<> alloc;
                diagnostics[uriToFilePath(lspWrapper->config(), (*diagnosticParams)->uri)] = move(
                    PublishDiagnosticsParams::fromJSONValue(*(*diagnosticParams)->toJSONValue(alloc))->diagnostics);
            }
        }
    }
}

std::string ProtocolTest::readFile(std::string_view uri) {
    auto readFileResponses = send(LSPMessage(make_unique<RequestMessage>(
        "2.0", nextId++, LSPMethod::SorbetReadFile, make_unique<TextDocumentIdentifier>(string(uri)))));
    EXPECT_EQ(readFileResponses.size(), 1);
    if (readFileResponses.size() == 1) {
        auto &readFileResponse = readFileResponses.at(0);
        EXPECT_TRUE(readFileResponse->isResponse());
        auto &readFileResult = get<unique_ptr<TextDocumentItem>>(*readFileResponse->asResponse().result);
        return readFileResult->text;
    }
    return "";
}

vector<unique_ptr<Location>> ProtocolTest::getDefinitions(std::string_view uri, int line, int character) {
    auto defResponses = send(*getDefinition(uri, line, character));
    EXPECT_EQ(defResponses.size(), 1);
    if (defResponses.size() == 1) {
        auto &defResponse = defResponses.at(0);
        EXPECT_TRUE(defResponse->isResponse());
        auto &defResult = get<variant<JSONNullObject, vector<unique_ptr<Location>>>>(*defResponse->asResponse().result);
        return move(get<vector<unique_ptr<Location>>>(defResult));
    }
    return {};
}

void ProtocolTest::assertDiagnostics(vector<unique_ptr<LSPMessage>> messages, vector<ExpectedDiagnostic> expected) {
    for (auto &msg : messages) {
        ASSERT_NO_FATAL_FAILURE(assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *msg));
    }

    // Convert ExpectedDiagnostic into ErrorAssertion objects.
    vector<shared_ptr<ErrorAssertion>> errorAssertions;
    for (auto e : expected) {
        auto range = RangeAssertion::makeRange(e.line);
        errorAssertions.push_back(ErrorAssertion::make(e.path, range, e.line, e.message, "error"));
    }

    // Use same logic as main test runner.
    ErrorAssertion::checkAll(sourceFileContents, errorAssertions, diagnostics);
}

} // namespace sorbet::test::lsp
