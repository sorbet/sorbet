#include "../infer.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "namer/namer.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>
#include <memory>

namespace spd = spdlog;
using namespace std;

namespace ruby_typer {
namespace infer {
namespace test {

auto console = spd::stderr_color_mt("infer");

class InferFixture : public ::testing::Test {
public:
    void SetUp() {
        ctxPtr = make_unique<core::GlobalState>(*console);
    }
    core::Context getCtx() {
        return core::Context(*ctxPtr, ctxPtr->defn_root());
    }

private:
    unique_ptr<core::GlobalState> ctxPtr;
};

unique_ptr<ast::Expression> getTree(core::GlobalState &cb, string str) {
    auto ast = parser::Parser::run(cb, "<test>", str);
    ruby_typer::core::Context ctx(cb, cb.defn_root());
    return ast::desugar::node2Tree(ctx, ast);
}

TEST_F(InferFixture, LiteralsSubtyping) {
    auto ctx = getCtx();
    auto intLit = make_shared<core::Literal>(1);
    auto intClass = make_shared<core::ClassType>(core::GlobalState::defn_Integer());
    auto floatLit = make_shared<core::Literal>(1.0f);
    auto floatClass = make_shared<core::ClassType>(core::GlobalState::defn_Float());
    auto trueLit = make_shared<core::Literal>(true);
    auto trueClass = make_shared<core::ClassType>(core::GlobalState::defn_TrueClass());
    auto stringLit = make_shared<core::Literal>(core::Names::assignTemp());
    auto stringClass = make_shared<core::ClassType>(core::GlobalState::defn_String());
    EXPECT_TRUE(core::Types::isSubType(ctx, intLit, intClass));
    EXPECT_TRUE(core::Types::isSubType(ctx, floatLit, floatClass));
    EXPECT_TRUE(core::Types::isSubType(ctx, trueLit, trueClass));
    EXPECT_TRUE(core::Types::isSubType(ctx, stringLit, stringClass));

    EXPECT_TRUE(core::Types::isSubType(ctx, intLit, intLit));
    EXPECT_TRUE(core::Types::isSubType(ctx, floatLit, floatLit));
    EXPECT_TRUE(core::Types::isSubType(ctx, trueLit, trueLit));
    EXPECT_TRUE(core::Types::isSubType(ctx, stringLit, stringLit));

    EXPECT_FALSE(core::Types::isSubType(ctx, intClass, intLit));
}

TEST_F(InferFixture, ClassesLubs) {
    auto ctx = getCtx();
    auto tree = getTree(ctx, "class Bar; end; class Foo < Bar; end");
    namer::Namer::run(ctx, move(tree));
    auto &rootScope = core::GlobalState::defn_root().info(ctx);

    auto barPair = rootScope.members[rootScope.members.size() - 2];
    auto fooPair = rootScope.members[rootScope.members.size() - 1];
    ASSERT_EQ("Foo", fooPair.first.name(ctx).toString(ctx));
    ASSERT_EQ("Bar", barPair.first.name(ctx).toString(ctx));

    auto fooType = make_shared<core::ClassType>(fooPair.second);
    auto barType = make_shared<core::ClassType>(barPair.second);

    ASSERT_TRUE(core::Types::isSubType(ctx, fooType, barType));
    ASSERT_TRUE(core::Types::isSubType(ctx, fooType, fooType));
    ASSERT_TRUE(core::Types::isSubType(ctx, barType, barType));
    ASSERT_FALSE(core::Types::isSubType(ctx, barType, fooType));
}

TEST_F(InferFixture, ClassesSubtyping) {
    auto ctx = getCtx();
    auto tree = getTree(ctx, "class Bar; end; class Foo1 < Bar; end; class Foo2 < Bar;  end");
    namer::Namer::run(ctx, move(tree));
    auto &rootScope = core::GlobalState::defn_root().info(ctx);

    auto barPair = rootScope.members[rootScope.members.size() - 3];
    auto foo1Pair = rootScope.members[rootScope.members.size() - 2];
    auto foo2Pair = rootScope.members[rootScope.members.size() - 1];
    ASSERT_EQ("Foo2", foo2Pair.first.name(ctx).toString(ctx));
    ASSERT_EQ("Foo1", foo1Pair.first.name(ctx).toString(ctx));
    ASSERT_EQ("Bar", barPair.first.name(ctx).toString(ctx));

    auto foo1Type = make_shared<core::ClassType>(foo1Pair.second);
    auto foo2Type = make_shared<core::ClassType>(foo2Pair.second);
    auto barType = make_shared<core::ClassType>(barPair.second);

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
}

} // namespace test
} // namespace infer
} // namespace ruby_typer
