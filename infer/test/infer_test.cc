#include "../infer.h"
#include "ast/ast.h"
#include "common/common.h"
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

TEST_F(InferFixture, Literals) {
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
} // namespace test
} // namespace infer
} // namespace ruby_typer
