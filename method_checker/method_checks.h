#ifndef SORBET_METHOD_CHECKS_H
#define SORBET_METHOD_CHECKS_H

#include "ast/ast.h"
#include "core/core.h"

#include <vector>

namespace sorbet::method_checks {

void validateSymbols(core::GlobalState &gs);

ast::ParsedFile validateSymbolsOne(core::Context ctx, ast::ParsedFile tree);
std::vector<ast::ParsedFile> validateSymbolsTwo(core::Context ctx, std::vector<ast::ParsedFile> trees);

} // namespace sorbet::method_checks

#endif
