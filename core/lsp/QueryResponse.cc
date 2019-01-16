#include "core/lsp/QueryResponse.h"
#include "core/GlobalState.h"
template class std::unique_ptr<sorbet::core::QueryResponse>;

using namespace std;
namespace sorbet::core {

void QueryResponse::pushQueryResponse(core::Context ctx, core::QueryResponse::Kind kind, core::SymbolRef owner,
                                      core::DispatchResult::ComponentVec dispatchComponents,
                                      const shared_ptr<core::TypeConstraint> &constraint, core::Loc termLoc,
                                      core::LocalVariable variable, core::NameRef name, core::TypeAndOrigins receiver,
                                      core::TypeAndOrigins retType) {
    auto queryResponse = make_unique<core::QueryResponse>();
    queryResponse->kind = kind;
    queryResponse->dispatchComponents = std::move(dispatchComponents);
    queryResponse->constraint = constraint;
    queryResponse->termLoc = termLoc;
    queryResponse->retType = retType;
    queryResponse->receiver = receiver;
    queryResponse->variable = variable;
    queryResponse->name = name;
    queryResponse->owner = owner;

    ctx.state.errorQueue->pushQueryResponse(std::move(queryResponse));
}

} // namespace sorbet::core