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
string makePackageRB(string name, string strictDeps, string layer, vector<string> imports = {},
                     vector<string> exports = {}) {
    string importList = "";
    for (auto &import : imports) {
        importList += fmt::format("  import {}\n", import);
    }
    return fmt::format("class {} < PackageSpec\n"
                       "  strict_dependencies '{}'\n"
                       "  layer '{}'\n"
                       "{}"
                       "end\n",
                       name, strictDeps, layer, importList);
}

string falsePackageA = makePackageRB("FalsePackageA", "false", "lib");
string falsePackageAPath = "falseA/__package.rb";

string falsePackageB = makePackageRB("FalsePackageB", "false", "lib");
string falsePackageBPath = "falseB/__package.rb";

string layeredPackageA = makePackageRB("LayeredPackageA", "layered", "lib");
string layeredPackageAPath = "layeredA/__package.rb";

string layeredPackageB = makePackageRB("LayeredPackageB", "layered", "lib");
string layeredPackageBPath = "layeredB/__package.rb";

string layeredDagPackageA = makePackageRB("LayeredDagPackageA", "layered_dag", "lib");
string layeredDagPackageAPath = "layered_dagA/__package.rb";

string layeredDagPackageB = makePackageRB("LayeredDagPackageB", "layered_dag", "lib");
string layeredDagPackageBPath = "layered_dagB/__package.rb";

string dagPackageA = makePackageRB("DagPackageA", "dag", "lib");
string dagPackageAPath = "dagA/__package.rb";

string dagPackageB = makePackageRB("DagPackageB", "dag", "lib");
string dagPackageBPath = "dagB/__package.rb";

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

// Tests with layering and strict dependencies checks on
TEST_CASE("Add imports to strict_dependencies 'false' package") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();
    {
        core::UnfreezeNameTable packageNS(gs);
        core::packages::UnfreezePackages unfreezeToEnterPackagerOptionsPackageDB = gs.unfreezePackages();
        gs.setPackagerOptions({}, {}, {}, {}, {}, {"lib", "app"}, "");
    }

    string pkg_source = makePackageRB("MyPackage", "false", "app",
                                      {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});

    auto parsedFiles = enterPackages(gs, {{"my_Package/__package.rb", pkg_source},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {layeredDagPackageAPath, layeredDagPackageA},
                                          {dagPackageAPath, dagPackageA},
                                          {falsePackageBPath, falsePackageB},
                                          {layeredPackageBPath, layeredPackageB},
                                          {layeredDagPackageBPath, layeredDagPackageB},
                                          {dagPackageBPath, dagPackageB}});
    auto &myPkg = getPackageForFile(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    {
        auto &falsePkgB = getPackageForFile(gs, parsedFiles[5].file);
        ENFORCE(falsePkgB.exists());
        auto addImport = myPkg.addImport(gs, falsePkgB, false);
        string expected =
            makePackageRB("MyPackage", "false", "app",
                          {"FalsePackageA", "FalsePackageB", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &layeredPkgB = getPackageForFile(gs, parsedFiles[6].file);
        ENFORCE(layeredPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredPkgB, false);
        string expected =
            makePackageRB("MyPackage", "false", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "LayeredPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &layeredDagPkgB = getPackageForFile(gs, parsedFiles[7].file);
        ENFORCE(layeredDagPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredDagPkgB, false);
        string expected = makePackageRB(
            "MyPackage", "false", "app",
            {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "LayeredDagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &dagPkgB = getPackageForFile(gs, parsedFiles[8].file);
        ENFORCE(dagPkgB.exists());
        auto addImport = myPkg.addImport(gs, dagPkgB, false);
        string expected =
            makePackageRB("MyPackage", "false", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "DagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }
}

TEST_CASE("Add imports to strict_dependencies 'layered' package") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();
    {
        core::UnfreezeNameTable packageNS(gs);
        core::packages::UnfreezePackages unfreezeToEnterPackagerOptionsPackageDB = gs.unfreezePackages();
        gs.setPackagerOptions({}, {}, {}, {}, {}, {"lib", "app"}, "");
    }

    string pkg_source = makePackageRB("MyPackage", "layered", "app",
                                      {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});

    auto parsedFiles = enterPackages(gs, {{"my_Package/__package.rb", pkg_source},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {layeredDagPackageAPath, layeredDagPackageA},
                                          {dagPackageAPath, dagPackageA},
                                          {falsePackageBPath, falsePackageB},
                                          {layeredPackageBPath, layeredPackageB},
                                          {layeredDagPackageBPath, layeredDagPackageB},
                                          {dagPackageBPath, dagPackageB}});
    auto &myPkg = getPackageForFile(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    {
        auto &falsePkgB = getPackageForFile(gs, parsedFiles[5].file);
        ENFORCE(falsePkgB.exists());
        auto addImport = myPkg.addImport(gs, falsePkgB, false);
        string expected =
            makePackageRB("MyPackage", "layered", "app",
                          {"FalsePackageA", "FalsePackageB", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &layeredPkgB = getPackageForFile(gs, parsedFiles[6].file);
        ENFORCE(layeredPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredPkgB, false);
        string expected =
            makePackageRB("MyPackage", "layered", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &layeredDagPkgB = getPackageForFile(gs, parsedFiles[7].file);
        ENFORCE(layeredDagPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredDagPkgB, false);
        string expected = makePackageRB(
            "MyPackage", "layered", "app",
            {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredDagPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &dagPkgB = getPackageForFile(gs, parsedFiles[8].file);
        ENFORCE(dagPkgB.exists());
        auto addImport = myPkg.addImport(gs, dagPkgB, false);
        string expected =
            makePackageRB("MyPackage", "layered", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "DagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }
}

TEST_CASE("Add imports to strict_dependencies 'layered_dag' package") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();
    {
        core::UnfreezeNameTable packageNS(gs);
        core::packages::UnfreezePackages unfreezeToEnterPackagerOptionsPackageDB = gs.unfreezePackages();
        gs.setPackagerOptions({}, {}, {}, {}, {}, {"lib", "app"}, "");
    }

    string pkg_source = makePackageRB("MyPackage", "layered_dag", "app",
                                      {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});

    auto parsedFiles = enterPackages(gs, {{"my_Package/__package.rb", pkg_source},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {layeredDagPackageAPath, layeredDagPackageA},
                                          {dagPackageAPath, dagPackageA},
                                          {falsePackageBPath, falsePackageB},
                                          {layeredPackageBPath, layeredPackageB},
                                          {layeredDagPackageBPath, layeredDagPackageB},
                                          {dagPackageBPath, dagPackageB}});
    auto &myPkg = getPackageForFile(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    {
        auto &falsePkgB = getPackageForFile(gs, parsedFiles[5].file);
        ENFORCE(falsePkgB.exists());
        auto addImport = myPkg.addImport(gs, falsePkgB, false);
        string expected =
            makePackageRB("MyPackage", "layered_dag", "app",
                          {"FalsePackageA", "FalsePackageB", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &layeredPkgB = getPackageForFile(gs, parsedFiles[6].file);
        ENFORCE(layeredPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredPkgB, false);
        string expected =
            makePackageRB("MyPackage", "layered_dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &layeredDagPkgB = getPackageForFile(gs, parsedFiles[7].file);
        ENFORCE(layeredDagPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredDagPkgB, false);
        string expected = makePackageRB(
            "MyPackage", "layered_dag", "app",
            {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredDagPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &dagPkgB = getPackageForFile(gs, parsedFiles[8].file);
        ENFORCE(dagPkgB.exists());
        auto addImport = myPkg.addImport(gs, dagPkgB, false);
        string expected =
            makePackageRB("MyPackage", "layered_dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "DagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }
}

TEST_CASE("Add imports to strict_dependencies 'dag' package") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();
    {
        core::UnfreezeNameTable packageNS(gs);
        core::packages::UnfreezePackages unfreezeToEnterPackagerOptionsPackageDB = gs.unfreezePackages();
        gs.setPackagerOptions({}, {}, {}, {}, {}, {"lib", "app"}, "");
    }

    string pkg_source = makePackageRB("MyPackage", "dag", "app",
                                      {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});

    auto parsedFiles = enterPackages(gs, {{"my_Package/__package.rb", pkg_source},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {layeredDagPackageAPath, layeredDagPackageA},
                                          {dagPackageAPath, dagPackageA},
                                          {falsePackageBPath, falsePackageB},
                                          {layeredPackageBPath, layeredPackageB},
                                          {layeredDagPackageBPath, layeredDagPackageB},
                                          {dagPackageBPath, dagPackageB}});
    auto &myPkg = getPackageForFile(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    {
        auto &falsePkgB = getPackageForFile(gs, parsedFiles[5].file);
        ENFORCE(falsePkgB.exists());
        auto addImport = myPkg.addImport(gs, falsePkgB, false);
        string expected =
            makePackageRB("MyPackage", "dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "FalsePackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &layeredPkgB = getPackageForFile(gs, parsedFiles[6].file);
        ENFORCE(layeredPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredPkgB, false);
        string expected =
            makePackageRB("MyPackage", "dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &layeredDagPkgB = getPackageForFile(gs, parsedFiles[7].file);
        ENFORCE(layeredDagPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredDagPkgB, false);
        string expected = makePackageRB(
            "MyPackage", "dag", "app",
            {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredDagPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }

    {
        auto &dagPkgB = getPackageForFile(gs, parsedFiles[8].file);
        ENFORCE(dagPkgB.exists());
        auto addImport = myPkg.addImport(gs, dagPkgB, false);
        string expected =
            makePackageRB("MyPackage", "dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "DagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(pkg_source);
        CHECK_EQ(expected, replaced);
    }
}

TEST_CASE("Edge cases") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();
    {
        core::UnfreezeNameTable packageNS(gs);
        core::packages::UnfreezePackages unfreezeToEnterPackagerOptionsPackageDB = gs.unfreezePackages();
        gs.setPackagerOptions({}, {}, {}, {}, {}, {"lib", "app"}, "");
    }

    string packageWithFakeImport = makePackageRB("HasFakeImport", "false", "app", {"FakeImport"});
    string packageWithFakeImportPath = "has_fake_import/__package.rb";

    string libPackage = makePackageRB("LibPackage", "false", "lib", {"FalsePackageA"});
    string libPackagePath = "lib_pkg/__package.rb";

    string appPackage = makePackageRB("AppPackage", "false", "app", {});
    string appPackagePath = "app_pkg/__package.rb";

    auto parsedFiles = enterPackages(gs, {{packageWithFakeImportPath, packageWithFakeImport},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {libPackagePath, libPackage},
                                          {appPackagePath, appPackage}});

    {
        // Import list contains non-existent package
        auto &fakeImportPkg = getPackageForFile(gs, parsedFiles[0].file);
        ENFORCE(fakeImportPkg.exists());
        auto &layeredPkgA = getPackageForFile(gs, parsedFiles[2].file);
        ENFORCE(layeredPkgA.exists());

        auto addImport = fakeImportPkg.addImport(gs, layeredPkgA, false);
        string expected = makePackageRB("HasFakeImport", "false", "app", {"FakeImport", "LayeredPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(packageWithFakeImport);
        CHECK_EQ(expected, replaced);
    }

    {
        // Import added to start of import list
        auto &libPkg = getPackageForFile(gs, parsedFiles[3].file);
        ENFORCE(libPkg.exists());
        auto &appPkg = getPackageForFile(gs, parsedFiles[4].file);
        ENFORCE(appPkg.exists());

        auto addImport = libPkg.addImport(gs, appPkg, false);
        string expected = makePackageRB("LibPackage", "false", "lib", {"AppPackage", "FalsePackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = addImport->applySingleEditForTesting(libPackage);
        CHECK_EQ(expected, replaced);
    }
}

} // namespace sorbet
