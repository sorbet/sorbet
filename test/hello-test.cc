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
auto errorQueue = std::make_shared<ruby_typer::core::ErrorQueue>(*logger);

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
        ClassDef *preTransformClassDef(core::MutableContext ctx, ClassDef *original) {
            count++;
            return original;
        }
        MethodDef *preTransformMethodDef(core::MutableContext ctx, MethodDef *original) {
            count++;
            return original;
        }

        If *preTransformIf(core::MutableContext ctx, If *original) {
            count++;
            return original;
        }

        While *preTransformWhile(core::MutableContext ctx, While *original) {
            count++;
            return original;
        }

        Break *postTransformBreak(core::MutableContext ctx, Break *original) {
            count++;
            return original;
        }

        Next *postTransformNext(core::MutableContext ctx, Next *original) {
            count++;
            return original;
        }

        Return *preTransformReturn(core::MutableContext ctx, Return *original) {
            count++;
            return original;
        }

        Rescue *preTransformRescue(core::MutableContext ctx, Rescue *original) {
            count++;
            return original;
        }

        Ident *postTransformIdent(core::MutableContext ctx, Ident *original) {
            count++;
            return original;
        }

        Assign *preTransformAssign(core::MutableContext ctx, Assign *original) {
            count++;
            return original;
        }

        Send *preTransformSend(core::MutableContext ctx, Send *original) {
            count++;
            return original;
        }

        Hash *preTransformHash(core::MutableContext ctx, Hash *original) {
            count++;
            return original;
        }

        Array *preTransformArray(core::MutableContext ctx, Array *original) {
            count++;
            return original;
        }

        FloatLit *postTransformFloatLit(core::MutableContext ctx, FloatLit *original) {
            count++;
            return original;
        }

        IntLit *postTransformIntLit(core::MutableContext ctx, IntLit *original) {
            count++;
            return original;
        }

        StringLit *postTransformStringLit(core::MutableContext ctx, StringLit *original) {
            count++;
            return original;
        }

        ConstantLit *postTransformConstantLit(core::MutableContext ctx, ConstantLit *original) {
            count++;
            return original;
        }

        ArraySplat *preTransformArraySplat(core::MutableContext ctx, ArraySplat *original) {
            count++;
            return original;
        }

        HashSplat *preTransformHashSplat(core::MutableContext ctx, HashSplat *original) {
            count++;
            return original;
        }

        Self *postTransformSelf(core::MutableContext ctx, Self *original) {
            count++;
            return original;
        }

        Block *preTransformBlock(core::MutableContext ctx, Block *original) {
            count++;
            return original;
        }

        InsSeq *preTransformInsSeq(core::MutableContext ctx, InsSeq *original) {
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
    auto methodSym = ctx.state.enterMethodSymbol(loc, classSym, name);
    auto empty = vector<core::SymbolRef>();
    auto argumentSym = core::LocalVariable(name, 0);
    unique_ptr<Expression> rhs(new IntLit(loc, 5));
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

    auto r = TreeMap<Counter, core::MutableContext>::apply(ctx, c, move(tree));
    EXPECT_EQ(c.count, 3);
}
} // namespace ruby_typer
