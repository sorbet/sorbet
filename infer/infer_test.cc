#include "ast/ast.h"
#include "common/common.h"
#include "infer.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>

namespace spd = spdlog;

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

TEST_F(InferFixture, HelloWorld) {
    auto ctx = getCtx();
    //    auto tree = hello_world(ctx);
    //    namer::Namer::run(ctx, std::move(tree));
    //    auto &objectScope = ast::GlobalState::defn_object().info(ctx);
    //    ASSERT_EQ(ast::GlobalState::defn_root(), objectScope.owner);
    //
    //    ASSERT_EQ(1, objectScope.members.size());
    //    auto methodPair = objectScope.members[0];
    //    ASSERT_EQ("hello_world", methodPair.first.name(ctx).toString(ctx));
    //    auto &symbol = methodPair.second.info(ctx);
    //    ASSERT_EQ(ast::GlobalState::defn_object(), symbol.owner);
    //    ASSERT_EQ(0, symbol.arguments().size());
    //    ASSERT_EQ(ast::GlobalState::defn_todo(), symbol.result());
}
} // namespace test
} // namespace infer
} // namespace ruby_typer
