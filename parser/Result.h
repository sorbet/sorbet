#ifndef SRUBY_PARSER_RESULT_H
#define SRUBY_PARSER_RESULT_H

#include <memory>
#include <string>

#include "ast/Context.h"

#include "ruby_parser/diagnostic.hh"

namespace ruby_typer {
namespace parser {

class Node;

/*
 * parser::Result contains the result of parsing a piece of Ruby source. It is
 * the owner of all memory allocated during the parse or referenced by the
 * returned objects, and must outlive any references to the AST or diagnostics
 * information.
 */
class Result {
public:
    ast::FileRef file();
    const ruby_parser::diagnostics_t &diagnostics();
    std::unique_ptr<Node> &ast();
    std::unique_ptr<Node> release();

    ~Result();

    Result(const Result &) = delete;
    Result(Result &&other);

    class Impl;

private:
    Result(std::unique_ptr<Impl> &&impl);

    friend Result parse_ruby(ruby_typer::ast::GlobalState &gs, const std::string &path, const std::string &src);

    std::unique_ptr<Impl> impl_;
};

Result parse_ruby(ruby_typer::ast::GlobalState &gs, const std::string &path, const std::string &src);
} // namespace parser
}; // namespace ruby_typer

#endif
