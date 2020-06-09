#include "doctest.h"
#include <cxxopts.hpp>
// has to go first as it violates our requirements

#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/common.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/GlobalSubstitution.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

namespace spd = spdlog;

auto logger = spd::stderr_color_mt("hello-test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

namespace sorbet {

namespace spd = spdlog;

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
        ast::TreePtr preTransformClassDef(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }
        ast::TreePtr preTransformMethodDef(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformIf(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformWhile(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr postTransformBreak(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr postTransformNext(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformReturn(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformRescue(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr postTransformConstantLit(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformAssign(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformSend(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformHash(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformArray(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr postTransformLiteral(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr postTransformUnresolvedConstantLit(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformBlock(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
        }

        ast::TreePtr preTransformInsSeq(core::MutableContext ctx, ast::TreePtr original) {
            count++;
            return original;
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
    cb.enterTypeArgument(loc, methodSym, cb.enterNameConstant(name), sorbet::core::Variance::CoVariant);

    auto empty = vector<core::SymbolRef>();
    auto argumentSym = core::LocalVariable(name, 0);
    auto rhs(ast::MK::Int(loc.offsets(), 5));
    auto arg = ast::make_tree<ast::Local>(loc.offsets(), argumentSym);
    ast::MethodDef::ARGS_store args;
    args.emplace_back(std::move(arg));

    ast::MethodDef::Flags flags;
    auto methodDef =
        ast::make_tree<ast::MethodDef>(loc.offsets(), loc, methodSym, name, std::move(args), std::move(rhs), flags);
    auto emptyTree = ast::MK::EmptyTree();
    auto cnst = ast::make_tree<ast::UnresolvedConstantLit>(loc.offsets(), std::move(emptyTree), name);

    ast::ClassDef::RHS_store classrhs;
    classrhs.emplace_back(std::move(methodDef));
    auto tree =
        ast::make_tree<ast::ClassDef>(loc.offsets(), loc, classSym, std::move(cnst), ast::ClassDef::ANCESTORS_store(),
                                      std::move(classrhs), ast::ClassDef::Kind::Class);
    Counter c;
    sorbet::core::MutableContext ctx(cb, core::Symbols::root(), loc.file());

    auto r = ast::TreeMap::apply(ctx, c, std::move(tree));
    CHECK_EQ(c.count, 3);
}

TEST_CASE("CloneSubstitutePayload") {
    auto logger = spd::stderr_color_mt("ClonePayload");
    auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

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
    REQUIRE_EQ("<U test new name>", subst.substitute(n1).showRaw(*c2));
    REQUIRE_EQ(c1->symbolsUsed(), c2->symbolsUsed());
    REQUIRE_EQ(c1->symbolsUsed(), gs.symbolsUsed());
}
} // namespace sorbet
