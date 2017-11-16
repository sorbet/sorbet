#include "Context.h"
#include "Hashing.h"
#include "common/common.h"
#include <algorithm>
#include <sstream>
#include <string>

using namespace std;

namespace ruby_typer {
namespace core {

SymbolRef Context::selfClass() {
    Symbol &info = this->owner.info(this->state);
    if (info.isClass())
        return info.singletonClass(this->state);
    return this->contextClass();
}

SymbolRef Context::enclosingMethod() {
    SymbolRef owner = this->owner;
    while (owner != GlobalState::defn_root() && !owner.info(this->state, false).isMethod()) {
        Error::check(owner.exists());
        owner = owner.info(this->state).owner;
    }
    if (owner == GlobalState::defn_root()) {
        return GlobalState::noSymbol();
    }
    return owner;
}

SymbolRef Context::enclosingClass() {
    SymbolRef owner = this->owner;
    while (owner != GlobalState::defn_root() && !owner.info(this->state, false).isClass()) {
        Error::check(owner.exists());
        owner = owner.info(this->state).owner;
    }
    if (owner == GlobalState::defn_root()) {
        return GlobalState::noSymbol();
    }
    return owner;
}

SymbolRef Context::contextClass() {
    SymbolRef owner = this->owner;
    while (!owner.info(this->state, false).isClass()) {
        Error::check(owner.exists());
        owner = owner.info(this->state).owner;
    }
    return owner;
}

} // namespace core
} // namespace ruby_typer
