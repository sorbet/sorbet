#ifndef RUBY_TYPER_SYMBOLREF_H
#define RUBY_TYPER_SYMBOLREF_H

#include "common/common.h"
#include "core/DebugOnlyCheck.h"
#include "core/FileRef.h"
#include "core/ShowOptions.h"

namespace sorbet::core {
class ClassOrModule;
class GlobalState;
class NameRef;
class Loc;
class TypePtr;
struct SymbolDataDebugCheck {
    const GlobalState &gs;
    const unsigned int symbolCountAtCreation;

    SymbolDataDebugCheck(const GlobalState &gs);
    void check() const;
};

/** These classes are intended to be a safe way to pass symbol references around.
 *  Entering new symbols can invalidate `Symbol &`s and thus they are generally unsafe.
 *  This class ensures that all accesses are safe in debug builds and effectively is a `Symbol &` in optimized builds.
 */
class ClassOrModuleData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    ClassOrModule &symbol;

public:
    ClassOrModuleData(ClassOrModule &ref, GlobalState &gs);

    ClassOrModule *operator->();
    const ClassOrModule *operator->() const;
};
CheckSize(ClassOrModuleData, 8, 8);

class ConstClassOrModuleData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    const ClassOrModule &symbol;

public:
    ConstClassOrModuleData(const ClassOrModule &ref, const GlobalState &gs);

    const ClassOrModule *operator->() const;
};
CheckSize(ConstClassOrModuleData, 8, 8);

class Method;
class MethodData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    Method &method;

public:
    MethodData(Method &ref, GlobalState &gs);

    Method *operator->();
    const Method *operator->() const;
};
CheckSize(MethodData, 8, 8);

class ConstMethodData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    const Method &method;

public:
    ConstMethodData(const Method &ref, const GlobalState &gs);

    const Method *operator->() const;
};
CheckSize(ConstMethodData, 8, 8);

class Field;
class FieldData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    Field &field;

public:
    FieldData(Field &ref, GlobalState &gs);

    Field *operator->();
    const Field *operator->() const;
};
CheckSize(FieldData, 8, 8);

class ConstFieldData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    const Field &field;

public:
    ConstFieldData(const Field &ref, const GlobalState &gs);

    const Field *operator->() const;
};
CheckSize(ConstFieldData, 8, 8);

class TypeParameter;
class TypeParameterData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    TypeParameter &typeParam;

public:
    TypeParameterData(TypeParameter &ref, GlobalState &gs);

    TypeParameter *operator->();
    const TypeParameter *operator->() const;
};
CheckSize(TypeParameterData, 8, 8);

class ConstTypeParameterData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    const TypeParameter &typeParam;

public:
    ConstTypeParameterData(const TypeParameter &ref, const GlobalState &gs);

    const TypeParameter *operator->() const;
};
CheckSize(ConstTypeParameterData, 8, 8);

class ClassOrModuleRef final {
    uint32_t _id;

    friend class SymbolRef;
    friend class GlobalState;

private:
    std::string toStringWithOptions(const GlobalState &gs, int tabs = 0, bool showFull = false,
                                    bool showRaw = false) const;

public:
    ClassOrModuleRef() : _id(0){};
    ClassOrModuleRef(const GlobalState &from, uint32_t id);

    uint32_t id() const {
        return _id;
    }

    bool exists() const {
        return _id != 0;
    }

    static ClassOrModuleRef fromRaw(uint32_t id) {
        ClassOrModuleRef ref;
        ref._id = id;
        return ref;
    }

    ClassOrModuleData data(GlobalState &gs) const;
    ClassOrModuleData dataAllowingNone(GlobalState &gs) const;
    ConstClassOrModuleData data(const GlobalState &gs) const;
    ConstClassOrModuleData dataAllowingNone(const GlobalState &gs) const;

    bool operator==(const ClassOrModuleRef &rhs) const;
    bool operator!=(const ClassOrModuleRef &rhs) const;

    std::string toString(const GlobalState &gs) const {
        bool showFull = false;
        bool showRaw = false;
        return toStringWithOptions(gs, 0, showFull, showRaw);
    }

    std::string_view showKind(const GlobalState &gs) const;
    std::string showFullName(const GlobalState &gs) const;
    std::string toStringFullName(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;

    bool isOnlyDefinedInFile(const GlobalState &gs, core::FileRef file) const;

    // Given a symbol like <PackageSpecRegistry>::Project::Foo, returns true.
    // Given any other symbol, returns false.
    // Also returns false if called on core::Symbols::noClassOrModule().
    bool isPackageSpecSymbol(const GlobalState &gs) const;

    // Certain classes that need to be generic in the standard library already have a definition for
    // the `[]` method, which would otherwise be the way to apply type arguments to a generic class.
    // For example, `Set[1, 2, 3]` creates a `Set` of `Integer`s.
    //
    // To allow people to continue this syntax, we create certain forwarder classes under the `T::`
    // namespace so that the `[]` method does not conflict with any existing method.
    //
    // This method tells whether the current ClassOrModuleRef is one of those forwarder classes.
    bool isBuiltinGenericForwarder() const;
    // Unwraps things like `T::Hash` to `Hash`, otherwise returns itself.
    ClassOrModuleRef maybeUnwrapBuiltinGenericForwarder() const;
    // Gets the `T::` forwarder class for the builtin generic (like `::Array` -> `::T::Array`)
    // Returns Symbols::noClassOrModule if there is no forwarder.
    ClassOrModuleRef forwarderForBuiltinGeneric() const;

    // Before stabilizing Sorbet's type syntax (indeed, before Sorbet even supported generic type
    // syntax), it was allowed to use `Array` in place of `T::Array[T.untyped]`. Out of a desire to
    // avoid a large code migration, we preserve that behavior for the select stdlib classes it
    // applied to at the time.
    //
    // The set of stdlib classes receiving this special behavior should not grow over time.
    bool isLegacyStdlibGeneric() const;
};
CheckSize(ClassOrModuleRef, 4, 4);

class MethodRef final {
    uint32_t _id;
    friend class SymbolRef;

private:
    std::string toStringWithOptions(const GlobalState &gs, int tabs = 0, bool showFull = false,
                                    bool showRaw = false) const;

public:
    MethodRef() : _id(0){};
    MethodRef(const GlobalState &from, uint32_t id);

    uint32_t id() const {
        return _id;
    }

    bool exists() const {
        return _id != 0;
    }

    static MethodRef fromRaw(uint32_t id) {
        MethodRef ref;
        ref._id = id;
        return ref;
    }

    std::string toString(const GlobalState &gs) const {
        bool showFull = false;
        bool showRaw = false;
        return toStringWithOptions(gs, 0, showFull, showRaw);
    }

    MethodData data(GlobalState &gs) const;
    ConstMethodData data(const GlobalState &gs) const;
    MethodData dataAllowingNone(GlobalState &gs) const;

    ClassOrModuleRef enclosingClass(const GlobalState &gs) const;
    std::string_view showKind(const GlobalState &gs) const;
    std::string showFullName(const GlobalState &gs) const;
    std::string toStringFullName(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;

    bool operator==(const MethodRef &rhs) const;

    bool operator!=(const MethodRef &rhs) const;
};
CheckSize(MethodRef, 4, 4);

class FieldRef final {
    uint32_t _id;

    friend class SymbolRef;

private:
    std::string toStringWithOptions(const GlobalState &gs, int tabs = 0, bool showFull = false,
                                    bool showRaw = false) const;

public:
    FieldRef() : _id(0){};
    FieldRef(const GlobalState &from, uint32_t id);

    uint32_t id() const {
        return _id;
    }

    bool exists() const {
        return _id != 0;
    }

    static FieldRef fromRaw(uint32_t id) {
        FieldRef ref;
        ref._id = id;
        return ref;
    }

    FieldData data(GlobalState &gs) const;
    ConstFieldData data(const GlobalState &gs) const;
    ConstFieldData dataAllowingNone(const GlobalState &gs) const;
    FieldData dataAllowingNone(GlobalState &gs) const;
    std::string_view showKind(const GlobalState &gs) const;
    std::string showFullName(const GlobalState &gs) const;
    std::string toStringFullName(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;

    bool operator==(const FieldRef &rhs) const;

    bool operator!=(const FieldRef &rhs) const;
};
CheckSize(FieldRef, 4, 4);

class TypeMemberRef final {
    uint32_t _id;

    friend class SymbolRef;

private:
    std::string toStringWithOptions(const GlobalState &gs, int tabs = 0, bool showFull = false,
                                    bool showRaw = false) const;

public:
    TypeMemberRef() : _id(0){};
    TypeMemberRef(const GlobalState &from, uint32_t id);

    uint32_t id() const {
        return _id;
    }

    bool exists() const {
        return _id != 0;
    }

    static TypeMemberRef fromRaw(uint32_t id) {
        TypeMemberRef ref;
        ref._id = id;
        return ref;
    }

    TypeParameterData data(GlobalState &gs) const;
    ConstTypeParameterData data(const GlobalState &gs) const;
    TypeParameterData dataAllowingNone(GlobalState &gs) const;
    std::string_view showKind(const GlobalState &gs) const;
    std::string showFullName(const GlobalState &gs) const;
    std::string toStringFullName(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;

    bool operator==(const TypeMemberRef &rhs) const;

    bool operator!=(const TypeMemberRef &rhs) const;
};
CheckSize(TypeMemberRef, 4, 4);

class TypeArgumentRef final {
    uint32_t _id;

    friend class SymbolRef;
    friend class MethodRef;

private:
    std::string toStringWithOptions(const GlobalState &gs, int tabs = 0, bool showFull = false,
                                    bool showRaw = false) const;

public:
    TypeArgumentRef() : _id(0){};
    TypeArgumentRef(const GlobalState &from, uint32_t id);

    uint32_t id() const {
        return _id;
    }

    bool exists() const {
        return _id != 0;
    }

    static TypeArgumentRef fromRaw(uint32_t id) {
        TypeArgumentRef ref;
        ref._id = id;
        return ref;
    }

    TypeParameterData data(GlobalState &gs) const;
    ConstTypeParameterData data(const GlobalState &gs) const;
    TypeParameterData dataAllowingNone(GlobalState &gs) const;
    std::string_view showKind(const GlobalState &gs) const;
    std::string showFullName(const GlobalState &gs) const;
    std::string toStringFullName(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;

    bool operator==(const TypeArgumentRef &rhs) const;

    bool operator!=(const TypeArgumentRef &rhs) const;
};
CheckSize(TypeArgumentRef, 4, 4);

class SymbolRef final {
    friend class GlobalState;
    friend class ClassOrModule;
    // For toStringWithOptions.
    friend class ClassOrModuleRef;
    friend class MethodRef;

    // Stores the symbol's Kind and Index. Kind occupies the lower bits.
    uint32_t _id;
    uint32_t unsafeTableIndex() const {
        return _id >> KIND_BITS;
    }

private:
    std::string toStringWithOptions(const GlobalState &gs, int tabs = 0, bool showFull = false,
                                    bool showRaw = false) const;

public:
    // If you add Symbol Kinds, make sure KIND_BITS is kept in sync!
    enum class Kind : uint8_t {
        ClassOrModule = 0,
        Method = 1,
        FieldOrStaticField = 2,
        TypeArgument = 3,
        TypeMember = 4,
    };

    // Kind takes up this many bits in _id.
    static constexpr uint32_t KIND_BITS = 3;
    static constexpr uint32_t ID_BITS = 32 - KIND_BITS;
    static constexpr uint32_t KIND_MASK = (1 << KIND_BITS) - 1;
    static constexpr uint32_t MAX_ID = (1 << ID_BITS) - 1;

    Kind kind() const {
        return static_cast<Kind>(_id & KIND_MASK);
    }

    uint32_t rawId() const {
        return _id;
    }

    inline bool isClassOrModule() const {
        return kind() == Kind::ClassOrModule;
    }

    inline bool isMethod() const {
        return kind() == Kind::Method;
    }

    inline bool isFieldOrStaticField() const {
        return kind() == Kind::FieldOrStaticField;
    }

    inline bool isTypeArgument() const {
        return kind() == Kind::TypeArgument;
    }

    inline bool isTypeMember() const {
        return kind() == Kind::TypeMember;
    }

    bool isTypeAlias(const GlobalState &gs) const;
    bool isField(const GlobalState &gs) const;
    bool isStaticField(const GlobalState &gs) const;
    bool isClassAlias(const GlobalState &gs) const;

    uint32_t classOrModuleIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::ClassOrModule);
        return unsafeTableIndex();
    }

    uint32_t methodIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::Method);
        return unsafeTableIndex();
    }

    uint32_t fieldIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::FieldOrStaticField);
        return unsafeTableIndex();
    }

    uint32_t typeArgumentIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::TypeArgument);
        return unsafeTableIndex();
    }

    uint32_t typeMemberIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::TypeMember);
        return unsafeTableIndex();
    }

    SymbolRef(GlobalState const *from, Kind type, uint32_t id);
    SymbolRef(const GlobalState &from, Kind type, uint32_t id);
    // This constructor is not marked explicit so that we can implicitly convert ClassOrModuleRef to SymbolRefs as
    // method arguments. This conversion is always safe and never throws.
    SymbolRef(ClassOrModuleRef kls);
    SymbolRef(MethodRef kls);
    SymbolRef(FieldRef kls);
    SymbolRef(TypeMemberRef kls);
    SymbolRef(TypeArgumentRef kls);
    SymbolRef() : _id(0){};

    // From experimentation, in the common case, methods typically have 2 or fewer arguments.
    // Placed here so it can be used across packages for common case optimizations.
    static constexpr int EXPECTED_METHOD_ARGS_COUNT = 2;

    static SymbolRef fromRaw(uint32_t raw) {
        auto ref = SymbolRef();
        ref._id = raw;
        return ref;
    }

    bool inline exists() const {
        // 0th index is reserved on all symbol vectors for the non existant symbol.
        return unsafeTableIndex() != 0;
    }

    bool isOnlyDefinedInFile(const GlobalState &gs, core::FileRef file) const;

    bool isSynthetic() const;

    // If Kind is ClassOrModule, returns a ClassOrModuleRef.
    ClassOrModuleRef asClassOrModuleRef() const {
        ENFORCE_NO_TIMER(kind() == Kind::ClassOrModule);
        return ClassOrModuleRef::fromRaw(unsafeTableIndex());
    }

    // If Kind is Method, returns a MethodRef.
    MethodRef asMethodRef() const {
        ENFORCE_NO_TIMER(kind() == Kind::Method);
        return MethodRef::fromRaw(unsafeTableIndex());
    }

    FieldRef asFieldRef() const {
        ENFORCE_NO_TIMER(kind() == Kind::FieldOrStaticField);
        return FieldRef::fromRaw(unsafeTableIndex());
    }

    TypeMemberRef asTypeMemberRef() const {
        ENFORCE_NO_TIMER(kind() == Kind::TypeMember);
        return TypeMemberRef::fromRaw(unsafeTableIndex());
    }

    TypeArgumentRef asTypeArgumentRef() const {
        ENFORCE_NO_TIMER(kind() == Kind::TypeArgument);
        return TypeArgumentRef::fromRaw(unsafeTableIndex());
    }

public:
    bool operator==(const SymbolRef &rhs) const;

    bool operator!=(const SymbolRef &rhs) const;

    // TODO(jvilk): Remove as many of these methods as possible in favor of callsites using .data on the more specific
    // symbol *Ref classes (e.g., ClassOrModuleRef). These were introduced to wean the codebase from calling
    // SymbolRef::data.
    // Please do not add methods to this list.
    ClassOrModuleRef enclosingClass(const GlobalState &gs) const;
    std::string_view showKind(const GlobalState &gs) const;
    core::NameRef name(const GlobalState &gs) const;
    core::SymbolRef owner(const GlobalState &gs) const;
    core::Loc loc(const GlobalState &gs) const;
    bool isPrintable(const GlobalState &gs) const;
    using LOC_store = InlinedVector<Loc, 2>;
    const LOC_store &locs(const GlobalState &gs) const;
    void removeLocsForFile(GlobalState &gs, core::FileRef file) const;
    const TypePtr &resultType(const GlobalState &gs) const;
    void setResultType(GlobalState &gs, const TypePtr &typePtr) const;
    SymbolRef dealias(const GlobalState &gs) const;
    // End methods that should be removed.

    // Prints the fully qualified name of the symbol in a format that is suitable for showing to the user (e.g.
    // "Owner::SymbolName")
    std::string showFullName(const GlobalState &gs) const;
    std::string toStringFullName(const GlobalState &gs) const;

    std::string showRaw(const GlobalState &gs) const {
        bool showFull = false;
        bool showRaw = true;
        return toStringWithOptions(gs, 0, showFull, showRaw);
    }
    std::string toString(const GlobalState &gs) const {
        bool showFull = false;
        bool showRaw = false;
        return toStringWithOptions(gs, 0, showFull, showRaw);
    }
    // Renders the full name of this Symbol in a form suitable for user display.
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;

    /*
     * Returns true if symbol is under the namespace of otherClass. Foo::Bar::A is under its own namespace, also is
     * under the namespace of Foo::Bar and Foo.
     */
    bool isUnderNamespace(const GlobalState &gs, core::ClassOrModuleRef otherClass) const;
};
CheckSize(SymbolRef, 4, 4);

class Symbols {
    Symbols() = delete;

public:
    static SymbolRef noSymbol() {
        return SymbolRef();
    }

    static ClassOrModuleRef noClassOrModule() {
        return ClassOrModuleRef();
    }

    static ClassOrModuleRef top() {
        return ClassOrModuleRef::fromRaw(1);
    }

    static ClassOrModuleRef bottom() {
        return ClassOrModuleRef::fromRaw(2);
    }

    static ClassOrModuleRef root() {
        return ClassOrModuleRef::fromRaw(3);
    }

    static ClassOrModuleRef rootSingleton() {
        return ClassOrModuleRef::fromRaw(4);
    }

    static ClassOrModuleRef todo() {
        return ClassOrModuleRef::fromRaw(5);
    }

    static ClassOrModuleRef Object() {
        return ClassOrModuleRef::fromRaw(6);
    }

    static ClassOrModuleRef Integer() {
        return ClassOrModuleRef::fromRaw(7);
    }

    static ClassOrModuleRef Float() {
        return ClassOrModuleRef::fromRaw(8);
    }

    static ClassOrModuleRef String() {
        return ClassOrModuleRef::fromRaw(9);
    }

    static ClassOrModuleRef Symbol() {
        return ClassOrModuleRef::fromRaw(10);
    }

    static ClassOrModuleRef Array() {
        return ClassOrModuleRef::fromRaw(11);
    }

    static ClassOrModuleRef Hash() {
        return ClassOrModuleRef::fromRaw(12);
    }

    static ClassOrModuleRef TrueClass() {
        return ClassOrModuleRef::fromRaw(13);
    }

    static ClassOrModuleRef FalseClass() {
        return ClassOrModuleRef::fromRaw(14);
    }

    static ClassOrModuleRef NilClass() {
        return ClassOrModuleRef::fromRaw(15);
    }

    static ClassOrModuleRef untyped() {
        return ClassOrModuleRef::fromRaw(16);
    }

    static ClassOrModuleRef T() {
        return ClassOrModuleRef::fromRaw(17);
    }

    static ClassOrModuleRef TSingleton() {
        return ClassOrModuleRef::fromRaw(18);
    }

    static ClassOrModuleRef Class() {
        return ClassOrModuleRef::fromRaw(19);
    }

    static ClassOrModuleRef BasicObject() {
        return ClassOrModuleRef::fromRaw(20);
    }

    static ClassOrModuleRef Kernel() {
        return ClassOrModuleRef::fromRaw(21);
    }

    static ClassOrModuleRef Range() {
        return ClassOrModuleRef::fromRaw(22);
    }

    static ClassOrModuleRef Regexp() {
        return ClassOrModuleRef::fromRaw(23);
    }

    static ClassOrModuleRef Magic() {
        return ClassOrModuleRef::fromRaw(24);
    }

    static ClassOrModuleRef MagicSingleton() {
        return ClassOrModuleRef::fromRaw(25);
    }

    static ClassOrModuleRef Module() {
        return ClassOrModuleRef::fromRaw(26);
    }

    static ClassOrModuleRef Exception() {
        return ClassOrModuleRef::fromRaw(27);
    }

    static ClassOrModuleRef StandardError() {
        return ClassOrModuleRef::fromRaw(28);
    }

    static ClassOrModuleRef Complex() {
        return ClassOrModuleRef::fromRaw(29);
    }

    static ClassOrModuleRef Rational() {
        return ClassOrModuleRef::fromRaw(30);
    }

    static ClassOrModuleRef T_Array() {
        return ClassOrModuleRef::fromRaw(31);
    }

    static ClassOrModuleRef T_Hash() {
        return ClassOrModuleRef::fromRaw(32);
    }

    static ClassOrModuleRef T_Proc() {
        return ClassOrModuleRef::fromRaw(33);
    }

    static ClassOrModuleRef Proc() {
        return ClassOrModuleRef::fromRaw(34);
    }

    static ClassOrModuleRef Enumerable() {
        return ClassOrModuleRef::fromRaw(35);
    }

    static ClassOrModuleRef Set() {
        return ClassOrModuleRef::fromRaw(36);
    }

    static ClassOrModuleRef Struct() {
        return ClassOrModuleRef::fromRaw(37);
    }

    static ClassOrModuleRef File() {
        return ClassOrModuleRef::fromRaw(38);
    }

    static ClassOrModuleRef Sorbet() {
        return ClassOrModuleRef::fromRaw(39);
    }

    static ClassOrModuleRef Sorbet_Private() {
        return ClassOrModuleRef::fromRaw(40);
    }

    static ClassOrModuleRef Sorbet_Private_Static() {
        return ClassOrModuleRef::fromRaw(41);
    }

    static ClassOrModuleRef Sorbet_Private_StaticSingleton() {
        return ClassOrModuleRef::fromRaw(42);
    }

    // Used as the superclass for symbols created to populate unresolvable ruby
    // constants
    static ClassOrModuleRef StubModule() {
        return ClassOrModuleRef::fromRaw(43);
    }

    // Used to mark the presence of a mixin that we were unable to
    // statically resolve to a module
    static ClassOrModuleRef StubMixin() {
        return ClassOrModuleRef::fromRaw(44);
    }

    // Used to mark the presence of a mixin that will be replaced with a real
    // ClassOrModuleRef or StubMixin once resolution completes.
    static ClassOrModuleRef PlaceholderMixin() {
        return ClassOrModuleRef::fromRaw(45);
    }

    // Used to mark the presence of a superclass that we were unable to
    // statically resolve to a class
    static ClassOrModuleRef StubSuperClass() {
        return ClassOrModuleRef::fromRaw(46);
    }

    static ClassOrModuleRef T_Enumerable() {
        return ClassOrModuleRef::fromRaw(47);
    }

    static ClassOrModuleRef T_Range() {
        return ClassOrModuleRef::fromRaw(48);
    }

    static ClassOrModuleRef T_Set() {
        return ClassOrModuleRef::fromRaw(49);
    }

    static ClassOrModuleRef void_() {
        return ClassOrModuleRef::fromRaw(50);
    }

    // Synthetic symbol used by resolver to mark type alias assignments.
    static ClassOrModuleRef typeAliasTemp() {
        return ClassOrModuleRef::fromRaw(51);
    }

    static ClassOrModuleRef T_Configuration() {
        return ClassOrModuleRef::fromRaw(52);
    }

    static ClassOrModuleRef T_Generic() {
        return ClassOrModuleRef::fromRaw(53);
    }

    static ClassOrModuleRef Tuple() {
        return ClassOrModuleRef::fromRaw(54);
    }

    static ClassOrModuleRef Shape() {
        return ClassOrModuleRef::fromRaw(55);
    }

    static ClassOrModuleRef Subclasses() {
        return ClassOrModuleRef::fromRaw(56);
    }

    static ClassOrModuleRef Sorbet_Private_Static_ImplicitModuleSuperClass() {
        return ClassOrModuleRef::fromRaw(57);
    }

    static ClassOrModuleRef Sorbet_Private_Static_ReturnTypeInference() {
        return ClassOrModuleRef::fromRaw(58);
    }

    static MethodRef noMethod() {
        return MethodRef();
    }

    static FieldRef noField() {
        return FieldRef::fromRaw(0);
    }

    static TypeArgumentRef noTypeArgument() {
        return TypeArgumentRef::fromRaw(0);
    }

    static TypeMemberRef noTypeMember() {
        return TypeMemberRef::fromRaw(0);
    }

    static MethodRef Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder() {
        return MethodRef::fromRaw(1);
    }

    static TypeArgumentRef
    Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant() {
        return TypeArgumentRef::fromRaw(1);
    }

    static TypeArgumentRef
    Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant() {
        return TypeArgumentRef::fromRaw(2);
    }

    static TypeArgumentRef todoTypeArgument() {
        return TypeArgumentRef::fromRaw(3);
    }

    static ClassOrModuleRef T_Sig() {
        return ClassOrModuleRef::fromRaw(59);
    }

    static FieldRef Magic_undeclaredFieldStub() {
        return FieldRef::fromRaw(1);
    }

    static MethodRef Sorbet_Private_Static_badAliasMethodStub() {
        return MethodRef::fromRaw(2);
    }

    static ClassOrModuleRef T_Helpers() {
        return ClassOrModuleRef::fromRaw(60);
    }

    static ClassOrModuleRef DeclBuilderForProcs() {
        return ClassOrModuleRef::fromRaw(61);
    }

    static ClassOrModuleRef DeclBuilderForProcsSingleton() {
        return ClassOrModuleRef::fromRaw(62);
    }

    static ClassOrModuleRef Net() {
        return ClassOrModuleRef::fromRaw(63);
    }

    static ClassOrModuleRef Net_IMAP() {
        return ClassOrModuleRef::fromRaw(64);
    }

    static ClassOrModuleRef Net_Protocol() {
        return ClassOrModuleRef::fromRaw(65);
    }

    static ClassOrModuleRef T_Sig_WithoutRuntime() {
        return ClassOrModuleRef::fromRaw(66);
    }

    static ClassOrModuleRef Enumerator() {
        return ClassOrModuleRef::fromRaw(67);
    }

    static ClassOrModuleRef T_Enumerator() {
        return ClassOrModuleRef::fromRaw(68);
    }

    static ClassOrModuleRef T_Enumerator_Lazy() {
        return ClassOrModuleRef::fromRaw(69);
    }

    static ClassOrModuleRef T_Enumerator_Chain() {
        return ClassOrModuleRef::fromRaw(70);
    }

    static ClassOrModuleRef T_Struct() {
        return ClassOrModuleRef::fromRaw(71);
    }

    static ClassOrModuleRef Singleton() {
        return ClassOrModuleRef::fromRaw(72);
    }

    static ClassOrModuleRef T_Enum() {
        return ClassOrModuleRef::fromRaw(73);
    }

    static MethodRef sig() {
        return MethodRef::fromRaw(3);
    }

    static ClassOrModuleRef Enumerator_Lazy() {
        return ClassOrModuleRef::fromRaw(74);
    }

    static ClassOrModuleRef Enumerator_Chain() {
        return ClassOrModuleRef::fromRaw(75);
    }

    static ClassOrModuleRef T_Private() {
        return ClassOrModuleRef::fromRaw(76);
    }

    static ClassOrModuleRef T_Private_Types() {
        return ClassOrModuleRef::fromRaw(77);
    }

    static ClassOrModuleRef T_Private_Types_Void() {
        return ClassOrModuleRef::fromRaw(78);
    }

    static ClassOrModuleRef T_Private_Types_Void_VOID() {
        return ClassOrModuleRef::fromRaw(79);
    }

    static ClassOrModuleRef T_Private_Types_Void_VOIDSingleton() {
        return ClassOrModuleRef::fromRaw(80);
    }

    static ClassOrModuleRef T_Private_Methods() {
        return ClassOrModuleRef::fromRaw(81);
    }

    static ClassOrModuleRef T_Private_Methods_DeclBuilder() {
        return ClassOrModuleRef::fromRaw(82);
    }

    static ClassOrModuleRef T_Sig_WithoutRuntimeSingleton() {
        return ClassOrModuleRef::fromRaw(83);
    }

    static MethodRef sigWithoutRuntime() {
        return MethodRef::fromRaw(4);
    }

    static ClassOrModuleRef T_NonForcingConstants() {
        return ClassOrModuleRef::fromRaw(84);
    }

    static MethodRef SorbetPrivateStaticSingleton_sig() {
        return MethodRef::fromRaw(5);
    }

    static ClassOrModuleRef PackageSpecRegistry() {
        return ClassOrModuleRef::fromRaw(85);
    }

    static ClassOrModuleRef PackageSpec() {
        return ClassOrModuleRef::fromRaw(86);
    }

    static ClassOrModuleRef PackageSpecSingleton() {
        return ClassOrModuleRef::fromRaw(87);
    }

    static MethodRef PackageSpec_import() {
        return MethodRef::fromRaw(6);
    }

    static MethodRef PackageSpec_test_import() {
        return MethodRef::fromRaw(7);
    }

    static MethodRef PackageSpec_export() {
        return MethodRef::fromRaw(8);
    }

    static MethodRef PackageSpec_restrict_to_service() {
        return MethodRef::fromRaw(9);
    }

    static ClassOrModuleRef Encoding() {
        return ClassOrModuleRef::fromRaw(88);
    }

    static ClassOrModuleRef Thread() {
        return ClassOrModuleRef::fromRaw(89);
    }

    static MethodRef Class_new() {
        return MethodRef::fromRaw(10);
    }

    static MethodRef todoMethod() {
        return MethodRef::fromRaw(11);
    }

    static MethodRef rootStaticInit() {
        return MethodRef::fromRaw(12);
    }

    static MethodRef PackageSpec_autoloader_compatibility() {
        return MethodRef::fromRaw(13);
    }

    static MethodRef PackageSpec_visible_to() {
        return MethodRef::fromRaw(14);
    }

    static MethodRef PackageSpec_export_all() {
        return MethodRef::fromRaw(15);
    }

    static ClassOrModuleRef Sorbet_Private_Static_ResolvedSig() {
        return ClassOrModuleRef::fromRaw(90);
    }

    static ClassOrModuleRef Sorbet_Private_Static_ResolvedSigSingleton() {
        return ClassOrModuleRef::fromRaw(91);
    }

    static ClassOrModuleRef T_Private_Compiler() {
        return ClassOrModuleRef::fromRaw(92);
    }

    static ClassOrModuleRef T_Private_CompilerSingleton() {
        return ClassOrModuleRef::fromRaw(93);
    }

    static ClassOrModuleRef MagicBindToAttachedClass() {
        return ClassOrModuleRef::fromRaw(94);
    }

    static ClassOrModuleRef MagicBindToSelfType() {
        return ClassOrModuleRef::fromRaw(95);
    }

    static ClassOrModuleRef T_Types() {
        return ClassOrModuleRef::fromRaw(96);
    }

    static ClassOrModuleRef T_Types_Base() {
        return ClassOrModuleRef::fromRaw(97);
    }

    static ClassOrModuleRef Data() {
        return ClassOrModuleRef::fromRaw(98);
    }

    static ClassOrModuleRef T_Class() {
        return ClassOrModuleRef::fromRaw(99);
    }

    static MethodRef T_Generic_squareBrackets() {
        return MethodRef::fromRaw(16);
    }

    static ClassOrModuleRef Magic_UntypedSource() {
        return ClassOrModuleRef::fromRaw(101);
    }

    static FieldRef Magic_UntypedSource_super() {
        return FieldRef::fromRaw(4);
    }

    static FieldRef Magic_UntypedSource_proc() {
        return FieldRef::fromRaw(5);
    }

    static constexpr int MAX_PROC_ARITY = 10;
    static ClassOrModuleRef Proc0() {
        return ClassOrModuleRef::fromRaw(MAX_SYNTHETIC_CLASS_SYMBOLS - MAX_PROC_ARITY * 2 - 2);
    }

    static ClassOrModuleRef Proc(int argc) {
        if (argc > MAX_PROC_ARITY) {
            return ClassOrModuleRef();
        }
        return ClassOrModuleRef::fromRaw(Proc0().id() + argc * 2);
    }

    static ClassOrModuleRef last_proc() {
        return Proc(MAX_PROC_ARITY);
    }

    // Keep as last and update to match the last entry
    static ClassOrModuleRef last_synthetic_class_sym() {
        ENFORCE(last_proc().id() == MAX_SYNTHETIC_CLASS_SYMBOLS - 2);
        return ClassOrModuleRef::fromRaw(MAX_SYNTHETIC_CLASS_SYMBOLS - 1);
    }

    static const int MAX_SYNTHETIC_CLASS_SYMBOLS;
    static const int MAX_SYNTHETIC_METHOD_SYMBOLS;
    static const int MAX_SYNTHETIC_FIELD_SYMBOLS;
    static const int MAX_SYNTHETIC_TYPEARGUMENT_SYMBOLS;
    static const int MAX_SYNTHETIC_TYPEMEMBER_SYMBOLS;
};

template <typename H> H AbslHashValue(H h, const SymbolRef &m) {
    return H::combine(std::move(h), m.rawId());
}

template <typename H> H AbslHashValue(H h, const ClassOrModuleRef &m) {
    return H::combine(std::move(h), m.id());
}

template <typename H> H AbslHashValue(H h, const MethodRef &m) {
    return H::combine(std::move(h), m.id());
}

template <typename H> H AbslHashValue(H h, const FieldRef &m) {
    return H::combine(std::move(h), m.id());
}

template <typename H> H AbslHashValue(H h, const TypeArgumentRef &m) {
    return H::combine(std::move(h), m.id());
}

template <typename H> H AbslHashValue(H h, const TypeMemberRef &m) {
    return H::combine(std::move(h), m.id());
}

} // namespace sorbet::core
#endif // RUBY_TYPER_SYMBOLREF_H
