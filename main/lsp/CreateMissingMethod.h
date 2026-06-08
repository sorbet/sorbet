#ifndef SORBET_CREATE_MISSING_METHOD_H
#define SORBET_CREATE_MISSING_METHOD_H

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

const core::lsp::SendResponse *isMissingMethodResponse(const core::GlobalState &gs,
                                                       const vector<unique_ptr<core::lsp::QueryResponse>> &responses);

std::vector<std::unique_ptr<TextDocumentEdit>> getAddMissingMethodEdits(LSPTypecheckerDelegate &typechecker,

                                                                        const LSPConfiguration &config,
                                                                        const core::lsp::SendResponse &resp);
} // namespace sorbet::realmain::lsp

#endif
