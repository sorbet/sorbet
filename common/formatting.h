#ifndef SORBET_COMMON_FORMATTING_HPP
#define SORBET_COMMON_FORMATTING_HPP

#include "common/common.h"
#include "spdlog/fmt/fmt.h"

namespace fmt {
template <typename It, typename Char, class UnaryOp, typename UnaryOpResult> struct arg_map_join {
    It begin;
    It end;
    basic_string_view<Char> sep;
    const UnaryOp &mapper;

    arg_map_join(It begin, It end, basic_string_view<Char> sep, const UnaryOp &mapper)
        : begin(begin), end(end), sep(sep), mapper(mapper) {}
};

template <typename It, typename Char, class UnaryOp, typename UnaryOpResult>
struct formatter<arg_map_join<It, Char, UnaryOp, UnaryOpResult>, Char> : formatter<UnaryOpResult, Char> {
    template <typename FormatContext>
    auto format(const arg_map_join<It, Char, UnaryOp, UnaryOpResult> &value, FormatContext &ctx)
        -> decltype(ctx.out()) {
        using base = formatter<UnaryOpResult, Char>;

        auto it = value.begin;
        auto out = ctx.out();
        if (it != value.end) {
            out = base::format(std::invoke(value.mapper, *it++), ctx);
            while (it != value.end) {
                out = std::copy(value.sep.begin(), value.sep.end(), out);
                ctx.advance_to(out);
                out = base::format(std::invoke(value.mapper, *it++), ctx);
            }
        }
        return out;
    }
};

template <typename It, class UnaryOp> auto map_join(It begin, It end, std::string_view sep, const UnaryOp &mapper) {
    return arg_map_join<It, char, UnaryOp,
                        typename std::invoke_result<UnaryOp, typename std::iterator_traits<It>::value_type>::type>(
        begin, end, sep, mapper);
}
template <typename Container, class UnaryOp>
auto map_join(const Container &collection, std::string_view sep, const UnaryOp &mapper) {
    return arg_map_join<
        typename Container::const_iterator, char, UnaryOp,
        typename std::invoke_result<
            UnaryOp, typename std::iterator_traits<typename Container::const_iterator>::value_type>::type>(
        collection.begin(), collection.end(), sep, mapper);
}
} // namespace fmt

#endif
