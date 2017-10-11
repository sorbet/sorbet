#ifndef SRUBY_COMMON_HPP
#define SRUBY_COMMON_HPP

#include <cstring>
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
    static_assert(ExpectedSize == RealSize, "Size is off!");
};

template <typename ToCheck, std::size_t ExpectedAlign, std::size_t RealAlign = alignof(ToCheck)> struct check_align {
    static_assert(ExpectedAlign == RealAlign, "Align is off!");
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

class SRubyException {
public:
    /**
     * Creates an exception given the message and the stack trace.
     *
     * @param message contains information about exceptional situation.
     * @param stackTrace the stack trace where this exception happened.
     */
    SRubyException(const std::string &message, const std::string &stackTrace = "")
        : _message(message), _stackTrace(stackTrace) {}

    /**
     * Returns information about the exceptional situation.
     */
    inline const std::string &message() const {
        return _message;
    }

    inline const std::string &stackTrace() const {
        return _stackTrace;
    }

    /**
     * Shows this.
     */
    friend std::ostream &operator<<(std::ostream &os, const SRubyException &ex);

private:
    std::string _message;

    std::string _stackTrace;

    // std::stringstream _message;
};

class Error {
public:
    template <typename... TArgs>[[noreturn]] static void raise(const TArgs &... args) __attribute__((noreturn));

    template <typename T, typename... TArgs>
    static inline void assertEquals(const T &expected, const T &actual, const TArgs &... args) {
        DEBUG_ONLY(check(expected == actual, "assertEqual failed: expected=", expected, ", actual=", actual,
                         ", message: ", args...));
    }

    template <typename... TArgs> static inline void check(bool cond, const TArgs &... args) {
        if (debug_mode)
            if (!cond) {
                raise(args...);
            }
    }

    [[noreturn]] static inline void notImplemented() {
        if (debug_mode)
            raise("Not Implemented");
    }

private:
    static inline void _raise(std::ostream &) {}

    template <typename TArg, typename... TArgs>
    static inline void _raise(std::ostream &os, const TArg &arg, const TArgs &... args) {
        os << arg;
        _raise(os, args...);
    }
};

template <typename... TArgs>[[noreturn]] void Error::raise(const TArgs &... args) {
    std::stringstream message;
    _raise(message, args...);

    std::stringstream stackTrace;

    throw SRubyException(message.str(), stackTrace.str());
}

class File {
public:
    static std::string read(const char *filename);
};
}

#endif