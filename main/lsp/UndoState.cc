#include "main/lsp/UndoState.h"
#include "common/sort/sort.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
UndoState::UndoState(unique_ptr<core::GlobalState> evictedGs, UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS,
                     vector<uint16_t> fileToStratum, uint16_t lastStratum, const vector<core::FileRef> &workspaceFiles,
                     uint32_t epoch)
    : evictedGs(move(evictedGs)), evictedIndexedFinalGS(std::move(evictedIndexedFinalGS)),
      fileToStratum{move(fileToStratum)}, lastStratum{lastStratum}, initialWorkspaceFilesSize{workspaceFiles.size()},
      epoch(epoch) {}

void UndoState::restore(unique_ptr<core::GlobalState> &gs, UnorderedMap<int, ast::ParsedFile> &indexedFinalGS,
                        vector<uint16_t> &fileToStratum, uint16_t &lastStratum, vector<core::FileRef> &workspaceFiles) {
    indexedFinalGS = std::move(evictedIndexedFinalGS);
    gs = move(evictedGs);

    fileToStratum = move(this->fileToStratum);
    lastStratum = this->lastStratum;

    if (workspaceFiles.size() != this->initialWorkspaceFilesSize) {
        workspaceFiles.erase(workspaceFiles.begin() + this->initialWorkspaceFilesSize, workspaceFiles.end());
    }
}

const unique_ptr<core::GlobalState> &UndoState::getEvictedGs() {
    return evictedGs;
}

} // namespace sorbet::realmain::lsp
