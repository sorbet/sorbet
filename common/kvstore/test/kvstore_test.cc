#include "gtest/gtest.h"
// has to go first as it violates our requirements
#include "spdlog/spdlog.h"
// has to go above null_sink.h; this comment prevents reordering.
#include "absl/strings/str_split.h" // For StripAsciiWhitespace
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
