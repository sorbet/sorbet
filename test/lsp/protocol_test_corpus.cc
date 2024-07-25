#include "doctest/doctest.h"
// ^ Violates linting rules, so include first.
#include "ProtocolTest.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/common.h"
#include "test/helpers/lsp.h"

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

// Adds two new files that have errors, and asserts that Sorbet returns errors for both of them.
TEST_CASE_FIXTURE(ProtocolTest, "AddFile") {
    assertErrorDiagnostics(initializeLSP(), {});
    assertErrorDiagnostics(send(*openFile("yolo1.rb", "")), {});

    ExpectedDiagnostic yolo1Diagnostic = {"yolo1.rb", 3, "Expected `Integer`"};
    assertErrorDiagnostics(
        send(*changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 2)),
        {yolo1Diagnostic});
    assertErrorDiagnostics(send(*openFile("yolo2.rb", "")), {yolo1Diagnostic});

    ExpectedDiagnostic yolo2Diagnostic = {"yolo2.rb", 4, "Expected `Integer`"};
    assertErrorDiagnostics(
        send(*changeFile("yolo2.rb", "# typed: true\nclass Foo2\n\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 2)),
        {yolo1Diagnostic, yolo2Diagnostic});

    // Slightly change text so that error changes line and contents.
    ExpectedDiagnostic yolo2Diagnostic2 = {"yolo2.rb", 5, "stuff3"};
    assertErrorDiagnostics(
        send(
            *changeFile("yolo2.rb", "# typed: true\nclass Foo2\n\n\n def branch\n    1 + \"stuff3\"\n  end\nend\n", 3)),
        {yolo1Diagnostic, yolo2Diagnostic2});
}

// Write to the same file twice. Sorbet should only return errors from the second version.
TEST_CASE_FIXTURE(ProtocolTest, "AddFileJoiningRequests") {
    assertErrorDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(openFile("yolo1.rb", "# typed: true\nclass Foo2\n  def branch\n    2 + \"dog\"\n  end\nend\n"));
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"bear\"\n  end\nend\n", 3));
    assertErrorDiagnostics(send(move(requests)), {{"yolo1.rb", 3, "bear"}});
}

// Cancels requests before they are processed, and ensures that they are actually not processed.
TEST_CASE_FIXTURE(ProtocolTest, "Cancellation") {
    assertErrorDiagnostics(initializeLSP(), {});
    assertErrorDiagnostics(
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

    INFO("Expected three cancellation responses in response to three cancellation requests.");
    REQUIRE_EQ(errors.size(), 3);

    for (auto &errorMsg : errors) {
        {
            INFO(fmt::format("Expected cancellation response, received:\n{}", errorMsg->toJSON()));
            REQUIRE(errorMsg->isResponse());
        }
        auto idIt = requestIds.find((*errorMsg->id()).asInt());
        {
            INFO(fmt::format("Received cancellation response for invalid request id: {}", (*errorMsg->id()).asInt()));
            REQUIRE_NE(idIt, requestIds.end());
        }
        requestIds.erase(idIt);
        assertResponseError(-32800, "cancel", *errorMsg);
    }
    REQUIRE_EQ(requestIds.size(), 0);
}

// Asserts that Sorbet returns an empty result when requesting definitions in untyped Ruby files.
TEST_CASE_FIXTURE(ProtocolTest, "DefinitionError") {
    assertErrorDiagnostics(initializeLSP(), {});
    assertErrorDiagnostics(send(*openFile("foobar.rb", "class Foobar\n  def bar\n    1\n  end\nend\n\nbar\n")), {});
    auto defResponses = send(*getDefinition("foobar.rb", 6, 1));
    INFO("Expected a single response to a definition request to an untyped document.");
    const auto numResponses = absl::c_count_if(defResponses, [](const auto &m) { return m->isResponse(); });
    REQUIRE_EQ(1, numResponses);
    const auto numRequests = absl::c_count_if(defResponses, [](const auto &m) { return m->isRequest(); });
    REQUIRE_EQ(0, numRequests);
    const auto numNotifications = absl::c_count_if(defResponses, [](const auto &m) { return m->isNotification(); });
    REQUIRE_EQ(0, numNotifications);
    // Ensure the lone response is at the front.
    absl::c_partition(defResponses, [](const auto &m) { return m->isResponse(); });
    assertResponseMessage(nextId - 1, *defResponses.at(0));

    auto &respMsg = defResponses.at(0)->asResponse();
    REQUIRE(respMsg.result);
    auto &result = get<variant<JSONNullObject, vector<unique_ptr<Location>>>>(*(respMsg.result));
    auto &array = get<vector<unique_ptr<Location>>>(result);
    REQUIRE_EQ(array.size(), 0);
}

// Ensures that Sorbet merges didChanges that are interspersed with canceled requests.
TEST_CASE_FIXTURE(ProtocolTest, "MergeDidChangeAfterCancellation") {
    assertErrorDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    // File is fine at first.
    requests.push_back(openFile("foo.rb", ""));
    // Invalid: Returns false.
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    false\n  end\nend\n",
                                  2));
    requests.push_back(workspaceSymbol("Foo"));
    auto cancelId1 = nextId - 1;
    // Invalid: Returns float
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    3.0\n  end\nend\n",
                                  3));
    requests.push_back(workspaceSymbol("Foo"));
    auto cancelId2 = nextId - 1;
    // Invalid: Returns unknown identifier.
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    blah\n  end\nend\n",
                                  4));
    requests.push_back(workspaceSymbol("Foo"));
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
        } else if (msg->isNotification() && msg->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            diagnosticCount++;
        } else {
            FAIL_CHECK(fmt::format("Unexpected response:\n{}", msg->toJSON()));
        }
    }
    assertErrorDiagnostics({}, {{"foo.rb", 7, "Method `blah` does not exist"}});
    CHECK_EQ(cancelRequestCount, 3);
    // Expected a diagnostic error for foo.rb
    CHECK_EQ(diagnosticCount, 1);
}

// Applies all consecutive file changes at once.
TEST_CASE_FIXTURE(ProtocolTest, "MergesDidChangesAcrossFiles") {
    assertErrorDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    // File is fine at first.
    requests.push_back(openFile("foo.rb", ""));
    // Invalid: Returns false.
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    false\n  end\nend\n",
                                  2));
    requests.push_back(openFile("bar.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"));
    // Invalid: Returns float
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    3.0\n  end\nend\n",
                                  3));
    writeFilesToFS({{"baz.rb", "# typed: true\nclass Foo2\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    writeFilesToFS({{"bat.rb", "# typed: true\nclass Foo3\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    requests.push_back(watchmanFileUpdate({"baz.rb"}));
    // Final state: Returns unknown identifier.
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    blah\n  end\nend\n",
                                  4));
    requests.push_back(closeFile("bat.rb"));

    auto msgs = send(move(requests));
    INFO("Expected only 4 diagnostic responses to the merged file changes");
    CHECK_EQ(msgs.size(), 4);
    assertErrorDiagnostics(move(msgs), {{"bar.rb", 3, "Expected `Integer`"},
                                        {"baz.rb", 3, "Expected `Integer`"},
                                        {"bat.rb", 3, "Expected `Integer`"},
                                        {"foo.rb", 7, "Method `blah` does not exist"}});
}

TEST_CASE_FIXTURE(ProtocolTest, "MergesDidChangesAcrossDelayableRequests") {
    assertErrorDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    // Invalid: Returns false.
    requests.push_back(openFile("foo.rb", "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                          "{returns(Integer)}\n  def bar\n    false\n  end\nend\n"));
    // Document symbol is delayable.
    requests.push_back(documentSymbol("foo.rb"));
    // Invalid: Returns float
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    3.0\n  end\nend\n",
                                  3));
    requests.push_back(documentSymbol("foo.rb"));
    // Invalid: Returns unknown identifier.
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    blah\n  end\nend\n",
                                  4));

    auto msgs = send(move(requests));
    REQUIRE_GT(msgs.size(), 0);
    CHECK(msgs.at(0)->isNotification());
    CHECK_EQ(msgs.at(0)->method(), LSPMethod::TextDocumentPublishDiagnostics);
    assertErrorDiagnostics({}, {{"foo.rb", 7, "Method `blah` does not exist"}});

    INFO("Expected five diagnostic notifications and two document symbol responses.");

    int diagnostics = 0;
    auto documentSymbols = 0;
    for (auto const &m : msgs) {
        if (m->isResponse()) {
            documentSymbols++;
        }
        if (m->isNotification()) {
            diagnostics++;
        }
    }

    REQUIRE_EQ(msgs.size(), 7);
    REQUIRE_EQ(diagnostics, 5);
    REQUIRE_EQ(documentSymbols, 2);
}

TEST_CASE_FIXTURE(ProtocolTest, "DoesNotMergeFileChangesAcrossNonDelayableRequests") {
    assertErrorDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(openFile("foo.rb", "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                          "{returns(Integer)}\n  def bar\n    false\n  end\nend\n"));
    // Should block ^ and V from merging.
    requests.push_back(hover("foo.rb", 1, 1));
    requests.push_back(changeFile("foo.rb",
                                  "# typed: true\n\nclass Opus::CIBot::Tasks::Foo\n  extend T::Sig\n\n  sig "
                                  "{returns(Integer)}\n  def bar\n    blah\n  end\nend\n",
                                  4));

    auto msgs = send(move(requests));
    // [diagnostics, documentsymbol, diagnostics, diagnostics, diagnostics]
    CHECK_EQ(msgs.size(), 5);
    if (auto diagnosticParams = getPublishDiagnosticParams(msgs.at(0)->asNotification())) {
        CHECK((*diagnosticParams)->uri.find("foo.rb") != string::npos);
        auto &diagnostics = (*diagnosticParams)->diagnostics;
        CHECK_EQ(diagnostics.size(), 1);
        CHECK(diagnostics.at(0)->message.find("for method result type") != string::npos);
    }
    CHECK(msgs.at(1)->isResponse());
    CHECK(msgs.at(2)->isNotification());
    CHECK(msgs.at(3)->isNotification());
    if (auto diagnosticParams = getPublishDiagnosticParams(msgs.at(4)->asNotification())) {
        CHECK((*diagnosticParams)->uri.find("foo.rb") != string::npos);
        auto &diagnostics = (*diagnosticParams)->diagnostics;
        CHECK_EQ(diagnostics.size(), 1);
        CHECK(diagnostics.at(0)->message.find("Method `blah` does not exist") != string::npos);
    }
}

TEST_CASE_FIXTURE(ProtocolTest, "NotInitialized") {
    // Don't use `getDefinition`; it only works post-initialization.
    auto msgs = send(*makeDefinitionRequest(nextId++, "foo.rb", 12, 24));
    REQUIRE_EQ(msgs.size(), 1);
    auto &msg1 = msgs.at(0);
    assertResponseError(-32002, "not initialize", *msg1);
}

// There's a different code path that checks for workspace edits before initialization occurs.
TEST_CASE_FIXTURE(ProtocolTest, "WorkspaceEditIgnoredWhenNotInitialized") {
    // Purposefully send a vector of requests to trigger merging, which should turn this into a WorkspaceEdit.
    vector<unique_ptr<LSPMessage>> toSend;
    // Avoid using `openFile`, as it only works post-initialization.
    toSend.push_back(makeOpen("bar.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 1));
    // This update should be ignored.
    assertErrorDiagnostics(send(move(toSend)), {});
    // We shouldn't have any code errors post-initialization since the previous edit was ignored.
    assertErrorDiagnostics(initializeLSP(), {});
}

TEST_CASE_FIXTURE(ProtocolTest, "InitializeAndShutdown") {
    assertErrorDiagnostics(initializeLSP(), {});
    auto resp = send(LSPMessage(make_unique<RequestMessage>("2.0", nextId++, LSPMethod::Shutdown, JSONNullObject())));
    INFO("Expected a single response to shutdown request.");
    REQUIRE_EQ(resp.size(), 1);
    auto &r = resp.at(0)->asResponse();
    REQUIRE_EQ(r.requestMethod, LSPMethod::Shutdown);
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::Exit, JSONNullObject()))),
                           {});
}

// Some clients send an empty string for the root uri.
TEST_CASE_FIXTURE(ProtocolTest, "EmptyRootUriInitialization") {
    // Manually reset rootUri before initializing.
    rootUri = "";
    assertErrorDiagnostics(initializeLSP(), {});

    // Manually construct an openFile with text that has a typechecking error.
    auto didOpenParams = make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(
        "memory://yolo1.rb", "ruby", 1, "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"));
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(didOpenParams));
    auto diagnostics = send(LSPMessage(move(didOpenNotif)));

    // Check the response for the expected URI.
    CHECK_EQ(diagnostics.size(), 1);
    auto &msg = diagnostics.at(0);
    assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *msg);
    // Will fail test if this does not parse.
    if (auto diagnosticParams = getPublishDiagnosticParams(msg->asNotification())) {
        CHECK_EQ((*diagnosticParams)->uri, "memory://yolo1.rb");
    }
}

// Root path is technically optional since it's deprecated.
TEST_CASE_FIXTURE(ProtocolTest, "MissingRootPathInitialization") {
    // Null is functionally equivalent to an empty rootUri. Manually reset rootUri before initializing.
    rootUri = "";
    const bool supportsMarkdown = true;
    auto params =
        make_unique<RequestMessage>("2.0", nextId++, LSPMethod::Initialize,
                                    makeInitializeParams(nullopt, JSONNullObject(), supportsMarkdown, false, nullopt));
    {
        auto responses = send(LSPMessage(move(params)));
        INFO("Expected only a single response to the initialize request.");
        REQUIRE_EQ(responses.size(), 1);
        auto &respMsg = responses.at(0);
        CHECK(respMsg->isResponse());
        auto &resp = respMsg->asResponse();
        CHECK_EQ(resp.requestMethod, LSPMethod::Initialize);
    }
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::Initialized,
                                                                            make_unique<InitializedParams>()))),
                           {});

    // Manually construct an openFile with text that has a typechecking error.
    auto didOpenParams = make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(
        "memory://yolo1.rb", "ruby", 1, "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"));
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(didOpenParams));
    auto diagnostics = send(LSPMessage(move(didOpenNotif)));

    // Check the response for the expected URI.
    CHECK_EQ(diagnostics.size(), 1);
    auto &msg = diagnostics.at(0);
    assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *msg);
    // Will fail test if this does not parse.
    if (auto diagnosticParams = getPublishDiagnosticParams(msg->asNotification())) {
        CHECK_EQ((*diagnosticParams)->uri, "memory://yolo1.rb");
    }
}

// Monaco sends null for the root URI.
TEST_CASE_FIXTURE(ProtocolTest, "MonacoInitialization") {
    // Null is functionally equivalent to an empty rootUri. Manually reset rootUri before initializing.
    rootUri = "";
    const bool supportsMarkdown = true;
    auto params = make_unique<RequestMessage>(
        "2.0", nextId++, LSPMethod::Initialize,
        makeInitializeParams(JSONNullObject(), JSONNullObject(), supportsMarkdown, false, nullopt));
    {
        auto responses = send(LSPMessage(move(params)));
        INFO("Expected only a single response to the initialize request");
        REQUIRE_EQ(responses.size(), 1);
        auto &respMsg = responses.at(0);
        CHECK(respMsg->isResponse());
        auto &resp = respMsg->asResponse();
        CHECK_EQ(resp.requestMethod, LSPMethod::Initialize);
    }
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::Initialized,
                                                                            make_unique<InitializedParams>()))),
                           {});

    // Manually construct an openFile with text that has a typechecking error.
    auto didOpenParams = make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(
        "memory://yolo1.rb", "ruby", 1, "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"));
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(didOpenParams));
    auto diagnostics = send(LSPMessage(move(didOpenNotif)));

    // Check the response for the expected URI.
    CHECK_EQ(diagnostics.size(), 1);
    auto &msg = diagnostics.at(0);
    assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *msg);
    // Will fail test if this does not parse.
    if (auto diagnosticParams = getPublishDiagnosticParams(msg->asNotification())) {
        CHECK_EQ((*diagnosticParams)->uri, "memory://yolo1.rb");
    }
}

TEST_CASE_FIXTURE(ProtocolTest, "CompletionOnNonClass") {
    assertErrorDiagnostics(initializeLSP(), {});
    assertErrorDiagnostics(send(*openFile("yolo1.rb", "# typed: true\nclass A\nend\nA")), {});

    // TODO: Once we have better helpers for completion, clean this up.
    auto completionParams = make_unique<CompletionParams>(make_unique<TextDocumentIdentifier>(getUri("yolo1.rb")),
                                                          make_unique<Position>(3, 1));
    completionParams->context = make_unique<CompletionContext>(CompletionTriggerKind::Invoked);

    auto completionReq =
        make_unique<RequestMessage>("2.0", 100, LSPMethod::TextDocumentCompletion, move(completionParams));
    // We don't care about the result. We just care that Sorbet didn't die due to an ENFORCE failure.
    send(LSPMessage(move(completionReq)));
}

// Ensures that unrecognized notifications are ignored.
TEST_CASE_FIXTURE(ProtocolTest, "IgnoresUnrecognizedNotifications") {
    assertErrorDiagnostics(initializeLSP(), {});
    assertErrorDiagnostics(sendRaw("{\"jsonrpc\":\"2.0\",\"method\":\"workspace/"
                                   "didChangeConfiguration\",\"params\":{\"settings\":{\"ruby-typer\":{}}}}"),
                           {});
}

// Ensures that notifications that have an improper params shape are handled gracefully / not responded to.
TEST_CASE_FIXTURE(ProtocolTest, "IgnoresNotificationsThatDontTypecheck") {
    assertErrorDiagnostics(initializeLSP(), {});
    assertErrorDiagnostics(sendRaw(R"({"jsonrpc":"2.0","method":"textDocument/didChange","params":{}})"), {});
}

// Ensures that unrecognized requests are responded to.
TEST_CASE_FIXTURE(ProtocolTest, "RejectsUnrecognizedRequests") {
    assertErrorDiagnostics(initializeLSP(), {});
    auto responses = sendRaw("{\"jsonrpc\":\"2.0\",\"method\":\"sorbet/"
                             "fooBar\",\"id\":9001,\"params\":{\"settings\":{\"highlightUntyped\": false}}}");
    REQUIRE_EQ(responses.size(), 1);
    auto &response = responses.at(0);
    REQUIRE(response->isResponse());
    auto &r = response->asResponse();
    REQUIRE_FALSE(r.result);
    REQUIRE(r.error);
    auto &error = *r.error;
    REQUIRE_NE(error->message.find("Unsupported LSP method"), std::string::npos);
    REQUIRE_EQ(error->code, (int)LSPErrorCodes::MethodNotFound);
}

// Ensures that requests that have an improper params shape are responded to with an error.
TEST_CASE_FIXTURE(ProtocolTest, "RejectsRequestsThatDontTypecheck") {
    assertErrorDiagnostics(initializeLSP(), {});
    auto responses = sendRaw("{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/"
                             "hover\",\"id\":9001,\"params\":{\"settings\":{\"ruby-typer\":{}}}}");
    REQUIRE_EQ(responses.size(), 1);
    auto &response = responses.at(0);
    REQUIRE(response->isResponse());
    auto &r = response->asResponse();
    REQUIRE_FALSE(r.result);
    REQUIRE(r.error);
    auto &error = *r.error;
    REQUIRE_NE(error->message.find("Unable to deserialize LSP request"), std::string::npos);
    REQUIRE_EQ(error->code, (int)LSPErrorCodes::InvalidParams);
}

// Ensures that the server ignores invalid JSON.
TEST_CASE_FIXTURE(ProtocolTest, "SilentlyIgnoresInvalidJSONMessages") {
    assertErrorDiagnostics(initializeLSP(), {});
    assertErrorDiagnostics(sendRaw("{"), {});
}

// If a client doesn't support markdown, send hover as plaintext.
TEST_CASE_FIXTURE(ProtocolTest, "RespectsHoverTextLimitations") {
    assertErrorDiagnostics(initializeLSP(false /* supportsMarkdown */), {});

    assertErrorDiagnostics(send(*openFile("foobar.rb", "# typed: true\n1\n")), {});

    auto hoverResponses = send(LSPMessage(make_unique<RequestMessage>(
        "2.0", nextId++, LSPMethod::TextDocumentHover,
        make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(getUri("foobar.rb")),
                                                make_unique<Position>(1, 0)))));
    REQUIRE_EQ(hoverResponses.size(), 1);
    auto &hoverResponse = hoverResponses.at(0);
    REQUIRE(hoverResponse->isResponse());
    auto &hoverResult = get<variant<JSONNullObject, unique_ptr<Hover>>>(*hoverResponse->asResponse().result);
    auto &hover = get<unique_ptr<Hover>>(hoverResult);
    auto &contents = hover->contents;
    REQUIRE_EQ(contents->kind, MarkupKind::Plaintext);
    REQUIRE_EQ(contents->value, "Integer(1)");
}

// Tests that Sorbet returns sorbet: URIs for payload references & files not on client, and that readFile works on
// them.
TEST_CASE_FIXTURE(ProtocolTest, "SorbetURIsWork") {
    const bool supportsMarkdown = false;
    const bool supportsCodeActionResolve = false;
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->supportsSorbetURIs = true;
    lspWrapper->opts->lspDirsMissingFromClient.emplace_back("/folder");
    assertErrorDiagnostics(initializeLSP(supportsMarkdown, supportsCodeActionResolve, move(initOptions)), {});

    string fileContents = "# typed: true\n[0,1,2,3].select {|x| x > 0}\ndef myMethod; end;\n";
    assertErrorDiagnostics(send(*openFile("folder/foo.rb", fileContents)), {});

    auto selectDefinitions = getDefinitions("folder/foo.rb", 1, 11);
    REQUIRE_EQ(selectDefinitions.size(), 1);
    auto &selectLoc = selectDefinitions.at(0);
    REQUIRE(absl::StartsWith(selectLoc->uri, "sorbet:https://github.com/"));
    REQUIRE_FALSE(readFile(selectLoc->uri).empty());

    auto myMethodDefinitions = getDefinitions("folder/foo.rb", 2, 5);
    REQUIRE_EQ(myMethodDefinitions.size(), 1);
    auto &myMethodDefLoc = myMethodDefinitions.at(0);
    REQUIRE_EQ(myMethodDefLoc->uri, "sorbet:folder/foo.rb");
    REQUIRE_EQ(readFile(myMethodDefLoc->uri), fileContents);

    // VS Code replaces : in https with something URL-escaped; test that we handle this use-case.
    auto arrayRBI = readFile(selectLoc->uri);
    auto arrayRBIURLEncodeColon =
        readFile(absl::StrReplaceAll(selectLoc->uri, {{"https://github.com/", "https%3A//github.com/"}}));
    REQUIRE_EQ(arrayRBI, arrayRBIURLEncodeColon);
}

// Tests that Sorbet URIs are not typechecked.
TEST_CASE_FIXTURE(ProtocolTest, "DoesNotTypecheckSorbetURIs") {
    const bool supportsMarkdown = false;
    const bool supportsCodeActionResolve = false;
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->supportsSorbetURIs = true;
    initOptions->enableTypecheckInfo = true;
    lspWrapper->opts->lspDirsMissingFromClient.emplace_back("/folder");
    // Don't assert diagnostics; it will fail due to the spurious typecheckinfo message.
    initializeLSP(supportsMarkdown, supportsCodeActionResolve, move(initOptions));

    string fileContents = "# typed: true\n[0,1,2,3].select {|x| x > 0}\ndef myMethod; end;\n";
    send(*openFile("folder/foo.rb", fileContents));

    auto selectDefinitions = getDefinitions("folder/foo.rb", 1, 11);
    REQUIRE_EQ(selectDefinitions.size(), 1);
    auto &selectLoc = selectDefinitions.at(0);
    REQUIRE(absl::StartsWith(selectLoc->uri, "sorbet:https://github.com/"));
    auto contents = readFile(selectLoc->uri);

    // Test that opening and closing one of these files doesn't cause a slow path.
    vector<unique_ptr<LSPMessage>> openClose;
    openClose.push_back(makeOpen(selectLoc->uri, contents, 1));
    openClose.push_back(makeClose(selectLoc->uri));
    auto responses = send(move(openClose));
    REQUIRE_EQ(0, responses.size());
}

// Tests that files with url encoded characters in their name are matched to local files
TEST_CASE_FIXTURE(ProtocolTest, "MatchesFilesWithUrlEncodedNames") {
    initializeLSP(false, false, {});

    string filename = "test file@123+%&*#!.rbi";
    string encodedFilename = "test%20file%40123%2B%25%26*%23!.rbi";

    send(*openFile(filename, "# typed: true\nclass Foo; end;\n"));

    auto rbi = readFile(getUri(filename));
    auto rbiURLEncoded = readFile(getUri(encodedFilename));
    REQUIRE_EQ(rbi, rbiURLEncoded);
}

// Tests that Sorbet does not crash when a file URI falls outside of the workspace.
TEST_CASE_FIXTURE(ProtocolTest, "DoesNotCrashOnNonWorkspaceURIs") {
    const bool supportsMarkdown = false;
    const bool supportsCodeActionResolve = false;
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->supportsSorbetURIs = true;

    // Manually invoke to customize rootURI and rootPath.
    auto initializeResponses = sorbet::test::initializeLSP(
        "/Users/jvilk/stripe/areallybigfoldername", "file://Users/jvilk/stripe/areallybigfoldername", *lspWrapper,
        nextId, supportsMarkdown, supportsCodeActionResolve, make_optional(move(initOptions)));

    auto fileUri = "file:///Users/jvilk/Desktop/test.rb";
    auto didOpenParams =
        make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(fileUri, "ruby", 1, "# typed: true\n1\n"));
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(didOpenParams));
    getLSPResponsesFor(*lspWrapper, make_unique<LSPMessage>(move(didOpenNotif)));
}

// Tests that Sorbet does not crash when attempting to format a file URI outside of the workspace.
TEST_CASE_FIXTURE(ProtocolTest, "DoesNotCrashOnFormattingNonWorkspaceURIs") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->supportsSorbetURIs = true;

    // Manually invoke to customize rootURI and rootPath.
    auto initializeResponses = sorbet::test::initializeLSP(
        "/Users/jvilk/stripe/areallybigfoldername", "file://Users/jvilk/stripe/areallybigfoldername", *lspWrapper,
        nextId, false, false, make_optional(move(initOptions)));

    auto fileUri = "file:///Users/jvilk/Desktop/test.rb";
    auto documentFormattingParams = make_unique<DocumentFormattingParams>(
        make_unique<TextDocumentIdentifier>(string(fileUri)), make_unique<FormattingOptions>(0, 0));
    auto resp = send(LSPMessage(make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentFormatting,
                                                            move(documentFormattingParams))));
    // Just assert that this doesn't crash -- it's really a no-op
    INFO("Expected only a single response to the formatting request.");
    REQUIRE_EQ(resp.size(), 1);
}

// Tests that Sorbet reports metrics about the request's response status for certain requests
TEST_CASE_FIXTURE(ProtocolTest, "RequestReportsEmptyResultsMetrics") {
    assertErrorDiagnostics(initializeLSP(), {});

    // Create a new file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\n"
                                                    "class A\n"
                                                    "def foo; end\n"
                                                    "end\n"
                                                    "A.new.fo\n"
                                                    "A.new.no_completion_results\n"
                                                    "A.new.foo\n"
                                                    "T.unsafe(nil).foo\n"
                                                    "\n")),
                           {
                               {"foo.rb", 4, "does not exist"},
                               {"foo.rb", 5, "does not exist"},
                           });

    // clear counters
    getCounters();

    send(*completion("foo.rb", 4, 8));

    auto counters1 = getCounters();
    CHECK_EQ(counters1.getCategoryCounter("lsp.messages.processed", "textDocument.completion"), 1);
    CHECK_EQ(counters1.getCategoryCounter("lsp.messages.canceled", "textDocument.completion"), 0);
    CHECK_EQ(counters1.getCategoryCounter("lsp.messages.run.succeeded", "textDocument.completion"), 1);
    CHECK_EQ(counters1.getCategoryCounter("lsp.messages.run.emptyresult", "textDocument.completion"), 0);
    CHECK_EQ(counters1.getCategoryCounter("lsp.messages.run.errored", "textDocument.completion"), 0);

    send(*completion("foo.rb", 5, 27));

    auto counters2 = getCounters();
    CHECK_EQ(counters2.getCategoryCounter("lsp.messages.processed", "textDocument.completion"), 1);
    CHECK_EQ(counters2.getCategoryCounter("lsp.messages.canceled", "textDocument.completion"), 0);
    CHECK_EQ(counters2.getCategoryCounter("lsp.messages.run.succeeded", "textDocument.completion"), 0);
    CHECK_EQ(counters2.getCategoryCounter("lsp.messages.run.emptyresult", "textDocument.completion"), 1);
    CHECK_EQ(counters2.getCategoryCounter("lsp.messages.run.errored", "textDocument.completion"), 0);

    send(*getDefinition("foo.rb", 6, 7));

    auto counters3 = getCounters();
    CHECK_EQ(counters3.getCategoryCounter("lsp.messages.processed", "textDocument.definition"), 1);
    CHECK_EQ(counters3.getCategoryCounter("lsp.messages.canceled", "textDocument.definition"), 0);
    CHECK_EQ(counters3.getCategoryCounter("lsp.messages.run.succeeded", "textDocument.definition"), 1);
    CHECK_EQ(counters3.getCategoryCounter("lsp.messages.run.emptyresult", "textDocument.definition"), 0);
    CHECK_EQ(counters3.getCategoryCounter("lsp.messages.run.errored", "textDocument.definition"), 0);

    send(*getDefinition("foo.rb", 5, 7));

    auto counters4 = getCounters();
    CHECK_EQ(counters4.getCategoryCounter("lsp.messages.processed", "textDocument.definition"), 1);
    CHECK_EQ(counters4.getCategoryCounter("lsp.messages.canceled", "textDocument.definition"), 0);
    CHECK_EQ(counters4.getCategoryCounter("lsp.messages.run.succeeded", "textDocument.definition"), 0);
    CHECK_EQ(counters4.getCategoryCounter("lsp.messages.run.emptyresult", "textDocument.definition"), 1);
    CHECK_EQ(counters4.getCategoryCounter("lsp.messages.run.errored", "textDocument.definition"), 0);

    send(*hover("foo.rb", 6, 7));

    auto counters5 = getCounters();
    CHECK_EQ(counters5.getCategoryCounter("lsp.messages.processed", "textDocument.hover"), 1);
    CHECK_EQ(counters5.getCategoryCounter("lsp.messages.canceled", "textDocument.hover"), 0);
    CHECK_EQ(counters5.getCategoryCounter("lsp.messages.run.succeeded", "textDocument.hover"), 1);
    CHECK_EQ(counters5.getCategoryCounter("lsp.messages.run.emptyresult", "textDocument.hover"), 0);
    CHECK_EQ(counters5.getCategoryCounter("lsp.messages.run.errored", "textDocument.hover"), 0);

    send(*hover("foo.rb", 7, 16));

    auto counters6 = getCounters();
    CHECK_EQ(counters6.getCategoryCounter("lsp.messages.processed", "textDocument.hover"), 1);
    CHECK_EQ(counters6.getCategoryCounter("lsp.messages.canceled", "textDocument.hover"), 0);
    CHECK_EQ(counters6.getCategoryCounter("lsp.messages.run.succeeded", "textDocument.hover"), 0);
    CHECK_EQ(counters6.getCategoryCounter("lsp.messages.run.emptyresult", "textDocument.hover"), 1);
    CHECK_EQ(counters6.getCategoryCounter("lsp.messages.run.errored", "textDocument.hover"), 0);
}

TEST_CASE_FIXTURE(ProtocolTest, "ReportsSyntaxErrors") {
    assertErrorDiagnostics(initializeLSP(), {});

    // Create a new file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\n"
                                                    "class A\n"
                                                    "def foo; end\n"
                                                    "end\n"
                                                    "\n")),
                           {});

    // clear counters
    getCounters();

    assertErrorDiagnostics(send(*changeFile("foo.rb",
                                            "# typed: true\n"
                                            "class A\n"
                                            "def foo(; end\n"
                                            "end\n"
                                            "\n",
                                            2)),
                           {
                               {"foo.rb", 1, "class definition in method body"},
                               {"foo.rb", 2, "unexpected token \";\""},
                           });

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.slow_path_reason", "syntax_error"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.slow_path_reason", "changed_definition"), 0);
}

TEST_CASE_FIXTURE(ProtocolTest, "DidChangeConfigurationNotificationUpdatesHighlightUntypedSetting") {
    assertErrorDiagnostics(initializeLSP(), {});

    // Create a new file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\n"
                                                    "class A\n"
                                                    "def foo; end\n"
                                                    "end\n"
                                                    "\n")),
                           {});
    auto settings = make_unique<SorbetInitializationOptions>();
    settings->highlightUntyped = true;
    auto config = make_unique<DidChangeConfigurationParams>(move(settings));
    send(*makeConfigurationChange(move(config)));

    assertUntypedDiagnostics(send(*changeFile("foo.rb",
                                              "# typed: true\n"
                                              "class A\n"
                                              "  def foo(x)\n;"
                                              "    x.foo\n"
                                              "    5\n"
                                              "  end\n"
                                              "end\n"
                                              "\n",
                                              2)),
                             {
                                 {"foo.rb", 3, "Call to method `foo` on `T.untyped`"},
                             });
}

} // namespace sorbet::test::lsp
