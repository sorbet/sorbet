#include "doctest/doctest.h"
// ^ Violates linting rules, so include first.
#include "absl/strings/match.h"
#include "common/common.h"
#include "common/sort/sort.h"
#include "test/helpers/lsp.h"
#include "test/lsp/ProtocolTest.h"

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

namespace {

optional<unique_ptr<SorbetTypecheckRunInfo>> getTypecheckRunInfo(LSPMessage &msg) {
    if (msg.isNotification() && msg.method() == LSPMethod::SorbetTypecheckRunInfo) {
        unique_ptr<SorbetTypecheckRunInfo> &typecheckInfo =
            get<unique_ptr<SorbetTypecheckRunInfo>>(msg.asNotification().params);
        return move(typecheckInfo);
    }
    return nullopt;
}

optional<SorbetTypecheckRunStatus> getTypecheckRunStatus(const LSPMessage &msg) {
    if (msg.isNotification() && msg.method() == LSPMethod::SorbetTypecheckRunInfo) {
        auto &typecheckInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(msg.asNotification().params);
        return typecheckInfo->status;
    }
    return nullopt;
}

void sortTimersByStartTime(vector<unique_ptr<CounterImpl::Timing>> &times) {
    fast_sort(times, [](const auto &a, const auto &b) -> bool { return a->start.usec < b->start.usec; });
}

void checkDiagnosticTimes(vector<unique_ptr<CounterImpl::Timing>> times, size_t expectedSize,
                          bool assertUniqueStartTimes) {
    CHECK_EQ(times.size(), expectedSize);
    sortTimersByStartTime(times);

    if (assertUniqueStartTimes) {
        // No two diagnostic latency timers should have the same start timestamp.
        for (size_t i = 1; i < times.size(); i++) {
            CHECK_LT(times[i - 1]->start.usec, times[i]->start.usec);
        }
    }
}

// The resolution of MONOTONIC_COARSE seems to be ~1ms, so we use >1ms delay to ensure unique clock values.
constexpr auto timestampGranularity = chrono::milliseconds(2);

class MultithreadedProtocolTest : public ProtocolTest {
public:
    MultithreadedProtocolTest() : ProtocolTest(/*multithreading*/ true, /*caching*/ false) {}
};

} // namespace

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "MultithreadedWrapperWorks") {
    assertErrorDiagnostics(initializeLSP(), {});
    {
        auto initCounters = getCounters();
        CHECK_EQ(initCounters.getCategoryCounter("lsp.messages.processed", "initialize"), 1);
        CHECK_EQ(initCounters.getCategoryCounter("lsp.messages.processed", "initialized"), 1);
        CHECK_EQ(initCounters.getCategoryCounter("lsp.updates", "slowpath"), 1);
        CHECK_EQ(initCounters.getCategoryCounterSum("lsp.updates"), 1);
        CHECK_EQ(initCounters.getTimings("initial_index").size(), 1);
        CHECK_EQ(initCounters.getCategoryCounterSum("lsp.messages.canceled"), 0);
    }

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*openFile("yolo1.rb", "# typed: true\nclass Foo2\n  def branch\n    2 + \"dog\"\n  end\nend\n"));
    // Pause to differentiate message times.
    this_thread::sleep_for(timestampGranularity);
    sendAsync(*changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"bear\"\n  end\nend\n", 3));

    // Pause so that all latency timers for the above operations get reported.
    this_thread::sleep_for(chrono::milliseconds(2));

    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt))),
                           {{"yolo1.rb", 3, "bear"}});

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 1);
    CHECK_EQ(counters.getTimings("task_latency", {{"method", "sorbet.workspaceEdit"}}).size(), 1);
    CHECK_EQ(counters.getTimings("task_latency").size(), counters.getHistogramCount("task_latency"));
    CHECK_EQ(counters.getCategoryCounterSum("lsp.messages.canceled"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounterSum("lsp.updates"), 1);
    // 1 per edit
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 2, /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CancelsSlowPathWhenNewEditWouldTakeFastPathWithOldEdits") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create three files.
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\n"
                                                    "\n"
                                                    "class Foo\n"
                                                    "\n"
                                                    "end\n")),
                           {});
    assertErrorDiagnostics(send(*openFile("bar.rb", "# typed: true\n"
                                                    "\n"
                                                    "class Bar\n"
                                                    "extend T::Sig\n"
                                                    "\n"
                                                    "sig{returns(String)}\n"
                                                    "def hello\n"
                                                    "\"hi\"\n"
                                                    "end\n"
                                                    "end\n")),
                           {});
    // baz calls the method defined in bar
    assertErrorDiagnostics(send(*openFile("baz.rb", "# typed: true\n"
                                                    "\n"
                                                    "class Baz\n"
                                                    "extend T::Sig\n"
                                                    "\n"
                                                    "sig{returns(String)}\n"
                                                    "def hello\n"
                                                    "Bar.new.hello\n"
                                                    "end\n"
                                                    "end\n")),
                           {});

    // clear counters
    getCounters();

    // Slow path edits two files. One introduces error.
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Something that will introduce slow path in foo.rb
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "\n"
                          "class Foo\n"
                          "  module Inner; end\n"
                          "end\n",
                          2, true));
    // Pause to differentiate message times
    this_thread::sleep_for(chrono::milliseconds(2));
    // Typechecking error in bar.rb
    sendAsync(*changeFile("bar.rb",
                          "# typed: true\n"
                          "\n"
                          "class Bar\n"
                          "extend T::Sig\n"
                          "\n"
                          "sig{returns(Integer)}\n" // String -> Integer
                          "def hello\n"
                          "\"hi\"\n"
                          "end\n"
                          "end\n",
                          2, true));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));
    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Pause for a moderate amount of time so that we can check that the subsequent fast path includes the latency
    // timer from the initial slow path.
    this_thread::sleep_for(chrono::milliseconds(5));

    // Make another edit that undoes the slow path in foo.rb
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "\n"
                          "class Foo\n"
                          "end\n",
                          2, false));

    // Wait for first typecheck run to get canceled.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Cancelled);
    }

    // Send a no-op to clear out the pipeline. Should have errors in bar and baz, but not foo.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                           {
                               {"bar.rb", 7, "Expected `Integer` but found `String(\"hi\")` for method result type"},
                               {"baz.rb", 7, "Expected `String` but found `Integer` for method result type"},
                           });

    auto counters = getCounters();
    // N.B.: lsp.messages.processed contains canceled slow paths.
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 2);
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 2);

    // We don't report task latencies for merged edits or canceled slow paths.
    auto taskLatency = counters.getTimings("task_latency", {{"method", "sorbet.workspaceEdit"}});
    CHECK_EQ(taskLatency.size(), 1);
    CHECK_GE(taskLatency[0]->end.usec - taskLatency[0]->start.usec, 5'000 /* 5ms */);
    CHECK_EQ(counters.getCategoryCounterSum("lsp.messages.canceled"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "query"), 0);

    auto lastDiagnosticLatency = counters.getTimings("last_diagnostic_latency");
    sortTimersByStartTime(lastDiagnosticLatency);
    CHECK_GE(lastDiagnosticLatency[0]->end.usec - lastDiagnosticLatency[0]->start.usec, 5'000 /* 5ms */);
    CHECK_GE(lastDiagnosticLatency[1]->end.usec - lastDiagnosticLatency[1]->start.usec, 5'000 /* 5ms */);
    // 1 per edit
    checkDiagnosticTimes(move(lastDiagnosticLatency), 3, /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CancelsSlowPathWhenNewEditWouldTakeSlowPath") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Initial state: Two empty files.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    assertErrorDiagnostics(send(*openFile("bar.rb", "")), {});

    // clear counters
    getCounters();

    // Slow path 1: Edit foo to have an error since Bar doesn't exist. Expect a cancelation.
    sendAsync(*changeFile(
        "foo.rb", "# typed: true\n\nclass Foo\nextend T::Sig\nsig{returns(Integer)}\ndef foo\nBar.new.bar\nend\nend\n",
        2, true));

    // Wait for typechecking to begin to avoid races.
    {
        auto message = readAsync();
        auto status = getTypecheckRunStatus(*message);
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Slow path 2: Bar defines the expected method, but declared with a non-integer return value (so foo now has a
    // new error).
    sendAsync(*changeFile("bar.rb",
                          "# typed: true\n\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef bar\n10\nend\nend\n", 2,
                          false));
    // Pause so that all latency timers for the above operations get reported.
    this_thread::sleep_for(chrono::milliseconds(2));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Wait for first typecheck run to get canceled.
    {
        auto message = readAsync();
        // Errors are reported after each phase
        // So we might receive a publishDiagnostics notification
        // The next one after should be a typechecking status
        while (message->isNotification() && message->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            message = readAsync();
        }
        auto status = getTypecheckRunStatus(*message);
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Cancelled);
    }

    // Send a no-op to clear out the pipeline. Should have one error per file.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                           {
                               {"foo.rb", 6, "Expected `Integer` but found `String` for method result type"},
                               {"bar.rb", 6, "Expected `String` but found `Integer(10)` for method result type"},
                           });

    auto counters = getCounters();
    // N.B.: lsp.messages.processed contains canceled slow paths.
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 2);
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 1);

    // We don't report task latencies for merged edits or canceled slow paths.
    CHECK_EQ(counters.getTimings("task_latency", {{"method", "sorbet.workspaceEdit"}}).size(), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "query"), 0);
    // 1 per edit
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 2, /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptSlowPathWithHover") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create a new file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});

    // clear counters
    getCounters();

    // Slow path: Edit foo to have a class with a documentation string.
    sendAsync(*changeFile("foo.rb", "# typed: true\n# A class that does things.\nclass Foo\nextend T::Sig\nend\n", 2,
                          false, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Send a hover to request the documentation string.
    sendAsync(*hover("foo.rb", 2, 6));
    // Pause so that all latency timers for the above operations get reported.

    this_thread::sleep_for(chrono::milliseconds(2));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // First response should be hover.
    {
        auto response = readAsync();
        while (response->isNotification() && response->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            response = readAsync();
        }
        REQUIRE(response->isResponse());
        auto &hoverText =
            get<unique_ptr<Hover>>(get<variant<JSONNullObject, unique_ptr<Hover>>>(*response->asResponse().result));
        CHECK(absl::StrContains(hoverText->contents->value, "A class that does things"));
    }

    // Second should be typecheck run signaling that typechecking completed.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }

    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "textDocument.hover"), 1);

    CHECK_EQ(counters.getTimings("task_latency", {{"method", "sorbet.workspaceEdit"}}).size(), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "query"), 1);
    // 1 per edit
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 1, /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptSlowPathWithCodeAction") {
    // Note:
    //
    // VS Code sends a code action request after almost every edit, trying to figure out which
    // actions are available at the new cursor position, so this case is actually abundantly common.

    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create a new file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});

    // clear counters
    getCounters();

    // Add something to the empty file to trigger a slow path
    auto preemptionsExpected = 1;
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class A\n"
                          "  def example; end\n"
                          "  \n"
                          "end\n",
                          2, false, preemptionsExpected));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Send a codeAction request at the cursor position
    sendAsync(*codeAction("foo.rb", 3, 2));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // First response should be codeAction.
    {
        auto response = readAsync();
        REQUIRE(response->isResponse());
        auto &codeActions = get<vector<unique_ptr<CodeAction>>>(
            get<variant<JSONNullObject, vector<unique_ptr<CodeAction>>>>(*response->asResponse().result));
        CHECK_EQ(codeActions.size(), 0);
    }

    // Second should be typecheck run signaling that typechecking completed.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }

    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "textDocument.codeAction"), 1);

    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "query"), 1);
    // 1 per edit
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 1, /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptSlowPathWithHoverAndReturnsErrors") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create a new file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});

    // Slow path: Edit foo to have a class with a documentation string and a method with an error.
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n# A class that does things.\nclass Foo\nextend "
                          "T::Sig\nsig{returns(String)}\ndef bar\n3\nend\nend\n",
                          2, false, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Send a hover to request the documentation string.
    sendAsync(*hover("foo.rb", 2, 6));

    // First response should be hover.
    {
        auto response = readAsync();
        REQUIRE(response->isResponse());
        auto &hoverText =
            get<unique_ptr<Hover>>(get<variant<JSONNullObject, unique_ptr<Hover>>>(*response->asResponse().result));
        CHECK(absl::StrContains(hoverText->contents->value, "A class that does things"));
    }

    // Send a no-op to clear out the pipeline. Should have one error in `foo.rb`.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                           {
                               {"foo.rb", 6, "Expected `String` but found `Integer(3)` for method result type"},
                           });
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 2,
                         /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptSlowPathWithFastPath") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create two new files.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    assertErrorDiagnostics(send(*openFile("bar.rb", "")), {});

    // Slow path: Edit foo to have a class with two methods and two errors, and add an error to bar.
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\nclass Foo\nextend T::Sig\nsig{returns(Integer)}\ndef "
                          "bar\nbaz\nend\nsig{returns(Float)}\ndef baz\n'not a float'\nend\nend\n",
                          2, false, 1));
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef branch\n1\nend\nend\n", 3));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path 1: Correct _one_ error.
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\nclass Foo\nextend T::Sig\nsig{returns(Integer)}\ndef "
                          "bar\n10\nend\nsig{returns(Float)}\ndef baz\n'not a float'\nend\nend\n",
                          3));

    // Send a no-op to clear out the pipeline. Should have two error now: bar.rb from slow path and foo.rb from fast
    // path.
    assertErrorDiagnostics(
        send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
        {
            {"foo.rb", 9, "Expected `Float` but found `String(\"not a float\")` for method result type"},
            {"bar.rb", 5, "Expected `String` but found `Integer(1)` for method result type"},
        });
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 5,
                         /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptSlowPathWithFastPathThatFixesAllErrors") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create two new files.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    assertErrorDiagnostics(send(*openFile("bar.rb", "")), {});

    // Slow path: Edit foo to have a class with an error that also causes an error in bar
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  extend T::Sig\n"
                          "  sig{returns(Integer)}\n"
                          "  def bar\n"
                          "    'hello'\n"
                          "  end\n"
                          "end\n",
                          2, false, 1));
    sendAsync(*changeFile("bar.rb",
                          "# typed: true\n"
                          "class Bar\n"
                          "  extend T::Sig\n"
                          "  sig{returns(String)}\n"
                          "  def str\n"
                          "    Foo.new.bar\n"
                          "  end\n"
                          "end\n",
                          3));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path 1: Correct return type on Foo#bar, which should fix foo.rb and bar.rb.
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  extend T::Sig\n"
                          "  sig{returns(String)}\n"
                          "  def bar\n"
                          "    'hello'\n"
                          "  end\n"
                          "end\n",
                          3));
    // Send a no-op to clear out the pipeline. Should have no errors at end of both typechecking runs.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});

    // Need to re-evaluate the test strategy for timings
    // checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 5,
    //                      /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptSlowPathWithFastPathAndThenCancelBoth") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create three new files! foo.rb defines a class, bar.rb defines a class and method used in baz.rb.
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend")), {});
    assertErrorDiagnostics(
        send(*openFile("bar.rb",
                       "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef str\n'hi'\nend\nend\n")),
        {});
    assertErrorDiagnostics(
        send(*openFile(
            "baz.rb",
            "# typed: true\nclass Baz\nextend T::Sig\nsig{returns(String)}\ndef bar\nBar.new.str\nend\nend\n")),
        {});

    // Slow path: foo.rb will have a syntax error
    sendAsync(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\n", 2, true, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path [preempt]: Change return type in bar.rb, which indirectly causes an error in baz.rb too.
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(Integer)}\ndef str\n'hi'\nend\nend\n", 4));

    // Wait for typechecking to begin to avoid races.
    {
        // Errors are reported after each phase
        // So we might receive a publishDiagnostics notification
        // The next one after should be a typechecking status
        auto message = readAsync();
        while (message->isNotification() && message->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            message = readAsync();
        }
        auto status = getTypecheckRunStatus(*message);
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path [cancel]: Fix syntax error. Foo should not have any errors.
    assertErrorDiagnostics(send(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend", 4)),
                           {
                               {"bar.rb", 5, "Expected `Integer` but found `String(\"hi\")` for method result type"},
                               {"baz.rb", 5, "Expected `String` but found `Integer` for method result type"},
                           });

    auto counters = getCounters();
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 6, /* assertUniqueStartTimes */ false);

    // N.B.: This counter contains canceled edits.
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 6);
    // mergedEdits should not count the preempted fast path, but should count the canceled slow path.
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 1);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptSlowPathWithFastPathAndBothErrorsAreReported") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create three new files! foo.rb defines a class, bar.rb defines a class and method used in baz.rb.
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend")), {});
    assertErrorDiagnostics(
        send(*openFile("bar.rb",
                       "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef str\n'hi'\nend\nend\n")),
        {});
    assertErrorDiagnostics(
        send(*openFile(
            "baz.rb",
            "# typed: true\nclass Baz\nextend T::Sig\nsig{returns(String)}\ndef bar\nBar.new.str\nend\nend\n")),
        {});

    // Slow path: foo.rb will have a syntax error
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "extend(T::Sig",
                          2, false, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto message = readAsync();
        auto status = getTypecheckRunStatus(*message);
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path [preempt]: Change return type in bar.rb, which indirectly causes an error in baz.rb too.
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(Integer)}\ndef str\n'hi'\nend\nend\n", 4));

    // We should receive errors for `foo`, `bar`, and `baz`.
    // Send a no-op to clear out the pipeline.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                           {
                               {"foo.rb", 2, "unexpected token"},
                               {"bar.rb", 5, "Expected `Integer` but found `String(\"hi\")` for method result type"},
                               {"baz.rb", 5, "Expected `String` but found `Integer` for method result type"},
                           });
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 5,
                         /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanCancelSlowPathWithFastPathThatReintroducesOldError") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // foo stands alone
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend\n")), {});
    // bar defines method
    assertErrorDiagnostics(
        send(*openFile("bar.rb",
                       "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(Integer)}\ndef str\n10\nend\nend\n")),
        {});
    // baz1 uses it
    assertErrorDiagnostics(
        send(*openFile(
            "baz1.rb",
            "# typed: true\nclass Baz\nextend T::Sig\nsig{returns(String)}\ndef bar\nBar.new.str\nend\nend\n")),
        {{"baz1.rb", 5, "Expected `String` but found `Integer` for method result type"}});

    // Slow path: Introduce syntax error to foo.rb and change method sig in bar.rb to fix error in baz1.rb
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\n", 2, true));
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\n\nclass Bar\nextend T::Sig\n\nsig{returns(String)}\ndef str\n'10'\nend\nend\n", 2,
        true));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Wait for typechecking to begin to avoid races.
    {
        auto message = readAsync();
        while (message->isNotification() && message->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            message = readAsync();
        }
        auto status = getTypecheckRunStatus(*message);
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path: Undo previous changes. Should re-introduce error on baz1.rb.
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend\n", 3));
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\n\nclass Bar\nextend T::Sig\n\nsig{returns(Integer)}\ndef str\n10\nend\nend\n", 3));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    {
        auto message = readAsync();
        while (message->isNotification() && message->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            message = readAsync();
        }
        auto info = getTypecheckRunInfo(*message);
        REQUIRE(info.has_value());
        REQUIRE_EQ((*info)->typecheckingPath, TypecheckingPath::Slow);
        REQUIRE_EQ((*info)->status, SorbetTypecheckRunStatus::Cancelled);
    }

    {
        auto message = readAsync();
        while (message->isNotification() && message->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            message = readAsync();
        }
        auto info = getTypecheckRunInfo(*message);
        REQUIRE(info.has_value());
        REQUIRE_EQ((*info)->typecheckingPath, TypecheckingPath::Fast);
        REQUIRE_EQ((*info)->status, SorbetTypecheckRunStatus::Started);
    }
    {
        auto message = readAsync();
        while (message->isNotification() && message->method() == LSPMethod::TextDocumentPublishDiagnostics) {
            message = readAsync();
        }
        auto info = getTypecheckRunInfo(*message);
        REQUIRE(info.has_value());
        REQUIRE_EQ((*info)->typecheckingPath, TypecheckingPath::Fast);
        REQUIRE_EQ((*info)->status, SorbetTypecheckRunStatus::Ended);
    }
    // Send a no-op to clear out the pipeline. Should have one error on baz1.rb.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                           {{"baz1.rb", 5, "Expected `String` but found `Integer` for method result type"}});
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 7,
                         /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanCancelSlowPathEvenIfAddsFile") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // bar has no error
    assertErrorDiagnostics(
        send(*openFile("bar.rb",
                       "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(Integer)}\ndef hi\n10\nend\nend\n")),
        {});

    // Slow path: New file, no error
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*openFile("foo.rb", ""));
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\nclass Foo\nextend T::Sig\nsig {returns(Integer)}\ndef foo\n10\nend\nend\n", 2,
                          true, 1));

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Introduce error fast path: Will preempt.
    sendAsync(*changeFile(
        "foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nsig {returns(Integer)}\ndef foo\n'hi'\nend\nend\n", 2));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Introduce slow path in unrelated file; will cancel.
    sendAsync(*openFile("bar.rb", "# typed: true\n"
                                  "class Bar\n"
                                  "extend T::Sig\n"
                                  "sig{returns(Integer)\n"
                                  "def hi\n"
                                  "10\n"
                                  "end"));

    // Send fence to clear out the pipeline.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                           {{"foo.rb", 5, "Expected `Integer` but found `String(\"hi\")` for method result type"},
                            {"bar.rb", 1, "Hint: this \"class\" token is not closed before the end of the file"},
                            {"bar.rb", 6, "unexpected"}});
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 5,
                         /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "SlowFooThenFastBarThenUndoSlowFoo") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\n"
                                                    "class Foo\n"
                                                    "end\n")),
                           {});
    assertErrorDiagnostics(send(*openFile("bar.rb", "# typed: true\n"
                                                    "class Bar\n"
                                                    "  extend T::Sig\n"
                                                    "  sig {returns(Integer)}\n"
                                                    "  def example\n"
                                                    "    1+1\n"
                                                    "  end\n"
                                                    "end\n")),
                           {});

    // Slow path in foo.rb
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class FooNew\n"
                          "end\n",
                          2, true));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Fast path edit in bar.rb preempts previous foo.rb edit
    sendAsync(*changeFile("bar.rb",
                          "# typed: true\n"
                          "class Bar\n"
                          "  extend T::Sig\n"
                          "  sig {returns(String)}\n"
                          "  def example\n"
                          "    1+1\n"
                          "  end\n"
                          "end\n",
                          2, false));
    // Undo change in foo.rb
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "end\n",
                          3));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Send a no-op to clear out the pipeline. Should have one error on baz.rb.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                           {{"bar.rb", 5, "Expected `String` but found `Integer` for method result type"}});
}
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanceledRequestsDontReportLatencyMetrics") {
    assertErrorDiagnostics(initializeLSP(), {});

    // Create a new file.
    assertErrorDiagnostics(
        send(*openFile("foo.rb", "# typed: true\n# A class that does things.\nclass Foo\nextend T::Sig\nend\n")), {});

    // clear counters
    getCounters();

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Send a hover to request the documentation string.
    sendAsync(*hover("foo.rb", 2, 6));
    // Pause so that all latency timer for the hover is >1ms and would normally get reported.
    this_thread::sleep_for(chrono::milliseconds(2));
    // Cancel the hover.
    sendAsync(*cancelRequest(nextId - 1));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));
    // Clear out the pipeline to wait for previous requests to finish.
    send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20)));

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "textDocument.hover"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.canceled", "textDocument.hover"), 1);
    CHECK_EQ(counters.getTimings("task_latency", {{"method", "textDocument.hover"}}).size(), 0);
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 0,
                         /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "ErrorIntroducedInSlowPathPreemptionByFastPathClearedByNewSlowPath") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create new file
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\n"
                                                    "module M\n"
                                                    "end\n"
                                                    "class Foo\n"
                                                    "end")),
                           {});

    // Slow path: no errors
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "module M\n"
                          "end\n"
                          "class Foo\n"
                          "  extend M\n"
                          "end",
                          2, true, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path [preempt]: Introduce an error with an edit that could take fast path
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "module M\n"
                          "end\n"
                          "class Foo\n"
                          "  extend M\n"
                          "  self.method_on_m()\n"
                          "end",
                          3));

    // Wait for typechecking of fast path so it and subsequent slow path change aren't combined
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Slow path: error will be resolved
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "module M\n"
                          "  def method_on_m; end\n"
                          "end\n"
                          "class Foo\n"
                          "  include M\n" // Force slow path
                          "  extend M\n"
                          "  self.method_on_m()\n"
                          "end",
                          4, false, 0));

    // Send a no-op to clear out the pipeline.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "StallInSlowPathWorks") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create a simple file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  def foo\n"
                          "  end\n"
                          "end\n",
                          2));

    // Wait for initial typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Wait for initial typechecking to finish.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }

    // Turn on slow path blocking, and send an edit that is going to cause a slow path.
    setSlowPathBlocked(true);
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Bar\n"
                          "  def foo\n"
                          "  end\n"
                          "end\n",
                          3, false, 0));

    // Wait for typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // We expect the slow path to be blocked, so we want to make sure that we _don't_ receive any messages within a
    // pretty long time frame---let's say 2000ms.
    {
        auto &wrapper = dynamic_cast<MultiThreadedLSPWrapper &>(*lspWrapper);
        auto msg = wrapper.read(2000);
        REQUIRE(msg == nullptr);
    }

    // Unblock the slow path.
    setSlowPathBlocked(false);

    // The slow path should now be unblocked. Wait for typechecking to end.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }
}

} // namespace sorbet::test::lsp
