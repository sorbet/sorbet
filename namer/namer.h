#ifndef SORBET_NAMER_NAMER_H
#define SORBET_NAMER_NAMER_H
#include "ast/ast.h"
#include "core/FileHash.h"
#include <memory>

namespace sorbet {
class WorkerPool;
}

namespace sorbet::namer {

class Namer final {
    [[nodiscard]] static bool
    runInternal(core::GlobalState &gs, absl::Span<ast::ParsedFile> trees, WorkerPool &workers,
                UnorderedMap<core::FileRef, std::shared_ptr<const core::FileHash>> &&oldFoundDefHashesForFiles,
                core::FoundDefHashes *foundHashesOut, std::vector<core::ClassOrModuleRef> &updatedSymbols);

public:
    // Note: foundHashes is an optional out parameter.
    //
    // Setting it to a non-nullptr requests that Namer compute a fingerprint of the FoundDefinitions
    // it found while running. (Thus, it's usually nullptr except when pipeline::nameAndResolve is called
    // for the purpose of computing a FileHash.)
    [[nodiscard]] static bool run(core::GlobalState &gs, absl::Span<ast::ParsedFile> trees, WorkerPool &workers,
                                  core::FoundDefHashes *foundHashesOut);

    // Version of Namer that accepts the old FoundDefHashes for each file to run Namer, which
    // it uses to figure out how to mutate the already-populated GlobalState into the right shape
    // when considering that only the files in `trees` were edited.
    //
    // `trees` and `foundDef` should have the same number of elements, and
    // `foundDefHashesForFiles[i]` should be the `FoundDefHashes` for `trees[i]`.
    // (Done this way, instead of using something like a `std::pair`, to avoid intermediate
    // allocations for phases that don't actually need to operate on the `FoundDefHashes`.)
    [[nodiscard]] static bool
    runIncremental(core::GlobalState &gs, absl::Span<ast::ParsedFile> trees,
                   UnorderedMap<core::FileRef, std::shared_ptr<const core::FileHash>> &&oldFoundDefHashesForFiles,
                   WorkerPool &workers, std::vector<core::ClassOrModuleRef> &updatedSymbols);

    Namer() = delete;
};

} // namespace sorbet::namer

#endif
