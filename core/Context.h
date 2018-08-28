#ifndef SORBET_CONTEXT_H
#define SORBET_CONTEXT_H

#include "GlobalState.h"
#include "common/common.h"
#include <vector>

namespace sorbet {
namespace core {

class Context {
public:
    const GlobalState &state;
    SymbolRef owner;

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

    void trace(const std::string &msg) const {
        state.trace(msg);
    }
};
CheckSize(Context, 16, 8);

class MutableContext final {
public:
    GlobalState &state;
    SymbolRef owner;
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

    void trace(const std::string &msg) const {
        state.trace(msg);
    }
};
CheckSize(MutableContext, 16, 8);

class GlobalSubstitution {
public:
    GlobalSubstitution(const GlobalState &from, GlobalState &to, const GlobalState *optionalCommonParent = nullptr);

    NameRef substitute(NameRef from, bool allowSameFromTo = false) const {
#ifdef DEBUG_MODE
        if (!allowSameFromTo) {
            ENFORCE(from.globalStateId != toGlobalStateId, "substituting a name twice!");
        }
#endif
        ENFORCE(from._id < nameSubstitution.size(), "name substitution index out of bounds, got " +
                                                        std::to_string(from._id) + " where subsitution size is " +
                                                        std::to_string(nameSubstitution.size()));
        return nameSubstitution[from._id];
    }

    bool useFastPath() const;

private:
    std::vector<NameRef> nameSubstitution;
    // set if no substitution is actually necessary
    bool fastPath;

#ifdef DEBUG_MODE
    const int toGlobalStateId;
#endif
};

} // namespace core
} // namespace sorbet

#endif // SORBET_CONTEXT_H
