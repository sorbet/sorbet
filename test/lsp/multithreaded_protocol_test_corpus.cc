#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "ProtocolTest.h"
#include "absl/strings/match.h"
#include "common/common.h"
#include "test/helpers/lsp.h"

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

optional<SorbetTypecheckRunStatus> getTypecheckRunStatus(const LSPMessage &msg) {
    if (msg.isNotification() && msg.method() == LSPMethod::SorbetTypecheckRunInfo) {
        auto &typecheckInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(msg.asNotification().params);
        return typecheckInfo->status;
    }
    return nullopt;
}

TEST_P(ProtocolTest, MultithreadedWrapperWorks) {
    assertDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo2\n  def branch\n    2 + \"dog\"\n  end\nend\n", 2));
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"bear\"\n  end\nend\n", 3));
    assertDiagnostics(send(move(requests)), {{"yolo1.rb", 3, "bear"}});
}

TEST_P(ProtocolTest, CancelsSlowPathWhenNewEditWouldTakeFastPath) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Create three files.
    assertDiagnostics(send(*openFile("foo.rb", "# typed: true\n\nclass Foo\n\nend\n")), {});
    assertDiagnostics(
        send(*openFile(
            "bar.rb",
            "# typed: true\n\nclass Bar\nextend T::Sig\n\nsig{returns(String)}\ndef hello\n\"hi\"\nend\nend\n")),
        {});
    // baz calls the method defined in bar
    assertDiagnostics(send(*openFile("baz.rb", "# typed: true\n\nclass Baz\nextend "
                                               "T::Sig\n\nsig{returns(String)}\ndef hello\nBar.new.hello\nend\nend\n")),
                      {});

    // Slow path edits two files. One introduces error.
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Syntax error in foo.rb.
    sendAsync(*changeFile("foo.rb", "# typed: true\n\nclass Foo\ndef noend\nend\n", 2, true));
    // Typechecking error in bar.rb
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\n\nclass Bar\nextend T::Sig\n\nsig{returns(Integer)}\ndef hello\n\"hi\"\nend\nend\n",
        2, true));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));
    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Make another edit that fixes syntax error and should take fast path.
    sendAsync(*changeFile("foo.rb", "# typed: true\n\nclass Foo\nend\n", 2, false));

    // Wait for first typecheck run to get canceled.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Cancelled);
    }

    // Send a no-op to clear out the pipeline. Should have errors in bar and baz, but not foo.
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                      {
                          {"bar.rb", 7, "Returning value that does not conform to method result type"},
                          {"baz.rb", 7, "Returning value that does not conform to method result type"},
                      });
}

TEST_P(ProtocolTest, CancelsSlowPathWhenNewEditWouldTakeSlowPath) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Initial state: Two empty files.
    assertDiagnostics(send(*openFile("foo.rb", "")), {});
    assertDiagnostics(send(*openFile("bar.rb", "")), {});

    // Slow path 1: Edit foo to have an error since Bar doesn't exist. Expect a cancelation.
    sendAsync(*changeFile(
        "foo.rb", "# typed: true\n\nclass Foo\nextend T::Sig\nsig{returns(Integer)}\ndef foo\nBar.new.bar\nend\nend\n",
        2, true));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Slow path 2: Bar defines the expected method, but declared with a non-integer return value (so foo now has a new
    // error).
    sendAsync(*changeFile("bar.rb",
                          "# typed: true\n\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef bar\n10\nend\nend\n", 2,
                          false));

    // Wait for first typecheck run to get canceled.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Cancelled);
    }

    // Send a no-op to clear out the pipeline. Should have one error per file.
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                      {
                          {"foo.rb", 6, "Returning value that does not conform to method result type"},
                          {"bar.rb", 6, "Returning value that does not conform to method result type"},
                      });
}

// Run these tests in multi-threaded mode.
INSTANTIATE_TEST_SUITE_P(MultithreadedProtocolTests, ProtocolTest, testing::Values(true));

} // namespace sorbet::test::lsp
