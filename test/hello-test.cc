#include "ast/ast.h"
#include "common/common.h"
#include "main/hello-greet.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <cxxopts.hpp>

namespace ruby_typer {
using namespace ast;

TEST(HelloTest, GetGreet) {
    EXPECT_EQ(get_greet("Bazel"), "Hello Bazel");
}

namespace spd = spdlog;

auto console = spd::stdout_color_mt("console");

TEST(HelloTest, GetSpdlog) {
    console->info("Welcome to spdlog!");
}

TEST(HelloTest, GetCXXopts) {
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
}

TEST(TreeMap, CountTrees) {
    class Counter {
    public:
        int count = 0;
        Statement *transformClassDef(Context ctx, ClassDef *original) {
            count++;
            return original;
        }
        Statement *transformMethodDef(Context ctx, MethodDef *original) {
            count++;
            return original;
        }

        Statement *transformIf(Context ctx, If *original) {
            count++;
            return original;
        }

        Statement *transformWhile(Context ctx, While *original) {
            count++;
            return original;
        }

        Statement *transformFor(Context ctx, For *original) {
            count++;
            return original;
        }

        Statement *transformBreak(Context ctx, Break *original) {
            count++;
            return original;
        }

        Statement *transformNext(Context ctx, Next *original) {
            count++;
            return original;
        }

        Statement *transformReturn(Context ctx, Return *original) {
            count++;
            return original;
        }

        Statement *transformRescue(Context ctx, Rescue *original) {
            count++;
            return original;
        }

        Statement *transformIdent(Context ctx, Ident *original) {
            count++;
            return original;
        }

        Statement *transformAssign(Context ctx, Assign *original) {
            count++;
            return original;
        }

        Statement *transformSend(Context ctx, Send *original) {
            count++;
            return original;
        }

        Statement *transformNew(Context ctx, New *original) {
            count++;
            return original;
        }

        Statement *transformNamedArg(Context ctx, NamedArg *original) {
            count++;
            return original;
        }

        Statement *transformHash(Context ctx, Hash *original) {
            count++;
            return original;
        }

        Statement *transformArray(Context ctx, Array *original) {
            count++;
            return original;
        }

        Statement *transformFloatLit(Context ctx, FloatLit *original) {
            count++;
            return original;
        }

        Statement *transformIntLit(Context ctx, IntLit *original) {
            count++;
            return original;
        }

        Statement *transformStringLit(Context ctx, StringLit *original) {
            count++;
            return original;
        }

        Statement *transformConstantLit(Context ctx, ConstantLit *original) {
            count++;
            return original;
        }

        Statement *transformArraySplat(Context ctx, ArraySplat *original) {
            count++;
            return original;
        }

        Statement *transformHashSplat(Context ctx, HashSplat *original) {
            count++;
            return original;
        }

        Statement *transformSelf(Context ctx, Self *original) {
            count++;
            return original;
        }

        Statement *transformBlock(Context ctx, Block *original) {
            count++;
            return original;
        }

        Statement *transformInsSeq(Context ctx, InsSeq *original) {
            count++;
            return original;
        }
    };

    ruby_typer::ast::ContextBase cb(*console);
    ruby_typer::ast::Context ctx(cb, cb.defn_root());
    static const char *foo_str = "Foo";
    static UTF8Desc foo_DESC{(char *)foo_str, (int)std::strlen(foo_str)};

    auto name = ctx.state.enterNameUTF8(foo_DESC);
    auto classSym = ctx.state.getTopLevelClassSymbol(name);
    auto argTypes = std::vector<SymbolRef>{ctx.state.defn_top()};
    auto methodSym = ctx.state.enterSymbol(classSym, name, ctx.state.defn_top(), argTypes, true);
    auto empty = std::vector<SymbolRef>();
    auto argumentSym = ctx.state.enterSymbol(methodSym, name, ctx.state.defn_top(), empty, false);
    std::unique_ptr<Expression> rhs(new IntLit(5));
    auto arg = std::unique_ptr<Expression>(new Ident(argumentSym));
    auto args = std::vector<unique_ptr<Expression>>();
    args.emplace_back(std::move(arg));

    std::unique_ptr<Statement> methodDef(new MethodDef(methodSym, name, args, std::move(rhs), false));
    auto emptyTree = std::unique_ptr<Expression>(new EmptyTree());
    auto cnst = std::unique_ptr<Expression>(new ConstantLit(std::move(emptyTree), name));

    std::vector<std::unique_ptr<Statement>> classrhs;
    classrhs.emplace_back(std::move(methodDef));
    std::unique_ptr<Statement> tree(new ClassDef(classSym, std::move(cnst), classrhs, false));
    Counter c;

    auto r = TreeMap<Counter>::apply(ctx, c, std::move(tree));
    EXPECT_EQ(c.count, 3);
}
} // namespace ruby_typer
