#include "absl/algorithm/container.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet {
namespace realmain {
namespace lsp {
void LSPLoop::addSignatureHelpItem(rapidjson::Value &signatures, core::SymbolRef method,
                                   const core::QueryResponse &resp, int activeParameter) {
    // signature helps only exist for methods.
    if (!method.exists() || !method.data(*finalGs).isMethod() || hideSymbol(method)) {
        return;
    }
    rapidjson::Value sig;
    sig.SetObject();
    // Label is mandatory, so method name (i.e B#add) is shown for now. Might want to add markup highlighting
    // wtih respect to activeParameter here.
    sig.AddMember("label", (string)method.data(*finalGs).show(*finalGs), alloc);
    rapidjson::Value parameters;
    parameters.SetArray();
    // Documentation is set to be a markdown element that highlights which parameter you are currently typing in.
    string methodDocumentation = "(";
    auto args = method.data(*finalGs).arguments();
    int i = 0;
    for (auto arg : args) {
        rapidjson::Value parameter;
        parameter.SetObject();
        // label field is populated with the name of the variable.
        // Not sure why VSCode does not display this for now.
        parameter.AddMember("label", (string)arg.data(*finalGs).name.show(*finalGs), alloc);
        if (i == activeParameter) {
            // this bolds the active parameter in markdown
            methodDocumentation += "**_" + (string)arg.data(*finalGs).name.show(*finalGs) + "_**";
        } else {
            methodDocumentation += (string)arg.data(*finalGs).name.show(*finalGs);
        }
        if (i != args.size() - 1) {
            methodDocumentation += ", ";
        }
        parameter.AddMember("documentation",
                            (string)getResultType(arg, resp.receiver.type, resp.constraint)->show(*finalGs), alloc);
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

void LSPLoop::handleTextSignatureHelp(rapidjson::Value &result, rapidjson::Document &d) {
    result.SetObject();
    rapidjson::Value signatures;
    signatures.SetArray();

    if (setupLSPQueryByLoc(d, LSPMethod::TextDocumentSignatureHelp(), false)) {
        auto queryResponses = errorQueue->drainQueryResponses();
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);
            auto receiverType = resp->receiver.type;
            // only triggers on sends. Some SignatureHelps are triggered when the variable is being typed.
            if (resp->kind == core::QueryResponse::Kind::SEND) {
                auto sendLocIndex = resp->termLoc.beginPos();

                auto uri = string(d["params"]["textDocument"]["uri"].GetString(),
                                  d["params"]["textDocument"]["uri"].GetStringLength());
                auto fref = uri2FileRef(uri);
                if (!fref.exists()) {
                    return;
                }
                auto src = fref.data(*finalGs).source();
                auto loc = lspPos2Loc(fref, d, *finalGs);
                if (!loc) {
                    return;
                }
                string call_str = (string)src.substr(sendLocIndex, loc->endPos() - sendLocIndex);
                int numberCommas = absl::c_count(call_str, ',');
                // Active parameter depends on number of ,'s in the current string being typed. (0 , = first arg, 1 , =
                // 2nd arg)
                result.AddMember("activeParameter", numberCommas, alloc);

                auto firstDispatchComponentMethod = resp->dispatchComponents.front().method;
                addSignatureHelpItem(signatures, firstDispatchComponentMethod, *resp, numberCommas);
            }
        }
    }

    result.AddMember("signatures", move(signatures), alloc);
    sendResult(d, result);
}
} // namespace lsp
} // namespace realmain
} // namespace sorbet