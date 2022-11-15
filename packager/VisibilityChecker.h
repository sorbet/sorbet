#ifndef PACKAGER_VISIBILITY_CHECKER_H
#define PACKAGER_VISIBILITY_CHECKER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet::packager {

class VisibilityChecker final {
    ~VisibilityChecker() = default;

public:
    static std::vector<ast::ParsedFile> run(core::GlobalState &gs, WorkerPool &workers,
                                            std::vector<ast::ParsedFile> files);
};

} // namespace sorbet::packager

#endif
