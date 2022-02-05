#include <ruby_parser/driver.hh>
#include <ruby_parser/lexer.hh>

// Autogenerated code
#include "parser/parser/typedruby_debug_bison.h"
#include "parser/parser/typedruby_release_bison.h"

namespace ruby_parser {

base_driver::base_driver(ruby_version version, std::string_view source, sorbet::StableStringStorage<> &scratch,
                         const struct builder &builder, bool traceLexer)
    : build(builder), lex(diagnostics, version, source, scratch, traceLexer), pending_error(false), def_level(0),
      ast(nullptr) {}

const char *const base_driver::token_name(token_type type) {
    // We have two tokens matching `..` and `...`. Bison won't let us specify the same
    // human-readable string for both of them (because bison lets you use those names like ".."
    // directly in the productcion rules, which would lead to ambiguity).
    //
    // This hack allows to display the real token string instead of tBDOT2/tBDOT3 in parsing errors.
    switch (type) {
        case token_type::tBDOT2:
            return "\"..\"";
        case token_type::tBDOT3:
            return "\"...\"";
        default:
            return this->yytname[this->yytranslate(static_cast<int>(type))];
    }
}

void base_driver::rewind_and_reset(size_t newPos) {
    this->clear_lookahead();
    this->lex.rewind_and_reset_to_expr_end(newPos);
}

typedruby_release27::typedruby_release27(std::string_view source, sorbet::StableStringStorage<> &scratch,
                                         const struct builder &builder, bool traceLexer)
    : base_driver(ruby_version::RUBY_27, source, scratch, builder, traceLexer) {}

ForeignPtr typedruby_release27::parse(SelfPtr self, bool) {
    bison::typedruby_release27::parser p(*this, self);
    p.parse();
    return ast;
}

typedruby_debug27::typedruby_debug27(std::string_view source, sorbet::StableStringStorage<> &scratch,
                                     const struct builder &builder, bool traceLexer)
    : base_driver(ruby_version::RUBY_27, source, scratch, builder, traceLexer) {}

ForeignPtr typedruby_debug27::parse(SelfPtr self, bool traceParser) {
    bison::typedruby_debug27::parser p(*this, self);
    p.set_debug_level(traceParser ? 1 : 0);
    p.parse();
    return ast;
}

} // namespace ruby_parser
