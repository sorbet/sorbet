#ifndef SORBET_SYMBOLS_H
#define SORBET_SYMBOLS_H

#include "Loc.h"
#include "Names.h"
#include "common/common.h"
#include "core/Names/core.h"
#include <memory>
#include <tuple>
#include <vector>

namespace sorbet {
namespace core {
class Symbol;
class GlobalState;
class Type;
class MutableContext;
class Context;
class TypeAndOrigins;
class SendAndBlockLink;

enum class Variance { CoVariant = 1, ContraVariant = -1, Invariant = 0 };

class SymbolRef final {
    friend class GlobalState;
    friend class Symbol;

public:
    SymbolRef(GlobalState const *from, u4 _id);
    SymbolRef(const GlobalState &from, u4 _id);
    SymbolRef() : _id(0){};

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

    Symbol &data(GlobalState &gs, bool allowNone = false) const;
    const Symbol &data(const GlobalState &gs, bool allowNone = false) const;

    bool operator==(const SymbolRef &rhs) const;

    bool operator!=(const SymbolRef &rhs) const;

    bool operator!() {
        return !_id;
    }

    std::string toString(const GlobalState &gs, int tabs = 0, bool showHidden = false) const;
    std::string show(const GlobalState &gs) const;

    SymbolRef dealiasAt(GlobalState &gs, SymbolRef klass) const;

    u4 _id;

private:
};
#ifndef DEBUG_MODE
CheckSize(SymbolRef, 4, 4);
#endif

class LocalVariable final {
public:
    NameRef _name;
    u4 unique;
    LocalVariable(NameRef name, u4 unique);
    LocalVariable();
    bool exists() const;
    bool isSyntheticTemporary(const GlobalState &gs) const;
    bool isAliasForGlobal(const GlobalState &gs) const;
    LocalVariable(const LocalVariable &) = default;
    LocalVariable(LocalVariable &&) = default;
    LocalVariable &operator=(LocalVariable &&) = default;
    LocalVariable &operator=(const LocalVariable &) = default;

    bool operator==(const LocalVariable &rhs) const;

    bool operator!=(const LocalVariable &rhs) const;
    inline bool operator<(const LocalVariable &rhs) const {
        if (this->_name.id() < rhs._name.id()) {
            return true;
        }
        if (this->_name.id() > rhs._name.id()) {
            return false;
        }
        return this->unique < rhs.unique;
    }

    static inline LocalVariable noVariable() {
        return LocalVariable(NameRef::noName(), 0);
    };

    static inline LocalVariable blockCall() {
        return LocalVariable(Names::blockCall(), 0);
    }
    std::string toString(const GlobalState &gs) const;
};

namespace serialize {
class SerializerImpl;
}

class Symbol final {
public:
    Symbol(const Symbol &) = delete;
    Symbol() = default;
    Symbol(Symbol &&) noexcept = default;

    class Flags {
    public:
        static constexpr int NONE = 0;

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
        static constexpr int CLASS_ABSTRACT = 0x0040;
        static constexpr int CLASS_INTERFACE = 0x0020;

        // Method argument flags
        static constexpr int ARGUMENT_OPTIONAL = 0x0100;
        static constexpr int ARGUMENT_KEYWORD = 0x0080;
        static constexpr int ARGUMENT_REPEATED = 0x0040;
        static constexpr int ARGUMENT_BLOCK = 0x0020;

        // Method flags
        static constexpr int METHOD_PROTECTED = 0x0100;
        static constexpr int METHOD_PRIVATE = 0x0080;
        static constexpr int METHOD_OVERLOADED = 0x0040;
        static constexpr int METHOD_ABSTRACT = 0x0020;
        static constexpr int METHOD_GENERIC = 0x0010;

        // Type flags
        static constexpr int TYPE_COVARIANT = 0x0100;
        static constexpr int TYPE_INVARIANT = 0x0080;
        static constexpr int TYPE_CONTRAVARIANT = 0x0040;
        static constexpr int TYPE_FIXED = 0x0020;

        // Static Field flags
        static constexpr int STATIC_FIELD_TYPE_ALIAS = 0x0100;

        // Flags applying to all symbol types

        // Synthesized by C++ code in a DSL pass
        static constexpr int DSL_SYNTHESIZED = 0x0001;
    };

    SymbolRef owner;
    Loc loc;
    mutable u4 uniqueCounter =
        1; // used as a counter for local variables inside CFG. not shared by design TODO: check that is actually isn't
    u4 flags = Flags::NONE;

    /** Type aliases are introduced by resolver and SHOULD NOT be serialized */
    std::vector<std::pair<SymbolRef, SymbolRef>> typeAliases;
    SymbolRef superClass;
    std::shared_ptr<Type> resultType;

    unsigned int hash(const GlobalState &gs) const;

    inline std::vector<SymbolRef> &arguments() {
        ENFORCE(!isClass());
        return argumentsOrMixins;
    }

    inline const std::vector<SymbolRef> &arguments() const {
        ENFORCE(!isClass());
        return argumentsOrMixins;
    }

    std::vector<std::shared_ptr<Type>> selfTypeArgs(const GlobalState &gs) const;

    // selfType and externalType return the type of an instance of this Symbol
    // (which must be isClass()), if instantiated without specific type
    // parameters, as seen from inside or outside of the class, respectively.
    std::shared_ptr<Type> selfType(const GlobalState &gs) const;
    std::shared_ptr<Type> externalType(const GlobalState &gs) const;

    inline std::vector<SymbolRef> &mixins() {
        ENFORCE(isClass());
        return argumentsOrMixins;
    }

    inline const std::vector<SymbolRef> &mixins() const {
        ENFORCE(isClass());
        return argumentsOrMixins;
    }

    inline std::vector<SymbolRef> &typeMembers() {
        ENFORCE(isClass());
        return typeParams;
    }

    inline const std::vector<SymbolRef> &typeMembers() const {
        ENFORCE(isClass());
        return typeParams;
    }

    // Return the number of type parameters that must be passed to instantiate
    // this generic type. May differ from typeMembers.size() if some type
    // members have fixed values.
    int typeArity(const GlobalState &gs) const;

    inline std::vector<SymbolRef> &typeArguments() {
        ENFORCE(isMethod());
        return typeParams;
    }

    inline const std::vector<SymbolRef> &typeArguments() const {
        ENFORCE(isMethod());
        return typeParams;
    }

    bool derivesFrom(const GlobalState &gs, SymbolRef sym) const;

    inline SymbolRef parent(const GlobalState &gs) const {
        ENFORCE(isClass());
        return superClass;
    }

    // TODO(dmitry) perf: most calls to this method could be eliminated as part of perf work.
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

    inline bool isTypeMember() const {
        return (flags & Symbol::Flags::TYPE_MEMBER) != 0;
    }

    inline bool isTypeArgument() const {
        return (flags & Symbol::Flags::TYPE_ARGUMENT) != 0;
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
        ENFORCE(isMethod());
        return (flags & Symbol::Flags::METHOD_OVERLOADED) != 0;
    }

    inline bool isAbstract() const {
        ENFORCE(isMethod());
        return (flags & Symbol::Flags::METHOD_ABSTRACT) != 0;
    }

    inline bool isGenericMethod() const {
        ENFORCE(isMethod());
        return (flags & Symbol::Flags::METHOD_GENERIC) != 0;
    }

    inline bool isKeyword() const {
        ENFORCE(isMethodArgument());
        return (flags & Symbol::Flags::ARGUMENT_KEYWORD) != 0;
    }

    inline bool isBlockArgument() const {
        ENFORCE(isMethodArgument());
        return (flags & Symbol::Flags::ARGUMENT_BLOCK) != 0;
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

    inline bool isFixed() const {
        ENFORCE(isTypeArgument() || isTypeMember());
        return (flags & Symbol::Flags::TYPE_FIXED) != 0;
    }

    Variance variance() const {
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

    bool isBlockSymbol(const GlobalState &gs) const;

    inline bool isClassModule() const {
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

    inline bool isClassClass() const {
        return !isClassModule();
    }

    inline bool isClassAbstract() const {
        ENFORCE(isClass());
        return (flags & Symbol::Flags::CLASS_ABSTRACT) != 0;
    }

    inline bool isClassInterface() const {
        ENFORCE(isClass());
        return (flags & Symbol::Flags::CLASS_INTERFACE) != 0;
    }

    inline void setClass() {
        ENFORCE(!isStaticField() && !isField() && !isMethod() && !isTypeArgument() && !isTypeMember());
        flags = flags | Symbol::Flags::CLASS;
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

    inline void setCovariant() {
        ENFORCE(isTypeArgument() || isTypeMember());
        ENFORCE(!isContravariant() && !isInvariant());
        flags |= Symbol::Flags::TYPE_COVARIANT;
    }

    inline void setContravariant() {
        ENFORCE(isTypeArgument() || isTypeMember());
        ENFORCE(!isCovariant() && !isInvariant());
        flags |= Symbol::Flags::TYPE_CONTRAVARIANT;
    }

    inline void setInvariant() {
        ENFORCE(isTypeArgument() || isTypeMember());
        ENFORCE(!isCovariant() && !isContravariant());
        flags |= Symbol::Flags::TYPE_INVARIANT;
    }

    inline void setFixed() {
        ENFORCE(isTypeArgument() || isTypeMember());
        flags |= Symbol::Flags::TYPE_FIXED;
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
        ENFORCE(isMethod());
        flags |= Symbol::Flags::METHOD_OVERLOADED;
    }

    inline void setAbstract() {
        ENFORCE(isMethod());
        flags |= Symbol::Flags::METHOD_ABSTRACT;
    }

    inline void setGenericMethod() {
        ENFORCE(isMethod());
        flags |= Symbol::Flags::METHOD_GENERIC;
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

    inline void setClassAbstract() {
        ENFORCE(isClass());
        flags |= Symbol::Flags::CLASS_ABSTRACT;
    }

    inline void setClassInterface() {
        ENFORCE(isClass());
        flags |= Symbol::Flags::CLASS_INTERFACE;
    }

    inline void setStaticTypeAlias() {
        ENFORCE(isStaticField());
        flags |= Symbol::Flags::STATIC_FIELD_TYPE_ALIAS;
    }
    inline bool isStaticTypeAlias() {
        ENFORCE(isStaticField());
        return (flags & Symbol::Flags::STATIC_FIELD_TYPE_ALIAS) != 0;
    }

    inline void setDSLSynthesized() {
        flags |= Symbol::Flags::DSL_SYNTHESIZED;
    }
    inline bool isDSLSynthesized() {
        return (flags & Symbol::Flags::DSL_SYNTHESIZED) != 0;
    }

    SymbolRef findMember(const GlobalState &gs, NameRef name) const;
    SymbolRef findMemberTransitive(const GlobalState &gs, NameRef name) const;
    SymbolRef findConcreteMethodTransitive(const GlobalState &gs, NameRef name) const;

    /* transitively finds a member with the most similar name */

    struct FuzzySearchResult {
        SymbolRef symbol;
        NameRef name;
        int distance;
    };

    std::vector<FuzzySearchResult> findMemberFuzzyMatch(const GlobalState &gs, NameRef name, int betterThan = -1) const;

    std::string fullName(const GlobalState &gs) const;

    // Not printed when showing name table
    bool isHiddenFromPrinting(const GlobalState &gs) const;
    std::string toString(const GlobalState &gs, int tabs = 0, bool showHidden = false) const;
    std::string toJSON(const GlobalState &gs, int tabs = 0, bool showHidden = false) const;
    // Renders the full name of this Symbol in a form suitable for user display.
    std::string show(const GlobalState &gs) const;

    // Returns the singleton class for this class, lazily instantiating it if it
    // doesn't exist.
    SymbolRef singletonClass(GlobalState &gs);

    // Returns the singleton class or noSymbol
    SymbolRef lookupSingletonClass(const GlobalState &gs) const;

    // Returns attached class or noSymbol if it does not exist
    SymbolRef attachedClass(const GlobalState &gs) const;

    SymbolRef dealias(const GlobalState &gs) const;

    NameRef name; // todo: move out? it should not matter but it's important for
    // name resolution
    std::vector<std::pair<NameRef, SymbolRef>>
        members; // TODO: replace with https://github.com/greg7mdp/sparsepp . Should be only in ClassSymbol
    // optimize for absence

    Symbol deepCopy(const GlobalState &to) const;
    void sanityCheck(const GlobalState &gs) const;
    SymbolRef enclosingMethod(const GlobalState &gs) const;
    SymbolRef enclosingClass(const GlobalState &gs) const;

    class IntrinsicMethod {
    public:
        virtual std::shared_ptr<Type> apply(Context ctx, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                            std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                            std::shared_ptr<SendAndBlockLink> linkType) const = 0;
    };

    // All `IntrinsicMethod`s in sorbet should be statically-allocated, which is
    // why raw pointers are safe.
    const IntrinsicMethod *intrinsic = nullptr;

private:
    friend class serialize::SerializerImpl;
    FuzzySearchResult findMemberFuzzyMatchUTF8(const GlobalState &gs, NameRef name, int betterThan = -1) const;
    std::vector<FuzzySearchResult> findMemberFuzzyMatchConstant(const GlobalState &gs, NameRef name,
                                                                int betterThan = -1) const;

    /*
     * mixins and superclasses: `superClass` is *not* included in the
     *   `argumentsOrMixins` list. `superClass` may not exist even if
     *   `isClass()`, which implies that this symbol is either a module or one
     *   of our magic synthetic classes. During parsing+naming, `superClass ==
     *   todo()` iff every definition we've seen for this class has had an
     *   implicit superclass (`class Foo` with no `< Parent`); Once we hit
     *   Resolver::finalize(), these will be rewritten to `Object()`.
     */
    // TODO: make into tiny
    std::vector<SymbolRef> argumentsOrMixins;

    /** For Class or module - ordered type members of the class,
     * for method - ordered type generic type arguments of the class
     */
    std::vector<SymbolRef> typeParams;

    SymbolRef findMemberTransitiveInternal(const GlobalState &gs, NameRef name, int mask, int flags,
                                           int maxDepth = 100) const;
};

// CheckSize(Symbol, 152, 8); // This is under too much churn to be worth checking

class Symbols {
    Symbols() = delete;

public:
    static SymbolRef noSymbol() {
        return SymbolRef(nullptr, 0);
    }

    static SymbolRef top() {
        return SymbolRef(nullptr, 1);
    }

    static SymbolRef bottom() {
        return SymbolRef(nullptr, 2);
    }

    static SymbolRef root() {
        return SymbolRef(nullptr, 3);
    }

    static SymbolRef todo() {
        return SymbolRef(nullptr, 4);
    }

    static SymbolRef Object() {
        return SymbolRef(nullptr, 5);
    }

    static SymbolRef Integer() {
        return SymbolRef(nullptr, 6);
    }

    static SymbolRef Float() {
        return SymbolRef(nullptr, 7);
    }

    static SymbolRef String() {
        return SymbolRef(nullptr, 8);
    }

    static SymbolRef Symbol() {
        return SymbolRef(nullptr, 9);
    }

    static SymbolRef Array() {
        return SymbolRef(nullptr, 10);
    }

    static SymbolRef Hash() {
        return SymbolRef(nullptr, 11);
    }

    static SymbolRef TrueClass() {
        return SymbolRef(nullptr, 12);
    }

    static SymbolRef FalseClass() {
        return SymbolRef(nullptr, 13);
    }

    static SymbolRef NilClass() {
        return SymbolRef(nullptr, 14);
    }

    static SymbolRef untyped() {
        return SymbolRef(nullptr, 15);
    }

    static SymbolRef Opus() {
        return SymbolRef(nullptr, 16);
    }

    static SymbolRef T() {
        return SymbolRef(nullptr, 17);
    }

    static SymbolRef Class() {
        return SymbolRef(nullptr, 18);
    }

    static SymbolRef BasicObject() {
        return SymbolRef(nullptr, 19);
    }

    static SymbolRef Kernel() {
        return SymbolRef(nullptr, 20);
    }

    static SymbolRef Range() {
        return SymbolRef(nullptr, 21);
    }

    static SymbolRef Regexp() {
        return SymbolRef(nullptr, 22);
    }

    static SymbolRef Magic() {
        return SymbolRef(nullptr, 23);
    }

    static SymbolRef Module() {
        return SymbolRef(nullptr, 24);
    }

    static SymbolRef StandardError() {
        return SymbolRef(nullptr, 25);
    }

    static SymbolRef Complex() {
        return SymbolRef(nullptr, 26);
    }

    static SymbolRef Rational() {
        return SymbolRef(nullptr, 27);
    }

    static SymbolRef T_Array() {
        return SymbolRef(nullptr, 28);
    }

    static SymbolRef T_Hash() {
        return SymbolRef(nullptr, 29);
    }

    static SymbolRef T_Proc() {
        return SymbolRef(nullptr, 30);
    }

    static SymbolRef Proc() {
        return SymbolRef(nullptr, 31);
    }

    static SymbolRef Enumerable() {
        return SymbolRef(nullptr, 32);
    }

    static SymbolRef Set() {
        return SymbolRef(nullptr, 33);
    }

    static SymbolRef Struct() {
        return SymbolRef(nullptr, 34);
    }

    static SymbolRef File() {
        return SymbolRef(nullptr, 35);
    }

    static SymbolRef RubyTyper() {
        return SymbolRef(nullptr, 36);
    }

    static SymbolRef StubClass() {
        return SymbolRef(nullptr, 37);
    }

    static SymbolRef T_Enumerable() {
        return SymbolRef(nullptr, 38);
    }

    static SymbolRef T_Range() {
        return SymbolRef(nullptr, 39);
    }

    static SymbolRef T_Set() {
        return SymbolRef(nullptr, 40);
    }

    static SymbolRef Configatron() {
        return SymbolRef(nullptr, 41);
    }

    static SymbolRef Configatron_Store() {
        return SymbolRef(nullptr, 42);
    }

    static SymbolRef Configatron_RootStore() {
        return SymbolRef(nullptr, 43);
    }

    static SymbolRef Sinatra() {
        return SymbolRef(nullptr, 44);
    }

    static SymbolRef SinatraBase() {
        return SymbolRef(nullptr, 45);
    }

    static SymbolRef void_() {
        return SymbolRef(nullptr, 46);
    }

    // Synthetic symbol used by resolver to mark type alias assignments.
    static SymbolRef typeAliasTemp() {
        return SymbolRef(nullptr, 47);
    }

    static SymbolRef Chalk() {
        return SymbolRef(nullptr, 48);
    }
    static SymbolRef Chalk_Tools() {
        return SymbolRef(nullptr, 49);
    }
    static SymbolRef Chalk_Tools_Accessible() {
        return SymbolRef(nullptr, 50);
    }

    static SymbolRef T_Generic() {
        return SymbolRef(nullptr, 51);
    }

    static SymbolRef Tuple() {
        return SymbolRef(nullptr, 52);
    }

    static SymbolRef Subclasses() {
        return SymbolRef(nullptr, 53);
    }

    static constexpr int MAX_PROC_ARITY = 10;
    static SymbolRef Proc0() {
        return SymbolRef(nullptr, MAX_SYNTHETIC_SYMBOLS - MAX_PROC_ARITY * 2 - 2);
    }

    static SymbolRef Proc(int argc) {
        if (argc > MAX_PROC_ARITY) {
            return noSymbol();
        }
        return SymbolRef(nullptr, Proc0()._id + argc * 2);
    }

    static SymbolRef last_proc() {
        return Proc(MAX_PROC_ARITY);
    }

    // Keep as last and update to match the last entry
    static SymbolRef last_synthetic_sym() {
        ENFORCE(last_proc()._id == MAX_SYNTHETIC_SYMBOLS - 2);
        return SymbolRef(nullptr, MAX_SYNTHETIC_SYMBOLS - 1);
    }

    static constexpr int MAX_SYNTHETIC_SYMBOLS = 200;
};

} // namespace core
} // namespace sorbet

namespace std {
template <> struct hash<sorbet::core::SymbolRef> {
    std::size_t operator()(const sorbet::core::SymbolRef k) const {
        return k._id;
    }
};

template <> struct hash<sorbet::core::LocalVariable> {
    std::size_t operator()(const sorbet::core::LocalVariable k) const {
        return k._name._id * 63 + k.unique;
    }
};
} // namespace std
#endif // SORBET_SYMBOLS_H
