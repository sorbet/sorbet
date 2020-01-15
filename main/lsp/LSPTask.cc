#include "main/lsp/LSPTask.h"
#include "common/sort.h"
#include "main/lsp/LSPOutput.h"

namespace sorbet::realmain::lsp {
using namespace std;

LSPTask::LSPTask(const LSPConfiguration &config) : config(config) {}

LSPRequestTask::LSPRequestTask(const LSPConfiguration &config, MessageId id) : LSPTask(config), id(move(id)) {}

void LSPRequestTask::run(LSPTypecheckerDelegate &typechecker) {
    auto response = runRequest(typechecker);
    ENFORCE(response != nullptr);
    config.output->write(move(response));
}

LSPQueryResult LSPTask::queryByLoc(LSPTypecheckerDelegate &typechecker, string_view uri, const Position &pos,
                                   const LSPMethod forMethod, bool errorIfFileIsUntyped) const {
    Timer timeit(config.logger, "setupLSPQueryByLoc");
    const core::GlobalState &gs = typechecker.state();
    auto fref = config.uri2FileRef(gs, uri);
    if (!fref.exists()) {
        auto error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidParams,
            fmt::format("Did not find file at uri {} in {}", uri, convertLSPMethodToString(forMethod)));
        return LSPQueryResult{{}, move(error)};
    }

    if (errorIfFileIsUntyped && fref.data(gs).strictLevel < core::StrictLevel::True) {
        config.logger->info("Ignoring request on untyped file `{}`", uri);
        // Act as if the query returned no results.
        return LSPQueryResult{{}};
    }

    auto loc = config.lspPos2Loc(fref, pos, gs);
    return typechecker.query(core::lsp::Query::createLocQuery(loc), {fref});
}

LSPQueryResult LSPTask::queryBySymbolInFiles(LSPTypecheckerDelegate &typechecker, core::SymbolRef sym,
                                             vector<core::FileRef> frefs) const {
    Timer timeit(config.logger, "setupLSPQueryBySymbolInFiles");
    ENFORCE(sym.exists());
    return typechecker.query(core::lsp::Query::createSymbolQuery(sym), frefs);
}

LSPQueryResult LSPTask::queryBySymbol(LSPTypecheckerDelegate &typechecker, core::SymbolRef sym) const {
    Timer timeit(config.logger, "setupLSPQueryBySymbol");
    ENFORCE(sym.exists());
    vector<core::FileRef> frefs;
    const core::GlobalState &gs = typechecker.state();
    const core::NameHash symNameHash(gs, sym.data(gs)->name.data(gs));
    // Locate files that contain the same Name as the symbol. Is an overapproximation, but a good first filter.
    int i = -1;
    for (auto &hash : typechecker.getFileHashes()) {
        i++;
        const auto &usedSends = hash.usages.sends;
        const auto &usedConstants = hash.usages.constants;
        auto ref = core::FileRef(i);

        const bool fileIsValid = ref.exists() && ref.data(gs).sourceType == core::File::Type::Normal;
        if (fileIsValid &&
            (std::find(usedSends.begin(), usedSends.end(), symNameHash) != usedSends.end() ||
             std::find(usedConstants.begin(), usedConstants.end(), symNameHash) != usedConstants.end())) {
            frefs.emplace_back(ref);
        }
    }

    return typechecker.query(core::lsp::Query::createSymbolQuery(sym), frefs);
}

vector<unique_ptr<Location>>
LSPTask::extractLocations(const core::GlobalState &gs,
                          const vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                          vector<unique_ptr<Location>> locations) const {
    for (auto &q : queryResponses) {
        core::Loc loc = q->getLoc();
        if (loc.exists() && loc.file().exists()) {
            auto fileIsTyped = loc.file().data(gs).strictLevel >= core::StrictLevel::True;
            // If file is untyped, only support responses involving constants and definitions.
            if (fileIsTyped || q->isConstant() || q->isField() || q->isDefinition()) {
                addLocIfExists(gs, locations, loc);
            }
        }
    }
    // Dedupe locations
    fast_sort(locations,
              [](const unique_ptr<Location> &a, const unique_ptr<Location> &b) -> bool { return a->cmp(*b) < 0; });
    locations.resize(std::distance(locations.begin(),
                                   std::unique(locations.begin(), locations.end(),
                                               [](const unique_ptr<Location> &a,
                                                  const unique_ptr<Location> &b) -> bool { return a->cmp(*b) == 0; })));
    return locations;
}

vector<unique_ptr<Location>> LSPTask::getReferencesToSymbol(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol,
                                                            vector<unique_ptr<Location>> locations) const {
    if (symbol.exists()) {
        auto run2 = queryBySymbol(typechecker, symbol);
        locations = extractLocations(typechecker.state(), run2.responses, move(locations));
    }
    return locations;
}

vector<unique_ptr<DocumentHighlight>>
LSPTask::getHighlightsToSymbolInFile(LSPTypecheckerDelegate &typechecker, string_view const uri, core::SymbolRef symbol,
                                     vector<unique_ptr<DocumentHighlight>> highlights) const {
    if (symbol.exists()) {
        auto fref = config.uri2FileRef(typechecker.state(), uri);
        if (fref.exists()) {
            auto run2 = queryBySymbolInFiles(typechecker, symbol, {fref});
            auto locations = extractLocations(typechecker.state(), run2.responses);
            for (auto const &location : locations) {
                auto highlight = make_unique<DocumentHighlight>(move(location->range));
                highlights.push_back(move(highlight));
            }
        }
    }
    return highlights;
}

void LSPTask::addLocIfExists(const core::GlobalState &gs, vector<unique_ptr<Location>> &locs, core::Loc loc) const {
    auto location = config.loc2Location(gs, loc);
    if (location != nullptr) {
        locs.push_back(std::move(location));
    }
}

} // namespace sorbet::realmain::lsp