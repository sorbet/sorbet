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
class SorbetException : public std::logic_error {
public:
    SorbetException(const std::string &message) : logic_error(message) {}
    SorbetException(const char *message) : logic_error(message) {}
};

class FileNotFoundException : public SorbetException {
public:
    FileNotFoundException(const std::string &message) : SorbetException(message) {}
};

class FileNotDirException : SorbetException {
public:
    FileNotDirException() : SorbetException("File is not a directory") {}
};

class CreateDirException : SorbetException {
public:
    CreateDirException(const std::string &message) : SorbetException(message) {}
};

class RemoveFileException : SorbetException {
public:
    RemoveFileException(const std::string &message) : SorbetException(message) {}
};

class Exception final {
public:
    template <typename... TArgs>
    [[noreturn]] static bool raise(fmt::format_string<TArgs...> fmt, TArgs &&...args) __attribute__((noreturn));

    [[noreturn]] static inline void notImplemented() {
        raise("Not Implemented");
    }

    static void printBacktrace() noexcept;
    static void failInFuzzer() noexcept;

    [[noreturn]] static inline bool enforce_handler(std::string_view check, std::string_view file, int line)
        __attribute__((noreturn)) {
        enforce_handler(check, file, line, "(no message provided)");
    }
    template <typename... TArgs>
    [[noreturn]] static inline bool enforce_handler(std::string_view check, std::string_view file, int line,
                                                    fmt::format_string<TArgs...> message, TArgs &&...args)
        __attribute__((noreturn)) {
        raise("{}:{} enforced condition {} has failed: {}", file, line, check,
              fmt::format(message, std::forward<TArgs>(args)...));
    }
};

template <typename... TArgs> [[noreturn]] bool Exception::raise(fmt::format_string<TArgs...> fmt, TArgs &&...args) {
    Exception::failInFuzzer();
    std::string message = fmt::format(fmt, std::forward<TArgs>(args)...);

    if (message.size() > 0) {
        fatalLogger->error("Exception::raise(): {}\n", message);
    } else {
        fatalLogger->error("Exception::raise() (sadly without a message)\n");
    }
    printBacktrace();
    stopInDebugger();
    throw SorbetException(message);
}
} // namespace sorbet
#endif // SORBET_ERRO_H
