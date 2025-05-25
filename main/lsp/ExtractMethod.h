#ifndef SORBET_EXTRACT_METHOD_H
#define SORBET_EXTRACT_METHOD_H

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

class MethodExtractor {
    const core::Loc selectionLoc;
    ast::ExpressionPtr matchingNode;

public:
    MethodExtractor(const core::Loc selectionLoc) : selectionLoc(selectionLoc) {}
    std::vector<std::unique_ptr<TextDocumentEdit>> getExtractEdits(const LSPTypecheckerDelegate &typechecker,
                                                                   const LSPConfiguration &config);
};

} // namespace sorbet::realmain::lsp

#endif
