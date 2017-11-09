#include "ast/ast.h"
#include "common/common.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <cxxopts.hpp>

using namespace std;

namespace ruby_typer {
using namespace ast;

TEST(HelloTest, GetGreet) {
    EXPECT_EQ("Hello Bazel", "Hello Bazel");
}

namespace spd = spdlog;

auto console = spd::stderr_color_mt("console");

TEST(HelloTest, GetSpdlog) {
    console->info("Welcome to spdlog!");
}

TEST(HelloTest, GetCXXopts) {
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
}

TEST(PreOrderTreeMap, CountTrees) {
    class Counter {
    public:
        int count = 0;
        ClassDef *preTransformClassDef(Context ctx, ClassDef *original) {
            count++;
            return original;
        }
        MethodDef *preTransformMethodDef(Context ctx, MethodDef *original) {
            count++;
            return original;
        }

        If *preTransformIf(Context ctx, If *original) {
            count++;
            return original;
        }

        While *preTransformWhile(Context ctx, While *original) {
            count++;
            return original;
        }

        For *preTransformFor(Context ctx, For *original) {
            count++;
            return original;
        }

        Break *postTransformBreak(Context ctx, Break *original) {
            count++;
            return original;
        }

        Next *postTransformNext(Context ctx, Next *original) {
            count++;
            return original;
        }

        Return *preTransformReturn(Context ctx, Return *original) {
            count++;
            return original;
        }

        Rescue *preTransformRescue(Context ctx, Rescue *original) {
            count++;
            return original;
        }

        Ident *postTransformIdent(Context ctx, Ident *original) {
            count++;
            return original;
        }

        Assign *preTransformAssign(Context ctx, Assign *original) {
            count++;
            return original;
        }

        Send *preTransformSend(Context ctx, Send *original) {
            count++;
            return original;
        }

        NamedArg *preTransformNamedArg(Context ctx, NamedArg *original) {
            count++;
            return original;
        }

        Hash *preTransformHash(Context ctx, Hash *original) {
            count++;
            return original;
        }

        Array *preTransformArray(Context ctx, Array *original) {
            count++;
            return original;
        }

        FloatLit *postTransformFloatLit(Context ctx, FloatLit *original) {
            count++;
            return original;
        }

        IntLit *postTransformIntLit(Context ctx, IntLit *original) {
            count++;
            return original;
        }

        StringLit *postTransformStringLit(Context ctx, StringLit *original) {
            count++;
            return original;
        }

        ConstantLit *postTransformConstantLit(Context ctx, ConstantLit *original) {
            count++;
            return original;
        }

        ArraySplat *preTransformArraySplat(Context ctx, ArraySplat *original) {
            count++;
            return original;
        }

        HashSplat *preTransformHashSplat(Context ctx, HashSplat *original) {
            count++;
            return original;
        }

        Self *postTransformSelf(Context ctx, Self *original) {
            count++;
            return original;
        }

        Block *preTransformBlock(Context ctx, Block *original) {
            count++;
            return original;
        }

        InsSeq *preTransformInsSeq(Context ctx, InsSeq *original) {
            count++;
            return original;
        }
    };

    ruby_typer::ast::GlobalState cb(*console);
    ruby_typer::ast::Context ctx(cb, cb.defn_root());
    static const char *foo_str = "Foo";
    static UTF8Desc foo_DESC{(char *)foo_str, (int)strlen(foo_str)};
    ruby_typer::ast::Loc loc(0, 42, 91);

    auto name = ctx.state.enterNameUTF8(foo_DESC);
    auto classSym = ctx.state.enterClassSymbol(loc, ruby_typer::ast::GlobalState::defn_root(), name);
    auto methodSym = ctx.state.enterMethodSymbol(loc, classSym, name);
    auto empty = vector<SymbolRef>();
    auto argumentSym = ctx.state.enterLocalSymbol(methodSym, name);
    unique_ptr<Expression> rhs(new IntLit(loc, 5));
    auto arg = unique_ptr<Expression>(new Ident(loc, argumentSym));
    auto args = vector<unique_ptr<Expression>>();
    args.emplace_back(move(arg));

    unique_ptr<Expression> methodDef(new MethodDef(loc, methodSym, name, args, move(rhs), false));
    auto emptyTree = unique_ptr<Expression>(new EmptyTree(loc));
    auto cnst = unique_ptr<Expression>(new ConstantLit(loc, move(emptyTree), name));

    ClassDef::RHS_store classrhs;
    classrhs.emplace_back(move(methodDef));
    vector<unique_ptr<Expression>> parents;
    unique_ptr<Expression> tree(new ClassDef(loc, classSym, move(cnst), parents, classrhs, ClassDefKind::Class));
    Counter c;

    auto r = TreeMap<Counter>::apply(ctx, c, move(tree));
    EXPECT_EQ(c.count, 3);
}
} // namespace ruby_typer
