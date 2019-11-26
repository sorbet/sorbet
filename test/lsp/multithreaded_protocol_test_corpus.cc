#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "ProtocolTest.h"
#include "absl/strings/match.h"
#include "common/common.h"
#include "test/helpers/lsp.h"

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

namespace {
void assertTypecheckRunInfo(const unique_ptr<LSPMessage> &msg, SorbetTypecheckRunStatus status, bool fastPath) {
    ASSERT_NE(nullptr, msg);
    ASSERT_EQ(true, msg->isNotification());
    ASSERT_EQ(LSPMethod::SorbetTypecheckRunInfo, msg->method());

    auto &info = get<unique_ptr<SorbetTypecheckRunInfo>>(msg->asNotification().params);
    ASSERT_EQ(status, info->status);
    ASSERT_EQ(fastPath, info->isFastPath);
}

void getMessagesDuringTypecheck(ProtocolTest &test, vector<unique_ptr<LSPMessage>> &diagnostics, bool fastPath,
                                SorbetTypecheckRunStatus finalStatus = SorbetTypecheckRunStatus::ended,
                                bool typecheckRunAlreadyStarted = false) {
    if (!typecheckRunAlreadyStarted) {
        ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(test.readAsync(), SorbetTypecheckRunStatus::started, fastPath));
    }

    // Keep reading until fast path ends.
    while (true) {
        auto msg = test.readAsync();
        ASSERT_NE(nullptr, msg);
        if (msg->isNotification() && msg->method() == LSPMethod::SorbetTypecheckRunInfo) {
            ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(msg, finalStatus, fastPath));
            break;
        }
        diagnostics.push_back(move(msg));
    }
}
} // namespace

TEST_P(ProtocolTest, MultithreadedWrapperWorks) {
    assertDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo2\n  def branch\n    2 + \"dog\"\n  end\nend\n", 2));
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"bear\"\n  end\nend\n", 3));
    assertDiagnostics(send(move(requests)), {{"yolo1.rb", 3, "bear"}});
}

TEST_P(ProtocolTest, CancelingSlowPathForFastPathUpdatesDiagnostics) {
    auto opts = make_unique<SorbetInitializationOptions>();
    opts->enableTypecheckInfo = true;
    initializeLSP(true, move(opts));
    // New file; should take slow path. It will have an error.
    send(*openFile("foo.rb",
                   "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  10\nend\nend"));
    assertDiagnostics({}, {{"foo.rb", 5, "Returning value that does not conform to method result type"}});

    // Send an update that should take slow path, and that will wait for a preemption to happen.
    // The following happens asynchronously.
    sendAsync(*changeFile(
        "foo.rb", "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  10\nend\ndef me\nend", 2,
        true /* expected cancelation */));

    // Wait for update to start.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::started, false));

    // Send an update that should take fast path (corrects syntax error).
    sendAsync(*changeFile(
        "foo.rb", "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  '10'\nend\nend", 3));

    // Ensure that cancelation occurs.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::canceled, false));

    // Should receive a diagnostic to clear earlier error.
    vector<unique_ptr<LSPMessage>> diagnostics;
    ASSERT_NO_FATAL_FAILURE(getMessagesDuringTypecheck(*this, diagnostics, true));
    assertDiagnostics(move(diagnostics), {});
}

TEST_P(ProtocolTest, CancelingSlowPathForSlowPathUpdatesDiagnostics) {
    auto opts = make_unique<SorbetInitializationOptions>();
    opts->enableTypecheckInfo = true;
    initializeLSP(true, move(opts));
    // New file; should take slow path. It will have an error.
    send(*openFile("foo.rb",
                   "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  10\nend\nend"));
    assertDiagnostics({}, {{"foo.rb", 5, "Returning value that does not conform to method result type"}});

    // Send an update that should take slow path, and that will wait for a preemption to happen.
    // The following happens asynchronously.
    sendAsync(*changeFile(
        "foo.rb", "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  10\nend\ndef me\nend", 2,
        true /* expected cancelation */));

    // Wait for update to start.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::started, false));

    // Send an update that should take slow path.
    sendAsync(*changeFile("foo.rb",
                          "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  '10'\nend\ndef "
                          "new_method\nend\nend",
                          3));

    // Ensure that cancelation occurs.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::canceled, false));

    // Should receive a diagnostic to clear earlier error.
    vector<unique_ptr<LSPMessage>> diagnostics;
    ASSERT_NO_FATAL_FAILURE(getMessagesDuringTypecheck(*this, diagnostics, false));
    assertDiagnostics(move(diagnostics), {});
}

TEST_P(ProtocolTest, CanPreemptSlowPathWithFastPath) {
    auto opts = make_unique<SorbetInitializationOptions>();
    opts->enableTypecheckInfo = true;
    initializeLSP(true, move(opts));
    // New file; should take slow path. It will not have an error.
    send(*openFile("foo.rb",
                   "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  '10'\nend\nend"));
    assertDiagnostics({}, {});

    // Take slow path, but expect 1 preemption. Introduces a typechecking error in return value of `hello`.
    sendAsync(*changeFile(
        "foo.rb",
        "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  10\nend\ndef hello2\nend\nend", 2,
        0 /* expected cancelation */, 1 /* Expected preemptions */));

    // Wait for update to start.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::started, false));

    // Fast path: Introduces new error to file. Ensures that files that are typechecked during preemption trump
    // typechecker failures during slow path.
    sendAsync(*changeFile("foo.rb",
                          "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  hello3\n  "
                          "10\nend\ndef hello2\nend\nend",
                          2, 0 /* expected cancelation */, 1 /* Expected preemptions */));

    vector<unique_ptr<LSPMessage>> diagnostics;
    ASSERT_NO_FATAL_FAILURE(getMessagesDuringTypecheck(*this, diagnostics, true));
    ASSERT_NO_FATAL_FAILURE(getMessagesDuringTypecheck(*this, diagnostics, false, SorbetTypecheckRunStatus::ended,
                                                       true /* typecheck run already started */));

    assertDiagnostics(move(diagnostics),
                      {{"foo.rb", 5, "Method `hello3` does not exist"},
                       {"foo.rb", 6, "Returning value that does not conform to method result type"}});
}

TEST_P(ProtocolTest, CanPreemptSlowPathWithFastPathThatFixesSecondOrderErrors) {
    auto opts = make_unique<SorbetInitializationOptions>();
    opts->enableTypecheckInfo = true;
    initializeLSP(true, move(opts));
    // New files; should take slow path. It will not have an error.
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(openFile(
        "foo.rb", "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  '10'\nend\nend"));
    messages.push_back(openFile(
        "bar.rb",
        "#typed: true\nclass Bar\nextend T::Sig\nsig {returns(String)}\ndef hello\n  Foo.new.hello\nend\nend"));
    send(move(messages));
    assertDiagnostics({}, {});

    // Take slow path, but expect 1 preemption. Introduces a typechecking error in return value of `hello`, and change
    // return type of `hello` to break `bar.rb`.
    sendAsync(*changeFile(
        "foo.rb",
        "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(Integer)}\ndef hello\n  '10'\nend\ndef hello2\nend\nend",
        2, 0 /* expected cancelation */, 1 /* Expected preemptions */));

    // Wait for update to start.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::started, false));

    // Fast path: Fix both errors, introduce new error.
    sendAsync(*changeFile("foo.rb",
                          "#typed: true\nclass Foo\nextend T::Sig\nsig {returns(String)}\ndef hello\n  hello3\n  "
                          "10\nend\ndef hello2\nend\nend",
                          2, 0 /* expected cancelation */, 1 /* Expected preemptions */));

    vector<unique_ptr<LSPMessage>> diagnostics;
    ASSERT_NO_FATAL_FAILURE(getMessagesDuringTypecheck(*this, diagnostics, true));
    ASSERT_NO_FATAL_FAILURE(getMessagesDuringTypecheck(*this, diagnostics, false, SorbetTypecheckRunStatus::ended,
                                                       true /* typecheck run already started */));

    assertDiagnostics(move(diagnostics),
                      {{"foo.rb", 5, "Method `hello3` does not exist"},
                       {"foo.rb", 6, "Returning value that does not conform to method result type"}});
}

// Tests:

// Preemption typecheck
// - Slow path modified a file that causes an error in two files.
// - Fast path preempts and fixes both errors (i.e. fix is in file not explicitly changed)
// Final state: Both errors are resolved.

// Symbols exist during preemption
// - Slow path adds a new file with new symbols.
// - Go to definition runs on new file and should work.
// - Bonus: Making 3 go-to-definition requests -- all three preempt despite queueing.

// Preemption followed by cancelation.
// - Slow path modified a file that causes an error.
// - Fast path preempts and adds a new error to same file (possible?).
// - Slow path gets canceled for a fast path with a third error.
// Final state: Errors from fast path are the final ones reported.

// Preemption clear errors in unedited files.
// - Slow path modifies `foo.rb`, introduces error in `bar.rb`.
// - Preempted fast path modifies `fez.rb`, causes `bar.rb` to be re-typechecked with two errors.
// - Fast path cancels slow path that fixes `foo.rb` and undoes change to `fez.rb` which fixes one error in `bar.rb`.
// ---> Important that `bar.rb` gets retypechecked not because of `fez` but because it had error before.

// Preemption re-introduces error in unedited file.
// - Init: `bar.rb` has error due to `fez.rb`. `foo.rb` has a single method in it.
// - Slow path modifies `foo.rb`.
// - Fast path modifies `fez.rb`, fixes error in `bar.rb`.
// - Fast path cancels slow path that re-introduces error in `fez.rb`.
// ---> Important that `bar.rb` gets retypechecked not because of `fez` but because it had error before.

// Unit tests?

// Cancelation does not affect fast path.
// Cancelation edits can merge.
// Cannot preempt resolver.

// Later:

// - Find all references stalls pipeline.

// TODO: Could we take fast path but the hashes on a new file disagree?

// Run these tests in multi-threaded mode.
INSTANTIATE_TEST_SUITE_P(MultithreadedProtocolTests, ProtocolTest, testing::Values(true));

} // namespace sorbet::test::lsp
