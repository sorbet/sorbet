#ifndef RUBY_PARSER_LOCATION_HH
#define RUBY_PARSER_LOCATION_HH

#include <cstddef>
#include <iostream>
#include <stdint.h>

namespace ruby_parser {

// TODO(jez) I think that I only need line information for tokens.
// struct location can be used for nonterminals as well, which is weird.
struct location {
    size_t begin;
    size_t end;

    location() : begin(SIZE_MAX), end(SIZE_MAX) {}
    location(size_t begin, size_t end) : begin(begin), end(end) {}

    location(const location &other) = default;
    location &operator=(const location &other) = default;

    bool exists() {
        return begin != SIZE_MAX && end != SIZE_MAX;
    };
};

} // namespace ruby_parser

std::ostream &operator<<(std::ostream &o, const ruby_parser::location &location);

#endif
