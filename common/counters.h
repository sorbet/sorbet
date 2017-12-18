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

    ConstExprStr(const std::string s) : str(s.c_str()), size(s.size()) {}
};

void counterInc(ConstExprStr counter);
void counterAdd(ConstExprStr counter, unsigned int value);
void categoryCounterInc(ConstExprStr category, ConstExprStr counter);
void categoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned int value);
void histogramInc(ConstExprStr histogram, int key);
void histogramAdd(ConstExprStr histogram, int key, unsigned int value);
std::string getCounterStatistics();
bool submitCountersToStatsd(std::string host, int port, std::string prefix);
} // namespace ruby_typer
#endif // SRUBY_COUNTERS_H
