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

void Result::emit_diagnostics_as_errors(GlobalState &gs, FileRef file) {
    auto &diagnostics = impl_->driver.diagnostics;
    if (diagnostics.size() == 0) {
        return;
    }

    for (auto &diag : diagnostics) {
        std::string level = "unknown";
        switch (diag.level()) {
            case ruby_parser::dlevel::NOTE:
                level = "Note";
                break;
            case ruby_parser::dlevel::WARNING:
                level = "Warning";
                break;
            case ruby_parser::dlevel::ERROR:
                level = "Error";
                break;
            case ruby_parser::dlevel::FATAL:
                level = "Fatal";
                break;
        }
        std::string msg("Parse {}: ");
        msg.append(ruby_parser::dclass_strings[(int)diag.error_class()]);
        Loc loc(file, diag.location().begin_pos - 1, diag.location().end_pos - 1);
        gs.errors.error(loc, ast::ErrorClass::ParserError, msg, level, diag.data());
    }
}

unique_ptr<Node> &Result::ast() {
    return impl_->node;
}

unique_ptr<Node> Result::release() {
    return move(ast());
}

FileRef Result::file() {
    return impl_->file;
}

Result::Result(unique_ptr<Impl> &&impl) : impl_(move(impl)) {}
Result::~Result() {}

Result parse_ruby(GlobalState &gs, const string &path, const string &src) {
    FileRef file = gs.enterFile(path, src);
    unique_ptr<Result::Impl> impl(new Result::Impl(file, src));
    Result result(move(impl));

    Builder builder(gs, result);
    result.impl_->node = unique_ptr<Node>(builder.build(&result.impl_->driver));

    result.emit_diagnostics_as_errors(gs, file);

    return result;
}

Result::Result(Result &&other) : impl_(move(other.impl_)) {
    other.impl_ = nullptr;
}
}; // namespace parser
}; // namespace ruby_typer
