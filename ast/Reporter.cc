#include "Context.h"
#include "spdlog/fmt/ostr.h"

namespace ruby_typer {
namespace ast {

using namespace std;

void Reporter::_error(Loc loc, ErrorClass what, const string &formatted) {
    u4 begin_line, begin_col, end_line, end_col;
    loc.position(ctx_, begin_line, begin_col, end_line, end_col);

    ctx_.logger.error("{}:{}:{}-{}:{}: {}", loc.file.file(ctx_).path(), begin_line, begin_col, end_line, end_col,
                      formatted);
}
} // namespace ast
} // namespace ruby_typer
