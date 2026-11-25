#ifndef SORBET_CREATE_MISSING_METHOD_H
#define SORBET_CREATE_MISSING_METHOD_H

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

namespace create_missing_method {
const core::lsp::SendResponse *
isMissingMethodResponse(const core::GlobalState &gs,
                        const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &responses);

std::vector<std::unique_ptr<TextDocumentEdit>> getCreateMissingMethodEdits(LSPTypecheckerDelegate &typechecker,
                                                                           const LSPConfiguration &config,
                                                                           const core::lsp::SendResponse &resp);
} // namespace create_missing_method
} // namespace sorbet::realmain::lsp

#endif
