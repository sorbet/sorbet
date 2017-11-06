#ifndef SRUBY_ERRO_H
#define SRUBY_ERRO_H

#include <cstdio>
namespace ruby_typer {
class SRubyException {
public:
    /**
     * Creates an exception given the message and the stack trace.
     *
     * @param message contains information about exceptional situation.
     * @param stackTrace the stack trace where this exception happened.
     */
    SRubyException(const std::string &message) : _message(message) {}

    /**
     * Returns information about the exceptional situation.
     */
    inline const std::string &message() const {
        return _message;
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

class FileNotFoundException : SRubyException {
public:
    FileNotFoundException() : SRubyException("File not found") {}
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
        raise("Not Implemented");
    }

    static void print_backtrace();

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

    fprintf(stderr, "%s", message.str().c_str());
    print_backtrace();

    throw SRubyException(message.str());
}
} // namespace ruby_typer
#endif // SRUBY_ERRO_H
