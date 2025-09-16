#include "main/lsp/requests/signature_help.h"
#include "core/lsp/QueryResponse.h"
#include "core/source_generator/source_generator.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
void addSignatureHelpItem(const core::GlobalState &gs, core::MethodRef method,
                          vector<unique_ptr<SignatureInformation>> &sigs, const core::lsp::SendResponse &resp,
                          int activeParameter) {
    // signature helps only exist for methods.
    if (!method.exists() || hideSymbol(gs, method)) {
        return;
    }
    // Label is mandatory, so method name (i.e B#add) is shown for now. Might want to add markup highlighting
    // with respect to activeParameter here.
    auto sig = make_unique<SignatureInformation>(method.show(gs));

    vector<unique_ptr<ParameterInformation>> parameters;
    // Documentation is set to be a markdown element that highlights which parameter you are currently typing in.
    string methodDocumentation = "(";
    auto &params = method.data(gs)->parameters;
    int i = 0;
    for (const auto &param : params) {
        // label field is populated with the name of the variable.
        // Not sure why VSCode does not display this for now.
        auto paramName = string(param.parameterName(gs));
        auto parameter = make_unique<ParameterInformation>(paramName);
        if (i == activeParameter) {
            // this bolds the active parameter in markdown
            methodDocumentation += "**_" + paramName + "_**";
        } else {
            methodDocumentation += paramName;
        }
        if (i != params.size() - 1) {
            methodDocumentation += ", ";
        }
        parameter->documentation =
            core::source_generator::getResultType(gs, param.type, method, resp.dispatchResult->main.receiver).show(gs);
        parameters.push_back(move(parameter));
        i += 1;
    }
    methodDocumentation += ")";

    auto markupContents = make_unique<MarkupContent>(MarkupKind::Markdown, methodDocumentation);
    sig->documentation = move(markupContents);
    sig->parameters = move(parameters);
    sigs.push_back(move(sig));
}

bool hasAngleBrackets(string_view haystack) {
    return absl::c_any_of(haystack, [](char c) { return c == '<' || c == '>'; });
}
} // namespace

SignatureHelpTask::SignatureHelpTask(const LSPConfiguration &config, MessageId id,
                                     unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentSignatureHelp), params(move(params)) {}

unique_ptr<ResponseMessage> SignatureHelpTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentSignatureHelp);
    if (!config.opts.lspSignatureHelpEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Signature Help` LSP feature is experimental and disabled by default.");
        return response;
    }

    const core::GlobalState &gs = typechecker.state();
    const auto &uri = params->textDocument->uri;
    auto result = LSPQuery::byLoc(config, typechecker, uri, *params->position, LSPMethod::TextDocumentSignatureHelp);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto &queryResponses = result.responses;
    int activeParameter = -1;
    vector<unique_ptr<SignatureInformation>> signatures;
    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);
        // only triggers on sends. Some SignatureHelps are triggered when the variable is being typed.
        if (auto sendResp = resp->isSend()) {
            if (hasAngleBrackets(sendResp->callerSideName.shortName(gs))) {
                // The method location doesn't exist, which means that we're dealing with a synthesized send. Don't
                // generate any help.
                response->result = make_unique<SignatureHelp>(move(signatures));
                return response;
            }
            auto sendLocIndex = sendResp->termLoc().beginPos();

            auto fref = config.uri2FileRef(gs, uri);
            if (!fref.exists()) {
                response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                                             fmt::format("Unknown file: `{}`", uri));
                return response;
            }
            auto src = fref.data(gs).source();
            auto loc = params->position->toLoc(gs, fref);
            ENFORCE(loc.has_value(), "LSPQuery::byLoc should have reported an error earlier if nullopt");
            string_view call_str = src.substr(sendLocIndex, loc.value().endPos() - sendLocIndex);
            int numberCommas = absl::c_count(call_str, ',');
            // Active parameter depends on number of ,'s in the current string being typed. (0 , = first arg, 1 , =
            // 2nd arg)
            activeParameter = numberCommas;

            auto firstDispatchComponentMethod = sendResp->dispatchResult->main.method;

            addSignatureHelpItem(gs, firstDispatchComponentMethod, signatures, *sendResp, numberCommas);
        }
    }
    auto sigHelp = make_unique<SignatureHelp>(move(signatures));
    if (activeParameter != -1) {
        sigHelp->activeParameter = activeParameter;
    }
    response->result = move(sigHelp);
    return response;
}
} // namespace sorbet::realmain::lsp
