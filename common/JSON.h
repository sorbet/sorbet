#ifndef SORBET_JSON_H
#define SORBET_JSON_H

#include <string>

namespace sorbet::core {

struct JSON {
    static std::string escape(std::string from);
};

} // namespace sorbet::core
#endif // SORBET_JSON
