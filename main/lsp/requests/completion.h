#ifndef RUBY_TYPER_LSP_REQUESTS_COMPLETION_H
#define RUBY_TYPER_LSP_REQUESTS_COMPLETION_H

#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class CompletionParams;
class CompletionItem;
class CompletionTask final : public LSPRequestTask {
    std::unique_ptr<CompletionParams> params;

    struct MethodSearchParams {
        // TODO(jez) Remember: you should be able to construct a fake DispatchResult for <ErrorNode>
        // constant responses.
        std::shared_ptr<core::DispatchResult> dispatchResult;
        size_t totalArgs;
        bool isPrivateOk;
    };

    struct SearchParams {
        // Point where completion results were requested
        core::Loc queryLoc;

        // Might be ""
        std::string_view prefix;

        // If nullopt, won't suggest methods.
        std::optional<MethodSearchParams> forMethods;

        // If false, won't suggest Ruby keywords
        bool suggestKeywords;

        // If not exists(), won't suggest locals for the enclosing method.
        core::MethodRef enclosingMethod;

        // If empty(), won't suggest constants.
        core::lsp::ConstantResponse::Scopes scopes;
    };
    static MethodSearchParams methodSearchParamsForEmptyAssign(const core::GlobalState &gs,
                                                               core::MethodRef enclosingMethod);
    static SearchParams searchParamsForEmptyAssign(const core::GlobalState &gs, core::Loc queryLoc,
                                                   core::MethodRef enclosingMethod,
                                                   core::lsp::ConstantResponse::Scopes scopes);
    std::vector<std::unique_ptr<CompletionItem>> getCompletionItems(LSPTypecheckerDelegate &typechecker,
                                                                    SearchParams &params);

    std::unique_ptr<CompletionItem> getCompletionItemForUntyped(const core::GlobalState &gs, core::Loc queryLoc,
                                                                size_t sortIdx, std::string_view message);

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
