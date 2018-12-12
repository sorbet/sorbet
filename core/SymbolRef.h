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

    bool inline exists() const {
        return _id;
    }

    bool isSynthetic() const;

    SymbolData data(GlobalState &gs, bool allowNone = false) const;
    const SymbolData data(const GlobalState &gs, bool allowNone = false) const;

    bool operator==(const SymbolRef &rhs) const;

    bool operator!=(const SymbolRef &rhs) const;

    std::string toString(const GlobalState &gs, int tabs = 0, bool showHidden = false) const;
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

    // Used as the superclass for symbols created to populate unresolvable ruby
    // constants
    static SymbolRef StubClass() {
        return SymbolRef(nullptr, 37);
    }

    // Used to mark the presence of an ancestor that we were unable to
    // statically resolve to a class or module
    static SymbolRef StubAncestor() {
        return SymbolRef(nullptr, 38);
    }

    static SymbolRef T_Enumerable() {
        return SymbolRef(nullptr, 39);
    }

    static SymbolRef T_Range() {
        return SymbolRef(nullptr, 40);
    }

    static SymbolRef T_Set() {
        return SymbolRef(nullptr, 41);
    }

    static SymbolRef Configatron() {
        return SymbolRef(nullptr, 42);
    }

    static SymbolRef Configatron_Store() {
        return SymbolRef(nullptr, 43);
    }

    static SymbolRef Configatron_RootStore() {
        return SymbolRef(nullptr, 44);
    }

    static SymbolRef Sinatra() {
        return SymbolRef(nullptr, 45);
    }

    static SymbolRef SinatraBase() {
        return SymbolRef(nullptr, 46);
    }

    static SymbolRef void_() {
        return SymbolRef(nullptr, 47);
    }

    // Synthetic symbol used by resolver to mark type alias assignments.
    static SymbolRef typeAliasTemp() {
        return SymbolRef(nullptr, 48);
    }

    static SymbolRef Chalk() {
        return SymbolRef(nullptr, 49);
    }
    static SymbolRef Chalk_Tools() {
        return SymbolRef(nullptr, 50);
    }
    static SymbolRef Chalk_Tools_Accessible() {
        return SymbolRef(nullptr, 51);
    }

    static SymbolRef T_Generic() {
        return SymbolRef(nullptr, 52);
    }

    static SymbolRef Tuple() {
        return SymbolRef(nullptr, 53);
    }

    static SymbolRef Shape() {
        return SymbolRef(nullptr, 54);
    }

    static SymbolRef Subclasses() {
        return SymbolRef(nullptr, 55);
    }

    static SymbolRef Sorbet() {
        return SymbolRef(nullptr, 56);
    }

    static SymbolRef RubyTyper_ImplicitModuleSuperClass() {
        return SymbolRef(nullptr, 57);
    }

    static SymbolRef RubyTyper_ReturnTypeInference() {
        return SymbolRef(nullptr, 58);
    }

    static SymbolRef RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder() {
        return SymbolRef(nullptr, 59);
    }

    static SymbolRef RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant() {
        return SymbolRef(nullptr, 60);
    }

    static SymbolRef RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant() {
        return SymbolRef(nullptr, 61);
    }

    static SymbolRef Sorbet_Private() {
        return SymbolRef(nullptr, 62);
    }

    static SymbolRef Sorbet_Private_Builder() {
        return SymbolRef(nullptr, 63);
    }

    static SymbolRef T_Sig() {
        return SymbolRef(nullptr, 64);
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

template <typename H> H AbslHashValue(H h, const SymbolRef &m) {
    return H::combine(std::move(h), m._id);
}

} // namespace sorbet::core
#endif // RUBY_TYPER_SYMBOLREF_H
