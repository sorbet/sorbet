#include "Loc.h"
#include "spdlog/fmt/fmt.h"

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

    void _error(Error error);
    std::vector<Error> errors;

public:
    template <typename... Args> void error(Loc loc, ErrorClass what, const std::string &msg, const Args &... args) {
        std::string formatted = fmt::format(msg, args...);
        _error(Error(loc, what, formatted));
    }

    bool keepErrorsInMemory = false;
    std::vector<Error> getAndEmptyErrors();

private:
    Reporter(GlobalState &ctx) : ctx_(ctx) {}
    GlobalState &ctx_;
};

} // namespace ast
} // namespace ruby_typer
