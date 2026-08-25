#include "doctest/doctest.h"
// has to go first as it violates our requirements

#include "ast/ast.h"
#include "common/sort/sort.h"
#include "core/ErrorQueue.h"
#include "core/GlobalState.h"
#include "core/Unfreeze.h"
#include "core/packages/Stratum.h"
#include "main/lsp/UndoState.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace sorbet::realmain::lsp::test {
using namespace std;

namespace {
auto logger = spdlog::stderr_color_mt("undo_state_test");
auto errorQueue = make_shared<core::ErrorQueue>(*logger, *logger);

unique_ptr<core::GlobalState> makeGlobalState() {
    auto gs = make_unique<core::GlobalState>(errorQueue);
    gs->initEmpty();
    return gs;
}

// The ids in `files`, sorted: `restore` guarantees membership, not order (the slow path reorders the live vector).
vector<uint32_t> sortedIds(const vector<core::FileRef> &files) {
    vector<uint32_t> result;
    result.reserve(files.size());
    for (auto file : files) {
        result.emplace_back(file.id());
    }
    fast_sort(result);
    return result;
}

struct Fixture {
    unique_ptr<core::GlobalState> gs = makeGlobalState();
    vector<core::FileRef> workspaceFiles;
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;
    vector<core::packages::Stratum> fileToStratum;
    core::packages::Stratum lastStratum{0};

    Fixture() {
        core::UnfreezeFileTable unfreeze(*gs);
        // A package file followed by two source files, i.e. already in the order that partitionPackageFiles leaves
        // behind (package files first).
        workspaceFiles.emplace_back(gs->enterFile("__package.rb", ""));
        workspaceFiles.emplace_back(gs->enterFile("a.rb", ""));
        workspaceFiles.emplace_back(gs->enterFile("b.rb", ""));
        fileToStratum.assign(gs->filesUsed(), core::packages::Stratum(0));
    }

    UndoState snapshot(uint32_t epoch) {
        auto evictedGs = gs->deepCopyGlobalState();
        return UndoState(move(evictedGs), move(indexedFinalGS), fileToStratum, lastStratum, workspaceFiles, epoch);
    }

    // What the slow path does to `workspaceFiles` after the snapshot: applyFileTableUpdates appends the edit's new
    // files, then partitionPackageFiles moves package files to the front without preserving order, so an appended
    // `__package.rb` ends up in the front block and a pre-existing source file is displaced past the old size.
    void appendNewFilesAndPartition(vector<core::FileRef> &files, vector<core::FileRef> &newFiles) {
        core::UnfreezeFileTable unfreeze(*gs);
        newFiles.emplace_back(gs->enterFile("foo/__package.rb", ""));
        newFiles.emplace_back(gs->enterFile("foo/c.rb", ""));
        files.insert(files.end(), newFiles.begin(), newFiles.end());
        swap(files[1], files[3]);
    }
};
} // namespace

TEST_CASE_FIXTURE(Fixture, "RestoreRemovesExactlyTheFilesTheCanceledEditAdded") {
    auto expected = sortedIds(workspaceFiles);
    auto undoState = snapshot(/* epoch */ 7);

    vector<core::FileRef> newFiles;
    appendNewFilesAndPartition(workspaceFiles, newFiles);
    REQUIRE_EQ(workspaceFiles.size(), expected.size() + newFiles.size());
    // A pre-existing file now sits past the old size, and an appended file sits below it.
    REQUIRE_EQ(workspaceFiles[1].id(), newFiles[0].id());
    REQUIRE_EQ(workspaceFiles[3].id(), expected[1]);

    undoState.restore(gs, indexedFinalGS, fileToStratum, lastStratum, workspaceFiles);

    CHECK_EQ(sortedIds(workspaceFiles), expected);
}

TEST_CASE_FIXTURE(Fixture, "RestoreBringsBackTheEvictedGlobalStateAndStrata") {
    auto filesBefore = gs->filesUsed();
    fileToStratum.assign(gs->filesUsed(), core::packages::Stratum(3));
    lastStratum = core::packages::Stratum(3);
    auto undoState = snapshot(/* epoch */ 9);

    // The slow path grows the file table and recomputes strata before it is canceled.
    {
        core::UnfreezeFileTable unfreeze(*gs);
        workspaceFiles.emplace_back(gs->enterFile("new.rb", ""));
    }
    fileToStratum.assign(gs->filesUsed(), core::packages::Stratum(0));
    lastStratum = core::packages::Stratum(0);

    undoState.restore(gs, indexedFinalGS, fileToStratum, lastStratum, workspaceFiles);

    CHECK_EQ(gs->filesUsed(), filesBefore);
    CHECK_EQ(fileToStratum.size(), filesBefore);
    CHECK(lastStratum == core::packages::Stratum(3));
    CHECK_EQ(workspaceFiles.size(), 3);
}

} // namespace sorbet::realmain::lsp::test
