#ifndef RUBY_TYPER_LSP_REQUESTS_SORBET_QUERY_SENDS_H
#define RUBY_TYPER_LSP_REQUESTS_SORBET_QUERY_SENDS_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class TextDocumentPositionParams;
class SorbetQuerySendsTask final : public LSPRequestTask {
    std::unique_ptr<QuerySendsParams> params;

    struct MethodMap {
        UnorderedMap<core::MethodRef, size_t> table;
        std::vector<std::string> names;

        size_t indexForMethod(const core::GlobalState &gs, core::MethodRef method);
    };

    MethodMap callerMap;
    MethodMap calleeMap;

public:
    SorbetQuerySendsTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<QuerySendsParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
