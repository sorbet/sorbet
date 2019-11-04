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
optional<int> scoreSymbolAgainstQuery(const string_view symbol, const string query) {
    int score = 0;
    auto symbolIter = symbol.begin();
    auto symbolEnd = symbol.end();
    bool previousSymbolCharWasBoundary = true;
    int matchSize = -query.length(); // When we find the first matching char, we'll set this to 0
    for (auto queryCh : query) {
        bool found = false;
        bool queryIsLower = islower(queryCh);
        int gap = 0;
        while (symbolIter != symbolEnd) {
            char symbolCh = *symbolIter++;
            matchSize++;
            char symbolCharIsUpper = std::isupper(symbolCh);
            if (queryCh == symbolCh || (queryIsLower && symbolCharIsUpper && queryCh == std::tolower(symbolCh))) {
                found = true;
                if (matchSize < 0) {
                    matchSize = 0; // found a matching char, start here...
                }
                if (gap == 0) {
                    // no penalty for adjacent matching characters
                } else if (previousSymbolCharWasBoundary || symbolCharIsUpper) {
                    score += 100 + gap; // being *on* a boundary is good
                } else {
                    score += 200 + gap; // being *off* a boundary is bad
                }
                break;
            } else {
                if (previousSymbolCharWasBoundary || symbolCharIsUpper) {
                    gap += 50; // small penalty for skipping a boundary
                }
                previousSymbolCharWasBoundary = !isalpha(symbolCh);
                gap++;
            }
        }
        if (!found && symbolIter == symbolEnd) {
            return nullopt;
        }
    }
    score += matchSize;
    return score;
}
} // namespace

vector<unique_ptr<SymbolInformation>> SymbolMatcher::doQuery(string_view query_view, int limit) {
    vector<unique_ptr<SymbolInformation>> results;
    string query(query_view);
    if (query.empty()) {
        return results;
    }
    vector<pair<u4, int>> symbolScores;
    for (u4 symbolIndex = 1; symbolIndex < gs.symbolsUsed(); symbolIndex++) {
        auto symbolRef = core::SymbolRef(gs, symbolIndex);
        auto nameData = symbolRef.data(gs)->name.data(gs);
        if (nameData->kind == core::NameKind::UNIQUE) {
            continue;
        }
        auto shortName = nameData->shortName(gs);
        auto score = scoreSymbolAgainstQuery(shortName, query);
        if (score.has_value()) {
            symbolScores.emplace_back(symbolIndex, *score);
        }
    }
    fast_sort(symbolScores,
              [](pair<u4, int> &left, pair<u4, int> &right) -> bool { return left.second < right.second; });
    for (auto symbolScore : symbolScores) {
        core::SymbolRef ref(gs, symbolScore.first);
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
