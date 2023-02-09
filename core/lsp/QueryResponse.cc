#include "core/lsp/QueryResponse.h"
#include "core/ErrorQueue.h"
#include "core/GlobalState.h"

template class std::unique_ptr<sorbet::core::lsp::QueryResponse>;

using namespace std;
namespace sorbet::core::lsp {

void QueryResponse::pushQueryResponse(core::Context ctx, QueryResponseVariant response) {
    ctx.state.errorQueue->pushQueryResponse(ctx.file, make_unique<QueryResponse>(std::move(response)));
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

const ConstantResponse *QueryResponse::isConstant() const {
    return get_if<ConstantResponse>(&response);
}

const FieldResponse *QueryResponse::isField() const {
    return get_if<FieldResponse>(&response);
}

const MethodDefResponse *QueryResponse::isMethodDef() const {
    return get_if<MethodDefResponse>(&response);
}

const EditResponse *QueryResponse::isEdit() const {
    return get_if<EditResponse>(&response);
}

core::Loc QueryResponse::getLoc() const {
    if (auto ident = isIdent()) {
        return ident->termLoc;
    } else if (auto send = isSend()) {
        return send->termLoc();
    } else if (auto literal = isLiteral()) {
        return literal->termLoc;
    } else if (auto constant = isConstant()) {
        return constant->termLoc;
    } else if (auto field = isField()) {
        return field->termLoc;
    } else if (auto def = isMethodDef()) {
        return def->termLoc;
    } else if (auto edit = isEdit()) {
        return edit->loc;
    } else {
        return core::Loc::none();
    }
}

core::TypePtr QueryResponse::getRetType() const {
    if (auto ident = isIdent()) {
        return ident->retType.type;
    } else if (auto send = isSend()) {
        return send->dispatchResult->returnType;
    } else if (auto literal = isLiteral()) {
        return literal->retType.type;
    } else if (auto constant = isConstant()) {
        return constant->retType.type;
    } else if (auto field = isField()) {
        return field->retType.type;
    } else if (auto def = isMethodDef()) {
        return def->retType.type;
    } else {
        // Should never happen, as the above checks should be exhaustive.
        Exception::raise("QueryResponse is of type that does not have retType.");
    }
}

const core::TypeAndOrigins &QueryResponse::getTypeAndOrigins() const {
    if (auto ident = isIdent()) {
        return ident->retType;
    } else if (auto literal = isLiteral()) {
        return literal->retType;
    } else if (auto constant = isConstant()) {
        return constant->retType;
    } else if (auto field = isField()) {
        return field->retType;
    } else if (auto def = isMethodDef()) {
        return def->retType;
    } else {
        Exception::raise("QueryResponse is of type that does not have TypeAndOrigins.");
    }
}

} // namespace sorbet::core::lsp
