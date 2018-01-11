#ifndef SRUBY_COMMON_HPP
#define SRUBY_COMMON_HPP

#if __cplusplus < 201402L
#define STRINGIZE(x) "C++ = " #x
#define SSTRINGIZE(x) STRINGIZE(x)
#pragma message(SSTRINGIZE(__cplusplus))
static_assert(false, "Need c++14 to compile this codebase");
#endif

#include "absl/container/inlined_vector.h"
#include "absl/strings/string_view.h"
#include <cstring>
#include <functional>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>

#if !defined(NDEBUG)
// So you can use `cout` when debugging. Not included in production as it is a
// performance hit.
#include <iostream>
#endif

namespace ruby_typer {

template <class T, size_t N> using InlinedVector = absl::InlinedVector<T, N>;
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
#define ENFORCE(x, ...)                  \
    (::ruby_typer::debug_mode && !(x) && \
     ::ruby_typer::Error::enforce_handler(#x, __FILE__, __LINE__ _MAYBE_ADD_COMMA(__VA_ARGS__)))

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

#define CheckSize(T, ExpSize, ExpAlign)                                                  \
    inline void _##T##is##ExpSize##_bytes_long_() {                                      \
        ruby_typer::check_size<T, ExpSize> UNUSED(_##T##is##ExpSize##_bytes_long);       \
        ruby_typer::check_align<T, ExpAlign> UNUSED(_##T##is##ExpAlign##_bytes_aligned); \
    }

/**
 * Represents a byte inside the Java Class File.
 * The sizeof(u1) must be equal to 1.
 */
typedef unsigned char u1;
CheckSize(u1, 1, 1);

/**
 * Represents two bytes inside the Java Class File.
 * The sizeof(u2) must be equal to 2.
 */
typedef unsigned short u2;
CheckSize(u2, 2, 2);

/**
 * Represents four bytes inside the Java Class File.
 * The sizeof(u4) must be equal to 4.
 */
typedef unsigned int u4;
CheckSize(u4, 4, 4);

/**
 * Represents eight bytes inside the Java Class File.
 * The sizeof(u8) must be equal to 8.
 */
typedef unsigned long u8;
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

class File final {
public:
    static std::string read(const char *filename);
    static void write(const char *filename, const std::vector<ruby_typer::u4> &data);
    static absl::string_view getFileName(const absl::string_view path);
    static absl::string_view getExtension(const absl::string_view path);
};

class Strings final {
public:
    static std::string escapeCString(absl::string_view what);
};
} // namespace ruby_typer
std::string strprintf(const char *__restrict, va_list) __attribute__((format(printf, 1, 0)));
;

std::string strprintf(const char *__restrict, ...) __attribute__((format(printf, 1, 2)));
;

std::string demangle(const char *mangled);

#include "Error.h"
#include "ThreadQueue.h"
#include "counters.h"
#include "typecase.h"

#endif
