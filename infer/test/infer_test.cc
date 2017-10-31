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
        ctxPtr = make_unique<ast::GlobalState>(*console);
    }
    ast::Context getCtx() {
        return ast::Context(*ctxPtr, ctxPtr->defn_root());
    }

private:
    unique_ptr<ast::GlobalState> ctxPtr;
};

std::unique_ptr<ast::Statement> getTree(ast::GlobalState &cb, std::string str) {
    auto result = parser::parse_ruby(cb, "<test>", str);
    ruby_typer::ast::Context ctx(cb, cb.defn_root());
    return ast::desugar::node2Tree(ctx, result.ast());
}

TEST_F(InferFixture, LiteralsSubtyping) {
    auto ctx = getCtx();
    auto intLit = make_shared<Literal>(1);
    auto intClass = make_shared<ClassType>(ast::GlobalState::defn_Integer());
    auto floatLit = make_shared<Literal>(1.0f);
    auto floatClass = make_shared<ClassType>(ast::GlobalState::defn_Float());
    auto trueLit = make_shared<Literal>(true);
    auto trueClass = make_shared<ClassType>(ast::GlobalState::defn_TrueClass());
    auto stringLit = make_shared<Literal>(ast::Names::assignTemp());
    auto stringClass = make_shared<ClassType>(ast::GlobalState::defn_String());
    EXPECT_TRUE(Types::isSubType(ctx, intLit, intClass));
    EXPECT_TRUE(Types::isSubType(ctx, floatLit, floatClass));
    EXPECT_TRUE(Types::isSubType(ctx, trueLit, trueClass));
    EXPECT_TRUE(Types::isSubType(ctx, stringLit, stringClass));

    EXPECT_TRUE(Types::isSubType(ctx, intLit, intLit));
    EXPECT_TRUE(Types::isSubType(ctx, floatLit, floatLit));
    EXPECT_TRUE(Types::isSubType(ctx, trueLit, trueLit));
    EXPECT_TRUE(Types::isSubType(ctx, stringLit, stringLit));

    EXPECT_FALSE(Types::isSubType(ctx, intClass, intLit));
}

TEST_F(InferFixture, ClassesLubs) {
    auto ctx = getCtx();
    auto tree = getTree(ctx, "class Bar; end; class Foo < Bar; end");
    namer::Namer::run(ctx, std::move(tree));
    auto &rootScope = ast::GlobalState::defn_root().info(ctx);

    auto barPair = rootScope.members[rootScope.members.size() - 2];
    auto fooPair = rootScope.members[rootScope.members.size() - 1];
    ASSERT_EQ("Foo", fooPair.first.name(ctx).toString(ctx));
    ASSERT_EQ("Bar", barPair.first.name(ctx).toString(ctx));

    auto fooType = make_shared<ClassType>(fooPair.second);
    auto barType = make_shared<ClassType>(barPair.second);

    ASSERT_TRUE(Types::isSubType(ctx, fooType, barType));
    ASSERT_TRUE(Types::isSubType(ctx, fooType, fooType));
    ASSERT_TRUE(Types::isSubType(ctx, barType, barType));
    ASSERT_FALSE(Types::isSubType(ctx, barType, fooType));
}

TEST_F(InferFixture, ClassesSubtyping) {
    auto ctx = getCtx();
    auto tree = getTree(ctx, "class Bar; end; class Foo1 < Bar; end; class Foo2 < Bar;  end");
    namer::Namer::run(ctx, std::move(tree));
    auto &rootScope = ast::GlobalState::defn_root().info(ctx);

    auto barPair = rootScope.members[rootScope.members.size() - 3];
    auto foo1Pair = rootScope.members[rootScope.members.size() - 2];
    auto foo2Pair = rootScope.members[rootScope.members.size() - 1];
    ASSERT_EQ("Foo2", foo2Pair.first.name(ctx).toString(ctx));
    ASSERT_EQ("Foo1", foo1Pair.first.name(ctx).toString(ctx));
    ASSERT_EQ("Bar", barPair.first.name(ctx).toString(ctx));

    auto foo1Type = make_shared<ClassType>(foo1Pair.second);
    auto foo2Type = make_shared<ClassType>(foo2Pair.second);
    auto barType = make_shared<ClassType>(barPair.second);

    auto barNfoo1 = Types::lub(ctx, barType, foo1Type);
    auto foo1Nbar = Types::lub(ctx, foo1Type, barType);
    auto barNfoo2 = Types::lub(ctx, barType, foo2Type);
    auto foo2Nbar = Types::lub(ctx, foo2Type, barType);
    auto foo1Nfoo2 = Types::lub(ctx, foo1Type, foo2Type);
    auto foo2Nfoo1 = Types::lub(ctx, foo2Type, foo1Type);

    ASSERT_EQ("ClassType", barNfoo1->typeName());
    ASSERT_TRUE(Types::isSubType(ctx, barNfoo1, barType));
    ASSERT_TRUE(Types::isSubType(ctx, barNfoo1, foo1Type));
    ASSERT_EQ("ClassType", barNfoo2->typeName());
    ASSERT_TRUE(Types::isSubType(ctx, barNfoo2, barType));
    ASSERT_TRUE(Types::isSubType(ctx, barNfoo2, foo2Type));
    ASSERT_EQ("ClassType", foo1Nbar->typeName());
    ASSERT_TRUE(Types::isSubType(ctx, foo1Nbar, barType));
    ASSERT_TRUE(Types::isSubType(ctx, foo1Nbar, foo1Type));
    ASSERT_EQ("ClassType", foo2Nbar->typeName());
    ASSERT_TRUE(Types::isSubType(ctx, foo2Nbar, barType));
    ASSERT_TRUE(Types::isSubType(ctx, foo2Nbar, foo2Type));

    ASSERT_TRUE(Types::equiv(ctx, barNfoo2, foo2Nbar));
    ASSERT_TRUE(Types::equiv(ctx, barNfoo1, foo1Nbar));
    ASSERT_TRUE(Types::equiv(ctx, foo1Nfoo2, foo2Nfoo1));
}

} // namespace test
} // namespace infer
} // namespace ruby_typer
