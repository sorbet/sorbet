#include "doctest.h"
// has to go first as it violates our requirements
#include "core/Error.h"
#include "core/ErrorCollector.h"
#include "core/ErrorQueue.h"
#include "core/GlobalSubstitution.h"
#include "core/TypePtr.h"
#include "core/Unfreeze.h"
#include "core/core.h"
#include "core/errors/internal.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace spd = spdlog;
using namespace std;

namespace sorbet::core {
auto logger = spd::stderr_color_mt("parse");
auto errorCollector = make_shared<core::ErrorCollector>();
auto errorQueue = make_shared<ErrorQueue>(*logger, *logger, errorCollector);

struct Offset2PosTest {
    string src;
    u4 off;
    u4 line;
    u4 col;
};

TEST_CASE("TestOffset2Pos") {
    GlobalState gs(errorQueue);
    gs.initEmpty();
    UnfreezeFileTable fileTableAccess(gs);

    vector<Offset2PosTest> cases = {{"hello", 0, 1, 1},
                                    {"line 1\nline 2", 1, 1, 2},
                                    {"line 1\nline 2", 7, 2, 1},
                                    {"line 1\nline 2", 11, 2, 5},
                                    {"a long line with no newlines\n", 20, 1, 21},
                                    {"line 1\nline 2\nline3\n", 7, 2, 1},
                                    {"line 1\nline 2\nline3", 6, 1, 7},
                                    {"line 1\nline 2\nline3", 7, 2, 1}};
    int i = 0;
    for (auto &tc : cases) {
        auto name = string("case: ") + to_string(i);
        INFO(name);
        FileRef f = gs.enterFile(move(name), tc.src);

        auto detail = Loc::offset2Pos(f.data(gs), tc.off);

        CHECK_EQ(tc.col, detail.column);
        CHECK_EQ(tc.line, detail.line);
        i++;
    }
}

TEST_CASE("Errors") {
    GlobalState gs(errorQueue);
    gs.initEmpty();
    UnfreezeFileTable fileTableAccess(gs);
    FileRef f = gs.enterFile(string("a/foo.rb"), string("def foo\n  hi\nend\n"));
    if (auto e = gs.beginError(Loc{f, 0, 3}, errors::Internal::InternalError)) {
        e.setHeader("Use of metavariable: `{}`", "foo");
    }
    gs.errorQueue->flushAllErrors(gs);
    REQUIRE(gs.hadCriticalError());
    REQUIRE_EQ(1, errorCollector->drainErrors().size());
}

TEST_CASE("SymbolRef") {
    GlobalState gs(errorQueue);
    gs.initEmpty();
    SymbolRef ref = Symbols::Object();
    CHECK_EQ(ref, ref.data(gs)->ref(gs));
}

struct FileIsTypedCase {
    string_view src;
    StrictLevel strict;
};

TEST_CASE("FileIsTyped") { // NOLINT
    vector<FileIsTypedCase> cases = {
        {"", StrictLevel::None},
        {"# typed: true", StrictLevel::True},
        {"\n# typed: true\n", StrictLevel::True},
        {"not a typed: sigil\n# typed: true\n", StrictLevel::True},
        {"typed:\n# typed: nonsense\n", StrictLevel::None},
        {"# typed: strict\n", StrictLevel::Strict},
        {"# typed: strong\n", StrictLevel::Strong},
        {"# typed: autogenerated\n", StrictLevel::Autogenerated},
        {"# typed: false\n", StrictLevel::False},
        {"# typed: lax\n", StrictLevel::None},
        {"# typed: ignore\n", StrictLevel::Ignore},
        {"#    typed:      true\n", StrictLevel::True},
        {"typed: true\n", StrictLevel::None},

        // We no longer support the old sigil
        {"# @typed", StrictLevel::None},
        {"\n# @typed\n", StrictLevel::None},
    };
    for (auto &tc : cases) {
        CHECK_EQ(tc.strict, File::fileStrictSigil(tc.src));
    }
}

TEST_CASE("Substitute") { // NOLINT
    GlobalState gs1(errorQueue);
    gs1.initEmpty();

    GlobalState gs2(errorQueue);
    gs2.initEmpty();

    NameRef foo1, bar1, other1, cnstBaz1, uniqueBaz1, otherCnstBart1;
    NameRef foo2, bar2, cnstBaz2, uniqueBaz2;
    {
        UnfreezeNameTable thaw1(gs1);
        UnfreezeNameTable thaw2(gs2);

        foo1 = gs1.enterNameUTF8("foo");
        bar1 = gs1.enterNameUTF8("bar");
        other1 = gs1.enterNameUTF8("other");
        cnstBaz1 = gs1.enterNameConstant("Baz");
        uniqueBaz1 = gs1.freshNameUnique(UniqueNameKind::Namer, cnstBaz1, 1);
        otherCnstBart1 = gs1.enterNameConstant("Bart");

        foo2 = gs2.enterNameUTF8("foo");
        bar2 = gs2.enterNameUTF8("bar");
        cnstBaz2 = gs2.enterNameConstant("Baz");
        uniqueBaz2 = gs2.freshNameUnique(UniqueNameKind::Namer, cnstBaz1, 1);
        gs1.enterNameUTF8("name");
    }

    GlobalSubstitution subst(gs1, gs2);

    CHECK_EQ(subst.substitute(foo1), foo2);
    CHECK_EQ(subst.substitute(bar1), bar2);
    CHECK_EQ(subst.substitute(uniqueBaz1), uniqueBaz2);
    CHECK_EQ(subst.substitute(cnstBaz1), cnstBaz2);

    auto other2 = subst.substitute(other1);
    REQUIRE(other2.exists());
    REQUIRE(other2.kind() == NameKind::UTF8);
    REQUIRE_EQ("<U other>", other2.showRaw(gs2));
}

// Privileged class that is friends with TypePtr
class TypePtrTestHelper {
public:
    static std::atomic<u4> *counter(const TypePtr &ptr) {
        CHECK(ptr.containsPtr());
        return ptr.counter;
    }

    static u8 value(const TypePtr &ptr) {
        CHECK(!ptr.containsPtr());
        return ptr.value;
    }

    static u4 inlinedValue(const TypePtr &ptr) {
        CHECK(!ptr.containsPtr());
        return ptr.inlinedValue();
    }

    static TypePtr::tagged_storage store(const TypePtr &ptr) {
        return ptr.store;
    }

    static void *get(const TypePtr &ptr) {
        CHECK(ptr.containsPtr());
        return ptr.get();
    }

    static TypePtr create(TypePtr::Tag tag, void *type) {
        return TypePtr(tag, type);
    }

    static TypePtr createInlined(TypePtr::Tag tag, u4 inlinedValue, u8 value) {
        return TypePtr(tag, inlinedValue, value);
    }
};

TEST_SUITE("TypePtr") {
    TEST_CASE("Does not allocate a counter for null type") {
        TypePtr ptr;
        CHECK_EQ(0, TypePtrTestHelper::value(ptr));
    }

    TEST_CASE("Properly manages counter") {
        auto ptr = make_type<UnresolvedClassType>(Symbols::untyped(), vector<NameRef>{});
        auto counter = TypePtrTestHelper::counter(ptr);
        REQUIRE_NE(nullptr, counter);
        CHECK_EQ(1, counter->load());

        {
            // Copy should increment counter
            TypePtr ptrCopy(ptr);
            REQUIRE_EQ(counter, TypePtrTestHelper::counter(ptrCopy));
            CHECK_EQ(2, counter->load());
        }

        // Destruction of copy should decrement counter
        CHECK_EQ(1, counter->load());

        {
            TypePtr ptrCopy(ptr);
            // Assigning/overwriting should decrement counter
            ptr = TypePtr();
            CHECK_EQ(1, counter->load());

            // Moving should keep counter the same
            ptr = move(ptrCopy);
            CHECK_EQ(1, counter->load());

            // Moving should clear counter from ptrCopy and make it an empty TypePtr
            CHECK_EQ(0, TypePtrTestHelper::value(ptrCopy));
            CHECK_EQ(0, TypePtrTestHelper::store(ptrCopy));
            CHECK_EQ(TypePtr(), ptrCopy);

            // Assigning to nullptr should increment counter (and not try to increment the null counter field in
            // ptrCopy)
            ptrCopy = ptr;
            CHECK_EQ(2, counter->load());
        }
        CHECK_EQ(1, counter->load());
    }

    TEST_CASE("Tagging works as expected") {
        // This tag is < 8. Will be deleted / managed by TypePtr.
        {
            auto rawPtr = new SelfType();
            auto ptr = TypePtrTestHelper::create(TypePtr::Tag::SelfType, rawPtr);
            CHECK_EQ(TypePtr::Tag::SelfType, ptr.tag());
            CHECK_EQ(rawPtr, TypePtrTestHelper::get(ptr));
        }

        // This tag is > 8
        {
            auto rawPtr = new UnresolvedClassType(Symbols::untyped(), {});
            auto ptr = TypePtrTestHelper::create(TypePtr::Tag::UnresolvedClassType, rawPtr);
            CHECK_EQ(TypePtr::Tag::UnresolvedClassType, ptr.tag());
            CHECK_EQ(rawPtr, TypePtrTestHelper::get(ptr));
        }
    }

    TEST_CASE("Supports inlined values") {
        // Let's try edge cases.
        std::list<pair<u4, u8>> valuesArray = {
            {0, 0},
            {1, 1},
            {0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF},
        };

        for (auto values : valuesArray) {
            SUBCASE(fmt::format("{}, {}", values.first, values.second).c_str()) {
                auto type = TypePtrTestHelper::createInlined(TypePtr::Tag::SelfType, values.first, values.second);
                CHECK_EQ(TypePtr::Tag::SelfType, type.tag());
                CHECK_EQ(values.first, TypePtrTestHelper::inlinedValue(type));
                CHECK_EQ(values.second, TypePtrTestHelper::value(type));
            }
        }
    }
}

} // namespace sorbet::core
