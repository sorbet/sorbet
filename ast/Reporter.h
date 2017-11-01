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
    void _error(Loc loc, ErrorClass what, const std::string &formatted);

public:
    template <typename... Args> void error(Loc loc, ErrorClass what, const std::string &msg, const Args &... args) {
        std::string formatted = fmt::format(msg, args...);
        _error(loc, what, formatted);
    }

private:
    Reporter(GlobalState &ctx) : ctx_(ctx) {}
    GlobalState &ctx_;
};

} // namespace ast
} // namespace ruby_typer
