#include "main/lsp/LSPFileUpdates.h"
#include "core/core.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPFileUpdates::mergeOlder(const LSPFileUpdates &older) {
    editCount += older.editCount;
    committedEditCount += older.committedEditCount;
    hasNewFiles = hasNewFiles || older.hasNewFiles;
    cancellationExpected = cancellationExpected || older.cancellationExpected;
    preemptionsExpected += older.preemptionsExpected;

    ENFORCE(updatedFiles.size() == updatedFileIndexes.size());
    ENFORCE(older.updatedFiles.size() == older.updatedFileIndexes.size());

    // For updates, we prioritize _newer_ updates.
    UnorderedSet<string> encountered;
    for (auto &f : updatedFiles) {
        encountered.emplace(f->path());
    }

    int i = -1;
    for (auto &f : older.updatedFiles) {
        i++;
        if (encountered.contains(f->path())) {
            continue;
        }
        encountered.emplace(f->path());
        updatedFiles.push_back(f);
        auto &ast = older.updatedFileIndexes[i];
        vector<u1> tree = *ast.tree;
        updatedFileIndexes.push_back(ast::CompressedParsedFile{make_unique<vector<u1>>(move(tree)), ast.file});
    }
    canTakeFastPath = false;
}

LSPFileUpdates LSPFileUpdates::copy() const {
    LSPFileUpdates copy;
    copy.epoch = epoch;
    copy.editCount = editCount;
    copy.committedEditCount = committedEditCount;
    copy.canTakeFastPath = canTakeFastPath;
    copy.hasNewFiles = hasNewFiles;
    copy.updatedFiles = updatedFiles;
    copy.cancellationExpected = cancellationExpected;
    copy.preemptionsExpected = preemptionsExpected;
    for (auto &ast : updatedFileIndexes) {
        // TODO: Make this a shared_ptr to reduce copying.
        vector<u1> tree = *ast.tree;
        copy.updatedFileIndexes.push_back(ast::CompressedParsedFile{make_unique<vector<u1>>(move(tree)), ast.file});
    }
    return copy;
}
} // namespace sorbet::realmain::lsp
