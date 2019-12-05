#include "gtest/gtest.h"
// has to go first as it violates our requirements

#include "main/lsp/LSPOutput.h"
#include "main/lsp/TimeTravelingGlobalState.h"
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
    auto config = make_shared<LSPConfiguration>(opts, make_shared<LSPOutputToVector>(), *workers, logger, true, false);
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

TimeTravelingGlobalState makeTTGS(const shared_ptr<LSPConfiguration> &config = nullConfig, u4 initialVersion = 0) {
    return TimeTravelingGlobalState(config, makeGS(config->opts), initialVersion);
}

LSPPreprocessor makePreprocessor(const shared_ptr<LSPConfiguration> &config = nullConfig, u4 initialVersion = 0) {
    return LSPPreprocessor(makeGS(config->opts), config, initialVersion);
}

bool comesBeforeSymmetric(const TimeTravelingGlobalState &ttgs, u4 a, u4 b) {
    return ttgs.comesBefore(a, b) && !ttgs.comesBefore(b, a);
}

unique_ptr<LSPMessage> makeWatchman(vector<string> files) {
    auto params = make_unique<WatchmanQueryResponse>("", "", false, files);
    auto msg = make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(params)));
    return msg;
}

optional<LSPFileUpdates *> getUpdates(QueueState &state, int i) {
    EXPECT_LT(i, state.pendingRequests.size());
    if (i >= state.pendingRequests.size()) {
        return nullopt;
    }
    auto &msg = state.pendingRequests[i];
    EXPECT_TRUE(msg->isNotification() && msg->method() == LSPMethod::SorbetWorkspaceEdit);
    auto &params = get<unique_ptr<SorbetWorkspaceEditParams>>(msg->asNotification().params);
    return &params->updates;
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

LSPFileUpdates makeUpdates(u4 &version, vector<pair<string, string>> files) {
    LSPFileUpdates updates;
    updates.versionStart = version++;
    updates.versionEnd = updates.versionStart;
    for (auto &[path, contents] : files) {
        updates.updatedFiles.push_back(make_shared<core::File>(move(path), move(contents), core::File::Type::Normal));
    }
    return updates;
}

} // namespace

TEST(TimeTravelingGlobalState, ComesBefore) { // NOLINT
    // Positive maximum version tests
    {
        // '1' is the maximum version seen thus far.
        TimeTravelingGlobalState ttgs = makeTTGS(nullConfig, 1);
        // Simple cases: Previous version comes before current version.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 0, 1);
        EXPECT_FALSE(ttgs.comesBefore(0, 0));
        // Properly handles maxVersion + 1 to support <= maxVersion comparison.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 1, 2);
        // maxVersion + 2 is considered to be from before version wrapped around.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 3, 2);
        // 3 and 4 are from previous trip round the integer space, so 4 came after 3.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 3, 4);
        // Uint limits.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, UINT32_MAX, 0);
        // A and B are equal but come after maxVersion + 1.
        EXPECT_FALSE(ttgs.comesBefore(5, 5));
        // A and B come before one another on last trip through the uint space.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, UINT32_MAX - 10, UINT32_MAX - 9);
    }
    // Negative maximum version tests
    {
        TimeTravelingGlobalState ttgs = makeTTGS(nullConfig, UINT32_MAX);
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 2, 3);
        // Uint limits.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, UINT32_MAX, 0);
        // maxVersion + 2 is part of previous trip 'round the space of uints.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 1, 0);
        // A and B come before one another on last trip through the uint space.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, UINT32_MAX - 10, UINT32_MAX - 9);
    }
    // Maximum version + 1 is UINT32_MAX
    {
        TimeTravelingGlobalState ttgs = makeTTGS(nullConfig, UINT32_MAX - 1);
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 0, UINT32_MAX);
    }
    // Maximum version + 1 is 0
    {
        TimeTravelingGlobalState ttgs = makeTTGS(nullConfig, UINT32_MAX);
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, UINT32_MAX, 0);
    }
}

TEST(TimeTravelingGlobalState, Undo) { // NOLINT
    TimeTravelingGlobalState ttgs = makeTTGS(nullConfig, 1);
    string foo1 = "# typed: strict\ndef foo; end";
    string foo2 = "# typed: strict\ndef foo; 1 + 1; end";
    string bar1 = "1 + 1";
    u4 version = 2;
    auto v2 = makeUpdates(version, {{"foo.rb", foo1}});
    ttgs.commitEdits(v2);
    auto v3 = makeUpdates(version, {{"bar.rb", bar1}});
    ttgs.commitEdits(v3);
    version = 100;
    auto v100 = makeUpdates(version, {{"foo.rb", foo2}});
    ttgs.commitEdits(v100);
    {
        const auto &gs = ttgs.getGlobalState();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), foo2);
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), bar1);
    }
    // going back to v99 is the same as going back to v3, since that's the next smallest version.
    ttgs.travel(99);
    {
        const auto &gs = ttgs.getGlobalState();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), foo1);
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), bar1);
    }
    // going back to v3 when already technically on v3 should cause no change.
    ttgs.travel(3);
    {
        const auto &gs = ttgs.getGlobalState();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), foo1);
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), bar1);
    }
    // go back to v2
    ttgs.travel(2);
    {
        const auto &gs = ttgs.getGlobalState();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), foo1);
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), "");
    }
    // Travel to before *all* edits.
    ttgs.travel(1);
    {
        const auto &gs = ttgs.getGlobalState();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), "");
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), "");
    }
    // Delete undo log prior to latest version. Note that ttgs is explicitly at a version whose history is getting
    // erased, so this checks that ttgs properly time-travels forward to avoid forgetting how to get to v100.
    ttgs.pruneBefore(100);
    // Should know how to go from 100 => 3, but no longer knows how to get to versions 2 or 1.
    // This is the same as traveling to v3.
    ttgs.travel(1);
    {
        const auto &gs = ttgs.getGlobalState();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), foo1);
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), bar1);
    }
    // Back to v100.
    ttgs.travel(100);
    {
        const auto &gs = ttgs.getGlobalState();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), foo2);
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), bar1);
    }
}

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
    EXPECT_EQ(updates->versionEnd - updates->versionStart, 0);
    EXPECT_FALSE(updates->canTakeFastPath);
    EXPECT_TRUE(updates->hasNewFiles);
    ASSERT_EQ(updates->updatedFiles.size(), 1);
    EXPECT_EQ(updates->updatedFiles[0]->source(), fileContents);
    EXPECT_EQ(updates->updatedFiles[0]->path(),
              updates->updatedFileIndexes[0].file.data(*updates->updatedGS.value()).path());
    // Contents should match the contents of the editor.
    EXPECT_EQ(fileContents, updates->updatedFileIndexes[0].file.data(*updates->updatedGS.value()).source());
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
        ASSERT_FALSE(updates->canTakeFastPath);
        // Should have the newest version of the update.
        EXPECT_EQ(updates->updatedFileIndexes[0].file.data(*updates->updatedGS.value()).source(), fileV2);
    }

    // Append another edit that will get merged with the existing edit.
    // V3 will take the fast path relative to V2, forcing a situation in which a new global state will need to be
    // created.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV3, 3), mtx);
    {
        // Should have the newest version of the update.
        const auto updates = getUpdates(state, 0).value();
        ASSERT_FALSE(updates->canTakeFastPath);
        EXPECT_EQ(updates->updatedFileIndexes[0].file.data(*updates->updatedGS.value()).source(), fileV3);
    }

    // Append another edit that will get merged with the existing edit.
    // V4 will take the slow path relative to V3, and the global state created for it should be re-used.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV4, 4), mtx);
    {
        const auto updates = getUpdates(state, 0).value();
        ASSERT_FALSE(updates->canTakeFastPath);
        EXPECT_EQ(updates->updatedFileIndexes[0].file.data(*updates->updatedGS.value()).source(), fileV4);
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

    auto outputMsgs = output->getOutput();
    ASSERT_EQ(2, outputMsgs.size());
    EXPECT_EQ(outputMsgs[0]->method(), LSPMethod::SorbetShowOperation);
    EXPECT_EQ(outputMsgs[1]->method(), LSPMethod::SorbetShowOperation);
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
    // New file. Should not be present in any cloned global states for earlier edits.
    preprocessor.preprocessAndEnqueue(state, makeOpen("bar.rb", barV1, 1), mtx);

    vector<pair<int, bool>> fastPathDecisions = {{0, false}, {2, true}, {4, true}, {6, true}};
    for (auto &[messageId, canTakeFastPath] : fastPathDecisions) {
        auto updates = getUpdates(state, messageId).value();
        EXPECT_EQ(updates->canTakeFastPath, canTakeFastPath);
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
        EXPECT_EQ(updates->versionEnd - updates->versionStart + 1, 1 + i);
        EXPECT_EQ(updates->updatedFiles[0]->source(), fooContents);
        const auto &gs = *updates->updatedGS.value();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), fooContents);
        // bar.rb shouldn't be defined for this earlier file update.
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), "");
    }

    // Push a new edit that takes the slow path.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fileV5, 5), mtx);
    {
        // Ensure GS for new edit has all previous edits, including the contents of bar.rb.
        const auto updates = getUpdates(state, state.pendingRequests.size() - 1).value();
        ASSERT_FALSE(updates->canTakeFastPath);
        const auto &gs = *updates->updatedGS.value();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), fileV5);
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), barV1);
    }
}

// Ensures that we don't throw away undo history for merged edits.
TEST(LSPPreprocessor, MakesCorrectFastPathDecisionsOnSimultaneousEdits) { // NOLINT
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;

    // V1: New file, slow path
    string fooV1 = "# typed: true\ndef foo; end";
    string barV1 = "# typed: true\ndef bar; end";

    // Commit new files first. The 'new file flag' is handled specially.
    preprocessor.preprocessAndEnqueue(state, makeOpen("foo.rb", fooV1, 1), mtx);
    preprocessor.preprocessAndEnqueue(state, makeOpen("bar.rb", barV1, 1), mtx);
    // Clear out of queue to emulate typechecking thread 'processing' it.
    state.pendingRequests.clear();

    // barV1 => V2: Slow path
    string barV2 = "# typed: true\ndef bar2; end";
    // fooV1 => V2: Fast path
    string fooV2 = "# typed: true\ndef foo; 1 + 2; end";
    // fooV2 => V3: Fast path
    string fooV3 = "# typed: true\ndef foo; 1 + 3; end";
    // fooV3 => V4: Fast path
    string fooV4 = "# typed: true\ndef foo; 1 + 4; end";

    preprocessor.preprocessAndEnqueue(state, makeChange("bar.rb", barV2, 2), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV2, 2), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV3, 3), mtx);
    // With old buggy logic, preprocessor will 'forget' about the barV2 update, causing it to mistakenly think these
    // four edits can take fast path.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV4, 4), mtx);

    const auto updates = getUpdates(state, 0).value();
    EXPECT_FALSE(updates->canTakeFastPath);
}

// Defines an empty class 'foo.rb' wth method 'foo' and returns the resulting GlobalState.
unique_ptr<core::GlobalState> initCancelSlowPathTest(LSPPreprocessor &preprocessor, QueueState &state,
                                                     absl::Mutex &mtx) {
    // New file, slow path. Can't avoid, so emulate processing it.
    string fooV1 = "# typed: true\ndef foo; end";
    preprocessor.preprocessAndEnqueue(state, makeOpen("foo.rb", fooV1, 1), mtx);

    // Grab GS.
    unique_ptr<core::GlobalState> gs;
    {
        auto updates = getUpdates(state, 0).value();
        gs = move(updates->updatedGS.value());
        state.pendingRequests.clear();
    }
    return gs;
}

u4 emulateProcessEditAtHeadOfQueue(QueueState &state, core::GlobalState &gs) {
    // Emulate typechecking thread: begin 'processing' this edit.
    const auto updates = getUpdates(state, 0).value();
    auto epoch = updates->versionEnd;
    gs.startCommitEpoch(updates->versionStart - 1, epoch);
    state.pendingRequests.clear();
    return epoch;
}

TEST(SlowPathCancelation, CancelsRunningSlowPathWhenFastPathEditComesIn) { // NOLINT
    QueueState state;
    absl::Mutex mtx;
    auto preprocessor = makePreprocessor();
    unique_ptr<core::GlobalState> gs = initCancelSlowPathTest(preprocessor, state, mtx);

    // Introduce a syntax error, which causes a slow path.
    string fooV2 = "# typed: true\n{def foo; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV2, 2), mtx);
    u4 epoch = emulateProcessEditAtHeadOfQueue(state, *gs);

    // Introduce a fix to syntax error. Should course-correct to a fast path.
    string fooV3 = "# typed: true\ndef foo; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV3, 3), mtx);
    EXPECT_TRUE(gs->wasTypecheckingCanceled());

    // Processor thread: Try to typecheck. Should cancel.
    EXPECT_FALSE(gs->tryCommitEpoch(epoch, true, []() -> void {}));

    // GS should no longer register a cancellation, since the epoch didn't commit.
    EXPECT_FALSE(gs->wasTypecheckingCanceled());
}

TEST(SlowPathCancelation, CancelsRunningSlowPathWhenSlowPathEditComesIn) { // NOLINT
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;
    unique_ptr<core::GlobalState> gs = initCancelSlowPathTest(preprocessor, state, mtx);

    // Introduce a syntax error, which causes a slow path.
    string fooV2 = "# typed: true\n{def foo; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV2, 2), mtx);
    u4 epoch = emulateProcessEditAtHeadOfQueue(state, *gs);

    // Introduce another slow path here: new method
    string fooV3 = "# typed: true\ndef foo; end\ndef bar;end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV3, 3), mtx);
    EXPECT_TRUE(gs->wasTypecheckingCanceled());

    // Processor thread: Try to typecheck. Should return false because it has been canceled.
    EXPECT_FALSE(gs->tryCommitEpoch(epoch, true, []() -> void {}));

    // Ensure that new update has a new global state defined.
    auto maybeUpdates = getUpdates(state, 0);
    ASSERT_TRUE(maybeUpdates.has_value());
    auto &updates = maybeUpdates.value();
    EXPECT_TRUE(updates->updatedGS.has_value());
}

TEST(SlowPathCancelation, DoesNotCancelRunningSlowPathWhenFastPathEditComesIn) { // NOLINT
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;
    unique_ptr<core::GlobalState> gs = initCancelSlowPathTest(preprocessor, state, mtx);

    // Introduce a new method, which causes a slow path.
    string fooV2 = "# typed: true\ndef foo; end\ndef bar; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV2, 2), mtx);
    u4 epoch = emulateProcessEditAtHeadOfQueue(state, *gs);

    // Edit the body of the new method which should take the fast path.
    string fooV3 = "# typed: true\ndef foo; end\ndef bar; 1 + 1; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV3, 3), mtx);
    EXPECT_FALSE(gs->wasTypecheckingCanceled());

    // Processor thread: Try to typecheck. Should return true because typechecking hasn't been canceled.
    EXPECT_TRUE(gs->tryCommitEpoch(epoch, true, []() -> void {}));
}

TEST(SlowPathCancelation, CancelsRunningSlowPathAfterBlockingRequestGetsCanceled) { // NOLINT
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;
    unique_ptr<core::GlobalState> gs = initCancelSlowPathTest(preprocessor, state, mtx);

    // Introduce a syntax error, which causes a slow path.
    string fooV2 = "# typed: true\n{def foo; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV2, 2), mtx);
    u4 epoch = emulateProcessEditAtHeadOfQueue(state, *gs);

    // Blocking hover.
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(5, "foo.rb"), mtx);

    // Fixes parse error, but blocked by hover.
    string fooV3 = "# typed: true\ndef foo; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV3, 3), mtx);
    EXPECT_FALSE(gs->wasTypecheckingCanceled());

    // Cancel hover, which should cause the slow path to be canceled.
    preprocessor.preprocessAndEnqueue(state, makeCancel(5), mtx);
    EXPECT_TRUE(gs->wasTypecheckingCanceled());

    // Processor thread: Try to typecheck, but get denied because canceled.
    EXPECT_FALSE(gs->tryCommitEpoch(epoch, true, []() -> void {}));
}

// Ensure that the preprocessor prunes TTGS properly during the event of a version rollover.
TEST(SlowPathCancelation, PruneDuringVersionRollover) { // NOLINT
    QueueState state;
    absl::Mutex mtx;
    auto preprocessor = makePreprocessor(nullConfig, 0xFFFFFFFF - 4);

    // Edit 0xFFFFFFFF-3, committed during this function, introduces new files.
    unique_ptr<core::GlobalState> gs = initCancelSlowPathTest(preprocessor, state, mtx);

    // Edit 0xFFFFFFFF-2 introduces barV1.
    string barV1 = "# typed: true\ndef foo; end";
    preprocessor.preprocessAndEnqueue(state, makeOpen("bar.rb", barV1, 1), mtx);

    // Emulate typechecker thread: 'process' changes
    emulateProcessEditAtHeadOfQueue(state, *gs);
    ASSERT_TRUE(gs->tryCommitEpoch(0xFFFFFFFF - 2, true, []() -> void {}));

    // These two get bundled together:
    // Edit 0xFFFFFFFF-1 introduces syntax error into foo
    string fooV2 = "# typed: true\ndef { foo; end";
    // Edit 0xFFFFFFFF introduces fast path update to bar
    string barV2 = "# typed: true\ndef foo; 1; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV2, 2), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("bar.rb", barV2, 2), mtx);

    // Emulate typechecker thread: claim that we are actively and concurrently typechecking these changes.
    emulateProcessEditAtHeadOfQueue(state, *gs);

    // Edit 0 fixes parse error and cancels slow path from previous bundle.
    string fooV3 = "# typed: true\ndef foo; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV3, 3), mtx);

    EXPECT_TRUE(gs->wasTypecheckingCanceled());

    // Without proper rollover, it'll forget about change to `barV2` because 0 < 0xFFFFFFFF.
    const auto maybeUpdates = getUpdates(state, 0);
    ASSERT_TRUE(maybeUpdates.has_value());
    const auto updates = maybeUpdates.value();

    EXPECT_EQ(updates->versionEnd, 0);
    EXPECT_EQ(updates->versionStart, 0xFFFFFFFF - 1);
    EXPECT_EQ(updates->canTakeFastPath, true);
    ASSERT_EQ(updates->updatedFiles.size(), 2);

    const bool fooFirst = updates->updatedFiles[0]->path() == "foo.rb";
    const auto &foo = fooFirst ? updates->updatedFiles[0] : updates->updatedFiles[1];
    const auto &bar = fooFirst ? updates->updatedFiles[1] : updates->updatedFiles[0];

    EXPECT_EQ(foo->source(), fooV3);
    EXPECT_EQ(bar->source(), barV2);
}

TEST(SlowPathCancelation, DoesNotIncludeOldEditsInCombinedEdit) { // NOLINT
    auto preprocessor = makePreprocessor();
    QueueState state;
    absl::Mutex mtx;
    // Defines foo.rb.
    unique_ptr<core::GlobalState> gs = initCancelSlowPathTest(preprocessor, state, mtx);
    // Define bar.rb.
    preprocessor.preprocessAndEnqueue(state, makeOpen("bar.rb", "# typed: true\ndef bar; 99999; end", 1), mtx);
    // 'process' those messages.
    state.pendingRequests.clear();

    // Fast path bar.rb update.
    string barV2 = "# typed: true\ndef bar; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("bar.rb", barV2, 2), mtx);
    // Hover request: Blocking.
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(10, "foo.rb"), mtx);

    // Slow path foo.rb update (parse error)
    string fooV3 = "# typed: true\ndef {foo; 1; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV3, 3), mtx);

    // 'process' first two messages
    state.pendingRequests.pop_front();
    state.pendingRequests.pop_front();
    // Emulate typechecking thread typechecking slow path in parallel.
    emulateProcessEditAtHeadOfQueue(state, *gs);

    // New edit: Fixes parse error, and cancels running slow path.
    string fooV4 = "# typed: true\ndef foo; 1; end";
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", fooV4, 4), mtx);

    EXPECT_TRUE(gs->wasTypecheckingCanceled());

    auto updates = getUpdates(state, 0).value();

    // Mega update should *not* contain `bar.rb` update.
    ASSERT_EQ(1, updates->updatedFiles.size());
    ASSERT_EQ(updates->updatedFiles[0]->source(), fooV4);
}

} // namespace sorbet::realmain::lsp::test
