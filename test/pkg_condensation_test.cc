#include "doctest/doctest.h"

#include "core/ErrorQueue.h"
#include "core/GlobalState.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "test/helpers/packages.h"

using namespace std;

namespace sorbet::test {

namespace {
auto logger = spdlog::stderr_color_mt("pkg-autocorrects-test");
auto errorQueue = make_shared<core::ErrorQueue>(*logger, *logger);
} // namespace

TEST_CASE("Condensation Graph - One package") {
    core::GlobalState gs(errorQueue);
    PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

    // Enter a single package, and verify that we get out a condensation with two strata.
    auto parsedFiles = PackageHelpers::enterPackages(
        gs, {{"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib")}});

    auto &condensation = gs.packageDB().condensation();
    {
        INFO("The condensation graph should contain two packages: one for application code and one for test code");
        CHECK_EQ(2, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include two strata, each with a single SCC");
        CHECK_EQ(2, traversal.strata.size());
        CHECK_EQ(2, traversal.sccs.size());
        for (auto stratum : traversal.strata) {
            for (auto scc : stratum) {
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

TEST_CASE("Condensation Graph - Two packages") {
    core::GlobalState gs(errorQueue);
    PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

    auto parsedFiles = PackageHelpers::enterPackages(
        gs,
        {{"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib", {"Lib::Foo::B"})},
         {"lib/foo/b/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "lib")}});

    auto &condensation = gs.packageDB().condensation();
    {
        INFO("The condensation graph should contain two nodes for each package");
        CHECK_EQ(4, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include three strata, and four SCCs");
        REQUIRE_EQ(3, traversal.strata.size());
        CHECK_EQ(4, traversal.sccs.size());
    }

    {
        INFO("The first stratum should be B's application code");
        CHECK_EQ(1, absl::c_count_if(traversal.strata[0], [](auto &scc) { return !scc.isTest; }));
        CHECK_EQ(0, absl::c_count_if(traversal.strata[0], [](auto &scc) { return scc.isTest; }));
    }

    {
        INFO("The second stratum should be A's application code and B's test");
        CHECK_EQ(1, absl::c_count_if(traversal.strata[1], [](auto &scc) { return !scc.isTest; }));
        CHECK_EQ(1, absl::c_count_if(traversal.strata[1], [](auto &scc) { return scc.isTest; }));
    }

    {
        INFO("The third stratum should be A's application code");
        CHECK_EQ(0, absl::c_count_if(traversal.strata[2], [](auto &scc) { return !scc.isTest; }));
        CHECK_EQ(1, absl::c_count_if(traversal.strata[2], [](auto &scc) { return scc.isTest; }));
    }
}

TEST_CASE("Condensation Graph - Four packages without deps") {
    core::GlobalState gs(errorQueue);
    PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

    auto parsedFiles = PackageHelpers::enterPackages(
        gs, {
                {"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib")},
                {"lib/foo/b/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "lib")},
                {"lib/foo/c/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::C", "layered", "lib")},
                {"lib/foo/d/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::D", "layered", "lib")},
            });

    auto &condensation = gs.packageDB().condensation();
    {
        INFO("The condensation graph should contain two nodes for each package");
        CHECK_EQ(8, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include two strata, each with four SCCs");
        REQUIRE_EQ(2, traversal.strata.size());
        CHECK_EQ(8, traversal.sccs.size());
        for (auto stratum : traversal.strata) {
            CHECK_EQ(4, stratum.size());
            for (auto scc : stratum) {
                CHECK_EQ(1, scc.members.size());
            }
        }
    }

    {
        INFO("The first stratum should be all application code");
        for (auto scc : traversal.strata[0]) {
            CHECK(!scc.isTest);
        }
    }

    {
        INFO("The second stratum should be all test code");
        for (auto scc : traversal.strata[1]) {
            CHECK(scc.isTest);
        }
    }
}

TEST_CASE("Condensation Graph - Four packages with a cycle of three") {
    core::GlobalState gs(errorQueue);
    PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

    auto parsedFiles = PackageHelpers::enterPackages(
        gs, {{"lib/foo/a/__package.rb",
              PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib", {"Lib::Foo::B"}, {"Lib::Foo::B"})},
             {"lib/foo/b/__package.rb",
              PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "lib", {"Lib::Foo::D"}, {"Lib::Foo::C"})},
             {"lib/foo/c/__package.rb",
              PackageHelpers::makePackageRB("Lib::Foo::C", "layered", "lib", {"Lib::Foo::B"}, {})},
             {"lib/foo/d/__package.rb",
              PackageHelpers::makePackageRB("Lib::Foo::D", "layered", "lib", {"Lib::Foo::A"}, {})}});

    auto &condensation = gs.packageDB().condensation();
    auto traversal = condensation.computeTraversal(gs);

    auto pkgA = PackageHelpers::packageInfoFor(gs, parsedFiles[0].file).mangledName();
    auto pkgB = PackageHelpers::packageInfoFor(gs, parsedFiles[1].file).mangledName();
    auto pkgC = PackageHelpers::packageInfoFor(gs, parsedFiles[2].file).mangledName();
    auto pkgD = PackageHelpers::packageInfoFor(gs, parsedFiles[3].file).mangledName();

    {
        INFO("There should be three strata in the resulting traversal");
        REQUIRE_EQ(3, traversal.strata.size());
    }

    {
        INFO("The first stratum should be the A->B->D->A application code cycle");
        REQUIRE_EQ(1, traversal.strata[0].size());
        CHECK_EQ(1, absl::c_count_if(traversal.strata[0], [](auto &scc) { return !scc.isTest; }));

        for (auto pkg : {pkgA, pkgB, pkgD}) {
            INFO("Checking for application package " << pkg.owner.showFullName(gs));

            auto found = false;
            for (auto scc : traversal.strata[0]) {
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
        INFO("The second stratum should include only the application code of C");
        REQUIRE_EQ(1, traversal.strata[1].size());
        CHECK_EQ(1, absl::c_count_if(traversal.strata[1], [](auto &scc) { return !scc.isTest; }));

        for (auto pkg : {pkgC}) {
            INFO("Checking for application package " << pkg.owner.showFullName(gs));

            auto found = false;
            for (auto scc : traversal.strata[1]) {
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
        INFO("The third stratum should include all of the test packages");
        REQUIRE_EQ(1, traversal.strata[2].size());
        CHECK_EQ(1, absl::c_count_if(traversal.strata[2], [](auto &scc) { return scc.isTest; }));

        for (auto pkg : {pkgA, pkgB, pkgC, pkgD}) {
            INFO("Checking for test package " << pkg.owner.showFullName(gs));

            auto found = false;
            for (auto scc : traversal.strata[2]) {
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

TEST_CASE("Condensation Graph - Two prelude packages, two normal packages") {
    core::GlobalState gs(errorQueue);
    PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

    auto parsedFiles = PackageHelpers::enterPackages(
        gs,
        {
            {"prelude/a/__package.rb", PackageTextBuilder()
                                           .withName("Prelude::A")
                                           .withStrictDeps("layered")
                                           .withLayer("lib")
                                           .withPreludePackage()
                                           .build()},
            {"prelude/b/__package.rb", PackageTextBuilder()
                                           .withName("Prelude::B")
                                           .withStrictDeps("layered")
                                           .withLayer("lib")
                                           .withImports({"Prelude::A"})
                                           .withPreludePackage()
                                           .build()},
            // No imports, so that it would be treated as a package root without prelude packages being present.
            {"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "app")},

            // Only importing Prelude::A, so that it technically could be in the same stratum as Prelude::B without
            // the requirement that all prelude packages are processed first.
            {"lib/foo/b/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "app", {"Prelude::A"})},
        });

    auto &condensation = gs.packageDB().condensation();
    {
        INFO("The condensation graph should contain two nodes for each package");
        CHECK_EQ(8, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        REQUIRE_EQ(3, traversal.strata.size());
        CHECK_EQ(condensation.nodes().size(), traversal.sccs.size());
    }

    {
        INFO("The first stratum should be all prelude packages");
        REQUIRE_EQ(4, traversal.strata[0].size());
        for (auto scc : traversal.strata[0]) {
            for (auto pkg : scc.members) {
                CHECK(gs.packageDB().getPackageInfo(pkg).isPreludePackage());
            }
        }
        CHECK_EQ(2, absl::c_count_if(traversal.strata[0], [](auto &scc) { return scc.isTest; }));
        CHECK_EQ(2, absl::c_count_if(traversal.strata[0], [](auto &scc) { return !scc.isTest; }));
    }

    {
        INFO("There should be no prelude packages in layers other than the first");
        for (auto &stratum : absl::MakeSpan(traversal.strata).subspan(1)) {
            for (auto scc : stratum) {
                for (auto pkg : scc.members) {
                    CHECK(!gs.packageDB().getPackageInfo(pkg).isPreludePackage());
                }
            }
        }
    }

    {
        // Lib::Foo::A is here because it's the first stratum where a non-prelude package could show up.
        // Lib::Foo::B is here because all of its imports have been satisfied while traversing the prelude set, and it's
        // the first stratum where a non-prelude package could show up.
        INFO("The second stratum should be the application code of Lib::Foo::A and Lib::Foo::B");
        REQUIRE_EQ(2, traversal.strata[1].size());
        for (auto scc : traversal.strata[1]) {
            CHECK(!scc.isTest);
        }
    }

    {
        INFO("The third stratum should be the test code of Lib::Foo::A and Lib::Foo::B");
        REQUIRE_EQ(2, traversal.strata[2].size());
        for (auto scc : traversal.strata[2]) {
            CHECK(scc.isTest);
        }
    }
}

TEST_CASE("Condensation Graph - Two packages, one is test-only") {
    core::GlobalState gs(errorQueue);
    PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

    auto parsedFiles = PackageHelpers::enterPackages(
        gs, {
                {"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib")},
                {"lib/foo/test/b/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::Test::B", "layered", "lib")},
            });

    auto &condensation = gs.packageDB().condensation();
    {
        INFO("The condensation graph should contain two nodes for Lib::Foo::A, and one for Lib::Foo::Test::B");
        CHECK_EQ(3, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include the same number of nodes as the condensation");
        CHECK_EQ(condensation.nodes().size(), traversal.sccs.size());
    }

    {
        INFO("The traversal should include two strata");
        REQUIRE_EQ(2, traversal.strata.size());
    }

    {
        INFO("The first stratum is a mix of application and test packages");
        CHECK_EQ(2, traversal.strata[0].size());
        CHECK_EQ(1, absl::c_count_if(traversal.strata[0], [](auto &scc) { return !scc.isTest; }));
        CHECK_EQ(1, absl::c_count_if(traversal.strata[0], [](auto &scc) { return scc.isTest; }));
    }

    {
        INFO("The second stratum should be all test code");
        CHECK_EQ(1, traversal.strata[1].size());
        CHECK_EQ(1, absl::c_count_if(traversal.strata[1], [](auto &scc) { return scc.isTest; }));
    }
}

} // namespace sorbet::test
