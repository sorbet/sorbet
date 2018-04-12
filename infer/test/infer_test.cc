#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "core/ErrorQueue.h"
#include "core/Names/infer.h"
#include "core/Unfreeze.h"
#include "dsl/dsl.h"
#include "infer/infer.h"
#include "namer/namer.h"
#include "resolver/resolver.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>
#include <memory>

namespace spd = spdlog;
using namespace std;

namespace ruby_typer {
namespace infer {
namespace test {

auto logger = spd::stderr_color_mt("infer_test");
auto errorQueue = std::make_shared<ruby_typer::core::ErrorQueue>(*logger, *logger);

class InferFixture : public ::testing::Test {
public:
    void SetUp() override {
        ctxPtr = make_unique<core::GlobalState>(errorQueue);
        ctxPtr->initEmpty();
    }
    core::MutableContext getCtx() {
        return core::MutableContext(*ctxPtr, core::Symbols::root());
    }

private:
    unique_ptr<core::GlobalState> ctxPtr;
};

void processSource(core::GlobalState &cb, string str) {
    ruby_typer::core::UnfreezeNameTable nt(cb);
    ruby_typer::core::UnfreezeSymbolTable st(cb);
    ruby_typer::core::UnfreezeFileTable ft(cb);
    auto ast = parser::Parser::run(cb, "<test>", str);
    ruby_typer::core::MutableContext ctx(cb, core::Symbols::root());
    auto tree = ast::desugar::node2Tree(ctx, move(ast));
    tree = dsl::DSL::run(ctx, move(tree));
    tree = namer::Namer::run(ctx, move(tree));
    vector<unique_ptr<ast::Expression>> trees;
    trees.emplace_back(move(tree));
    resolver::Resolver::run(ctx, move(trees));
}

TEST_F(InferFixture, LiteralsSubtyping) { // NOLINT
    auto ctx = getCtx();
    auto intLit = make_shared<core::LiteralType>(int64_t(1));
    auto intClass = make_shared<core::ClassType>(core::Symbols::Integer());
    auto floatLit = make_shared<core::LiteralType>(1.0f);
    auto floatClass = make_shared<core::ClassType>(core::Symbols::Float());
    auto trueLit = make_shared<core::LiteralType>(true);
    auto trueClass = make_shared<core::ClassType>(core::Symbols::TrueClass());
    auto stringLit = make_shared<core::LiteralType>(core::Symbols::String(), core::Names::assignTemp());
    auto stringClass = make_shared<core::ClassType>(core::Symbols::String());
    EXPECT_TRUE(core::Types::isSubType(ctx, intLit, intClass));
    EXPECT_TRUE(core::Types::isSubType(ctx, floatLit, floatClass));
    EXPECT_TRUE(core::Types::isSubType(ctx, trueLit, trueClass));
    EXPECT_TRUE(core::Types::isSubType(ctx, stringLit, stringClass));

    EXPECT_TRUE(core::Types::isSubType(ctx, intLit, intLit));
    EXPECT_TRUE(core::Types::isSubType(ctx, floatLit, floatLit));
    EXPECT_TRUE(core::Types::isSubType(ctx, trueLit, trueLit));
    EXPECT_TRUE(core::Types::isSubType(ctx, stringLit, stringLit));

    EXPECT_FALSE(core::Types::isSubType(ctx, intClass, intLit));
    EXPECT_TRUE(core::Types::isSubType(ctx, core::Types::top(), core::Types::dynamic()));
    EXPECT_TRUE(core::Types::isSubType(ctx, core::Types::dynamic(), core::Types::top()));
}

TEST_F(InferFixture, ClassesSubtyping) { // NOLINT
    auto ctx = getCtx();
    processSource(ctx, "class Bar; end; class Foo < Bar; end");
    auto &rootScope = core::Symbols::root().data(ctx);

    auto barSymbol = rootScope.findMember(ctx, ctx.state.enterNameConstant("Bar"));
    auto fooSymbol = rootScope.findMember(ctx, ctx.state.enterNameConstant("Foo"));
    ASSERT_EQ("<constant:Bar>", barSymbol.data(ctx).name.data(ctx).toString(ctx));
    ASSERT_EQ("<constant:Foo>", fooSymbol.data(ctx).name.data(ctx).toString(ctx));

    auto barType = make_shared<core::ClassType>(barSymbol);
    auto fooType = make_shared<core::ClassType>(fooSymbol);

    ASSERT_TRUE(core::Types::isSubType(ctx, fooType, barType));
    ASSERT_TRUE(core::Types::isSubType(ctx, fooType, fooType));
    ASSERT_TRUE(core::Types::isSubType(ctx, barType, barType));
    ASSERT_FALSE(core::Types::isSubType(ctx, barType, fooType));
}

TEST_F(InferFixture, ClassesLubs) { // NOLINT
    auto ctx = getCtx();
    processSource(ctx, "class Bar; end; class Foo1 < Bar; end; class Foo2 < Bar;  end");
    auto &rootScope = core::Symbols::root().data(ctx);

    auto barSymbol = rootScope.findMember(ctx, ctx.state.enterNameConstant("Bar"));
    auto foo1Symbol = rootScope.findMember(ctx, ctx.state.enterNameConstant("Foo1"));
    auto foo2Symbol = rootScope.findMember(ctx, ctx.state.enterNameConstant("Foo2"));
    ASSERT_EQ("<constant:Bar>", barSymbol.data(ctx).name.data(ctx).toString(ctx));
    ASSERT_EQ("<constant:Foo1>", foo1Symbol.data(ctx).name.data(ctx).toString(ctx));
    ASSERT_EQ("<constant:Foo2>", foo2Symbol.data(ctx).name.data(ctx).toString(ctx));

    auto barType = make_shared<core::ClassType>(barSymbol);
    auto foo1Type = make_shared<core::ClassType>(foo1Symbol);
    auto foo2Type = make_shared<core::ClassType>(foo2Symbol);

    auto barNfoo1 = core::Types::lub(ctx, barType, foo1Type);
    auto foo1Nbar = core::Types::lub(ctx, foo1Type, barType);
    auto barNfoo2 = core::Types::lub(ctx, barType, foo2Type);
    auto foo2Nbar = core::Types::lub(ctx, foo2Type, barType);
    auto foo1Nfoo2 = core::Types::lub(ctx, foo1Type, foo2Type);
    auto foo2Nfoo1 = core::Types::lub(ctx, foo2Type, foo1Type);

    ASSERT_EQ("ClassType", barNfoo1->typeName());
    ASSERT_TRUE(core::Types::isSubType(ctx, barType, barNfoo1));
    ASSERT_TRUE(core::Types::isSubType(ctx, foo1Type, barNfoo1));
    ASSERT_EQ("ClassType", barNfoo2->typeName());
    ASSERT_TRUE(core::Types::isSubType(ctx, barType, barNfoo2));
    ASSERT_TRUE(core::Types::isSubType(ctx, foo2Type, barNfoo2));
    ASSERT_EQ("ClassType", foo1Nbar->typeName());
    ASSERT_TRUE(core::Types::isSubType(ctx, barType, foo1Nbar));
    ASSERT_TRUE(core::Types::isSubType(ctx, foo1Type, foo1Nbar));
    ASSERT_EQ("ClassType", foo2Nbar->typeName());
    ASSERT_TRUE(core::Types::isSubType(ctx, barType, foo2Nbar));
    ASSERT_TRUE(core::Types::isSubType(ctx, foo2Type, foo2Nbar));

    ASSERT_TRUE(core::Types::equiv(ctx, barNfoo2, foo2Nbar));
    ASSERT_TRUE(core::Types::equiv(ctx, barNfoo1, foo1Nbar));
    ASSERT_TRUE(core::Types::equiv(ctx, foo1Nfoo2, foo2Nfoo1));

    auto intType = make_shared<core::ClassType>(core::Symbols::Integer());
    auto intNfoo1 = core::Types::lub(ctx, foo1Type, intType);
    auto intNbar = core::Types::lub(ctx, barType, intType);
    auto intNfoo1Nbar = core::Types::lub(ctx, intNfoo1, barType);
    ASSERT_TRUE(core::Types::equiv(ctx, intNfoo1Nbar, intNbar));
    auto intNfoo1Nfoo2 = core::Types::lub(ctx, intNfoo1, foo2Type);
    auto intNfoo1Nfoo2Nbar = core::Types::lub(ctx, intNfoo1Nfoo2, barType);
    ASSERT_TRUE(core::Types::equiv(ctx, intNfoo1Nfoo2Nbar, intNbar));
}

TEST_F(InferFixture, ClassesGlbs) { // NOLINT
    auto ctx = getCtx();
    processSource(ctx, "class Bar; end; class Foo1 < Bar; end; class Foo2 < Bar;  end");
    auto &rootScope = core::Symbols::root().data(ctx);

    auto barSymbol = rootScope.findMember(ctx, ctx.state.enterNameConstant("Bar"));
    auto foo1Symbol = rootScope.findMember(ctx, ctx.state.enterNameConstant("Foo1"));
    auto foo2Symbol = rootScope.findMember(ctx, ctx.state.enterNameConstant("Foo2"));
    ASSERT_EQ("<constant:Bar>", barSymbol.data(ctx).name.data(ctx).toString(ctx));
    ASSERT_EQ("<constant:Foo1>", foo1Symbol.data(ctx).name.data(ctx).toString(ctx));
    ASSERT_EQ("<constant:Foo2>", foo2Symbol.data(ctx).name.data(ctx).toString(ctx));

    auto barType = make_shared<core::ClassType>(barSymbol);
    auto foo1Type = make_shared<core::ClassType>(foo1Symbol);
    auto foo2Type = make_shared<core::ClassType>(foo2Symbol);

    auto barOrfoo1 = core::Types::glb(ctx, barType, foo1Type);
    auto foo1Orbar = core::Types::glb(ctx, foo1Type, barType);
    auto barOrfoo2 = core::Types::glb(ctx, barType, foo2Type);
    auto foo2Orbar = core::Types::glb(ctx, foo2Type, barType);
    auto foo1Orfoo2 = core::Types::glb(ctx, foo1Type, foo2Type);
    auto foo2Orfoo1 = core::Types::glb(ctx, foo2Type, foo1Type);

    ASSERT_EQ("ClassType", barOrfoo1->typeName());
    ASSERT_TRUE(core::Types::isSubType(ctx, barOrfoo1, barType));
    ASSERT_TRUE(core::Types::isSubType(ctx, barOrfoo1, foo1Type));
    ASSERT_EQ("ClassType", barOrfoo2->typeName());
    ASSERT_TRUE(core::Types::isSubType(ctx, barOrfoo2, barType));
    ASSERT_TRUE(core::Types::isSubType(ctx, barOrfoo2, foo2Type));
    ASSERT_EQ("ClassType", foo1Orbar->typeName());
    ASSERT_TRUE(core::Types::isSubType(ctx, foo1Orbar, barType));
    ASSERT_TRUE(core::Types::isSubType(ctx, foo1Orbar, foo1Type));
    ASSERT_EQ("ClassType", foo2Orbar->typeName());
    ASSERT_TRUE(core::Types::isSubType(ctx, foo2Orbar, barType));
    ASSERT_TRUE(core::Types::isSubType(ctx, foo2Orbar, foo2Type));

    ASSERT_TRUE(core::Types::equiv(ctx, barOrfoo2, foo2Orbar));
    ASSERT_TRUE(core::Types::equiv(ctx, barOrfoo1, foo1Orbar));
    ASSERT_TRUE(core::Types::equiv(ctx, foo1Orfoo2, foo2Orfoo1));
}

} // namespace test
} // namespace infer
} // namespace ruby_typer
