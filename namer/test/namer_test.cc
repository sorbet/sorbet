#include "doctest.h"
// has to go first as it violates our requirements

#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "local_vars/local_vars.h"
#include "namer/namer.h"
#include "rewriter/rewriter.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace spd = spdlog;

using namespace std;

namespace sorbet::namer::test {

namespace {
auto logger = spd::stderr_color_mt("namer_test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

static string_view testClass_str = "Test"sv;

ast::ParsedFile getTree(unique_ptr<core::GlobalState> &gs, string str) {
    sorbet::core::UnfreezeNameTable nameTableAccess(*gs); // enters original strings
    sorbet::core::UnfreezeFileTable ft(*gs);              // enters original strings
    string testPath = "<test>";
    core::FileRef file = gs->findFileByPath(testPath);
    if (file.exists()) {
        gs = core::GlobalState::replaceFile(
            move(gs), file, make_shared<core::File>(string(testPath), string(str), core::File::Type::Normal));
    } else {
        file = gs->enterFile(testPath, str);
    }
    auto tree = parser::Parser::run(*gs, file);
    file.data(*gs).strictLevel = core::StrictLevel::Strict;
    sorbet::core::MutableContext ctx(*gs, core::Symbols::root(), file);
    auto ast = ast::desugar::node2Tree(ctx, move(tree));
    ast = rewriter::Rewriter::run(ctx, move(ast));
    return ast::ParsedFile{move(ast), file};
}

ast::ParsedFile hello_world(unique_ptr<core::GlobalState> &gs) {
    return getTree(gs, "def hello_world; end");
}

vector<ast::ParsedFile> runNamer(core::GlobalState &gs, ast::ParsedFile tree) {
    vector<ast::ParsedFile> v;
    v.emplace_back(move(tree));
    auto workers = WorkerPool::create(0, *logger);
    return move(namer::Namer::run(gs, move(v), *workers).result());
}

pair<core::Loc, core::Loc> runNamerIncrementalAndGetLocs(unique_ptr<core::GlobalState> &gs, string before,
                                                         string after) {
    core::Loc locBefore, locAfter;
    {
        auto tree = getTree(gs, before);
        auto localTree = sorbet::local_vars::LocalVars::run(*gs, move(tree));
        sorbet::core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
        sorbet::core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
        runNamer(*gs, move(localTree));
        auto rootScope =
            core::Symbols::root().data(*gs)->findMember(*gs, gs->enterNameConstant(testClass_str)).data(*gs);

        REQUIRE_EQ(1, rootScope->locs().size());
        locBefore = rootScope->loc();
    }
    // Re-run namer on same file.
    {
        auto tree = getTree(gs, after);
        auto localTree = sorbet::local_vars::LocalVars::run(*gs, move(tree));
        sorbet::core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
        sorbet::core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
        runNamer(*gs, move(localTree));
        auto rootScope =
            core::Symbols::root().data(*gs)->findMember(*gs, gs->enterNameConstant(testClass_str)).data(*gs);
        REQUIRE_EQ(1, rootScope->locs().size());
        locAfter = rootScope->loc();
    }
    return make_pair(locBefore, locAfter);
}

} // namespace

TEST_CASE("namer tests") {
    auto gs = make_unique<core::GlobalState>(errorQueue);
    gs->initEmpty();

    SUBCASE("HelloWorld") {
        auto tree = hello_world(gs);
        {
            auto localTree = sorbet::local_vars::LocalVars::run(*gs, move(tree));
            sorbet::core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
            runNamer(*gs, move(localTree));
        }

        const auto &objectScope = core::Symbols::Object().data(*gs);
        REQUIRE_EQ(core::Symbols::root(), objectScope->owner);

        REQUIRE_EQ(4, objectScope->members().size());
        auto methodSym = objectScope->members().at(gs->enterNameUTF8("hello_world"));
        const auto &symbol = methodSym.data(*gs);
        REQUIRE_EQ(core::Symbols::Object(), symbol->owner);
        REQUIRE_EQ(1, symbol->arguments().size());
    }

    SUBCASE("Idempotent") { // NOLINT
        auto baseSymbols = gs->symbolsUsedTotal();
        auto baseMethods = gs->methodsUsed();
        auto baseNames = gs->namesUsedTotal();

        auto tree = hello_world(gs);
        ast::ParsedFile treeCopy{tree.tree.deepCopy(), tree.file};
        vector<ast::ParsedFile> trees;
        {
            auto localTree = sorbet::local_vars::LocalVars::run(*gs, move(tree));
            sorbet::core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
            trees = runNamer(*gs, move(localTree));
        }
        auto helloWorldMethod = 1;
        auto staticInit = 1;
        auto extraSymbols = helloWorldMethod + staticInit;

        REQUIRE_EQ(baseSymbols + extraSymbols, gs->symbolsUsedTotal());
        REQUIRE_EQ(baseMethods + extraSymbols, gs->methodsUsed());
        REQUIRE_EQ(baseNames + 2, gs->namesUsedTotal());

        // Run it again and get the same numbers
        auto localTree = sorbet::local_vars::LocalVars::run(*gs, move(treeCopy));
        runNamer(*gs, move(localTree));

        REQUIRE_EQ(baseSymbols + extraSymbols, gs->symbolsUsedTotal());
        REQUIRE_EQ(baseMethods + extraSymbols, gs->methodsUsed());
        REQUIRE_EQ(baseNames + 2, gs->namesUsedTotal());
    }

    SUBCASE("NameClass") { // NOLINT
        auto tree = getTree(gs, "class Test; class Foo; end; end");
        {
            auto localTree = sorbet::local_vars::LocalVars::run(*gs, move(tree));
            sorbet::core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
            runNamer(*gs, move(localTree));
        }
        const auto &rootScope =
            core::Symbols::root().data(*gs)->findMember(*gs, gs->enterNameConstant(testClass_str)).data(*gs);

        REQUIRE_EQ(3, rootScope->members().size());
        auto fooSym = rootScope->members().at(gs->enterNameConstant("Foo"));
        const auto &fooInfo = fooSym.data(*gs);
        REQUIRE_EQ(1, fooInfo->members().size());
    }

    SUBCASE("InsideClass") { // NOLINT
        auto tree = getTree(gs, "class Test; class Foo; def bar; end; end; end");
        {
            auto localTree = sorbet::local_vars::LocalVars::run(*gs, move(tree));
            sorbet::core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
            runNamer(*gs, move(localTree));
        }
        const auto &rootScope =
            core::Symbols::root().data(*gs)->findMember(*gs, gs->enterNameConstant(testClass_str)).data(*gs);

        REQUIRE_EQ(3, rootScope->members().size());
        auto fooSym = rootScope->members().at(gs->enterNameConstant("Foo"));
        const auto &fooInfo = fooSym.data(*gs);
        REQUIRE_EQ(2, fooInfo->members().size());

        auto barSym = fooInfo->members().at(gs->enterNameUTF8("bar"));
        REQUIRE_EQ(fooSym, barSym.data(*gs)->owner);
    }

    SUBCASE("ImplicitModuleDefinitionsIncrementalPathStableLocs") {
        // Ensures that the loc for the implicitly defined module does not change when re-running namer.
        core::Loc locBefore, locAfter;
        string fileContents = "class Test::Submodule; end\nclass Test::AnotherSubmodule; end";
        auto locs = runNamerIncrementalAndGetLocs(gs, fileContents, fileContents);
        REQUIRE_EQ(locs.first.beginPos(), locs.second.beginPos());
        REQUIRE_EQ(locs.first.endPos(), locs.second.endPos());
    }

    SUBCASE("ImplicitModuleDefinitionsIncrementalPathStableLocsChange") {
        // Ensures that the loc for the implicitly defined module changes when the definition site moves.
        core::Loc locBefore, locAfter;
        string fileContents = "class Test::Submodule; end\nclass Test::AnotherSubmodule; end";
        string fileContentsAfter = "\n\n\n" + fileContents;
        auto locs = runNamerIncrementalAndGetLocs(gs, fileContents, fileContentsAfter);
        // The locs should update by three characters.
        REQUIRE_EQ(locs.first.beginPos() + 3, locs.second.beginPos());
        REQUIRE_EQ(locs.first.endPos() + 3, locs.second.endPos());
        // The current behavior chooses the _last_ loc in the file if there are multiple implicit definitions.
        REQUIRE_EQ(locs.first.beginPos(), 33);
    }

    SUBCASE("ExplicitModuleDefinitionsTakePrecedenceOverImplicit") {
        // Ensures that "module Foo" is the definition site over "module Foo::Bar"
        core::Loc locBefore, locAfter;
        string fileContents = "class Test::Submodule; end\nclass Test; end\nclass Test::AnotherSubmodule; end";
        string fileContentsAfter = "\n\n\n" + fileContents;
        auto locs = runNamerIncrementalAndGetLocs(gs, fileContents, fileContentsAfter);
        // Covers "class Test"
        REQUIRE_EQ(locs.first.beginPos(), 27);
        REQUIRE_EQ(locs.first.endPos(), 37);
        // The locs should update by three characters.
        REQUIRE_EQ(locs.first.beginPos() + 3, locs.second.beginPos());
        REQUIRE_EQ(locs.first.endPos() + 3, locs.second.endPos());
    }
} // namespace sorbet::namer::test

} // namespace sorbet::namer::test
