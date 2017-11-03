#include "Loc.h"
#include "spdlog/fmt/fmt.h"
#include <initializer_list>

namespace ruby_typer {
namespace ast {

enum class ErrorClass {
    Internal = 1001, // Internal Compiler Error

    InvalidSingletonDef = 2001, // Desugarer Errors

    IncludeMutipleParam = 4001, // Namer Errors
    IncludeNotConstant = 4002,
    IncludePassedBlock = 4003,

    DynamicConstant = 5001, // Resolver errors
    StubConstant = 5002,
    InvalidMethodSignature = 5003,

    PinnedVariableMismatch = 7001, // Inferencer Errors
    MethodArgumentMismatch = 7002,
    UnknownMethod = 7003,
    MethodArgumentCountMismatch = 7004,
    ReturnTypeMismatch = 7005,
};

class Reporter {
    friend GlobalState;

private:
    struct Error {
        Loc loc;
        ErrorClass what;
        std::string formatted;
        Error(Loc loc, ErrorClass what, std::string formatted) : loc(loc), what(what), formatted(formatted) {}
        std::string toString(GlobalState &ctx);
    };

public:
    struct ErrorLine {
        Loc loc;
        std::string formattedMessage;
        ErrorLine(Loc loc, std::string formattedMessage) : loc(loc), formattedMessage(formattedMessage){};

        template <typename... Args> static ErrorLine from(Loc loc, const std::string &msg, const Args &... args) {
            std::string formatted = fmt::format(msg, args...);
            return ErrorLine(loc, formatted);
        }
        std::string toString(GlobalState &ctx);
    };

    struct ErrorSection {
        std::string header;
        std::vector<ErrorLine> messages;
        ErrorSection(std::string header, std::initializer_list<ErrorLine> messages)
            : header(header), messages(messages) {}
        ErrorSection(std::string header, std::vector<ErrorLine> messages) : header(header), messages(messages) {}
        std::string toString(GlobalState &ctx);
    };

    struct ComplexError {
        Loc loc;
        ErrorClass what;
        std::string header;
        std::vector<ErrorSection> sections;
        std::string toString(GlobalState &ctx);
        ComplexError(Loc loc, ErrorClass what, std::string header, std::initializer_list<ErrorSection> sections)
            : loc(loc), what(what), header(header), sections(sections) {}
        ComplexError(Loc loc, ErrorClass what, std::string header, std::vector<ErrorSection> sections)
            : loc(loc), what(what), header(header), sections(sections) {}
    };

    void _error(Error error);
    void _error(ComplexError error);
    std::vector<Error> errors;

public:
    template <typename... Args> void error(Loc loc, ErrorClass what, const std::string &msg, const Args &... args) {
        std::string formatted = fmt::format(msg, args...);
        _error(Error(loc, what, formatted));
    }

    void error(ComplexError error) {
        _error(error);
    }

    bool keepErrorsInMemory = false;
    std::vector<Error> getAndEmptyErrors();

private:
    Reporter(GlobalState &ctx) : ctx_(ctx) {}
    GlobalState &ctx_;
};

} // namespace ast
} // namespace ruby_typer
