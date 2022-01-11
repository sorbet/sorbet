#include <ruby_parser/token.hh>
#include <map>

using namespace ruby_parser;

token::token(token_type type, size_t start, size_t end, std::string_view str)
    : _type(type), _start(start), _end(end), _string(str)
{}

token_type token::type() const {
  return _type;
}

size_t token::start() const {
  return _start;
}

size_t token::end() const {
  return _end;
}

const std::string& token::string() const {
  return _string;
}
