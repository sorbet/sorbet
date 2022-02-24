#include <ruby_parser/driver.hh>
#include <ruby_parser/lexer.hh>

// Autogenerated code
#include "parser/parser/typedruby_debug_bison.h"
#include "parser/parser/typedruby_release_bison.h"

namespace ruby_parser {

base_driver::base_driver(ruby_version version, std::string_view source, sorbet::StableStringStorage<> &scratch,
                         const struct builder &builder, bool traceLexer, bool indentationAware)
    : build(builder), lex(diagnostics, version, source, scratch, traceLexer), pending_error(false), def_level(0),
      ast(nullptr), indentationAware(indentationAware) {}

const char *const base_driver::token_name(token_type type) {
    // We have several tokens that have the same human-readable string, but Bison won't
    // let us specify the same human-readable string for both of them (because bison
    // lets you use those names like ".." directory in production rules, which would
    // lead to ambiguity.
    //
    // Instead, we have this translation layer, which intercepts certain tokens and
    // displays their proper human-readable string.
    switch (type) {
        case token_type::tBDOT2:
            return "\"..\"";
        case token_type::tBDOT3:
            return "\"...\"";
        case token_type::tBACK_REF:
            return "\"`\"";
        case token_type::tAMPER2:
            return "\"&\"";
        case token_type::tSTAR2:
            return "\"*\"";
        case token_type::tLBRACK2:
            return "\"[\"";
        case token_type::tLPAREN2:
            return "\"(\"";
        case token_type::tCOLON3:
            return "\"::\"";
        case token_type::tPOW:
            return "\"**\"";
        case token_type::tUPLUS:
            return "\"unary +\"";
        case token_type::tUMINUS:
            return "\"unary -\"";
        default:
            return this->yytname[this->yytranslate(static_cast<int>(type))];
    }
}

void base_driver::rewind_and_reset(size_t newPos) {
    this->clear_lookahead();
    this->lex.rewind_and_reset_to_expr_end(newPos);
}

void base_driver::rewind_if_dedented(token_t token, token_t kEND, bool force) {
    if ((force || this->indentationAware) && this->lex.compare_indent_level(token, kEND) > 0) {
        this->rewind_and_reset(kEND->start());
        const char *token_str_name = this->token_name(token->type());
        this->diagnostics.emplace_back(dlevel::ERROR, dclass::DedentedEnd, token, token_str_name, kEND);
    }
}

typedruby_release::typedruby_release(std::string_view source, sorbet::StableStringStorage<> &scratch,
                                     const struct builder &builder, bool traceLexer, bool indentationAware)
    : base_driver(ruby_version::RUBY_31, source, scratch, builder, traceLexer, indentationAware) {}

ForeignPtr typedruby_release::parse(SelfPtr self, bool) {
    bison::typedruby_release::parser p(*this, self);
    p.parse();
    return ast;
}

typedruby_debug::typedruby_debug(std::string_view source, sorbet::StableStringStorage<> &scratch,
                                 const struct builder &builder, bool traceLexer, bool indentationAware)
    : base_driver(ruby_version::RUBY_31, source, scratch, builder, traceLexer, indentationAware) {}

ForeignPtr typedruby_debug::parse(SelfPtr self, bool traceParser) {
    bison::typedruby_debug::parser p(*this, self);
    p.set_debug_level(traceParser ? 1 : 0);
    p.parse();
    return ast;
}

} // namespace ruby_parser
