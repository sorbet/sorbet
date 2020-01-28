#include "main/lsp/requests/workspace_symbols.h"
#include "common/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/lsp.h"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <optional>

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

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
};

SymbolMatcher::SymbolMatcher(const LSPConfiguration &config, const core::GlobalState &gs) : config(config), gs(gs) {}

/**
 * Converts a symbol into any (supported) SymbolInformation objects.
 */
vector<unique_ptr<SymbolInformation>> SymbolMatcher::symbolRef2SymbolInformations(core::SymbolRef symRef,
                                                                                  size_t maxLocations) {
    vector<unique_ptr<SymbolInformation>> results;
    auto sym = symRef.data(gs);
    for (auto loc : sym->locs()) {
        if (results.size() >= maxLocations) {
            break;
        }
        if (!loc.file().exists()) {
            continue;
        }
        auto location = config.loc2Location(gs, loc);
        if (location == nullptr) {
            continue;
        }
        auto result =
            make_unique<SymbolInformation>(sym->name.show(gs), symbolRef2SymbolKind(gs, symRef), std::move(location));
        result->containerName = sym->owner.data(gs)->showFullName(gs);
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

inline bool isEligibleSymbol(const core::NameData &nameData) {
    if (nameData->kind == core::NameKind::UNIQUE) {
        return false;
    }
    return true;
}

struct PartialMatch {
    uint score = 0;
    string_view::const_iterator matchEnd = nullptr; // progress in query match
};

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
} // namespace

vector<unique_ptr<SymbolInformation>> SymbolMatcher::doQuery(string_view query_view, size_t maxResults) {
    vector<unique_ptr<SymbolInformation>> results;
    string_view::const_iterator queryBegin = query_view.begin();
    string_view::const_iterator queryEnd = query_view.end();

    if (queryBegin == queryEnd) {
        return results;
    }
    size_t ceilingScore = INT_MAX;
    vector<PartialMatch> partialMatches(gs.symbolsUsed());
    // First pass: prefix-only matches on namespace
    {
        Timer timeit(gs.tracer(), "SymbolMatcher::doQuery::pass1");
        for (u4 symbolIndex = 1; symbolIndex < gs.symbolsUsed(); symbolIndex++) {
            auto symbolRef = core::SymbolRef(gs, symbolIndex);
            auto symbolData = symbolRef.data(gs);
            auto nameData = symbolData->name.data(gs);
            if (!isEligibleSymbol(nameData)) {
                continue;
            }
            auto shortName = nameData->shortName(gs);
            auto &partialMatch = partialMatches[symbolIndex] =
                partialMatchSymbol(shortName, queryBegin, queryEnd, true, ceilingScore);
            for (auto previousAncestorRef = symbolRef, ancestorRef = symbolData->owner;
                 previousAncestorRef != ancestorRef && ancestorRef.exists();
                 previousAncestorRef = ancestorRef, ancestorRef = ancestorRef.data(gs)->owner) {
                auto &ancestorMatch = partialMatches[ancestorRef._id];
                auto ancestorEnd = ancestorMatch.matchEnd;
                if (ancestorEnd == queryBegin || ancestorEnd == nullptr) {
                    break; // no further ancestor will be of any help
                }
                if (ancestorEnd == queryEnd) {
                    continue; // ancestor matched everything, so skip to its parent
                }
                auto ancestorScore = ancestorMatch.score;
                auto plusMatch =
                    partialMatchSymbol(shortName, ancestorEnd, queryEnd, true, ceilingScore - ancestorScore);
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
    }

    // Second pass: record matches and (try a little harder by relaxing the
    // prefix-only requirement for non-matches) Don't update partialMatches,
    // because we will continue using it to keep the prefix-only scores for
    // owner-namespaces.
    vector<pair<u4, int>> candidates;
    {
        Timer timeit(gs.tracer(), "SymbolMatcher::doQuery::pass2");
        for (u4 symbolIndex = 1; symbolIndex < gs.symbolsUsed(); symbolIndex++) {
            auto symbolRef = core::SymbolRef(gs, symbolIndex);
            auto symbolData = symbolRef.data(gs);
            auto nameData = symbolData->name.data(gs);
            if (!isEligibleSymbol(nameData)) {
                continue;
            }
            auto shortName = nameData->shortName(gs);
            uint bestScore = ceilingScore;
            auto [partialScore, partialMatchEnd] =
                partialMatchSymbol(shortName, queryBegin, queryEnd, false, ceilingScore);
            if (partialMatchEnd == queryEnd) {
                bestScore = partialScore;
            }
            for (auto previousAncestorRef = symbolRef, ancestorRef = symbolData->owner;
                 previousAncestorRef != ancestorRef && ancestorRef.exists();
                 previousAncestorRef = ancestorRef, ancestorRef = ancestorRef.data(gs)->owner) {
                auto [ancestorScore, ancestorEnd] = partialMatches[ancestorRef._id];
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
                candidates.emplace_back(symbolIndex, bestScore);
            }
        }
    }
    fast_sort(candidates, [](pair<u4, int> &left, pair<u4, int> &right) -> bool { return left.second < right.second; });
    for (auto &candidate : candidates) {
        core::SymbolRef ref(gs, candidate.first);
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
} // namespace sorbet::realmain::lsp

WorkspaceSymbolsTask::WorkspaceSymbolsTask(const LSPConfiguration &config, MessageId id,
                                           unique_ptr<WorkspaceSymbolParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::WorkspaceSymbol), params(move(params)) {}

bool WorkspaceSymbolsTask::isDelayable() const {
    return true;
}

unique_ptr<ResponseMessage> WorkspaceSymbolsTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    Timer timeit(typechecker.state().tracer(), "LSPLoop::handleWorkspaceSymbols");
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::WorkspaceSymbol);
    ShowOperation op(config, "References", "Workspace symbol search...");
    prodCategoryCounterInc("lsp.messages.processed", "workspace.symbols");
    SymbolMatcher matcher(config, typechecker.state());
    response->result = matcher.doQuery(params->query);
    return response;
}
} // namespace sorbet::realmain::lsp
