#ifndef PACKAGER_VISIBILITY_CHECKER_H
#define PACKAGER_VISIBILITY_CHECKER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet::packager {

class VisibilityChecker final {
    ~VisibilityChecker() = default;

public:
    // Report errors in the `export` lines of the resolved `__package.rb` files in `files`.
    static void reportExportErrors(const core::GlobalState &gs, absl::Span<const ast::ParsedFile> files);

    // Report references in `files` to packages that they may not reference (e.g., missing imports or layering
    // violations), and record which packages each file references.
    //
    // References to constants that another package doesn't export are reported by resolver.
    static void checkReferences(core::GlobalState &gs, WorkerPool &workers, absl::Span<const ast::ParsedFile> files);
};

} // namespace sorbet::packager

#endif
