#include "main/lsp/requests/hover.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_join.h"
#include "common/sort/sort.h"
#include "core/lsp/QueryResponse.h"
#include "core/source_generator/source_generator.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

string methodInfoString(const core::GlobalState &gs, const core::TypePtr &retType,
                        const core::DispatchResult &dispatchResult, const unique_ptr<core::TypeConstraint> &constraint,
                        const core::ShowOptions options) {
    string contents;
    auto start = &dispatchResult;
    ;
    while (start != nullptr) {
        auto &component = start->main;
        if (component.method.exists()) {
            if (!contents.empty()) {
                contents += "\n";
            }
            contents = absl::StrCat(contents, core::source_generator::prettyTypeForMethod(gs, component.method,
                                                                                          component.receiver, retType,
                                                                                          constraint.get(), options));
        }
        start = start->secondary.get();
    }

    return contents;
}

HoverTask::HoverTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentHover), params(move(params)) {}

unique_ptr<ResponseMessage> HoverTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentHover);

    const core::GlobalState &gs = typechecker.state();
    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
                                  LSPMethod::TextDocumentHover, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto &queryResponses = result.responses;
    auto clientHoverMarkupKind = config.getClientConfig().clientHoverMarkupKind;
    if (queryResponses.empty()) {
        auto fref = config.uri2FileRef(gs, params->textDocument->uri);
        ENFORCE(fref.exists());
        auto level = fref.data(gs).strictLevel;
        if (level < core::StrictLevel::True) {
            auto text = level == core::StrictLevel::Ignore
                            ? "This file is `# typed: ignore`.\n"
                              "No Sorbet IDE features will work in this file."
                            : "This file is `# typed: false`.\n"
                              "Most Hover results will not appear until the file is `# typed: true` or higher.";
            response->result = make_unique<Hover>(make_unique<MarkupContent>(clientHoverMarkupKind, text));
        } else {
            // Note: Need to specifically specify the variant type here so the null gets placed into the proper slot.
            response->result = variant<JSONNullObject, unique_ptr<Hover>>(JSONNullObject());
        }
        return response;
    }

    auto resp = skipLiteralIfMethodDef(gs, queryResponses);
    auto options = core::ShowOptions();
    vector<core::Loc> documentationLocations;
    string typeString;

    if (auto s = resp->isSend()) {
        auto start = s->dispatchResult.get();
        if (start != nullptr && start->main.method.exists() && !start->main.receiver.isUntyped()) {
            auto loc = start->main.method.data(gs)->loc();
            if (loc.exists()) {
                documentationLocations.emplace_back(loc);
            }
        }

        auto retType = s->dispatchResult->returnType;
        auto &constraint = s->dispatchResult->main.constr;
        if (constraint) {
            retType = core::Types::instantiate(gs, retType, *constraint);
        }
        if (s->dispatchResult->main.method.exists() &&
            s->dispatchResult->main.method.data(gs)->owner == core::Symbols::MagicSingleton()) {
            // Most <Magic>.<foo> are not meant to be exposed to the user. Instead, just show
            // the result type.
            typeString = retType.showWithMoreInfo(gs);
        } else {
            typeString = methodInfoString(gs, retType, *s->dispatchResult, constraint, options);
        }
    } else if (auto c = resp->isConstant()) {
        for (auto loc : c->symbolBeforeDealias.locs(gs)) {
            if (loc.exists()) {
                documentationLocations.emplace_back(loc);
            }
        }
        auto dealiased = c->symbolBeforeDealias.dealias(gs);
        if (dealiased != c->symbolBeforeDealias) {
            for (auto loc : dealiased.locs(gs)) {
                if (loc.exists()) {
                    documentationLocations.emplace_back(loc);
                }
            }
        }

        typeString = prettyTypeForConstant(gs, c->symbolBeforeDealias);
    } else if (auto d = resp->isMethodDef()) {
        for (auto loc : d->symbol.data(gs)->locs()) {
            if (loc.exists()) {
                documentationLocations.emplace_back(loc);
            }
        }

        typeString =
            core::source_generator::prettyTypeForMethod(gs, d->symbol, nullptr, d->retType.type, nullptr, options);
    } else if (resp->isField()) {
        const auto &origins = resp->getTypeAndOrigins().origins;
        for (auto loc : origins) {
            if (loc.exists()) {
                documentationLocations.emplace_back(loc);
            }
        }

        auto retType = resp->getRetType();
        // Some untyped arguments have null types.
        if (!retType) {
            retType = core::Types::untypedUntracked();
        }
        typeString = retType.showWithMoreInfo(gs);
    } else {
        auto retType = resp->getRetType();
        // Some untyped arguments have null types.
        if (!retType) {
            retType = core::Types::untypedUntracked();
        }
        typeString = retType.showWithMoreInfo(gs);
    }

    // Sort so documentation order is deterministic.
    fast_sort(documentationLocations, [](const auto a, const auto b) -> bool { return a.beginPos() < b.beginPos(); });

    vector<string> documentation;
    for (auto loc : documentationLocations) {
        auto doc = findDocumentation(loc.file().data(gs).source(), loc.beginPos());
        if (doc.has_value() && !doc->empty()) {
            documentation.emplace_back(*doc);
        }
    }
    optional<string> docString;
    if (!documentation.empty()) {
        docString = absl::StrJoin(documentation, "\n\n");
    }

    response->result = make_unique<Hover>(formatRubyMarkup(clientHoverMarkupKind, typeString, docString));
    return response;
}

} // namespace sorbet::realmain::lsp
