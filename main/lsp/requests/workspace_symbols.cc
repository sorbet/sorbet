#include "main/lsp/requests/workspace_symbols.h"
#include "common/sort/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <optional>

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

struct PartialMatch {
    uint score = 0;
    string_view::const_iterator matchEnd = nullptr; // progress in query match
};

class SymbolMatcher final {
public:
    static constexpr size_t MAX_RESULTS = 50;
    static constexpr size_t MAX_LOCATIONS_PER_SYMBOL = 10;

    // Given the best match, don't return things more than 100x worse
    static constexpr size_t WORST_TO_BEST_RATIO = 100;

    SymbolMatcher(const LSPConfiguration &config, const core::GlobalState &gs);

    vector<unique_ptr<SymbolInformation>> doQuery(string_view query, size_t maxResults = MAX_RESULTS);

private:
    vector<unique_ptr<SymbolInformation>> symbolRef2SymbolInformations(core::SymbolRef symRef, size_t maxLocations);

    const LSPConfiguration &config;
    const core::GlobalState &gs;
    vector<PartialMatch> classOrModuleMatches;
    vector<PartialMatch> methodMatches;
    vector<PartialMatch> fieldMatches;
    vector<PartialMatch> typeArgumentMatches;
    vector<PartialMatch> typeMemberMatches;

    PartialMatch &getPartialMatch(core::SymbolRef ref) {
        switch (ref.kind()) {
            case core::SymbolRef::Kind::ClassOrModule:
                return classOrModuleMatches[ref.classOrModuleIndex()];
            case core::SymbolRef::Kind::Method:
                return methodMatches[ref.methodIndex()];
            case core::SymbolRef::Kind::FieldOrStaticField:
                return fieldMatches[ref.fieldIndex()];
            case core::SymbolRef::Kind::TypeArgument:
                return typeArgumentMatches[ref.typeArgumentIndex()];
            case core::SymbolRef::Kind::TypeMember:
                return typeMemberMatches[ref.typeMemberIndex()];
        }
    }

    void updatePartialMatch(core::SymbolRef symbolRef, string_view::const_iterator queryBegin,
                            string_view::const_iterator queryEnd, size_t &ceilingScore);
};

SymbolMatcher::SymbolMatcher(const LSPConfiguration &config, const core::GlobalState &gs)
    : config(config), gs(gs), classOrModuleMatches(gs.classAndModulesUsed()), methodMatches(gs.methodsUsed()),
      fieldMatches(gs.fieldsUsed()), typeArgumentMatches(gs.typeArgumentsUsed()),
      typeMemberMatches(gs.typeMembersUsed()) {}

/**
 * Converts a symbol into any (supported) SymbolInformation objects.
 */
vector<unique_ptr<SymbolInformation>> SymbolMatcher::symbolRef2SymbolInformations(core::SymbolRef symRef,
                                                                                  size_t maxLocations) {
    vector<unique_ptr<SymbolInformation>> results;
    for (auto loc : symRef.locs(gs)) {
        if (results.size() >= maxLocations) {
            break;
        }
        if (!loc.file().exists()) {
            continue;
        }

        // Don't report definitions in __package.rb files, as they're references to symbols defined elsewhere.
        if (this->config.opts.stripePackages && loc.file().data(gs).isPackage()) {
            continue;
        }

        auto location = config.loc2Location(gs, loc);
        if (location == nullptr) {
            continue;
        }
        // To be able to get this information, we'd have to walk the tree. It's not that important for
        // this method, so let's just fall back to treating attributes as methods.
        auto isAttr = false;

        // VSCode does its own internal ranking based on comparing the query string against the result name.
        // Therefore have the name be the fully qualified name if it makes sense (e.g. Foo::Bar instead of Bar)
        auto result = make_unique<SymbolInformation>(symRef.show(gs), symbolRef2SymbolKind(gs, symRef, isAttr),
                                                     std::move(location));
        auto container = symRef.owner(gs);
        if (container != core::Symbols::root()) {
            result->containerName = container.show(gs);
        }
        results.emplace_back(move(result));
    }
    return results;
}

/** Is a symbol that can act as a namespace separator in user queries. */
inline bool canBeLeadingNamespaceSeparator(char ch) {
    return ch == ':' || ch == '.' || ch == '#' || isspace(ch);
}

/** Is symbol that can skip to the next word boundary within a symbol. */
inline bool canMatchWordBoundary(char ch) {
    return isspace(ch);
}

inline bool isEligibleSymbol(core::NameRef name, const core::GlobalState &gs) {
    if (name.kind() == core::NameKind::UNIQUE) {
        return false;
    }
    return true;
}

/** Returns a PartialMatch for the given symbol/query. */
PartialMatch partialMatchSymbol(string_view symbol, string_view::const_iterator queryBegin,
                                string_view::const_iterator queryEnd, bool prefixOnly, uint ceilingScore) {
    auto symbolIter = symbol.begin();
    auto symbolEnd = symbol.end();
    auto queryIter = queryBegin;
    // Consume leading namespacing punctuation, e.g. to make `::f` matchable
    // against `module Foo`.
    while (queryIter != queryEnd && canBeLeadingNamespaceSeparator(*queryIter)) {
        queryIter++;
    }
    while (symbolIter != symbolEnd && canBeLeadingNamespaceSeparator(*symbolIter)) {
        symbolIter++;
    }
    auto matchEnd = queryIter;
    char previousSymbolCh = 0;
    char symbolCh = 0;
    uint score = 1;
    while (queryIter != queryEnd && symbolIter != symbolEnd && score < ceilingScore) {
        auto queryCh = *queryIter++;
        bool okToMatchWordBoundary = canMatchWordBoundary(queryCh);
        bool queryCharIsLower = islower(queryCh);
        int symbolCharsConsumed = 0;
        int boundariesCrossed = 0;
        bool previousWordBoundary = true;
        while (symbolIter != symbolEnd) {
            previousSymbolCh = symbolCh;
            symbolCh = *symbolIter++;
            symbolCharsConsumed++;
            bool onWordBoundary = !isalnum(previousSymbolCh) || isupper(symbolCh);
            if (onWordBoundary && !previousWordBoundary) {
                boundariesCrossed += 1;
            }
            if (queryCh == symbolCh || (queryCharIsLower && tolower(queryCh) == tolower(symbolCh))) {
                if (symbolCharsConsumed == 1) {
                    score += 1;
                    if (queryCh != symbolCh) {
                        score += 2; // penalize case-insensitive match
                    }
                    matchEnd = queryIter;
                    break;
                } else if (onWordBoundary) {
                    // On a word boundary
                    score = score + 5 * boundariesCrossed;
                    matchEnd = queryIter;
                    break;
                } else if (!prefixOnly) {
                    // middle of word...can sometimes match, but steep penalty
                    score = 5 * (score + symbolCharsConsumed);
                    matchEnd = queryIter;
                    break;
                }
            } else if (onWordBoundary && okToMatchWordBoundary) {
                // consume queryCh, but put symbolCh back to let it match next queryCh
                // (allows whitespace to be used for more natural searches)
                symbolIter--;
                break;
            }
            previousWordBoundary = onWordBoundary;
        }
    }
    score += symbol.length();
    if (score >= ceilingScore) {
        return {1, queryBegin};
    }
    return {score, matchEnd};
}

void SymbolMatcher::updatePartialMatch(core::SymbolRef symbolRef, string_view::const_iterator queryBegin,
                                       string_view::const_iterator queryEnd, size_t &ceilingScore) {
    auto name = symbolRef.name(gs);
    if (!isEligibleSymbol(name, gs)) {
        return;
    }
    auto shortName = name.shortName(gs);
    auto &partialMatch = getPartialMatch(symbolRef);
    partialMatch = partialMatchSymbol(shortName, queryBegin, queryEnd, true, ceilingScore);
    for (auto previousAncestorRef = symbolRef, ancestorRef = symbolRef.owner(gs);
         previousAncestorRef != ancestorRef && ancestorRef.exists();
         previousAncestorRef = ancestorRef, ancestorRef = ancestorRef.owner(gs)) {
        auto &ancestorMatch = getPartialMatch(ancestorRef);
        auto ancestorEnd = ancestorMatch.matchEnd;
        if (ancestorEnd == queryBegin || ancestorEnd == nullptr) {
            break; // no further ancestor will be of any help
        }
        if (ancestorEnd == queryEnd) {
            continue; // ancestor matched everything, so skip to its parent
        }
        auto ancestorScore = ancestorMatch.score;
        auto plusMatch = partialMatchSymbol(shortName, ancestorEnd, queryEnd, true, ceilingScore - ancestorScore);
        if (plusMatch.matchEnd - partialMatch.matchEnd > 0) {
            partialMatch.score = ancestorScore + plusMatch.score;
            partialMatch.matchEnd = plusMatch.matchEnd;
        } else if (plusMatch.matchEnd == partialMatch.matchEnd) {
            auto combinedScore = ancestorScore + plusMatch.score;
            if (combinedScore < partialMatch.score) {
                partialMatch.score = combinedScore;
            }
        }
    }
    if (partialMatch.matchEnd == queryEnd) {
        // update ceiling so we stop looking at bad matches
        ceilingScore = min(ceilingScore, partialMatch.score * WORST_TO_BEST_RATIO);
    }
}

vector<unique_ptr<SymbolInformation>> SymbolMatcher::doQuery(string_view query_view, size_t maxResults) {
    vector<unique_ptr<SymbolInformation>> results;
    string_view::const_iterator queryBegin = query_view.begin();
    string_view::const_iterator queryEnd = query_view.end();

    if (queryBegin == queryEnd) {
        return results;
    }
    size_t ceilingScore = INT_MAX;

    const std::array<pair<core::SymbolRef::Kind, size_t>, 3> symbolKinds{
        std::make_pair(core::SymbolRef::Kind::ClassOrModule, gs.classAndModulesUsed()),
        std::make_pair(core::SymbolRef::Kind::FieldOrStaticField, gs.fieldsUsed()),
        std::make_pair(core::SymbolRef::Kind::Method, gs.methodsUsed()),
    };

    // First pass: prefix-only matches on namespace
    {
        Timer timeit(gs.tracer(), "SymbolMatcher::doQuery::pass1");
        for (auto [kind, size] : symbolKinds) {
            for (uint32_t i = 0; i < size; ++i) {
                auto sym = core::SymbolRef(gs, kind, i);
                if (sym.exists()) {
                    updatePartialMatch(core::SymbolRef(gs, kind, i), queryBegin, queryEnd, ceilingScore);
                }
            }
        }
    }

    // Second pass: record matches and (try a little harder by relaxing the
    // prefix-only requirement for non-matches) Don't update partialMatches,
    // because we will continue using it to keep the prefix-only scores for
    // owner-namespaces.
    vector<pair<core::SymbolRef, int>> candidates;
    {
        Timer timeit(gs.tracer(), "SymbolMatcher::doQuery::pass2");
        for (auto [kind, size] : symbolKinds) {
            for (uint32_t i = 0; i < size; ++i) {
                auto symbolRef = core::SymbolRef(gs, kind, i);
                if (!symbolRef.exists()) {
                    continue;
                }
                auto name = symbolRef.name(gs);
                if (!isEligibleSymbol(name, gs)) {
                    continue;
                }
                auto shortName = name.shortName(gs);
                uint bestScore = ceilingScore;
                auto [partialScore, partialMatchEnd] =
                    partialMatchSymbol(shortName, queryBegin, queryEnd, false, ceilingScore);
                if (partialMatchEnd == queryEnd) {
                    bestScore = partialScore;
                }
                for (auto previousAncestorRef = symbolRef, ancestorRef = symbolRef.owner(gs);
                     previousAncestorRef != ancestorRef && ancestorRef.exists();
                     previousAncestorRef = ancestorRef, ancestorRef = ancestorRef.owner(gs)) {
                    auto [ancestorScore, ancestorEnd] = getPartialMatch(ancestorRef);
                    if (ancestorEnd == queryBegin || ancestorEnd == nullptr) {
                        break; // no further ancestor will be of any help
                    }
                    if (ancestorEnd == queryEnd) {
                        continue; // ancestor matched everything, so skip to its parent
                    }
                    if (ancestorScore >= bestScore) {
                        continue; // matching this ancestor would be worse
                    }
                    auto [plusScore, plusEnd] =
                        partialMatchSymbol(shortName, ancestorEnd, queryEnd, false, bestScore - ancestorScore);
                    if (plusEnd == queryEnd) {
                        bestScore = min(bestScore, ancestorScore + plusScore);
                    }
                }
                if (bestScore < ceilingScore) {
                    // update ceiling so we stop looking at bad matches
                    ceilingScore = min(ceilingScore, bestScore * WORST_TO_BEST_RATIO);
                    candidates.emplace_back(symbolRef, bestScore);
                }
            }
        }
    }
    fast_sort(candidates, [](auto &left, auto &right) -> bool { return left.second < right.second; });
    for (auto &candidate : candidates) {
        auto ref = candidate.first;
        auto maxLocations = min(MAX_LOCATIONS_PER_SYMBOL, maxResults - results.size());
        for (auto &symbolInformation : symbolRef2SymbolInformations(ref, maxLocations)) {
            results.emplace_back(move(symbolInformation));
        }
        if (results.size() >= maxResults) {
            break;
        }
    }
    ENFORCE(results.size() <= maxResults);
    return results;
}
} // namespace

WorkspaceSymbolsTask::WorkspaceSymbolsTask(const LSPConfiguration &config, MessageId id,
                                           unique_ptr<WorkspaceSymbolParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::WorkspaceSymbol), params(move(params)) {}

bool WorkspaceSymbolsTask::isDelayable() const {
    return true;
}

unique_ptr<ResponseMessage> WorkspaceSymbolsTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    Timer timeit(typechecker.state().tracer(), "LSPLoop::handleWorkspaceSymbols");
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::WorkspaceSymbol);
    ShowOperation op(config, ShowOperation::Kind::References);
    SymbolMatcher matcher(config, typechecker.state());
    response->result = matcher.doQuery(params->query);
    return response;
}
} // namespace sorbet::realmain::lsp
