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

namespace {

class ErrorToError {
    static uint32_t translatePos(size_t pos, uint32_t maxOff) {
        if (pos == 0) {
            return pos;
        }
        return min((uint32_t)(pos), maxOff);
    }

    static core::ErrorClass dclassToErrorClass(ruby_parser::dclass diagClass) {
        switch (diagClass) {
            case ruby_parser::dclass::DedentedEnd:
                return core::errors::Parser::ErrorRecoveryHint;
            default:
                return core::errors::Parser::ParserError;
        }
    }

    static void explainError(core::GlobalState &gs, core::ErrorBuilder &e, core::Loc loc,
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
            case ruby_parser::dclass::DedentedEnd:
                e.addErrorNote("Sorbet found a syntax error it could not recover from.\n"
                               "    To provide a better message, it re-parsed the file while tracking indentation.\n"
                               "    If this message obscures the root cause, please let us know.");
                break;
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
            if (auto e = gs.beginError(loc, dclassToErrorClass(diag.error_class()))) {
                e.setHeader("{}",
                            fmt::vformat(dclassStrings[(int)diag.error_class()], fmt::make_format_args(diag.data())));
                explainError(gs, e, loc, diag.error_class());
            }
        }
    }
};

unique_ptr<ruby_parser::base_driver> makeDriver(Parser::Settings settings, string_view buffer,
                                                StableStringStorage<> &scratch, const vector<string> &initialLocals) {
    unique_ptr<ruby_parser::base_driver> driver;
    if (settings.traceParser) {
        driver = make_unique<ruby_parser::typedruby_debug27>(buffer, scratch, Builder::interface, !!settings.traceLexer,
                                                             !!settings.indentationAware);
    } else {
        driver = make_unique<ruby_parser::typedruby_release27>(buffer, scratch, Builder::interface,
                                                               !!settings.traceLexer, !!settings.indentationAware);
    }

    for (string local : initialLocals) {
        driver->lex.declare(local);
    }

    return driver;
}

} // namespace

unique_ptr<Node> Parser::run(core::GlobalState &gs, core::FileRef file, Parser::Settings settings,
                             vector<string> initialLocals) {
    // Marked `const` so that we can re-use across multiple `build()` invocations
    const Builder builder(gs, file);

    // `buffer` and `scratch` can also be shared by both drivers
    auto source = file.data(gs).source();
    // The lexer requires that its buffers end with a null terminator, which core::File
    // does not guarantee.  Parsing heredocs for some mysterious reason requires two.
    string buffer;
    buffer.reserve(source.size() + 2);
    buffer += source;
    buffer += "\0\0"sv;
    StableStringStorage<> scratch;

    auto driver = makeDriver(settings, buffer, scratch, initialLocals);
    auto ast = builder.build(driver.get(), settings.traceParser);
    if (ast != nullptr) {
        // Successful parse on first try
        ErrorToError::run(gs, file, driver->diagnostics);
        return ast;
    }

    auto driverRetry = makeDriver(settings.withIndentationAware(), buffer, scratch, initialLocals);
    auto astRetry = builder.build(driverRetry.get(), settings.traceParser);

    if (astRetry == nullptr) {
        // Retry did not produce a parse result; flush original errors and make empty parse result
        ErrorToError::run(gs, file, driver->diagnostics);
        return make_unique<Begin>(core::LocOffsets{0, 0}, NodeVec{});
    }

    // Make sure that at least one error is printed. Probably doesn't make sense to show errors from
    // BOTH runs (could be confusing and/or report the same error(s) twice). Probably if the second
    // run produced a parse and it also has errors, they're more relevant, so show those.
    if (!driverRetry->diagnostics.empty()) {
        ErrorToError::run(gs, file, driverRetry->diagnostics);
        return astRetry;
    }

    ENFORCE(false, "Error-recovery mode of the parser should always emit an error if it produced a parse result.");
    // Report original run's set of errors, but still use the second parse.
    ErrorToError::run(gs, file, driver->diagnostics);
    return astRetry;
}
}; // namespace sorbet::parser
