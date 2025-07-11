#include "main/lsp/UndoState.h"
#include "common/sort/sort.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
UndoState::UndoState(unique_ptr<core::GlobalState> evictedGs, UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS,
                     uint32_t epoch)
    : evictedGs(move(evictedGs)), evictedIndexedFinalGS(std::move(evictedIndexedFinalGS)), epoch(epoch) {}

void UndoState::restore(unique_ptr<core::GlobalState> &gs, UnorderedMap<int, ast::ParsedFile> &indexedFinalGS) {
    indexedFinalGS = std::move(evictedIndexedFinalGS);
    gs = move(evictedGs);
}

const unique_ptr<core::GlobalState> &UndoState::getEvictedGs() {
    return evictedGs;
}

} // namespace sorbet::realmain::lsp
