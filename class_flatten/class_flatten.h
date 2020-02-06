#ifndef SORBET_FLATTEN_H
#define SORBET_FLATTEN_H

#include "ast/ast.h"

namespace sorbet::class_flatten {

ast::ParsedFile runOne(core::Context ctx, ast::ParsedFile trees);

} // namespace sorbet::class_flatten

#endif
