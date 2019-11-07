#include "common/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/lsp.h"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <optional>

using namespace std;

namespace sorbet::realmain::lsp {

class SymbolMatcher {
public:
    static constexpr int MAX_RESULTS = 50;

    SymbolMatcher(const LSPConfiguration &config, const core::GlobalState &gs);

    virtual ~SymbolMatcher() = default;

    vector<unique_ptr<SymbolInformation>> doQuery(string_view query, int limit = MAX_RESULTS);

private:
    SymbolInformation *symbolRef2SymbolInformation(core::SymbolRef symRef);

    const LSPConfiguration &config;
    const core::GlobalState &gs;
};

SymbolMatcher::SymbolMatcher(const LSPConfiguration &config, const core::GlobalState &gs) : config(config), gs(gs) {}

/**
 * Converts a symbol into a SymbolInformation object.
 * Returns `nullptr` if symbol kind is not supported by LSP
 */
SymbolInformation *SymbolMatcher::symbolRef2SymbolInformation(core::SymbolRef symRef) {
    auto sym = symRef.data(gs);
    if (!sym->loc().file().exists() || hideSymbol(gs, symRef)) {
        return nullptr;
    }

    auto location = config.loc2Location(gs, sym->loc());
    if (location == nullptr) {
        return nullptr;
    }
    auto result = new SymbolInformation(sym->name.show(gs), symbolRef2SymbolKind(gs, symRef), std::move(location));
    result->containerName = sym->owner.data(gs)->showFullName(gs);
    return result;
}

namespace {

/** Returns a pair of {score, query_length_matched} for the given symbol/query. */
pair<int, int> scoreSymbol(const string_view symbol, const string_view query) {
    auto symbolIter = symbol.begin();
    auto symbolEnd = symbol.end();
    bool atBoundary = true;
    int score = symbol.length();
    int queryMatchLength = 0;
    for (auto queryCh : query) {
        int gap = 0;
        int symbolCharsConsumed = 0;
        bool queryCharIsLower = islower(queryCh);
        while (symbolIter != symbolEnd) {
            char symbolCh = *symbolIter++;
            symbolCharsConsumed++;
            // Check if the current character is boundary-starting
            atBoundary = atBoundary || isupper(symbolCh);
            if (queryCh == symbolCh || (queryCharIsLower && tolower(queryCh) == tolower(symbolCh))) {
                queryMatchLength++;
                if (gap == 0) {
                    // no penalty for adjacent matching characters
                } else if (atBoundary) {
                    score += 100 + gap; // being *on* a boundary is good
                } else {
                    score += 200 + 5 * gap; // being *off* a boundary is bad
                }
                break;
            }
            atBoundary = !isalnum(symbolCh);
            gap++;
        }
        if (queryMatchLength > 1) {
            score += symbolCharsConsumed;
        }
        if (symbolIter == symbolEnd) {
            break;
        }
    }
    return make_pair(score, queryMatchLength);
}
} // namespace

vector<unique_ptr<SymbolInformation>> SymbolMatcher::doQuery(string_view query_view, int limit) {
    vector<unique_ptr<SymbolInformation>> results;
    string query(query_view);
    if (query.empty()) {
        return results;
    }
    const int queryLength = query.length();

    struct ScoreInfo {
        u4 symbolIndex = 0;
        int lengthMatched = 0;
        int score = 0;
    };
    vector<ScoreInfo> scoreInfos(gs.symbolsUsed());
    // First pass: match short names
    for (u4 symbolIndex = 1; symbolIndex < gs.symbolsUsed(); symbolIndex++) {
        auto symbolData = core::SymbolRef(gs, symbolIndex).data(gs);
        auto nameData = symbolData->name.data(gs);
        if (nameData->kind == core::NameKind::UNIQUE) {
            continue;
        }
        auto shortName = nameData->shortName(gs);
        auto [score, lengthMatched] = scoreSymbol(shortName, query);
        scoreInfos[symbolIndex].symbolIndex = symbolIndex;
        scoreInfos[symbolIndex].score = score;
        scoreInfos[symbolIndex].lengthMatched = lengthMatched;
    }

    // Second pass: in some cases, try again if matching long name might improve the result
    for (u4 symbolIndex = 1; symbolIndex < gs.symbolsUsed(); symbolIndex++) {
        ScoreInfo &scoreInfo = scoreInfos[symbolIndex];
        if (scoreInfo.symbolIndex == 0) {
            continue; // symbol ineligible
        }
        auto symbolData = core::SymbolRef(gs, symbolIndex).data(gs);
        auto ownerRef = symbolData->owner;
        if (!ownerRef.exists()) {
            continue; // no owner
        }
        ScoreInfo &ownerScoreInfo = scoreInfos[ownerRef._id];
        if (ownerScoreInfo.lengthMatched == 0) {
            continue; // owner can't improve search result
        }
        if (ownerScoreInfo.lengthMatched == queryLength) {
            continue; // don't bother retrying: it fully matches the container
        }
        if (ownerScoreInfo.lengthMatched <= scoreInfo.lengthMatched && ownerScoreInfo.score >= scoreInfo.score) {
            continue; // owner's score was same-or-worse on equal-or-shorter match, don't bother
        }
        auto fullName = symbolData->showFullName(gs);
        auto [score, lengthMatched] = scoreSymbol(fullName, query);
        if (lengthMatched > scoreInfo.lengthMatched ||
            (lengthMatched == scoreInfo.lengthMatched && score <= scoreInfo.score)) {
            // Found a better match (longer or better score)
            scoreInfo.score = score;
            scoreInfo.lengthMatched = lengthMatched;
        }
    }
    fast_sort(scoreInfos, [queryLength](ScoreInfo &left, ScoreInfo &right) -> bool {
        int leftScore = (left.lengthMatched != queryLength) ? INT_MAX : left.score;
        int rightScore = (right.lengthMatched != queryLength) ? INT_MAX : right.score;
        return leftScore < rightScore;
    });
    for (ScoreInfo &scoreInfo : scoreInfos) {
        core::SymbolRef ref(gs, scoreInfo.symbolIndex);
        if (scoreInfo.lengthMatched != queryLength) {
            break; // end of legit matches
        }
        auto data = symbolRef2SymbolInformation(ref);
        if (data != nullptr) {
            results.emplace_back(data);
            if (results.size() >= limit) {
                break;
            }
        }
    }
    return results;

} // namespace sorbet::realmain::lsp

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
    ShowOperation op(*config, "WorkspaceSymbols", fmt::format("Searching for symbol `{}`...", params.query));
    SymbolMatcher matcher(*config, typechecker.state());
    response->result = matcher.doQuery(params.query);
    return response;
}
} // namespace sorbet::realmain::lsp
