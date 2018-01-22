#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "core/Unfreeze.h"
#include "namer/namer.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>

namespace spd = spdlog;

using namespace std;

namespace ruby_typer {
namespace namer {
namespace test {

auto console = spd::stderr_color_mt("namer");

class NamerFixture : public ::testing::Test {
public:
    void SetUp() override {
        ctxPtr = make_unique<core::GlobalState>(*console);
        ctxPtr->initEmpty();
    }
    core::Context getCtx() {
        return core::Context(*ctxPtr, ctxPtr->defn_root());
    }

private:
    unique_ptr<core::GlobalState> ctxPtr;
};

static const char *testClass_str = "Test";

unique_ptr<ast::Expression> getTree(core::GlobalState &gs, string str) {
    ruby_typer::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings
    ruby_typer::core::UnfreezeFileTable ft(gs);              // enters original strings
    auto ast = parser::Parser::run(gs, "<test>", str);
    ruby_typer::core::Context ctx(gs, gs.defn_root());
    return ast::desugar::node2Tree(ctx, ast);
}

unique_ptr<ast::Expression> hello_world(core::GlobalState &gs) {
    return getTree(gs, "def hello_world; end");
}

TEST_F(NamerFixture, HelloWorld) { // NOLINT
    auto ctx = getCtx();
    auto tree = hello_world(ctx);
    {
        ruby_typer::core::UnfreezeNameTable nameTableAccess(ctx);     // creates singletons and class names
        ruby_typer::core::UnfreezeSymbolTable symbolTableAccess(ctx); // enters symbols
        namer::Namer::run(ctx, move(tree));
    }

    auto &objectScope = core::GlobalState::defn_Object().info(ctx);
    ASSERT_EQ(core::GlobalState::defn_root(), objectScope.owner);

    ASSERT_EQ(2, objectScope.members.size());
    auto methodPair = objectScope.members[1];
    ASSERT_EQ("hello_world", methodPair.first.name(ctx).toString(ctx));
    auto &symbol = methodPair.second.info(ctx);
    ASSERT_EQ(core::GlobalState::defn_Object(), symbol.owner);
    ASSERT_EQ(0, symbol.arguments().size());
}

TEST_F(NamerFixture, Idempotent) { // NOLINT
    auto ctx = getCtx();
    auto baseSymbols = ctx.state.symbolsUsed();
    auto baseNames = ctx.state.namesUsed();

    auto tree = hello_world(ctx);
    std::unique_ptr<ruby_typer::ast::Expression> newtree;
    {
        ruby_typer::core::UnfreezeNameTable nameTableAccess(ctx);     // creates singletons and class names
        ruby_typer::core::UnfreezeSymbolTable symbolTableAccess(ctx); // enters symbols
        newtree = namer::Namer::run(ctx, move(tree));
    }
    ASSERT_EQ(baseSymbols + 1, ctx.state.symbolsUsed());
    ASSERT_EQ(baseNames + 1, ctx.state.namesUsed());

    // Run it again and get the same numbers
    namer::Namer::run(ctx, move(newtree));
    ASSERT_EQ(baseSymbols + 1, ctx.state.symbolsUsed());
    ASSERT_EQ(baseNames + 1, ctx.state.namesUsed());

    auto errors = ctx.state.drainErrors();
    ASSERT_EQ(1, errors.size()); // Method redeclaration
}

TEST_F(NamerFixture, NameClass) { // NOLINT
    auto ctx = getCtx();
    auto tree = getTree(ctx, "class Test; class Foo; end; end");
    {
        ruby_typer::core::UnfreezeNameTable nameTableAccess(ctx);     // creates singletons and class names
        ruby_typer::core::UnfreezeSymbolTable symbolTableAccess(ctx); // enters symbols
        namer::Namer::run(ctx, move(tree));
    }
    auto &rootScope =
        core::GlobalState::defn_root().info(ctx).findMember(ctx, ctx.state.enterNameConstant(testClass_str)).info(ctx);

    ASSERT_EQ(3, rootScope.members.size());
    auto fooPair = rootScope.members[1];
    ASSERT_EQ("<constant:Foo>", fooPair.first.name(ctx).toString(ctx));
    auto &fooInfo = fooPair.second.info(ctx);
    ASSERT_EQ(1, fooInfo.members.size());
}

TEST_F(NamerFixture, InsideClass) { // NOLINT
    auto ctx = getCtx();
    auto tree = getTree(ctx, "class Test; class Foo; def bar; end; end; end");
    {
        ruby_typer::core::UnfreezeNameTable nameTableAccess(ctx);     // creates singletons and class names
        ruby_typer::core::UnfreezeSymbolTable symbolTableAccess(ctx); // enters symbols
        namer::Namer::run(ctx, move(tree));
    }
    auto &rootScope =
        core::GlobalState::defn_root().info(ctx).findMember(ctx, ctx.state.enterNameConstant(testClass_str)).info(ctx);

    ASSERT_EQ(3, rootScope.members.size());
    auto fooSym = rootScope.members[1].second;
    auto &fooInfo = fooSym.info(ctx);
    ASSERT_EQ(2, fooInfo.members.size());

    auto barPair = fooInfo.members[1];
    ASSERT_EQ("bar", barPair.first.name(ctx).toString(ctx));
    ASSERT_EQ(fooSym, barPair.second.info(ctx).owner);
}

} // namespace test
} // namespace namer
} // namespace ruby_typer
