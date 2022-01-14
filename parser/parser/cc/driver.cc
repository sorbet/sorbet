#include <ruby_parser/driver.hh>
#include <ruby_parser/lexer.hh>

// Autogenerated code
#include "parser/parser/typedruby_bison.h"

namespace ruby_parser {

base_driver::base_driver(ruby_version version, std::string_view source, sorbet::StableStringStorage<> &scratch,
                         const struct builder &builder)
    : build(builder), lex(diagnostics, version, source, scratch), pending_error(false), def_level(0), ast(nullptr) {}

typedruby27::typedruby27(std::string_view source, sorbet::StableStringStorage<> &scratch, const struct builder &builder)
    : base_driver(ruby_version::RUBY_27, source, scratch, builder) {}

ForeignPtr typedruby27::parse(SelfPtr self, bool trace) {
    bison::typedruby27::parser p(*this, self);
#if DEBUG_MODE
    p.set_debug_level(trace ? 1 : 0);
#endif
    p.parse();
    return ast;
}

} // namespace ruby_parser
