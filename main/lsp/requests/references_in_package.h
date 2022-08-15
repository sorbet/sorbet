#ifndef RUBY_TYPER_LSP_REQUESTS_REFERENCES_IN_PACKAGE_H
#define RUBY_TYPER_LSP_REQUESTS_REFERENCES_IN_PACKAGE_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class ReferenceParams;
class ReferencesInPackageTask final : public LSPRequestTask {
    std::unique_ptr<ReferenceParams> params;
    core::SymbolRef getRealSymFromPackageSym(const core::GlobalState &gs, core::SymbolRef packageSym);

public:
    ReferencesInPackageTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<ReferenceParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerInterface &typechecker) override;

    bool needsMultithreading(const LSPIndexer &indexer) const override;
};

} // namespace sorbet::realmain::lsp

#endif
