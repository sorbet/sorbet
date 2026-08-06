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

namespace {

// A comment describing what kind of constant this is, rendered above the constant's type.
// Empty when the type itself already spells out the definition (aliases).
string constantKindHeader(const core::GlobalState &gs, core::SymbolRef constant) {
    if (constant == core::Symbols::StubModule()) {
        return "";
    }

    // Order matters: type aliases and class aliases are also static fields, so they
    // must be checked before isStaticField.
    if (constant.isTypeAlias(gs) || constant.isClassAlias(gs)) {
        // Aliases render as their definition (`Name = Target`, `Name = T.type_alias {...}`),
        // so a kind comment wouldn't add anything.
        return "";
    } else if (constant.isTypeMember()) {
        // Render a skeleton of the enclosing definition so it's clear where the type
        // member lives and how it's declared, e.g.
        //   # class Box
        //   #   Elem = type_member(:out) { {upper: Numeric} }
        //   # end
        auto tmData = constant.asTypeMemberRef().data(gs);
        auto enclosing = constant.owner(gs).asClassOrModuleRef();
        string keyword = "type_member";
        if (enclosing.data(gs)->isSingletonClass(gs)) {
            // A type_template is declared on the singleton class of its enclosing class.
            keyword = "type_template";
            enclosing = enclosing.data(gs)->attachedClass(gs);
        }

        // Variance: `:out` (covariant), `:in` (contravariant), or nothing (invariant).
        string variance;
        if (tmData->flags.isCovariant) {
            variance = "(:out)";
        } else if (tmData->flags.isContravariant) {
            variance = "(:in)";
        }

        // Bounds: `{ {fixed: T} }`, or `lower:`/`upper:` (in canonical autocorrect order)
        // when they aren't the trivial `<bottom>`/`<top>` defaults.
        string bounds;
        if (auto lambdaParam = core::cast_type<core::LambdaParam>(tmData->resultType)) {
            if (tmData->flags.isFixed) {
                bounds = absl::StrCat(" { {fixed: ", lambdaParam->upperBound.show(gs), "} }");
            } else {
                vector<string> parts;
                if (!lambdaParam->lowerBound.isBottom()) {
                    parts.emplace_back(absl::StrCat("lower: ", lambdaParam->lowerBound.show(gs)));
                }
                if (!lambdaParam->upperBound.isTop()) {
                    parts.emplace_back(absl::StrCat("upper: ", lambdaParam->upperBound.show(gs)));
                }
                if (!parts.empty()) {
                    bounds = absl::StrCat(" { {", absl::StrJoin(parts, ", "), "} }");
                }
            }
        }

        auto classOrModule = enclosing.data(gs)->isModule() ? "module" : "class";
        return fmt::format("# {} {}\n#   {} = {}{}{}\n# end", classOrModule, enclosing.show(gs),
                           constant.name(gs).show(gs), keyword, variance, bounds);
    } else if (constant.isClassOrModule()) {
        auto classOrModule = constant.asClassOrModuleRef().data(gs)->isModule() ? "module" : "class";
        return fmt::format("# {} {}", classOrModule, constant.show(gs));
    } else if (constant.isStaticField(gs)) {
        // Reconstruct the definition in the context of its enclosing scope, eliding
        // the value like method hover does for default arguments, e.g.
        //   # class Config
        //   #   TIMEOUT = T.let(…, Integer)
        //   # end
        const auto &resultType = constant.resultType(gs);
        auto type = resultType == nullptr ? core::Types::untyped(constant) : resultType;
        auto definition = fmt::format("{} = T.let(…, {})", constant.name(gs).show(gs), type.show(gs));

        auto owner = constant.owner(gs).asClassOrModuleRef();
        if (owner == core::Symbols::root()) {
            return fmt::format("# {}", definition);
        }
        if (owner.data(gs)->isSingletonClass(gs)) {
            // A static field declared inside `class << self` is owned by the singleton
            // class; render the attached class with a nested `class << self` block.
            auto attached = owner.data(gs)->attachedClass(gs);
            auto classOrModule = attached.data(gs)->isModule() ? "module" : "class";
            return fmt::format("# {} {}\n#   class << self\n#     {}\n#   end\n# end", classOrModule, attached.show(gs),
                               definition);
        }
        auto classOrModule = owner.data(gs)->isModule() ? "module" : "class";
        return fmt::format("# {} {}\n#   {}\n# end", classOrModule, owner.show(gs), definition);
    }
    return "";
}

string prettyConstantForHover(const core::GlobalState &gs, core::SymbolRef constant) {
    auto type = prettyTypeForConstant(gs, constant);

    if (constant != core::Symbols::StubModule() && constant.isTypeAlias(gs)) {
        // Render the alias the way it's written in source, e.g. `X = T.type_alias {Y}`.
        // Only hover does this: `prettyTypeForConstant` is shared with completion, where
        // the name is already shown next to the documentation.
        return fmt::format("{} = {}", constant.name(gs).show(gs), type);
    }

    auto header = constantKindHeader(gs, constant);
    if (header.empty()) {
        return type;
    }
    return fmt::format("{}\n{}", header, type);
}

} // namespace

void handleHoverKeywordArg(const core::GlobalState &gs, const core::lsp::KeywordArgResponse *kw, string &typeString) {
    if (!typeString.empty()) {
        typeString += '\n';
    }
    // nullptr implies no type provided in sig
    auto paramType = kw->paramType == nullptr ? "T.untyped" : kw->paramType.showWithMoreInfo(gs);
    typeString += fmt::format("# {}\n(kwparam) {}: {}", kw->owner.show(gs), kw->paramName.show(gs), paramType);
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

    auto resp = skipLiteralIfMethodDef(gs, queryResponses);
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

        typeString = prettyConstantForHover(gs, c->symbolBeforeDealias);
    } else if (auto d = resp->isMethodDef()) {
        for (auto loc : d->symbol.data(gs)->locs()) {
            if (loc.exists()) {
                documentationLocations.emplace_back(loc);
            }
        }

        typeString = core::source_generator::prettyTypeForMethod(gs, d->symbol, nullptr, options);
    } else if (auto d = resp->isClassDef()) {
        for (auto loc : d->symbol.data(gs)->locs()) {
            if (loc.exists()) {
                documentationLocations.emplace_back(loc);
            }
        }

        auto symData = d->symbol.data(gs);
        auto classOrModule = symData->flags.isModule ? "module" : "class";
        auto superClass = symData->superClass().exists() ? fmt::format("< {}", symData->superClass().show(gs)) : "";
        typeString = fmt::format("{} {}{}", classOrModule, d->symbol.show(gs), superClass);
    } else if (auto f = resp->isField()) {
        const auto &origins = f->retType.origins;
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
    } else if (auto *kw = resp->isKeywordArg()) {
        // Have to do this one separately, because it was stolen out of the queryResponses vector
        handleHoverKeywordArg(gs, kw, typeString);
        // Want to find everything, for the case of methods with multiple dispatch components.
        for (const auto &resp : queryResponses) {
            if (resp == nullptr) {
                continue;
            }
            auto *kw = resp->isKeywordArg();
            if (kw == nullptr) {
                continue;
            }

            handleHoverKeywordArg(gs, kw, typeString);
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

core::packages::Stratum HoverTask::preemptionStratum(FileStratumMapping info) const {
    return info.getStratumForUri(this->params->textDocument->uri);
}

} // namespace sorbet::realmain::lsp
