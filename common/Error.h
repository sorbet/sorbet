#ifndef SRUBY_ERRO_H
#define SRUBY_ERRO_H

#include "os/os.h"
#include "spdlog/spdlog.h"
#include <cstdio>
#include <memory>
#include <string>

namespace ruby_typer {
extern std::shared_ptr<spdlog::logger> fatalLogger;
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

private:
    std::string _message;

    std::string _stackTrace;

    // std::stringstream _message;
};

class FileNotFoundException : SRubyException {
public:
    FileNotFoundException() : SRubyException("File not found") {}
};

class Error final {
public:
    template <typename... TArgs>[[noreturn]] static bool raise(const TArgs &... args) __attribute__((noreturn));

    template <typename... TArgs> static inline void check(bool cond, const TArgs &... args) {
        if (debug_mode)
            if (!cond) {
                raise(args...);
            }
    }

    [[noreturn]] static inline void notImplemented() {
        raise("Not Implemented");
    }

    static void print_backtrace() noexcept;

    template <typename... TArgs>
    [[noreturn]] static inline bool enforce_handler(std::string check, std::string file, int line,
                                                    const TArgs &... args) __attribute__((noreturn)) {
        raise(file + ":" + std::to_string(line), " enforced condition ", check, " has failed: ", args...);
    }

private:
    static inline void _raise(std::ostream &) {}

    template <typename TArg, typename... TArgs>
    static inline void _raise(std::ostream &os, const TArg &arg, const TArgs &... args) {
        os << arg;
        _raise(os, args...);
    }
};

template <typename... TArgs>
[[noreturn]] bool Error::raise(const TArgs &... args) {
    std::stringstream message;
    _raise(message, args...);

    if (message.str().size() > 0) {
        fatalLogger->error("Error::raise(): {}\n", message.str().c_str());
    } else {
        fatalLogger->error("Error::raise() (sadly without a message)\n");
    }
    print_backtrace();
    stopInDebugger();
    throw SRubyException(message.str());
}
} // namespace ruby_typer
#endif // SRUBY_ERRO_H
