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

// Asserts that Sorbet forbids requesting definitions on untyped Ruby files, and displays a helpful messages in VS Code.
TEST_F(ProtocolTest, DefinitionError) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(
        send(*openFile("foobar.rb", "class Foobar\n  sig {returns(Integer)}\n  def bar\n    1\n  end\nend\n\nbar\n")),
        {});
    auto defResponses = send(*getDefinition("foobar.rb", 7, 1));
    ASSERT_EQ(defResponses.size(), 2) << "Expected two responses to request: An error, and a showMessage notification.";

    // Order doesn't matter; one should be a notification, the other should be a response
    auto &respMsg = defResponses.at(0)->isResponse() ? defResponses.at(0) : defResponses.at(1);
    auto &notifMsg = defResponses.at(0)->isNotification() ? defResponses.at(0) : defResponses.at(1);
    assertResponseMessage(nextId - 1, *respMsg);
    assertNotificationMessage("window/showMessage", *notifMsg);
    assertResponseError(-32602, "only works correctly on typed ruby files", *respMsg);

    auto showMessageParams = ShowMessageParams::fromJSONValue(lspWrapper->alloc, notifMsg->params(), "root.params");
    ASSERT_NE(showMessageParams->message.find("only works correctly on typed ruby files"), string::npos);
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
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", "textDocument/didOpen");
    didOpenNotif->params = didOpenParams->toJSONValue(lspWrapper->alloc);
    auto diagnostics = send(LSPMessage(move(didOpenNotif)));

    // Check the response for the expected URI.
    EXPECT_EQ(diagnostics.size(), 1);
    auto &msg = diagnostics.at(0);
    if (assertNotificationMessage("textDocument/publishDiagnostics", *msg)) {
        // Will fail test if this does not parse.
        if (auto diagnosticParams = getPublishDiagnosticParams(lspWrapper->alloc, msg->asNotification())) {
            EXPECT_EQ((*diagnosticParams)->uri, "memory://yolo1.rb");
        }
    }
}

} // namespace sorbet::test::lsp
