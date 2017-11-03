#include "Result.h"
#include "parser/Builder.h"
#include "parser/parser.h"
#include "ruby_parser/driver.hh"

template class std::unique_ptr<ruby_typer::parser::Result::Impl>;
template class std::unique_ptr<ruby_typer::parser::Node>;

namespace ruby_typer {
namespace parser {

using ruby_typer::ast::FileRef;
using ruby_typer::ast::GlobalState;
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

Result::Result(unique_ptr<Impl> &&impl) : impl_(move(impl)) {}
Result::~Result() {}

Result parse_ruby(GlobalState &ctx, const std::string &path, const string &src) {
    FileRef file = ctx.enterFile(path, src);
    unique_ptr<Result::Impl> impl(new Result::Impl(file, src));
    Result result(move(impl));

    Builder builder(ctx, result);
    result.impl_->node = unique_ptr<Node>(builder.build(&result.impl_->driver));

    return result;
}

Result::Result(Result &&other) : impl_(move(other.impl_)) {
    other.impl_ = nullptr;
}
}; // namespace parser
}; // namespace ruby_typer
