#ifndef PACKAGER_VISIBILITY_CHECKER_H
#define PACKAGER_VISIBILITY_CHECKER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet::packager {

class VisibilityChecker final {
    ~VisibilityChecker() = default;

public:
    // With `recordReferences`, also records for every file which symbols and packages it references
    // (`GlobalState::setSymbolsReferencedByFile`, `PackageInfo::trackPackageReferences`). Only `--gen-packages` and the
    // language server's "add missing export" code action read those records, and they cost a few hundred bytes per
    // file for the rest of the run, so a batch typecheck that is not generating packages leaves them empty.
    static void run(core::GlobalState &gs, WorkerPool &workers, absl::Span<const ast::ParsedFile> files,
                    bool recordReferences);
};

} // namespace sorbet::packager

#endif
