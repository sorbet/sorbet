#include "doctest/doctest.h"

#include "common/sort/sort.h"
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

    // Enter a single package, and verify that we get out a condensation with one node.
    auto parsedFiles = PackageHelpers::enterPackages(
        gs, {{"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib")}});

    auto &condensation = gs.packageDB().condensation();
    {
        INFO("The condensation graph should contain one node");
        CHECK_EQ(1, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include one stratum with a single SCC");
        CHECK_EQ(1, traversal.strata.size());
        CHECK_EQ(1, traversal.sccs.size());
        for (auto stratum : traversal.strata) {
            for (auto scc : stratum) {
                CHECK_EQ(1, scc.members.size());
            }
        }
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
        INFO("The condensation graph should contain one node for each package");
        CHECK_EQ(2, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include two strata, and two SCCs");
        REQUIRE_EQ(2, traversal.strata.size());
        CHECK_EQ(2, traversal.sccs.size());
    }

    {
        INFO("The first stratum should be B (no deps)");
        CHECK_EQ(1, traversal.strata[0].size());
    }

    {
        INFO("The second stratum should be A (depends on B)");
        CHECK_EQ(1, traversal.strata[1].size());
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
        INFO("The condensation graph should contain one node for each package");
        CHECK_EQ(4, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        INFO("The traversal should include one stratum with four SCCs");
        REQUIRE_EQ(1, traversal.strata.size());
        CHECK_EQ(4, traversal.sccs.size());
        for (auto stratum : traversal.strata) {
            CHECK_EQ(4, stratum.size());
            for (auto scc : stratum) {
                CHECK_EQ(1, scc.members.size());
            }
        }
    }
}

TEST_CASE("Condensation Graph - Four packages with a cycle of three") {
    core::GlobalState gs(errorQueue);
    PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

    auto parsedFiles = PackageHelpers::enterPackages(
        gs,
        {{"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib", {"Lib::Foo::B"})},
         {"lib/foo/b/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "lib", {"Lib::Foo::D"})},
         {"lib/foo/c/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::C", "layered", "lib", {"Lib::Foo::B"})},
         {"lib/foo/d/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::D", "layered", "lib", {"Lib::Foo::A"})}});

    auto &condensation = gs.packageDB().condensation();
    auto traversal = condensation.computeTraversal(gs);

    auto pkgA = PackageHelpers::packageInfoFor(gs, parsedFiles[0].file).mangledName();
    auto pkgB = PackageHelpers::packageInfoFor(gs, parsedFiles[1].file).mangledName();
    auto pkgC = PackageHelpers::packageInfoFor(gs, parsedFiles[2].file).mangledName();
    auto pkgD = PackageHelpers::packageInfoFor(gs, parsedFiles[3].file).mangledName();

    {
        INFO("There should be two strata in the resulting traversal");
        REQUIRE_EQ(2, traversal.strata.size());
    }

    {
        INFO("The first stratum should be the A->B->D->A cycle (one SCC with 3 members)");
        REQUIRE_EQ(1, traversal.strata[0].size());

        for (auto pkg : {pkgA, pkgB, pkgD}) {
            INFO("Checking for package " << pkg.owner.showFullName(gs));

            auto found = false;
            for (auto scc : traversal.strata[0]) {
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
        INFO("The second stratum should include only C (depends on B which is in the cycle)");
        REQUIRE_EQ(1, traversal.strata[1].size());

        for (auto pkg : {pkgC}) {
            INFO("Checking for package " << pkg.owner.showFullName(gs));

            auto found = false;
            for (auto scc : traversal.strata[1]) {
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

// A imports B and C, and both import A, forming one SCC.
// Reversing A's imports changes discovery order within the SCC, but its member order should stay the same.
TEST_CASE("Condensation Graph - Stable package ordering within an SCC when imports are reordered") {
    for (auto imports : {vector<string>{"Lib::Foo::B", "Lib::Foo::C"}, vector<string>{"Lib::Foo::C", "Lib::Foo::B"}}) {
        INFO("First import: " << imports.front());
        core::GlobalState gs(errorQueue);
        PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

        // Enter packages in a nonalphabetical order; compute the expected order from their mangled names below.
        auto parsedFiles = PackageHelpers::enterPackages(
            gs, {{"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib", imports)},
                 {"lib/foo/c/__package.rb",
                  PackageHelpers::makePackageRB("Lib::Foo::C", "layered", "lib", {"Lib::Foo::A"})},
                 {"lib/foo/b/__package.rb",
                  PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "lib", {"Lib::Foo::A"})}});

        auto traversal = gs.packageDB().condensation().computeTraversal(gs);
        REQUIRE_EQ(1, traversal.strata.size());
        REQUIRE_EQ(1, traversal.sccs.size());

        vector<core::packages::MangledName> expected;
        for (auto &file : parsedFiles) {
            expected.emplace_back(PackageHelpers::packageInfoFor(gs, file.file).mangledName());
        }
        fast_sort(expected, [](auto left, auto right) { return left.owner.id() < right.owner.id(); });
        for (auto stratum : traversal.strata) {
            REQUIRE_EQ(1, stratum.size());
            CHECK(absl::c_equal(stratum[0].members, expected));
        }
    }
}

// A imports two independent cycles: B <-> D and C <-> E, which form separate SCCs in the same stratum.
// Reversing A's imports changes which SCC is discovered first, but their order within each stratum should stay the
// same.
TEST_CASE("Condensation Graph - Stable SCC ordering within a stratum when imports are reordered") {
    for (auto imports : {vector<string>{"Lib::Foo::B", "Lib::Foo::C"}, vector<string>{"Lib::Foo::C", "Lib::Foo::B"}}) {
        INFO("First import: " << imports.front());
        core::GlobalState gs(errorQueue);
        PackageHelpers::makeDefaultPackagerGlobalState(gs, PackageHelpers::LAYERS_UTIL_LIB_APP);

        // Enter packages in a nonalphabetical order; compute the expected order from their mangled names below.
        auto parsedFiles = PackageHelpers::enterPackages(
            gs, {{"lib/foo/a/__package.rb", PackageHelpers::makePackageRB("Lib::Foo::A", "layered", "lib", imports)},
                 {"lib/foo/c/__package.rb",
                  PackageHelpers::makePackageRB("Lib::Foo::C", "layered", "lib", {"Lib::Foo::E"})},
                 {"lib/foo/e/__package.rb",
                  PackageHelpers::makePackageRB("Lib::Foo::E", "layered", "lib", {"Lib::Foo::C"})},
                 {"lib/foo/b/__package.rb",
                  PackageHelpers::makePackageRB("Lib::Foo::B", "layered", "lib", {"Lib::Foo::D"})},
                 {"lib/foo/d/__package.rb",
                  PackageHelpers::makePackageRB("Lib::Foo::D", "layered", "lib", {"Lib::Foo::B"})}});

        auto pkgA = PackageHelpers::packageInfoFor(gs, parsedFiles[0].file).mangledName();
        auto pkgC = PackageHelpers::packageInfoFor(gs, parsedFiles[1].file).mangledName();
        auto pkgE = PackageHelpers::packageInfoFor(gs, parsedFiles[2].file).mangledName();
        auto pkgB = PackageHelpers::packageInfoFor(gs, parsedFiles[3].file).mangledName();
        auto pkgD = PackageHelpers::packageInfoFor(gs, parsedFiles[4].file).mangledName();

        auto traversal = gs.packageDB().condensation().computeTraversal(gs);
        vector<vector<core::packages::MangledName>> expectedCycles = {{pkgB, pkgD}, {pkgC, pkgE}};
        for (auto &members : expectedCycles) {
            fast_sort(members, [](auto left, auto right) { return left.owner.id() < right.owner.id(); });
        }
        fast_sort(expectedCycles, [](const auto &left, const auto &right) {
            return absl::c_lexicographical_compare(
                left, right, [](auto left, auto right) { return left.owner.id() < right.owner.id(); });
        });
        vector<vector<vector<core::packages::MangledName>>> expectedStrata = {
            {expectedCycles[0], expectedCycles[1]},
            {{pkgA}},
        };
        REQUIRE_EQ(expectedStrata.size(), traversal.strata.size());
        CHECK_EQ(3, traversal.sccs.size());
        size_t offset = 0;
        for (size_t i = 0; i < expectedStrata.size(); ++i) {
            INFO("Stratum: " << i);
            auto stratum = traversal.strata[i];
            REQUIRE_EQ(expectedStrata[i].size(), stratum.size());
            for (size_t j = 0; j < stratum.size(); ++j) {
                INFO("SCC: " << j);
                auto &scc = stratum[j];
                CHECK(absl::c_equal(scc.members, expectedStrata[i][j]));
                CHECK(scc.members.data() == traversal.packages.data() + offset);
                offset += scc.members.size();
            }
        }
        CHECK_EQ(traversal.packages.size(), offset);
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
        INFO("The condensation graph should contain one node for each package");
        CHECK_EQ(4, condensation.nodes().size());
    }

    auto traversal = condensation.computeTraversal(gs);

    {
        REQUIRE_EQ(2, traversal.strata.size());
        CHECK_EQ(condensation.nodes().size(), traversal.sccs.size());
    }

    {
        INFO("The first stratum should be all prelude packages");
        REQUIRE_EQ(2, traversal.strata[0].size());
        for (auto scc : traversal.strata[0]) {
            for (auto pkg : scc.members) {
                CHECK(gs.packageDB().getPackageInfo(pkg).isPreludePackage());
            }
        }

        auto pkgA = PackageHelpers::packageInfoFor(gs, parsedFiles[0].file).mangledName();
        auto pkgB = PackageHelpers::packageInfoFor(gs, parsedFiles[1].file).mangledName();
        vector<core::packages::MangledName> expected = {pkgA, pkgB};
        fast_sort(expected, [](auto left, auto right) { return left.owner.id() < right.owner.id(); });
        for (size_t i = 0; i < expected.size(); ++i) {
            auto &scc = traversal.strata[0][i];
            REQUIRE_EQ(1, scc.members.size());
            CHECK_EQ(expected[i], scc.members[0]);
        }
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
        INFO("The second stratum should be Lib::Foo::A and Lib::Foo::B");
        REQUIRE_EQ(2, traversal.strata[1].size());
    }
}

} // namespace sorbet::test
