#ifndef RUBY_TYPER_HASHING_HASHING_H
#define RUBY_TYPER_HASHING_HASHING_H

#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "core/Files.h"

namespace sorbet {
class OwnedKeyValueStore;
}

namespace sorbet::ast {
struct ParsedFile;
}

namespace sorbet::realmain::options {
struct Options;
}

namespace sorbet::hashing {

/**
 * Contains logic pertaining to producing ShortNameHashes for files, which are used in LSP mode to:
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
                                  WorkerPool &workers, const realmain::options::Options &opts);

    /**
     * Parses and indexes the provided files using `gs` _and_ computes their hashes.
     *
     * For each `ParsedFile`, this function:
     * - Parses and indexes the file with `gs`.
     * - Checks if the file has a hash. If not, it uses the AST for hashing purposes, which involves copying and
     * rewriting the AST to refer to NameRefs in a fresh GlobalState object. The ShortNameHash is stored in the file object.
     *
     * This procedure is more efficient than indexing and computing file hashes independently as files are parsed from
     * source text only once. If kvstore is supplied, this method attempts to fetch trees from the cache.
     *
     * Note: ASTs are returned in `FileRef` order (not input order).
     */
    static std::vector<ast::ParsedFile>
    indexAndComputeFileHashes(std::unique_ptr<core::GlobalState> &gs, const realmain::options::Options &opts,
                              spdlog::logger &logger, std::vector<core::FileRef> &files, WorkerPool &workers,
                              const std::unique_ptr<const OwnedKeyValueStore> &kvstore);
};
} // namespace sorbet::hashing

#endif
