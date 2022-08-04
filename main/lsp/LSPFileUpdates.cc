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
LSPFileUpdates::fastPathFilesToTypecheck(const core::GlobalState &gs, const LSPConfiguration &config) const {
    FastPathFilesToTypecheckResult result;
    Timer timeit(config.logger, "compute_fast_path_file_set");
    vector<core::SymbolHash> changedMethodSymbolHashes;
    vector<core::SymbolHash> changedFieldSymbolHashes;
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
        if (fref.exists()) {
            // Update to existing file on fast path
            ENFORCE(fref.data(gs).getFileHash() != nullptr);
            const auto &oldSymbolHashes = fref.data(gs).getFileHash()->localSymbolTableHashes;
            const auto &newSymbolHashes = updatedFile->getFileHash()->localSymbolTableHashes;
            const auto &oldMethodHashes = oldSymbolHashes.methodHashes;
            const auto &newMethodHashes = newSymbolHashes.methodHashes;

            if (config.opts.lspExperimentalFastPathEnabled) {
                // Find which hashes changed. Note: methodHashes are sorted, so set_difference should work.
                // This will insert two entries into `changedMethodHashes` for each changed method, but they
                // will get deduped later.
                absl::c_set_symmetric_difference(oldMethodHashes, newMethodHashes,
                                                 std::back_inserter(changedMethodSymbolHashes));
            } else {
                // Both oldHash and newHash should have the same methods, since this is the fast path!
                ENFORCE(validateIdenticalFingerprints(oldMethodHashes, newMethodHashes),
                        "definitionHash should have failed");

                // Find which hashes changed. Note: methodHashes are sorted, so set_difference should work.
                // This will insert two entries into `changedMethodHashes` for each changed method, but they
                // will get deduped later.
                absl::c_set_difference(oldMethodHashes, newMethodHashes, std::back_inserter(changedMethodSymbolHashes));
            }

            const auto &oldFieldHashes = oldSymbolHashes.staticFieldHashes;
            const auto &newFieldHashes = newSymbolHashes.staticFieldHashes;

            ENFORCE(validateIdenticalFingerprints(oldFieldHashes, newFieldHashes), "definitionHash should have failed");

            absl::c_set_difference(oldFieldHashes, newFieldHashes, std::back_inserter(changedFieldSymbolHashes));

            result.changedFiles.emplace(fref, idx);
        }
    }

    result.changedSymbolNameHashes.reserve(changedMethodSymbolHashes.size() + changedFieldSymbolHashes.size());
    absl::c_transform(changedMethodSymbolHashes, std::back_inserter(result.changedSymbolNameHashes),
                      [](const auto &symhash) { return symhash.nameHash; });
    absl::c_transform(changedFieldSymbolHashes, std::back_inserter(result.changedSymbolNameHashes),
                      [](const auto &symhash) { return symhash.nameHash; });
    core::ShortNameHash::sortAndDedupe(result.changedSymbolNameHashes);

    if (!result.changedSymbolNameHashes.empty()) {
        // ^ optimization--skip the loop over every file in the project (`gs.getFiles()`) if
        // the set of changed symbols is empty (e.g., running a completion request inside a
        // method body)
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
            vector<core::ShortNameHash> intersection;
            absl::c_set_intersection(result.changedSymbolNameHashes, oldHash.usages.nameHashes,
                                     std::back_inserter(intersection));
            if (intersection.empty()) {
                continue;
            }

            result.extraFiles.emplace_back(ref);
        }
    }

    return result;
}

} // namespace sorbet::realmain::lsp
