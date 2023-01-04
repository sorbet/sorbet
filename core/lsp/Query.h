#ifndef SORBET_CORE_LSP_QUERYRESPONSE
#define SORBET_CORE_LSP_QUERYRESPONSE

#include "core/Loc.h"
#include "core/LocalVariable.h"

namespace sorbet::core::lsp {
/**
 * Represents an LSP query.
 */
class Query final {
public:
    // Queries of different kinds have different active fields.
    enum class Kind {
        // No query active. The default state.
        NONE,
        // Looking for the item at a specific location.
        LOC,
        // Looking for all references to the given symbol.
        SYMBOL,
        // Looking for all references to the given variable.
        VAR,
        // Looking for the definition of a certain method for the purpose of suggesting a sig.
        SUGGEST_SIG,
    };

    Kind kind;
    // If Kind == VAR, this is the loc of the MethodDef that encloses the variable
    // If Kind == LOC, this is the loc to look for.
    // Otherwise, this is meaningless.
    core::Loc loc;
    // If Kind == SYMBOL, this is the symbol that the query is looking for.
    // If Kind == SUGGEST_SIG, this is the method to suggest a sig for.
    // If Kind == VAR, this is the owner of the variable.
    core::SymbolRef symbol;
    core::LocalVariable variable;

    static Query noQuery();
    static Query createLocQuery(core::Loc loc);
    static Query createSymbolQuery(core::SymbolRef symbol);
    static Query createVarQuery(core::SymbolRef owner, core::Loc enclosingLoc, core::LocalVariable variable);
    static Query createSuggestSigQuery(core::MethodRef method);

    bool matchesSymbol(const core::SymbolRef &symbol) const;
    bool matchesLoc(const core::Loc &loc) const;
    bool matchesVar(const core::SymbolRef &owner, const core::LocalVariable &var) const;
    bool matchesSuggestSig(const core::SymbolRef &method) const;
    bool isEmpty() const;

private:
    Query(Kind kind, core::Loc loc, core::SymbolRef symbol, core::LocalVariable variable);
};
} // namespace sorbet::core::lsp

#endif // SORBET_CORE_LSP_QUERYRESPONSE
