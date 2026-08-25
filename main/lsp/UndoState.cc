#include "main/lsp/UndoState.h"
#include "common/sort/sort.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include <algorithm>

using namespace std;

namespace sorbet::realmain::lsp {
UndoState::UndoState(unique_ptr<core::GlobalState> evictedGs, UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS,
                     vector<core::packages::Stratum> fileToStratum, core::packages::Stratum lastStratum,
                     const vector<core::FileRef> &workspaceFiles, uint32_t epoch)
    : evictedGs(move(evictedGs)), evictedIndexedFinalGS(std::move(evictedIndexedFinalGS)),
      fileToStratum{move(fileToStratum)}, lastStratum{lastStratum}, initialWorkspaceFilesSize{workspaceFiles.size()},
      epoch(epoch) {}

void UndoState::restore(unique_ptr<core::GlobalState> &gs, UnorderedMap<int, ast::ParsedFile> &indexedFinalGS,
                        vector<core::packages::Stratum> &fileToStratum, core::packages::Stratum &lastStratum,
                        vector<core::FileRef> &workspaceFiles) {
    indexedFinalGS = std::move(evictedIndexedFinalGS);
    gs = move(evictedGs);

    fileToStratum = move(this->fileToStratum);
    lastStratum = this->lastStratum;

    // Drop the files that the canceled edit added: exactly the files the restored file table does not know about,
    // wherever the slow path's in-place partitioning of `workspaceFiles` moved them. Truncating to the old size instead
    // would drop whichever pre-existing files the partition displaced past it.
    auto usedFiles = gs->filesUsed();
    workspaceFiles.erase(
        remove_if(workspaceFiles.begin(), workspaceFiles.end(), [usedFiles](auto f) { return f.id() >= usedFiles; }),
        workspaceFiles.end());
    ENFORCE(workspaceFiles.size() == this->initialWorkspaceFilesSize);
}

const unique_ptr<core::GlobalState> &UndoState::getEvictedGs() {
    return evictedGs;
}

} // namespace sorbet::realmain::lsp
