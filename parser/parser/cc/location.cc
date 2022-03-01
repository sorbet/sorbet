#include "location.hh"

std::ostream &operator<<(std::ostream &o, const ruby_parser::location &location) {
    return o << "[" << location.beginPos() << "," << location.endPos() << ")";
}
