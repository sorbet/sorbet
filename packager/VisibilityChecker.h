#ifndef PACKAGER_VISIBILITY_CHECKER_H
#define PACKAGER_VISIBILITY_CHECKER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet::packager {

class VisibilityChecker final {
    ~VisibilityChecker() = default;

public:
    static void run(core::GlobalState &gs, WorkerPool &workers, absl::Span<const ast::ParsedFile> files);
};

} // namespace sorbet::packager

#endif
