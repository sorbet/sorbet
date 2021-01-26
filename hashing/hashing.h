#ifndef RUBY_TYPER_HASHING_HASHING_H
#define RUBY_TYPER_HASHING_HASHING_H

#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "core/Files.h"

namespace sorbet::hashing {

/**
 * Contains logic pertaining to producing NameHashes for files, which are used in LSP mode to:
 * - Determine if a file change can be typechecked incrementally or not.
 * - Determine if a file contains a reference to a particular identifier (fast filter for _find references_).
 *
 * However, this file is not just used in the LSP package! `realmain` invokes hashing when the `--store-state` argument
 * is passed so that it can serialize file contents and their hashes to disk. This is really important for the payload
 * files embedded in Sorbet itself. As a result, this package is standalone from //main/lsp.
 */
class Hashing final {
public:
    // Computes file hashes for the given files, and stores them in the files. If supplied, attempts to retrieve hashes
    // from the key-value store. Returns 'true' if it had to compute any file hashes.
    static void computeFileHashes(const std::vector<std::shared_ptr<core::File>> &files, spdlog::logger &logger,
                                  WorkerPool &workers);
};
} // namespace sorbet::hashing

#endif