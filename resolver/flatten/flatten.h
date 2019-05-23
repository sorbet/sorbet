#ifndef SORBET_FLATTEN_H
#define SORBET_FLATTEN_H

#include "ast/ast.h"

namespace sorbet::flatten {

ast::ParsedFile run(core::Context ctx, ast::ParsedFile tree);
std::optional<core::Loc> extractClassInitLoc(core::Context ctx, std::unique_ptr<ast::ClassDef> &klass);

} // namespace sorbet::flatten

#endif
