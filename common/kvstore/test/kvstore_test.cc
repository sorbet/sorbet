#include "gtest/gtest.h"
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

namespace {
class KeyValueStoreTest : public ::testing::Test {
protected:
    const string directory;
    const shared_ptr<spdlog::sinks::null_sink_mt> sink;
    const shared_ptr<spdlog::logger> logger;

    KeyValueStoreTest()
        : directory(string(absl::StripAsciiWhitespace(exec("mktemp -d")))),
          sink(make_shared<spdlog::sinks::null_sink_mt>()), logger(make_shared<spdlog::logger>("null", sink)) {}

    void TearDown() override {
        exec(fmt::format("rm -r {}", directory));
    }
};
} // namespace

TEST_F(KeyValueStoreTest, CommitsChangesToDisk) {
    {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        owned->writeString("hello", "testing");
        EXPECT_EQ(owned->readString("hello"), "testing");
        OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
    }
    {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        EXPECT_EQ(owned->readString("hello"), "testing");
    }
}

TEST_F(KeyValueStoreTest, AbortsChangesByDefault) {
    {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        owned->writeString("hello", "testing");
        EXPECT_EQ(owned->readString("hello"), "testing");
    }
    {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        EXPECT_EQ(owned->readString("hello"), "");
    }
}

TEST_F(KeyValueStoreTest, CanBeReowned) {
    auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
    auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
    owned->writeString("hello", "testing");
    EXPECT_EQ(owned->readString("hello"), "testing");
    kvstore = OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
    owned = make_unique<OwnedKeyValueStore>(move(kvstore));
    EXPECT_EQ(owned->readString("hello"), "testing");
}

TEST_F(KeyValueStoreTest, AbortsChangesWhenAborted) {
    auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
    auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
    owned->writeString("hello", "testing");
    EXPECT_EQ(owned->readString("hello"), "testing");
    kvstore = OwnedKeyValueStore::abort(move(owned));
    owned = make_unique<OwnedKeyValueStore>(move(kvstore));
    EXPECT_EQ(owned->readString("hello"), "");
}

TEST_F(KeyValueStoreTest, ClearsChangesOnVersionChange) {
    {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        owned->writeString("hello", "testing");
        EXPECT_EQ(owned->readString("hello"), "testing");
        OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
    }
    {
        auto kvstore = make_unique<KeyValueStore>("2", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        EXPECT_EQ(owned->readString("hello"), "");
    }
}

TEST_F(KeyValueStoreTest, FlavorsHaveDifferentContents) {
    {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "vanilla");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        owned->writeString("hello", "testing");
        EXPECT_EQ(owned->readString("hello"), "testing");
        OwnedKeyValueStore::bestEffortCommit(*logger, move(owned));
    }
    {
        auto kvstore = make_unique<KeyValueStore>("1", directory, "coldbrewcoffeewithchocolateflakes");
        auto owned = make_unique<OwnedKeyValueStore>(move(kvstore));
        EXPECT_EQ(owned->readString("hello"), "");
    }
}

TEST_F(KeyValueStoreTest, CannotCreateTwoKvstores) {
    auto kvstore1 = make_unique<KeyValueStore>("1", directory, "vanilla");
    EXPECT_THROW(make_unique<KeyValueStore>("1", directory, "vanilla"), invalid_argument);
}

TEST_F(KeyValueStoreTest, LeavesNoStaleTransactions) {
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
    auto lockTable = kvstore->getReaderLockTable();
    EXPECT_TRUE(lockTable.find("(no active readers)") != string::npos) << lockTable;
    testFinished.Notify();
}
