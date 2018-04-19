#ifndef SRUBY_CORE_STRICT_LEVEL_H
#define SRUBY_CORE_STRICT_LEVEL_H

namespace ruby_typer {
namespace core {
enum class StrictLevel {
    Ruby = 0,
    Typed = 1,
    Strict = 2,
    Strong = 3,
};
}
} // namespace ruby_typer

#endif
