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

// `restore` guarantees which files are in `workspaceFiles`, not their order: the slow path reorders the live vector.
vector<uint32_t> sortedIds(const vector<core::FileRef> &files) {
    vector<uint32_t> result;
    for (auto file : files) {
        result.emplace_back(file.id());
    }
    fast_sort(result);
    return result;
}
} // namespace

TEST_CASE("UndoStateRestoreRemovesExactlyTheFilesTheCanceledEditAdded") {
    auto gs = make_unique<core::GlobalState>(errorQueue);
    gs->initEmpty();

    vector<core::FileRef> workspaceFiles;
    {
        core::UnfreezeFileTable unfreeze(*gs);
        for (auto path : {"__package.rb", "a.rb", "b.rb"}) {
            workspaceFiles.emplace_back(gs->enterFile(path, ""));
        }
    }
    auto expected = sortedIds(workspaceFiles);

    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;
    vector<core::packages::Stratum> fileToStratum(gs->filesUsed(), core::packages::Stratum(0));
    core::packages::Stratum lastStratum(0);
    UndoState undoState(gs->deepCopyGlobalState(), move(indexedFinalGS), fileToStratum, lastStratum, workspaceFiles,
                        /* epoch */ 7);

    // The canceled edit adds a package file and a source file; partitionPackageFiles then moves the package file to
    // the front, displacing `a.rb` past the old size.
    {
        core::UnfreezeFileTable unfreeze(*gs);
        workspaceFiles.emplace_back(gs->enterFile("foo/__package.rb", ""));
        workspaceFiles.emplace_back(gs->enterFile("foo/c.rb", ""));
    }
    swap(workspaceFiles[1], workspaceFiles[3]);
    REQUIRE_EQ(workspaceFiles[3].id(), expected[1]);

    undoState.restore(gs, indexedFinalGS, fileToStratum, lastStratum, workspaceFiles);

    CHECK_EQ(sortedIds(workspaceFiles), expected);
}

} // namespace sorbet::realmain::lsp::test
