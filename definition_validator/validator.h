#ifndef SORBET_METHOD_CHECKS_H
#define SORBET_METHOD_CHECKS_H

#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::definition_validator {

void runOne(core::Context ctx, const ast::ParsedFile &tree);

} // namespace sorbet::definition_validator

#endif
