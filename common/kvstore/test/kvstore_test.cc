#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "spdlog/spdlog.h"
// has to go above stdout_sinks.h; this comment prevents reordering.
#include "absl/strings/str_split.h" // For StripAsciiWhitespace
#include "absl/synchronization/notification.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/kvstore/CacheLock.h"
#include "common/kvstore/KeyValueStore.h"
#include "spdlog/sinks/stdout_sinks.h"
#include <cerrno>
#include <filesystem>

using namespace std;
using namespace sorbet;

string exec(string cmd);

TEST_CASE("kvstore") {
    const string directory(absl::StripAsciiWhitespace(exec("mktemp -d")));
    const auto sink = make_shared<spdlog::sinks::stderr_sink_mt>();
    const auto logger = make_shared<spdlog::logger>("test", sink);
    // Uncomment this for debugging (or copy it into the test case you care about)
    // logger->set_level(spdlog::level::trace);
    auto maxSize = 4096 * 50;

    SUBCASE("CommitsChangesToDisk") {
        {
            auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            owned->writeString("hello", "testing");
            CHECK_EQ(owned->readString("hello"), "testing");
            OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
        }
        {
            auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            CHECK_EQ(owned->readString("hello"), "testing");
        }
    }

    SUBCASE("AbortsChangesByDefault") {
        {
            auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            owned->writeString("hello", "testing");
            CHECK_EQ(owned->readString("hello"), "testing");
        }
        {
            auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            CHECK_EQ(owned->readString("hello"), nullopt);
        }
    }

    SUBCASE("CanBeReowned") {
        auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        owned->writeString("hello", "testing");
        CHECK_EQ(owned->readString("hello"), "testing");
        kvstore = OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
        owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        CHECK_EQ(owned->readString("hello"), "testing");
    }

    SUBCASE("AbortsChangesWhenAborted") {
        auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        owned->writeString("hello", "testing");
        CHECK_EQ(owned->readString("hello"), "testing");
        kvstore = OwnedKeyValueStore::abort(move(owned));
        owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        CHECK_EQ(owned->readString("hello"), nullopt);
    }

    SUBCASE("ClearsChangesOnVersionChange") {
        {
            auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            owned->writeString("hello", "testing");
            CHECK_EQ(owned->readString("hello"), "testing");
            OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
        }
        {
            auto kvstore = make_unique<KeyValueStore>(logger, "2", directory, "vanilla", maxSize);
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            CHECK_EQ(owned->readString("hello"), nullopt);
        }
    }

    SUBCASE("FlavorsHaveDifferentContents") {
        {
            auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            owned->writeString("hello", "testing");
            CHECK_EQ(owned->readString("hello"), "testing");
            OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
        }
        {
            auto kvstore =
                make_unique<KeyValueStore>(logger, "1", directory, "coldbrewcoffeewithchocolateflakes", maxSize);
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            CHECK_EQ(owned->readString("hello"), nullopt);
        }
    }

    SUBCASE("CannotCreateTwoKvstores") {
        auto kvstore1 = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
        CHECK_THROWS_AS((void)make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize),
                        std::invalid_argument);
    }

    // Shared access must prevent exclusive access until the environment closes, even after a transaction ends.
    SUBCASE("HoldsSharedLockUntilEnvironmentCloses") {
        auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
        {
            auto sharedLock = CacheLock::acquireShared(directory, /* blocking */ false);
            REQUIRE(sharedLock.has_value());
        }
        CHECK_FALSE(CacheLock::acquireExclusive(directory, /* blocking */ false).has_value());
        CHECK_EQ(errno, EWOULDBLOCK);

        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        kvstore = OwnedKeyValueStore::abort(move(owned));
        CHECK_FALSE(CacheLock::acquireExclusive(directory, /* blocking */ false).has_value());
        CHECK_EQ(errno, EWOULDBLOCK);

        kvstore.reset();
        CHECK(FileOps::exists(fmt::format("{}/{}", directory, CacheLock::ACCESS_LOCK_FILE)));
        auto exclusiveLock = CacheLock::acquireExclusive(directory, /* blocking */ false);
        CHECK(exclusiveLock.has_value());
    }

    // Wait for exclusive access to end before opening LMDB, without reserving the process-wide kvstore slot.
    SUBCASE("WaitsForExclusiveLockBeforeOpeningDatabase") {
        auto exclusiveLock = CacheLock::acquireExclusive(directory, /* blocking */ false);
        REQUIRE(exclusiveLock.has_value());

        absl::Notification starting;
        absl::Notification opened;
        auto opener = runInAThread("cache-opener", [&]() {
            starting.Notify();
            CHECK_NOTHROW([&]() {
                auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
                opened.Notify();
            }());
        });
        starting.WaitForNotification();
        CHECK_FALSE(opened.WaitForNotificationWithTimeout(absl::Milliseconds(100)));
        CHECK_FALSE(FileOps::exists(directory + "/data.mdb"));

        // Waiting for the file lock must not reserve the process-wide kvstore slot.
        CHECK_NOTHROW((void)make_unique<KeyValueStore>(logger, "1", directory + "/other", "vanilla", maxSize));

        exclusiveLock.reset();
        opener.reset();
        CHECK(opened.HasBeenNotified());
    }

    // A failed LMDB open must release both the access lock and the process-wide kvstore slot.
    SUBCASE("ReleasesLockAfterFailedOpen") {
        auto dataPath = directory + "/data.mdb";
        filesystem::create_directory(dataPath);
        CHECK_THROWS_AS((void)make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize),
                        std::invalid_argument);
        {
            auto exclusiveLock = CacheLock::acquireExclusive(directory, /* blocking */ false);
            CHECK(exclusiveLock.has_value());
        }
        filesystem::remove(dataPath);
        CHECK_NOTHROW((void)make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize));
    }

    // Move assignment releases the old lock and keeps the transferred lock held until its new owner releases it.
    SUBCASE("MoveAssignmentTransfersLockOwnership") {
        auto otherDirectory = directory + "/other";
        FileOps::createDir(otherDirectory);
        auto destination = CacheLock::acquireExclusive(directory, /* blocking */ false);
        auto source = CacheLock::acquireExclusive(otherDirectory, /* blocking */ false);
        REQUIRE(destination.has_value());
        REQUIRE(source.has_value());

        destination = move(source);
        source.reset();
        CHECK(CacheLock::acquireExclusive(directory, /* blocking */ false).has_value());
        CHECK_FALSE(CacheLock::acquireShared(otherDirectory, /* blocking */ false).has_value());
        CHECK_EQ(errno, EWOULDBLOCK);

        // Self-move assignment must preserve the transferred lock.
        auto &sameLock = destination;
        destination = move(sameLock);
        CHECK_FALSE(CacheLock::acquireShared(otherDirectory, /* blocking */ false).has_value());
        CHECK_EQ(errno, EWOULDBLOCK);

        destination.reset();
        CHECK(CacheLock::acquireExclusive(otherDirectory, /* blocking */ false).has_value());
    }

    SUBCASE("LeavesNoStaleTransactions") {
        auto kvstore = make_unique<KeyValueStore>(logger, "1", directory, "vanilla", maxSize);
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        absl::Notification readFinished;
        absl::Notification testFinished;
        // Create a reader transaction.
        // Note: We have to keep the thread alive, otherwise LMDB will clean up transactions that don't point to a live
        // thread when we check the reader lock table.
        auto thread = runInAThread("reader", [&owned, &readFinished, &testFinished]() {
            owned->readString("hello");
            readFinished.Notify();
            testFinished.WaitForNotification();
        });
        readFinished.WaitForNotification();
        // Disown KVstore to end transactions
        kvstore = OwnedKeyValueStore::abort(move(owned));

        // Thread is now ended; if it left a reader transaction, it'll appear stale in the table.
        CHECK_NOTHROW(kvstore->enforceNoOutstandingReaders());
        testFinished.Notify();
    }

    exec(fmt::format("rm -r {}", directory));
}
