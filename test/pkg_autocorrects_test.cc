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

string examplePackage = "class ExamplePackage < PackageSpec\nend\n";
string examplePackagePath = "example/__package.rb";

namespace sorbet {
vector<ast::ParsedFile> enterPackages(core::GlobalState &gs, vector<pair<string, string>> packageSources) {
    vector<core::FileRef> files;
    {
        core::UnfreezeFileTable fileTableAccess(gs);
        for (auto &[filename, source] : packageSources) {
            files.emplace_back(gs.enterFile(filename, source));
        }
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
        packager::Packager::run(gs, *workers, absl::Span<ast::ParsedFile>(parsedFiles));
    }
    return parsedFiles;
}

const core::packages::PackageInfo &getPackageForFile(const core::GlobalState &gs, core::FileRef file) {
    return gs.packageDB().getPackageForFile(gs, file);
}

const core::SymbolRef getConstantRef(core::GlobalState &gs, vector<string> rawName) {
    core::UnfreezeNameTable nameTableAccess(gs);
    core::UnfreezeSymbolTable symbolTableAccess(gs);
    core::ClassOrModuleRef sym = core::Symbols::root();

    for (auto &n : rawName) {
        sym = gs.enterClassSymbol(core::Loc(), sym, gs.enterNameConstant(gs.enterNameUTF8(n)));
    }
    return sym;
}

TEST_CASE("Simple add import") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import SomethingElse\n"
                      "  import ExamplePackage\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = getPackageForFile(gs, parsedFiles[0].file);
    auto &myPkg = getPackageForFile(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, false);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Simple test import") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import SomethingElse\n"
                      "  test_import ExamplePackage\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = getPackageForFile(gs, parsedFiles[0].file);
    auto &myPkg = getPackageForFile(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, true);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Add import with only existing exports") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  export SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import ExamplePackage\n"
                      "  export SomethingElse\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = getPackageForFile(gs, parsedFiles[0].file);
    auto &myPkg = getPackageForFile(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, false);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Add test import with only existing exports") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  export SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  test_import ExamplePackage\n"
                      "  export SomethingElse\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = getPackageForFile(gs, parsedFiles[0].file);
    auto &myPkg = getPackageForFile(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, true);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Add import to package with neither imports nor exports") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import ExamplePackage\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = getPackageForFile(gs, parsedFiles[0].file);
    auto &myPkg = getPackageForFile(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, false);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Add test import to package with neither imports nor exports") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  test_import ExamplePackage\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = getPackageForFile(gs, parsedFiles[0].file);
    auto &myPkg = getPackageForFile(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, true);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = addImport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

TEST_CASE("Simple add export") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  export MyPackage::This\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  export MyPackage::This\n"
                      "  export MyPackage::NewExport\n"
                      "end\n";

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source}});
    auto &myPkg = getPackageForFile(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    auto addExport = myPkg.addExport(gs, getConstantRef(gs, {"MyPackage", "NewExport"}));
    ENFORCE(addExport, "Expected to get an autocorrect from `addExport`");
    auto replaced = addExport->applySingleEditForTesting(pkg_source);
    CHECK_EQ(expected, replaced);
}

} // namespace sorbet
