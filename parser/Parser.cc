#include "parser.h"
#include "Builder.h"
#include "core/Loc.h"
#include "core/errors/parser.h"
#include "ruby_parser/driver.hh"
#include <algorithm>

template class std::unique_ptr<sorbet::parser::Node>;

using namespace std;

namespace sorbet::parser {

extern const char *dclassStrings[];

using namespace std;

class ErrorToError {
    static u4 translatePos(size_t pos, u4 maxOff) {
        if (pos == 0) {
            return pos;
        }
        return min((u4)(pos), maxOff);
    }

public:
    static void run(core::GlobalState &gs, core::FileRef file, ruby_parser::diagnostics_t diagnostics) {
        if (diagnostics.empty()) {
            return;
        }
        u4 maxOff = file.data(gs).source().size();

        for (auto &diag : diagnostics) {
            string_view level = "unknown"sv;
            switch (diag.level()) {
                case ruby_parser::dlevel::NOTE:
                case ruby_parser::dlevel::WARNING:
                    continue;
                case ruby_parser::dlevel::ERROR:
                    level = "Error"sv;
                    break;
                case ruby_parser::dlevel::FATAL:
                    level = "Fatal"sv;
                    break;
                default:
                    Exception::notImplemented();
            }
            string msg("Parse {}: ");
            msg.append(dclassStrings[(int)diag.error_class()]);
            core::Loc loc(file, translatePos(diag.location().beginPos, maxOff),
                          translatePos(diag.location().endPos, maxOff));
            if (auto e = gs.beginError(loc, core::errors::Parser::ParserError)) {
                e.setHeader(msg, level, diag.data());
            }
        }
    }
};

unique_ptr<Node> Parser::run(sorbet::core::GlobalState &gs, core::FileRef file) {
    Builder builder(gs, file);
    auto source = file.data(gs).source();
    ruby_parser::typedruby25 driver(string(source.begin(), source.end()), Builder::interface);
    auto ast = unique_ptr<Node>(builder.build(&driver));
    ErrorToError::run(gs, file, driver.diagnostics);

    if (!ast) {
        core::Loc loc(file, 0, 0);
        NodeVec empty;
        return make_unique<Begin>(loc, std::move(empty));
    }

    return ast;
}

unique_ptr<Node> Parser::run(sorbet::core::GlobalState &gs, string_view path, string_view src) {
    core::FileRef file = gs.enterFile(path, src);
    return run(gs, file);
}

}; // namespace sorbet::parser
