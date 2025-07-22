#ifndef SORBET_DESUGAR_H
#define SORBET_DESUGAR_H

#include "ast/ast.h"
#include "parser/parser.h"
#include <memory>

// Any changes made to `Desugar.h` here should also be reflected in `PrismDesugar.h`
namespace sorbet::ast::desugar {

// preserveConcreteSyntax is used to skip some of desugarings, to aid in implementation of the Extract to Variable code
// action. It should not be used elsewhere.
ExpressionPtr node2Tree(core::MutableContext ctx, std::unique_ptr<parser::Node> what,
                        bool preserveConcreteSyntax = false);
} // namespace sorbet::ast::desugar

#endif // SORBET_DESUGAR_H
