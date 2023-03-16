#ifndef SORBET_COMMON_HPP
#define SORBET_COMMON_HPP

#if __cplusplus < 201703L
#define STRINGIZE(x) "C++ = " #x
#define SSTRINGIZE(x) STRINGIZE(x)
#pragma message(SSTRINGIZE(__cplusplus))
static_assert(false, "Need c++17 to compile this codebase");
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

// Used for cases like https://xkcd.com/2200/
// where there is some assumption that you believe should always hold.
// Please use this to explicitly write down what assumptions was the code written under.
// One day they might be violated and you'll help the next person debug the issue.
// Emits a timer so that expensive checks show up in traces in debug builds.
#define ENFORCE(...)                                                                                              \
    do {                                                                                                          \
        if (::sorbet::debug_mode) {                                                                               \
            auto __enforceTimer =                                                                                 \
                ::sorbet::Timer(*(::spdlog::default_logger_raw()), "ENFORCE(" __FILE__ ":" QUOTED(__LINE__) ")"); \
            ENFORCE_NO_TIMER(__VA_ARGS__);                                                                        \
        }                                                                                                         \
    } while (false);

#ifdef SKIP_SLOW_ENFORCE
constexpr bool skip_slow_enforce = true;
#else
constexpr bool skip_slow_enforce = false;
#endif

// Some ENFORCEs are super slow and/or don't pass on Stripe's codebase.
// Long term we should definitely make these checks pass and maybe even make them fast,
// but for now we provide a way to skip slow debug checks (for example, when compiling debug release builds)
#define SLOW_ENFORCE(...)                   \
    do {                                    \
        if (!::sorbet::skip_slow_enforce) { \
            ENFORCE(__VA_ARGS__);           \
        }                                   \
    } while (false);

#define DEBUG_ONLY(X) \
    if (debug_mode) { \
        X;            \
    }

#define SLOW_DEBUG_ONLY(X)                                      \
    if constexpr (!::sorbet::skip_slow_enforce && debug_mode) { \
        X;                                                      \
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
    [[maybe_unused]] inline void _##T##is##ExpSize##_bytes_long_() {                 \
        sorbet::check_size<T, ExpSize> UNUSED(_##T##is##ExpSize##_bytes_long);       \
        sorbet::check_align<T, ExpAlign> UNUSED(_##T##is##ExpAlign##_bytes_aligned); \
    }

template <class From, class To> To *fast_cast(From *what) {
    constexpr bool isFinal = std::is_final<To>::value;
    if (std::is_same<From, To>::value) {
        return static_cast<To *>(what);
    }
    if (what == nullptr) {
        return nullptr;
    }
    if (isFinal) {
        From &nonNull = *what;
        const std::type_info &ty = typeid(nonNull);
        if (ty == typeid(To)) {
            return static_cast<To *>(what);
        }
        return nullptr;
    }
    return dynamic_cast<To *>(what);
};

// Rounds the provided number up to the nearest power of two. If v is already a power of two, it returns v.
uint32_t nextPowerOfTwo(uint32_t v);

std::vector<int> findLineBreaks(std::string_view s);

// To get exhaustiveness checking with std::visit.
// From: https://en.cppreference.com/w/cpp/utility/variant/visit#Example
template <class> inline constexpr bool always_false_v = false;

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

#include "enforce_no_timer/EnforceNoTimer.h"
#include "exception/Exception.h"
#include "timers/Timer.h"
#endif
