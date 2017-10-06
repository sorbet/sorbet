#include <memory>
#include <string>

#include "ast/Context.h"

#include "ruby_parser/diagnostic.hh"

namespace sruby {
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
    const ruby_parser::diagnostics_t &diagnostics();
    Node *ast();

    ~Result();

    Result(const Result &) = delete;
    Result(Result &&) = default;

    class Impl;

private:
    Result(std::unique_ptr<Impl> &&impl);

    friend Result parse_ruby(sruby::ast::ContextBase &ctx, const std::string &src);

    std::unique_ptr<Impl> impl_;
};

Result parse_ruby(sruby::ast::ContextBase &ctx, const std::string &src);
} // namespace parser
}; // namespace sruby
