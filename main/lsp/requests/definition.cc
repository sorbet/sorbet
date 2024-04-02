#include "main/lsp/requests/definition.h"
#include "ast/treemap/treemap.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/NextMethodFinder.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

core::MethodRef firstMethodAfterQuery(LSPTypecheckerDelegate &typechecker, const core::Loc queryLoc) {
    const auto &gs = typechecker.state();
    auto files = vector<core::FileRef>{queryLoc.file()};
    auto resolved = typechecker.getResolved(files);

    NextMethodFinder nextMethodFinder(queryLoc);
    for (auto &t : resolved) {
        auto ctx = core::Context(gs, core::Symbols::root(), t.file);
        ast::ConstTreeWalk::apply(ctx, nextMethodFinder, t.tree);
    }

    return nextMethodFinder.result();
}

// TODO(jez) Can replace this with findMemberTransitiveAncestors once 7212 lands.
core::MethodRef findParentMethod(const core::GlobalState &gs, core::MethodRef childMethod) {
    const auto &klassData = childMethod.data(gs)->owner.data(gs);
    auto name = childMethod.data(gs)->name;

    for (const auto &mixin : klassData->mixins()) {
        auto superMethod = mixin.data(gs)->findMethod(gs, name);
        if (superMethod.exists()) {
            return superMethod;
        }
    }

    if (klassData->superClass().exists()) {
        return klassData->superClass().data(gs)->findMethodTransitive(gs, name);
    }

    return core::Symbols::noMethod();
}

} // namespace

DefinitionTask::DefinitionTask(const LSPConfiguration &config, MessageId id,
                               unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDefinition), params(move(params)) {}

unique_ptr<ResponseMessage> DefinitionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
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
    if (!queryResponses.empty()) {
        const bool fileIsTyped =
            config.uri2FileRef(gs, params->textDocument->uri).data(gs).strictLevel >= core::StrictLevel::True;
        auto resp = skipLiteralIfMethodDef(queryResponses);

        // Only support go-to-definition on constants and fields in untyped files.
        if (auto c = resp->isConstant()) {
            auto sym = c->symbolBeforeDealias;
            vector<pair<core::Loc, unique_ptr<Location>>> locMapping;
            for (auto loc : sym.locs(gs)) {
                locMapping.emplace_back(loc, config.loc2Location(gs, loc));
            }
            {
                // Erase all invalid locations
                auto validLocations = absl::c_partition(locMapping, [](const auto &p) { return p.second != nullptr; });
                locMapping.erase(validLocations, locMapping.end());
            }
            // If we have any non-RBI location, eliminate definitions in RBI files for classes and
            // modules (not methods), since class definitions in RBI files are usually only there to
            // specify missing methods on the class (and the user doesn't care about those
            // missing_method.rbi definitions).
            auto notIsRBI = [&gs](const auto &p) { return !p.first.file().data(gs).isRBI(); };
            if (sym.isClassOrModule() && !locMapping.empty() && absl::c_any_of(locMapping, notIsRBI)) {
                auto startOfRBIDefs = absl::c_partition(locMapping, notIsRBI);
                locMapping.erase(startOfRBIDefs, locMapping.end());
            }
            std::transform(locMapping.begin(), locMapping.end(), std::back_inserter(locations),
                           [](auto &p) { return std::move(p.second); });
        } else if (resp->isField() || (fileIsTyped && resp->isIdent())) {
            const auto &retType = resp->getTypeAndOrigins();
            for (auto &originLoc : retType.origins) {
                addLocIfExists(gs, locations, originLoc);
            }
        } else if (fileIsTyped && resp->isMethodDef()) {
            auto sym = resp->isMethodDef()->symbol;
            for (auto loc : sym.data(gs)->locs()) {
                addLocIfExists(gs, locations, loc);
            }
        } else if (fileIsTyped && resp->isSend()) {
            auto sendResp = resp->isSend();
            auto start = sendResp->dispatchResult.get();
            while (start != nullptr) {
                if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                    auto loc = start->main.method.data(gs)->loc();
                    if (start->main.method == core::Symbols::T_Private_Methods_DeclBuilder_override()) {
                        auto nextMethod = firstMethodAfterQuery(typechecker, sendResp->termLoc());
                        if (nextMethod.exists()) {
                            auto parentMethod = findParentMethod(gs, nextMethod);
                            if (parentMethod.exists()) {
                                // actually, jump to the definition of the abstract method, instead of
                                // the definition of `override` in builder.rbi
                                loc = parentMethod.data(gs)->loc();
                            }
                        }
                    }

                    addLocIfExists(gs, locations, loc);
                }
                start = start->secondary.get();
            }
        }
    }
    response->result = move(locations);
    return response;
}

} // namespace sorbet::realmain::lsp
