#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "class_flatten/class_flatten.h"
#include "common/common.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Names.h"
#include "core/Unfreeze.h"
#include "infer/infer.h"
#include "local_vars/local_vars.h"
#include "namer/namer.h"
#include "resolver/resolver.h"
#include "rewriter/rewriter.h"
#include "spdlog/spdlog.h"
// has to come before the next one. This comment stops formatter from reordering them
#include "spdlog/sinks/stdout_color_sinks.h"
#include <fstream>
#include <memory>

using namespace std;

namespace sorbet::infer::test {

auto logger = spdlog::stderr_color_mt("infer_test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

void processSource(core::GlobalState &cb, string str) {
    sorbet::core::UnfreezeNameTable nt(cb);
    sorbet::core::UnfreezeSymbolTable st(cb);
    sorbet::core::UnfreezeFileTable ft(cb);
    core::FileRef fileId = cb.enterFile("<test>", str);
    auto settings = parser::Parser::Settings{};
    auto ast = parser::Parser::run(cb, fileId, settings);
    sorbet::core::MutableContext ctx(cb, core::Symbols::root(), fileId);
    auto tree = ast::ParsedFile{ast::desugar::node2Tree(ctx, move(ast)), fileId};
    tree = local_vars::LocalVars::run(ctx, move(tree));
    tree.tree = rewriter::Rewriter::run(ctx, move(tree.tree));
    vector<ast::ParsedFile> trees;
    trees.emplace_back(move(tree));
    auto workers = WorkerPool::create(0, *logger);
    core::FoundDefHashes foundHashes; // compute this just for test coverage
    auto cancelled = namer::Namer::run(cb, absl::Span<ast::ParsedFile>(trees), *workers, &foundHashes);
    ENFORCE(!cancelled);
    auto resolved = resolver::Resolver::run(cb, move(trees), *workers);
    for (auto &tree : resolved.result()) {
        sorbet::core::MutableContext ctx(cb, core::Symbols::root(), tree.file);
        tree = class_flatten::runOne(ctx, move(tree));
    }
}

TEST_CASE("Infer") {
    core::GlobalState gs(errorQueue);
    gs.initEmpty();

    SUBCASE("LiteralsSubtyping") {
        auto intLit = core::make_type<core::IntegerLiteralType>(int64_t(1));
        auto intClass = core::make_type<core::ClassType>(core::Symbols::Integer());
        auto floatLit = core::make_type<core::FloatLiteralType>(1.0f);
        auto floatClass = core::make_type<core::ClassType>(core::Symbols::Float());
        auto trueLit = core::Types::trueClass();
        auto trueClass = core::make_type<core::ClassType>(core::Symbols::TrueClass());
        auto stringLit = core::make_type<core::NamedLiteralType>(core::Symbols::String(), core::Names::assignTemp());
        auto stringClass = core::make_type<core::ClassType>(core::Symbols::String());
        REQUIRE(core::Types::isSubType(gs, intLit, intClass));
        REQUIRE(core::Types::isSubType(gs, floatLit, floatClass));
        REQUIRE(core::Types::isSubType(gs, trueLit, trueClass));
        REQUIRE(core::Types::isSubType(gs, stringLit, stringClass));

        REQUIRE(core::Types::isSubType(gs, intLit, intLit));
        REQUIRE(core::Types::isSubType(gs, floatLit, floatLit));
        REQUIRE(core::Types::isSubType(gs, trueLit, trueLit));
        REQUIRE(core::Types::isSubType(gs, stringLit, stringLit));

        REQUIRE_FALSE(core::Types::isSubType(gs, intClass, intLit));
        REQUIRE(core::Types::isSubType(gs, core::Types::top(), core::Types::untypedUntracked()));
        REQUIRE(core::Types::isSubType(gs, core::Types::untypedUntracked(), core::Types::top()));
    }

    SUBCASE("ClassesSubtyping") {
        processSource(gs, "class Bar; end; class Foo < Bar; end");
        const auto &rootScope = core::Symbols::root().data(gs);

        auto barSymbol = rootScope->findMember(gs, gs.enterNameConstant("Bar"));
        auto fooSymbol = rootScope->findMember(gs, gs.enterNameConstant("Foo"));
        REQUIRE_EQ("<C <U Bar>>", barSymbol.name(gs).showRaw(gs));
        REQUIRE_EQ("<C <U Foo>>", fooSymbol.name(gs).showRaw(gs));

        auto barType = core::make_type<core::ClassType>(barSymbol.asClassOrModuleRef());
        auto fooType = core::make_type<core::ClassType>(fooSymbol.asClassOrModuleRef());

        REQUIRE(core::Types::isSubType(gs, fooType, barType));
        REQUIRE(core::Types::isSubType(gs, fooType, fooType));
        REQUIRE(core::Types::isSubType(gs, barType, barType));
        REQUIRE_FALSE(core::Types::isSubType(gs, barType, fooType));
    }

    SUBCASE("ClassesLubs") {
        processSource(gs, "class Bar; end; class Foo1 < Bar; end; class Foo2 < Bar;  end");
        const auto &rootScope = core::Symbols::root().data(gs);

        auto barSymbol = rootScope->findMember(gs, gs.enterNameConstant("Bar"));
        auto foo1Symbol = rootScope->findMember(gs, gs.enterNameConstant("Foo1"));
        auto foo2Symbol = rootScope->findMember(gs, gs.enterNameConstant("Foo2"));
        REQUIRE_EQ("<C <U Bar>>", barSymbol.name(gs).showRaw(gs));
        REQUIRE_EQ("<C <U Foo1>>", foo1Symbol.name(gs).showRaw(gs));
        REQUIRE_EQ("<C <U Foo2>>", foo2Symbol.name(gs).showRaw(gs));

        auto barType = core::make_type<core::ClassType>(barSymbol.asClassOrModuleRef());
        auto foo1Type = core::make_type<core::ClassType>(foo1Symbol.asClassOrModuleRef());
        auto foo2Type = core::make_type<core::ClassType>(foo2Symbol.asClassOrModuleRef());

        auto barNfoo1 = core::Types::any(gs, barType, foo1Type);
        auto foo1Nbar = core::Types::any(gs, foo1Type, barType);
        auto barNfoo2 = core::Types::any(gs, barType, foo2Type);
        auto foo2Nbar = core::Types::any(gs, foo2Type, barType);
        auto foo1Nfoo2 = core::Types::any(gs, foo1Type, foo2Type);
        auto foo2Nfoo1 = core::Types::any(gs, foo2Type, foo1Type);

        REQUIRE_EQ("ClassType", barNfoo1.typeName());
        REQUIRE(core::Types::isSubType(gs, barType, barNfoo1));
        REQUIRE(core::Types::isSubType(gs, foo1Type, barNfoo1));
        REQUIRE_EQ("ClassType", barNfoo2.typeName());
        REQUIRE(core::Types::isSubType(gs, barType, barNfoo2));
        REQUIRE(core::Types::isSubType(gs, foo2Type, barNfoo2));
        REQUIRE_EQ("ClassType", foo1Nbar.typeName());
        REQUIRE(core::Types::isSubType(gs, barType, foo1Nbar));
        REQUIRE(core::Types::isSubType(gs, foo1Type, foo1Nbar));
        REQUIRE_EQ("ClassType", foo2Nbar.typeName());
        REQUIRE(core::Types::isSubType(gs, barType, foo2Nbar));
        REQUIRE(core::Types::isSubType(gs, foo2Type, foo2Nbar));

        REQUIRE(core::Types::equiv(gs, barNfoo2, foo2Nbar));
        REQUIRE(core::Types::equiv(gs, barNfoo1, foo1Nbar));
        REQUIRE(core::Types::equiv(gs, foo1Nfoo2, foo2Nfoo1));

        auto intType = core::make_type<core::ClassType>(core::Symbols::Integer());
        auto intNfoo1 = core::Types::any(gs, foo1Type, intType);
        auto intNbar = core::Types::any(gs, barType, intType);
        auto intNfoo1Nbar = core::Types::any(gs, intNfoo1, barType);
        REQUIRE(core::Types::equiv(gs, intNfoo1Nbar, intNbar));
        auto intNfoo1Nfoo2 = core::Types::any(gs, intNfoo1, foo2Type);
        auto intNfoo1Nfoo2Nbar = core::Types::any(gs, intNfoo1Nfoo2, barType);
        REQUIRE(core::Types::equiv(gs, intNfoo1Nfoo2Nbar, intNbar));
    }

    SUBCASE("ClassesGlbs") {
        processSource(gs, "class Bar; end; class Foo1 < Bar; end; class Foo2 < Bar;  end");
        const auto &rootScope = core::Symbols::root().data(gs);

        auto barSymbol = rootScope->findMember(gs, gs.enterNameConstant("Bar"));
        auto foo1Symbol = rootScope->findMember(gs, gs.enterNameConstant("Foo1"));
        auto foo2Symbol = rootScope->findMember(gs, gs.enterNameConstant("Foo2"));
        REQUIRE_EQ("<C <U Bar>>", barSymbol.name(gs).showRaw(gs));
        REQUIRE_EQ("<C <U Foo1>>", foo1Symbol.name(gs).showRaw(gs));
        REQUIRE_EQ("<C <U Foo2>>", foo2Symbol.name(gs).showRaw(gs));

        auto barType = core::make_type<core::ClassType>(barSymbol.asClassOrModuleRef());
        auto foo1Type = core::make_type<core::ClassType>(foo1Symbol.asClassOrModuleRef());
        auto foo2Type = core::make_type<core::ClassType>(foo2Symbol.asClassOrModuleRef());

        auto barOrfoo1 = core::Types::all(gs, barType, foo1Type);
        auto foo1Orbar = core::Types::all(gs, foo1Type, barType);
        auto barOrfoo2 = core::Types::all(gs, barType, foo2Type);
        auto foo2Orbar = core::Types::all(gs, foo2Type, barType);
        auto foo1Orfoo2 = core::Types::all(gs, foo1Type, foo2Type);
        auto foo2Orfoo1 = core::Types::all(gs, foo2Type, foo1Type);

        REQUIRE_EQ("ClassType", barOrfoo1.typeName());
        REQUIRE(core::Types::isSubType(gs, barOrfoo1, barType));
        REQUIRE(core::Types::isSubType(gs, barOrfoo1, foo1Type));
        REQUIRE_EQ("ClassType", barOrfoo2.typeName());
        REQUIRE(core::Types::isSubType(gs, barOrfoo2, barType));
        REQUIRE(core::Types::isSubType(gs, barOrfoo2, foo2Type));
        REQUIRE_EQ("ClassType", foo1Orbar.typeName());
        REQUIRE(core::Types::isSubType(gs, foo1Orbar, barType));
        REQUIRE(core::Types::isSubType(gs, foo1Orbar, foo1Type));
        REQUIRE_EQ("ClassType", foo2Orbar.typeName());
        REQUIRE(core::Types::isSubType(gs, foo2Orbar, barType));
        REQUIRE(core::Types::isSubType(gs, foo2Orbar, foo2Type));

        REQUIRE(core::Types::equiv(gs, barOrfoo2, foo2Orbar));
        REQUIRE(core::Types::equiv(gs, barOrfoo1, foo1Orbar));
        REQUIRE(core::Types::equiv(gs, foo1Orfoo2, foo2Orfoo1));
    }
}

} // namespace sorbet::infer::test
