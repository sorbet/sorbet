#include "gtest/gtest.h"
// has to go first as it violates our requirements

#include "common/sort.h"
#include "core/Error.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/lsp.h"
#include "payload/payload.h"
#include "spdlog/sinks/null_sink.h"
#include "test/helpers/MockFileSystem.h"
#include "test/helpers/lsp.h"
#include <climits>
#include <memory>

using namespace std;
using namespace sorbet::test;

namespace sorbet::realmain::lsp::test {

namespace {

options::Options makeOptions(string_view rootPath) {
    options::Options opts;
    opts.rawInputDirNames.emplace_back(string(rootPath));
    opts.runLSP = true;
    opts.fs = make_shared<sorbet::test::MockFileSystem>(rootPath);
    return opts;
}

auto nullSink = make_shared<spd::sinks::null_sink_mt>();
auto logger = make_shared<spd::logger>("console", nullSink);
auto typeErrorsConsole = make_shared<spd::logger>("typeDiagnostics", nullSink);
auto nullOpts = makeOptions("");
auto workers = WorkerPool::create(0, *logger);

shared_ptr<LSPConfiguration> makeConfig(const options::Options &opts = nullOpts, bool enableShowOpNotifs = false,
                                        bool initialize = true) {
    auto config = make_shared<LSPConfiguration>(opts, make_shared<LSPOutputToVector>(), logger, true, false);
    InitializeParams initParams("", "", make_unique<ClientCapabilities>());
    initParams.initializationOptions = make_unique<SorbetInitializationOptions>();
    initParams.initializationOptions.value()->supportsOperationNotifications = enableShowOpNotifs;
    config->setClientConfig(make_shared<LSPClientConfiguration>(initParams));
    if (initialize) {
        config->markInitialized();
    }
    return config;
}

unique_ptr<core::GlobalState> makeGS(const options::Options &opts = nullOpts) {
    auto gs = make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger)));
    unique_ptr<KeyValueStore> kvstore;
    payload::createInitialGlobalState(gs, opts, kvstore);
    gs->errorQueue->ignoreFlushes = true;
    return gs;
}

auto nullConfig = makeConfig();

LSPPreprocessor makePreprocessor(const shared_ptr<LSPConfiguration> &config = nullConfig, u4 initialVersion = 0) {
    return LSPPreprocessor(config, initialVersion);
}

unique_ptr<LSPMessage> makeWatchman(vector<string> files) {
    auto params = make_unique<WatchmanQueryResponse>("", "", false, files);
    auto msg = make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(params)));
    return msg;
}

optional<SorbetWorkspaceEditParams *> getUpdates(QueueState &state, int i) {
    EXPECT_LT(i, state.pendingRequests.size());
    if (i >= state.pendingRequests.size()) {
        return nullopt;
    }
    auto &msg = state.pendingRequests[i];
    EXPECT_TRUE(msg->isNotification() && msg->method() == LSPMethod::SorbetWorkspaceEdit);
    auto &params = get<unique_ptr<SorbetWorkspaceEditParams>>(msg->asNotification().params);
    return params.get();
}

unique_ptr<LSPMessage> makeHoverReq(int id, string_view file, int line = 0, int col = 0) {
    auto params = make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(string(file)),
                                                          make_unique<Position>(line, col));
    return make_unique<LSPMessage>(make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentHover, move(params)));
}

unique_ptr<LSPMessage> makeCancel(int id) {
    auto params = make_unique<CancelParams>(id);
    return make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::$CancelRequest, move(params)));
}

class CountingTask final : public core::lsp::Task {
public:
    int runCount = 0;
    shared_ptr<core::lsp::PreemptionTaskManager> preemptManager;
    core::GlobalState &gs;

    CountingTask(shared_ptr<core::lsp::PreemptionTaskManager> preemptManager, core::GlobalState &gs)
        : preemptManager(move(preemptManager)), gs(gs) {}

    void run() override {
        // The task should run with typecheck mutex held with a write lock.
        preemptManager->assertTypecheckMutexHeld();
        // Emulate behavior of most LSP Tasks and drain all diagnostics and query responses.
        // This should never drain error queue items from the preempted task.
        gs.errorQueue->drainWithQueryResponses();
        EXPECT_TRUE(gs.errorQueue->ignoreFlushes);
        runCount++;
    }
};

} // namespace

TEST(LSPPreprocessor, IgnoresWatchmanUpdatesFromOpenFiles) { // NOLINT
    auto opts = makeOptions("");
    auto preprocessor = makePreprocessor(makeConfig(opts));
    QueueState state;
    absl::Mutex mtx;

    string fileContents = "# typed: true\n1+1";
    opts.fs->writeFile("foo.rb", "");
    preprocessor.preprocessAndEnqueue(state, makeOpen("foo.rb", fileContents, 1), mtx);
    preprocessor.preprocessAndEnqueue(state, makeWatchman({"foo.rb"}), mtx);

    ASSERT_EQ(1, state.pendingRequests.size());

    const auto updates = getUpdates(state, 0).value();
    // Version didn't change because it ignored the watchman update.
    EXPECT_EQ(updates->mergeCount, 0);
    ASSERT_EQ(updates->updates.size(), 1);
    EXPECT_EQ(updates->updates[0]->source(), fileContents);
}

// When deepCopying initialGS for typechecking, it should always have all previous updates applied to it.
TEST(LSPPreprocessor, ClonesTypecheckingGSAtCorrectLogicalTime) { // NOLINT
    string fileV1 = "# typed: true";
    // V1 => V2: Slow path
    string fileV2 = "# typed: true\ndef foo; end";
    // V2 => V3: Fast path
    string fileV3 = "# typed: true\ndef foo; 1 + 1; end";
    // V3 => V4: Slow path
    string fileV4 = "# typed: true\ndef foo; 1 + 1; end; def bar; end";
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;

    // Apply one update, slow path.
    preprocessor.preprocessAndEnqueue(state, makeOpen("foo.rb", fileV1, 1), mtx);
    // Pop it off the queue to prevent a merge.
    state.pendingRequests.clear();
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV2, 2), mtx);
    {
        const auto updates = getUpdates(state, 0).value();
        // Should have the newest version of the update.
        EXPECT_EQ(fileV2, updates->updates[0]->source());
    }

    // Append another edit that will get merged with the existing edit.
    // V3 will take the fast path relative to V2, forcing a situation in which a new global state will need to be
    // created.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV3, 3), mtx);
    {
        // Should have the newest version of the update.
        const auto updates = getUpdates(state, 0).value();
        EXPECT_EQ(fileV3, updates->updates[0]->source());
    }

    // Append another edit that will get merged with the existing edit.
    // V4 will take the slow path relative to V3, and the global state created for it should be re-used.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV4, 4), mtx);
    {
        const auto updates = getUpdates(state, 0).value();
        EXPECT_EQ(fileV4, updates->updates[0]->source());
    }
}

TEST(LSPPreprocessor, PauseAndResume) { // NOLINT
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;
    {
        auto msg = make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, JSONNullObject()));
        preprocessor.preprocessAndEnqueue(state, move(msg), mtx);
    }
    EXPECT_TRUE(state.paused);
    {
        auto msg =
            make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, JSONNullObject()));
        preprocessor.preprocessAndEnqueue(state, move(msg), mtx);
    }
    EXPECT_FALSE(state.paused);
}

TEST(LSPPreprocessor, Exit) { // NOLINT
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;
    auto msg = make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::Exit, JSONNullObject()));
    preprocessor.preprocessAndEnqueue(state, move(msg), mtx);
    EXPECT_TRUE(state.terminate);
    EXPECT_EQ(state.errorCode, 0);
}

TEST(LSPPreprocessor, Initialized) { // NOLINT
    QueueState state;
    absl::Mutex mtx;
    auto options = makeOptions("");
    auto config = makeConfig(options, true, false);
    auto preprocessor = makePreprocessor(config);
    auto output = dynamic_pointer_cast<LSPOutputToVector>(config->output);

    // Sending a request prior to initialization should cause an error.
    EXPECT_FALSE(config->isInitialized());
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(1, "foo.rb", 1, 1), mtx);
    auto errorMsgs = output->getOutput();
    ASSERT_EQ(1, errorMsgs.size());
    ASSERT_TRUE(errorMsgs[0]->isResponse());
    ASSERT_TRUE(errorMsgs[0]->asResponse().error.has_value());

    auto msg = make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::Initialized, make_unique<InitializedParams>()));
    preprocessor.preprocessAndEnqueue(state, move(msg), mtx);
    EXPECT_TRUE(config->isInitialized());

    ASSERT_EQ(1, state.pendingRequests.size());
    EXPECT_EQ(state.pendingRequests[0]->method(), LSPMethod::Initialized);
}

// When a request in the queue is canceled, the preprocessor should merge any edits that happen immediately before and
// after the canceled request.
TEST(LSPPreprocessor, MergesFileUpdatesProperlyAfterCancelation) { // NOLINT
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;
    // V1: Slow path.
    string fileV1 = "# typed: true\ndef foo; end";
    // V1 => V2: Fast path
    string fileV2 = "# typed: true\ndef foo; 1 + 1; end";
    // V2 => V3: Fast path
    string fileV3 = "# typed: true\ndef foo; 1 + 2; end";
    // V3 => V4: Fast path
    string fileV4 = "# typed: true\ndef foo; 1 + 3; end";
    // V4 => V5: Slow path
    string fileV5 = "# typed: true\ndef foo2; 1 + 3; end";
    int id = 0;
    string barV1 = "2+3+4";

    preprocessor.preprocessAndEnqueue(state, makeOpen("foo.rb", fileV1, 1), mtx);
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(id++, "foo.rb", 0, 0), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV2, 2), mtx);
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(id++, "foo.rb", 0, 0), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV3, 3), mtx);
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(id++, "foo.rb", 0, 0), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV4, 4), mtx);
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(id++, "foo.rb", 0, 0), mtx);
    preprocessor.preprocessAndEnqueue(state, makeOpen("bar.rb", barV1, 1), mtx);

    vector<pair<int, string>> messageContents = {{0, fileV1}, {2, fileV2}, {4, fileV3}, {6, fileV4}};
    for (auto &[messageId, contents] : messageContents) {
        auto updates = getUpdates(state, messageId).value();
        EXPECT_EQ("foo.rb", updates->updates[0]->path());
        EXPECT_EQ(contents, updates->updates[0]->source());
    }

    // Cancel hover requests, and ensure that initialGS has the proper value of foo.rb
    vector<pair<int, string>> entries = {
        {0, fileV2},
        {1, fileV3},
        {2, fileV4},
    };
    int i = 0;
    for (auto &[hoverId, fooContents] : entries) {
        i++;
        // Cancel a hover.
        preprocessor.preprocessAndEnqueue(state, makeCancel(hoverId), mtx);
        // Check that the next edit was merged into the first edit.
        ASSERT_EQ(state.pendingRequests[0]->method(), LSPMethod::SorbetWorkspaceEdit);
        auto updates = getUpdates(state, 0).value();
        EXPECT_EQ(updates->mergeCount, i);
        EXPECT_EQ(fooContents, updates->updates[0]->source());
    }

    // Push a new edit that takes the slow path.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV5, 5), mtx);
    {
        // Ensure GS for new edit has all previous edits, including the contents of bar.rb.
        auto updates = getUpdates(state, state.pendingRequests.size() - 1).value();
        fast_sort(updates->updates, [](auto &a, auto &b) -> bool { return a->path().compare(b->path()) < 0; });
        EXPECT_EQ(barV1, updates->updates[0]->source());
        EXPECT_EQ(fileV5, updates->updates[1]->source());
    }
}

TEST(PreemptionTasks, PreemptionTasksWorkAsExpected) {
    auto gs = makeGS();
    // Note: needs to be > 0 otherwise an enforce triggers.
    gs->lspTypecheckCount++;
    auto preemptManager = make_shared<core::lsp::PreemptionTaskManager>(gs->epochManager);

    // Put an error in the queue.
    gs->errorQueue->pushError(
        *gs, make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True}, "MyError",
                                      vector<core::ErrorSection>(), vector<core::AutocorrectSuggestion>(), false));

    // No preemption task registered.
    EXPECT_FALSE(preemptManager->tryRunScheduledPreemptionTask(*gs));

    auto task = make_shared<CountingTask>(preemptManager, *gs);
    // Should fail because a slow path is not running, so there's nothing to preempt.
    EXPECT_FALSE(preemptManager->trySchedulePreemptionTask(task));
    // No slow path running, so we cannot cancel it.
    EXPECT_FALSE(gs->epochManager->tryCancelSlowPath(3));

    // Signify to GlobalState that a slow path is beginning.
    gs->epochManager->startCommitEpoch(2);
    EXPECT_FALSE(gs->epochManager->wasTypecheckingCanceled());

    // Preempting should work now.
    EXPECT_TRUE(preemptManager->trySchedulePreemptionTask(task));

    // This should run + clear the scheduled task.
    EXPECT_TRUE(preemptManager->tryRunScheduledPreemptionTask(*gs));
    EXPECT_EQ(1, task->runCount);

    // Our error should still be there.
    auto errors = gs->errorQueue->drainAllErrors();
    ASSERT_EQ(1, errors.size());
    EXPECT_EQ("MyError", errors[0]->header);

    // We can cancel the slow path.
    EXPECT_TRUE(gs->epochManager->tryCancelSlowPath(3));
    EXPECT_TRUE(gs->epochManager->wasTypecheckingCanceled());

    // We should not be able to schedule further tasks after cancelation.
    EXPECT_FALSE(preemptManager->trySchedulePreemptionTask(task));
}

} // namespace sorbet::realmain::lsp::test
