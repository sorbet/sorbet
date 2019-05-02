#ifndef SORBET_JSON_H
#define SORBET_JSON_H

#include <string>

namespace sorbet {

struct JSON {
    static std::string escape(std::string from);
};

} // namespace sorbet
#endif // SORBET_JSON
