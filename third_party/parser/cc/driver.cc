#include <ruby_parser/driver.hh>
#include <ruby_parser/lexer.hh>
#include "grammars/typedruby24.hh"

namespace ruby_parser {

base_driver::base_driver(ruby_version version, const std::string& source, const struct builder& builder)
	: build(builder),
	lex(diagnostics, version, source),
	pending_error(false),
	def_level(0),
	ast(nullptr)
{
}

typedruby24::typedruby24(const std::string& source, const struct builder& builder)
	: base_driver(ruby_version::RUBY_24, source, builder)
{}

foreign_ptr typedruby24::parse(self_ptr self) {
	bison::typedruby24::parser p(*this, self);
	p.parse();
	return ast;
}

}
