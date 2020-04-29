#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "absl/strings/match.h"
#include "common/common.h"
#include "common/sort.h"
#include "test/helpers/lsp.h"
#include "test/lsp/ProtocolTest.h"

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

namespace {
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
    EXPECT_EQ(times.size(), expectedSize);
    sortTimersByStartTime(times);

    if (assertUniqueStartTimes) {
        // No two diagnostic latency timers should have the same start timestamp.
        for (size_t i = 1; i < times.size(); i++) {
            EXPECT_LT(times[i - 1]->start.usec, times[i]->start.usec);
        }
    }
}

// The resolution of MONOTONIC_COARSE seems to be ~1ms, so we use >1ms delay to ensure unique clock values.
constexpr auto timestampGranularity = chrono::milliseconds(2);

} // namespace

TEST_P(ProtocolTest, MultithreadedWrapperWorks) {
    assertDiagnostics(initializeLSP(), {});
    {
        auto initCounters = getCounters();
        EXPECT_EQ(initCounters.getCategoryCounter("lsp.messages.processed", "initialize"), 1);
        EXPECT_EQ(initCounters.getCategoryCounter("lsp.messages.processed", "initialized"), 1);
        EXPECT_EQ(initCounters.getCategoryCounter("lsp.updates", "slowpath"), 1);
        EXPECT_EQ(initCounters.getCategoryCounterSum("lsp.updates"), 1);
        EXPECT_EQ(initCounters.getTimings("initial_index").size(), 1);
        EXPECT_EQ(initCounters.getCategoryCounterSum("lsp.messages.canceled"), 0);
    }

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*openFile("yolo1.rb", "# typed: true\nclass Foo2\n  def branch\n    2 + \"dog\"\n  end\nend\n"));
    // Pause to differentiate message times.
    this_thread::sleep_for(timestampGranularity);
    sendAsync(*changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"bear\"\n  end\nend\n", 3));

    // Pause so that all latency timers for the above operations get reported.
    this_thread::sleep_for(chrono::milliseconds(2));

    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt))),
                      {{"yolo1.rb", 3, "bear"}});

    auto counters = getCounters();
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 1);
    EXPECT_EQ(counters.getTimings("task_latency", {{"method", "sorbet.workspaceEdit"}}).size(), 1);
    EXPECT_EQ(counters.getTimings("task_latency").size(), counters.getHistogramCount("task_latency"));
    EXPECT_EQ(counters.getCategoryCounterSum("lsp.messages.canceled"), 0);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    EXPECT_EQ(counters.getCategoryCounterSum("lsp.updates"), 1);
    // 1 per edit
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 2, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CancelsSlowPathWhenNewEditWouldTakeFastPathWithOldEdits) {
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

    // clear counters
    getCounters();

    // Slow path edits two files. One introduces error.
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Syntax error in foo.rb.
    sendAsync(*changeFile("foo.rb", "# typed: true\n\nclass Foo\ndef noend\nend\n", 2, true));
    // Pause to differentiate message times
    this_thread::sleep_for(chrono::milliseconds(2));
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

    // Pause for a moderate amount of time so that we can check that the subsequent fast path includes the latency timer
    // from the initial slow path.
    this_thread::sleep_for(chrono::milliseconds(5));

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

    auto counters = getCounters();
    // N.B.: lsp.messages.processed contains canceled slow paths.
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 2);
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 2);

    // We don't report task latencies for merged edits or canceled slow paths.
    auto taskLatency = counters.getTimings("task_latency", {{"method", "sorbet.workspaceEdit"}});
    EXPECT_EQ(taskLatency.size(), 1);
    EXPECT_GE(taskLatency[0]->end.usec - taskLatency[0]->start.usec, 5'000 /* 5ms */);
    EXPECT_EQ(counters.getCategoryCounterSum("lsp.messages.canceled"), 0);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 1);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 0);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 1);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "query"), 0);

    auto lastDiagnosticLatency = counters.getTimings("last_diagnostic_latency");
    sortTimersByStartTime(lastDiagnosticLatency);
    EXPECT_GE(lastDiagnosticLatency[0]->end.usec - lastDiagnosticLatency[0]->start.usec, 5'000 /* 5ms */);
    EXPECT_GE(lastDiagnosticLatency[1]->end.usec - lastDiagnosticLatency[1]->start.usec, 5'000 /* 5ms */);
    // 1 per edit
    checkDiagnosticTimes(move(lastDiagnosticLatency), 3, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CancelsSlowPathWhenNewEditWouldTakeSlowPath) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Initial state: Two empty files.
    assertDiagnostics(send(*openFile("foo.rb", "")), {});
    assertDiagnostics(send(*openFile("bar.rb", "")), {});

    // clear counters
    getCounters();

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

    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    // Slow path 2: Bar defines the expected method, but declared with a non-integer return value (so foo now has a new
    // error).
    sendAsync(*changeFile("bar.rb",
                          "# typed: true\n\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef bar\n10\nend\nend\n", 2,
                          false));
    // Pause so that all latency timers for the above operations get reported.
    this_thread::sleep_for(timestampGranularity);
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

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

    auto counters = getCounters();
    // N.B.: lsp.messages.processed contains canceled slow paths.
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 2);
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 1);

    // We don't report task latencies for merged edits or canceled slow paths.
    EXPECT_EQ(counters.getTimings("task_latency", {{"method", "sorbet.workspaceEdit"}}).size(), 1);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 0);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 1);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "query"), 0);
    // 1 per edit
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 2, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CanPreemptSlowPathWithHover) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Create a new file.
    assertDiagnostics(send(*openFile("foo.rb", "")), {});

    // clear counters
    getCounters();

    // Slow path: Edit foo to have a class with a documentation string.
    sendAsync(*changeFile("foo.rb", "# typed: true\n# A class that does things.\nclass Foo\nextend T::Sig\nend\n", 2,
                          false, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
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
        ASSERT_TRUE(response->isResponse());
        auto &hoverText =
            get<unique_ptr<Hover>>(get<variant<JSONNullObject, unique_ptr<Hover>>>(*response->asResponse().result));
        EXPECT_TRUE(absl::StrContains(hoverText->contents->value, "A class that does things"));
    }

    // Second should be typecheck run signaling that typechecking completed.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }

    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});

    auto counters = getCounters();
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 1);
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 0);
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "textDocument.hover"), 1);

    EXPECT_EQ(counters.getTimings("task_latency", {{"method", "sorbet.workspaceEdit"}}).size(), 1);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 0);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 0);
    EXPECT_EQ(counters.getCategoryCounter("lsp.updates", "query"), 1);
    // 1 per edit
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 1, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CanPreemptSlowPathWithHoverAndReturnsErrors) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Create a new file.
    assertDiagnostics(send(*openFile("foo.rb", "")), {});

    // Slow path: Edit foo to have a class with a documentation string and a method with an error.
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n# A class that does things.\nclass Foo\nextend "
                          "T::Sig\nsig{returns(String)}\ndef bar\n3\nend\nend\n",
                          2, false, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Send a hover to request the documentation string.
    sendAsync(*hover("foo.rb", 2, 6));

    // First response should be hover.
    {
        auto response = readAsync();
        ASSERT_TRUE(response->isResponse());
        auto &hoverText =
            get<unique_ptr<Hover>>(get<variant<JSONNullObject, unique_ptr<Hover>>>(*response->asResponse().result));
        EXPECT_TRUE(absl::StrContains(hoverText->contents->value, "A class that does things"));
    }

    // Send a no-op to clear out the pipeline. Should have one error in `foo.rb`.
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                      {
                          {"foo.rb", 6, "Returning value that does not conform to method result type"},
                      });
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 2, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CanPreemptSlowPathWithFastPath) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Create two new files.
    assertDiagnostics(send(*openFile("foo.rb", "")), {});
    assertDiagnostics(send(*openFile("bar.rb", "")), {});

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
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path 1: Correct _one_ error.
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\nclass Foo\nextend T::Sig\nsig{returns(Integer)}\ndef "
                          "bar\n10\nend\nsig{returns(Float)}\ndef baz\n'not a float'\nend\nend\n",
                          3));

    // Send a no-op to clear out the pipeline. Should have two error now: bar.rb from slow path and foo.rb from fast
    // path.
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                      {
                          {"foo.rb", 9, "Returning value that does not conform to method result type"},
                          {"bar.rb", 5, "Returning value that does not conform to method result type"},
                      });
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 5, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CanPreemptSlowPathWithFastPathThatFixesAllErrors) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Create two new files.
    assertDiagnostics(send(*openFile("foo.rb", "")), {});
    assertDiagnostics(send(*openFile("bar.rb", "")), {});

    // Slow path: Edit foo to have a class with an error that also causes an error in bar
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\nclass Foo\nextend T::Sig\nsig{returns(Integer)}\ndef "
                          "bar\n'hello'\nend\nend\n",
                          2, false, 1));
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef str\nFoo.new.bar\nend\nend\n",
        3));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path 1: Correct return type on foo::bar, which should fix foo.rb and bar.rb.
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\nclass Foo\nextend T::Sig\nsig{returns(String)}\ndef "
                          "bar\n'hello'\nend\nend\n",
                          3));

    // Send a no-op to clear out the pipeline. Should have no errors at end of both typechecking runs.
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 5, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CanPreemptSlowPathWithFastPathAndThenCancelBoth) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Create three new files! foo.rb defines a class, bar.rb defines a class and method used in baz.rb.
    assertDiagnostics(send(*openFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend")), {});
    assertDiagnostics(
        send(*openFile("bar.rb",
                       "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef str\n'hi'\nend\nend\n")),
        {});
    assertDiagnostics(
        send(*openFile(
            "baz.rb",
            "# typed: true\nclass Baz\nextend T::Sig\nsig{returns(String)}\ndef bar\nBar.new.str\nend\nend\n")),
        {});

    // Slow path: foo.rb will have a syntax error
    sendAsync(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\n", 2, true, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path [preempt]: Change return type in bar.rb, which indirectly causes an error in baz.rb too.
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(Integer)}\ndef str\n'hi'\nend\nend\n", 4));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path [cancel]: Fix syntax error. Foo should not have any errors.
    assertDiagnostics(send(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend", 4)),
                      {
                          {"bar.rb", 5, "Returning value that does not conform to method result type"},
                          {"baz.rb", 5, "Returning value that does not conform to method result type"},
                      });

    auto counters = getCounters();
    checkDiagnosticTimes(counters.getTimings("last_diagnostic_latency"), 6, /* assertUniqueStartTimes */ false);

    // N.B.: This counter contains canceled edits.
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.workspaceEdit"), 6);
    // mergedEdits should not count the preempted fast path, but should count the canceled slow path.
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 1);
}

TEST_P(ProtocolTest, CanPreemptSlowPathWithFastPathAndBothErrorsAreReported) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Create three new files! foo.rb defines a class, bar.rb defines a class and method used in baz.rb.
    assertDiagnostics(send(*openFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend")), {});
    assertDiagnostics(
        send(*openFile("bar.rb",
                       "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(String)}\ndef str\n'hi'\nend\nend\n")),
        {});
    assertDiagnostics(
        send(*openFile(
            "baz.rb",
            "# typed: true\nclass Baz\nextend T::Sig\nsig{returns(String)}\ndef bar\nBar.new.str\nend\nend\n")),
        {});

    // Slow path: foo.rb will have a syntax error
    sendAsync(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig", 2, false, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path [preempt]: Change return type in bar.rb, which indirectly causes an error in baz.rb too.
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(Integer)}\ndef str\n'hi'\nend\nend\n", 4));

    // We should receive errors for `foo`, `bar`, and `baz`.
    // Send a no-op to clear out the pipeline.
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                      {
                          {"foo.rb", 2, "unexpected token"},
                          {"bar.rb", 5, "Returning value that does not conform to method result type"},
                          {"baz.rb", 5, "Returning value that does not conform to method result type"},
                      });
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 5, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CanCancelSlowPathWithFastPathThatReintroducesOldError) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // foo stands alone
    assertDiagnostics(send(*openFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend\n")), {});
    // bar defines method
    assertDiagnostics(
        send(*openFile("bar.rb",
                       "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(Integer)}\ndef str\n10\nend\nend\n")),
        {});
    // baz uses it
    assertDiagnostics(
        send(*openFile(
            "baz.rb",
            "# typed: true\nclass Baz\nextend T::Sig\nsig{returns(String)}\ndef bar\nBar.new.str\nend\nend\n")),
        {{"baz.rb", 5, "Returning value that does not conform to method result type"}});

    // Slow path: Introduce syntax error to foo.rb and change method sig in bar.rb to fix error in baz.rb
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\n", 2, true));
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\n\nclass Bar\nextend T::Sig\n\nsig{returns(String)}\ndef str\n'10'\nend\nend\n", 2,
        true));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path: Undo previous changes. Should re-introduce error on baz.rb.
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend\n", 3));
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\n\nclass Bar\nextend T::Sig\n\nsig{returns(Integer)}\ndef str\n10\nend\nend\n", 3));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Send a no-op to clear out the pipeline. Should have one error on baz.rb.
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                      {{"baz.rb", 5, "Returning value that does not conform to method result type"}});
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 7, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CanCancelSlowPathEvenIfAddsFile) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // bar has no error
    assertDiagnostics(
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
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Introduce error fast path: Will preempt.
    sendAsync(*changeFile(
        "foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nsig {returns(Integer)}\ndef foo\n'hi'\nend\nend\n", 2));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Introduce slow path in unrelated file; will cancel.
    sendAsync(*openFile("bar.rb", "# typed: true\nclass Bar\nextend T::Sig\nsig{returns(Integer)}\ndef hi\n10\nend"));

    // Send fence to clear out the pipeline.
    assertDiagnostics(
        send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
        {{"foo.rb", 5, "Returning value that does not conform to method result type"}, {"bar.rb", 6, "unexpected"}});
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 5, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, CanceledRequestsDontReportLatencyMetrics) {
    assertDiagnostics(initializeLSP(), {});

    // Create a new file.
    assertDiagnostics(
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
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.processed", "textDocument.hover"), 0);
    EXPECT_EQ(counters.getCategoryCounter("lsp.messages.canceled", "textDocument.hover"), 1);
    EXPECT_EQ(counters.getTimings("task_latency", {{"method", "textDocument.hover"}}).size(), 0);
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 0, /* assertUniqueStartTimes */ false);
}

TEST_P(ProtocolTest, ErrorIntroducedInSlowPathPreemptionByFastPathClearedByNewSlowPath) {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertDiagnostics(initializeLSP(true /* supportsMarkdown */, move(initOptions)), {});

    // Create new file
    assertDiagnostics(send(*openFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend")), {});

    // Slow path: no errors
    sendAsync(*changeFile(
        "foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nsig{returns(Integer)}\ndef str\n1\nend\nend", 2, true, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path [preempt]: Change return type and introduce an error
    sendAsync(*changeFile(
        "foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nsig{returns(Integer)}\ndef str\n'hi'\nend\nend\n", 3));

    // Wait for typechecking of fast path so it and subsequent slow path change aren't combined
    {
        auto status = getTypecheckRunStatus(*readAsync());
        ASSERT_TRUE(status.has_value());
        ASSERT_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Slow path: error will be resolved
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\nclass Foo\nextend T::Sig\nsig{returns(String)}\ndef "
                          "str\n'hi'\nend\nsig{returns(Integer)}\ndef int\n1\nend\nend",
                          4, false, 0));

    // // Send a no-op to clear out the pipeline.
    assertDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});
}

// Run these tests in multi-threaded mode.
INSTANTIATE_TEST_SUITE_P(MultithreadedProtocolTests, ProtocolTest, testing::Values(ProtocolTestConfig{true}));

} // namespace sorbet::test::lsp
