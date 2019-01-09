#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void addSignatureHelpItem(const core::GlobalState &gs, core::SymbolRef method,
                          vector<unique_ptr<SignatureInformation>> &sigs, const core::QueryResponse &resp,
                          int activeParameter) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.signatureHelp");
    // signature helps only exist for methods.
    if (!method.exists() || !method.data(gs)->isMethod() || hideSymbol(gs, method)) {
        return;
    }
    // Label is mandatory, so method name (i.e B#add) is shown for now. Might want to add markup highlighting
    // wtih respect to activeParameter here.
    auto sig = make_unique<SignatureInformation>(method.data(gs)->show(gs));

    vector<unique_ptr<ParameterInformation>> parameters;
    // Documentation is set to be a markdown element that highlights which parameter you are currently typing in.
    string methodDocumentation = "(";
    auto args = method.data(gs)->arguments();
    int i = 0;
    for (auto arg : args) {
        // label field is populated with the name of the variable.
        // Not sure why VSCode does not display this for now.
        auto parameter = make_unique<ParameterInformation>(arg.data(gs)->name.show(gs));
        if (i == activeParameter) {
            // this bolds the active parameter in markdown
            methodDocumentation += "**_" + arg.data(gs)->name.show(gs) + "_**";
        } else {
            methodDocumentation += arg.data(gs)->name.show(gs);
        }
        if (i != args.size() - 1) {
            methodDocumentation += ", ";
        }
        parameter->documentation = getResultType(gs, arg, resp.receiver.type, resp.constraint)->show(gs);
        parameters.push_back(move(parameter));
        i += 1;
    }
    methodDocumentation += ")";

    auto markupContents = make_unique<MarkupContent>(MarkupKind::Markdown, methodDocumentation);
    sig->documentation = move(markupContents);
    sig->parameters = move(parameters);
    sigs.push_back(move(sig));
}

unique_ptr<core::GlobalState> LSPLoop::handleTextSignatureHelp(unique_ptr<core::GlobalState> gs,
                                                               rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.signatureHelp");
    auto finalGs = move(gs);
    auto run = setupLSPQueryByLoc(move(finalGs), d, LSPMethod::TextDocumentSignatureHelp(), false);
    finalGs = move(run.gs);
    auto &queryResponses = run.responses;
    int activeParameter = -1;
    vector<unique_ptr<SignatureInformation>> signatures;
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
            activeParameter = numberCommas;

            auto firstDispatchComponentMethod = resp->dispatchComponents.front().method;

            addSignatureHelpItem(*finalGs, firstDispatchComponentMethod, signatures, *resp, numberCommas);
        }
    }
    auto result = make_unique<SignatureHelp>(move(signatures));
    if (activeParameter != -1) {
        result->activeParameter = activeParameter;
    }

    sendResult(d, *result);
    return finalGs;
}
} // namespace sorbet::realmain::lsp
