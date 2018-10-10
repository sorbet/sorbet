#include "common/common.h"
#include "core/Context.h"
#include "core/Errors.h"
#include "core/Unfreeze.h"
#include "main/errorqueue/ConcurrentErrorQueue.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

using namespace std;

namespace spd = spdlog;

namespace sorbet {

TEST(ErrorTest, AutocorrectTest) { // NOLINT
    auto logger = spd::stderr_color_mt("autocorrect-test");
    auto errorQueue = make_shared<sorbet::realmain::ConcurrentErrorQueue>(*logger, *logger);

    sorbet::core::GlobalState gs(errorQueue);
    gs.initEmpty();
    gs.autocorrect = true;

    core::FileRef file(1);
    sorbet::core::UnfreezeFileTable ft(gs);

    map<core::FileRef, string> sources;
    map<core::FileRef, string> result;

    sources[file] = "123";
    vector<core::AutocorrectSuggestion> autocorrects;
    autocorrects.emplace_back(core::Loc(file, 0, 0), "start");
    autocorrects.emplace_back(core::Loc(file, 1, 1), "middle");
    autocorrects.emplace_back(core::Loc(file, 2, 2), "end");
    result = core::AutocorrectSuggestion::apply(move(autocorrects), sources);
    ASSERT_EQ("start1middle2end3", result[file]);

    sources[file] = "12345";
    autocorrects.emplace_back(core::Loc(file, 0, 2), "start");
    autocorrects.emplace_back(core::Loc(file, 1, 3), "middle");
    result = core::AutocorrectSuggestion::apply(move(autocorrects), sources);
    ASSERT_EQ("1middle45", result[file]);

    sources[file] = "123";
    autocorrects.emplace_back(core::Loc(file, 1, 2), "same");
    autocorrects.emplace_back(core::Loc(file, 1, 2), "same");
    result = core::AutocorrectSuggestion::apply(move(autocorrects), sources);
    ASSERT_EQ("1same3", result[file]);
}

} // namespace sorbet
