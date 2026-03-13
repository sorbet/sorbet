#ifndef SORBET_AST_DESUGAR_PRISM_TRANSLATOR_H
#define SORBET_AST_DESUGAR_PRISM_TRANSLATOR_H

#include "parser/prism/Parser.h"

// Any changes made here should also be reflected in the original parser variant in `ast/desugar/Desugar.h`
namespace sorbet::ast::Desugar::Prism {

using sorbet::parser::Prism::ParseResult;

// The public entry point for desugaring a Prism parse tree into a Sorbet expression tree.
// preserveConcreteSyntax is used to skip some of desugarings, to aid in implementation of the Extract to Variable code
// action. It should not be used elsewhere.
ast::ExpressionPtr node2Tree(core::MutableContext ctx, ParseResult parseResult, bool preserveConcreteSyntax = false);

} // namespace sorbet::ast::Desugar::Prism
#endif // SORBET_AST_DESUGAR_PRISM_TRANSLATOR_H
