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

        // If not exists(), won't suggest kwargs for the method that this query occurs in the arg list of.
        core::MethodRef kwargsMethod;

        // The location of the method in a send, if applicable.
        core::LocOffsets funLoc;

        // The location of a method receiver, if applicable.
        core::LocOffsets receiverLoc;

        // If empty(), won't suggest constants.
        core::lsp::ConstantResponse::Scopes scopes;
    };
    static MethodSearchParams methodSearchParamsForEmptyAssign(const core::GlobalState &gs,
                                                               core::MethodRef enclosingMethod);
    static SearchParams searchParamsForEmptyAssign(const core::GlobalState &gs, core::Loc queryLoc,
                                                   core::MethodRef enclosingMethod,
                                                   core::lsp::ConstantResponse::Scopes scopes);
    std::vector<std::unique_ptr<CompletionItem>>
    getCompletionItems(LSPTypecheckerDelegate &typechecker, SearchParams &params, const ast::ParsedFile &resolved);

    std::unique_ptr<CompletionItem> getCompletionItemForUntyped(const core::GlobalState &gs, core::Loc queryLoc,
                                                                size_t sortIdx, std::string_view message);

    std::unique_ptr<CompletionItem> getCompletionItemForMethod(LSPTypecheckerDelegate &typechecker,
                                                               const SearchParams &params, core::MethodRef what,
                                                               const core::TypePtr &receiverType, core::Loc queryLoc,
                                                               const ast::ParsedFile &resolved, std::string_view prefix,
                                                               size_t sortIdx, uint16_t totalArgs) const;

public:
    CompletionTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<CompletionParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
