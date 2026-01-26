#include "core/lsp/QueryResponse.h"
#include "core/ErrorQueue.h"
#include "core/GlobalState.h"

template class std::unique_ptr<sorbet::core::lsp::QueryResponse>;

using namespace std;
namespace sorbet::core::lsp {

void QueryResponse::pushQueryResponse(core::Context ctx, QueryResponseVariant response) {
    ctx.state.errorQueue->pushQueryResponse(ctx.file, make_unique<QueryResponse>(std::move(response)));
}

void QueryResponse::pushQueryResponse(const GlobalState &gs, FileRef file, QueryResponseVariant response) {
    gs.errorQueue->pushQueryResponse(file, make_unique<QueryResponse>(std::move(response)));
}

QueryResponse::QueryResponse(QueryResponseVariant response) : response(std::move(response)) {}

const SendResponse *QueryResponse::isSend() const {
    return get_if<SendResponse>(&response);
}

const optional<core::Loc> SendResponse::getMethodNameLoc(const core::GlobalState &gs) const {
    auto existsNonEmpty = (this->funLocOffsets.exists() && !this->funLocOffsets.empty());
    return existsNonEmpty ? make_optional<core::Loc>(this->funLoc()) : nullopt;
}

const IdentResponse *QueryResponse::isIdent() const {
    return get_if<IdentResponse>(&response);
}

const LiteralResponse *QueryResponse::isLiteral() const {
    return get_if<LiteralResponse>(&response);
}

const KeywordArgResponse *QueryResponse::isKeywordArg() const {
    return get_if<KeywordArgResponse>(&response);
}

const ConstantResponse *QueryResponse::isConstant() const {
    return get_if<ConstantResponse>(&response);
}

const FieldResponse *QueryResponse::isField() const {
    return get_if<FieldResponse>(&response);
}

const MethodDefResponse *QueryResponse::isMethodDef() const {
    return get_if<MethodDefResponse>(&response);
}

const ClassDefResponse *QueryResponse::isClassDef() const {
    return get_if<ClassDefResponse>(&response);
}

const EditResponse *QueryResponse::isEdit() const {
    return get_if<EditResponse>(&response);
}

core::Loc QueryResponse::getLoc() const {
    return visit(
        [](auto &&res) -> core::Loc {
            using T = decay_t<decltype(res)>;
            if constexpr (is_same_v<T, IdentResponse>) {
                return res.termLoc;
            } else if constexpr (is_same_v<T, SendResponse>) {
                return res.termLoc();
            } else if constexpr (is_same_v<T, LiteralResponse>) {
                return res.termLoc;
            } else if constexpr (is_same_v<T, KeywordArgResponse>) {
                return res.termLoc;
            } else if constexpr (is_same_v<T, ConstantResponse>) {
                return res.termLoc;
            } else if constexpr (is_same_v<T, FieldResponse>) {
                return res.termLoc;
            } else if constexpr (is_same_v<T, MethodDefResponse>) {
                return res.declLoc; // TODO(jez) Maybe we should have this be termLoc?
            } else if constexpr (is_same_v<T, ClassDefResponse>) {
                return res.termLoc;
            } else if constexpr (is_same_v<T, EditResponse>) {
                return res.loc;
            } else {
                static_assert(always_false_v<T>, "Should never happen, as the above checks should be exhaustive.");
            }
        },
        this->response);
}

core::TypePtr QueryResponse::getRetType() const {
    // TODO(jez) C++26: Convert this to variant::visit instance method
    return visit(
        [](auto &&res) -> core::TypePtr {
            using T = decay_t<decltype(res)>;
            if constexpr (is_same_v<T, IdentResponse>) {
                return res.retType.type;
            } else if constexpr (is_same_v<T, SendResponse>) {
                return res.dispatchResult->returnType;
            } else if constexpr (is_same_v<T, LiteralResponse>) {
                return res.retType.type;
            } else if constexpr (is_same_v<T, KeywordArgResponse>) {
                return res.paramType;
            } else if constexpr (is_same_v<T, ConstantResponse>) {
                return res.retType.type;
            } else if constexpr (is_same_v<T, FieldResponse>) {
                return res.retType.type;
            } else if constexpr (is_same_v<T, MethodDefResponse>) {
                return res.retType.type;
            } else if constexpr (is_same_v<T, ClassDefResponse>) {
                Exception::raise("QueryResponse is of type that does not have retType");
            } else if constexpr (is_same_v<T, EditResponse>) {
                Exception::raise("QueryResponse is of type that does not have retType.");
            } else {
                static_assert(always_false_v<T>, "Should never happen, as the above checks should be exhaustive.");
            }
        },
        this->response);
}

} // namespace sorbet::core::lsp
