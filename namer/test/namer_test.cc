#include "../namer.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
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
    void SetUp() {
        ctxPtr = make_unique<ast::GlobalState>(*console);
    }
    ast::Context getCtx() {
        return ast::Context(*ctxPtr, ctxPtr->defn_root());
    }

private:
    unique_ptr<ast::GlobalState> ctxPtr;
};

static const char *testClass = "Test";
static ast::UTF8Desc testClass_DESC{(char *)testClass, (int)strlen(testClass)};

unique_ptr<ast::Statement> getTree(ast::GlobalState &gs, string str) {
    auto result = parser::parse_ruby(gs, "<test>", str);
    ruby_typer::ast::Context ctx(gs, gs.defn_root());
    return ast::desugar::node2Tree(ctx, result.ast());
}

unique_ptr<ast::Statement> hello_world(ast::GlobalState &gs) {
    return getTree(gs, "def hello_world; end");
}

TEST_F(NamerFixture, HelloWorld) {
    auto ctx = getCtx();
    auto tree = hello_world(ctx);
    namer::Namer::run(ctx, move(tree));
    auto &objectScope = ast::GlobalState::defn_object().info(ctx);
    ASSERT_EQ(ast::GlobalState::defn_root(), objectScope.owner);

    ASSERT_EQ(1, objectScope.members.size());
    auto methodPair = objectScope.members[0];
    ASSERT_EQ("hello_world", methodPair.first.name(ctx).toString(ctx));
    auto &symbol = methodPair.second.info(ctx);
    ASSERT_EQ(ast::GlobalState::defn_object(), symbol.owner);
    ASSERT_EQ(0, symbol.arguments().size());
}

TEST_F(NamerFixture, Idempotent) {
    auto ctx = getCtx();
    auto baseSymbols = ctx.state.symbolsUsed();
    auto baseNames = ctx.state.namesUsed();

    auto tree = hello_world(ctx);
    auto newtree = namer::Namer::run(ctx, move(tree));
    ASSERT_EQ(baseSymbols + 1, ctx.state.symbolsUsed());
    ASSERT_EQ(baseNames + 1, ctx.state.namesUsed());

    // Run it again and get the same numbers
    namer::Namer::run(ctx, move(newtree));
    ASSERT_EQ(baseSymbols + 1, ctx.state.symbolsUsed());
    ASSERT_EQ(baseNames + 1, ctx.state.namesUsed());
}

TEST_F(NamerFixture, NameClass) {
    auto ctx = getCtx();
    auto tree = getTree(ctx, "class Test; class Foo; end; end");
    namer::Namer::run(ctx, move(tree));
    auto &rootScope =
        ast::GlobalState::defn_root().info(ctx).findMember(ctx.state.enterNameUTF8(testClass_DESC)).info(ctx);

    ASSERT_EQ(1, rootScope.members.size());
    auto fooPair = rootScope.members[0];
    ASSERT_EQ("Foo", fooPair.first.name(ctx).toString(ctx));
    auto &fooInfo = fooPair.second.info(ctx);
    ASSERT_EQ(0, fooInfo.members.size());
}

TEST_F(NamerFixture, InsideClass) {
    auto ctx = getCtx();
    auto tree = getTree(ctx, "class Test; class Foo; def bar; end; end; end");
    namer::Namer::run(ctx, move(tree));
    auto &rootScope =
        ast::GlobalState::defn_root().info(ctx).findMember(ctx.state.enterNameUTF8(testClass_DESC)).info(ctx);

    ASSERT_EQ(1, rootScope.members.size());
    auto fooSym = rootScope.members[0].second;
    auto &fooInfo = fooSym.info(ctx);
    ASSERT_EQ(1, fooInfo.members.size());

    auto barPair = fooInfo.members[0];
    ASSERT_EQ("bar", barPair.first.name(ctx).toString(ctx));
    ASSERT_EQ(fooSym, barPair.second.info(ctx).owner);
}

} // namespace test
} // namespace namer
} // namespace ruby_typer
