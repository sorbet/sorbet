#ifndef SORBET_COMMON_HPP
#define SORBET_COMMON_HPP

#if __cplusplus < 201402L
#define STRINGIZE(x) "C++ = " #x
#define SSTRINGIZE(x) STRINGIZE(x)
#pragma message(SSTRINGIZE(__cplusplus))
static_assert(false, "Need c++14 to compile this codebase");
#endif

#include "absl/container/inlined_vector.h"
#include "absl/strings/string_view.h"
#include "unordered_map.hpp"
#include <cstring>
#include <functional>
#include <ostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

#if !defined(NDEBUG)
// So you can use `cout` when debugging. Not included in production as it is a
// performance hit.
#include <iostream>
#endif

namespace sorbet {

template <class T, size_t N> using InlinedVector = absl::InlinedVector<T, N>;
template <class K, class V> using UnorderedMap = ska::unordered_map<K, V>;
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

template <typename ToCheck, std::size_t ExpectedSize, std::size_t RealSize = sizeof(ToCheck)> struct check_size {
    static_assert(debug_mode || ExpectedSize == RealSize, "Size is off!");
};

template <typename ToCheck, std::size_t ExpectedAlign, std::size_t RealAlign = alignof(ToCheck)> struct check_align {
    static_assert(debug_mode || ExpectedAlign == RealAlign, "Align is off!");
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
    static std::string read(const absl::string_view filename);
    static void write(const absl::string_view filename, const std::vector<sorbet::u1> &data);
    static void write(const absl::string_view filename, const absl::string_view text);
    static absl::string_view getFileName(const absl::string_view path);
    static absl::string_view getExtension(const absl::string_view path);
};

} // namespace sorbet
std::string strprintf(const char *__restrict, va_list) __attribute__((format(printf, 1, 0)));
;

std::string strprintf(const char *__restrict, ...) __attribute__((format(printf, 1, 2)));
;

std::string demangle(const char *mangled);

#include "Error.h"
#include "JSON.h"
#include "Levenstein.h"
#include "typecase.h"

#endif
