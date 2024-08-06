#include "main/lsp/requests/workspace_symbols.h"
#include "absl/strings/match.h"
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

    SymbolMatcher(const LSPConfiguration &config, const core::GlobalState &gs);

    vector<unique_ptr<SymbolInformation>> doQuery(string_view query, size_t maxResults = MAX_RESULTS);

private:
    vector<unique_ptr<SymbolInformation>> symbolRef2SymbolInformation(core::SymbolRef symRef, size_t maxLocations);

    const LSPConfiguration &config;
    const core::GlobalState &gs;
};

SymbolMatcher::SymbolMatcher(const LSPConfiguration &config, const core::GlobalState &gs)
    : config(config), gs(gs) {}

/**
 * Converts a symbol into any (supported) SymbolInformation objects.
 */
vector<unique_ptr<SymbolInformation>> SymbolMatcher::symbolRef2SymbolInformation(core::SymbolRef symRef,
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

inline bool isEligibleSymbol(core::NameRef name, const core::GlobalState &gs) {
    if (name.kind() == core::NameKind::UNIQUE) {
        return false;
    }
    return true;
}

vector<unique_ptr<SymbolInformation>> SymbolMatcher::doQuery(string_view query_view, size_t maxResults) {
    vector<unique_ptr<SymbolInformation>> results;

    if (query_view.size() == 0) {
        return results;
    }

    const std::array<pair<core::SymbolRef::Kind, size_t>, 3> symbolKinds{
        std::make_pair(core::SymbolRef::Kind::ClassOrModule, gs.classAndModulesUsed()),
        std::make_pair(core::SymbolRef::Kind::FieldOrStaticField, gs.fieldsUsed()),
        std::make_pair(core::SymbolRef::Kind::Method, gs.methodsUsed()),
    };

    vector<core::SymbolRef> candidates;
    {
        Timer timeit(gs.tracer(), "SymbolMatcher::doQuery::pass1");
        for (auto [kind, size] : symbolKinds) {
            for (uint32_t i = 0; i < size; ++i) {
                auto sym = core::SymbolRef(gs, kind, i);
                auto name = sym.name(gs);
                if (!isEligibleSymbol(name, gs)) {
                    continue;
                }
                auto shortName = name.shortName(gs);
                if (absl::StrContains(shortName, query_view)) {
                    candidates.push_back(sym);
                }
            }
        }
    }

    for (auto &candidate : candidates) {
        for (auto &symbolInformation : symbolRef2SymbolInformation(candidate, MAX_LOCATIONS_PER_SYMBOL)) {
            results.emplace_back(move(symbolInformation));
            if (results.size() >= maxResults) {
                return results;
            }
        }
    }

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
    ShowOperation op(config, ShowOperation::Kind::SymbolSearch);
    SymbolMatcher matcher(config, typechecker.state());
    response->result = matcher.doQuery(params->query);
    return response;
}
} // namespace sorbet::realmain::lsp
