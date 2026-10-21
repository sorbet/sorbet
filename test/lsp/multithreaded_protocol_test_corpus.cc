#include "doctest/doctest.h"
// ^ Violates linting rules, so include first.
#include "absl/strings/match.h"
#include "common/common.h"
#include "common/sort/sort.h"
#include "test/helpers/lsp.h"
#include "test/helpers/packages.h"
#include "test/lsp/ProtocolTest.h"

using namespace std;

namespace sorbet::test::lsp {
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

    // The timeout here is a balance: we want to wait long enough for all slow path edits to
    // complete, but not so long that we're just busy waiting. Because the files involved in this
    // test are generally tiny, 500ms should be plenty of time, but if this is causing flakiness we
    // will need to adjust this or, ideally, make the slow path faster ;-)
    void assertSlowPathBlockedAndUnblock(int timeout_ms = 500) {
        auto &wrapper = dynamic_cast<MultiThreadedLSPWrapper &>(*lspWrapper);
        auto msg = wrapper.read(timeout_ms);
        REQUIRE(msg == nullptr);
        setSlowPathBlocked(false);
    }

    // Initializes Sorbet in package mode over `files` (written to the workspace, in this order) and clears the
    // counters.
    void initializePackagedWorkspace(bool packageDirected, const vector<pair<string, string>> &files,
                                     optional<int> sleepInSlowPathSeconds = nullopt) {
        auto opts = make_shared<realmain::options::Options>();
        opts->cacheSensitiveOptions.sorbetPackages = true;
        opts->packageDirected = packageDirected;
        opts->sleepInSlowPathSeconds = sleepInSlowPathSeconds;
        resetState(opts);

        writeFilesToFS(files);
        for (auto &[path, _] : files) {
            lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", rootPath, path));
        }

        auto initOptions = make_unique<SorbetInitializationOptions>();
        initOptions->enableTypecheckInfo = true;
        assertErrorDiagnostics(
            initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});
        getCounters();
    }

    // Starts a slow path by creating `newFile`, cancels it by creating `cancelFile` once it has started, and returns
    // the messages of a fence sent afterwards. `cancellationExpected` on the first edit makes the slow path wait for
    // the cancellation only once it has rearranged `workspaceFiles`.
    vector<unique_ptr<LSPMessage>> cancelSlowPathWithNewFile(const pair<string, string> &newFile,
                                                             const pair<string, string> &cancelFile) {
        sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
        sendAsync(*openFile(newFile.first, ""));
        sendAsync(*changeFile(newFile.first, newFile.second, 2, /* cancellationExpected */ true));
        sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));
        {
            auto status = getTypecheckRunStatus(*readAsync());
            REQUIRE(status.has_value());
            REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
        }

        sendAsync(*openFile(cancelFile.first, cancelFile.second));
        {
            auto status = getTypecheckRunStatus(*readAsync());
            REQUIRE(status.has_value());
            REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Cancelled);
        }

        return send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20)));
    }

    void runTestExportPreemption(bool packageDirected) {
        vector<pair<string, string>> files = {
            {
                "__package.rb",
                PackageTextBuilder()
                    .withName("Root")
                    .withExports({"Test::Root::Helper"})
                    .withTestImports({"Root::Foo"})
                    .build(),
            },
            {
                "a.rb",
                "# typed: true\n"
                "module Root\n"
                "  class A; end\n"
                "end\n",
            },
            {
                "test/helper.rb",
                "# typed: true\n"
                "class Test::Root::Helper\n"
                "end\n",
            },

            {
                "foo/__package.rb",
                PackageTextBuilder().withName("Root::Foo").build(),
            },
            {
                "foo/b.rb",
                "# typed: true\n"
                "module Root::Foo\n"
                "end\n",
            },
        };
        initializePackagedWorkspace(packageDirected, files);

        assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
        assertErrorDiagnostics(send(*openFile("foo/b.rb", files[4].second)), {});

        setSlowPathBlocked(true);
        sendAsync(*changeFile("foo/b.rb",
                              "# typed: true\n"
                              "module Root::Foo\n"
                              "  class NewClassTakesSlowPath; end\n"
                              "end\n",
                              2, /* cancellationExpected */ false, /* preemptionsExpected */ 1));

        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);

        // Adding a method uses the incremental namer, which re-indexes Root's package file to restore visibility while
        // preempting the slow path.
        sendAsync(*changeFile("a.rb",
                              "# typed: true\n"
                              "module Root\n"
                              "  class A\n"
                              "    def new_method_takes_fast_path; end\n"
                              "  end\n"
                              "end\n",
                              2));
        setSlowPathBlocked(false);

        for (auto expectedStatus : {SorbetTypecheckRunStatus::Started, SorbetTypecheckRunStatus::Ended}) {
            auto message = readAsync();
            REQUIRE(message->isNotification());
            if (message->asNotification().method == LSPMethod::TextDocumentPublishDiagnostics) {
                FAIL_CHECK(
                    fmt::format("Expected fast path status, but received diagnostics:\n{}", message->toJSON(true)));
                return;
            }
            REQUIRE_EQ(message->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
            auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(message->asNotification().params);
            CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Fast);
            CHECK_EQ(runInfo->status, expectedStatus);
        }

        auto slowPathEnd = readAsync();
        REQUIRE(slowPathEnd->isNotification());
        REQUIRE_EQ(slowPathEnd->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(slowPathEnd->asNotification().params);
        CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Slow);
        CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Ended);
    }
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
        auto status = getTypecheckRunStatus(*readAsync());
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
    this_thread::sleep_for(timestampGranularity);
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Wait for first typecheck run to get canceled.
    {
        auto status = getTypecheckRunStatus(*readAsync());
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

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptSlowPathWithEditToFileWithStrictnessOverride") {
    // bar.rb defines a class, but the override hides it from Sorbet.
    auto opts = make_shared<realmain::options::Options>();
    opts->strictnessOverrides[fmt::format("{}/bar.rb", this->rootPath)] = core::StrictLevel::Ignore;
    resetState(opts);

    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    assertErrorDiagnostics(send(*openFile("bar.rb", "# typed: true\n"
                                                    "class Bar\n"
                                                    "end\n")),
                           {});

    // clear counters
    getCounters();

    // Slow path: Edit foo to have a class. Expect the edit below to preempt.
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "end\n",
                          2, false, 1));

    // Wait for typechecking to begin to avoid races.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path: Sorbet still ignores the contents of bar, so no definition changes.
    sendAsync(*changeFile("bar.rb",
                          "# typed: true\n"
                          "class Bar\n"
                          "end\n"
                          "# A comment.\n",
                          2));

    // Send a no-op to clear out the pipeline.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 0);
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
        auto status = getTypecheckRunStatus(*readAsync());
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
        auto status = getTypecheckRunStatus(*readAsync());
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
    // baz uses it
    assertErrorDiagnostics(
        send(*openFile(
            "baz.rb",
            "# typed: true\nclass Baz\nextend T::Sig\nsig{returns(String)}\ndef bar\nBar.new.str\nend\nend\n")),
        {{"baz.rb", 5, "Expected `String` but found `Integer` for method result type"}});

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
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path: Undo previous changes. Should re-introduce error on baz.rb.
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, nullopt)));
    sendAsync(*changeFile("foo.rb", "# typed: true\nclass Foo\nextend T::Sig\nend\n", 3));
    sendAsync(*changeFile(
        "bar.rb", "# typed: true\n\nclass Bar\nextend T::Sig\n\nsig{returns(Integer)}\ndef str\n10\nend\nend\n", 3));
    sendAsync(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, nullopt)));

    // Send a no-op to clear out the pipeline. Should have one error on baz.rb.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))),
                           {{"baz.rb", 5, "Expected `String` but found `Integer` for method result type"}});
    checkDiagnosticTimes(getCounters().getTimings("last_diagnostic_latency"), 7,
                         /* assertUniqueStartTimes */ false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CancelPackageDirectedSlowPathDiscardsQueuedPackageErrors") {
    auto packageText = PackageTextBuilder().withName("Root").withExports({"Root::Value"}).build();
    initializePackagedWorkspace(
        true, {{"__package.rb", packageText}, {"a.rb", "# typed: strict\nmodule Root\n  Value = 1\nend\n"}});
    assertErrorDiagnostics(send(*openFile("__package.rb", packageText)), {});
    getCounters();

    // Packager queues the invalid-argument diagnostic immediately. The cancellation wait is
    // in the application stratum, before the package AST can drain that queue in its test stratum.
    sendAsync(*changeFile("__package.rb",
                          "# typed: strict\nclass Root < PackageSpec\n"
                          "  export Root::Value\n  bogus Root::Value\nend\n",
                          2, true));
    auto status = getTypecheckRunStatus(*readAsync());
    REQUIRE(status.has_value());
    REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    sendAsync(*changeFile("__package.rb", packageText, 3));
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});
    CHECK_EQ(getCounters().getCategoryCounter("lsp.updates", "slowpath_canceled"), 1);
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

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CancelingSlowPathThatAddsPackageFileKeepsExistingWorkspaceFiles") {
    // Canceling a slow path must leave `LSPTypechecker::workspaceFiles` with its pre-edit membership. The slow path
    // appends the edit's new files and then partitions the vector in place (package files first, order not preserved),
    // so a new `__package.rb` displaces a pre-existing source file past the old size; a rollback that truncated to the
    // old size dropped that file from the workspace for good.
    initializePackagedWorkspace(
        /* packageDirected */ false,
        {
            {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A", "Root::B"}).build()},
            // `a.rb` and `b.rb` reference each other, so losing either is an unresolved
            // constant in the other.
            {"a.rb", "# typed: true\n"
                     "module Root\n"
                     "  class A\n"
                     "    def fun\n"
                     "      B.new.bar\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
            {"b.rb", "# typed: true\n"
                     "module Root\n"
                     "  class B\n"
                     "    def bar\n"
                     "      A.new.fun\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
        });

    // The canceling edit lives in the new package and uses the root package's exports, so it only typechecks cleanly
    // if nothing was lost along the way.
    assertErrorDiagnostics(
        cancelSlowPathWithNewFile(
            {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
            {"foo/c.rb", "# typed: true\n"
                         "module Root::Foo\n"
                         "  class C\n"
                         "    def c\n"
                         "      Root::A.new.fun\n"
                         "    end\n"
                         "  end\n"
                         "end\n"}),
        {});

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 1);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest,
                  "CancelingPackageDirectedSlowPathThatAddsLowStratumFileKeepsExistingWorkspaceFiles") {
    // Same invariant for the second way the slow path reorders `workspaceFiles`: with package-directed typechecking,
    // computePackageStrata sorts the non-package files in place by (stratum, id), so a new low-stratum file displaces
    // the last high-stratum file past the old size -- no new package file involved.
    initializePackagedWorkspace(
        /* packageDirected */ true,
        {
            {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
            {"a.rb", "# typed: true\n"
                     "module Root\n"
                     "  class A\n"
                     "  end\n"
                     "end\n"},
            {"foo/__package.rb",
             PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).withExports({"Root::Foo::B"}).build()},
            // Root::Foo depends on Root, so `foo/b.rb` is the last file after the stratum
            // sort and the one a new file in Root displaces.
            {"foo/b.rb", "# typed: true\n"
                         "module Root::Foo\n"
                         "  class B\n"
                         "  end\n"
                         "end\n"},
        });

    assertErrorDiagnostics(cancelSlowPathWithNewFile({"d.rb", "# typed: true\n"
                                                              "module Root\n"
                                                              "  class D\n"
                                                              "  end\n"
                                                              "end\n"},
                                                     {"e.rb", "# typed: true\n"
                                                              "module Root\n"
                                                              "  class E\n"
                                                              "  end\n"
                                                              "end\n"}),
                           {});

    // `foo/b.rb` is still part of the workspace: its class is in the symbol table.
    auto responses = send(*workspaceSymbol("Root::Foo::B"));
    REQUIRE_EQ(responses.size(), 1);
    auto &result =
        get<variant<JSONNullObject, vector<unique_ptr<SymbolInformation>>>>(responses[0]->asResponse().result.value());
    CHECK_EQ(get<vector<unique_ptr<SymbolInformation>>>(result).size(), 1);

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 1);
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

    assertSlowPathBlockedAndUnblock();

    // The slow path should now be unblocked. Wait for typechecking to end.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PreemptionCanceledForSlowPathWorks") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create a simple file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
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

    // Clear counters
    getCounters();

    // Turn on slow path blocking, and send an edit that is going to cause a slow path.
    setSlowPathBlocked(true);
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include Mod\n"
                          "end\n",
                          3, false, 0));

    // Wait for typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Make a completion query in the queue so that we schedule preemption.
    sendAsync(*completion("foo.rb", 4, 14));

    // Send another update that finishes the include. This should cancel preemption
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include ModuleA\n"
                          "end\n",
                          4, false, 0));

    assertSlowPathBlockedAndUnblock();

    // The slow path should now be unblocked, and will be canceled immediately.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Cancelled);
    }

    // Now we'll see the second slow path edit start
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Then the response to the completion request
    {
        auto msg = readAsync();
        REQUIRE(msg->isResponse());
        REQUIRE_EQ(msg->asResponse().requestMethod, LSPMethod::TextDocumentCompletion);
    }

    // And finally the slow path finishes
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }

    // Verify the path that we took during cancellation.
    {
        auto counters = getCounters();
        CHECK_EQ(counters.getCategoryCounter("lsp.preemption.cancelation", "single"), 1);
        CHECK_EQ(counters.getCategoryCounter("lsp.preemption.cancelation", "merged"), 0);
    }

    // Make another slow path edit that deletes part of the module name
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include Modu\n"
                          "end\n",
                          5, false, 0));

    // Wait for typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // An error will be raised for the include argument
    {
        vector<unique_ptr<LSPMessage>> msgs;
        msgs.emplace_back(readAsync());
        // TODO(trevor): why does this report the same error twice?
        assertErrorDiagnostics(move(msgs), {{"foo.rb", 4, "Unable to resolve constant `Modu`"},
                                            {"foo.rb", 4, "Unable to resolve constant `Modu`"}});
    }

    // And finally the slow path finishes
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PreemptionCanceledForSlowPathWorksWithDelete") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create a simple file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include Foo::Bar::Baz::Foo\n"
                          "end\n",
                          2));

    // Wait for initial typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Wait for initial typechecking to finish, with errors.
    {
        auto msg = readAsync();
        REQUIRE(msg->isNotification());
        REQUIRE_EQ(msg->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);

        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }

    // Clear counters
    getCounters();

    // Turn on slow path blocking, and send an edit that is going to cause a slow path.
    setSlowPathBlocked(true);
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include Mod\n"
                          "end\n",
                          3, false, 0));

    // Wait for typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Make a completion query at the end of the old include into the queue so that we schedule preemption.
    sendAsync(*completion("foo.rb", 4, 28));

    // Send another update that finishes the include. This should cancel preemption
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include ModuleA\n"
                          "end\n",
                          4, false, 0));

    assertSlowPathBlockedAndUnblock();

    // The slow path should now be unblocked, and will be canceled immediately.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Cancelled);
    }

    // Now we'll see the second slow path edit start
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Then the response to the completion request
    {
        auto msg = readAsync();
        REQUIRE(msg->isResponse());
        REQUIRE_EQ(msg->asResponse().requestMethod, LSPMethod::TextDocumentCompletion);
    }

    // And finally the slow path finishes after reporting that there are no more errors.
    {
        std::vector<unique_ptr<LSPMessage>> msgs;
        msgs.emplace_back(readAsync());
        assertErrorDiagnostics(move(msgs), {});

        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }

    // Verify the path that we took during cancellation.
    {
        auto counters = getCounters();
        CHECK_EQ(counters.getCategoryCounter("lsp.preemption.cancelation", "single"), 1);
        CHECK_EQ(counters.getCategoryCounter("lsp.preemption.cancelation", "merged"), 0);
    }

    // Make another slow path edit that deletes part of the module name
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include Modu\n"
                          "end\n",
                          5, false, 0));

    // Wait for typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // An error will be raised for the include argument
    {
        vector<unique_ptr<LSPMessage>> msgs;
        msgs.emplace_back(readAsync());
        // TODO(trevor): why does this report the same error twice?
        assertErrorDiagnostics(move(msgs), {{"foo.rb", 4, "Unable to resolve constant `Modu`"},
                                            {"foo.rb", 4, "Unable to resolve constant `Modu`"}});
    }

    // And finally the slow path finishes
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "MergingEditsWhenCancellingPreemptionWorks") {
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Create a simple file.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "\n"
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

    // Clear counters
    getCounters();

    // Turn on slow path blocking, and send an edit that is going to cause a slow path.
    setSlowPathBlocked(true);
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include Mod\n"
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

    // Send another change that would take the fast path
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include Mod\n"
                          "  def foo\n"
                          "    puts :hi\n"
                          "  end\n"
                          "end\n",
                          4, false, 0));

    // Make a completion query in the queue so that we schedule preemption.
    sendAsync(*completion("foo.rb", 4, 14));

    // Send another update that finishes the include. This should cancel preemption
    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo\n"
                          "  module ModuleA; end\n"
                          "  module ModuleB; end\n"
                          "  include ModuleA\n"
                          "  def foo\n"
                          "    puts :hi\n"
                          "  end\n"
                          "end\n",
                          5, false, 0));

    assertSlowPathBlockedAndUnblock();

    // The slow path should now be unblocked, and will be canceled immediately.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Cancelled);
    }

    // Now we'll see the second slow path edit start
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Then the response to the completion request
    {
        auto msg = readAsync();
        REQUIRE(msg->isResponse());
        REQUIRE_EQ(msg->asResponse().requestMethod, LSPMethod::TextDocumentCompletion);
    }

    // And finally the slow path finishes
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Ended);
    }

    // Verify the path that we took during cancellation.
    {
        auto counters = getCounters();
        CHECK_EQ(counters.getCategoryCounter("lsp.preemption.cancelation", "single"), 0);
        CHECK_EQ(counters.getCategoryCounter("lsp.preemption.cancelation", "merged"), 1);
    }
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "CanPreemptDoesNotCrashOnEvictedFilesShortCircuit") {
    // Set lspMaxFilesOnFastPath to 1 so the short-circuit in fastPathFilesToTypecheck
    // triggers with just a few extra files: totalChanged > 2 * 1 = 2.
    // (We could do this without, but it would involve making dozens of files that use the symbol)
    auto opts = make_shared<realmain::options::Options>();
    opts->lspMaxFilesOnFastPath = 1;
    resetState(opts);

    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Sequence of edits:
    //
    // - Slow path edit in foo.rb
    // - While slow path is going, fast path edit in bar.rb that implicates more than lspMaxFilesOnFastPath

    assertErrorDiagnostics(send(*openFile("bar.rb", "# typed: true\n"
                                                    "class Bar\n"
                                                    "  extend T::Sig\n"
                                                    "  sig { returns(String) }\n"
                                                    "  def compute_value = 'hi'\n"
                                                    "end\n")),
                           {});
    assertErrorDiagnostics(send(*openFile("baz.rb", "# typed: true\n"
                                                    "class Baz\n"
                                                    "  def baz_method = Bar.new.compute_value\n"
                                                    "end\n")),
                           {});
    assertErrorDiagnostics(send(*openFile("qux.rb", "# typed: true\n"
                                                    "class Qux\n"
                                                    "  def qux_method = Bar.new.compute_value\n"
                                                    "end\n")),
                           {});
    assertErrorDiagnostics(send(*openFile("foo.rb", "# typed: true\n"
                                                    "class Foo\n"
                                                    "end")),
                           {});

    setSlowPathBlocked(true);

    sendAsync(*changeFile("foo.rb",
                          "# typed: true\n"
                          "class Foo2\n"
                          "end",
                          2));

    // Wait for the slow path to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Fast path edit changes compute_value: which means 3 files on the fast path, triggering the
    // short-circuit in `fastPathFilesToTypecheck`.
    sendAsync(*changeFile("bar.rb",
                          "# typed: true\n"
                          "class Bar\n"
                          "  extend T::Sig\n"
                          "  sig { returns(Integer) }\n"
                          "  def compute_value = 0\n"
                          "end\n",
                          4));

    // Unblock the slow path.
    setSlowPathBlocked(false);

    // Drain the pipeline with a fence.
    assertErrorDiagnostics(send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20))), {});
}

// TODO(jez) We might want to do something like this for the whole multithreaded_protocol_test_corpus?
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "MonolithicPreemptionWithTestExports") {
    runTestExportPreemption(false);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedPreemptionWithTestExports") {
    runTestExportPreemption(true);
}

// This case shows how preemption works when the preemption tasks occur in a stratum that's at or before the one that
// contains the file that kicked off the slow path.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedPreemptionBeforeSlowPathStratum") {
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    resetState(opts);

    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    def fun\n"
                 "    end\n"
                 "  end\n"
                 "end\n"},
        {"foo/__package.rb",
         PackageTextBuilder().withName("Root::Foo").withExports({"Root::Foo::A"}).withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "  class A\n"
                     "    def foo\n"
                     "      Root::A.new()\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
    };

    writeFilesToFS(files);

    for (auto &[path, _] : files) {
        this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", this->rootPath, path));
    }

    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});

    assertErrorDiagnostics(send(*openFile("foo/b.rb", "# typed: true\n"
                                                      "module Root::Foo\n"
                                                      "  class Bar\n"
                                                      "  end\n"
                                                      "end\n")),
                           {});

    setSlowPathBlocked(true);

    {
        auto cancellationExpected = false;

        // We're expecting one preemption from each of the following sub-cases.
        auto preemptionsExpected = 1;

        sendAsync(*changeFile("foo/b.rb",
                              "# typed: true\n"
                              "module Root::Foo\n"
                              "  class Bar\n"
                              "    class Inner\n"
                              "      def bar\n"
                              "        Root::A.n\n"
                              "      end\n"
                              "    end\n"
                              "  end\n"
                              "end\n",
                              2, cancellationExpected, preemptionsExpected));
    }

    // Wait for typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    SUBCASE("completion") {
        // Send a completion request at the end of `Root::A.`
        sendAsync(*completion("foo/b.rb", 5, 16));

        setSlowPathBlocked(false);

        // This is the completion result
        auto resp = readAsync();
        REQUIRE(resp->isResponse());
        CHECK_EQ(resp->asResponse().requestMethod, LSPMethod::TextDocumentCompletion);
    }

    SUBCASE("hover") {
        // Send a hover request at the end of `Root::A.`
        sendAsync(*hover("foo/b.rb", 5, 14));

        setSlowPathBlocked(false);

        // This is the hover result
        auto resp = readAsync();
        REQUIRE(resp->isResponse());
        CHECK_EQ(resp->asResponse().requestMethod, LSPMethod::TextDocumentHover);
    }

    SUBCASE("fast_path_edit") {
        // Make a fast-path edit to a.rb
        sendAsync(*changeFile("a.rb",
                              "# typed: true\n"
                              "module Root\n"
                              "  class A\n"
                              "    def fun\n"
                              "      A.new\n"
                              "    end\n"
                              "  end\n"
                              "end\n",
                              2));

        setSlowPathBlocked(false);

        // We should see a fast path start notification
        {
            auto diag = readAsync();
            REQUIRE(diag->isNotification());
            REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
            auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
            CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Fast);
            CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Started);
            CHECK(runInfo->filesTypechecked.empty());
        }

        // And then a fast path end notification
        {
            auto diag = readAsync();
            REQUIRE(diag->isNotification());
            REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
            auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
            CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Fast);
            CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Ended);
            REQUIRE_EQ(runInfo->filesTypechecked.size(), 1);
            CHECK_EQ(runInfo->filesTypechecked.front(), fmt::format("{}/a.rb", this->rootPath));
        }
    }

    // This is the slow path publishing a notification about the call to Roo::A.n
    auto diag = readAsync();
    REQUIRE(diag->isNotification());
    CHECK_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);

    // Finally, we'll see the slow path end
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
        CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Slow);
        CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Ended);
    }
}

// This case shows how preemption works when the preemption tasks occur in a stratum that's after the one that contains
// the file that kicked off the slow path.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedPreemptionAfterSlowPathStratum") {
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    resetState(opts);

    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    def fun\n"
                 "    end\n"
                 "  end\n"
                 "end\n"},
        {"foo/__package.rb", PackageTextBuilder()
                                 .withName("Root::Foo")
                                 .withExports({"Root::Foo::A", "Test::Root::Foo::Helper"})
                                 .withImports({"Root"})
                                 .build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "  class A\n"
                     "    def foo\n"
                     "      Root::A.new()\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
        {"foo/test/helper.rb", "# typed: true\nmodule Test::Root::Foo\n  class Helper; end\nend\n"},
    };

    writeFilesToFS(files);

    for (auto &[path, _] : files) {
        this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", this->rootPath, path));
    }

    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    // Add a new file in the first stratum
    assertErrorDiagnostics(send(*openFile("b.rb", "# typed: true\n"
                                                  "module Root\n"
                                                  "  class B\n"
                                                  "  end\n"
                                                  "end\n")),
                           {});

    assertErrorDiagnostics(send(*openFile("foo/a.rb", files[3].second)), {});

    setSlowPathBlocked(true);

    {
        auto cancellationExpected = false;

        // We're expecting one preemption from each of the following sub-cases.
        auto preemptionsExpected = 1;

        // We're modifying a file in the first stratum, and making preemption queries for files in the second.
        sendAsync(*changeFile("b.rb",
                              "# typed: true\n"
                              "module Root\n"
                              "  class B\n"
                              "    class Inner\n"
                              "      def bar\n"
                              "        Root::A.n\n"
                              "      end\n"
                              "    end\n"
                              "  end\n"
                              "end\n",
                              2, cancellationExpected, preemptionsExpected));
    }

    // Wait for typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    SUBCASE("hover") {
        // Send a hover request at the end of `Root::A.`
        sendAsync(*hover("foo/a.rb", 4, 12));

        setSlowPathBlocked(false);

        // This is the slow path publishing a notification about the call to Roo::A.n in b.rb, which happens on an
        // earlier stratum than the hover in foo/a.rb.
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        CHECK_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);

        // This is the hover result
        auto resp = readAsync();
        REQUIRE(resp->isResponse());
        CHECK_EQ(resp->asResponse().requestMethod, LSPMethod::TextDocumentHover);
    }

    SUBCASE("package_hover") {
        // Preempt the slow path with a query on a test export in the package file. Its target is
        // only defined in Foo's test stratum, so the query must wait until that has been processed.
        sendAsync(*hover("foo/__package.rb", 5, 30));
        setSlowPathBlocked(false);

        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        CHECK_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);

        auto resp = readAsync();
        REQUIRE(resp->isResponse());
        CHECK_EQ(resp->asResponse().requestMethod, LSPMethod::TextDocumentHover);
        auto &hoverResult =
            get<unique_ptr<Hover>>(get<variant<JSONNullObject, unique_ptr<Hover>>>(*resp->asResponse().result));
        REQUIRE(hoverResult);
        CHECK(absl::StrContains(hoverResult->contents->value, "Test::Root::Foo::Helper"));
    }

    SUBCASE("fast_path_edit") {
        // Make a fast-path edit to foo/a.rb
        sendAsync(*changeFile("foo/a.rb",
                              "# typed: true\n"
                              "module Root::Foo\n"
                              "  class A\n"
                              "    def foo\n"
                              "      Root::A.new()\n"
                              "      puts :hi\n"
                              "    end\n"
                              "  end\n"
                              "end\n",
                              2));

        setSlowPathBlocked(false);

        // This is the slow path publishing a notification about the call to Roo::A.n in b.rb, which happens on an
        // earlier stratum than the fast path edit to foo/a.rb
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        CHECK_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);

        // We should see a fast path start notification
        {
            auto diag = readAsync();
            REQUIRE(diag->isNotification());
            REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
            auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
            CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Fast);
            CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Started);
            CHECK(runInfo->filesTypechecked.empty());
        }

        // And then a fast path end notification
        {
            auto diag = readAsync();
            REQUIRE(diag->isNotification());
            REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
            auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
            CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Fast);
            CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Ended);
            REQUIRE_EQ(runInfo->filesTypechecked.size(), 1);
            CHECK_EQ(runInfo->filesTypechecked.front(), fmt::format("{}/foo/a.rb", this->rootPath));
        }
    }

    // Finally, we'll see the slow path end
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
        CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Slow);
        CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Ended);
    }
}

// A fast path edit can typecheck files beyond the ones that it edits: when an edit adds a method, every file that
// mentions the method's name is typechecked with it (using the incremental namer). Those extra files can belong to a
// later stratum than the edited file, so a preempting fast path must wait for the slow path to reach the stratum of
// the extra files too, not just the stratum of the edited ones. Otherwise the incremental namer enters the extra
// files' symbols into a symbol table that the slow path hasn't reached yet, and nothing ever linearizes them.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedPreemptionWaitsForStratumOfFastPathExtraFiles") {
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    resetState(opts);

    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\n"
                 "module Root\n"
                 "  class B\n"
                 "  end\n"
                 "end\n"},
        {"foo/__package.rb",
         PackageTextBuilder().withName("Root::Foo").withExports({"Root::Foo::A"}).withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "  class A\n"
                     "    def foo\n"
                     "      T.unsafe(Root::A.new).bar\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
    };

    writeFilesToFS(files);

    for (auto &[path, _] : files) {
        this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", this->rootPath, path));
    }

    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});

    setSlowPathBlocked(true);

    {
        auto cancellationExpected = false;
        auto preemptionsExpected = 1;

        // Adding a class to b.rb takes the slow path, which starts over from the first stratum.
        sendAsync(*changeFile("b.rb",
                              "# typed: true\n"
                              "module Root\n"
                              "  class B\n"
                              "    class Inner\n"
                              "      def baz\n"
                              "        Root::A.n\n"
                              "      end\n"
                              "    end\n"
                              "  end\n"
                              "end\n",
                              2, cancellationExpected, preemptionsExpected));
    }

    // Wait for typechecking to start.
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Adding a method to a.rb (first stratum) takes the fast path, which also typechecks foo/a.rb (second stratum)
    // because it mentions `bar`.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    def bar; end\n"
                          "  end\n"
                          "end\n",
                          2));

    setSlowPathBlocked(false);

    // This is the slow path publishing a notification about the call to Root::A.n in b.rb, which happens in the first
    // stratum, before the slow path reaches the stratum that contains foo/a.rb.
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
        auto &params = get<unique_ptr<PublishDiagnosticsParams>>(diag->asNotification().params);
        CHECK_EQ(params->uri, fmt::format("file://{}/b.rb", this->rootPath));
    }

    // We should see a fast path start notification
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
        CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Fast);
        CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Started);
    }

    // And then a fast path end notification
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
        CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Fast);
        CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Ended);
        CHECK(absl::c_contains(runInfo->filesTypechecked, fmt::format("{}/foo/a.rb", this->rootPath)));
    }

    // Finally, we'll see the slow path end
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(diag->asNotification().params);
        CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Slow);
        CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Ended);
    }
}

// Like PackageDirectedPreemptionWithTestExports, but the `__package.rb` that the incremental namer re-checks belongs to
// a fast path *extra* file (foo/a.rb mentions `bar`), not to the edited file. Root::Foo is a legacy package whose test
// stratum comes after Root::Baz, which itself comes after Root::Foo's application stratum.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedPreemptionWaitsForPackageFileOfFastPathExtraFiles") {
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\n"
                 "module Root\n"
                 "  class B\n"
                 "  end\n"
                 "end\n"},
        {"foo/__package.rb", PackageTextBuilder()
                                 .withName("Root::Foo")
                                 .withExports({"Root::Foo::A", "Test::Root::Foo::Helper"})
                                 .withImports({"Root"})
                                 .withTestImports({"Root::Baz"})
                                 .build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "  class A\n"
                     "    def foo\n"
                     "      T.unsafe(Root::A.new).bar\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
        {"foo/test/helper.rb", "# typed: true\n"
                               "class Test::Root::Foo::Helper\n"
                               "end\n"},
        {"baz/__package.rb", PackageTextBuilder().withName("Root::Baz").withImports({"Root::Foo"}).build()},
        {"baz/b.rb", "# typed: true\n"
                     "module Root::Baz\n"
                     "end\n"},
    };
    initializePackagedWorkspace(true, files);

    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class B\n"
                          "    class NewClassTakesSlowPath; end\n"
                          "  end\n"
                          "end\n",
                          2, /* cancellationExpected */ false, /* preemptionsExpected */ 1));

    auto status = getTypecheckRunStatus(*readAsync());
    REQUIRE(status.has_value());
    REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);

    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    def bar; end\n"
                          "  end\n"
                          "end\n",
                          2));
    setSlowPathBlocked(false);

    for (auto expectedStatus : {SorbetTypecheckRunStatus::Started, SorbetTypecheckRunStatus::Ended}) {
        auto message = readAsync();
        REQUIRE(message->isNotification());
        if (message->asNotification().method == LSPMethod::TextDocumentPublishDiagnostics) {
            FAIL_CHECK(fmt::format("Expected fast path status, but received diagnostics:\n{}", message->toJSON(true)));
            return;
        }
        REQUIRE_EQ(message->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(message->asNotification().params);
        CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Fast);
        CHECK_EQ(runInfo->status, expectedStatus);
        if (expectedStatus == SorbetTypecheckRunStatus::Ended) {
            CHECK(absl::c_contains(runInfo->filesTypechecked, fmt::format("{}/foo/__package.rb", this->rootPath)));
        }
    }

    auto slowPathEnd = readAsync();
    REQUIRE(slowPathEnd->isNotification());
    REQUIRE_EQ(slowPathEnd->asNotification().method, LSPMethod::SorbetTypecheckRunInfo);
    auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(slowPathEnd->asNotification().params);
    CHECK_EQ(runInfo->typecheckingPath, TypecheckingPath::Slow);
    CHECK_EQ(runInfo->status, SorbetTypecheckRunStatus::Ended);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedDeferredEditMergedWithFastPath") {
    const string updatedA = "# typed: true\n"
                            "module Root\n"
                            "  class A\n"
                            "    extend T::Sig\n"
                            "    sig {returns(Integer)}\n"
                            "    def value = 1\n"
                            "  end\n"
                            "end\n";
    const string updatedB = "# typed: true\nmodule Root\n  class B\n    class Inner\n"
                            "      def bar = Root::A.n\n"
                            "    end\n"
                            "  end\nend\n# latest edit\n";
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    extend T::Sig\n"
                 "    sig {returns(String)}\n"
                 "    def value = \"old\"\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\nmodule Root\n  class B; end\nend\n"},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\nmodule Root::Foo\n  class Consumer\n"
                     "    def value = Root::A.new.value\n"
                     "  end\nend\n"},
    };
    // Leave time after Root's diagnostics for the newer edit to merge before the slow path reaches Root::Foo.
    initializePackagedWorkspace(true, files, 1);
    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\nmodule Root\n  class B\n    class Inner\n"
                          "      def bar = Root::A.n\n"
                          "    end\n"
                          "  end\nend\n",
                          2, /* cancellationExpected */ false, /* preemptionsExpected */ 1));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Editing A#value pulls foo/a.rb into the fast path. Preemption indexes this edit in the Root stratum, then
    // defers it until Root::Foo.
    sendAsync(*changeFile("a.rb", updatedA, 2));
    setSlowPathBlocked(false);
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
        auto params = getPublishDiagnosticParams(diag->asNotification());
        REQUIRE(params.has_value());
        CHECK_EQ((*params)->uri, getUri("b.rb"));
    }

    // A newer fast-path edit merges with the delayed edit without canceling the slow path. The merged task must
    // still contain a.rb, even though indexing moved that file out of the delayed task's params.
    sendAsync(*changeFile("b.rb", updatedB, 3));
    send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20)));
    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 0);
    CHECK_EQ(readFile(getUri("b.rb")), updatedB);
    CHECK_EQ(readFile(getUri("a.rb")), updatedA);
}

// When a fast path edit's preemption is deferred to a later stratum, it stays in the task queue. A newer edit that
// arrives in the meantime can be merged into it, and must still be typechecked.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedNewerEditMergedIntoDeferredPreemption") {
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    extend T::Sig\n"
                 "    sig {returns(String)}\n"
                 "    def value\n"
                 "      \"old\"\n"
                 "    end\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\n"
                 "module Root\n"
                 "  class B\n"
                 "  end\n"
                 "end\n"},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "  class Consumer\n"
                     "    def value\n"
                     "      Root::A.new.value\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
    };
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    // Pause after resolving each stratum (before its preemption check), to give the newer edit time to arrive while
    // the preemption is deferred.
    opts->sleepInSlowPathSeconds = 1;
    resetState(opts);
    writeFilesToFS(files);
    for (auto &[path, _] : files) {
        lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", rootPath, path));
    }
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class B\n"
                          "    class Inner\n"
                          "      def bar\n"
                          "        Root::A.n\n"
                          "      end\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2, /* cancellationExpected */ false, /* preemptionsExpected */ 1));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // This fast path also typechecks foo/a.rb, so its preemption is deferred from Root's stratum to Root::Foo's.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def value\n"
                          "      1\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2));
    setSlowPathBlocked(false);

    // Root's stratum (and with it, the deferral) is done once its diagnostics are published.
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
    }

    // Root::Foo's stratum is now sleeping before its preemption check. This edit gets merged into the deferred one.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def value\n"
                          "      \"not an integer\"\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          3));

    // The newer edit has a return type error, which is only reported if it was typechecked.
    bool sawNewerEditError = false;
    for (int i = 0; i < 12 && !sawNewerEditError; i++) {
        auto msg = readAsync();
        REQUIRE(msg->isNotification());
        if (msg->asNotification().method == LSPMethod::TextDocumentPublishDiagnostics) {
            auto params = getPublishDiagnosticParams(msg->asNotification());
            REQUIRE(params.has_value());
            sawNewerEditError = (*params)->uri == getUri("a.rb") && !(*params)->diagnostics.empty();
        } else if (msg->asNotification().method == LSPMethod::SorbetTypecheckRunInfo) {
            auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(msg->asNotification().params);
            if (runInfo->typecheckingPath == TypecheckingPath::Slow &&
                runInfo->status == SorbetTypecheckRunStatus::Ended) {
                break;
            }
        }
    }
    CHECK(sawNewerEditError);
}

// When a fast path edit's preemption is deferred to a later stratum and a slow path edit arrives behind it (with a
// non-edit request in between, so LSPPreprocessor doesn't merge them), the processing thread cancels the scheduled
// preemption, moves the edits to the front of the queue, and merges them into one slow path edit.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedSlowPathEditPromotedPastDeferredPreemption") {
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    extend T::Sig\n"
                 "    sig {returns(String)}\n"
                 "    def value\n"
                 "      \"old\"\n"
                 "    end\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\n"
                 "module Root\n"
                 "  class B\n"
                 "  end\n"
                 "end\n"},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "  class Consumer\n"
                     "    def value\n"
                     "      Root::A.new.value\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
    };
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    // Pause after resolving each stratum, to give the later requests time to arrive while the preemption is deferred.
    opts->sleepInSlowPathSeconds = 1;
    resetState(opts);
    writeFilesToFS(files);
    for (auto &[path, _] : files) {
        lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", rootPath, path));
    }
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class B\n"
                          "    class Inner\n"
                          "      def bar\n"
                          "        Root::A.n\n"
                          "      end\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // This fast path also typechecks foo/a.rb, so its preemption is deferred from Root's stratum to Root::Foo's.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def value\n"
                          "      1\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2));
    setSlowPathBlocked(false);

    // Root's stratum (and with it, the deferral) is done once its diagnostics are published.
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
    }

    // While Root::Foo's stratum sleeps: a request (which stops LSPPreprocessor from merging the edits), then a slow
    // path edit with a type error.
    sendAsync(*hover("a.rb", 2, 9));
    sendAsync(*changeFile("b.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class B\n"
                          "    class Inner\n"
                          "      def bar\n"
                          "        Root::A.n\n"
                          "      end\n"
                          "    end\n"
                          "    class AnotherNewClass; end\n"
                          "    T.let(1, String)\n"
                          "  end\n"
                          "end\n",
                          3));

    bool sawHover = false;
    bool sawNewerEditError = false;
    for (int i = 0; i < 20 && !(sawHover && sawNewerEditError); i++) {
        auto msg = readAsync();
        if (msg->isResponse()) {
            sawHover = sawHover || msg->asResponse().requestMethod == LSPMethod::TextDocumentHover;
        } else if (msg->asNotification().method == LSPMethod::TextDocumentPublishDiagnostics) {
            auto params = getPublishDiagnosticParams(msg->asNotification());
            REQUIRE(params.has_value());
            if ((*params)->uri == getUri("b.rb")) {
                sawNewerEditError = absl::c_any_of((*params)->diagnostics, [](auto &diag) {
                    return absl::StrContains(diag->message, "does not have asserted type");
                });
            }
        }
    }
    CHECK(sawHover);
    CHECK(sawNewerEditError);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedDeferredEditMergedWithNoOp") {
    const string updatedA = "# typed: true\n"
                            "module Root\n"
                            "  class A\n"
                            "    extend T::Sig\n"
                            "    sig {returns(Integer)}\n"
                            "    def value = 1\n"
                            "  end\n"
                            "end\n";
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    extend T::Sig\n"
                 "    sig {returns(String)}\n"
                 "    def value = \"old\"\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\nmodule Root\n  class B; end\nend\n"},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\nmodule Root::Foo\n  class Consumer\n"
                     "    def value = Root::A.new.value\n"
                     "  end\nend\n"},
    };
    initializePackagedWorkspace(true, files, 1);
    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\nmodule Root\n  class B\n    class Inner\n"
                          "      def bar = Root::A.n\n"
                          "    end\n"
                          "  end\nend\n",
                          2, /* cancellationExpected */ false, /* preemptionsExpected */ 1));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // This edit is indexed in Root, but its fast path is deferred until Root::Foo.
    sendAsync(*changeFile("a.rb", updatedA, 2));
    setSlowPathBlocked(false);
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
        auto params = getPublishDiagnosticParams(diag->asNotification());
        REQUIRE(params.has_value());
        CHECK_EQ((*params)->uri, getUri("b.rb"));
    }

    // The newer edit is identical to the indexed file, so indexing filters it out after it merges into the
    // deferred task. It still counts as a message with a diagnostic latency timer.
    sendAsync(*changeFile("a.rb", updatedA, 3));
    send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20)));
    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 0);
    CHECK_EQ(readFile(getUri("a.rb")), updatedA);
}

TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedDeferredEditMergedIntoRunSpecial") {
    const string updatedB = "# typed: true\nmodule Root\n  class B\n    class Inner\n"
                            "      def bar = Root::A.n\n"
                            "    end\n"
                            "    class AnotherNewClass; end\n"
                            "  end\nend\n";
    const string updatedFoo = "# typed: true\nmodule Root::Foo\n  class Consumer\n"
                              "    def value = Root::A.new.value\n"
                              "  end\nend\n# latest edit\n";
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\nmodule Root\n  class A\n"
                 "    def value = 1\n"
                 "  end\nend\n"},
        {"b.rb", "# typed: true\nmodule Root\n  class B; end\nend\n"},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\nmodule Root::Foo\n  class Consumer\n"
                     "    def value = Root::A.new.value\n"
                     "  end\nend\n"},
    };
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    opts->sleepInSlowPathSeconds = 1;
    opts->lspMaxFilesOnFastPath = 1;
    resetState(opts);
    writeFilesToFS(files);
    for (auto &[path, _] : files) {
        lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", rootPath, path));
    }
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});
    getCounters();

    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});
    assertErrorDiagnostics(send(*openFile("foo/a.rb", files[4].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\nmodule Root\n  class B\n    class Inner\n"
                          "      def bar = Root::A.n\n"
                          "    end\n"
                          "  end\nend\n",
                          2));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // This comment edit is indexed while Root is running, then deferred until Root::Foo's stratum.
    sendAsync(*changeFile("foo/a.rb", updatedFoo, 2));
    setSlowPathBlocked(false);
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
        auto params = getPublishDiagnosticParams(diag->asNotification());
        REQUIRE(params.has_value());
        CHECK_EQ((*params)->uri, getUri("b.rb"));
    }

    // The newer slow edit merges into the deferred task. Together they exceed the one-file indexing limit,
    // so the merged task must commit both edits through runSpecial.
    sendAsync(*changeFile("b.rb", updatedB, 3));
    send(LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, 20)));
    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.messages.processed", "sorbet.mergedEdits"), 1);
    CHECK_EQ(readFile(getUri("b.rb")), updatedB);
    CHECK_EQ(readFile(getUri("foo/a.rb")), updatedFoo);
}

// Like PackageDirectedSlowPathEditPromotedPastDeferredPreemption, but a newer fast path edit is first merged into the
// deferred edit (without canceling the slow path), before the slow path edit is promoted past it.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedFastMergeThenSlowEditPromotedPastDeferredPreemption") {
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    extend T::Sig\n"
                 "    sig {returns(String)}\n"
                 "    def value\n"
                 "      \"old\"\n"
                 "    end\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\n"
                 "module Root\n"
                 "  class B\n"
                 "  end\n"
                 "end\n"},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "  class Consumer\n"
                     "    def value\n"
                     "      Root::A.new.value\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
    };
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    // Pause after resolving each stratum, to give the later requests time to arrive while the preemption is deferred.
    opts->sleepInSlowPathSeconds = 1;
    resetState(opts);
    writeFilesToFS(files);
    for (auto &[path, _] : files) {
        lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", rootPath, path));
    }
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class B\n"
                          "    class Inner\n"
                          "      def bar\n"
                          "        Root::A.n\n"
                          "      end\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // This fast path also typechecks foo/a.rb, so its preemption is deferred from Root's stratum to Root::Foo's.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def value\n"
                          "      1\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2));
    setSlowPathBlocked(false);

    // Root's stratum (and with it, the deferral) is done once its diagnostics are published.
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
    }

    // While Root::Foo's stratum sleeps: a newer fast path edit (merged into the deferred one), a request (which stops
    // LSPPreprocessor from merging the edits that follow), then a slow path edit with a type error.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def value\n"
                          "      \"not an integer\"\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          3));
    sendAsync(*hover("a.rb", 2, 9));
    sendAsync(*changeFile("b.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class B\n"
                          "    class Inner\n"
                          "      def bar\n"
                          "        Root::A.n\n"
                          "      end\n"
                          "    end\n"
                          "    class AnotherNewClass; end\n"
                          "    T.let(1, String)\n"
                          "  end\n"
                          "end\n",
                          4));

    bool sawHover = false;
    bool sawNewerEditError = false;
    bool sawMergedEditError = false;
    for (int i = 0; i < 20 && !(sawHover && sawNewerEditError && sawMergedEditError); i++) {
        auto msg = readAsync();
        if (msg->isResponse()) {
            sawHover = sawHover || msg->asResponse().requestMethod == LSPMethod::TextDocumentHover;
        } else if (msg->asNotification().method == LSPMethod::TextDocumentPublishDiagnostics) {
            auto params = getPublishDiagnosticParams(msg->asNotification());
            REQUIRE(params.has_value());
            if ((*params)->uri == getUri("a.rb")) {
                sawMergedEditError = !(*params)->diagnostics.empty();
            }
            if ((*params)->uri == getUri("b.rb")) {
                sawNewerEditError = absl::c_any_of((*params)->diagnostics, [](auto &diag) {
                    return absl::StrContains(diag->message, "does not have asserted type");
                });
            }
        }
    }
    CHECK(sawHover);
    CHECK(sawNewerEditError);
    CHECK(sawMergedEditError);
}

// A deferred fast path edit can absorb newer edits that push it over the fast path file limit, so it commits through
// runSpecial (as a slow path) once the running slow path finishes. If a newer slow path edit then cancels that slow
// path, the replacement slow path must still include the deferred edit's changes.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedCancelSlowPathOfDeferredEditMergedIntoRunSpecial") {
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    extend T::Sig\n"
                 "    sig {returns(String)}\n"
                 "    def value\n"
                 "      \"old\"\n"
                 "    end\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\n"
                 "module Root\n"
                 "  class B\n"
                 "  end\n"
                 "end\n"},
        {"c.rb", "# typed: true\nmodule Root\n  class C; end\nend\n"},
        {"d.rb", "# typed: true\nmodule Root\n  class D; end\nend\n"},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "  class Consumer\n"
                     "    def value\n"
                     "      Root::A.new.value\n"
                     "    end\n"
                     "  end\n"
                     "end\n"},
    };
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    // a.rb plus its one extra file fit on the fast path, but a.rb, c.rb, and d.rb together don't.
    opts->lspMaxFilesOnFastPath = 2;
    // Pause after resolving each stratum, to give later edits time to arrive.
    opts->sleepInSlowPathSeconds = 1;
    resetState(opts);
    writeFilesToFS(files);
    for (auto &[path, _] : files) {
        lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", rootPath, path));
    }
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});
    assertErrorDiagnostics(send(*openFile("c.rb", files[3].second)), {});
    assertErrorDiagnostics(send(*openFile("d.rb", files[4].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class B\n"
                          "    class Inner\n"
                          "      def bar\n"
                          "        Root::A.n\n"
                          "      end\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // This fast path also typechecks foo/a.rb, so its preemption is deferred from Root's stratum to Root::Foo's.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def value\n"
                          "      1\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2));
    setSlowPathBlocked(false);

    // Root's stratum (and with it, the deferral) is done once its diagnostics are published.
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
    }

    // These merge into the deferred edit, and together they exceed the fast path file limit.
    // Each is a fast path edit on its own (a comment change).
    sendAsync(*changeFile("c.rb", "# typed: true\nmodule Root\n  class C; end\nend\n# edited\n", 2));
    sendAsync(*changeFile("d.rb", "# typed: true\nmodule Root\n  class D; end\nend\n# edited\n", 2));

    // Once the first slow path is done (or canceled), the merged edit's slow path starts. Cancel that one with a newer
    // slow path edit, and wait for the replacement slow path to end.
    bool firstSlowPathEnded = false;
    bool sentCancelingEdit = false;
    bool sawCancellation = false;
    bool replacementEnded = false;
    for (int i = 0; i < 40 && !replacementEnded; i++) {
        auto msg = readAsync();
        if (!msg->isNotification() || msg->asNotification().method != LSPMethod::SorbetTypecheckRunInfo) {
            continue;
        }
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(msg->asNotification().params);
        if (runInfo->typecheckingPath != TypecheckingPath::Slow) {
            continue;
        }
        if (!firstSlowPathEnded) {
            // The first slow path either finishes, or the merged edit cancels it.
            REQUIRE_NE(runInfo->status, SorbetTypecheckRunStatus::Started);
            firstSlowPathEnded = true;
        } else if (!sentCancelingEdit) {
            REQUIRE_EQ(runInfo->status, SorbetTypecheckRunStatus::Started);
            sendAsync(*changeFile("b.rb",
                                  "# typed: true\n"
                                  "module Root\n"
                                  "  class B\n"
                                  "    class Inner\n"
                                  "      def bar\n"
                                  "        Root::A.n\n"
                                  "      end\n"
                                  "    end\n"
                                  "    class AnotherNewClass; end\n"
                                  "  end\n"
                                  "end\n",
                                  3));
            sentCancelingEdit = true;
        } else if (runInfo->status == SorbetTypecheckRunStatus::Cancelled) {
            sawCancellation = true;
        } else if (runInfo->status == SorbetTypecheckRunStatus::Ended) {
            replacementEnded = true;
        }
    }
    REQUIRE(sawCancellation);
    REQUIRE(replacementEnded);

    // The typechecker must have a.rb's newer signature.
    sendAsync(*hover("a.rb", 5, 8));
    unique_ptr<LSPMessage> resp;
    for (int i = 0; i < 10; i++) {
        resp = readAsync();
        if (resp->isResponse()) {
            break;
        }
    }
    REQUIRE(resp->isResponse());
    auto &hoverResult =
        get<unique_ptr<Hover>>(get<variant<JSONNullObject, unique_ptr<Hover>>>(*resp->asResponse().result));
    REQUIRE(hoverResult);
    CHECK(absl::StrContains(hoverResult->contents->value, "Integer"));
}

// Two fast path edits that each fit within the fast path file limit, but whose extra files together don't, must not
// take the fast path once they're merged.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedMergedFastEditsExtraFilesExceedFileLimit") {
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder().withName("Root").withExports({"Root::A"}).build()},
        {"a.rb", "# typed: true\n"
                 "module Root\n"
                 "  class A\n"
                 "    extend T::Sig\n"
                 "    sig {returns(String)}\n"
                 "    def x = \"x\"\n"
                 "    sig {returns(String)}\n"
                 "    def y = \"y\"\n"
                 "  end\n"
                 "end\n"},
        {"b.rb", "# typed: true\nmodule Root\n  class B; end\nend\n"},
        {"f2.rb", "# typed: true\nmodule Root\n  class F2\n    def call = Root::A.new.y\n  end\nend\n"},
        {"f3.rb", "# typed: true\nmodule Root\n  class F3\n    def call = Root::A.new.y\n  end\nend\n"},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root"}).build()},
        {"foo/f1.rb", "# typed: true\nmodule Root::Foo\n  class F1\n    def call = Root::A.new.x\n  end\nend\n"},
    };
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    opts->packageDirected = true;
    // Each edit below checks at most 3 files, but merged, they check 4.
    opts->lspMaxFilesOnFastPath = 3;
    // Pause after resolving each stratum, to give the newer edit time to arrive while the preemption is deferred.
    opts->sleepInSlowPathSeconds = 1;
    resetState(opts);
    writeFilesToFS(files);
    for (auto &[path, _] : files) {
        lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/{}", rootPath, path));
    }
    auto initOptions = make_unique<SorbetInitializationOptions>();
    initOptions->enableTypecheckInfo = true;
    assertErrorDiagnostics(
        initializeLSP(true /* supportsMarkdown */, true /* supportsCodeActionResolve */, move(initOptions)), {});

    assertErrorDiagnostics(send(*openFile("a.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("b.rb", files[2].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("b.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class B\n"
                          "    class Inner\n"
                          "      def bar\n"
                          "        Root::A.n\n"
                          "      end\n"
                          "    end\n"
                          "  end\n"
                          "end\n",
                          2));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // Changing A#x's signature pulls in foo/f1.rb, so this fast path is deferred to Root::Foo's stratum.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def x = 1\n"
                          "    sig {returns(String)}\n"
                          "    def y = \"y\"\n"
                          "  end\n"
                          "end\n",
                          2));
    setSlowPathBlocked(false);

    // Root's stratum (and with it, the deferral) is done once its diagnostics are published.
    {
        auto diag = readAsync();
        REQUIRE(diag->isNotification());
        REQUIRE_EQ(diag->asNotification().method, LSPMethod::TextDocumentPublishDiagnostics);
    }

    // Changing A#y's signature too pulls in f2.rb and f3.rb. This merges into the deferred edit.
    sendAsync(*changeFile("a.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class A\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def x = 1\n"
                          "    sig {returns(Integer)}\n"
                          "    def y = 2\n"
                          "  end\n"
                          "end\n",
                          3));

    bool sawOversizedFastPath = false;
    bool slowPathEnded = false;
    for (int i = 0; i < 30 && !slowPathEnded; i++) {
        auto msg = readAsync();
        if (!msg->isNotification() || msg->asNotification().method != LSPMethod::SorbetTypecheckRunInfo) {
            continue;
        }
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(msg->asNotification().params);
        if (runInfo->status != SorbetTypecheckRunStatus::Ended) {
            continue;
        }
        if (runInfo->typecheckingPath == TypecheckingPath::Fast) {
            auto checked = [&](string_view path) {
                return absl::c_linear_search(runInfo->filesTypechecked, fmt::format("{}/{}", rootPath, path));
            };
            sawOversizedFastPath =
                sawOversizedFastPath || (checked("foo/f1.rb") && checked("f2.rb") && checked("f3.rb"));
        } else {
            slowPathEnded = true;
        }
    }
    REQUIRE(slowPathEnded);
    CHECK_FALSE(sawOversizedFastPath);
}

// A queued edit's cached typechecking path is computed against the indexer's files at the time. If an edit ahead of
// it in the queue is indexed first (here, by the same preemption), the cached result can be stale. In particular, the
// cache can say that the fast path won't run the incremental namer when it will, in which case preemption doesn't wait
// for the strata of the package files that the fast path rechecks.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedPreemptionAfterEarlierEditIndexed") {
    const string originalX = "# typed: true\n"
                             "module Root::Bar\n"
                             "  class X; end\n"
                             "end\n";
    vector<pair<string, string>> files = {
        {"__package.rb", PackageTextBuilder()
                             .withName("Root")
                             .withExports({"Test::Root::Helper"})
                             .withTestImports({"Root::Foo"})
                             .build()},
        {"z.rb", "# typed: true\n"
                 "module Root\n"
                 "  class Z\n"
                 "    def m = 1\n"
                 "  end\n"
                 "end\n"},
        {"test/helper.rb", "# typed: true\n"
                           "class Test::Root::Helper\n"
                           "end\n"},
        {"bar/__package.rb", PackageTextBuilder().withName("Root::Bar").build()},
        {"bar/x.rb", originalX},
        {"foo/__package.rb", PackageTextBuilder().withName("Root::Foo").withImports({"Root::Baz"}).build()},
        {"foo/a.rb", "# typed: true\n"
                     "module Root::Foo\n"
                     "end\n"},
        {"baz/__package.rb", PackageTextBuilder().withName("Root::Baz").build()},
        {"baz/b.rb", "# typed: true\n"
                     "module Root::Baz\n"
                     "end\n"},
    };
    // Strata: Root's, Root::Bar's, and Root::Baz's application code are first. Root::Bar's `__package.rb` (its test
    // stratum) and Root::Foo's application code are second. Root's `__package.rb` (its test stratum, which imports
    // Root::Foo) is third.
    //
    // Pause after resolving each stratum, so that every edit is queued before the first preemption runs.
    initializePackagedWorkspace(true, files, 1);
    assertErrorDiagnostics(send(*openFile("z.rb", files[1].second)), {});
    assertErrorDiagnostics(send(*openFile("bar/x.rb", originalX)), {});
    assertErrorDiagnostics(send(*openFile("baz/b.rb", files[8].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("baz/b.rb",
                          "# typed: true\n"
                          "module Root::Baz\n"
                          "  class NewClassTakesSlowPath; end\n"
                          "end\n",
                          2, /* cancellationExpected */ false, /* preemptionsExpected */ 1));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // E1 adds a method, so it uses the incremental namer and waits for Root::Bar's `__package.rb` (second stratum).
    sendAsync(*changeFile("bar/x.rb",
                          "# typed: true\n"
                          "module Root::Bar\n"
                          "  class X\n"
                          "    def new_method; end\n"
                          "  end\n"
                          "end\n",
                          2));
    // Keeps LSPPreprocessor from merging E1 and E2.
    sendAsync(*hover("bar/x.rb", 2, 8));
    // E2 merges two edits: one reverts E1, and one changes a method body in Root. Relative to the files before E1, it
    // changes no symbols, so it wouldn't use the incremental namer. Relative to E1, it deletes a method, so it does,
    // and it rechecks Root's `__package.rb` (third stratum).
    sendAsync(*changeFile("bar/x.rb", originalX, 3));
    sendAsync(*changeFile("z.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class Z\n"
                          "    def m = 2\n"
                          "  end\n"
                          "end\n",
                          2));
    setSlowPathBlocked(false);

    auto fastPathsEnded = 0;
    auto sawHover = false;
    auto slowPathEnded = false;
    for (int i = 0; i < 20 && !slowPathEnded; i++) {
        auto msg = readAsync();
        if (msg->isResponse()) {
            sawHover = true;
            continue;
        }
        REQUIRE(msg->isNotification());
        auto &notif = msg->asNotification();
        if (notif.method == LSPMethod::TextDocumentPublishDiagnostics) {
            auto params = getPublishDiagnosticParams(notif);
            REQUIRE(params.has_value());
            if (!(*params)->diagnostics.empty()) {
                FAIL_CHECK(fmt::format("Expected no errors, but received diagnostics:\n{}", msg->toJSON(true)));
            }
            continue;
        }
        REQUIRE_EQ(notif.method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(notif.params);
        if (runInfo->status != SorbetTypecheckRunStatus::Ended) {
            continue;
        }
        if (runInfo->typecheckingPath == TypecheckingPath::Fast) {
            fastPathsEnded++;
        } else {
            slowPathEnded = true;
        }
    }
    REQUIRE(slowPathEnded);
    CHECK(sawHover);
    CHECK_EQ(fastPathsEnded, 2);
}

// Like PackageDirectedPreemptionAfterEarlierEditIndexed, but the cached result's extra files are what go stale: an edit
// ahead of this one in the queue makes another file mention a method that this edit changes.
TEST_CASE_FIXTURE(MultithreadedProtocolTest, "PackageDirectedPreemptionAfterEarlierEditChangesExtraFiles") {
    const string originalZ = "# typed: true\n"
                             "module Root\n"
                             "  class Z\n"
                             "    def m = 1\n"
                             "  end\n"
                             "end\n";
    vector<pair<string, string>> files = {
        {"__package.rb",
         PackageTextBuilder().withName("Root").withImports({"Root::Bar"}).withExports({"Test::Root::Helper"}).build()},
        {"z.rb", originalZ},
        {"test/helper.rb", "# typed: true\n"
                           "class Test::Root::Helper\n"
                           "end\n"},
        {"bar/__package.rb", PackageTextBuilder().withName("Root::Bar").withExports({"Root::Bar::X"}).build()},
        {"bar/x.rb", "# typed: true\n"
                     "module Root::Bar\n"
                     "  class X\n"
                     "    extend T::Sig\n"
                     "    sig {returns(String)}\n"
                     "    def foo = \"foo\"\n"
                     "  end\n"
                     "end\n"},
        {"bar/y.rb", "# typed: true\n"
                     "module Root::Bar\n"
                     "end\n"},
    };
    // Strata: Root::Bar's application code is first. Root::Bar's `__package.rb` (its test stratum) and Root's
    // application code (which imports Root::Bar) are second. Root's `__package.rb` (its test stratum) is third.
    //
    // Pause after resolving each stratum, so that every edit is queued before the first preemption runs.
    initializePackagedWorkspace(true, files, 1);
    assertErrorDiagnostics(send(*openFile("z.rb", originalZ)), {});
    assertErrorDiagnostics(send(*openFile("bar/x.rb", files[4].second)), {});
    assertErrorDiagnostics(send(*openFile("bar/y.rb", files[5].second)), {});

    setSlowPathBlocked(true);
    sendAsync(*changeFile("bar/y.rb",
                          "# typed: true\n"
                          "module Root::Bar\n"
                          "  class NewClassTakesSlowPath; end\n"
                          "end\n",
                          2, /* cancellationExpected */ false, /* preemptionsExpected */ 1));
    {
        auto status = getTypecheckRunStatus(*readAsync());
        REQUIRE(status.has_value());
        REQUIRE_EQ(*status, SorbetTypecheckRunStatus::Started);
    }

    // E2 only changes a method body, so it doesn't use the incremental namer. It waits for z.rb's stratum (the second),
    // but not for Root's `__package.rb`. It makes z.rb mention `foo`.
    sendAsync(*changeFile("z.rb",
                          "# typed: true\n"
                          "module Root\n"
                          "  class Z\n"
                          "    def m = Root::Bar::X.new.foo\n"
                          "  end\n"
                          "end\n",
                          2));
    // Keeps LSPPreprocessor from merging E2 and E3.
    sendAsync(*hover("z.rb", 3, 8));
    // E3 changes `foo`'s signature. Relative to the files before E2, no other file mentions `foo`. Relative to E2, z.rb
    // does, so E3's fast path also typechecks z.rb, and rechecks Root's `__package.rb` (third stratum).
    sendAsync(*changeFile("bar/x.rb",
                          "# typed: true\n"
                          "module Root::Bar\n"
                          "  class X\n"
                          "    extend T::Sig\n"
                          "    sig {returns(Integer)}\n"
                          "    def foo = 1\n"
                          "  end\n"
                          "end\n",
                          2));
    setSlowPathBlocked(false);

    auto fastPathsEnded = 0;
    auto sawHover = false;
    auto slowPathEnded = false;
    for (int i = 0; i < 20 && !slowPathEnded; i++) {
        auto msg = readAsync();
        if (msg->isResponse()) {
            sawHover = true;
            continue;
        }
        REQUIRE(msg->isNotification());
        auto &notif = msg->asNotification();
        if (notif.method == LSPMethod::TextDocumentPublishDiagnostics) {
            auto params = getPublishDiagnosticParams(notif);
            REQUIRE(params.has_value());
            if (!(*params)->diagnostics.empty()) {
                FAIL_CHECK(fmt::format("Expected no errors, but received diagnostics:\n{}", msg->toJSON(true)));
            }
            continue;
        }
        REQUIRE_EQ(notif.method, LSPMethod::SorbetTypecheckRunInfo);
        auto &runInfo = get<unique_ptr<SorbetTypecheckRunInfo>>(notif.params);
        if (runInfo->status != SorbetTypecheckRunStatus::Ended) {
            continue;
        }
        if (runInfo->typecheckingPath == TypecheckingPath::Fast) {
            fastPathsEnded++;
        } else {
            slowPathEnded = true;
        }
    }
    REQUIRE(slowPathEnded);
    CHECK(sawHover);
    CHECK_EQ(fastPathsEnded, 2);
}

} // namespace sorbet::test::lsp
