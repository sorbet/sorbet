#ifndef RUBY_PARSER_LOCATION_HH
#define RUBY_PARSER_LOCATION_HH

#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdint.h>

#include "token.hh"

namespace ruby_parser {

struct location {
    token_t begin;
    token_t end;

    location() : begin(nullptr), end(nullptr) {}
    location(token_t begin, token_t end) : begin(begin), end(end) {}

    location(const location &other) = default;
    location &operator=(const location &other) = default;

    bool exists() const {
        return begin != nullptr && end != nullptr;
    };

    size_t beginPos() const {
        assert(exists());
        return begin->start();
    }

    size_t endPos() const {
        assert(exists());
        return end->end();
    }
};

} // namespace ruby_parser

std::ostream &operator<<(std::ostream &o, const ruby_parser::location &location);

#endif
