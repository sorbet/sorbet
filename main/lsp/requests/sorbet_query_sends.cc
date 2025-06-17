#include "main/lsp/requests/sorbet_query_sends.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

SorbetQuerySendsTask::SorbetQuerySendsTask(const LSPConfiguration &config, MessageId id,
                                           unique_ptr<QuerySendsParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::SorbetQuerySends), params(move(params)) {}

size_t SorbetQuerySendsTask::MethodMap::indexForMethod(const core::GlobalState &gs, core::MethodRef method) {
    auto it = table.find(method);
    if (it != table.end()) {
        return it->second;
    }

    size_t methodIndex = names.size();
    table[method] = methodIndex;
    names.emplace_back(method.show(gs));

    return methodIndex;
}

unique_ptr<ResponseMessage> SorbetQuerySendsTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::SorbetQuerySends);

    const core::GlobalState &gs = typechecker.state();
    auto result = LSPQuery::findSends(config, typechecker, params->textDocument->uri);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto &queryResponses = result.responses;
    if (queryResponses.empty()) {
        // Note: Need to specifically specify the variant type here so the null gets placed into the proper slot.
        response->result = variant<JSONNullObject, unique_ptr<QuerySendsResponse>>(JSONNullObject());
        return response;
    }

    vector<unique_ptr<SendInformation>> sends;

    for (auto &response : queryResponses) {
        auto s = response->isSend();

        if (s == nullptr) {
            // Eventually we might want these to send back a little more information?
            ENFORCE(false, "should never get non-send responses!");
            continue;
        }

        size_t methodIndex = callerMap.indexForMethod(gs, s->enclosingMethod);

        // TODO: propagate regionId into the SendResponse.
        auto regionId = 0;
        auto it = s->dispatchResult.get();
        while (it != nullptr) {
            auto calledMethod = it->main.method;
            // TODO: we should keep some count of "failed" sends here?
            // and/or some count of untyped sends?
            if (calledMethod.exists()) {
                size_t calledIndex = calleeMap.indexForMethod(gs, calledMethod);
                sends.push_back(make_unique<SendInformation>(methodIndex, calledIndex, regionId));
            }

            it = it->secondary.get();
        }
    }

    response->result =
        make_unique<QuerySendsResponse>(std::move(callerMap.names), std::move(calleeMap.names), std::move(sends));
    return response;
}

} // namespace sorbet::realmain::lsp
