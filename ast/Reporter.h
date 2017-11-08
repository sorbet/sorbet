#include "Loc.h"
#include "spdlog/fmt/fmt.h"
#include <initializer_list>

namespace ruby_typer {
namespace ast {

enum class ErrorClass {
    Internal = 1001, // Internal Compiler Error

    ParserError = 2001, // Parser Errors

    InvalidSingletonDef = 3001, // Desugarer Errors

    IncludeMutipleParam = 4001, // Namer Errors
    IncludeNotConstant = 4002,
    IncludePassedBlock = 4003,
    DynamicConstantDefinition = 4004,
    RedefinitionOfParents = 4005,

    DynamicConstant = 5001, // Resolver errors
    StubConstant = 5002,
    InvalidMethodSignature = 5003,
    InvalidTypeDeclaration = 5004,
    InvalidDeclareVariables = 5005,
    DuplicateVariableDeclaration = 5006,
    UndeclaredVariable = 5007,

    PinnedVariableMismatch = 7001, // Inferencer Errors
    MethodArgumentMismatch = 7002,
    UnknownMethod = 7003,
    MethodArgumentCountMismatch = 7004,
    ReturnTypeMismatch = 7005,
};

class Reporter {
    friend GlobalState;

private:
    struct BasicError {
        Loc loc;
        ErrorClass what;
        std::string formatted;
        BasicError(Loc loc, ErrorClass what, std::string formatted) : loc(loc), what(what), formatted(formatted) {}
        virtual std::string toString(GlobalState &gs);
        virtual ~BasicError() = default;
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
        std::string toString(GlobalState &gs);
    };

    struct ErrorSection {
        std::string header;
        std::vector<ErrorLine> messages;
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
    };

    void _error(BasicError error);
    void _error(ComplexError error);
    std::vector<std::unique_ptr<ruby_typer::ast::Reporter::BasicError>> errors;

public:
    template <typename... Args> void error(Loc loc, ErrorClass what, const std::string &msg, const Args &... args) {
        std::string formatted = fmt::format(msg, args...);
        _error(BasicError(loc, what, formatted));
    }

    void error(ComplexError error) {
        _error(error);
    }

    bool keepErrorsInMemory = false;
    std::vector<std::unique_ptr<ruby_typer::ast::Reporter::BasicError>> getAndEmptyErrors();

private:
    Reporter(GlobalState &gs) : gs_(gs) {}
    GlobalState &gs_;
};

} // namespace ast
} // namespace ruby_typer
