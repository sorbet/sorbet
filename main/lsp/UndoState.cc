#include "main/lsp/UndoState.h"
#include "common/sort.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
UndoState::UndoState(unique_ptr<core::GlobalState> evictedGs, UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS)
    : evictedGs(move(evictedGs)), evictedIndexedFinalGS(std::move(evictedIndexedFinalGS)) {}

void UndoState::recordEvictedState(ast::ParsedFile evictedIndexTree) {
    const auto id = evictedIndexTree.file.id();
    // The first time a file gets evicted, it's an index tree from the old global state.
    // Subsequent times it is evicting old index trees from the new global state, and we don't care.
    // Also, ignore updates to new files (id >= size of file table)
    if (id < evictedGs->getFiles().size() && !evictedIndexed.contains(id)) {
        evictedIndexed[id] = move(evictedIndexTree);
    }
}

void UndoState::restore(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> &indexed,
                        UnorderedMap<int, ast::ParsedFile> &indexedFinalGS) {
    // Replace evicted index trees.
    for (auto &entry : evictedIndexed) {
        indexed[entry.first] = move(entry.second);
    }

    indexedFinalGS = std::move(evictedIndexedFinalGS);
    gs = move(evictedGs);
}

} // namespace sorbet::realmain::lsp
