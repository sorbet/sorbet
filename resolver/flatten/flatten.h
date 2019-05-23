#ifndef SORBET_FLATTEN_H
#define SORBET_FLATTEN_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet::flatten {

    std::vector<ast::ParsedFile> run(core::Context ctx, std::vector<ast::ParsedFile> trees, WorkerPool &workers);
std::optional<core::Loc> extractClassInitLoc(core::Context ctx, std::unique_ptr<ast::ClassDef> &klass);

} // namespace sorbet::flatten

#endif
