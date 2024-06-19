#ifndef RUBY_TYPER_LSP_REQUESTS_REFERENCES_H
#define RUBY_TYPER_LSP_REQUESTS_REFERENCES_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class ReferenceParams;
class ReferencesTask final : public LSPRequestTask {
    std::unique_ptr<ReferenceParams> params;
    std::vector<core::SymbolRef> getSymsToCheckWithinPackage(const core::GlobalState &gs, core::SymbolRef symInPackage,
                                                             core::packages::MangledName packageName);
    core::SymbolRef findSym(const core::GlobalState &gs, const std::vector<core::NameRef> &fullName,
                            core::SymbolRef underNamespace);

private:
    bool notifyAboutUntypedFile = false;
    std::vector<std::unique_ptr<Location>>
    getLocationsFromQueryResponse(LSPTypecheckerDelegate &typechecker, const core::GlobalState &gs, core::FileRef fref,
                                  bool fileIsTyped, std::unique_ptr<core::lsp::QueryResponse> resp);

public:
    ReferencesTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<ReferenceParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;

    bool needsMultithreading(const LSPIndexer &indexer) const override;
};

} // namespace sorbet::realmain::lsp

#endif
