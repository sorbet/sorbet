#ifndef SORBET_CONSTEXPRSTR_H
#define SORBET_CONSTEXPRSTR_H
#include <cstddef> // std::size_t

namespace sorbet {
struct ConstExprStr {
    char const *str;
    std::size_t size;

    // can only construct from a char[] literal
    template <std::size_t N>
    constexpr ConstExprStr(char const (&s)[N])
        : str(s), size(N - 1) // not count the trailing null
    {}

    ConstExprStr() = delete;
};
}; // namespace sorbet

#endif
