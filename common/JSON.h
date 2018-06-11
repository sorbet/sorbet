#ifndef SORBET_JSON_H
#define SORBET_JSON_H

#include <string>

namespace sorbet {
namespace core {

struct JSON {
    static std::string escape(std::string from);
};

} // namespace core
} // namespace sorbet
#endif // SORBET_JSON
