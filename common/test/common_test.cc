#include "doctest/doctest.h"
// violates our requirements, thus has to go first
#include "common/FileOps.h"
#include "common/Levenstein.h"
#include "common/UIntSet.h"
#include "common/UIntSetForEach.h"
#include "common/common.h"

namespace sorbet::common {

TEST_CASE("Levenstein") { // NOLINT
    CHECK_EQ(2, Levenstein::distance("Mama", "Papa", 10));
    CHECK_EQ(5, Levenstein::distance("Ruby", "Scala", 10));
    CHECK_EQ(3, Levenstein::distance("Java", "Scala", 10));
    CHECK_EQ(INT_MAX, Levenstein::distance("Java", "S", 1));
}

TEST_CASE("FileOps::ensureDir") {
    if (FileOps::dirExists("common_test_dir")) {
        FileOps::removeDir("common_test_dir");
    }

    CHECK(FileOps::ensureDir("common_test_dir"));
    CHECK_FALSE(FileOps::ensureDir("common_test_dir"));

    FileOps::removeDir("common_test_dir");
}

TEST_SUITE("UIntSet") {
    TEST_CASE("single element") {
        UIntSet set(128);
        CHECK(set.empty());
        CHECK_EQ(0, set.size());
        set.add(1);
        CHECK_FALSE(set.empty());
        CHECK_EQ(1, set.size());
        CHECK(set.contains(1));
        set.remove(1);
        CHECK(set.empty());
        CHECK_FALSE(set.contains(1));
        CHECK_EQ(0, set.size());
    }

    TEST_CASE("single element, but on a secondary integer") {
        // Try setting an element backed by a different uint32_t
        UIntSet set(128);
        set.add(32);
        CHECK_FALSE(set.empty());
        CHECK(set.contains(32));
        CHECK_EQ(1, set.size());
        set.remove(32);
        CHECK(set.empty());
        CHECK_EQ(0, set.size());
        CHECK_FALSE(set.contains(32));
        set.add(33);
        CHECK_FALSE(set.empty());
        CHECK_EQ(1, set.size());
        CHECK(set.contains(33));
        set.remove(33);
        CHECK(set.empty());
        CHECK_EQ(0, set.size());
        CHECK_FALSE(set.contains(33));
    }

    TEST_CASE("multiple elements at integer boundaries") {
        UIntSet set(128);
        // Set multiple / 0 case
        set.add(0);
        CHECK_FALSE(set.empty());
        CHECK_EQ(1, set.size());
        CHECK(set.contains(0));
        // Same bit, different backing integer...
        CHECK_FALSE(set.contains(32));
        set.add(32);
        CHECK_EQ(2, set.size());
        CHECK_FALSE(set.empty());
        CHECK(set.contains(32));
        set.remove(0);
        CHECK_FALSE(set.contains(0));
        CHECK(set.contains(32));
        CHECK_EQ(1, set.size());
        CHECK_FALSE(set.empty());
        set.remove(32);
        CHECK(set.empty());
        CHECK_EQ(0, set.size());
        CHECK_FALSE(set.contains(32));
    }

    TEST_CASE("single element in last position") {
        UIntSet set(128);
        set.add(127);
        CHECK(set.contains(127));
        set.remove(127);
        CHECK_FALSE(set.contains(127));
        CHECK(set.empty());
    }

    TEST_CASE("add set, intersection set, remove set") {
        // a and b have 2 common elements spread across multiple backing integers
        UIntSet a(128);
        a.add(0);
        a.add(1);
        a.add(2);
        a.add(64);
        a.add(65);
        a.add(66);
        CHECK_EQ(6, a.size());

        UIntSet b(128);
        b.add(0);
        b.add(3);
        b.add(4);
        b.add(64);
        b.add(67);
        b.add(68);
        CHECK_EQ(6, b.size());

        // a + b
        {
            UIntSet c = a;
            c.add(b);
            CHECK_EQ(10, c.size());
            CHECK(c.contains(0));
            CHECK(c.contains(1));
            CHECK(c.contains(2));
            CHECK(c.contains(3));
            CHECK(c.contains(4));
            CHECK(c.contains(64));
            CHECK(c.contains(65));
            CHECK(c.contains(66));
            CHECK(c.contains(67));
            CHECK(c.contains(68));
        }
        // a + a
        {
            UIntSet c = a;
            c.add(a);
            CHECK_EQ(6, c.size());
            CHECK(c.contains(0));
            CHECK(c.contains(1));
            CHECK(c.contains(2));
            CHECK(c.contains(64));
            CHECK(c.contains(65));
            CHECK(c.contains(66));
        }
        // a - a
        {
            UIntSet c = a;
            c.remove(a);
            CHECK(c.empty());
        }
        // a - b
        {
            UIntSet c = a;
            c.remove(b);
            CHECK_EQ(4, c.size());
            CHECK(c.contains(1));
            CHECK(c.contains(2));
            CHECK(c.contains(65));
            CHECK(c.contains(66));
        }
        // a intersection a
        {
            UIntSet c = a;
            c.intersect(a);
            CHECK_EQ(6, c.size());
            CHECK(c.contains(0));
            CHECK(c.contains(1));
            CHECK(c.contains(2));
            CHECK(c.contains(64));
            CHECK(c.contains(65));
            CHECK(c.contains(66));
        }
        // a intersection b
        {
            UIntSet c = a;
            c.intersect(b);
            CHECK_EQ(2, c.size());
            CHECK(c.contains(0));
            CHECK(c.contains(64));
        }
    }

    TEST_CASE("forEach") {
        UIntSet a(128);

        // Empty case
        a.forEach([](uint32_t local) -> void { FAIL("Expected forEach on an empty set to not call the lambda."); });

        // Single case
        a.add(0);
        int callCount = 0;
        a.forEach([&callCount](uint32_t local) -> void {
            CHECK_EQ(0, local);
            callCount++;
        });
        CHECK_EQ(1, callCount);

        // Multiple case
        callCount = 0;
        a.add(64);
        a.forEach([&callCount](uint32_t local) -> void {
            if (callCount == 0) {
                CHECK_EQ(0, local);
            } else if (callCount == 1) {
                CHECK_EQ(64, local);
            } else {
                FAIL("Unexpected forEach call");
            }
            callCount++;
        });
        CHECK_EQ(2, callCount);

        // Full case
        for (int i = 0; i < 128; i++) {
            a.add(i);
        }

        callCount = 0;
        a.add(64);
        a.forEach([&callCount](uint32_t local) -> void {
            CHECK_EQ(callCount, local);
            callCount++;
        });
        CHECK_EQ(128, callCount);
    }

    TEST_CASE("rounds up size to nearest 32") {
        UIntSet set(10);
        set.add(31);
        CHECK(set.contains(31));
        CHECK_EQ(1, set.size());

        UIntSet bigger(33);
        bigger.add(33);
        CHECK(bigger.contains(33));
        CHECK_EQ(1, bigger.size());
    }
}

} // namespace sorbet::common
