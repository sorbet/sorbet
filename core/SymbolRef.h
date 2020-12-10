#ifndef RUBY_TYPER_SYMBOLREF_H
#define RUBY_TYPER_SYMBOLREF_H

#include "common/common.h"
#include "core/DebugOnlyCheck.h"

namespace sorbet::core {
class Symbol;
class GlobalState;
struct SymbolDataDebugCheck {
    const GlobalState &gs;
    const unsigned int symbolCountAtCreation;

    SymbolDataDebugCheck(const GlobalState &gs);
    void check() const;
};

/** This class is intended to be a safe way to pass `Symbol &` around.
 *  Entering new symbols can invalidate `Symbol &`s and thus they are generally unsafe.
 *  This class ensures that all accesses are safe in debug builds and effectively is a `Symbol &` in optimized builds.
 */
class SymbolData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    Symbol &symbol;

public:
    SymbolData(Symbol &ref, GlobalState &gs);

    Symbol *operator->();
    const Symbol *operator->() const;
};
CheckSize(SymbolData, 8, 8);

class ConstSymbolData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    const Symbol &symbol;

public:
    ConstSymbolData(const Symbol &ref, const GlobalState &gs);

    const Symbol *operator->() const;
};
CheckSize(ConstSymbolData, 8, 8);

class Symbol;

class ClassOrModuleRef final {
    u4 _id;

public:
    ClassOrModuleRef() : _id(0){};
    ClassOrModuleRef(const GlobalState &from, u4 id);

    u4 id() const {
        return _id;
    }

    bool exists() const {
        return _id != 0;
    }

    static ClassOrModuleRef fromRaw(u4 id) {
        ClassOrModuleRef ref;
        ref._id = id;
        return ref;
    }

    SymbolData data(GlobalState &gs) const;
    ConstSymbolData data(const GlobalState &gs) const;

    bool operator==(const ClassOrModuleRef &rhs) const;

    bool operator!=(const ClassOrModuleRef &rhs) const;
};
CheckSize(ClassOrModuleRef, 4, 4);

class SymbolRef final {
    friend class GlobalState;
    friend class Symbol;

    // Stores the symbol's Kind and Index. Kind occupies the upper bits.
    u4 _id;
    u4 unsafeTableIndex() const {
        return _id & ID_MASK;
    }

public:
    // If you add Symbol Kinds, make sure KIND_BITS is kept in sync!
    enum class Kind : u1 {
        ClassOrModule = 0,
        Method = 1,
        FieldOrStaticField = 2,
        TypeArgument = 3,
        TypeMember = 4,
    };

    // Kind takes up this many bits in _id.
    static constexpr u4 KIND_BITS = 3;
    static constexpr u4 ID_BITS = 32 - KIND_BITS;
    static constexpr u4 ID_MASK = (1 << ID_BITS) - 1;

    Kind kind() const {
        return static_cast<Kind>(_id >> ID_BITS);
    }

    u4 rawId() const {
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

    bool isField(const GlobalState &gs) const;
    bool isStaticField(const GlobalState &gs) const;

    u4 classOrModuleIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::ClassOrModule);
        return unsafeTableIndex();
    }

    u4 methodIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::Method);
        return unsafeTableIndex();
    }

    u4 fieldIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::FieldOrStaticField);
        return unsafeTableIndex();
    }

    u4 typeArgumentIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::TypeArgument);
        return unsafeTableIndex();
    }

    u4 typeMemberIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::TypeMember);
        return unsafeTableIndex();
    }

    SymbolRef(GlobalState const *from, Kind type, u4 id);
    SymbolRef(const GlobalState &from, Kind type, u4 id);
    SymbolRef(ClassOrModuleRef kls);
    SymbolRef() : _id(0){};

    // From experimentation, in the common case, methods typically have 2 or fewer arguments.
    // Placed here so it can be used across packages for common case optimizations.
    static constexpr int EXPECTED_METHOD_ARGS_COUNT = 2;

    static SymbolRef fromRaw(u4 raw) {
        auto ref = SymbolRef();
        ref._id = raw;
        return ref;
    }

    bool inline exists() const {
        // 0th index is reserved on all symbol vectors for the non existant symbol.
        return unsafeTableIndex() != 0;
    }

    bool isSynthetic() const;

    // If Kind is ClassOrModule, returns a ClassOrModuleRef.
    ClassOrModuleRef asClassOrModuleRef() const;

    SymbolData data(GlobalState &gs) const;
    ConstSymbolData data(const GlobalState &gs) const;
    SymbolData dataAllowingNone(GlobalState &gs) const;
    ConstSymbolData dataAllowingNone(const GlobalState &gs) const;

    bool operator==(const SymbolRef &rhs) const;

    bool operator!=(const SymbolRef &rhs) const;

    std::string showRaw(const GlobalState &gs) const;
    std::string toString(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const;
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

    static ClassOrModuleRef Opus() {
        return ClassOrModuleRef::fromRaw(17);
    }

    static ClassOrModuleRef T() {
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

    static ClassOrModuleRef StandardError() {
        return ClassOrModuleRef::fromRaw(27);
    }

    static ClassOrModuleRef Complex() {
        return ClassOrModuleRef::fromRaw(28);
    }

    static ClassOrModuleRef Rational() {
        return ClassOrModuleRef::fromRaw(29);
    }

    static ClassOrModuleRef T_Array() {
        return ClassOrModuleRef::fromRaw(30);
    }

    static ClassOrModuleRef T_Hash() {
        return ClassOrModuleRef::fromRaw(31);
    }

    static ClassOrModuleRef T_Proc() {
        return ClassOrModuleRef::fromRaw(32);
    }

    static ClassOrModuleRef Proc() {
        return ClassOrModuleRef::fromRaw(33);
    }

    static ClassOrModuleRef Enumerable() {
        return ClassOrModuleRef::fromRaw(34);
    }

    static ClassOrModuleRef Set() {
        return ClassOrModuleRef::fromRaw(35);
    }

    static ClassOrModuleRef Struct() {
        return ClassOrModuleRef::fromRaw(36);
    }

    static ClassOrModuleRef File() {
        return ClassOrModuleRef::fromRaw(37);
    }

    static ClassOrModuleRef Sorbet() {
        return ClassOrModuleRef::fromRaw(38);
    }

    static ClassOrModuleRef Sorbet_Private() {
        return ClassOrModuleRef::fromRaw(39);
    }

    static ClassOrModuleRef Sorbet_Private_Static() {
        return ClassOrModuleRef::fromRaw(40);
    }

    static ClassOrModuleRef Sorbet_Private_StaticSingleton() {
        return ClassOrModuleRef::fromRaw(41);
    }

    // Used as the superclass for symbols created to populate unresolvable ruby
    // constants
    static ClassOrModuleRef StubModule() {
        return ClassOrModuleRef::fromRaw(42);
    }

    // Used to mark the presence of a mixin that we were unable to
    // statically resolve to a module
    static ClassOrModuleRef StubMixin() {
        return ClassOrModuleRef::fromRaw(43);
    }

    // Used to mark the presence of a superclass that we were unable to
    // statically resolve to a class
    static ClassOrModuleRef StubSuperClass() {
        return ClassOrModuleRef::fromRaw(44);
    }

    static ClassOrModuleRef T_Enumerable() {
        return ClassOrModuleRef::fromRaw(45);
    }

    static ClassOrModuleRef T_Range() {
        return ClassOrModuleRef::fromRaw(46);
    }

    static ClassOrModuleRef T_Set() {
        return ClassOrModuleRef::fromRaw(47);
    }

    static ClassOrModuleRef Configatron() {
        return ClassOrModuleRef::fromRaw(48);
    }

    static ClassOrModuleRef Configatron_Store() {
        return ClassOrModuleRef::fromRaw(49);
    }

    static ClassOrModuleRef Configatron_RootStore() {
        return ClassOrModuleRef::fromRaw(50);
    }

    static ClassOrModuleRef void_() {
        return ClassOrModuleRef::fromRaw(51);
    }

    // Synthetic symbol used by resolver to mark type alias assignments.
    static ClassOrModuleRef typeAliasTemp() {
        return ClassOrModuleRef::fromRaw(52);
    }

    static ClassOrModuleRef Chalk() {
        return ClassOrModuleRef::fromRaw(53);
    }

    static ClassOrModuleRef Chalk_Tools() {
        return ClassOrModuleRef::fromRaw(54);
    }

    static ClassOrModuleRef Chalk_Tools_Accessible() {
        return ClassOrModuleRef::fromRaw(55);
    }

    static ClassOrModuleRef T_Generic() {
        return ClassOrModuleRef::fromRaw(56);
    }

    static ClassOrModuleRef Tuple() {
        return ClassOrModuleRef::fromRaw(57);
    }

    static ClassOrModuleRef Shape() {
        return ClassOrModuleRef::fromRaw(58);
    }

    static ClassOrModuleRef Subclasses() {
        return ClassOrModuleRef::fromRaw(59);
    }

    static ClassOrModuleRef Sorbet_Private_Static_ImplicitModuleSuperClass() {
        return ClassOrModuleRef::fromRaw(60);
    }

    static ClassOrModuleRef Sorbet_Private_Static_ReturnTypeInference() {
        return ClassOrModuleRef::fromRaw(61);
    }

    static SymbolRef noMethod() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 0);
    }

    static SymbolRef noField() {
        return SymbolRef(nullptr, SymbolRef::Kind::FieldOrStaticField, 0);
    }

    static SymbolRef noTypeArgument() {
        return SymbolRef(nullptr, SymbolRef::Kind::TypeArgument, 0);
    }

    static SymbolRef noTypeMember() {
        return SymbolRef(nullptr, SymbolRef::Kind::TypeMember, 0);
    }

    static SymbolRef Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 1);
    }

    static SymbolRef
    Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant() {
        return SymbolRef(nullptr, SymbolRef::Kind::TypeArgument, 1);
    }

    static SymbolRef Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant() {
        return SymbolRef(nullptr, SymbolRef::Kind::TypeArgument, 2);
    }

    static ClassOrModuleRef T_Sig() {
        return ClassOrModuleRef::fromRaw(62);
    }

    static SymbolRef Magic_undeclaredFieldStub() {
        return SymbolRef(nullptr, SymbolRef::Kind::FieldOrStaticField, 1);
    }

    static SymbolRef Sorbet_Private_Static_badAliasMethodStub() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 2);
    }

    static ClassOrModuleRef T_Helpers() {
        return ClassOrModuleRef::fromRaw(63);
    }

    static ClassOrModuleRef DeclBuilderForProcs() {
        return ClassOrModuleRef::fromRaw(64);
    }

    static ClassOrModuleRef DeclBuilderForProcsSingleton() {
        return ClassOrModuleRef::fromRaw(65);
    }

    static ClassOrModuleRef Net() {
        return ClassOrModuleRef::fromRaw(66);
    }

    static ClassOrModuleRef Net_IMAP() {
        return ClassOrModuleRef::fromRaw(67);
    }

    static ClassOrModuleRef Net_Protocol() {
        return ClassOrModuleRef::fromRaw(68);
    }

    static ClassOrModuleRef T_Sig_WithoutRuntime() {
        return ClassOrModuleRef::fromRaw(69);
    }

    static ClassOrModuleRef Enumerator() {
        return ClassOrModuleRef::fromRaw(70);
    }

    static ClassOrModuleRef T_Enumerator() {
        return ClassOrModuleRef::fromRaw(71);
    }

    static ClassOrModuleRef T_Struct() {
        return ClassOrModuleRef::fromRaw(72);
    }

    static ClassOrModuleRef Singleton() {
        return ClassOrModuleRef::fromRaw(73);
    }

    static ClassOrModuleRef T_Enum() {
        return ClassOrModuleRef::fromRaw(74);
    }

    static SymbolRef sig() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 3);
    }

    static ClassOrModuleRef Enumerator_Lazy() {
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

    static ClassOrModuleRef T_Sig_WithoutRuntimeSingleton() {
        return ClassOrModuleRef::fromRaw(81);
    }

    static SymbolRef sigWithoutRuntime() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 4);
    }

    static ClassOrModuleRef T_NonForcingConstants() {
        return ClassOrModuleRef::fromRaw(82);
    }

    static ClassOrModuleRef Chalk_ODM() {
        return ClassOrModuleRef::fromRaw(83);
    }

    static ClassOrModuleRef Chalk_ODM_DocumentDecoratorHelper() {
        return ClassOrModuleRef::fromRaw(84);
    }

    static SymbolRef SorbetPrivateStaticSingleton_sig() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 5);
    }

    static ClassOrModuleRef PackageRegistry() {
        return ClassOrModuleRef::fromRaw(85);
    }

    static ClassOrModuleRef PackageSpec() {
        return ClassOrModuleRef::fromRaw(86);
    }

    static ClassOrModuleRef PackageSpecSingleton() {
        return ClassOrModuleRef::fromRaw(87);
    }

    static SymbolRef PackageSpec_import() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 6);
    }

    static SymbolRef PackageSpec_export() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 7);
    }

    static SymbolRef PackageSpec_export_methods() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 8);
    }

    static ClassOrModuleRef Encoding() {
        return ClassOrModuleRef::fromRaw(88);
    }

    static SymbolRef Class_new() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 9);
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

    static constexpr int MAX_SYNTHETIC_CLASS_SYMBOLS = 200;
    static constexpr int MAX_SYNTHETIC_METHOD_SYMBOLS = 33;
    static constexpr int MAX_SYNTHETIC_FIELD_SYMBOLS = 3;
    static constexpr int MAX_SYNTHETIC_TYPEARGUMENT_SYMBOLS = 3;
    static constexpr int MAX_SYNTHETIC_TYPEMEMBER_SYMBOLS = 100;
};

template <typename H> H AbslHashValue(H h, const SymbolRef &m) {
    return H::combine(std::move(h), m.rawId());
}

} // namespace sorbet::core
#endif // RUBY_TYPER_SYMBOLREF_H
