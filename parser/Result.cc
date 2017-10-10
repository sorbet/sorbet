#include "parser/Result.h"
#include "parser/Builder.h"
#include "parser/Node.h"

#include "ruby_parser/driver.hh"

namespace ruby_typer {
namespace parser {

using ruby_typer::ast::ContextBase;
using std::string;
using std::unique_ptr;

class Result::Impl {
public:
    Impl(const string &src) : driver(src, Builder::interface) {}

    ruby_parser::typedruby24 driver;
    unique_ptr<Node> node;
};

const ruby_parser::diagnostics_t &Result::diagnostics() {
    return impl_->driver.diagnostics;
}

Node *Result::ast() {
    return impl_->node.get();
}

Result::Result(std::unique_ptr<Result::Impl> &&impl) : impl_(std::move(impl)) {}
Result::~Result() {}

Result parse_ruby(ContextBase &ctx, const string &src) {
    unique_ptr<Result::Impl> impl(new Result::Impl(src));
    Result result(std::move(impl));

    Builder builder(ctx, result);
    result.impl_->node = unique_ptr<Node>(builder.build(&result.impl_->driver));

    return result;
}

ruby_typer::parser::Result::Result(Result &&) {}
}; // namespace parser
}; // namespace ruby_typer
