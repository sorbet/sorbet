#include "parser/parser.h"
#include "ruby_parser/driver.hh"

namespace sruby {
namespace parser {

extern ruby_parser::builder builder;

void parse_ruby(const std::string &src) {
  ruby_parser::typedruby24 driver(src, builder);
  driver.parse(nullptr);
}
};
};
