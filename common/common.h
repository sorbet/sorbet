#ifndef SORBET_COMMON_HPP
#define SORBET_COMMON_HPP

#if __cplusplus < 201402L
#define STRINGIZE(x) "C++ = " #x
#define SSTRINGIZE(x) STRINGIZE(x)
#pragma message(SSTRINGIZE(__cplusplus))
static_assert(false, "Need c++14 to compile this codebase");
#endif

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/container/inlined_vector.h"
#include "sorbet_version/sorbet_version.h"
#include "spdlog/spdlog.h"
#include <stdint.h>
#include <string>
#include <string_view>
#include <type_traits>

namespace sorbet {

template <class T, size_t N> using InlinedVector = absl::InlinedVector<T, N>;
template <class K, class V> using UnorderedMap = absl::flat_hash_map<K, V>;
template <class E> using UnorderedSet = absl::flat_hash_set<E>;
// Uncomment to make vectors debuggable
// template <class T, size_t N> using InlinedVector = std::vector<T>;

// Wraps input in double quotes. https://stackoverflow.com/a/6671729
#define Q(x) #x
#define QUOTED(x) Q(x)

#define _MAYBE_ADD_COMMA(...) , ##__VA_ARGS__

// Used for cases like https://xkcd.com/2200/
// where there is some assumption that you believe should always hold.
// Please use this to explicitly write down what assumptions was the code written under.
// One day they might be violated and you'll help the next person debug the issue.
#define ENFORCE(x, ...)                                                                                           \
    do {                                                                                                          \
        if (::sorbet::debug_mode) {                                                                               \
            auto __enforceTimer =                                                                                 \
                ::sorbet::Timer(*(::spdlog::default_logger_raw()), "ENFORCE(" __FILE__ ":" QUOTED(__LINE__) ")"); \
            if (!(x)) {                                                                                           \
                ::sorbet::Exception::failInFuzzer();                                                              \
                if (stopInDebugger()) {                                                                           \
                    (void)!(x);                                                                                   \
                }                                                                                                 \
                ::sorbet::Exception::enforce_handler(#x, __FILE__, __LINE__ _MAYBE_ADD_COMMA(__VA_ARGS__));       \
            }                                                                                                     \
        }                                                                                                         \
    } while (false);

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
using u1 = uint8_t;
CheckSize(u1, 1, 1);

using u2 = uint16_t;
CheckSize(u2, 2, 2);

using u4 = uint32_t;
CheckSize(u4, 4, 4);

using u8 = uint64_t;
CheckSize(u8, 8, 8);

template <class From, class To> To *fast_cast(From *what) {
    constexpr bool isFinal = std::is_final<To>::value;
    if (std::is_same<From, To>::value)
        return static_cast<To *>(what);
    if (what == nullptr) {
        return nullptr;
    }
    if (isFinal) {
        From &nonNull = *what;
        const std::type_info &ty = typeid(nonNull);
        if (ty == typeid(To))
            return static_cast<To *>(what);
        return nullptr;
    }
    return dynamic_cast<To *>(what);
};

} // namespace sorbet

std::string demangle(const char *mangled);

/* use fast_sort */
#pragma GCC poison sort c_sort

/* use absl::c_ alternatives */
#pragma GCC poison any_of find_if linear_search min_element max_element iota all_of
// I wish I could add replace and find, but those names are too generic
//         accumulate upper_bound are used by <random>
//         lower_bound is needed for parser

/* String handling functions. Use C++ alternatives */
#pragma GCC poison strcpy wcscpy stpcpy wcpcpy
#pragma GCC poison strdup
#pragma GCC poison gets puts
#pragma GCC poison strcat wcscat
#pragma GCC poison wcrtomb wctob
#pragma GCC poison sprintf vsprintf vfprintf
#pragma GCC poison asprintf vasprintf
#pragma GCC poison strncpy wcsncpy
#pragma GCC poison strtok wcstok

/* Signal related */
#pragma GCC poison longjmp siglongjmp
#pragma GCC poison setjmp sigsetjmp

/* File API's */
#pragma GCC poison tmpnam tempnam

/* Misc */
#pragma GCC poison cuserid
#pragma GCC poison rexec rexec_af

#include "Exception.h"
#include "Timer.h"
#endif
