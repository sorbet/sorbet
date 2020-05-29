#include "doctest.h"
// has to go first as it violates our requirements
#include "spdlog/spdlog.h"
// has to go above null_sink.h; this comment prevents reordering.
#include "absl/strings/str_split.h" // For StripAsciiWhitespace
#include "absl/synchronization/notification.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/kvstore/KeyValueStore.h"
#include "spdlog/sinks/null_sink.h"

using namespace std;
using namespace sorbet;

string exec(string cmd);

TEST_CASE("kvstore") {
    const string directory(absl::StripAsciiWhitespace(exec("mktemp -d")));
    const auto sink = make_shared<spdlog::sinks::null_sink_mt>();
    const auto logger = make_shared<spdlog::logger>("null", sink);

    SUBCASE("CommitsChangesToDisk") {
        {
            auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            owned->writeString("hello", "testing");
            CHECK_EQ(owned->readString("hello"), "testing");
            OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
        }
        {
            auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            CHECK_EQ(owned->readString("hello"), "testing");
        }
    }

    SUBCASE("AbortsChangesByDefault") {
        {
            auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            owned->writeString("hello", "testing");
            CHECK_EQ(owned->readString("hello"), "testing");
        }
        {
            auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            CHECK_EQ(owned->readString("hello"), "");
        }
    }

    SUBCASE("CanBeReowned") {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        owned->writeString("hello", "testing");
        CHECK_EQ(owned->readString("hello"), "testing");
        kvstore = OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
        owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        CHECK_EQ(owned->readString("hello"), "testing");
    }

    SUBCASE("AbortsChangesWhenAborted") {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        owned->writeString("hello", "testing");
        CHECK_EQ(owned->readString("hello"), "testing");
        kvstore = OwnedKeyValueStore::abort(move(owned));
        owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        CHECK_EQ(owned->readString("hello"), "");
    }

    SUBCASE("ClearsChangesOnVersionChange") {
        {
            auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            owned->writeString("hello", "testing");
            CHECK_EQ(owned->readString("hello"), "testing");
            OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
        }
        {
            auto kvstore = make_unique<KeyValueStore>("2", directory, "vanilla");
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            CHECK_EQ(owned->readString("hello"), "");
        }
    }

    SUBCASE("FlavorsHaveDifferentContents") {
        {
            auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            owned->writeString("hello", "testing");
            CHECK_EQ(owned->readString("hello"), "testing");
            OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
        }
        {
            auto kvstore = make_unique<KeyValueStore>("1", directory, "coldbrewcoffeewithchocolateflakes");
            auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
            CHECK_EQ(owned->readString("hello"), "");
        }
    }

    SUBCASE("CannotCreateTwoKvstores") {
        auto kvstore1 = make_unique<KeyValueStore>("1", directory, "vanilla");
        CHECK_THROWS_AS(make_unique<KeyValueStore>("1", directory, "vanilla"), std::invalid_argument);
    }

    SUBCASE("LeavesNoStaleTransactions") {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
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
