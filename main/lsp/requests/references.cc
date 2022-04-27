#include "main/lsp/requests/references.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

ReferencesTask::ReferencesTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<ReferenceParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentReferences), params(move(params)) {}

bool ReferencesTask::needsMultithreading(const LSPIndexer &indexer) const {
    return true;
}

unique_ptr<ResponseMessage> ReferencesTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentReferences);
    ShowOperation op(config, ShowOperation::Kind::References);

    const core::GlobalState &gs = typechecker.state();
    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
                                  LSPMethod::TextDocumentReferences, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
    response->result = variant<JSONNullObject, vector<unique_ptr<Location>>>(JSONNullObject());
    auto &queryResponses = result.responses;
    bool notifyAboutUntypedFile = false;
    core::FileRef fref = config.uri2FileRef(gs, params->textDocument->uri);
    bool fileIsTyped = false;
    if (fref.exists()) {
        fileIsTyped = fref.data(gs).strictLevel >= core::StrictLevel::True;
    }
    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);
        // N.B.: Ignores literals.
        // If file is untyped, only supports find reference requests from constants and class definitions.
        if (auto constResp = resp->isConstant()) {
            response->result =
                extractLocations(typechecker.state(), getReferencesToSymbol(typechecker, constResp->symbol));
        } else if (auto fieldResp = resp->isField()) {
            // This could be a `prop` or `attr_*`, which have multiple associated symbols.
            response->result = extractLocations(
                typechecker.state(),
                getReferencesToAccessor(typechecker, getAccessorInfo(typechecker.state(), fieldResp->symbol),
                                        fieldResp->symbol));
        } else if (auto defResp = resp->isMethodDef()) {
            if (fileIsTyped) {
                // This could be a `prop` or `attr_*`, which have multiple associated symbols.
                response->result = extractLocations(
                    typechecker.state(),
                    getReferencesToAccessor(typechecker, getAccessorInfo(typechecker.state(), defResp->symbol),
                                            defResp->symbol));
            } else {
                notifyAboutUntypedFile = true;
            }
        } else if (auto identResp = resp->isIdent()) {
            if (fileIsTyped) {
                auto loc = identResp->termLoc;
                if (loc.exists()) {
                    auto run2 = typechecker.query(
                        core::lsp::Query::createVarQuery(identResp->enclosingMethod, identResp->variable),
                        {loc.file()});
                    response->result = extractLocations(gs, run2.responses);
                }
            } else {
                notifyAboutUntypedFile = true;
            }
        } else if (auto sendResp = resp->isSend()) {
            if (fileIsTyped) {
                auto start = sendResp->dispatchResult.get();
                vector<unique_ptr<core::lsp::QueryResponse>> responses;
                while (start != nullptr) {
                    if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                        // This could be a `prop` or `attr_*`, which has multiple associated symbols.
                        responses = getReferencesToAccessor(typechecker,
                                                            getAccessorInfo(typechecker.state(), start->main.method),
                                                            start->main.method, move(responses));
                    }
                    start = start->secondary.get();
                }
                response->result = extractLocations(typechecker.state(), responses);
            } else {
                notifyAboutUntypedFile = true;
            }
        }
    } else if (fref.exists() && !fileIsTyped) {
        // The first check ensures that the file actually exists (and therefore
        // we could have gotten responses) and the second check is what we are
        // actually interested in.
        notifyAboutUntypedFile = true;
    }

    if (notifyAboutUntypedFile) {
        ENFORCE(fref.exists());
        auto level = fref.data(gs).strictLevel;
        ENFORCE(level < core::StrictLevel::True);
        string asString = level == core::StrictLevel::Ignore ? "ignore" : "false";
        auto msg = fmt::format("File is `# typed: {}`, could not determine references", asString);
        auto params = make_unique<ShowMessageParams>(MessageType::Info, msg);
        this->config.output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::WindowShowMessage, move(params))));
    }
    return response;
}

} // namespace sorbet::realmain::lsp
