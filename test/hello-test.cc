#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/common.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <cxxopts.hpp>

using namespace std;

namespace ruby_typer {
using namespace ast;

TEST(HelloTest, GetGreet) { // NOLINT
    EXPECT_EQ("Hello Bazel", "Hello Bazel");
}

namespace spd = spdlog;

auto console = spd::stderr_color_mt("console");

TEST(HelloTest, GetSpdlog) { // NOLINT
    console->info("Welcome to spdlog!");
}

TEST(HelloTest, GetCXXopts) { // NOLINT
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
}

TEST(PreOrderTreeMap, CountTrees) { // NOLINT
    class Counter {
    public:
        int count = 0;
        ClassDef *preTransformClassDef(core::Context ctx, ClassDef *original) {
            count++;
            return original;
        }
        MethodDef *preTransformMethodDef(core::Context ctx, MethodDef *original) {
            count++;
            return original;
        }

        If *preTransformIf(core::Context ctx, If *original) {
            count++;
            return original;
        }

        While *preTransformWhile(core::Context ctx, While *original) {
            count++;
            return original;
        }

        Break *postTransformBreak(core::Context ctx, Break *original) {
            count++;
            return original;
        }

        Next *postTransformNext(core::Context ctx, Next *original) {
            count++;
            return original;
        }

        Return *preTransformReturn(core::Context ctx, Return *original) {
            count++;
            return original;
        }

        Rescue *preTransformRescue(core::Context ctx, Rescue *original) {
            count++;
            return original;
        }

        Ident *postTransformIdent(core::Context ctx, Ident *original) {
            count++;
            return original;
        }

        Assign *preTransformAssign(core::Context ctx, Assign *original) {
            count++;
            return original;
        }

        Send *preTransformSend(core::Context ctx, Send *original) {
            count++;
            return original;
        }

        NamedArg *preTransformNamedArg(core::Context ctx, NamedArg *original) {
            count++;
            return original;
        }

        Hash *preTransformHash(core::Context ctx, Hash *original) {
            count++;
            return original;
        }

        Array *preTransformArray(core::Context ctx, Array *original) {
            count++;
            return original;
        }

        FloatLit *postTransformFloatLit(core::Context ctx, FloatLit *original) {
            count++;
            return original;
        }

        IntLit *postTransformIntLit(core::Context ctx, IntLit *original) {
            count++;
            return original;
        }

        StringLit *postTransformStringLit(core::Context ctx, StringLit *original) {
            count++;
            return original;
        }

        ConstantLit *postTransformConstantLit(core::Context ctx, ConstantLit *original) {
            count++;
            return original;
        }

        ArraySplat *preTransformArraySplat(core::Context ctx, ArraySplat *original) {
            count++;
            return original;
        }

        HashSplat *preTransformHashSplat(core::Context ctx, HashSplat *original) {
            count++;
            return original;
        }

        Self *postTransformSelf(core::Context ctx, Self *original) {
            count++;
            return original;
        }

        Block *preTransformBlock(core::Context ctx, Block *original) {
            count++;
            return original;
        }

        InsSeq *preTransformInsSeq(core::Context ctx, InsSeq *original) {
            count++;
            return original;
        }
    };

    ruby_typer::core::GlobalState cb(*console);
    ruby_typer::core::Context ctx(cb, cb.defn_root());
    static const char *foo_str = "Foo";
    static core::UTF8Desc foo_DESC{(char *)foo_str, (int)strlen(foo_str)};
    ruby_typer::core::Loc loc(0, 42, 91);

    auto name = ctx.state.enterNameUTF8(foo_DESC);
    auto classSym = ctx.state.enterClassSymbol(loc, ruby_typer::core::GlobalState::defn_root(), name);
    auto methodSym = ctx.state.enterMethodSymbol(loc, classSym, name);
    auto empty = vector<core::SymbolRef>();
    auto argumentSym = ctx.state.enterLocalSymbol(methodSym, name);
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

    auto r = TreeMap<Counter>::apply(ctx, c, move(tree));
    EXPECT_EQ(c.count, 3);
}
} // namespace ruby_typer
