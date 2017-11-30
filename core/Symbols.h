#ifndef SRUBY_SYMBOLS_H
#define SRUBY_SYMBOLS_H

#include "Loc.h"
#include "Names.h"
#include "common/common.h"
#include <memory>
#include <vector>

namespace ruby_typer {
namespace core {
class Symbol;
class GlobalState;
class Type;

class SymbolRef final {
    friend class GlobalState;

public:
    constexpr SymbolRef(u4 _id) : _id(_id){};

    SymbolRef() : _id(0){};

    unsigned inline int classId() const {
        DEBUG_ONLY(Error::check((_id & 3) == 3, "not a classId"));
        return _id >> 2;
    }

    unsigned inline int defId() const {
        DEBUG_ONLY(Error::check((_id & 3) == 2, "not a defId"));
        return _id >> 2;
    }

    unsigned inline int valId() const {
        DEBUG_ONLY(Error::check((_id & 3) == 1, "not a valId"));
        return _id >> 2;
    }

    unsigned inline int typeId() const {
        DEBUG_ONLY(Error::check((_id & 3) == 0, "not a valId"));
        return _id >> 2;
    }

    bool inline exists() const {
        return _id;
    }

    inline SymbolRef orElse(SymbolRef other) const {
        if (exists())
            return *this;
        else
            return other;
    }

    bool isSynthetic() const;

    /** Not printed when showing name table */
    bool isHiddenFromPrinting(GlobalState &gs) const;

    bool isPrimitive() const;

    Symbol &info(GlobalState &gs, bool allowNone = false) const;

    bool operator==(const SymbolRef &rhs) const;

    bool operator!=(const SymbolRef &rhs) const;

    bool operator!() {
        return !_id;
    }

    std::string toString(GlobalState &gs, int tabs = 0) const;

    u4 _id;
};

CheckSize(SymbolRef, 4, 4);

class LocalVariable final {
public:
    NameRef name;
    LocalVariable(NameRef name);
    LocalVariable();
    bool exists();
    bool isSyntheticTemporary(GlobalState &gs);
    LocalVariable(const LocalVariable &) = default;
    LocalVariable(LocalVariable &&) = default;
    LocalVariable &operator=(LocalVariable &&) = default;
    LocalVariable &operator=(const LocalVariable &) = default;

    bool operator==(const LocalVariable &rhs) const;

    bool operator!=(const LocalVariable &rhs) const;
};

class Symbol final {
public:
    Symbol(const Symbol &) = delete;
    Symbol() = default;
    Symbol(Symbol &&) noexcept = default;

    bool isConstructor(GlobalState &gs) const;

    class Flags {
    public:
        static constexpr int CLASS = 0x8000;
        static constexpr int METHOD = 0x4000;
        static constexpr int FIELD = 0x2000;
        static constexpr int STATIC_FIELD = 0x1000;
        static constexpr int METHOD_ARGUMENT = 0x0800;

        // Method argument flags
        static constexpr int ARGUMENT_OPTIONAL = 0x400;
        static constexpr int ARGUMENT_KEYWORD = 0x200;
        static constexpr int ARGUMENT_REPEATED = 0x100;
        static constexpr int ARGUMENT_BLOCK = 0x0080;

        // Method flags
        static constexpr int METHOD_PROTECTED = 0x400;
        static constexpr int METHOD_PRIVATE = 0x200;

        static constexpr int ARGUMENT_MASK = 0x0f80;
        static constexpr int METHOD_MASK = 0x4600;
    };

    SymbolRef owner;
    Loc definitionLoc;
    u4 uniqueCounter = 0;
    u4 flags;

    /*
     * mixins and superclasses: `superClass` is *not* included in the
     *   `argumentsOrMixins` list. `superClass` may not exist even if
     *   `isClass()`, which implies that this symbol is either a module or one
     *   of our magic synthetic classes. During parsing+naming, `superClass ==
     *   defn_todo()` iff every definition we've seen for this class has had an
     *   implicit superclass (`class Foo` with no `< Parent`); Once we hit
     *   Resolver::finalize(), these will be rewritten to `defn_Object()`.
     */
    // TODO: make into tiny
    std::vector<SymbolRef> argumentsOrMixins;
    SymbolRef superClass;
    std::shared_ptr<Type> resultType;

    inline std::vector<SymbolRef> &arguments() {
        Error::check(!isClass());
        return argumentsOrMixins;
    }

    bool derivesFrom(GlobalState &gs, SymbolRef sym);

    inline std::vector<SymbolRef> &mixins(GlobalState &gs) {
        Error::check(isClass());
        return argumentsOrMixins;
    }

    inline SymbolRef parent(GlobalState &gs) {
        Error::check(isClass());
        return superClass;
    }

    SymbolRef ref(GlobalState &gs) const;

    inline bool isClass() const {
        return (flags & Symbol::Flags::CLASS) != 0;
    }

    inline bool isStaticField() const {
        return (flags & Symbol::Flags::STATIC_FIELD) != 0;
    }

    inline bool isField() const {
        return (flags & Symbol::Flags::FIELD) != 0;
    }

    inline bool isMethod() const {
        return (flags & Symbol::Flags::METHOD) != 0;
    }

    inline bool isMethodArgument() const {
        return (flags & Symbol::Flags::METHOD_ARGUMENT) != 0;
    }

    inline bool isOptional() const {
        DEBUG_ONLY(Error::check(isMethodArgument()));
        return (flags & Symbol::Flags::ARGUMENT_OPTIONAL) != 0;
    }

    inline bool isRepeated() const {
        DEBUG_ONLY(Error::check(isMethodArgument()));
        return (flags & Symbol::Flags::ARGUMENT_REPEATED) != 0;
    }

    inline bool isKeyword() const {
        DEBUG_ONLY(Error::check(isMethodArgument()));
        return (flags & Symbol::Flags::ARGUMENT_KEYWORD) != 0;
    }

    inline bool isBlockArgument() const {
        DEBUG_ONLY(Error::check(isMethodArgument()));
        return (flags & Symbol::Flags::ARGUMENT_BLOCK) != 0;
    }

    inline bool isPublic() const {
        DEBUG_ONLY(Error::check(isMethod()));
        return !isProtected() && !isPrivate();
    }

    inline bool isProtected() const {
        DEBUG_ONLY(Error::check(isMethod()));
        return (flags & Symbol::Flags::METHOD_PROTECTED) != 0;
    }

    inline bool isPrivate() const {
        DEBUG_ONLY(Error::check(isMethod()));
        return (flags & Symbol::Flags::METHOD_PRIVATE) != 0;
    }

    inline bool isBlockSymbol(GlobalState &gs) const {
        core::Name &nm = name.name(gs);
        return nm.kind == NameKind::UNIQUE && nm.unique.original == Names::blockTemp();
    }

    inline void setClass() {
        DEBUG_ONLY(Error::check(!isStaticField() && !isField() && !isMethod()));
        flags = flags | Symbol::Flags::CLASS;
    }

    inline void setStaticField() {
        DEBUG_ONLY(Error::check(!isClass() && !isField() && !isMethod()));
        flags = flags | Symbol::Flags::STATIC_FIELD;
    }

    inline void setField() {
        DEBUG_ONLY(Error::check(!isClass() && !isStaticField() && !isMethod()));
        flags = flags | Symbol::Flags::FIELD;
    }

    inline void setMethod() {
        DEBUG_ONLY(Error::check(!isClass() && !isStaticField() && !isField()));
        flags = flags | Symbol::Flags::METHOD;
    }

    inline void setOptional() {
        DEBUG_ONLY(Error::check(isMethodArgument()));
        flags |= Symbol::Flags::ARGUMENT_OPTIONAL;
    }

    inline void setKeyword() {
        DEBUG_ONLY(Error::check(isMethodArgument()));
        flags |= Symbol::Flags::ARGUMENT_KEYWORD;
    }

    inline void setRepeated() {
        DEBUG_ONLY(Error::check(isMethodArgument()));
        flags |= Symbol::Flags::ARGUMENT_REPEATED;
    }

    inline void setBlockArgument() {
        DEBUG_ONLY(Error::check(isMethodArgument()));
        flags |= Symbol::Flags::ARGUMENT_BLOCK;
    }

    inline void setPublic() {
        DEBUG_ONLY(Error::check(isMethod()));
        flags &= ~Symbol::Flags::METHOD_PRIVATE;
        flags &= ~Symbol::Flags::METHOD_PROTECTED;
    }

    inline void setProtected() {
        DEBUG_ONLY(Error::check(isMethod()));
        flags |= Symbol::Flags::METHOD_PROTECTED;
    }

    inline void setPrivate() {
        DEBUG_ONLY(Error::check(isMethod()));
        flags |= Symbol::Flags::METHOD_PRIVATE;
    }

    SymbolRef findMember(NameRef name);
    SymbolRef findMemberTransitive(GlobalState &gs, NameRef name, int MaxDepth = 100);

    std::string fullName(GlobalState &gs) const;

    // Returns the singleton class for this class, lazily instantiating it if it
    // doesn't exist.
    SymbolRef singletonClass(GlobalState &gs);

    // Returns attached class or noSymbol if it does not exist
    SymbolRef attachedClass(GlobalState &gs);

    NameRef name; // todo: move out? it should not matter but it's important for
    // name resolution
    std::vector<std::pair<NameRef, SymbolRef>>
        members; // TODO: replace with https://github.com/greg7mdp/sparsepp . Should be only in ClassSymbol
    // optimize for absence
};

// CheckSize(Symbol, 88, 8); // This is under too much churn to be worth checking

} // namespace core
} // namespace ruby_typer

namespace std {
template <> struct hash<ruby_typer::core::SymbolRef> {
    std::size_t operator()(const ruby_typer::core::SymbolRef k) const {
        return k._id;
    }
};

template <> struct hash<ruby_typer::core::LocalVariable> {
    std::size_t operator()(const ruby_typer::core::LocalVariable k) const {
        return k.name._id;
    }
};
} // namespace std
#endif // SRUBY_SYMBOLS_H
