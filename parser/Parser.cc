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

uint32_t translatePos(size_t pos, uint32_t maxOff) {
    if (pos == 0) {
        return pos;
    }
    return min((uint32_t)(pos), maxOff);
}

core::Loc rangeToLoc(const core::GlobalState &gs, core::FileRef file, const ruby_parser::diagnostic::range &range) {
    uint32_t maxOff = file.data(gs).source().size();
    core::Loc loc(file, translatePos(range.beginPos, maxOff - 1), translatePos(range.endPos, maxOff));
    return core::Loc(file, translatePos(range.beginPos, maxOff - 1), translatePos(range.endPos, maxOff));
}

core::ErrorClass dclassToErrorClass(ruby_parser::dclass diagClass) {
    switch (diagClass) {
        case ruby_parser::dclass::DedentedEnd:
            return core::errors::Parser::ErrorRecoveryHint;
        default:
            return core::errors::Parser::ParserError;
    }
}

void explainError(core::GlobalState &gs, core::FileRef file, core::ErrorBuilder &e, core::Loc loc,
                  ruby_parser::diagnostic diag) {
    switch (diag.error_class()) {
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
            ENFORCE(diag.extra_location().has_value());
            e.addErrorLine(rangeToLoc(gs, file, diag.extra_location().value()), "Matching token was here");
            e.addErrorNote("Sorbet found a syntax error it could not recover from.\n"
                           "    To provide a better message, it re-parsed the file while tracking indentation.");
            break;
        default:
            break;
    }
}

void reportDiagnostics(core::GlobalState &gs, core::FileRef file, ruby_parser::diagnostics_t diagnostics,
                       bool onlyHints) {
    for (auto &diag : diagnostics) {
        switch (diag.level()) {
            case ruby_parser::dlevel::NOTE:
            case ruby_parser::dlevel::WARNING:
                continue;
            case ruby_parser::dlevel::ERROR:
            case ruby_parser::dlevel::FATAL:
                break;
        }
        auto errorClass = dclassToErrorClass(diag.error_class());
        if (onlyHints && errorClass != core::errors::Parser::ErrorRecoveryHint) {
            continue;
        }
        auto loc = rangeToLoc(gs, file, diag.location());
        if (auto e = gs.beginError(loc, errorClass)) {
            e.setHeader("{}", fmt::vformat(dclassStrings[(int)diag.error_class()], fmt::make_format_args(diag.data())));
            explainError(gs, file, e, loc, diag);
        }
    }
}

void errorToError(core::GlobalState &gs, core::FileRef file, ruby_parser::diagnostics_t diagnostics) {
    if (diagnostics.empty()) {
        return;
    }
    file.data(gs).setHasParseErrors(true);
    auto onlyHints = false;
    reportDiagnostics(gs, file, diagnostics, onlyHints);
}

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

    // Always report the original parse errors
    errorToError(gs, file, driver->diagnostics);

    if (ast != nullptr) {
        // Successful parse on first try
        return ast;
    }

    auto driverRetry = makeDriver(settings.withIndentationAware(), buffer, scratch, initialLocals);
    auto astRetry = builder.build(driverRetry.get(), settings.traceParser);

    if (astRetry == nullptr) {
        // Retry did not produce a parse result
        return make_unique<Begin>(core::LocOffsets{0, 0}, NodeVec{});
    }

    ENFORCE(absl::c_any_of(driverRetry->diagnostics,
                           [](const auto &diag) {
                               return dclassToErrorClass(diag.error_class()) == core::errors::Parser::ErrorRecoveryHint;
                           }),
            "Error-recovery mode of the parser should always emit an error hint if it produced a parse result.");
    auto onlyHints = true;
    reportDiagnostics(gs, file, driverRetry->diagnostics, onlyHints);
    return astRetry;
}
}; // namespace sorbet::parser
