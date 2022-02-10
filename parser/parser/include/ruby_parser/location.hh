#ifndef RUBY_PARSER_LOCATION_HH
#define RUBY_PARSER_LOCATION_HH

#include <cstddef>
#include <iostream>
#include <stdint.h>

namespace ruby_parser {

struct location {
    size_t begin;
    size_t end;
    size_t lineStart;

    location() : begin(SIZE_MAX), end(SIZE_MAX), lineStart(SIZE_MAX) {}
    location(size_t begin, size_t end, size_t lineStart) : begin(begin), end(end), lineStart(lineStart) {}

    location(const location &other) = default;
    location &operator=(const location &other) = default;

    bool exists() {
        return begin != SIZE_MAX && end != SIZE_MAX && lineStart != SIZE_MAX;
    };
};

} // namespace ruby_parser

std::ostream &operator<<(std::ostream &o, const ruby_parser::location &location);

#endif
