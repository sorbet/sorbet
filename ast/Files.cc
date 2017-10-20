#include "Context.h"

using namespace std;

namespace ruby_typer {
namespace ast {

File &FileRef::file(ContextBase &ctx) const {
    return ctx.files[_id];
}

UTF8Desc File::path() {
    return this->path_;
}

UTF8Desc File::source() {
    return this->source_;
}

} // namespace ast
} // namespace ruby_typer
