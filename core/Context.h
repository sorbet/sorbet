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
    bool frozenConstraint = false;
    operator GlobalState &() {
        return state;
    }

    operator const GlobalState &() const {
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
    bool permitOverloadDefinitions();

    // Returns the SymbolRef corresponding to the class `self.class`, unless the
    // context is a class, in which case return it. This class is most notably
    // the class in which to look up class variables.
    SymbolRef contextClass();

    Context withOwner(SymbolRef sym) {
        Context r = Context(*this);
        r.owner = sym;
        return r;
    }

    Context withFrozenConstraint() {
        Context r = Context(*this);
        r.frozenConstraint = true;
        return r;
    }
    SymbolRef newTemporary(UniqueNameKind kind, NameRef name, SymbolRef owner);
};

class GlobalSubstitution {
public:
    GlobalSubstitution(const GlobalState &from, GlobalState &to);

    NameRef substitute(NameRef from) const {
        ENFORCE(from._id < nameSubstitution.size(), "name substitution index out of bounds");
        return nameSubstitution[from._id];
    }

private:
    std::vector<NameRef> nameSubstitution;
    std::vector<FileRef> fileSubstitution;
};

} // namespace core
} // namespace ruby_typer

#endif // SRUBY_CONTEXT_H
