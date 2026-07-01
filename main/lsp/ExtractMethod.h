#ifndef SORBET_EXTRACT_METHOD_H
#define SORBET_EXTRACT_METHOD_H

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

namespace extract_method {
std::vector<std::unique_ptr<TextDocumentEdit>> getExtractMethodEdits(LSPTypecheckerDelegate &typechecker,
                                                                     const LSPConfiguration &config,
                                                                     const core::Loc selectionLoc);
} // namespace extract_method
} // namespace sorbet::realmain::lsp

#endif
