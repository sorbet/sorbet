#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "ProtocolTest.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/common.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/serialize/serialize.h"
#include "main/cache/cache.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "sorbet_version/sorbet_version.h"
#include "spdlog/sinks/null_sink.h"
#include "test/helpers/lsp.h"
#include <signal.h>

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

namespace {
// Inspired by https://github.com/google/googletest/issues/1153#issuecomment-428247477
int wait_for_child_fork(int pid) {
    int status;
    if (0 > waitpid(pid, &status, 0)) {
        std::cerr << "[----------]  Waitpid error!" << std::endl;
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        std::cerr << "[----------]  Non-normal exit from child!" << std::endl;
        return -2;
    }
}

function<void(int)> _handler;
// Used to register a lambda as a signal handler.
void baseHandler(int signal) {
    _handler(signal);
}
} // namespace

TEST_P(ProtocolTest, LSPUsesCache) {
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
        assertDiagnostics(initializeLSP(),
                          {{relativeFilepath, 4, "Returning value that does not conform to method result type"}});

        // Update the file on disk to a different version. This change should not be synced to disk.
        assertDiagnostics(send(*openFile(relativeFilepath, updatedFileContents)), {});
    }

    // LSP should have written cache to disk with file hashes from initialization.
    // It should not include data from file updates made during the editor session.
    {
        auto opts = lspWrapper->opts;

        // Release cache lock.
        lspWrapper = nullptr;

        unique_ptr<const OwnedKeyValueStore> kvstore = realmain::cache::maybeCreateKeyValueStore(*opts);
        EXPECT_EQ(kvstore->read(updatedKey), nullptr);

        auto contents = kvstore->read(key);
        ASSERT_NE(contents, nullptr);

        auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("null", sink);
        auto gs = make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*logger, *logger)));
        payload::createInitialGlobalState(gs, *opts, kvstore);

        // If caching fails, gs gets modified during payload creation.
        EXPECT_FALSE(gs->wasModified());

        auto cachedFile = core::serialize::Serializer::loadFile(*gs, core::FileRef{10}, contents);
        EXPECT_TRUE(cachedFile.file->cached);
        EXPECT_EQ(cachedFile.file->path(), filePath);
        EXPECT_EQ(cachedFile.file->source(), fileContents);
        EXPECT_NE(cachedFile.file->getFileHash(), nullptr);
    }

    // LSP should read from the cache when files on disk match. There should be no cache misses this time since disk
    // state has not changed.
    {
        resetState();
        lspWrapper->opts->inputFileNames.push_back(filePath);
        writeFilesToFS({{relativeFilepath, fileContents}});
        assertDiagnostics(initializeLSP(),
                          {{relativeFilepath, 4, "Returning value that does not conform to method result type"}});

        auto counters = getCounters();
        EXPECT_EQ(counters.getCounter("types.input.files.kvstore.miss"), 0);
        EXPECT_GT(counters.getCounter("types.input.files.kvstore.hit"), 0);
    }

    // LSP should not use the cached file when a file on disk changes.
    {
        resetState();
        lspWrapper->opts->inputFileNames.push_back(filePath);
        writeFilesToFS({{relativeFilepath, updatedFileContents}});
        assertDiagnostics(initializeLSP(), {});

        auto counters = getCounters();
        EXPECT_EQ(counters.getCounter("types.input.files.kvstore.miss"), 1);
    }

    // LSP should update the cache when seeing an updated file during initialization.
    {
        auto opts = lspWrapper->opts;

        // Release cache lock.
        lspWrapper = nullptr;
        unique_ptr<const OwnedKeyValueStore> kvstore = realmain::cache::maybeCreateKeyValueStore(*opts);
        auto updatedFileData = kvstore->read(updatedKey);
        ASSERT_NE(updatedFileData, nullptr);

        auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("null", sink);
        auto gs = make_unique<core::GlobalState>(make_shared<core::ErrorQueue>(*logger, *logger));
        payload::createInitialGlobalState(gs, *opts, kvstore);

        auto cachedFile = core::serialize::Serializer::loadFile(*gs, core::FileRef{10}, updatedFileData);
        EXPECT_TRUE(cachedFile.file->cached);
        EXPECT_EQ(cachedFile.file->path(), filePath);
        EXPECT_EQ(cachedFile.file->source(), updatedFileContents);
        EXPECT_NE(cachedFile.file->getFileHash(), nullptr);
    }
}

TEST_P(ProtocolTest, LSPDoesNotUseCacheIfModified) {
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
        assertDiagnostics(initializeLSP(),
                          {{relativeFilepath, 4, "Returning value that does not conform to method result type"}});
    }

    // LSP should have written cache to disk with file hashes from initialization.
    {
        auto opts = lspWrapper->opts;

        // Release cache lock.
        lspWrapper = nullptr;

        unique_ptr<const OwnedKeyValueStore> kvstore = realmain::cache::maybeCreateKeyValueStore(*opts);
        auto contents = kvstore->read(key);
        ASSERT_NE(contents, nullptr);

        auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("null", sink);
        auto gs = make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*logger, *logger)));
        payload::createInitialGlobalState(gs, *opts, kvstore);

        // If caching fails, gs gets modified during payload creation.
        EXPECT_FALSE(gs->wasModified());

        auto cachedFile = core::serialize::Serializer::loadFile(*gs, core::FileRef{10}, contents);
        EXPECT_TRUE(cachedFile.file->cached);
        EXPECT_EQ(cachedFile.file->path(), filePath);
        EXPECT_EQ(cachedFile.file->source(), fileContents);
        EXPECT_NE(cachedFile.file->getFileHash(), nullptr);
    }

    // LSP should read from disk when the cache gets updated by a different process mid-process.
    {
        // Register signal handler _before_ forking to avoid race.
        bool signaled = false;
        _handler = [&signaled](int code) { signaled = true; };
        signal(SIGHUP, baseHandler);
        // Fork before grabbing DB lock.
        const int child_pid = fork();
        if (child_pid == 0) {
            // Child process; wait for signal before writing to cache.
            while (!signaled) {
            }

            // Let's update a file and write over the cache.
            resetState();
            writeFilesToFS({{relativeFilepath, updatedFileContents}});

            lspWrapper->opts->inputFileNames.push_back(filePath);
            assertDiagnostics(initializeLSP(), {});

            // File was updated, so no cache hits.
            auto counters = getCounters();
            EXPECT_GT(counters.getCounter("types.input.files.kvstore.miss"), 0);
            EXPECT_EQ(counters.getCounter("types.input.files.kvstore.hit"), 0);
            EXPECT_EQ(counters.getCounter("cache.committed"), 1);

            // Exit explicitly here to stop fork from running the rest of the test suite.
            exit(testing::Test::HasFailure());
        } else {
            resetState();
            // Tell child process to mutate the cache.
            kill(child_pid, SIGHUP);

            // Wait for child process to finish mutating the cache.
            EXPECT_EQ(0, wait_for_child_fork(child_pid));

            lspWrapper->opts->inputFileNames.push_back(filePath);
            writeFilesToFS({{relativeFilepath, fileContents}});
            assertDiagnostics(initializeLSP(),
                              {{relativeFilepath, 4, "Returning value that does not conform to method result type"}});

            // We should not use the cache since it has been dirtied.
            auto counters = getCounters();
            EXPECT_EQ(counters.getCounter("types.input.files.kvstore.hit"), 0);
            EXPECT_EQ(counters.getCounter("cache.committed"), 0);
        }
    }
}

// Run these tests in multi-threaded mode with caching
INSTANTIATE_TEST_SUITE_P(MultithreadedProtocolTests, ProtocolTest, testing::Values(ProtocolTestConfig{true, true}));
} // namespace sorbet::test::lsp
