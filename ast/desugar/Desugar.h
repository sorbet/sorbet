#ifndef SORBET_DESUGAR_H
#define SORBET_DESUGAR_H

#include "ast/ast.h"
#include "parser/parser.h"
#include <memory>

namespace sorbet::ast::desugar {

TreePtr node2Tree(core::MutableContext ctx, std::unique_ptr<parser::Node> what);
} // namespace sorbet::ast::desugar

#endif // SORBET_DESUGAR_H
