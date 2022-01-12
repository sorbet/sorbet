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
    static uint32_t translatePos(size_t pos, uint32_t maxOff) {
        if (pos == 0) {
            return pos;
        }
        return min((uint32_t)(pos), maxOff);
    }

public:
    static void run(core::GlobalState &gs, core::FileRef file, ruby_parser::diagnostics_t diagnostics) {
        if (diagnostics.empty()) {
            return;
        }
        uint32_t maxOff = file.data(gs).source().size();
        file.data(gs).setHasParseErrors(true);
        for (auto &diag : diagnostics) {
            switch (diag.level()) {
                case ruby_parser::dlevel::NOTE:
                case ruby_parser::dlevel::WARNING:
                    continue;
                case ruby_parser::dlevel::ERROR:
                case ruby_parser::dlevel::FATAL:
                    break;
            }
            core::Loc loc(file, translatePos(diag.location().beginPos, maxOff - 1),
                          translatePos(diag.location().endPos, maxOff));
            if (auto e = gs.beginError(loc, core::errors::Parser::ParserError)) {
                e.setHeader("{}",
                            fmt::vformat(dclassStrings[(int)diag.error_class()], fmt::make_format_args(diag.data())));
            }
        }
    }
};

unique_ptr<Node> Parser::run(sorbet::core::GlobalState &gs, core::FileRef file, bool trace,
                             std::vector<std::string> initialLocals) {
    Builder builder(gs, file);
    auto source = file.data(gs).source();
    // The lexer requires that its buffers end with a null terminator, which core::File
    // does not guarantee.  Parsing heredocs for some mysterious reason requires two.
    string buffer;
    buffer.reserve(source.size() + 2);
    buffer += source;
    buffer += "\0\0"sv;
    vector<char> scratch;
    // This is perhaps a little optimistic, but at least it is a reasonable default.
    scratch.reserve(buffer.size());
    ruby_parser::typedruby27 driver(buffer, scratch, Builder::interface);

    for (string local : initialLocals) {
        driver.lex.declare(local);
    }

    auto ast = unique_ptr<Node>(builder.build(&driver, trace));
    ErrorToError::run(gs, file, driver.diagnostics);

    if (!ast) {
        core::LocOffsets loc{0, 0};
        NodeVec empty;
        return make_unique<Begin>(loc, std::move(empty));
    }

    return ast;
}
}; // namespace sorbet::parser
