#include "core/TypeErrorDiagnostics.h"

#include "common/typecase.h"

#include <algorithm>

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

void flattenAndType(vector<TypePtr> &results, const AndType &type) {
    typecase(
        type.left, [&results](const AndType &type) { flattenAndType(results, type); },
        [&results](const TypePtr &def) { results.emplace_back(def); });

    typecase(
        type.right, [&results](const AndType &type) { flattenAndType(results, type); },
        [&results](const TypePtr &def) { results.emplace_back(def); });
}

void flattenOrType(vector<TypePtr> &results, const OrType &type) {
    typecase(
        type.left, [&results](const OrType &type) { flattenOrType(results, type); },
        [&results](const TypePtr &def) { results.emplace_back(def); });

    typecase(
        type.right, [&results](const OrType &type) { flattenOrType(results, type); },
        [&results](const TypePtr &def) { results.emplace_back(def); });
}

void explainCompositeSubtypingFailure(const GlobalState &gs, ErrorBuilder &e, const OrType &composite, const TypePtr &expected) {
    vector<TypePtr> parts;
    flattenOrType(parts, composite);

    const auto numParts = parts.size();
    auto it = std::remove_if(parts.begin(), parts.end(),
                             [&](auto &part) -> bool { return core::Types::isSubType(gs, part, expected); });
    parts.erase(it, parts.end());

    // All the types are not subtypes, maybe the user doesn't need any help here?
    if (parts.size() == numParts) {
        return;
    }

    for (auto &part : parts) {
        e.addErrorNote("`{}` is not a subtype of `{}`", part.show(gs), expected.show(gs));
    }
}

void explainCompositeSubtypingFailure(const GlobalState &gs, ErrorBuilder &e, const TypePtr &got, const AndType &expected) {
    vector<TypePtr> parts;
    flattenAndType(parts, expected);

    const auto numParts = parts.size();
    auto it = std::remove_if(parts.begin(), parts.end(),
                             [&](auto &part) -> bool { return core::Types::isSubType(gs, got, part); });
    parts.erase(it, parts.end());

    // All the types are not subtypes, maybe the user doesn't need any help here?
    if (parts.size() == numParts) {
        return;
    }

    for (auto &part : parts) {
        e.addErrorNote("`{}` is not a subtype of `{}`", got.show(gs), part.show(gs));
    }
}

} // namespace

void TypeErrorDiagnostics::explainTypeMismatch(const GlobalState &gs, ErrorBuilder &e, const TypePtr &expected,
                                               const TypePtr &got) {
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
            "It looks like you're using Sorbet type syntax in a runtime value position.\n"
            "    If you really mean to use types as values, use `{}` to hide the type syntax from the type checker.\n"
            "    Otherwise, you're likely using the type system in a way it wasn't meant to be used.",
            "T::Utils.coerce");
        return;
    }

    if (isa_type<AppliedType>(got) && isa_type<AppliedType>(expected)) {
        auto &gotRef = cast_type_nonnull<AppliedType>(got);
        auto &expectedRef = cast_type_nonnull<AppliedType>(expected);
        if (gotRef.klass != expectedRef.klass) {
            return;
        }
        // We could extend this to multiple args, but the common cases are `T::Set`
        // and `T::Array`, so go with the simple code for now.
        if (gotRef.targs.size() != expectedRef.targs.size() || gotRef.targs.size() != 1) {
            return;
        }

        auto &innerGot = gotRef.targs[0];
        auto &innerExpected = expectedRef.targs[0];

        if (!isa_type<OrType>(innerGot)) {
            return;
        }

        // The idea here is that we got Container[T.any(...)] and we expected
        // Container[$TYPE].  We suspect the `T.any(...)` actually has a number
        // of components, but only a few of them may be wrong; let's try to show
        // the user exactly which ones are incorrect.
        //
        // TODO: should we just limit this to `T.class_of`-style types?
        explainCompositeSubtypingFailure(gs, e, cast_type_nonnull<OrType>(innerGot), innerExpected);
        return;
    }

    if (isa_type<OrType>(got)) {
        explainCompositeSubtypingFailure(gs, e, cast_type_nonnull<OrType>(got), expected);
        return;
    }
    if (isa_type<AndType>(expected)) {
        explainCompositeSubtypingFailure(gs, e, got, cast_type_nonnull<AndType>(expected));
        return;
    }

    // TODO(jez) Add more cases
}

void TypeErrorDiagnostics::maybeAutocorrect(const GlobalState &gs, ErrorBuilder &e, Loc loc, TypeConstraint &constr,
                                            const TypePtr &expectedType, const TypePtr &actualType) {
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
        } else if (isa_type<MetaType>(actualType) && !isa_type<MetaType>(expectedType) &&
                   core::Types::isSubTypeUnderConstraint(gs, constr,
                                                         core::Symbols::T_Types_Base().data(gs)->externalType(),
                                                         expectedType, UntypedMode::AlwaysCompatible)) {
            e.replaceWith("Wrap in `T::Utils.coerce`", loc, "T::Utils.coerce({})", loc.source(gs).value());
        }
    }
}
} // namespace sorbet::core
