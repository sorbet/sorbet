#ifndef SORBET_LSP_QUERYRESPONSE
#define SORBET_LSP_QUERYRESPONSE
#include "core/Loc.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

namespace sorbet {
namespace core {
struct QueryResponse {
    enum class Kind { SEND, IDENT, LITERAL, CONSTANT };

    Kind kind;
    DispatchResult::ComponentVec dispatchComponents;
    std::shared_ptr<TypeConstraint> constraint;
    Loc termLoc; // `termLoc` is the `loc` of the `cfg::Binding` at which this response was generated.
    TypeAndOrigins retType;
};
} // namespace core
} // namespace sorbet
#endif
