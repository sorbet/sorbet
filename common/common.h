#ifndef SRUBY_COMMON_HPP
#define SRUBY_COMMON_HPP

#include <cstring>
#include <functional>
#include <ostream>
#include <sstream>
#include <string>

namespace ruby_typer {

#ifdef NDEBUG
constexpr bool debug_mode = false;
#else
constexpr bool debug_mode = true;
#endif

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

class File {
public:
    static std::string read(const char *filename);
};
}

std::string demangle(const char *mangled);

#include "Error.h"
#include "typecase.h"

#endif