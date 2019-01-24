#include "core/lsp/QueryResponse.h"
#include "core/GlobalState.h"
template class std::unique_ptr<sorbet::core::lsp::QueryResponse>;

using namespace std;
namespace sorbet::core::lsp {

SendResponse::SendResponse(core::DispatchResult::ComponentVec dispatchComponents,
                           const std::shared_ptr<core::TypeConstraint> &constraint, core::Loc termLoc,
                           core::NameRef name, core::TypeAndOrigins receiver, core::TypeAndOrigins retType)
    : dispatchComponents(std::move(dispatchComponents)), constraint(constraint), termLoc(termLoc), name(name),
      receiver(receiver), retType(retType) {}

IdentResponse::IdentResponse(core::SymbolRef owner, core::Loc termLoc, core::LocalVariable variable,
                             core::TypeAndOrigins retType)
    : owner(owner), termLoc(termLoc), variable(variable), retType(retType) {}

LiteralResponse::LiteralResponse(core::SymbolRef owner, core::Loc termLoc, core::TypeAndOrigins retType)
    : owner(owner), termLoc(termLoc), retType(retType) {}

ConstantResponse::ConstantResponse(core::SymbolRef owner, core::DispatchResult::ComponentVec dispatchComponents,
                                   core::Loc termLoc, core::NameRef name, core::TypeAndOrigins receiver,
                                   core::TypeAndOrigins retType)
    : owner(owner), dispatchComponents(std::move(dispatchComponents)), termLoc(termLoc), name(name), receiver(receiver),
      retType(retType) {}

DefinitionResponse::DefinitionResponse(core::DispatchResult::ComponentVec dispatchComponents, core::Loc termLoc,
                                       core::NameRef name, core::TypeAndOrigins retType)
    : dispatchComponents(std::move(dispatchComponents)), termLoc(termLoc), name(name), retType(retType) {}

void QueryResponse::pushQueryResponse(core::Context ctx, QueryResponseVariant response) {
    ctx.state.errorQueue->pushQueryResponse(make_unique<QueryResponse>(std::move(response)));
}

QueryResponse::QueryResponse(QueryResponseVariant response) : response(std::move(response)) {}

const SendResponse *QueryResponse::isSend() const {
    return get_if<SendResponse>(&response);
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

const DefinitionResponse *QueryResponse::isDefinition() const {
    return get_if<DefinitionResponse>(&response);
}

core::Loc QueryResponse::getLoc() const {
    if (auto ident = isIdent()) {
        return ident->termLoc;
    } else if (auto send = isSend()) {
        return send->termLoc;
    } else if (auto literal = isLiteral()) {
        return literal->termLoc;
    } else if (auto constant = isConstant()) {
        return constant->termLoc;
    } else if (auto def = isDefinition()) {
        return def->termLoc;
    } else {
        return core::Loc::none();
    }
}

core::TypePtr QueryResponse::getRetType() const {
    if (auto ident = isIdent()) {
        return ident->retType.type;
    } else if (auto send = isSend()) {
        return send->retType.type;
    } else if (auto literal = isLiteral()) {
        return literal->retType.type;
    } else if (auto constant = isConstant()) {
        return constant->retType.type;
    } else if (auto def = isDefinition()) {
        return def->retType.type;
    } else {
        return core::TypePtr();
    }
}

const core::DispatchResult::ComponentVec emptyDispatchComponents;

const core::DispatchResult::ComponentVec &QueryResponse::getDispatchComponents() const {
    if (auto send = get_if<SendResponse>(&response)) {
        return send->dispatchComponents;
    } else if (auto constant = get_if<ConstantResponse>(&response)) {
        return constant->dispatchComponents;
    } else if (auto def = get_if<DefinitionResponse>(&response)) {
        return def->dispatchComponents;
    } else {
        return emptyDispatchComponents;
    }
}

} // namespace sorbet::core::lsp