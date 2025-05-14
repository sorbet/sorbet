#include "doctest/doctest.h"
// has to go first as it violates our requirements

#include "ast/ast.h"
#include "common/common.h"
#include "core/Error.h"
#include "core/ErrorCollector.h"
#include "core/ErrorQueue.h"
#include "core/GlobalState.h"
#include "core/Unfreeze.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/options/options.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

namespace sorbet {

namespace {
auto logger = spdlog::stderr_color_mt("error-check-test");
auto errorCollector = make_shared<sorbet::core::ErrorCollector>();
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger, errorCollector);
} // namespace

// Tests cancellation when there aren't enough files to trigger parallelism
TEST_CASE("CanceledWithFewFiles") {
    vector<pair<string, string>> sources{
        {"foo.rb", "# typed: true\n"
                   "class Foo\n"
                   "end"},

        {"bar.rb", "# typed: true\n"
                   "class Bar\n"
                   "end"},
    };

    auto kvstore = nullptr;
    realmain::options::Options opts;
    sorbet::core::GlobalState gs(errorQueue);
    payload::createInitialGlobalState(gs, opts, kvstore);

    vector<core::FileRef> files;
    {
        core::UnfreezeFileTable fileTableAccess(gs);
        for (auto &[path, source] : sources) {
            files.emplace_back(gs.enterFile(path, source));
        }
    }

    gs.epochManager->startCommitEpoch(1);
    REQUIRE(gs.epochManager->tryCancelSlowPath(2));

    auto workers = WorkerPool::create(0, *logger);
    auto cancelable = true;
    auto result = realmain::pipeline::index(gs, absl::MakeSpan(files), opts, *workers, kvstore, cancelable);

    // As the slow path was already canceled, indexing will be canceled immediately.
    REQUIRE(!result.hasResult());
}

// Tests cancelation when there are more than two files, triggering the parallel indexer.
TEST_CASE("CanceledWithMoreFiles") {
    vector<pair<string, string>> sources{
        {"foo.rb", "# typed: true\n"
                   "class Foo\n"
                   "end"},

        {"bar.rb", "# typed: true\n"
                   "class Bar\n"
                   "end"},

        {"baz.rb", "# typed: true\n"
                   "class Baz\n"
                   "end"},

        {"bot.rb", "# typed: true\n"
                   "class Bot\n"
                   "end"},
    };

    auto kvstore = nullptr;
    realmain::options::Options opts;
    sorbet::core::GlobalState gs(errorQueue);
    payload::createInitialGlobalState(gs, opts, kvstore);

    vector<core::FileRef> files;
    {
        core::UnfreezeFileTable fileTableAccess(gs);
        for (auto &[path, source] : sources) {
            files.emplace_back(gs.enterFile(path, source));
        }
    }

    gs.epochManager->startCommitEpoch(1);
    REQUIRE(gs.epochManager->tryCancelSlowPath(2));

    auto workers = WorkerPool::create(0, *logger);
    auto cancelable = true;
    auto result = realmain::pipeline::index(gs, absl::MakeSpan(files), opts, *workers, kvstore, cancelable);

    // As the slow path was already canceled, indexing will be canceled immediately.
    REQUIRE(!result.hasResult());
}

} // namespace sorbet
