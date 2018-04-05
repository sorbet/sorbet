#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/common.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <cxxopts.hpp>

using namespace std;

namespace spd = spdlog;

auto logger = spd::stderr_color_mt("hello-test");
auto errorQueue = std::make_shared<ruby_typer::core::ErrorQueue>(*logger, *logger);

namespace ruby_typer {
using namespace ast;

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
        unique_ptr<ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ClassDef> original) {
            count++;
            return original;
        }
        unique_ptr<MethodDef> preTransformMethodDef(core::MutableContext ctx, unique_ptr<MethodDef> original) {
            count++;
            return original;
        }

        unique_ptr<If> preTransformIf(core::MutableContext ctx, unique_ptr<If> original) {
            count++;
            return original;
        }

        unique_ptr<While> preTransformWhile(core::MutableContext ctx, unique_ptr<While> original) {
            count++;
            return original;
        }

        unique_ptr<Break> postTransformBreak(core::MutableContext ctx, unique_ptr<Break> original) {
            count++;
            return original;
        }

        unique_ptr<Next> postTransformNext(core::MutableContext ctx, unique_ptr<Next> original) {
            count++;
            return original;
        }

        unique_ptr<Return> preTransformReturn(core::MutableContext ctx, unique_ptr<Return> original) {
            count++;
            return original;
        }

        unique_ptr<Rescue> preTransformRescue(core::MutableContext ctx, unique_ptr<Rescue> original) {
            count++;
            return original;
        }

        unique_ptr<Ident> postTransformIdent(core::MutableContext ctx, unique_ptr<Ident> original) {
            count++;
            return original;
        }

        unique_ptr<Assign> preTransformAssign(core::MutableContext ctx, unique_ptr<Assign> original) {
            count++;
            return original;
        }

        unique_ptr<Send> preTransformSend(core::MutableContext ctx, unique_ptr<Send> original) {
            count++;
            return original;
        }

        unique_ptr<Hash> preTransformHash(core::MutableContext ctx, unique_ptr<Hash> original) {
            count++;
            return original;
        }

        unique_ptr<Array> preTransformArray(core::MutableContext ctx, unique_ptr<Array> original) {
            count++;
            return original;
        }

        unique_ptr<Literal> postTransformLiteral(core::MutableContext ctx, unique_ptr<Literal> original) {
            count++;
            return original;
        }

        unique_ptr<ConstantLit> postTransformConstantLit(core::MutableContext ctx, unique_ptr<ConstantLit> original) {
            count++;
            return original;
        }

        unique_ptr<ArraySplat> preTransformArraySplat(core::MutableContext ctx, unique_ptr<ArraySplat> original) {
            count++;
            return original;
        }

        unique_ptr<HashSplat> preTransformHashSplat(core::MutableContext ctx, unique_ptr<HashSplat> original) {
            count++;
            return original;
        }

        unique_ptr<Self> postTransformSelf(core::MutableContext ctx, unique_ptr<Self> original) {
            count++;
            return original;
        }

        unique_ptr<Block> preTransformBlock(core::MutableContext ctx, unique_ptr<Block> original) {
            count++;
            return original;
        }

        unique_ptr<InsSeq> preTransformInsSeq(core::MutableContext ctx, unique_ptr<InsSeq> original) {
            count++;
            return original;
        }
    };

    ruby_typer::core::GlobalState cb(errorQueue);
    cb.initEmpty();
    ruby_typer::core::MutableContext ctx(cb, core::Symbols::root());
    static const char *foo_str = "Foo";
    ruby_typer::core::Loc loc(ruby_typer::core::FileRef(), 42, 91);
    ruby_typer::core::UnfreezeNameTable nt(ctx);
    ruby_typer::core::UnfreezeSymbolTable st(ctx);

    auto name = ctx.state.enterNameUTF8(foo_str);
    auto classSym =
        ctx.state.enterClassSymbol(loc, ruby_typer::core::Symbols::root(), ctx.state.enterNameConstant(name));

    // see if it crashes via failed ENFORCE
    ctx.state.enterTypeMember(loc, classSym, ctx.state.enterNameConstant(name), ruby_typer::core::Variance::CoVariant);
    auto methodSym = ctx.state.enterMethodSymbol(loc, classSym, name);

    // see if it crashes via failed ENFORCE
    ctx.state.enterTypeArgument(loc, methodSym, ctx.state.enterNameConstant(name),
                                ruby_typer::core::Variance::CoVariant);

    auto empty = vector<core::SymbolRef>();
    auto argumentSym = core::LocalVariable(name, 0);
    unique_ptr<Expression> rhs(ast::MK::Int(loc, 5));
    auto arg = unique_ptr<Expression>(new Local(loc, argumentSym));
    MethodDef::ARGS_store args;
    args.emplace_back(move(arg));

    unique_ptr<Expression> methodDef(new MethodDef(loc, methodSym, name, move(args), move(rhs), false));
    auto emptyTree = unique_ptr<Expression>(new EmptyTree(loc));
    auto cnst = unique_ptr<Expression>(new ConstantLit(loc, move(emptyTree), name));

    ClassDef::RHS_store classrhs;
    classrhs.emplace_back(move(methodDef));
    unique_ptr<Expression> tree(
        new ClassDef(loc, classSym, move(cnst), ClassDef::ANCESTORS_store(), move(classrhs), ClassDefKind::Class));
    Counter c;

    auto r = TreeMap::apply(ctx, c, move(tree));
    EXPECT_EQ(c.count, 3);
}
} // namespace ruby_typer
