#ifndef SORBET_LSP_QUERYRESPONSE
#define SORBET_LSP_QUERYRESPONSE
#include "core/Loc.h"
#include "core/Types.h"

namespace sorbet::core {
class TypeConstraint;
struct QueryResponse {
    enum class Kind { SEND, IDENT, LITERAL, CONSTANT, DEFINITION };

    Kind kind;
    DispatchResult::ComponentVec dispatchComponents;
    std::shared_ptr<TypeConstraint> constraint;
    Loc termLoc; // `termLoc` is the `loc` of the `cfg::Binding` at which this response was generated.
    TypeAndOrigins retType;
    core::NameRef name;
    TypeAndOrigins receiver;

    static void setQueryResponse(core::Context ctx, core::QueryResponse::Kind kind,
                                 core::DispatchResult::ComponentVec dispatchComponents,
                                 const std::shared_ptr<core::TypeConstraint> &constraint, core::Loc termLoc,
                                 core::NameRef name, core::TypeAndOrigins receiver, core::TypeAndOrigins retType);
};

} // namespace sorbet::core
#endif
