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

class SymbolRef final {
    friend class GlobalState;
    friend class Symbol;

public:
    SymbolRef(GlobalState const *from, u4 _id);
    SymbolRef(const GlobalState &from, u4 _id);
    SymbolRef() : _id(0){};

    // From experimentation, in the common case, methods typically have 2 or fewer arguments.
    // Placed here so it can be used across packages for common case optimizations.
    static constexpr int EXPECTED_METHOD_ARGS_COUNT = 2;

    bool inline exists() const {
        return _id;
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

    u4 _id;
};
CheckSize(SymbolRef, 4, 4);

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

    static SymbolRef rootSingleton() {
        return SymbolRef(nullptr, 4);
    }

    static SymbolRef todo() {
        return SymbolRef(nullptr, 6);
    }

    static SymbolRef Object() {
        return SymbolRef(nullptr, 7);
    }

    static SymbolRef Integer() {
        return SymbolRef(nullptr, 8);
    }

    static SymbolRef Float() {
        return SymbolRef(nullptr, 9);
    }

    static SymbolRef String() {
        return SymbolRef(nullptr, 10);
    }

    static SymbolRef Symbol() {
        return SymbolRef(nullptr, 11);
    }

    static SymbolRef Array() {
        return SymbolRef(nullptr, 12);
    }

    static SymbolRef Hash() {
        return SymbolRef(nullptr, 13);
    }

    static SymbolRef TrueClass() {
        return SymbolRef(nullptr, 14);
    }

    static SymbolRef FalseClass() {
        return SymbolRef(nullptr, 15);
    }

    static SymbolRef NilClass() {
        return SymbolRef(nullptr, 16);
    }

    static SymbolRef untyped() {
        return SymbolRef(nullptr, 17);
    }

    static SymbolRef Opus() {
        return SymbolRef(nullptr, 18);
    }

    static SymbolRef T() {
        return SymbolRef(nullptr, 19);
    }

    static SymbolRef Class() {
        return SymbolRef(nullptr, 20);
    }

    static SymbolRef BasicObject() {
        return SymbolRef(nullptr, 21);
    }

    static SymbolRef Kernel() {
        return SymbolRef(nullptr, 22);
    }

    static SymbolRef Range() {
        return SymbolRef(nullptr, 23);
    }

    static SymbolRef Regexp() {
        return SymbolRef(nullptr, 24);
    }

    static SymbolRef Magic() {
        return SymbolRef(nullptr, 25);
    }

    static SymbolRef MagicSingleton() {
        return SymbolRef(nullptr, 26);
    }

    static SymbolRef Module() {
        return SymbolRef(nullptr, 28);
    }

    static SymbolRef StandardError() {
        return SymbolRef(nullptr, 29);
    }

    static SymbolRef Complex() {
        return SymbolRef(nullptr, 30);
    }

    static SymbolRef Rational() {
        return SymbolRef(nullptr, 31);
    }

    static SymbolRef T_Array() {
        return SymbolRef(nullptr, 32);
    }

    static SymbolRef T_Hash() {
        return SymbolRef(nullptr, 33);
    }

    static SymbolRef T_Proc() {
        return SymbolRef(nullptr, 34);
    }

    static SymbolRef Proc() {
        return SymbolRef(nullptr, 35);
    }

    static SymbolRef Enumerable() {
        return SymbolRef(nullptr, 36);
    }

    static SymbolRef Set() {
        return SymbolRef(nullptr, 37);
    }

    static SymbolRef Struct() {
        return SymbolRef(nullptr, 38);
    }

    static SymbolRef File() {
        return SymbolRef(nullptr, 39);
    }

    static SymbolRef Sorbet() {
        return SymbolRef(nullptr, 40);
    }

    static SymbolRef Sorbet_Private() {
        return SymbolRef(nullptr, 41);
    }

    static SymbolRef Sorbet_Private_Static() {
        return SymbolRef(nullptr, 42);
    }

    static SymbolRef Sorbet_Private_StaticSingleton() {
        return SymbolRef(nullptr, 43);
    }

    // 44 is the attached class of Sorbet_Private_StaticSingleton

    // Used as the superclass for symbols created to populate unresolvable ruby
    // constants
    static SymbolRef StubModule() {
        return SymbolRef(nullptr, 45);
    }

    // Used to mark the presence of a mixin that we were unable to
    // statically resolve to a module
    static SymbolRef StubMixin() {
        return SymbolRef(nullptr, 46);
    }

    // Used to mark the presence of a superclass that we were unable to
    // statically resolve to a class
    static SymbolRef StubSuperClass() {
        return SymbolRef(nullptr, 47);
    }

    static SymbolRef T_Enumerable() {
        return SymbolRef(nullptr, 48);
    }

    static SymbolRef T_Range() {
        return SymbolRef(nullptr, 49);
    }

    static SymbolRef T_Set() {
        return SymbolRef(nullptr, 50);
    }

    static SymbolRef Configatron() {
        return SymbolRef(nullptr, 51);
    }

    static SymbolRef Configatron_Store() {
        return SymbolRef(nullptr, 52);
    }

    static SymbolRef Configatron_RootStore() {
        return SymbolRef(nullptr, 53);
    }

    static SymbolRef void_() {
        return SymbolRef(nullptr, 54);
    }

    // Synthetic symbol used by resolver to mark type alias assignments.
    static SymbolRef typeAliasTemp() {
        return SymbolRef(nullptr, 55);
    }

    static SymbolRef Chalk() {
        return SymbolRef(nullptr, 56);
    }

    static SymbolRef Chalk_Tools() {
        return SymbolRef(nullptr, 57);
    }

    static SymbolRef Chalk_Tools_Accessible() {
        return SymbolRef(nullptr, 58);
    }

    static SymbolRef T_Generic() {
        return SymbolRef(nullptr, 59);
    }

    static SymbolRef Tuple() {
        return SymbolRef(nullptr, 60);
    }

    static SymbolRef Shape() {
        return SymbolRef(nullptr, 61);
    }

    static SymbolRef Subclasses() {
        return SymbolRef(nullptr, 62);
    }

    static SymbolRef Sorbet_Private_Static_ImplicitModuleSuperClass() {
        return SymbolRef(nullptr, 63);
    }

    static SymbolRef Sorbet_Private_Static_ReturnTypeInference() {
        return SymbolRef(nullptr, 64);
    }

    static SymbolRef Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder() {
        return SymbolRef(nullptr, 65);
    }

    static SymbolRef
    Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant() {
        return SymbolRef(nullptr, 66);
    }

    static SymbolRef Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant() {
        return SymbolRef(nullptr, 67);
    }

    static SymbolRef T_Sig() {
        return SymbolRef(nullptr, 68);
    }

    static SymbolRef Magic_undeclaredFieldStub() {
        return SymbolRef(nullptr, 69);
    }

    static SymbolRef Sorbet_Private_Static_badAliasMethodStub() {
        return SymbolRef(nullptr, 70);
    }

    static SymbolRef T_Helpers() {
        return SymbolRef(nullptr, 71);
    }

    static SymbolRef DeclBuilderForProcs() {
        return SymbolRef(nullptr, 72);
    }

    static SymbolRef DeclBuilderForProcsSingleton() {
        return SymbolRef(nullptr, 73);
    }

    // 74 is the attached class of DeclBuilderForProcsSingleton

    static SymbolRef Net() {
        return SymbolRef(nullptr, 75);
    }

    static SymbolRef Net_IMAP() {
        return SymbolRef(nullptr, 76);
    }

    static SymbolRef Net_Protocol() {
        return SymbolRef(nullptr, 77);
    }

    static SymbolRef T_Sig_WithoutRuntime() {
        return SymbolRef(nullptr, 78);
    }

    static SymbolRef Enumerator() {
        return SymbolRef(nullptr, 79);
    }

    static SymbolRef T_Enumerator() {
        return SymbolRef(nullptr, 80);
    }

    static SymbolRef T_Struct() {
        return SymbolRef(nullptr, 81);
    }

    static SymbolRef Singleton() {
        return SymbolRef(nullptr, 82);
    }

    static SymbolRef T_Enum() {
        return SymbolRef(nullptr, 83);
    }

    static SymbolRef sig() {
        return SymbolRef(nullptr, 84);
    }

    static SymbolRef Enumerator_Lazy() {
        return SymbolRef(nullptr, 85);
    }

    static SymbolRef T_Private() {
        return SymbolRef(nullptr, 86);
    }

    static SymbolRef T_Private_Types() {
        return SymbolRef(nullptr, 87);
    }

    static SymbolRef T_Private_Types_Void() {
        return SymbolRef(nullptr, 88);
    }

    static SymbolRef T_Private_Types_Void_VOID() {
        return SymbolRef(nullptr, 89);
    }

    static SymbolRef T_Private_Types_Void_VOIDSingleton() {
        return SymbolRef(nullptr, 90);
    }

    // 91 is the attached class of VOIDSingleton

    static SymbolRef T_Sig_WithoutRuntimeSingleton() {
        return SymbolRef(nullptr, 92);
    }

    // 93 is the attached class of T_Sig_WithoutRuntimeSingleton

    static SymbolRef sigWithoutRuntime() {
        return SymbolRef(nullptr, 94);
    }

    static SymbolRef T_NonForcingConstants() {
        return SymbolRef(nullptr, 95);
    }

    static SymbolRef Chalk_ODM() {
        return SymbolRef(nullptr, 96);
    }

    static SymbolRef Chalk_ODM_DocumentDecoratorHelper() {
        return SymbolRef(nullptr, 97);
    }

    static SymbolRef SorbetPrivateStaticSingleton_sig() {
        return SymbolRef(nullptr, 98);
    }

    static SymbolRef PackageRegistry() {
        return SymbolRef(nullptr, 99);
    }

    static constexpr int MAX_PROC_ARITY = 10;
    static SymbolRef Proc0() {
        return SymbolRef(nullptr, MAX_SYNTHETIC_SYMBOLS - MAX_PROC_ARITY * 3 - 3);
    }

    static SymbolRef Proc(int argc) {
        if (argc > MAX_PROC_ARITY) {
            return noSymbol();
        }
        return SymbolRef(nullptr, Proc0()._id + argc * 3);
    }

    static SymbolRef last_proc() {
        return Proc(MAX_PROC_ARITY);
    }

    // Keep as last and update to match the last entry
    static SymbolRef last_synthetic_sym() {
        ENFORCE(last_proc()._id == MAX_SYNTHETIC_SYMBOLS - 3);
        return SymbolRef(nullptr, MAX_SYNTHETIC_SYMBOLS - 1);
    }

    static constexpr int MAX_SYNTHETIC_SYMBOLS = 400;
};

template <typename H> H AbslHashValue(H h, const SymbolRef &m) {
    return H::combine(std::move(h), m._id);
}

} // namespace sorbet::core
#endif // RUBY_TYPER_SYMBOLREF_H
