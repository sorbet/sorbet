#include "doctest/doctest.h"

#include "core/GlobalState.h"
#include "core/ErrorQueue.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "test/helpers/packages.h"

using namespace std;

namespace sorbet {

namespace {
auto logger = spdlog::stderr_color_mt("pkg-autocorrects-test");
auto errorQueue = make_shared<core::ErrorQueue>(*logger, *logger);
} // namespace

TEST_CASE("Condensation Graph - One package") {
    core::GlobalState gs(errorQueue);
    test::PackageHelpers::makeDefaultPackagerGlobalState(gs, test::PackageHelpers::LAYERS_UTIL_LIB_APP);

    // Enter a single package, and verify that we get out a condensation with two parallel layers.
    auto parsedFiles = test::PackageHelpers::enterPackages(
        gs, {{"lib/foo/a/__package.rb", test::PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib")}});

    auto &condensation = gs.packageDB().condensation();
    {
        INFO("The condensation graph should contain two packages: one for application code and one for test code");
        CHECK_EQ(2, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include two parallel layers, each with a single SCC");
        CHECK_EQ(2, traversal.parallel.size());
        CHECK_EQ(2, traversal.sccs.size());
        for (auto layer : traversal.parallel) {
            for (auto scc : layer) {
                CHECK_EQ(1, scc.members.size());
            }
        }
    }

    {
        INFO("Test packages must come after application code in the traversal");
        CHECK(!traversal.sccs[0].isTest);
        CHECK(traversal.sccs[1].isTest);
    }
}

TEST_CASE("Condensation Graph - Four packages without deps") {
    core::GlobalState gs(errorQueue);
    test::PackageHelpers::makeDefaultPackagerGlobalState(gs, test::PackageHelpers::LAYERS_UTIL_LIB_APP);

    auto parsedFiles = test::PackageHelpers::enterPackages(
        gs, {
                {"lib/foo/a/__package.rb", test::PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib")},
                {"lib/foo/b/__package.rb", test::PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "lib")},
                {"lib/foo/c/__package.rb", test::PackageHelpers::makePackageRB("Lib::Foo::C", "layered", "lib")},
                {"lib/foo/d/__package.rb", test::PackageHelpers::makePackageRB("Lib::Foo::D", "layered", "lib")},
            });

    auto &condensation = gs.packageDB().condensation();
    {
        INFO("The condensation graph should contain two nodes for each package");
        CHECK_EQ(8, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include two parallel layers, each with four SCCs");
        REQUIRE_EQ(2, traversal.parallel.size());
        CHECK_EQ(8, traversal.sccs.size());
        for (auto layer : traversal.parallel) {
            CHECK_EQ(4, layer.size());
            for (auto scc : layer) {
                CHECK_EQ(1, scc.members.size());
            }
        }
    }

    {
        INFO("The first layer should be all application code");
        for (auto scc : traversal.parallel[0]) {
            CHECK(!scc.isTest);
        }
    }

    {
        INFO("The second layer should be all test code");
        for (auto scc : traversal.parallel[1]) {
            CHECK(scc.isTest);
        }
    }
}

TEST_CASE("Condensation Graph - Four packages with a cycle of three") {
    core::GlobalState gs(errorQueue);
    test::PackageHelpers::makeDefaultPackagerGlobalState(gs, test::PackageHelpers::LAYERS_UTIL_LIB_APP);

    auto parsedFiles = test::PackageHelpers::enterPackages(
        gs, {{"lib/foo/a/__package.rb",
              test::PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib", {"Lib::Foo::B"}, {"Lib::Foo::B"})},
             {"lib/foo/b/__package.rb",
              test::PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "lib", {"Lib::Foo::D"}, {"Lib::Foo::C"})},
             {"lib/foo/c/__package.rb",
              test::PackageHelpers::makePackageRB("Lib::Foo::C", "layered", "lib", {"Lib::Foo::B"}, {})},
             {"lib/foo/d/__package.rb",
              test::PackageHelpers::makePackageRB("Lib::Foo::D", "layered", "lib", {"Lib::Foo::A"}, {})}});

    auto &condensation = gs.packageDB().condensation();
    auto traversal = condensation.computeTraversal(gs);

    auto pkgA = test::PackageHelpers::packageInfoFor(gs, parsedFiles[0].file).mangledName();
    auto pkgB = test::PackageHelpers::packageInfoFor(gs, parsedFiles[1].file).mangledName();
    auto pkgC = test::PackageHelpers::packageInfoFor(gs, parsedFiles[2].file).mangledName();
    auto pkgD = test::PackageHelpers::packageInfoFor(gs, parsedFiles[3].file).mangledName();

    {
        INFO("There should be three layers in the resulting traversal");
        REQUIRE_EQ(3, traversal.parallel.size());
    }

    {
        INFO("The first layer should be the A->B->D->A application code cycle");
        REQUIRE_EQ(1, traversal.parallel[0].size());
        CHECK_EQ(1, absl::c_count_if(traversal.parallel[0], [](auto &scc) { return !scc.isTest; }));

        for (auto pkg : {pkgA, pkgB, pkgD}) {
            INFO("Checking for application package " << pkg.owner.showFullName(gs));

            auto found = false;
            for (auto scc : traversal.parallel[0]) {
                CHECK(!scc.isTest);
                auto it = absl::c_find(scc.members, pkg);
                if (it != scc.members.end()) {
                    found = true;
                    break;
                }
            }
            CHECK(found);
        }
    }

    {
        INFO("The second layer should include only the application code of C");
        REQUIRE_EQ(1, traversal.parallel[1].size());
        CHECK_EQ(1, absl::c_count_if(traversal.parallel[1], [](auto &scc) { return !scc.isTest; }));

        for (auto pkg : {pkgC}) {
            INFO("Checking for application package " << pkg.owner.showFullName(gs));

            auto found = false;
            for (auto scc : traversal.parallel[1]) {
                if (scc.isTest) {
                    continue;
                }
                auto it = absl::c_find(scc.members, pkg);
                if (it != scc.members.end()) {
                    found = true;
                    break;
                }
            }
            CHECK(found);
        }
    }

    {
        INFO("The third layer should include all of the test packages");
        REQUIRE_EQ(1, traversal.parallel[2].size());
        CHECK_EQ(1, absl::c_count_if(traversal.parallel[2], [](auto &scc) { return scc.isTest; }));

        for (auto pkg : {pkgA, pkgB, pkgC, pkgD}) {
            INFO("Checking for test package " << pkg.owner.showFullName(gs));

            auto found = false;
            for (auto scc : traversal.parallel[2]) {
                if (!scc.isTest) {
                    continue;
                }
                auto it = absl::c_find(scc.members, pkg);
                if (it != scc.members.end()) {
                    found = true;
                    break;
                }
            }
            CHECK(found);
        }
    }
}

} // namespace sorbet
