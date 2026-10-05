#include "doctest/doctest.h"
// violates our requirements, thus has to go first
#include "common/FileOps.h"
#include "common/Levenstein.h"
#include "common/UIntSet.h"
#include "common/UIntSetForEach.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "spdlog/spdlog.h"

#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>

namespace sorbet::common {

namespace {

class TemporaryDirectory final {
public:
    TemporaryDirectory() {
        static std::atomic<unsigned int> nextId = 0;
        const auto tempPath = std::filesystem::temp_directory_path();
        for (;;) {
            root = tempPath / ("sorbet-common-test-" + std::to_string(nextId.fetch_add(1)));
            std::error_code error;
            if (std::filesystem::create_directory(root, error)) {
                return;
            }
            REQUIRE_FALSE(error);
        }
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(root, error);
    }

    std::filesystem::path root;
};

void writeFile(const std::filesystem::path &path, std::string_view contents = "") {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path);
    REQUIRE(output.good());
    output << contents;
    REQUIRE(output.good());
}

std::vector<std::string> discoverFiles(const std::filesystem::path &root, int workerCount, bool recursive = true,
                                       std::vector<std::string> absoluteIgnores = {},
                                       std::vector<std::string> relativeIgnores = {}) {
    auto logger = spdlog::default_logger();
    auto workers = WorkerPool::create(workerCount, *logger);
    return FileOps::listFilesInDir(root.string(), {".rb", ".rbi"}, *workers, recursive, absoluteIgnores,
                                   relativeIgnores);
}

} // namespace

TEST_CASE("Levenstein") {
    Levenstein levenstein;
    CHECK_EQ(2, levenstein.distance("Mama", "Papa", 10));
    CHECK_EQ(5, levenstein.distance("Ruby", "Scala", 10));
    CHECK_EQ(3, levenstein.distance("Java", "Scala", 10));
    CHECK_EQ(INT_MAX, levenstein.distance("Java", "S", 1));
}

TEST_CASE("FileOps::ensureDir") {
    if (FileOps::dirExists("common_test_dir")) {
        FileOps::removeDir("common_test_dir");
    }

    CHECK(FileOps::ensureDir("common_test_dir"));
    CHECK_FALSE(FileOps::ensureDir("common_test_dir"));

    FileOps::removeDir("common_test_dir");
}

TEST_CASE("FileOps::listFilesInDir") {
    auto logger = spdlog::default_logger();
    auto workers = WorkerPool::create(0, *logger);

    SUBCASE("lists files recursively") {
        const std::string root = "common_test_list_dir";
        std::filesystem::remove_all(root);
        FileOps::ensureDir(root);
        FileOps::ensureDir(root + "/nested");
        FileOps::write(root + "/a.rb", "");
        FileOps::write(root + "/nested/b.rb", "");
        FileOps::write(root + "/nested/c.txt", "");

        auto files = FileOps::listFilesInDir(root, {".rb"}, *workers, true, {}, {});
        CHECK_EQ(files, std::vector<std::string>{root + "/a.rb", root + "/nested/b.rb"});

        std::filesystem::remove_all(root);
    }

    SUBCASE("a missing root names itself in the error") {
        const std::string root = "common_test_missing_dir";
        std::filesystem::remove_all(root);

        try {
            FileOps::listFilesInDir(root, {".rb"}, *workers, true, {}, {});
            FAIL("expected listFilesInDir to throw");
        } catch (FileNotFoundException &e) {
            CHECK(std::string(e.what()).find(root) != std::string::npos);
        }
    }

    SUBCASE("a root that is not a directory still raises") {
        const std::string root = "common_test_not_a_dir.rb";
        std::filesystem::remove_all(root);
        FileOps::write(root, "");

        CHECK_THROWS_AS(FileOps::listFilesInDir(root, {".rb"}, *workers, true, {}, {}), FileNotDirException);

        std::filesystem::remove_all(root);
    }
}

TEST_CASE("FileOps follows nested directory symlinks and preserves symlink roots") {
    TemporaryDirectory temp;
    const auto root = temp.root / "project";
    const auto external = temp.root / "external-rbi";
    writeFile(root / "lib" / "local.rb");
    writeFile(external / "nested" / "shared.rb");
    writeFile(external / "nested" / "types.rbi");
    writeFile(external / "nested" / "ignored.txt");
    std::filesystem::create_directory_symlink(external, root / "lib" / "linked");

    const std::vector<std::string> linkedFiles = {(root / "lib" / "linked" / "nested" / "shared.rb").string(),
                                                  (root / "lib" / "linked" / "nested" / "types.rbi").string(),
                                                  (root / "lib" / "local.rb").string()};
    for (const int workerCount : {0, 4}) {
        INFO("worker count: " << workerCount);
        CHECK_EQ(discoverFiles(root, workerCount), linkedFiles);
    }

    const auto physicalRoot = temp.root / "physical-root";
    const auto lexicalRoot = temp.root / "root-link";
    writeFile(physicalRoot / "source.rbi");
    std::filesystem::create_directory_symlink(physicalRoot, lexicalRoot);
    const std::vector<std::string> lexicalRootFiles = {(lexicalRoot / "source.rbi").string()};
    for (const int workerCount : {0, 4}) {
        INFO("worker count: " << workerCount);
        CHECK_EQ(discoverFiles(lexicalRoot, workerCount), lexicalRootFiles);
    }
}

TEST_CASE("FileOps prunes ancestor cycles but preserves lexical aliases") {
    TemporaryDirectory temp;
    const auto root = temp.root / "project";
    writeFile(root / "base.rb");
    writeFile(root / "nested" / "inside.rb");
    writeFile(root / "nested" / "deeper" / "deep.rb");
    writeFile(root / "shared" / "shared.rb");
    writeFile(root / "mutual" / "first" / "first.rb");
    writeFile(root / "mutual" / "second" / "second.rb");

    std::filesystem::create_directory_symlink(".", root / "self");
    std::filesystem::create_directory_symlink("..", root / "nested" / "parent");
    std::filesystem::create_directory_symlink("..", root / "nested" / "deeper" / "parent");
    std::filesystem::create_directory_symlink(root, root / "nested" / "deeper" / "absolute-root");
    std::filesystem::create_directory_symlink("../..", root / "nested" / "deeper" / "dotdot-root");
    std::filesystem::create_directory_symlink("nested/../nested", root / "nested-alias");
    std::filesystem::create_directory_symlink("shared", root / "alias-one");
    std::filesystem::create_directory_symlink("shared", root / "alias-two");
    std::filesystem::create_directory_symlink("../second", root / "mutual" / "first" / "to-second");
    std::filesystem::create_directory_symlink("../first", root / "mutual" / "second" / "to-first");

    const std::vector<std::string> expected = {
        (root / "alias-one" / "shared.rb").string(),
        (root / "alias-two" / "shared.rb").string(),
        (root / "base.rb").string(),
        (root / "mutual" / "first" / "first.rb").string(),
        (root / "mutual" / "first" / "to-second" / "second.rb").string(),
        (root / "mutual" / "second" / "second.rb").string(),
        (root / "mutual" / "second" / "to-first" / "first.rb").string(),
        (root / "nested-alias" / "deeper" / "deep.rb").string(),
        (root / "nested-alias" / "inside.rb").string(),
        (root / "nested" / "deeper" / "deep.rb").string(),
        (root / "nested" / "inside.rb").string(),
        (root / "shared" / "shared.rb").string(),
    };
    for (const int workerCount : {0, 4}) {
        INFO("worker count: " << workerCount);
        CHECK_EQ(discoverFiles(root, workerCount), expected);
    }
}

TEST_CASE("FileOps applies ignores to lexical routes and keeps leaf symlinks non-recursively") {
    TemporaryDirectory temp;
    const auto root = temp.root / "project";
    const auto external = temp.root / "external";
    writeFile(root / "local.rb");
    writeFile(root / "ignored" / "hidden.rb");
    writeFile(root / "nested" / "nested.rb");
    writeFile(external / "target.rb");
    std::filesystem::create_directory_symlink(external, root / "links");
    std::filesystem::create_symlink(external / "target.rb", root / "file-link.rb");
    std::filesystem::create_symlink(root / "missing.rb", root / "dangling.rb");

    const std::vector<std::string> withoutLinks = {
        (root / "dangling.rb").string(), (root / "file-link.rb").string(), (root / "ignored" / "hidden.rb").string(),
        (root / "local.rb").string(), (root / "nested" / "nested.rb").string()};
    const std::vector<std::string> withLinkedTarget = {(root / "dangling.rb").string(),
                                                       (root / "file-link.rb").string(),
                                                       (root / "ignored" / "hidden.rb").string(),
                                                       (root / "links" / "target.rb").string(),
                                                       (root / "local.rb").string(),
                                                       (root / "nested" / "nested.rb").string()};

    for (const int workerCount : {0, 4}) {
        INFO("worker count: " << workerCount);
        CHECK_EQ(discoverFiles(root, workerCount, true, {"/links"}), withoutLinks);
        CHECK_EQ(discoverFiles(root, workerCount, true, {}, {"/links"}), withoutLinks);
        CHECK_EQ(discoverFiles(root, workerCount, true, {external.string()}), withLinkedTarget);
        CHECK_EQ(discoverFiles(root, workerCount, false),
                 std::vector<std::string>{(root / "dangling.rb").string(), (root / "file-link.rb").string(),
                                          (root / "local.rb").string()});
    }
}

TEST_CASE("FileOps skips nested ELOOP entries and preserves root errors") {
    TemporaryDirectory temp;
    const auto root = temp.root / "project";
    writeFile(root / "before.rb");
    std::filesystem::create_directory_symlink("b", root / "a");
    std::filesystem::create_directory_symlink("a", root / "b");

    const std::vector<std::string> expected = {(root / "before.rb").string()};
    const auto missingRoot = temp.root / "missing";
    const auto notDirectory = temp.root / "not-directory.rb";
    writeFile(notDirectory);
    for (const int workerCount : {0, 4}) {
        INFO("worker count: " << workerCount);
        CHECK_EQ(discoverFiles(root, workerCount), expected);

        try {
            discoverFiles(root / "a", workerCount);
            FAIL("expected a symlink loop supplied as a root to fail");
        } catch (FileNotFoundException &e) {
            CHECK(std::string(e.what()).find((root / "a").string()) != std::string::npos);
        }

        try {
            discoverFiles(missingRoot, workerCount);
            FAIL("expected a missing root to fail");
        } catch (FileNotFoundException &e) {
            CHECK(std::string(e.what()).find(missingRoot.string()) != std::string::npos);
        }
        CHECK_THROWS_AS(discoverFiles(notDirectory, workerCount), FileNotDirException);
    }
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

} // namespace sorbet::common
