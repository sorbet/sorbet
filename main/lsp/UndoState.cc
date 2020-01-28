#include "main/lsp/UndoState.h"
#include "common/sort.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"

using namespace std;

namespace sorbet::realmain::lsp {
UndoState::UndoState(const LSPConfiguration &config, unique_ptr<core::GlobalState> evictedGs,
                     UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS,
                     vector<core::FileRef> evictedFilesThatHaveErrors)
    : config(config), evictedGs(move(evictedGs)), evictedIndexedFinalGS(std::move(evictedIndexedFinalGS)),
      evictedFilesThatHaveErrors(move(evictedFilesThatHaveErrors)) {}

void UndoState::recordEvictedState(ast::ParsedFile evictedIndexTree, core::FileHash evictedFileHash) {
    const auto id = evictedIndexTree.file.id();
    // The first time a file gets evicted, it's an index tree from the old global state.
    // Subsequent times it is evicting old index trees from the new global state, and we don't care.
    // Also, ignore updates to new files (id >= size of file table)
    if (id < evictedGs->getFiles().size() && !evictedIndexed.contains(id)) {
        evictedIndexed[id] = move(evictedIndexTree);
        // file hashes should be kept in-sync with indexed.
        ENFORCE(!evictedFileHashes.contains(id));
        evictedFileHashes[id] = move(evictedFileHash);
    }
}

vector<core::FileRef> UndoState::restore(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> &indexed,
                                         UnorderedMap<int, ast::ParsedFile> &indexedFinalGS,
                                         vector<core::FileHash> &globalStateHashes,
                                         std::vector<core::FileRef> &filesThatHaveErrors) {
    // Replace evicted index trees and file hashes.
    for (auto &entry : evictedIndexed) {
        indexed[entry.first] = move(entry.second);
        ENFORCE(evictedFileHashes.contains(entry.first));
        globalStateHashes[entry.first] = move(evictedFileHashes[entry.first]);
    }
    indexedFinalGS = std::move(evictedIndexedFinalGS);

    // Restore `filesThatHaveErrors` to its previous state both here, and on the client.
    // We send empty error lists for files that newly have errors from the canceled slow path.
    // If those new files still have errors, they will show up in the next typecheck operation.
    // We specifically do this because the old GS might not have the new files introduced in the canceled slow path,
    // and we expect `gs` to always contain all of the files that previously had errors.
    // TODO: Update with the reverse when we switch to tombstoning files.
    vector<string> newPathsThatHaveErrors = config.frefsToPaths(*gs, filesThatHaveErrors);
    vector<string> oldPathsThatHaveErrors = config.frefsToPaths(*evictedGs, evictedFilesThatHaveErrors);
    fast_sort(newPathsThatHaveErrors);
    fast_sort(oldPathsThatHaveErrors);
    vector<string> clearErrorsFor;
    std::set_difference(newPathsThatHaveErrors.begin(), newPathsThatHaveErrors.end(), oldPathsThatHaveErrors.begin(),
                        oldPathsThatHaveErrors.end(), std::inserter(clearErrorsFor, clearErrorsFor.begin()));
    config.logger->debug("[Typechecker] Restoring empty error list for {} files", clearErrorsFor.size());
    for (auto &file : clearErrorsFor) {
        vector<unique_ptr<Diagnostic>> diagnostics;
        auto params = make_unique<PublishDiagnosticsParams>(config.localName2Remote(file), move(diagnostics));
        config.output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentPublishDiagnostics, move(params))));
    }
    filesThatHaveErrors = evictedFilesThatHaveErrors;

    // Finally, restore the old global state.
    gs = move(evictedGs);
    // Inform caller that we need to retypecheck all files that used to have errors.
    // TODO(jvilk): Can probably filter using diagnostic epoch to avoid extra work.
    return evictedFilesThatHaveErrors;
}

} // namespace sorbet::realmain::lsp
