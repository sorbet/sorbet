#ifndef SRUBY_COUNTERS_H
#define SRUBY_COUNTERS_H

#include "common.h"
namespace ruby_typer {
struct ConstExprStr {
    char const *str;
    const std::size_t size;

    // can only construct from a char[] literal
    template <std::size_t N>
    constexpr ConstExprStr(char const (&s)[N])
        : str(s), size(N - 1) // not count the trailing nul
    {}
};

void counterInc(ConstExprStr counter);
void categoryCounterInc(ConstExprStr category, ConstExprStr counter);
void histogramInc(ConstExprStr histogram, int value);
std::string getCounterStatistics();
} // namespace ruby_typer
#endif // SRUBY_COUNTERS_H
