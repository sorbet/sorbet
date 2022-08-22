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

ast::ParsedFile getTree(core::GlobalState &gs, string str) {
    sorbet::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings
    sorbet::core::UnfreezeFileTable ft(gs);              // enters original strings
    auto file = gs.enterFile("<test>", str);
    auto settings = parser::Parser::Settings{};
    auto tree = parser::Parser::run(gs, file, settings);
    file.data(gs).strictLevel = core::StrictLevel::Strict;
    sorbet::core::MutableContext ctx(gs, core::Symbols::root(), file);
    auto ast = ast::desugar::node2Tree(ctx, move(tree));
    ast = rewriter::Rewriter::run(ctx, move(ast));
    return ast::ParsedFile{move(ast), file};
}

ast::ParsedFile hello_world(core::GlobalState &gs) {
    return getTree(gs, "def hello_world; end");
}

vector<ast::ParsedFile> runNamer(core::GlobalState &gs, ast::ParsedFile tree) {
    vector<ast::ParsedFile> v;
    v.emplace_back(move(tree));
    auto workers = WorkerPool::create(0, *logger);
    core::FoundDefHashes foundHashes; // compute this just for test coverage
    return move(namer::Namer::run(gs, move(v), *workers, &foundHashes).result());
}

} // namespace

TEST_CASE("namer tests") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    SUBCASE("HelloWorld") {
        auto tree = hello_world(gs);
        {
            auto localTree = sorbet::local_vars::LocalVars::run(gs, move(tree));
            sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
            runNamer(gs, move(localTree));
        }

        const auto &objectScope = core::Symbols::Object().data(gs);
        REQUIRE_EQ(core::Symbols::root(), objectScope->owner);

        REQUIRE_EQ(4, objectScope->members().size());
        auto methodSym = objectScope->members().at(gs.enterNameUTF8("hello_world")).asMethodRef();
        const auto &symbol = methodSym.data(gs);
        REQUIRE_EQ(core::Symbols::Object(), symbol->owner);
        REQUIRE_EQ(1, symbol->arguments.size());
    }

    SUBCASE("Idempotent") { // NOLINT
        auto baseSymbols = gs.symbolsUsedTotal();
        auto baseMethods = gs.methodsUsed();
        auto baseNames = gs.namesUsedTotal();

        auto tree = hello_world(gs);
        ast::ParsedFile treeCopy{tree.tree.deepCopy(), tree.file};
        vector<ast::ParsedFile> trees;
        {
            auto localTree = sorbet::local_vars::LocalVars::run(gs, move(tree));
            sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
            trees = runNamer(gs, move(localTree));
        }
        auto helloWorldMethod = 1;
        auto staticInit = 1;
        auto extraSymbols = helloWorldMethod + staticInit;

        REQUIRE_EQ(baseSymbols + extraSymbols, gs.symbolsUsedTotal());
        REQUIRE_EQ(baseMethods + extraSymbols, gs.methodsUsed());
        REQUIRE_EQ(baseNames + 2, gs.namesUsedTotal());

        // Run it again and get the same numbers
        auto localTree = sorbet::local_vars::LocalVars::run(gs, move(treeCopy));
        runNamer(gs, move(localTree));

        REQUIRE_EQ(baseSymbols + extraSymbols, gs.symbolsUsedTotal());
        REQUIRE_EQ(baseMethods + extraSymbols, gs.methodsUsed());
        REQUIRE_EQ(baseNames + 2, gs.namesUsedTotal());
    }

    SUBCASE("NameClass") { // NOLINT
        auto tree = getTree(gs, "class Test; class Foo; end; end");
        {
            auto localTree = sorbet::local_vars::LocalVars::run(gs, move(tree));
            sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
            runNamer(gs, move(localTree));
        }
        const auto &rootScope = core::Symbols::root()
                                    .data(gs)
                                    ->findMember(gs, gs.enterNameConstant(testClass_str))
                                    .asClassOrModuleRef()
                                    .data(gs);

        REQUIRE_EQ(3, rootScope->members().size());
        auto fooSym = rootScope->members().at(gs.enterNameConstant("Foo")).asClassOrModuleRef();
        const auto &fooInfo = fooSym.data(gs);
        REQUIRE_EQ(1, fooInfo->members().size());
    }

    SUBCASE("InsideClass") { // NOLINT
        auto tree = getTree(gs, "class Test; class Foo; def bar; end; end; end");
        {
            auto localTree = sorbet::local_vars::LocalVars::run(gs, move(tree));
            sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
            runNamer(gs, move(localTree));
        }
        const auto &rootScope = core::Symbols::root()
                                    .data(gs)
                                    ->findMember(gs, gs.enterNameConstant(testClass_str))
                                    .asClassOrModuleRef()
                                    .data(gs);

        REQUIRE_EQ(3, rootScope->members().size());
        auto fooSym = rootScope->members().at(gs.enterNameConstant("Foo")).asClassOrModuleRef();
        const auto &fooInfo = fooSym.data(gs);
        REQUIRE_EQ(2, fooInfo->members().size());

        auto barSym = fooInfo->members().at(gs.enterNameUTF8("bar"));
        REQUIRE_EQ(core::SymbolRef(fooSym), barSym.owner(gs));
    }
}

} // namespace sorbet::namer::test
