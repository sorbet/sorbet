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

    // Like `MutableContext::lookupSelfClass`
    ClassOrModuleRef lookupSelfClass() const;

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

    // If we're in a method, returns the owner of that method.
    // So for an instance method, gets the instance class, and for a singleton class method, gets the singleton class.
    // If we're in a class (i.e., at the class-top-level), gets the singleton class of the current class.
    //
    // If the singleton class does not exist, creates it.
    //
    // In general, `ctx.selfClass()->selfType()` is the type of `self` at any point in a tree traversal.
    ClassOrModuleRef selfClass();

    // Like `selfClass`, but does not create singleton classes (returns a non-existent symbol if it does not exist).
    ClassOrModuleRef lookupSelfClass() const;

    MutableContext withOwner(SymbolRef sym) const;
    MutableContext withFile(FileRef file) const;
    ErrorBuilder beginError(LocOffsets loc, ErrorClass what) const;

    // A version of `beginError` that's specific to the index phase of the pipeline, as it will record that index errors
    // have been seen on the file associated with the loc.
    ErrorBuilder beginIndexerError(LocOffsets loc, ErrorClass what) const;

    Loc locAt(LocOffsets offset) const;

    void trace(std::string_view msg) const;
};
CheckSize(MutableContext, 16, 8);

} // namespace sorbet::core

#endif // SORBET_CONTEXT_H
