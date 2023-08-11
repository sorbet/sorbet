#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/serialize/serialize.h"
#include "main/cache/cache.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "sorbet_version/sorbet_version.h"
#include "spdlog/sinks/null_sink.h"
#include "test/helpers/CounterStateDatabase.h"
#include "test/helpers/lsp.h"
#include "test/lsp/ProtocolTest.h"
#include <sys/wait.h>

#include <iostream> // for cerr

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

namespace {
// Inspired by https://github.com/google/googletest/issues/1153#issuecomment-428247477
int wait_for_child_fork(int pid) {
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        cerr << "[----------]  Waitpid error!" << std::endl;
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        cerr << "[----------]  Non-normal exit from child!" << std::endl;
        return -2;
    }
}

class CacheProtocolTest : public ProtocolTest {
public:
    CacheProtocolTest() : ProtocolTest(/* use multithreading */ true, /* use caching */ true) {}
};

} // namespace

TEST_CASE_FIXTURE(CacheProtocolTest, "LSPUsesCache") {
    // Write a file to disk.
    auto relativeFilepath = "test.rb";
    auto filePath = fmt::format("{}/{}", rootPath, relativeFilepath);
    // This file has an error to indirectly assert that LSP is actually typechecking the file during initialization.
    auto fileContents = "# typed: true\nclass Foo extend T::Sig\nsig {returns(Integer)}\ndef bar\n'hello'\nend\nend\n";
    auto key =
        realmain::pipeline::fileKey(core::File(string(filePath), string(fileContents), core::File::Type::Normal, 0));

    // Note: We need to introduce a new name, otherwise nametable doesn't change and we don't update the cache.
    auto updatedFileContents = "# typed: true\nclass NewName\nend\n";
    auto updatedKey = realmain::pipeline::fileKey(
        core::File(string(filePath), string(updatedFileContents), core::File::Type::Normal, 0));

    // LSP should write a cache to disk corresponding to initialization state.
    {
        writeFilesToFS({{relativeFilepath, fileContents}});

        lspWrapper->opts->inputFileNames.push_back(filePath);
        assertErrorDiagnostics(
            initializeLSP(),
            {{relativeFilepath, 4, "Expected `Integer` but found `String(\"hello\")` for method result type"}});

        // Update the file on disk to a different version. This change should not be synced to disk.
        assertErrorDiagnostics(send(*openFile(relativeFilepath, updatedFileContents)), {});
    }

    // LSP should have written cache to disk with file hashes from initialization.
    // It should not include data from file updates made during the editor session.
    {
        auto opts = lspWrapper->opts;

        // Release cache lock.
        lspWrapper = nullptr;

        auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("null", sink);
        unique_ptr<const OwnedKeyValueStore> kvstore = realmain::cache::maybeCreateKeyValueStore(logger, *opts);
        CHECK_EQ(kvstore->read(updatedKey).data, nullptr);

        auto contents = kvstore->read(key);
        REQUIRE_NE(contents.data, nullptr);

        auto gs = make_unique<core::GlobalState>(make_shared<core::ErrorQueue>(*logger, *logger));
        payload::createInitialGlobalState(gs, *opts, kvstore);

        // If caching fails, gs gets modified during payload creation.
        CHECK_FALSE(gs->wasModified());

        core::File file{string(filePath), string(fileContents), core::File::Type::Normal};
        auto tree = core::serialize::Serializer::loadTree(*gs, file, contents.data);
        CHECK(file.cached());
        CHECK_NE(file.getFileHash(), nullptr);
        CHECK_NE(tree, nullptr);

        // Loading should fail if file is too small
        core::File smallFile{"", "", core::File::Type::Normal};
        CHECK_EQ(core::serialize::Serializer::loadTree(*gs, smallFile, contents.data), nullptr);
    }

    // LSP should read from the cache when files on disk match. There should be no cache misses this time since disk
    // state has not changed.
    {
        resetState();
        lspWrapper->opts->inputFileNames.push_back(filePath);
        writeFilesToFS({{relativeFilepath, fileContents}});
        assertErrorDiagnostics(
            initializeLSP(),
            {{relativeFilepath, 4, "Expected `Integer` but found `String(\"hello\")` for method result type"}});

        auto counters = getCounters();
        CHECK_EQ(counters.getCounter("types.input.files.kvstore.miss"), 0);
        CHECK_GT(counters.getCounter("types.input.files.kvstore.hit"), 0);
    }

    // LSP should not use the cached file when a file on disk changes.
    {
        resetState();
        lspWrapper->opts->inputFileNames.push_back(filePath);
        writeFilesToFS({{relativeFilepath, updatedFileContents}});
        assertErrorDiagnostics(initializeLSP(), {});

        auto counters = getCounters();
        CHECK_EQ(counters.getCounter("types.input.files.kvstore.miss"), 1);
    }

    // LSP should update the cache when seeing an updated file during initialization.
    {
        auto opts = lspWrapper->opts;

        // Release cache lock.
        lspWrapper = nullptr;
        auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("null", sink);
        unique_ptr<const OwnedKeyValueStore> kvstore = realmain::cache::maybeCreateKeyValueStore(logger, *opts);
        auto updatedFileData = kvstore->read(updatedKey);
        REQUIRE_NE(updatedFileData.data, nullptr);

        auto gs = make_unique<core::GlobalState>(make_shared<core::ErrorQueue>(*logger, *logger));
        payload::createInitialGlobalState(gs, *opts, kvstore);

        core::File file{string(filePath), string(updatedFileContents), core::File::Type::Normal};
        auto cachedFile = core::serialize::Serializer::loadTree(*gs, file, updatedFileData.data);
        CHECK(file.cached());
        CHECK_NE(file.getFileHash(), nullptr);
        CHECK_NE(cachedFile, nullptr);
    }
}

TEST_CASE_FIXTURE(CacheProtocolTest, "LSPDoesNotUseCacheIfModified") {
    // Write a file to disk.
    auto relativeFilepath = "test.rb";
    auto filePath = fmt::format("{}/{}", rootPath, relativeFilepath);
    // This file has an error to indirectly assert that LSP is actually typechecking the file during initialization.
    auto fileContents = "# typed: true\nclass Foo extend T::Sig\nsig {returns(Integer)}\ndef bar\n'hello'\nend\nend\n";
    auto key =
        realmain::pipeline::fileKey(core::File(string(filePath), string(fileContents), core::File::Type::Normal, 0));

    // Note: We need to introduce a new name, otherwise nametable doesn't change and we don't update the cache.
    auto updatedFileContents = "# typed: true\nclass NewName\nend\n";

    // LSP should write a cache to disk corresponding to initialization state.
    {
        writeFilesToFS({{relativeFilepath, fileContents}});

        lspWrapper->opts->inputFileNames.push_back(filePath);
        assertErrorDiagnostics(
            initializeLSP(),
            {{relativeFilepath, 4, "Expected `Integer` but found `String(\"hello\")` for method result type"}});
    }

    auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    auto nullLogger = std::make_shared<spdlog::logger>("null", sink);

    // LSP should have written cache to disk with file hashes from initialization.
    {
        auto opts = lspWrapper->opts;

        // Release cache lock.
        lspWrapper = nullptr;

        unique_ptr<const OwnedKeyValueStore> kvstore = realmain::cache::maybeCreateKeyValueStore(nullLogger, *opts);
        auto contents = kvstore->read(key);
        REQUIRE_NE(contents.data, nullptr);

        auto gs = make_unique<core::GlobalState>(make_shared<core::ErrorQueue>(*nullLogger, *nullLogger));
        payload::createInitialGlobalState(gs, *opts, kvstore);

        // If caching fails, gs gets modified during payload creation.
        CHECK_FALSE(gs->wasModified());

        core::File file{string(filePath), string(fileContents), core::File::Type::Normal};
        auto tree = core::serialize::Serializer::loadTree(*gs, file, contents.data);
        CHECK(file.cached());
        CHECK_NE(file.getFileHash(), nullptr);
        CHECK_NE(tree, nullptr);
    }

    // LSP should read from disk when the cache gets updated by a different process mid-process.
    {
        // Note: I had trouble getting signals to work in CI. Even in a loop where the parent process sent a signal
        // to the child, the child never received the signal. So, I'm using a file to communicate instead.
        auto signalFile = cacheDir + "/signal_file";
        // Fork before grabbing DB lock.
        const int child_pid = fork();
        if (child_pid == 0) {
            // Child process; wait for file to exist before writing to cache.
            while (!FileOps::exists(signalFile)) {
                Timer::timedSleep(chrono::microseconds(1000), *nullLogger, "Waiting for signal");
            }

            // Let's update a file and write over the cache.
            resetState();
            writeFilesToFS({{relativeFilepath, updatedFileContents}});

            lspWrapper->opts->inputFileNames.push_back(filePath);
            assertErrorDiagnostics(initializeLSP(), {});

            // File was updated, so no cache hits.
            auto counters = getCounters();
            const uint32_t kvstoreMissCount = counters.getCounter("types.input.files.kvstore.miss");
            const uint32_t kvstoreHitCount = counters.getCounter("types.input.files.kvstore.hit");
            const uint32_t cacheCommitted = counters.getCounter("cache.committed");
            CHECK_GT(kvstoreMissCount, 0);
            CHECK_EQ(kvstoreHitCount, 0);
            CHECK_EQ(cacheCommitted, 1);

            // Exit explicitly here to stop fork from running the rest of the test suite.
            if (kvstoreMissCount > 0 && kvstoreHitCount == 0 && cacheCommitted == 1) {
                exit(0);
            } else {
                exit(-1);
            }
        } else {
            resetState();

            // Tell child process to mutate the cache by writing a file.
            FileOps::write(signalFile, " ");

            // Wait for child process to finish mutating the cache.
            CHECK_EQ(0, wait_for_child_fork(child_pid));

            lspWrapper->opts->inputFileNames.push_back(filePath);
            writeFilesToFS({{relativeFilepath, fileContents}});
            assertErrorDiagnostics(
                initializeLSP(),
                {{relativeFilepath, 4, "Expected `Integer` but found `String(\"hello\")` for method result type"}});

            // We should not use the cache since it has been dirtied.
            auto counters = getCounters();
            CHECK_EQ(counters.getCounter("types.input.files.kvstore.hit"), 0);
            CHECK_EQ(counters.getCounter("cache.committed"), 0);
        }
    }
}
} // namespace sorbet::test::lsp
