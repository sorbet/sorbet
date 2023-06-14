#include "main/lsp/requests/sorbet_show_symbol.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

SorbetShowSymbolTask::SorbetShowSymbolTask(const LSPConfiguration &config, MessageId id,
                                           std::unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::SorbetShowSymbol), params(move(params)) {}

unique_ptr<ResponseMessage> SorbetShowSymbolTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::SorbetShowSymbol);

    const core::GlobalState &gs = typechecker.state();
    // To match the behavior of Go To Definition, we don't error in an untyped file, but instead
    // be okay with returning an empty result for certain queries.
    auto errorIfFileIsUntyped = false;
    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
                                  LSPMethod::SorbetShowSymbol, errorIfFileIsUntyped);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto &queryResponses = result.responses;
    if (queryResponses.empty()) {
        // Note: Need to specifically specify the variant type here so the null gets placed into the proper slot.
        response->result = variant<JSONNullObject, unique_ptr<SymbolInformation>>(JSONNullObject());
        return response;
    }

    auto resp = move(queryResponses[0]);

    core::SymbolRef sym;
    if (auto c = resp->isConstant()) {
        // Using symbolBeforeDealias instead of symbol here lets us show the name of the actual
        // constant under the user's cursor, not what it aliases to.
        sym = c->symbolBeforeDealias;
    } else if (auto d = resp->isMethodDef()) {
        sym = d->symbol;
    } else if (auto f = resp->isField()) {
        sym = f->symbol;
    } else if (auto s = resp->isSend()) {
        if (s->dispatchResult->secondary == nullptr) {
            // Multiple results not currently supported.
            sym = s->dispatchResult->main.method;
        }
    }

    if (!sym.exists()) {
        // At time of writing, we decided that it only makes sense to respond with information in
        // this request when there's a Symbol (in the GlobalState sense), and not to respond
        // with results for say local variables and other things not in the symbol table.
        response->result = variant<JSONNullObject, unique_ptr<SymbolInformation>>(JSONNullObject());
        return response;
    }

    // To be able to get this information, we'd have to walk the tree. It's not that important for
    // this method, so let's just fall back to treating attributes as methods.
    auto isAttrBestEffortUIOnly = false;

    auto symInfo = make_unique<SymbolInformation>(sym.show(gs), symbolRef2SymbolKind(gs, sym, isAttrBestEffortUIOnly),
                                                  config.loc2Location(gs, sym.loc(gs)));

    auto container = sym.owner(gs);
    if (container != core::Symbols::root()) {
        symInfo->containerName = container.show(gs);
    }

    response->result = std::move(symInfo);
    return response;
}

} // namespace sorbet::realmain::lsp
