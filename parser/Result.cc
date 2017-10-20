#include "parser/Builder.h"
#include "parser/parser.h"
#include "ruby_parser/driver.hh"

namespace ruby_typer {
namespace parser {

using ruby_typer::ast::ContextBase;
using ruby_typer::ast::FileRef;
using std::string;
using std::unique_ptr;

class Result::Impl {
public:
    Impl(FileRef file, const string &src) : file(file), driver(src, Builder::interface) {}

    FileRef file;
    ruby_parser::typedruby24 driver;
    unique_ptr<Node> node;
};

const ruby_parser::diagnostics_t &Result::diagnostics() {
    return impl_->driver.diagnostics;
}

Node *Result::ast() {
    return impl_->node.get();
}

FileRef Result::file() {
    return impl_->file;
}

Result::Result(unique_ptr<Result::Impl> &&impl) : impl_(move(impl)) {}
Result::~Result() {}

Result parse_ruby(ContextBase &ctx, const std::string &path, const string &src) {
    FileRef file = ctx.enterFile(path, src);
    unique_ptr<Result::Impl> impl(new Result::Impl(file, src));
    Result result(move(impl));

    Builder builder(ctx, result);
    result.impl_->node = unique_ptr<Node>(builder.build(&result.impl_->driver));

    return result;
}

ruby_typer::parser::Result::Result(Result &&) {}
}; // namespace parser
}; // namespace ruby_typer
