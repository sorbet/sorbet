#ifndef SORBET_CONTEXT_H
#define SORBET_CONTEXT_H

#include "common/common.h"
#include "core/NameRef.h"
#include "core/SymbolRef.h"

namespace sorbet::core {
class GlobalState;
class FileRef;
class MutableContext;

class Context {
public:
    const GlobalState &state;
    const SymbolRef owner;

    operator const GlobalState &() const noexcept {
        return state;
    }

    Context(const GlobalState &state, SymbolRef owner) noexcept : state(state), owner(owner) {}
    Context(const Context &other) noexcept : state(other.state), owner(other.owner) {}
    Context(const MutableContext &other) noexcept;

    static bool permitOverloadDefinitions(const core::GlobalState &gs, FileRef sigLoc, core::SymbolRef owner);

    Context withOwner(SymbolRef sym) const;

    void trace(std::string_view msg) const;
};
CheckSize(Context, 16, 8);

class MutableContext final {
public:
    GlobalState &state;
    const SymbolRef owner;
    operator GlobalState &() {
        return state;
    }

    operator const GlobalState &() const {
        return state;
    }

    // 👋 Stepped here in the debugger? Type 'finish' to step back out.
    MutableContext(GlobalState &state, SymbolRef owner) noexcept : state(state), owner(owner) {}
    MutableContext(const MutableContext &other) noexcept : state(other.state), owner(other.owner) {}

    // Returns a SymbolRef corresponding to the class `self.class` for code
    // executed in this MutableContext, or, if `self` is a class,
    // `self.singleton_class` (We model classes as being normal instances of
    // their singleton classes for most purposes)
    SymbolRef selfClass();

    bool permitOverloadDefinitions(FileRef sigLoc) const;

    MutableContext withOwner(SymbolRef sym) const {
        return MutableContext(state, sym);
    }

    void trace(std::string_view msg) const;
};
CheckSize(MutableContext, 16, 8);

} // namespace sorbet::core

#endif // SORBET_CONTEXT_H
