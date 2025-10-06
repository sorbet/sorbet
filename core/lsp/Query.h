#ifndef SORBET_CORE_LSP_QUERYRESPONSE
#define SORBET_CORE_LSP_QUERYRESPONSE

#include "core/Loc.h"
#include "core/LocalVariable.h"
#include "core/SymbolRef.h"

namespace sorbet::core::lsp {

/**
 * Represents an LSP query.
 */
class Query final {
public:
    // Looking for the item at a specific location.
    struct Loc {
        core::Loc loc;
    };

    // Looking for all references to the given symbol.
    //
    // Allows searching for multiple symbols (as a performance optimization, instead of having to
    // run multiple typecheck operations).
    struct Symbol {
        // 1 will be the most common length by far.
        //
        // 4 chosen as the largest number that does not increase the storage above how many bytes an
        // `InlinedVector<SymbolRef, 1>` would require.
        using STORAGE = InlinedVector<core::SymbolRef, 4>;
        STORAGE symbols;
    };

    // Looking for all references to the given variable.
    struct Var {
        // Only look for variables inside this method
        core::MethodRef owner;
        // The loc of the MethodDef that encloses the variable
        core::Loc enclosingLoc;
        core::LocalVariable variable;
    };

    // Looking for the definition of a certain method for the purpose of suggesting a sig.
    struct SuggestSig {
        core::MethodRef method;
    };

    // Queries of different kinds have different active fields.
    std::variant<std::monostate, Loc, Symbol, Var, SuggestSig> query;

    static Query noQuery();
    static Query createLocQuery(core::Loc loc);
    static Query createSymbolQuery(core::SymbolRef symbol);
    static Query createSymbolQuery(Symbol::STORAGE &&symbols);
    static Query createSymbolQuery(absl::Span<const core::SymbolRef> symbols);
    static Query createVarQuery(core::MethodRef owner, core::Loc enclosingLoc, core::LocalVariable variable);
    static Query createSuggestSigQuery(core::MethodRef method);

    bool matchesSymbol(core::SymbolRef symbol) const;
    bool matchesLoc(const core::Loc &loc) const;
    bool matchesVar(core::MethodRef owner, const core::LocalVariable &var) const;
    bool matchesSuggestSig(core::MethodRef method) const;
    bool isEmpty() const;
};
CheckSize(Query, 32, 8);
} // namespace sorbet::core::lsp

#endif // SORBET_CORE_LSP_QUERYRESPONSE
