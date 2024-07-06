#ifndef SORBET_EXTRACT_VARIABLE_H
#define SORBET_EXTRACT_VARIABLE_H

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

struct MultipleOccurrenceResult {
    std::vector<std::unique_ptr<TextDocumentEdit>> documentEdits;
    size_t numOccurences;
};

class VariableExtractor {
    const core::Loc selectionLoc;
    std::optional<std::reference_wrapper<const ast::ExpressionPtr>> matchingNode;
    std::optional<std::reference_wrapper<const ast::ExpressionPtr>> enclosingClassOrMethod;
    std::vector<core::LocOffsets> skippedLocs;

public:
    VariableExtractor(const core::Loc selectionLoc)
        : selectionLoc(selectionLoc), matchingNode(std::nullopt), enclosingClassOrMethod(std::nullopt) {}
    std::vector<std::unique_ptr<TextDocumentEdit>>
    getExtractSingleOccurrenceEdits(const LSPTypecheckerDelegate &typechecker, const LSPConfiguration &config);
    MultipleOccurrenceResult getExtractMultipleOccurrenceEdits(const LSPTypecheckerDelegate &typechecker,
                                                               const LSPConfiguration &config);
};

} // namespace sorbet::realmain::lsp

#endif
