#ifndef SRUBY_JSON_H
#define SRUBY_JSON_H

#include <string>

namespace ruby_typer {
namespace core {

struct JSON {
    static std::string escape(std::string from);
};

} // namespace core
} // namespace ruby_typer
#endif // SRUBY_JSON
