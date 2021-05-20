#include "core/TypeDrivenAutocorrect.h"

using namespace std;

namespace sorbet::core {

void typeDrivenAutocorrect(const core::GlobalState &gs, core::ErrorBuilder &e, core::Loc loc,
                           core::TypeConstraint &constr, core::TypePtr expectedType, core::TypePtr actualType) {
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
