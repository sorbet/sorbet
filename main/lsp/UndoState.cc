#include "main/lsp/UndoState.h"
#include "common/sort/sort.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
UndoState::UndoState(unique_ptr<core::GlobalState> evictedGs, UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS,
                     vector<core::packages::Stratum> fileToStratum, core::packages::Stratum lastStratum,
                     const vector<core::FileRef> &workspaceFiles, uint32_t epoch)
    : evictedGs(move(evictedGs)), evictedIndexedFinalGS(std::move(evictedIndexedFinalGS)),
      fileToStratum{move(fileToStratum)}, lastStratum{lastStratum}, savedWorkspaceFiles(workspaceFiles), epoch(epoch) {}

void UndoState::restore(unique_ptr<core::GlobalState> &gs, UnorderedMap<int, ast::ParsedFile> &indexedFinalGS,
                        vector<core::packages::Stratum> &fileToStratum, core::packages::Stratum &lastStratum,
                        vector<core::FileRef> &workspaceFiles) {
    indexedFinalGS = std::move(evictedIndexedFinalGS);
    gs = move(evictedGs);

    fileToStratum = move(this->fileToStratum);
    lastStratum = this->lastStratum;

    // Restore the snapshot wholesale. The slow path appends new files to the live vector and then permutes it in place
    // (partitionPackageFiles), so rolling back by truncating to the old size would erase whatever ended up at the tail,
    // not the new files.
    workspaceFiles = std::move(this->savedWorkspaceFiles);
}

const unique_ptr<core::GlobalState> &UndoState::getEvictedGs() {
    return evictedGs;
}

} // namespace sorbet::realmain::lsp
