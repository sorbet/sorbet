#include "core/TypeErrorDiagnostics.h"
#include "absl/algorithm/container.h"
#include "absl/strings/str_join.h"
#include "core/errors/infer.h"

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

    // TODO(jez) Add more cases
}

void TypeErrorDiagnostics::maybeAutocorrect(const GlobalState &gs, ErrorBuilder &e, Loc loc,
                                            const TypeConstraint &constrOrig, const TypePtr &expectedType,
                                            const TypePtr &actualType) {
    if (!loc.exists()) {
        return;
    }

    if (gs.suggestUnsafe.has_value()) {
        e.replaceWith(fmt::format("Wrap in `{}`", *gs.suggestUnsafe), loc, "{}({})", *gs.suggestUnsafe,
                      loc.source(gs).value());
    } else {
        // Duplicate the current constraint, so that we don't record any additional constraints
        // simply from trying to generate autocorrects, and then solve it so that each additional
        // heuristic here doesn't interfere with later heuristics.
        auto constr = constrOrig.deepCopy();
        if (!constr->solve(gs)) {
            // Constraint already doesn't solve. This will be an error later in infer.
            // For now, let's just give up on our type-driven autocorrects.
            return;
        }

        auto withoutNil = Types::dropNil(gs, actualType);
        if (!withoutNil.isBottom() &&
            Types::isSubTypeUnderConstraint(gs, *constr, withoutNil, expectedType, UntypedMode::AlwaysCompatible)) {
            e.replaceWith("Wrap in `T.must`", loc, "T.must({})", loc.source(gs).value());
        } else if (isa_type<MetaType>(actualType) && !isa_type<MetaType>(expectedType) &&
                   core::Types::isSubTypeUnderConstraint(gs, *constr,
                                                         core::Symbols::T_Types_Base().data(gs)->externalType(),
                                                         expectedType, UntypedMode::AlwaysCompatible)) {
            e.replaceWith("Wrap in `T::Utils.coerce`", loc, "T::Utils.coerce({})", loc.source(gs).value());
        }
    }
}

void TypeErrorDiagnostics::insertUntypedTypeArguments(const GlobalState &gs, ErrorBuilder &e, ClassOrModuleRef klass,
                                                      core::Loc replaceLoc) {
    // if we're looking at `Array`, we want the autocorrect to include `T::`, but we don't need to
    // if we're already looking at `T::Array` instead.
    klass = klass.maybeUnwrapBuiltinGenericForwarder();
    auto typePrefixSym = klass.forwarderForBuiltinGeneric();
    if (!typePrefixSym.exists()) {
        typePrefixSym = klass;
    }

    auto loc = replaceLoc;
    if (loc.exists()) {
        if (klass == core::Symbols::Hash() || klass == core::Symbols::T_Hash()) {
            // Hash is special because it has arity 3 but you're only supposed to write the first 2
            e.replaceWith("Add type arguments", loc, "{}[T.untyped, T.untyped]", typePrefixSym.show(gs));
        } else {
            auto numTypeArgs = klass.data(gs)->typeArity(gs);
            vector<string> untypeds;
            for (int i = 0; i < numTypeArgs; i++) {
                untypeds.emplace_back("T.untyped");
            }
            e.replaceWith("Add type arguments", loc, "{}[{}]", typePrefixSym.show(gs), absl::StrJoin(untypeds, ", "));
        }
    }
}

void TypeErrorDiagnostics::explainUntyped(const GlobalState &gs, ErrorBuilder &e, ErrorClass what,
                                          const TypeAndOrigins &untyped, Loc originForUninitialized) {
    e.addErrorSection(untyped.explainGot(gs, originForUninitialized));
    if (what == core::errors::Infer::UntypedValue) {
        e.addErrorNote("Support for `{}` is minimal. Consider using `{}` instead.", "typed: strong", "typed: strict");
    }
}

void TypeErrorDiagnostics::explainUntyped(const GlobalState &gs, ErrorBuilder &e, ErrorClass what, TypePtr untyped,
                                          Loc origin, Loc originForUninitialized) {
    auto untypedTpo = TypeAndOrigins{untyped, origin};
    e.addErrorSection(untypedTpo.explainGot(gs, originForUninitialized));
    if (what == core::errors::Infer::UntypedValue) {
        e.addErrorNote("Support for `{}` is minimal. Consider using `{}` instead.", "typed: strong", "typed: strict");
    }
}

// dslOwner can be noClassOrModule() to simply unconditionally insert the `dsl` string
// dsl can be `""` to simply insert `extend {dslOwner}` if dslOwner is not already an ancestor
optional<core::AutocorrectSuggestion::Edit> TypeErrorDiagnostics::editForDSLMethod(const Context &ctx,
                                                                                   ClassOrModuleRef inWhatRef,
                                                                                   ClassOrModuleRef dslOwner,
                                                                                   string_view dsl) {
    auto inWhat = inWhatRef.data(ctx);
    auto inWhatSingleton = inWhat->lookupSingletonClass(ctx);

    auto needsDslOwner = false;
    if (dslOwner.exists()) {
        needsDslOwner = !inWhatSingleton.data(ctx)->derivesFrom(ctx, dslOwner);
    }

    auto inCurrentFile = [&](const auto &loc) { return loc.file() == ctx.file; };
    auto &classLocs = inWhat->locs();
    auto classLoc = absl::c_find_if(classLocs, inCurrentFile);

    if (classLoc == classLocs.end()) {
        // Couldn't find a loc for the class in this file; give up.
        // An alternative heuristic here might be "found a file that we know we can write to"
        return nullopt;
    }

    auto [classStart, classEnd] = classLoc->position(ctx);

    core::Loc::Detail thisLineStart = {classStart.line, 1};
    auto thisLineLoc = core::Loc::fromDetails(ctx, classLoc->file(), thisLineStart, thisLineStart);
    ENFORCE(thisLineLoc.has_value());
    auto [_, thisLinePadding] = thisLineLoc.value().findStartOfLine(ctx);

    core::Loc::Detail nextLineStart = {classStart.line + 1, 1};
    auto nextLineLoc = core::Loc::fromDetails(ctx, classLoc->file(), nextLineStart, nextLineStart);
    if (!nextLineLoc.has_value()) {
        return nullopt;
    }
    auto [replacementLoc, nextLinePadding] = nextLineLoc.value().findStartOfLine(ctx);

    // Preserve the indentation of the line below us.
    string prefix(max(thisLinePadding + 2, nextLinePadding), ' ');

    if (needsDslOwner) {
        return core::AutocorrectSuggestion::Edit{
            nextLineLoc.value(),
            fmt::format("{}extend {}\n{}{}\n", prefix, dslOwner.show(ctx), dsl != "" ? prefix : "", dsl),
        };
    } else if (dsl != "") {
        return core::AutocorrectSuggestion::Edit{nextLineLoc.value(), fmt::format("{}{}\n", prefix, dsl)};
    } else {
        return nullopt;
    }
}

} // namespace sorbet::core
