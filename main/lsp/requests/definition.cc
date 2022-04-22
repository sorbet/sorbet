#include "main/lsp/requests/definition.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
DefinitionTask::DefinitionTask(const LSPConfiguration &config, MessageId id,
                               unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDefinition), params(move(params)) {}

unique_ptr<ResponseMessage> DefinitionTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDefinition);
    const core::GlobalState &gs = typechecker.state();
    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
                                  LSPMethod::TextDocumentDefinition, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto &queryResponses = result.responses;
    vector<unique_ptr<Location>> locations;
    bool notifyAboutUntypedFile = false;
    core::FileRef fref = config.uri2FileRef(gs, params->textDocument->uri);
    bool fileIsTyped = false;
    if (fref.exists()) {
        fileIsTyped = fref.data(gs).strictLevel >= core::StrictLevel::True;
    }
    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);

        // Only support go-to-definition on constants and fields in untyped files.
        if (auto c = resp->isConstant()) {
            auto sym = c->symbol;
            for (auto loc : sym.locs(gs)) {
                addLocIfExists(gs, locations, loc);
            }
        } else if (resp->isField() || (fileIsTyped && (resp->isIdent() || resp->isLiteral()))) {
            const auto &retType = resp->getTypeAndOrigins();
            for (auto &originLoc : retType.origins) {
                addLocIfExists(gs, locations, originLoc);
            }
        } else if (!fileIsTyped && (resp->isIdent() || resp->isLiteral())) {
            notifyAboutUntypedFile = true;
        } else if (resp->isMethodDef()) {
            if (!fileIsTyped) {
                notifyAboutUntypedFile = true;
            } else {
                auto sym = resp->isMethodDef()->symbol;
                for (auto loc : sym.data(gs)->locs()) {
                    addLocIfExists(gs, locations, loc);
                }
            }
        } else if (resp->isSend()) {
            if (!fileIsTyped) {
                notifyAboutUntypedFile = true;
            } else {
                auto sendResp = resp->isSend();
                auto start = sendResp->dispatchResult.get();
                while (start != nullptr) {
                    if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                        addLocIfExists(gs, locations, start->main.method.data(gs)->loc());
                    }
                    start = start->secondary.get();
                }
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
        auto offsets = core::File::locStrictSigil(fref.data(gs).source());
        core::Loc loc{fref, offsets};
        auto msg = fmt::format("Could not go to definition because the file is not at least `# typed: true`");
        auto params = make_unique<ShowMessageParams>(MessageType::Info, msg);
        this->config.output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::WindowShowMessage, move(params))));
        // Jump the user to the sigil location.
        ENFORCE(locations.empty());
        addLocIfExists(gs, locations, loc);
    }
    response->result = move(locations);
    return response;
}

} // namespace sorbet::realmain::lsp
