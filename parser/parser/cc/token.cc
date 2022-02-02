#include <map>
#include <ruby_parser/token.hh>

using namespace ruby_parser;

token::token(token_type type, size_t start, size_t end, std::string_view str)
    : _type(type), _start(start), _end(end), _string(str) {}

token_type token::type() const {
    return _type;
}

size_t token::start() const {
    return _start;
}

size_t token::end() const {
    return _end;
}

void token::setEnd(size_t end) {
    this->_end = end;
}

std::string_view token::view() const {
    return _string;
}

std::string token::asString() const {
    return std::string(_string);
}

std::ostream &operator<<(std::ostream &o, const ruby_parser::token &token) {
    return o << "ruby_parser::token{.start = " << token.start() << ", .end = " << token.end()
             << ", .type = " << ruby_parser::token::tokenTypeName(token.type()) << ", .str = \"" << token.view()
             << "\" }";
}
