#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPLoop::addSignatureHelpItem(const core::GlobalState &gs, rapidjson::Value &signatures, core::SymbolRef method,
                                   const core::QueryResponse &resp, int activeParameter) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.signatureHelp");
    // signature helps only exist for methods.
    if (!method.exists() || !method.data(gs)->isMethod() || hideSymbol(gs, method)) {
        return;
    }
    rapidjson::Value sig;
    sig.SetObject();
    // Label is mandatory, so method name (i.e B#add) is shown for now. Might want to add markup highlighting
    // wtih respect to activeParameter here.
    sig.AddMember("label", method.data(gs)->show(gs), alloc);
    rapidjson::Value parameters;
    parameters.SetArray();
    // Documentation is set to be a markdown element that highlights which parameter you are currently typing in.
    string methodDocumentation = "(";
    auto args = method.data(gs)->arguments();
    int i = 0;
    for (auto arg : args) {
        rapidjson::Value parameter;
        parameter.SetObject();
        // label field is populated with the name of the variable.
        // Not sure why VSCode does not display this for now.
        parameter.AddMember("label", arg.data(gs)->name.show(gs), alloc);
        if (i == activeParameter) {
            // this bolds the active parameter in markdown
            methodDocumentation += "**_" + arg.data(gs)->name.show(gs) + "_**";
        } else {
            methodDocumentation += arg.data(gs)->name.show(gs);
        }
        if (i != args.size() - 1) {
            methodDocumentation += ", ";
        }
        parameter.AddMember("documentation", getResultType(gs, arg, resp.receiver.type, resp.constraint)->show(gs),
                            alloc);
        parameters.PushBack(move(parameter), alloc);
        i += 1;
    }
    methodDocumentation += ")";
    rapidjson::Value markupContents;
    markupContents.SetObject();
    markupContents.AddMember("kind", "markdown", alloc);
    markupContents.AddMember("value", methodDocumentation, alloc);
    sig.AddMember("documentation", markupContents, alloc);

    sig.AddMember("parameters", move(parameters), alloc);
    signatures.PushBack(move(sig), alloc);
}

unique_ptr<core::GlobalState> LSPLoop::handleTextSignatureHelp(unique_ptr<core::GlobalState> gs,
                                                               rapidjson::Value &result, rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.signatureHelp");
    result.SetObject();
    rapidjson::Value signatures;
    signatures.SetArray();

    auto finalGs = move(gs);
    auto run = setupLSPQueryByLoc(move(finalGs), d, LSPMethod::TextDocumentSignatureHelp(), false);
    finalGs = move(run.gs);
    auto &queryResponses = run.responses;
    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);
        auto receiverType = resp->receiver.type;
        // only triggers on sends. Some SignatureHelps are triggered when the variable is being typed.
        if (resp->kind == core::QueryResponse::Kind::SEND) {
            auto sendLocIndex = resp->termLoc.beginPos();

            auto uri = string_view(d["params"]["textDocument"]["uri"].GetString(),
                                   d["params"]["textDocument"]["uri"].GetStringLength());
            auto fref = uri2FileRef(uri);
            if (!fref.exists()) {
                return finalGs;
            }
            auto src = fref.data(*finalGs).source();
            auto loc = lspPos2Loc(fref, d, *finalGs);
            if (!loc) {
                return finalGs;
            }
            string_view call_str = src.substr(sendLocIndex, loc->endPos() - sendLocIndex);
            int numberCommas = absl::c_count(call_str, ',');
            // Active parameter depends on number of ,'s in the current string being typed. (0 , = first arg, 1 , =
            // 2nd arg)
            result.AddMember("activeParameter", numberCommas, alloc);

            auto firstDispatchComponentMethod = resp->dispatchComponents.front().method;
            addSignatureHelpItem(*finalGs, signatures, firstDispatchComponentMethod, *resp, numberCommas);
        }
    }

    result.AddMember("signatures", move(signatures), alloc);
    sendResult(d, result);
    return finalGs;
}
} // namespace sorbet::realmain::lsp
