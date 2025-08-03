#include "doctest/doctest.h"

#include "ast/ast.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "packager/packager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "test/helpers/MockFileSystem.h"
#include "test/helpers/packages.h"

using namespace std;

auto logger = spdlog::stderr_color_mt("pkg-unpackaged-test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

namespace sorbet {

TEST_CASE("Unpackaged files are not assigned to any package") {
    core::GlobalState gs(errorQueue);
    test::PackageHelpers::makeDefaultPackagerGlobalState(gs, test::PackageHelpers::LAYERS_UTIL_LIB_APP);
    
    // Create some unpackaged files and one packaged file for contrast
    vector<pair<string, string>> sources = {
        // Unpackaged files (no __package.rb in their directories)
        {"unpackaged_file.rb", "# frozen_string_literal: true\n"
                               "# typed: true\n"
                               "class UnpackagedClass\nend\n"},
        {"subdir/another_unpackaged.rb", "# frozen_string_literal: true\n"
                                         "# typed: true\n"
                                         "class AnotherUnpackaged\nend\n"},
        // Regular packaged file for comparison
        {"lib/mypackage/__package.rb", "class Lib::MyPackage < PackageSpec\nend\n"},
        {"lib/mypackage/packaged_file.rb", "# frozen_string_literal: true\n"
                                           "# typed: true\n"
                                           "class Lib::MyPackage::PackagedClass\nend\n"}
    };
    
    auto parsedFiles = test::PackageHelpers::enterPackages(gs, sources);
    
    // Find the file references
    core::FileRef unpackagedFile;
    core::FileRef anotherUnpackagedFile; 
    core::FileRef packagedFile;
    
    for (const auto &pf : parsedFiles) {
        string path{pf.file.data(gs).path()};
        if (path == "unpackaged_file.rb") {
            unpackagedFile = pf.file;
        } else if (path == "subdir/another_unpackaged.rb") {
            anotherUnpackagedFile = pf.file;
        } else if (path == "lib/mypackage/packaged_file.rb") {
            packagedFile = pf.file;
        }
    }
    
    // Verify files were found
    CHECK(unpackagedFile.exists());
    CHECK(anotherUnpackagedFile.exists());
    CHECK(packagedFile.exists());
    
    // Test that unpackaged files are NOT assigned to any package
    {
        auto unpackagedPkg = gs.packageDB().getPackageNameForFile(unpackagedFile);
        CHECK_FALSE(unpackagedPkg.exists());
    }
    
    // Test that another unpackaged file is also not assigned to any package
    {
        auto anotherUnpackagedPkg = gs.packageDB().getPackageNameForFile(anotherUnpackagedFile);
        CHECK_FALSE(anotherUnpackagedPkg.exists());
    }
    
    // Test that regular packaged file gets its proper package
    {
        auto packagedPkg = gs.packageDB().getPackageNameForFile(packagedFile);
        
        CHECK(packagedPkg.exists());
        
        const auto &pkgInfo = gs.packageDB().getPackageInfo(packagedPkg);
        auto fullName = pkgInfo.fullName();
        CHECK_EQ(2, fullName.size());  // Should be ["Lib", "MyPackage"]
        CHECK_EQ("Lib", fullName[0].show(gs));
        CHECK_EQ("MyPackage", fullName[1].show(gs));
    }
}

TEST_CASE("No synthetic __UNPACKAGED__ package is created") {
    core::GlobalState gs(errorQueue);
    test::PackageHelpers::makeDefaultPackagerGlobalState(gs, test::PackageHelpers::LAYERS_UTIL_LIB_APP);
    
    // Create at least one unpackaged file
    vector<pair<string, string>> sources = {
        {"unpackaged.rb", "# frozen_string_literal: true\nclass Test\nend\n"}
    };
    
    test::PackageHelpers::enterPackages(gs, sources);
    
    // Check that NO __UNPACKAGED__ package appears in the package list
    auto packages = gs.packageDB().packages();
    bool foundUnpackaged = false;
    
    for (auto pkg : packages) {
        const auto &pkgInfo = gs.packageDB().getPackageInfo(pkg);
        if (pkgInfo.exists()) {
            auto fullName = pkgInfo.fullName();
            if (fullName.size() == 1 && fullName[0].show(gs) == "__UNPACKAGED__") {
                foundUnpackaged = true;
                break;
            }
        }
    }
    
    CHECK_FALSE(foundUnpackaged);
}

} // namespace sorbet