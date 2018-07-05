#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/common.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <cxxopts.hpp>

using namespace std;

namespace spd = spdlog;

auto logger = spd::stderr_color_mt("hello-test");
auto errorQueue = std::make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

namespace sorbet {

TEST(HelloTest, GetGreet) { // NOLINT
    EXPECT_EQ("Hello Bazel", "Hello Bazel");
}

namespace spd = spdlog;

TEST(HelloTest, GetSpdlog) { // NOLINT
    logger->info("Welcome to spdlog!");
}

TEST(HelloTest, GetCXXopts) { // NOLINT
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
}

TEST(PreOrderTreeMap, CountTrees) { // NOLINT
    class Counter {
    public:
        int count = 0;
        unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
            count++;
            return original;
        }
        unique_ptr<ast::MethodDef> preTransformMethodDef(core::MutableContext ctx,
                                                         unique_ptr<ast::MethodDef> original) {
            count++;
            return original;
        }

        unique_ptr<ast::If> preTransformIf(core::MutableContext ctx, unique_ptr<ast::If> original) {
            count++;
            return original;
        }

        unique_ptr<ast::While> preTransformWhile(core::MutableContext ctx, unique_ptr<ast::While> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Break> postTransformBreak(core::MutableContext ctx, unique_ptr<ast::Break> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Next> postTransformNext(core::MutableContext ctx, unique_ptr<ast::Next> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Return> preTransformReturn(core::MutableContext ctx, unique_ptr<ast::Return> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Rescue> preTransformRescue(core::MutableContext ctx, unique_ptr<ast::Rescue> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Ident> postTransformIdent(core::MutableContext ctx, unique_ptr<ast::Ident> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Assign> preTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Send> preTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Hash> preTransformHash(core::MutableContext ctx, unique_ptr<ast::Hash> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Array> preTransformArray(core::MutableContext ctx, unique_ptr<ast::Array> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Literal> postTransformLiteral(core::MutableContext ctx, unique_ptr<ast::Literal> original) {
            count++;
            return original;
        }

        unique_ptr<ast::ConstantLit> postTransformConstantLit(core::MutableContext ctx,
                                                              unique_ptr<ast::ConstantLit> original) {
            count++;
            return original;
        }

        unique_ptr<ast::ArraySplat> preTransformArraySplat(core::MutableContext ctx,
                                                           unique_ptr<ast::ArraySplat> original) {
            count++;
            return original;
        }

        unique_ptr<ast::HashSplat> preTransformHashSplat(core::MutableContext ctx,
                                                         unique_ptr<ast::HashSplat> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Self> postTransformSelf(core::MutableContext ctx, unique_ptr<ast::Self> original) {
            count++;
            return original;
        }

        unique_ptr<ast::Block> preTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> original) {
            count++;
            return original;
        }

        unique_ptr<ast::InsSeq> preTransformInsSeq(core::MutableContext ctx, unique_ptr<ast::InsSeq> original) {
            count++;
            return original;
        }
    };

    sorbet::core::GlobalState cb(errorQueue);
    cb.initEmpty();
    sorbet::core::MutableContext ctx(cb, core::Symbols::root());
    static const char *foo_str = "Foo";
    sorbet::core::Loc loc(sorbet::core::FileRef(), 42, 91);
    sorbet::core::UnfreezeNameTable nt(ctx);
    sorbet::core::UnfreezeSymbolTable st(ctx);

    auto name = ctx.state.enterNameUTF8(foo_str);
    auto classSym = ctx.state.enterClassSymbol(loc, sorbet::core::Symbols::root(), ctx.state.enterNameConstant(name));

    // see if it crashes via failed ENFORCE
    ctx.state.enterTypeMember(loc, classSym, ctx.state.enterNameConstant(name), sorbet::core::Variance::CoVariant);
    auto methodSym = ctx.state.enterMethodSymbol(loc, classSym, name);

    // see if it crashes via failed ENFORCE
    ctx.state.enterTypeArgument(loc, methodSym, ctx.state.enterNameConstant(name), sorbet::core::Variance::CoVariant);

    auto empty = vector<core::SymbolRef>();
    auto argumentSym = core::LocalVariable(name, 0);
    unique_ptr<ast::Expression> rhs(ast::MK::Int(loc, 5));
    auto arg = unique_ptr<ast::Expression>(new ast::Local(loc, argumentSym));
    ast::MethodDef::ARGS_store args;
    args.emplace_back(move(arg));

    unique_ptr<ast::Expression> methodDef(new ast::MethodDef(loc, methodSym, name, move(args), move(rhs), false));
    auto emptyTree = unique_ptr<ast::Expression>(new ast::EmptyTree(loc));
    auto cnst = unique_ptr<ast::Expression>(new ast::ConstantLit(loc, move(emptyTree), name));

    ast::ClassDef::RHS_store classrhs;
    classrhs.emplace_back(move(methodDef));
    unique_ptr<ast::Expression> tree(new ast::ClassDef(loc, classSym, move(cnst), ast::ClassDef::ANCESTORS_store(),
                                                       move(classrhs), ast::ClassDefKind::Class));
    Counter c;

    auto r = ast::TreeMap::apply(ctx, c, move(tree));
    EXPECT_EQ(c.count, 3);
}

TEST(PayloadTests, CloneSubstitutePayload) {
    auto logger = spd::stderr_color_mt("ClonePayload");
    auto errorQueue = std::make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

    sorbet::core::GlobalState gs(errorQueue);
    sorbet::core::serialize::Serializer::loadGlobalState(gs, getNameTablePayload);

    auto c1 = gs.deepCopy();
    auto c2 = gs.deepCopy();

    sorbet::core::NameRef n1;
    {
        sorbet::core::UnfreezeNameTable thaw1(*c1);
        n1 = c1->enterNameUTF8("test new name");
    }

    sorbet::core::GlobalSubstitution subst(*c1, *c2);
    ASSERT_EQ("test new name", subst.substitute(n1).toString(*c2));
    ASSERT_EQ(c1->symbolsUsed(), c2->symbolsUsed());
    ASSERT_EQ(c1->symbolsUsed(), gs.symbolsUsed());
}
} // namespace sorbet
