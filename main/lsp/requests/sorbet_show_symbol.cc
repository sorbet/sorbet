#include "main/lsp/requests/sorbet_show_symbol.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

SorbetShowSymbolTask::SorbetShowSymbolTask(const LSPConfiguration &config, MessageId id,
                                           std::unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::SorbetShowSymbol), params(move(params)) {}

unique_ptr<ResponseMessage> SorbetShowSymbolTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::SorbetShowSymbol);

    const core::GlobalState &gs = typechecker.state();
    auto result = queryByLoc(typechecker, params->textDocument->uri, *params->position, LSPMethod::SorbetShowSymbol);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        auto &queryResponses = result.responses;
        if (queryResponses.empty()) {
            // Note: Need to specifically specify the variant type here so the null gets placed into the proper slot.
            response->result = variant<JSONNullObject, unique_ptr<SymbolInformation>>(JSONNullObject());
            return response;
        }

        auto resp = move(queryResponses[0]);

        core::SymbolRef sym;
        if (auto c = resp->isConstant()) {
            sym = c->symbol;
        } else if (auto d = resp->isDefinition()) {
            sym = d->symbol;
        } else if (auto f = resp->isField()) {
            sym = f->symbol;
        } else {
            response->result = variant<JSONNullObject, unique_ptr<SymbolInformation>>(JSONNullObject());
            return response;
        }

        auto result = make_unique<SymbolInformation>(sym.show(gs), symbolRef2SymbolKind(gs, sym),
                                                     config.loc2Location(gs, sym.data(gs)->loc()));

        auto container = sym.data(gs)->owner;
        if (container != core::Symbols::root()) {
            result->containerName = container.show(gs);
        }

        response->result = std::move(result);
    }
    return response;
}

} // namespace sorbet::realmain::lsp
