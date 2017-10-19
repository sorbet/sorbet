#include "Symbols.h"
#include "Context.h"
#include <sstream>

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

bool SymbolRef::isSynthetic() const {
    return this->_id <= ContextBase::defn_last_synthetic_sym()._id;
}

std::string SymbolRef::toString(ContextBase &ctx) const {
    std::ostringstream os;
    auto myInfo = info(ctx, true);
    auto name = myInfo.name.toString(ctx);
    auto members = myInfo.members;
    os << name;
    if (members.size() > 0) {
        os << " with children:" << std::endl;
        for (auto pair: members) {
            os << pair.second.toString(ctx);
        }
    }
    return os.str();
}


} // namespace ast
} // namespace ruby_typer
