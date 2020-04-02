#include "gtest/gtest.h"
// has to go first as it violates our requirements

#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
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

auto logger = spd::stderr_color_mt("namer_test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

class NamerFixture : public ::testing::Test {
public:
    void SetUp() override {
        gsPtr = make_unique<core::GlobalState>(errorQueue);
        gsPtr->initEmpty();
    }
    core::GlobalState &getGs() {
        return *gsPtr;
    }

private:
    unique_ptr<core::GlobalState> gsPtr;
};

static string_view testClass_str = "Test"sv;

ast::ParsedFile getTree(core::GlobalState &gs, string str) {
    sorbet::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings
    sorbet::core::UnfreezeFileTable ft(gs);              // enters original strings
    auto file = gs.enterFile("<test>", str);
    auto tree = parser::Parser::run(gs, file);
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
    return namer::Namer::run(gs, move(v));
}

TEST_F(NamerFixture, HelloWorld) { // NOLINT
    auto &gs = getGs();
    auto tree = hello_world(gs);
    {
        auto localTree = sorbet::local_vars::LocalVars::run(gs, move(tree));
        sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
        sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
        runNamer(gs, move(localTree));
    }

    const auto &objectScope = core::Symbols::Object().data(gs);
    ASSERT_EQ(core::Symbols::root(), objectScope->owner);

    ASSERT_EQ(4, objectScope->members().size());
    auto methodSym = objectScope->members().at(gs.enterNameUTF8("hello_world"));
    const auto &symbol = methodSym.data(gs);
    ASSERT_EQ(core::Symbols::Object(), symbol->owner);
    ASSERT_EQ(1, symbol->arguments().size());
}

TEST_F(NamerFixture, Idempotent) { // NOLINT
    auto &gs = getGs();
    auto baseSymbols = gs.symbolsUsed();
    auto baseNames = gs.namesUsed();

    auto tree = hello_world(gs);
    ast::ParsedFile treeCopy{tree.tree->deepCopy(), tree.file};
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

    ASSERT_EQ(baseSymbols + extraSymbols, gs.symbolsUsed());
    ASSERT_EQ(baseNames + 2, gs.namesUsed());

    // Run it again and get the same numbers
    auto localTree = sorbet::local_vars::LocalVars::run(gs, move(treeCopy));
    runNamer(gs, move(localTree));

    ASSERT_EQ(baseSymbols + extraSymbols, gs.symbolsUsed());
    ASSERT_EQ(baseNames + 2, gs.namesUsed());
}

TEST_F(NamerFixture, NameClass) { // NOLINT
    auto &gs = getGs();
    auto tree = getTree(gs, "class Test; class Foo; end; end");
    {
        auto localTree = sorbet::local_vars::LocalVars::run(gs, move(tree));
        sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
        sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
        runNamer(gs, move(localTree));
    }
    const auto &rootScope =
        core::Symbols::root().data(gs)->findMember(gs, gs.enterNameConstant(testClass_str)).data(gs);

    ASSERT_EQ(3, rootScope->members().size());
    auto fooSym = rootScope->members().at(gs.enterNameConstant("Foo"));
    const auto &fooInfo = fooSym.data(gs);
    ASSERT_EQ(1, fooInfo->members().size());
}

TEST_F(NamerFixture, InsideClass) { // NOLINT
    auto &gs = getGs();
    auto tree = getTree(gs, "class Test; class Foo; def bar; end; end; end");
    {
        auto localTree = sorbet::local_vars::LocalVars::run(gs, move(tree));
        sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
        sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
        runNamer(gs, move(localTree));
    }
    const auto &rootScope =
        core::Symbols::root().data(gs)->findMember(gs, gs.enterNameConstant(testClass_str)).data(gs);

    ASSERT_EQ(3, rootScope->members().size());
    auto fooSym = rootScope->members().at(gs.enterNameConstant("Foo"));
    const auto &fooInfo = fooSym.data(gs);
    ASSERT_EQ(2, fooInfo->members().size());

    auto barSym = fooInfo->members().at(gs.enterNameUTF8("bar"));
    ASSERT_EQ(fooSym, barSym.data(gs)->owner);
}

} // namespace sorbet::namer::test
