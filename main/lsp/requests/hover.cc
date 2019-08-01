#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

string methodSignatureString(const core::GlobalState &gs, const core::TypePtr &retType,
                             const core::DispatchResult &dispatchResult,
                             const unique_ptr<core::TypeConstraint> &constraint) {
    string contents = "";
    auto start = &dispatchResult;
    ;
    while (start != nullptr) {
        auto &dispatchComponent = start->main;
        if (dispatchComponent.method.exists()) {
            if (!contents.empty()) {
                contents += " ";
            }
            contents += methodDetail(gs, dispatchComponent.method, dispatchComponent.receiver, retType, constraint);
        }
        start = start->secondary.get();
    }
    return contents;
}

unique_ptr<MarkupContent> formatRubyCode(MarkupKind markupKind, string_view sigString, string_view docString) {
    string content = "";

    if (docString.length() > 0) {
        absl::StrAppend(&content, docString);
    }

    if (sigString.length() > 0) {
        // Add a newline if docString does not contain one
        if (content.length() > 0 && content.back() != '\n') {
            absl::StrAppend(&content, "\n");
        }
        absl::StrAppend(&content, sigString);
    }

    if (markupKind == MarkupKind::Markdown && content.length() > 0) {
        content = fmt::format("```ruby\n{}\n```", content);
    }

    return make_unique<MarkupContent>(markupKind, move(content));
}

LSPResult LSPLoop::handleTextDocumentHover(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const TextDocumentPositionParams &params) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentHover);
    prodCategoryCounterInc("lsp.messages.processed", "textDocument.hover");

    auto result =
        setupLSPQueryByLoc(move(gs), params.textDocument->uri, *params.position, LSPMethod::TextDocumentHover);
    if (auto run = get_if<TypecheckRun>(&result)) {
        gs = move(run->gs);
        auto &queryResponses = run->responses;
        if (queryResponses.empty()) {
            // Note: Need to specifically specify the variant type here so the null gets placed into the proper slot.
            response->result = variant<JSONNullObject, unique_ptr<Hover>>(JSONNullObject());
            return LSPResult::make(move(gs), move(response));
        }

        // Find documentation for this file
        auto fref = uri2FileRef(params.textDocument->uri);
        ENFORCE(fref.exists());
        auto loc = lspPos2Loc(fref, *params.position, *gs);
        optional<string> optionalDocumentation = findDocumentation(fref.data(*gs).source(), loc->beginPos());
        string documentation = optionalDocumentation.value_or("");


        auto resp = move(queryResponses[0]);
        if (auto sendResp = resp->isSend()) {
            auto retType = sendResp->dispatchResult->returnType;
            auto &constraint = sendResp->dispatchResult->main.constr;
            if (constraint) {
                retType = core::Types::instantiate(core::Context(*gs, core::Symbols::root()), retType, *constraint);
            }
            response->result = make_unique<Hover>(formatRubyCode(
                clientHoverMarkupKind, methodSignatureString(*gs, retType, *sendResp->dispatchResult, constraint), documentation));
        } else if (auto defResp = resp->isDefinition()) {
            response->result = make_unique<Hover>(formatRubyCode(
                clientHoverMarkupKind, methodDetail(*gs, defResp->symbol, nullptr, defResp->retType.type, nullptr), documentation));
        } else if (auto constResp = resp->isConstant()) {
            const auto &data = constResp->symbol.data(*gs);
            auto type = constResp->retType.type;
            if (data->isClass()) {
                auto singletonClass = data->lookupSingletonClass(*gs);
                ENFORCE(singletonClass.exists(), "Every class should have a singleton class by now.");
                type = singletonClass.data(*gs)->externalType(*gs);
            } else if (data->isStaticField() && data->isTypeAlias()) {
                // By wrapping the type in `MetaType`, we display a type alias of `Foo` as `<Type: Foo>` rather than
                // `Foo`.
                type = core::make_type<core::MetaType>(type);
            }
            response->result = make_unique<Hover>(formatRubyCode(clientHoverMarkupKind, type->showWithMoreInfo(*gs), documentation));
        } else {
            response->result =
                make_unique<Hover>(formatRubyCode(clientHoverMarkupKind, resp->getRetType()->showWithMoreInfo(*gs), documentation));
        }
    } else if (auto error = get_if<pair<unique_ptr<ResponseError>, unique_ptr<core::GlobalState>>>(&result)) {
        // An error happened while setting up the query.
        response->error = move(error->first);
        gs = move(error->second);
    } else {
        // Should never happen, but satisfy the compiler.
        ENFORCE(false, "Internal error: setupLSPQueryByLoc returned invalid value.");
    }
    return LSPResult::make(move(gs), move(response));
}
} // namespace sorbet::realmain::lsp
