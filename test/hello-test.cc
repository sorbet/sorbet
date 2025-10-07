#include "doctest/doctest.h"
#include <cxxopts.hpp>
// has to go first as it violates our requirements

#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/common.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/NameSubstitution.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

auto logger = spdlog::stderr_color_mt("hello-test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

namespace sorbet {

TEST_CASE("GetGreet") {
    CHECK_EQ("Hello Bazel", "Hello Bazel");
}

TEST_CASE("GetSpdlog") {
    logger->info("Welcome to spdlog!");
}

TEST_CASE("GetCXXopts") {
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
}

TEST_CASE("CountTrees") {
    class Counter {
    public:
        int count = 0;
        void preTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }
        void preTransformMethodDef(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformIf(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformWhile(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void postTransformBreak(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void postTransformNext(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformReturn(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformRescue(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void postTransformConstantLit(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformAssign(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformSend(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformHash(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformArray(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void postTransformLiteral(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void postTransformUnresolvedConstantLit(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformBlock(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }

        void preTransformInsSeq(core::MutableContext ctx, ast::ExpressionPtr &original) {
            count++;
        }
    };

    sorbet::core::GlobalState cb(errorQueue);
    cb.initEmpty();
    static constexpr string_view foo_str = "Foo"sv;
    sorbet::core::Loc loc(sorbet::core::FileRef(), 42, 91);
    sorbet::core::UnfreezeNameTable nt(cb);
    sorbet::core::UnfreezeSymbolTable st(cb);

    auto name = cb.enterNameUTF8(foo_str);
    auto classSym = cb.enterClassSymbol(loc, sorbet::core::Symbols::root(), cb.enterNameConstant(name));

    // see if it crashes via failed ENFORCE
    cb.enterTypeMember(loc, classSym, cb.enterNameConstant(name), sorbet::core::Variance::CoVariant);
    auto methodSym = cb.enterMethodSymbol(loc, classSym, name);

    // see if it crashes via failed ENFORCE
    cb.enterTypeParameter(loc, methodSym, cb.enterNameConstant(name), sorbet::core::Variance::CoVariant);

    auto empty = vector<core::SymbolRef>();
    auto argumentSym = core::LocalVariable(name, 0);
    auto rhs(ast::MK::Int(loc.offsets(), 5));
    auto param = ast::make_expression<ast::Local>(loc.offsets(), argumentSym);
    ast::MethodDef::PARAMS_store params;
    params.emplace_back(std::move(param));

    ast::MethodDef::Flags flags;
    auto methodDef = ast::make_expression<ast::MethodDef>(loc.offsets(), loc.offsets(), methodSym, name,
                                                          std::move(params), std::move(rhs), flags);
    auto emptyTree = ast::MK::EmptyTree();
    auto cnst = ast::make_expression<ast::UnresolvedConstantLit>(loc.offsets(), std::move(emptyTree), name);

    ast::ClassDef::RHS_store classrhs;
    classrhs.emplace_back(std::move(methodDef));
    auto tree = ast::make_expression<ast::ClassDef>(loc.offsets(), loc.offsets(), classSym, std::move(cnst),
                                                    ast::ClassDef::ANCESTORS_store(), std::move(classrhs),
                                                    ast::ClassDef::Kind::Class);
    Counter c;
    sorbet::core::MutableContext ctx(cb, core::Symbols::root(), loc.file());

    ast::TreeWalk::apply(ctx, c, tree);
    CHECK_EQ(c.count, 3);
}

TEST_CASE("CloneSubstitutePayload") {
    auto logger = spdlog::stderr_color_mt("ClonePayload");
    auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

    sorbet::core::GlobalState gs(errorQueue);
    sorbet::core::serialize::Serializer::loadGlobalState(gs, PAYLOAD_SYMBOL_TABLE, PAYLOAD_NAME_TABLE,
                                                         PAYLOAD_FILE_TABLE);

    auto c1 = gs.deepCopyGlobalState();
    auto c2 = gs.deepCopyGlobalState();

    sorbet::core::NameRef n1;
    {
        sorbet::core::UnfreezeNameTable thaw1(*c1);
        n1 = c1->enterNameUTF8("test new name");
    }

    sorbet::core::NameSubstitution subst(*c1, *c2);
    REQUIRE_EQ("<U test new name>", subst.substitute(n1).showRaw(*c2));
    REQUIRE_EQ(c1->symbolsUsedTotal(), c2->symbolsUsedTotal());
    REQUIRE_EQ(c1->symbolsUsedTotal(), gs.symbolsUsedTotal());
}
} // namespace sorbet
