#include "core/lsp/QueryResponse.h"

namespace sorbet {
namespace core {

void QueryResponse::setQueryResponse(core::Context ctx, core::QueryResponse::Kind kind,
                                     core::DispatchResult::ComponentVec dispatchComponents,
                                     const std::shared_ptr<core::TypeConstraint> &constraint, core::Loc termLoc,
                                     core::NameRef name, core::TypeAndOrigins receiver, core::TypeAndOrigins retType) {
    auto queryResponse = std::make_unique<core::QueryResponse>();
    queryResponse->kind = kind;
    queryResponse->dispatchComponents = std::move(dispatchComponents);
    queryResponse->constraint = constraint;
    queryResponse->termLoc = termLoc;
    queryResponse->retType = retType;
    queryResponse->receiver = receiver;
    queryResponse->name = name;

    ctx.state.errorQueue->pushQueryResponse(std::move(queryResponse));
}

} // namespace core
} // namespace sorbet
