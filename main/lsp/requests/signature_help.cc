#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void addSignatureHelpItem(const core::GlobalState &gs, core::SymbolRef method,
                          vector<unique_ptr<SignatureInformation>> &sigs, const core::lsp::SendResponse &resp,
                          int activeParameter) {
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
        auto parameter = make_unique<ParameterInformation>(arg.data(gs)->argumentName(gs));
        if (i == activeParameter) {
            // this bolds the active parameter in markdown
            methodDocumentation += "**_" + arg.data(gs)->argumentName(gs) + "_**";
        } else {
            methodDocumentation += arg.data(gs)->argumentName(gs);
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

LSPResult LSPLoop::handleTextSignatureHelp(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const TextDocumentPositionParams &params) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentSignatureHelp);
    if (!opts.lspSignatureHelpEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Signature Help` LSP feature is experimental and disabled by default.");
        return LSPResult::make(move(gs), move(response));
    }

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.signatureHelp");
    auto result = setupLSPQueryByLoc(move(gs), params.textDocument->uri, *params.position,
                                     LSPMethod::TextDocumentSignatureHelp, false);
    if (auto run = get_if<TypecheckRun>(&result)) {
        gs = move(run->gs);
        auto &queryResponses = run->responses;
        int activeParameter = -1;
        vector<unique_ptr<SignatureInformation>> signatures;
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);
            // only triggers on sends. Some SignatureHelps are triggered when the variable is being typed.
            if (auto sendResp = resp->isSend()) {
                auto sendLocIndex = sendResp->termLoc.beginPos();

                auto fref = uri2FileRef(params.textDocument->uri);
                if (!fref.exists()) {
                    // TODO(jvilk): This should probably return *something*; it's a request!
                    return LSPResult{move(gs), {}};
                }
                auto src = fref.data(*gs).source();
                auto loc = lspPos2Loc(fref, *params.position, *gs);
                if (!loc) {
                    return LSPResult{move(gs), {}};
                }
                string_view call_str = src.substr(sendLocIndex, loc->endPos() - sendLocIndex);
                int numberCommas = absl::c_count(call_str, ',');
                // Active parameter depends on number of ,'s in the current string being typed. (0 , = first arg, 1 , =
                // 2nd arg)
                activeParameter = numberCommas;

                auto firstDispatchComponentMethod = sendResp->dispatchComponents.front().method;

                addSignatureHelpItem(*gs, firstDispatchComponentMethod, signatures, *sendResp, numberCommas);
            }
        }
        auto result = make_unique<SignatureHelp>(move(signatures));
        if (activeParameter != -1) {
            result->activeParameter = activeParameter;
        }
        response->result = move(result);
    } else if (auto error = get_if<pair<unique_ptr<ResponseError>, unique_ptr<core::GlobalState>>>(&result)) {
        // An error happened while setting up the query.
        response->error = move(error->first);
    } else {
        // Should never happen, but satisfy the compiler.
        ENFORCE(false, "Internal error: setupLSPQueryByLoc returned invalid value.");
    }
    return LSPResult::make(move(gs), move(response));
}
} // namespace sorbet::realmain::lsp
