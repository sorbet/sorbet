#ifndef SORBET_EXTRACT_VARIABLE_H
#define SORBET_EXTRACT_VARIABLE_H

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

std::vector<std::unique_ptr<TextDocumentEdit>> getExtractVariableEdits(LSPTypecheckerDelegate &typechecker,
                                                                       const LSPConfiguration &config,
                                                                       std::unique_ptr<Range> selectionRange,
                                                                       const core::Loc selectionLoc);

} // namespace sorbet::realmain::lsp

#endif
