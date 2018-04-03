#ifndef SRUBY_CONTEXT_H
#define SRUBY_CONTEXT_H

#include "GlobalState.h"
#include "common/common.h"
#include <vector>

namespace ruby_typer {
namespace core {

class Context {
public:
    const GlobalState &state;
    SymbolRef owner;
    bool frozenConstraint = false;

    operator const GlobalState &() const noexcept {
        return state;
    }

    Context(GlobalState &state, SymbolRef owner) noexcept : state(state), owner(owner) {}
    Context(const Context &other) noexcept : state(other.state), owner(other.owner) {}
    Context(const MutableContext &other) noexcept;

    // Returns a SymbolRef corresponding to the class `self.class` for code
    // executed in this MutableContext, or, if `self` is a class,
    // `self.singleton_class` (We model classes as being normal instances of
    // their singleton classes for most purposes)
    SymbolRef selfClass();

    bool permitOverloadDefinitions() const;

    // Returns the SymbolRef corresponding to the class `self.class`, unless the
    // context is a class, in which case return it. This class is most notably
    // the class in which to look up class variables.
    SymbolRef contextClass() const;

    Context withOwner(SymbolRef sym) const {
        Context r = Context(*this);
        r.owner = sym;
        return r;
    }

    Context withFrozenConstraint() const {
        Context r = Context(*this);
        r.frozenConstraint = true;
        return r;
    }
    void trace(const std::string &msg) const {
        state.trace(msg);
    }
};

class MutableContext final {
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

    MutableContext(GlobalState &state, SymbolRef owner) noexcept : state(state), owner(owner) {}
    MutableContext(const MutableContext &other) noexcept : state(other.state), owner(other.owner) {}

    // Returns a SymbolRef corresponding to the class `self.class` for code
    // executed in this MutableContext, or, if `self` is a class,
    // `self.singleton_class` (We model classes as being normal instances of
    // their singleton classes for most purposes)
    SymbolRef selfClass();

    bool permitOverloadDefinitions() const;

    // Returns the SymbolRef corresponding to the class `self.class`, unless the
    // context is a class, in which case return it. This class is most notably
    // the class in which to look up class variables.
    SymbolRef contextClass() const;

    MutableContext withOwner(SymbolRef sym) const {
        MutableContext r = MutableContext(*this);
        r.owner = sym;
        return r;
    }

    MutableContext withFrozenConstraint() {
        MutableContext r = MutableContext(*this);
        r.frozenConstraint = true;
        return r;
    }
    void trace(const std::string &msg) const {
        state.trace(msg);
    }
};

class GlobalSubstitution {
public:
    GlobalSubstitution(const GlobalState &from, GlobalState &to, const GlobalState *optionalCommonParent = nullptr);

    NameRef substitute(NameRef from) const {
        ENFORCE(from._id < nameSubstitution.size(), "name substitution index out of bounds");
        return nameSubstitution[from._id];
    }

    bool useFastPath() const;

private:
    std::vector<NameRef> nameSubstitution;
    // set if no substitution is actually necessary
    bool fastPath;
};

} // namespace core
} // namespace ruby_typer

#endif // SRUBY_CONTEXT_H
