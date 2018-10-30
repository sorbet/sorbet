#ifndef SORBET_ERRO_H
#define SORBET_ERRO_H

#include "absl/strings/str_cat.h"
#include "common/os/os.h"
#include "spdlog/spdlog.h"
#include <cstdio>
#include <memory>
#include <string>

namespace sorbet {
extern std::shared_ptr<spdlog::logger> fatalLogger;
class SRubyException {
public:
    /**
     * Creates an exception given the message and the stack trace.
     *
     * @param message contains information about exceptional situation.
     * @param stackTrace the stack trace where this exception happened.
     */
    SRubyException(std::string_view message) : _message(message) {}

    /**
     * Returns information about the exceptional situation.
     */
    inline const std::string_view message() const {
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

class Exception final {
public:
    template <typename... TArgs>[[noreturn]] static bool raise(const TArgs &... args) __attribute__((noreturn));

    [[noreturn]] static inline void notImplemented() {
        raise("Not Implemented");
    }

    static void print_backtrace() noexcept;

    template <typename... TArgs>
    [[noreturn]] static inline bool enforce_handler(std::string check, std::string file, int line,
                                                    const TArgs &... args) __attribute__((noreturn)) {
        raise(file + ":" + std::to_string(line), " enforced condition ", check, " has failed: ", args...);
    }
};

template <typename... TArgs>[[noreturn]] bool Exception::raise(const TArgs &... args) {
    std::string message = absl::StrCat("", args...);

    if (message.size() > 0) {
        fatalLogger->error("Exception::raise(): {}\n", message);
    } else {
        fatalLogger->error("Exception::raise() (sadly without a message)\n");
    }
    print_backtrace();
    stopInDebugger();
    throw SRubyException(message);
}
} // namespace sorbet
#endif // SORBET_ERRO_H
