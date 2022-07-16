#ifndef SORBET_TYPE_ERROR_DIAGNOSTICS_H
#define SORBET_TYPE_ERROR_DIAGNOSTICS_H

#include "core/Error.h"
#include "core/GlobalState.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

namespace sorbet::core {

class TypeErrorDiagnostics final {
public:
    // Uses heuristics to insert an autocorrect that converts from `expectedType` to `actualType`.
    //
    // `loc` should be the location of the expression to fix up.
    // (It should usually be a cfg::Send::argLocs element or cfg::Return::whatLoc)
    //
    // Statefully accumulates the autocorrect directly onto the provided `ErrorBuilder`.
    static void maybeAutocorrect(const GlobalState &gs, ErrorBuilder &e, Loc loc, TypeConstraint &constr,
                                 const TypePtr &expectedType, const TypePtr &actualType);

    static void explainTypeMismatch(const GlobalState &gs, ErrorBuilder &e, const TypePtr &expected,
                                    const TypePtr &got);

    static void insertUntypedTypeArguments(const GlobalState &gs, ErrorBuilder &e, ClassOrModuleRef klass,
                                           core::Loc replaceLoc);
};

} // namespace sorbet::core

#endif
