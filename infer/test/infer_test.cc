#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Names.h"
#include "core/Unfreeze.h"
#include "infer/environment.h"
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
    auto ast = parser::Parser::run(cb, fileId, settings).tree;
    sorbet::core::MutableContext ctx(cb, core::Symbols::root(), fileId);
    auto tree = ast::ParsedFile{ast::desugar::node2Tree(ctx, move(ast)), fileId};
    tree.tree = rewriter::Rewriter::run(ctx, move(tree.tree));
    tree = local_vars::LocalVars::run(ctx, move(tree));
    vector<ast::ParsedFile> trees;
    trees.emplace_back(move(tree));
    auto workers = WorkerPool::create(0, *logger);
    core::FoundDefHashesResult foundHashes; // compute this just for test coverage
    auto cancelled = namer::Namer::run(cb, absl::Span<ast::ParsedFile>(trees), *workers, &foundHashes);
    ENFORCE(!cancelled);
    auto resolved = resolver::Resolver::run(cb, move(trees), *workers);
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

TEST_CASE("VariableTable") {
    const auto numLocals = 10;
    auto local = [](uint32_t id) { return cfg::LocalRef(id); };

    VariableTable table;
    table.init(numLocals, numLocals);
    CHECK_EQ(0, table.size());
    CHECK_FALSE(table.contains(local(3)));
    CHECK_EQ(nullptr, table.find(local(3)));

    SUBCASE("holds variables in insertion order, looked up by local") {
        table[local(3)].knownTruthy = true;
        table[local(7)].knownTruthy = false;
        table[local(1)].knownTruthy = true;
        CHECK_EQ(3, table.size());
        CHECK(table.contains(local(3)));
        CHECK(table.contains(local(7)));
        CHECK(table.contains(local(1)));
        CHECK_FALSE(table.contains(local(2)));
        CHECK_FALSE(table.contains(local(9)));

        // A second `operator[]` finds the same state rather than inserting another.
        table[local(3)].knownTruthy = false;
        CHECK_EQ(3, table.size());
        REQUIRE_NE(nullptr, table.find(local(3)));
        CHECK_FALSE(table.find(local(3))->knownTruthy);
        CHECK(table.find(local(1))->knownTruthy);

        std::vector<uint32_t> order;
        for (auto entry : table) {
            order.emplace_back(entry.local.id());
        }
        CHECK_EQ(std::vector<uint32_t>{3, 7, 1}, order);
    }

    SUBCASE("references to states stay valid across insertions") {
        auto &first = table[local(0)];
        for (uint32_t id = 1; id < numLocals; id++) {
            table[local(id)];
        }
        first.knownTruthy = true;
        CHECK_EQ(numLocals, table.size());
        CHECK(table.find(local(0))->knownTruthy);
        CHECK_EQ(&first, table.find(local(0)));
    }

    SUBCASE("a copy shares states until one side writes") {
        table[local(2)].knownTruthy = true;
        VariableTable copy = table;
        CHECK_EQ(table.find(local(2)), copy.find(local(2)));

        copy[local(2)].knownTruthy = false;
        copy[local(5)];
        CHECK_NE(table.find(local(2)), copy.find(local(2)));
        CHECK(table.find(local(2))->knownTruthy);
        CHECK_FALSE(copy.find(local(2))->knownTruthy);
        CHECK_EQ(1, table.size());
        CHECK_EQ(2, copy.size());
    }

    SUBCASE("a variable held without a state reads as absent until written") {
        table.insert(local(6));
        CHECK(table.contains(local(6)));
        CHECK_EQ(nullptr, table.find(local(6)));
        REQUIRE_NE(nullptr, table.findRef(local(6)));
        CHECK(table.findRef(local(6))->isNull());

        table.findMutable(local(6))->knownTruthy = true;
        REQUIRE_NE(nullptr, table.find(local(6)));
        CHECK(table.find(local(6))->knownTruthy);
    }

    SUBCASE("init empties the table and release frees it") {
        table[local(4)];
        table.init(numLocals, numLocals);
        CHECK_EQ(0, table.size());
        CHECK_FALSE(table.contains(local(4)));
        table[local(4)];
        table.release();
        CHECK_EQ(0, table.size());
        CHECK_FALSE(table.contains(local(4)));
    }
}

} // namespace sorbet::infer::test
