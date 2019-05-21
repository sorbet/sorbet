#ifndef SORBET_FLATTEN_H
#define SORBET_FLATTEN_H

#include "ast/ast.h"

namespace sorbet::flatten {

    std::vector<ast::ParsedFile> run(core::MutableContext ctx, std::vector<ast::ParsedFile> trees);

} // namespace sorbet::flatten

#endif
