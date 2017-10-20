#include "Loc.h"
#include "spdlog/fmt/fmt.h"

namespace ruby_typer {
namespace ast {

class Reporter {
    friend ContextBase;
private:
    void _error(Loc loc, const std::string &formatted);

public:
    template<typename...Args> void error(Loc loc, const std::string &msg, const Args&...args) {
        std::string formatted = fmt::format(msg, args...);
        _error(loc, formatted);
    }

private:
    Reporter(ContextBase &ctx) : ctx_(ctx) {}
    ContextBase &ctx_;
};

} // namespace ast
} // namespace ruby_typer
