#include "parser.h"
#include "Builder.h"
#include "ruby_parser/driver.hh"

template class std::unique_ptr<ruby_typer::parser::Node>;

namespace ruby_typer {
namespace parser {

using namespace std;

class DiagnosticToError {
public:
    static void run(ast::GlobalState &gs, ast::FileRef file, ruby_parser::diagnostics_t diagnostics) {
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
                default:
                    Error::notImplemented();
            }
            std::string msg("Parse {}: ");
            msg.append(ruby_parser::dclass_strings[(int)diag.error_class()]);
            Loc loc(file, diag.location().begin_pos - 1, diag.location().end_pos - 1);
            gs.errors.error(loc, ast::ErrorClass::ParserError, msg, level, diag.data());
        }
    }
};

std::unique_ptr<Node> Parser::run(ruby_typer::ast::GlobalState &gs, ast::FileRef file) {
    Builder builder(gs, file);
    ruby_parser::typedruby24 driver(file.file(gs).source().toString(), Builder::interface);
    auto ast = unique_ptr<Node>(builder.build(&driver));
    DiagnosticToError::run(gs, file, driver.diagnostics);

    return ast;
}

std::unique_ptr<Node> Parser::run(ruby_typer::ast::GlobalState &gs, const string &path, const string &src) {
    ast::FileRef file = gs.enterFile(path, src);
    return run(gs, file);
}

}; // namespace parser
}; // namespace ruby_typer
