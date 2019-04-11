#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "ProtocolTest.h"
#include "common/common.h"
#include "test/helpers/lsp.h"

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

// Adds two new files that have errors, and asserts that Sorbet returns errors for both of them.
TEST_F(ProtocolTest, AddFile) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(send(*openFile("yolo1.rb", "")), {});

    ExpectedDiagnostic yolo1Diagnostic = {"yolo1.rb", 3, "doesn't match `Integer`"};
    assertDiagnostics(
        send(*changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 2)),
        {yolo1Diagnostic});
    assertDiagnostics(send(*openFile("yolo2.rb", "")), {yolo1Diagnostic});

    ExpectedDiagnostic yolo2Diagnostic = {"yolo2.rb", 4, "doesn't match `Integer`"};
    assertDiagnostics(
        send(*changeFile("yolo2.rb", "# typed: true\nclass Foo2\n\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 2)),
        {yolo1Diagnostic, yolo2Diagnostic});

    // Slightly change text so that error changes line and contents.
    ExpectedDiagnostic yolo2Diagnostic2 = {"yolo2.rb", 5, "stuff3"};
    assertDiagnostics(
        send(
            *changeFile("yolo2.rb", "# typed: true\nclass Foo2\n\n\n def branch\n    1 + \"stuff3\"\n  end\nend\n", 3)),
        {yolo1Diagnostic, yolo2Diagnostic2});
}

// Write to the same file twice. Sorbet should only return errors from the second version.
TEST_F(ProtocolTest, AddFileJoiningRequests) {
    assertDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo2\n  def branch\n    2 + \"dog\"\n  end\nend\n", 2));
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"bear\"\n  end\nend\n", 3));
    assertDiagnostics(send(move(requests)), {{"yolo1.rb", 3, "bear"}});
}

// Purposefully makes a change to a file that causes the slow path to trigger.
TEST_F(ProtocolTest, CacheInvalidation1) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(send(*openFile("yolo1.rb", "")), {});

    ExpectedDiagnostic yolo1Diagnostic = {"yolo1.rb", 3, "doesn't match `Integer`"};
    assertDiagnostics(
        send(*changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 2)),
        {yolo1Diagnostic});

    ExpectedDiagnostic yolo2Diagnostic1 = {"yolo2.rb", 3, "doesn't match `Integer`"};
    ExpectedDiagnostic yolo2Diagnostic2a = {"yolo2.rb", 1, "Unable to resolve constant `A`"};
    assertDiagnostics(
        send(*openFile("yolo2.rb", "# typed: true\nclass Foo2 < A\n  def branch\n    1 + \"stuff\"\n  end\nend\n")),
        {yolo1Diagnostic, yolo2Diagnostic1, yolo2Diagnostic2a});

    // Trigger slow path by changing Foo2's superclass in yolo2.rb
    ExpectedDiagnostic yolo2Diagnostic2b = {"yolo2.rb", 1, "Unable to resolve constant `B`"};
    assertDiagnostics(
        send(
            *changeFile("yolo2.rb", "# typed: true\nclass Foo2 < B\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 2)),
        {yolo1Diagnostic, yolo2Diagnostic1, yolo2Diagnostic2b});
}

// Cancels requests before they are processed, and ensures that they are actually not processed.
TEST_F(ProtocolTest, Cancellation) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(
        send(*openFile("foo.rb",
                       "#typed: true\nmodule Bar\n    CONST = 2\n\n    def self.meth(x)\n        x\n    "
                       "end\nend\n\nlocal = 131\nlocaler = local + 2\nlocaler2 = localer + 2\nlocal3 = localer + "
                       "local + 2\n\nconst_to_local = Bar::CONST;\nconst_add = Bar::CONST + "
                       "local\nconst_add_reverse = local + Bar::CONST;\n\nBar.meth(local)\nputs(Bar::CONST)\n")),
        {});

    // Make 3 requests that are immediately canceled.
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(getDefinition("foo.rb", 10, 12));
    requests.push_back(getDefinition("foo.rb", 18, 6));
    requests.push_back(getDefinition("foo.rb", 10, 2));

    int lastDefId = nextId - 1;
    requests.push_back(cancelRequest(lastDefId - 2));
    requests.push_back(cancelRequest(lastDefId - 1));
    requests.push_back(cancelRequest(lastDefId));

    UnorderedSet<int> requestIds = {lastDefId, lastDefId - 1, lastDefId - 2};
    auto errors = send(move(requests));

    ASSERT_EQ(errors.size(), 3) << "Expected three cancellation responses in response to three cancellation requests.";

    for (auto &errorMsg : errors) {
        ASSERT_TRUE(errorMsg->isResponse())
            << fmt::format("Expected cancellation response, received:\n{}", errorMsg->toJSON());
        auto idIt = requestIds.find((*errorMsg->id()).asInt());
        ASSERT_NE(idIt, requestIds.end())
            << fmt::format("Received cancellation response for invalid request id: {}", (*errorMsg->id()).asInt());
        requestIds.erase(idIt);
        assertResponseError(-32800, "cancel", *errorMsg);
    }
    ASSERT_EQ(requestIds.size(), 0);
}

// Asserts that Sorbet returns an empty result when requesting definitions in untyped Ruby files.
TEST_F(ProtocolTest, DefinitionError) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(
        send(*openFile("foobar.rb", "class Foobar\n  sig {returns(Integer)}\n  def bar\n    1\n  end\nend\n\nbar\n")),
        {});
    auto defResponses = send(*getDefinition("foobar.rb", 7, 1));
    ASSERT_EQ(defResponses.size(), 1) << "Expected a single response to a definition request to an untyped document.";

    assertResponseMessage(nextId - 1, *defResponses.at(0));

    auto &respMsg = defResponses.at(0)->asResponse();
    ASSERT_TRUE(respMsg.result);
    ASSERT_TRUE((*respMsg.result)->IsArray());
    ASSERT_EQ((*respMsg.result)->GetArray().Size(), 0);
}

// Tests that Sorbet LSP removes diagnostics from future responses when they are fixed.
TEST_F(ProtocolTest, FixErrors) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(
        send(*openFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n")),
        {{"yolo1.rb", 3, "doesn't match `Integer`"}});
    assertDiagnostics(
        send(*changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + 2\n  end\nend\n", 2)), {});
}

// Ensures that Sorbet merges didChanges that are interspersed with canceled requests.
TEST_F(ProtocolTest, MergeDidChangeAfterCancellation) {
    assertDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    // File is fine at first.
    requests.push_back(openFile("foo.rb", ""));
    // Invalid: Returns false.
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    false\n  end\nend\n",
                                  2));
    requests.push_back(documentSymbol("foo.rb"));
    auto cancelId1 = nextId - 1;
    // Invalid: Returns float
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    3.0\n  end\nend\n",
                                  3));
    requests.push_back(documentSymbol("foo.rb"));
    auto cancelId2 = nextId - 1;
    // Invalid: Returns unknown identifier.
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    blah\n  end\nend\n",
                                  4));
    requests.push_back(documentSymbol("foo.rb"));
    auto cancelId3 = nextId - 1;
    requests.push_back(cancelRequest(cancelId1));
    requests.push_back(cancelRequest(cancelId2));
    requests.push_back(cancelRequest(cancelId3));

    auto msgs = send(move(requests));
    // Expectation: Three cancellation requests, and an error from the final change.
    int cancelRequestCount = 0;
    int diagnosticCount = 0;
    for (auto &msg : msgs) {
        if (msg->isResponse()) {
            assertResponseError(-32800, "cancel", *msg);
            cancelRequestCount++;
        } else if (msg->isNotification()) {
            vector<unique_ptr<LSPMessage>> m;
            m.push_back(move(msg));
            assertDiagnostics(move(m), {{"foo.rb", 7, "Method `blah` does not exist"}});
            diagnosticCount++;
        } else {
            ADD_FAILURE() << fmt::format("Unexpected response:\n{}", msg->toJSON());
        }
    }
    EXPECT_EQ(cancelRequestCount, 3) << "Expected three cancellation messages.";
    EXPECT_EQ(diagnosticCount, 1) << "Expected a diagnostic error for foo.rb";
}

TEST_F(ProtocolTest, NotInitialized) {
    auto msgs = send(*getDefinition("foo.rb", 12, 24));
    ASSERT_EQ(msgs.size(), 1);
    auto &msg1 = msgs.at(0);
    assertResponseError(-32002, "not initialize", *msg1);
}

// Monaco doesn't send a root URI.
TEST_F(ProtocolTest, EmptyRootUri) {
    // Manually reset rootUri before initializing.
    rootUri = "";
    assertDiagnostics(initializeLSP(), {});

    // Manually construct an openFile with text that has a typechecking error.
    auto didOpenParams = make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(
        "memory://yolo1.rb", "ruby", 1, "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"));
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(didOpenParams));
    auto diagnostics = send(LSPMessage(move(didOpenNotif)));

    // Check the response for the expected URI.
    EXPECT_EQ(diagnostics.size(), 1);
    auto &msg = diagnostics.at(0);
    if (assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *msg)) {
        // Will fail test if this does not parse.
        if (auto diagnosticParams = getPublishDiagnosticParams(lspWrapper->alloc, msg->asNotification())) {
            EXPECT_EQ((*diagnosticParams)->uri, "memory://yolo1.rb");
        }
    }
}

TEST_F(ProtocolTest, CompletionOnNonClass) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(send(*openFile("yolo1.rb", "# typed: true\nclass A\nend\nA")), {});

    // TODO: Once we have better helpers for completion, clean this up.
    auto completionParams = make_unique<CompletionParams>(make_unique<TextDocumentIdentifier>(getUri("yolo1.rb")),
                                                          make_unique<Position>(3, 1));
    completionParams->context = make_unique<CompletionContext>(CompletionTriggerKind::Invoked);

    auto completionReq =
        make_unique<RequestMessage>("2.0", 100, LSPMethod::TextDocumentCompletion, move(completionParams));
    // We don't care about the result. We just care that Sorbet didn't die due to an ENFORCE failure.
    send(LSPMessage(move(completionReq)));
}

// Hides synthetic, internal arguments when sending hover results to client.
TEST_F(ProtocolTest, HidesSyntheticArgumentsOnHover) {
    assertDiagnostics(initializeLSP(), {});

    // There will be diagnostics, but we don't care about them.
    send(*openFile("yolo1.rb",
                   "# typed: true\nclass A\n  extend T::Sig\n\n  sig {params(x: Integer).returns(String)}\n "
                   " def bar(x)\n    x.to_s\n  end\nend\n\ndef main\n  A.new.bar(\"91\")\nend"));

    // TODO: Make this nicer once we have good hover helpers.
    auto hoverParams = make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(getUri("yolo1.rb")),
                                                               make_unique<Position>(11, 9));
    auto hoverReq = make_unique<RequestMessage>("2.0", 100, LSPMethod::TextDocumentHover, move(hoverParams));

    auto responses = send(LSPMessage(move(hoverReq)));
    EXPECT_EQ(responses.size(), 1);
    if (responses.size() == 1) {
        auto &result = responses.at(0);
        EXPECT_TRUE(result->isResponse());
        auto &response = result->asResponse();
        EXPECT_TRUE(response.result);
        auto &hoverOrNull = get<std::variant<JSONNullObject, std::unique_ptr<Hover>>>(*response.result);
        auto &hover = get<std::unique_ptr<Hover>>(hoverOrNull);
        auto content = MarkupContent::fromJSONValue(lspWrapper->alloc, *hover->contents);
        EXPECT_EQ(content->value, "sig {params(x: Integer).returns(String)}");
    }
}

// Ensures that unrecognized notifications are ignored.
TEST_F(ProtocolTest, IgnoresUnrecognizedNotifications) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(sendRaw("{\"jsonrpc\":\"2.0\",\"method\":\"workspace/"
                              "didChangeConfiguration\",\"params\":{\"settings\":{\"ruby-typer\":{}}}}"),
                      {});
}

// Ensures that notifications that have an improper params shape are handled gracefully / not responded to.
TEST_F(ProtocolTest, IgnoresNotificationsThatDontTypecheck) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(sendRaw("{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/didChange\",\"params\":{}}"), {});
}

// Ensures that unrecognized requests are responded to.
TEST_F(ProtocolTest, RejectsUnrecognizedRequests) {
    assertDiagnostics(initializeLSP(), {});
    auto responses = sendRaw("{\"jsonrpc\":\"2.0\",\"method\":\"workspace/"
                             "didChangeConfiguration\",\"id\":9001,\"params\":{\"settings\":{\"ruby-typer\":{}}}}");
    ASSERT_EQ(responses.size(), 1);
    auto &response = responses.at(0);
    ASSERT_TRUE(response->isResponse());
    auto &r = response->asResponse();
    ASSERT_FALSE(r.result);
    ASSERT_TRUE(r.error);
    auto &error = *r.error;
    ASSERT_NE(error->message.find("Unsupported LSP method"), std::string::npos);
    ASSERT_EQ(error->code, (int)LSPErrorCodes::MethodNotFound);
}

// Ensures that requests that have an improper params shape are responded to with an error.
TEST_F(ProtocolTest, RejectsRequestsThatDontTypecheck) {
    assertDiagnostics(initializeLSP(), {});
    auto responses = sendRaw("{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/"
                             "hover\",\"id\":9001,\"params\":{\"settings\":{\"ruby-typer\":{}}}}");
    ASSERT_EQ(responses.size(), 1);
    auto &response = responses.at(0);
    ASSERT_TRUE(response->isResponse());
    auto &r = response->asResponse();
    ASSERT_FALSE(r.result);
    ASSERT_TRUE(r.error);
    auto &error = *r.error;
    ASSERT_NE(error->message.find("Unable to deserialize LSP request"), std::string::npos);
    ASSERT_EQ(error->code, (int)LSPErrorCodes::InvalidParams);
}

// Ensures that the server ignores invalid JSON.
TEST_F(ProtocolTest, SilentlyIgnoresInvalidJSONMessages) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(sendRaw("{"), {});
}

} // namespace sorbet::test::lsp
