#include "Context.h"
#include "spdlog/fmt/ostr.h"

namespace ruby_typer {
namespace ast {

using namespace std;

void Reporter::_error(Loc loc, ErrorClass what, const string &formatted) {
    auto pos = loc.position(ctx_);

    ctx_.logger.error("{}:{}:{}-{}:{}: {}", loc.file.file(ctx_).path(), pos.first.line, pos.first.column,
                      pos.second.line, pos.second.column, formatted);
}
} // namespace ast
} // namespace ruby_typer
