#ifndef PACKAGER_VISIBILITY_CHECKER_H
#define PACKAGER_VISIBILITY_CHECKER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet::packager {

class VisibilityChecker final {
    ~VisibilityChecker() = default;

public:
    // Set the export bits of the symbols in the packages of `files`.
    static void propagateExports(core::GlobalState &gs, absl::Span<const ast::ParsedFile> files);

    // Report references in `files` to constants that aren't visible to them.
    static void checkReferences(core::GlobalState &gs, WorkerPool &workers, absl::Span<const ast::ParsedFile> files);
};

} // namespace sorbet::packager

#endif
