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

const std::string &token::asString() const {
    return _string;
}
