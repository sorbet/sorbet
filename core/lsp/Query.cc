#include "core/GlobalState.h"
#include "core/lsp/QueryResponse.h"

using namespace std;
namespace sorbet::core::lsp {

Query Query::noQuery() {
    return Query{};
}

Query Query::createLocQuery(core::Loc loc) {
    ENFORCE(loc.exists());
    return Query{Loc{loc}};
}

Query Query::createSymbolQuery(core::SymbolRef symbol) {
    ENFORCE(symbol.exists());
    auto symbols = InlinedVector<core::SymbolRef, 4>{1, symbol};
    return Query{Symbol{symbols}};
}

Query Query::createSymbolQuery(Symbol::STORAGE &&symbols) {
    ENFORCE(absl::c_all_of(symbols, [](auto symbol) { return symbol.exists(); }));
    return Query{Symbol{move(symbols)}};
}

Query Query::createSymbolQuery(absl::Span<const core::SymbolRef> symbols) {
    ENFORCE(absl::c_all_of(symbols, [](auto symbol) { return symbol.exists(); }));
    auto result = Symbol{};
    absl::c_copy(symbols, back_inserter(result.symbols));
    return Query{move(result)};
}

Query Query::createVarQuery(core::MethodRef owner, core::Loc enclosingLoc, core::LocalVariable variable) {
    ENFORCE(owner.exists());
    ENFORCE(variable.exists());
    return Query{Var{owner, enclosingLoc, variable}};
}

Query Query::createSuggestSigQuery(core::MethodRef method) {
    ENFORCE(method.exists());
    return Query{SuggestSig{method}};
}

bool Query::matchesSymbol(core::SymbolRef symbol) const {
    if (auto *query = get_if<Symbol>(&this->query)) {
        return absl::c_any_of(query->symbols, [symbol](auto curr) { return curr == symbol; });
    }
    return false;
}

bool Query::matchesLoc(const core::Loc &loc) const {
    // N.B.: Sorbet inserts zero-length Locs for items that are implicitly inserted during parsing.
    // Example: `foo` may be translated into `self.foo`, where `self.` has a 0-length loc.
    // We disregard these in LSP matches, as they don't correspond to source text that the user is pointing at.
    if (auto *query = get_if<Loc>(&this->query)) {
        return loc.exists() && !loc.empty() && loc.contains(query->loc);
    }
    return false;
}

bool Query::matchesVar(core::MethodRef owner, const core::LocalVariable &var) const {
    if (auto *query = get_if<Var>(&this->query)) {
        return var.exists() && query->owner == owner && query->variable == var;
    }
    return false;
}

bool Query::matchesSuggestSig(core::MethodRef method) const {
    if (auto *query = get_if<SuggestSig>(&this->query)) {
        return query->method == method;
    }
    return false;
}

bool Query::isEmpty() const {
    return holds_alternative<monostate>(this->query);
}

} // namespace sorbet::core::lsp
