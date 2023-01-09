#include "doctest/doctest.h"

#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/NameSubstitution.h"
#include "core/Unfreeze.h"
#include "local_vars/local_vars.h"
#include "packager/packager.h"
#include "parser/parser.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

auto logger = spdlog::stderr_color_mt("pkg-autocorrects-test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

string examplePackage = "class Opus::ExamplePackage < PackageSpec\nend\n";
string examplePackagePath = "example/__package.rb";

namespace sorbet {
struct TestPackageFile {
    // the `ParsedFile` corresponding to the package in which we want
    // to make the edit
    ast::ParsedFile targetParsedFile;
    // the `ParsedFile` corresponding to the package that we want to
    // add.
    ast::ParsedFile newParsedFile;

    TestPackageFile(ast::ParsedFile targetParsedFile, ast::ParsedFile newParsedFile)
        : targetParsedFile(move(targetParsedFile)), newParsedFile(move(newParsedFile)) {}

    static TestPackageFile create(core::GlobalState &gs, string filename, string source) {
        // add the package file
        vector<core::FileRef> files;
        {
            core::UnfreezeFileTable fileTableAccess(gs);
            files.emplace_back(gs.enterFile(filename, source));
            files.emplace_back(gs.enterFile(examplePackagePath, examplePackage));
        }

        // run through the pipeline up through the packager
        // start by parsing and desugaring
        vector<ast::ParsedFile> parsedFiles;
        {
            core::UnfreezeNameTable nameTableAccess(gs);
            // run parser
            for (auto file : files) {
                auto settings = parser::Parser::Settings{};
                auto nodes = parser::Parser::run(gs, file, settings);

                core::MutableContext ctx(gs, core::Symbols::root(), file);
                auto parsedFile = ast::ParsedFile{ast::desugar::node2Tree(ctx, move(nodes)), file};
                parsedFiles.emplace_back(local_vars::LocalVars::run(ctx, move(parsedFile)));
            }
        }

        {
            // and then finally the packager!
            auto workers = WorkerPool::create(0, gs.tracer());
            parsedFiles = packager::Packager::run(gs, *workers, move(parsedFiles));
        }

        TestPackageFile pkgFile(move(parsedFiles.front()), move(parsedFiles[1]));

        return pkgFile;
    }

    const core::packages::PackageInfo &targetPackage(core::GlobalState &gs) const {
        return gs.packageDB().getPackageForFile(gs, targetParsedFile.file);
    }

    const core::packages::PackageInfo &newPackage(core::GlobalState &gs) const {
        return gs.packageDB().getPackageForFile(gs, newParsedFile.file);
    }

    const core::SymbolRef getConstantRef(core::GlobalState &gs, vector<string> rawName) const {
        core::UnfreezeNameTable nameTableAccess(gs);
        core::UnfreezeSymbolTable symbolTableAccess(gs);
        core::ClassOrModuleRef sym = core::Symbols::root();

        for (auto &n : rawName) {
            sym = gs.enterClassSymbol(core::Loc(), sym, gs.enterNameConstant(gs.enterNameUTF8(n)));
        }
        return sym;
    }
};

TEST_CASE("Simple add import") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class Opus::MyPackage < PackageSpec\n"
                        "  import Opus::SomethingElse\n"
                        "end\n";

    string expected = "class Opus::MyPackage < PackageSpec\n"
                      "  import Opus::SomethingElse\n"
                      "  import Opus::ExamplePackage\n"
                      "end\n";

    auto test = TestPackageFile::create(gs, "my_package/__package.rb", pkg_source);

    auto &package = test.targetPackage(gs);
    ENFORCE(package.exists());
    auto addImport = package.addImport(gs, test.newPackage(gs), false);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Simple test import") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class Opus::MyPackage < PackageSpec\n"
                        "  import Opus::SomethingElse\n"
                        "end\n";

    string expected = "class Opus::MyPackage < PackageSpec\n"
                      "  import Opus::SomethingElse\n"
                      "  test_import Opus::ExamplePackage\n"
                      "end\n";

    auto test = TestPackageFile::create(gs, "my_package/__package.rb", pkg_source);

    auto &package = test.targetPackage(gs);
    ENFORCE(package.exists());
    auto addImport = package.addImport(gs, test.newPackage(gs), true);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Add import with only existing exports") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class Opus::MyPackage < PackageSpec\n"
                        "  export Opus::SomethingElse\n"
                        "end\n";

    string expected = "class Opus::MyPackage < PackageSpec\n"
                      "  import Opus::ExamplePackage\n"
                      "  export Opus::SomethingElse\n"
                      "end\n";

    auto test = TestPackageFile::create(gs, "my_package/__package.rb", pkg_source);

    auto &package = test.targetPackage(gs);
    ENFORCE(package.exists());
    auto addImport = package.addImport(gs, test.newPackage(gs), false);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Add test import with only existing exports") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class Opus::MyPackage < PackageSpec\n"
                        "  export Opus::SomethingElse\n"
                        "end\n";

    string expected = "class Opus::MyPackage < PackageSpec\n"
                      "  test_import Opus::ExamplePackage\n"
                      "  export Opus::SomethingElse\n"
                      "end\n";

    auto test = TestPackageFile::create(gs, "my_package/__package.rb", pkg_source);

    auto &package = test.targetPackage(gs);
    ENFORCE(package.exists());
    auto addImport = package.addImport(gs, test.newPackage(gs), true);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Add import to package with neither imports nor exports") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class Opus::MyPackage < PackageSpec\n"
                        "end\n";

    string expected = "class Opus::MyPackage < PackageSpec\n"
                      "  import Opus::ExamplePackage\n"
                      "end\n";

    auto test = TestPackageFile::create(gs, "my_package/__package.rb", pkg_source);

    auto &package = test.targetPackage(gs);
    ENFORCE(package.exists());
    auto addImport = package.addImport(gs, test.newPackage(gs), false);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Add test import to package with neither imports nor exports") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class Opus::MyPackage < PackageSpec\n"
                        "end\n";

    string expected = "class Opus::MyPackage < PackageSpec\n"
                      "  test_import Opus::ExamplePackage\n"
                      "end\n";

    auto test = TestPackageFile::create(gs, "my_package/__package.rb", pkg_source);

    auto &package = test.targetPackage(gs);
    ENFORCE(package.exists());
    auto addImport = package.addImport(gs, test.newPackage(gs), true);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Simple add export") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class Opus::MyPackage < PackageSpec\n"
                        "  export Opus::MyPackage::This\n"
                        "end\n";

    string expected = "class Opus::MyPackage < PackageSpec\n"
                      "  export Opus::MyPackage::This\n"
                      "  export Opus::MyPackage::NewExport\n"
                      "end\n";

    auto test = TestPackageFile::create(gs, "my_package/__package.rb", pkg_source);

    auto &package = test.targetPackage(gs);
    ENFORCE(package.exists());
    auto addExport = package.addExport(gs, test.getConstantRef(gs, {"Opus", "MyPackage", "NewExport"}));
    ENFORCE(addExport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addExport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

} // namespace sorbet
