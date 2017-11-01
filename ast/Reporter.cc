#include "Context.h"
#include "spdlog/fmt/ostr.h"

namespace ruby_typer {
namespace ast {

using namespace std;

void Reporter::_error(Error error) {
    if (keepErrorsInMemory) {
        errors.push_back(error);
        return;
    }
    ctx_.logger.error("{}", error.toString(ctx_));
}

std::string Reporter::Error::toString(GlobalState &ctx) {
    stringstream buf;
    if (loc.is_none()) {
        buf << "???:";
    } else {
        auto pos = loc.position(ctx);
        buf << loc.file.file(ctx).path() << ":";
        buf << pos.first.line << ":" << pos.first.column;
        buf << "-";
        buf << pos.second.line << ":" << pos.second.column;
    }
    buf << " " << formatted;
    return buf.str();
}

std::vector<Reporter::Error> Reporter::getAndEmptyErrors() {
    return errors;
}

} // namespace ast
} // namespace ruby_typer
