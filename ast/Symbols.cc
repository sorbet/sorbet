#include "Symbols.h"
#include "Context.h"

namespace ruby_typer {
namespace ast {

bool SymbolRef::operator==(const SymbolRef &rhs) const {
    return _id == rhs._id;
}

bool SymbolRef::operator!=(const SymbolRef &rhs) const {
    return !(rhs == *this);
}

bool SymbolRef::isPrimitive() const {
    Error::notImplemented();
}
bool SymbolInfo::isConstructor(ContextBase &ctx) const {
    return this->name._id == 1;
}
SymbolInfo &SymbolRef::info(ContextBase &ctx, bool allowNone) const {
    Error::check(_id < ctx.symbols.size());
    if (!allowNone)
        Error::check(this->exists());

    return ctx.symbols[this->_id];
}

} // namespace ast
} // namespace ruby_typer