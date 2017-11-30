#include "parser.h"
#include "../core/Loc.h"
#include "Builder.h"
#include "ruby_parser/driver.hh"
#include <algorithm>

template class std::unique_ptr<ruby_typer::parser::Node>;

namespace ruby_typer {
namespace parser {

extern const char *dclass_strings[];

using namespace std;

class DiagnosticToError {
public:
    static void run(core::GlobalState &gs, core::FileRef file, ruby_parser::diagnostics_t diagnostics) {
        if (diagnostics.empty()) {
            return;
        }
        u4 max_off = file.file(gs).source().to;

        for (auto &diag : diagnostics) {
            std::string level = "unknown";
            switch (diag.level()) {
                case ruby_parser::dlevel::NOTE:
                case ruby_parser::dlevel::WARNING:
                    continue;
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
            msg.append(dclass_strings[(int)diag.error_class()]);
            Loc loc(file, min((u4)(diag.location().begin_pos - 1), max_off),
                    min((u4)(diag.location().end_pos - 1), max_off));
            gs.errors.error(loc, core::ErrorClass::ParserError, msg, level, diag.data());
        }
    }
};

std::unique_ptr<Node> Parser::run(ruby_typer::core::GlobalState &gs, core::FileRef file) {
    Builder builder(gs, file);
    ruby_parser::typedruby24 driver(file.file(gs).source().toString(), Builder::interface);
    auto ast = unique_ptr<Node>(builder.build(&driver));
    DiagnosticToError::run(gs, file, driver.diagnostics);

    if (!ast) {
        core::Loc loc(file, 0, 0);
        NodeVec empty;
        return make_unique<Begin>(loc, move(empty));
    }

    return ast;
}

std::unique_ptr<Node> Parser::run(ruby_typer::core::GlobalState &gs, const string &path, const string &src) {
    core::FileRef file = gs.enterFile(path, src);
    return run(gs, file);
}

}; // namespace parser
}; // namespace ruby_typer
