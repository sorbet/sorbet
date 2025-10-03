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
    auto resolved = typechecker.getResolved(queryLoc.file());

    NextMethodFinder nextMethodFinder(queryLoc);
    auto ctx = core::Context(gs, core::Symbols::root(), resolved.file);
    ast::ConstTreeWalk::apply(ctx, nextMethodFinder, resolved.tree);

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
    const auto &uri = params->textDocument->uri;
    auto result =
        LSPQuery::byLoc(config, typechecker, uri, *params->position, LSPMethod::TextDocumentDefinition, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto fref = config.uri2FileRef(gs, uri);
    // LSPQuery::byLoc reports an error if the file or loc don't exist
    auto queryLoc = params->position->toLoc(gs, fref).value();

    auto &queryResponses = result.responses;
    vector<unique_ptr<Location>> locations;
    if (!queryResponses.empty()) {
        const bool fileIsTyped = fref.data(gs).strictLevel >= core::StrictLevel::True;
        auto resp = skipLiteralIfMethodDef(gs, queryResponses);

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
        } else if (auto f = resp->isField()) {
            for (auto &originLoc : f->retType.origins) {
                addLocIfExists(gs, locations, originLoc);
            }
        } else if (!fileIsTyped) {
            // everything after this requires a typed: true or higher file
        } else if (resp->isIdent()) {
            auto identResp = resp->isIdent();
            for (auto &originLoc : identResp->retType.origins) {
                addLocIfExists(gs, locations, originLoc);
            }
        } else if (resp->isMethodDef()) {
            auto sym = resp->isMethodDef()->symbol;
            for (auto loc : sym.data(gs)->locs()) {
                addLocIfExists(gs, locations, loc);
            }
        } else if (resp->isSend()) {
            auto sendResp = resp->isSend();
            // Don't want to show hover results if we're hovering over, e.g., the arguments, and there's nothing there.
            if (sendResp->funLoc().exists() && sendResp->funLoc().contains(queryLoc)) {
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
        } else if (auto kw = resp->isKeywordArg()) {
            // We only store one loc for a ParamInfo, which disguises the full set of locs if there are multiple.
            addLocIfExists(gs, locations, kw->paramLoc);

            // Loop over all (in case of multiple dispatch targets)
            for (const auto &resp : queryResponses) {
                if (resp == nullptr) {
                    continue;
                }
                auto *kw = resp->isKeywordArg();
                if (kw == nullptr) {
                    continue;
                }

                addLocIfExists(gs, locations, kw->paramLoc);
            }
        }
    }
    response->result = move(locations);
    return response;
}

} // namespace sorbet::realmain::lsp
