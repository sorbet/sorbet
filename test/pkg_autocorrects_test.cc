#include "doctest/doctest.h"

#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "common/strings/formatting.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/NameSubstitution.h"
#include "core/Unfreeze.h"
#include "local_vars/local_vars.h"
#include "main/options/options.h"
#include "main/pipeline/pipeline.h"
#include "namer/namer.h"
#include "packager/packager.h"
#include "parser/parser.h"
#include "resolver/resolver.h"
#include "rewriter/rewriter.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "test/helpers/MockFileSystem.h"
#include "test/helpers/expectations.h"

using namespace std;

auto logger = spdlog::stderr_color_mt("pkg-autocorrects-test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

string examplePackage = "class ExamplePackage < PackageSpec\nend\n";
string examplePackagePath = "example/__package.rb";

namespace sorbet::test {
string applySuggestion(const core::GlobalState &gs, core::AutocorrectSuggestion suggestion) {
    test::MockFileSystem fs("");
    // Assumption: all the edits are in the same file
    auto fileRef = suggestion.edits[0].loc.file();
    fs.writeFile(string(fileRef.data(gs).path()), fileRef.data(gs).source());
    auto result = core::AutocorrectSuggestion::apply(gs, fs, {suggestion});
    return result[fileRef];
}

string makePackageRB(string name, string strictDeps, string layer, vector<string> imports = {},
                     vector<string> testImports = {}) {
    auto importList =
        fmt::map_join(imports, "", [&](const auto &import) { return fmt::format("  import {}\n", import); });

    auto testImportList = fmt::map_join(
        testImports, "", [&](const auto &testImport) { return fmt::format("  test_import {}\n", testImport); });

    return fmt::format("class {} < PackageSpec\n"
                       "  strict_dependencies '{}'\n"
                       "  layer '{}'\n"
                       "{}"
                       "{}"
                       "end",
                       name, strictDeps, layer, importList, testImportList);
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

const vector<string> NO_LAYERS;
const vector<string> LAYERS_LIB_APP = {"lib", "app"};
const vector<string> LAYERS_UTIL_LIB_APP = {"util", "lib", "app"};

void makeDefaultPackagerGlobalState(core::GlobalState &gs, const vector<string> &packagerLayers = NO_LAYERS) {
    gs.initEmpty();
    realmain::options::Options opts;
    opts.cacheSensitiveOptions.sorbetPackages = true;
    opts.packagerLayers = packagerLayers;
    realmain::pipeline::setGlobalStateOptions(gs, opts);
}

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
            auto nodes = parser::Parser::run(gs, file, settings).tree;

            core::MutableContext ctx(gs, core::Symbols::root(), file);
            auto parsedFile = ast::ParsedFile{ast::desugar::node2Tree(ctx, move(nodes)), file};
            parsedFile.tree = rewriter::Rewriter::run(ctx, move(parsedFile.tree));
            parsedFiles.emplace_back(local_vars::LocalVars::run(ctx, move(parsedFile)));
        }
    }

    auto workers = WorkerPool::create(0, gs.tracer());

    {
        core::UnfreezeNameTable nameTableAccess(gs);
        core::UnfreezeSymbolTable symbolTableAccess(gs);
        auto foundHashes = nullptr;
        auto canceled = namer::Namer::run(gs, absl::MakeSpan(parsedFiles), *workers, foundHashes);
        ENFORCE(!canceled);
    }

    packager::Packager::run(gs, *workers, absl::Span<ast::ParsedFile>(parsedFiles));

    {
        core::UnfreezeNameTable nameTableAccess(gs);
        core::UnfreezeSymbolTable symbolTableAccess(gs);
        auto maybeResult = resolver::Resolver::run(gs, move(parsedFiles), *workers);
        ENFORCE(maybeResult.hasResult());
        parsedFiles = move(maybeResult.result());
    }

    parsedFiles = packager::VisibilityChecker::run(gs, *workers, move(parsedFiles));

    return parsedFiles;
}

const core::packages::PackageInfo &packageInfoFor(const core::GlobalState &gs, core::FileRef file) {
    return gs.packageDB().getPackageInfo(gs.packageDB().getPackageNameForFile(file));
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
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import SomethingElse\n"
                      "  import ExamplePackage\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::Normal);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "msg");
}

TEST_CASE("Simple test helper import") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import SomethingElse\n"
                      "  test_import ExamplePackage\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestHelper);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Simple test unit import") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import SomethingElse\n"
                      "  test_import ExamplePackage, only: \"test_rb\"\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestUnit);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add import with only existing exports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  export SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import ExamplePackage\n"
                      "  export SomethingElse\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::Normal);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add import and test_import to package with imports and test imports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import A\n"
                        "  import B\n"
                        "  test_import C\n"
                        "  test_import D\n"
                        "end\n";

    auto parsedFiles = enterPackages(gs, {{examplePackagePath, examplePackage},
                                          {"my_package/__package.rb", pkg_source},
                                          {"a/__package.rb", "class A < PackageSpec\nend\n"},
                                          {"b/__package.rb", "class B < PackageSpec\nend\n"},
                                          {"c/__package.rb", "class C < PackageSpec\nend\n"},
                                          {"d/__package.rb", "class D < PackageSpec\nend\n"}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    {
        string expected = "class MyPackage < PackageSpec\n"
                          "  import A\n"
                          "  import B\n"
                          "  import ExamplePackage\n"
                          "  test_import C\n"
                          "  test_import D\n"
                          "end\n";
        auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::Normal);
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        string expected = "class MyPackage < PackageSpec\n"
                          "  import A\n"
                          "  import B\n"
                          "  test_import C\n"
                          "  test_import D\n"
                          "  test_import ExamplePackage\n"
                          "end\n";
        auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestHelper);
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Add test import with only existing exports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  export SomethingElse\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  test_import ExamplePackage\n"
                      "  export SomethingElse\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestHelper);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add import to package with neither imports nor exports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import ExamplePackage\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::Normal);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add test import to package with neither imports nor exports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  test_import ExamplePackage\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestHelper);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add test unit import to package with neither imports nor exports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  test_import ExamplePackage, only: \"test_rb\"\n"
                      "end\n";

    auto parsedFiles =
        enterPackages(gs, {{examplePackagePath, examplePackage}, {"my_package/__package.rb", pkg_source}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestUnit);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add export that goes before existing exports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  export MyPackage::This\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  export MyPackage::NewExport\n"
                      "  export MyPackage::This\n"
                      "end\n";

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source}});
    auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    auto addExport = myPkg.addExport(gs, getConstantRef(gs, {"MyPackage", "NewExport"}));
    ENFORCE(addExport, "Expected to get an autocorrect from `addExport`");
    auto replaced = applySuggestion(gs, *addExport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add export to package with no existing exports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  export MyPackage::NewExport\n"
                      "end\n";

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source}});
    auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    auto addExport = myPkg.addExport(gs, getConstantRef(gs, {"MyPackage", "NewExport"}));
    ENFORCE(addExport, "Expected to get an autocorrect from `addExport`");
    auto replaced = applySuggestion(gs, *addExport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add export that goes in the middle of existing exports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  export MyPackage::A\n"
                        "  export MyPackage::This\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  export MyPackage::A\n"
                      "  export MyPackage::NewExport\n"
                      "  export MyPackage::This\n"
                      "end\n";

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source}});
    auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    auto addExport = myPkg.addExport(gs, getConstantRef(gs, {"MyPackage", "NewExport"}));
    ENFORCE(addExport, "Expected to get an autocorrect from `addExport`");
    auto replaced = applySuggestion(gs, *addExport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Add export that goes at the end") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  export MyPackage::A\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  export MyPackage::A\n"
                      "  export MyPackage::NewExport\n"
                      "end\n";

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source}});
    auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    auto addExport = myPkg.addExport(gs, getConstantRef(gs, {"MyPackage", "NewExport"}));
    ENFORCE(addExport, "Expected to get an autocorrect from `addExport`");
    auto replaced = applySuggestion(gs, *addExport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

// Tests with layering and strict dependencies checks on
TEST_CASE("Add imports to strict_dependencies 'false' package") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_LIB_APP);

    string pkg_source = makePackageRB("MyPackage", "false", "app",
                                      {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {layeredDagPackageAPath, layeredDagPackageA},
                                          {dagPackageAPath, dagPackageA},
                                          {falsePackageBPath, falsePackageB},
                                          {layeredPackageBPath, layeredPackageB},
                                          {layeredDagPackageBPath, layeredDagPackageB},
                                          {dagPackageBPath, dagPackageB}});
    auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    {
        auto &falsePkgB = packageInfoFor(gs, parsedFiles[5].file);
        ENFORCE(falsePkgB.exists());
        auto addImport = myPkg.addImport(gs, falsePkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "false", "app",
                          {"FalsePackageA", "FalsePackageB", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &layeredPkgB = packageInfoFor(gs, parsedFiles[6].file);
        ENFORCE(layeredPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredPkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "false", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "LayeredPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &layeredDagPkgB = packageInfoFor(gs, parsedFiles[7].file);
        ENFORCE(layeredDagPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredDagPkgB, core::packages::ImportType::Normal);
        string expected = makePackageRB(
            "MyPackage", "false", "app",
            {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "LayeredDagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &dagPkgB = packageInfoFor(gs, parsedFiles[8].file);
        ENFORCE(dagPkgB.exists());
        auto addImport = myPkg.addImport(gs, dagPkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "false", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "DagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Add imports to strict_dependencies 'layered' package") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_LIB_APP);

    string pkg_source = makePackageRB("MyPackage", "layered", "app",
                                      {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {layeredDagPackageAPath, layeredDagPackageA},
                                          {dagPackageAPath, dagPackageA},
                                          {falsePackageBPath, falsePackageB},
                                          {layeredPackageBPath, layeredPackageB},
                                          {layeredDagPackageBPath, layeredDagPackageB},
                                          {dagPackageBPath, dagPackageB}});
    auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    {
        auto &falsePkgB = packageInfoFor(gs, parsedFiles[5].file);
        ENFORCE(falsePkgB.exists());
        auto addImport = myPkg.addImport(gs, falsePkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "layered", "app",
                          {"FalsePackageA", "FalsePackageB", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &layeredPkgB = packageInfoFor(gs, parsedFiles[6].file);
        ENFORCE(layeredPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredPkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "layered", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &layeredDagPkgB = packageInfoFor(gs, parsedFiles[7].file);
        ENFORCE(layeredDagPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredDagPkgB, core::packages::ImportType::Normal);
        string expected = makePackageRB(
            "MyPackage", "layered", "app",
            {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredDagPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &dagPkgB = packageInfoFor(gs, parsedFiles[8].file);
        ENFORCE(dagPkgB.exists());
        auto addImport = myPkg.addImport(gs, dagPkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "layered", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "DagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Add imports to strict_dependencies 'layered_dag' package") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_LIB_APP);

    string pkg_source = makePackageRB("MyPackage", "layered_dag", "app",
                                      {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {layeredDagPackageAPath, layeredDagPackageA},
                                          {dagPackageAPath, dagPackageA},
                                          {falsePackageBPath, falsePackageB},
                                          {layeredPackageBPath, layeredPackageB},
                                          {layeredDagPackageBPath, layeredDagPackageB},
                                          {dagPackageBPath, dagPackageB}});
    auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    {
        auto &falsePkgB = packageInfoFor(gs, parsedFiles[5].file);
        ENFORCE(falsePkgB.exists());
        auto addImport = myPkg.addImport(gs, falsePkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "layered_dag", "app",
                          {"FalsePackageA", "FalsePackageB", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &layeredPkgB = packageInfoFor(gs, parsedFiles[6].file);
        ENFORCE(layeredPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredPkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "layered_dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &layeredDagPkgB = packageInfoFor(gs, parsedFiles[7].file);
        ENFORCE(layeredDagPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredDagPkgB, core::packages::ImportType::Normal);
        string expected = makePackageRB(
            "MyPackage", "layered_dag", "app",
            {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredDagPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &dagPkgB = packageInfoFor(gs, parsedFiles[8].file);
        ENFORCE(dagPkgB.exists());
        auto addImport = myPkg.addImport(gs, dagPkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "layered_dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "DagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Add imports to strict_dependencies 'dag' package") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_LIB_APP);

    string pkg_source = makePackageRB("MyPackage", "dag", "app",
                                      {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});

    auto parsedFiles = enterPackages(gs, {{"my_package/__package.rb", pkg_source},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {layeredDagPackageAPath, layeredDagPackageA},
                                          {dagPackageAPath, dagPackageA},
                                          {falsePackageBPath, falsePackageB},
                                          {layeredPackageBPath, layeredPackageB},
                                          {layeredDagPackageBPath, layeredDagPackageB},
                                          {dagPackageBPath, dagPackageB}});
    auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
    ENFORCE(myPkg.exists());

    {
        auto &falsePkgB = packageInfoFor(gs, parsedFiles[5].file);
        ENFORCE(falsePkgB.exists());
        auto addImport = myPkg.addImport(gs, falsePkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "dag", "app",
                          {"FalsePackageA", "FalsePackageB", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &layeredPkgB = packageInfoFor(gs, parsedFiles[6].file);
        ENFORCE(layeredPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredPkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &layeredDagPkgB = packageInfoFor(gs, parsedFiles[7].file);
        ENFORCE(layeredDagPkgB.exists());
        auto addImport = myPkg.addImport(gs, layeredDagPkgB, core::packages::ImportType::Normal);
        string expected = makePackageRB(
            "MyPackage", "dag", "app",
            {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "LayeredDagPackageB", "DagPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &dagPkgB = packageInfoFor(gs, parsedFiles[8].file);
        ENFORCE(dagPkgB.exists());
        auto addImport = myPkg.addImport(gs, dagPkgB, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("MyPackage", "dag", "app",
                          {"FalsePackageA", "LayeredPackageA", "LayeredDagPackageA", "DagPackageA", "DagPackageB"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Edge cases") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_LIB_APP);

    string packageWithFakeImport = makePackageRB("HasFakeImport", "false", "app", {"FakeImport"});
    string packageWithFakeImportPath = "has_fake_import/__package.rb";

    string libPackage = makePackageRB("LibPackage", "false", "lib", {"FalsePackageA"});
    string libPackagePath = "lib_pkg/__package.rb";

    string appPackage = makePackageRB("AppPackage", "layered", "app", {});
    string appPackagePath = "app_pkg/__package.rb";

    string packageWithComments = "class HasComments < PackageSpec\n"
                                 "  layer 'app'\n"
                                 "  strict_dependencies 'false'\n"
                                 "  import FalsePackageA # a comment\n"
                                 "end\n";
    string packageWithCommentsPath = "has_comments/__package.rb";

    string packageWithTestImports =
        makePackageRB("HasTestImports", "dag", "app", {}, {"FalsePackageA", "LayeredPackageA"});
    string packageWithTestImportsPath = "has_test_imports/__package.rb";

    string packageWithLayeringViolations = makePackageRB("HasLayeringViolations", "false", "lib", {"AppPackage"});
    string packageWithLayeringViolationsPath = "has_layering_violations/__package.rb";

    auto parsedFiles = enterPackages(gs, {{packageWithFakeImportPath, packageWithFakeImport},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {libPackagePath, libPackage},
                                          {appPackagePath, appPackage},
                                          {packageWithCommentsPath, packageWithComments},
                                          {packageWithTestImportsPath, packageWithTestImports},
                                          {dagPackageAPath, dagPackageA},
                                          {packageWithLayeringViolationsPath, packageWithLayeringViolations}});

    {
        // Import list contains non-existent package
        auto &fakeImportPkg = packageInfoFor(gs, parsedFiles[0].file);
        ENFORCE(fakeImportPkg.exists());
        auto &layeredPkgA = packageInfoFor(gs, parsedFiles[2].file);
        ENFORCE(layeredPkgA.exists());

        auto addImport = fakeImportPkg.addImport(gs, layeredPkgA, core::packages::ImportType::Normal);
        string expected = makePackageRB("HasFakeImport", "false", "app", {"FakeImport", "LayeredPackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        // Import added to start of import list
        auto &libPkg = packageInfoFor(gs, parsedFiles[3].file);
        ENFORCE(libPkg.exists());
        auto &appPkg = packageInfoFor(gs, parsedFiles[4].file);
        ENFORCE(appPkg.exists());

        auto addImport = libPkg.addImport(gs, appPkg, core::packages::ImportType::Normal);
        string expected = makePackageRB("LibPackage", "false", "lib", {"AppPackage", "FalsePackageA"});
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        // Import list with comments in it
        auto &hasCommentsPkg = packageInfoFor(gs, parsedFiles[5].file);
        ENFORCE(hasCommentsPkg.exists());
        auto &layeredPkgA = packageInfoFor(gs, parsedFiles[2].file);
        ENFORCE(layeredPkgA.exists());

        auto addImport = hasCommentsPkg.addImport(gs, layeredPkgA, core::packages::ImportType::Normal);
        string expected = "class HasComments < PackageSpec\n"
                          "  layer 'app'\n"
                          "  strict_dependencies 'false'\n"
                          "  import FalsePackageA\n"
                          "  import LayeredPackageA # a comment\n"
                          "end\n";
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        // Add import to a package with a bunch of test imports
        auto &hasTestImportsPkg = packageInfoFor(gs, parsedFiles[6].file);
        ENFORCE(hasTestImportsPkg.exists());
        auto &dagPkgA = packageInfoFor(gs, parsedFiles[7].file);
        ENFORCE(dagPkgA.exists());

        auto addImport = hasTestImportsPkg.addImport(gs, dagPkgA, core::packages::ImportType::Normal);
        string expected =
            makePackageRB("HasTestImports", "dag", "app", {"DagPackageA"}, {"FalsePackageA", "LayeredPackageA"});
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        // Add import to a package with layering violations
        auto &hasLayeringViolationsPkg = packageInfoFor(gs, parsedFiles[8].file);
        ENFORCE(hasLayeringViolationsPkg.exists());
        auto &falsePkgA = packageInfoFor(gs, parsedFiles[1].file);
        ENFORCE(falsePkgA.exists());

        auto addImport = hasLayeringViolationsPkg.addImport(gs, falsePkgA, core::packages::ImportType::Normal);
        string expected = makePackageRB("HasLayeringViolations", "false", "lib", {"AppPackage", "FalsePackageA"});
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Convert test_import to import") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_LIB_APP);

    string myPackage =
        makePackageRB("MyPackage", "layered", "app", {"FalsePackageA", "DagPackageA"}, {"LayeredPackageA"});
    string myPackagePath = "my_package/__package.rb";

    auto parsedFiles = enterPackages(gs, {{myPackagePath, myPackage},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {dagPackageAPath, dagPackageA}});

    {
        auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
        ENFORCE(myPkg.exists());
        auto &layeredPkgA = packageInfoFor(gs, parsedFiles[2].file);
        ENFORCE(layeredPkgA.exists());

        auto addImport = myPkg.addImport(gs, layeredPkgA, core::packages::ImportType::Normal);
        string expected = "class MyPackage < PackageSpec\n"
                          "  strict_dependencies 'layered'\n"
                          "  layer 'app'\n"
                          "  import FalsePackageA\n"
                          "  import LayeredPackageA\n"
                          "  import DagPackageA\n"
                          // This extra line is not great, but if we change the autocorrect to delete the '\n'
                          // after the test_import, the autocorrect show the next line in the preview, which would
                          // make the user think that entire next line will be deleted, which is incorrect.
                          // TODO(neil): look into ways to modify the preview so we don't have this problem and we can
                          // delete the '\n' too
                          "\n"
                          "end";
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Convert test unit import to test helper import") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_LIB_APP);

    string myPackage = "class MyPackage < PackageSpec\n"
                       "  strict_dependencies 'layered'\n"
                       "  layer 'app'\n"
                       "  import FalsePackageA\n"
                       "  import DagPackageA\n"
                       "\n"
                       "  test_import LayeredPackageA, only: \"test_rb\"\n"
                       "end";
    string myPackagePath = "my_package/__package.rb";

    auto parsedFiles = enterPackages(gs, {{myPackagePath, myPackage},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {dagPackageAPath, dagPackageA}});

    {
        auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
        ENFORCE(myPkg.exists());
        auto &layeredPkgA = packageInfoFor(gs, parsedFiles[2].file);
        ENFORCE(layeredPkgA.exists());

        gs.tracer().error("before");
        auto addImport = myPkg.addImport(gs, layeredPkgA, core::packages::ImportType::TestHelper);
        gs.tracer().error("after");
        string expected = "class MyPackage < PackageSpec\n"
                          "  strict_dependencies 'layered'\n"
                          "  layer 'app'\n"
                          "  import FalsePackageA\n"
                          "  import DagPackageA\n"
                          "  test_import LayeredPackageA\n"
                          // This extra line is not great, but if we change the autocorrect to delete the '\n'
                          // after the test_import, the autocorrect show the next line in the preview, which would
                          // make the user think that entire next line will be deleted, which is incorrect.
                          // TODO(neil): look into ways to modify the preview so we don't have this problem and we can
                          // delete the '\n' too
                          "\n\n"
                          "end";
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Convert test unit import to normal import") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_LIB_APP);

    string myPackage = "class MyPackage < PackageSpec\n"
                       "  strict_dependencies 'layered'\n"
                       "  layer 'app'\n"
                       "  import FalsePackageA\n"
                       "  import DagPackageA\n"
                       "\n"
                       "  test_import LayeredPackageA, only: \"test_rb\"\n"
                       "end";
    string myPackagePath = "my_package/__package.rb";

    auto parsedFiles = enterPackages(gs, {{myPackagePath, myPackage},
                                          {falsePackageAPath, falsePackageA},
                                          {layeredPackageAPath, layeredPackageA},
                                          {dagPackageAPath, dagPackageA}});

    {
        auto &myPkg = packageInfoFor(gs, parsedFiles[0].file);
        ENFORCE(myPkg.exists());
        auto &layeredPkgA = packageInfoFor(gs, parsedFiles[2].file);
        ENFORCE(layeredPkgA.exists());

        gs.tracer().error("before");
        auto addImport = myPkg.addImport(gs, layeredPkgA, core::packages::ImportType::Normal);
        gs.tracer().error("after");
        string expected = "class MyPackage < PackageSpec\n"
                          "  strict_dependencies 'layered'\n"
                          "  layer 'app'\n"
                          "  import FalsePackageA\n"
                          "  import LayeredPackageA\n"
                          "  import DagPackageA\n"
                          // This extra line is not great, but if we change the autocorrect to delete the '\n'
                          // after the test_import, the autocorrect show the next line in the preview, which would
                          // make the user think that entire next line will be deleted, which is incorrect.
                          // TODO(neil): look into ways to modify the preview so we don't have this problem and we can
                          // delete the '\n' too
                          "\n\n"
                          "end";
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Ordering by alphabetical") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs, LAYERS_UTIL_LIB_APP);

    string myPackage = makePackageRB("MyPackage", "layered", "lib", {"Lib::Foo::B::A"});

    auto parsedFiles =
        enterPackages(gs, {{"lib/foo/a/__package.rb", makePackageRB("Lib::Foo::A", "layered", "lib")},
                           {"lib/foo/b/__package.rb", makePackageRB("Lib::Foo::B", "layered", "lib")},
                           {"lib/foo/b/a/__package.rb", makePackageRB("Lib::Foo::B::A", "layered", "lib")},
                           {"lib/foo/c/__package.rb", makePackageRB("Lib::Foo::C", "layered", "lib")},
                           {"lib/foo/d/__package.rb", makePackageRB("Lib::Foo::D", "layered", "app")},
                           {"my_package/__package.rb", myPackage}});

    {
        auto &myPkg = packageInfoFor(gs, parsedFiles[5].file);
        ENFORCE(myPkg.exists());
        auto &libFooB = packageInfoFor(gs, parsedFiles[1].file);
        ENFORCE(libFooB.exists());

        string expected = makePackageRB("MyPackage", "layered", "lib", {"Lib::Foo::B", "Lib::Foo::B::A"});
        auto addImport = myPkg.addImport(gs, libFooB, core::packages::ImportType::Normal);
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &myPkg = packageInfoFor(gs, parsedFiles[5].file);
        ENFORCE(myPkg.exists());
        auto &libFooC = packageInfoFor(gs, parsedFiles[3].file);
        ENFORCE(libFooC.exists());

        string expected = makePackageRB("MyPackage", "layered", "lib", {"Lib::Foo::B::A", "Lib::Foo::C"});
        auto addImport = myPkg.addImport(gs, libFooC, core::packages::ImportType::Normal);
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }

    {
        auto &myPkg = packageInfoFor(gs, parsedFiles[5].file);
        ENFORCE(myPkg.exists());
        auto &libFooD = packageInfoFor(gs, parsedFiles[4].file);
        ENFORCE(libFooD.exists());

        string expected = makePackageRB("MyPackage", "layered", "lib", {"Lib::Foo::D", "Lib::Foo::B::A"});
        auto addImport = myPkg.addImport(gs, libFooD, core::packages::ImportType::Normal);
        ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
        auto replaced = applySuggestion(gs, *addImport);
        CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
    }
}

TEST_CASE("Adding a test unit import with existing imports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import A\n"
                        "  import B\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import A\n"
                      "  import B\n"
                      "  test_import ExamplePackage, only: \"test_rb\"\n"
                      "end\n";

    auto parsedFiles = enterPackages(gs, {{examplePackagePath, examplePackage},
                                          {"my_package/__package.rb", pkg_source},
                                          {"a/__package.rb", "class A < PackageSpec\nend\n"},
                                          {"b/__package.rb", "class B < PackageSpec\nend\n"}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestUnit);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Adding a test unit import with existing imports and test imports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import A\n"
                        "  test_import B\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import A\n"
                      "  test_import B\n"
                      "  test_import ExamplePackage, only: \"test_rb\"\n"
                      "end\n";

    auto parsedFiles = enterPackages(gs, {{examplePackagePath, examplePackage},
                                          {"my_package/__package.rb", pkg_source},
                                          {"a/__package.rb", "class A < PackageSpec\nend\n"},
                                          {"b/__package.rb", "class B < PackageSpec\nend\n"}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestUnit);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

TEST_CASE("Adding a test unit import with existing imports, test imports, and test unit imports") {
    core::GlobalState gs(errorQueue);
    makeDefaultPackagerGlobalState(gs);

    string pkg_source = "class MyPackage < PackageSpec\n"
                        "  import A\n"
                        "  test_import B\n"
                        "  test_import C, only: \"test_rb\"\n"
                        "end\n";

    string expected = "class MyPackage < PackageSpec\n"
                      "  import A\n"
                      "  test_import B\n"
                      "  test_import C, only: \"test_rb\"\n"
                      "  test_import ExamplePackage, only: \"test_rb\"\n"
                      "end\n";

    auto parsedFiles = enterPackages(gs, {{examplePackagePath, examplePackage},
                                          {"my_package/__package.rb", pkg_source},
                                          {"a/__package.rb", "class A < PackageSpec\nend\n"},
                                          {"b/__package.rb", "class B < PackageSpec\nend\n"},
                                          {"c/__package.rb", "class C < PackageSpec\nend\n"}});
    auto &examplePkg = packageInfoFor(gs, parsedFiles[0].file);
    auto &myPkg = packageInfoFor(gs, parsedFiles[1].file);
    ENFORCE(examplePkg.exists());
    ENFORCE(myPkg.exists());

    auto addImport = myPkg.addImport(gs, examplePkg, core::packages::ImportType::TestUnit);
    ENFORCE(addImport, "Expected to get an autocorrect from `addImport`");
    auto replaced = applySuggestion(gs, *addImport);
    CHECK_EQ_DIFF(expected, replaced, "-expected,+replaced");
}

} // namespace sorbet::test
