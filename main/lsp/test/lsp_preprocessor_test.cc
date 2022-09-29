#include "doctest.h"
// has to go first as it violates our requirements

#include "common/concurrency/WorkerPool.h"
#include "common/sort.h"
#include "core/Error.h"
#include "core/ErrorCollector.h"
#include "core/ErrorQueue.h"
#include "core/NullFlusher.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/Task.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPTask.h"
#include "main/lsp/notifications/sorbet_workspace_edit.h"
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
    auto config = make_shared<LSPConfiguration>(opts, make_shared<LSPOutputToVector>(), logger, false);
    InitializeParams initParams("", make_unique<ClientCapabilities>());
    initParams.rootPath = "";
    initParams.initializationOptions = make_unique<SorbetInitializationOptions>();
    initParams.initializationOptions.value()->supportsOperationNotifications = enableShowOpNotifs;
    config->setClientConfig(make_shared<LSPClientConfiguration>(initParams));
    if (initialize) {
        config->markInitialized();
    }
    return config;
}

unique_ptr<core::GlobalState> makeGS(shared_ptr<core::ErrorFlusher> errorFlusher = make_shared<core::NullFlusher>(),
                                     const options::Options &opts = nullOpts) {
    auto gs =
        make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger, errorFlusher)));
    unique_ptr<const OwnedKeyValueStore> kvstore;
    payload::createInitialGlobalState(gs, opts, kvstore);
    return gs;
}

auto nullConfig = makeConfig();

LSPPreprocessor makePreprocessor(shared_ptr<TaskQueue> queue, const shared_ptr<LSPConfiguration> &config = nullConfig,
                                 uint32_t initialVersion = 0) {
    return LSPPreprocessor(config, move(queue), initialVersion);
}

unique_ptr<LSPMessage> makeWatchman(vector<string> files) {
    auto params = make_unique<WatchmanQueryResponse>("", "", false, files);
    auto msg = make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(params)));
    return msg;
}

optional<const SorbetWorkspaceEditParams *> getUpdates(TaskQueue &state, int i) {
    absl::MutexLock lck{state.getMutex()};
    CHECK_LT(i, state.tasks().size());
    if (i >= state.tasks().size()) {
        return nullopt;
    }
    auto &task = state.tasks()[i];
    CHECK(task->method == LSPMethod::SorbetWorkspaceEdit);
    if (auto *editTask = dynamic_cast<SorbetWorkspaceEditTask *>(task.get())) {
        return &editTask->getParams();
    } else {
        return nullopt;
    }
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
        gs.errorQueue->flushAllErrors(gs);
        runCount++;
    }
};

} // namespace

TEST_CASE("IgnoresWatchmanUpdatesFromOpenFiles") {
    auto opts = makeOptions("");
    auto state = make_shared<TaskQueue>();
    auto preprocessor = makePreprocessor(state, makeConfig(opts));

    string fileContents = "# typed: true\n1+1";
    opts.fs->writeFile("foo.rb", "");
    preprocessor.preprocessAndEnqueue(makeOpen("foo.rb", fileContents, 1));
    preprocessor.preprocessAndEnqueue(makeWatchman({"foo.rb"}));

    {
        absl::MutexLock lck{state->getMutex()};
        REQUIRE_EQ(1, state->tasks().size());
    }

    const auto updates = getUpdates(*state, 0).value();
    // Version didn't change because it ignored the watchman update.
    CHECK_EQ(updates->mergeCount, 0);
    REQUIRE_EQ(updates->updates.size(), 1);
    CHECK_EQ(updates->updates[0]->source(), fileContents);
}

// When deepCopying initialGS for typechecking, it should always have all previous updates applied to it.
TEST_CASE("ClonesTypecheckingGSAtCorrectLogicalTime") {
    string fileV1 = "# typed: true";
    // V1 => V2: Slow path
    string fileV2 = "# typed: true\ndef foo; end";
    // V2 => V3: Fast path
    string fileV3 = "# typed: true\ndef foo; 1 + 1; end";
    // V3 => V4: Slow path
    string fileV4 = "# typed: true\ndef foo; 1 + 1; end; def bar; end";
    auto state = make_shared<TaskQueue>();
    auto preprocessor = makePreprocessor(state);

    // Apply one update, slow path.
    preprocessor.preprocessAndEnqueue(makeOpen("foo.rb", fileV1, 1));
    // Pop it off the queue to prevent a merge.
    {
        absl::MutexLock lck{state->getMutex()};
        state->tasks().clear();
    }
    preprocessor.preprocessAndEnqueue(makeChange("foo.rb", fileV2, 2));
    {
        const auto updates = getUpdates(*state, 0).value();
        // Should have the newest version of the update.
        CHECK_EQ(fileV2, updates->updates[0]->source());
    }

    // Append another edit that will get merged with the existing edit.
    // V3 will take the fast path relative to V2, forcing a situation in which a new global state will need to be
    // created.
    preprocessor.preprocessAndEnqueue(makeChange("foo.rb", fileV3, 3));
    {
        // Should have the newest version of the update.
        const auto updates = getUpdates(*state, 0).value();
        CHECK_EQ(fileV3, updates->updates[0]->source());
    }

    // Append another edit that will get merged with the existing edit.
    // V4 will take the slow path relative to V3, and the global state created for it should be re-used.
    preprocessor.preprocessAndEnqueue(makeChange("foo.rb", fileV4, 4));
    {
        const auto updates = getUpdates(*state, 0).value();
        CHECK_EQ(fileV4, updates->updates[0]->source());
    }
}

TEST_CASE("PauseAndResume") {
    auto state = make_shared<TaskQueue>();
    auto preprocessor = makePreprocessor(state);

    {
        auto msg = make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::PAUSE, JSONNullObject()));
        preprocessor.preprocessAndEnqueue(move(msg));
    }
    {
        absl::MutexLock lck{state->getMutex()};
        CHECK(state->isPaused());
    }
    {
        auto msg =
            make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::RESUME, JSONNullObject()));
        preprocessor.preprocessAndEnqueue(move(msg));
    }

    {
        absl::MutexLock lck{state->getMutex()};
        CHECK_FALSE(state->isPaused());
    }
}

TEST_CASE("Exit") {
    auto state = make_shared<TaskQueue>();
    auto preprocessor = makePreprocessor(state);
    auto msg = make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::Exit, JSONNullObject()));
    preprocessor.preprocessAndEnqueue(move(msg));
    {
        absl::MutexLock lck{state->getMutex()};
        CHECK(state->isTerminated());
        CHECK_EQ(state->getErrorCode(), 0);
    }
}

TEST_CASE("Initialized") {
    auto state = make_shared<TaskQueue>();
    auto options = makeOptions("");
    auto config = makeConfig(options, true, false);
    auto preprocessor = makePreprocessor(state, config);
    auto output = dynamic_pointer_cast<LSPOutputToVector>(config->output);

    // Sending a request prior to initialization should cause an error.
    CHECK_FALSE(config->isInitialized());
    preprocessor.preprocessAndEnqueue(makeHoverReq(1, "foo.rb", 1, 1));
    auto errorMsgs = output->getOutput();
    REQUIRE_EQ(1, errorMsgs.size());
    REQUIRE(errorMsgs[0]->isResponse());
    REQUIRE(errorMsgs[0]->asResponse().error.has_value());

    auto msg = make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::Initialized, make_unique<InitializedParams>()));
    preprocessor.preprocessAndEnqueue(move(msg));
    CHECK(config->isInitialized());

    {
        absl::MutexLock lck{state->getMutex()};
        REQUIRE_EQ(1, state->tasks().size());
        CHECK_EQ(state->tasks()[0]->method, LSPMethod::Initialized);
    }
}

// When a request in the queue is canceled, the preprocessor should merge any edits that happen immediately before and
// after the canceled request.
TEST_CASE("MergesFileUpdatesProperlyAfterCancelation") {
    auto state = make_shared<TaskQueue>();
    auto preprocessor = makePreprocessor(state);
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

    preprocessor.preprocessAndEnqueue(makeOpen("foo.rb", fileV1, 1));
    preprocessor.preprocessAndEnqueue(makeHoverReq(id++, "foo.rb", 0, 0));
    preprocessor.preprocessAndEnqueue(makeChange("foo.rb", fileV2, 2));
    preprocessor.preprocessAndEnqueue(makeHoverReq(id++, "foo.rb", 0, 0));
    preprocessor.preprocessAndEnqueue(makeChange("foo.rb", fileV3, 3));
    preprocessor.preprocessAndEnqueue(makeHoverReq(id++, "foo.rb", 0, 0));
    preprocessor.preprocessAndEnqueue(makeChange("foo.rb", fileV4, 4));
    preprocessor.preprocessAndEnqueue(makeHoverReq(id++, "foo.rb", 0, 0));
    preprocessor.preprocessAndEnqueue(makeOpen("bar.rb", barV1, 1));

    vector<pair<int, string>> messageContents = {{0, fileV1}, {2, fileV2}, {4, fileV3}, {6, fileV4}};
    for (auto &[messageId, contents] : messageContents) {
        auto updates = getUpdates(*state, messageId).value();
        CHECK_EQ("foo.rb", updates->updates[0]->path());
        CHECK_EQ(contents, updates->updates[0]->source());
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
        preprocessor.preprocessAndEnqueue(makeCancel(hoverId));
        // Check that the next edit was merged into the first edit.
        {
            absl::MutexLock lck{state->getMutex()};
            REQUIRE_EQ(state->tasks()[0]->method, LSPMethod::SorbetWorkspaceEdit);
        }
        auto updates = getUpdates(*state, 0).value();
        CHECK_EQ(updates->mergeCount, i);
        CHECK_EQ(fooContents, updates->updates[0]->source());
    }

    // Push a new edit that takes the slow path.
    preprocessor.preprocessAndEnqueue(makeChange("foo.rb", fileV5, 5));
    {
        int len;
        {
            absl::MutexLock lck{state->getMutex()};
            len = state->tasks().size();
        }

        // Ensure GS for new edit has all previous edits, including the contents of bar.rb.
        auto updates = getUpdates(*state, len - 1).value();
        // updates is const, so copy the vector.
        vector<shared_ptr<core::File>> updatesCopy = updates->updates;
        fast_sort(updatesCopy, [](auto &a, auto &b) -> bool { return a->path().compare(b->path()) < 0; });
        CHECK_EQ(barV1, updatesCopy[0]->source());
        CHECK_EQ(fileV5, updatesCopy[1]->source());
    }
}

TEST_CASE("PreemptionTasksWorkAsExpected") {
    auto errorCollector = make_shared<core::ErrorCollector>();
    auto gs = makeGS(errorCollector);
    // Note: needs to be > 0 otherwise an enforce triggers.
    gs->lspTypecheckCount++;
    auto preemptManager = make_shared<core::lsp::PreemptionTaskManager>(gs->epochManager);

    // Put an error in the queue.
    gs->errorQueue->pushError(
        *gs, make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True}, "MyError",
                                      vector<core::ErrorSection>(), vector<core::AutocorrectSuggestion>(), false));

    // No preemption task registered.
    CHECK_FALSE(preemptManager->tryRunScheduledPreemptionTask(*gs));

    auto task = make_shared<CountingTask>(preemptManager, *gs);
    // Should fail because a slow path is not running, so there's nothing to preempt.
    CHECK_FALSE(preemptManager->trySchedulePreemptionTask(task));
    // No slow path running, so we cannot cancel it.
    CHECK_FALSE(gs->epochManager->tryCancelSlowPath(3));

    // Signify to GlobalState that a slow path is beginning.
    gs->epochManager->startCommitEpoch(2);
    CHECK_FALSE(gs->epochManager->wasTypecheckingCanceled());

    // Preempting should work now.
    CHECK(preemptManager->trySchedulePreemptionTask(task));

    // This should run + clear the scheduled task.
    CHECK(preemptManager->tryRunScheduledPreemptionTask(*gs));
    CHECK_EQ(1, task->runCount);

    // Our error should still be there.
    gs->errorQueue->flushAllErrors(*gs);
    auto errors = errorCollector->drainErrors();
    REQUIRE_EQ(1, errors.size());
    CHECK_EQ("MyError", errors[0]->header);

    // We can cancel the slow path.
    CHECK(gs->epochManager->tryCancelSlowPath(3));
    CHECK(gs->epochManager->wasTypecheckingCanceled());

    // We should not be able to schedule further tasks after cancelation.
    CHECK_FALSE(preemptManager->trySchedulePreemptionTask(task));
}

} // namespace sorbet::realmain::lsp::test
