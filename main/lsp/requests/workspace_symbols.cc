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
        auto location = config.loc2Location(gs, sym->loc());
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

inline bool isNamespaceSeparator(char ch) {
    return ch == ':' || ch == '.';
}

/** Returns a pair of {rank, query_length_matched} for the given symbol/query. */
pair<int, string_view::const_iterator> partialMatchSymbol(string_view symbol, string_view::const_iterator queryBegin,
                                                          string_view::const_iterator queryEnd, bool prefixOnly) {
    auto symbolIter = symbol.begin();
    auto symbolEnd = symbol.end();
    auto queryIter = queryBegin;
    pair<int, string_view::const_iterator> result = {0, queryIter};
    // Consume leading namespacing punctuation, e.g. to make `::f` matchable against `module Foo`.
    while (queryIter != queryEnd && isNamespaceSeparator(*queryIter)) {
        queryIter++;
    }
    char previousSymbolCh = 0;
    char symbolCh = 0;
    while (queryIter != queryEnd) {
        auto queryCh = *queryIter++;
        bool queryCharIsLower = islower(queryCh);
        int symbolCharsConsumed = 0;
        while (symbolIter != symbolEnd) {
            previousSymbolCh = symbolCh;
            symbolCh = *symbolIter++;
            symbolCharsConsumed++;
            if (queryCh == symbolCh || (queryCharIsLower && tolower(queryCh) == tolower(symbolCh))) {
                if (symbolCharsConsumed == 1) {
                    if (queryCh != symbolCh) {
                        result.first += 1; // matching character not quite as good
                    }
                    result.second = queryIter;
                    break;
                } else if (!isalnum(previousSymbolCh) || isupper(symbolCh)) {
                    // On a word boundary
                    result.first += 100 + symbolCharsConsumed;
                    result.second = queryIter;
                    break;
                } else if (!prefixOnly) {
                    // middle of word...can sometimes match, but steep penalty
                    result.first += 200 + symbolCharsConsumed;
                    result.second = queryIter;
                    break;
                }
            }
        }
    }
    if (result.second != queryBegin) {
        result.first += symbol.length(); // penalize longer symbols
    }
    return result;
}
} // namespace

vector<unique_ptr<SymbolInformation>> SymbolMatcher::doQuery(string_view query_view, size_t maxResults) {
    vector<unique_ptr<SymbolInformation>> results;
    string_view::const_iterator queryBegin = query_view.begin();
    string_view::const_iterator queryEnd = query_view.end();

    if (queryBegin == queryEnd) {
        return results;
    }
    struct ScoreInfo {
        u4 symbolIndex = 0;
        int score = 0;
        string_view::const_iterator queryIter = nullptr; // progress in query match
    };
    vector<ScoreInfo> scoreInfos(gs.symbolsUsed());
    scoreInfos[0].queryIter = queryBegin;
    // First pass: prefix-only matches on namespace
    for (u4 symbolIndex = 1; symbolIndex < gs.symbolsUsed(); symbolIndex++) {
        auto &scoreInfo = scoreInfos[symbolIndex];
        auto symbolData = core::SymbolRef(gs, symbolIndex).data(gs);
        auto nameData = symbolData->name.data(gs);
        if (nameData->kind == core::NameKind::UNIQUE) {
            continue;
        }
        auto [ownerSymbolIndex, ownerScore, ownerQueryIter] = scoreInfos[symbolData->owner._id];
        if (ownerSymbolIndex == 0 || ownerQueryIter == nullptr) {
            ownerScore = 0;
            ownerQueryIter = queryBegin;
        }
        auto shortName = nameData->shortName(gs);
        auto [partialScore, partialQueryIter] = partialMatchSymbol(shortName, ownerQueryIter, queryEnd, true);
        scoreInfo.symbolIndex = symbolIndex;
        scoreInfo.score = ownerScore + partialScore;
        scoreInfo.queryIter = partialQueryIter;
    }

    // Second pass: record matches and (try a little harder by relaxing the prefix-only requirement for non-matches)
    // Don't update partialScoreInfos, because we are using it to keep the prefix-only scores for owner-namespaces.
    vector<pair<u4, int>> candidates;
    for (u4 symbolIndex = 1; symbolIndex < gs.symbolsUsed(); symbolIndex++) {
        auto &scoreInfo = scoreInfos[symbolIndex];
        auto symbolRef = core::SymbolRef(gs, scoreInfo.symbolIndex);
        if (hideSymbol(gs, symbolRef)) {
            continue; // symbol ineligible
        }
        auto symbolData = core::SymbolRef(gs, scoreInfo.symbolIndex).data(gs);
        auto [ownerSymbolIndex, ownerScore, ownerQueryIter] = scoreInfos[symbolData->owner._id];
        optional<int> bestScore = nullopt;
        if (scoreInfo.queryIter == queryEnd && !(ownerQueryIter == queryEnd && ownerScore <= scoreInfo.score)) {
            bestScore = scoreInfo.score;
        }
        auto shortName = symbolData->name.data(gs)->shortName(gs);
        auto [score, queryIter] = partialMatchSymbol(shortName, queryBegin, queryEnd, false);
        if (queryIter == queryEnd && (!bestScore.has_value() || *bestScore > score)) {
            bestScore = score;
        }
        if (ownerQueryIter != nullptr && ownerQueryIter != queryBegin && ownerQueryIter != queryEnd) {
            auto [score, queryIter] = partialMatchSymbol(shortName, ownerQueryIter, queryEnd, false);
            if (queryIter == queryEnd && (!bestScore.has_value() || *bestScore > score)) {
                bestScore = ownerScore + score;
            }
        }
        if (bestScore.has_value()) {
            candidates.emplace_back(scoreInfo.symbolIndex, *bestScore);
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
}

unique_ptr<ResponseMessage> LSPLoop::handleWorkspaceSymbols(LSPTypechecker &typechecker, const MessageId &id,
                                                            const WorkspaceSymbolParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::WorkspaceSymbol);
    if (!config->opts.lspWorkspaceSymbolsEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Workspace Symbols` LSP feature is experimental and disabled by default.");
        return response;
    }

    prodCategoryCounterInc("lsp.messages.processed", "workspace.symbols");
    SymbolMatcher matcher(*config, typechecker.state());
    response->result = matcher.doQuery(params.query);
    return response;
}
} // namespace sorbet::realmain::lsp
