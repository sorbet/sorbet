#include "doctest/doctest.h"

#include "core/ErrorQueue.h"
#include "core/GlobalState.h"
#include "main/options/options.h"
#include "main/pipeline/pipeline.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "test/helpers/packages.h"

using namespace std;

namespace sorbet::test {

namespace {
auto logger = spdlog::stderr_color_mt("pkg-strata-test");
auto errorQueue = make_shared<core::ErrorQueue>(*logger, *logger);
} // namespace

TEST_CASE("Package strata describe only the selected traversal") {
    core::GlobalState gs(errorQueue);
    PackageHelpers::makeDefaultPackagerGlobalState(gs);
    auto packageFiles = PackageHelpers::enterPackages(
        gs, {{"base/__package.rb", PackageHelpers::makePackageRB("Base", "false", "")},
             {"app/__package.rb", PackageHelpers::makePackageRB("App", "false", "", {"Base"})},
             {"other/__package.rb", PackageHelpers::makePackageRB("Other", "false", "", {"Long"})},
             {"long/__package.rb", PackageHelpers::makePackageRB("Long", "false", "", {"Base"})}});
    auto sources =
        realmain::pipeline::reserveFiles(gs, {"base/value.rb", "base/test/value.rb", "app/value.rb",
                                              "app/test/value.rb", "other/value.rb", "long/value.rb", "global.rbi"});
    const auto originalSources = sources;
    vector<core::FileRef> originalPackages;
    for (auto &package : packageFiles) {
        originalPackages.emplace_back(package.file);
    }
    for (auto file : sources) {
        gs.packageDB().setPackageNameForFile(file, gs.packageDB().findPackageByPath(gs, file));
    }

    realmain::options::Options opts;
    opts.packageDirected = true;
    bool selected = false;
    SUBCASE("Full traversal") {}
    SUBCASE("Selected consumer excludes other consumers of its dependency") {
        opts.typecheckPackages = {"App"};
        selected = true;
    }
    SUBCASE("Package declarations without ordinary sources") {
        opts.typecheckPackages = {"App"};
        auto result = realmain::pipeline::computePackageStrata(gs, packageFiles, {}, opts);
        REQUIRE_EQ(2, result.strata.size());
        for (auto &stratum : result.strata) {
            CHECK(stratum.sourceFiles.empty());
            CHECK_FALSE(stratum.packageFiles.empty());
        }
        return;
    }

    auto result = realmain::pipeline::computePackageStrata(gs, packageFiles, absl::MakeSpan(sources), opts);
    REQUIRE_EQ(selected ? 2 : 4, result.strata.size());

    // Application and legacy test sources keep their dependency order in the selected traversal.
    const vector<uint16_t> expectedStrata = {0, 1, 1, 2, 2, 1, 0};
    for (size_t i = 0; i < originalSources.size(); ++i) {
        auto file = originalSources[i];
        auto expected = selected && (i == 1 || i == 3 || i == 4 || i == 5)
                            ? realmain::pipeline::PackageStrata::UNSELECTED
                            : core::packages::Stratum(expectedStrata[i]);
        CHECK(result.fileToStratum[file.id()] == expected);
        // Strata construction must work before ordinary sources have been read.
        CHECK(file.dataAllowingUnsafe(gs).sourceType == core::File::Type::NotYetRead);
    }

    UnorderedSet<core::FileRef> seenSources;
    UnorderedMap<core::FileRef, int> seenPackages;
    for (size_t i = 0; i < result.strata.size(); ++i) {
        auto &stratum = result.strata[i];
        CHECK_FALSE(stratum.packageFiles.empty());
        for (auto file : stratum.sourceFiles) {
            CHECK(seenSources.insert(file).second);
            CHECK(result.fileToStratum[file.id()] == core::packages::Stratum(i));
        }
        for (auto &package : stratum.packageFiles) {
            CHECK(package.tree);
            ++seenPackages[package.file];
        }
    }
    CHECK_EQ(selected ? 3 : 7, seenSources.size());
    CHECK_EQ(selected ? 2 : 4, seenPackages.size());
    for (size_t i = 0; i < originalPackages.size(); ++i) {
        auto file = originalPackages[i];
        if (selected && i >= 2) {
            CHECK(result.fileToStratum[file.id()] == realmain::pipeline::PackageStrata::UNSELECTED);
            CHECK_FALSE(seenPackages.contains(file));
        } else {
            // Only the full traversal needs both application and test versions of legacy package declarations.
            CHECK_EQ(selected ? 1 : 2, seenPackages.at(file));
        }
    }
}

} // namespace sorbet::test
