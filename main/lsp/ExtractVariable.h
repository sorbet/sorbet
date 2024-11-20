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
    ast::ExpressionPtr matchingNode;
    ast::ExpressionPtr enclosingClassOrMethod;
    std::vector<core::LocOffsets> skippedLocsRange;
    std::vector<core::LocOffsets> skippedLocsExact;

public:
    VariableExtractor(const core::Loc selectionLoc)
        : selectionLoc(selectionLoc), matchingNode(), enclosingClassOrMethod(nullptr) {}
    std::vector<std::unique_ptr<TextDocumentEdit>>
    getExtractSingleOccurrenceEdits(const LSPTypecheckerDelegate &typechecker, const LSPConfiguration &config);
    MultipleOccurrenceResult getExtractMultipleOccurrenceEdits(const LSPTypecheckerDelegate &typechecker,
                                                               const LSPConfiguration &config);
};

} // namespace sorbet::realmain::lsp

#endif
