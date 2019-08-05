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

unique_ptr<MarkupContent> formatRubyCode(MarkupKind markupKind, string_view sigString,
                                         optional<string_view> optionalDocString) {
    string_view docString = optionalDocString.value_or("");
    // Get rid of at most 1 trailing newline
    if (docString.length() > 0 && docString.back() == '\n') {
        docString.remove_suffix(1);
    }
    if (sigString.length() > 0 && sigString.back() == '\n') {
        sigString.remove_suffix(1);
    }

    string content = "";

    // Add docs
    absl::StrAppend(&content, docString);
    if (docString.length() > 0)
        absl::StrAppend(&content, "\n");

    // Add sig
    if (markupKind == MarkupKind::Markdown && sigString.length() > 0) {
        sigString = fmt::format("```ruby\n{}\n```", sigString);
    }
    absl::StrAppend(&content, sigString);

    // Add newline at the end if needed
    if (content.length() > 0 && content.back() == '\n')
        absl::StrAppend(&content, "\n");

    return make_unique<MarkupContent>(markupKind, move(content));
}

LSPResult LSPLoop::handleTextDocumentHover(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const TextDocumentPositionParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentHover);
    prodCategoryCounterInc("lsp.messages.processed", "textDocument.hover");

    auto result =
        setupLSPQueryByLoc(move(gs), params.textDocument->uri, *params.position, LSPMethod::TextDocumentHover);
    gs = move(result.gs);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        auto &queryResponses = result.responses;
        if (queryResponses.empty()) {
            // Note: Need to specifically specify the variant type here so the null gets placed into the proper slot.
            response->result = variant<JSONNullObject, unique_ptr<Hover>>(JSONNullObject());
            return LSPResult::make(move(gs), move(response));
        }

        auto resp = move(queryResponses[0]);
        if (auto sendResp = resp->isSend()) {
            auto retType = sendResp->dispatchResult->returnType;
            // main.method is <none> when calling .new on a class.
            auto methodDefLoc = sendResp->dispatchResult->main.method.data(*gs)->loc();
            auto &constraint = sendResp->dispatchResult->main.constr;
            if (constraint) {
                retType = core::Types::instantiate(core::Context(*gs, core::Symbols::root()), retType, *constraint);
            }
            response->result = make_unique<Hover>(formatRubyCode(
                clientHoverMarkupKind, methodSignatureString(*gs, retType, *sendResp->dispatchResult, constraint),
                findDocumentation(methodDefLoc.file().data(*gs).source(), methodDefLoc.beginPos())));
        } else if (auto defResp = resp->isDefinition()) {
            response->result = make_unique<Hover>(formatRubyCode(
                clientHoverMarkupKind, methodDetail(*gs, defResp->symbol, nullptr, defResp->retType.type, nullptr),
                findDocumentation(defResp->termLoc.file().data(*gs).source(), defResp->termLoc.beginPos())));
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
            response->result = make_unique<Hover>(formatRubyCode(
                clientHoverMarkupKind, type->showWithMoreInfo(*gs),
                findDocumentation(constResp->termLoc.file().data(*gs).source(), constResp->termLoc.beginPos())));
        } else {
            response->result = make_unique<Hover>(
                formatRubyCode(clientHoverMarkupKind, resp->getRetType()->showWithMoreInfo(*gs), ""));
        }
    }
    return LSPResult::make(move(gs), move(response));
}
} // namespace sorbet::realmain::lsp
