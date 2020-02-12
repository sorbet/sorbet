#include "gtest/gtest.h"
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
TEST_P(ProtocolTest, AddFile) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(send(*openFile("yolo1.rb", "")), {});

    ExpectedDiagnostic yolo1Diagnostic = {"yolo1.rb", 3, "Expected `Integer`"};
    assertDiagnostics(
        send(*changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 2)),
        {yolo1Diagnostic});
    assertDiagnostics(send(*openFile("yolo2.rb", "")), {yolo1Diagnostic});

    ExpectedDiagnostic yolo2Diagnostic = {"yolo2.rb", 4, "Expected `Integer`"};
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
TEST_P(ProtocolTest, AddFileJoiningRequests) {
    assertDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(openFile("yolo1.rb", "# typed: true\nclass Foo2\n  def branch\n    2 + \"dog\"\n  end\nend\n"));
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"bear\"\n  end\nend\n", 3));
    assertDiagnostics(send(move(requests)), {{"yolo1.rb", 3, "bear"}});
}

// Cancels requests before they are processed, and ensures that they are actually not processed.
TEST_P(ProtocolTest, Cancellation) {
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
        ASSERT_NO_FATAL_FAILURE(assertResponseError(-32800, "cancel", *errorMsg));
    }
    ASSERT_EQ(requestIds.size(), 0);
}

// Asserts that Sorbet returns an empty result when requesting definitions in untyped Ruby files.
TEST_P(ProtocolTest, DefinitionError) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(send(*openFile("foobar.rb", "class Foobar\n  def bar\n    1\n  end\nend\n\nbar\n")), {});
    auto defResponses = send(*getDefinition("foobar.rb", 6, 1));
    ASSERT_EQ(defResponses.size(), 1) << "Expected a single response to a definition request to an untyped document.";

    ASSERT_NO_FATAL_FAILURE(assertResponseMessage(nextId - 1, *defResponses.at(0)));

    auto &respMsg = defResponses.at(0)->asResponse();
    ASSERT_TRUE(respMsg.result);
    auto &result = get<variant<JSONNullObject, vector<unique_ptr<Location>>>>(*(respMsg.result));
    auto &array = get<vector<unique_ptr<Location>>>(result);
    ASSERT_EQ(array.size(), 0);
}

// Ensures that Sorbet merges didChanges that are interspersed with canceled requests.
TEST_P(ProtocolTest, MergeDidChangeAfterCancellation) {
    assertDiagnostics(initializeLSP(), {});
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
            ASSERT_NO_FATAL_FAILURE(assertResponseError(-32800, "cancel", *msg));
            cancelRequestCount++;
        } else if (msg->isNotification() && msg->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            diagnosticCount++;
        } else {
            ADD_FAILURE() << fmt::format("Unexpected response:\n{}", msg->toJSON());
        }
    }
    assertDiagnostics({}, {{"foo.rb", 7, "Method `blah` does not exist"}});
    EXPECT_EQ(cancelRequestCount, 3) << "Expected three cancellation messages.";
    EXPECT_EQ(diagnosticCount, 1) << "Expected a diagnostic error for foo.rb";
}

// Applies all consecutive file changes at once.
TEST_P(ProtocolTest, MergesDidChangesAcrossFiles) {
    assertDiagnostics(initializeLSP(), {});
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
    EXPECT_EQ(msgs.size(), 4) << "Expected only 4 diagnostic responses to the merged file changes";
    assertDiagnostics(move(msgs), {{"bar.rb", 3, "Expected `Integer`"},
                                   {"baz.rb", 3, "Expected `Integer`"},
                                   {"bat.rb", 3, "Expected `Integer`"},
                                   {"foo.rb", 7, "Method `blah` does not exist"}});
}

TEST_P(ProtocolTest, MergesDidChangesAcrossDelayableRequests) {
    assertDiagnostics(initializeLSP(), {});
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
    ASSERT_GT(msgs.size(), 0);
    EXPECT_TRUE(msgs.at(0)->isNotification());
    EXPECT_EQ(msgs.at(0)->method(), LSPMethod::TextDocumentPublishDiagnostics);
    assertDiagnostics({}, {{"foo.rb", 7, "Method `blah` does not exist"}});

    ASSERT_EQ(msgs.size(), 3) << "Expected a diagnostic error, followed by two document symbol responses.";
    EXPECT_TRUE(msgs.at(1)->isResponse());
    EXPECT_TRUE(msgs.at(2)->isResponse());
}

TEST_P(ProtocolTest, DoesNotMergeFileChangesAcrossNonDelayableRequests) {
    assertDiagnostics(initializeLSP(), {});
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
    // [diagnostics, documentsymbol, diagnostics]
    EXPECT_EQ(msgs.size(), 3);
    if (auto diagnosticParams = getPublishDiagnosticParams(msgs.at(0)->asNotification())) {
        EXPECT_TRUE((*diagnosticParams)->uri.find("foo.rb") != string::npos);
        auto &diagnostics = (*diagnosticParams)->diagnostics;
        EXPECT_EQ(diagnostics.size(), 1);
        EXPECT_TRUE(diagnostics.at(0)->message.find("Returning value") != string::npos);
    }
    EXPECT_TRUE(msgs.at(1)->isResponse());
    if (auto diagnosticParams = getPublishDiagnosticParams(msgs.at(2)->asNotification())) {
        EXPECT_TRUE((*diagnosticParams)->uri.find("foo.rb") != string::npos);
        auto &diagnostics = (*diagnosticParams)->diagnostics;
        EXPECT_EQ(diagnostics.size(), 1);
        EXPECT_TRUE(diagnostics.at(0)->message.find("Method `blah` does not exist") != string::npos);
    }
}

TEST_P(ProtocolTest, NotInitialized) {
    // Don't use `getDefinition`; it only works post-initialization.
    auto msgs = send(*makeDefinitionRequest(nextId++, "foo.rb", 12, 24));
    ASSERT_EQ(msgs.size(), 1);
    auto &msg1 = msgs.at(0);
    ASSERT_NO_FATAL_FAILURE(assertResponseError(-32002, "not initialize", *msg1));
}

// There's a different code path that checks for workspace edits before initialization occurs.
TEST_P(ProtocolTest, WorkspaceEditIgnoredWhenNotInitialized) {
    // Purposefully send a vector of requests to trigger merging, which should turn this into a WorkspaceEdit.
    vector<unique_ptr<LSPMessage>> toSend;
    // Avoid using `openFile`, as it only works post-initialization.
    toSend.push_back(makeOpen("bar.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n", 1));
    // This update should be ignored.
    assertDiagnostics(send(move(toSend)), {});
    // We shouldn't have any code errors post-initialization since the previous edit was ignored.
    assertDiagnostics(initializeLSP(), {});
}

TEST_P(ProtocolTest, InitializeAndShutdown) {
    assertDiagnostics(initializeLSP(), {});
    auto resp = send(LSPMessage(make_unique<RequestMessage>("2.0", nextId++, LSPMethod::Shutdown, JSONNullObject())));
    ASSERT_EQ(resp.size(), 1) << "Expected a single response to shutdown request.";
    auto &r = resp.at(0)->asResponse();
    ASSERT_EQ(r.requestMethod, LSPMethod::Shutdown);
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::Exit, JSONNullObject()))), {});
}

// Some clients send an empty string for the root uri.
TEST_P(ProtocolTest, EmptyRootUriInitialization) {
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
    ASSERT_NO_FATAL_FAILURE(assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *msg));
    // Will fail test if this does not parse.
    if (auto diagnosticParams = getPublishDiagnosticParams(msg->asNotification())) {
        EXPECT_EQ((*diagnosticParams)->uri, "memory://yolo1.rb");
    }
}

// Monaco sends null for the root URI.
TEST_P(ProtocolTest, MonacoInitialization) {
    // Null is functionally equivalent to an empty rootUri. Manually reset rootUri before initializing.
    rootUri = "";
    const bool supportsMarkdown = true;
    auto params = make_unique<RequestMessage>(
        "2.0", nextId++, LSPMethod::Initialize,
        makeInitializeParams(JSONNullObject(), JSONNullObject(), supportsMarkdown, nullopt));
    auto responses = send(LSPMessage(move(params)));
    ASSERT_EQ(responses.size(), 1) << "Expected only a single response to the initialize request.";
    auto &respMsg = responses.at(0);
    EXPECT_TRUE(respMsg->isResponse());
    auto &resp = respMsg->asResponse();
    EXPECT_EQ(resp.requestMethod, LSPMethod::Initialize);
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::Initialized,
                                                                       make_unique<InitializedParams>()))),
                      {});

    // Manually construct an openFile with text that has a typechecking error.
    auto didOpenParams = make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(
        "memory://yolo1.rb", "ruby", 1, "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"));
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(didOpenParams));
    auto diagnostics = send(LSPMessage(move(didOpenNotif)));

    // Check the response for the expected URI.
    EXPECT_EQ(diagnostics.size(), 1);
    auto &msg = diagnostics.at(0);
    ASSERT_NO_FATAL_FAILURE(assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *msg));
    // Will fail test if this does not parse.
    if (auto diagnosticParams = getPublishDiagnosticParams(msg->asNotification())) {
        EXPECT_EQ((*diagnosticParams)->uri, "memory://yolo1.rb");
    }
}

TEST_P(ProtocolTest, CompletionOnNonClass) {
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

// Ensures that unrecognized notifications are ignored.
TEST_P(ProtocolTest, IgnoresUnrecognizedNotifications) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(sendRaw("{\"jsonrpc\":\"2.0\",\"method\":\"workspace/"
                              "didChangeConfiguration\",\"params\":{\"settings\":{\"ruby-typer\":{}}}}"),
                      {});
}

// Ensures that notifications that have an improper params shape are handled gracefully / not responded to.
TEST_P(ProtocolTest, IgnoresNotificationsThatDontTypecheck) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(sendRaw(R"({"jsonrpc":"2.0","method":"textDocument/didChange","params":{}})"), {});
}

// Ensures that unrecognized requests are responded to.
TEST_P(ProtocolTest, RejectsUnrecognizedRequests) {
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
TEST_P(ProtocolTest, RejectsRequestsThatDontTypecheck) {
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
TEST_P(ProtocolTest, SilentlyIgnoresInvalidJSONMessages) {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(sendRaw("{"), {});
}

// If a client doesn't support markdown, send hover as plaintext.
TEST_P(ProtocolTest, RespectsHoverTextLimitations) {
    assertDiagnostics(initializeLSP(false /* supportsMarkdown */), {});

    assertDiagnostics(send(*openFile("foobar.rb", "# typed: true\n1\n")), {});

    auto hoverResponses = send(LSPMessage(make_unique<RequestMessage>(
        "2.0", nextId++, LSPMethod::TextDocumentHover,
        make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(getUri("foobar.rb")),
                                                make_unique<Position>(1, 0)))));
    ASSERT_EQ(hoverResponses.size(), 1);
    auto &hoverResponse = hoverResponses.at(0);
    ASSERT_TRUE(hoverResponse->isResponse());
    auto &hoverResult = get<variant<JSONNullObject, unique_ptr<Hover>>>(*hoverResponse->asResponse().result);
    auto &hover = get<unique_ptr<Hover>>(hoverResult);
    auto &contents = hover->contents;
    ASSERT_EQ(contents->kind, MarkupKind::Plaintext);
    ASSERT_EQ(contents->value, "Integer(1)");
}

// Tests that Sorbet returns sorbet: URIs for payload references & files not on client, and that readFile works on them.
TEST_P(ProtocolTest, SorbetURIsWork) {
    const bool supportsMarkdown = false;
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->supportsSorbetURIs = true;
    lspWrapper->opts->lspDirsMissingFromClient.emplace_back("/folder");
    assertDiagnostics(initializeLSP(supportsMarkdown, move(initOptions)), {});

    string fileContents = "# typed: true\n[0,1,2,3].select {|x| x > 0}\ndef myMethod; end;\n";
    assertDiagnostics(send(*openFile("folder/foo.rb", fileContents)), {});

    auto selectDefinitions = getDefinitions("folder/foo.rb", 1, 11);
    ASSERT_EQ(selectDefinitions.size(), 1);
    auto &selectLoc = selectDefinitions.at(0);
    ASSERT_TRUE(absl::StartsWith(selectLoc->uri, "sorbet:https://github.com/"));
    ASSERT_FALSE(readFile(selectLoc->uri).empty());

    auto myMethodDefinitions = getDefinitions("folder/foo.rb", 2, 5);
    ASSERT_EQ(myMethodDefinitions.size(), 1);
    auto &myMethodDefLoc = myMethodDefinitions.at(0);
    ASSERT_EQ(myMethodDefLoc->uri, "sorbet:folder/foo.rb");
    ASSERT_EQ(readFile(myMethodDefLoc->uri), fileContents);

    // VS Code replaces : in https with something URL-escaped; test that we handle this use-case.
    auto arrayRBI = readFile(selectLoc->uri);
    auto arrayRBIURLEncodeColon =
        readFile(absl::StrReplaceAll(selectLoc->uri, {{"https://github.com/", "https%3A//github.com/"}}));
    ASSERT_EQ(arrayRBI, arrayRBIURLEncodeColon);
}

// Tests that Sorbet URIs are not typechecked.
TEST_P(ProtocolTest, DoesNotTypecheckSorbetURIs) {
    const bool supportsMarkdown = false;
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->supportsSorbetURIs = true;
    initOptions->enableTypecheckInfo = true;
    lspWrapper->opts->lspDirsMissingFromClient.emplace_back("/folder");
    // Don't assert diagnostics; it will fail due to the spurious typecheckinfo message.
    initializeLSP(supportsMarkdown, move(initOptions));

    string fileContents = "# typed: true\n[0,1,2,3].select {|x| x > 0}\ndef myMethod; end;\n";
    send(*openFile("folder/foo.rb", fileContents));

    auto selectDefinitions = getDefinitions("folder/foo.rb", 1, 11);
    ASSERT_EQ(selectDefinitions.size(), 1);
    auto &selectLoc = selectDefinitions.at(0);
    ASSERT_TRUE(absl::StartsWith(selectLoc->uri, "sorbet:https://github.com/"));
    auto contents = readFile(selectLoc->uri);

    // Test that opening and closing one of these files doesn't cause a slow path.
    vector<unique_ptr<LSPMessage>> openClose;
    openClose.push_back(makeOpen(selectLoc->uri, contents, 1));
    openClose.push_back(makeClose(selectLoc->uri));
    auto responses = send(move(openClose));
    ASSERT_EQ(0, responses.size());
}

// Tests that Sorbet does not crash when a file URI falls outside of the workspace.
TEST_P(ProtocolTest, DoesNotCrashOnNonWorkspaceURIs) {
    const bool supportsMarkdown = false;
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->supportsSorbetURIs = true;

    // Manually invoke to customize rootURI and rootPath.
    auto initializeResponses = sorbet::test::initializeLSP(
        "/Users/jvilk/stripe/areallybigfoldername", "file://Users/jvilk/stripe/areallybigfoldername", *lspWrapper,
        nextId, supportsMarkdown, make_optional(move(initOptions)));

    auto fileUri = "file:///Users/jvilk/Desktop/test.rb";
    auto didOpenParams =
        make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(fileUri, "ruby", 1, "# typed: true\n1\n"));
    auto didOpenNotif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(didOpenParams));
    getLSPResponsesFor(*lspWrapper, make_unique<LSPMessage>(move(didOpenNotif)));
}

// Run these tests in single-threaded mode.
INSTANTIATE_TEST_SUITE_P(SingleThreadedProtocolTests, ProtocolTest, testing::Values(false));

} // namespace sorbet::test::lsp
