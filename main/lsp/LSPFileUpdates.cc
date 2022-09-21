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
        updatedFileIndexes.push_back(ast::ParsedFile{ast.tree.deepCopy(), ast.file});
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
        copy.updatedFileIndexes.push_back(ast::ParsedFile{ast.tree.deepCopy(), ast.file});
    }
    return copy;
}

namespace {

// In debug builds, asserts that we have not accidentally taken the fast path after a change to the set of
// methods in a file.
bool validateIdenticalFingerprints(const std::vector<core::SymbolHash> &a, const std::vector<core::SymbolHash> &b) {
    if (a.size() != b.size()) {
        return false;
    }

    core::SymbolHash previousHash; // Initializes to <0, 0>.
    auto bIt = b.begin();
    for (const auto &methodA : a) {
        const auto &methodB = *bIt;
        if (methodA.nameHash != methodB.nameHash) {
            return false;
        }

        // Enforce that hashes are sorted in ascending order.
        if (methodA < previousHash) {
            return false;
        }

        previousHash = methodA;
        bIt++;
    }

    return true;
}

} // namespace

LSPFileUpdates::FastPathFilesToTypecheckResult
LSPFileUpdates::fastPathFilesToTypecheck(const core::GlobalState &gs, const LSPConfiguration &config,
                                         const vector<shared_ptr<core::File>> &updatedFiles,
                                         const UnorderedMap<core::FileRef, shared_ptr<core::File>> &evictedFiles) {
    FastPathFilesToTypecheckResult result;
    Timer timeit(config.logger, "compute_fast_path_file_set");
    vector<core::SymbolHash> changedDeletableSymbolHashes;
    auto idx = -1;
    for (const auto &updatedFile : updatedFiles) {
        idx++;
        auto fref = gs.findFileByPath(updatedFile->path());
        // We don't support new files on the fast path. This enforce failing indicates a bug in our fast/slow
        // path logic in LSPPreprocessor.
        ENFORCE(fref.exists());
        ENFORCE(updatedFile->getFileHash() != nullptr);
        if (config.opts.stripePackages && updatedFile->isPackage()) {
            // Only relevant in --stripe-packages mode. Package declarations do not have method
            // hashes. Instead we rely on recomputing packages if any __package.rb source
            // changes.
            continue;
        }
        if (!fref.exists()) {
            // Defensive (?)
            continue;
        }

        // When run from the indexer, the old file will actually have been evicted from the initialGS
        // so that the initialGS containing the new file can be given to the pipeline to be indexed
        // (and thus have the new hashes computed), so that the `updatedFile` and
        // `fref.data(gs)` actually are the same File object. For this case, we have to find the
        // old File's hashes in `evictedFiles`.
        //
        // When run from the typechecker, the old file will not yet have been evicted before
        // fastPathFilesToTypecheck is called (it will be evicted later on that thread, right
        // before calling into the pipeline). On that thread, `updatedFile` represents the new
        // File, and the thing in GlobalState is the old File.
        const auto &oldLocalSymbolTableHashes = evictedFiles.empty()
                                                    ? fref.data(gs).getFileHash()->localSymbolTableHashes
                                                    : evictedFiles.at(fref)->getFileHash()->localSymbolTableHashes;
        const auto &newLocalSymbolTableHashes = updatedFile->getFileHash()->localSymbolTableHashes;
        const auto &oldDeletableSymbolHashes = oldLocalSymbolTableHashes.deletableSymbolHashes;
        const auto &newDeletableSymbolHashes = newLocalSymbolTableHashes.deletableSymbolHashes;

        if (!config.opts.lspExperimentalFastPathEnabled) {
            // Both oldHash and newHash should have the same methods, since this is the fast path!
            ENFORCE(validateIdenticalFingerprints(oldDeletableSymbolHashes, newDeletableSymbolHashes),
                    "definitionHash should have failed");
        }

        // Find which hashes changed. Note: deletableSymbolHashes are pre-sorted, so set_difference should work.
        // This will insert two entries into `deletableSymbolHashes` for each changed method, but they
        // will get deduped later.
        absl::c_set_symmetric_difference(oldDeletableSymbolHashes, newDeletableSymbolHashes,
                                         std::back_inserter(changedDeletableSymbolHashes));

        result.changedFiles.emplace(fref, idx);
    }

    result.changedSymbolNameHashes.reserve(changedDeletableSymbolHashes.size());
    absl::c_transform(changedDeletableSymbolHashes, std::back_inserter(result.changedSymbolNameHashes),
                      [](const auto &symhash) { return symhash.nameHash; });
    core::WithoutUniqueNameHash::sortAndDedupe(result.changedSymbolNameHashes);

    if (result.changedSymbolNameHashes.empty()) {
        // Optimization--skip the loop over every file in the project (`gs.getFiles()`) if
        // the set of changed symbols is empty (e.g., running a completion request inside a
        // method body)
        return result;
    }

    int i = -1;
    for (auto &oldFile : gs.getFiles()) {
        i++;
        if (oldFile == nullptr) {
            continue;
        }

        auto ref = core::FileRef(i);
        if (result.changedFiles.contains(ref)) {
            continue;
        }

        if (config.opts.stripePackages && oldFile->isPackage()) {
            continue; // See note above about --stripe-packages.
        }

        if (oldFile->isPayload()) {
            // Don't retypecheck files in the payload via incremental namer, as that might
            // cause well-known symbols to get deleted and assigned a new SymbolRef ID.
            continue;
        }

        ENFORCE(oldFile->getFileHash() != nullptr);
        const auto &oldHash = *oldFile->getFileHash();
        vector<core::WithoutUniqueNameHash> intersection;
        absl::c_set_intersection(result.changedSymbolNameHashes, oldHash.usages.nameHashes,
                                 std::back_inserter(intersection));
        if (intersection.empty()) {
            continue;
        }

        result.extraFiles.emplace_back(ref);

        if (result.changedFiles.size() + result.extraFiles.size() > (2 * config.opts.lspMaxFilesOnFastPath)) {
            // Short circuit, as a performance optimization.
            // (gs.getFiles() is usually 3-4 orders of magnitude larger than lspMaxFilesOnFastPath)
            //
            // The "2 * ..." is so that we can get a rough idea of whether there's an easy
            // bang-for-buck bump we could make to the threshold by reading the logs.
            //
            // One of two things could be true:
            // - We're running on the indexer thread to decide canTakeFastPath, which only cares about how
            //   many extra files there are, not what they are.
            // - We're running on the typechecker thread (knowing that canTakeFastPath was already true)
            //   and simply need to compute the list of files to typecheck. But that would be a
            //   contradiction--because otherwise the indexer would have marked the update as not being
            //   able to take the fast path.
            //
            // So it's actually only the first thing that's true.

            // Crude indicator of being on indexer thread, as the typechecker thread always
            // calls us with an empty map of evictedFiles
            ENFORCE(!evictedFiles.empty());
            return result;
        }
    }

    return result;
}

namespace {
const UnorderedMap<core::FileRef, shared_ptr<core::File>> EMPTY_CONST_MAP;

}

LSPFileUpdates::FastPathFilesToTypecheckResult
LSPFileUpdates::fastPathFilesToTypecheck(const core::GlobalState &gs, const LSPConfiguration &config,
                                         const vector<shared_ptr<core::File>> &updatedFiles) {
    return fastPathFilesToTypecheck(gs, config, updatedFiles, EMPTY_CONST_MAP);
}

LSPFileUpdates::FastPathFilesToTypecheckResult
LSPFileUpdates::fastPathFilesToTypecheck(const core::GlobalState &gs, const LSPConfiguration &config) const {
    return fastPathFilesToTypecheck(gs, config, this->updatedFiles);
}

} // namespace sorbet::realmain::lsp
