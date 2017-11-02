#include "Loc.h"
#include "spdlog/fmt/fmt.h"
#include <initializer_list>

namespace ruby_typer {
namespace ast {

enum class ErrorClass {
    Internal, // Internal Compiler Error

    IncludeMutipleParam, // Namer Errors
    IncludeNotConstant,
    IncludePassedBlock,

    DynamicConstant, // Resolver errors
    StubConstant,
    InvalidMethodSignature,

    PinnedVariableMismatch, // Inferencer Errors
    MethodArgumentMismatch,
    UnknownMethod,
    MethodArgumentCountMismatch,
    ReturnTypeMismatch,
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
        ErrorClass what;
        std::string header;
        std::vector<ErrorSection> sections;
        std::string toString(GlobalState &ctx);
        ComplexError(ErrorClass what, std::string header, std::initializer_list<ErrorSection> sections)
            : what(what), header(header), sections(sections) {}
        ComplexError(ErrorClass what, std::string header, std::vector<ErrorSection> sections)
            : what(what), header(header), sections(sections) {}
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
