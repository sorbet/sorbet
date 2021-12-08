#ifndef RUBY_TYPER_LSP_REQUESTS_COMPLETION_H
#define RUBY_TYPER_LSP_REQUESTS_COMPLETION_H

#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class CompletionParams;
class CompletionItem;
class CompletionTask final : public LSPRequestTask {
    std::unique_ptr<CompletionParams> params;

    void findSimilarConstants(const core::GlobalState &gs, const core::lsp::ConstantResponse &resp, core::Loc queryLoc,
                              std::vector<std::unique_ptr<CompletionItem>> &items) const;

    std::unique_ptr<CompletionItem> getCompletionItemForMethod(LSPTypecheckerDelegate &typechecker,
                                                               core::DispatchResult &dispatchResult,
                                                               core::MethodRef what, const core::TypePtr &receiverType,
                                                               const core::TypeConstraint *constraint,
                                                               core::Loc queryLoc, std::string_view prefix,
                                                               size_t sortIdx, uint16_t totalArgs) const;

public:
    CompletionTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<CompletionParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
