#include "parser.h"
#include "Builder.h"
#include "common/StableStringStorage.h"
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

    static void maybeAddAutocorrect(core::GlobalState &gs, core::ErrorBuilder &e, core::Loc loc,
                                    ruby_parser::dclass errorClass) {
        switch (errorClass) {
            case ruby_parser::dclass::IfInsteadOfItForTest:
                e.replaceWith("Replace with `it`", loc, "it");
                break;
            case ruby_parser::dclass::MissingCommaBetweenKwargs:
                e.replaceWith("Insert a comma", loc, ", ");
                break;
            case ruby_parser::dclass::CurlyBracesAroundBlockPass: {
                auto source = loc.source(gs);
                if (source.has_value()) {
                    auto replacement = string(source.value());
                    size_t lCurlyPos = replacement.find("{");
                    size_t rCurlyPos = replacement.rfind("}");
                    if (lCurlyPos != string::npos && rCurlyPos != string::npos) {
                        replacement[lCurlyPos] = '(';
                        replacement[rCurlyPos] = ')';
                        e.replaceWith("Replace the curly braces with parens", loc, replacement);
                    }
                }
                break;
            }
            default:
                break;
        }
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
                maybeAddAutocorrect(gs, e, loc, diag.error_class());
            }
        }
    }
};

unique_ptr<Node> Parser::run(sorbet::core::GlobalState &gs, core::FileRef file, Parser::Settings settings,
                             std::vector<std::string> initialLocals) {
    Builder builder(gs, file);
    auto source = file.data(gs).source();
    // The lexer requires that its buffers end with a null terminator, which core::File
    // does not guarantee.  Parsing heredocs for some mysterious reason requires two.
    string buffer;
    buffer.reserve(source.size() + 2);
    buffer += source;
    buffer += "\0\0"sv;
    StableStringStorage<> scratch;
    unique_ptr<ruby_parser::base_driver> driver;
    if (settings.traceParser) {
        driver =
            make_unique<ruby_parser::typedruby_debug27>(buffer, scratch, Builder::interface, !!settings.traceLexer);
    } else {
        driver =
            make_unique<ruby_parser::typedruby_release27>(buffer, scratch, Builder::interface, !!settings.traceLexer);
    }

    for (string local : initialLocals) {
        driver->lex.declare(local);
    }

    auto ast = unique_ptr<Node>(builder.build(driver.get(), settings.traceParser));
    ErrorToError::run(gs, file, driver->diagnostics);

    if (!ast) {
        core::LocOffsets loc{0, 0};
        NodeVec empty;
        return make_unique<Begin>(loc, std::move(empty));
    }

    return ast;
}
}; // namespace sorbet::parser
