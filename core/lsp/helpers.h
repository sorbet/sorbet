#ifndef SORBET_CORE_LSP_HELPERS_H
#define SORBET_CORE_LSP_HELPERS_H

#include "core/core.h"

namespace sorbet::core::lsp {

core::TypePtr getResultType(const core::GlobalState &gs, const core::TypePtr &type, core::SymbolRef inWhat,
                            core::TypePtr receiver, const core::TypeConstraint *constr);

std::string prettyTypeForMethod(const core::GlobalState &gs, core::MethodRef method, const core::TypePtr &receiver,
                                const core::TypePtr &retType, const core::TypeConstraint *constraint);

} // namespace sorbet::core::lsp
#endif // SORBET_CORE_LSP_HELPERS_H
