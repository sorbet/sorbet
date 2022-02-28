#include <cassert>
#include <iostream>
#include <map>
#include <ruby_parser/token.hh>

using namespace ruby_parser;

token::token(token_type type, size_t start, size_t end, std::string_view str, size_t lineStart)
    : _type(type), _start(start), _end(end), _string(str), _lineStart(lineStart) {
    assert((type == token_type::tNL) == (lineStart == SIZE_MAX));
}

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

size_t token::lineStart() const {
    return _lineStart;
}

std::string_view token::view() const {
    return _string;
}

std::string token::asString() const {
    return std::string(_string);
}

std::ostream &operator<<(std::ostream &o, const ruby_parser::token &token) {
    auto &res = o << "ruby_parser::token{.start = " << token.start() << ", .end = " << token.end();
    if (token.type() == ruby_parser::token_type::tNL) {
        res << ", .lineStart = SIZE_MAX";
    } else {
        res << ", .lineStart = " << token.lineStart();
    }
    return (res << ", .type = " << ruby_parser::token::tokenTypeName(token.type()) << ", .str = \"" << token.view()
                << "\" }");
}
