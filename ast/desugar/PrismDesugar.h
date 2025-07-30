#ifndef SORBET_PRISM_DESUGAR_H
#define SORBET_PRISM_DESUGAR_H

#include "ast/ast.h"
#include "parser/parser.h"
#include <memory>

// The `PrismDesugar.h` should be a near exact copy of `Desugar.h`, just with a different namespace.
namespace sorbet::ast::prismDesugar {

// preserveConcreteSyntax is used to skip some of desugarings, to aid in implementation of the Extract to Variable code
// action. It should not be used elsewhere.
ExpressionPtr node2Tree(core::MutableContext ctx, std::unique_ptr<parser::Node> what,
                        bool preserveConcreteSyntax = false);
} // namespace sorbet::ast::prismDesugar

#endif // SORBET_PRISM_DESUGAR_H
