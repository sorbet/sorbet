#ifndef SORBET_EXTRACT_VARIABLE_H
#define SORBET_EXTRACT_VARIABLE_H

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

class VariableExtractor {
public:
    static std::vector<std::unique_ptr<TextDocumentEdit>>
    getEdits(LSPTypecheckerDelegate &typechecker, const LSPConfiguration &config, const core::Loc selectionLoc);
};

} // namespace sorbet::realmain::lsp

#endif
