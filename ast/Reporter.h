#include "Loc.h"
#include "spdlog/fmt/fmt.h"

namespace ruby_typer {
namespace ast {

    enum class ErrorClass {
        Internal, // Internal Compiler Error
    };

class Reporter {
    friend ContextBase;
private:
    void _error(Loc loc, ErrorClass what, const std::string &formatted);

public:
    template<typename...Args> void error(Loc loc, ErrorClass what, const std::string &msg, const Args&...args) {
        std::string formatted = fmt::format(msg, args...);
        _error(loc, what, formatted);
    }

private:
    Reporter(ContextBase &ctx) : ctx_(ctx) {}
    ContextBase &ctx_;
};

} // namespace ast
} // namespace ruby_typer
