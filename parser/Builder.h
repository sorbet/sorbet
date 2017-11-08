#ifndef SRUBY_PARSER_BUILDER_H
#define SRUBY_PARSER_BUILDER_H

#include "ruby_parser/builder.hh"
#include "ruby_parser/driver.hh"

#include "ast/Context.h"

#include <memory>

namespace ruby_typer {
namespace parser {

class Node;

class Builder {
public:
    Builder(ruby_typer::ast::GlobalState &gs, ruby_typer::ast::FileRef file);
    ~Builder();

    static ruby_parser::builder interface;

    std::unique_ptr<Node> build(ruby_parser::base_driver *driver);

    class Impl;

private:
    std::unique_ptr<Impl> impl_;
};
}; // namespace parser
}; // namespace ruby_typer

#endif
