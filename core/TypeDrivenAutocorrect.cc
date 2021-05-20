#include "core/TypeDrivenAutocorrect.h"

using namespace std;

namespace sorbet::core {

void TypeDrivenAutocorrect::maybeAutocorrect(const GlobalState &gs, ErrorBuilder &e, Loc loc, TypeConstraint &constr,
                                             TypePtr expectedType, TypePtr actualType) {
    if (!loc.exists()) {
        return;
    }

    if (gs.suggestUnsafe.has_value()) {
        e.replaceWith(fmt::format("Wrap in `{}`", *gs.suggestUnsafe), loc, "{}({})", *gs.suggestUnsafe,
                      loc.source(gs).value());
    } else {
        auto withoutNil = Types::approximateSubtract(gs, actualType, Types::nilClass());
        if (!withoutNil.isBottom() &&
            Types::isSubTypeUnderConstraint(gs, constr, withoutNil, expectedType, UntypedMode::AlwaysCompatible)) {
            e.replaceWith("Wrap in `T.must`", loc, "T.must({})", loc.source(gs).value());
        }
    }
}
} // namespace sorbet::core
