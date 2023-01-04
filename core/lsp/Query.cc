#include "core/GlobalState.h"
#include "core/lsp/QueryResponse.h"

using namespace std;
namespace sorbet::core::lsp {

Query::Query(Kind kind, core::Loc loc, core::SymbolRef symbol, core::LocalVariable variable)
    : kind(kind), loc(loc), symbol(symbol), variable(variable) {}

Query Query::noQuery() {
    return Query(Query::Kind::NONE, core::Loc::none(), core::Symbols::noSymbol(), core::LocalVariable());
}

Query Query::createLocQuery(core::Loc loc) {
    ENFORCE(loc.exists());
    return Query(Query::Kind::LOC, loc, core::Symbols::noSymbol(), core::LocalVariable());
}

Query Query::createSymbolQuery(core::SymbolRef symbol) {
    ENFORCE(symbol.exists());
    return Query(Query::Kind::SYMBOL, core::Loc::none(), symbol, core::LocalVariable());
}

Query Query::createVarQuery(core::SymbolRef owner, core::Loc enclosingLoc, core::LocalVariable variable) {
    ENFORCE(owner.exists());
    ENFORCE(variable.exists());
    return Query(Query::Kind::VAR, enclosingLoc, owner, variable);
}

Query Query::createSuggestSigQuery(core::MethodRef method) {
    ENFORCE(method.exists());
    return Query(Query::Kind::SUGGEST_SIG, core::Loc::none(), method, core::LocalVariable());
}

bool Query::matchesSymbol(const core::SymbolRef &symbol) const {
    return kind == Query::Kind::SYMBOL && this->symbol == symbol;
}

bool Query::matchesLoc(const core::Loc &loc) const {
    // N.B.: Sorbet inserts zero-length Locs for items that are implicitly inserted during parsing.
    // Example: `foo` may be translated into `self.foo`, where `self.` has a 0-length loc.
    // We disregard these in LSP matches, as they don't correspond to source text that the user is pointing at.
    return this->kind == Query::Kind::LOC && loc.exists() && !loc.empty() && loc.contains(this->loc);
}

bool Query::matchesVar(const core::SymbolRef &owner, const core::LocalVariable &var) const {
    return kind == Query::Kind::VAR && var.exists() && this->symbol == owner && this->variable == var;
}

bool Query::matchesSuggestSig(const core::SymbolRef &method) const {
    return kind == Query::Kind::SUGGEST_SIG && this->symbol == method;
}

bool Query::isEmpty() const {
    return kind == Query::Kind::NONE;
}

} // namespace sorbet::core::lsp
