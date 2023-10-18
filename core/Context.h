#ifndef SORBET_CONTEXT_H
#define SORBET_CONTEXT_H

#include "common/common.h"
#include "core/Error.h"
#include "core/Files.h"
#include "core/Loc.h"
#include "core/NameRef.h"
#include "core/SymbolRef.h"

namespace sorbet::core {
class GlobalState;
class FileRef;
class MutableContext;
class ErrorBuilder;

class Context {
public:
    const GlobalState &state;
    const SymbolRef owner;
    const FileRef file;

    operator const GlobalState &() const noexcept {
        return state;
    }

    Context(const GlobalState &state, SymbolRef owner, FileRef file) noexcept
        : state(state), owner(owner), file(file) {}
    Context(const Context &other) noexcept : state(other.state), owner(other.owner), file(other.file) {}
    Context(const MutableContext &other) noexcept;

    ErrorBuilder beginError(LocOffsets loc, ErrorClass what) const;

    Context withOwner(SymbolRef sym) const;
    Context withFile(FileRef file) const;

    Loc locAt(LocOffsets offset) const;

    void trace(std::string_view msg) const;
};
CheckSize(Context, 16, 8);

class MutableContext final {
public:
    GlobalState &state;
    const SymbolRef owner;
    const FileRef file;
    operator GlobalState &() {
        return state;
    }

    operator const GlobalState &() const {
        return state;
    }

    // 👋 Stepped here in the debugger? Type 'finish' to step back out.
    MutableContext(GlobalState &state, SymbolRef owner, FileRef file) noexcept
        : state(state), owner(owner), file(file) {}
    MutableContext(const MutableContext &other) noexcept : state(other.state), owner(other.owner), file(other.file) {}

    // Returns a ClassOrModuleRef corresponding to the class `self.class` for code
    // executed in this MutableContext, or, if `self` is a class,
    // `self.singleton_class` (We model classes as being normal instances of
    // their singleton classes for most purposes)
    ClassOrModuleRef selfClass();

    MutableContext withOwner(SymbolRef sym) const;
    MutableContext withFile(FileRef file) const;
    ErrorBuilder beginError(LocOffsets loc, ErrorClass what) const;

    Loc locAt(LocOffsets offset) const;

    void trace(std::string_view msg) const;
};
CheckSize(MutableContext, 16, 8);

} // namespace sorbet::core

#endif // SORBET_CONTEXT_H
