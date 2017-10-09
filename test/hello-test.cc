#include "../ast/Context.h"
#include "ast/TreeMap.h"
#include "ast/Trees.h"
#include "common/common.h"
#include "main/hello-greet.h"
#include "parser/Result.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <cxxopts.hpp>

namespace sruby {
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
        Stat *transformClassDef(Context ctx, ClassDef *original) {
            count++;
            return original;
        }
        Stat *transformMethodDef(Context ctx, MethodDef *original) {
            count++;
            return original;
        }

        Stat *transformSelfMethodDef(Context ctx, SelfMethodDef *original) {
            count++;
            return original;
        }

        Stat *transformSelfMethodDef(Context ctx, ConstDef *original) {
            count++;
            return original;
        }

        Stat *transformIf(Context ctx, If *original) {
            count++;
            return original;
        }

        Stat *transformWhile(Context ctx, While *original) {
            count++;
            return original;
        }

        Stat *transformFor(Context ctx, For *original) {
            count++;
            return original;
        }

        Stat *transformBreak(Context ctx, Break *original) {
            count++;
            return original;
        }

        Stat *transformNext(Context ctx, Next *original) {
            count++;
            return original;
        }

        Stat *transformReturn(Context ctx, Return *original) {
            count++;
            return original;
        }

        Stat *transformRescue(Context ctx, Rescue *original) {
            count++;
            return original;
        }

        Stat *transformIdent(Context ctx, Ident *original) {
            count++;
            return original;
        }

        Stat *transformAssign(Context ctx, Assign *original) {
            count++;
            return original;
        }

        Stat *transformSend(Context ctx, Send *original) {
            count++;
            return original;
        }

        Stat *transformNew(Context ctx, New *original) {
            count++;
            return original;
        }

        Stat *transformNamedArg(Context ctx, NamedArg *original) {
            count++;
            return original;
        }

        Stat *transformHash(Context ctx, Hash *original) {
            count++;
            return original;
        }

        Stat *transformArray(Context ctx, Array *original) {
            count++;
            return original;
        }

        Stat *transformFloatLit(Context ctx, FloatLit *original) {
            count++;
            return original;
        }

        Stat *transformIntLit(Context ctx, IntLit *original) {
            count++;
            return original;
        }

        Stat *transformStringLit(Context ctx, StringLit *original) {
            count++;
            return original;
        }

        Stat *transformConstantLit(Context ctx, ConstantLit *original) {
            count++;
            return original;
        }

        Stat *transformArraySplat(Context ctx, ArraySplat *original) {
            count++;
            return original;
        }

        Stat *transformHashSplat(Context ctx, HashSplat *original) {
            count++;
            return original;
        }

        Stat *transformSelf(Context ctx, Self *original) {
            count++;
            return original;
        }

        Stat *transformClosure(Context ctx, Closure *original) {
            count++;
            return original;
        }

        Stat *transformInsSeq(Context ctx, InsSeq *original) {
            count++;
            return original;
        }
    };

    sruby::ast::ContextBase cb(*console);
    sruby::ast::Context ctx(cb, cb.defn_root());
    static const char *foo_str = "Foo";
    static UTF8Desc foo_DESC{(char *)foo_str, (int)std::strlen(foo_str)};

    auto name = ctx.state.enterNameUTF8(foo_DESC);
    auto classSym = ctx.state.getTopLevelClassSymbol(name);
    auto argTypes = std::vector<SymbolRef>{ctx.state.defn_top()};
    auto methodSym = ctx.state.enterSymbol(classSym, name, ctx.state.defn_top(), argTypes, true);
    auto empty = std::vector<SymbolRef>();
    auto argumentSym = ctx.state.enterSymbol(methodSym, name, ctx.state.defn_top(), empty, false);
    std::unique_ptr<Expr> rhs(new IntLit(5));
    auto args = std::vector<SymbolRef>{argumentSym};

    std::unique_ptr<Stat> classRhs(new MethodDef(methodSym, args, std::move(rhs)));

    std::unique_ptr<Stat> tree(new ClassDef(classSym, std::move(classRhs)));
    Counter c;

    auto r = TreeMap<Counter>::apply(ctx, c, std::move(tree));
    EXPECT_EQ(c.count, 3);
}
} // namespace sruby
