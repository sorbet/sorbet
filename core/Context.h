#ifndef SRUBY_CONTEXT_H
#define SRUBY_CONTEXT_H

#include "GlobalState.h"
#include "common/common.h"
#include <vector>

namespace ruby_typer {
namespace core {

class Context final {
public:
    GlobalState &state;
    SymbolRef owner;
    operator GlobalState &() {
        return state;
    }

    Context(GlobalState &state, SymbolRef owner) : state(state), owner(owner) {}
    Context(const Context &other) : state(other.state), owner(other.owner) {}

    // Returns a SymbolRef corresponding to the class `self.class` for code
    // executed in this Context, or, if `self` is a class,
    // `self.singleton_class` (We model classes as being normal instances of
    // their singleton classes for most purposes)
    SymbolRef selfClass();

    SymbolRef enclosingMethod();
    SymbolRef enclosingClass();

    // Returns the SymbolRef corresponding to the class `self.class`, unless the
    // context is a class, in which case return it. This class is most notably
    // the class in which to look up class variables.
    SymbolRef contextClass();

    Context withOwner(SymbolRef sym) {
        Context r = Context(*this);
        r.owner = sym;
        return r;
    }
    SymbolRef newTemporary(UniqueNameKind kind, NameRef name, SymbolRef owner);
};

} // namespace core
} // namespace ruby_typer

#endif // SRUBY_CONTEXT_H
