#include "main/lsp/requests/sorbet_query_sends.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

SorbetQuerySendsTask::SorbetQuerySendsTask(const LSPConfiguration &config, MessageId id,
                                           unique_ptr<QuerySendsParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::SorbetQuerySends), params(move(params)) {}

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
        response->result = variant<JSONNullObject, vector<unique_ptr<SendInformation>>>(JSONNullObject());
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

        auto enclosingMethod = s->enclosingMethod.show(gs);
        // TODO: propagate regionId into the SendResponse.
        auto regionId = 0;
        auto it = &(*s->dispatchResult);
        while (it != nullptr) {
            auto calledMethod = it->main.method;
            if (calledMethod.exists()) {
                sends.push_back(make_unique<SendInformation>(enclosingMethod, calledMethod.show(gs), regionId));
            }

            it = it->secondary.get();
        }
    }

    response->result = std::move(sends);
    return response;
}

} // namespace sorbet::realmain::lsp
