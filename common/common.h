#ifndef SORBET_COMMON_HPP
#define SORBET_COMMON_HPP

#if __cplusplus < 201402L
#define STRINGIZE(x) "C++ = " #x
#define SSTRINGIZE(x) STRINGIZE(x)
#pragma message(SSTRINGIZE(__cplusplus))
static_assert(false, "Need c++14 to compile this codebase");
#endif

#include <string_view>
#if !defined(EMSCRIPTEN)
#include "absl/container/inlined_vector.h"
#include "unordered_map.hpp"
#else
#include <unordered_map>
#include <unordered_set>
#include <vector>
#endif
#include "spdlog/fmt/fmt.h"
#include <cstring>
#include <functional>
#include <stdint.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <vector>

#if !defined(NDEBUG)
// So you can use `cout` when debugging. Not included in production as it is a
// performance hit.
#include <iostream>
#endif

namespace sorbet {

#if !defined(EMSCRIPTEN)
template <class T, size_t N> using InlinedVector = absl::InlinedVector<T, N>;
template <class K, class V> using UnorderedMap = ska::unordered_map<K, V>;
template <class E> using UnorderedSet = ska::unordered_set<E>;
constexpr bool emscripten_build = false;
#else
template <class T, size_t N> using InlinedVector = std::vector<T>;
template <class K, class V> using UnorderedMap = std::unordered_map<K, V>;
template <class E> using UnorderedSet = std::unordered_set<E>;
constexpr bool emscripten_build = true;
#endif
// Uncomment to make vectors debuggable
// template <class T, size_t N> using InlinedVector = std::vector<T>;

#if defined(NDEBUG) && !defined(FORCE_DEBUG)
constexpr bool debug_mode = false;
#undef DEBUG_MODE
#else
#define DEBUG_MODE
constexpr bool debug_mode = true;
#endif
#define _MAYBE_ADD_COMMA(...) , ##__VA_ARGS__
#define ENFORCE(x, ...)                                                                         \
    ((::sorbet::debug_mode && !(x)) ? ({                                                        \
        if (stopInDebugger()) {                                                                 \
            (void)!(x);                                                                         \
        }                                                                                       \
        ::sorbet::Error::enforce_handler(#x, __FILE__, __LINE__ _MAYBE_ADD_COMMA(__VA_ARGS__)); \
    })                                                                                          \
                                    : false)

#define DEBUG_ONLY(X) \
    if (debug_mode) { \
        X;            \
    }

constexpr bool skip_check_memory_layout = debug_mode || emscripten_build;

template <typename ToCheck, std::size_t ExpectedSize, std::size_t RealSize = sizeof(ToCheck)> struct check_size {
    static_assert(skip_check_memory_layout || ExpectedSize == RealSize, "Size is off!");
};

template <typename ToCheck, std::size_t ExpectedAlign, std::size_t RealAlign = alignof(ToCheck)> struct check_align {
    static_assert(skip_check_memory_layout || ExpectedAlign == RealAlign, "Align is off!");
};

#ifdef UNUSED
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_##x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#else
#define UNUSED(x) x
#endif

#define CheckSize(T, ExpSize, ExpAlign)                                              \
    inline void _##T##is##ExpSize##_bytes_long_() {                                  \
        sorbet::check_size<T, ExpSize> UNUSED(_##T##is##ExpSize##_bytes_long);       \
        sorbet::check_align<T, ExpAlign> UNUSED(_##T##is##ExpAlign##_bytes_aligned); \
    }

/**
 * Shorter aliases for unsigned ints of specified byte widths.
 */
typedef uint8_t u1;
CheckSize(u1, 1, 1);

typedef uint16_t u2;
CheckSize(u2, 2, 2);

typedef uint32_t u4;
CheckSize(u4, 4, 4);

typedef uint64_t u8;
CheckSize(u8, 8, 8);

template <class From, class To> To *fast_cast(From *what) {
    constexpr bool isFinal = std::is_final<To>::value;
    if (std::is_same<From, To>::value)
        return static_cast<To *>(what);
    if (what == nullptr) {
        return nullptr;
    }
    if (isFinal) {
        const std::type_info &ty = typeid(*what);
        if (ty == typeid(To))
            return static_cast<To *>(what);
        return nullptr;
    }
    return dynamic_cast<To *>(what);
};

class FileOps final {
public:
    static std::string read(const std::string_view filename);
    static void write(const std::string_view filename, const std::vector<sorbet::u1> &data);
    static void write(const std::string_view filename, const std::string_view text);
    static std::string_view getFileName(const std::string_view path);
    static std::string_view getExtension(const std::string_view path);
};

} // namespace sorbet

std::string demangle(const char *mangled);

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
        typedef formatter<UnaryOpResult, Char> base;

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
                        typename std::result_of<UnaryOp(typename std::iterator_traits<It>::value_type)>::type>(
        begin, end, sep, mapper);
}
} // namespace fmt

#include "Error.h"
#include "JSON.h"
#include "Levenstein.h"
#include "typecase.h"

#endif
