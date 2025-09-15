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

    // For updates, we prioritize _newer_ updates.
    UnorderedSet<string> encountered;
    for (auto &f : updatedFiles) {
        encountered.emplace(f->path());
    }

    for (auto &f : older.updatedFiles) {
        if (encountered.contains(f->path())) {
            continue;
        }
        encountered.emplace(f->path());
        updatedFiles.push_back(f);
    }
    typecheckingPath = TypecheckingPath::Slow;
}

LSPFileUpdates LSPFileUpdates::copy() const {
    LSPFileUpdates copy;
    copy.epoch = epoch;
    copy.editCount = editCount;
    copy.committedEditCount = committedEditCount;
    copy.typecheckingPath = typecheckingPath;
    copy.hasNewFiles = hasNewFiles;
    copy.updatedFiles = updatedFiles;
    copy.cancellationExpected = cancellationExpected;
    copy.preemptionsExpected = preemptionsExpected;
    return copy;
}

namespace {

// Returns `true` if the two containers share a value, and `false` otherwise. This requires that the two containers are
// sorted, which is also a requirement of using `absl::c_set_intersetion`, so this is a drop-in replacement when the
// resulting set isn't needed.
//
// This is adapted from
// https://github.com/llvm/llvm-project/blob/b89e774672678ef26baf8f94c616f43551d29428/libcxx/include/__algorithm/set_intersection.h#L47-L123
// and modified to return early when any intersection is found.
bool intersects(const vector<core::WithoutUniqueNameHash> &changed, const vector<core::WithoutUniqueNameHash> &used) {
    auto changedIt = changed.begin();
    auto changedEnd = changed.end();
    auto usedIt = used.begin();
    auto usedEnd = used.end();

    bool prevEqual = false;

    while (usedIt != usedEnd) {
        auto changedNext = std::lower_bound(changedIt, changedEnd, *usedIt);
        std::swap(changedNext, changedIt);
        bool changedEqual = changedNext == changedIt;

        // If we didn't advance `changedIt`, and the previous lower_bound call for `used` also didn't advance `usedIt`,
        // we've found a match.
        if (changedEqual && prevEqual) {
            return true;
        }
        prevEqual = changedEqual;

        if (changedIt == changedEnd) {
            break;
        }

        auto usedNext = std::lower_bound(usedIt, usedEnd, *changedIt);
        std::swap(usedNext, usedIt);
        bool usedEqual = usedNext == usedIt;

        // If we didn't advance `usedIt`, and the previous lower_bound call for `changed` also didn't advance
        // `changedIt`, we've found a match.
        if (usedEqual && prevEqual) {
            return true;
        }
        prevEqual = usedEqual;
    }

    return false;
}

// An output iterator that only writes out the `nameHash` component of a `SymbolHash`. This allows us to avoid
// materializing a vector of `WithoutUniqueNameHash` when we're only interested in the `nameHash` components.
class NameHashOutputIterator final {
public:
    using container_type = vector<core::WithoutUniqueNameHash>;

    NameHashOutputIterator(container_type &container) : container{container} {}

    NameHashOutputIterator &operator=(const core::SymbolHash &symHash) {
        container.emplace_back(symHash.nameHash);
        return *this;
    }

    NameHashOutputIterator &operator++() {
        return *this;
    }

    NameHashOutputIterator &operator*() {
        return *this;
    }

private:
    container_type &container;
};

} // namespace

LSPFileUpdates::FastPathFilesToTypecheckResult
LSPFileUpdates::fastPathFilesToTypecheck(const core::GlobalState &gs, const LSPConfiguration &config,
                                         const vector<shared_ptr<core::File>> &updatedFiles,
                                         const UnorderedMap<core::FileRef, shared_ptr<core::File>> &evictedFiles) {
    UnorderedMap<core::FileRef, size_t> changedFiles;
    vector<core::WithoutUniqueNameHash> changedSymbolNameHashes;

    FastPathFilesToTypecheckResult result;
    Timer timeit(config.logger, "compute_fast_path_file_set");
    auto idx = -1;
    for (const auto &updatedFile : updatedFiles) {
        idx++;
        auto fref = gs.findFileByPath(updatedFile->path());
        // We don't support new files on the fast path. This enforce failing indicates a bug in our fast/slow
        // path logic in LSPPreprocessor.
        ENFORCE(fref.exists());
        ENFORCE(updatedFile->getFileHash() != nullptr);
        if (config.opts.cacheSensitiveOptions.sorbetPackages && updatedFile->isPackage(gs)) {
            // Only relevant in --stripe-packages mode. Package declarations do not have method
            // hashes. Instead we rely on recomputing packages if any __package.rb source
            // changes.
            continue;
        }
        if (!fref.exists()) {
            // Defensive (?)
            continue;
        }

        // When run from the indexer, the old file will actually have been evicted from the GlobalState so that the new
        // file can be given to the pipeline to be indexed (and thus have the new hashes computed), so that the
        // `updatedFile` and `fref.data(gs)` actually are the same File object. For this case, we have to find the old
        // File's hashes in `evictedFiles`.
        //
        // When run from the typechecker, the old file will not yet have been evicted before
        // fastPathFilesToTypecheck is called (it will be evicted later on that thread, right
        // before calling into the pipeline). On that thread, `updatedFile` represents the new
        // File, and the thing in GlobalState is the old File.
        const auto &oldLocalSymbolTableHashes = evictedFiles.empty()
                                                    ? fref.data(gs).getFileHash()->localSymbolTableHashes
                                                    : evictedFiles.at(fref)->getFileHash()->localSymbolTableHashes;
        const auto &newLocalSymbolTableHashes = updatedFile->getFileHash()->localSymbolTableHashes;
        const auto &oldRetypecheckableSymbolHashes = oldLocalSymbolTableHashes.retypecheckableSymbolHashes;
        const auto &newRetypecheckableSymbolHashes = newLocalSymbolTableHashes.retypecheckableSymbolHashes;

        // Find which hashes changed. Note: retypecheckableSymbolHashes are pre-sorted, so set_difference should work.
        // This will insert two entries into `retypecheckableSymbolHashes` for each changed method, but they
        // will get deduped later.
        absl::c_set_symmetric_difference(oldRetypecheckableSymbolHashes, newRetypecheckableSymbolHashes,
                                         NameHashOutputIterator(changedSymbolNameHashes));

        changedFiles.emplace(fref, idx);
    }

    result.totalChanged = changedFiles.size();

    if (changedSymbolNameHashes.empty()) {
        // Optimization--skip the loop over every file in the project (`gs.getFiles()`) if
        // the set of changed symbols is empty (e.g., running a completion request inside a
        // method body)
        return result;
    }

    result.useIncrementalNamer = true;

    core::WithoutUniqueNameHash::sortAndDedupe(changedSymbolNameHashes);

    size_t i = 0;
    // skip idx 0 (corresponds to File that does not exist, so it contains nullptr)
    for (auto &oldFile : gs.getFiles().subspan(1)) {
        i++;

        auto ref = core::FileRef(i);
        if (changedFiles.contains(ref)) {
            continue;
        }

        if (config.opts.cacheSensitiveOptions.sorbetPackages && oldFile->isPackage(gs)) {
            continue; // See note above about --stripe-packages.
        }

        if (oldFile->isPayload()) {
            // Don't retypecheck files in the payload via incremental namer, as that might
            // cause well-known symbols to get deleted and assigned a new SymbolRef ID.
            continue;
        }

        ENFORCE(oldFile->getFileHash() != nullptr);
        if (!intersects(changedSymbolNameHashes, oldFile->getFileHash()->usages.nameHashes)) {
            continue;
        }

        result.extraFiles.emplace_back(oldFile->path());
        result.totalChanged += 1;

        if (result.totalChanged > (2 * config.opts.lspMaxFilesOnFastPath)) {
            // Short circuit, as a performance optimization.
            // (gs.getFiles() is usually 3-4 orders of magnitude larger than lspMaxFilesOnFastPath)
            //
            // The "2 * ..." is so that we can get a rough idea of whether there's an easy
            // bang-for-buck bump we could make to the threshold by reading the logs.
            //
            // One of two things could be true:
            // - We're running on the indexer thread to decide typecheckingPath, which only cares about how
            //   many extra files there are, not what they are.
            // - We're running on the typechecker thread (knowing that typecheckingPath was already
            //   TypecheckingPath::Fast) and simply need to compute the list of files to typecheck.
            //   But that would be a contradiction--because otherwise the indexer would have marked
            //   the update as not being able to take the fast path.
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

} // namespace sorbet::realmain::lsp
