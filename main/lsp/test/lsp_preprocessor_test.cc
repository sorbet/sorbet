#include "gtest/gtest.h"
// has to go first as it violates our requirements

#include "main/lsp/TimeTravelingGlobalState.h"
#include "main/lsp/lsp.h"
#include "payload/payload.h"
#include "spdlog/sinks/null_sink.h"
#include "test/helpers/MockFileSystem.h"
#include <climits>
#include <memory>

using namespace std;

namespace sorbet::realmain::lsp::test {

namespace {

options::Options makeOptions(string_view rootPath) {
    options::Options opts;
    opts.rawInputDirNames.emplace_back(string(rootPath));
    opts.runLSP = true;
    opts.fs = make_shared<sorbet::test::MockFileSystem>(rootPath);
    return opts;
}

static auto nullSink = make_shared<spd::sinks::null_sink_mt>();
static auto logger = make_shared<spd::logger>("console", nullSink);
static auto typeErrorsConsole = make_shared<spd::logger>("typeDiagnostics", nullSink);
static auto nullOpts = makeOptions("");
static auto workers = WorkerPool::create(0, *logger);

LSPConfiguration makeConfig(const options::Options &opts = nullOpts) {
    return LSPConfiguration(opts, logger, true, false);
}

unique_ptr<core::GlobalState> makeGS(const options::Options &opts = nullOpts) {
    auto gs = make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger)));
    unique_ptr<KeyValueStore> kvstore;
    payload::createInitialGlobalState(gs, opts, kvstore);
    gs->errorQueue->ignoreFlushes = true;
    return gs;
}

static auto nullConfig = makeConfig();

TimeTravelingGlobalState makeTTGS(const LSPConfiguration &config = nullConfig, u4 initialVersion = 0) {
    return TimeTravelingGlobalState(config, logger, *workers, makeGS(config.opts), initialVersion);
}

LSPPreprocessor makePreprocessor(const LSPConfiguration &config = nullConfig) {
    return LSPPreprocessor(makeGS(config.opts), config, *workers, logger);
}

bool comesBeforeSymmetric(const TimeTravelingGlobalState &ttgs, u4 a, u4 b) {
    return ttgs.comesBefore(a, b) && !ttgs.comesBefore(b, a);
}

unique_ptr<LSPMessage> makeOpen(string_view path, u4 version, string_view source) {
    auto params = make_unique<DidOpenTextDocumentParams>(
        make_unique<TextDocumentItem>(string(path), "ruby", version, string(source)));
    return make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(params)));
}

unique_ptr<LSPMessage> makeChange(string_view path, u4 version, string_view source) {
    auto contentChange = make_unique<TextDocumentContentChangeEvent>(string(source));
    vector<unique_ptr<TextDocumentContentChangeEvent>> changes;
    changes.push_back(move(contentChange));
    auto params = make_unique<DidChangeTextDocumentParams>(
        make_unique<VersionedTextDocumentIdentifier>(string(path), version), move(changes));
    return make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidChange, move(params)));
}

unique_ptr<LSPMessage> makeWatchman(vector<string> files) {
    auto params = make_unique<WatchmanQueryResponse>("", "", false, files);
    auto msg = make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(params)));
    return msg;
}

optional<pair<LSPFileUpdates *, SorbetWorkspaceEditCounts *>> getUpdates(QueueState &state, int i) {
    EXPECT_LT(i, state.pendingRequests.size());
    if (i >= state.pendingRequests.size()) {
        return nullopt;
    }
    auto &msg = state.pendingRequests[i];
    EXPECT_TRUE(msg->isNotification() && msg->method() == LSPMethod::SorbetWorkspaceEdit);
    auto &params = get<unique_ptr<SorbetWorkspaceEditParams>>(msg->asNotification().params);
    return make_pair(&params->updates, params->counts.get());
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

TEST(LSPPreprocessor, IgnoresWatchmanUpdatesFromOpenFiles) { // NOLINT
    auto opts = makeOptions("");
    auto preprocessor = makePreprocessor(makeConfig(opts));
    QueueState state;
    absl::Mutex mtx;

    string fileContents = "# typed: true\n1+1";
    opts.fs->writeFile("foo.rb", "");
    preprocessor.preprocessAndEnqueue(state, makeOpen("foo.rb", 1, fileContents), mtx);
    preprocessor.preprocessAndEnqueue(state, makeWatchman({"foo.rb"}), mtx);

    ASSERT_TRUE(state.pendingRequests.size() == 1);

    const auto [updates, counts] = getUpdates(state, 0).value();
    EXPECT_EQ(counts->textDocumentDidOpen, 1);
    EXPECT_EQ(counts->sorbetWatchmanFileChange, 1);

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
    preprocessor.preprocessAndEnqueue(state, makeOpen("foo.rb", 1, fileV1), mtx);
    // Pop it off the queue to prevent a merge.
    state.pendingRequests.clear();
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", 2, fileV2), mtx);
    {
        const auto [updates, counts] = getUpdates(state, 0).value();
        ASSERT_FALSE(updates->canTakeFastPath);
        // Should have the newest version of the update.
        EXPECT_EQ(updates->updatedFileIndexes[0].file.data(*updates->updatedGS.value()).source(), fileV2);
    }

    // Append another edit that will get merged with the existing edit.
    // V3 will take the fast path relative to V2, forcing a situation in which a new global state will need to be
    // created.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", 3, fileV3), mtx);
    {
        // Should have the newest version of the update.
        const auto [updates, counts] = getUpdates(state, 0).value();
        ASSERT_FALSE(updates->canTakeFastPath);
        EXPECT_EQ(updates->updatedFileIndexes[0].file.data(*updates->updatedGS.value()).source(), fileV3);
    }

    // Append another edit that will get merged with the existing edit.
    // V4 will take the slow path relative to V3, and the global state created for it should be re-used.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", 4, fileV4), mtx);
    {
        const auto [updates, counts] = getUpdates(state, 0).value();
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
    auto config = makeConfig(options);
    config.enableOperationNotifications = true;
    auto preprocessor = makePreprocessor(config);
    auto msg = make_unique<LSPMessage>(
        make_unique<NotificationMessage>("2.0", LSPMethod::Initialized, make_unique<InitializedParams>()));
    preprocessor.preprocessAndEnqueue(state, move(msg), mtx);

    ASSERT_EQ(3, state.pendingRequests.size());
    EXPECT_EQ(state.pendingRequests[0]->method(), LSPMethod::SorbetShowOperation);
    EXPECT_EQ(state.pendingRequests[1]->method(), LSPMethod::SorbetShowOperation);
    EXPECT_EQ(state.pendingRequests[2]->method(), LSPMethod::Initialized);
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

    preprocessor.preprocessAndEnqueue(state, makeOpen("foo.rb", 1, fileV1), mtx);
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(id++, "foo.rb", 0, 0), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", 2, fileV2), mtx);
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(id++, "foo.rb", 0, 0), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", 3, fileV3), mtx);
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(id++, "foo.rb", 0, 0), mtx);
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", 4, fileV4), mtx);
    preprocessor.preprocessAndEnqueue(state, makeHoverReq(id++, "foo.rb", 0, 0), mtx);
    // New file. Should not be present in any cloned global states for earlier edits.
    preprocessor.preprocessAndEnqueue(state, makeOpen("bar.rb", 1, barV1), mtx);

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
        auto [updates, count] = getUpdates(state, 0).value();
        EXPECT_EQ(count->textDocumentDidOpen, 1);
        EXPECT_EQ(count->textDocumentDidChange, i);
        EXPECT_EQ(updates->updatedFiles[0]->source(), fooContents);
        const auto &gs = *updates->updatedGS.value();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), fooContents);
        // bar.rb shouldn't be defined for this earlier file update.
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), "");
    }

    // Push a new edit that takes the slow path.
    preprocessor.preprocessAndEnqueue(state, makeChange("foo.rb", 5, fileV5), mtx);
    {
        // Ensure GS for new edit has all previous edits, including the contents of bar.rb.
        const auto [updates, counts] = getUpdates(state, state.pendingRequests.size() - 1).value();
        ASSERT_FALSE(updates->canTakeFastPath);
        const auto &gs = *updates->updatedGS.value();
        EXPECT_EQ(gs.findFileByPath("foo.rb").data(gs).source(), fileV5);
        EXPECT_EQ(gs.findFileByPath("bar.rb").data(gs).source(), barV1);
    }
}
} // namespace sorbet::realmain::lsp::test
