#include "core/TypeErrorDiagnostics.h"

using namespace std;

namespace sorbet::core {

namespace {

[[nodiscard]] bool checkForAttachedClassHint(const GlobalState &gs, ErrorBuilder &e, const SelfTypeParam expected,
                                             const ClassType got) {
    if (expected.definition.name(gs) != Names::Constants::AttachedClass()) {
        return false;
    }

    auto attachedClass = got.symbol.data(gs)->lookupSingletonClass(gs);
    if (!attachedClass.exists()) {
        return false;
    }

    if (attachedClass != expected.definition.owner(gs).asClassOrModuleRef()) {
        return false;
    }

    auto gotStr = got.show(gs);
    auto expectedStr = expected.show(gs);
    e.addErrorNote(
        "`{}` is incompatible with `{}` because when this method is called on a subclass `{}` will represent a more "
        "specific subclass, meaning `{}` will not be specific enough. See https://sorbet.org/docs/attached-class for "
        "more.",
        gotStr, expectedStr, expectedStr, gotStr);
    return true;
}
} // namespace

void TypeErrorDiagnostics::explainTypeMismatch(const GlobalState &gs, ErrorBuilder &e, const TypePtr expected,
                                               const TypePtr got) {
    auto expectedSelfTypeParam = isa_type<SelfTypeParam>(expected);
    auto gotClassType = isa_type<ClassType>(got);
    if (expectedSelfTypeParam && gotClassType) {
        if (checkForAttachedClassHint(gs, e, cast_type_nonnull<SelfTypeParam>(expected),
                                      cast_type_nonnull<ClassType>(got))) {
            return;
        }
    }

    if (isa_type<MetaType>(got) && !isa_type<MetaType>(expected)) {
        e.addErrorNote(
            "It looks like you're using Sorbet type syntax in a runtime value position. If you really mean to reflect "
            "on runtime types as values, use `{}` to hide the type syntax from the type checker. Otherwise, you've "
            "done something that the type system doesn't support.",
            "T::Utils.coerce");
        return;
    }

    // TODO(jez) Add more cases
}

void TypeErrorDiagnostics::maybeAutocorrect(const GlobalState &gs, ErrorBuilder &e, Loc loc, TypeConstraint &constr,
                                            TypePtr expectedType, TypePtr actualType) {
    if (!loc.exists()) {
        return;
    }

    if (gs.suggestUnsafe.has_value()) {
        e.replaceWith(fmt::format("Wrap in `{}`", *gs.suggestUnsafe), loc, "{}({})", *gs.suggestUnsafe,
                      loc.source(gs).value());
    } else {
        auto withoutNil = Types::dropNil(gs, actualType);
        if (!withoutNil.isBottom() &&
            Types::isSubTypeUnderConstraint(gs, constr, withoutNil, expectedType, UntypedMode::AlwaysCompatible)) {
            e.replaceWith("Wrap in `T.must`", loc, "T.must({})", loc.source(gs).value());
        } else if (Types::isSubTypeUnderConstraint(gs, constr, expectedType, Types::Boolean(),
                                                   UntypedMode::AlwaysCompatible)) {
            if (core::isa_type<ClassType>(actualType)) {
                auto classSymbol = core::cast_type_nonnull<ClassType>(actualType).symbol;
                if (classSymbol.exists() && classSymbol.data(gs)->owner == core::Symbols::root() &&
                    classSymbol.data(gs)->name == core::Names::Constants::Boolean()) {
                    e.replaceWith("Prepend `!!`", loc, "!!({})", loc.source(gs).value());
                }
            }
        } else if (isa_type<MetaType>(actualType) && !isa_type<MetaType>(expectedType)) {
            e.replaceWith("Wrap in `T::Utils.coerce`", loc, "T::Utils.coerce({})", loc.source(gs).value());
        }
    }
}
} // namespace sorbet::core
