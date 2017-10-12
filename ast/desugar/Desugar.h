//
// Created by Dmitry Petrashko on 10/11/17.
//

#ifndef SRUBY_DESUGAR_H
#define SRUBY_DESUGAR_H

#include "ast/ast.h"
#include "parser/parser.h"
#include <memory>

namespace ruby_typer {
namespace ast {
namespace desugar {

std::unique_ptr<Stat> node2Tree(Context ctx, parser::Node *what);
} // namespace desugar
} // namespace ast
} // namespace ruby_typer

#endif // SRUBY_DESUGAR_H
