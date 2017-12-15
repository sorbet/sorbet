#ifndef SRUBY_REPORTER_H
#define SRUBY_REPORTER_H

#include "Loc.h"
#include "spdlog/fmt/fmt.h"
#include <initializer_list>
#include <memory>

namespace ruby_typer {
namespace core {

class ErrorClass {
public:
    u2 code;

    constexpr ErrorClass(u2 code) : code(code){};
    ErrorClass(const ErrorClass &rhs) = default;
    ErrorClass &operator=(const ErrorClass &rhs) = default;

    bool operator==(const ErrorClass &rhs) const {
        return code == rhs.code;
    }

    bool operator!=(const ErrorClass &rhs) const {
        return !(*this == rhs);
    }
};

class Reporter final {
    friend GlobalState;

public:
    struct BasicError {
        Loc loc;
        ErrorClass what;
        std::string formatted;
        BasicError(Loc loc, ErrorClass what, std::string formatted) : loc(loc), what(what), formatted(formatted) {}
        virtual std::string toString(GlobalState &gs);
        virtual ~BasicError() = default;
        static std::string filePosToString(GlobalState &gs, Loc loc);
    };

    struct ErrorLine {
        Loc loc;
        std::string formattedMessage;
        ErrorLine(Loc loc, std::string formattedMessage) : loc(loc), formattedMessage(formattedMessage){};

        template <typename... Args> static ErrorLine from(Loc loc, const std::string &msg, const Args &... args) {
            std::string formatted = fmt::format(msg, args...);
            return ErrorLine(loc, formatted);
        }
        std::string toString(GlobalState &gs);
    };

    struct ErrorSection {
        std::string header;
        std::vector<ErrorLine> messages;
        ErrorSection(std::string header) : header(header) {}
        ErrorSection(std::string header, std::initializer_list<ErrorLine> messages)
            : header(header), messages(messages) {}
        ErrorSection(std::string header, std::vector<ErrorLine> messages) : header(header), messages(messages) {}
        std::string toString(GlobalState &gs);
    };

    struct ComplexError : public BasicError {
        std::vector<ErrorSection> sections;
        virtual std::string toString(GlobalState &gs);
        ComplexError(Loc loc, ErrorClass what, std::string header, std::initializer_list<ErrorSection> sections)
            : BasicError(loc, what, header), sections(sections) {}
        ComplexError(Loc loc, ErrorClass what, std::string header, std::vector<ErrorSection> sections)
            : BasicError(loc, what, header), sections(sections) {}
        ComplexError(Loc loc, ErrorClass what, std::string header, ErrorLine other_line)
            : BasicError(loc, what, header), sections({ErrorSection("", {other_line})}) {}
    };

    void _error(std::unique_ptr<BasicError> error);
    std::vector<std::unique_ptr<ruby_typer::core::Reporter::BasicError>> errors;

    bool hadCriticalError() {
        return hadCriticalError_;
    };

    template <typename... Args> void error(Loc loc, ErrorClass what, const std::string &msg, const Args &... args) {
        std::string formatted = fmt::format(msg, args...);
        _error(std::make_unique<BasicError>(BasicError(loc, what, formatted)));
    }
    void error(ComplexError error) {
        _error(std::make_unique<ComplexError>(error));
    }

    bool keepErrorsInMemory = false;
    std::vector<std::unique_ptr<ruby_typer::core::Reporter::BasicError>> getAndEmptyErrors();

    std::vector<std::pair<int, int>> errorHistogram;
    int totalErrors();

private:
    Reporter(GlobalState &gs) : gs_(gs), hadCriticalError_(false) {}
    GlobalState &gs_;
    bool hadCriticalError_;
};

} // namespace core
} // namespace ruby_typer

#endif
