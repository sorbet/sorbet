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

/** This class is indended to be a safe way to pass `Symbol &` around.
 *  Entering new symbols can invalidate `Symbol &`s and thus they are generally unsafe.
 *  This class ensures that all accesses are safe in debug builds and effectively is a `Symbol &` in optimized builds.
 */
class SymbolData : private DebugOnlyCheck<SymbolDataDebugCheck> {
    Symbol &symbol;

public:
    SymbolData(Symbol &ref, const GlobalState &gs);
    Symbol *operator->();
    const Symbol *operator->() const;
};
CheckSize(SymbolData, 8, 8);

class Symbol;

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
        Field = 2,
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

    u4 classOrModuleIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::ClassOrModule);
        return unsafeTableIndex();
    }

    u4 methodIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::Method);
        return unsafeTableIndex();
    }

    u4 fieldIndex() const {
        ENFORCE_NO_TIMER(kind() == Kind::Field);
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
        //  ClassOrModule and id are both 0, for SymbolRef::noSymbol.
        return _id != 0;
    }

    bool isSynthetic() const;

    SymbolData data(GlobalState &gs) const;
    const SymbolData data(const GlobalState &gs) const;
    SymbolData dataAllowingNone(GlobalState &gs) const;
    const SymbolData dataAllowingNone(const GlobalState &gs) const;

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

    static SymbolRef top() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 1);
    }

    static SymbolRef bottom() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 2);
    }

    static SymbolRef root() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 3);
    }

    static SymbolRef rootSingleton() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 4);
    }

    static SymbolRef todo() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 5);
    }

    static SymbolRef Object() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 6);
    }

    static SymbolRef Integer() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 7);
    }

    static SymbolRef Float() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 8);
    }

    static SymbolRef String() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 9);
    }

    static SymbolRef Symbol() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 10);
    }

    static SymbolRef Array() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 11);
    }

    static SymbolRef Hash() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 12);
    }

    static SymbolRef TrueClass() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 13);
    }

    static SymbolRef FalseClass() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 14);
    }

    static SymbolRef NilClass() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 15);
    }

    static SymbolRef untyped() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 16);
    }

    static SymbolRef Opus() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 17);
    }

    static SymbolRef T() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 18);
    }

    static SymbolRef Class() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 19);
    }

    static SymbolRef BasicObject() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 20);
    }

    static SymbolRef Kernel() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 21);
    }

    static SymbolRef Range() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 22);
    }

    static SymbolRef Regexp() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 23);
    }

    static SymbolRef Magic() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 24);
    }

    static SymbolRef MagicSingleton() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 25);
    }

    static SymbolRef Module() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 26);
    }

    static SymbolRef StandardError() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 27);
    }

    static SymbolRef Complex() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 28);
    }

    static SymbolRef Rational() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 29);
    }

    static SymbolRef T_Array() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 30);
    }

    static SymbolRef T_Hash() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 31);
    }

    static SymbolRef T_Proc() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 32);
    }

    static SymbolRef Proc() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 33);
    }

    static SymbolRef Enumerable() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 34);
    }

    static SymbolRef Set() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 35);
    }

    static SymbolRef Struct() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 36);
    }

    static SymbolRef File() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 37);
    }

    static SymbolRef Sorbet() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 38);
    }

    static SymbolRef Sorbet_Private() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 39);
    }

    static SymbolRef Sorbet_Private_Static() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 40);
    }

    static SymbolRef Sorbet_Private_StaticSingleton() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 41);
    }

    // Used as the superclass for symbols created to populate unresolvable ruby
    // constants
    static SymbolRef StubModule() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 42);
    }

    // Used to mark the presence of a mixin that we were unable to
    // statically resolve to a module
    static SymbolRef StubMixin() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 43);
    }

    // Used to mark the presence of a superclass that we were unable to
    // statically resolve to a class
    static SymbolRef StubSuperClass() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 44);
    }

    static SymbolRef T_Enumerable() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 45);
    }

    static SymbolRef T_Range() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 46);
    }

    static SymbolRef T_Set() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 47);
    }

    static SymbolRef Configatron() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 48);
    }

    static SymbolRef Configatron_Store() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 49);
    }

    static SymbolRef Configatron_RootStore() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 50);
    }

    static SymbolRef void_() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 51);
    }

    // Synthetic symbol used by resolver to mark type alias assignments.
    static SymbolRef typeAliasTemp() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 52);
    }

    static SymbolRef Chalk() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 53);
    }

    static SymbolRef Chalk_Tools() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 54);
    }

    static SymbolRef Chalk_Tools_Accessible() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 55);
    }

    static SymbolRef T_Generic() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 56);
    }

    static SymbolRef Tuple() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 57);
    }

    static SymbolRef Shape() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 58);
    }

    static SymbolRef Subclasses() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 59);
    }

    static SymbolRef Sorbet_Private_Static_ImplicitModuleSuperClass() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 60);
    }

    static SymbolRef Sorbet_Private_Static_ReturnTypeInference() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 61);
    }

    static SymbolRef Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 0);
    }

    static SymbolRef
    Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant() {
        return SymbolRef(nullptr, SymbolRef::Kind::TypeArgument, 0);
    }

    static SymbolRef Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant() {
        return SymbolRef(nullptr, SymbolRef::Kind::TypeArgument, 1);
    }

    static SymbolRef T_Sig() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 62);
    }

    static SymbolRef Magic_undeclaredFieldStub() {
        return SymbolRef(nullptr, SymbolRef::Kind::Field, 0);
    }

    static SymbolRef Sorbet_Private_Static_badAliasMethodStub() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 1);
    }

    static SymbolRef T_Helpers() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 63);
    }

    static SymbolRef DeclBuilderForProcs() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 64);
    }

    static SymbolRef DeclBuilderForProcsSingleton() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 65);
    }

    static SymbolRef Net() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 66);
    }

    static SymbolRef Net_IMAP() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 67);
    }

    static SymbolRef Net_Protocol() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 68);
    }

    static SymbolRef T_Sig_WithoutRuntime() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 69);
    }

    static SymbolRef Enumerator() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 70);
    }

    static SymbolRef T_Enumerator() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 71);
    }

    static SymbolRef T_Struct() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 72);
    }

    static SymbolRef Singleton() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 73);
    }

    static SymbolRef T_Enum() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 74);
    }

    static SymbolRef sig() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 2);
    }

    static SymbolRef Enumerator_Lazy() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 75);
    }

    static SymbolRef T_Private() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 76);
    }

    static SymbolRef T_Private_Types() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 77);
    }

    static SymbolRef T_Private_Types_Void() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 78);
    }

    static SymbolRef T_Private_Types_Void_VOID() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 79);
    }

    static SymbolRef T_Private_Types_Void_VOIDSingleton() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 80);
    }

    static SymbolRef T_Sig_WithoutRuntimeSingleton() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 81);
    }

    static SymbolRef sigWithoutRuntime() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 3);
    }

    static SymbolRef T_NonForcingConstants() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 82);
    }

    static SymbolRef Chalk_ODM() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 83);
    }

    static SymbolRef Chalk_ODM_DocumentDecoratorHelper() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 84);
    }

    static SymbolRef SorbetPrivateStaticSingleton_sig() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 4);
    }

    static SymbolRef PackageRegistry() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 85);
    }

    static SymbolRef PackageSpec() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 86);
    }

    static SymbolRef PackageSpecSingleton() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 87);
    }

    static SymbolRef PackageSpec_import() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 5);
    }

    static SymbolRef PackageSpec_export() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 6);
    }

    static SymbolRef PackageSpec_export_methods() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 7);
    }

    static SymbolRef Encoding() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, 88);
    }

    static SymbolRef Class_new() {
        return SymbolRef(nullptr, SymbolRef::Kind::Method, 8);
    }

    static constexpr int MAX_PROC_ARITY = 10;
    static SymbolRef Proc0() {
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, MAX_SYNTHETIC_CLASS_SYMBOLS - MAX_PROC_ARITY * 2 - 2);
    }

    static SymbolRef Proc(int argc) {
        if (argc > MAX_PROC_ARITY) {
            return noSymbol();
        }
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, Proc0().classOrModuleIndex() + argc * 2);
    }

    static SymbolRef last_proc() {
        return Proc(MAX_PROC_ARITY);
    }

    // Keep as last and update to match the last entry
    static SymbolRef last_synthetic_class_sym() {
        ENFORCE(last_proc().classOrModuleIndex() == MAX_SYNTHETIC_CLASS_SYMBOLS - 2);
        return SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, MAX_SYNTHETIC_CLASS_SYMBOLS - 1);
    }

    static constexpr int MAX_SYNTHETIC_CLASS_SYMBOLS = 200;
    static constexpr int MAX_SYNTHETIC_METHOD_SYMBOLS = 32;
    static constexpr int MAX_SYNTHETIC_FIELD_SYMBOLS = 1;
    static constexpr int MAX_SYNTHETIC_TYPEARGUMENT_SYMBOLS = 2;
    static constexpr int MAX_SYNTHETIC_TYPEMEMBER_SYMBOLS = 99;
};

template <typename H> H AbslHashValue(H h, const SymbolRef &m) {
    return H::combine(std::move(h), m.rawId());
}

} // namespace sorbet::core
#endif // RUBY_TYPER_SYMBOLREF_H
