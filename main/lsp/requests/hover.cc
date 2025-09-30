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

string methodInfoString(const core::GlobalState &gs, const core::DispatchResult &dispatchResult,
                        const core::ShowOptions options) {
    string contents;
    auto start = &dispatchResult;

    while (start != nullptr) {
        auto &component = start->main;
        if (component.method.exists()) {
            if (!contents.empty()) {
                contents += "\n";
            }
            contents = absl::StrCat(
                move(contents), "# ", component.method.show(gs), ":\n",
                core::source_generator::prettyTypeForMethod(gs, component.method, component.receiver, options));
        }
        start = start->secondary.get();
    }

    // contents being empty implies that there were no components that existed, which means that
    // there was an error. We don't show any hover results, so that the only thing that's shown on
    // hover is any relevant diagnostics (e.g., we could show `result type: T.untyped` but for
    // errors that would just be misleading--people might think the problem is _caused_ by untyped,
    // but the untyped is an artifact of how we recover from errors).
    if (!contents.empty()) {
        // Reads from returnType on the overall DispatchResult, which will have aggregated all the
        // components (e.g., unions and intersections)
        contents = absl::StrCat(move(contents), "\n\n# result type:\n", dispatchResult.returnType.showWithMoreInfo(gs));
    }

    return contents;
}

// TODO(jez) There's an opportunity here to use this helper in places like definition.cc to go to the
// definition of keyword arguments.
pair<const core::lsp::SendResponse *, core::NameRef>
enclosingSendForKwarg(const core::GlobalState &gs, const core::lsp::LiteralResponse &l,
                      const vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                      const vector<unique_ptr<core::lsp::QueryResponse>>::iterator respIt) {
    if (!core::isa_type<core::NamedLiteralType>(l.retType.type)) {
        return {nullptr, core::NameRef::noName()};
    }

    auto litType = core::cast_type_nonnull<core::NamedLiteralType>(l.retType.type);
    if (litType.kind != core::NamedLiteralType::Kind::Symbol) {
        return {nullptr, core::NameRef::noName()};
    }

    if (respIt == queryResponses.end() || respIt + 1 == queryResponses.end()) {
        return {nullptr, core::NameRef::noName()};
    }

    auto &nextResp = *(respIt + 1);
    auto send = nextResp->isSend();
    if (send == nullptr) {
        return {nullptr, core::NameRef::noName()};
    }

    auto argLoc =
        absl::c_find_if(send->argLocOffsets, [&](auto loc) { return l.termLoc == core::Loc(send->file, loc); });
    if (argLoc == send->argLocOffsets.end()) {
        return {nullptr, core::NameRef::noName()};
    }

    auto argIdx = distance(send->argLocOffsets.begin(), argLoc);
    if (argIdx < send->numPosArgs) {
        return {nullptr, core::NameRef::noName()};
    }

    return {send, litType.name};
}

string handleHoverKeywordArg(const core::GlobalState &gs, const core::lsp::SendResponse *send,
                             core::NameRef kwargName) {
    string typeString;
    auto curr = send->dispatchResult.get();
    while (curr != nullptr) {
        if (curr->main.method.exists() && !curr->main.receiver.isUntyped()) {
            auto &parameters = curr->main.method.data(gs)->parameters;
            auto param = absl::c_find_if(
                parameters, [&](auto &p) { return p.flags.isKeyword && !p.flags.isRepeated && p.name == kwargName; });
            if (param != parameters.end()) {
                if (!typeString.empty()) {
                    typeString += '\n';
                }
                // nullptr implies no type provided in sig
                auto paramType = param->type == nullptr ? "T.untyped" : param->type.showWithMoreInfo(gs);
                typeString += fmt::format("# {}\n(kwparam) {}: {}", curr->main.method.show(gs),
                                          param->parameterName(gs), paramType);
            }
        }
        curr = curr->secondary.get();
    }

    return typeString;
}

HoverTask::HoverTask(const LSPConfiguration &config, MessageId id, unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentHover), params(move(params)) {}

unique_ptr<ResponseMessage> HoverTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentHover);

    const core::GlobalState &gs = typechecker.state();
    const auto &uri = params->textDocument->uri;
    auto result = LSPQuery::byLoc(config, typechecker, uri, *params->position, LSPMethod::TextDocumentHover, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto fref = config.uri2FileRef(gs, uri);
    // LSPQuery::byLoc reports an error if the file or loc don't exist
    auto queryLoc = params->position->toLoc(gs, fref).value();

    auto &queryResponses = result.responses;
    auto clientHoverMarkupKind = config.getClientConfig().clientHoverMarkupKind;
    if (queryResponses.empty()) {
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

    auto respIt = skipLiteralIfMethodDef(gs, queryResponses);
    auto resp = move(*respIt);
    auto options = core::ShowOptions();
    vector<core::Loc> documentationLocations;
    string typeString;

    if (auto s = resp->isSend()) {
        // Don't want to show hover results if we're hovering over, e.g., the arguments, and there's nothing there.
        if (s->funLoc().exists() && s->funLoc().contains(queryLoc)) {
            auto start = s->dispatchResult.get();
            while (start != nullptr) {
                if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                    auto loc = start->main.method.data(gs)->loc();
                    if (loc.exists()) {
                        documentationLocations.emplace_back(loc);
                    }
                }
                start = start->secondary.get();
            }

            if (s->dispatchResult->main.method.exists() &&
                s->dispatchResult->main.method.data(gs)->owner == core::Symbols::MagicSingleton()) {
                // Most <Magic>.<foo> are not meant to be exposed to the user. Instead, just show
                // the result type.
                typeString = s->dispatchResult->returnType.showWithMoreInfo(gs);
            } else {
                typeString = methodInfoString(gs, *s->dispatchResult, options);
            }
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

        typeString = core::source_generator::prettyTypeForMethod(gs, d->symbol, nullptr, options);
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
    } else if (auto l = resp->isLiteral()) {
        auto [send, kwargName] = enclosingSendForKwarg(gs, *l, queryResponses, respIt);
        if (send != nullptr) {
            typeString = handleHoverKeywordArg(gs, send, kwargName);
        }

        if (typeString.empty()) {
            // fallback, in case it wasn't a keyword arg
            typeString = resp->getRetType().showWithMoreInfo(gs);
        }
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
