#include "doctest/doctest.h"
// violates our requirements, thus has to go first
#include "absl/algorithm/container.h"
#include "common/FileOps.h"
#include "common/Levenstein.h"
#include "common/SparseUIntSet.h"
#include "common/UIntSet.h"
#include "common/UIntSetForEach.h"
#include "common/common.h"
#include "common/concurrency/Parallel.h"
#include "common/concurrency/WorkerPool.h"
#include "spdlog/spdlog.h"
// has to come before the next one. This comment stops formatter from reordering them
#include "spdlog/sinks/stdout_color_sinks.h"

#include <array>
#include <atomic>

namespace sorbet::common {

TEST_CASE("Levenstein") {
    Levenstein levenstein;
    CHECK_EQ(2, levenstein.distance("Mama", "Papa", 10));
    CHECK_EQ(5, levenstein.distance("Ruby", "Scala", 10));
    CHECK_EQ(3, levenstein.distance("Java", "Scala", 10));
    CHECK_EQ(INT_MAX, levenstein.distance("Java", "S", 1));
}

TEST_CASE("FileOps::read") {
    const std::string path = "common_test_read.txt";
    // A file larger than the stdio buffer was, with every byte value in it.
    std::string contents;
    for (int i = 0; i < 10000; i++) {
        contents.push_back(static_cast<char>(i));
    }
    FileOps::write(path, contents);
    CHECK_EQ(contents, FileOps::read(path));

    FileOps::write(path, "");
    CHECK_EQ("", FileOps::read(path));
    FileOps::removeFile(path);

    CHECK_THROWS_AS(FileOps::read("common_test_does_not_exist.txt"), FileNotFoundException);
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

        // Multiple bits in words, not at word boundaries.
        callCount = 0;
        a.remove(0);
        a.remove(64);
        std::array<uint32_t, 7> bits = {5, 8, 13, 21, 34, 55, 89};
        for (auto bit : bits) {
            a.add(bit);
        }

        a.forEach([&bits, &callCount](uint32_t bit) -> void {
            CHECK(callCount < bits.size());
            CHECK_EQ(bits[callCount], bit);
            callCount++;
        });
        CHECK_EQ(bits.size(), callCount);

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

namespace {

// The items of a set, in the order `forEach` yields them.
template <class Set> std::vector<uint32_t> itemsOf(const Set &set) {
    std::vector<uint32_t> items;
    set.forEach([&items](uint32_t item) { items.emplace_back(item); });
    return items;
}

// A deterministic sequence of pseudo-random items below `capacity`, so that the two set representations can be put
// through the same operations and compared.
std::vector<uint32_t> pseudoRandomItems(uint32_t seed, size_t count, uint32_t capacity) {
    std::vector<uint32_t> items;
    uint32_t state = seed;
    for (size_t i = 0; i < count; i++) {
        state = state * 1664525u + 1013904223u;
        items.emplace_back((state >> 8) % capacity);
    }
    return items;
}

} // namespace

TEST_SUITE("SparseUIntSet") {
    TEST_CASE("single element") {
        SparseUIntSet set(128);
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

    TEST_CASE("elements in different words") {
        SparseUIntSet set(4096);
        set.add(4000);
        set.add(0);
        set.add(64);
        set.add(65);
        CHECK_EQ(4, set.size());
        CHECK(set.contains(0));
        CHECK(set.contains(64));
        CHECK(set.contains(65));
        CHECK(set.contains(4000));
        CHECK_FALSE(set.contains(32));
        CHECK_FALSE(set.contains(66));
        // `forEach` yields items in ascending order whatever the insertion order.
        CHECK_EQ(std::vector<uint32_t>{0, 64, 65, 4000}, itemsOf(set));

        // Removing the last item of a word drops the word; a word that is not present is left alone.
        set.remove(4000);
        set.remove(32);
        CHECK_EQ(3, set.size());
        CHECK_FALSE(set.contains(4000));
        std::vector<std::pair<uint32_t, uint32_t>> words;
        set.forEachWord([&words](uint32_t index, uint32_t bits) { words.emplace_back(index, bits); });
        CHECK_EQ(std::vector<std::pair<uint32_t, uint32_t>>{{0, 1u}, {2, 3u}}, words);
    }

    TEST_CASE("set operations agree with UIntSet") {
        const uint32_t capacity = 2048;
        for (uint32_t seed = 1; seed <= 8; seed++) {
            // Sparse: a few items far apart. Denser: many items, some shared with the sparse ones.
            auto sparseItems = pseudoRandomItems(seed, 6, capacity);
            auto denseItems = pseudoRandomItems(seed + 100, 200, capacity);
            denseItems.insert(denseItems.end(), sparseItems.begin(), sparseItems.begin() + 3);

            UIntSet denseA(capacity), denseB(capacity);
            SparseUIntSet sparseA(capacity), sparseB(capacity);
            for (auto item : sparseItems) {
                denseA.add(item);
                sparseA.add(item);
            }
            for (auto item : denseItems) {
                denseB.add(item);
                sparseB.add(item);
            }
            CHECK_EQ(itemsOf(denseA), itemsOf(sparseA));
            CHECK_EQ(itemsOf(denseB), itemsOf(sparseB));
            CHECK_EQ(denseB.size(), sparseB.size());

            for (auto [left, right] : {std::pair{0, 1}, std::pair{1, 0}, std::pair{0, 0}}) {
                const UIntSet &denseL = left == 0 ? denseA : denseB;
                const UIntSet &denseR = right == 0 ? denseA : denseB;
                const SparseUIntSet &sparseL = left == 0 ? sparseA : sparseB;
                const SparseUIntSet &sparseR = right == 0 ? sparseA : sparseB;

                UIntSet denseUnion = denseL;
                denseUnion.add(denseR);
                SparseUIntSet sparseUnion = sparseL;
                sparseUnion.add(sparseR);
                CHECK_EQ(itemsOf(denseUnion), itemsOf(sparseUnion));

                UIntSet denseDifference = denseL;
                denseDifference.remove(denseR);
                SparseUIntSet sparseDifference = sparseL;
                sparseDifference.remove(sparseR);
                CHECK_EQ(itemsOf(denseDifference), itemsOf(sparseDifference));

                UIntSet denseIntersection = denseL;
                denseIntersection.intersect(denseR);
                SparseUIntSet sparseIntersection = sparseL;
                sparseIntersection.intersect(sparseR);
                CHECK_EQ(itemsOf(denseIntersection), itemsOf(sparseIntersection));

                UIntSet denseOverwritten(capacity);
                denseOverwritten.add(capacity - 1);
                denseOverwritten.overwriteWithUnion(denseL, denseR);
                SparseUIntSet sparseOverwritten(capacity);
                sparseOverwritten.add(capacity - 1);
                sparseOverwritten.overwriteWithUnion(sparseL, sparseR);
                CHECK_EQ(itemsOf(denseOverwritten), itemsOf(sparseOverwritten));

                // The mixed-representation operations the CFG builder uses: a sparse set intersected with a dense one,
                // and a dense set receiving a sparse one's words.
                SparseUIntSet sparseIntersectedWithDense = sparseL;
                sparseIntersectedWithDense.intersectWords([&denseR](uint32_t index) { return denseR.word(index); });
                CHECK_EQ(itemsOf(denseIntersection), itemsOf(sparseIntersectedWithDense));

                UIntSet denseReceivingSparse = denseL;
                sparseR.forEachWord([&denseReceivingSparse](uint32_t index, uint32_t bits) {
                    denseReceivingSparse.addWord(index, bits);
                });
                CHECK_EQ(itemsOf(denseUnion), itemsOf(denseReceivingSparse));
            }
        }
    }

    TEST_CASE("clear") {
        SparseUIntSet set(128);
        set.add(5);
        set.add(100);
        set.clear();
        CHECK(set.empty());
        CHECK_EQ(0, set.size());
        CHECK_FALSE(set.contains(5));
    }
}

TEST_CASE("Parallel::iterateChunked") {
    auto logger = spdlog::stderr_color_mt("common_test");
    // With no workers the iteration runs on this thread; with some it runs on theirs.
    for (auto numWorkers : {0, 3}) {
        auto workers = WorkerPool::create(numWorkers, *logger);
        std::vector<int> visits(1000, 0);
        std::atomic<size_t> chunks = 0;
        std::atomic<size_t> oversizedChunks = 0;
        Parallel::iterateChunked(*workers, "iterateChunked", absl::MakeSpan(visits), 64, [&](absl::Span<int> chunk) {
            chunks++;
            if (chunk.size() > 64) {
                oversizedChunks++;
            }
            for (auto &visit : chunk) {
                visit++;
            }
        });
        // Every element is visited exactly once, in 15 chunks of 64 and one of 40.
        CHECK(absl::c_all_of(visits, [](int visit) { return visit == 1; }));
        CHECK_EQ(16, chunks.load());
        CHECK_EQ(0, oversizedChunks.load());
    }
    spdlog::drop("common_test");
}

} // namespace sorbet::common
