#include "gtest/gtest.h"
// has to go first as it violates our requirements

#include "main/lsp/TimeTravelingGlobalState.h"
#include "main/lsp/lsp.h"
#include "payload/payload.h"
#include "spdlog/sinks/null_sink.h"
#include <climits>

using namespace std;

namespace sorbet::realmain::lsp::test {

namespace {

options::Options makeOptions(string_view rootPath) {
    options::Options opts;
    opts.rawInputDirNames.emplace_back(string(rootPath));
    opts.runLSP = true;
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

TimeTravelingGlobalState makeTTGS(const LSPConfiguration &config = nullConfig, int initialVersion = 0) {
    return TimeTravelingGlobalState(config, logger, *workers, makeGS(config.opts), initialVersion);
}

LSPPreprocessor makePreprocessor(const LSPConfiguration &config = nullConfig) {
    return LSPPreprocessor(makeGS(config.opts), config, *workers, logger);
}

bool comesBeforeSymmetric(const TimeTravelingGlobalState &ttgs, int a, int b) {
    return ttgs.comesBefore(a, b) && !ttgs.comesBefore(b, a);
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
        // Negative versions
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, -1, 0);
        // Integer limits.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, INT_MAX, INT_MIN);
        // A and B are equal but come after maxVersion + 1.
        EXPECT_FALSE(ttgs.comesBefore(5, 5));
        // Negative versions.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, -10, -9);
    }
    // Negative maximum version tests
    {
        TimeTravelingGlobalState ttgs = makeTTGS(nullConfig, -1);
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 2, 3);
        // Handles maxVersion + 1.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, -1, 0);
        // maxVersion + 2 is part of previous trip 'round the space of ints.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, 1, 0);
        // Negative versions.
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, -10, -9);
    }
    // Maximum version + 1 is MAX_INT
    {
        TimeTravelingGlobalState ttgs = makeTTGS(nullConfig, INT_MAX - 1);
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, INT_MIN, INT_MAX);
    }
    // Maximum version + 1 is MIN_INT
    {
        TimeTravelingGlobalState ttgs = makeTTGS(nullConfig, INT_MAX);
        EXPECT_PRED3(comesBeforeSymmetric, ttgs, INT_MAX, INT_MIN);
    }
}

TEST(LSPPreprocessor, Lol) { // NOLINT
    auto preprocessor = makePreprocessor();
    //
}

} // namespace sorbet::realmain::lsp::test