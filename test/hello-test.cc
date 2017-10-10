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

TEST(HelloTest, SimpleParse) {
    sruby::ast::ContextBase ctx(*console);
    sruby::parser::parse_ruby(ctx, "def hello_world; p :hello; end");
}

TEST(HelloTest, GetCXXopts) {
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
}

TEST(TreeMap, CountTrees) {
    class Counter {
    public:
        int count = 0;
        Stat *transformClassDef(ClassDef *original) {
            count++;
            return original;
        }
        Stat *transformMethodDef(MethodDef *original) {
            count++;
            return original;
        }

        Stat *transformSelfMethodDef(SelfMethodDef *original) {
            count++;
            return original;
        }

        Stat *transformSelfMethodDef(ConstDef *original) {
            count++;
            return original;
        }

        Stat *transformIf(If *original) {
            count++;
            return original;
        }

        Stat *transformWhile(While *original) {
            count++;
            return original;
        }

        Stat *transformFor(For *original) {
            count++;
            return original;
        }

        Stat *transformBreak(Break *original) {
            count++;
            return original;
        }

        Stat *transformNext(Next *original) {
            count++;
            return original;
        }

        Stat *transformReturn(Return *original) {
            count++;
            return original;
        }

        Stat *transformRescue(Rescue *original) {
            count++;
            return original;
        }

        Stat *transformIdent(Ident *original) {
            count++;
            return original;
        }

        Stat *transformAssign(Assign *original) {
            count++;
            return original;
        }

        Stat *transformSend(Send *original) {
            count++;
            return original;
        }

        Stat *transformNew(New *original) {
            count++;
            return original;
        }

        Stat *transformNamedArg(NamedArg *original) {
            count++;
            return original;
        }

        Stat *transformHash(Hash *original) {
            count++;
            return original;
        }

        Stat *transformArray(Array *original) {
            count++;
            return original;
        }

        Stat *transformFloatLit(FloatLit *original) {
            count++;
            return original;
        }

        Stat *transformIntLit(IntLit *original) {
            count++;
            return original;
        }

        Stat *transformStringLit(StringLit *original) {
            count++;
            return original;
        }

        Stat *transformConstantLit(ConstantLit *original) {
            count++;
            return original;
        }

        Stat *transformArraySplat(ArraySplat *original) {
            count++;
            return original;
        }

        Stat *transformHashSplat(HashSplat *original) {
            count++;
            return original;
        }

        Stat *transformSelf(Self *original) {
            count++;
            return original;
        }

        Stat *transformClosure(Closure *original) {
            count++;
            return original;
        }

        Stat *transformInsSeq(InsSeq *original) {
            count++;
            return original;
        }
    };

    sruby::ast::ContextBase ctx(*console);
    static const char *foo_str = "Foo";
    static UTF8Desc foo_DESC{(char *)foo_str, (int)std::strlen(foo_str)};

    auto name = ctx.enterNameUTF8(foo_DESC);
    auto classSym = ctx.getTopLevelClassSymbol(name);
    auto argTypes = std::vector<SymbolRef>{ctx.defn_top()};
    auto methodSym = ctx.enterSymbol(classSym, name, ctx.defn_top(), argTypes, true);
    auto empty = std::vector<SymbolRef>();
    auto argumentSym = ctx.enterSymbol(methodSym, name, ctx.defn_top(), empty, false);
    std::unique_ptr<Expr> rhs(new IntLit(5));
    auto args = std::vector<SymbolRef>{argumentSym};

    std::unique_ptr<Stat> classRhs(new MethodDef(methodSym, args, std::move(rhs)));

    std::unique_ptr<Stat> tree(new ClassDef(classSym, std::move(classRhs)));
    Counter c;
    auto r = TreeMap<Counter>::apply(c, std::move(tree));
    EXPECT_EQ(c.count, 3);
}
} // namespace sruby