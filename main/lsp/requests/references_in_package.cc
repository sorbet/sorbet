#include "main/lsp/requests/references_in_package.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

ReferencesInPackageTask::ReferencesInPackageTask(const LSPConfiguration &config, MessageId id,
                                                 std::unique_ptr<ReferenceParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentReferencesInPackage), params(move(params)) {}

bool ReferencesInPackageTask::needsMultithreading(const LSPIndexer &indexer) const {
    return true;
}

// Symbol in package file is under the <PackageSpec> namespace, find it under the root namespace
unique_ptr<ResponseMessage> ReferencesInPackageTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentReferencesInPackage);
    ShowOperation op(config, ShowOperation::Kind::ReferencesInPackage);

    const core::GlobalState &gs = typechecker.state();
    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
                                  LSPMethod::TextDocumentReferencesInPackage, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
    response->result = variant<JSONNullObject, vector<unique_ptr<Location>>>(JSONNullObject());
    auto &queryResponses = result.responses;
    core::FileRef fref = config.uri2FileRef(gs, params->textDocument->uri);
    if (!fref.exists() || !fref.data(typechecker.state()).isPackage()) { // only works for package files
        return response;
    }

    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);
        if (auto constResp = resp->isConstant()) {
            auto realSym = getRealSymFromPackageSym(gs, constResp->symbol);
            response->result =
                extractLocations(typechecker.state(), getReferencesInPackageToSymbol(typechecker, fref, realSym));
        }
    }

    return response;
}

core::SymbolRef ReferencesInPackageTask::getRealSymFromPackageSym(const core::GlobalState &gs,
                                                                  core::SymbolRef packageSym) {
    std::vector<core::NameRef> fullName;

    auto sym = packageSym;
    while (sym.exists() && sym != core::Symbols::PackageSpecRegistry()) {
        fullName.emplace_back(sym.name(gs));
        sym = sym.owner(gs);
    }
    reverse(fullName.begin(), fullName.end());

    core::SymbolRef realSym = core::Symbols::root();
    for (auto &part : fullName) {
        realSym = realSym.asClassOrModuleRef().data(gs)->findMember(gs, part);
    }

    return realSym;
}

} // namespace sorbet::realmain::lsp
