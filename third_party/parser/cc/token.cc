#include <ruby_parser/token.hh>
#include <map>

using namespace ruby_parser;

token::token(token_type type, size_t start, size_t end, const std::string& str)
    : _type(type), _start(start), _end(end), _string(str)
{}

token_type token::type() const {
  return _type;
}

const std::string& token::type_name() const {
  static std::map<int, std::string> type_names {
    #define XX(name, value) { value, #name },
    RUBY_PARSER_TOKEN_TYPES(XX)
    #undef XX
  };

  return type_names.at(static_cast<int>(type()));
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

std::ostream& operator<<(std::ostream& os, const token& tok) {
  os << tok.type_name() << "(" << tok.string() << ")";
  return os;
}
