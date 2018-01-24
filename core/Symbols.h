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

enum class Variance { CoVariant = 1, ContraVariant = -1, Invariant = 0 };

class SymbolRef final {
    friend class GlobalState;
    friend class Symbol;

public:
    SymbolRef(GlobalState const *from, u4 _id);
    SymbolRef(const GlobalState &from, u4 _id);
    SymbolRef() : _id(0){};

    unsigned inline int classId() const {
        ENFORCE((_id & 3) == 3, "not a classId");
        return _id >> 2;
    }

    unsigned inline int defId() const {
        ENFORCE((_id & 3) == 2, "not a defId");
        return _id >> 2;
    }

    unsigned inline int valId() const {
        ENFORCE((_id & 3) == 1, "not a valId");
        return _id >> 2;
    }

    unsigned inline int typeId() const {
        ENFORCE((_id & 3) == 0, "not a valId");
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

    std::string toString(GlobalState &gs, int tabs = 0, bool showHidden = false) const;
    SymbolRef dealiasAt(GlobalState &gs, core::SymbolRef klass);

    u4 _id;

private:
};
#ifndef DEBUG_MODE
CheckSize(SymbolRef, 4, 4);
#endif

class LocalVariable final {
public:
    NameRef name;
    LocalVariable(NameRef name);
    LocalVariable();
    bool exists();
    bool isSyntheticTemporary(GlobalState &gs) const;
    bool isAliasForGlobal(GlobalState &gs) const;
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
        static constexpr int TYPE_ARGUMENT = 0x0400;
        static constexpr int TYPE_MEMBER = 0x0200;

        // Class flags
        static constexpr int CLASS_CLASS = 0x0100;
        static constexpr int CLASS_MODULE = 0x0080;

        // Method argument flags
        static constexpr int ARGUMENT_OPTIONAL = 0x0100;
        static constexpr int ARGUMENT_KEYWORD = 0x0080;
        static constexpr int ARGUMENT_REPEATED = 0x0040;
        static constexpr int ARGUMENT_BLOCK = 0x0020;

        // Method flags
        static constexpr int METHOD_PROTECTED = 0x0100;
        static constexpr int METHOD_PRIVATE = 0x0080;
        static constexpr int METHOD_OVERLOADED = 0x0040;

        // Type flags
        static constexpr int TYPE_COVARIANT = 0x0100;
        static constexpr int TYPE_INVARIANT = 0x0080;
        static constexpr int TYPE_CONTRAVARIANT = 0x0040;
    };

    SymbolRef owner;
    Loc definitionLoc;
    u4 uniqueCounter = 1;
    u4 flags;

    /*
     * mixins and superclasses: `superClass`A is *not* included in the
     *   `argumentsOrMixins` list. `superClass` may not exist even if
     *   `isClass()`, which implies that this symbol is either a module or one
     *   of our magic synthetic classes. During parsing+naming, `superClass ==
     *   defn_todo()` iff every definition we've seen for this class has had an
     *   implicit superclass (`class Foo` with no `< Parent`); Once we hit
     *   Resolver::finalize(), these will be rewritten to `defn_Object()`.
     */
    // TODO: make into tiny
    std::vector<SymbolRef> argumentsOrMixins;
    /** For Class or module - ordered type members of the class,
     * for method - ordered type generic type arguments of the class
     */
    std::vector<SymbolRef> typeParams;

    /** Type alisases are introduced by resolver and SHOULD NOT be seriazlied */
    std::vector<std::pair<SymbolRef, SymbolRef>> typeAliases;
    SymbolRef superClass;
    std::shared_ptr<Type> resultType;

    inline std::vector<SymbolRef> &arguments() {
        ENFORCE(!isClass());
        return argumentsOrMixins;
    }

    bool derivesFrom(GlobalState &gs, SymbolRef sym);

    inline std::vector<SymbolRef> &mixins(GlobalState &gs) {
        ENFORCE(isClass());
        return argumentsOrMixins;
    }

    inline SymbolRef parent(GlobalState &gs) {
        ENFORCE(isClass());
        return superClass;
    }

    SymbolRef ref(const GlobalState &gs) const;

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
        ENFORCE(isMethodArgument());
        return (flags & Symbol::Flags::ARGUMENT_OPTIONAL) != 0;
    }

    inline bool isRepeated() const {
        ENFORCE(isMethodArgument());
        return (flags & Symbol::Flags::ARGUMENT_REPEATED) != 0;
    }

    inline bool isOverloaded() const {
        DEBUG_ONLY(Error::check(isMethod()));
        return (flags & Symbol::Flags::METHOD_OVERLOADED) != 0;
    }

    inline bool isKeyword() const {
        ENFORCE(isMethodArgument());
        return (flags & Symbol::Flags::ARGUMENT_KEYWORD) != 0;
    }

    inline bool isBlockArgument() const {
        ENFORCE(isMethodArgument());
        return (flags & Symbol::Flags::ARGUMENT_BLOCK) != 0;
    }

    inline bool isTypeMember() const {
        return (flags & Symbol::Flags::TYPE_MEMBER) != 0;
    }

    inline bool isTypeArgument() const {
        return (flags & Symbol::Flags::TYPE_ARGUMENT) != 0;
    }

    inline bool isCovariant() const {
        ENFORCE(isTypeArgument() || isTypeMember());
        return (flags & Symbol::Flags::TYPE_COVARIANT) != 0;
    }

    inline bool isInvariant() const {
        ENFORCE(isTypeArgument() || isTypeMember());
        return (flags & Symbol::Flags::TYPE_INVARIANT) != 0;
    }

    inline bool isContravariant() const {
        ENFORCE(isTypeArgument() || isTypeMember());
        return (flags & Symbol::Flags::TYPE_CONTRAVARIANT) != 0;
    }

    core::Variance variance() const {
        if (isInvariant())
            return Variance::Invariant;
        if (isCovariant())
            return Variance ::CoVariant;
        if (isContravariant())
            return Variance ::ContraVariant;
        Error::raise("Should not happen");
    }

    inline bool isPublic() const {
        ENFORCE(isMethod());
        return !isProtected() && !isPrivate();
    }

    inline bool isProtected() const {
        ENFORCE(isMethod());
        return (flags & Symbol::Flags::METHOD_PROTECTED) != 0;
    }

    inline bool isPrivate() const {
        ENFORCE(isMethod());
        return (flags & Symbol::Flags::METHOD_PRIVATE) != 0;
    }

    bool isBlockSymbol(GlobalState &gs) const;

    inline void setClass() {
        ENFORCE(!isStaticField() && !isField() && !isMethod() && !isTypeArgument() && !isTypeMember());
        flags = flags | Symbol::Flags::CLASS;
    }

    inline void setIsModule(bool isModule) {
        ENFORCE(isClass());
        if (isModule) {
            ENFORCE((flags & Symbol::Flags::CLASS_CLASS) == 0);
            flags = flags | Symbol::Flags::CLASS_MODULE;
        } else {
            ENFORCE((flags & Symbol::Flags::CLASS_MODULE) == 0);
            flags = flags | Symbol::Flags::CLASS_CLASS;
        }
    }

    inline bool isClassModule() {
        ENFORCE(isClass());
        if (flags & Symbol::Flags::CLASS_MODULE)
            return true;
        if (flags & Symbol::Flags::CLASS_CLASS)
            return false;
        Error::raise("Should never happen");
    }

    inline bool isClassModuleSet() {
        ENFORCE(isClass());
        return flags & (Symbol::Flags::CLASS_MODULE | Symbol::Flags::CLASS_CLASS);
    }

    inline bool isClassClass() {
        return !isClassModule();
    }
    inline void setStaticField() {
        ENFORCE(!isClass() && !isField() && !isMethod() && !isTypeArgument() && !isTypeMember());
        flags = flags | Symbol::Flags::STATIC_FIELD;
    }

    inline void setField() {
        ENFORCE(!isClass() && !isStaticField() && !isMethod() && !isTypeArgument() && !isTypeMember());
        flags = flags | Symbol::Flags::FIELD;
    }

    inline void setMethod() {
        ENFORCE(!isClass() && !isStaticField() && !isField() && !isTypeArgument() && !isTypeMember());
        flags = flags | Symbol::Flags::METHOD;
    }

    inline void setTypeArgument() {
        ENFORCE(!isClass() && !isStaticField() && !isField() && !isMethod() && !isTypeMember());
        flags = flags | Symbol::Flags::TYPE_ARGUMENT;
    }

    inline void setTypeMember() {
        ENFORCE(!isClass() && !isStaticField() && !isField() && !isMethod() && !isTypeArgument());
        flags = flags | Symbol::Flags::TYPE_MEMBER;
    }

    inline void setCovariant() {
        ENFORCE(!isContravariant() && !isInvariant());
        flags |= Symbol::Flags::TYPE_COVARIANT;
    }

    inline void setContravariant() {
        ENFORCE(!isCovariant() && !isInvariant());
        flags |= Symbol::Flags::TYPE_CONTRAVARIANT;
    }

    inline void setInvariant() {
        ENFORCE(!isCovariant() && !isContravariant());
        flags |= Symbol::Flags::TYPE_INVARIANT;
    }

    inline void setOptional() {
        ENFORCE(isMethodArgument());
        flags |= Symbol::Flags::ARGUMENT_OPTIONAL;
    }

    inline void setKeyword() {
        ENFORCE(isMethodArgument());
        flags |= Symbol::Flags::ARGUMENT_KEYWORD;
    }

    inline void setRepeated() {
        ENFORCE(isMethodArgument());
        flags |= Symbol::Flags::ARGUMENT_REPEATED;
    }

    inline void setOverloaded() {
        DEBUG_ONLY(Error::check(isMethod()));
        flags |= Symbol::Flags::METHOD_OVERLOADED;
    }

    inline void setBlockArgument() {
        ENFORCE(isMethodArgument());
        flags |= Symbol::Flags::ARGUMENT_BLOCK;
    }

    inline void setPublic() {
        ENFORCE(isMethod());
        flags &= ~Symbol::Flags::METHOD_PRIVATE;
        flags &= ~Symbol::Flags::METHOD_PROTECTED;
    }

    inline void setProtected() {
        ENFORCE(isMethod());
        flags |= Symbol::Flags::METHOD_PROTECTED;
    }

    inline void setPrivate() {
        ENFORCE(isMethod());
        flags |= Symbol::Flags::METHOD_PRIVATE;
    }

    SymbolRef findMember(GlobalState &gs, NameRef name);
    SymbolRef findMemberTransitive(GlobalState &gs, NameRef name, int MaxDepth = 100);

    std::string fullName(GlobalState &gs) const;

    // Returns the singleton class for this class, lazily instantiating it if it
    // doesn't exist.
    SymbolRef singletonClass(GlobalState &gs);

    // Returns attached class or noSymbol if it does not exist
    SymbolRef attachedClass(GlobalState &gs);

    SymbolRef dealias(GlobalState &gs);

    NameRef name; // todo: move out? it should not matter but it's important for
    // name resolution
    std::vector<std::pair<NameRef, SymbolRef>>
        members; // TODO: replace with https://github.com/greg7mdp/sparsepp . Should be only in ClassSymbol
    // optimize for absence

    Symbol deepCopy(const GlobalState &to) const;
    void sanityCheck(const GlobalState &gs) const;
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
